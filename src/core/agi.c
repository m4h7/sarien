/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id: agi.c,v 1.44 2001/10/31 00:27:25 cmatsuoka Exp $
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "sarien.h"
#include "agi.h"
#include "opcodes.h"
#include "keyboard.h"
#include "rand.h"
#include "menu.h"
#include "savegame.h"

static struct agi_loader *loader;		/* loader */


#ifndef PALMOS
extern struct agi_loader agi_v2;
extern struct agi_loader agi_v3;
#else
extern struct agi_loader agi_v4;
#endif


static void init_pri_bands ()
{
	int i, p, y = 0;

#if 0
	if (*game.pri_table != 0xff)
		return;
#endif

	for (p = 1; p < 15; p++) {
		for (i = 0; i < 12; i++) {
			game.pri_table[y++] = p < 4 ? 4 : p;
		}
	}
}

int agi_init ()
{
	int ec, i;

	_D("initializing");

	/* reset all flags to false and all variables to 0 */
	for (i = 0; i < MAX_FLAGS; i++)
		setflag (i, 0);
	for (i = 0; i < MAX_VARS; i++)
		setvar(i, 0);

	/* clear all resources and events */
	memset (&game.views, 0, MAX_DIRS * sizeof (struct agi_view));
	memset (&game.pictures, 0, MAX_DIRS * sizeof (struct agi_picture));
	memset (&game.logics, 0, MAX_DIRS * sizeof (struct agi_logic));
	memset (&game.sounds, 0, MAX_DIRS * sizeof (struct agi_sound));
	memset (&game.ev_scan, 0, MAX_DIRS * sizeof (struct agi_event));
	memset (&game.ev_keyp, 0, MAX_DIRS * sizeof (struct agi_event));

	init_words ();
	set_rnd_seed ();
	init_menus ();
	init_pri_bands ();

	/* clear string buffer */
	for (i = 0; i < MAX_WORDS1; i++)
		game.strings[i][0] = 0;

	/* setup emulation */

	report ("Emulating Sierra AGI v");
	switch (loader->int_version >> 12) {
	case 2:
		report ("%x.%03x\n",
			(int)(loader->int_version >> 12) & 0xF,
			(int)(loader->int_version) & 0xFFF);
		break;
	case 3:
		report ("%x.002.%03x\n",
			(int)(loader->int_version >> 12) & 0xF,
			(int)(loader->int_version) & 0xFFF);
		break;
	}

	game.game_flags |= opt.amiga ? ID_AMIGA : 0;
	game.game_flags |= opt.agds ? ID_AGDS : 0;

	if (game.game_flags & ID_AMIGA)
		report ("Amiga padded game detected.\n");

	if (game.game_flags & ID_AGDS)
		report ("AGDS mode enabled.\n");

	ec = loader->init ();		/* load vol files, etc */

	if (ec == err_OK)
		ec = loader->load_objects(OBJECTS);

	/* CM: ec= commented out, demogs has no words.tok */
	if(ec == err_OK)
		/*ec =*/ loader->load_words(WORDS);

	/* FIXME: load IIgs instruments and samples */
	/* load_instruments("kq.sys16"); */

	/* Load logic 0 into memory, set cache flag for logic 0 */
	if (ec == err_OK) {
		ec = loader->load_resource (rLOGIC, 0);
		game.dir_logic[0].flags |= RES_CACHED;	/* keep this one cached */
	}

	/* if cached, enable caching options */
	if (opt.cache) {
		for (i = 0; i < MAX_DIRS; i++) {
			game.dir_logic[i].flags |= RES_CACHED;
			game.dir_pic[i].flags |= RES_CACHED;
			game.dir_view[i].flags |= RES_CACHED;
			game.dir_sound[i].flags |= RES_CACHED;
		}
	}

	/* if forced, load all cacheable objects */
	if (opt.forceload && ec == err_OK) {
		for(i = 0; i < MAX_DIRS; i++) {
			loader->load_resource (rLOGIC, i);
			loader->load_resource (rPICTURE, i);
			loader->load_resource (rVIEW, i);
		}
		printf("\n");
	}

	return ec;
}


/* unload all resources */
static void unload_resources ()
{
	int i;

	for(i = 0; i < MAX_DIRS; i++) {
		game.dir_view[i].flags &= ~RES_CACHED;	/* clear cache flag */
		loader->unload_resource (rVIEW, i);	/* free view */

		game.dir_pic[i].flags &= ~RES_CACHED;	/* clear cache flag */
		loader->unload_resource (rPICTURE, i);	/* free resource */

		game.dir_logic[i].flags &= ~RES_CACHED;	/* clear cache flag */
		loader->unload_resource (rLOGIC, i);	/* free resource */

		game.dir_sound[i].flags &= ~RES_CACHED;	/* clear cache flag */
		loader->unload_resource (rSOUND, i);	/* free resource */
	}
}


int agi_deinit ()
{
	int ec;

	clean_input ();			/* remove all words from memory */
	deinit_menus ();		/* unload the menus */
	unload_resources ();		/* unload resources in memory */
	ec = loader->deinit ();
	unload_objects();
	unload_words();

	clear_image_stack();

	return ec;
}


int agi_detect_game (char *gn)
{
	int ec = err_OK;

	_D ("(gn = %s)", gn);
	if (gn == NULL)		/* assume current directory */
		gn = get_current_directory ();

#ifndef PALMOS
	loader = &agi_v2;
	ec = loader->detect_game (gn);

	if (ec != err_OK) {
		loader = &agi_v3;
		ec = loader->detect_game (gn);
	}

#else
	loader = &agi_v4;
	ec = loader->detect_game(gn);
#endif

	return ec;
}


int agi_version ()
{
	return loader->version;
}


int agi_get_release ()
{
	return loader->int_version;
}


void agi_set_release (int n)
{
	loader->int_version = n;
}


int agi_load_resource (int r, int n)
{
	int i;

	i = loader->load_resource (r, n);
#ifdef DISABLE_COPYPROTECTION
	if (r == rLOGIC)
		patch_logic (n);
#endif

	return i;
}


int agi_unload_resource (int r, int n)
{
	return loader->unload_resource (r, n);
}


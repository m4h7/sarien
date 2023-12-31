/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id: winmain.cpp,v 1.3.2.1 2001/11/10 12:13:55 cmatsuoka Exp $
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

/*
 * Massively modified by Vasyl Tsvirkunov <vasyl@pacbell.net> for Pocket PC/WinCE port
 */

#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include "sarien.h"
#include "agi.h"
#include "text.h"
#include "graphics.h"
#include "browse.h"

#ifndef _TRACE
void _D (char *s, ...) { }
#endif


volatile UINT32 clock_ticks;
volatile UINT32 clock_count;

//extern "C" UINT8 *font, font_english[];

struct sarien_options opt;
struct game_id_list game_info;
struct agi_game game;


static bool open_file (HINSTANCE hThisInst, char *s)
{
	TCHAR buffer[MAX_PATH];
	EXTIMAGE ei[] =
	{
		{ TEXT(""), 1 },
		{ TEXT("0"), 1 },
		{ TEXT("1"), 1 },
		{ TEXT("2"), 1 },
		{ TEXT("3"), 1 },
		{ TEXT("4"), 1 },
		{ TEXT("5"), 1 },
		{ TEXT("TOK"), 1 },
		{ NULL, 0 }
	};
	BROWSEFILES bf;
	bf.szCaption = TEXT("Open game");
	bf.dwFlags = BF_SELECTEXISTING;
	bf.nTemplateId = 1000;
	bf.szStartDir = TEXT(".");
	bf.szDefExt = TEXT("sav");
	bf.pBuffer = buffer;
	bf.nBufSize = MAX_PATH;
	bf.pBufferShort = NULL;
	bf.nBufShortSize = MAX_PATH;
	bf.nImageId = 1000;
	bf.pExtImages = ei;
	if(BrowseFiles(&bf))
	{
		WideCharToMultiByte(CP_ACP, 0, buffer, -1, s, MAX_PATH, NULL, NULL);
		char* lastdir = strrchr(s, '\\');
		if(lastdir)
			*lastdir = 0;
		return true;
	}
	else
		return false;
}

int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPWSTR lpszArgs, int nWinMode)
{
	int ec = err_OK;
	LPTSTR c_unc;
	char c_asc[1024];
	char* c, filename[MAX_PATH];
	
	game.clock_enabled = FALSE;
	game.state = STATE_INIT;

	c_unc = GetCommandLine();
	WideCharToMultiByte(CP_ACP, 0, c_unc, -1, c_asc, 1024, NULL, NULL);

	filename[0] = 0;
	c = c_asc;
	if(*c)
	{
		int l;
		if(*c == '\"')
		{
			for(c++; *c && *c != '\"'; c++) ;
			if(*c)
				c++;
		}
		else
			for(; *c && *c != ' '; c++) ; 
		
		for(; *c && *c == ' '; c++) ;

		if(*c == '\"')
		{
			c ++;
			if(*c)
			{
				l = strlen(c);
				if(c[l-1] == '\"')
					c[l-1] = 0;
			}
		}
		l = strlen(c);
		if(l > 4 && c[l-4] == '.' && tolower(c[l-3]) == 'o' && tolower(c[l-2]) == 'v' && tolower(c[l-1]) == 'l')
		{
			for(l--; l>=0 && c[l] != '\\' && c[l] != '/'; l--);
			if(l>=0)
				c[l] = 0;
		}
		strcpy(filename, c);
	}

	if(!filename[0] && !open_file (hThisInst, filename))
		exit(0);

	init_machine (1, 0);

	game.color_fg = 15;
	game.color_bg = 0;

//	font = font_english;

	if (init_video () != err_OK) {
		ec = err_Unk;
		goto bail_out;
	}
	console_init ();
	report ("--- Starting console ---\n\n");
	if (!opt.gfxhacks)
		report ("Graphics driver hacks disabled (if any)\n");

	if (	agi_detect_game (filename) == err_OK ||
		agi_detect_game (get_current_directory ()) == err_OK)
	{
		game.state = STATE_LOADED;
	}
	else
		goto bail_out;

	init_sound ();

	report (" \nSarien " VERSION " is ready.\n");
	if (game.state < STATE_LOADED) {
       		console_prompt ();
		do { main_cycle (); } while (game.state < STATE_RUNNING);
	}

	/* Execute the game */
    	do {
		if (game.state < STATE_RUNNING) {
    			ec = agi_init ();
			game.state = STATE_RUNNING;
		}

		if (ec == err_OK) {
   			/* setup machine specific AGI flags, etc */
    			setvar (V_computer, 0);	/* IBM PC */
    			setvar (V_soundgen, 1);	/* IBM PC SOUND */
    			setvar (V_max_input_chars, 38);
    			setvar (V_monitor, 0x3); /* EGA monitor */
   			game.horizon = HORIZON;
			game.player_control = FALSE;

			ec = run_game();
    		}

    		/* deinit our resources */
    		agi_deinit ();
			game.state = STATE_INIT; /* otherwise it will crash on restart */
    	} while (ec == err_RestartGame || game.state == STATE_RUNNING);

	deinit_sound ();
	deinit_video ();

bail_out:
	if (ec == err_OK || ec == err_DoNothing) {
		deinit_machine ();
		exit (ec);
	}

#if 0
	printf ("Error %04i: ", ec);

	switch (ec) {
	case err_BadCLISwitch:
		printf("Bad CLI switch.\n");
		break;
	case err_InvalidAGIFile:
		printf("Invalid or inexistent AGI file.\n");
		break;
	case err_BadFileOpen:
		printf("Unable to open file.\n");
		break;
	case err_NotEnoughMemory:
		printf("Not enough memory.\n");
		break;
	case err_BadResource:
		printf("Error in resource.\n");
		break;
	case err_UnknownAGIVersion:
		printf("Unknown AGI version.\n");
		break;
	case err_NoGameList:
		printf("No game ID List was found!\n");
		break;
	}
	printf("\nUse parameter -h to list the command line options\n");
#endif

	deinit_machine ();

	return ec;
}


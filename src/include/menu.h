/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id: menu.h,v 1.5 2001/05/07 03:51:03 cmatsuoka Exp $
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_MENU_H
#define __AGI_MENU_H

#ifdef __cplusplus
extern "C"{
#endif

void	init_menus	(void);
void	deinit_menus	(void);
void	add_menu	(char *);
void	add_menu_item	(char *, int);
void	submit_menu	(void);
void	menu_set_item	(int, int);
int	menu_keyhandler	(int);

#ifdef __cplusplus
};
#endif

#endif

/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id: sprite.h,v 1.6 2001/07/29 16:39:23 cmatsuoka Exp $
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_SPRITE_H
#define __AGI_SPRITE_H

void	erase_upd_sprites	(void);
void	erase_nonupd_sprites	(void);
void	erase_both		(void);
void	blit_upd_sprites	(void);
void	blit_nonupd_sprites	(void);
void	blit_both		(void);
void	commit_upd_sprites	(void);
void	commit_nonupd_sprites	(void);
void	commit_both		(void);
void	add_to_pic		(int, int, int, int, int, int, int);
void	show_obj		(int);
void	commit_block		(int, int, int, int);

#endif /* __AGI_SPRITE_H */

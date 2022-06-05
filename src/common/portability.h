/* $Id: portability.h,v 5.4 2002/01/27 12:41:15 bertg Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Include portability related stuff in one file.
 */
#ifndef PORTABILITY_H_INCLUDED
#define PORTABILITY_H_INCLUDED

/*
 * Macros to block out Windows only code (and never Windows code)
 */
#ifndef _WINDOWS
#define IFNWINDOWS(x)	x
#else
#define IFNWINDOWS(x)
#endif


#ifdef _WINDOWS
#define PATHNAME_SEP    '\\'
#else
#define PATHNAME_SEP    '/'
#endif


/*
 * Prototypes for OS function wrappers in portability.c.
 */
extern int Get_process_id(void);	/* getpid */
extern void Get_login_name(char *buf, int size);

#endif /* PORTABILITY_H_INCLUDED */

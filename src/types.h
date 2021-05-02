/* $Id: types.h,v 3.14 1995/03/04 20:11:12 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
 *
 *      Bjørn Stabell        (bjoerns@staff.cs.uit.no)
 *      Ken Ronny Schouten   (kenrsc@stud.cs.uit.no)
 *      Bert Gÿsbers         (bert@mc.bio.uva.nl)
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

#ifndef	TYPES_H
#define	TYPES_H

#ifdef VMS
#include <types.h>
#else
#include <sys/types.h>
#endif

#ifdef VMS
typedef char byte;
#else
typedef signed char	byte;
#endif
typedef unsigned char	u_byte;

#ifdef VMS
#if !defined(CADDR_T) && !defined(__CADDR_T) && !defined(__SOCKET_TYPEDEFS)
typedef char *caddr_t;
#define CADDR_T
#define __CADDR_T
#endif
#ifndef __SOCKET_TYPEDEFS
typedef unsigned short  u_short;
typedef unsigned short  u_long;
typedef unsigned short  u_char;
#define __SOCKET_TYPEDEFS
#endif
#endif


#if (_SEQUENT_)
typedef unsigned short	u_short;
#endif

#if 0
typedef enum {true = 1, false = 0} bool;
#else
#define false	0
#define true	1
typedef int bool;
#endif

typedef struct { float x, y; }	vector;
typedef vector			position;
typedef struct { int x, y; }	ivec;
typedef ivec			ipos;

#if !defined(select) && defined(__linux__)
#define select(N, R, W, E, T)	select((N),		\
	(fd_set*)(R), (fd_set*)(W), (fd_set*)(E), (T))
#endif

#endif

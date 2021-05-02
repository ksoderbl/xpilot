/* $Id: types.h,v 3.23 1997/11/27 20:09:38 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-97 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
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

#include <sys/types.h>

#ifdef VMS
typedef char byte;
#else
#ifndef	_WINDOWS
typedef signed char	byte;
#endif
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

/*
 * On some systems an enum is smaller than an int.
 * On others bool is already a builtin type.
 * Using preprocessor macros to circumvent both situations.
 */
#define false	0
#define true	1

#ifndef	_XPMONNT_
#define bool	int
#endif

/*
 * Windows does all its FPU work in doubles.  Using floats gives warnings
 * and causes everything to be promoted to doubles anyway...
 */
#ifndef	_WINDOWS
typedef	float	DFLOAT;
#else
typedef	double	DFLOAT;
#endif

typedef struct { DFLOAT x, y; }	vector;
typedef vector			position;
typedef struct { int x, y; }	ivec;
typedef ivec			ipos;

#ifdef	_WINDOWS
#define	strncasecmp(__s, __t, __l)	strnicmp(__s, __t, __l)
#define	strcasecmp(__s, __t)	stricmp(__s, __t)
#endif
#endif

/* $Id: error.h,v 5.0 2001/04/07 20:00:59 dik Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
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
/*
 * Adapted from 'The UNIX Programming Environment' by Kernighan & Pike
 * and an example from the manualpage for vprintf by
 * Gaute Nessan, University of Tromsoe (gaute@staff.cs.uit.no).
 *
 * Modified by Bjoern Stabell <bjoern@xpilot.org>.
 * Dick Balaska <dick@xpilot.org> added the memory/leak checking.
 */

#ifndef	ERROR_H
#define	ERROR_H

#ifndef _WINDOWS
# if defined(__STDC__) && !defined(__sun__) || defined(__cplusplus)
#  include <stdarg.h>
    extern void error(const char *fmt, ...);
# else
#  include <varargs.h>
    extern void error();
# endif
#endif

#ifdef _WINDOWS
    extern void error();
# ifdef	_DEBUG
#  define	Trace _Trace
# else
#  define	Trace
# endif
#endif

extern void init_error(const char *prog);

#endif	/* ERROR_H */

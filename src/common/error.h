/* $Id: error.h,v 5.3 2001/05/30 18:34:15 dik Exp $
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
 * Adapted from 'The UNIX Programming Environment' by Kernighan & Pike
 * and an example from the manualpage for vprintf by
 * Gaute Nessan, University of Tromsoe (gaute@staff.cs.uit.no).
 *
 * Modified by Bjoern Stabell <bjoern@xpilot.org>.
 * Dick Balaska <dick@xpilot.org> added the memory/leak checking.
 * Bert Gijsbers <bert@xpilot.org> added warn, fatal, coredump.
 */

#ifndef	ERROR_H
#define	ERROR_H

# if (defined(__STDC__) && !defined(__sun__) || defined(__cplusplus) || defined(_WINDOWS))
#  include <stdarg.h>
    extern void warn(const char *fmt, ...);
    extern void error(const char *fmt, ...);
    extern void fatal(const char *fmt, ...);
    extern void dumpcore(const char *fmt, ...);
# else
#  include <varargs.h>
    extern void warn();
    extern void error();
    extern void fatal();
    extern void dumpcore();
# endif

#ifdef _WINDOWS
# ifdef	_DEBUG
#  define	Trace _Trace
# else
#  define	Trace
# endif
#endif

extern void init_error(const char *prog);

#endif	/* ERROR_H */

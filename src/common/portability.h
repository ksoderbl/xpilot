/* $Id: portability.h,v 4.2 1999/10/20 19:35:02 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
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

/*
 * Include portability related stuff in one file.
 */
#ifndef PORTABILITY_H_INCLUDED
#define PORTABILITY_H_INCLUDED

#ifdef _WINDOWS
#undef max
#undef min

#define	strncasecmp(__a, __b, __c)	strnicmp(__a, __b, __c)


#endif /* _WINDOWS */

/*
 * Portability fixes for Sony NeWS.
 */
#ifdef sony_news
#define setvbuf(A,B,C,D)	setlinebuf(A)

typedef unsigned int    sigset_t;

#define sigemptyset(set)        (*(set) = 0)
#define sigfillset(set)         (*(set) = ~(sigset_t)0, 0)
#define sigaddset(set,signo)    (*(set) |= sigmask(signo), 0)
#define sigdelset(set,signo)    (*(set) &= ~sigmask(signo), 0)
#define sigismember(set,signo)  ((*(set) & sigmask(signo)) != 0)

#define SIG_BLOCK		1
#define SIG_UNBLOCK		2
#define SIG_SETMASK		3

extern int sigprocmask(int how, const sigset_t *set, sigset_t *oset);

/*
 * Sony NEWS doesn't have sigaction(), using sigvec() instead.
 */
#define sigaction	sigvec
#define sa_handler	sv_handler
#define sa_mask		sv_mask
#define sa_flags	sv_flags

#endif

#ifdef VMS
/* blabla */
#endif

#ifdef SUNCMW
#include "cmw.h"
#endif /* SUNCMW */

/*
 * In Windows, just exiting won't tell the user the reason.
 * So, try to gracefully shutdown just the server thread
 */
#ifdef	_WINDOWS
extern	int ServerKilled;
#define	ServerExit() ServerKilled = TRUE; return;
#else
#define	ServerExit() exit(1);
#endif

/*
 * Macros to block out Windows only code (and never Windows code)
 */
#ifdef	_WINDOWS
#define IFWINDOWS(x)	x
#else
#define IFWINDOWS(x)
#endif

#ifndef	_WINDOWS
#define IFNWINDOWS(x)	x
#else
#define IFNWINDOWS(x)
#endif

/*
 * Prototypes for OS function wrappers in portability.c.
 */
extern int Get_process_id(void);	/* getpid */
extern void Get_login_name(char *buf, int size);

/*
 * Prototypes for other (ANSI C library) wrapper functions.
 */
extern void move_memory(void *dst, void *src, size_t len);

#endif /* PORTABILITY_H_INCLUDED */

/* $Id: error.h,v 3.7 1996/10/13 15:01:09 bert Exp $
 *
 * Adapted from 'The UNIX Programming Environment' by Kernighan & Pike
 * and an example from the manualpage for vprintf by
 * Gaute Nessan, University of Tromsoe (gaute@staff.cs.uit.no).
 *
 * Modified by Bjoern Stabell <bjoern@xpilot.org>.
 */

#ifndef	ERROR_H
#define	ERROR_H

/*
 * Prototypes and include files.
 */
#ifdef VAX
#include <perror.h>
#endif
#include <errno.h>
#include <stdio.h>

#if defined(__STDC__) && !defined(__sun__) || defined(__cplusplus)
#   include <stdarg.h>
    extern void error(const char *fmt, ...);
#else
#   include <varargs.h>
    extern void error();
#endif

extern void init_error(const char *prog);

#endif	/* ERROR_H */

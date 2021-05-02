/* $Id: error.h,v 3.0 1993/05/21 18:35:16 bjoerns Exp $
 *
 * Adapted from 'The UNIX Programming Environment' by Kernighan & Pike
 * and an example from the manualpage for vprintf by
 * Gaute Nessan, University of Tromsoe (gaute@staff.cs.uit.no).
 *
 * Modified by Bjoern Stabell (bjoerns@staff.cs.uit.no).
 */

#ifndef	ERROR_H
#define	ERROR_H

/*
 * Prototypes and include files.
 */

#include <errno.h>
#include <stdio.h>
#if defined(__STDC__) && !defined(__sun__)
#   include <stdarg.h>
#else
#   include <varargs.h>
#endif
#include <string.h>

#if defined(__STDC__) && !defined(__sun__)
    extern void init_error(char *prog);
    extern void error(char *fmt, ...);
#else
    extern void init_error();
    extern void error();
#endif

#endif	/* ERROR_H */

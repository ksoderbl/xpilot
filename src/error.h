/* $Id: error.h,v 3.11 1998/01/28 08:50:05 bert Exp $
 *
 * Adapted from 'The UNIX Programming Environment' by Kernighan & Pike
 * and an example from the manualpage for vprintf by
 * Gaute Nessan, University of Tromsoe (gaute@staff.cs.uit.no).
 *
 * Modified by Bjoern Stabell <bjoern@xpilot.org>.
 * Dick Balaska <dick@xpilot.org> added the memory/leak checking.
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

#ifndef	_WINDOWS
#if defined(__STDC__) && !defined(__sun__) || defined(__cplusplus)
#   include <stdarg.h>
    extern void error(const char *fmt, ...);
#else
#   include <varargs.h>
    extern void error();
#endif

#else
    extern void error();
#ifdef	_DEBUG
#define	Trace _Trace
#else
#define	Trace
#endif
#endif

extern void init_error(const char *prog);

#ifdef	_WINDOWS
// extern	HWND	alarmWnd;		// Window for alarm timers
#endif
/*
 * memory leak checking
 */
#if (defined(_WINDOWS) && defined(_DEBUG)) || !defined(_WINDOWS)

/*#define	_MEMPOD	1*/

#ifdef	_MEMPOD
extern	void xpmemShutdown();

extern	void* xpmalloc(size_t, char*, int);
extern	void  xpfree(void*, char*, int);
extern	void* xprealloc(void*, size_t, char*, int);
extern	char* xpstrdup(const char*, char*, int);
extern	void* xpcalloc(size_t num, size_t size, char*, int);

#define	malloc(__m) \
	(xpmalloc(__m, __FILE__, __LINE__))
#define	realloc(__m, __s) \
	(xprealloc(__m, __s, __FILE__, __LINE__))
#define	calloc(__m, __s) \
	(xpcalloc(__m, __s, __FILE__, __LINE__))
#define	free(__m) \
	(xpfree(__m, __FILE__, __LINE__))
#define	strdup(__s) \
	(xpstrdup(__s, __FILE__, __LINE__))
#endif
#endif

#endif	/* ERROR_H */

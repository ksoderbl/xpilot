/* $Id: error.c,v 5.5 2001/05/27 17:15:04 bertg Exp $
 *
 * Adapted from 'The UNIX Programming Environment' by Kernighan & Pike
 * and an example from the manualpage for vprintf by
 * Gaute Nessan, University of Tromsoe (gaute@staff.cs.uit.no).
 *
 * Modified by Bjoern Stabell <bjoern@xpilot.org>.
 * Windows mods and memory leak detection by Dick Balaska <dick@xpilot.org>.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined(_WINDOWS)
#	ifdef	_XPILOTNTSERVER_
#		include "../server/NT/winServer.h"
		extern char *showtime(void);
#	elif !defined(_XPMONNT_)
#		include "NT/winX.h"
#		include "../client/NT/winClient.h"
#	endif
	static void Win_show_error(char *errmsg);
#endif

#include "version.h"
#include "config.h"
#include "const.h"
#include "error.h"
#include "portability.h"
#include "commonproto.h"


#undef HAVE_STDARG
#undef HAVE_VARARG
#ifndef _WINDOWS
# if (defined(__STDC__) && !defined(__sun__) || defined(__cplusplus))
#  define HAVE_STDARG 1
# else
#  define HAVE_VARARG 1
# endif
#endif


char error_version[] = VERSION;


/*
 * This file defines two entry points:
 *
 * init_error()		- Initialize the error routine, accepts program name
 *			  as input.
 * error()		- perror() with printf functionality.
 */



/*
 * File local static data.
 */
#define	MAX_PROG_LENGTH	32
static char		progname[MAX_PROG_LENGTH];



static const char* prog_basename(const char *prog)
{
#ifndef _WINDOWS
    char *p;

    p = strrchr(prog, '/');

    return (p != NULL) ? (p + 1) : prog;
#else
    return "xpilot";
#endif
}


/*
 * Functions.
 */
void init_error(const char *prog)
{
    const char *p = prog_basename(prog);   /* Beautify arv[0] */

    strlcpy(progname, p, MAX_PROG_LENGTH);
}



#if HAVE_STDARG
/*
 * Ok, let's do it the ANSI C way.
 */
void error(const char *fmt, ...)
{
    va_list	 ap;
    int		 e = errno;

    va_start(ap, fmt);

    if (progname[0] != '\0') {
	fprintf(stderr, "%s: ", progname);
    }

    vfprintf(stderr, fmt, ap);

    if (e != 0) {
	fprintf(stderr, ": (%s)", strerror(e));
    }
    fprintf(stderr, "\n");

    va_end(ap);
}

void warn(const char *fmt, ...)
{
    int		len;
    va_list	ap;

    va_start(ap, fmt);

    if (progname[0] != '\0') {
	fprintf(stderr, "%s: ", progname);
    }

    vfprintf(stderr, fmt, ap);

    len = strlen(fmt);
    if (len == 0 || fmt[len - 1] != '\n') {
	fprintf(stderr, "\n");
    }

    va_end(ap);
}

void fatal(const char *fmt, ...)
{
    va_list	 ap;

    va_start(ap, fmt);

    if (progname[0] != '\0') {
	fprintf(stderr, "%s: ", progname);
    }

    vfprintf(stderr, fmt, ap);

    fprintf(stderr, "\n");

    va_end(ap);

    exit(1);
}

void dumpcore(const char *fmt, ...)
{
    va_list	 ap;

    va_start(ap, fmt);

    if (progname[0] != '\0') {
	fprintf(stderr, "%s: ", progname);
    }

    vfprintf(stderr, fmt, ap);

    fprintf(stderr, "\n");

    va_end(ap);

    abort();
}

#endif


#if HAVE_VARARG
/*
 * Hm, we'd better stick to the K&R way.
 */
void
    error(va_alist)
va_dcl
{
    va_list	 args;
    int		 e = errno;		/* Store errno */
    extern int	 sys_nerr;
    extern char *sys_errlist[];
    char	*fmt;


    va_start(args);

    if (progname[0] != '\0')
	fprintf(stderr, "%s: ", progname);

    fmt = va_arg(args, char *);
    (void) vfprintf(stderr, fmt, args);

    if (e > 0 && e < sys_nerr)
	fprintf(stderr, " (%s)", sys_errlist[e]);

    fprintf(stderr, "\n");

    va_end(args);
}

void
    warn(va_alist)
va_dcl
{
    va_list	 args;
    char	*fmt;


    va_start(args);

    if (progname[0] != '\0')
	fprintf(stderr, "%s: ", progname);

    fmt = va_arg(args, char *);
    (void) vfprintf(stderr, fmt, args);

    fprintf(stderr, "\n");

    va_end(args);
}

void
    fatal(va_alist)
va_dcl
{
    va_list	 args;
    char	*fmt;


    va_start(args);

    if (progname[0] != '\0')
	fprintf(stderr, "%s: ", progname);

    fmt = va_arg(args, char *);
    (void) vfprintf(stderr, fmt, args);

    fprintf(stderr, "\n");

    va_end(args);

    exit(1);
}

void
    dumpcore(va_alist)
va_dcl
{
    va_list	 args;
    char	*fmt;


    va_start(args);

    if (progname[0] != '\0')
	fprintf(stderr, "%s: ", progname);

    fmt = va_arg(args, char *);
    (void) vfprintf(stderr, fmt, args);

    fprintf(stderr, "\n");

    va_end(args);

    abort();
}

#endif


#ifdef _WINDOWS
static void Win_show_error(char *s)
{
    IFWINDOWS( Trace("Error: %s\n", s); )
/*  inerror = TRUE; */
    {
#       ifdef   _XPILOTNTSERVER_
	/* putting up a message box on the server is a bad thing.
	   It kinda halts the server, which is a bad thing to do for
	   the simple info messages (nick in use) that call this routine
	*/
	xpprintf("%s %s\n", showtime(), s);
#       else
	if (MessageBox(NULL, s, "Error", MB_OKCANCEL | MB_TASKMODAL) == IDCANCEL)
	{
#           ifdef   _XPMON_
		xpmemShutdown();
#           endif
	    ExitProcess(1);
	}
#       endif
    }
}


void error(const char *fmt, ...)
{
    va_list	ap;
    char	s[512];

    va_start(ap, fmt);

    vsprintf(s, fmt, ap);

    Win_show_error(s);

    va_end(ap);
}

void warn(const char *fmt, ...)
{
    va_list	ap;
    char	s[512];

    va_start(ap, fmt);

    vsprintf(s, fmt, ap);

    Win_show_error(s);

    va_end(ap);
}

void fatal(const char *fmt, ...)
{
    va_list	ap;
    char	s[512];

    va_start(ap, fmt);

    vsprintf(s, fmt, ap);

    Win_show_error(s);

    va_end(ap);

    exit(1);
}

void dumpcore(const char *fmt, ...)
{
    va_list	ap;
    char	s[512];

    va_start(ap, fmt);

    vsprintf(s, fmt, ap);

    Win_show_error(s);

    va_end(ap);

    exit(1);
}

#endif

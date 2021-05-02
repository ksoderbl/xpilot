/* $Id: error.c,v 4.0 1998/03/18 15:17:47 bert Exp $
 *
 * Adapted from 'The UNIX Programming Environment' by Kernighan & Pike
 * and an example from the manualpage for vprintf by
 * Gaute Nessan, University of Tromsoe (gaute@staff.cs.uit.no).
 *
 * Modified by Bjoern Stabell <bjoern@xpilot.org>.
 * Windows mods and memory leak detection by Dick Balaska <dick@xpilot.org>.
 */

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: error.c,v 4.0 1998/03/18 15:17:47 bert Exp $";
#endif

#include <stdlib.h>
#include <string.h>

#if defined(_WINDOWS)
#	ifdef	_XPILOTNTSERVER_
#		include "../server/NT/winServer.h"
		extern char *showtime(void);
#	elif !defined(_XPMONNT_)
#		include "NT/winX.h"
#		include "../client/NT/winClient.h"
#	endif
#endif

#include "version.h"
#include "config.h"
#include "const.h"
#include "error.h"
#include "portability.h"


char error_version[] = VERSION;


/*
 * This file defines two entry points:
 *
 * init_error()		- Initialize the error routine, accepts program name
 *			  as input.
 * error()		- perror() with printf functionality.
 */



/*
 * Global data.
 */
#define	MAX_PROG_LENGTH	256
static char		progname[MAX_PROG_LENGTH];



/*
 * Functions.
 */
void init_error(const char *prog)
{
#ifndef	_WINDOWS
#ifdef VMS
    char *p = strrchr(prog, ']');
#else
    char *p = strrchr(prog, '/');
#endif

    strncpy(progname, p ? p+1 : prog, MAX_PROG_LENGTH);   /* Beautify arv[0] */
#else
	strcpy(progname, "xpilot");
#endif
}



#if defined(__STDC__) && !defined(__sun__) || defined(__cplusplus) || defined (_WINDOWS)
/*
 * Ok, let's do it the ANSI C way.
 */
void error(const char *fmt, ...)
{
    va_list	 ap;			/* Argument pointer */
    int		 e = errno;		/* Store errno */
#ifdef	_WINDOWS
	char	s[512];
#endif
#ifdef VMS
    if (e == EVMSERR)
	e = 0/*__gnu_vaxc_errno__*/;
#endif

    va_start(ap, fmt);

    if (progname[0] != '\0')
	fprintf(stderr, "%s: ", progname);

#ifdef	_WINDOWS
    vsprintf(s, fmt, ap);
#else
    vfprintf(stderr, fmt, ap);
#endif

    if (e != 0)
	fprintf(stderr, " (%s)", strerror(e));


#ifdef	_WINDOWS
	IFWINDOWS( Trace("Error: %s\n", s); )
/*	inerror = TRUE; */
	{
#		ifdef	_XPILOTNTSERVER_
		/* putting up a message box on the server is a bad thing.
		   It kinda halts the server, which is a bad thing to do for
		   the simple info messages (nick in use) that call this routine
		*/
		xpprintf("%s %s\n", showtime(), s);
#		else
 		if (MessageBox(NULL, s, "Error", MB_OKCANCEL | MB_TASKMODAL) == IDCANCEL)
		{
#			ifdef	_XPMON_
				xpmemShutdown();
#			endif
			ExitProcess(1);
		}
#		endif
	}
#else
    fprintf(stderr, "\n");
#endif
    va_end(ap);
}

#else

/*
 * Hm, we'd better stick to the K&R way.
 */
void
    error(va_alist)
va_dcl		/* Note that the format argument cannot be separately	*
		 * declared because of the definition of varargs.	*/
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

#endif

/***************************************************************************\
* memory management stuff - i.e. debug leak detection						*
\***************************************************************************/
#if	defined(_DEBUG)
#define	MAXMEMPOD	1000				/* number of entries in each list */
typedef struct	mempod {
	void*	mem;	/* address returned */
	size_t	size;	/* size of request */
	char*	file;	/* file that malloced the memory */
	int		line;	/* line number of the malloc */
} mempod;

typedef	struct memlink memlink;
typedef struct memlink {
	memlink*	nextlink;
	int			nextslot;
	mempod		mem[MAXMEMPOD];
};

memlink*	memanchor = NULL;
memlink*	curmemblock = NULL;

#undef	malloc
#undef	realloc
#undef	free
#undef	strdup

void*	xpmalloc(size_t amount, char* file, int line)
{
	void*	tptr;
	tptr = malloc(amount);

	if (!memanchor)
	{
		memanchor = malloc(sizeof(memlink));
		curmemblock = memanchor;
		memset(curmemblock, 0, sizeof(memlink));
	}
	if (curmemblock->nextslot == MAXMEMPOD)
	{
		xpprintf("Mallocing mempod block\n");
		curmemblock->nextlink = malloc(sizeof(memlink));
		memset(curmemblock, 0, sizeof(memlink));
	}
	curmemblock->mem[curmemblock->nextslot].mem  = tptr;
	curmemblock->mem[curmemblock->nextslot].size = amount;
	curmemblock->mem[curmemblock->nextslot].file = file;
	curmemblock->mem[curmemblock->nextslot].line = line;

	_Trace("malloc: %p %5d bytes from %4d of %s\n", 
		curmemblock->mem[curmemblock->nextslot].mem, 
		curmemblock->mem[curmemblock->nextslot].size, 
		curmemblock->mem[curmemblock->nextslot].line, 
		curmemblock->mem[curmemblock->nextslot].file);
	curmemblock->nextslot++;
	return(tptr);
}

void*  xprealloc(void* tptr, size_t size, char* file, int line)
{
	memlink*	l = curmemblock;
	int			i;
	while(l)
	{
		for (i=0; i<l->nextslot; i++)
		{
			if (l->mem[i].mem == tptr)
			{
				tptr = realloc(tptr, size);
				l->mem[i].mem  = tptr;
				l->mem[i].size = size;
				l->mem[i].file = file;
				l->mem[i].line = line;
				_Trace("realloc: %p %5d bytes from %4d of %s\n", 
					l->mem[i].mem, l->mem[i].size, l->mem[i].line, l->mem[i].file);
				return (tptr);
			}
		}
		l = l->nextlink;
	}
	error("Can't match memblock %p", tptr);
	return(NULL);
}

void*	xpcalloc(size_t size, size_t amount, char* file, int line)
{
	void*	t = xpmalloc(size*amount, file, line);
	return(t);
}

void	xpfree(void* tptr, char* file, int line)
{
	memlink*	l = curmemblock;
	int			i;
	while(l)
	{
		for (i=0; i<l->nextslot; i++)
		{
			if (l->mem[i].mem == tptr)
			{
				l->mem[i].mem = NULL;
				free(tptr);
				return;
			}
		}
		l = l->nextlink;
	}
}

char*	xpstrdup(const char* s, char* file, int line)
{
	char*	t = xpmalloc(strlen(s)+1, file, line);
	strcpy(t, s);
	return(t);
}

void	xpmemShutdown()
{
	memlink*	l = curmemblock;
	int			i;
	int			hits = 0;
	while(l)
	{
		for (i=0; i<l->nextslot; i++)
		{
			if (l->mem[i].mem)
				_Trace("leak: %p %5d bytes from %4d of %s\n", 
					l->mem[i].mem, l->mem[i].size, l->mem[i].line, l->mem[i].file);
		}
		hits += l->nextslot;
		l = l->nextlink;
	}
	IFWINDOWS( Trace("xpmem tracked %d mallocs\n", hits); )
}

#endif

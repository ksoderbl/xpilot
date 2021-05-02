/* config.h,v 1.3 1992/05/11 15:31:01 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

/*
 * Configure these variables, that's what they're here for.
 */
#ifndef LOCALGURU
#define LOCALGURU	"bjoerns@stud.cs.uit.no"
#endif

#ifndef LIBDIR
#define LIBDIR		"/usr/local/lib/xpilot/"
#endif

#ifndef HELPFILE
#define HELPFILE	LIBDIR "help.txt"
#endif
#ifndef MOTDFILE
#define MOTDFILE	LIBDIR "motd.txt"
#endif
#ifndef LOGFILE
#define LOGFILE		LIBDIR "log"
#endif
#ifndef MAPDIR
#define MAPDIR		LIBDIR "maps/"
#endif

/*
 * This is really nasty, but it's so much easier this way.  Will substitute
 * the name of the machine where the server runs for %s.
 */
#ifndef XHOST_CMD
#define XHOST_CMD	"/usr/bin/X11/xhost +%s >/dev/null" /*2>/dev/null"*/
#endif


/*
 * Leave these alone.
 */
#define REPORT_ADDRESS	"bjoerns@stud.cs.uit.no"

#ifdef	DEBUG
#define D(x)		{ {x}; fflush(stdout); }
#else
#define D(x)		{ ; }
#endif

#define warn(s)		{ fprintf(stderr, s); }

/* config.h,v 1.10 1992/06/26 17:04:14 bjoerns Exp
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
#    define LOCALGURU		"bjoerns@stud.cs.uit.no"
#endif

#ifndef LIBDIR
#    define LIBDIR		"/usr/local/games/lib/xpilot/"
#endif

#ifndef MOTDFILE
#    define MOTDFILE		LIBDIR "motd"
#endif
#ifndef LOGFILE
#    define LOGFILE		LIBDIR "log"
#endif
#ifndef MAPDIR
#    define MAPDIR		LIBDIR "maps/"
#endif

#ifndef	UPDATES_PR_FRAME
#    define UPDATES_PR_FRAME	1
#endif

/*
 * If XHOST is defined, the hand-shake program will open a connection to the
 * specified host by executing XHOST_OPEN (%s will be substituted with the
 * server host) and closing it again with XHOST_CLOSE.  Note however that this
 * will only be done if the server complains that it couldn't open the display.
 *
 * All in all, a not perfect, but adequate solution, highly recommended for
 * X environments where Xauthority is used.
 */
#define	XHOST
#define XHOST_OPEN	"/usr/bin/X11/xhost +%s >/dev/null"
#define XHOST_CLOSE	"/usr/bin/X11/xhost -%s >/dev/null"

/*
 * Leave these alone.
 */
#define REPORT_ADDRESS	"bjoerns@stud.cs.uit.no"

#ifdef	DEBUG
#    define D(x)	{ {x}; fflush(stdout); }
#else
#    define D(x)	{ ; }
#endif

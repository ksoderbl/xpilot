/* $Id: config.h,v 1.14 1992/08/27 00:25:49 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

/*
 * Configure these, that's what they're here for.
 */
#ifndef LOCALGURU
#    define LOCALGURU		"ftp@csc.liv.ac.uk"
#endif

#ifndef LIBDIR
#    define LIBDIR		"/usr/local/lib/X11/xpilot/"
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


/*
 * The following macros decide the speed of the game and
 * how often the server should draw a frame.  (Hmm...)
 */

#ifndef	UPDATES_PR_FRAME
#    define UPDATES_PR_FRAME	1
#endif

#ifndef FRAMES_PR_SEC
#    define FRAMES_PR_SEC	10
#endif

/*
 *  If COMPRESSED_MAPS is defined, the server will attempt to uncompress
 * maps on the fly (but only if neccessary). ZCAT_FORMAT should produce
 * a command that will unpack the given .Z file to stdout (for use in popen).
 */

#define COMPRESSED_MAPS

#ifndef ZCAT_FORMAT
#    define ZCAT_FORMAT "zcat < %s"
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
#define REPORT_ADDRESS	"ftp@csc.liv.ac.uk"

#ifdef	DEBUG
#    define D(x)	{ {x}; fflush(stdout); }
#else
#    define D(x)	{ ; }
#endif

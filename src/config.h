/* $Id: config.h,v 1.2 1993/03/24 23:34:10 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bj�rn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef CONFIG_H
#define CONFIG_H

#ifdef MOD2
#  error MOD2 already defined - config.h should be included before const.h
#endif

/*
 * Configure these, that's what they're here for.
 */
#ifndef LOCALGURU
#    define LOCALGURU		"rkl@csc.liv.ac.uk"
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

/*
 * Uncomment this if your machine doesn't use
 * two's complement negative numbers.
 */
/* #define MOD2(x, m)	mod(x, m)	/* */


/*
 * The following macros decide the speed of the game and
 * how often the server should draw a frame.  (Hmm...)
 */

#ifndef	UPDATES_PR_FRAME
#    define UPDATES_PR_FRAME	1
#endif

#ifndef FPS
#    define FPS	framesPerSecond
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

#ifndef DEFAULTS_FILE_NAME
#    define DEFAULTS_FILE_NAME	LIBDIR "server-defaults"
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
#define REPORT_ADDRESS	"xpilot@cs.uit.no"

#ifdef	DEBUG
#    define D(x)	{ {x}; fflush(stdout); }
#else
#    define D(x)	{ ; }
#endif

#endif /* CONFIG_H */

/* $Id: config.h,v 3.7 1993/06/28 20:53:28 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
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
#    define LOCALGURU		"xpilot@cs.uit.no"
#endif

#ifndef	DEFAULT_MAP
#  define DEFAULT_MAP		"globe.map"
#endif

#ifndef LIBDIR
#    define LIBDIR		"/usr/local/games/lib/xpilot/"
#endif

#ifndef DEFAULTS_FILE_NAME
#    define DEFAULTS_FILE_NAME	LIBDIR "defaults"
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
#ifndef SOUNDFILE
#    define SOUNDFILE		LIBDIR "sounds"
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
 * If COMPRESSED_MAPS is defined, the server will attempt to uncompress
 * maps on the fly (but only if neccessary). ZCAT_FORMAT should produce
 * a command that will unpack the given .Z file to stdout (for use in popen).
 */

#undef COMPRESSED_MAPS

#ifndef ZCAT_FORMAT
#    define ZCAT_FORMAT "gzip -d -c < %s"
#endif

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

/* $Id: config.h,v 3.32 1997/11/27 20:09:08 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-97 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef CONFIG_H
#define CONFIG_H

#ifdef MOD2
#error "MOD2 already defined - config.h should be included before const.h"
#endif

/*
 * Configure these, that's what they're here for.
 * Explanation about all these compile time configuration options
 * is in the Makefile.std and in the Imakefile.
 */
#ifndef LOCALGURU
#    define LOCALGURU		"xpilot@xpilot.org"
#endif

#ifndef	DEFAULT_MAP
#	ifdef	_WINDOWS
#		define DEFAULT_MAP		"default.map"
#	else
#		define DEFAULT_MAP		"globe.map"
#	endif
#endif

#ifndef LIBDIR
#    ifdef VMS
#        define LIBPREFIX	"lib_disk:[lib.xgames.xpilot341.lib"
#        define LIBDIR		LIBPREFIX "]"
#    elif defined(_WINDOWS)
#        define LIBDIR		"lib/"
#	 else
#        define LIBDIR		"/usr/local/games/lib/xpilot/"
#    endif
#endif

#ifndef DEFAULTS_FILE_NAME
#    ifdef VMS
#        define DEFAULTS_FILE_NAME	LIBDIR "defaults"
#    elif defined(_WINDOWS)
#        define DEFAULTS_FILE_NAME	LIBDIR "defaults.txt"
#    else
#        define DEFAULTS_FILE_NAME	LIBDIR "/defaults"
#    endif
#endif
#ifndef ROBOTFILE
#    ifdef VMS
#        define ROBOTFILE	LIBDIR "robots"
#    elif defined(_WINDOWS)
#		 define	ROBOTFILE	LIBDIR "robots.txt"
#    else
#        define ROBOTFILE	LIBDIR "/robots"
#    endif
#endif
#ifndef SERVERMOTDFILE
#    ifdef VMS
#        define SERVERMOTDFILE	LIBDIR "servermotd"
#    elif defined(_WINDOWS)
#		 define	SERVERMOTDFILE	LIBDIR "servermotd.txt"
#	 else
#        define SERVERMOTDFILE	LIBDIR "/servermotd"
#    endif
#endif
#ifndef LOCALMOTDFILE
#    ifdef VMS
#        define LOCALMOTDFILE	LIBDIR "localmotd"
#    elif defined(_WINDOWS)
#		 define	LOCALMOTDFILE	LIBDIR "localmotd.txt"
#    else
#        define LOCALMOTDFILE	LIBDIR "/localmotd"
#    endif
#endif
#ifndef LOGFILE
#    ifdef VMS
#        define LOGFILE		LIBDIR "log"
#	 elif defined(_WINDOWS)
#		define	LOGFILE		LIBDIR "log.txt"
#    else
#        define LOGFILE		LIBDIR "/log"
#    endif
#endif
#ifndef MAPDIR
#    ifdef VMS
#        define MAPDIR		LIBPREFIX ".maps]"
#    else
#        define MAPDIR		LIBDIR "/maps/"
#    endif
#endif
#ifndef SHIP_FILE
#    ifdef VMS
#        define SHIP_FILE       LIBDIR "tkxpi.shp"
#    elif defined(_WINDOWS)
#		 define SHIP_FILE		"XPilot.shp"
#	 else
#        define SHIP_FILE       ""
#    endif
#endif
#ifndef TEXTUREDIR
#    ifdef VMS
#        define TEXTUREDIR	LIBPREFIX ".textures]"
#    else
#        define TEXTUREDIR	LIBDIR "/textures/"
#    endif
#endif
#ifndef SOUNDFILE
#    ifdef VMS
#        define SOUNDFILE	LIBDIR "sounds"
#    elif defined(_WINDOWS)
#        define SOUNDFILE	LIBDIR "sounds.txt"
#    else
#        define SOUNDFILE	LIBDIR "/sounds"
#    endif
#endif

/*
 * Uncomment this if your machine doesn't use
 * two's complement negative numbers.
 */
/* #define MOD2(x, m)	mod(x, m) */


/*
 * The following macros decide the speed of the game and
 * how often the server should draw a frame.  (Hmm...)
 */

#ifndef	UPDATES_PR_FRAME
#    define UPDATES_PR_FRAME	1
#endif

/*
 * If COMPRESSED_MAPS is defined, the server will attempt to uncompress
 * maps on the fly (but only if neccessary). ZCAT_FORMAT should produce
 * a command that will unpack the given .Z file to stdout (for use in popen).
 * ZCAT_EXT should define the proper compressed file extension.
 */

#if defined(VMS) || defined(_WINDOWS)
#    ifdef COMPRESSED_MAPS
	/*
	 * Couldn't find a popen(), also compress and gzip don't exist.
	 */
#        undef COMPRESSED_MAPS
#    endif
#else
#    define COMPRESSED_MAPS
#endif

#ifndef ZCAT_EXT
#    define ZCAT_EXT	".gz"
#endif

#ifndef ZCAT_FORMAT
#    define ZCAT_FORMAT "gzip -d -c < %s"
#endif

/*
 * Leave these alone.
 */
#define REPORT_ADDRESS	"xpilot@xpilot.org"

#ifdef	_WINDOWS
#	ifdef	_DEBUG
#		define	DEBUG	1
#		define	D(x)	{x;}
#	else
#		define	D(x)
#	endif
#else
#	ifdef	DEBUG
#		define D(x)	{ {x}; fflush(stdout); }
#	else
#		define D(x)
#	endif
#endif

/* Windows doesn't play with stdin/out well at all... */
/* So for the client i route the "debug" printfs to the debug stream */
/* The server gets 'real' messages routed to the messages window */
#ifdef	_WINDOWS
#	ifdef	_XPILOTNTSERVER_
#	define	xpprintf	xpprintfW
/*#	define	xpprintf	_Trace */
#	else
#	define	xpprintf	_Trace
#	endif
#else
#	define	xpprintf	printf
#endif

#endif /* CONFIG_H */

/* $Id: config.c,v 1.6 2000/03/21 10:37:22 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "version.h"
#include "config.h"

#ifdef _WINDOWS
/* #include "NT/winClient.h" */
/* #include "NT/winAudio.h" */
#endif /* _WINDOWS */


/*
 * Configure these, that's what they're here for.
 * Explanation about all these compile time configuration options
 * is in the Makefile.std and in the Imakefile.
 */
#ifndef LOCALGURU
#    define LOCALGURU		"xpilot@xpilot.org"
#endif

#ifndef	DEFAULT_MAP
#    ifdef	_WINDOWS
#         define DEFAULT_MAP		"default.xp"
#    else
#         define DEFAULT_MAP		"globe.xp"
#    endif
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
#    if defined(_WINDOWS)
#        define DEFAULTS_FILE_NAME	LIBDIR "defaults.txt"
#    else
#        define DEFAULTS_FILE_NAME	LIBDIR "defaults"
#    endif
#endif
#ifndef ROBOTFILE
#    if defined(_WINDOWS)
#		 define	ROBOTFILE	LIBDIR "robots.txt"
#    else
#        define ROBOTFILE	LIBDIR "robots"
#    endif
#endif
#ifndef SERVERMOTDFILE
#    if defined(_WINDOWS)
#	 define	SERVERMOTDFILE	LIBDIR "servermotd.txt"
#    else
#        define SERVERMOTDFILE	LIBDIR "servermotd"
#    endif
#endif
#ifndef LOCALMOTDFILE
#    if defined(_WINDOWS)
#	 define	LOCALMOTDFILE	LIBDIR "localmotd.txt"
#    else
#        define LOCALMOTDFILE	LIBDIR "localmotd"
#    endif
#endif
#ifndef LOGFILE
#    if defined(_WINDOWS)
#	 define	LOGFILE		LIBDIR "log.txt"
#    else
#        define LOGFILE		LIBDIR "log"
#    endif
#endif
#ifndef MAPDIR
#    ifdef VMS
#        define MAPDIR		LIBPREFIX ".maps]"
#    else
#        define MAPDIR		LIBDIR "maps/"
#    endif
#endif
#ifndef SHIP_FILE
#    ifdef VMS
#        define SHIP_FILE       LIBDIR "tkxpi.shp"
#    elif defined(_WINDOWS)
#	 define SHIP_FILE	"XPilot.shp"
#    else
#        define SHIP_FILE       ""
#    endif
#endif
#ifndef TEXTUREDIR
#    ifdef VMS
#        define TEXTUREDIR	LIBPREFIX ".textures]"
#    else
#        define TEXTUREDIR	LIBDIR "textures/"
#    endif
#endif
#ifndef	SOUNDDIR
#    ifdef VMS
#        define SOUNDDIR	LIBPREFIX ".sound]"
#    else
#        define SOUNDDIR	LIBDIR "sound/"
#    endif
#endif

#ifndef SOUNDFILE
#    if defined(_WINDOWS)
#        define SOUNDFILE	SOUNDDIR "sounds.txt"
#    else
#        define SOUNDFILE	SOUNDDIR "sounds"
#    endif
#endif

#ifndef SOUNDDIR
#    define SOUNDDIR	LIBDIR "sound/"
#endif

#ifndef ZCAT_EXT
#    define ZCAT_EXT	".gz"
#endif

#ifndef ZCAT_FORMAT
#    define ZCAT_FORMAT "gzip -d -c < %s"
#endif

/*
 * Please don't change this one.
 */
#ifndef CONTACTADDRESS
#    define CONTACTADDRESS	"xpilot@xpilot.org"
#endif


char config_version[] = VERSION;


#ifndef	lint
static char sourceid[] =
    "@(#)$Id: config.c,v 1.6 2000/03/21 10:37:22 bert Exp $";
#endif


char *Conf_libdir(void)
{
    static char conf[] = LIBDIR;

    return conf;
}

char *Conf_defaults_file_name(void)
{
    static char conf[] = DEFAULTS_FILE_NAME;

    return conf;
}

char *Conf_mapdir(void)
{
    static char conf[] = MAPDIR;

    return conf;
}

/* needed by server/cmdline.c */
char conf_default_map_string[] = DEFAULT_MAP;

char *Conf_default_map(void)
{
    return conf_default_map_string;
}

char *Conf_servermotdfile(void)
{
    static char conf[] = SERVERMOTDFILE;

    return conf;
}

char *Conf_localmotdfile(void)
{
    static char conf[] = LOCALMOTDFILE;

    return conf;
}

char *Conf_logfile(void)
{
    static char conf[] = LOGFILE;

    return conf;
}

/* needed by client/default.c */
char conf_ship_file_string[] = SHIP_FILE;

char *Conf_ship_file(void)
{
    return conf_ship_file_string;
}

/* needed by client/default.c */
char conf_texturedir_string[] = TEXTUREDIR;

char *Conf_texturedir(void)
{
    return conf_texturedir_string;
}

/* needed by client/default.c */
char conf_soundfile_string[] = SOUNDFILE;

char *Conf_soundfile(void)
{
    return conf_soundfile_string;
}

char *Conf_localguru(void)
{
    static char conf[] = LOCALGURU;

    return conf;
}

char *Conf_contactaddress(void)
{
    static char conf[] = CONTACTADDRESS;

    return conf;
}

/* needed by server/cmdline.c */
char conf_robotfile_string[] = ROBOTFILE;

char *Conf_robotfile(void)
{
    return conf_robotfile_string;
}

char *Conf_zcat_ext(void)
{
    static char conf[] = ZCAT_EXT;

    return conf;
}

char *Conf_zcat_format(void)
{
    static char conf[] = ZCAT_FORMAT;

    return conf;
}

/*#ifndef _WINDOWS */
#if 1
char *Conf_sounddir(void)
{
    static char conf[] = SOUNDDIR;

    return conf;
}
#else
/* dick: i don't know where this came from but it doesn't build
 * p and sounds are both undefined.  sound on Windows sucks anyway */
char *Conf_sounddir(void)
{
    static char snddir[1024];

    if (!snddir[0]) {
	strcpy(snddir, sounds);
	p = strrchr(snddir, '\\');
	if (p != NULL) {
	    *++p = '\0';
	} else {
	    p = strrchr(snddir, '/');
	    if (snddir != NULL) {
		*++p = '\0';
	    } else {
		snddir[0] = '\0';
	    }
	}
    }
    return snddir;
}
#endif


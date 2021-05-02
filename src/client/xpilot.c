/* $Id: xpilot.c,v 5.0 2001/04/07 20:00:59 dik Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>

#ifndef _WINDOWS
# include <unistd.h>
# ifndef __hpux
#  include <sys/time.h>
# endif
# include <sys/param.h>
# include <netdb.h>
#endif

#ifdef _WINDOWS
# include "NT/winNet.h"
# include "NT/winClient.h"
#endif

#include "version.h"
#include "config.h"
#include "const.h"
#include "types.h"
#include "pack.h"
#include "bit.h"
#include "error.h"
#include "socklib.h"
#include "net.h"
#include "connectparam.h"
#include "protoclient.h"
#ifdef SUNCMW
# include "cmw.h"
#endif /* SUNCMW */
#include "portability.h"
#include "checknames.h"

char xpilot_version[] = VERSION;

#ifndef	lint
static char versionid[] = "@(#)$" TITLE " $";
static char sourceid[] =
    "@(#)$Id: xpilot.c,v 5.0 2001/04/07 20:00:59 dik Exp $";
#endif

#define MAX_LINE	256	/* should not be smaller than MSG_LEN */

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN	64
#endif


char			hostname[MAXHOSTNAMELEN];

char			**Argv;
int			Argc;


static void Check_client_versions(void);


static void printfile(const char *filename)
{
    FILE		*fp;
    int			c;


    if ((fp = fopen(filename, "r")) == NULL) {
	return;
    }

    while ((c = fgetc(fp)) != EOF)
	putchar(c);

    fclose(fp);
}


/*
 * Oh glorious main(), without thee we cannot exist.
 */
int main(int argc, char *argv[])
{
    int				result;
    int				auto_connect = false,
				text = false,
				list_servers = false,
				auto_shutdown = false,
				noLocalMotd = false;
    char			*cp;
    Connect_param_t		*conpar;
    static char			shutdown_reason[MAX_CHARS];

    /*
     * --- Output copyright notice ---
     */
    printf("  " COPYRIGHT ".\n"
	   "  " TITLE " comes with ABSOLUTELY NO WARRANTY; "
	      "for details see the\n"
	   "  provided LICENSE file.\n\n");
    if (strcmp(Conf_localguru(), "xpilot@xpilot.org")
	&& strcmp(Conf_localguru(), "xpilot@cs.uit.no")) {
	printf("  %s is responsible for the local installation.\n\n",
	       Conf_localguru());
    }

    Argc = argc;
    Argv = argv;

    /*
     * --- Miscellaneous initialization ---
     */
#ifdef SUNCMW
    cmw_priv_init();
#endif /* CMW */

    init_error(argv[0]);

#ifdef _WINDOWS
    srand( (unsigned)time( NULL ) );
#else
    srand( (unsigned)time((time_t *)0) * getpid());
#endif

    Check_client_versions();

    conpar = (Connect_param_t *) calloc(1, sizeof(Connect_param_t));
    if (!conpar) {
	error("Not enough memory");
	exit(1);
    }
    conpar->contact_port = SERVER_PORT;
    conpar->team = TEAM_NOT_SET;

    cp = getenv("XPILOTHOST");
    if (cp) {
	strncpy(hostname, cp, sizeof(hostname) - 1);
    }
    else {
        sock_get_local_hostname(hostname, sizeof hostname, 0);
    }
    if (Check_host_name(hostname) == NAME_ERROR) {
	xpprintf("fixing host from \"%s\" ", hostname);
	Fix_host_name(hostname);
	xpprintf("to \"%s\"\n", hostname);
    }

    cp = getenv("XPILOTUSER");
    if (cp) {
	strncpy(conpar->real_name, cp, sizeof(conpar->real_name) - 1);
    }
    else {
	Get_login_name(conpar->real_name, sizeof(conpar->real_name) - 1);
    }
    if (Check_real_name(conpar->real_name) == NAME_ERROR) {
	xpprintf("fixing name from \"%s\" ", conpar->real_name);
	Fix_real_name(conpar->real_name);
	xpprintf("to \"%s\"\n", conpar->real_name);
    }

#ifdef _WINDOWS
    conpar->disp_name[0] = '\0';
#endif

    /*
     * --- Check commandline arguments and resource files ---
     */
    Parse_options(&argc, argv, conpar->real_name,
		  &conpar->contact_port, &conpar->team,
		  &text, &list_servers,
		  &auto_connect, &noLocalMotd,
		  conpar->nick_name, conpar->disp_name,
		  shutdown_reason);
    if (Check_nick_name(conpar->nick_name) == NAME_ERROR) {
	xpprintf("fixing nick from \"%s\" ", conpar->nick_name);
	Fix_nick_name(conpar->nick_name);
	xpprintf("to \"%s\"\n", conpar->nick_name);
    }

    if (list_servers) {
	auto_connect = true;
    }
    if (shutdown_reason[0] != '\0') {
	auto_shutdown = true;
	auto_connect = true;
    }

    /*
     * --- Message of the Day ---
     */
    if (!noLocalMotd)
	printfile(Conf_localmotdfile());

#ifdef	LIMIT_ACCESS
    /*
     * If sysadm's have complained alot, check for free machines before
     * letting the user play.  If room is crowded, don't let him play.
     */
    if (!list_servers && Is_allowed(conpar->disp_name) == false)
	exit (-1);
#endif
#ifdef SIMULATING_ONLY
Simulate_init();
Simulate_frames();
exit(0);
#endif

#if 0 || _WINDOWS
    if (list_servers)
	printf("LISTING AVAILABLE SERVERS:\n");
    result = Contact_servers(argc - 1, &argv[1],
			     auto_connect, list_servers,
			     auto_shutdown, shutdown_reason,
			     0, 0, 0, 0,
			     conpar);
#else
    if (text || auto_connect || argv[1]) {
	if (list_servers)
	    printf("LISTING AVAILABLE SERVERS:\n");

	result = Contact_servers(argc - 1, &argv[1],
				 auto_connect, list_servers,
				 auto_shutdown, shutdown_reason,
				 0, 0, 0, 0,
				 conpar);
    }
    else {
	result = Welcome_screen(conpar);
    }
#endif

    if (result == 1) {
	return Join(conpar->server_addr, conpar->server_name, conpar->login_port,
		    conpar->real_name, conpar->nick_name, conpar->team,
		    conpar->disp_name, conpar->server_version);
    }
    return 1;
}

/*
 * Verify that all source files making up this program have been
 * compiled for the same version.  Too often bugs have been reported
 * for incorrectly compiled programs.
 */
static void Check_client_versions(void)
{
#ifndef _WINDOWS	/* gotta put this back in before source released */
#ifdef SOUND
    extern char		audio_version[];
#endif
    extern char		about_version[],
			caudio_version[],
			checknames_version[],
			client_version[],
			colors_version[],
			configure_version[],
			dbuff_version[],
			default_version[],
			error_version[],
			join_version[],
			math_version[],
			net_version[],
			netclient_version[],
			paint_version[],
			paintdata_version[],
			painthud_version[],
			paintmap_version[],
			paintobjects_version[],
			paintradar_version[],
			portability_version[],
			query_version[],
			record_version[],
			shipshape_version[],
			socklib_version[],
			talk_version[],
			texture_version[],
			widget_version[],
			xevent_version[],
			xinit_version[],
			xpilot_version[],
			xpmread_version[];
    static struct file_version {
	char		filename[16];
	char		*versionstr;
    } file_versions[] = {
	{ "about", about_version },
#ifdef SOUND
	{ "*audio", audio_version },
#endif
	{ "caudio", caudio_version },
	{ "checknames", checknames_version },
	{ "client", client_version },
	{ "colors", colors_version },
	{ "configure", configure_version },
	{ "dbuff", dbuff_version },
	{ "default", default_version },
	{ "error", error_version },
	{ "join", join_version },
	{ "math", math_version },
	{ "net", net_version },
	{ "netclient", netclient_version },
	{ "paint", paint_version },
	{ "paintdata", paintdata_version },
	{ "painthud", painthud_version },
	{ "paintmap", paintmap_version },
	{ "paintobjects", paintobjects_version },
	{ "paintradar", paintradar_version },
	{ "portability", portability_version },
	{ "query", query_version },
	{ "record", record_version },
	{ "shipshape", shipshape_version },
	{ "socklib", socklib_version },
	{ "talk", talk_version },
	{ "texture", texture_version },
	{ "widget", widget_version },
	{ "xevent", xevent_version },
	{ "xinit", xinit_version },
	{ "xpilot", xpilot_version },
	{ "xpmread", xpmread_version },
    };
    int			i;
    int			oops = 0;

    for (i = 0; i < NELEM(file_versions); i++) {
	if (strcmp(VERSION, file_versions[i].versionstr)) {
	    oops++;
	    error("Source file %s.c (\"%s\") is not compiled "
		  "for the current version (\"%s\")!",
		  file_versions[i].filename,
		  file_versions[i].versionstr,
		  VERSION);
	}
    }
    if (oops) {
	error("%d version inconsistency errors, cannot continue.", oops);
	error("Please recompile this program properly.");
	exit(1);
    }
#endif
}

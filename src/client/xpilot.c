/* $Id: xpilot.c,v 5.9 2004/02/11 18:58:52 dik Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
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
#include "xpconfig.h"
#include "const.h"
#include "types.h"
#include "pack.h"
#include "bit.h"
#include "error.h"
#include "socklib.h"
#include "net.h"
#include "connectparam.h"
#include "protoclient.h"
#include "portability.h"
#include "checknames.h"
#include "commonproto.h"

char xpilot_version[] = VERSION;

#ifndef	lint
char xpilot_versionid[] = "@(#)$" TITLE " $";
#endif


char			hostname[SOCK_HOSTNAME_LENGTH];

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
    init_error(argv[0]);

    seedMT( (unsigned)time((time_t *)0) ^ Get_process_id());

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
	strlcpy(hostname, cp, sizeof(hostname));
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
	strlcpy(conpar->real_name, cp, sizeof(conpar->real_name));
    }
    else {
	Get_login_name(conpar->real_name, sizeof(conpar->real_name) - 1);
    }
    if (Check_real_name(conpar->real_name) == NAME_ERROR) {
	xpprintf("fixing name from \"%s\" ", conpar->real_name);
	Fix_real_name(conpar->real_name);
	xpprintf("to \"%s\"\n", conpar->real_name);
    }

    IFWINDOWS( conpar->disp_name[0] = '\0'; )

    /*
     * --- Check commandline arguments and resource files ---
     */
    Parse_options(&argc, argv, conpar->real_name,
		  &conpar->contact_port, &conpar->team,
		  &text, &list_servers,
		  &auto_connect, &noLocalMotd,
		  conpar->nick_name, conpar->disp_name,
		  hostname, shutdown_reason);
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

    Simulate();

    if (text || auto_connect || argv[1] || is_this_windows()) {
	if (list_servers)
	    printf("LISTING AVAILABLE SERVERS:\n");

	result = Contact_servers(argc - 1, &argv[1],
				 auto_connect, list_servers,
				 auto_shutdown, shutdown_reason,
				 0, 0, 0, 0, 0,
				 conpar);
    }
    else {
	IFNWINDOWS(result = Welcome_screen(conpar);)
    }

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
extern char about_version[];
#ifdef SOUND
extern char audio_version[];
#endif
extern char blockbitmaps_version[];
extern char caudio_version[];
extern char checknames_version[];
extern char client_version[];
extern char colors_version[];
extern char config_version[];
extern char configure_version[];
extern char datagram_version[];
extern char dbuff_version[];
extern char default_version[];
extern char error_version[];
extern char gfx2d_version[];
extern char guimap_version[];
extern char guiobjects_version[];
extern char join_version[];
extern char math_version[];
extern char net_version[];
extern char netclient_version[];
extern char paint_version[];
extern char paintdata_version[];
extern char painthud_version[];
extern char paintmap_version[];
extern char paintobjects_version[];
extern char paintradar_version[];
extern char portability_version[];
extern char query_version[];
extern char record_version[];
extern char shipshape_version[];
extern char socklib_version[];
extern char talk_version[];
extern char talkmacros_version[];
extern char textinterface_version[];
extern char texture_version[];
extern char welcome_version[];
extern char widget_version[];
extern char xevent_version[];
extern char xeventhandlers_version[];
extern char xinit_version[];
extern char xpilot_version[];
extern char xpmread_version[];


static void Check_client_versions(void)
{
#ifndef _WINDOWS	/* gotta put this back in before source released */
    static struct file_version {
	char		filename[16];
	char		*versionstr;
    } file_versions[] = {
	{ "about", about_version },
#ifdef SOUND
	{ "audio", audio_version },
#endif
	{ "blockbitmaps", blockbitmaps_version },
	{ "caudio", caudio_version },
	{ "checknames", checknames_version },
	{ "client", client_version },
	{ "colors", colors_version },
	{ "config", config_version },
	{ "configure", configure_version },
	{ "datagram", datagram_version },
	{ "dbuff", dbuff_version },
	{ "default", default_version },
	{ "error", error_version },
	{ "gfx2d", gfx2d_version },
	{ "guimap", guimap_version },
	{ "guiobjects", guiobjects_version },
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
	{ "talkmacros", talkmacros_version },
	{ "textinterface", textinterface_version },
	{ "texture", texture_version },
	{ "welcome", welcome_version },
	{ "widget", widget_version },
	{ "xevent", xevent_version },
	{ "xeventhandlers", xeventhandlers_version },
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


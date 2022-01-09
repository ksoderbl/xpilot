/*
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

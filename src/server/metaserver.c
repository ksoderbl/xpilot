/* $Id: metaserver.c,v 5.5 2001/11/29 14:48:12 bertg Exp $
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
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>

#ifndef _WINDOWS
# include <unistd.h>
# ifndef __hpux
#  include <sys/time.h>
# endif
#endif

#ifdef _WINDOWS
# include "NT/winServer.h"
#endif

#define SERVER
#include "config.h"
#include "version.h"
#include "serverconst.h"
#include "types.h"
#include "global.h"
#include "proto.h"
#include "socklib.h"
#include "map.h"
#include "pack.h"
#include "metaserver.h"
#include "saudio.h"
#include "error.h"
#include "netserver.h"
#include "commonproto.h"

#if defined(_WINDOWS)
#define	META_VERSION	TITLE
#else
#define META_VERSION	VERSION
#endif

char metaserver_version[] = VERSION;


struct MetaServer {
    char		name[64];
    char		addr[16];
};
struct MetaServer	meta_servers[2] = {
    {
	META_HOST,
	META_IP
    },
    {
	META_HOST_TWO,
	META_IP_TWO
    },
};

extern sock_t	contactSocket;
static char	msg[MSG_LEN];

extern int	NumPlayers, NumRobots, NumPseudoPlayers, NumQueuedPlayers;
extern int	login_in_progress;
extern time_t	serverTime;

void Meta_send(char *mesg, int len)
{
    int			i;

    if (!reportToMetaServer) {
	return;
    }

    for (i = 0; i < NELEM(meta_servers); i++) {
	if (sock_send_dest(&contactSocket, meta_servers[i].addr, META_PORT, mesg, len) != len) {
	    sock_get_error(&contactSocket);
	    sock_send_dest(&contactSocket, meta_servers[i].addr, META_PORT, mesg, len);
	}
    }
}

int Meta_from(char *addr, int port)
{
    int			i;

    for (i = 0; i < NELEM(meta_servers); i++) {
	if (!strcmp(addr, meta_servers[i].addr)) {
	    return (port == META_PORT);
	}
    }
    return 0;
}

void Meta_gone(void)
{
    if (reportToMetaServer) {
	sprintf(msg, "server %s\nremove", Server.host);
	Meta_send(msg, strlen(msg) + 1);
    }
}

void Meta_init(void)
{
    int			i;
    char		*addr;

    if (!reportToMetaServer) {
	return;
    }

#ifndef SILENT
    xpprintf("%s Locating Internet Meta server... ", showtime()); fflush(stdout);
#endif
    for (i = 0; i < NELEM(meta_servers); i++) {
	addr = sock_get_addr_by_name(meta_servers[i].name);
	if (addr) {
	    strlcpy(meta_servers[i].addr, addr,
		    sizeof(meta_servers[i].addr));
	}
#ifndef SILENT
	if (addr) {
	    xpprintf("found %d", i + 1);
	} else {
	    xpprintf("%d not found", i + 1);
	}
	if (i + 1 == NELEM(meta_servers)) {
	    xpprintf("\n");
	} else {
	    xpprintf("... ");
	}
	fflush(stdout);
#endif
    }
}

void Meta_update(int change)
{
#ifdef SOUND
#define SOUND_SUPPORT_STR	"yes"
#else
#define SOUND_SUPPORT_STR	"no"
#endif
#define GIVE_META_SERVER_A_HINT	180

    char 		string[MAX_STR_LEN];
    int			i, j, len;
    int			num_active_players;
    bool		first = true;
    time_t		currentTime;
    const char		*game_mode;
    char		freebases[120];
    int			active_per_team[MAX_TEAMS];
    static time_t	lastMetaSendTime = 0;
    static int		queue_length = 0;


    if (!reportToMetaServer)
	return;

    currentTime = time(NULL);
    if (!change) {
	if (currentTime - lastMetaSendTime < GIVE_META_SERVER_A_HINT) {
	    if (NumQueuedPlayers == queue_length ||
		currentTime - lastMetaSendTime < 5) {
		return;
	    }
	}
    }
    lastMetaSendTime = currentTime;
    queue_length = NumQueuedPlayers;

    /* Find out the number of active players. */
    num_active_players = 0;
    memset(active_per_team, 0, sizeof active_per_team);
    for (i = 0; i < NumPlayers; i++) {
	if (IS_HUMAN_IND(i) && !BIT(Players[i]->status, PAUSE)) {
	    num_active_players++;
	    if (BIT(World.rules->mode, TEAM_PLAY)) {
		active_per_team[i]++;
	    }
	}
    }

    game_mode = (game_lock && ShutdownServer == -1) ? "locked"
		: (!game_lock && ShutdownServer != -1) ? "shutting down"
		: (game_lock && ShutdownServer != -1) ? "locked and shutting down"
		: "ok";

    /* calculate number of available homebases per team. */
    freebases[0] = '\0';
    if (BIT(World.rules->mode, TEAM_PLAY)) {
	j = 0;
	for (i = 0; i < MAX_TEAMS; i++) {
	    if (i == robotTeam && reserveRobotTeam) {
		continue;
	    }
	    if (World.teams[i].NumBases > 0) {
		sprintf(&freebases[j], "%d=%d,", i,
			World.teams[i].NumBases - active_per_team[i]);
		j += strlen(&freebases[j]);
	    }
	}
	/* strip trailing comma. */
	if (j) {
	    freebases[j-1] = '\0';
	}
    }
    else {
	sprintf(freebases, "=%d",
		World.NumBases - num_active_players - login_in_progress);
    }

    sprintf(string,
	    "add server %s\n"
	    "add users %d\n"
	    "add version %s\n"
	    "add map %s\n"
	    "add sizeMap %3dx%3d\n"
	    "add author %s\n"
	    "add bases %d\n"
	    "add fps %d\n"
	    "add port %d\n"
	    "add mode %s\n"
	    "add teams %d\n"
	    "add free %s\n"
	    "add timing %d\n"
	    "add stime %ld\n"
	    "add queue %d\n"
	    "add sound " SOUND_SUPPORT_STR "\n",
	    Server.host, num_active_players,
	    META_VERSION, World.name, World.x, World.y, World.author,
	    World.NumBases, FPS, contactPort,
	    game_mode, World.NumTeamBases, freebases,
	    BIT(World.rules->mode, TIMING) ? 1:0,
	    (long)(time(NULL) - serverTime),
	    queue_length);


    /*
     * 'len' must always hold the exact number of
     * non-zero bytes which are in string[].
     */
    len = strlen(string);

    for (i = 0; i < NumPlayers; i++) {
	if (IS_HUMAN_IND(i) && !BIT(Players[i]->status, PAUSE)) {
	    if ((len + (4 * MAX_CHARS)) < sizeof(string)) {
		sprintf(string + len,
			"%s%s=%s@%s",
			(first) ? "add players " : ",",
			Players[i]->name,
			Players[i]->realname,
			Players[i]->hostname);
		len += strlen(&string[len]);

		if (BIT(World.rules->mode, TEAM_PLAY)) {
		    sprintf(string + len,"{%d}",Players[i]->team);
		    len += strlen(&string[len]);
		}

		first = false;
	    }
	}
    }

    if (len + MSG_LEN < sizeof(string)) {
	char status[MAX_STR_LEN];

	strlcpy(&string[len], "\nadd status ", sizeof(string) - len);
	len += strlen(&string[len]);

	Server_info(status, sizeof(status));

	strlcpy(&string[len], status, sizeof(string) - len);
	len += strlen(&string[len]);
    }

    Meta_send(string, len + 1);
}


/* $Id: metaserver.c,v 4.5 1998/04/16 17:41:37 bert Exp $
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

#ifdef	_WINDOWS
#include "NT/winServer.h"
#include <time.h>
#else
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#if !defined(__hpux)
#include <sys/time.h>
#endif
#endif

#define SERVER
#include "config.h"
#include "version.h"
#include "const.h"
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

#ifdef VMS
#define META_VERSION	VERSION "-VMS"
#elif defined(_WINDOWS)
#define	META_VERSION	TITLE
#else
#define META_VERSION	VERSION
#endif

char metaserver_version[] = VERSION;

#ifndef	lint
char sourceid[] =
    "@(#)$Id: metaserver.c,v 4.5 1998/04/16 17:41:37 bert Exp $";
#endif

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

static int	Socket = -1;
static char	msg[MSG_LEN];

extern int	NumPlayers, NumRobots, NumPseudoPlayers, NumQueuedPlayers;
extern int	login_in_progress;
extern int	game_lock;
extern time_t	serverTime;

void Meta_send(char *mesg, int len)
{
    int			i;

    if (!reportToMetaServer) {
	return;
    }

    for (i = 0; i < NELEM(meta_servers); i++) {
	if (DgramSend(Socket, meta_servers[i].addr, META_PORT, mesg, len) != len) {
	    GetSocketError(Socket);
	    DgramSend(Socket, meta_servers[i].addr, META_PORT, mesg, len);
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

void Meta_init(int fd)
{
    int			i;
    char		*addr;

    Socket = fd;

    if (!reportToMetaServer) {
	return;
    }

#ifndef SILENT
    xpprintf("%s Locating Meta... ", showtime()); fflush(stdout);
#endif
    for (i = 0; i < NELEM(meta_servers); i++) {
	addr = GetAddrByName(meta_servers[i].name);
	if (addr) {
	    strncpy(meta_servers[i].addr, addr,
		    sizeof(meta_servers[i].addr));
	    meta_servers[i].addr[sizeof(meta_servers[i].addr) - 1] = '\0';
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
    char 		status[MAX_STR_LEN];
    int			i, j;
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

    Server_info(status, sizeof(status));

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
	if (j) { freebases[j-1] = '\0'; }
    }
    else {
	sprintf(freebases, "=%d", World.NumBases - num_active_players);
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


    for (i=0; i < NumPlayers; i++) {
	if (IS_HUMAN_IND(i) && !BIT(Players[i]->status, PAUSE)) {
	    sprintf(string + strlen(string),
		    "%s%s=%s@%s",
		    (first) ? "add players " : ",",
		    Players[i]->name,
		    Players[i]->realname,
		    Players[i]->hostname);
	    if (BIT(World.rules->mode, TEAM_PLAY)) {
		sprintf(status,"{%d}",Players[i]->team);
		strcat(string,status);
	    }

	    first = false;
	}
    }

    strcat(string,"\nadd status ");
    if (strlen(string) + strlen(status) >= sizeof(string)) {
	/* Prevent array overflow */
	strcpy(&status[sizeof(string) - (strlen(string) + 2)], "\n");
    }
    strcat(string, status);

    Meta_send(string, strlen(string) + 1);
}


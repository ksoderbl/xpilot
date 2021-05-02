/* $Id: command.c,v 4.3 2000/03/23 20:44:53 bert Exp $
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
#else
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#endif

#include <ctype.h>
#include <string.h>

#define SERVER
#include "version.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "error.h"
#include "netserver.h"


char command_version[] = VERSION;


#ifndef	lint
static char sourceid[] =
    "@(#)$Id: command.c,v 4.3 2000/03/23 20:44:53 bert Exp $";
#endif


static int Get_player_index_by_name(char *name)
{
    int			i, j, len;

    if (!name || !*name) {
	return -1;
    }

    /* Id given directly */
    if (isdigit(*name)) {
	i = atoi(name);
	if ((i > 0 && i <= NUM_IDS)
	    && (j = GetInd[i]) >= 0
	    && j < NumPlayers
	    && Players[j]->id == i) {
	    return j;
	}
	else {
	    return -1;
	}
    }

    /* look for an exact match on player nickname. */
    for (i = 0; i < NumPlayers; i++) {
	if (strcasecmp(Players[i]->name, name) == 0) {
	    return i;
	}
    }

    /* now look for a partial match on both nick and realname. */
    len = strlen(name);
    for (j = -1, i = 0; i < NumPlayers; i++) {
	if (strncasecmp(Players[i]->name, name, len) == 0
	    || strncasecmp(Players[i]->realname, name, len) == 0) {
	    j = (j == -1) ? i : -2;
	}
    }

    return j;
}


static void Send_info_about_player(player * pl)
{
    int			i;

    for (i = 0; i < NumPlayers; i++) {
	if (Players[i]->conn != NOT_CONNECTED) {
	    Send_player(Players[i]->conn, pl->id);
	    Send_score(Players[i]->conn, pl->id, pl->score, pl->life,
		       pl->mychar);
	    Send_base(Players[i]->conn, pl->id, pl->home_base);
	}
    }
}


#define CMD_RESULT_SUCCESS		0
#define CMD_RESULT_ERROR		(-1)
#define CMD_RESULT_NOT_OPERATOR		(-2)
#define CMD_RESULT_NO_NAME		(-3)


static int Cmd_help(char *arg, player *pl, int oper, char *msg);
static int Cmd_team(char *arg, player *pl, int oper, char *msg);
static int Cmd_version(char *arg, player *pl, int oper, char *msg);
static int Cmd_lock(char *arg, player *pl, int oper, char *msg);
static int Cmd_password(char *arg, player *pl, int oper, char *msg);
static int Cmd_pause(char *arg, player *pl, int oper, char *msg);
static int Cmd_reset(char *arg, player *pl, int oper, char *msg);
static int Cmd_set(char *arg, player *pl, int oper, char *msg);
static int Cmd_kick(char *arg, player *pl, int oper, char *msg);
static int Cmd_queue(char *arg, player *pl, int oper, char *msg);
static int Cmd_advance(char *arg, player *pl, int oper, char *msg);


typedef struct {
    const char		*name;
    const char		*abbrev;
    const char		*help;
    int			oper_only;
    int			(*cmd)(char *arg, player *pl, int oper, char *msg);
} Command_info;


static Command_info commands[] = {
    {
	"advance",
	"adv",
	"/advance <name of player in the queue>. "
	"Move the player to the front of the queue.  (operator)",
	1,
	Cmd_advance
    },
    {
	"help",
	"h",
	"Print command list.  /help <command> gives more info.",
	0,
	Cmd_help
    },
    {
	"kick",
	"k",
	"/kick <player name or ID number>.  Remove a player from game.  (operator)",
	1,
	Cmd_kick
    },
    {
	"lock",
	"l",
	"Just /lock tells lock status.  /lock 1 locks, /lock 0 unlocks.  (operator)",
	0,      /* checked in the function */
	Cmd_lock
    },
    {
	"password",
	"pas",
	"/password <string>.  If string matches -password option "
	"gives operator status.",
	0,
	Cmd_password
    },
    {
	"pause",
	"pau",
	"/pause <player name or ID number>.  Pauses player.  (operator)",
	1,
	Cmd_pause
    },
    {
	"queue",
	"q",
	"/queue.  Show the names of players waiting to enter.",
	0,
	Cmd_queue
    },
    {
	"reset",
	"r",
	"Just /reset re-starts the round. "
	"/reset.  Resets all scores to 0.  (operator)",
	1,
	Cmd_reset
    },
    {
	"set",
	"set",
	"/set <option> <value>.  Sets a server option.  (operator)",
	1,
	Cmd_set
    },
    {
	"team",
	"t",
	"/team <team number> swaps you to given team.",
	0,
	Cmd_team
    },
    {
	"version",
	"v",
	"Print server version.",
	0,
	Cmd_version
    },
};


/*
 * cmd parameter has no leading slash.
 */
void Handle_player_command(player *pl, char *cmd)
{
    int			i, result;
    char		*args;
    char		msg[MSG_LEN];

    if (!cmd || !*cmd) {
	strcpy(msg, "No command given.  Type /help for help.  [*Server reply*]");
	Set_player_message(pl, msg);
	return;
    }

    args = strchr(cmd + 1, ' ');
    if (!args) {
	/* point to end of string. */
	args = cmd + strlen(cmd);
    }
    else {
	/* zero terminate cmd and advance 1 byte. */
	*args++ = '\0';
    }

    for (i = 0; i < NELEM(commands); i++) {
	if (!strncasecmp(cmd, commands[i].abbrev, strlen(commands[i].abbrev))) {
	    break;
	}
    }
    if (i == NELEM(commands)) {
	sprintf(msg, "Unknown command '%s'.  [*Server reply*]", cmd);
	Set_player_message(pl, msg);
	return;
    }

    msg[0] = '\0';
    result = (*commands[i].cmd)(args, pl, pl->isoperator, msg);
    switch (result) {
    case CMD_RESULT_SUCCESS:
	break;

    case CMD_RESULT_ERROR:
	if (msg[0] == '\0') {
	    strcpy(msg, "Error.");
	}
	break;

    case CMD_RESULT_NOT_OPERATOR:
	if (msg[0] == '\0') {
	    strcpy(msg, "You need operator status to use this command.");
	}
	break;

    case CMD_RESULT_NO_NAME:
	if (msg[0] == '\0') {
	    strcpy(msg, "You must give a player name as an argument.");
	}
	break;

    default:
	strcpy(msg, "Bug.");
	break;
    }

    if (msg[0]) {
	sprintf(msg + strlen(msg), " [*Server reply*]");
	Set_player_message(pl, msg);
    }
}


static int Cmd_advance(char *arg, player *pl, int oper, char *msg)
{
    int			result;

    if (!oper) {
	return CMD_RESULT_NOT_OPERATOR;
    }

    if (!arg || !*arg) {
	return CMD_RESULT_NO_NAME;
    }

    result = Queue_advance_player(arg, msg);

    if (result < 0) {
	return CMD_RESULT_ERROR;
    }

    return CMD_RESULT_SUCCESS;
}

static int Cmd_queue(char *arg, player *pl, int oper, char *msg)
{
    int			result;

    result = Queue_show_list(msg);

    if (result < 0) {
	return CMD_RESULT_ERROR;
    }

    return CMD_RESULT_SUCCESS;
}


static int Cmd_team(char *arg, player *pl, int oper, char *msg)
{
    int			i;
    int			ind = GetInd[pl->id];
    int			team;
    int			swap_allowed;

    /*
     * Assume nothing will be said or done.
     */
    msg[0] = '\0';
    swap_allowed = false;
    team = pl->team;

    if (!BIT(World.rules->mode, TEAM_PLAY)) {
	sprintf(msg, "No team play going on.");
    }
    else if (pl->team >= MAX_TEAMS) {
	sprintf(msg, "You do not currently have a team.");
    }
    else if (!arg) {
	sprintf(msg, "No team specified.");
    }
    else if (!isdigit(*arg)) {
	sprintf(msg, "Invalid team specification.");
    }
    else {
	team = atoi(arg);
	if (team < 0 || team >= MAX_TEAMS) {
	    sprintf(msg, "Team %d is not a valid team.", team);
	}
	else if (team == pl->team) {
	    sprintf(msg, "You already are on team %d.", team);
	}
	else if (World.teams[team].NumBases == 0) {
	    sprintf(msg, "There are no bases for team %d on this map.", team);
	}
	else if (reserveRobotTeam && team == robotTeam) {
	    sprintf(msg, "You cannot join the robot team on this server.");
	}
	else if (World.teams[team].NumBases <= World.teams[team].NumMembers) {
	    sprintf(msg, "Team %d is full.", team);
	}
	else {
	    swap_allowed = true;
	}
    }

    if (swap_allowed != true) {
	return CMD_RESULT_ERROR;
    }

    sprintf(msg, "%s has swapped to team %d.", pl->name, team);
    Set_message(msg);
    if (BIT(pl->have, OBJ_BALL)) {
	Detach_ball(GetInd[pl->id], -1);
    }
    World.teams[pl->team].NumMembers--;
    pl->team = team;
    World.teams[pl->team].NumMembers++;
    if (BIT(World.rules->mode, LIMITED_LIVES)) {
	for (i = 0; i < NumPlayers; i++) {
	    if (!TEAM(ind, i) && !BIT(Players[i]->status, PAUSE)) {
		/* put team swapping player waiting mode. */
		if (pl->mychar == ' ') {
		    pl->mychar = 'W';
		}
		pl->prev_life = pl->life = 0;
		SET_BIT(pl->status, GAME_OVER | PLAYING);
		CLR_BIT(pl->status, SELF_DESTRUCT);
		pl->count = -1;
		break;
	    }
	}
    }
    Pick_startpos(GetInd[pl->id]);
    Send_info_about_player(pl);
    strcpy(msg, "");

    return CMD_RESULT_SUCCESS;
}


static int Cmd_kick(char *arg, player *pl, int oper, char *msg)
{
    int			i;

    if (!oper) {
	return CMD_RESULT_NOT_OPERATOR;
    }

    if (!arg || !*arg) {
	return CMD_RESULT_NO_NAME;
    }

    i = Get_player_index_by_name(arg);
    if (i >= 0) {
	sprintf(msg, "%s kicked %s out! [*Server notice*]",
		pl->name, Players[i]->name);
	if (Players[i]->conn == NOT_CONNECTED) {
	    Delete_player(i);
	}
#if 0	/* needs testing. */
	else if (pl == &Players[i]) {
	    strcpy(msg, "Cannot kick self this way.");
	    return CMD_RESULT_ERROR;
	}
#endif
	else {
	    Destroy_connection(Players[i]->conn, "kicked out");
	}
	Set_message(msg);
	strcpy(msg, "");
	return CMD_RESULT_SUCCESS;
    }
    else if (i == -1) {
	sprintf(msg, "Name does not match any player.");
    }
    else if (i == -2) {
	sprintf(msg, "Name matches several players.");
    }
    else {
	sprintf(msg, "Error.");
    }

    return CMD_RESULT_ERROR;
}


static int Cmd_version(char *arg, player *pl, int oper, char *msg)
{
    sprintf(msg, "XPilot version %s.", VERSION);
    return CMD_RESULT_SUCCESS;
}


static int Cmd_help(char *arg, player *pl, int oper, char *msg)
{
    int			i;

    if (!*arg) {
	strcpy(msg, "Commands: "
		    "advance "
		    "help "
		    "kick "
		    "lock "
		    "password "
		    "pause "
		    "queue "
		    "reset "
		    "set "
		    "team "
		    "version "
	      );
    }
    else {
	for (i = 0; i < NELEM(commands); i++) {
	    if (!strncasecmp(arg, commands[i].name,
			     strlen(commands[i].abbrev))) {
		break;
	    }
	}
	if (i == NELEM(commands)) {
	    sprintf(msg, "No help for nonexistent command '%s'.", arg);
	}
	else {
	    strcpy(msg, commands[i].help);
	}
    }

    return CMD_RESULT_SUCCESS;
}


static int Cmd_reset(char *arg, player *pl, int oper, char *msg)
{
    int			i;

    if (!oper) {
	return CMD_RESULT_NOT_OPERATOR;
    }

    if (arg && !strcasecmp(arg, "all")) {
	Reset_all_players();
	if (gameDuration == -1) {
	    gameDuration = 0;
	}
	roundsPlayed = 0;
	for (i = NumPlayers - 1; i >= 0; i--) {
	    Players[i]->score = 0;
	}

	sprintf(msg, " < Total reset by %s! >", pl->name);
	Set_message(msg);
	strcpy(msg, "");

	return CMD_RESULT_SUCCESS;
    }
    else {
	Reset_all_players();
	if (gameDuration == -1) {
	    gameDuration = 0;
	}

	sprintf(msg, " < Round reset by %s! >", pl->name);
	Set_message(msg);
	strcpy(msg, "");

	return CMD_RESULT_SUCCESS;
    }
}


static int Cmd_password(char *arg, player *pl, int oper, char *msg)
{
    if (!password || !arg || strcmp(arg, password)) {
	strcpy(msg, "Wrong.");
	if (pl->isoperator) {
	    pl->isoperator = 0;
	    strcat(msg, "  You lost operator status.");
	}
    }
    else {
	if (!pl->isoperator) {
	    pl->isoperator = 1;
	}
	strcpy(msg, "You got operator status.");
    }
    return CMD_RESULT_SUCCESS;
}


static int Cmd_lock(char *arg, player *pl, int oper, char *msg)
{
    int			new_lock;

    if (!arg || !*arg) {
	sprintf(msg, "The game is currently %s.",
		game_lock ? "locked" : "unlocked");
	return CMD_RESULT_SUCCESS;
    }

    if (!oper) {
	return CMD_RESULT_NOT_OPERATOR;
    }

    if (!strcmp(arg, "1")) {
	new_lock = 1;
    }
    else if (!strcmp(arg, "0")) {
	new_lock = 0;
    }
    else {
	sprintf(msg, "Invalid argument '%s'.  Specify either 0 or 1.", arg);
	return CMD_RESULT_ERROR;
    }

    if (new_lock == game_lock) {
	sprintf(msg, "Game is already %s.",
		game_lock ? "locked" : "unlocked");
    }
    else {
	game_lock = new_lock;
	sprintf(msg, " < The game has been %s by %s! >",
		game_lock ? "locked" : "unlocked",
		pl->name);
	Set_message(msg);
	strcpy(msg, "");
    }

    return CMD_RESULT_SUCCESS;
}


static int Cmd_set(char *arg, player *pl, int oper, char *msg)
{
    int			i;
    char		*option;
    char		*value;

    if (!oper) {
	return CMD_RESULT_NOT_OPERATOR;
    }

    if (!arg
	|| !(option = strtok(arg, " "))
	|| !(value = strtok(NULL, " "))) {

	sprintf(msg, "Usage: /set option value.");
	return CMD_RESULT_ERROR;
    }

    i = Tune_option(option, value);
    if (i == 1) {
	if (!strcasecmp(option, "password"))
	    sprintf(msg, "Operation successful.");
	else {
	    sprintf(msg, " < Option %s set to %s by %s. >",
		    option, value, pl->name);
	    Set_message(msg);
	    strcpy(msg, "");
	    return CMD_RESULT_SUCCESS;
	}
    }
    else if (i == 0) {
	sprintf(msg, "Invalid value.");
    }
    else if (i == -1) {
	sprintf(msg, "This option cannot be changed at runtime.");
    }
    else if (i == -2) {
	sprintf(msg, "No option named '%s'.", option);
    }
    else {
	sprintf(msg, "Error.");
    }

    return CMD_RESULT_ERROR;
}


static int Cmd_pause(char *arg, player *pl, int oper, char *msg)
{
    int			i;

    if (!oper) {
	return CMD_RESULT_NOT_OPERATOR;
    }

    if (!arg || !*arg) {
	return CMD_RESULT_NO_NAME;
    }

    i = Get_player_index_by_name(arg);
    if (i >= 0) {
	if (Players[i]->conn != NOT_CONNECTED) {
	    if (BIT(Players[i]->status, PLAYING | PAUSE | GAME_OVER | KILLED)
		== PLAYING) {
		Kill_player(i);
	    }
	    Pause_player(i, 1);
	    sprintf(msg, "%s was paused by %s.", Players[i]->name, pl->name);
	    Set_message(msg);
	    strcpy(msg, "");
	    return CMD_RESULT_SUCCESS;
	}
    }

    sprintf(msg, "Invalid player id.");

    return CMD_RESULT_ERROR;
}



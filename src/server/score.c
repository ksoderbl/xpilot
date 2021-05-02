/* $Id: score.c,v 5.7 2002/01/21 22:04:03 kimiko Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
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
#include <stdio.h>
#include <math.h>
#include <limits.h>

#ifdef _WINDOWS
# include "NT/winServer.h"
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "score.h"
#include "netserver.h"


char score_version[] = VERSION;


void SCORE(int ind, DFLOAT points, int x, int y, const char *msg)
{
    player	*pl = Players[ind];

    if (BIT(World.rules->mode, TEAM_PLAY)) {
	if (!teamShareScore) {
	    pl->score += points;
	}
	TEAM_SCORE(pl->team, points);
    } else {
	if (pl->alliance != ALLIANCE_NOT_SET && teamShareScore) {
	    Alliance_score(pl->alliance, points);
	} else {
	    pl->score += points;
	}
    }

    if (pl->conn != NOT_CONNECTED)
	Send_score_object(pl->conn, points, x, y, msg);

    updateScores = true;
}

void TEAM_SCORE(int team, DFLOAT points)
{
    if (team == TEAM_NOT_SET)	/* could happen if teamCannons is off */
	return;
    
    World.teams[team].score += points;
    if (teamShareScore) {
	int i;
	DFLOAT share = World.teams[team].score / World.teams[team].NumMembers;
	for (i = 0; i < NumPlayers; i++) {
	    if (Players[i]->team == team) {
		Players[i]->score = share;
	    }
	}
    }

    updateScores = true;
}

void Alliance_score(int id, DFLOAT points)
{
    int		i;
    int		member_count = Get_alliance_member_count(id);
    DFLOAT	share = points / member_count;

    for (i = 0; i < NumPlayers; i++) {
	if (Players[i]->alliance == id) {
	    Players[i]->score += share;
	}
    }
}

DFLOAT Rate(DFLOAT winner, DFLOAT loser)
{
    DFLOAT t;

    t = ((RATE_SIZE / 2) * RATE_RANGE) / (ABS(loser - winner) + RATE_RANGE);
    if (loser > winner)
	t = RATE_SIZE - t;
    return (t);
}

/*
 * Cause `winner' to get `winner_score' points added with message
 * `winner_msg', and similarly with the `loser' and equivalent
 * variables.
 *
 * In general the winner_score should be positive, and the loser_score
 * negative, but this need not be true.
 *
 * If the winner and loser players are on the same team, the scores are
 * made negative, since you shouldn't gain points by killing team members,
 * or being killed by a team member (it is both players faults).
 *
 * KK 28-4-98: Same for killing your own tank.
 * KK 7-11-1: And for killing a member of your alliance
 */
void Score_players(int winner, DFLOAT winner_score, char *winner_msg,
		   int loser, DFLOAT loser_score, char *loser_msg)
{
    if (TEAM(winner, loser)
	|| (Players[winner]->alliance != ALLIANCE_NOT_SET
	    && Players[winner]->alliance == Players[loser]->alliance)
	|| (IS_TANK_IND(loser)
	    && GetInd[Players[loser]->lock.pl_id] == winner)) {
	if (winner_score > 0)
	    winner_score = -winner_score;
	if (loser_score > 0)
	    loser_score = -loser_score;
    }
    SCORE(winner, winner_score,
	  OBJ_X_IN_BLOCKS(Players[loser]),
	  OBJ_Y_IN_BLOCKS(Players[loser]),
	  winner_msg);
    SCORE(loser, loser_score,
	  OBJ_X_IN_BLOCKS(Players[loser]),
	  OBJ_Y_IN_BLOCKS(Players[loser]),
	  loser_msg);
}


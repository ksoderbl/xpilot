/* $Id: score.c,v 5.2 2001/06/03 17:28:37 bertg Exp $
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
#include "const.h"
#include "global.h"
#include "proto.h"
#include "score.h"
#include "netserver.h"


char score_version[] = VERSION;


void SCORE(int ind, int points, int x, int y, const char *msg)
{
    player	*pl = Players[ind];

    pl->score += (points);

    if (pl->conn != NOT_CONNECTED)
	Send_score_object(pl->conn, points, x, y, msg);

    updateScores = true;
}

int Rate(int winner, int loser)
{
    int t;

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
 * BD 28-4-98: Same for killing your own tank.
 */
void Score_players(int winner, int winner_score, char *winner_msg,
		   int loser, int loser_score, char *loser_msg)
{
    if (TEAM(winner, loser)
	|| (IS_TANK_IND(loser)
	    && GetInd[Players[loser]->lock.pl_id] == winner)) {
	if (winner_score > 0)
	    winner_score = -winner_score;
	if (loser_score > 0)
	    loser_score = -loser_score;
    }
    SCORE(winner, winner_score,
	  OBJ_X_IN_BLOCKS(Players[winner]),
	  OBJ_Y_IN_BLOCKS(Players[winner]),
	  winner_msg);
    SCORE(loser, loser_score,
	  OBJ_X_IN_BLOCKS(Players[loser]),
	  OBJ_Y_IN_BLOCKS(Players[loser]),
	  loser_msg);
}


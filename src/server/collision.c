/* $Id: collision.c,v 4.23 1999/11/08 04:56:08 dick Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
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

#ifdef	_WINDOWS
#include "NT/winServer.h"
#include <math.h>
#include <limits.h>
#else
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "score.h"
#include "saudio.h"
#include "item.h"
#include "netserver.h"
#include "pack.h"
#include "error.h"
#include "objpos.h"

char collision_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: collision.c,v 4.23 1999/11/08 04:56:08 dick Exp $";
#endif

#if 0
/* this macro doesn't work for edgewrap: */
#define in_range_old(o1, o2, r)			\
    (DELTA((o1)->pos.x, (o2)->pos.x) < (r)	\
     && DELTA((o1)->pos.y, (o2)->pos.y) < (r))
#else
/* proposed edgewrap version: */
#define in_range_old(o1, o2, r)						\
    (BIT(World.rules->mode, WRAP_PLAY)					\
	? ((((o1)->pos.x > (o2)->pos.x)					\
	    ? (((o1)->pos.x - (o2)->pos.x > (World.width >> 1))		\
		? ((o1)->pos.x - (o2)->pos.x > World.width - (r))	\
		: ((o1)->pos.x - (o2)->pos.x < (r)))			\
	    : (((o2)->pos.x - (o1)->pos.x > (World.width >> 1))		\
		? ((o2)->pos.x - (o1)->pos.x > World.width - (r))	\
		: ((o2)->pos.x - (o1)->pos.x < (r))))			\
	    && (((o1)->pos.y > (o2)->pos.y)				\
	    ? (((o1)->pos.y - (o2)->pos.y > (World.height >> 1))	\
		? ((o1)->pos.y - (o2)->pos.y > World.height - (r))	\
		: ((o1)->pos.y - (o2)->pos.y < (r)))			\
	    : (((o2)->pos.y - (o1)->pos.y > (World.height >> 1))	\
		? ((o2)->pos.y - (o1)->pos.y > World.height - (r))	\
		: ((o2)->pos.y - (o1)->pos.y < (r)))))			\
	: (DELTA((o1)->pos.x, (o2)->pos.x) < (r)			\
	&& DELTA((o1)->pos.y, (o2)->pos.y) < (r)))
#endif

/*
 * The very first "analytical" collision patch, XPilot 3.6.2
 * Faster than other patches and accurate below half warp-speed
 * Trivial common subexpressions are eliminated by any reasonable compiler, 
 * and kept here for readability.
 * Written by Pontus (Rakk, Kepler) pontus@ctrl-c.liu.se Jan 1998
 * Kudos to Svenske and Mad Gurka for beta testing, and Murx for
 * invaluable insights.
 */
#if 0
int in_range_acd(int p1x, int p1y, int p2x, int p2y,
		 int q1x, int q1y, int q2x, int q2y,
		 int r)
{
    long		fac1, fac2;
    double		tmin, fminx, fminy;
    long		top, bot;
    long		dpx, dpy, dqx, dqy;
    long		dx, dy, dox, doy;

    /*
     * Get the wrapped coordinates straight 
     */
    if (BIT(World.rules->mode, WRAP_PLAY)) {
	if (ABS(p2x - p1x) > World.width / 2) {
	    if (p1x < p2x)
		p1x += World.width;
	    else
		p2x += World.width;
	}
	if (ABS(p2y - p1y) > World.height / 2) {
	    if (p1y < p2y)
		p1y += World.height;
	    else
		p2y += World.height;
	}
	if (ABS(q2x - q1x) > World.width / 2) {
	    if (q1x < q2x)
		q1x += World.width;
	    else
		q2x += World.width;
	}
	if (ABS(q2y - q1y) > World.height / 2) {
	    if (q1y < q2y)
		q1y += World.height;
	    else
		q2y += World.height;
	}
    }

    dx = WRAP_DX(q2x - p2x);
    dy = WRAP_DY(q2y - p2y);
    if (sqr(dx) + sqr(dy) < sqr(r))
	return 1;

    dox = WRAP_DX(p1x - q1x);
    doy = WRAP_DY(p1y - q1y);
    if (sqr(dox) + sqr(doy) < sqr(r))
	return 1;

    dpx = WRAP_DX(p2x - p1x);
    dpy = WRAP_DY(p2y - p1y);
    dqx = WRAP_DX(q2x - q1x);
    dqy = WRAP_DY(q2y - q1y);

    /*
     * Do the detection 
     */
    fac1 = dpx - dqx;
    fac2 = dpy - dqy;
    top = -(fac1 * dx + fac2 * dy);
    bot = (fac1 * fac1 + fac2 * fac2);
    if (top < 0 || bot < 1 || top > bot)
	return 0;
    tmin = ((double)top) / ((double)bot);	/* BG: could make top&bot doubles. */
    fminx = dx + fac1 * tmin;
    fminy = dy + fac2 * tmin;
    if (fminx * fminx + fminy * fminy < r * r)
	return 1;
    else
	return 0;
}
#else
int in_range_acd(int p1x, int p1y, int p2x, int p2y,
		 int q1x, int q1y, int q2x, int q2y,
		 int r)
{
    long	fac1, fac2;
    double	tmin, fminx, fminy;
    long	top, bot;
    bool	mpx, mpy, mqx, mqy;

    /*
     * Get the wrapped coordinates straight 
     */
    if (BIT(World.rules->mode, WRAP_PLAY)) {
	if ((mpx = (ABS(p2x - p1x) > World.width / 2))) {
	    if (p1x > p2x)
		p1x -= World.width;
	    else
		p2x -= World.width;
	}
	if ((mpy = (ABS(p2y - p1y) > World.height / 2))) {
	    if (p1y > p2y)
		p1y -= World.height;
	    else
		p2y -= World.height;
	}
	if ((mqx = (ABS(q2x - q1x) > World.width / 2))) {
	    if (q1x > q2x)
		q1x -= World.width;
	    else
		q2x -= World.width;
	}
	if ((mqy = (ABS(q2y - q1y) > World.height / 2))) {
	    if (q1y > q2y)
		q1y -= World.height;
	    else
		q2y -= World.height;
	}

	if (mpx && !mqx && (q2x > World.width / 2 || q1x > World.width / 2)) {
	    q1x -= World.width;
	    q2x -= World.width;
	}

	if (mqy && !mpy && (q2y > World.height / 2 || q1y > World.height / 2)) {
	    q1y -= World.height;
	    q2y -= World.height;
	}

	if (mqx && !mpx && (p2x > World.width / 2 || p1x > World.width / 2)) {
	    p1x -= World.width;
	    p2x -= World.width;
	}

	if (mqy && !mpy && (p2y > World.height / 2 || p1y > World.height / 2)) {
	    p1y -= World.height;
	    p2y -= World.height;
	}
    }

    /*
     * Do the detection 
     */
    if ((p2x - q2x) * (p2x - q2x) + (p2y - q2y) * (p2y - q2y) < r * r)
	return 1;
    fac1 = -p1x + p2x + q1x - q2x;
    fac2 = -p1y + p2y + q1y - q2y;
    top = -(fac1 * (-p2x + q2x) + fac2 * (-p2y + q2y));
    bot = (fac1 * fac1 + fac2 * fac2);
    if (top < 0 || bot < 1 || top > bot)
	return 0;
    tmin = ((double)top) / ((double)bot);
    fminx = -p2x + q2x + fac1 * tmin;
    fminy = -p2y + q2y + fac2 * tmin;
    if (fminx * fminx + fminy * fminy < r * r)
	return 1;
    else
	return 0;
}
#endif

/*
 * Globals
 */
extern long KILLING_SHOTS;
static char msg[MSG_LEN];

static object ***Cells;
static object **CellsUsed[MAX_TOTAL_SHOTS];
static int cells_used_count;


int Rate(int winner, int looser);
static void PlayerCollision(void);
static void LaserCollision(void);
static void PlayerObjectCollision(int ind);


void Free_cells(void)
{
    if (Cells) {
	free(Cells);
	Cells = NULL;
    }
    cells_used_count = 0;
}


void Alloc_cells(void)
{
    unsigned		size;
    object		**objp;
    int			x, y;

    Free_cells();

    size = sizeof(object ***) * World.x;
    size += sizeof(object **) * World.x * World.y;
    if (!(Cells = (object ***) malloc(size))) {
	error("No Cell mem");
	End_game();
    }
    objp = (object **) &Cells[World.x];
    for (x = 0; x < World.x; x++) {
	Cells[x] = objp;
	for (y = 0; y < World.y; y++) {
	    *objp++ = NULL;
	}
    }
}


static void Cell_objects_init(void)
{
    int			i,
			x,
			y;
    object		*obj,
			**cell;

    for (i = 0; i < cells_used_count; i++) {
	*CellsUsed[i] = NULL;
    }
    cells_used_count = 0;
    for (i = 0; i < NumObjs; i++) {
	obj = Obj[i];
	if (obj->life <= 0) {
	    continue;
	}
	x = OBJ_X_IN_BLOCKS(obj);
	y = OBJ_Y_IN_BLOCKS(obj);
	cell = &Cells[x][y];
	if (!(obj->cell_list = *cell)) {
	    CellsUsed[cells_used_count++] = cell;
	}
	*cell = obj;
    }
}


static void Cell_objects_get(int x, int y, int r, object ***list, int *count)
{
    static object	*ObjectList[MAX_TOTAL_SHOTS + 1];
    int			i,
			minx, maxx, miny, maxy,
			xr, yr, xw, yw;
    object		*obj;

    if (BIT(World.rules->mode, WRAP_PLAY)) {
	if (2*r > World.x) {
	    r = World.x / 2;
	}
	if (2*r > World.y) {
	    r = World.y / 2;
	}
    }
    else {
	if (r > World.x) {
	    r = World.x;
	}
	if (r > World.y) {
	    r = World.y;
	}
    }
    minx = x - r;
    maxx = x + r;
    miny = y - r;
    maxy = y + r;
    if (BIT(World.rules->mode, WRAP_PLAY)) {
	if (minx < 0) {
	    minx += World.x;
	    maxx += World.x;
	}
	if (miny < 0) {
	    miny += World.y;
	    maxy += World.y;
	}
    }
    else {
	if (minx < 0) {
	    minx = 0;
	}
	if (miny < 0) {
	    miny = 0;
	}
	if (maxx >= World.x) {
	    maxx = World.x - 1;
	}
	if (maxy >= World.y) {
	    maxy = World.y - 1;
	}
    }
    i = 0;
    for (xr = xw = minx; xr <= maxx; xr++, xw++) {
	if (xw >= World.x) {
	    xw -= World.x;
	}
	for (yr = yw = miny; yr <= maxy; yr++, yw++) {
	    if (yw >= World.y) {
		yw -= World.y;
	    }
	    for (obj = Cells[xw][yw]; obj; obj = obj->cell_list) {
		ObjectList[i++] = obj;
	    }
	}
    }
    ObjectList[i] = NULL;
    *list = &ObjectList[0];
    if (count != NULL) {
	*count = i;
    }
}

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
static void Score_players(int winner, int winner_score, char *winner_msg,
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

void Check_collision(void)
{
    if (rdelay == 0) {
	Cell_objects_init();
	PlayerCollision();
    }
    LaserCollision();
}

static void PlayerCollision(void)
{
    int			i, j, sc, sc2;
    player		*pl;

    /* Player - player, checkpoint, treasure, object and wall */
    for (i=0; i<NumPlayers; i++) {
	pl = Players[i];
	if (BIT(pl->status, PLAYING|PAUSE|GAME_OVER|KILLED) != PLAYING)
	    continue;

	if (pl->pos.x < 0 || pl->pos.y < 0
	    || pl->pos.x >= World.width
	    || pl->pos.y >= World.height) {
	    SET_BIT(pl->status, KILLED);
	    sprintf(msg, "%s left the known universe.", pl->name);
	    Set_message(msg);
	    sc = Rate(WALL_SCORE, pl->score);
	    SCORE(i, -sc,
		  OBJ_X_IN_BLOCKS(pl),
		  OBJ_Y_IN_BLOCKS(pl),
		  pl->name);
	    continue;
	}

	if (BIT(pl->used, OBJ_PHASING_DEVICE))
	    continue;

	/* Player - player */
	if (BIT(World.rules->mode, CRASH_WITH_PLAYER | BOUNCE_WITH_PLAYER)) {
	    for (j=i+1; j<NumPlayers; j++) {
		if (BIT(Players[j]->status, PLAYING|PAUSE|GAME_OVER|KILLED)
		    != PLAYING) {
		    continue;
		}
		if (BIT(Players[j]->used, OBJ_PHASING_DEVICE))
		    continue;
		if (!anaColDet) {
		    if (!in_range_old((object *)pl, (object *)Players[j],
				      2*SHIP_SZ-6)) {
			continue;
		    }
		} else {
		    if (!in_range_acd(pl->prevpos.x, pl->prevpos.y, pl->pos.x, pl->pos.y, 
				      Players[j]->prevpos.x, Players[j]->prevpos.y, 
				      Players[j]->pos.x, Players[j]->pos.y, 
				      2*SHIP_SZ-6)) {
			continue;
		    }
		}

		/*
		 * Here we can add code to do more accurate player against
		 * player collision detection.
		 * A new algorithm could be based on the following idea:
		 *
		 * - If we can draw an uninterupted line between two players:
		 *   - Then test for both ships:
		 *     - For the three points which make up a ship:
		 *       - If we can draw a line between its previous
		 *         position and its current position which does not
		 *         cross the first line.
		 * Then the ships have not collided even though they may be
		 * very close to one another.
		 * The choosing of the first line may not be easy however.
		 */

		if (TEAM_IMMUNE(i, j) || PSEUDO_TEAM(i, j)) {
		    continue;
		}
		sound_play_sensors(pl->pos.x, pl->pos.y, PLAYER_HIT_PLAYER_SOUND);
		if (BIT(World.rules->mode, BOUNCE_WITH_PLAYER)) {
		    if (BIT(pl->used, (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD)) !=
			(OBJ_SHIELD|OBJ_EMERGENCY_SHIELD)) {
			Add_fuel(&(pl->fuel), (long)ED_PL_CRASH);
			Item_damage(i, destroyItemInCollisionProb);
		    }
		    if (BIT(Players[j]->used, (OBJ_SHIELD|
					       OBJ_EMERGENCY_SHIELD)) !=
			(OBJ_SHIELD|OBJ_EMERGENCY_SHIELD)) {
			Add_fuel(&(Players[j]->fuel), (long)ED_PL_CRASH);
			Item_damage(j, destroyItemInCollisionProb);
		    }
		    pl->forceVisible = 20;
		    Players[j]->forceVisible = 20;
		    Obj_repel((object *)pl, (object *)Players[j],
			      2*SHIP_SZ);
		}
		if (!BIT(World.rules->mode, CRASH_WITH_PLAYER)) {
		    continue;
		}

		if (pl->fuel.sum <= 0
		    || (!BIT(pl->used, OBJ_SHIELD)
			&& !BIT(pl->have, OBJ_ARMOR))) {
		    SET_BIT(pl->status, KILLED);
		}
		if (Players[j]->fuel.sum <= 0
		    || (!BIT(Players[j]->used, OBJ_SHIELD)
			&& !BIT(Players[j]->have, OBJ_ARMOR))) {
		    SET_BIT(Players[j]->status, KILLED);
		}

		if (!BIT(pl->used, OBJ_SHIELD)
		    && BIT(pl->have, OBJ_ARMOR)) {
		    Player_hit_armor(i);
		}
		if (!BIT(Players[j]->used, OBJ_SHIELD)
		    && BIT(Players[j]->have, OBJ_ARMOR)) {
		    Player_hit_armor(j);
		}

		if (BIT(Players[j]->status, KILLED)) {
		    if (BIT(pl->status, KILLED)) {
			sprintf(msg, "%s and %s crashed.",
				pl->name, Players[j]->name);
			Set_message(msg);
			if (!IS_TANK_IND(i) && !IS_TANK_IND(j)) {
			    sc = (int)floor(Rate(Players[j]->score, pl->score)
			 		    * crashScoreMult);
			    sc2 = (int)floor(Rate(pl->score, Players[j]->score)
					     * crashScoreMult);
			    Score_players(i, -sc, Players[j]->name,
					  j, -sc2, pl->name);
			} else if (IS_TANK_IND(i)) {
			    int i_tank_owner = GetInd[Players[i]->lock.pl_id];
			    sc = (int)floor(Rate(Players[i_tank_owner]->score,
						 Players[j]->score)
					    * tankKillScoreMult);
			    Score_players(i_tank_owner, sc, Players[j]->name,
					  j, -sc, pl->name);
			} else if (IS_TANK_IND(j)) {
			    int j_tank_owner = GetInd[Players[j]->lock.pl_id];
			    sc = (int)floor(Rate(Players[j_tank_owner]->score,
						 pl->score)
					    * tankKillScoreMult);
			    Score_players(j_tank_owner, sc, pl->name,
					  i, -sc, Players[j]->name);
			} /* don't bother scoring two tanks */
		    } else {
			int i_tank_owner = i;
			if (IS_TANK_IND(i)) {
			    i_tank_owner = GetInd[Players[i]->lock.pl_id];
			    if (i_tank_owner == j) {
				i_tank_owner = i;
			    }
			}
			sprintf(msg, "%s ran over %s.",
				pl->name, Players[j]->name);
			Set_message(msg);
			sound_play_sensors(Players[j]->pos.x,
					   Players[j]->pos.y,
					   PLAYER_RAN_OVER_PLAYER_SOUND);
			pl->kills++;
			if (IS_TANK_IND(i)) {
			    sc = (int)floor(Rate(Players[i_tank_owner]->score,
						 Players[j]->score)
					    * tankKillScoreMult);
			} else {
			    sc = (int)floor(Rate(pl->score, Players[j]->score)
					    * runoverKillScoreMult);
			}
			Score_players(i_tank_owner, sc, Players[j]->name,
				      j, -sc, pl->name);
		    }

		} else {
		    if (BIT(pl->status, KILLED)) {
			int j_tank_owner = j;
			if (IS_TANK_IND(j)) {
			    j_tank_owner = GetInd[Players[j]->lock.pl_id];
			    if (j_tank_owner == i) {
				j_tank_owner = j;
			    }
			}
			sprintf(msg, "%s ran over %s.",
				Players[j]->name, pl->name);
			Set_message(msg);
			sound_play_sensors(pl->pos.x, pl->pos.y,
					   PLAYER_RAN_OVER_PLAYER_SOUND);
			Players[j]->kills++;
			sc = (int)floor(Rate(Players[j]->score, pl->score)
				   * runoverKillScoreMult);
			Score_players(j_tank_owner, sc, pl->name,
				      i, -sc, Players[j]->name);
		    }
		}

		if (BIT(Players[j]->status, KILLED)) {
		    if (IS_ROBOT_IND(j)
			&& Robot_war_on_player(j) == pl->id) {
			Robot_reset_war(j);
		    }
		}

		if (BIT(pl->status, KILLED)) {
		    if (IS_ROBOT_PTR(pl)
			&& Robot_war_on_player(i) == Players[j]->id) {
			Robot_reset_war(i);
		    }
		    /* cannot crash with more than one player at the same time? */
		    /* hmm, if 3 players meet at the same point at the same time? */
		    /* break; */
		}
	    }
	}

	/* Player picking up ball/treasure */
	if (!BIT(pl->used, OBJ_CONNECTOR) || BIT(pl->used, OBJ_PHASING_DEVICE)) {
	    pl->ball = NULL;
	}
	else if (pl->ball != NULL) {
	    object *ball = pl->ball;
	    if (ball->life <= 0 || ball->id != -1)
		pl->ball = NULL;
	    else {
		DFLOAT distance = Wrap_length(pl->pos.x - ball->pos.x,
					     pl->pos.y - ball->pos.y);
		if (distance >= BALL_STRING_LENGTH) {
		    ball->id = pl->id;
		    /* this is only the team of the owner of the ball,
		       not the team the ball belongs to. the latter is
		       found through the ball's treasure */
		    ball->team = pl->team;
		    if (ball->owner == -1)
			ball->life=LONG_MAX;  /* for frame counter */
		    ball->owner = pl->id;
		    ball->length = distance;
		    SET_BIT(ball->status, GRAVITY);
		    if (ball->treasure != -1)
			World.treasures[ball->treasure].have = false;
		    SET_BIT(pl->have, OBJ_BALL);
		    pl->ball = NULL;
		    sound_play_sensors(pl->pos.x, pl->pos.y,
				       CONNECT_BALL_SOUND);
		}
	    }
	} else {
	    /*
	     * We want a separate list of balls to avoid searching
	     * the object list for balls.
	     */
	    int dist, mindist = BALL_STRING_LENGTH;
	    for (j = 0; j < NumObjs; j++) {
		if (BIT(Obj[j]->type, OBJ_BALL) && Obj[j]->id == -1) {
		    dist = Wrap_length(pl->pos.x - Obj[j]->pos.x,
				       pl->pos.y - Obj[j]->pos.y);
		    if (dist < mindist) {
			object *ball = Obj[j];
			int bteam = -1;

			if (ball->treasure != -1)
			    bteam = World.treasures[ball->treasure].team;

			/*
			 * The treasure's team cannot connect before
			 * somebody else has owned the ball.
			 * This was done to stop team members
			 * taking and hiding with the ball... this was
			 * considered bad gamesmanship.
			 */
			if (ball->owner != -1
			    || (   pl->team != TEAM_NOT_SET
				&& pl->team != bteam)) {
			    pl->ball = Obj[j];
			    mindist = dist;
			}
		    }
		}
	    }
	}

	PlayerObjectCollision(i);

	/* Player checkpoint */
	if (BIT(World.rules->mode, TIMING)
	    && BIT(pl->status, PAUSE|GAME_OVER) == 0) {
	    if (pl->round != 0) {
		pl->time++;
	    }
	    if (BIT(pl->status, PLAYING|KILLED) == PLAYING
		&& Wrap_length(pl->pos.x - World.check[pl->check].x * BLOCK_SZ,
			       pl->pos.y - World.check[pl->check].y * BLOCK_SZ)
		    < checkpointRadius * BLOCK_SZ
		&& !IS_TANK_PTR(pl)) {

		if (pl->check == 0) {
		    pl->round++;
		    pl->last_lap_time = pl->time - pl->last_lap;
		    if ((pl->best_lap > pl->last_lap_time
			    || pl->best_lap == 0)
			&& pl->time != 0
			&& pl->round != 1) {
			pl->best_lap = pl->last_lap_time;
		    }
		    pl->last_lap = pl->time;
		    if (pl->round > raceLaps) {
			Player_death_reset(i);
			pl->mychar = 'D';
			SET_BIT(pl->status, GAME_OVER|FINISH);
			sprintf(msg,
				"%s finished the race. Last lap time: %.2fs. "
				"Personal race best lap time: %.2fs.",
				pl->name,
				(DFLOAT) pl->last_lap_time / FPS,
				(DFLOAT) pl->best_lap / FPS);
		    }
		    else if (pl->round > 1) {
			sprintf(msg,
				"%s completes lap %d in %.2fs. "
				"Personal race best lap time: %.2fs.",
				pl->name,
				pl->round-1,
				(DFLOAT) pl->last_lap_time / FPS,
				(DFLOAT) pl->best_lap / FPS);
		    }
		    else {
			sprintf(msg, "%s starts lap 1 of %d", pl->name,
				raceLaps);
		    }
		    Set_message(msg);
		}

		if (++pl->check == World.NumChecks)
		    pl->check = 0;
		pl->last_check_dir = pl->dir;

		updateScores = true;
	    }
	}
    }
}

int IsOffensiveItem(enum Item i)
{
    if (BIT(1 << i,
	    ITEM_BIT_WIDEANGLE |
	    ITEM_BIT_REARSHOT |
	    ITEM_BIT_MINE |
	    ITEM_BIT_MISSILE |
	    ITEM_BIT_LASER)) {
	return true;
    }
    return false;
}

int IsDefensiveItem(enum Item i)
{
    if (BIT(1 << i,
	    ITEM_BIT_CLOAK |
	    ITEM_BIT_ECM |
	    ITEM_BIT_TRANSPORTER | 
	    ITEM_BIT_TRACTOR_BEAM |
	    ITEM_BIT_EMERGENCY_SHIELD |
	    ITEM_BIT_MIRROR |
	    ITEM_BIT_DEFLECTOR |
	    ITEM_BIT_HYPERJUMP |
	    ITEM_BIT_PHASING |
	    ITEM_BIT_TANK |
	    ITEM_BIT_ARMOR)) {
	return true;
    }
    return false;
}

int CountOffensiveItems(player *pl)
{
    return (pl->item[ITEM_WIDEANGLE] + pl->item[ITEM_REARSHOT] + 
	    pl->item[ITEM_MINE] + pl->item[ITEM_MISSILE] + 
	    pl->item[ITEM_LASER]); 
}

int CountDefensiveItems(player *pl)
{
    int count;

    count = pl->item[ITEM_CLOAK] + pl->item[ITEM_ECM] + pl->item[ITEM_ARMOR] +
	    pl->item[ITEM_TRANSPORTER] + pl->item[ITEM_TRACTOR_BEAM] + 
	    pl->item[ITEM_EMERGENCY_SHIELD] + pl->fuel.num_tanks +
	    pl->item[ITEM_DEFLECTOR] + pl->item[ITEM_HYPERJUMP] +
	    pl->item[ITEM_PHASING] + pl->item[ITEM_MIRROR];
    if (pl->emergency_shield_left) 
 	count++;
    if (pl->phasing_left)
	count++;
    return count;
}

static void PlayerObjectCollision(int ind)
{
    int		j, killer, old_have, range, radius, sc, hit, obj_count;
    long	drain;
    player	*pl = Players[ind];
    object	*obj, **obj_list;
    DFLOAT   	rel_velocity, drainfactor;
    enum Item	item_index;


    /*
     * Collision between a player and an object.
     */
    if (BIT(pl->status, PLAYING|PAUSE|GAME_OVER|KILLED) != PLAYING)
	return;

    Cell_objects_get(OBJ_X_IN_BLOCKS(pl), OBJ_Y_IN_BLOCKS(pl), 4,
		     &obj_list, &obj_count);

    for (j=0; j<obj_count; j++) {
	obj = obj_list[j];
	if (obj->life <= 0)
	    continue;

	range = SHIP_SZ + obj->pl_range;
	if (!anaColDet) {
	    if (!in_range_old((object *)pl, obj, range)) {
		continue;
	    }
	} else {
	    if (!in_range_acd(pl->prevpos.x, pl->prevpos.y, pl->pos.x, pl->pos.y,
			      obj->prevpos.x, obj->prevpos.y, obj->pos.x, obj->pos.y,
			      range)) {
		continue;
	    }
	}

	if (obj->id != -1) {
	    if (obj->id == pl->id) {
		if (BIT(obj->type, OBJ_SPARK|OBJ_MINE)
		    && BIT(obj->status, OWNERIMMUNE)) {
		    continue;
		}
	    } else if (BIT(World.rules->mode, TEAM_PLAY)
		       && teamImmunity
		       && obj->team == pl->team) {
		continue;
	    }
	}

	if (obj->type == OBJ_ITEM) {
	    if (BIT(pl->used, OBJ_SHIELD) && !shieldedItemPickup) {
		SET_BIT(obj->status, GRAVITY);
		Delta_mv((object *)pl, obj);
		continue;
	    }
	}
	else if (BIT(obj->type, OBJ_HEAT_SHOT | OBJ_SMART_SHOT | OBJ_TORPEDO
				| OBJ_SHOT)) {
	    if (pl->id == obj->id && obj->life > obj->fuselife) {
		continue;
	    }
	}
	else if (BIT(obj->type, OBJ_MINE)) {
	    if (BIT(obj->status, CONFUSED)) {
		continue;
	    }
	}

	/*
	 * Objects actually only hit the player if they are really close.
	 */
	radius = SHIP_SZ + obj->pl_radius;
	if (radius >= range) {
	    hit = 1;
	} else {
	    if (!anaColDet) {
		hit = in_range_old((object *)pl, obj, radius);
	    } else {
		hit = in_range_acd(pl->prevpos.x,pl->prevpos.y,pl->pos.x,pl->pos.y,
				   obj->prevpos.x,obj->prevpos.y,obj->pos.x,obj->pos.y,
				   range);
	    }
	}
	rel_velocity = LENGTH(pl->vel.x - obj->vel.x, pl->vel.y - obj->vel.y);

	/*
	 * Object collision.
	 */
	switch (obj->type) {
	case OBJ_BALL:
	    if (! hit)
		continue;

	    /*
	     * The ball is special, usually players bounce off of it with
	     * shields up, or die with shields down.  The treasure may
	     * be destroyed.
	     */
	    Obj_repel((object *)pl, obj, radius);
	    if (BIT(pl->used, (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD))
		!= (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD)) {
		Add_fuel(&(pl->fuel), (long)ED_BALL_HIT);
		if (treasureCollisionDestroys) {
		    obj->life = 0;
		}
	    }
	    if (pl->fuel.sum > 0) {
		if (!treasureCollisionMayKill || BIT(pl->used, OBJ_SHIELD))
		    continue;
		if (!BIT(pl->used, OBJ_SHIELD) && BIT(pl->have, OBJ_ARMOR)) {
		    Player_hit_armor(ind);
		    continue;
		}
	    }
	    if (obj->owner == -1) {
		sprintf(msg, "%s was killed by a ball.", pl->name);
		SCORE(ind, PTS_PR_PL_SHOT,
		      OBJ_X_IN_BLOCKS(pl),
		      OBJ_Y_IN_BLOCKS(pl),
		      "Ball");
	    } else {
		killer = GetInd[obj->owner];

		sprintf(msg, "%s was killed by a ball owned by %s.",
			pl->name, Players[killer]->name);

		if (killer == ind) {
		    strcat(msg, "  How strange!");
		    SCORE(ind, PTS_PR_PL_SHOT,
			  OBJ_X_IN_BLOCKS(pl),
			  OBJ_Y_IN_BLOCKS(pl),
			  Players[killer]->name);
		} else {
		    Players[killer]->kills++;
		    sc = (int)floor(Rate(Players[killer]->score, pl->score)
			       * ballKillScoreMult);
		    Score_players(killer, sc, pl->name,
				  ind, -sc, Players[killer]->name);
		}
	    }
	    Set_message(msg);
	    SET_BIT(pl->status, KILLED);
	    return;

	case OBJ_ITEM:
	    if (IsOffensiveItem((enum Item) obj->info)) {
		int off_items = CountOffensiveItems(pl);
		if (off_items >= maxOffensiveItems) {
		    /* Set_player_message(pl, "No space left for offensive items."); */
		    Delta_mv((object *)pl, obj);
		    continue;
		} 
		else if (obj->count > 1
			 && off_items + obj->count > maxOffensiveItems) {
		    obj->count = maxOffensiveItems - off_items;
		}
	    } 
	    else if (IsDefensiveItem((enum Item) obj->info)) {
		int def_items = CountDefensiveItems(pl);
		if (def_items >= maxDefensiveItems) {
		    /* Set_player_message(pl, "No space for left for defensive items."); */
		    Delta_mv((object *)pl, obj);
		    continue;
		} 
		else if (obj->count > 1
			 && def_items + obj->count > maxDefensiveItems) {
		    obj->count = maxDefensiveItems - def_items;
		}
	    }

	    item_index = (enum Item) obj->info;

	    switch (item_index) {
	    case ITEM_WIDEANGLE:
		pl->item[item_index] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		sound_play_sensors(pl->pos.x, pl->pos.y,
				   WIDEANGLE_SHOT_PICKUP_SOUND);
		break;
	    case ITEM_ECM:
		pl->item[item_index] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		sound_play_sensors(pl->pos.x, pl->pos.y, ECM_PICKUP_SOUND);
		break;
	    case ITEM_ARMOR:
		pl->item[item_index]++;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		if (pl->item[item_index] > 0)
		    SET_BIT(pl->have, OBJ_ARMOR);
		sound_play_sensors(pl->pos.x, pl->pos.y,
				   ARMOR_PICKUP_SOUND);
		break;
	    case ITEM_TRANSPORTER:
		pl->item[item_index] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		sound_play_sensors(pl->pos.x, pl->pos.y, TRANSPORTER_PICKUP_SOUND);
		break;
	    case ITEM_MIRROR:
		pl->item[ITEM_MIRROR] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		if (pl->item[item_index] > 0)
		    SET_BIT(pl->have, OBJ_MIRROR);
		sound_play_sensors(pl->pos.x, pl->pos.y, MIRROR_PICKUP_SOUND);
		break;
	    case ITEM_DEFLECTOR:
		pl->item[ITEM_DEFLECTOR] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		if (pl->item[item_index] > 0)
		    SET_BIT(pl->have, OBJ_DEFLECTOR);
		sound_play_sensors(pl->pos.x, pl->pos.y, DEFLECTOR_PICKUP_SOUND);
		break;
	    case ITEM_HYPERJUMP:
		pl->item[item_index] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		sound_play_sensors(pl->pos.x, pl->pos.y, HYPERJUMP_PICKUP_SOUND);
		break;
	    case ITEM_PHASING:
		pl->item[item_index] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		if (pl->item[item_index] > 0)
		    SET_BIT(pl->have, OBJ_PHASING_DEVICE);
		sound_play_sensors(pl->pos.x, pl->pos.y, PHASING_DEVICE_PICKUP_SOUND);
		break;
	    case ITEM_SENSOR:
		pl->item[item_index] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		pl->updateVisibility = 1;
		sound_play_sensors(pl->pos.x, pl->pos.y, SENSOR_PACK_PICKUP_SOUND);
		break;
	    case ITEM_AFTERBURNER:
		pl->item[item_index] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		if (pl->item[item_index] > 0)
		    SET_BIT(pl->have, OBJ_AFTERBURNER);
		sound_play_sensors(pl->pos.x, pl->pos.y, AFTERBURNER_PICKUP_SOUND);
		break;
	    case ITEM_REARSHOT:
		pl->item[item_index] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		sound_play_sensors(pl->pos.x, pl->pos.y, BACK_SHOT_PICKUP_SOUND);
		break;
	    case ITEM_MISSILE:
		pl->item[item_index] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		sound_play_sensors(pl->pos.x, pl->pos.y, ROCKET_PACK_PICKUP_SOUND);
		break;
	    case ITEM_CLOAK:
		pl->item[item_index] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		if (pl->item[item_index] > 0)
		    SET_BIT(pl->have, OBJ_CLOAKING_DEVICE);
		pl->updateVisibility = 1;
		sound_play_sensors(pl->pos.x, pl->pos.y, CLOAKING_DEVICE_PICKUP_SOUND);
		break;
	    case ITEM_FUEL:
		Add_fuel(&(pl->fuel), ENERGY_PACK_FUEL);
		sound_play_sensors(pl->pos.x, pl->pos.y, ENERGY_PACK_PICKUP_SOUND);
		break;
	    case ITEM_MINE:
		pl->item[item_index] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		sound_play_sensors(pl->pos.x, pl->pos.y, MINE_PACK_PICKUP_SOUND);
		break;
	    case ITEM_LASER:
		pl->item[item_index] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		sound_play_sensors(pl->pos.x, pl->pos.y, LASER_PICKUP_SOUND);
		break;
	    case ITEM_EMERGENCY_THRUST:
		pl->item[item_index] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		if (pl->item[item_index] > 0)
		    SET_BIT(pl->have, OBJ_EMERGENCY_THRUST);
		sound_play_sensors(pl->pos.x, pl->pos.y,
				   EMERGENCY_THRUST_PICKUP_SOUND);
		break;
	    case ITEM_EMERGENCY_SHIELD:
		old_have = pl->have;
		pl->item[item_index] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		if (pl->item[item_index] > 0)
		    SET_BIT(pl->have, OBJ_EMERGENCY_SHIELD);
		sound_play_sensors(pl->pos.x, pl->pos.y,
				   EMERGENCY_SHIELD_PICKUP_SOUND);
	        /*
	         * New feature since 3.2.7:
	         * If we're playing in a map where shields are not allowed
	         * and a player picks up her first emergency shield item
	         * then we'll immediately turn on emergency shield.
	         */
	        if (!BIT(old_have, OBJ_SHIELD | OBJ_EMERGENCY_SHIELD)
		    && pl->item[ITEM_EMERGENCY_SHIELD] == 1) {
		    Emergency_shield(ind, 1);
		}
		break;
	    case ITEM_TRACTOR_BEAM:
		pl->item[item_index] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		if (pl->item[item_index] > 0)
		    SET_BIT(pl->have, OBJ_TRACTOR_BEAM);
		sound_play_sensors(pl->pos.x, pl->pos.y,
				   TRACTOR_BEAM_PICKUP_SOUND);
		break;
	    case ITEM_AUTOPILOT:
		pl->item[item_index] += obj->count;
		LIMIT(pl->item[item_index], 0, World.items[item_index].limit);
		if (pl->item[item_index] > 0)
		    SET_BIT(pl->have, OBJ_AUTOPILOT);
		sound_play_sensors(pl->pos.x, pl->pos.y,
				   AUTOPILOT_PICKUP_SOUND);
		break;

	    case ITEM_TANK:
		if (pl->fuel.num_tanks < World.items[ITEM_TANK].limit) {
		    Player_add_tank(ind, TANK_FUEL(pl->fuel.num_tanks + 1));
		} else {
		    Add_fuel(&(pl->fuel), TANK_FUEL(MAX_TANKS));
		}
		sound_play_sensors(pl->pos.x, pl->pos.y, TANK_PICKUP_SOUND);
		break;
	    case NUM_ITEMS:
		/* impossible */
		break;
	    }
	    break;

	case OBJ_MINE:
	    sound_play_sensors(pl->pos.x, pl->pos.y, PLAYER_HIT_MINE_SOUND);
	    killer = -1;
	    if (obj->id == -1 && obj->owner == -1) {
		sprintf(msg, "%s hit %s.",
			pl->name,
			Describe_shot(obj->type, obj->status, obj->mods, 1));
	    }
	    else if (obj->owner == obj->id) {
		killer = GetInd[obj->owner];
		sprintf(msg, "%s hit %s %s by %s.", pl->name,
			Describe_shot(obj->type, obj->status, obj->mods,1),
			BIT(obj->status, GRAVITY) ? "thrown " : "dropped ",
			Players[killer]->name);
	    }
	    else if (obj->owner == -1) {
		const char *reprogrammer_name = "some jerk";
		if (obj->id != -1) {
		    killer = GetInd[obj->id];
		    reprogrammer_name = Players[killer]->name;
		}
		sprintf(msg, "%s hit %s reprogrammed by %s.",
			pl->name,
			Describe_shot(obj->type, obj->status, obj->mods, 1),
			reprogrammer_name);
	    }
	    else {
		const char *reprogrammer_name = "some jerk";
		if (obj->id != -1) {
		    killer = GetInd[obj->id];
		    reprogrammer_name = Players[killer]->name;
		}
		sprintf(msg, "%s hit %s %s by %s and reprogrammed by %s.",
			pl->name,
			Describe_shot(obj->type, obj->status, obj->mods,1),
			BIT(obj->status, GRAVITY) ? "thrown " : "dropped ",
			Players[GetInd[obj->owner]]->name,
			reprogrammer_name);
	    }
	    if (killer != -1) {
		/*
		 * Question with this is if we want to give the same points for
		 * a high-scored-player hitting a low-scored-player's mine as
		 * for a low-scored-player hitting a high-scored-player's mine.
		 * Maybe not.
		 */
		sc = (int)floor(Rate(Players[killer]->score, pl->score)
			   * mineScoreMult);
		Score_players(killer, sc, pl->name,
			      ind, -sc, Players[killer]->name);
	    }
	    Set_message(msg);
	    break;

	case OBJ_WRECKAGE:
	case OBJ_DEBRIS: {
		DFLOAT		v = VECTOR_LENGTH(obj->vel);
		long		tmp = (long) (2 * obj->mass * v);
		long		cost = ABS(tmp);

		if (BIT(pl->used, (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD))
		    != (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD))
		    Add_fuel(&pl->fuel, - cost);
		if (pl->fuel.sum == 0
		    || (obj->type == OBJ_WRECKAGE
			&& wreckageCollisionMayKill
			&& !BIT(pl->used, OBJ_SHIELD)
			&& !BIT(pl->have, OBJ_ARMOR))) {
		    SET_BIT(pl->status, KILLED);
		    sprintf(msg, "%s succumbed to an explosion.", pl->name);
		    killer = -1;
		    if (obj->id != -1) {
			killer = GetInd[obj->id];
			sprintf(msg + strlen(msg) - 1, " from %s.",
				Players[killer]->name);
			if (obj->id == pl->id) {
			    sprintf(msg + strlen(msg), "  How strange!");
			}
		    }
		    Set_message(msg);
		    if (killer == -1 || killer == ind) {
			SCORE(ind, PTS_PR_PL_SHOT,
			      OBJ_X_IN_BLOCKS(pl),
			      OBJ_Y_IN_BLOCKS(pl),
			      (killer == -1) ? "[Explosion]" : pl->name);
		    } else {
			Players[killer]->kills++;
			sc = (int)floor(Rate(Players[killer]->score, pl->score)
				   * explosionKillScoreMult);
			Score_players(killer, sc, pl->name,
				      ind, -sc, Players[killer]->name);
		    }
		    return;
		}
		if (obj->type == OBJ_WRECKAGE
		    && wreckageCollisionMayKill
		    && !BIT(pl->used, OBJ_SHIELD)
		    && BIT(pl->have, OBJ_ARMOR)) {
		    Player_hit_armor(ind);
		}
	    }
	    break;

	default:
	    break;
	}

	obj->life = 0;
	if (hit) Delta_mv((object *)pl, (object *)obj);

	if (!BIT(obj->type, KILLING_SHOTS))
	    continue;

	/*
	 * Player got hit by a potentially deadly object.
	 *
	 * When a player has shields up, and not enough fuel
	 * to `absorb' the shot then shields are lowered.
	 * This is not very logical, rather in this case
	 * the shot should be considered to be deadly too.
	 *
	 * Sound effects are missing when shot is deadly.
	 */

	if (BIT(pl->used, OBJ_SHIELD)
	    || BIT(pl->have, OBJ_ARMOR)
	    || (obj->type == OBJ_TORPEDO
		&& BIT(obj->mods.nuclear, NUCLEAR)
		&& (int)(rfrac() >= 0.25f))) {
	    switch (obj->type) {
	    case OBJ_TORPEDO:
		sound_play_sensors(pl->pos.x, pl->pos.y,
				   PLAYER_EAT_TORPEDO_SHOT_SOUND);
		break;
	    case OBJ_HEAT_SHOT:
		sound_play_sensors(pl->pos.x, pl->pos.y,
				   PLAYER_EAT_HEAT_SHOT_SOUND);
		break;
	    case OBJ_SMART_SHOT:
		sound_play_sensors(pl->pos.x, pl->pos.y,
				   PLAYER_EAT_SMART_SHOT_SOUND);
		break;
	    }

	    switch(obj->type) {
	    case OBJ_TORPEDO:
	    case OBJ_HEAT_SHOT:
	    case OBJ_SMART_SHOT:
		if (obj->id == -1)
		    sprintf(msg, "%s ate %s.", pl->name,
			    Describe_shot(obj->type, obj->status,
					  obj->mods, 1));
		else
		    sprintf(msg, "%s ate %s from %s.", pl->name,
			    Describe_shot(obj->type, obj->status,
					  obj->mods, 1),
			    Players[ killer=GetInd[obj->id] ]->name);
		drain = (long)(ED_SMART_SHOT_HIT /
		    ((obj->mods.mini + 1) * (obj->mods.power + 1)));
		if (BIT(pl->used, (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD))
		    != (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD))
		    Add_fuel(&(pl->fuel), drain);
		pl->forceVisible += 2;
		Set_message(msg);
		break;

	    case OBJ_SHOT:
		sound_play_sensors(pl->pos.x, pl->pos.y,
				   PLAYER_EAT_SHOT_SOUND);
		if (BIT(pl->used, (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD))
		    != (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD)) {
#ifdef DRAINFACTOR
/* BG: this is bad: one shot causes way too much damage. */
		    drainfactor = (rel_velocity * rel_velocity * ABS(obj->mass))
				/ (ShotsSpeed * ShotsSpeed * ShotsMass);
#else
		    drainfactor = 1;
#endif
		    drain = (long)(ED_SHOT_HIT * drainfactor * SHOT_MULT(obj));
		    Add_fuel(&(pl->fuel), drain);
		}
		pl->forceVisible = (int)(pl->forceVisible + SHOT_MULT(obj));
		break;

	    default:
		xpprintf("%s You were hit by what?\n", showtime());
		break;
	    }
	    if (pl->fuel.sum <= 0) {
		CLR_BIT(pl->used, OBJ_SHIELD);
	    }
	    if (!BIT(pl->used, OBJ_SHIELD) && BIT(pl->have, OBJ_ARMOR)) {
		Player_hit_armor(ind);
	    }
	} else {
	    switch (obj->type) {
	    case OBJ_TORPEDO:
	    case OBJ_SMART_SHOT:
	    case OBJ_HEAT_SHOT:
	    case OBJ_SHOT:
		if (BIT(obj->status, FROMCANNON)) {
		    sound_play_sensors(pl->pos.x, pl->pos.y,
				       PLAYER_HIT_CANNONFIRE_SOUND);
		    sprintf(msg, "%s was hit by cannonfire.", pl->name);
		    Set_message(msg);
		    sc = Rate(CANNON_SCORE, pl->score)/4;
		    SCORE(ind, -sc,
			  OBJ_X_IN_BLOCKS(pl),
			  OBJ_Y_IN_BLOCKS(pl),
			  "Cannon");
		    SET_BIT(pl->status, KILLED);
		    return;
		}

		if (obj->id == -1) {
		    sprintf(msg, "%s was killed by %s.", pl->name,
			    Describe_shot(obj->type, obj->status,
					  obj->mods, 1));
		    SCORE(ind, PTS_PR_PL_SHOT,
			  OBJ_X_IN_BLOCKS(pl),
			  OBJ_Y_IN_BLOCKS(pl),
			  "N/A");
		    killer = ind;
		} else {
		    sprintf(msg, "%s was killed by %s from %s.", pl->name,
			    Describe_shot(obj->type, obj->status,
					  obj->mods, 1),
			    Players[killer=GetInd[obj->id]]->name);
		    if (killer == ind) {
			sound_play_sensors(pl->pos.x, pl->pos.y,
					   PLAYER_SHOT_THEMSELF_SOUND);
			strcat(msg, "  How strange!");
			SCORE(ind, PTS_PR_PL_SHOT,
			      OBJ_X_IN_BLOCKS(pl),
			      OBJ_Y_IN_BLOCKS(pl),
			      Players[killer]->name);
		    } else {
			DFLOAT factor;
			Players[killer]->kills++;
			switch (obj->type) {
			case OBJ_SHOT:
			    if (BIT(obj->mods.warhead, CLUSTER)) {
				factor = clusterKillScoreMult;
			    } else {
				factor = shotKillScoreMult;
			    }
			    break;
			case OBJ_TORPEDO:
			    factor = torpedoKillScoreMult;
			    break;
			case OBJ_SMART_SHOT:
			    factor = smartKillScoreMult;
			    break;
			case OBJ_HEAT_SHOT:
			    factor = heatKillScoreMult;
			    break;
			default:
			    factor = shotKillScoreMult;
			    break;
			}
			sc = (int)floor(Rate(Players[killer]->score, pl->score)
				   * factor);
			Score_players(killer, sc, pl->name,
				      ind, -sc, Players[killer]->name);
		    }
		}
		Set_message(msg);
		SET_BIT(pl->status, KILLED);
		Robot_war(ind, killer);
		return;

	    default:
		break;
	    }
	}
    }
}

int wormXY(int x, int y)
{
/*-BA Faster way to do this
 *    static int cache;
 *    int i;
 *
 *    if (World.wormHoles[cache].pos.x == x &&
 *	World.wormHoles[cache].pos.y == y)
 *	return cache;
 *
 *    for (i = 0; i < World.NumWormholes; i++)
 *	if (World.wormHoles[i].pos.x == x &&
 *	    World.wormHoles[i].pos.y == y)
 *	    break;
 *
 *    cache = i;
 *
 *    return i;
 */
    return World.itemID[x][y];
}


static void LaserCollision(void)
{
    typedef struct victim {
	int			ind;	/* player index */
	position		pos;
	DFLOAT			prev_dist;
    } victim_t;

    int				ind, i, j, p, max, hits, sc,
				max_victims = 0,
				num_victims = 0,
				objnum = -1;
    victim_t			*victims = NULL;
    DFLOAT			x, y, x1, x2, y1, y2, dx, dy, dist;
    player			*pl, *vic;
    pulse_t			*pulse;
    object			*obj = NULL;
    char			msg[MSG_LEN];

    for (p = 0; p < NumPulses; p++) {
	    pulse = Pulses[p];
	    if (pulse->id != -1) {
		ind = GetInd[pulse->id];
		pl = Players[ind];
	    } else {
		ind = -1;
		pl = NULL;
	    }
	    if (--pulse->life < 0) {
		free(Pulses[p]);
		if (--NumPulses > p) {
		    Pulses[p] = Pulses[NumPulses];
		    p--;
		}
		if (pl)
		    pl->num_pulses--;
		continue;
	    }
	    if (pulse->len < PULSE_LENGTH) {
		pulse->len = 0;
		continue;
	    }
	    if (obj == NULL) {
		if (NumObjs >= MAX_TOTAL_SHOTS) {
		    pulse->len = 0;
		    continue;
		}
		objnum = NumObjs++;
		obj = Obj[objnum];
	    }

	    pulse->pos.x += tcos(pulse->dir) * PULSE_SPEED;
	    pulse->pos.y += tsin(pulse->dir) * PULSE_SPEED;
	    if (BIT(World.rules->mode, WRAP_PLAY)) {
		if (pulse->pos.x < 0) {
		    pulse->pos.x += World.width;
		}
		else if (pulse->pos.x >= World.width) {
		    pulse->pos.x -= World.width;
		}
		if (pulse->pos.y < 0) {
		    pulse->pos.y += World.height;
		}
		else if (pulse->pos.y >= World.height) {
		    pulse->pos.y -= World.height;
		}
		x1 = pulse->pos.x;
		y1 = pulse->pos.y;
		x2 = x1 + tcos(pulse->dir) * pulse->len;
		y2 = y1 + tsin(pulse->dir) * pulse->len;
	    } else {
		x1 = pulse->pos.x;
		y1 = pulse->pos.y;
		if (x1 < 0 || x1 >= World.width
		    || y1 < 0 || y1 >= World.height) {
		    pulse->len = 0;
		    continue;
		}
		x2 = x1 + tcos(pulse->dir) * pulse->len;
		if (x2 < 0) {
		    pulse->len = (int)(pulse->len * (0 - x1) / (x2 - x1));
		    x2 = x1 + tcos(pulse->dir) * pulse->len;
		}
		if (x2 >= World.width) {
		    pulse->len = (int)(pulse->len * (World.width - 1 - x1)
			/ (x2 - x1));
		    x2 = x1 + tcos(pulse->dir) * pulse->len;
		}
		y2 = y1 + tsin(pulse->dir) * pulse->len;
		if (y2 < 0) {
		    pulse->len = (int)(pulse->len * (0 - y1) / (y2 - y1));
		    x2 = x1 + tcos(pulse->dir) * pulse->len;
		    y2 = y1 + tsin(pulse->dir) * pulse->len;
		}
		if (y2 > World.height) {
		    pulse->len = (int)(pulse->len * (World.height - 1 - y1)
			/ (y2 - y1));
		    x2 = x1 + tcos(pulse->dir) * pulse->len;
		    y2 = y1 + tsin(pulse->dir) * pulse->len;
		}
		if (pulse->len <= 0) {
		    pulse->len = 0;
		    continue;
		}
	    }

	    if (rdelay == 0) {
	    	num_victims = 0;
	    	for (i = 0; i < NumPlayers; i++) {
		    vic = Players[i];
		    if (BIT(vic->status, PLAYING|GAME_OVER|KILLED|PAUSE)
		    	!= PLAYING) {
		    	continue;
		    }
		    if (BIT(vic->used, OBJ_PHASING_DEVICE)) {
		    	continue;
		    }
		    if (BIT(World.rules->mode, TEAM_PLAY)
			&& teamImmunity
			&& vic->team == pulse->team
			&& vic->id != pulse->id) {
			continue;
		    }
		    if (vic->id == pulse->id && !pulse->refl) {
			continue;
		    }
		    if (Wrap_length(vic->pos.x - x1, vic->pos.y - y1)
		    	> pulse->len + SHIP_SZ) {
		    	continue;
		    }
		    if (max_victims == 0) {
		    	victims = (victim_t *) malloc(NumPlayers * sizeof(victim_t));
		    	if (victims == NULL) {
			    break;
		    	}
		    	max_victims = NumPlayers;
		    }
		    victims[num_victims].ind = i;
		    victims[num_victims].pos.x = vic->pos.x;
		    victims[num_victims].pos.y = vic->pos.y;
		    victims[num_victims].prev_dist = 1e10;
		    num_victims++;
	    	}
	    }

	    dx = x2 - x1;
	    dy = y2 - y1;
	    max = (int)MAX(ABS(dx), ABS(dy));
	    if (max == 0) {
		continue;
	    }

	    obj->type = OBJ_PULSE;
	    obj->life = 1;
	    obj->owner = pulse->id;
	    obj->id = pulse->id;
	    obj->team = pulse->team;
	    obj->count = 0;
	    if (pulse->id == -1)
		obj->status = FROMCANNON;
	    Object_position_init_pixels(obj, x1, y1);

	    for (i = hits = 0; i <= max; i += PULSE_SAMPLE_DISTANCE) {
		x = x1 + (i * dx) / max;
		y = y1 + (i * dy) / max;
		obj->vel.x = (x - CLICK_TO_FLOAT(obj->pos.cx));
		obj->vel.y = (y - CLICK_TO_FLOAT(obj->pos.cy));
		/* changed from = x - obj->pos.x to make lasers disappear
		   less frequently when wrapping. There's still a small
		   chance of it happening though. */
		Move_object(objnum);
		if (obj->life == 0) {
		    break;
		}
		if (BIT(World.rules->mode, WRAP_PLAY)) {
		    if (x < 0) {
			x += World.width;
			x1 += World.width;
		    }
		    else if (x >= World.width) {
			x -= World.width;
			x1 -= World.width;
		    }
		    if (y < 0) {
			y += World.height;
			y1 += World.height;
		    }
		    else if (y >= World.height) {
			y -= World.height;
			y1 -= World.height;
		    }
		}
		if (rdelay > 0)
		    continue;	/* don't check collision with players */
		for (j = 0; j < num_victims; j++) {
		    dist = Wrap_length(x - victims[j].pos.x,
				       y - victims[j].pos.y);
		    if (dist <= SHIP_SZ) {
			vic = Players[victims[j].ind];
			vic->forceVisible++;
			if (BIT(vic->have, OBJ_MIRROR)
			    && (rfrac() * (2 * vic->item[ITEM_MIRROR])) >= 1) {
			    pulse->pos.x = x - tcos(pulse->dir) * 0.5
						* PULSE_SAMPLE_DISTANCE;
			    pulse->pos.y = y - tsin(pulse->dir) * 0.5
						* PULSE_SAMPLE_DISTANCE;
			    pulse->dir = (int)Wrap_findDir(vic->pos.x - pulse->pos.x,
						      vic->pos.y - pulse->pos.y)
					 * 2 - RES / 2 - pulse->dir;
			    pulse->dir = MOD2(pulse->dir, RES);
			    pulse->life += vic->item[ITEM_MIRROR];
			    pulse->len = PULSE_LENGTH;
			    pulse->refl = true;
			    continue;
			}
			hits++;
			sound_play_sensors(vic->pos.x, vic->pos.y,
					   PLAYER_EAT_LASER_SOUND);
			if (BIT(vic->used, (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD))
			    == (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD))
			    continue;
			if (!BIT(obj->type, KILLING_SHOTS))
			    continue;
			if (BIT(pulse->mods.laser, STUN)
			    || (laserIsStunGun == true
				&& allowLaserModifiers == false)) {
			    if (BIT(vic->used, OBJ_SHIELD|OBJ_LASER|OBJ_SHOT)
				|| BIT(vic->status, THRUSTING)) {
				if (pl) {
				    sprintf(msg,
					"%s got paralysed by %s's stun laser.",
					vic->name, pl->name);
				    if (vic->id == pl->id)
					strcat(msg, " How strange!");
				} else {
				    sprintf(msg,
					"%s got paralysed by a stun laser.",
					vic->name);
				}
				Set_message(msg);
				CLR_BIT(vic->used,
					OBJ_SHIELD|OBJ_LASER|OBJ_SHOT);
				CLR_BIT(vic->status, THRUSTING);
				vic->stunned += 5;
			    }
			} else if (BIT(pulse->mods.laser, BLIND)) {
			    vic->damaged += (FPS + 6);
			    vic->forceVisible += (FPS + 6);
			    if (pl)
				Record_shove(vic, pl, frame_loops + FPS + 6);
			} else {
			    Add_fuel(&(vic->fuel), (long)ED_LASER_HIT);
			    if (!BIT(vic->used, OBJ_SHIELD)
				&& !BIT(vic->have, OBJ_ARMOR)) {
				SET_BIT(vic->status, KILLED);
				if (pl) {
				    sprintf(msg,
					"%s got roasted alive by %s's laser.",
					vic->name, pl->name);
				    if (vic->id == pl->id) {
					SCORE(victims[j].ind, PTS_PR_PL_SHOT,
					      OBJ_X_IN_BLOCKS(vic),
					      OBJ_Y_IN_BLOCKS(vic),
					      vic->name);
					strcat(msg, " How strange!");
				    } else {
					sc = (int)floor(Rate(pl->score,
							     vic->score)
					     * laserKillScoreMult);
					Score_players(ind, sc, vic->name,
						      victims[j].ind, -sc,
						      pl->name);
				    }
				} else {
				    sc = Rate(CANNON_SCORE, vic->score) / 4;
				    SCORE(victims[j].ind, -sc,
					  OBJ_X_IN_BLOCKS(vic),
					  OBJ_Y_IN_BLOCKS(vic),
					  "Cannon");
				    sprintf(msg,
					"%s got roasted alive by cannonfire.",
					vic->name);
				}
				sound_play_sensors(vic->pos.x, vic->pos.y,
						   PLAYER_ROASTED_SOUND);
				Set_message(msg);
				if (pl && pl->id != vic->id) {
				    pl->kills++;
				    Robot_war(victims[j].ind, ind);
				}
			    }
			    if (!BIT(vic->used, OBJ_SHIELD)
				&& BIT(vic->have, OBJ_ARMOR)) {
				Player_hit_armor(victims[j].ind);
			    }

			}
		    }
		    else if (dist >= victims[j].prev_dist) {
			victims[j] = victims[--num_victims];
			j--;
		    } else {
			victims[j].prev_dist = dist;
		    }
		}
		if (hits > 0) {
		    break;
		}
	    }
	    if (i < max) {
		pulse->len = (pulse->len * i) / max;
	    }
    }
    if (max_victims > 0 && victims != NULL) {
	free(victims);
    }
    if (objnum >= 0 && obj != NULL) {
	obj->type = OBJ_DEBRIS;
	obj->life = 0;
    }
}

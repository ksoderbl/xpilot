/* $Id: collision.c,v 3.136 1996/04/10 15:14:50 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
 *
 *      Bj�rn Stabell        (bjoerns@staff.cs.uit.no)
 *      Ken Ronny Schouten   (kenrsc@stud.cs.uit.no)
 *      Bert G�sbers         (bert@mc.bio.uva.nl)
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

#define SERVER
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

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

char collision_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: collision.c,v 3.136 1996/04/10 15:14:50 bert Exp $";
#endif

#define in_range(o1, o2, r)			\
    (ABS((o1)->pos.x - (o2)->pos.x) < (r)	\
     && ABS((o1)->pos.y - (o2)->pos.y) < (r))	\
    /* this in_range() macro still needs an edgewrap modification */


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
	x = obj->pos.x / BLOCK_SZ;
	y = obj->pos.y / BLOCK_SZ;
#if 0
	if (x < 0 || x > World.x || y < 0 || y > World.y) {
	    printf("Object cell bad pos (%d,%d)\n", x, y);
	    continue;
	}
#endif
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

void SCORE(int ind, int points, int x, int y, char *msg)
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

    t = ((RATE_SIZE/2) * RATE_RANGE) / (ABS(loser-winner) + RATE_RANGE);
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
 */
void Score_players(int winner, int winner_score, char *winner_msg,
		   int loser, int loser_score, char *loser_msg)
{
    if (TEAM(winner, loser)) {
	if (winner_score > 0)
	    winner_score = -winner_score;
	if (loser_score > 0)
	    loser_score = -loser_score;
    }
    SCORE(winner, winner_score,
	  Players[winner]->pos.x/BLOCK_SZ, Players[winner]->pos.y/BLOCK_SZ,
	  winner_msg);
    SCORE(loser, loser_score,
	  Players[loser]->pos.x/BLOCK_SZ, Players[loser]->pos.y/BLOCK_SZ,
	  loser_msg);
}

void Check_collision(void)
{
    Cell_objects_init();
    PlayerCollision();
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
		  (int) pl->pos.x/ BLOCK_SZ,
		  (int) pl->pos.y/BLOCK_SZ,
		  pl->name);
	    continue;
	}

	/* Player - player */
	if (BIT(World.rules->mode, CRASH_WITH_PLAYER | BOUNCE_WITH_PLAYER)) {
	    for (j=i+1; j<NumPlayers; j++) {
		if (BIT(Players[j]->status, PLAYING|PAUSE|GAME_OVER|KILLED)
		    != PLAYING) {
		    continue;
		}
		if (!in_range((object *)pl, (object *)Players[j],
				2*SHIP_SZ-6)) {
		    continue;
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

		if (TEAM_IMMUNE(i, j) || PSEUDO_TEAM(i,j)) {
		    continue;
		}
		sound_play_sensors(pl->pos.x, pl->pos.y, PLAYER_HIT_PLAYER_SOUND);
		if (BIT(World.rules->mode, BOUNCE_WITH_PLAYER)) {
		    if (BIT(pl->used, (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD)) !=
			(OBJ_SHIELD|OBJ_EMERGENCY_SHIELD)) {
			Add_fuel(&(pl->fuel), ED_PL_CRASH);
			Item_damage(i, destroyItemInCollisionProb);
		    }
		    if (BIT(Players[j]->used, (OBJ_SHIELD|
					       OBJ_EMERGENCY_SHIELD)) !=
			(OBJ_SHIELD|OBJ_EMERGENCY_SHIELD)) {
			Add_fuel(&(Players[j]->fuel), ED_PL_CRASH);
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
		if (pl->fuel.sum <= 0 || !BIT(pl->used, OBJ_SHIELD))
		    SET_BIT(pl->status, KILLED);
		if (Players[j]->fuel.sum <= 0
		    || !BIT(Players[j]->used, OBJ_SHIELD))
		    SET_BIT(Players[j]->status, KILLED);

		if (BIT(Players[j]->status, KILLED)) {
		    if (BIT(pl->status, KILLED)) {
			sprintf(msg, "%s and %s crashed.",
				pl->name, Players[j]->name);
			Set_message(msg);
			sc = Rate(Players[j]->score, pl->score) / 3;
			sc2 = Rate(pl->score, Players[j]->score) / 3;
			Score_players(i, -sc, Players[j]->name,
				      j, -sc2, pl->name);
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
			sc = Rate(pl->score, Players[j]->score) / 3;
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
			sc = Rate(Players[j]->score, pl->score) / 3;
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
	if (!BIT(pl->used, OBJ_CONNECTOR)) {
	    pl->ball = NULL;
	}
	else if (pl->ball != NULL) {
	    object *ball = pl->ball;
	    if (ball->life <= 0 || ball->id != -1)
		pl->ball = NULL;
	    else {
		float distance = Wrap_length(pl->pos.x - ball->pos.x,
					     pl->pos.y - ball->pos.y);
		if (distance >= BALL_STRING_LENGTH) {
		    ball->id = pl->id;
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
	    for (j = 0; j < NumObjs; j++) {
		if (BIT(Obj[j]->type, OBJ_BALL) && Obj[j]->id == -1) {
		    if (Wrap_length(pl->pos.x - Obj[j]->pos.x,
				    pl->pos.y - Obj[j]->pos.y)
			  < BALL_STRING_LENGTH) {
			object *ball = Obj[j];
			int bteam = -1;

			if (ball->treasure != -1)
			    bteam = World.treasures[ball->treasure].team;

			/*
			 * If the treasure's team cannot connect before
			 * other non-team members wait until somebody has
			 * else has owned the ball before allowing a
			 * connection.  This was done to stop team members
			 * taking and hiding with the ball... this was
			 * considered bad gamesmanship.
			 */
			if (ball->owner != -1
			    || (   pl->team != TEAM_NOT_SET
				&& pl->team != bteam))
			    pl->ball = Obj[j];
			break;
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
				(float) pl->last_lap_time / FPS,
				(float) pl->best_lap / FPS);
		    }
		    else if (pl->round > 1) {
			sprintf(msg,
				"%s completes lap %d in %.2fs. "
				"Personal race best lap time: %.2fs.",
				pl->name,
				pl->round-1,
				(float) pl->last_lap_time / FPS,
				(float) pl->best_lap / FPS);
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

static int IsOffensiveItem(enum Item i)
{
    if (i == ITEM_WIDEANGLE || i == ITEM_REARSHOT || i == ITEM_MINE ||
	i == ITEM_MISSILE || i == ITEM_LASER )
	return true;
    return false;
}

static int IsDefensiveItem(enum Item i)
{
    if (i == ITEM_CLOAK || i == ITEM_ECM || i == ITEM_TRANSPORTER || 
	i == ITEM_TRACTOR_BEAM || i == ITEM_EMERGENCY_SHIELD || i == ITEM_TANK)
	return true;
    return false;
}

static int CountOffensiveItems(player *pl)
{
    return (pl->item[ITEM_WIDEANGLE] + pl->item[ITEM_REARSHOT] + 
	    pl->item[ITEM_MINE] + pl->item[ITEM_MISSILE] + 
	    pl->item[ITEM_LASER]); 
}

static int CountDefensiveItems(player *pl)
{
    int count;

    count = pl->item[ITEM_CLOAK] + pl->item[ITEM_ECM] + 
	    pl->item[ITEM_TRANSPORTER] + pl->item[ITEM_TRACTOR_BEAM] + 
	    pl->item[ITEM_EMERGENCY_SHIELD] + pl->fuel.num_tanks;
    if (pl->emergency_shield_left) 
 	count++;
    return count;
}

static void PlayerObjectCollision(int ind)
{
    int		j, killer, old_have, range, radius, sc, hit, obj_count;
    long	drain;
    player	*pl = Players[ind];
    object	*obj, **obj_list;


    /*
     * Collision between a player and an object.
     */
    if (BIT(pl->status, PLAYING|PAUSE|GAME_OVER|KILLED) != PLAYING)
	return;

    Cell_objects_get(pl->pos.x / BLOCK_SZ, pl->pos.y / BLOCK_SZ, 4,
		     &obj_list, &obj_count);

    for (j=0; j<obj_count; j++) {
	obj = obj_list[j];
	if (obj->life <= 0)
	    continue;

	range = SHIP_SZ + obj->pl_range;
	if (!in_range((object *)pl, obj, range))
	    continue;

	if (obj->id != -1) {
	    if (obj->id == pl->id) {
		if (BIT(obj->type, OBJ_SPARK|OBJ_MINE)
		    && BIT(obj->status, OWNERIMMUNE)) {
		    continue;
		}
	    }
	    else if (TEAM_IMMUNE(ind, GetInd[obj->id])) {
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
	hit = (radius >= range || in_range((object *)pl, obj, radius));

	/*
	 * Object collision.
	 */
	switch (obj->type) {
	case OBJ_BALL:
	    if (! hit)
		continue;

	    /*
	     * The ball is special, usually players bounce of it with
	     * shields up, or die with shields down.  The treasure may
	     * be destroyed.
	     */
	    Obj_repel((object *)pl, obj, radius);
	    if (BIT(pl->used, (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD))
		!= (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD)) {
		Add_fuel(&(pl->fuel), ED_BALL_HIT);
		if (treasureCollisionDestroys) {
		    obj->life = 0;
		}
	    }
	    if (pl->fuel.sum > 0
		&& (!treasureCollisionMayKill
		    || BIT(pl->used,OBJ_SHIELD))) {
		continue;
	    }
	    if (obj->owner == -1) {
		sprintf(msg, "%s was killed by a ball.", pl->name);
		SCORE(ind, PTS_PR_PL_SHOT,
		      (int) pl->pos.x/BLOCK_SZ,
		      (int) pl->pos.y/BLOCK_SZ,
		      "Ball");
	    } else {
		killer = GetInd[obj->owner];

		sprintf(msg, "%s was killed by a ball owned by %s.",
			pl->name, Players[killer]->name);

		if (killer == ind) {
		    strcat(msg, "  How strange!");
		    SCORE(ind, PTS_PR_PL_SHOT,
			  (int) pl->pos.x/BLOCK_SZ,
			  (int) pl->pos.y/BLOCK_SZ,
			  Players[killer]->name);
		} else {
		    Players[killer]->kills++;
		    sc = Rate(Players[killer]->score, pl->score);
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

	    switch ((enum Item) obj->info) {
	    case ITEM_WIDEANGLE:
		pl->item[ITEM_WIDEANGLE] += obj->count;
		sound_play_sensors(pl->pos.x, pl->pos.y,
				   WIDEANGLE_SHOT_PICKUP_SOUND);
		break;
	    case ITEM_ECM:
		pl->item[ITEM_ECM] += obj->count;
		sound_play_sensors(pl->pos.x, pl->pos.y, ECM_PICKUP_SOUND);
		break;
	    case ITEM_TRANSPORTER:
		pl->item[ITEM_TRANSPORTER] += obj->count;
		sound_play_sensors(pl->pos.x, pl->pos.y, TRANSPORTER_PICKUP_SOUND);
		break;
	    case ITEM_SENSOR:
		pl->item[ITEM_SENSOR] += obj->count;
		pl->updateVisibility = 1;
		sound_play_sensors(pl->pos.x, pl->pos.y, SENSOR_PACK_PICKUP_SOUND);
		break;
	    case ITEM_AFTERBURNER:
		SET_BIT(pl->have, OBJ_AFTERBURNER);
		if ((pl->item[ITEM_AFTERBURNER] += obj->count) > MAX_AFTERBURNER)
		    pl->item[ITEM_AFTERBURNER] = MAX_AFTERBURNER;
		sound_play_sensors(pl->pos.x, pl->pos.y, AFTERBURNER_PICKUP_SOUND);
		break;
	    case ITEM_REARSHOT:
		pl->item[ITEM_REARSHOT] += obj->count;
		sound_play_sensors(pl->pos.x, pl->pos.y, BACK_SHOT_PICKUP_SOUND);
		break;
	    case ITEM_MISSILE:
		pl->item[ITEM_MISSILE] += obj->count;
		sound_play_sensors(pl->pos.x, pl->pos.y, ROCKET_PACK_PICKUP_SOUND);
		break;
	    case ITEM_CLOAK:
		SET_BIT(pl->have, OBJ_CLOAKING_DEVICE);
		pl->item[ITEM_CLOAK] += obj->count;
		pl->updateVisibility = 1;
		sound_play_sensors(pl->pos.x, pl->pos.y, CLOAKING_DEVICE_PICKUP_SOUND);
		break;
	    case ITEM_FUEL:
		Add_fuel(&(pl->fuel), ENERGY_PACK_FUEL);
		sound_play_sensors(pl->pos.x, pl->pos.y, ENERGY_PACK_PICKUP_SOUND);
		break;
	    case ITEM_MINE:
		pl->item[ITEM_MINE] += obj->count;
		sound_play_sensors(pl->pos.x, pl->pos.y, MINE_PACK_PICKUP_SOUND);
		break;
	    case ITEM_LASER:
		if ((pl->item[ITEM_LASER] += obj->count) > MAX_LASERS) {
		    pl->item[ITEM_LASER] = MAX_LASERS;
		}
		sound_play_sensors(pl->pos.x, pl->pos.y, LASER_PICKUP_SOUND);
		break;
	    case ITEM_EMERGENCY_THRUST:
		SET_BIT(pl->have, OBJ_EMERGENCY_THRUST);
		pl->item[ITEM_EMERGENCY_THRUST] += obj->count;
		sound_play_sensors(pl->pos.x, pl->pos.y,
				   EMERGENCY_THRUST_PICKUP_SOUND);
		break;
	    case ITEM_EMERGENCY_SHIELD:
		old_have = pl->have;
		SET_BIT(pl->have, OBJ_EMERGENCY_SHIELD);
		pl->item[ITEM_EMERGENCY_SHIELD]++;
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
		SET_BIT(pl->have, OBJ_TRACTOR_BEAM);
		if ((pl->item[ITEM_TRACTOR_BEAM] += obj->count) > MAX_TRACTORS) {
		    pl->item[ITEM_TRACTOR_BEAM] = MAX_TRACTORS;
		}
		sound_play_sensors(pl->pos.x, pl->pos.y,
				   TRACTOR_BEAM_PICKUP_SOUND);
		break;
	    case ITEM_AUTOPILOT:
		SET_BIT(pl->have, OBJ_AUTOPILOT);
		pl->item[ITEM_AUTOPILOT] += obj->count;
		sound_play_sensors(pl->pos.x, pl->pos.y,
				   AUTOPILOT_PICKUP_SOUND);
		break;

	    case ITEM_TANK:
		if (pl->fuel.num_tanks < MAX_TANKS) {
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
		char *reprogrammer_name = "some jerk";
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
		char *reprogrammer_name = "some jerk";
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
		sc = Rate(Players[killer]->score, pl->score) / 6;
		Score_players(killer, sc, pl->name,
			      ind, -sc, Players[killer]->name);
	    }
	    Set_message(msg);
	    break;

	case OBJ_DEBRIS: {
		float		v = VECTOR_LENGTH(obj->vel);
		long		tmp = (long) (2 * obj->mass * v);
		long		cost = ABS(tmp);

		if (BIT(pl->used, (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD))
		    != (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD))
		    Add_fuel(&pl->fuel, - cost);
		if (pl->fuel.sum == 0) {
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
			      (int) pl->pos.x/BLOCK_SZ,
			      (int) pl->pos.y/BLOCK_SZ,
			      (killer == -1) ? "[Explosion]" : pl->name);
		    } else {
			Players[killer]->kills++;
			sc = Rate(Players[killer]->score, pl->score) / 3;
			Score_players(killer, sc, pl->name,
				      ind, -sc, Players[killer]->name);
		    }
		    return;
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
	    || (obj->type == OBJ_TORPEDO
		&& BIT(obj->mods.nuclear, NUCLEAR)
		&& (rand()&3))) {
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
		drain = ED_SMART_SHOT_HIT /
		    ((obj->mods.mini + 1) * (obj->mods.power + 1));
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
		    != (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD))
		    Add_fuel(&(pl->fuel), (ED_SHOT_HIT * SHOT_MULT(obj)));
		pl->forceVisible += SHOT_MULT(obj);
		break;

	    default:
		printf("You were hit by what?\n");
		break;
	    }
	    if (pl->fuel.sum <= 0) {
		CLR_BIT(pl->used, OBJ_SHIELD);
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
			  (int) pl->pos.x/BLOCK_SZ,
			  (int) pl->pos.y/BLOCK_SZ, "Cannon");
		    SET_BIT(pl->status, KILLED);
		    return;
		}

		if (obj->id == -1) {
		    sprintf(msg, "%s was killed by %s.", pl->name,
			    Describe_shot(obj->type, obj->status,
					  obj->mods, 1));
		    SCORE(ind, PTS_PR_PL_SHOT,
			  (int) pl->pos.x/BLOCK_SZ,
			  (int) pl->pos.y/BLOCK_SZ, "N/A");
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
			      (int) pl->pos.x/BLOCK_SZ,
			      (int) pl->pos.y/BLOCK_SZ,
			      Players[killer]->name);
		    } else {
			Players[killer]->kills++;
			sc = Rate(Players[killer]->score, pl->score);
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
    static int cache;
    int i;

    if (World.wormHoles[cache].pos.x == x &&
	World.wormHoles[cache].pos.y == y)
	return cache;

    for (i = 0; i < World.NumWormholes; i++)
	if (World.wormHoles[i].pos.x == x &&
	    World.wormHoles[i].pos.y == y)
	    break;

    cache = i;

    return i;
}


static void LaserCollision(void)
{
    typedef struct victim {
	int			ind;	/* player index */
	position		pos;
	float			prev_dist;
    } victim_t;

    int				ind, i, j, k, max, hits, sc,
				max_victims = 0,
				num_victims = 0,
				objnum = -1;
    unsigned			size;
    victim_t			*victims = NULL;
    float			x, y, x1, x2, y1, y2, dx, dy, dist;
    player			*pl, *vic;
    pulse_t			*pulse;
    object			*obj = NULL;
    char			msg[MSG_LEN];

    for (ind = 0; ind < NumPlayers; ind++) {
	pl = Players[ind];
	if (BIT(pl->used, OBJ_LASER) != 0) {
	    if (BIT(pl->status, PLAYING|GAME_OVER|KILLED|PAUSE) != PLAYING
		|| pl->item[ITEM_LASER] <= 0) {
		CLR_BIT(pl->used, OBJ_LASER);
	    }
	    else if (pl->item[ITEM_LASER] > pl->num_pulses
		&& pl->velocity < PULSE_SPEED - PULSE_SAMPLE_DISTANCE) {
		if (pl->fuel.sum <= -ED_LASER) {
		    CLR_BIT(pl->used, OBJ_LASER);
		} else {
		    Add_fuel(&(pl->fuel), ED_LASER);
		    if (pl->num_pulses >= pl->max_pulses) {
			size = pl->item[ITEM_LASER] * sizeof(pulse_t);
			if (pl->max_pulses <= 0) {
			    pl->pulses = (pulse_t *)malloc(size);
			    pl->num_pulses = 0;
			} else {
			    pl->pulses = (pulse_t *)realloc(pl->pulses, size);
			}
			if (pl->pulses == NULL) {
			    pl->max_pulses = 0;
			    pl->num_pulses = 0;
			    continue;
			}
			pl->max_pulses = pl->item[ITEM_LASER];
		    }
		    pulse = &pl->pulses[pl->num_pulses++];
		    pulse->dir = pl->dir;
		    pulse->len = PULSE_LENGTH;
		    pulse->life = PULSE_LIFE(pl->item[ITEM_LASER]);
		    pulse->mods = pl->mods;
		    pulse->pos.x = pl->pos.x + pl->ship->m_gun[pl->dir].x
				    - PULSE_SPEED * tcos(pulse->dir);
		    pulse->pos.y = pl->pos.y + pl->ship->m_gun[pl->dir].y
				    - PULSE_SPEED * tsin(pulse->dir);
		    sound_play_sensors(pulse->pos.x, pulse->pos.y,
				       FIRE_LASER_SOUND);
		}
	    }
	}
	for (k = 0; k < pl->num_pulses; k++) {
	    pulse = &pl->pulses[k];
	    if (--pulse->life < 0) {
		if (--pl->num_pulses > k) {
		    pl->pulses[k] = pl->pulses[pl->num_pulses];
		    k--;
		}
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
		    pulse->len = pulse->len * (0 - x1) / (x2 - x1);
		    x2 = x1 + tcos(pulse->dir) * pulse->len;
		}
		if (x2 >= World.width) {
		    pulse->len = pulse->len * (World.width - 1 - x1)
			/ (x2 - x1);
		    x2 = x1 + tcos(pl->dir) * pulse->len;
		}
		y2 = y1 + tsin(pl->dir) * pulse->len;
		if (y2 < 0) {
		    pulse->len = pulse->len * (0 - y1) / (y2 - y1);
		    x2 = x1 + tcos(pl->dir) * pulse->len;
		    y2 = y1 + tsin(pl->dir) * pulse->len;
		}
		if (y2 > World.height) {
		    pulse->len = pulse->len * (World.height - 1 - y1)
			/ (y2 - y1);
		    x2 = x1 + tcos(pl->dir) * pulse->len;
		    y2 = y1 + tsin(pl->dir) * pulse->len;
		}
		if (pulse->len <= 0) {
		    pulse->len = 0;
		    continue;
		}
	    }

	    num_victims = 0;
	    for (i = 0; i < NumPlayers; i++) {
		vic = Players[i];
		if (BIT(vic->status, PLAYING|GAME_OVER|KILLED|PAUSE)
		    != PLAYING) {
		    continue;
		}
		if (TEAM_IMMUNE(i, ind) || PSEUDO_TEAM(i, ind)) {
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
		victims[num_victims].pos = vic->pos;
		victims[num_victims].prev_dist = 1e10;
		num_victims++;
	    }

	    dx = x2 - x1;
	    dy = y2 - y1;
	    max = MAX(ABS(dx), ABS(dy));
	    if (max == 0) {
		continue;
	    }

	    obj->type = OBJ_PULSE;
	    obj->life = 1;
	    obj->owner = pl->id;
	    obj->id = pl->id;
	    obj->count = 0;
	    obj->pos.x = obj->prevpos.x = x1;
	    obj->pos.y = obj->prevpos.y = y1;

	    for (i = hits = 0; i <= max; i += PULSE_SAMPLE_DISTANCE) {

		x = x1 + (i * dx) / max;
		y = y1 + (i * dy) / max;
		obj->vel.x = x - obj->pos.x;
		obj->vel.y = y - obj->pos.y;
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
		for (j = 0; j < num_victims; j++) {
		    dist = Wrap_length(x - victims[j].pos.x,
				       y - victims[j].pos.y);
		    if (dist <= SHIP_SZ) {
			hits++;
			vic = Players[victims[j].ind];
			vic->forceVisible++;
			sound_play_sensors(vic->pos.x, vic->pos.y, PLAYER_EAT_LASER_SOUND);
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
				sprintf(msg,
					"%s got paralysed by %s's stun laser.",
					vic->name, pl->name);
				Set_message(msg);
				CLR_BIT(vic->used,
					OBJ_SHIELD|OBJ_LASER|OBJ_SHOT);
				CLR_BIT(vic->status, THRUSTING);
			    }
			} else if (BIT(pulse->mods.laser, BLIND)) {
			    vic->damaged += (FPS+6);
			    vic->forceVisible += (FPS+6);
			} else {
			    Add_fuel(&(vic->fuel), ED_LASER_HIT);
			    if (BIT(vic->used, OBJ_SHIELD) == 0) {
				SET_BIT(vic->status, KILLED);
				sc = Rate(pl->score, vic->score);
				Score_players(ind, sc, vic->name,
					      victims[j].ind, -sc, pl->name);
				sound_play_sensors(vic->pos.x, vic->pos.y,
						   PLAYER_ROASTED_SOUND);
				sprintf(msg,
					"%s got roasted alive by %s's laser.",
					vic->name, pl->name);
				Set_message(msg);
				pl->kills++;
				Robot_war(victims[j].ind, ind);
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
    }
    if (max_victims > 0 && victims != NULL) {
	free(victims);
    }
    if (objnum >= 0 && obj != NULL) {
	obj->type = OBJ_DEBRIS;
	obj->life = 0;
    }
}



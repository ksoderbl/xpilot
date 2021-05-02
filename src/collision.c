/* $Id: collision.c,v 3.90 1994/04/23 16:56:59 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-94 by
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
#include <assert.h>
#include <time.h>
#include "global.h"
#include "map.h"
#include "object.h"
#include "score.h"
#include "robot.h"
#include "saudio.h"
#include "bit.h"
#include "item.h"
#include "netserver.h"
#include "pack.h"

#if 0
#define NUM_POINTS	3
#else
#define NUM_POINTS	(pl->ship->num_points)
#endif

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: collision.c,v 3.90 1994/04/23 16:56:59 bert Exp $";
#endif

#define FLOAT_TO_INT(F)		((F) < 0 ? -(int)(0.5f-(F)) : (int)((F)+0.5f))
#define DOUBLE_TO_INT(D)	((D) < 0 ? -(int)(0.5-(D)) : (int)((D)+0.5))

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


static int Rate(int winner, int looser);
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


void Cell_objects_init(void)
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


void Cell_objects_get(int x, int y, int r, object ***list, int *count)
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

static int Rate(int winner, int loser)
{
    int t;

    t = ((RATE_SIZE/2) * RATE_RANGE) / (ABS(loser-winner) + RATE_RANGE);
    if (loser > winner)
	t = RATE_SIZE - t;
    return (t);
}


void Check_collision(void)
{
    Cell_objects_init();
    PlayerCollision();
    LaserCollision();
}

static void PlayerCollision(void)
{
    int		i, j, sc, sc2;
    player	*pl;

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
		    Add_fuel(&(pl->fuel), ED_PL_CRASH);
		    Add_fuel(&(Players[j]->fuel), ED_PL_CRASH);
		    Item_damage(i, destroyItemInCollisionProb);
		    Item_damage(j, destroyItemInCollisionProb);
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
			sprintf(msg, "[%s]", Players[j]->name);
			SCORE(i, -sc, 
			      (int) pl->pos.x/BLOCK_SZ, 
			      (int) pl->pos.y/BLOCK_SZ,
			      msg);
			sprintf(msg, "[%s]", pl->name);
			SCORE(j, -sc2, 
			      (int) Players[j]->pos.x/BLOCK_SZ, 
			      (int) Players[j]->pos.y/BLOCK_SZ,
			      msg);

		    } else {
			sprintf(msg, "%s ran over %s.",
				pl->name, Players[j]->name);
			Set_message(msg);
			sound_play_sensors(Players[j]->pos.x,
					   Players[j]->pos.y,
					   PLAYER_RAN_OVER_PLAYER_SOUND);
			sc = Rate(pl->score, Players[j]->score) / 3;
			SCORE(i, sc,
			      (int) pl->pos.x/BLOCK_SZ,
			      (int) pl->pos.y/BLOCK_SZ,
			      Players[j]->name);
			SCORE(j, -sc,
			      (int) Players[j]->pos.x/BLOCK_SZ, 
			      (int) Players[j]->pos.y/BLOCK_SZ,
			      pl->name);
		    }

		} else {
		    if (BIT(pl->status, KILLED)) {
			sprintf(msg, "%s ran over %s.",
				Players[j]->name, pl->name);
			sound_play_sensors(pl->pos.x, pl->pos.y,
					   PLAYER_RAN_OVER_PLAYER_SOUND);
			sc = Rate(Players[j]->score, pl->score) / 3;
			SCORE(i, -sc, 
			      (int) pl->pos.x/BLOCK_SZ,
			      (int) pl->pos.y/BLOCK_SZ,
			      Players[j]->name);
			SCORE(j, sc, 
			      (int) Players[j]->pos.x/BLOCK_SZ, 
			      (int) Players[j]->pos.y/BLOCK_SZ,
			      pl->name);
			Set_message(msg);

		    }
		}

		if (Players[j]->robot_mode != RM_NOT_ROBOT
		    && Players[j]->robot_mode != RM_OBJECT
		    && BIT(Players[j]->status, KILLED)
		    && BIT(Players[j]->robot_lock, LOCK_PLAYER)
		    && Players[j]->robot_lock_id == pl->id)
		    CLR_BIT(Players[j]->robot_lock, LOCK_PLAYER);

		if (pl->robot_mode != RM_NOT_ROBOT
		    && pl->robot_mode != RM_OBJECT
		    && BIT(pl->status, KILLED)
		    && BIT(pl->robot_lock, LOCK_PLAYER)
		    && pl->robot_lock_id == Players[j]->id)
		    CLR_BIT(pl->robot_lock, LOCK_PLAYER);
	    }
	}

	/* Player checkpoint */
	if (BIT(World.rules->mode, TIMING))
	    if (Wrap_length(pl->pos.x - World.check[pl->check].x*BLOCK_SZ,
		       pl->pos.y - World.check[pl->check].y*BLOCK_SZ) < 200) {

		if (pl->check == 0) {
		    pl->round++;
		    if (((pl->best_lap > pl->time - pl->last_lap)
			 || (pl->best_lap == 0))
			&& (pl->time != 0)) {
			pl->best_lap = pl->time - pl->last_lap;
		    }
		    pl->last_lap_time = pl->time - pl->last_lap;
		    pl->last_lap = pl->time;
		}

		pl->check++;

		if (pl->check == World.NumChecks)
		    pl->check = 0;
	    }

	/* Player picking up ball/treasure */
	if (!BIT(pl->used, OBJ_CONNECTOR)) {
	    pl->ball = NULL;
	} else {
	    if (pl->ball != NULL) {
		if (pl->ball->life <= 0 || pl->ball->id != -1)
		    pl->ball = NULL;
		else if (Wrap_length(pl->pos.x - pl->ball->pos.x,
				     pl->pos.y - pl->ball->pos.y) 
			 > BALL_STRING_LENGTH) {
		    pl->ball->id = pl->id;
		    pl->ball->owner = pl->id;
		    SET_BIT(pl->ball->status, GRAVITY);
		    if (pl->ball->treasure != -1)
			World.treasures[pl->ball->treasure].have = false;
		    SET_BIT(pl->have, OBJ_BALL);
		    pl->ball = NULL;
		    sound_play_sensors(pl->pos.x, pl->pos.y,
				       CONNECT_BALL_SOUND);
		}
	    } else {
		for (j=0 ; j < NumObjs; j++) {
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
	}
  
	PlayerObjectCollision(i);
    }
}


static void PlayerObjectCollision(int ind)
{
    int		j, killer, range, radius, sc, hit, obj_count;
    long	drain;
    player	*pl;
    object	*obj, **obj_list;


    /*
     * Collision between a player and an object.
     */
    pl = Players[ind]; 
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

	if (BIT(obj->type, OBJ_ALL_ITEMS)) {
	    if (BIT(pl->used, OBJ_SHIELD) && !shieldedItemPickup) {
		SET_BIT(obj->status, GRAVITY);
		Delta_mv((object *)pl, obj);
		continue;
	    }
	}
	else if (BIT(obj->type, OBJ_TORPEDO)) {
	    if (pl->id == obj->id && obj->info < 8) {
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
	    Add_fuel(&(pl->fuel), ED_BALL_HIT);
	    if (treasureCollisionDestroys)
		obj->life = 0;
	    if (pl->fuel.sum > 0
		&& (!treasureCollisionMayKill || BIT(pl->used,OBJ_SHIELD))) {
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
		    sc = Rate(Players[killer]->score, pl->score);
		    SCORE(killer, sc,
			  (int) pl->pos.x/BLOCK_SZ,
			  (int) pl->pos.y/BLOCK_SZ,
			  pl->name);
		    SCORE(ind, -sc,
			  (int) pl->pos.x/BLOCK_SZ, 
			  (int) pl->pos.y/BLOCK_SZ,
			  Players[killer]->name);
		}
	    }
	    Set_message(msg);
	    SET_BIT(pl->status, KILLED);
	    return;

	case OBJ_WIDEANGLE_SHOT:
	    pl->extra_shots++;
	    sound_play_sensors(pl->pos.x, pl->pos.y, 
			       WIDEANGLE_SHOT_PICKUP_SOUND);
	    break;
	case OBJ_ECM:
	    pl->ecms++;
	    sound_play_sensors(pl->pos.x, pl->pos.y, ECM_PICKUP_SOUND);
	    break;
	case OBJ_TRANSPORTER:
	    pl->transporters++;
	    sound_play_sensors(pl->pos.x, pl->pos.y, TRANSPORTER_PICKUP_SOUND);
	    break;
	case OBJ_SENSOR_PACK:
	    pl->sensors++;
	    pl->updateVisibility = 1;
	    sound_play_sensors(pl->pos.x, pl->pos.y, SENSOR_PACK_PICKUP_SOUND);
	    break;
	case OBJ_AFTERBURNER:
	    SET_BIT(pl->have, OBJ_AFTERBURNER);
	    if (++pl->afterburners > MAX_AFTERBURNER)
		pl->afterburners = MAX_AFTERBURNER;
	    sound_play_sensors(pl->pos.x, pl->pos.y, AFTERBURNER_PICKUP_SOUND);
	    break;
	case OBJ_BACK_SHOT:
	    SET_BIT(pl->have, OBJ_BACK_SHOT);
	    pl->back_shots++;
	    sound_play_sensors(pl->pos.x, pl->pos.y, BACK_SHOT_PICKUP_SOUND);
	    break;
	case OBJ_ROCKET_PACK:
	    pl->missiles += MAX_MISSILES_PER_PACK;
	    sound_play_sensors(pl->pos.x, pl->pos.y, ROCKET_PACK_PICKUP_SOUND);
	    break;
	case OBJ_CLOAKING_DEVICE:
	    SET_BIT(pl->have, OBJ_CLOAKING_DEVICE);
	    pl->cloaks++;
	    pl->updateVisibility = 1;
	    sound_play_sensors(pl->pos.x, pl->pos.y, CLOAKING_DEVICE_PICKUP_SOUND);
	    break;
	case OBJ_ENERGY_PACK:
	    Add_fuel(&(pl->fuel), ENERGY_PACK_FUEL);
	    sound_play_sensors(pl->pos.x, pl->pos.y, ENERGY_PACK_PICKUP_SOUND);
	    break;
	case OBJ_MINE_PACK:
	    pl->mines += 1 + (rand()&1);
	    sound_play_sensors(pl->pos.x, pl->pos.y, MINE_PACK_PICKUP_SOUND);
	    break;
	case OBJ_LASER:
	    if (++pl->lasers > MAX_LASERS) {
		pl->lasers = MAX_LASERS;
	    }
	    sound_play_sensors(pl->pos.x, pl->pos.y, LASER_PICKUP_SOUND);
	    break;
	case OBJ_EMERGENCY_THRUST:
	    SET_BIT(pl->have, OBJ_EMERGENCY_THRUST);
	    pl->emergency_thrusts++;
	    sound_play_sensors(pl->pos.x, pl->pos.y,
			       EMERGENCY_THRUST_PICKUP_SOUND);
	    break;
	case OBJ_TRACTOR_BEAM:
	    SET_BIT(pl->have, OBJ_TRACTOR_BEAM);
	    if (++pl->tractor_beams > MAX_TRACTORS) {
		pl->tractor_beams = MAX_TRACTORS;
	    }
	    sound_play_sensors(pl->pos.x, pl->pos.y,
			       TRACTOR_BEAM_PICKUP_SOUND);
	    break;
	case OBJ_AUTOPILOT:
	    SET_BIT(pl->have, OBJ_AUTOPILOT);
	    pl->autopilots++;
	    sound_play_sensors(pl->pos.x, pl->pos.y,
			       AUTOPILOT_PICKUP_SOUND);
	    break;
	case OBJ_TANK: {
	    int c = pl->fuel.current;

	    if (pl->fuel.num_tanks < MAX_TANKS) {
		/*
		 * Set a new, empty tank in the list.
		 * update max-fuel
		 */
		int no = ++(pl->fuel.num_tanks);

		SET_BIT(pl->have, OBJ_TANK);
		pl->fuel.current = no;
		pl->fuel.max += TANK_CAP(no);
		pl->fuel.tank[no] = 0;
		pl->emptymass += TANK_MASS;
	    }
	    Add_fuel(&(pl->fuel), TANK_FUEL(pl->fuel.current));
	    pl->fuel.current = c;
	    sound_play_sensors(pl->pos.x, pl->pos.y, TANK_PICKUP_SOUND);
	    break;
	}

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
	        sc = Rate(Players[killer]->score, pl->score) / 6;
		SCORE(killer, sc, 
		      (int) obj->pos.x/BLOCK_SZ,
		      (int) obj->pos.y/BLOCK_SZ,
		      pl->name);
		SCORE(ind, -sc,
		      (int) Players[ind]->pos.x/BLOCK_SZ, 
		      (int) Players[ind]->pos.y/BLOCK_SZ,
		      Players[killer]->name);
	    }
	    Set_message(msg);
	    break;

	case OBJ_DEBRIS: {
		float		v = VECTOR_LENGTH(obj->vel);
		long		tmp = (long) (2 * obj->mass * v);
		long		cost = ABS(tmp);

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
			sc = Rate(Players[killer]->score, pl->score) / 3;
			SCORE(killer, sc,
			      (int) pl->pos.x/BLOCK_SZ,
			      (int) pl->pos.y/BLOCK_SZ,
			      pl->name);
			SCORE(ind, -sc,
			      (int) pl->pos.x/BLOCK_SZ, 
			      (int) pl->pos.y/BLOCK_SZ,
			      Players[killer]->name);
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

	if (BIT(pl->used, OBJ_SHIELD)
	    || obj->type == OBJ_TORPEDO
		&& BIT(obj->mods.nuclear, NUCLEAR)
		&& (rand()&3)) {
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
		Add_fuel(&(pl->fuel), drain);
		pl->forceVisible += 2;
		Set_message(msg);
		break;

	    case OBJ_SHOT:
		sound_play_sensors(pl->pos.x, pl->pos.y,
				   PLAYER_EAT_SHOT_SOUND);
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
			sc = Rate(Players[killer]->score, pl->score);
			SCORE(killer, sc, (int) pl->pos.x/BLOCK_SZ,
			      (int) pl->pos.y/BLOCK_SZ, pl->name);
			SCORE(ind, -sc, (int) pl->pos.x/BLOCK_SZ, 
			      (int) pl->pos.y/BLOCK_SZ, Players[killer]->name);
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

    if (itemLaserProb <= 0) {
	return;
    }
    for (ind = 0; ind < NumPlayers; ind++) {
	pl = Players[ind];
	if (BIT(pl->used, OBJ_LASER) != 0) {
	    if (BIT(pl->status, PLAYING|GAME_OVER|KILLED|PAUSE) != PLAYING
		|| pl->lasers <= 0) {
		CLR_BIT(pl->used, OBJ_LASER);
	    }
	    else if (pl->lasers > pl->num_pulses
		&& pl->velocity < PULSE_SPEED - PULSE_SAMPLE_DISTANCE) {
		if (pl->fuel.sum <= -ED_LASER) {
		    CLR_BIT(pl->used, OBJ_LASER);
		} else {
		    Add_fuel(&(pl->fuel), ED_LASER);
		    if (pl->num_pulses >= pl->max_pulses) {
			size = pl->lasers * sizeof(pulse_t);
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
			pl->max_pulses = pl->lasers;
		    }
		    pulse = &pl->pulses[pl->num_pulses++];
		    pulse->dir = pl->dir;
		    pulse->len = PULSE_LENGTH;
		    pulse->life = PULSE_LIFE(pl->lasers);
		    pulse->mods = pl->mods;
		    pulse->pos.x = pl->pos.x + pl->ship->pts[pl->dir][0].x
			- PULSE_SPEED * tcos(pulse->dir);
		    pulse->pos.y = pl->pos.y + pl->ship->pts[pl->dir][0].y
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
			if (BIT(pulse->mods.laser, STUN) || laserIsStunGun == true && allowLaserModifiers == false) {
			    if (BIT(vic->used, OBJ_SHIELD|OBJ_LASER|OBJ_FIRE)
				|| BIT(vic->status, THRUSTING)) {
				sprintf(msg,
					"%s got paralysed by %s's stun laser.",
					vic->name, pl->name);
				Set_message(msg);
				CLR_BIT(vic->used,
					OBJ_SHIELD|OBJ_LASER|OBJ_FIRE);
				CLR_BIT(vic->status, THRUSTING);
			    }
			} else if (BIT(pulse->mods.laser, BLIND)) {
			    vic->damaged += 2*FPS;
			    vic->forceVisible += 2*FPS;
			} else {
			    Add_fuel(&(vic->fuel), ED_LASER_HIT);
			    if (BIT(vic->used, OBJ_SHIELD) == 0) {
				SET_BIT(vic->status, KILLED);
				sc = Rate(pl->score, vic->score);
				SCORE(ind, sc, x / BLOCK_SZ, y / BLOCK_SZ,
				      vic->name);
				SCORE(victims[j].ind, -sc, x / BLOCK_SZ,
				      y / BLOCK_SZ, pl->name);
				sound_play_sensors(vic->pos.x, vic->pos.y, 
						   PLAYER_ROASTED_SOUND);
				sprintf(msg,
					"%s got roasted alive by %s's laser.",
					vic->name, pl->name);
				Set_message(msg);
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


/*
 * Wall collision detection and bouncing.
 *
 * The wall collision detection routines depend on repeatability
 * (getting the same result even after some "neutral" calculations)
 * and an exact determination whether a point is in space,
 * inside the wall (crash!) or on the edge.
 * This will be hard to achieve if only floating point would be used.
 * However, a resolution of a pixel is a bit rough and ugly.
 * Therefore a fixed point sub-pixel resolution is used called clicks.
 */

#define CLICK_SHIFT		6
#define CLICK			(1 << CLICK_SHIFT)
#define BLOCK_CLICKS		(BLOCK_SZ << CLICK_SHIFT)
#define PIXEL_TO_CLICKS(I)	((click_t)(I) << CLICK_SHIFT)
#define CLICKS_TO_PIXEL(C)	((int)((C) >> CLICK_SHIFT))
#define FLOAT_TO_CLICK(F)	((F)<0 ? -(int)(0.5f-(F)*CLICK) : (int)((F)*CLICK+0.5f))
#define CLICK_TO_FLOAT(C)	((float)(C) * (1.0f / CLICK))

typedef enum {
    NotACrash = 0,
    CrashUniverse = 0x01,
    CrashWall = 0x02,
    CrashTarget = 0x04,
    CrashTreasure = 0x08,
    CrashCannon = 0x10,
    CrashUnknown = 0x20,
    CrashWormHole = 0x40,
    CrashWallSpeed = 0x80,
    CrashWallNoFuel = 0x100,
    CrashWallAngle = 0x200
} move_crash_t;

typedef enum {
    NotABounce = 0,
    BounceHorLo = 0x01,
    BounceHorHi = 0x02,
    BounceVerLo = 0x04,
    BounceVerHi = 0x08,
    BounceLeftDown = 0x10,
    BounceLeftUp = 0x20,
    BounceRightDown = 0x40,
    BounceRightUp = 0x80,
    BounceEdge = 0x0100
} move_bounce_t;

typedef int click_t;

typedef struct {
    click_t		x, y;
} clpos;

typedef struct {
    click_t		x, y;
} clvec;

typedef struct {
    int			edge_wrap;
    int			edge_bounce;
    int			wall_bounce;
    int			cannon_crashes;
    int			target_crashes;
    int			treasure_crashes;
    int			wormhole_warps;
    object		*object;
    player		*player;
} move_info_t;

typedef struct {
    const move_info_t	*mip;
    move_crash_t	crash;
    move_bounce_t	bounce;
    clpos		pos;
    vector		vel;
    clvec		todo;
    clvec		done;
    int			dir;
    int			cannon;
    int			wormhole;
    int			target;
    int			treasure;
} move_state_t;

static struct move_parameters {
    click_t		click_width;		/* Map width in clicks */
    click_t		click_height;		/* Map width in clicks */

    int			max_shielded_angle;	/* max player bounce angle */
    int			max_unshielded_angle;	/* max player bounce angle */

    unsigned long	obj_bounce_mask;	/* which objects bounce? */
    unsigned long	obj_cannon_mask;	/* objects crash cannons? */
    unsigned long	obj_target_mask;	/* object target hit? */
    unsigned long	obj_treasure_mask;	/* objects treasure crash? */
} mp;

static float wallBounceExplosionMult;

void Move_init(void)
{
    mp.click_width = PIXEL_TO_CLICKS(World.width);
    mp.click_height = PIXEL_TO_CLICKS(World.height);

    LIMIT(maxObjectWallBounceSpeed, 0, World.hypotenuse);
    LIMIT(maxShieldedWallBounceSpeed, 0, World.hypotenuse);
    LIMIT(maxUnshieldedWallBounceSpeed, 0, World.hypotenuse);
    LIMIT(maxShieldedWallBounceAngle, 0, 180);
    LIMIT(maxUnshieldedWallBounceAngle, 0, 180);
    LIMIT(playerWallBrakeFactor, 0, 1);
    LIMIT(objectWallBrakeFactor, 0, 1);
    LIMIT(objectWallBounceLifeFactor, 0, 1);
    LIMIT(wallBounceFuelDrainMult, 0, 1000);
    wallBounceExplosionMult = sqrt(wallBounceFuelDrainMult);

    mp.max_shielded_angle = maxShieldedWallBounceAngle * RES / 360;
    mp.max_unshielded_angle = maxUnshieldedWallBounceAngle * RES / 360;

    mp.obj_bounce_mask = 0;
    if (sparksWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_SPARK);
    }
    if (debrisWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_DEBRIS);
    }
    if (shotsWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_SHOT);
    }
    if (itemsWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_ALL_ITEMS);
    }
    if (missilesWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_HEAT_SHOT);
    }
    if (minesWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_MINE);
    }
    if (ballsWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_BALL);
    }

    mp.obj_cannon_mask = (KILLING_SHOTS) | OBJ_MINE | OBJ_SHOT | OBJ_PULSE |
			OBJ_SMART_SHOT | OBJ_TORPEDO | OBJ_HEAT_SHOT;
    mp.obj_target_mask = mp.obj_cannon_mask | OBJ_BALL | OBJ_SPARK;
    mp.obj_treasure_mask = mp.obj_bounce_mask | OBJ_BALL | OBJ_PULSE;
}

static void Bounce_edge(move_state_t *ms, move_bounce_t bounce)
{
    if (bounce == BounceHorLo) {
	if (ms->mip->edge_bounce) {
	    ms->todo.x = -ms->todo.x;
	    ms->vel.x = -ms->vel.x;
	    if (!ms->mip->player) {
		ms->dir = MOD2(RES / 2 - ms->dir, RES);
	    }
	}
	else {
	    ms->todo.x = 0;
	    ms->vel.x = 0;
	    if (!ms->mip->player) {
		ms->dir = (ms->vel.y < 0) ? (3*RES/4) : RES/4;
	    }
	}
    }
    else if (bounce == BounceHorHi) {
	if (ms->mip->edge_bounce) {
	    ms->todo.x = -ms->todo.x;
	    ms->vel.x = -ms->vel.x;
	    if (!ms->mip->player) {
		ms->dir = MOD2(RES / 2 - ms->dir, RES);
	    }
	}
	else {
	    ms->todo.x = 0;
	    ms->vel.x = 0;
	    if (!ms->mip->player) {
		ms->dir = (ms->vel.y < 0) ? (3*RES/4) : RES/4;
	    }
	}
    }
    else if (bounce == BounceVerLo) {
	if (ms->mip->edge_bounce) {
	    ms->todo.y = -ms->todo.y;
	    ms->vel.y = -ms->vel.y;
	    if (!ms->mip->player) {
		ms->dir = MOD2(RES - ms->dir, RES);
	    }
	}
	else {
	    ms->todo.y = 0;
	    ms->vel.y = 0;
	    if (!ms->mip->player) {
		ms->dir = (ms->vel.x < 0) ? (RES/2) : 0;
	    }
	}
    }
    else if (bounce == BounceVerHi) {
	if (ms->mip->edge_bounce) {
	    ms->todo.y = -ms->todo.y;
	    ms->vel.y = -ms->vel.y;
	    if (!ms->mip->player) {
		ms->dir = MOD2(RES - ms->dir, RES);
	    }
	}
	else {
	    ms->todo.y = 0;
	    ms->vel.y = 0;
	    if (!ms->mip->player) {
		ms->dir = (ms->vel.x < 0) ? (RES/2) : 0;
	    }
	}
    }
    ms->bounce = BounceEdge;
}

static void Bounce_wall(move_state_t *ms, move_bounce_t bounce)
{
    if (!ms->mip->wall_bounce) {
	ms->crash = CrashWall;
	return;
    }
    if (bounce == BounceHorLo) {
	ms->todo.x = -ms->todo.x;
	ms->vel.x = -ms->vel.x;
	if (!ms->mip->player) {
	    ms->dir = MOD2(RES/2 - ms->dir, RES);
	}
    }
    else if (bounce == BounceHorHi) {
	ms->todo.x = -ms->todo.x;
	ms->vel.x = -ms->vel.x;
	if (!ms->mip->player) {
	    ms->dir = MOD2(RES/2 - ms->dir, RES);
	}
    }
    else if (bounce == BounceVerLo) {
	ms->todo.y = -ms->todo.y;
	ms->vel.y = -ms->vel.y;
	if (!ms->mip->player) {
	    ms->dir = MOD2(RES - ms->dir, RES);
	}
    }
    else if (bounce == BounceVerHi) {
	ms->todo.y = -ms->todo.y;
	ms->vel.y = -ms->vel.y;
	if (!ms->mip->player) {
	    ms->dir = MOD2(RES - ms->dir, RES);
	}
    }
    else {
	clvec t = ms->todo;
	vector v = ms->vel;
	if (bounce == BounceLeftDown) {
	    ms->todo.x = -t.y;
	    ms->todo.y = -t.x;
	    ms->vel.x = -v.y;
	    ms->vel.y = -v.x;
	    if (!ms->mip->player) {
		ms->dir = MOD2(3*RES/4 - ms->dir, RES);
	    }
	}
	else if (bounce == BounceLeftUp) {
	    ms->todo.x = t.y;
	    ms->todo.y = t.x;
	    ms->vel.x = v.y;
	    ms->vel.y = v.x;
	    if (!ms->mip->player) {
		ms->dir = MOD2(RES/4 - ms->dir, RES);
	    }
	}
	else if (bounce == BounceRightDown) {
	    ms->todo.x = t.y;
	    ms->todo.y = t.x;
	    ms->vel.x = v.y;
	    ms->vel.y = v.x;
	    if (!ms->mip->player) {
		ms->dir = MOD2(RES/4 - ms->dir, RES);
	    }
	}
	else if (bounce == BounceRightUp) {
	    ms->todo.x = -t.y;
	    ms->todo.y = -t.x;
	    ms->vel.x = -v.y;
	    ms->vel.y = -v.x;
	    if (!ms->mip->player) {
		ms->dir = MOD2(3*RES/4 - ms->dir, RES);
	    }
	}
    }
    ms->bounce = bounce;
}

/*
 * Move a point through one block and detect
 * wall collisions or bounces within that block.
 * Complications arise when the point starts at
 * the edge of a block.  E.g., if a point is on the edge
 * of a block to which block does it belong to?
 *
 * The caller supplies a set of input parameters and expects
 * the following output:
 *  - the number of pixels moved within this block.  (ms->done)
 *  - the number of pixels that still remain to be traversed. (ms->todo)
 *  - whether a crash happened, in which case no pixels will have been
 *    traversed. (ms->crash)
 *  - some extra optional output parameters depending upon the type
 *    of the crash. (ms->cannon, ms->wormhole, ms->target, ms->treasure)
 *  - whether the point bounced, in which case no pixels will have been
 *    traversed, only a change in direction. (ms->bounce, ms->vel, ms->todo)
 */
static void Move_segment(move_state_t *ms)
{
    int			i;
    int			block_type;	/* type of block we're going through */
    int			inside;		/* inside the block or else on edge */
    int			need_adjust;	/* other param (x or y) needs recalc */
    unsigned		wall_bounce;	/* are we bouncing? what direction? */
    ipos		block;		/* block index */
    ipos		blk2;		/* new block index */
    ivec		sign;		/* sign (-1 or 1) of direction */
    clpos		delta;		/* delta position in clicks */
    clpos		enter;		/* enter block position in clicks */
    clpos		leave;		/* leave block position in clicks */
    clpos		offset;		/* offset within block in clicks */
    clpos		off2;		/* last offset in block in clicks */
    clpos		mid;		/* the mean of (offset+off2)/2 */
    const move_info_t	*const mi = ms->mip;	/* alias */
    int			hole;		/* which wormhole */

    /*
     * Fill in default return values.
     */
    ms->crash = NotACrash;
    ms->bounce = NotABounce;
    ms->done.x = 0;
    ms->done.y = 0;

    enter = ms->pos;
    if (enter.x < 0 || enter.x >= mp.click_width
	|| enter.y < 0 || enter.y >= mp.click_height) {

	if (!mi->edge_wrap) {
	    ms->crash = CrashUniverse;
	    return;
	}
	if (enter.x < 0) {
	    enter.x += mp.click_width;
	    if (enter.x < 0) {
		ms->crash = CrashUniverse;
		return;
	    }
	}
	else if (enter.x >= mp.click_width) {
	    enter.x -= mp.click_width;
	    if (enter.x >= mp.click_width) {
		ms->crash = CrashUniverse;
		return;
	    }
	}
	if (enter.y < 0) {
	    enter.y += mp.click_height;
	    if (enter.y < 0) {
		ms->crash = CrashUniverse;
		return;
	    }
	}
	else if (enter.y >= mp.click_height) {
	    enter.y -= mp.click_height;
	    if (enter.y >= mp.click_height) {
		ms->crash = CrashUniverse;
		return;
	    }
	}
	ms->pos = enter;
    }

    sign.x = (ms->vel.x < 0) ? -1 : 1;
    sign.y = (ms->vel.y < 0) ? -1 : 1;

    block.x = enter.x / BLOCK_CLICKS;
    block.y = enter.y / BLOCK_CLICKS;
    offset.x = enter.x - block.x * BLOCK_CLICKS;
    offset.y = enter.y - block.y * BLOCK_CLICKS;
    inside = 1;
    if (offset.x == 0) {
	inside = 0;
	if (sign.x == -1 && (offset.x = BLOCK_CLICKS, --block.x < 0)) {
	    if (mi->edge_wrap) {
		block.x += World.x;
	    }
	    else {
		Bounce_edge(ms, BounceHorLo);
		return;
	    }
	}
    }
    else if (enter.x == mp.click_width - 1
	     && !mi->edge_wrap
	     && ms->vel.x > 0) {
	Bounce_edge(ms, BounceHorHi);
	return;
    }
    if (offset.y == 0) {
	inside = 0;
	if (sign.y == -1 && (offset.y = BLOCK_CLICKS, --block.y < 0)) {
	    if (mi->edge_wrap) {
		block.y += World.y;
	    }
	    else {
		Bounce_edge(ms, BounceVerLo);
		return;
	    }
	}
    }
    else if (enter.y == mp.click_height - 1
	     && !mi->edge_wrap
	     && ms->vel.y > 0) {
	Bounce_edge(ms, BounceVerHi);
	return;
    }

    need_adjust = 0;
    if (sign.x == -1) {
	if (offset.x + ms->todo.x < 0) {
	    leave.x = enter.x - offset.x;
	    need_adjust = 1;
	}
	else {
	    leave.x = enter.x + ms->todo.x;
	}
    }
    else {
	if (offset.x + ms->todo.x > BLOCK_CLICKS) {
	    leave.x = enter.x + BLOCK_CLICKS - offset.x;
	    need_adjust = 1;
	}
	else {
	    leave.x = enter.x + ms->todo.x;
	}
	if (leave.x == mp.click_width && !mi->edge_wrap) {
	    leave.x--;
	    need_adjust = 1;
	}
    }
    if (sign.y == -1) {
	if (offset.y + ms->todo.y < 0) {
	    leave.y = enter.y - offset.y;
	    need_adjust = 1;
	}
	else {
	    leave.y = enter.y + ms->todo.y;
	}
    }
    else {
	if (offset.y + ms->todo.y > BLOCK_CLICKS) {
	    leave.y = enter.y + BLOCK_CLICKS - offset.y;
	    need_adjust = 1;
	}
	else {
	    leave.y = enter.y + ms->todo.y;
	}
	if (leave.y == mp.click_height && !mi->edge_wrap) {
	    leave.y--;
	    need_adjust = 1;
	}
    }
    if (need_adjust && ms->todo.y && ms->todo.x) {
	double wx = (double)(leave.x - enter.x) / ms->todo.x;
	double wy = (double)(leave.y - enter.y) / ms->todo.y;
	if (wx > wy) {
	    double x = ms->todo.x * wy;
	    leave.x = enter.x + DOUBLE_TO_INT(x);
	}
	else if (wx < wy) {
	    double y = ms->todo.y * wx;
	    leave.y = enter.y + DOUBLE_TO_INT(y);
	}
    }

    delta.x = leave.x - enter.x;
    delta.y = leave.y - enter.y;

    block_type = World.block[block.x][block.y];

    /*
     * We test for several different bouncing directions against the wall.
     * Sometimes there is more than one bounce possible if the point
     * starts at the corner of a block.
     * Therefore we maintain a bit mask for the bouncing possibilities
     * and later we will determine which bounce is appropriate.
     */
    wall_bounce = 0;

    switch (block_type) {

    default:
	break;

    case WORMHOLE:
	if (!mi->wormhole_warps) {
	    break;
	}
	hole = wormXY(block.x, block.y);
	if (World.wormHoles[hole].type == WORM_OUT) {
	    break;
	}
	if (mi->player) {
	    blk2.x = mi->player->pos.x / BLOCK_SZ;
	    blk2.y = mi->player->pos.y / BLOCK_SZ;
	    if (BIT(mi->player->status, WARPED)) {
		if (World.block[blk2.x][blk2.y] == WORMHOLE) {
		    int oldhole = wormXY(blk2.x, blk2.y);
		    if (World.wormHoles[oldhole].type == WORM_NORMAL
			&& mi->player->wormHoleDest == oldhole) {
			/*
			 * Don't warp again if we are still on the
			 * same wormhole we have just been warped to.
			 */
			break;
		    }
		}
		CLR_BIT(mi->player->status, WARPED);
	    }
	    if (blk2.x == block.x && blk2.y == block.y) {
		ms->wormhole = hole;
		ms->crash = CrashWormHole;
		return;
	    }
	}
	else {
	    /*
	     * Warp object if this wormhole has ever warped a player.
	     * Warp the object to the same destination as the
	     * player has been warped to.
	     */
	    int last = World.wormHoles[hole].lastdest;
	    if (last >= 0
		&& last < World.NumWormholes
		&& World.wormHoles[last].type != WORM_IN
		&& last != hole
		&& (mi->object->pos.x / BLOCK_SZ != block.x
		 || mi->object->pos.y / BLOCK_SZ != block.y) ) {
		ms->done.x += (World.wormHoles[last].pos.x
		    - World.wormHoles[hole].pos.x) * BLOCK_CLICKS;
		ms->done.y += (World.wormHoles[last].pos.y
		    - World.wormHoles[hole].pos.y) * BLOCK_CLICKS;
		break;
	    }
	}
	break;

    case CANNON:
	if (!mi->cannon_crashes) {
	    break;
	}
	if (BIT(mi->object->status, FROMCANNON)) {
	    break;
	}
	for (i = 0; ; i++) {
	    if (World.cannon[i].pos.x == block.x
		&& World.cannon[i].pos.y == block.y) {
		break;
	    }
	}
	ms->cannon = i;

	{
	    /*
	     * Calculate how far the point can travel in the cannon block
	     * before hitting the cannon.
	     * To reduce duplicate code we first transform all the
	     * different cannon types into one by matrix multiplications.
	     * Later we transform the result back to the real type.
	     */

	    ivec mx, my, dir;
	    clpos mirx, miry, start, end, todo, done, diff, a, b;
	    double d, w;

	    mirx.x = 0;
	    mirx.y = 0;
	    miry.x = 0;
	    miry.y = 0;
	    switch (World.cannon[i].dir) {
	    case DIR_UP:
		mx.x = 1; mx.y = 0;
		my.x = 0; my.y = 1;
		break;
	    case DIR_DOWN:
		mx.x = 1; mx.y = 0;
		my.x = 0; my.y = -1;
		miry.y = BLOCK_CLICKS;
		break;
	    case DIR_RIGHT:
		mx.x = 0; mx.y = 1;
		my.x = -1; my.y = 0;
		miry.x = BLOCK_CLICKS;
		break;
	    case DIR_LEFT:
		mx.x = 0; mx.y = -1;
		my.x = 1; my.y = 0;
		mirx.y = BLOCK_CLICKS;
		break;
	    }
	    start.x = mirx.x + mx.x * offset.x + miry.x + my.x * offset.y;
	    start.y = mirx.y + mx.y * offset.x + miry.y + my.y * offset.y;
	    diff.x  =          mx.x * delta.x           + my.x * delta.y;
	    diff.y  =          mx.y * delta.x           + my.y * delta.y;
	    dir.x   =          mx.x * sign.x            + my.x * sign.y;
	    dir.y   =          mx.y * sign.x            + my.y * sign.y;
	    todo.x  =          mx.x * ms->todo.x       + my.x * ms->todo.y;
	    todo.y  =          mx.y * ms->todo.x       + my.y * ms->todo.y;

	    end.x = start.x + diff.x;
	    end.y = start.y + diff.y;

	    if (start.x <= BLOCK_CLICKS/2) {
		if (3 * start.y <= 2 * start.x) {
		    ms->crash = CrashCannon;
		    return;
		}
		if (end.x <= BLOCK_CLICKS/2) {
		    if (3 * end.y > 2 * end.x) {
			break;
		    }
		}
	    }
	    else {
		if (3 * start.y <= 2 * (BLOCK_CLICKS - start.x)) {
		    ms->crash = CrashCannon;
		    return;
		}
		if (end.x > BLOCK_CLICKS/2) {
		    if (3 * end.y > 2 * (BLOCK_CLICKS - end.x)) {
			break;
		    }
		}
	    }

	    done = diff;

	    /* is direction x-major? */
	    if (dir.x * diff.x >= dir.y * diff.y) {
		/* x-major */
		w = (double) todo.y / todo.x;
		if (3 * todo.y != 2 * todo.x) {
		    d = (3 * start.y - 2 * start.x) / (2 - 3 * w);
		    a.x = DOUBLE_TO_INT(d);
		    a.y = a.x * w;
		    if (dir.x * a.x < dir.x * done.x && dir.x * a.x >= 0) {
			if (start.y + a.y <= BLOCK_CLICKS/3) {
			    done = a;
			    if (!(done.x | done.y)) {
				ms->crash = CrashCannon;
				return;
			    }
			}
		    }
		}
		if (-3 * todo.y != 2 * todo.x) {
		    d = (2 * BLOCK_CLICKS - 2 * start.x - 3 * start.y) /
			(2 + 3 * w);
		    b.x = DOUBLE_TO_INT(d);
		    b.y = b.x * w;
		    if (dir.x * b.x < dir.x * done.x && dir.x * b.x >= 0) {
			if (start.y + b.y <= BLOCK_CLICKS/3) {
			    done = b;
			    if (!(done.x | done.y)) {
				ms->crash = CrashCannon;
				return;
			    }
			}
		    }
		}
	    } else {
		/* y-major */
		w = (double) todo.x / todo.y;
		d = (2 * start.x - 3 * start.y) / (3 - 2 * w);
		a.y = DOUBLE_TO_INT(d);
		a.x = a.y * w;
		if (dir.y * a.y < dir.y * done.y && dir.y * a.y >= 0) {
		    if (start.y + a.y <= BLOCK_CLICKS/3) {
			done = a;
			if (!(done.x | done.y)) {
			    ms->crash = CrashCannon;
			    return;
			}
		    }
		}
		d = (2 * BLOCK_CLICKS - 2 * start.x - 3 * start.y) /
		    (3 + 2 * w);
		b.y = DOUBLE_TO_INT(d);
		b.x = b.y * w;
		if (dir.y * b.y < dir.y * done.y && dir.y * b.y >= 0) {
		    if (start.y + b.y <= BLOCK_CLICKS/3) {
			done = b;
			if (!(done.x | done.y)) {
			    ms->crash = CrashCannon;
			    return;
			}
		    }
		}
	    }

	    delta.x = mx.x * done.x + mx.y * done.y;
	    delta.y = my.x * done.x + my.y * done.y;
	}
	break;

    case TREASURE:
	if (block_type == TREASURE) {
	    if (mi->treasure_crashes) {
		/*
		 * Test if the movement is within the upper half of
		 * the treasure, which is the upper half of a circle.
		 * If this is the case then we test if 3 samples
		 * are not hitting the treasure.
		 */
		const float r = 0.5f * BLOCK_CLICKS;
		off2.x = offset.x + delta.x;
		off2.y = offset.y + delta.y;
		mid.x = (offset.x + off2.x) / 2;
		mid.y = (offset.y + off2.y) / 2;
		if (offset.y > r
		    && off2.y > r
		    && LENGTH(mid.x - r, mid.y - r) > r
		    && LENGTH(off2.x - r, off2.y - r) > r
		    && LENGTH(offset.x - r, offset.y - r) > r) {
		    break;
		}

		for (i = 0; ; i++) {
		    if (World.treasures[i].pos.x == block.x
			&& World.treasures[i].pos.y == block.y) {
			break;
		    }
		}
		ms->treasure = i;
		ms->crash = CrashTreasure;

		/*
		 * We handle balls here, because the reaction
		 * depends on which team the treasure and the ball
		 * belong to.
		 */
		if (mi->object->type != OBJ_BALL) {
		    return;
		}
		if (ms->treasure == mi->object->treasure) {
		    mi->object->life = LONG_MAX;
		    ms->crash = NotACrash;
		    break;
		}
		mi->object->life = 0;
		if (mi->object->owner == -1) {
		    return;
		}
		if (World.treasures[ms->treasure].team ==
			Players[GetInd[mi->object->owner]]->team) {
		    /*
		     * Ball has been brought back to home treasure.
		     * The team should be punished.
		     */
		    if (Punish_team(GetInd[mi->object->owner],
				    mi->object->treasure, ms->treasure))
			CLR_BIT(mi->object->status, RECREATE);
		}
		return;
	    }
	}
	/*FALLTHROUGH*/

    case TARGET:
	if (block_type == TARGET) {
	    if (mi->target_crashes) {
		for (i = 0; ; i++) {
		    if (World.targets[i].pos.x == block.x
			&& World.targets[i].pos.y == block.y) {
			break;
		    }
		}
		ms->target = i;

		if (!targetTeamCollision) {
		    int team;
		    if (mi->player) {
			team = mi->player->team;
		    }
		    else if (BIT(mi->object->type, OBJ_BALL)) {
			if (mi->object->owner != -1) {
			    team = Players[GetInd[mi->object->owner]]->team;
			} else {
			    team = TEAM_NOT_SET;
			}
		    }
		    else if (mi->object->id == -1) {
			team = TEAM_NOT_SET;
		    }
		    else {
			team = Players[GetInd[mi->object->id]]->team;
		    }
		    if (team == World.targets[i].team) {
			break;
		    }
		}
		if (!mi->player) {
		    ms->crash = CrashTarget;
		    return;
		}
	    }
	}
	/*FALLTHROUGH*/

    case FUEL:
    case FILLED:
	if (inside) {
	    /* Could happen for targets reappearing and in case of bugs. */
	    ms->crash = CrashWall;
	    return;
	}
	if (offset.x == 0) {
	    if (ms->vel.x > 0) {
		wall_bounce |= BounceHorLo;
	    }
	}
	else if (offset.x == BLOCK_CLICKS) {
	    if (ms->vel.x < 0) {
		wall_bounce |= BounceHorHi;
	    }
	}
	if (offset.y == 0) {
	    if (ms->vel.y > 0) {
		wall_bounce |= BounceVerLo;
	    }
	}
	else if (offset.y == BLOCK_CLICKS) {
	    if (ms->vel.y < 0) {
		wall_bounce |= BounceVerHi;
	    }
	}
	if (wall_bounce) {
	    break;
	}
	if (!(ms->todo.x | ms->todo.y)) {
	    /* no bouncing possible and no movement.  OK. */
	    break;
	}
	if (!ms->todo.x && (offset.x == 0 || offset.x == BLOCK_CLICKS)) {
	    /* tricky */
	    break;
	}
	if (!ms->todo.y && (offset.y == 0 || offset.y == BLOCK_CLICKS)) {
	    /* tricky */
	    break;
	}
	/* what happened? we should never reach this */
	ms->crash = CrashWall;
	return;

    case REC_LD:
	/* test for bounces first. */
	if (offset.x == 0) {
	    if (ms->vel.x > 0) {
		wall_bounce |= BounceHorLo;
	    }
	    if (offset.y == BLOCK_CLICKS && ms->vel.x + ms->vel.y < 0) {
		wall_bounce |= BounceLeftDown;
	    }
	}
	if (offset.y == 0) {
	    if (ms->vel.y > 0) {
		wall_bounce |= BounceVerLo;
	    }
	    if (offset.x == BLOCK_CLICKS && ms->vel.x + ms->vel.y < 0) {
		wall_bounce |= BounceLeftDown;
	    }
	}
	if (wall_bounce) {
	    break;
	}
	if (offset.x + offset.y < BLOCK_CLICKS) {
	    ms->crash = CrashWall;
	    return;
	}
	if (offset.x + delta.x + offset.y + delta.y >= BLOCK_CLICKS) {
	    /* movement is entirely within the space part of the block. */
	    break;
	}
	/*
	 * Find out where we bounce exactly
	 * and how far we can move before bouncing.
	 */
	if (sign.x * ms->todo.x >= sign.y * ms->todo.y) {
	    double w = (double) ms->todo.y / ms->todo.x;
	    delta.x = (BLOCK_CLICKS - offset.x - offset.y) / (1 + w);
	    delta.y = delta.x * w;
	    if (offset.x + delta.x + offset.y + delta.y < BLOCK_CLICKS) {
		delta.x++;
		delta.y = delta.x * w;
	    }
	    leave.x = enter.x + delta.x;
	    leave.y = enter.y + delta.y;
	    if (!delta.x) {
		wall_bounce |= BounceLeftDown;
		break;
	    }
	}
	else {
	    double w = (double) ms->todo.x / ms->todo.y;
	    delta.y = (BLOCK_CLICKS - offset.x - offset.y) / (1 + w);
	    delta.x = delta.y * w;
	    if (offset.x + delta.x + offset.y + delta.y < BLOCK_CLICKS) {
		delta.y++;
		delta.x = delta.y * w;
	    }
	    leave.x = enter.x + delta.x;
	    leave.y = enter.y + delta.y;
	    if (!delta.y) {
		wall_bounce |= BounceLeftDown;
		break;
	    }
	}
	break;

    case REC_LU:
	if (offset.x == 0) {
	    if (ms->vel.x > 0) {
		wall_bounce |= BounceHorLo;
	    }
	    if (offset.y == 0 && ms->vel.x < ms->vel.y) {
		wall_bounce |= BounceLeftUp;
	    }
	}
	if (offset.y == BLOCK_CLICKS) {
	    if (ms->vel.y < 0) {
		wall_bounce |= BounceVerHi;
	    }
	    if (offset.x == BLOCK_CLICKS && ms->vel.x < ms->vel.y) {
		wall_bounce |= BounceLeftUp;
	    }
	}
	if (wall_bounce) {
	    break;
	}
	if (offset.x < offset.y) {
	    ms->crash = CrashWall;
	    return;
	}
	if (offset.x + delta.x >= offset.y + delta.y) {
	    break;
	}
	if (sign.x * ms->todo.x >= sign.y * ms->todo.y) {
	    double w = (double) ms->todo.y / ms->todo.x;
	    delta.x = (offset.y - offset.x) / (1 - w);
	    delta.y = delta.x * w;
	    if (offset.x + delta.x < offset.y + delta.y) {
		delta.x++;
		delta.y = delta.x * w;
	    }
	    leave.x = enter.x + delta.x;
	    leave.y = enter.y + delta.y;
	    if (!delta.x) {
		wall_bounce |= BounceLeftUp;
		break;
	    }
	}
	else {
	    double w = (double) ms->todo.x / ms->todo.y;
	    delta.y = (offset.x - offset.y) / (1 - w);
	    delta.x = delta.y * w;
	    if (offset.x + delta.x < offset.y + delta.y) {
		delta.y--;
		delta.x = delta.y * w;
	    }
	    leave.x = enter.x + delta.x;
	    leave.y = enter.y + delta.y;
	    if (!delta.y) {
		wall_bounce |= BounceLeftUp;
		break;
	    }
	}
	break;

    case REC_RD:
	if (offset.x == BLOCK_CLICKS) {
	    if (ms->vel.x < 0) {
		wall_bounce |= BounceHorHi;
	    }
	    if (offset.y == BLOCK_CLICKS && ms->vel.x > ms->vel.y) {
		wall_bounce |= BounceRightDown;
	    }
	}
	if (offset.y == 0) {
	    if (ms->vel.y > 0) {
		wall_bounce |= BounceVerLo;
	    }
	    if (offset.x == 0 && ms->vel.x > ms->vel.y) {
		wall_bounce |= BounceRightDown;
	    }
	}
	if (wall_bounce) {
	    break;
	}
	if (offset.x > offset.y) {
	    ms->crash = CrashWall;
	    return;
	}
	if (offset.x + delta.x <= offset.y + delta.y) {
	    break;
	}
	if (sign.x * ms->todo.x >= sign.y * ms->todo.y) {
	    double w = (double) ms->todo.y / ms->todo.x;
	    delta.x = (offset.y - offset.x) / (1 - w);
	    delta.y = delta.x * w;
	    if (offset.x + delta.x > offset.y + delta.y) {
		delta.x--;
		delta.y = delta.x * w;
	    }
	    leave.x = enter.x + delta.x;
	    leave.y = enter.y + delta.y;
	    if (!delta.x) {
		wall_bounce |= BounceRightDown;
		break;
	    }
	}
	else {
	    double w = (double) ms->todo.x / ms->todo.y;
	    delta.y = (offset.x - offset.y) / (1 - w);
	    delta.x = delta.y * w;
	    if (offset.x + delta.x > offset.y + delta.y) {
		delta.y++;
		delta.x = delta.y * w;
	    }
	    leave.x = enter.x + delta.x;
	    leave.y = enter.y + delta.y;
	    if (!delta.y) {
		wall_bounce |= BounceRightDown;
		break;
	    }
	}
	break;

    case REC_RU:
	if (offset.x == BLOCK_CLICKS) {
	    if (ms->vel.x < 0) {
		wall_bounce |= BounceHorHi;
	    }
	    if (offset.y == 0 && ms->vel.x + ms->vel.y > 0) {
		wall_bounce |= BounceRightUp;
	    }
	}
	if (offset.y == BLOCK_CLICKS) {
	    if (ms->vel.y < 0) {
		wall_bounce |= BounceVerHi;
	    }
	    if (offset.x == 0 && ms->vel.x + ms->vel.y > 0) {
		wall_bounce |= BounceRightUp;
	    }
	}
	if (wall_bounce) {
	    break;
	}
	if (offset.x + offset.y > BLOCK_CLICKS) {
	    ms->crash = CrashWall;
	    return;
	}
	if (offset.x + delta.x + offset.y + delta.y <= BLOCK_CLICKS) {
	    break;
	}
	if (sign.x * ms->todo.x >= sign.y * ms->todo.y) {
	    double w = (double) ms->todo.y / ms->todo.x;
	    delta.x = (BLOCK_CLICKS - offset.x - offset.y) / (1 + w);
	    delta.y = delta.x * w;
	    if (offset.x + delta.x + offset.y + delta.y > BLOCK_CLICKS) {
		delta.x--;
		delta.y = delta.x * w;
	    }
	    leave.x = enter.x + delta.x;
	    leave.y = enter.y + delta.y;
	    if (!delta.x) {
		wall_bounce |= BounceRightUp;
		break;
	    }
	}
	else {
	    double w = (double) ms->todo.x / ms->todo.y;
	    delta.y = (BLOCK_CLICKS - offset.x - offset.y) / (1 + w);
	    delta.x = delta.y * w;
	    if (offset.x + delta.x + offset.y + delta.y > BLOCK_CLICKS) {
		delta.y--;
		delta.x = delta.y * w;
	    }
	    leave.x = enter.x + delta.x;
	    leave.y = enter.y + delta.y;
	    if (!delta.y) {
		wall_bounce |= BounceRightUp;
		break;
	    }
	}
	break;
    }

    if (wall_bounce) {
	/*
	 * Bouncing.  As there may be more than one possible bounce
	 * test which bounce is not feasible because of adjacent walls.
	 * If there still is more than one possible then pick one randomly.
	 * Else if it turns out that none is feasible then we must have
	 * been trapped inbetween two blocks.  This happened in the early
	 * stages of this code.
	 */
	int count = 0;
	unsigned bit;
	unsigned save_wall_bounce = wall_bounce;
	unsigned block_mask = FILLED_BIT | FUEL_BIT;

	if (!mi->target_crashes) {
	    block_mask |= TARGET_BIT;
	}
	if (!mi->treasure_crashes) {
	    block_mask |= TREASURE_BIT;
	}
	for (bit = 1; bit <= wall_bounce; bit <<= 1) {
	    if (!(wall_bounce & bit)) {
		continue;
	    }

	    CLR_BIT(wall_bounce, bit);
	    switch (bit) {

	    case BounceHorLo:
		blk2.x = block.x - 1;
		if (blk2.x < 0) {
		    if (!mi->edge_wrap) {
			continue;
		    }
		    blk2.x += World.x;
		}
		blk2.y = block.y;
		if (BIT(1 << World.block[blk2.x][blk2.y],
			block_mask|REC_RU_BIT|REC_RD_BIT)) {
		    continue;
		}
		break;

	    case BounceHorHi:
		blk2.x = block.x + 1;
		if (blk2.x >= World.x) {
		    if (!mi->edge_wrap) {
			continue;
		    }
		    blk2.x -= World.x;
		}
		blk2.y = block.y;
		if (BIT(1 << World.block[blk2.x][blk2.y],
			block_mask|REC_LU_BIT|REC_LD_BIT)) {
		    continue;
		}
		break;

	    case BounceVerLo:
		blk2.x = block.x;
		blk2.y = block.y - 1;
		if (blk2.y < 0) {
		    if (!mi->edge_wrap) {
			continue;
		    }
		    blk2.y += World.y;
		}
		if (BIT(1 << World.block[blk2.x][blk2.y],
			block_mask|REC_RU_BIT|REC_LU_BIT)) {
		    continue;
		}
		break;

	    case BounceVerHi:
		blk2.x = block.x;
		blk2.y = block.y + 1;
		if (blk2.y >= World.y) {
		    if (!mi->edge_wrap) {
			continue;
		    }
		    blk2.y -= World.y;
		}
		if (BIT(1 << World.block[blk2.x][blk2.y],
			block_mask|REC_RD_BIT|REC_LD_BIT)) {
		    continue;
		}
		break;
	    }

	    SET_BIT(wall_bounce, bit);
	    count++;
	}

	if (!count) {
	    wall_bounce = save_wall_bounce;
	    switch (wall_bounce) {
	    case BounceHorLo|BounceVerLo:
		wall_bounce = BounceLeftDown;
		break;
	    case BounceHorLo|BounceVerHi:
		wall_bounce = BounceLeftUp;
		break;
	    case BounceHorHi|BounceVerLo:
		wall_bounce = BounceRightDown;
		break;
	    case BounceHorHi|BounceVerHi:
		wall_bounce = BounceRightUp;
		break;
	    default:
		switch (block_type) {
		case REC_LD:
		    if ((offset.x == 0) ? (offset.y == BLOCK_CLICKS)
			: (offset.x == BLOCK_CLICKS && offset.y == 0)
			&& ms->vel.x + ms->vel.y >= 0) {
			wall_bounce = 0;
		    }
		    break;
		case REC_LU:
		    if ((offset.x == 0) ? (offset.y == 0)
			: (offset.x == BLOCK_CLICKS && offset.y == BLOCK_CLICKS)
			&& ms->vel.x >= ms->vel.y) {
			wall_bounce = 0;
		    }
		    break;
		case REC_RD:
		    if ((offset.x == 0) ? (offset.y == 0)
			: (offset.x == BLOCK_CLICKS && offset.y == BLOCK_CLICKS)
			&& ms->vel.x <= ms->vel.y) {
			wall_bounce = 0;
		    }
		    break;
		case REC_RU:
		    if ((offset.x == 0) ? (offset.y == BLOCK_CLICKS)
			: (offset.x == BLOCK_CLICKS && offset.y == 0)
			&& ms->vel.x + ms->vel.y <= 0) {
			wall_bounce = 0;
		    }
		    break;
		}
		if (wall_bounce) {
		    ms->crash = CrashWall;
		    return;
		}
	    }
	}
	else if (count > 1) {
	    /*
	     * More than one bounce possible.
	     * Pick one randomly.
	     */
	    count = rand() % count;
	    for (bit = 1; bit <= wall_bounce; bit <<= 1) {
		if (wall_bounce & bit) {
		    if (count == 0) {
			wall_bounce = bit;
			break;
		    } else {
			count--;
		    }
		}
	    }
	}
    }
    if (wall_bounce) {
	Bounce_wall(ms, (move_bounce_t) wall_bounce);
    }
    else {
	ms->done.x += delta.x;
	ms->done.y += delta.y;
	ms->todo.x -= delta.x;
	ms->todo.y -= delta.y;
    }
}

static void Cannon_dies(move_state_t *ms)
{
    int			x = World.cannon[ms->cannon].pos.x;
    int			y = World.cannon[ms->cannon].pos.y;
    int			sc;

    World.cannon[ms->cannon].dead_time = CANNON_DEAD_TIME;
    World.cannon[ms->cannon].active = false;
    World.cannon[ms->cannon].conn_mask = 0;
    World.cannon[ms->cannon].last_change = loops;
    World.block[x][y] = SPACE;
    Explode_object(NULL,
		   (float)(x*BLOCK_SZ), (float)(y*BLOCK_SZ),
		   World.cannon[ms->cannon].dir, RES*0.4,
		   120);
    if (!ms->mip->player) {
	object *obj = ms->mip->object;
	if (obj->id >= 0) {
	    int killer = GetInd[obj->id];
	    player *pl = Players[killer];
	    sc = Rate(pl->score, CANNON_SCORE) / 4;
	    SCORE(killer, sc, x, y, "");
	}
    }
}

static void Object_hits_target(move_state_t *ms)
{
    target_t		*targ = &World.targets[ms->target];
    object		*obj = ms->mip->object;
    int			j, sc, x, y,
			killer;
    int			win_score = 0,
			lose_score = 0,
			somebody_flag = 0,
			targets_remaining = 0;

    /* a normal shot or a direct mine hit work, cannons don't */
    if (!BIT(obj->type, KILLING_SHOTS|OBJ_MINE|OBJ_PULSE)) {
	return;
    }
    if (obj->id <= 0) {
	return;
    }
    killer = GetInd[obj->id];
    if (targ->team == Players[killer]->team) {
	return;
    }

    switch(obj->type) {
    case OBJ_SHOT:
	targ->damage += (ED_SHOT_HIT * SHOT_MULT(obj));
	break;
    case OBJ_PULSE:
	targ->damage += ED_LASER_HIT;
	break;
    case OBJ_SMART_SHOT:
    case OBJ_TORPEDO:
    case OBJ_HEAT_SHOT:
	if (BIT(obj->mods.nuclear, NUCLEAR)) {
	    targ->damage = 0;
	}
	else {
	    targ->damage += ED_SMART_SHOT_HIT / (obj->mods.mini + 1);
	}
	break;
    case OBJ_MINE:
	targ->damage -= TARGET_DAMAGE / (obj->mods.mini + 1);
	break;
    default:
	/*???*/
	break;
    }

    targ->conn_mask = 0;
    targ->last_change = loops;
    if (targ->damage > 0)
	return;

    /*
     * Destroy target.
     * Turn it into a space to simplify other calculations.
     */
    x = targ->pos.x;
    y = targ->pos.y;
    World.block[x][y] = SPACE;

    targ->damage = TARGET_DAMAGE;
    targ->dead_time = TARGET_DEAD_TIME;

    Explode_object(NULL,
		   x*BLOCK_SZ + BLOCK_SZ/2, y*BLOCK_SZ + BLOCK_SZ/2,
		   0, RES, 200);
    sound_play_all(DESTROY_TARGET_SOUND);

    if (BIT(World.rules->mode, TEAM_PLAY)) {
	for (j = 0; j < NumPlayers; j++) {
	    if (Players[j]->robot_mode == RM_OBJECT) {
		continue;
	    }
	    if (BIT(Players[j]->status, PAUSE)) {
		continue;
	    }
	    if (Players[j]->team == targ->team) {
		lose_score += Players[j]->score;
		if (BIT(Players[j]->status, GAME_OVER) == 0) {
		    somebody_flag = 1;
		}
	    }
	    else if (Players[j]->team == Players[killer]->team)
		win_score += Players[j]->score;
	}
    }
    if (somebody_flag) {
	for (j = 0; j < World.NumTargets; j++) {
	    if (World.targets[j].team == targ->team) {
		if (World.targets[j].dead_time == 0) {
		    targets_remaining++;
		}
	    }
	}
    }
    if (!somebody_flag || targets_remaining > 0) {
	SCORE(killer, Rate(Players[killer]->score, CANNON_SCORE)/4,
	      targ->pos.x, targ->pos.y, "");
	return;
    }

    sprintf(msg, "%s blew up team %d's target.",
	    Players[killer]->name, (int) targ->team);
    Set_message(msg);

    sc = Rate(win_score, lose_score);
    for (j = 0; j < NumPlayers; j++) {
	if (Players[j]->team == targ->team) {
	    SCORE(j, -sc, targ->pos.x, targ->pos.y,
		  "Target: ");
	    if (targetKillTeam
		&& targets_remaining == 0
		&& !BIT(Players[j]->status, KILLED|PAUSE|GAME_OVER))
		SET_BIT(Players[j]->status, KILLED);
	}
	else if (Players[j]->team == Players[killer]->team &&
		 (Players[j]->team != TEAM_NOT_SET || j == killer)) {
	    SCORE(j, sc, targ->pos.x, targ->pos.y,
		  "Target: ");
	}
    }
}

static void Object_crash(move_state_t *ms)
{
    object		*obj = ms->mip->object;

    switch (ms->crash) {

    case CrashWormHole:
    default:
	break;

    case CrashTreasure:
	/*
	 * Ball type has already been handled.
	 */
	if (obj->type == OBJ_BALL) {
	    break;
	}
	obj->life = 0;
	break;

    case CrashTarget:
	obj->life = 0;
	Object_hits_target(ms);
	break;

    case CrashWall:
	obj->life = 0;
	break;

    case CrashUniverse:
	obj->life = 0;
	break;

    case CrashCannon:
	obj->life = 0;
	Cannon_dies(ms);
	break;

    case CrashUnknown:
	obj->life = 0;
	break;

    }

}

void Move_object(int ind)
{
    object		*obj = Obj[ind];
    int			nothing_done = 0;
    move_info_t		mi;
    move_state_t	ms;

    obj->prevpos = obj->pos;

    mi.player = NULL;
    mi.object = obj;
    mi.edge_wrap = BIT(World.rules->mode, WRAP_PLAY);
    mi.edge_bounce = edgeBounce;
    mi.wall_bounce = BIT(mp.obj_bounce_mask, obj->type);
    mi.cannon_crashes = BIT(mp.obj_cannon_mask, obj->type);
    mi.target_crashes = BIT(mp.obj_target_mask, obj->type);
    mi.treasure_crashes = BIT(mp.obj_treasure_mask, obj->type);
    mi.wormhole_warps = true;

    ms.pos.x = FLOAT_TO_CLICK(obj->pos.x);
    ms.pos.y = FLOAT_TO_CLICK(obj->pos.y);
    ms.vel = obj->vel;
    ms.todo.x = FLOAT_TO_CLICK(obj->pos.x + obj->vel.x) - ms.pos.x;
    ms.todo.y = FLOAT_TO_CLICK(obj->pos.y + obj->vel.y) - ms.pos.y;
    ms.dir = obj->dir;
    ms.mip = &mi;

    for (;;) {
	Move_segment(&ms);
	if (!(ms.done.x | ms.done.y)) {
	    if (ms.crash) {
		break;
	    }
	    if (ms.bounce && ms.bounce != BounceEdge) {
		obj->life *= objectWallBounceLifeFactor;
		if (obj->life <= 0) {
		    break;
		}
		/*
		 * Any bouncing sparks are no longer owner immune to give
		 * "reactive" thrust.  This is exactly like ground effect
		 * in the real world.  Very useful for stopping against walls.
		 *
		 * If the FROMBOUNCE bit is set the spark was caused by
		 * the player bouncing of a wall and thus although the spark
		 * should bounce, it is not reactive thrust otherwise wall
		 * bouncing would cause acceleration of the player.
		 */
		if (!BIT(obj->status, FROMBOUNCE) && BIT(obj->type, OBJ_SPARK))
		    CLR_BIT(obj->status, OWNERIMMUNE);
		if (VECTOR_LENGTH(ms.vel) > maxObjectWallBounceSpeed) {
		    obj->life = 0;
		    break;
		}
		ms.vel.x *= objectWallBrakeFactor;
		ms.vel.y *= objectWallBrakeFactor;
		ms.todo.x *= objectWallBrakeFactor;
		ms.todo.y *= objectWallBrakeFactor;
	    }
	    if (++nothing_done >= 5) {
		ms.crash = CrashUnknown;
		break;
	    }
	} else {
	    ms.pos.x += ms.done.x;
	    ms.pos.y += ms.done.y;
	    nothing_done = 0;
	}
	if (!(ms.todo.x | ms.todo.y)) {
	    break;
	}
    }
    if (mi.edge_wrap) {
	if (ms.pos.x < 0) {
	    ms.pos.x += mp.click_width;
	}
	if (ms.pos.x >= mp.click_width) {
	    ms.pos.x -= mp.click_width;
	}
	if (ms.pos.y < 0) {
	    ms.pos.y += mp.click_height;
	}
	if (ms.pos.y >= mp.click_height) {
	    ms.pos.y -= mp.click_height;
	}
    }
    obj->pos.x = CLICK_TO_FLOAT(ms.pos.x);
    obj->pos.y = CLICK_TO_FLOAT(ms.pos.y);
    obj->vel = ms.vel;
    obj->dir = ms.dir;
    if (ms.crash) {
	Object_crash(&ms);
    }
}

static void Player_crash(move_state_t *ms, int pt, bool turning)
{
    player		*pl = ms->mip->player;
    int			ind = GetInd[pl->id];
    char		*how = NULL;
    int			sc;

    msg[0] = '\0';

    switch (ms->crash) {

    default:
    case NotACrash:
	errno = 0;
	error("Player_crash not a crash %d", ms->crash);
	break;

    case CrashWormHole:
	SET_BIT(pl->status, WARPING);
	pl->wormHoleHit = ms->wormhole;
	break;

    case CrashWall:
	how = "crashed";
    case CrashWallSpeed:
	if (!how) how = "smashed";
    case CrashWallNoFuel:
	if (!how) how = "smacked";
    case CrashWallAngle:
	if (!how) how = "was trashed";

	SET_BIT(pl->status, KILLED);
	sprintf(msg, "%s %s %sagainst the wall%s.", pl->name, how,
	        (!pt) ? "head first " : "",
	        (turning) ? " while turning" : "");
	SCORE(ind, -Rate(WALL_SCORE, pl->score),
	      (int) pl->pos.x/BLOCK_SZ,
	      (int) pl->pos.y/BLOCK_SZ,
	      "[Wall]");
	sound_play_sensors(pl->pos.x, pl->pos.y, PLAYER_HIT_WALL_SOUND);
	break;

    case CrashTarget:
	SET_BIT(pl->status, KILLED);
	sprintf(msg, "%s smashed %sagainst a target.", pl->name,
	        (!pt) ? "head first " : "");
	SCORE(ind, -Rate(WALL_SCORE, pl->score),
	      (int) pl->pos.x/BLOCK_SZ,
	      (int) pl->pos.y/BLOCK_SZ,
	      "[Target]");
	sound_play_sensors(pl->pos.x, pl->pos.y, PLAYER_HIT_WALL_SOUND);
	break;

    case CrashTreasure:
	SET_BIT(pl->status, KILLED);
	sprintf(msg, "%s smashed %sagainst a treasure.", pl->name,
		(!pt) ? "head first " : "");
	sc = Rate(WALL_SCORE, pl->score);
	SCORE(ind, -sc, 
	      (int) pl->pos.x/BLOCK_SZ,
	      (int) pl->pos.y/BLOCK_SZ,
	      "[Treasure]");
	sound_play_sensors(pl->pos.x, pl->pos.y, PLAYER_HIT_WALL_SOUND);
	break;

    case CrashCannon:
	SET_BIT(pl->status, KILLED);
	sprintf(msg, "%s smashed %sagainst a cannon.", pl->name,
		(!pt) ? "head first " : "");
	sc = Rate(WALL_SCORE, pl->score);
	SCORE(ind, -sc,
	      (int) pl->pos.x/BLOCK_SZ,
	      (int) pl->pos.y/BLOCK_SZ,
	      "[Cannon]");
	Cannon_dies(ms);
	sound_play_sensors(pl->pos.x, pl->pos.y, PLAYER_HIT_CANNON_SOUND);
	break;

    case CrashUniverse:
	SET_BIT(pl->status, KILLED);
	sprintf(msg, "%s left the known universe.", pl->name);
	sc = Rate(WALL_SCORE, pl->score);
	SCORE(ind, -sc,
	      (int) pl->pos.x/BLOCK_SZ,
	      (int) pl->pos.y/BLOCK_SZ,
	      "[Universe]");
	sound_play_sensors(pl->pos.x, pl->pos.y, PLAYER_HIT_WALL_SOUND);
	break;

    case CrashUnknown:
	SET_BIT(pl->status, KILLED);
	sprintf(msg, "%s crashed for unknown reasons.", pl->name);
	sc = Rate(WALL_SCORE, pl->score);
	SCORE(ind, -sc,
	      (int) pl->pos.x/BLOCK_SZ,
	      (int) pl->pos.y/BLOCK_SZ,
	      "[Bug]");
	sound_play_sensors(pl->pos.x, pl->pos.y, PLAYER_HIT_WALL_SOUND);
	break;

    }

    if (msg[0]) {
	Set_message(msg);
    }

    if (BIT(pl->status, KILLED)
	&& pl->score < 0
	&& pl->robot_mode != RM_NOT_ROBOT 
	&& pl->robot_mode != RM_OBJECT) {
	pl->home_base = 0;
	Pick_startpos(ind);
    }
}

void Move_player(int ind)
{
    player		*pl = Players[ind];
    int			nothing_done = 0;
    int			i;
    move_info_t		mi;
    move_state_t	ms[RES];
    int			worst;
    int			crash;
    int			bounce;
    clpos		pos;
    clvec		todo;
    vector		vel;
    vector		r[RES];


    if (BIT(pl->status, PLAYING|PAUSE|GAME_OVER|KILLED) != PLAYING) {
	if (!BIT(pl->status, KILLED|PAUSE)) {
	    pl->prevpos = pl->pos;
	    pl->pos.x += pl->vel.x;
	    pl->pos.y += pl->vel.y;
	    if (BIT(World.rules->mode, WRAP_PLAY)) {
		if (pl->pos.x < 0) {
		    pl->pos.x += World.width;
		}
		else if (pl->pos.x >= World.width) {
		    pl->pos.x -= World.width;
		}
		if (pl->pos.y < 0) {
		    pl->pos.y += World.height;
		}
		else if (pl->pos.y >= World.height) {
		    pl->pos.y -= World.height;
		}
	    }
	}
	pl->velocity = VECTOR_LENGTH(pl->vel);
	return;
    }

    if (pl->pos.x < 0 || pl->pos.x >= World.width
	|| pl->pos.y < 0 || pl->pos.y >= World.height) {
	SET_BIT(pl->status, KILLED);
	sprintf(msg, "%s stepped out of this world.", pl->name);
	Set_message(msg);
	LIMIT(pl->pos.x, 0, World.width - 1);
	LIMIT(pl->pos.y, 0, World.height - 1);
	pl->prevpos = pl->pos;
	pl->velocity = VECTOR_LENGTH(pl->vel);
	sound_play_sensors(pl->pos.x, pl->pos.y, PLAYER_HIT_WALL_SOUND);
	return;
    }

    mi.player = pl;
    mi.object = (object *) pl;
    mi.edge_wrap = BIT(World.rules->mode, WRAP_PLAY);
    mi.edge_bounce = edgeBounce;
    mi.wall_bounce = true;
    mi.cannon_crashes = true;
    mi.treasure_crashes = true;
    mi.target_crashes = true;
    mi.wormhole_warps = true;

    if (!mi.edge_wrap) {
	if (pl->pos.x < 15) {
	    for (i = 0; i < NUM_POINTS; i++) {
		if (pl->pos.x + pl->ship->pts[pl->dir][i].x < 0) {
		    pl->pos.x = -pl->ship->pts[pl->dir][i].x;
		}
	    }
	}
	if (pl->pos.x >= World.width - 16) {
	    for (i = 0; i < NUM_POINTS; i++) {
		if (pl->pos.x + pl->ship->pts[pl->dir][i].x
		    >= World.width - 0.5f / CLICK) {
		    pl->pos.x = World.width - pl->ship->pts[pl->dir][i].x
				- 1.0f / CLICK;
		}
	    }
	}
	if (pl->pos.y < 15) {
	    for (i = 0; i < NUM_POINTS; i++) {
		if (pl->pos.y + pl->ship->pts[pl->dir][i].y < 0) {
		    pl->pos.y = -pl->ship->pts[pl->dir][i].y;
		}
	    }
	}
	if (pl->pos.y >= World.height - 16) {
	    for (i = 0; i < NUM_POINTS; i++) {
		if (pl->pos.y + pl->ship->pts[pl->dir][i].y
		    >= World.height - 0.5f / CLICK) {
		    pl->pos.y = World.height - pl->ship->pts[pl->dir][i].y
				- 1.0f / CLICK;
		}
	    }
	}
    }
    pl->prevpos = pl->pos;

    vel = pl->vel;
    todo.x = FLOAT_TO_CLICK(vel.x);
    todo.y = FLOAT_TO_CLICK(vel.y);
    for (i = 0; i < NUM_POINTS; i++) {
	float x = pl->pos.x + pl->ship->pts[pl->dir][i].x;
	float y = pl->pos.y + pl->ship->pts[pl->dir][i].y;
	ms[i].pos.x = FLOAT_TO_CLICK(x);
	ms[i].pos.y = FLOAT_TO_CLICK(y);
	ms[i].vel = vel;
	ms[i].todo = todo;
	ms[i].dir = pl->dir;
	ms[i].mip = &mi;
    }

    for (;;) {
	bounce = -1;
	crash = -1;
	for (i = 0; i < NUM_POINTS; i++) {
	    Move_segment(&ms[i]);
	    if (ms[i].crash) {
		crash = i;
		break;
	    }
	    if (ms[i].bounce) {
		if (bounce == -1) {
		    bounce = i;
		}
		else if (ms[bounce].bounce != BounceEdge
		    && ms[i].bounce == BounceEdge) {
		    bounce = i;
		}
		else if ((ms[bounce].bounce == BounceEdge)
		    == (ms[i].bounce == BounceEdge)) {
		    if ((rand() % (NUM_POINTS - bounce)) == i) {
			bounce = i;
		    }
		}
		worst = bounce;
	    }
	}
	if (crash != -1) {
	    worst = crash;
	    break;
	}
	else if (bounce != -1) {
	    worst = bounce;
	    if (ms[worst].bounce != BounceEdge) {
		float	speed = VECTOR_LENGTH(ms[worst].vel);
		int	v = (int) speed >> 2;
		int	m = (int) (pl->mass - pl->emptymass * 0.75f);
		float	b = 1 - 0.5f * playerWallBrakeFactor;
		long	cost = (long) (b * m * v);
		int	delta_dir,
			abs_delta_dir,
			wall_dir;
		float	max_speed = BIT(pl->used, OBJ_SHIELD)
				    ? maxShieldedWallBounceSpeed
				    : maxUnshieldedWallBounceSpeed;
		int	max_angle = BIT(pl->used, OBJ_SHIELD)
				    ? mp.max_shielded_angle
				    : mp.max_unshielded_angle;

		ms[worst].vel.x *= playerWallBrakeFactor;
		ms[worst].vel.y *= playerWallBrakeFactor;
		ms[worst].todo.x *= playerWallBrakeFactor;
		ms[worst].todo.y *= playerWallBrakeFactor;

		if (speed > max_speed) {
		    crash = worst;
		    ms[worst].crash = CrashWallSpeed;
		    break;
		}

		switch (ms[worst].bounce) {
		case BounceHorLo: wall_dir = 4*RES/8; break;
		case BounceHorHi: wall_dir = 0*RES/8; break;
		case BounceVerLo: wall_dir = 6*RES/8; break;
		default:
		case BounceVerHi: wall_dir = 2*RES/8; break;
		case BounceLeftDown: wall_dir = 1*RES/8; break;
		case BounceLeftUp: wall_dir = 7*RES/8; break;
		case BounceRightDown: wall_dir = 3*RES/8; break;
		case BounceRightUp: wall_dir = 5*RES/8; break;
		}
		if (pl->dir >= wall_dir) {
		    delta_dir = (pl->dir - wall_dir <= RES/2)
				? -(pl->dir - wall_dir)
				: (wall_dir + RES - pl->dir);
		} else {
		    delta_dir = (wall_dir - pl->dir <= RES/2)
				? (wall_dir - pl->dir)
				: -(pl->dir + RES - wall_dir);
		}
		abs_delta_dir = ABS(delta_dir);
		if (abs_delta_dir > max_angle) {
		    crash = worst;
		    ms[worst].crash = CrashWallAngle;
		    break;
		}
		if (abs_delta_dir <= RES/16) {
		    pl->float_dir += (1.0f - playerWallBrakeFactor) * delta_dir;
		    if (pl->float_dir >= RES) {
			pl->float_dir -= RES;
		    }
		    else if (pl->float_dir < 0) {
			pl->float_dir += RES;
		    }
		}

		/*
		 * Small explosion and fuel loss if survived a hit on a wall.
		 * This doesn't affect the player as the explosion is sparks
		 * which don't collide with player.
		 * Clumsy touches (head first) with wall are more costly.
		 */
		cost = (cost * (RES/2 + abs_delta_dir)) / RES;
		Add_fuel(&pl->fuel, -(cost << FUEL_SCALE_BITS)
				    * wallBounceFuelDrainMult);
		Item_damage(ind, (cost * wallBounceDestroyItemProb));
		if (!pl->fuel.sum) {
		    crash = worst;
		    ms[worst].crash = CrashWallNoFuel;
		    break;
		}
		if (cost) {
		    Explode_object((object *)pl,
				   pl->pos.x, pl->pos.y,
				   wall_dir, RES/2,
				   (int)(cost * wallBounceExplosionMult));
		    sound_play_sensors(pl->pos.x, pl->pos.y,
				       PLAYER_BOUNCED_SOUND);
		}
	    }
	}
	else {
	    for (i = 0; i < NUM_POINTS; i++) {
		r[i].x = (vel.x) ? (float) ms[i].todo.x / vel.x : 0;
		r[i].y = (vel.y) ? (float) ms[i].todo.y / vel.y : 0;
		r[i].x = ABS(r[i].x);
		r[i].y = ABS(r[i].y);
	    }
	    worst = 0;
	    for (i = 1; i < NUM_POINTS; i++) {
		if (r[i].x > r[worst].x || r[i].y > r[worst].y) {
		    worst = i;
		}
	    }
	}

	if (!(ms[worst].done.x | ms[worst].done.y)) {
	    if (++nothing_done >= 5) {
		ms[worst].crash = CrashUnknown;
		break;
	    }
	} else {
	    nothing_done = 0;
	    ms[worst].pos.x += ms[worst].done.x;
	    ms[worst].pos.y += ms[worst].done.y;
	}
	if (!(ms[worst].todo.x | ms[worst].todo.y)) {
	    break;
	}

	vel = ms[worst].vel;
	for (i = 0; i < NUM_POINTS; i++) {
	    if (i != worst) {
		ms[i].pos.x += ms[worst].done.x;
		ms[i].pos.y += ms[worst].done.y;
		ms[i].vel = vel;
		ms[i].todo = ms[worst].todo;
		ms[i].dir = ms[worst].dir;
	    }
	}
    }

    pos.x = FLOAT_TO_CLICK(pl->pos.x + pl->ship->pts[pl->dir][worst].x);
    pos.y = FLOAT_TO_CLICK(pl->pos.y + pl->ship->pts[pl->dir][worst].y);
    pl->pos.x += CLICK_TO_FLOAT(ms[worst].pos.x - pos.x);
    pl->pos.y += CLICK_TO_FLOAT(ms[worst].pos.y - pos.y);
    if (mi.edge_wrap) {
	if (pl->pos.x < 0) {
	    pl->pos.x += World.width;
	}
	else if (pl->pos.x >= World.width) {
	    pl->pos.x -= World.width;
	}
	if (pl->pos.y < 0) {
	    pl->pos.y += World.height;
	}
	else if (pl->pos.y >= World.height) {
	    pl->pos.y -= World.height;
	}
    }
    pl->vel = ms[worst].vel;
    pl->velocity = VECTOR_LENGTH(pl->vel);

    if (ms[worst].crash) {
	Player_crash(&ms[worst], worst, false);
    }
}

void Turn_player(int ind)
{
    player		*pl = Players[ind];
    int			i;
    move_info_t		mi;
    move_state_t	ms[RES];
    int			dir;
    int			old_dir = pl->dir;
    int			new_dir = MOD2((int)(pl->float_dir + 0.5f), RES);
    int			sign;
    int			crash = -1;
    int			nothing_done = 0;
    int			blocked = 0;
    position		pos;
    vector		salt;

    if (new_dir == pl->dir) {
	return;
    }
    if (BIT(pl->status, PLAYING|PAUSE|GAME_OVER|KILLED) != PLAYING) {
	pl->dir = new_dir;
	return;
    }

    if (pl->pos.x < 0 || pl->pos.x >= World.width
	|| pl->pos.y < 0 || pl->pos.y >= World.height) {
	sprintf(msg, "%s stepped out of this world.", pl->name);
	Set_message(msg);
	SET_BIT(pl->status, KILLED);
	LIMIT(pl->pos.x, 0, World.width - 1);
	LIMIT(pl->pos.y, 0, World.height - 1);
	pl->prevpos = pl->pos;
	return;
    }

    mi.player = pl;
    mi.object = (object *) pl;
    mi.edge_wrap = BIT(World.rules->mode, WRAP_PLAY);
    mi.edge_bounce = edgeBounce;
    mi.wall_bounce = true;
    mi.cannon_crashes = true;
    mi.treasure_crashes = true;
    mi.target_crashes = true;
    mi.wormhole_warps = false;

    if (new_dir > pl->dir) {
	sign = (new_dir - pl->dir <= RES + pl->dir - new_dir) ? 1 : -1;
    }
    else {
	sign = (pl->dir - new_dir <= RES + new_dir - pl->dir) ? -1 : 1;
    }

    pos = pl->pos;
    salt.x = (pl->vel.x > 0) ? 0.1f : (pl->vel.x < 0) ? -0.1f : 0;
    salt.y = (pl->vel.y > 0) ? 0.1f : (pl->vel.y < 0) ? -0.1f : 0;
    while (pl->dir != new_dir) {
	dir = MOD2(pl->dir + sign, RES);
	if (!mi.edge_wrap) {
	    if (pos.x < 15) {
		for (i = 0; i < NUM_POINTS; i++) {
		    if (pos.x + pl->ship->pts[dir][i].x < 0) {
			pos.x = -pl->ship->pts[dir][i].x;
		    }
		}
	    }
	    if (pos.x >= World.width - 16) {
		for (i = 0; i < NUM_POINTS; i++) {
		    if (pos.x + pl->ship->pts[dir][i].x
			>= World.width - 0.5f / CLICK) {
			pos.x = World.width - pl->ship->pts[dir][i].x - 1;
		    }
		}
	    }
	    if (pos.y < 15) {
		for (i = 0; i < NUM_POINTS; i++) {
		    if (pos.y + pl->ship->pts[dir][i].y < 0) {
			pos.y = -pl->ship->pts[dir][i].y;
		    }
		}
	    }
	    if (pos.y >= World.height - 16) {
		for (i = 0; i < NUM_POINTS; i++) {
		    if (pos.y + pl->ship->pts[dir][i].y
		        >= World.height - 0.5f / CLICK) {
			pos.y = World.height - 
			    pl->ship->pts[dir][i].y - 1;
		    }
		}
	    }
	}

	for (i = 0; i < NUM_POINTS; i++) {
	    ms[i].mip = &mi;
	    ms[i].pos.x = FLOAT_TO_CLICK(pl->pos.x + 
		pl->ship->pts[pl->dir][i].x);
	    ms[i].pos.y = FLOAT_TO_CLICK(pl->pos.y + 
		pl->ship->pts[pl->dir][i].y);
	    ms[i].todo.x = FLOAT_TO_CLICK(pos.x + 
		pl->ship->pts[dir][i].x) - ms[i].pos.x;
	    ms[i].todo.y = FLOAT_TO_CLICK(pos.y + 
		pl->ship->pts[dir][i].y) - ms[i].pos.y;
	    ms[i].vel.x = ms[i].todo.x + salt.x;
	    ms[i].vel.y = ms[i].todo.y + salt.y;

	    do {
		Move_segment(&ms[i]);
		if (ms[i].crash | ms[i].bounce) {
		    if (ms[i].crash) {
			if (ms[i].crash != CrashUniverse) {
			    crash = i;
			}
			blocked = 1;
			break;
		    }
		    if (ms[i].bounce != BounceEdge) {
			blocked = 1;
			break;
		    }
		    if (++nothing_done >= 5) {
			ms[i].crash = CrashUnknown;
			crash = i;
			break;
		    }
		}
		else if (ms[i].done.x | ms[i].done.y) {
		    ms[i].pos.x += ms[i].done.x;
		    ms[i].pos.y += ms[i].done.y;
		    nothing_done = 0;
		}
	    } while (ms[i].todo.x | ms[i].todo.y);
	    if (blocked) {
		break;
	    }
	}
	if (blocked) {
	    break;
	}
	pl->dir = dir;
	pl->pos = pos;
    }

    if (blocked) {
	pl->float_dir = (float) pl->dir;
    }
    if (pl->dir != old_dir) {
	if (mi.edge_wrap) {
	    if (pl->pos.x < 0) {
		pl->pos.x += World.width;
	    }
	    else if (pl->pos.x >= World.width) {
		pl->pos.x -= World.width;
	    }
	    if (pl->pos.y < 0) {
		pl->pos.y += World.height;
	    }
	    else if (pl->pos.y >= World.height) {
		pl->pos.y -= World.height;
	    }
	}
    }

    if (crash != -1) {
	Player_crash(&ms[crash], crash, true);
    }

}

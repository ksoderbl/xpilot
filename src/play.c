/* $Id: play.c,v 3.124 1995/11/30 21:48:05 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
 *
 *      Bjørn Stabell        (bjoerns@staff.cs.uit.no)
 *      Ken Ronny Schouten   (kenrsc@stud.cs.uit.no)
 *      Bert Gÿsbers         (bert@mc.bio.uva.nl)
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
#include <math.h>

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "score.h"
#include "saudio.h"
#include "bit.h"
#include "netserver.h"

char play_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: play.c,v 3.124 1995/11/30 21:48:05 bert Exp $";
#endif

#define MISSILE_POWER_SPEED_FACT	0.25
#define MISSILE_POWER_TURNSPEED_FACT	0.75
#define MINI_TORPEDO_SPREAD_TIME	6
#define MINI_TORPEDO_SPREAD_SPEED	20
#define MINI_TORPEDO_SPREAD_ANGLE	90
#define MINI_MINE_SPREAD_TIME		18
#define MINI_MINE_SPREAD_SPEED		8
#define MINI_MISSILE_SPREAD_ANGLE	45

extern int Rate(int winner, int loser);


/******************************
 * Functions for ship movement.
 */

void Thrust(int ind)
{
    player		*pl = Players[ind];
    const int		min_dir = (int)(pl->dir + RES/2 - RES*0.2 - 1);
    const int		max_dir = (int)(pl->dir + RES/2 + RES*0.2 + 1);
    const float		max_speed = 1 + (pl->power * 0.14);
    const int		max_life = 3 + (int)(pl->power * 0.35);
    static int		keep_rand;
    int			this_rand = (((keep_rand >>= 2)
					? (keep_rand)
					: (keep_rand = rand())) & 0x03);
    int			tot_sparks = (pl->power * 0.15) + this_rand + 1;
    float		x = pl->pos.x + pl->ship->engine[pl->dir].x;
    float		y = pl->pos.y + pl->ship->engine[pl->dir].y;
    int			afterburners, alt_sparks;

    sound_play_sensors(pl->pos.x, pl->pos.y, THRUST_SOUND);

    afterburners = (BIT(pl->used, OBJ_EMERGENCY_THRUST)
		    ? MAX_AFTERBURNER
		    : pl->item[ITEM_AFTERBURNER]);
    alt_sparks = afterburners
		    ? AFTER_BURN_SPARKS(tot_sparks-1, afterburners) + 1
		    : 0;

    Make_debris(
	/* pos.x, pos.y   */ x, y,
	/* vel.x, vel.y   */ pl->vel.x, pl->vel.y,
	/* owner id       */ pl->id,
	/* kind           */ OBJ_SPARK,
	/* mass           */ THRUST_MASS,
	/* status         */ GRAVITY | OWNERIMMUNE,
	/* color          */ RED,
	/* radius         */ 8,
	/* min,max debris */ tot_sparks-alt_sparks, tot_sparks-alt_sparks,
	/* min,max dir    */ min_dir, max_dir,
	/* min,max speed  */ 1, max_speed,
	/* min,max life   */ 3, max_life
	);

    Make_debris(
	/* pos.x, pos.y   */ x, y,
	/* vel.x, vel.y   */ pl->vel.x, pl->vel.y,
	/* owner id       */ pl->id,
	/* kind           */ OBJ_SPARK,
	/* mass           */ THRUST_MASS * ALT_SPARK_MASS_FACT,
	/* status         */ GRAVITY | OWNERIMMUNE,
	/* color          */ BLUE,
	/* radius         */ 8,
	/* min,max debris */ alt_sparks, alt_sparks,
	/* min,max dir    */ min_dir, max_dir,
	/* min,max speed  */ 1, max_speed,
	/* min,max life   */ 3, max_life
	);
}


#ifdef TURN_FUEL
void Turn_thrust(int ind,int num_sparks)
{
    player	*pl = Players[ind];
    int		x = pl->pos.x + pl->ship->pts[0][pl->dir].x;
    int		y = pl->pos.y + pl->ship->pts[0][pl->dir].y;
    int		dir = pl->dir + ((pl->turnacc > 0.0) ? (RES/4) : (3*(RES/4)));

    Make_debris(
	/* pos.x, pos.y   */ x, y,
	/* vel.x, vel.y   */ pl->vel.x, pl->vel.y,
	/* owner id       */ pl->id,
	/* kind           */ OBJ_SPARK,
	/* mass           */ THRUST_MASS,
	/* status         */ GRAVITY | OWNERIMMUNE,
	/* color          */ RED,
	/* radius         */ 1,
	/* min,max debris */ num_sparks, num_sparks,
	/* min,max dir    */ dir - (RES*0.1) -1, dir + (RES*0.1) + 1,
	/* min,max speed  */ 1, 3,
	/* min,max life   */ 1, 2*FPS
	);
}
#endif


/* Calculates the recoil if a ship fires a shot */
void Recoil(object *ship, object *shot)
{
    ship->vel.x -= ((tcos(shot->dir) * ABS(shot->vel.x-ship->vel.x) *
	shot->mass) / ship->mass);
    ship->vel.y -= ((tsin(shot->dir) * ABS(shot->vel.y-ship->vel.y) *
	shot->mass) / ship->mass);
}

void Record_shove(player *pl, player *pusher, long time)
{
    shove_t		*shove = &pl->shove_record[pl->shove_next];

    if (++pl->shove_next == MAX_RECORDED_SHOVES) {
	pl->shove_next = 0;
    }
    shove->pusher_id = pusher->id;
    shove->time = time;
}

/* Calculates the effect of a collision between to objects */
void Delta_mv(object *ship, object *obj)
{
    float	vx, vy, m;

    m = ship->mass + ABS(obj->mass);
    vx = (ship->vel.x * ship->mass + obj->vel.x * obj->mass) / m;
    vy = (ship->vel.y * ship->mass + obj->vel.y * obj->mass) / m;
    if (ship->type == OBJ_PLAYER
	&& obj->id != -1
	&& BIT(obj->status, COLLISIONSHOVE)) {
	player *pl = (player *)ship;
	player *pusher = Players[GetInd[obj->id]];
	if (pusher != pl) {
	    Record_shove(pl, pusher, loops);
	}
    }
    ship->vel.x = vx;
    ship->vel.y = vy;
    obj->vel.x = vx;
    obj->vel.y = vy;
}


void Obj_repel(object *obj1, object *obj2, int repel_dist)
{
    float		xd, yd,
			force, dm,
			dvx1, dvy1,
			dvx2, dvy2;
    int			obj_theta;

    xd = WRAP_DX(obj2->pos.x - obj1->pos.x);
    yd = WRAP_DY(obj2->pos.y - obj1->pos.y);
    force = (repel_dist - LENGTH(xd, yd));

    if (force <= 0)
	return;

    force = MIN(force, 10);

    obj_theta = (int)findDir(xd, yd);

    dm = obj1->mass / obj2->mass;
    dvx2 = tcos(obj_theta) * force * dm;
    dvy2 = tsin(obj_theta) * force * dm;

    dvx1 = -(tcos(obj_theta) * force / dm);
    dvy1 = -(tsin(obj_theta) * force / dm);

    if (obj1->type == OBJ_PLAYER && obj2->id != -1) {
	player *pl = (player *)obj1;
	player *pusher = Players[GetInd[obj2->id]];
	if (pusher != pl) {
	    Record_shove(pl, pusher, loops);
	}
    }

    if (obj2->type == OBJ_PLAYER && obj1->id != -1) {
	player *pl = (player *)obj2;
	player *pusher = Players[GetInd[obj1->id]];
	if (pusher != pl) {
	    Record_shove(pl, pusher, loops);
	}
    }

    obj1->vel.x += dvx1;
    obj1->vel.y += dvy1;

    obj2->vel.x += dvx2;
    obj2->vel.y += dvy2;
}

static void Item_update_flags(player *pl)
{
    if (pl->item[ITEM_CLOAK] <= 0
	&& BIT(pl->have, OBJ_CLOAKING_DEVICE)) {
	CLR_BIT(pl->have, OBJ_CLOAKING_DEVICE);
	pl->updateVisibility = 1;
    }
    if (pl->item[ITEM_AFTERBURNER] <= 0)
	CLR_BIT(pl->have, OBJ_AFTERBURNER);
    if (pl->item[ITEM_EMERGENCY_THRUST] <= 0
	&& !BIT(pl->used, OBJ_EMERGENCY_THRUST)
	&& pl->emergency_thrust_left == 0)
	CLR_BIT(pl->have, OBJ_EMERGENCY_THRUST);
    if (pl->item[ITEM_EMERGENCY_SHIELD] <= 0
	&& !BIT(pl->used, OBJ_EMERGENCY_SHIELD)
	&& pl->emergency_shield_left == 0) {
	if (BIT(pl->have, OBJ_EMERGENCY_SHIELD)) {
	    CLR_BIT(pl->have, OBJ_EMERGENCY_SHIELD);
	    if (!BIT(DEF_HAVE, OBJ_SHIELD) && pl->shield_time <= 0) {
		CLR_BIT(pl->have, OBJ_SHIELD);
		CLR_BIT(pl->used, OBJ_SHIELD);
	    }
	}
    }
    if (pl->item[ITEM_TRACTOR_BEAM] <= 0)
	CLR_BIT(pl->have, OBJ_TRACTOR_BEAM);
    if (pl->item[ITEM_AUTOPILOT] <= 0) {
	if (BIT(pl->used, OBJ_AUTOPILOT))
	    Autopilot (GetInd[pl->id], 0);
	CLR_BIT(pl->have, OBJ_AUTOPILOT);
    }
}

/*
 * Player loses some items after some event (collision, bounce).
 * The `prob' parameter gives the chance that items are lost
 * and, if they are lost, what percentage.
 */
void Item_damage(int ind, float prob)
{
    if (prob < 1.0f) {
	player		*pl = Players[ind];
	int		i;
	float		loss;

	loss = prob;
	LIMIT(loss, 0.0f, 1.0f);

	for (i = 0; i < NUM_ITEMS; i++) {
	    if (!BIT(1U << i, ITEM_BIT_FUEL|ITEM_BIT_TANK)) {
		if (pl->item[i]) {
		    float f = rfrac();
		    if (f < loss) {
			pl->item[i] = (int)(pl->item[i] * loss + 0.5f);
		    }
		}
	    }
	}

	Item_update_flags(pl);
    }
}

/***********************
 * Functions for shots.
 */

static object *objArray;

void Alloc_shots(int number)
{
    object *x = (object *)malloc(number*sizeof(object));
    int i;

    objArray = x;
    for (i=0; i<number; i++)
	Obj[i] = x++;
}


void Free_shots(void)
{
    if (objArray != NULL) {
	free(objArray);
	objArray = NULL;
    }
}


void Place_item(int item, player *pl)
{
    object		*obj;
    int			num_lose, num_per_pack,
			x, y,
			place_count,
			dir, dist,
			grav;
    float		fx, fy,
			vx, vy;
    item_concentrator_t	*con;

    if (NumObjs >= MAX_TOTAL_SHOTS) {
	if (pl && !BIT(pl->status, KILLED)) {
	    pl->item[item]--;
	}
	return;
    }

    if (pl) {
	if (BIT(pl->status, KILLED)) {
	    num_lose = pl->item[item] - World.items[item].initial;
	    if (num_lose <= 0) {
		return;
	    }
	    pl->item[item] -= num_lose;
	    num_per_pack = (int)(num_lose * dropItemOnKillProb);
	    if (num_per_pack < World.items[item].min_per_pack) {
		return;
	    }
	}
	else {
	    num_lose = pl->item[item];
	    if (num_lose <= 0) {
		return;
	    }
	    if (World.items[item].min_per_pack == World.items[item].max_per_pack) {
		num_per_pack = World.items[item].max_per_pack;
	    } else {
		num_per_pack = World.items[item].min_per_pack
			     + rand () % (1 + World.items[item].max_per_pack
					    - World.items[item].min_per_pack);
	    }
	    if (num_per_pack > num_lose) {
		num_per_pack = num_lose;
	    } else {
		num_lose = num_per_pack;
	    }
	    pl->item[item] -= num_lose;
	}
    } else {
	if (World.items[item].min_per_pack == World.items[item].max_per_pack) {
	    num_per_pack = World.items[item].max_per_pack;
	} else {
	    num_per_pack = World.items[item].min_per_pack
			 + rand () % (1 + World.items[item].max_per_pack
				        - World.items[item].min_per_pack);
	}
    }

    if (pl) {
	grav = GRAVITY;
	fx = pl->prevpos.x;
	fy = pl->prevpos.y;
	if (!BIT(pl->status, KILLED)) {
	    /*
	     * Player is dropping an item on purpose.
	     * Give the item some offset so that the
	     * player won't immediately pick it up again.
	     */
	    if (pl->vel.x >= 0)
		fx -= (BLOCK_SZ + (rand() & 0x07));
	    else
		fx += (BLOCK_SZ + (rand() & 0x07));
	    if (pl->vel.y >= 0)
		fy -= (BLOCK_SZ + (rand() & 0x07));
	    else
		fy += (BLOCK_SZ + (rand() & 0x07));
	    if (fx < 0)
		fx += World.width;
	    else if (fx >= World.width)
		fx -= World.width;
	    if (fy < 0)
		fy += World.height;
	    else if (fy >= World.height)
		fy -= World.height;
	}
	x = fx / BLOCK_SZ;
	y = fy / BLOCK_SZ;
	if (!BIT(1U << World.block[x][y], SPACE_BIT | BASE_BIT | WORMHOLE_BIT |
					  POS_GRAV_BIT | NEG_GRAV_BIT |
					  CWISE_GRAV_BIT | ACWISE_GRAV_BIT |
					  CHECK_BIT | DECOR_FILLED_BIT |
					  DECOR_LU_BIT | DECOR_LD_BIT |
					  DECOR_RU_BIT | DECOR_RD_BIT |
					  ITEM_CONCENTRATOR_BIT)) {
	    return;
	}
    } else {
	if (rfrac() < movingItemProb) {
	    grav = GRAVITY;
	} else {
	    grav = 0;
	}
	if (World.NumItemConcentrators > 0) {
	    con = &World.itemConcentrators[rand()%World.NumItemConcentrators];
	} else {
	    con = NULL;
	}
	/*
	 * This will take very long (or forever) with maps
	 * that hardly have any (or none) spaces.
	 * So bail out after a few retries.
	 */
	for (place_count = 0; ; place_count++) {
	    if (place_count >= 8) {
		return;
	    }
	    if (con) {
		dir = MOD2(rand(), RES);
		dist = rand() % ((itemConcentratorRadius * BLOCK_SZ) + 1);
		fx = con->pos.x * BLOCK_SZ + BLOCK_SZ / 2 + dist * tcos(dir);
		fy = con->pos.y * BLOCK_SZ + BLOCK_SZ / 2 + dist * tsin(dir);
		if (BIT(World.rules->mode, WRAP_PLAY)) {
		    if (fx < 0) fx += World.width;
		    if (fx >= World.width) fx -= World.width;
		    if (fy < 0) fy += World.height;
		    if (fy >= World.height) fy -= World.height;
		}
		if (fx < 0 || fx >= World.width
		    || fy < 0 || fy >= World.height) {
		    continue;
		}
		x = fx / BLOCK_SZ;
		y = fy / BLOCK_SZ;
	    }
	    else {
		fx = rand() % World.width;
		fy = rand() % World.height;
		x = fx / BLOCK_SZ;
		y = fy / BLOCK_SZ;
	    }
	    if (World.block[x][y] == SPACE) {
		break;
	    }
	}
    }
    vx = vy = 0;
    if (grav) {
	if (pl) {
	    vx += pl->vel.x;
	    vy += pl->vel.y;
	    if (!BIT(pl->status, KILLED)) {
		vx += (rand()&7)-3;
		vy += (rand()&7)-3;
	    }
	} else {
	    vx -= Gravity * World.gravity[x][y].x;
	    vy -= Gravity * World.gravity[x][y].y;
	    vx += (rand()&7)-3;
	    vy += (rand()&7)-3;
	}
    }

    obj = Obj[NumObjs++];
    obj->type = OBJ_ITEM;
    obj->info = item;
    obj->color = RED;
    obj->status = grav;
    obj->id = -1;
    obj->pos.x = fx;
    obj->pos.y = fy;
    obj->prevpos = obj->pos;
    obj->vel.x = vx;
    obj->vel.y = vy;
    obj->acc.x =
    obj->acc.y = 0.0;
    obj->mass = 10.0;
    obj->life = 1500 + (rand()&511);
    obj->count = num_per_pack;
    obj->pl_range = ITEM_SIZE/2;
    obj->pl_radius = ITEM_SIZE/2;

    World.items[item].num++;
}


void Throw_items(player *pl)
{
    int			num_items_to_throw, remain, item;

    if (!dropItemOnKillProb)
	return;

    for (item = 0; item < NUM_ITEMS; item++) {
	if (!BIT(1U << item, ITEM_BIT_FUEL | ITEM_BIT_TANK)) {
	    do {
		num_items_to_throw = pl->item[item] - World.items[item].initial;
		if (num_items_to_throw <= 0) {
		    break;
		}
		Place_item(item, pl);
		remain = pl->item[item] - World.items[item].initial;
	    } while (remain > 0 && remain < num_items_to_throw);
	}
    }

    Item_update_flags(pl);
}

/*
 * Cause some remaining mines or missiles to be launched in
 * a random direction with a small life time (ie. magazine has
 * gone off).
 */
void Detonate_items(int ind)
{
    player		*pl = Players[ind];
    int			i;
    modifiers		mods;

    if (!BIT(pl->status, KILLED))
	return;

    /*
     * These are always immune to detonation.
     */
    if ((pl->item[ITEM_MINE] -= World.items[ITEM_MINE].initial) < 0)
	pl->item[ITEM_MINE] = 0;
    if ((pl->item[ITEM_MISSILE] -= World.items[ITEM_MISSILE].initial) < 0)
	pl->item[ITEM_MISSILE] = 0;

    /*
     * Drop shields in order to launch mines and missiles.
     */
    CLR_BIT(pl->used, OBJ_SHIELD);

    /*
     * Mines are always affected by gravity and are sent in random directions
     * slowly out from the ship (velocity relative).
     */
    for (i = 0; i < pl->item[ITEM_MINE]; i++) {
	if (rfrac() < detonateItemOnKillProb) {
	    int dir = MOD2(rand(), RES);
	    float vel = ((rand() % 16) / 4.0);

	    mods = pl->mods;
	    if (BIT(mods.nuclear, NUCLEAR)
		&& pl->item[ITEM_MINE] < nukeMinMines) {
		CLR_BIT(mods.nuclear, NUCLEAR);
	    }
	    Place_general_mine(ind, GRAVITY,
			       pl->pos.x, pl->pos.y,
			       pl->vel.x + vel * tcos(dir),
			       pl->vel.y + vel * tsin(dir),
			       mods);
	}
    }
    for (i = 0; i < pl->item[ITEM_MISSILE]; i++) {
	if (rfrac() < detonateItemOnKillProb) {
	    int	type;

	    if (pl->shots >= pl->shot_max)
		break;

	    /*
	     * Missiles are random type at random players, which could
	     * mean a misfire.
	     */
	    SET_BIT(pl->lock.tagged, LOCK_PLAYER);
	    pl->lock.pl_id = Players[rand() % NumPlayers]->id;

	    switch(rand()%3) {
	    case 0:	type = OBJ_TORPEDO;	break;
	    case 1:	type = OBJ_HEAT_SHOT;	break;
	    default:	type = OBJ_SMART_SHOT;	break;
	    }

	    mods = pl->mods;
	    if (BIT(mods.nuclear, NUCLEAR)
		&& pl->item[ITEM_MISSILE] < nukeMinSmarts) {
		CLR_BIT(mods.nuclear, NUCLEAR);
	    }
	    Fire_general_shot(ind, pl->pos.x, pl->pos.y,
			      type, MOD2(rand(), RES),
			      pl->shot_speed, mods);
	}
    }
}

void Place_mine(int ind)
{
    player *pl = Players[ind];

    if (pl->item[ITEM_MINE] <= 0
	|| (BIT(pl->used, OBJ_SHIELD) && !shieldedMining))
	return;

    Place_general_mine(ind, 0,
		       pl->pos.x, pl->pos.y, 0.0, 0.0, pl->mods);
}


void Place_moving_mine(int ind)
{
    player *pl = Players[ind];

    if (pl->item[ITEM_MINE] <= 0
	|| (BIT(pl->used, OBJ_SHIELD) && !shieldedMining))
	return;

    Place_general_mine(ind, GRAVITY,
		       pl->pos.x, pl->pos.y, pl->vel.x, pl->vel.y, pl->mods);
}

void Place_general_mine(int ind, long status, float x, float y,
			float vx, float vy, modifiers mods)
{
    char		msg[MSG_LEN];
    player		*pl = (ind == -1 ? NULL : Players[ind]);
    int			used, life;
    long		drain;
    float		mass;
    int			i, minis;
    vector		mv;

    if (NumObjs + mods.mini >= MAX_TOTAL_SHOTS)
	return;
    if (BIT(World.rules->mode, WRAP_PLAY)) {
	if (x < 0) x += World.width;
	else if (x >= World.width) x -= World.width;
	if (y < 0) y += World.height;
	else if (y >= World.height) y -= World.height;
    }
    if (x < 0 || x >= World.width || y < 0 || y >= World.height) {
	return;
    }

    if (pl && BIT(pl->status, KILLED))
	life = rand() % FPS;
    else
	life = MINE_LIFETIME;

    if (!BIT(mods.warhead, CLUSTER))
	mods.velocity = 0;
    if (!mods.mini)
	mods.spread = 0;

    if (nukeMinSmarts <= 0) {
	CLR_BIT(mods.nuclear, NUCLEAR);
    }
    if (BIT(mods.nuclear, NUCLEAR)) {
	if (pl) {
	    used = (BIT(mods.nuclear, FULLNUCLEAR)
		    ? pl->item[ITEM_MINE]
		    : nukeMinMines);
	    if (pl->item[ITEM_MINE] < nukeMinMines) {
		sprintf(msg, "You need at least %d mines to %s %s!",
			nukeMinMines,
			(BIT(status, GRAVITY) ? "throw" : "drop"),
			Describe_shot (OBJ_MINE, status, mods, 0));
		Set_player_message (pl, msg);
		return;
	    }
	} else {
	    used = nukeMinMines;
	}
	mass = MINE_MASS * used * NUKE_MASS_MULT;
    } else {
	mass = MINE_MASS;
	used = 1;
    }

    if (pl) {
	drain = ED_MINE;
	if (BIT(mods.warhead, CLUSTER)) {
	    drain += CLUSTER_MASS_DRAIN(mass);
	}
	if (pl->fuel.sum < -drain) {
	    sprintf(msg, "You need at least %ld fuel units to %s %s!",
		    (-drain) >> FUEL_SCALE_BITS,
		    (BIT(status, GRAVITY) ? "throw" : "drop"),
		    Describe_shot(OBJ_MINE, status, mods, 0));
	    Set_player_message (pl, msg);
	    return;
	}
	Add_fuel(&(pl->fuel), drain);
	pl->item[ITEM_MINE] -= used;

	if (used > 1) {
	    sprintf(msg, "%s has %s %s!", pl->name,
		    (BIT(status, GRAVITY) ? "thrown" : "dropped"),
		    Describe_shot(OBJ_MINE, status, mods, 0));
	    Set_message(msg);
	    sound_play_all(NUKE_LAUNCH_SOUND);
	} else {
	    sound_play_sensors(pl->pos.x, pl->pos.y,
	      BIT(status, GRAVITY) ? DROP_MOVING_MINE_SOUND : DROP_MINE_SOUND);
	}
    }

    minis = (mods.mini + 1);
    SET_BIT(status, OWNERIMMUNE);

    for (i = 0; i < minis; i++) {
	object *mine = Obj[NumObjs++];

	mine->type = OBJ_MINE;
	mine->color = BLUE;
	mine->info = mineFuseTime;
	mine->status = status;
	mine->id = (pl ? pl->id : -1);
	mine->owner = mine->id;
	mine->pos.x = x;
	mine->pos.y = y;
	mine->prevpos = mine->pos;
	if (minis > 1) {
	    int		space = RES/minis;
	    int		dir;
	    float	spread;

	    spread = (float)((unsigned)mods.spread + 1);
	    /*
	     * Dir gives (S is ship upwards);
	     *
	     *			      o		    o   o
	     *	X2: o S	o	X3:   S		X4:   S
	     *			    o   o	    o   o
	     */
	    dir = (i * space) + space/2 + (minis-2)*(RES/2) + (pl?pl->dir:0);
	    dir += (rand() % (space/2)) - space/4;
	    dir = MOD2(dir, RES);
	    mv.x = MINI_MINE_SPREAD_SPEED * tcos(dir) / spread;
	    mv.y = MINI_MINE_SPREAD_SPEED * tsin(dir) / spread;
	    /*
	     * This causes the added initial velocity to reduce to
	     * zero over the MINI_MINE_SPREAD_TIME.
	     */
	    mine->spread_left = MINI_MINE_SPREAD_TIME;
	    mine->acc.x = -mv.x / (MINI_MINE_SPREAD_TIME+1);
	    mine->acc.y = -mv.y / (MINI_MINE_SPREAD_TIME+1);
	} else {
	    mv.x = mv.y = mine->acc.x = mine->acc.y = 0.0;
	    mine->spread_left = 0;
	}
	mine->vel = mv;
	mine->vel.x += vx * MINE_SPEED_FACT;
	mine->vel.y += vy * MINE_SPEED_FACT;
	mine->mass = mass / minis;
	mine->life = life / minis;
	mine->mods = mods;
	mine->pl_range = MINE_RANGE / minis;
	mine->pl_radius = MINE_RADIUS;
    }
}

/*
 * Up to and including 3.2.6 it was:
 *     Cause all of the given player's dropped/thrown mines to explode.
 * Since this caused a slowdown when many mines detonated it
 * is changed into:
 *     Cause the mine which is closest to a player and owned
 *     by that player to detonate.
 */
void Detonate_mines(int ind)
{
    player		*pl = Players[ind];
    int			i;
    int			closest = -1;
    float		dist;
    float		min_dist = World.hypotenuse + 1;

    for (i = 0; i < NumObjs; i++) {
	object *mine = Obj[i];

	if (! BIT(mine->type, OBJ_MINE))
	    continue;
	/*
	 * Mines which have been ECM reprogrammed should only be detonatable
	 * by the reprogrammer, not by the original mine placer:
	 */
	if (mine->id == pl->id) {
	    dist = Wrap_length(pl->pos.x - mine->pos.x, pl->pos.y - mine->pos.y);
	    if (dist < min_dist) {
		min_dist = dist;
		closest = i;
	    }
	}
    }
    if (closest != -1) {
	Obj[closest]->life = 0;
    }

    return;
}


void Cannon_fire(int ind)
{
    object *shot;
    int dir, speed;
    const int spread = (RES*0.3);
    const int spreadoffset = (spread/2);

    if (NumObjs >= MAX_TOTAL_SHOTS)
	return;
    shot = Obj[NumObjs++];
    dir = (rand()%(1+spread)) - spreadoffset - 1;	/* Tmp direction */
    speed = 14+(rand()%8);
    shot->color = WHITE;
    shot->id = -1;
    shot->pos.x = World.cannon[ind].pos.x * BLOCK_SZ+BLOCK_SZ/2;
    shot->pos.y = World.cannon[ind].pos.y * BLOCK_SZ+BLOCK_SZ/2;
    shot->prevpos = shot->pos;
    shot->status = FROMCANNON;
    if (ShotsGravity) {
	SET_BIT(shot->status, GRAVITY);
    }
    shot->acc.x = shot->acc.y = 0;
    shot->mass = 0.4;
    shot->type = OBJ_SHOT;
    shot->life = 25 + (rand()&0x17);
    shot->pl_range = 0;
    shot->pl_radius = 0;
    CLEAR_MODS(shot->mods);

    sound_play_sensors(shot->pos.x, shot->pos.y, CANNON_FIRE_SOUND);

    switch (World.cannon[ind].dir) {
    case DIR_UP:
	/* shot->pos.y -= BLOCK_SZ/6; */
	dir += DIR_UP;
	break;
    case DIR_DOWN:
	/* shot->pos.y += BLOCK_SZ/6; */
	dir += DIR_DOWN;
	break;
    case DIR_RIGHT:
	/* shot->pos.x -= BLOCK_SZ/6; */
	dir += DIR_RIGHT;
	break;
    case DIR_LEFT:
	/* shot->pos.x += BLOCK_SZ/6; */
	dir += DIR_LEFT;
	break;
    }

    shot->dir	= MOD2(dir, RES);
    shot->vel.x	= speed * tcos(shot->dir);
    shot->vel.y	= speed * tsin(shot->dir);
}


void Make_treasure_ball(int treasure)
{
    object *ball;
    treasure_t *t = &(World.treasures[treasure]);
    float	x = t->pos.x*BLOCK_SZ+(BLOCK_SZ/2),
		y = t->pos.y*BLOCK_SZ+10;

    if (t->have) {
	printf ("Failed Make_treasure_ball(treasure=%d):\n", treasure);
	printf ("\ttreasure: destroyed = %d, team = %d, have = %d\n",
		t->destroyed, t->team, t->have);
	return;
    }
    t->have = true;

    ball = Obj[NumObjs];

    ball->length = BALL_STRING_LENGTH;
    ball->life = LONG_MAX;
    ball->mass = 50;
    ball->vel.x = 0;	  	/* make the ball stuck a little */
    ball->vel.y = 0;		/* longer to the ground */
    ball->pos.x = x;
    ball->pos.y = y;
    ball->id = ball->owner = -1;
    ball->type = OBJ_BALL;
    ball->color = WHITE;
    ball->count = 0;
    ball->pl_range = BALL_RADIUS;
    ball->pl_radius = BALL_RADIUS;
    CLEAR_MODS(ball->mods);
    ball->status = RECREATE;
    ball->treasure = treasure;
    NumObjs++;
}


int Punish_team(int ind, int t_destroyed, int t_target)
{
    static char		msg[MSG_LEN];
    treasure_t		*td = &World.treasures[t_destroyed];
    treasure_t		*tt = &World.treasures[t_target];
    player		*pl = Players[ind];
    int			i;
    int			win_score = 0,lose_score = 0;
    int			win_team_members = 0, lose_team_members = 0;
    int			somebody_flag = 0;
    int			sc, por;

    Check_team_members (td->team);
    if (td->team == pl->team)
	return 0;

    if (BIT(World.rules->mode, TEAM_PLAY)) {
	for (i = 0; i < NumPlayers; i++) {
	    if (IS_TANK_IND(i)
		|| (BIT(Players[i]->status, PAUSE)
		    && Players[i]->count <= 0)
		|| (BIT(Players[i]->status, GAME_OVER)
		    && Players[i]->mychar == 'W'
		    && Players[i]->score == 0)) {
		continue;
	    }
	    if (Players[i]->team == td->team) {
		lose_score += Players[i]->score;
		lose_team_members++;
		if (BIT(Players[i]->status, GAME_OVER) == 0) {
		    somebody_flag = 1;
		}
	    }
	    else if (Players[i]->team == tt->team) {
		win_score += Players[i]->score;
		win_team_members++;
	    }
	}
    }
    if (!somebody_flag) {
	SCORE(ind, Rate(pl->score, CANNON_SCORE)/2,
	      tt->pos.x, tt->pos.y, "Treasure:");
	return 0;
    }

    sound_play_all(DESTROY_BALL_SOUND);
    sprintf(msg, " < %s's (%d) team has destroyed team %d treasure >",
	    pl->name, pl->team, td->team);
    Set_message(msg);

    td->destroyed++;
    World.teams[td->team].TreasuresLeft--;
    World.teams[tt->team].TreasuresDestroyed++;


    sc  = 3 * Rate(win_score, lose_score);
    por = (sc * lose_team_members) / (2 * win_team_members + 1);

    for (i = 0; i < NumPlayers; i++) {
	if (IS_TANK_IND(i)
	    || (BIT(Players[i]->status, PAUSE)
		&& Players[i]->count <= 0)
	    || (BIT(Players[i]->status, GAME_OVER)
		&& Players[i]->mychar == 'W'
		&& Players[i]->score == 0)) {
	    continue;
	}
	if (Players[i]->team == td->team) {
	    SCORE(i, -sc, tt->pos.x, tt->pos.y,
		  "Treasure: ");
	    if (treasureKillTeam)
		SET_BIT(Players[i]->status, KILLED);
	}
	else if (Players[i]->team == tt->team &&
		 (Players[i]->team != TEAM_NOT_SET || i == ind)) {
	    SCORE(i, (i == ind ? 3*por : 2*por), tt->pos.x, tt->pos.y,
		  "Treasure: ");
	}
    }

    if (treasureKillTeam) {
	Players[ind]->kills++;
    }

    updateScores = true;

    return 1;
}

/*
 * Describes shot of `type' which has `status' and `mods'.  If `hit' is
 * non-zero this description is part of a collision, otherwise its part
 * of a launch message.
 */
char *Describe_shot(int type, long status, modifiers mods, int hit)
{
    char		*name, *howmany = "a ", *plural = "";
    static char		msg[MSG_LEN];

    switch (type) {
    case OBJ_MINE:
	if (BIT(status, GRAVITY))
	    name = "bomb";
	else
	    name = "mine";
	break;
    case OBJ_SMART_SHOT:
	name = "smart missile";
	break;
    case OBJ_TORPEDO:
	name = "torpedo";
	break;
    case OBJ_HEAT_SHOT:
	name = "heat seaker";
	break;
    default:
	/*
	 * Cluster shots are actual debris from a cluster explosion
	 * so we describe it as "cluster debris".
	 */
	if (BIT(mods.warhead, CLUSTER)) {
	    howmany = "";
	    name = "debris";
	} else {
	    name = "shot";
	}
	break;
    }

    if (mods.mini && !hit) {
	howmany = "some ";
	plural = (type == OBJ_TORPEDO) ? "es" : "s";
    }

    sprintf (msg, "%s%s%s%s%s%s%s%s%s",
	     howmany,
	     ((mods.velocity || mods.spread || mods.power) ? "modified " : ""),
	     (mods.mini ? "mini " : ""),
	     (BIT(mods.nuclear, FULLNUCLEAR) ? "full " : ""),
	     (BIT(mods.nuclear, NUCLEAR) ? "nuclear " : ""),
	     (BIT(mods.warhead, IMPLOSION) ? "imploding " : ""),
	     (BIT(mods.warhead, CLUSTER) ? "cluster " : ""),
	     name,
	     plural);

    return msg;
}

void Fire_main_shot(int ind, int type, int dir)
{
    player *pl = Players[ind];
    float x,
	  y;

    if (pl->shots >= pl->shot_max || BIT(pl->used, OBJ_SHIELD))
	return;

    x = pl->pos.x + pl->ship->m_gun[pl->dir].x;
    y = pl->pos.y + pl->ship->m_gun[pl->dir].y;

    Fire_general_shot(ind, x, y, type, dir, pl->shot_speed, pl->mods);
}

void Fire_shot(int ind, int type, int dir)
{
    player *pl = Players[ind];

    if (pl->shots >= pl->shot_max || BIT(pl->used, OBJ_SHIELD))
	return;

    Fire_general_shot(ind, pl->pos.x, pl->pos.y,
		      type, dir, pl->shot_speed, pl->mods);
}

void Fire_left_shot(int ind, int type, int dir, int gun)
{
    player *pl = Players[ind];
    float x,
	  y;

    if (pl->shots >= pl->shot_max || BIT(pl->used, OBJ_SHIELD))
	return;

    x = pl->pos.x + pl->ship->l_gun[gun][pl->dir].x;
    y = pl->pos.y + pl->ship->l_gun[gun][pl->dir].y;

    Fire_general_shot(ind, x, y, type, dir, pl->shot_speed, pl->mods);

}

void Fire_right_shot(int ind, int type, int dir, int gun)
{
    player *pl = Players[ind];
    float x,
	  y;

    if (pl->shots >= pl->shot_max || BIT(pl->used, OBJ_SHIELD))
	return;

    x = pl->pos.x + pl->ship->r_gun[gun][pl->dir].x;
    y = pl->pos.y + pl->ship->r_gun[gun][pl->dir].y;

    Fire_general_shot(ind, x, y, type, dir, pl->shot_speed, pl->mods);

}

void Fire_left_rshot(int ind, int type, int dir, int gun)
{
    player *pl = Players[ind];
    float x,
	  y;

    if (pl->shots >= pl->shot_max || BIT(pl->used, OBJ_SHIELD))
	return;

    x = pl->pos.x + pl->ship->l_rgun[gun][pl->dir].x;
    y = pl->pos.y + pl->ship->l_rgun[gun][pl->dir].y;

    Fire_general_shot(ind, x, y, type, dir, pl->shot_speed, pl->mods);

}

void Fire_right_rshot(int ind, int type, int dir, int gun)
{
    player *pl = Players[ind];
    float x,
	  y;

    if (pl->shots >= pl->shot_max || BIT(pl->used, OBJ_SHIELD))
	return;

    x = pl->pos.x + pl->ship->r_rgun[gun][pl->dir].x;
    y = pl->pos.y + pl->ship->r_rgun[gun][pl->dir].y;

    Fire_general_shot(ind, x, y, type, dir, pl->shot_speed, pl->mods);

}

void Fire_general_shot(int ind, float x, float y, int type, int dir,
		       float speed, modifiers mods)
{
    char		msg[MSG_LEN];
    player		*pl = (ind == -1 ? NULL : Players[ind]);
    int			used, life, fuse = 0,
			lock = 0,
			status = GRAVITY,
			i, ldir, minis,
			pl_range,
			pl_radius,
			rack_no,
			racks_left,
			r,
			on_this_rack,
			side = 0,
			fired = 0;
    long		drain;
    float		mass,
			turnspeed = 0,
			max_speed = SPEED_LIMIT,
			angle,
			spread;
    vector		mv;

    if (NumObjs >= MAX_TOTAL_SHOTS)
	return;

    if (!BIT(mods.warhead, CLUSTER))
	mods.velocity = 0;
    if (!mods.mini)
	mods.spread = 0;

    if (pl) {
	mass = pl->shot_mass;
	life = pl->shot_life;
    } else {
	mass = ShotsMass;
	life = ShotsLife;
    }

    switch (type) {
    default:
	return;

    case OBJ_SHOT:
	CLEAR_MODS(mods);	/* Shots can't be modified! */
	pl_range = pl_radius = 0;
	if (pl) {
	    if (pl->fuel.sum < -ED_SHOT)
		return;
	    Add_fuel(&(pl->fuel), ED_SHOT);
	    sound_play_sensors(pl->pos.x, pl->pos.y, FIRE_SHOT_SOUND);
	    pl->shots++;
	}
	if (!ShotsGravity) {
	    CLR_BIT(status, GRAVITY);
	}
	break;

    case OBJ_TORPEDO:
    case OBJ_HEAT_SHOT:
    case OBJ_SMART_SHOT:
	/*
	 * Make sure there are enough object entries for the mini shots.
	 */
	if (NumObjs + mods.mini >= MAX_TOTAL_SHOTS)
	    return;

	if (pl && pl->item[ITEM_MISSILE] <= 0)
	    return;

	if (nukeMinSmarts <= 0) {
	    CLR_BIT(mods.nuclear, NUCLEAR);
	}
	if (BIT(mods.nuclear, NUCLEAR)) {
	    if (pl) {
		used = (BIT(mods.nuclear, FULLNUCLEAR)
			? pl->item[ITEM_MISSILE]
			: nukeMinSmarts);
		if (pl->item[ITEM_MISSILE] < nukeMinSmarts) {
		    sprintf(msg,
			    "You need at least %d missiles to fire %s!",
			    nukeMinSmarts,
			    Describe_shot (type, status, mods, 0));
		    Set_player_message (pl, msg);
		    return;
		}
	    } else {
		used = nukeMinSmarts;
	    }
	    mass = MISSILE_MASS * used * NUKE_MASS_MULT;
	    pl_range = (type == OBJ_TORPEDO) ? NUKE_RANGE : MISSILE_RANGE;
	} else {
	    mass = MISSILE_MASS;
	    used = 1;
	    pl_range = (type == OBJ_TORPEDO) ? TORPEDO_RANGE : MISSILE_RANGE;
	}
	pl_range /= mods.mini + 1;
	pl_radius = MISSILE_LEN;

	drain = used * ED_SMART_SHOT;
	if (BIT(mods.warhead, CLUSTER)) {
	    if (pl)
		drain += CLUSTER_MASS_DRAIN(mass);
	}

	if (pl && BIT(pl->status, KILLED))
	    life = rand() % FPS;
	else
	    life = MISSILE_LIFETIME;

	switch (type) {
	case OBJ_HEAT_SHOT:
#ifndef HEAT_LOCK
	    lock = -1;
#else  /* HEAT_LOCK */
	    if (pl == NULL
		|| !BIT(pl->lock.tagged, LOCK_PLAYER)
		|| ((pl->lock.distance > pl->sensor_range)
		    && BIT(World.rules->mode, LIMITED_VISIBILITY)))
		lock = -1;
	    else
		lock = pl->lock.pl_id;
#endif /* HEAT_LOCK */
	    if (pl) {
		sound_play_sensors(pl->pos.x, pl->pos.y, FIRE_HEAT_SHOT_SOUND);
	    }
	    max_speed = SMART_SHOT_MAX_SPEED * HEAT_SPEED_FACT;
	    turnspeed = SMART_TURNSPEED * HEAT_SPEED_FACT;
	    speed *= HEAT_SPEED_FACT;
	    break;

	case OBJ_SMART_SHOT:
	    if (pl == NULL
		|| !BIT(pl->lock.tagged, LOCK_PLAYER)
		|| ((pl->lock.distance > pl->sensor_range)
		    && BIT(World.rules->mode, LIMITED_VISIBILITY))
		|| !pl->visibility[GetInd[pl->lock.pl_id]].canSee)
		return;
	    lock = pl->lock.pl_id;
	    max_speed = SMART_SHOT_MAX_SPEED;
	    turnspeed = SMART_TURNSPEED;
	    break;

	case OBJ_TORPEDO:
	    lock = -1;
	    fuse = 8;
	    break;
	}

	if (pl) {
	    if (pl->fuel.sum < -drain) {
		sprintf(msg, "You need at least %ld fuel units to fire %s!",
			(-drain) >> FUEL_SCALE_BITS,
			Describe_shot(type, status, mods, 0));
		Set_player_message (pl, msg);
		return;
	    }
	    Add_fuel(&(pl->fuel), drain);
	    pl->item[ITEM_MISSILE] -= used;

	    if (used > 1) {
		sprintf(msg, "%s has launched %s!", pl->name,
			Describe_shot(type, status, mods, 0));
		Set_message(msg);
		sound_play_all(NUKE_LAUNCH_SOUND);
	    } else if (type == OBJ_SMART_SHOT) {
		sound_play_sensors(pl->pos.x, pl->pos.y, FIRE_SMART_SHOT_SOUND);
	    } else if (type == OBJ_TORPEDO) {
		sound_play_sensors(pl->pos.x, pl->pos.y, FIRE_TORPEDO_SOUND);
	    }
	}
	break;
    }

    minis = (mods.mini + 1);
    speed *= (1 + (mods.power * MISSILE_POWER_SPEED_FACT));
    max_speed *= (1 + (mods.power * MISSILE_POWER_SPEED_FACT));
    turnspeed *= (1 + (mods.power * MISSILE_POWER_TURNSPEED_FACT));
    spread = (float)((unsigned)mods.spread + 1);
    /*
     * Calculate the maxmimum time it would take to cross one ships width,
     * don't fuse the shot/missile/torpedo for the owner only until that
     * time passes.  This is a hack to stop various odd missile and shot
     * mounting points killing the player when they're firing.
     */
    fuse += (int)((2.0 * (float)SHIP_SZ) / speed + 1.0);

    /*
     * 			Missile Racks and Spread
     * 			------------------------
     *
     * 		    A short story by H. J. Thompson
     *
     * Once upon a time, back in the "good old days" of XPilot, it was
     * relatively easy thing to remember the few keys needed to fly and shoot.
     * It was the day of Sopwith Camels biplanes, albeit triangular ones,
     * doing close to-the-death machine gun combat with other triangular
     * Red Barons, the hard vacuum of space whistling silently by as only
     * something that doesn't exist could do (this was later augmented by
     * artificial aural feedback devices on certain advanced hardware).
     *
     * Eventually the weapon designers came up with "smart" missiles, and
     * another key was added to the control board, causing one missile to
     * launch straight forwards from the front of the triangular ship.
     * Soon other types of missiles were added, including "heat" seekers,
     * and fast straight travelling "torpedoes" (hark, is that the sonorous
     * ping-ping-ping of sonar equipment I hear?).
     *
     * Then one day along came a certain fellow who thought, among other
     * things, that it would be neat to fire up to four missiles with one
     * key press, just so the enemy pilot would be scared witless by the
     * sudden appearance of four missiles hot on their tail.  To make things
     * fair these "mini" missiles would have the same total damage of a
     * normal missile, but would travel at the speed of a normal missile.
     *
     * However this fellow mused that simply launching all the missiles in
     * the same direction and from the same point would cause the missiles
     * to appear on top of each other.  Thus he added code to "spread" the
     * missiles out at various angular offsets from the ship.  Indeed the
     * angular offsets could be controlled using a spread modifier, and yet
     * more keys appeared on a now crowded control desk.
     *
     * Interestingly the future would see the same fellow adding a two seater
     * variant of the standard single seater ship, allowing one person
     * to concentrate on flying the ship, while another could flick through
     * out-of-date manuals searching for the right key combinations on
     * the now huge console which would launch four full nuclear slow-cluster
     * imploding mini super speed close spread torpedoes at the currently
     * targetted enemy, and then engage emergency thrust and shields before
     * the ominous looking tri-winged dagger ship recoiled at high velocity
     * into a rocky wall half way across the other side of the universe.
     *
     * Back to our story, and this same fellow was musing at the design of
     * multiple "mini" missiles, and noted that the angle of launch would
     * also require a different launch point on the ship (actually it was
     * the same position as if the front of the ship was rotated to point in
     * the direction of missile launch, mainly because it was easier to
     * write the launch/guidance computer software that way).
     *
     * Later, some artistically (or sadistically) minded person decided that
     * triangular ships just didn't look good (even though they were very
     * spatially dynamic, cheap and easy to build), and wouldn't it be just
     * fantastic if one could have a ship shaped like a banana!  Sensibly,
     * however, he restricted missiles and guns to the normal single frontal
     * launching point.
     *
     * A few weeks later, somebody else decided how visually pleasing it
     * would be if one could design where missiles could be fired from by
     * adding "missile rack" points on the ship.  Up to four racks were
     * available, and missiles would fire from exactly these points on the
     * ship.  Since one to four missiles could be fired in one go, the
     * combinations with various ship designs were numerous (16).
     *
     * What would happen if somebody fired four missiles in one go, from a
     * ship that only had three missile racks?  How about two missiles from
     * one with four racks?  Sadly the missile launch software hadn't been
     * designed to take this sort of thing into account, and incredibly the
     * original programmer wasn't notified until after First Customer Ship
     * [sic], the launch software only slightly modified by the ship
     * designer, who didn't know the first thing about launch acceleration
     * curves or electronic owner immunity fuse timers.
     *
     * Pilots found their missiles were being fired from random points and
     * in sometimes very odd directions, occasionally even destroying the
     * ship without trace, severely annoying the ship's owners and several
     * insurance underwriters.  Not soon after several ship designers were
     * mysteriously killed in a freak "accident" involving a stray nuclear
     * cluster bomb, and the remaining ship designers became very careful
     * to place missile racks and extra gun turrets well away from the
     * ship's superstructure.
     *
     * The original programmer who invented multiple "mini" spreading
     * missiles quickly decided to revisit his code before any "accidents"
     * came his way, and spent a good few hours making sure one couldn't
     * shoot oneself in the "foot", and that missiles where launched in some
     * reasonable and sensible directions based on the position of the
     * missile racks.
     *
     * 			How It Actually Works
     *			---------------------
     *
     * The first obstacle is getting the right number of missiles fired
     * from each combination of missile rack configurations;
     *
     *
     *		Minis	1	2	3	4
     * Racks
     *	1		1	2	3	4
     *
     *	2		1/-	1/1	2/1	2/2
     *			-/1		1/2
     *
     *	3		1/-/-	1/1/-	1/1/1	2/1/1
     *			-/1/-	-/1/1		1/2/1
     *			-/-/1	1/-/1		1/1/2
     *
     *	4		1/-/-/-	1/1/-/-	1/1/1/-	1/1/1/1
     *			-/1/-/-	-/1/1/-	-/1/1/1
     *			-/-/1/-	-/-/1/1	1/-/1/1
     *			-/-/-/1 1/-/-/1	1/1/-/1
     *
     * To read; For example with 2 Minis and 3 Racks, the first round will
     * fire 1/1/-, which is one missile from left and middle racks.  The
     * next time fired will be -/1/1; middle and right, next fire is
     * 1/-/1; left and right.  Next time goes to the beggining state.
     *
     * 			Comment Point 1
     *			---------------
     *
     * The *starting* rack number for each salvo cycles through the number
     * of missiles racks.  This is stored in the player variable
     * `pl->missile_rack', and is only incremented after each salvo (not
     * after each mini missile is fired).  This value is used to initialise
     * `rack_no', which stores the current rack that missiles are fired from.
     *
     * `on_this_rack' is computed to be the number of missiles that will be
     * fired from `rack_no', and `r' is used as a counter to this value.
     *
     * `racks_left' count how many unused missiles racks are left on the ship
     * in this mini missile salvo.
     *
     * 			Comment Point 2
     *			---------------
     *
     * When `r' reaches `on_this_rack' all the missiles have been fired for
     * this rack, and the next rack should be used.  `rack_no' is incremented
     * modulo the number of available racks, and `racks_left' is decremented.
     * At this point `on_this_rack' is recomputed for the next rack, and `r'
     * reset to zero.  Thus initially these two variables are both zero, and
     * `rack_no' is one less, such that these variables can be computed inside
     * the loop to make the code simpler.
     *
     * The computation of `on_this_rack' is as follows;  Given that there
     * are M missiles and R racks remaining;
     *
     *	on_this_rack = int(M / R);	(ie. round down to lowest int)
     *
     * Then;
     *
     *	(M - on_this_rack) / (R - 1) < (M / R).
     *
     * That is, the number of missiles fired on the next rack will be
     * more precise, and trivially can be seen that when R is 1, will
     * give an exact number of missiles to fire on the last rack.
     *
     * In the code `M' is (minis - i), and `R' is racks_left.
     *
     *			Comment Point 3
     *			---------------
     *
     * In order that multiple missiles fired from one rack do not conincide,
     * each missile has to be "spread" based on the number of missiles
     * fired from this rack point.
     *
     * This is computed similar to the wide shot code;
     *
     *	angle = (N - 1 - 2 * i) / (N - 1)
     *
     * Where N is the number of shots/missiles to be fired, and i is a counter
     * from 0 .. N-1.
     *
     * 		i	0	1	2	3
     * N
     * 1		0
     * 2		1	-1
     * 3		1	0	-1
     * 4		1	0.333	-0.333	-1
     *
     * In this code `N' is `on_this_rack'.
     *
     * Also the position of the missile rack from the center line of the
     * ship (stored in `side') has a linear effect on the angle, such that
     * a point farthest from the center line contributes the largest angle;
     *
     * angle += (side / SHIP_SZ)
     *
     * Since the eventual `angle' value used in the code should be a
     * percentage of the unmodified launch angle, it should be ranged between
     * -1.00 and +1.00, and thus the first angle is reduced by 33% and the
     * second by 66%.
     *
     * Contact: harveyt@sco.com
     */

    if (pl && type != OBJ_SHOT) {
	/*
	 * Initialise missile rack spread variables. (See Comment Point 1)
	 */
	on_this_rack = 0;
	racks_left = pl->ship->num_m_rack;
	rack_no = pl->missile_rack - 1;
	if (++pl->missile_rack >= pl->ship->num_m_rack)
	    pl->missile_rack = 0;
    }

    for (r = 0, i = 0; i < minis; i++, r++) {
	object *shot = Obj[NumObjs++];

	shot->life 	= life / minis;
	shot->fuselife	= shot->life - fuse;
	shot->mass	= mass / minis;
	shot->max_speed = max_speed;
	shot->turnspeed = turnspeed;
	shot->count 	= 0;
	shot->info 	= lock;
	shot->type	= type;
	shot->id	= (pl ? pl->id : -1);
	shot->color	= (pl ? pl->color : WHITE);
	shot->prevpos	= shot->pos;
	shot->pos.x 	= x;
	shot->pos.y 	= y;

	if (pl && type != OBJ_SHOT) {
	    if (r == on_this_rack) {
		/*
		 * We've fired all the mini missiles for the current rack,
		 * we now move onto the next one. (See Comment Point 2)
		 */
		on_this_rack = (minis - i) / racks_left--;
		if (on_this_rack < 1) on_this_rack = 1;
		if (++rack_no >= pl->ship->num_m_rack)
		    rack_no = 0;
		r = 0;
	    }
	    shot->pos.x += pl->ship->m_rack[rack_no][pl->dir].x;
	    shot->pos.y += pl->ship->m_rack[rack_no][pl->dir].y;
	    side = pl->ship->m_rack[rack_no][0].y;
	}
	if (BIT(World.rules->mode, WRAP_PLAY)) {
	    if (shot->pos.x < 0) shot->pos.x += World.width;
	    else if (shot->pos.x >= World.width) shot->pos.x -= World.width;
	    if (shot->pos.y < 0) shot->pos.y += World.height;
	    else if (shot->pos.y >= World.height) shot->pos.y -= World.height;
	}
	if (shot->pos.x < 0 || shot->pos.x >= World.width
	    || shot->pos.y < 0 || shot->pos.y >= World.height) {
	    NumObjs--;
	    continue;
	}

	if (type == OBJ_SHOT || !pl) {
	    angle = 0.0;
	} else {
	    /*
	     * Calculate the percentage unmodified launch angle for missiles.
	     * (See Comment Point 3).
	     */
	    if (on_this_rack <= 1) {
		angle = 0.0;
	    } else {
		angle = (float)(on_this_rack - 1 - 2 * r);
		angle /= (3.0 * (float)(on_this_rack - 1));
	    }
	    angle += (float)(2 * side) / (float)(3 * SHIP_SZ);
	}

	/*
	 * Torpedoes spread like mines, except the launch direction
	 * is preset over the range +/- MINI_TORPEDO_SPREAD_ANGLE.
	 * (This is not modified by the spread, the initial velocity is)
	 *
	 * Other missiles are just launched in a different direction
	 * which varies over the range +/- MINI_MISSILE_SPREAD_ANGLE,
	 * which the spread modifier varies.
	 */
	switch (type) {
	case OBJ_TORPEDO:
	    angle *= (MINI_TORPEDO_SPREAD_ANGLE / 360.0) * RES;
	    ldir = MOD2(dir + (int)angle, RES);
	    mv.x = MINI_TORPEDO_SPREAD_SPEED * tcos(ldir) / spread;
	    mv.y = MINI_TORPEDO_SPREAD_SPEED * tsin(ldir) / spread;
	    /*
	     * This causes the added initial velocity to reduce to
	     * zero over the MINI_TORPEDO_SPREAD_TIME.
	     * FIX: torpedoes should have the same speed
	     *      regardless of minification.
	     */
	    shot->spread_left = MINI_TORPEDO_SPREAD_TIME;
	    shot->acc.x = -mv.x / (MINI_TORPEDO_SPREAD_TIME+1);
	    shot->acc.y = -mv.y / (MINI_TORPEDO_SPREAD_TIME+1);
	    ldir = dir;
	    break;

	default:
	    angle *= (MINI_MISSILE_SPREAD_ANGLE / 360.0) * RES / spread;
	    ldir = MOD2(dir + (int)angle, RES);
	    mv.x = mv.y = shot->acc.x = shot->acc.y = 0;
	    break;
	}

	shot->vel.x 	= mv.x + (pl ? pl->vel.x : 0.0) + tcos(ldir) * speed;
	shot->vel.y 	= mv.y + (pl ? pl->vel.y : 0.0) + tsin(ldir) * speed;
	shot->status	= status;
	shot->dir	= ldir;
	shot->mods  	= mods;
	shot->pl_range  = pl_range;
	shot->pl_radius = pl_radius;
	fired++;
    }

    /*
     * Recoil must be done instantaneously otherwise ship moves back after
     * firing each mini missile.
     */
    if (pl) {
	for (i = 1; i <= fired; i++)
	    Recoil((object *)pl, Obj[NumObjs - i]);
    }
}


void Fire_normal_shots(int ind)
{
    player		*pl = Players[ind];
    int			i, shot_angle;

    if (loops < pl->shot_time + fireRepeatRate) {
	return;
    }
    pl->shot_time = loops;

    shot_angle = MODS_SPREAD_MAX - pl->mods.spread;

    Fire_main_shot(ind, OBJ_SHOT, pl->dir);
    for (i = 0; i < pl->item[ITEM_WIDEANGLE]; i++) {
	if (pl->ship->num_l_gun > 0) {
	    Fire_left_shot(ind, OBJ_SHOT, MOD2(pl->dir + (1 + i) * shot_angle,
			   RES), i % pl->ship->num_l_gun);
	}
	else {
	    Fire_main_shot(ind, OBJ_SHOT, MOD2(pl->dir + (1 + i) * shot_angle,
			   RES));
	}
	if (pl->ship->num_r_gun > 0) {
	    Fire_right_shot(ind, OBJ_SHOT, MOD2(pl->dir - (1 + i) * shot_angle,
			    RES), i % pl->ship->num_r_gun);
	}
	else {
	    Fire_main_shot(ind, OBJ_SHOT, MOD2(pl->dir - (1 + i) * shot_angle,
			   RES));
	}
    }
    for (i = 0; i < pl->item[ITEM_REARSHOT]; i++) {
	if ((pl->item[ITEM_REARSHOT] - 1 - 2 * i) < 0) {
	    if (pl->ship->num_l_rgun > 0) {
		Fire_left_rshot(ind, OBJ_SHOT, MOD2(pl->dir + RES/2
		    + ((pl->item[ITEM_REARSHOT] - 1 - 2 * i) * shot_angle) / 2,
			RES), (i - (pl->item[ITEM_REARSHOT] + 1) / 2) % pl->ship->num_l_rgun);
	    }
	    else {
		Fire_shot(ind, OBJ_SHOT, MOD2(pl->dir + RES/2
		    + ((pl->item[ITEM_REARSHOT] - 1 - 2 * i) * shot_angle) / 2,
			RES));
	    }
	}
	if ((pl->item[ITEM_REARSHOT] - 1 - 2 * i) > 0) {
	    if (pl->ship->num_r_rgun > 0) {
		Fire_right_rshot(ind, OBJ_SHOT, MOD2(pl->dir + RES/2
		    + ((pl->item[ITEM_REARSHOT] - 1 - 2 * i) * shot_angle) / 2,
			RES), (pl->item[ITEM_REARSHOT] / 2 - i - 1) % pl->ship->num_r_rgun);
	    }
	    else {
		Fire_shot(ind, OBJ_SHOT, MOD2(pl->dir + RES/2
		    + ((pl->item[ITEM_REARSHOT] - 1 - 2 * i) * shot_angle) / 2,
			RES));
	    }
	}
	if ((pl->item[ITEM_REARSHOT] - 1 - 2 * i) == 0)
	     Fire_shot(ind, OBJ_SHOT, MOD2(pl->dir + RES/2
		+ ((pl->item[ITEM_REARSHOT] - 1 - 2 * i) * shot_angle) / 2,
			RES));
    }
}


/* Removes shot from array */
void Delete_shot(int ind)
{
    object		*shot = Obj[ind];	/* Used when swapping places */
    player		*pl;
    int			addMine = 0;
    int			addHeat = 0;
    int			addBall = 0;
    modifiers		mods;
    long		status;
    int			i;
    int			intensity;
    int			type, color;
    float		modv, speed_modv, life_modv, num_modv;
    float		mass;

    switch (shot->type) {

    case OBJ_SPARK:
    case OBJ_DEBRIS:
	break;

    case OBJ_BALL:
	if (shot->id != -1)
	    Detach_ball(GetInd[shot->id], ind);
	else {
	    /*
	     * Maybe some player is still busy trying to connect to this ball.
	     */
	    for (i = 0; i < NumPlayers; i++) {
		if (Players[i]->ball == shot) {
		    Players[i]->ball = NULL;
		}
	    }
	}
	if (shot->owner == -1) {
	    /*
	     * If the ball has never been owned, the only way it could
	     * have been destroyed is by being knocked out of the goal.
	     * Therefore we force the ball to be recreated.
	     */
	    World.treasures[shot->treasure].have = false;
	    SET_BIT(shot->status, RECREATE);
	}
	if (BIT(shot->status, RECREATE)) {
	    addBall = 1;
	    if (BIT(shot->status, NOEXPLOSION))
		break;
	    sound_play_sensors(shot->pos.x, shot->pos.y, EXPLODE_BALL_SOUND);
	    Make_debris(
		/* pos.x, pos.y   */ shot->prevpos.x, shot->prevpos.y,
		/* vel.x, vel.y   */ shot->vel.x, shot->vel.y,
		/* owner id       */ shot->id,
		/* kind           */ OBJ_DEBRIS,
		/* mass           */ DEBRIS_MASS,
		/* status         */ GRAVITY,
		/* color          */ RED,
		/* radius         */ 8,
		/* min,max debris */ 75, 150,
		/* min,max dir    */ 0, RES-1,
		/* min,max speed  */ 20, 95,
		/* min,max life   */ 10, 2*(FPS+15)
		);
	}
	break;
	/* Shots related to a player. */

    case OBJ_MINE:
    case OBJ_HEAT_SHOT:
    case OBJ_TORPEDO:
    case OBJ_SMART_SHOT:
	if (shot->mass == 0) {
	    break;
	}

	status = GRAVITY;
	if (shot->type == OBJ_MINE) {
	    status |= COLLISIONSHOVE;
	}

	if (BIT(shot->type, OBJ_MINE)) {
	    sound_play_sensors(shot->pos.x, shot->pos.y, MINE_EXPLOSION_SOUND);
	} else {
	    sound_play_sensors(shot->pos.x, shot->pos.y, OBJECT_EXPLOSION_SOUND);
	}

	if (BIT(shot->mods.nuclear, NUCLEAR)) {
	    sound_play_all(NUKE_EXPLOSION_SOUND);
	}

	if (BIT(shot->mods.warhead, CLUSTER)) {
	    type = OBJ_SHOT;
	    if (shot->id != -1) {
		player *pl = Players[GetInd[shot->id]];
		color = pl->color;
		mass = pl->shot_mass;
	    }
	    else {
		color = WHITE;
		mass = ShotsMass;
	    }
	    mass *= 3;
	    modv = 1 << shot->mods.velocity;
	    num_modv = 4;
	    if (BIT(shot->mods.nuclear, NUCLEAR)) {
		modv *= 4.0f;
		num_modv = 1;
	    }
	    life_modv = modv * 0.20f;
	    speed_modv = 1.0f / modv;
	    intensity = CLUSTER_MASS_SHOTS(shot->mass);
	} else {
	    type = OBJ_DEBRIS;
	    color = RED;
	    mass = DEBRIS_MASS;
	    modv = 1;
	    num_modv = 1;
	    life_modv = modv;
	    speed_modv = modv;
	    if (shot->type == OBJ_MINE) {
		intensity = 512;
	    } else {
		intensity = 32;
	    }
	    /*
	     * Writing it like this:
	     *   num_modv /= (shot->mods.mini + 1);
	     * triggers a bug in HP C A.09.19.
	     */
	    num_modv = num_modv / ((float)(unsigned)shot->mods.mini + 1.0f);
	}

	if (BIT(shot->mods.nuclear, NUCLEAR)) {
	    if (shot->type == OBJ_MINE)
		intensity *= NUKE_MINE_EXPL_MULT * shot->mass / MINE_MASS;
	    else
		intensity *= NUKE_SMART_EXPL_MULT * shot->mass / MISSILE_MASS;
	    intensity *= (shot->mods.mini + 1);
	    intensity /= SHOT_MULT(shot);
	}

	if (BIT(shot->mods.warhead, IMPLOSION)) {
	    intensity >>= 1;
	    mass = -mass;
	}

	if (BIT(shot->type, OBJ_TORPEDO|OBJ_HEAT_SHOT|OBJ_SMART_SHOT)) {
	    intensity /= (1 + shot->mods.power);
	}

	Make_debris(
	    /* pos.x, pos.y   */ shot->prevpos.x, shot->prevpos.y,
	    /* vel.x, vel.y   */ shot->vel.x, shot->vel.y,
	    /* owner id       */ shot->id,
	    /* kind           */ type,
	    /* mass           */ mass,
	    /* status         */ status,
	    /* color          */ color,
	    /* radius         */ 6,
	    /* min,max debris */ 0.20f * intensity * num_modv,
				 0.30f * intensity * num_modv,
	    /* min,max dir    */ 0, RES-1,
	    /* min,max speed  */ 20 * speed_modv,
				 (intensity >> 2) * speed_modv,
	    /* min,max life   */ 8 * life_modv,
				 (intensity >> 1) * life_modv
	    );
	break;

    case OBJ_SHOT:
	if (shot->id == -1
	    || BIT(shot->status, FROMCANNON)
	    || BIT(shot->mods.warhead, CLUSTER))
	    break;
	pl = Players[GetInd[shot->id]];
	if (shot->type == OBJ_SHOT) {
	    if (--pl->shots == 0) {
		pl->shots = 0;
	    }
	}
	break;

	/* Special items. */
    case OBJ_ITEM:

	switch (shot->info) {

	case ITEM_MISSILE:
	    if (shot->life == 0 && shot->color != WHITE) {
		shot->color = WHITE;
		shot->life  = FPS * WARN_TIME;
		return;
	    }
	    if (shot->life == 0 && rfrac() < rogueHeatProb) {
		addHeat = 1;
	    }
	    break;

	case ITEM_MINE:
	    if (!shot->life && shot->color != WHITE) {
		shot->color = WHITE;
		shot->life  = FPS * WARN_TIME;
		return;
	    }
	    if (shot->life == 0 && rfrac() < rogueMineProb) {
		addMine = 1;
	    }
	    break;
	}

	World.items[shot->info].num--;

	break;

    default:
	printf("Delete_shot(): Unkown shot type %d.\n", shot->type);
	break;
    }

    Obj[ind] = Obj[--NumObjs];
    Obj[NumObjs] = shot;

    if (addMine | addHeat) {
	CLEAR_MODS(mods);
	if (BIT(World.rules->mode, ALLOW_CLUSTERS) && (rand()%3) == 0) {
	    SET_BIT(mods.warhead, CLUSTER);
	}
	if (BIT(World.rules->mode, ALLOW_MODIFIERS) && (rand()%3) == 0) {
	    SET_BIT(mods.warhead, IMPLOSION);
	}
	if (BIT(World.rules->mode, ALLOW_MODIFIERS)) {
	    mods.velocity = rand() % (MODS_VELOCITY_MAX + 1);
	}
	if (BIT(World.rules->mode, ALLOW_MODIFIERS)) {
	    mods.power = rand() % (MODS_POWER_MAX + 1);
	}
	if (addMine) {
	    Place_general_mine (-1, (rand()%2 == 0 ? GRAVITY : 0),
				shot->pos.x, shot->pos.y, 0.0, 0.0, mods);
	}
	else if (addHeat) {
	    Fire_general_shot (-1, shot->pos.x, shot->pos.y, OBJ_HEAT_SHOT,
			       MOD2(rand(), RES), 1.0f, mods);
	}
    }
    else if (addBall) {
	Make_treasure_ball(shot->treasure);
    }
}


void do_transporter(player *pl)
{
    int			i;
    float		l,
			closestLength = TRANSPORTER_DISTANCE;
    player		*closestPlayer = NULL,
			*p;
    bool		done = false;
    char		msg[MSG_LEN],
			*what = 0;

    for (i = 0; i < NumPlayers; i++) {
	p = Players[i];

	if (p != pl
	    && BIT(p->status, PLAYING|PAUSE|GAME_OVER) == PLAYING
	    && !TEAM_IMMUNE(GetInd[pl->id], i)
	    && !IS_TANK_PTR(p)) {

	    l = Wrap_length(pl->pos.x - p->pos.x, pl->pos.y - p->pos.y);
	    if (l < closestLength) {
		closestLength = l;
		closestPlayer = p;
	    }
	}
    }

    if (!(p = closestPlayer)) {
	sound_play_sensors(pl->pos.x, pl->pos.y, TRANSPORTER_FAIL_SOUND);
	return;
    }

    sound_play_sensors(pl->pos.x, pl->pos.y, TRANSPORTER_SUCCESS_SOUND);
    pl->transInfo.pl_id = p->id;
    pl->transInfo.count = 5;

#define STEAL(ITEM, MSG)	\
    {				\
	if (!p->item[ITEM])	\
	    break;		\
	p->item[ITEM]--;	\
	pl->item[ITEM]++;	\
	what = MSG;		\
	done = true;		\
    }

    while (!done) {

	enum Item item = (enum Item) (rfrac() * NUM_ITEMS);

	switch (item) {

	case ITEM_AFTERBURNER:
	    STEAL(item, "an afterburner");
	    if (p->item[ITEM_AFTERBURNER] <= 0)
		CLR_BIT(p->have, OBJ_AFTERBURNER);
	    SET_BIT(pl->have, OBJ_AFTERBURNER);
	    if (pl->item[ITEM_AFTERBURNER] > MAX_AFTERBURNER)
		pl->item[ITEM_AFTERBURNER] = MAX_AFTERBURNER;
	    break;

	case ITEM_MISSILE:
	    STEAL(item, "some missiles");

	    if (p->item[ITEM_MISSILE] < 3) {
		pl->item[ITEM_MISSILE] += p->item[ITEM_MISSILE];
		p->item[ITEM_MISSILE] = 0;
	    }
	    else {
		p->item[ITEM_MISSILE] -= 3;
		pl->item[ITEM_MISSILE] += 3;
	    }
	    break;

	case ITEM_CLOAK:
	    STEAL(item, "a cloaking device");
	    p->updateVisibility = pl->updateVisibility = 1;
	    if (!p->item[ITEM_CLOAK]) {
		CLR_BIT(p->used, OBJ_CLOAKING_DEVICE);
		CLR_BIT(p->have, OBJ_CLOAKING_DEVICE);
	    }
	    SET_BIT(pl->have, OBJ_CLOAKING_DEVICE);
	    break;

	case ITEM_WIDEANGLE:
	    STEAL(item, "a wide");
	    break;

	case ITEM_REARSHOT:
	    STEAL(item, "a rear");
	    break;

	case ITEM_MINE:
	    STEAL(item, "a mine");
	    break;

	case ITEM_SENSOR:
	    STEAL(item, "a sensor");
	    p->updateVisibility = pl->updateVisibility = 1;
	    break;

	case ITEM_ECM:
	    STEAL(item, "an ECM");
	    break;

	case ITEM_TRANSPORTER:
	    STEAL(item, "a transporter");
	    break;

	case ITEM_LASER:
	    STEAL(item, "a laser");
	    if (pl->item[ITEM_LASER] > MAX_LASERS)
		pl->item[ITEM_LASER] = MAX_LASERS;
	    break;

	case ITEM_EMERGENCY_THRUST:
	    STEAL(item, "an emergency thrust");
	    if (!p->item[ITEM_EMERGENCY_THRUST]) {
		if (BIT(p->used, OBJ_EMERGENCY_THRUST))
		    Emergency_thrust(GetInd[p->id], 0);
		CLR_BIT(p->have, OBJ_EMERGENCY_THRUST);
	    }
	    SET_BIT(pl->have, OBJ_EMERGENCY_THRUST);
	    break;

	case ITEM_EMERGENCY_SHIELD:
	    STEAL(item, "an emergency shield");
	    if (!p->item[ITEM_EMERGENCY_SHIELD]) {
		if (BIT(p->used, OBJ_EMERGENCY_SHIELD))
		    Emergency_shield(GetInd[p->id], 0);
		CLR_BIT(p->have, OBJ_EMERGENCY_SHIELD);
		if (!BIT(DEF_HAVE, OBJ_SHIELD)) {
		    CLR_BIT(p->have, OBJ_SHIELD);
		    CLR_BIT(p->used, OBJ_SHIELD);
		}
	    }
	    SET_BIT(pl->have, OBJ_EMERGENCY_SHIELD);
	    break;

	case ITEM_TRACTOR_BEAM:
	    STEAL(item, "a tractor beam");
	    if (!p->item[ITEM_TRACTOR_BEAM])
		CLR_BIT(p->have, OBJ_TRACTOR_BEAM);
	    SET_BIT(pl->have, OBJ_TRACTOR_BEAM);
	    if (pl->item[ITEM_TRACTOR_BEAM] > MAX_TRACTORS)
		pl->item[ITEM_TRACTOR_BEAM] = MAX_TRACTORS;
	    break;

	case ITEM_AUTOPILOT:
	    STEAL(item, "an autopilot");
	    if (!p->item[ITEM_AUTOPILOT]) {
		if (BIT(p->used, OBJ_AUTOPILOT))
		    Autopilot(GetInd[p->id], 0);
		CLR_BIT(p->have, OBJ_AUTOPILOT);
	    }
	    SET_BIT(pl->have, OBJ_AUTOPILOT);
	    break;

	case ITEM_TANK:
	    if (p->fuel.num_tanks > 0 && pl->fuel.num_tanks < MAX_TANKS) {
		int tank_ind = (rand() % p->fuel.num_tanks) + 1;
		long tank_fuel = p->fuel.tank[tank_ind];
		Player_remove_tank(GetInd[p->id], tank_ind);
		Player_add_tank(GetInd[pl->id], tank_fuel);
		what = "a tank";
		done = true;
	    }
	    break;

	case ITEM_FUEL:	/* used to steal fuel */
#define MIN_FUEL_STEAL	10
#define MAX_FUEL_STEAL  50
	    {
		long            amount;
		float          percent;

		percent = ((rand() % (MAX_FUEL_STEAL - MIN_FUEL_STEAL) +
			    MIN_FUEL_STEAL) / 100.0);
		amount = (long)(p->fuel.sum * percent);
		sprintf(msg, "%s stole %ld units (%d%%) of fuel from %s",
			pl->name, amount >> FUEL_SCALE_BITS,
			(int) (percent * 100.0 + 0.5), p->name);
		Add_fuel(&(pl->fuel), amount);
		Add_fuel(&(p->fuel), -amount);
		Set_message(msg);
	    }
	    return;

	case NUM_ITEMS:
	    /*NOTREACHED*/
	    error("BUG: steal");
	    break;
	}
    }

    sprintf(msg, "%s stole %s from %s.", pl->name, what, p->name);
    Set_message(msg);
}

void do_lose_item(player *pl)
{
    int item = pl->lose_item;

    if (item < 0 || item >= NUM_ITEMS) {
	error("BUG: do_lose_item %d", item);
	return;
    }
    if (BIT(1U << pl->lose_item, ITEM_BIT_FUEL | ITEM_BIT_TANK)) {
	return;
    }
    if (pl->item[item] <= 0) {
	return;
    }

    if (loseItemDestroys == false) {
	Place_item(item, pl);
    }
    else {
	pl->item[item]--;
    }

    Item_update_flags(pl);
}

#define CONFUSED_UPDATE_GRANULARITY	10
#define CONFUSED_TIME			3

void do_ecm(player *pl)
{
    object		*shot;
    object		*closest_mine = NULL;
    float		closest_mine_range = World.hypotenuse;
    int			i, j, owner, ind = GetInd[pl->id];
    float		range, perim;
    player		*p;

    sound_play_sensors(pl->pos.x, pl->pos.y, ECM_SOUND);
    for (i = 0; i < NumObjs; i++) {
	shot = Obj[i];

	if (! BIT(shot->type, OBJ_SMART_SHOT|OBJ_MINE))
	    continue;
	if ((range = Wrap_length(pl->pos.x - shot->pos.x,
				 pl->pos.y - shot->pos.y)) > ECM_DISTANCE)
	    continue;

	/*
	 * Ignore mines owned by yourself which you are immune to,
	 * or missiles owned by you which are after somebody else.
	 *
	 * Ignore any object not owned by you which are owned by
	 * team members if team immunity is on.
	 */
	if (shot->id != -1) {
	    owner = GetInd[shot->id];
	    if (ind == owner) {
		if (shot->type == OBJ_MINE) {
		    if (BIT(shot->status, OWNERIMMUNE)) {
			continue;
		    }
		}
		if (shot->type == OBJ_SMART_SHOT) {
		    if (shot->info != owner) {
			continue;
		    }
		}
	    }
	    else if (TEAM_IMMUNE(ind, owner)) {
		continue;
	    }
	}


	switch (shot->type) {
	case OBJ_SMART_SHOT:
	    /*
	     * See Move_smart_shot() for re-lock probablities after confusion
	     * ends.
	     */
	    SET_BIT(shot->status, CONFUSED);
	    shot->ecm_range = range;
	    shot->count = CONFUSED_TIME;
	    if (BIT(pl->lock.tagged, LOCK_PLAYER)
		&& (pl->lock.distance <= pl->sensor_range
		    || !BIT(World.rules->mode, LIMITED_VISIBILITY))
		&& pl->visibility[GetInd[pl->lock.pl_id]].canSee)
		shot->new_info = pl->lock.pl_id;
	    else
		shot->new_info = Players[rand() % NumPlayers]->id;
	    /* Can't redirect missiles to team mates. */
	    if (TEAM_IMMUNE(ind, GetInd[shot->new_info])) {
		/* So let the missile keep on following this unlucky player. */
		shot->new_info = ind;
	    }
	    break;

	case OBJ_MINE:
	    shot->ecm_range = range;

	    /*
	     * perim is distance from the mine to its detonation perimeter
	     *
	     * range is the proportion from the mine detontation perimeter
	     * to the maximum ecm range.
	     * low values of range mean the mine is close
	     *
	     * remember the closest unconfused mine -- it gets reprogrammed
	     */
	    perim = MINE_RANGE / (shot->mods.mini+1);
	    range = (range - perim) / (ECM_DISTANCE - perim);

	    /*
	     * range%		explode%	confuse time (seconds)
	     * 100		5		2
	     *  50		10		6
	     *	 0 (closest)	15		10
	     */
	    if (range <= 0 || rand() % 100 < ((int)(10*(1-range)) + 5)) {
		shot->life = 0;
		break;
	    }
	    shot->count = ((int)(8*(1-range)) + 2) * FPS;
	    if (   !BIT(shot->status, CONFUSED)
		&& (closest_mine == NULL || range < closest_mine_range)) {
		closest_mine = shot;
		closest_mine_range = range;
	    }
	    SET_BIT(shot->status, CONFUSED);
	    if (shot->count <= 0)
		CLR_BIT(shot->status, CONFUSED);
	    break;
	}
    }

    /*
     * range%		reprogram%
     * 100		50
     *  50		75
     *	 0 (closest)	100
     */
    if (ecmsReprogramMines && closest_mine != NULL) {
	range = closest_mine_range;
	if (range <= 0 || rand() % 100 < (100 - (int)(50*range)))
	    closest_mine->id = pl->id;
    }


    for (i = 0; i < NumPlayers; i++) {
	p = Players[i];

	/*
	 * Team members are always immune from ECM effects from other
	 * team members.  Its too nasty otherwise.
	 */
	if (TEAM(ind, i))
	    continue;

	if (p != pl && BIT(p->status, PLAYING|GAME_OVER|PAUSE) == PLAYING) {
	    float damage;

	    range = Wrap_length(pl->pos.x - p->pos.x,
				pl->pos.y - p->pos.y);
	    if (range > ECM_DISTANCE)
		continue;

	    /* range is how close the player is to the center of ecm */
	    range = ((ECM_DISTANCE - range) / ECM_DISTANCE);

	    /*
	     * range%	damage (sec)	laser destroy%	reprogram%	drop%
	     * 100	4		75		100		25
	     * 50	2		50		75		15
	     * 0	0		25		50		5
	     */

	    /*
	     * should this be FPS dependant: damage = 4.0f * FPS * range; ?
	     * No, i think.
	     */
	    damage = 24.0f * range;

	    if (p->item[ITEM_CLOAK] <= 1) {
		p->forceVisible += (int)damage;
	    } else {
		p->forceVisible += (int)(damage
					 * pow(0.75, (p->item[ITEM_CLOAK]-1)));
	    }

	    /* ECM may cause balls to detach. */
	    if (BIT(p->have, OBJ_BALL)) {
		for (j = 0; j < NumObjs; j++) {
		    shot = Obj[j];
		    if (BIT(shot->type, OBJ_BALL) && shot->owner == p->id) {
			if (rand() % 100 < ((int)(20*range)+5))
			    Detach_ball(i, j);
		    }
		}
	    }

	    /* ECM damages sensitive equipment like lasers */
	    if (p->item[ITEM_LASER] > 0) {
		p->item[ITEM_LASER] = range * p->item[ITEM_LASER];
	    }

	    if (!IS_ROBOT_PTR(p)) {
		/* player is blinded by light flashes. */
		long duration = (int)(damage * pow(0.75, p->item[ITEM_SENSOR]));
		p->damaged += duration;
		Record_shove(p, pl, loops + duration);
	    } else {
		if (BIT(pl->lock.tagged, LOCK_PLAYER)
		    && (pl->lock.distance < pl->sensor_range
			|| !BIT(World.rules->mode, LIMITED_VISIBILITY))
		    && pl->visibility[GetInd[pl->lock.pl_id]].canSee
		    && pl->lock.pl_id != p->id
		    && !TEAM_IMMUNE(ind, GetInd[pl->lock.pl_id])) {

		    /*
		     * Player programs robot to seek target.
		     */
		    Robot_program(i, pl->lock.pl_id);
		    for (j = 0; j < NumPlayers; j++) {
			if (Players[j]->conn != NOT_CONNECTED) {
			    Send_seek(Players[j]->conn, pl->id,
				      p->id, pl->lock.pl_id);
			}
		    }
		}
	    }
	}
    }
}

void Fire_ecm(int ind)
{
    player		*pl = Players[ind];

    if (pl->item[ITEM_ECM] == 0
	|| pl->fuel.sum <= -ED_ECM
	|| pl->ecmInfo.count >= MAX_PLAYER_ECMS)
	return;

    SET_BIT(pl->used, OBJ_ECM);
    do_ecm(pl);
    pl->item[ITEM_ECM]--;
    Add_fuel(&(pl->fuel), ED_ECM);
}

void Move_ball(int ind)
{
#ifdef ORIGINAL_BALL

    /*
     * This is the original ball code from XPilot versions 2.0 till 3.3.1.
     * The `feature' which some people got dissatisfied with
     * is that trying to connect to a fast moving ball may result
     * in being launched with high speed into a wall.
     * Some like that feature reasoning that making everything
     * easy is boring.  Hence keeping the old code around.
     * It can be enabled by adding -DORIGINAL_BALL to the compilation flags.
     */

    object		*ball = Obj[ind];
    player		*pl = Players[ GetInd[ball->id] ];
    vector		F;
    const float		k = 10.0,
			a = 0.01,
			l = Wrap_length(pl->pos.x - ball->pos.x,
					pl->pos.y - ball->pos.y),
			c = k * (1.0 - BALL_STRING_LENGTH / l)
			    - a * ABS(ball->length - l) * (ball->length - l);

    if (l > BALL_STRING_LENGTH * 1.25
	|| l < BALL_STRING_LENGTH * 0.75) {
	Detach_ball(GetInd[ball->id], ind);
	return;
    }

    F.x = WRAP_DX(pl->pos.x - ball->pos.x) * c;
    F.y = WRAP_DY(pl->pos.y - ball->pos.y) * c;

    pl->vel.x -= F.x/pl->mass;
    pl->vel.y -= F.y/pl->mass;

    ball->vel.x += F.x/ball->mass;
    ball->vel.y += F.y/ball->mass;

    ball->length = l;

#else	/* ORIGINAL_BALL */

    /*
     * The new ball movement code since XPilot version 3.4.0 as made
     * by Bretton Wade.  The code was submitted in context diff format
     * by Mark Boyns.  Here is a an excerpt from a post in
     * rec.games.computer.xpilot by Bretton Wade dated 27 Jun 1995:
     *
     *     If I'm not mistaken (not having looked very closely at the code
     *     because I wasn't sure what it was trying to do), the original move_ball
     *     routine was trying to model a Hook's law spring, but squared the
     *     deformation term, which would lead to exagerated behavior as the spring
     *     stretched too far. Not really a divide by zero, but effectively producing
     *     large numbers.
     *
     *     When I coded up the spring myself, I found that I could recreate the
     *     effect by using a VERY strong spring. This can be defeated, however, by
     *     damping. Specifically, If you compute the critical damping factor, then
     *     you could have the cable always be the correct length. This makes me
     *     wonder how to decide when the cable snaps.
     *
     *     I chose a relatively strong spring, and a small damping factor, to make
     *     for a nice realistic bounce when you grab at the treasure. It also gives a
     *     fairley close approximation to the "normal" feel of the treasure.
     *
     *     I modeled the cable as having zero mass, or at least insignificant mass as
     *     compared to the ship and ball. This greatly simplifies the math, and leads
     *     to the conclusion that there will be no change in velocity when the cable
     *     breaks. You can check this by integrating the momentum along the cable,
     *     and the ship or ball.
     *
     *     If you assume that the cable snaps in the middle, then half of the
     *     potential energy goes to each object attached. However, as you said, the
     *     total momentum of the system cannot change. Because the weight of the
     *     cable is small, the vast majority of the potential energy will become
     *     heat. I've had two physicists verify this for me, and they both worked
     *     really hard on the problem because they found it interesting.
     *
     * End of post.
     *
     * Changes since then:
     *
     * Comment from people was that the string snaps too soon.
     * Changed the value (max_spring_ratio) at which the string snaps
     * from 0.25 to 0.30.  Not sure if that helps enough, or too much.
     */

    object		*ball = Obj[ind];
    player		*pl = Players[ GetInd[ball->id] ];
    vector		D;
    float		length, force, ratio, accell, cosine, pl_damping, ball_damping;
    const float		k = 1500.0, b = 2.0;
    const float		max_spring_ratio = 0.30;

    /* compute the normalized vector between the ball and the player */
    D.x = WRAP_DX(pl->pos.x - ball->pos.x);
    D.y = WRAP_DY(pl->pos.y - ball->pos.y);
    length = hypot(D.x, D.y);
    if (length > 0.0) {
	D.x /= length;
	D.y /= length;
    }
    else
	D.x = D.y = 0.0;

    /* compute the ratio for the spring action */
    ratio = (BALL_STRING_LENGTH - length) / (float) BALL_STRING_LENGTH;

    /* compute force by spring for this length */
    force = k * ratio;

    /* if the tether is too long or too short, release it */
    if (ABS(ratio) > max_spring_ratio) {
	Detach_ball(GetInd[ball->id], ind);
	return;
    }
    ball->length = length;

    /* compute damping for player */
    cosine = (pl->vel.x * D.x) + (pl->vel.y * D.y);
    pl_damping = -b * cosine;

    /* compute damping for ball */
    cosine = (ball->vel.x * -D.x) + (ball->vel.y * -D.y);
    ball_damping = -b * cosine;

    /* compute accelleration for player, assume t = 1 */
    accell = (force + pl_damping + ball_damping) / pl->mass;
    pl->vel.x += D.x * accell;
    pl->vel.y += D.y * accell;

    /* compute accelleration for ball, assume t = 1 */
    accell = (force + ball_damping + pl_damping) / ball->mass;
    ball->vel.x += -D.x * accell;
    ball->vel.y += -D.y * accell;

#endif	/* ORIGINAL_BALL */
}


void Move_smart_shot(int ind)
{
    object	*shot = Obj[ind];
    player	*pl;
    int		angle, theta;
    float	range = 0.0;
    float	acc;
    float	x_dif = 0.0;
    float	y_dif = 0.0;
    float	shot_speed;

    if (shot->type == OBJ_TORPEDO) {
	if (BIT(shot->mods.nuclear, NUCLEAR)) {
	    acc = (shot->info++ < NUKE_SPEED_TIME) ? NUKE_ACC : 0.0;
	} else {
	    acc = (shot->info++ < TORPEDO_SPEED_TIME) ? TORPEDO_ACC : 0.0;
	}
	acc *= (1 + (shot->mods.power * MISSILE_POWER_SPEED_FACT));
	if (shot->spread_left-- <= 0) {
	    shot->acc.x = 0;
	    shot->acc.y = 0;
	}
	shot->vel.x += acc * tcos(shot->dir);
	shot->vel.y += acc * tsin(shot->dir);
	return;
    }

    acc = SMART_SHOT_ACC;

    if (shot->type == OBJ_HEAT_SHOT) {
	acc = SMART_SHOT_ACC * HEAT_SPEED_FACT;
	if (shot->info >= 0) {
	    /* Get player and set min to distance */
	    pl = Players[ GetInd[shot->info] ];
	    range = Wrap_length(pl->pos.x-shot->pos.x, pl->pos.y-shot->pos.y);
	} else {
	    /* No player. Number of moves so that new target is searched */
	    pl = 0;
	    shot->count = HEAT_WIDE_TIMEOUT + HEAT_WIDE_ERROR;
	}
	if (pl && BIT(pl->status, THRUSTING)) {
	    /*
	     * Target is thrusting,
	     * set number to moves to correct error value
	     */
	    if (range < HEAT_CLOSE_RANGE) {
		shot->count = HEAT_CLOSE_ERROR;
	    } else if (range < HEAT_MID_RANGE) {
		shot->count = HEAT_MID_ERROR;
	    } else {
		shot->count = HEAT_WIDE_ERROR;
	    }
	} else {
	    shot->count++;
	    /* Look for new target */
	    if ((range < HEAT_CLOSE_RANGE
		 && shot->count > HEAT_CLOSE_TIMEOUT + HEAT_CLOSE_ERROR)
		|| (range < HEAT_MID_RANGE
		    && shot->count > HEAT_MID_TIMEOUT + HEAT_MID_ERROR)
		|| shot->count > HEAT_WIDE_TIMEOUT + HEAT_WIDE_ERROR) {
		float l;
		int i;

		range = HEAT_RANGE * (shot->count/HEAT_CLOSE_TIMEOUT);
		for (i=0; i<NumPlayers; i++) {
		    player *p = Players[i];

		    if (!BIT(p->status, THRUSTING))
			continue;

		    l = Wrap_length(p->pos.x - shot->pos.x,
				    p->pos.y - shot->pos.y);
		    /*
		     * After burners can be detected easier;
		     * so scale the length:
		     */
		    l *= MAX_AFTERBURNER + 1 - p->item[ITEM_AFTERBURNER];
		    l /= MAX_AFTERBURNER + 1;
		    if (BIT(p->have, OBJ_AFTERBURNER))
			l *= 16 - p->item[ITEM_AFTERBURNER];
		    if (l < range) {
			shot->info = Players[i]->id;
			range = l;
			shot->count =
			    l < HEAT_CLOSE_RANGE ?
				HEAT_CLOSE_ERROR : l < HEAT_MID_RANGE ?
				    HEAT_MID_ERROR : HEAT_WIDE_ERROR;
			pl = p;
		    }
		}
	    }
	}
	if (shot->info < 0)
	    return;
	/*
	 * Heat seekers cannot fly exactly, if target is far away or thrust
	 * isn't active.  So simulate the error:
	 */
	x_dif = (rand()&3) * shot->count;
	y_dif = (rand()&3) * shot->count;

    } else {

	if (BIT(shot->status, CONFUSED)
	    && (!(loops % CONFUSED_UPDATE_GRANULARITY)
		|| shot->count == CONFUSED_TIME)) {

	    if (shot->count) {
		shot->info = Players[rand() % NumPlayers]->id;
		shot->count--;
	    } else {
		CLR_BIT(shot->status, CONFUSED);

		/* range is percentage from center to periphery of ecm burst */
		range = (ECM_DISTANCE - shot->ecm_range) / ECM_DISTANCE;
		range *= 100.0;

		/*
		 * range%	lock%
		 * 100		100
		 *  50		75
		 *   0		50
		 */
		if (rand() % 100 <= ((int)(range/2)+50))
		    shot->info = shot->new_info;
	    }
	}
	pl = Players[GetInd[shot->info]];
    }

    /*
     * Use a little look ahead to fly more exact
     */
    acc *= (1 + (shot->mods.power * MISSILE_POWER_SPEED_FACT));
    if ((shot_speed = VECTOR_LENGTH(shot->vel)) < 1) shot_speed = 1;
    range = Wrap_length(pl->pos.x - shot->pos.x, pl->pos.y - shot->pos.y);
    x_dif += pl->vel.x * (range / shot_speed);
    y_dif += pl->vel.y * (range / shot_speed);
    theta = Wrap_findDir(pl->pos.x + x_dif - shot->pos.x,
			 pl->pos.y + y_dif - shot->pos.y);

    {
	float x, y, vx, vy;
	int i, xi, yi, j, freemax, k, foundw;
	static struct {
	    int dx, dy;
	} sur[8] = {
	    {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1}
	};

#define BLOCK_PARTS 2
	vx = shot->vel.x;
	vy = shot->vel.y;
	x = shot_speed / (BLOCK_SZ*BLOCK_PARTS);
	vx /= x; vy /= x;
	x = shot->pos.x; y = shot->pos.y;
	foundw = 0;

	for (i = SMART_SHOT_LOOK_AH; i > 0 && foundw == 0; i--) {
	    xi = (x += vx) / BLOCK_SZ;
	    yi = (y += vy) / BLOCK_SZ;
	    if (BIT(World.rules->mode, WRAP_PLAY)) {
		if (xi < 0) xi += World.x;
		else if (xi >= World.x) xi -= World.x;
		if (yi < 0) yi += World.y;
		else if (yi >= World.y) yi -= World.y;
	    }
	    if (xi < 0 || xi >= World.x || yi < 0 || yi >= World.y)
		break;

	    switch(World.block[xi][yi]) {
	    case TARGET:
	    case TREASURE:
	    case FUEL:
	    case FILLED:
	    case REC_LU:
	    case REC_RU:
	    case REC_LD:
	    case REC_RD:
	    case CANNON:
		if (range > (SMART_SHOT_LOOK_AH-i)*(BLOCK_SZ/BLOCK_PARTS)) {
		    if (shot_speed > SMART_SHOT_MIN_SPEED)
			shot_speed -= acc * (SMART_SHOT_DECFACT+1);
		}
		foundw = 1;
	    }
	}

	i = ((int)(shot->dir * 8 / RES)&7) + 8;
	xi = shot->pos.x / BLOCK_SZ;
	yi = shot->pos.y /BLOCK_SZ;

	for (j=2, angle=-1, freemax=0; j>=-2; --j) {
	    int si, xt, yt;

	    for (si=1, k=0; si >= -1; --si) {
		xt = xi + sur[(i+j+si)&7].dx;
		yt = yi + sur[(i+j+si)&7].dy;

		if (xt >= 0 && xt < World.x && yt >= 0 && yt < World.y)
		    switch (World.block[xt][yt]) {
		    case TARGET:
		    case TREASURE:
		    case FUEL:
		    case FILLED:
		    case REC_LU:
		    case REC_RU:
		    case REC_LD:
		    case REC_RD:
		    case CANNON:
			if (!si)
			    k = -32;
			break;
		    default:
			++k;
			break;
		    }
	    }
	    if (k > freemax
		|| (k == freemax
		    && ((j == -1 && (rand()&1)) || j == 0 || j == 1))) {
		freemax = k > 2 ? 2 : k;
		angle = i + j;
	    }

	    if (k == 3 && !j) {
		angle = -1;
		break;
	    }
	}

	if (angle >= 0) {
	    i = angle&7;
	    theta = Wrap_findDir((yi + sur[i].dy) * BLOCK_SZ
			    - (shot->pos.y + 2 * shot->vel.y),
			    (xi + sur[i].dx) * BLOCK_SZ
			    - (shot->pos.x - 2 * shot->vel.x));
#ifdef SHOT_EXTRA_SLOWDOWN
	    if (!foundw && range > (SHOT_LOOK_AH-i) * BLOCK_SZ) {
		if (shot_speed
		    > (SMART_SHOT_MIN_SPEED + SMART_SHOT_MAX_SPEED)/2)
		    shot_speed -= SMART_SHOT_DECC+SMART_SHOT_ACC;
	    }
#endif
	}
    }
    angle = theta;

    if (angle < 0)
	angle += RES;
    angle %= RES;

    if (angle < shot->dir)
	angle += RES;
    angle = angle - shot->dir - RES/2;

    if (angle < 0)
	shot->dir += ((-angle < shot->turnspeed) ? -angle : shot->turnspeed);
    else
	shot->dir -= ((angle < shot->turnspeed) ? angle : shot->turnspeed);

    shot->dir = MOD2(shot->dir, RES); /* NOTE!!!! */

    if (shot_speed < shot->max_speed)
	shot_speed += acc;

    /*  shot->velocity = MIN(shot->velocity, shot->max_speed);  */

    shot->vel.x = tcos(shot->dir) * shot_speed;
    shot->vel.y = tsin(shot->dir) * shot_speed;
}

void Move_mine(int ind)
{
    object	*shot = Obj[ind];

    if (BIT(shot->status, CONFUSED) && --shot->count <= 0)
	CLR_BIT(shot->status, CONFUSED);

    if (BIT(shot->status, OWNERIMMUNE) && shot->info) {
	if (--shot->info <= 0)
	    CLR_BIT(shot->status, OWNERIMMUNE);
    }

    if (shot->mods.mini && shot->spread_left-- <= 0) {
	shot->acc.x = 0;
	shot->acc.y = 0;
    }
}

/*
 * Add fuel to fighter's tanks.
 * Maybe use more than one of tank to store the fuel.
 */
void Add_fuel(pl_fuel_t *ft, long fuel)
{
    if (ft->sum + fuel > ft->max)
	fuel = ft->max - ft->sum;
    else if (ft->sum + fuel < 0)
	fuel = -ft->sum;
    ft->sum += fuel;
    ft->tank[ft->current] += fuel;
}


/*
 * Move fuel from add-on tanks to main tank,
 * handle over and underflow of tanks.
 */
void Update_tanks(pl_fuel_t *ft)
{
    if (ft->num_tanks) {
	int  t, check;
	long low_level;
	long fuel;
	long *f;

	/* Set low_level to minimum fuel in each tank */
	low_level = ft->sum / (ft->num_tanks + 1) - 1;
	if (low_level < 0)
	    low_level = 0;
	if (TANK_REFILL_LIMIT < low_level)
	    low_level = TANK_REFILL_LIMIT;

	t = ft->num_tanks;
	check = MAX_TANKS<<2;
	fuel = 0;
	f = ft->tank + t;

	while (t>=0 && check--) {
	    long m = TANK_CAP(t);

	    /* Add the previous over/underflow and do a new cut */
	    *f += fuel;
	    if (*f > m) {
		fuel = *f - m;
		*f = m;
	    } else if (*f < 0) {
		fuel = *f;
		*f = 0;
	    } else
		fuel = 0;

	    /* If there is no over/underflow, let the fuel run to main-tank */
	    if (!fuel) {
		if (t
		    && t != ft->current
		    && *f >= low_level + REFUEL_RATE
		    && *(f-1) <= TANK_CAP(t-1) - REFUEL_RATE) {

		    *f -= REFUEL_RATE;
		    fuel = REFUEL_RATE;
		} else if (t && *f < low_level) {
		    *f += REFUEL_RATE;
		    fuel = -REFUEL_RATE;
		}
	    }
	    if (fuel && t == 0) {
	       t = ft->num_tanks;
	       f = ft->tank + t;
	    } else {
		t--;
		f--;
	    }
	}
	if (!check) {
	    error("fuel problem");
	    fuel = ft->sum;
	    ft->sum =
	    ft->max = 0;
	    t = 0;
	    while (t <= ft->num_tanks) {
		if (fuel) {
		    if (fuel>TANK_CAP(t)) {
			ft->tank[t] = TANK_CAP(t);
			fuel -= TANK_CAP(t);
		    } else {
			ft->tank[t] = fuel;
			fuel = 0;
		    }
		    ft->sum += ft->tank[t];
		} else
		    ft->tank[t] = 0;
		ft->max += TANK_CAP(t);
		t++;
	    }
	}
    } else
	ft->tank[0] = ft->sum;
}


/*
 * Use current tank as dummy target for heat seeking missles.
 */
void Tank_handle_detach(player *pl)
{
    player		*dummy;
    int			i, ct;
    static char		tank_shape[] =
			    "(NM:fueltank)(AU:John E. Norlin)"
			    "(SH: 15,0 14,-5 9,-8 -5,-8 -3,-8 -3,0 "
			    "2,0 2,2 -3,2 -3,6 5,6 5,8 -5,8 -5,-8 "
			    "-9,-8 -14,-5 -15,0 -14,5 -9,8 9,8 14,5)"
			    "(EN: -15,0)(MG: 15,0)";

    /* Return, if no more players or no tanks */
    if (pl->fuel.num_tanks == 0
	|| NumPseudoPlayers == MAX_PSEUDO_PLAYERS
	|| Id >= MAX_ID) {
	return;
    }

    sound_play_sensors(pl->pos.x, pl->pos.y, TANK_DETACH_SOUND);

    /* If current tank is main, use another one */
    if ((ct = pl->fuel.current) == 0)
	ct = pl->fuel.num_tanks;

    Update_tanks(&(pl->fuel));
    /* Fork the current player */
    dummy               = Players[NumPlayers];
    /*
     * MWAAH: this was ... naieve at least:
     * *dummy              = *pl;
     * Player structures contain pointers to dynamic memory...
     */

    Init_player(NumPlayers, (allowShipShapes)
			    ? Parse_shape_str(tank_shape)
			    : NULL);
    /* Released tanks don't have tanks... */
    while (dummy->fuel.num_tanks > 0) {
	Player_remove_tank(NumPlayers, dummy->fuel.num_tanks);
    }
    SET_BIT(dummy->type_ext, OBJ_EXT_TANK);
    dummy->prevpos	= pl->prevpos;
    dummy->pos		= pl->pos;
    dummy->vel		= pl->vel;
    dummy->acc		= pl->acc;
    dummy->dir		= pl->dir;
    dummy->turnspeed	= pl->turnspeed;
    dummy->velocity	= pl->velocity;
    dummy->float_dir	= pl->float_dir;
    dummy->turnresistance = pl->turnresistance;
    dummy->turnvel	= pl->turnvel;
#ifdef TURN_FUEL
    dummy->oldturnvel	= pl->oldturnvel;
#endif
    dummy->turnacc	= pl->turnacc;
    dummy->power	= pl->power;

    strcpy(dummy->name, pl->name);
    strcat(dummy->name, "'s tank");
    strcpy(dummy->realname, "tank");
    strcpy(dummy->hostname, "tanks.org");
    dummy->home_base	= pl->home_base;
    dummy->team		= pl->team;
    dummy->pseudo_team	= pl->pseudo_team;
    dummy->mychar       = 'T';
    dummy->score	= pl->score - 500; /* It'll hurt to be hit by this */
    updateScores	= true;
    dummy->count	= -1;		/* Don't commit suicide :) */
    dummy->conn		= NOT_CONNECTED;
    dummy->audio	= NULL;

    /* Fuel is the one from choosen tank */
    dummy->fuel.sum     =
    dummy->fuel.tank[0] = pl->fuel.tank[ct];
    dummy->fuel.max     = TANK_CAP(ct);
    dummy->fuel.current = 0;
    dummy->fuel.num_tanks = 0;

    /* Init items with initialItems to have Throw_items() be useful. */
    for (i = 0; i < NUM_ITEMS; i++) {
	if (!BIT(1U << i, ITEM_BIT_FUEL | ITEM_BIT_TANK)) {
	    dummy->item[i] = World.items[i].initial;
	}
    }
    dummy->lose_item		= 0;
    dummy->lose_item_state	= 0;

    /* No lasers */
    dummy->num_pulses = 0;
    dummy->max_pulses = 0;
    dummy->pulses = NULL;

    /* Mass is only tank + fuel */
    dummy->mass = (dummy->emptymass = ShipMass) + FUEL_MASS(dummy->fuel.sum);
    dummy->have = DEF_HAVE;
    dummy->used = DEF_USED;
    dummy->power *= TANK_THRUST_FACT;

    /* Reset visibility. */
    dummy->updateVisibility = 1;
    for (i = 0; i <= NumPlayers; i++) {
	dummy->visibility[i].lastChange = 0;
	Players[i]->visibility[NumPlayers].lastChange = 0;
    }

    /* Remember whose tank this is */
    dummy->lock.pl_id = pl->id;

    NumPlayers++;
    NumPseudoPlayers++;
    Id++;
    updateScores = true;

    /* The tank uses shield and thrust */
    dummy->status = (DEF_BITS & ~KILL_BITS) | PLAYING | GRAVITY | THRUSTING;
    dummy->have = DEF_HAVE;
    dummy->used = (DEF_USED & ~USED_KILL & pl->have) | OBJ_SHIELD;
    if (playerShielding == 0) {
	dummy->shield_time = 30 * FPS;
	dummy->have |= OBJ_SHIELD;
    }

    /* Maybe heat-seekers to retarget? */
    for (i=0; i < NumObjs; i++)
	if (Obj[i]->type == OBJ_HEAT_SHOT
	    && Obj[i]->info > 0
	    && Players[ GetInd[Obj[i]->info] ] == pl)
	    Obj[i]->info = NumPlayers - 1;

    /* Remove tank, fuel and mass from myself */
    Player_remove_tank(GetInd[pl->id], ct);

    for (i = 0; i < NumPlayers - 1; i++) {
	if (Players[i]->conn != NOT_CONNECTED) {
	    Send_player(Players[i]->conn, dummy->id);
	    Send_score(Players[i]->conn, dummy->id, dummy->score, dummy->life,
		       dummy->mychar);
	}
    }
}


/****************************
 * Functions for explosions.
 */

/* Create debris particles */
void Make_debris(
    /* pos.x, pos.y   */ float  x,          float y,
    /* vel.x, vel.y   */ float  velx,       float vely,
    /* owner id       */ int    id,
    /* type           */ int    type,
    /* mass           */ float  mass,
    /* status         */ long   status,
    /* color          */ int    color,
    /* radius         */ int    radius,
    /* min,max debris */ int    min_debris, int    max_debris,
    /* min,max dir    */ int    min_dir,    int    max_dir,
    /* min,max speed  */ float  min_speed,  float  max_speed,
    /* min,max life   */ int    min_life,   int    max_life
)
{
    object		*debris;
    int			i, num_debris, life, fuse;
    modifiers		mods;

    if (BIT(World.rules->mode, WRAP_PLAY)) {
	if (x < 0) x += World.width;
	else if (x >= World.width) x -= World.width;
	if (y < 0) y += World.height;
	else if (y >= World.height) y -= World.height;
    }
    if (x < 0 || x >= World.width || y < 0 || y >= World.height) {
	return;
    }
    if (min_life > ShotsLife && ShotsLife >= FPS)
	min_life = ShotsLife;
    if (max_life > ShotsLife && ShotsLife >= FPS)
	max_life = ShotsLife;
    if (max_life < min_life)
	max_life = min_life;
    if (min_speed * max_life > World.hypotenuse)
	min_speed = World.hypotenuse / max_life;
    if (max_speed * min_life > World.hypotenuse)
	max_speed = World.hypotenuse / min_life;
    if (max_speed < min_speed)
	max_speed = min_speed;

    fuse = 0;

    CLEAR_MODS(mods);

    if (type == OBJ_SHOT) {
	SET_BIT(mods.warhead, CLUSTER);
	if (!ShotsGravity) {
	    CLR_BIT(status, GRAVITY);
	}
    }

    num_debris = min_debris + (int)(rfrac() * (max_debris - min_debris));
    if (num_debris > MAX_TOTAL_SHOTS - NumObjs) {
	num_debris = MAX_TOTAL_SHOTS - NumObjs;
    }
    for (i = 0; i < num_debris; i++, NumObjs++) {
	float		dx, dy, diroff, speed;
	int		dir, dirplus;

	debris = Obj[NumObjs];
	debris->color = color;
	debris->id = id;
	debris->pos.x = x;
	debris->pos.y = y;
	debris->prevpos = debris->pos;
	dir = MOD2(min_dir + (int)(rfrac() * (max_dir - min_dir)), RES);
	dirplus = MOD2(dir + 1, RES);
	diroff = rfrac();
	dx = tcos(dir) + (tcos(dirplus) - tcos(dir)) * diroff;
	dy = tsin(dir) + (tsin(dirplus) - tsin(dir)) * diroff;
	speed = min_speed + rfrac() * (max_speed - min_speed);
	debris->vel.x = velx + dx * speed;
	debris->vel.y = vely + dy * speed;
	debris->acc.x = 0;
	debris->acc.y = 0;
	debris->dir = dir;
	debris->mass = mass;
	debris->type = type;
	life = min_life + rfrac() * (max_life - min_life);
	debris->life = life;
	if (debris->life * speed > World.hypotenuse) {
	    debris->life = World.hypotenuse / speed;
	    life = min_life;
	}
	else if (++life > max_life) {
	    life = min_life;
	}
	debris->fuselife = debris->life - fuse;
	debris->spread_left = 0;
	debris->pl_range = radius;
	debris->pl_radius = radius;
	debris->status = status;
	debris->mods = mods;
    }
}

/* Explode a fighter */
void Explode_fighter(int ind)
{
    player *pl = Players[ind];
    int min_debris, max_debris;

    sound_play_sensors(pl->pos.x, pl->pos.y, PLAYER_EXPLOSION_SOUND);

    min_debris = (int)(1 + (pl->fuel.sum / (8.0 * FUEL_SCALE_FACT)));
    max_debris = (int)(min_debris + (pl->mass * 2.0));

    Make_debris(
	/* pos.x, pos.y   */ pl->pos.x, pl->pos.y,
	/* vel.x, vel.y   */ pl->vel.x, pl->vel.y,
	/* owner id       */ pl->id,
	/* kind           */ OBJ_DEBRIS,
	/* mass           */ 3.5,
	/* status         */ GRAVITY,
	/* color          */ RED,
	/* radius         */ 8,
	/* min,max debris */ min_debris, max_debris,
	/* min,max dir    */ 0, RES-1,
	/* min,max speed  */ 20, 20 + (((int)(pl->mass))>>1),
	/* min,max life   */ 5, 5 + (pl->mass * 1.5)
	);
}

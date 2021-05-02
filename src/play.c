/* $Id: play.c,v 3.71 1994/05/25 07:38:18 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-94 by
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
#include "global.h"
#include "draw.h"
#include "score.h"
#include "robot.h"
#include "saudio.h"
#include "bit.h"
#include "netserver.h"
#include "proto.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: play.c,v 3.71 1994/05/25 07:38:18 bert Exp $";
#endif

#define MISSILE_POWER_SPEED_FACT	0.25
#define MISSILE_POWER_TURNSPEED_FACT	0.75
#define MINI_TORPEDO_SPREAD_TIME	6
#define MINI_TORPEDO_SPREAD_SPEED	20
#define MINI_TORPEDO_SPREAD_ANGLE	90
#define MINI_MINE_SPREAD_TIME		18
#define MINI_MINE_SPREAD_SPEED		8
#define MINI_MISSILE_SPREAD_ANGLE	45

#define TREASURE_SCORE			25

/******************************
 * Functions for ship movement.
 */

void Thrust(int ind)
{
    player *pl = Players[ind];
    object *spark;
    int dir, num_sparks, the_color, alt_thrust, spread, afterburners;
    float the_mass;
    const int max_spread = 1 + RES*0.4;
    const int half_spread = max_spread / 2;
    const int spreadoffset = pl->dir + RES/2 - half_spread;
    const int max_speed = 1 + (int)(pl->power * 0.14);
    const int max_life = 1 + (int)(pl->power * 0.35);
    float x, y, speed;

    sound_play_sensors(pl->pos.x, pl->pos.y, THRUST_SOUND);
    num_sparks = (pl->power*0.3) + (rand()%3) + 2;
    afterburners = (BIT(pl->used, OBJ_EMERGENCY_THRUST) ? MAX_AFTERBURNER
		    : pl->afterburners);
    alt_thrust = (afterburners
		  ? AFTER_BURN_SPARKS(num_sparks-1, afterburners) + 1 : -1);
    the_color = RED;
    the_mass = THRUST_MASS;
    
    x = pl->pos.x + pl->ship->engine[pl->dir].x;
    y = pl->pos.y + pl->ship->engine[pl->dir].y;
    if (BIT(World.rules->mode, WRAP_PLAY)) {
	if (x < 0) x += World.width;
	else if (x >= World.width) x -= World.width;
	if (y < 0) y += World.height;
	else if (y >= World.height) y -= World.height;
    }
    if (x < 0 || x >= World.width || y < 0 || y >= World.height) {
	return;
    }

    if (num_sparks > MAX_TOTAL_SHOTS - NumObjs) {
	num_sparks = MAX_TOTAL_SHOTS - NumObjs;
    }
    for (;num_sparks; NumObjs++) {
	spread		= rand() % max_spread;
	dir		= MOD2(spreadoffset + spread, RES);
	speed		= 1 + rand() % max_speed;
	spark		= Obj[NumObjs];
	spark->dir	= dir;
	spark->vel.x	= pl->vel.x + tcos(dir) * speed;
	spark->vel.y	= pl->vel.y + tsin(dir) * speed;
	spark->pos.x	= x;
	spark->pos.y	= y;
	spark->prevpos  = spark->pos;
	spark->status	= GRAVITY|OWNERIMMUNE;
	spark->acc.x	= spark->acc.y = 0.0;
	spark->mass	= the_mass;
	spark->type	= OBJ_SPARK;
	spark->life	= 3 + (rand() % max_life);
	spark->color	= the_color;
	spark->id	= pl->id;
	spark->pl_range = 0;
	spark->pl_radius = 0;
        if (--num_sparks==alt_thrust) {
            the_color = BLUE;
            the_mass = THRUST_MASS*ALT_SPARK_MASS_FACT;
        }
    }
}


#ifdef TURN_FUEL
void Turn_thrust(int ind,int num_sparks)
{
    player *pl = Players[ind];
    object *spark;
    int i, dir;
    const int spread = (RES*0.2);
    const int spreadoffset = (spread/2);
    int x, y;
    int rate = ABS(Players[ind]->turnacc);

    x = pl->pos.x + pl->ship->pts[0][pl->dir].x;
    y = pl->pos.y + pl->ship->pts[0][pl->dir].y;
    if (BIT(World.rules->mode, WRAP_PLAY)) {
	if (x < 0) x += World.width;
	else if (x >= World.width) x -= World.width;
	if (y < 0) y += World.height;
	else if (y >= World.height) y -= World.height;
    }
    if (x < 0 || x >= World.width || y < 0 || y >= World.height) {
	return;
    }

    for (i=0; i<num_sparks && NumObjs<MAX_TOTAL_SHOTS; i++, NumObjs++) {
	spark = Obj[NumObjs];
	dir = pl->dir + (RES/4) + (rand()%(1+spread)) - spreadoffset - 1;

	if (pl->turnacc > 0.0)
	    dir = dir + RES/2;

	spark->color	= RED;
	spark->id	= pl->id;
	spark->pos.x	= x;
	spark->pos.y	= y;
	spark->prevpos  = spark->pos;
	spark->vel.x	= pl->vel.x + (tcos(dir) * (rand()&3));
	spark->vel.y	= pl->vel.y + (tsin(dir) * (rand()&3));
	spark->status	= GRAVITY|OWNERIMMUNE;
	spark->acc.x	= spark->acc.y = 0;
	spark->dir	= MOD2(spark->dir, RES);
	spark->mass	= THRUST_MASS;
	spark->type	= OBJ_SPARK;
	spark->life	= 1 + (rand()%(2*FPS));
	spark->pl_range = 0;
	spark->pl_radius = 0;
    }
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


/* Calculates the effect of a collision between to objects */
void Delta_mv(object *ship, object *obj)
{
#if OLD_DELTA_MV
    float	dvx, dvy;
    int		ship_theta, obj_theta;
    float	dm;


    ship_theta = findDir(ship->vel.x, ship->vel.y);
    obj_theta = findDir(obj->vel.x, obj->vel.y);
    if (obj_theta == 0 && obj->vel.x == 0.0) {
	if (ship_theta == 0 && ship->vel.x == 0.0) {
	    return;
	}
	obj_theta = MOD2(ship_theta + RES/2, RES);
    }
    else if (ship_theta == 0 && ship->vel.x == 0.0) {
	ship_theta = MOD2(obj_theta + RES/2, RES);
    }

    dm = obj->mass/ship->mass;
    dvx = ABS(obj->vel.x - ship->vel.x);
    dvy = ABS(obj->vel.y - ship->vel.y);
    if (LENGTH(dvx, dvy) > SPEED_LIMIT) {
	int theta = findDir(dvx, dvy);
	dvx = tcos(theta) * SPEED_LIMIT;
	dvy = tsin(theta) * SPEED_LIMIT;
    }
    ship->vel.x += tcos(obj_theta) * dvx * dm;
    ship->vel.y += tsin(obj_theta) * dvy * dm;

    obj->vel.x += tcos(ship_theta) * dvx / dm;
    obj->vel.y += tsin(ship_theta) * dvy / dm;
#else
    float	vx, vy, m;

    m = ship->mass + ABS(obj->mass);
    vx = (ship->vel.x * ship->mass + obj->vel.x * obj->mass) / m;
    vy = (ship->vel.y * ship->mass + obj->vel.y * obj->mass) / m;
    ship->vel.x = vx;
    ship->vel.y = vy;
    obj->vel.x = vx;
    obj->vel.y = vy;
#endif
}


void Obj_repel(object *obj1, object *obj2, int repel_dist)
{
    float xd, yd, force, dm;
    int obj_theta;
 
    xd = WRAP_DX(obj2->pos.x - obj1->pos.x);
    yd = WRAP_DY(obj2->pos.y - obj1->pos.y);
    force = (repel_dist - LENGTH(xd, yd));
 
    if (force <= 0)
	return;
 
    force = MIN(force, 10);
 
    obj_theta = (int)findDir(xd, yd);
 
    dm = obj1->mass/obj2->mass;
    obj2->vel.x += tcos(obj_theta) * force * dm;
    obj2->vel.y += tsin(obj_theta) * force * dm;

    obj1->vel.x -= tcos(obj_theta) * force / dm;
    obj1->vel.y -= tsin(obj_theta) * force / dm;
}

static void Item_update_flags(player *pl)
{
    if (pl->back_shots <= 0)
	CLR_BIT(pl->have, OBJ_BACK_SHOT);
    if (pl->cloaks <= 0) {
	CLR_BIT(pl->have, OBJ_CLOAKING_DEVICE);
	pl->updateVisibility = 1;
    }
    if (pl->afterburners <= 0)
	CLR_BIT(pl->have, OBJ_AFTERBURNER);
    if (pl->emergency_thrusts <= 0) {
	if (BIT(pl->used, OBJ_EMERGENCY_THRUST))
	    Emergency_thrust (GetInd[pl->id], 0);
	CLR_BIT(pl->have, OBJ_EMERGENCY_THRUST);
    }
    if (pl->emergency_shields <= 0) {
	if (BIT(pl->used, OBJ_EMERGENCY_SHIELD))
	    Emergency_shield (GetInd[pl->id], 0);
	CLR_BIT(pl->have, OBJ_EMERGENCY_SHIELD);
    }
    if (pl->tractor_beams <= 0)
	CLR_BIT(pl->have, OBJ_TRACTOR_BEAM);
    if (pl->autopilots <= 0) {
	if (BIT(pl->used, OBJ_AUTOPILOT))
	    Autopilot (GetInd[pl->id], 0);
	CLR_BIT(pl->have, OBJ_AUTOPILOT);
    }
}
 
void Item_damage(int ind, float prob)
{
    player *pl;
    int	p, rem;

    if (prob >= 0.0 && prob <= 1.0) {
	p = (prob * RAND_MAX);
	rem = 1;
    } else {
	p = RAND_MAX;
	rem = 1 + (int)prob;
    }

#ifdef OLD_ITEM_DAMAGE
    if (rand() > p)
	return;

#define ITEM_DAMAGE(item) (pl->item = rand() % (pl->item + 1))

#else /* OLD_ITEM_DAMAGE */

#define ITEM_DAMAGE(item) \
    if (pl->item && rand() <= p) { \
	if ((pl->item -= ((rand() % rem) + 1)) < 0) \
	    pl->item = 0; \
    }

#endif /* OLD_ITEM_DAMAGE */

    pl			= Players[ind];
    ITEM_DAMAGE(extra_shots);
    ITEM_DAMAGE(back_shots);
    ITEM_DAMAGE(missiles);
    ITEM_DAMAGE(mines);
    ITEM_DAMAGE(ecms);
    ITEM_DAMAGE(cloaks);
    ITEM_DAMAGE(sensors);
    ITEM_DAMAGE(lasers);
    ITEM_DAMAGE(transporters);
    ITEM_DAMAGE(afterburners);
    ITEM_DAMAGE(emergency_thrusts);
    ITEM_DAMAGE(emergency_shields);
    ITEM_DAMAGE(tractor_beams);
    ITEM_DAMAGE(autopilots);
    Item_update_flags(pl);
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


void Place_item(int type, player *pl, int count)
{
    object		*item;
    int			x, y,
			place_count = 0;
    float		vx, vy;
    bool		grav;


    if (NumObjs >= MAX_TOTAL_SHOTS)
        return;

    /*
     * Correct count if necessary.
     */
    switch (type) {
    case ITEM_ROCKET_PACK:
	if (count < 1 || count > maxMissilesPerPack)
	    count = maxMissilesPerPack;
	break;
    case ITEM_MINE_PACK:
	if (count < 1 || count > 2)
	    count = 1 + (rand()&1);
	break;
    default:
	count = 1;
	break;
    }

    if (pl) {
        grav = GRAVITY;
        x = pl->prevpos.x / BLOCK_SZ;
        y = pl->prevpos.y / BLOCK_SZ;
    } else {
        if ((rand()&127) < MovingItemsRand)
            grav = GRAVITY;
        else {
            grav = 0;
        }
	x = rand()%World.x;
	y = rand()%World.y;
    }
    while (World.block[x][y] != SPACE) {
	/*
	 * This will take very long (or forever) with maps
	 * that hardly have any (or none) spaces.
	 */
	if (place_count++ > 4)
	    return;
	x = rand()%World.x;
	y = rand()%World.y;
    }
    if (grav) {
        vx = (rand()&7)-3;
        vy = (rand()&7)-3;
        if (pl) {
            vx += pl->vel.x;
            vy += pl->vel.y;
        } else {
	    vy -= Gravity * 12;
	}
    } else {
	vx = vy = 0.0;
    }

    item = Obj[NumObjs++];
    item->color = RED;
    item->info = type;
    item->status = grav;
    item->id = -1;
    item->pos.x = x * BLOCK_SZ + BLOCK_SZ/2;
    item->pos.y = y * BLOCK_SZ + BLOCK_SZ/2;
    item->prevpos = item->pos;
    item->vel.x = vx;
    item->vel.y = vy;
    item->acc.x =
    item->acc.y = 0.0;
    item->mass = 10.0;
    item->life = 1500 + (rand()&511);
    item->count = count;
    item->pl_range = ITEM_SIZE/2;
    item->pl_radius = ITEM_SIZE/2;

    switch (type) {
    case ITEM_ROCKET_PACK:
	item->type = OBJ_ROCKET_PACK;
	break;
    case ITEM_SENSOR_PACK:
	item->type = OBJ_SENSOR_PACK;
	break;
    case ITEM_ECM:
	item->type = OBJ_ECM;
	break;
    case ITEM_MINE_PACK:
	item->type = OBJ_MINE_PACK;
	break;
    case ITEM_TANK:
	item->type = OBJ_TANK;
	break;
    case ITEM_CLOAKING_DEVICE:
	item->type = OBJ_CLOAKING_DEVICE;
	break;
    case ITEM_ENERGY_PACK:
	item->type = OBJ_ENERGY_PACK;
	break;
    case ITEM_WIDEANGLE_SHOT:
	item->type = OBJ_WIDEANGLE_SHOT;
	break;
    case ITEM_BACK_SHOT:
	item->type = OBJ_BACK_SHOT;
	break;
    case ITEM_AFTERBURNER:
	item->type = OBJ_AFTERBURNER;
	break;
    case ITEM_TRANSPORTER:
	item->type = OBJ_TRANSPORTER;
	break;
    case ITEM_LASER:
	item->type = OBJ_LASER;
	break;
    case ITEM_EMERGENCY_THRUST:
	item->type = OBJ_EMERGENCY_THRUST;
	break;
    case ITEM_EMERGENCY_SHIELD:
	item->type = OBJ_EMERGENCY_SHIELD;
	break;
    case ITEM_TRACTOR_BEAM:
	item->type = OBJ_TRACTOR_BEAM;
	break;
    case ITEM_AUTOPILOT:
	item->type = OBJ_AUTOPILOT;
	break;
    default:
	item->type = OBJ_ROCKET_PACK;
	break;
    }

    World.items[type].num++;
}


void Throw_items(player *pl)
{
    int i;

    if (!ThrowItemOnKillRand)
	return;

    for (i = pl->extra_shots - initialWideangles; i > 0; i--)
        if ((rand()&127) < ThrowItemOnKillRand) {
            Place_item(ITEM_WIDEANGLE_SHOT, pl, 1);
	    pl->extra_shots--;
	}
    for (i = pl->ecms - initialECMs; i > 0; i--)
        if ((rand()&127) < ThrowItemOnKillRand) {
            Place_item(ITEM_ECM, pl, 1);
	    pl->ecms--;
	}
    for (i = pl->sensors - initialSensors; i > 0; i--)
        if ((rand()&127) < ThrowItemOnKillRand) {
            Place_item(ITEM_SENSOR_PACK, pl, 1);
	    pl->sensors--;
	}
    for (i = pl->afterburners - initialAfterburners; i > 0; i--)
        if ((rand()&127) < ThrowItemOnKillRand) {
            Place_item(ITEM_AFTERBURNER, pl, 1);
	    pl->afterburners--;
	}
    for (i = pl->transporters - initialTransporters; i > 0; i--)
        if ((rand()&127) < ThrowItemOnKillRand) {
            Place_item(ITEM_TRANSPORTER, pl, 1);
	    pl->transporters--;
	}
    for (i = pl->back_shots - initialRearshots; i > 0; i--)
        if ((rand()&127) < ThrowItemOnKillRand) {
            Place_item(ITEM_BACK_SHOT, pl, 1);
	    pl->back_shots--;
	}
    for (i = pl->missiles - initialMissiles; i > 0; i-=4)
        if ((rand()&127) < ThrowItemOnKillRand) {
	    int n = (i > 4 ? 4 : i);
            Place_item(ITEM_ROCKET_PACK, pl, n);
	    pl->missiles -= n;
	}
    for (i = pl->cloaks - initialCloaks; i > 0; i--)
        if ((rand()&127) < ThrowItemOnKillRand) {
            Place_item(ITEM_CLOAKING_DEVICE, pl, 1);
	    pl->cloaks--;
	}
    for (i = pl->mines - initialMines; i > 0; i-=2)
        if ((rand()&127) < ThrowItemOnKillRand) {
	    int n = (i > 2 ? 2 : i);
            Place_item(ITEM_MINE_PACK, pl, n);
	    pl->mines -= n;
	}
    for (i = pl->lasers - initialLasers; i > 0; i--)
        if ((rand()&127) < ThrowItemOnKillRand) {
            Place_item(ITEM_LASER, pl, 1);
	    pl->lasers--;
	}
    for (i = pl->emergency_thrusts - initialEmergencyThrusts; i > 0; i--)
        if ((rand()&127) < ThrowItemOnKillRand) {
            Place_item(ITEM_EMERGENCY_THRUST, pl, 1);
	    pl->emergency_thrusts--;
	}
    for (i = pl->emergency_shields - initialEmergencyShields; i > 0; i--)
        if ((rand()&127) < ThrowItemOnKillRand) {
            Place_item(ITEM_EMERGENCY_SHIELD, pl, 1);
	    pl->emergency_shields--;
	}
    for (i = pl->tractor_beams - initialTractorBeams; i > 0; i--)
        if ((rand()&127) < ThrowItemOnKillRand) {
            Place_item(ITEM_TRACTOR_BEAM, pl, 1);
	    pl->tractor_beams--;
	}
    for (i = pl->autopilots - initialAutopilots; i > 0; i--)
	if ((rand()&127) < ThrowItemOnKillRand) {
	    Place_item(ITEM_AUTOPILOT, pl, 1);
	    pl->autopilots--;
	}
    /*
     * Would it be nice if Tanks could be thrown?
     */
    Item_update_flags(pl);
	    
}

/*
 * Cause some remaining mines or missiles to be launched in
 * a random direction with a small life time (ie. magazine has
 * gone off).
 */
void Detonate_items(player *pl)
{
    int	i;

    if (!BIT(pl->status, KILLED))
	return;

    /*
     * These are always immune to detonation.
     */
    if ((pl->mines -= initialMines) < 0)
	pl->mines = 0;
    if ((pl->missiles -= initialMissiles) < 0)
	pl->missiles = 0;

    /*
     * Drop shields in order to launch mines and missiles.
     */
    CLR_BIT(pl->used, OBJ_SHIELD);

    /*
     * Mines are always affected by gravity and are sent in random directions
     * slowly out from the ship (velocity relative).
     */
    for (i = 0; i < pl->mines; i++) {
	if ((rand()&127) < DetonateItemOnKillRand) {
	    int dir = rand() % RES;
	    float vel = ((rand() % 16) / 4.0);

	    Place_general_mine(GetInd[pl->id], GRAVITY,
			       pl->pos.x, pl->pos.y,
			       pl->vel.x + vel * tcos(dir),
			       pl->vel.y + vel * tsin(dir),
			       pl->mods);
	}
    }
    for (i = 0; i < pl->missiles; i++) {
	if ((rand()&127) < DetonateItemOnKillRand) {
	    int	type;

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

	    Fire_shot(GetInd[pl->id], type, (rand() % RES));
	}
    }
}

void Place_mine(int ind)
{
    player *pl = Players[ind];

    if (pl->mines <= 0 || (BIT(pl->used, OBJ_SHIELD) && !shieldedMining))
	return;

    Place_general_mine(ind, 0, 
		       pl->pos.x, pl->pos.y, 0.0, 0.0, pl->mods);
}


void Place_moving_mine(int ind)
{
    player *pl = Players[ind];

    if (pl->mines <= 0 || (BIT(pl->used, OBJ_SHIELD) && !shieldedMining))
	return;

    Place_general_mine(ind, GRAVITY,
		       pl->pos.x, pl->pos.y, pl->vel.x, pl->vel.y, pl->mods);
}

void Place_general_mine(int ind, int status, float x, float y,
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

    if (BIT(mods.nuclear, NUCLEAR)) {
	if (pl) {
	    used = (BIT(mods.nuclear, FULLNUCLEAR) ?
		    pl->mines : NUKE_MIN_MINE);
	    if (pl->mines < NUKE_MIN_MINE) {
		sprintf(msg, "You need at least %d mines to %s %s!",
			NUKE_MIN_MINE, 
			(BIT(status, GRAVITY) ? "throw" : "drop"),
			Describe_shot (OBJ_MINE, status, mods, 0));
		Set_player_message (pl, msg);
		return;
	    }
	} else {
	    used = NUKE_MIN_MINE;
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
	    sprintf(msg, "You need at least %d fuel units to %s %s!",
		    (-drain) >> FUEL_SCALE_BITS,
		    (BIT(status, GRAVITY) ? "throw" : "drop"),
		    Describe_shot(OBJ_MINE, status, mods, 0));
	    Set_player_message (pl, msg);
	    return;
	}
	Add_fuel(&(pl->fuel), drain);
	pl->mines -= used;

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

	    spread = (float)mods.spread + 1;
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
    shot->status = (GRAVITY|FROMCANNON);
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

    shot->dir	= MOD2(shot->dir, RES);
    shot->vel.x	= speed*tcos(dir);
    shot->vel.y	= speed*tsin(dir);
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
    int			i,
			num_destroyed_team_members = 0;

    Check_team_members (td->team);
    if (World.teams[td->team].NumMembers <= 0 || td->team == pl->team)
	return 0;

    sound_play_all(DESTROY_BALL_SOUND);
    sprintf(msg, " < %s's (%d) team has destroyed team %d treasure >",
	    pl->name, pl->team, td->team);
    Set_message(msg);    

    td->destroyed++;
    World.teams[td->team].TreasuresLeft--;
    World.teams[tt->team].TreasuresDestroyed++;
    
    for (i = 0; i < NumPlayers; i++) {
	if (Players[i]->team == td->team
	    && Players[i]->robot_mode != RM_OBJECT
	    && (!BIT(Players[i]->status, PAUSE)
		|| pl->count > 0)
	    && (!BIT(Players[i]->status, GAME_OVER)
		|| Players[i]->mychar != 'W'
		|| Players[i]->score != 0)) {
	    num_destroyed_team_members++;
	    if (BIT(pl->mode, LIMITED_LIVES))
		Players[i]->life = 0;
	    SCORE(i, -TREASURE_SCORE, tt->pos.x, tt->pos.y, "Treasure:");
	    if (treasureKillTeam)
		SET_BIT(Players[i]->status, KILLED);
	}
    }
    if (num_destroyed_team_members > 0) {
	for (i = 0; i < NumPlayers; i++) {
	    if (Players[i]->team == pl->team
		&& Players[i]->robot_mode != RM_OBJECT
		&& (!BIT(Players[i]->status, PAUSE)
		    || pl->count > 0)
		&& (!BIT(Players[i]->status, GAME_OVER)
		    || Players[i]->mychar != 'W'
		    || Players[i]->score != 0)) {
		SCORE(i, ((i == ind) ? (TREASURE_SCORE * 2) : TREASURE_SCORE),
		      tt->pos.x, tt->pos.y, "Treasure:");
	    }
	}
    }

    updateScores = true;

    return 1;
}

/*
 * Describes shot of `type' which has `status' and `mods'.  If `hit' is
 * non-zero this description is part of a collision, otherwise its part
 * of a launch message.
 */
char *Describe_shot(int type, int status, modifiers mods, int hit)
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

void Fire_general_shot(int ind, float x, float y, int type, int dir,
		       float speed, modifiers mods)
{
    char		msg[MSG_LEN];
    player		*pl = (ind == -1 ? NULL : Players[ind]);
    int			used, life,
			lock = 0,
			status = GRAVITY,
			i, ldir, minis,
			pl_range,
			pl_radius,
			rack_no;
    long		drain;
    float		mass,
			turnspeed = 0,
			max_speed = SPEED_LIMIT;
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
	break;

    case OBJ_TORPEDO:
    case OBJ_HEAT_SHOT:
    case OBJ_SMART_SHOT:
	/*
	 * Make sure there are enough object entries for the mini shots.
	 */
	if (NumObjs + mods.mini >= MAX_TOTAL_SHOTS)
	    return;

	if (pl && pl->missiles <= 0)
	    return;

	if (BIT(mods.nuclear, NUCLEAR)) {
	    if (pl) {
		used = (BIT(mods.nuclear, FULLNUCLEAR) ?
			pl->missiles : NUKE_MIN_SMART);
		if (pl->missiles < NUKE_MIN_SMART) {
		    sprintf(msg, 
			    "You need at least %d missiles to fire %s!",
			    NUKE_MIN_SMART,
			    Describe_shot (type, status, mods, 0));
		    Set_player_message (pl, msg);
		    return;
		}
	    } else {
		used = NUKE_MIN_SMART;
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
	    break;
	}

	if (pl) {
	    if (pl->fuel.sum < -drain) {
		sprintf(msg, "You need at least %d fuel units to fire %s!",
			(-drain) >> FUEL_SCALE_BITS,
			Describe_shot(type, status, mods, 0));
		Set_player_message (pl, msg);
		return;
	    }
	    Add_fuel(&(pl->fuel), drain);
	    pl->missiles -= used;

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

/* If more than one missile rack then choose one to start and spread minis *
 * across the number of racks */
    rack_no = rand() & pl->ship->num_m_rack;
    for (i = 0; i < minis; i++) {
	object *shot = Obj[NumObjs++];

	shot->life 	= life / minis;
	shot->mass	= mass / minis;
	shot->max_speed = max_speed;
	shot->turnspeed = turnspeed;
	shot->count 	= 0;
	shot->info 	= lock;
	shot->type	= type;
	shot->id	= (pl ? pl->id : -1);
	shot->color	= (pl ? pl->color : WHITE);
	shot->prevpos	= shot->pos;
	if (minis > 1) {
	    float	angle;
	    float	spread;

	    spread = (float)mods.spread + 1;
	    angle = ((float)(minis - 1 - 2 * i)) / ((float)(minis-1));

	    /*
	     * Torpedos spread like mines, except the launch direction
	     * is preset over the range +/- MINI_TORPEDO_SPREAD_ANGLE.
	     * (This is not modified by the spread, the initial velocity is)
	     * 
	     * Other missiles are just launched in a different direction
	     * which varies over the range +/- MINI_MISSILE_SPREAD_ANGLE,
	     * which spread varies.
	     */
	    switch (type) {
	    case OBJ_TORPEDO:
		spread = (float)mods.spread + 1;
		angle *= (MINI_TORPEDO_SPREAD_ANGLE / 360.0) * RES;
		ldir = MOD2(dir + (int)angle, RES);
		mv.x = MINI_TORPEDO_SPREAD_SPEED * tcos(ldir) / spread;
		mv.y = MINI_TORPEDO_SPREAD_SPEED * tsin(ldir) / spread;
		/*
		 * This causes the added initial velocity to reduce to
		 * zero over the MINI_TORPEDO_SPREAD_TIME.
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
 	} else {
	    mv.x = mv.y = shot->acc.x = shot->acc.y = 0;
	    ldir = dir;
	}

	if (type == OBJ_SHOT) {
	    shot->pos.x = (pl) ? x : x;
	    shot->pos.y = (pl) ? y : y;
	}
	else {
            shot->pos.x = (pl) ? x : x;
            shot->pos.y = (pl) ? y : y;
            rack_no = (rack_no + 1) % pl->ship->num_m_rack;
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
	shot->vel.x 	= mv.x + (pl ? pl->vel.x : 0.0) + tcos(ldir) * speed;
	shot->vel.y 	= mv.y + (pl ? pl->vel.y : 0.0) + tsin(ldir) * speed;
	shot->status	= status;
	shot->dir	= ldir;
	shot->mods  	= mods;
	shot->pl_range  = pl_range;
	shot->pl_radius = pl_radius;
    }

    /*
     * Recoil must be done instantaneously otherwise ship moves back after
     * firing each mini missile.
     */
    if (pl) {
	for (i = 1; i <= minis; i++)
	    Recoil((object *)pl, Obj[NumObjs - i]);
    }
}


void Fire_normal_shots(int ind)
{
    player		*pl = Players[ind];
    int			i, shot_angle;

    shot_angle = MODS_SPREAD_MAX - pl->mods.spread;

    pl->shot_time = loops;
    Fire_main_shot(ind, OBJ_SHOT, pl->dir);
    for (i = 0; i < pl->extra_shots; i++) {
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
    for (i = 0; i < pl->back_shots; i++) {
	Fire_shot(ind, OBJ_SHOT, MOD2(pl->dir + RES/2
		       + ((pl->back_shots - 1 - 2 * i) * shot_angle) / 2,
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

    switch (shot->type) {

    case OBJ_MINE:
	Explode_object(shot, shot->prevpos.x, shot->prevpos.y, 0, RES, 500);
	sound_play_sensors(shot->pos.x, shot->pos.y, MINE_EXPLOSION_SOUND);
    case OBJ_SPARK:
    case OBJ_DEBRIS:
	break;

    case OBJ_BALL:
	if (shot->id != -1)
	    Detach_ball(GetInd[shot->id], ind);
	if (shot->owner == -1) {
	    /*
	     * If the ball has never been owned, the only way it could have
	     * been destroyed is by being knocked out of the goal.  Therefore
	     * we force the ball to been recreated.
	     */
	    World.treasures[shot->treasure].have = false;
	    SET_BIT(shot->status, RECREATE);
	}
	if (BIT(shot->status, RECREATE)) {
	    addBall = 1;
	    sound_play_sensors(shot->pos.x, shot->pos.y, EXPLODE_BALL_SOUND);
	    Explode_object(shot, shot->prevpos.x, shot->prevpos.y, 0, RES,
			   300);
	}
	break;
	/* Shots related to a player. */

    case OBJ_HEAT_SHOT:
    case OBJ_TORPEDO:
    case OBJ_SMART_SHOT:
	Explode_object(shot, shot->pos.x, shot->pos.y, 0, RES, 30);
    case OBJ_SHOT:
        if (shot->id == -1
	    || BIT(shot->status, FROMCANNON)
	    || BIT(shot->mods.warhead, CLUSTER))
	    break;
	pl = Players[GetInd[shot->id]];
	if (shot->type == OBJ_SHOT)
	    pl->shots--;
	break;

	/* Special items. */
    case OBJ_ROCKET_PACK:
	if (shot->life == 0 && shot->color != WHITE) {
	    shot->color = WHITE;
	    shot->life	= FPS * WARN_TIME;
	    return;
	}
	World.items[ITEM_ROCKET_PACK].num--;
	if (shot->life == 0) {
	    addHeat = 1;
	}
	break;

    case OBJ_AFTERBURNER:
	World.items[ITEM_AFTERBURNER].num--;
	break;

    case OBJ_SENSOR_PACK:
	World.items[ITEM_SENSOR_PACK].num--;
	break;

    case OBJ_ECM:
	World.items[ITEM_ECM].num--;
	break;

    case OBJ_TRANSPORTER:
	World.items[ITEM_TRANSPORTER].num--;
	break;

    case OBJ_LASER:
	World.items[ITEM_LASER].num--;
	break;

    case OBJ_EMERGENCY_THRUST:
	World.items[ITEM_EMERGENCY_THRUST].num--;
	break;

    case OBJ_EMERGENCY_SHIELD:
	World.items[ITEM_EMERGENCY_SHIELD].num--;
	break;

    case OBJ_TRACTOR_BEAM:
	World.items[ITEM_TRACTOR_BEAM].num--;
	break;

    case OBJ_AUTOPILOT:
	World.items[ITEM_AUTOPILOT].num--;
	break;

    case OBJ_CLOAKING_DEVICE:
	World.items[ITEM_CLOAKING_DEVICE].num--;
	break;

    case OBJ_ENERGY_PACK:
	World.items[ITEM_ENERGY_PACK].num--;
	break;

    case OBJ_WIDEANGLE_SHOT:
	World.items[ITEM_WIDEANGLE_SHOT].num--;
	break;

    case OBJ_BACK_SHOT:
	World.items[ITEM_BACK_SHOT].num--;
	break;

    case OBJ_MINE_PACK:
	if (!shot->life && shot->color != WHITE) {
	    shot->color = WHITE;
	    shot->life  = FPS * WARN_TIME;
	    return;
	}
	World.items[ITEM_MINE_PACK].num--;
	if (shot->life == 0) {
	    addMine = 1;
	}
	break;

    case OBJ_TANK:
	World.items[ITEM_TANK].num--;
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
	    if (BIT(World.rules->mode, ALLOW_MODIFIERS) && (rand()%3) == 0) {
		mods.velocity = 1 + (rand() % MODS_VELOCITY_MAX);
	    }
	    if (BIT(World.rules->mode, ALLOW_MODIFIERS) && (rand()%3) == 0) {
		mods.power = 1 + (rand() % MODS_POWER_MAX);
	    }
	}
	else if (BIT(World.rules->mode, ALLOW_MODIFIERS) && (rand()%3) == 0) {
	    SET_BIT(mods.warhead, IMPLOSION);
	}
	if (addMine) {
	    Place_general_mine (-1, (rand()%2 == 0 ? GRAVITY : 0),
				shot->pos.x, shot->pos.y, 0.0, 0.0, mods);
	}
	else if (addHeat) {
	    Fire_general_shot (-1, shot->pos.x, shot->pos.y, OBJ_HEAT_SHOT,
			       MOD2(rand(), RES), 0.0, mods);
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
	    && p->robot_mode != RM_OBJECT) {

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

#define STEAL(item, msg)	\
{				\
    if (!p->item)		\
	break;			\
    p->item--;			\
    pl->item++;			\
    what = msg;			\
    done = true;		\
}

    while (!done)
	switch (1 << (rand() & 31)) {
	    case OBJ_AFTERBURNER:
		STEAL(afterburners, "an afterburner");
		if (p->afterburners <= 0)
		    CLR_BIT(p->have, OBJ_AFTERBURNER);
		SET_BIT(pl->have, OBJ_AFTERBURNER);
		if (pl->afterburners > MAX_AFTERBURNER)
		    pl->afterburners = MAX_AFTERBURNER;
		break;

	    case OBJ_ROCKET_PACK:
		STEAL(missiles, "some missiles");

		if (p->missiles < 3) {
		    pl->missiles += p->missiles;
		    p->missiles = 0;
		}
		else {
		    p->missiles -= 3;
		    pl->missiles += 3;
		}
		break;

	    case OBJ_CLOAKING_DEVICE:
		STEAL(cloaks, "a cloaking device");
		p->updateVisibility = pl->updateVisibility = 1;
		if (!p->cloaks) {
		    CLR_BIT(p->used, OBJ_CLOAKING_DEVICE);
		    CLR_BIT(p->have, OBJ_CLOAKING_DEVICE);
		}
		SET_BIT(pl->have, OBJ_CLOAKING_DEVICE);
		break;

	    case OBJ_WIDEANGLE_SHOT:
		STEAL(extra_shots, "a wide");
		break;

	    case OBJ_BACK_SHOT:
		STEAL(back_shots, "a rear");
		break;

	    case OBJ_MINE_PACK:
		STEAL(mines, "a mine");
		break;

	    case OBJ_SENSOR_PACK:
		STEAL(sensors, "a sensor");
		p->updateVisibility = pl->updateVisibility = 1;
		break;

	    case OBJ_ECM:
		STEAL(ecms, "an ECM");
		break;

	    case OBJ_TRANSPORTER:
		STEAL(transporters, "a transporter");
		break;

	    case OBJ_LASER:
		STEAL(lasers, "a laser");
		if (pl->lasers > MAX_LASERS)
		    pl->lasers = MAX_LASERS;
		break;

	    case OBJ_EMERGENCY_THRUST:
		STEAL(emergency_thrusts, "an emergency thrust");
		if (!p->emergency_thrusts) {
		    if (BIT(p->used, OBJ_EMERGENCY_THRUST))
			Emergency_thrust(GetInd[p->id], 0);
		    CLR_BIT(p->have, OBJ_EMERGENCY_THRUST);
		}
		SET_BIT(pl->have, OBJ_EMERGENCY_THRUST);
		break;

	    case OBJ_EMERGENCY_SHIELD:
		STEAL(emergency_shields, "an emergency shield");
		if (!p->emergency_shields) {
		    if (BIT(p->used, OBJ_EMERGENCY_SHIELD))
			Emergency_shield(GetInd[p->id], 0);
		    CLR_BIT(p->have, OBJ_EMERGENCY_SHIELD);
		}
		SET_BIT(pl->have, OBJ_EMERGENCY_SHIELD);
		break;

	    case OBJ_TRACTOR_BEAM:
		STEAL(tractor_beams, "a tractor beam");
		if (!p->tractor_beams)
		    CLR_BIT(p->have, OBJ_TRACTOR_BEAM);
		SET_BIT(pl->have, OBJ_TRACTOR_BEAM);
		if (pl->tractor_beams > MAX_TRACTORS)
		    pl->tractor_beams = MAX_TRACTORS;
		break;

	    case OBJ_AUTOPILOT:
		STEAL(autopilots, "an autopilot");
		if (!p->autopilots) {
		    if (BIT(p->used, OBJ_AUTOPILOT))
			Autopilot(GetInd[p->id], 0);
		    CLR_BIT(p->have, OBJ_AUTOPILOT);
		}
		SET_BIT(pl->have, OBJ_AUTOPILOT);
		break;

	    case OBJ_TANK:
		{
		    int             no, c,
		                    t;

		    if (pl->fuel.num_tanks == MAX_TANKS || !p->fuel.num_tanks)
			break;
		    t = (rand() % p->fuel.num_tanks) + 1;

		    /* remove the tank from the victim */
		    p->fuel.sum -= p->fuel.tank[t];
		    p->fuel.max -= TANK_CAP(t);
		    for (i = t; i < p->fuel.num_tanks; i++)
			p->fuel.tank[i] = p->fuel.tank[i + 1];
		    p->emptymass -= TANK_MASS;
		    p->fuel.num_tanks -= 1;
		    if (p->fuel.current)
			p->fuel.current--;

		    /* add the tank to the thief */
		    c = pl->fuel.current;
		    no = ++(pl->fuel.num_tanks);
		    SET_BIT(pl->have, OBJ_TANK);
		    pl->fuel.current = no;
		    pl->fuel.max += TANK_CAP(no);
		    pl->fuel.tank[no] = 0;
		    pl->emptymass += TANK_MASS;
		    Add_fuel(&(pl->fuel), p->fuel.tank[t]);
		    pl->fuel.current = c;

		    what = "a tank";
		    done = true;
		    break;
		}

	    case OBJ_ENERGY_PACK:	/* used to steal fuel */
#define MIN_FUEL_STEAL	10
#define MAX_FUEL_STEAL  50
		{
		    long            amount;
		    float          percent;

		    percent = ((rand() % (MAX_FUEL_STEAL - MIN_FUEL_STEAL) +
				MIN_FUEL_STEAL) / 100.0);
		    amount = (long)(p->fuel.sum * percent);
		    sprintf(msg, "%s stole %d units (%d%%) of fuel from %s",
			    pl->name, amount >> FUEL_SCALE_BITS,
			    (int) (percent * 100.0 + 0.5), p->name);
		    Add_fuel(&(pl->fuel), amount);
		    Add_fuel(&(p->fuel), -amount);
		    Set_message(msg);
		    return;
		}
	}

    sprintf(msg, "%s stole %s from %s.", pl->name, what, p->name);
    Set_message(msg);
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
	    if ((pl->lock.distance <= pl->sensor_range
		 || !BIT(World.rules->mode, LIMITED_VISIBILITY))
		&& pl->visibility[GetInd[pl->lock.pl_id]].canSee)
		shot->new_info = pl->lock.pl_id;
	    else
		shot->new_info = Players[rand() % NumPlayers]->id;
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
    if (ecmsReprogramMines && closest_mine!=NULL) {
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

	    /* should this be FPS dependant: damage = 4.0f * FPS * range; ?  no, i think. */
	    damage = 24.0f * range;

	    if (p->cloaks <= 1) {
		p->forceVisible += (int)damage;
	    } else {
		p->forceVisible += (int)(damage * pow(0.75, (p->cloaks-1)));
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
	    if (p->lasers > 0) {
		p->lasers = range * p->lasers;
	    }

	    if (p->robot_mode == RM_NOT_ROBOT || p->robot_mode == RM_OBJECT) {
		/* player is blinded by light flashes. */
		p->damaged += (int)(damage * pow(0.75, p->sensors));
	    } else {
		if (BIT(pl->lock.tagged, LOCK_PLAYER)
		    && (pl->lock.distance < pl->sensor_range
			|| !BIT(World.rules->mode, LIMITED_VISIBILITY))
		    && pl->visibility[GetInd[pl->lock.pl_id]].canSee
		    && pl->lock.pl_id != p->id) {

		    /*
		     * Player programs robot to seek target.
		     */
		    p->robot_lock_id = pl->lock.pl_id;
		    SET_BIT(p->robot_lock, LOCK_PLAYER);
		    for (j = 0; j < NumPlayers; j++) {
			if (Players[j]->conn != NOT_CONNECTED) {
			    Send_seek(Players[j]->conn, pl->id,
				      p->id, p->robot_lock_id);
			}
		    }
		}
	    }
	}
    }
}

void Fire_ecm(int ind)
{
    player *pl = Players[ind];

    if (pl->ecms == 0
	|| pl->fuel.sum <= -ED_ECM
	|| pl->ecmInfo.count >= MAX_PLAYER_ECMS)
	return;

    SET_BIT(pl->used, OBJ_ECM);
    do_ecm(pl);
    pl->ecms--;
    Add_fuel(&(pl->fuel), ED_ECM);
}

void Move_ball(int ind)
{
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
}


void Move_smart_shot(int ind)
{
    object *shot = Obj[ind];
    player *pl;
    int	   angle, theta;
    float range = 0.0;
    float acc;
    float x_dif = 0.0;
    float y_dif = 0.0;
    float shot_speed;

    if (shot->type == OBJ_TORPEDO) {
	if (BIT(shot->mods.nuclear, NUCLEAR)) {
	    acc = (shot->info++ < NUKE_SPEED_TIME) ? NUKE_ACC : 0.0;
	} else {
	    acc = (shot->info++ < TORPEDO_SPEED_TIME) ? TORPEDO_ACC : 0.0;
	}
	acc *= (1 + (shot->mods.power * MISSILE_POWER_SPEED_FACT));
	if (shot->mods.mini) {
	    if (shot->spread_left-- <= 0) {
		shot->acc.x = 0;
		shot->acc.y = 0;
	    }
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
                    l *= MAX_AFTERBURNER + 1 - p->afterburners;
                    l /= MAX_AFTERBURNER + 1;
                    if (BIT(p->have, OBJ_AFTERBURNER))
			l *= 16 - p->afterburners;
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
		if(range > (SMART_SHOT_LOOK_AH-i)*(BLOCK_SZ/BLOCK_PARTS)) {
		    if (shot_speed > SMART_SHOT_MIN_SPEED)
			shot_speed -= acc * (SMART_SHOT_DECFACT+1);
		}
		foundw = 1;
	    }
	}

	i = ((int)(shot->dir * 8 / RES)&7) + 8;
	xi = shot->pos.x / BLOCK_SZ;
	yi = shot->pos.y /BLOCK_SZ;

	for(j=2, angle=-1, freemax=0; j>=-2; --j) {
	    int si, xt, yt;

	    for(si=1, k=0; si >= -1; --si) {
		xt = xi + sur[(i+j+si)&7].dx;
		yt = yi + sur[(i+j+si)&7].dy;

		if(xt >= 0 && xt < World.x && yt >= 0 && yt < World.y)
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
			if(!si)
			    k = -32;
			break;
		    default:
			++k;
			break;
		    }
	    }
	    if (k > freemax || k == freemax
		&& ((j == -1 && (rand()&1)) || j == 0 || j == 1)) {
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
			    "(shape: 10,0 6,-3 2,-7 0,-8 -2,-7 -6,-3 -10,0"
			    " -6,3 -2,7 0,8 2,7 6,3)"
			    "(leftLight: -6,-3)"
			    "(rightLight: -6,3)";
    
    /* Return, if no more players or no tanks */
    if (pl->fuel.num_tanks == 0
	|| NumPseudoPlayers == MAX_PSEUDO_PLAYERS
	|| Id >= MAX_ID) {
	return;
    }

    sound_play_sensors(pl->pos.x, pl->pos.y, TANK_DETACH_SOUND);

    /* If current tank is main, use another one */
    if ((ct=pl->fuel.current) == 0)
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
    strcpy(dummy->realname, pl->realname);
    strcpy(dummy->hostname, pl->hostname);
    dummy->home_base	= pl->home_base;
    dummy->team		= pl->team;
    dummy->pseudo_team	= pl->pseudo_team;
    dummy->robot_mode   = RM_OBJECT;
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
    dummy->afterburners      = initialAfterburners;
    dummy->back_shots        = initialRearshots;
    dummy->cloaks            = initialCloaks;
    dummy->ecms              = initialECMs;
    dummy->extra_shots       = initialWideangles;
    dummy->lasers            = initialLasers;
    dummy->mines             = initialMines;
    dummy->missiles          = initialMissiles;
    dummy->sensors           = initialSensors;
    dummy->transporters      = initialTransporters;
    dummy->autopilots        = initialAutopilots;
    dummy->emergency_thrusts = initialEmergencyThrusts;
    dummy->emergency_shields = initialEmergencyShields;
    dummy->tractor_beams     = initialTractorBeams;

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
    dummy->robot_lock_id = pl->id;
    dummy->robot_lock = LOCK_NONE;

    /* Handling the id's and the Tables */
    dummy->id = Id;
    GetInd[Id] = NumPlayers;
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
    pl->fuel.sum -= pl->fuel.tank[ct];
    pl->fuel.max -= TANK_CAP(ct);

    for (i=ct; i < pl->fuel.num_tanks; i++)
        pl->fuel.tank[i] = pl->fuel.tank[i+1];

    pl->emptymass -= TANK_MASS;
    pl->fuel.num_tanks--;
    if (pl->fuel.current)
	pl->fuel.current--;

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

void Explode_object(object *shot,
		    float x, float y, int real_dir, int spread, int intensity)
{
    object		*debris;
    const player	*pl = NULL;
    int			i,
			num_debris,
			speed,
			dir,
			status,
			obj_id,
			color = RED,
			type = OBJ_DEBRIS,
			modv = 1;
    float		speed_modv,
			life_modv,
			impl_modv = 1,
			shot_mass,
			obj_mass,
			vx, vy;
    modifiers		obj_mods;
    const int		spreadoffset = (spread/2);

    if (BIT(World.rules->mode, WRAP_PLAY)) {
	if (x < 0) x += World.width;
	else if (x >= World.width) x -= World.width;
	if (y < 0) y += World.height;
	else if (y >= World.height) y -= World.height;
    }
    if (x < 0 || x >= World.width || y < 0 || y >= World.height) {
	return;
    }

    /*
     * We don't set FROMCANNON even if its a cannon explosion because
     * we want cluster explosions to kill cannons.
     */
    status = GRAVITY;
    shot_mass = ShotsMass;

    if (shot) {
	if (shot->id != -1) {
	    pl = Players[GetInd[shot->id]];
	    shot_mass = pl->shot_mass;
	}

	if (! shot->type || BIT(shot->type, OBJ_PLAYER)) {
	    /*
	     * This case is for wall bouncing, and thus we need a particle
	     * which will never affect the player.
	     */
	    type = OBJ_SPARK;
	    SET_BIT(status, (OWNERIMMUNE|FROMBOUNCE));
	} else {

	    if (BIT(shot->mods.warhead,
		    (CLUSTER|IMPLOSION)) == (CLUSTER|IMPLOSION)) {
		/*
		 * A cluster imploder is a mixture of a cluster
		 * explosion and a standard implosion.  Quite deadly
		 * because it sucks you into the explosion.
		 * Note that if killed by one of these weapons it will say
		 * either cluster or implosion depending on which part killed
		 * the player.
		 */
		shot->mass /= 2.0f;
		intensity >>= 1;

		/* Cluster part */
		CLR_BIT(shot->mods.warhead, IMPLOSION);
		Explode_object(shot, x, y, real_dir, spread, intensity);

		/* Implosion part */
		CLR_BIT(shot->mods.warhead, CLUSTER);
		SET_BIT(shot->mods.warhead, IMPLOSION);
		Explode_object(shot, x, y, real_dir, spread, intensity);
		return;
	    }

	    /*
	     * Explosion velocity is only affected by velocity modifier
	     * if the weapon is a cluster type.
	     * 
	     * The intensity depends on the mass for clusters
	     * and from the argument  modified by the mini factor
	     * for non-clusters.
	     */
	    if (BIT(shot->mods.warhead, CLUSTER)) {
		if (BIT(shot->mods.nuclear, NUCLEAR))
		    modv = 2 << shot->mods.velocity;
		else
		    modv = 1 << shot->mods.velocity;
		intensity = CLUSTER_MASS_SHOTS(shot->mass);
		type = OBJ_SHOT;
		color = (pl ? pl->color : WHITE);
	    } else {
		intensity /= (shot->mods.mini + 1);
	    }

	    if (BIT(shot->mods.nuclear, NUCLEAR)) {
		float	used;	/* Must be float because of mini weapons */

		/*
		 * Intensity is cumulative of number of weapons used
		 * when fired, multiplied by a special amount
		 * depending on the type of weapon.
		 */
		used = shot->mass / NUKE_MASS_MULT;
		if (shot->type == OBJ_MINE)
		    used *= (NUKE_MINE_EXPL_MULT / MINE_MASS);
		else
		    used *= (NUKE_SMART_EXPL_MULT / MISSILE_MASS);
		intensity *= (used / SHOT_MULT(shot));
		sound_play_all(NUKE_EXPLOSION_SOUND);
	    } else {
		sound_play_sensors(x, y, OBJECT_EXPLOSION_SOUND);
	    }

	    if (BIT(shot->mods.warhead, IMPLOSION)) {
		if (BIT(shot->type, OBJ_MINE|OBJ_TORPEDO|
				    OBJ_HEAT_SHOT|OBJ_SMART_SHOT)) {
		    impl_modv = -1;
		}
	    }

	    if (BIT(shot->type, OBJ_TORPEDO|OBJ_HEAT_SHOT|OBJ_SMART_SHOT))
		intensity /= (1 + shot->mods.power);
	}
	vx = shot->vel.x;
	vy = shot->vel.y;
    } else {
	sound_play_sensors(x, y, OBJECT_EXPLOSION_SOUND);
	vx = vy = 0.0;
    }

    switch (type) {
    case OBJ_SHOT:
	obj_mods = shot->mods;
	obj_mass = shot_mass * impl_modv;
	life_modv = (float) modv * 0.2;
	speed_modv = (1.0f / modv) * 0.5;
	break;

    default:
	CLEAR_MODS(obj_mods);
	obj_mass = DEBRIS_MASS * impl_modv;
	life_modv = 1;
	speed_modv = 1;
	break;
    }
    obj_id = (pl ? pl->id : -1);

    num_debris = (intensity/2) + (rand()%(1+intensity/2));
    if (num_debris > MAX_TOTAL_SHOTS - NumObjs) {
	num_debris = MAX_TOTAL_SHOTS - NumObjs;
    }
    for (i = 0; i < num_debris; i++, NumObjs++) {
	debris = Obj[NumObjs];

	debris->life = DEBRIS_LIFE(intensity) * life_modv;
	debris->mods = obj_mods;
	debris->mass = obj_mass;
	speed = DEBRIS_SPEED(intensity) * speed_modv;
	if (speed <= 0)
	    speed = 1;
	else {
	    if (debris->life > ShotsLife && ShotsLife >= FPS)
		debris->life = ShotsLife;
	    if (debris->life * speed > World.hypotenuse)
		debris->life = World.hypotenuse / speed;
	}

	dir = real_dir + (rand()%(1+spread)) - spreadoffset - 1;
	debris->color = color;
	debris->id = obj_id;
	debris->pos.x = x;
	debris->pos.y = y;
	debris->prevpos = debris->pos;
	debris->vel.x = vx + (tcos(dir) * speed);
	debris->vel.y = vy + (tsin(dir) * speed);
	debris->status = status;
	debris->acc.x = debris->acc.y = 0;
	debris->dir = dir;
	debris->type = type;
	debris->pl_range = 0;
	debris->pl_radius = 0;
    }
}


/* Explode a fighter */
void Explode(int ind)
{
    player *pl;
    object *debris;
    int i, dir, num_debris, speed;
    float x, y;

    pl = Players[ind];

    x = pl->pos.x;
    y = pl->pos.y;
    if (BIT(World.rules->mode, WRAP_PLAY)) {
	if (x < 0) x += World.width;
	else if (x >= World.width) x -= World.width;
	if (y < 0) y += World.height;
	else if (y >= World.height) y -= World.height;
    }
    if (x < 0 || x >= World.width || y < 0 || y >= World.height) {
	return;
    }

    sound_play_sensors(pl->pos.x, pl->pos.y, PLAYER_EXPLOSION_SOUND);
    num_debris = 1+(pl->fuel.sum/(8.0*FUEL_SCALE_FACT))
                 +(rand()%((int)(1+pl->mass*4.0)));
    if (num_debris > MAX_TOTAL_SHOTS - NumObjs) {
	num_debris = MAX_TOTAL_SHOTS - NumObjs;
    }
    for (i = 0; i < num_debris; i++, NumObjs++) {
	debris = Obj[NumObjs];
	dir = rand()%RES;
	speed = PL_DEBRIS_SPEED(pl->mass);
	debris->color=RED;
	debris->id = pl->id;
	debris->pos.x = x;
	debris->pos.y = y;
	debris->prevpos = debris->pos;
	debris->vel.x = pl->vel.x + (tcos(dir) * speed);
	debris->vel.y = pl->vel.y + (tsin(dir) * speed);
	debris->status = GRAVITY;
	debris->acc.x = debris->acc.y = 0;
	debris->dir = dir;
	debris->mass = PL_DEBRIS_MASS;
	debris->type = OBJ_DEBRIS;
	debris->life = PL_DEBRIS_LIFE(pl->mass);
	debris->pl_range = 0;
	debris->pl_radius = 0;
    }
}

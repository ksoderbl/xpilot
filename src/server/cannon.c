/* $Id: cannon.c,v 5.15 2002/01/27 23:26:55 kimiko Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
 *  	Kimiko Koopman
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
#include "bit.h"
#include "objpos.h"
#include "cannon.h"
#include "saudio.h"
#include "commonproto.h"

char cannon_version[] = VERSION;


#ifdef	SOUND
#define IFSOUND(__x)	__x
#else
#define IFSOUND(__x)
#endif

static int Cannon_select_weapon(int ind);
static void Cannon_aim(int ind, int weapon, int *target, int *dir);
static void Cannon_fire(int ind, int weapon, int target, int dir);
static int Cannon_in_danger(int ind);
static int Cannon_select_defense(int ind);
static void Cannon_defend(int ind, int defense);

/* the items that are useful to cannons.
   these are the items that cannon get 'for free' once in a while.
   cannons can get other items, but only by picking them up or
   stealing them from players. */
long CANNON_USE_ITEM = (ITEM_BIT_FUEL|ITEM_BIT_WIDEANGLE
		    	|ITEM_BIT_REARSHOT|ITEM_BIT_AFTERBURNER
			|ITEM_BIT_SENSOR|ITEM_BIT_TRANSPORTER
			|ITEM_BIT_TANK|ITEM_BIT_MINE
			|ITEM_BIT_ECM|ITEM_BIT_LASER
			|ITEM_BIT_EMERGENCY_THRUST|ITEM_BIT_ARMOR
			|ITEM_BIT_TRACTOR_BEAM|ITEM_BIT_MISSILE
			|ITEM_BIT_PHASING);

/* adds the given amount of an item to the cannon's inventory. the number of
   tanks is taken to be 1. amount is then the amount of fuel in that tank.
   fuel is given in 'units', but is stored in fuelpacks. */
void Cannon_add_item(int ind, int item, int amount)
{
    cannon_t	*c = World.cannon + ind;

    switch (item) {
    case ITEM_TANK:
	c->item[ITEM_TANK]++;
	LIMIT(c->item[ITEM_TANK], 0, World.items[ITEM_TANK].limit);
	/* FALLTHROUGH */
    case ITEM_FUEL:
	c->item[ITEM_FUEL] += (int)(amount / (ENERGY_PACK_FUEL >> FUEL_SCALE_BITS)
			      + 0.5);
	LIMIT(c->item[ITEM_FUEL], 0, (int)(World.items[ITEM_FUEL].limit
				     / (ENERGY_PACK_FUEL >> FUEL_SCALE_BITS)
				     + 0.5));
	break;
    default:
	c->item[item] += amount;
	LIMIT(c->item[item], 0, World.items[item].limit);
	break;
    }
}

void Cannon_throw_items(int ind)
{
    cannon_t	*c = World.cannon + ind;
    int		i, dir;
    object	*obj;
    DFLOAT	velocity;

    for (i = 0; i < NUM_ITEMS; i++) {
	if (i == ITEM_FUEL)
	    continue;
	c->item[i] -= World.items[i].initial;
	while (c->item[i] > 0) {
	    int amount = World.items[i].max_per_pack
			 - (int)(rfrac() * (1 + World.items[i].max_per_pack
					    - World.items[i].min_per_pack));
	    LIMIT(amount, 0, c->item[i]);
	    if (rfrac() < (dropItemOnKillProb * CANNON_DROP_ITEM_PROB)
		&& (obj = Object_allocate()) != NULL) {

		obj->type = OBJ_ITEM;
		obj->info = i;
		obj->color = RED;
		obj->status = GRAVITY;
		dir = (int)(c->dir
			   - (CANNON_SPREAD * 0.5)
			   + (rfrac() * CANNON_SPREAD));
		dir = MOD2(dir, RES);
		obj->id = NO_ID;
		obj->team = TEAM_NOT_SET;
		Object_position_init_pixels(obj, c->pix_pos.x, c->pix_pos.y);
		velocity = rfrac() * 6;
		obj->vel.x = tcos(dir) * velocity;
		obj->vel.y = tsin(dir) * velocity;
		obj->acc.x = 0;
		obj->acc.y = 0;
		obj->mass = 10;
		obj->life = 1500 + (int)(rfrac() * 512);
		obj->count = amount;
		obj->pl_range = ITEM_SIZE / 2;
		obj->pl_radius = ITEM_SIZE / 2;
		World.items[i].num++;
		Cell_add_object(obj);
	    }
	    c->item[i] -= amount;
	}
    }
}

/* initializes the given cannon at startup or after death and gives it some
   items. */
void Cannon_init(int ind)
{
    cannon_t	*c = World.cannon + ind;
    int		i;

    c->last_change = frame_loops;
    for (i = 0; i < NUM_ITEMS; i++) {
	c->item[i] = 0;
	if (cannonsUseItems)
	    Cannon_add_item(ind, i, (int)(rfrac() * (World.items[i].initial + 1)));
    }
    c->damaged = 0;
    c->tractor_target = -1;
    c->tractor_count = 0;
    c->tractor_is_pressor = false;
    c->used = 0;
    c->emergency_shield_left = 0;
    c->phasing_left = 0;
}

void Cannon_check_defense(int ind)
{
    int defense = Cannon_select_defense(ind);

    if (defense >= 0
	&& Cannon_in_danger(ind)) {
	Cannon_defend(ind, defense);
    }
}

void Cannon_check_fire(int ind)
{
    int	target = -1,
    	dir = 0,
	weapon = Cannon_select_weapon(ind);

    Cannon_aim(ind, weapon, &target, &dir);
    if (target != -1)
	Cannon_fire(ind, weapon, target, dir);
}

/* selects one of the available defenses. see cannon.h for descriptions. */
static int Cannon_select_defense(int ind)
{
    cannon_t	*c = World.cannon + ind;

    if (cannonSmartness == 0)
	return -1;	/* mode 0 does not defend */
    if (BIT(c->used, HAS_EMERGENCY_SHIELD)
	|| BIT(c->used, HAS_PHASING_DEVICE))
	return -1;	/* still protected */
    if (c->item[ITEM_EMERGENCY_SHIELD])
	return CD_EM_SHIELD;
    if (c->item[ITEM_PHASING])
	return CD_PHASING;
    return -1;	/* no defense available */
}

/* checks if a cannon is about to be hit by a hazardous object.
   mode 0 does not detect danger.
   modes 1 - 3 use progressively more accurate detection. */
static int Cannon_in_danger(int ind)
{
    cannon_t	*c = World.cannon + ind;
    const int	range = 4 * BLOCK_SZ;
    const long	kill_shots = (KILLING_SHOTS) | OBJ_MINE | OBJ_SHOT
	    			| OBJ_PULSE | OBJ_SMART_SHOT | OBJ_HEAT_SHOT
				| OBJ_TORPEDO | OBJ_ASTEROID;
    object	*shot, **obj_list;
    const int	max_objs = 100;
    int		obj_count, i, danger = false;
    int		npx, npy, tdx, tdy;
    int		cpx = (int)c->pix_pos.x, cpy = (int)c->pix_pos.y;

    if (cannonSmartness == 0)
	return false;

    Cell_get_objects(c->blk_pos.x, c->blk_pos.y, range, max_objs,
		     &obj_list, &obj_count);

    for (i = 0; (i < obj_count) && !danger; i++) {
	shot = obj_list[i];

	if (shot->life <= 0)
	    continue;
	if (!BIT(shot->type, kill_shots))
	    continue;
	if (BIT(shot->status, FROMCANNON))
	    continue;
	if (BIT(World.rules->mode, TEAM_PLAY)
	    && teamImmunity
	    && shot->team == c->team)
	    continue;

	npx = shot->pos.x;
	npy = shot->pos.y;
	if (cannonSmartness > 1) {
	    npx += shot->vel.x;
	    npy += shot->vel.y;
	    if (cannonSmartness > 2) {
		npx += shot->acc.x;
		npy += shot->acc.y;
	    }
	}
	tdx = WRAP_DX(npx - cpx);
	tdy = WRAP_DY(npy - cpy);
	if (LENGTH(tdx, tdy) <= ((4.5 - cannonSmartness) * BLOCK_SZ)) {
	    danger = true;
	    break;
	}
    }

    return danger;
}

/* activates the selected defense. */
static void Cannon_defend(int ind, int defense)
{
    cannon_t	*c = World.cannon + ind;
    IFSOUND( int sound = -1; )

    switch (defense) {
    case CD_EM_SHIELD:
	c->emergency_shield_left += 4 * FPS;
	SET_BIT(c->used, HAS_EMERGENCY_SHIELD);
	c->item[ITEM_EMERGENCY_SHIELD]--;
	IFSOUND( sound = EMERGENCY_SHIELD_ON_SOUND; )
	break;
    case CD_PHASING:
	c->phasing_left += 4 * FPS;
	SET_BIT(c->used, HAS_PHASING_DEVICE);
	c->tractor_count = 0;
	c->item[ITEM_PHASING]--;
	IFSOUND( sound = PHASING_ON_SOUND; )
	break;
    }
    IFSOUND( if (sound != -1)
		 sound_play_sensors(c->pix_pos.x, c->pix_pos.y, sound); )
}

/* selects one of the available weapons. see cannon.h for descriptions. */
static int Cannon_select_weapon(int ind)
{
    cannon_t	*c = World.cannon + ind;

    if (c->item[ITEM_MINE]
	&& rfrac() < 0.5f)
	return CW_MINE;
    if (c->item[ITEM_MISSILE]
	&& rfrac() < 0.5f)
	return CW_MISSILE;
    if (c->item[ITEM_LASER]
	&& (int)(rfrac() * (c->item[ITEM_LASER] + 1)))
	return CW_LASER;
    if (c->item[ITEM_ECM]
	&& rfrac() < 0.333f)
	return CW_ECM;
    if (c->item[ITEM_TRACTOR_BEAM]
	&& rfrac() < 0.5f)
	return CW_TRACTORBEAM;
    if (c->item[ITEM_TRANSPORTER]
	&& rfrac() < 0.333f)
	return CW_TRANSPORTER;
    if ((c->item[ITEM_AFTERBURNER]
	 || c->item[ITEM_EMERGENCY_THRUST])
	&& c->item[ITEM_FUEL]
	&& (int)(rfrac() * ((c->item[ITEM_EMERGENCY_THRUST] ?
		      MAX_AFTERBURNER :
		      c->item[ITEM_AFTERBURNER]) + 3)) > 2)
	return CW_GASJET;
    return CW_SHOT;
}

/* determines in which direction to fire.
   mode 0 fires straight ahead.
   mode 1 in a random direction.
   mode 2 aims at the current position of the closest player,
          then limits that to the sector in front of the cannon,
          then adds a small error.
   mode 3 calculates where the player will be when the shot reaches her,
          checks if that position is within limits and selects the player
          who will be closest in this way.
   the targeted player is also returned (for all modes).
   mode 0 always fires if it sees a player.
   modes 1 and 2 only fire if a player is within range of the selected weapon.
   mode 3 only fires if a player will be in range when the shot is expected to hit.
 */
static void Cannon_aim(int ind, int weapon, int *target, int *dir)
{
    cannon_t	*c = World.cannon + ind;
    int		speed = ShotsSpeed;
    int		range = CANNON_SHOT_LIFE_MAX * speed;
    int		cpx = (int)c->pix_pos.x;
    int		cpy = (int)c->pix_pos.y;
    int		visualrange = (int)(CANNON_DISTANCE
			      + 2 * c->item[ITEM_SENSOR] * BLOCK_SZ);
    bool	found = false, ready = false;
    int		closest = range, i;
    int		ddir;

    switch (weapon) {
    case CW_MINE:
	speed = (int)(speed * 0.5 + 0.1 * cannonSmartness);
	range = (int)(range * 0.5 + 0.1 * cannonSmartness);
	break;
    case CW_LASER:
	speed = PULSE_SPEED;
	range = (int)(PULSE_LIFE(CANNON_PULSES) * speed);
	break;
    case CW_ECM:
	/* smarter cannons wait a little longer before firing an ECM */
	if (cannonSmartness > 1) {
	    range = (int)((ECM_DISTANCE / cannonSmartness
		     + (int)(rfrac() * (int)(ECM_DISTANCE
				       - ECM_DISTANCE / cannonSmartness))));
	} else {
	    range = (int)ECM_DISTANCE;
	}
	break;
    case CW_TRACTORBEAM:
	range = TRACTOR_MAX_RANGE(c->item[ITEM_TRACTOR_BEAM]);
	break;
    case CW_TRANSPORTER:
	/* smarter cannons have a smaller chance of using a transporter when
	   target is out of range */
	if (cannonSmartness > 2
	    || (int)(rfrac() * sqr(cannonSmartness + 1)))
	    range = (int)TRANSPORTER_DISTANCE;
	break;
    case CW_GASJET:
	if (c->item[ITEM_EMERGENCY_THRUST]) {
	    speed *= 2;
	    range *= 2;
	}
	break;
    }

    for (i = 0; i < NumPlayers && !ready; i++) {
	player *pl = Players[i];
	int tdist, tdx, tdy;

	tdx = WRAP_DX(pl->pos.x - cpx);
	if (ABS(tdx) >= visualrange)
	    continue;
	tdy = WRAP_DY(pl->pos.y - cpy);
	if (ABS(tdy) >= visualrange)
	    continue;
	tdist = (int)LENGTH(tdx, tdy);
	if (tdist > visualrange)
	    continue;

	/* mode 3 also checks if a player is using a phasing device */
	if (BIT(pl->status, PLAYING|GAME_OVER|PAUSE|KILLED) != PLAYING
	    || (BIT(World.rules->mode, TEAM_PLAY)
		&& pl->team == c->team)
	    || (!pl->forceVisible
		&& BIT(pl->used, HAS_CLOAKING_DEVICE)
		&& (int)(rfrac() * (pl->item[ITEM_CLOAK] + 1))
		   > (int)(rfrac() * (c->item[ITEM_SENSOR] + 1)))
	    || (cannonSmartness > 2
		&& BIT(pl->used, HAS_PHASING_DEVICE)))
	    continue;

	switch (cannonSmartness) {
	case 0:
	    ready = true;
	    break;
	default:
	case 1:
	    if (tdist < range)
		ready = true;
	    break;
	case 2:
	    if (tdist < closest) {
		*dir = (int)findDir(tdx, tdy);
		found = true;
	    }
	    break;
	case 3:
	    if (tdist < range) {
		DFLOAT time = tdist / speed;
		int npx = (int)(pl->pos.x
				+ pl->vel.x * time
				+ pl->acc.x * time * time);
		int npy = (int)(pl->pos.y
				+ pl->vel.y * time
				+ pl->acc.y * time * time);
		int tdir;

		tdx = WRAP_DX(npx - cpx);
		tdy = WRAP_DY(npy - cpy);
		tdir = (int)findDir(tdx, tdy);
		ddir = MOD2(tdir - c->dir, RES);
		if ((ddir < (CANNON_SPREAD * 0.5)
		     || ddir > RES - (CANNON_SPREAD * 0.5))
		    && (int)LENGTH(tdx, tdy) < closest) {
		    *dir = tdir;
		    found = true;
		}
	    }
	    break;
	}
	if (found || ready) {
	    closest = tdist;
	    *target = i;
	}
    }
    if (!(found || ready)) {
	*target = -1;
	return;
    }

    switch (cannonSmartness) {
    case 0:
	*dir = c->dir;
	break;
    default:
    case 1:
	*dir = c->dir;
	*dir += (int)((rfrac() - 0.5f) * CANNON_SPREAD);
	break;
    case 2:
	ddir = MOD2(*dir - c->dir, RES);
	if (ddir > (CANNON_SPREAD * 0.5) && ddir < RES / 2) {
	    *dir = (int)(c->dir + (CANNON_SPREAD * 0.5) + 3);
	} else if (ddir < RES - (CANNON_SPREAD * 0.5) && ddir > RES / 2) {
	    *dir = (int)(c->dir - (CANNON_SPREAD * 0.5) - 3);
	}
	*dir += (int)(rfrac() * 7) - 3;
	break;
    case 3:
	/* nothing to be done for mode 3 */
	break;
    }
    *dir = MOD2(*dir, RES);
}

/* does the actual firing. also determines in which way to use weapons that
   have more than one possible use. */
static void Cannon_fire(int ind, int weapon, int target, int dir)
{
    cannon_t	*c = World.cannon + ind;
    player	*pl = Players[target];
    int		cpx = (int)c->pix_pos.x;
    int		cpy = (int)c->pix_pos.y;
    modifiers	mods;
    IFSOUND (int sound = CANNON_FIRE_SOUND;)
    int		i;
    int		speed = ShotsSpeed;

    CLEAR_MODS(mods);
    switch (weapon) {
    case CW_MINE:
	if (BIT(World.rules->mode, ALLOW_CLUSTERS) && (rfrac() < 0.25f))
	    SET_BIT(mods.warhead, CLUSTER);
	if (BIT(World.rules->mode, ALLOW_MODIFIERS)) {
	    if (rfrac() >= 0.2f)
		SET_BIT(mods.warhead, IMPLOSION);
	    mods.power = (int)(rfrac() * (MODS_POWER_MAX + 1));
	    mods.velocity = (int)(rfrac() * (MODS_VELOCITY_MAX + 1));
	}
	if (rfrac() < 0.5f) {	/* place mine in front of cannon */
	    Place_general_mine(-1, c->team, FROMCANNON, cpx, cpy,
			       0, 0, mods);
	    IFSOUND (sound = DROP_MINE_SOUND; )
	} else {		/* throw mine at player */
	    if (BIT(World.rules->mode, ALLOW_MODIFIERS)) {
		mods.mini = (int)(rfrac() * MODS_MINI_MAX) + 1;
		mods.spread = (int)(rfrac() * (MODS_SPREAD_MAX + 1));
	    }
	    speed = (int)(speed * 0.5 + 0.1 * cannonSmartness);
	    Place_general_mine(-1, c->team, GRAVITY|FROMCANNON, cpx, cpy,
			       tcos(dir) * speed, tsin(dir) * speed, mods);
	    IFSOUND(sound = DROP_MOVING_MINE_SOUND;)
	}
	c->item[ITEM_MINE]--;
	break;
    case CW_MISSILE:
	if (BIT(World.rules->mode, ALLOW_CLUSTERS) && (rfrac() < 0.333f))
	    SET_BIT(mods.warhead, CLUSTER);
	if (BIT(World.rules->mode, ALLOW_MODIFIERS)) {
	    if (rfrac() >= 0.25f)
		SET_BIT(mods.warhead, IMPLOSION);
	    mods.power = (int)(rfrac() * (MODS_POWER_MAX + 1));
	    mods.velocity = (int)(rfrac() * (MODS_VELOCITY_MAX + 1));
	    /* Because cannons don't have missile racks, all mini missiles
	       would be fired from the same point and appear to the players
	       as 1 missile (except heatseekers, which would appear to split
	       in midair because of navigation errors (see Move_smart_shot)).
	       Therefore, we don't minify cannon missiles.
	    mods.mini = (int)(rfrac() * MODS_MINI_MAX) + 1;
	    mods.spread = (int)(rfrac() * (MODS_SPREAD_MAX + 1));
	    */
	}
	/* smarter cannons use more advanced missile types */
	switch ((int)(rfrac() * (1 + cannonSmartness))) {
	default:
	    if (allowSmartMissiles) {
		Fire_general_shot(-1, c->team, 1, cpx, cpy, OBJ_SMART_SHOT,
				  dir, mods, target);
		IFSOUND(sound = FIRE_SMART_SHOT_SOUND;)
		break;
	    }
	    /* FALLTHROUGH */
	case 1:
	    if (allowHeatSeekers
		&& BIT(Players[target]->status, THRUSTING)) {
		Fire_general_shot(-1, c->team, 1, cpx, cpy, OBJ_HEAT_SHOT,
				  dir, mods, target);
		IFSOUND(sound = FIRE_HEAT_SHOT_SOUND;)
		break;
	    }
	    /* FALLTHROUGH */
	case 0:
	    Fire_general_shot(-1, c->team, 1, cpx, cpy, OBJ_TORPEDO,
			      dir, mods, -1);
	    IFSOUND(sound = FIRE_TORPEDO_SOUND;)
	    break;
	}
	c->item[ITEM_MISSILE]--;
	break;
    case CW_LASER:
	/* stun and blinding lasers are very dangerous,
	   so we don't use them often */
	if (BIT(World.rules->mode, ALLOW_LASER_MODIFIERS)
	    && (rfrac() * (8 - cannonSmartness)) >= 1) {
	    mods.laser = (int)(rfrac() * (MODS_LASER_MAX + 1));
	}
	Fire_general_laser(-1, c->team, cpx, cpy, dir, mods);
	IFSOUND(sound = FIRE_LASER_SOUND;)
	break;
    case CW_ECM:
	Fire_general_ecm(-1, c->team, cpx, cpy);
	c->item[ITEM_ECM]--;
	IFSOUND(sound = ECM_SOUND;)
	break;
    case CW_TRACTORBEAM:
	/* smarter cannons use tractors more often and also push/pull longer */
	c->tractor_is_pressor = (rfrac() * (cannonSmartness + 1) >= 1);
	c->tractor_target = pl->id;
	c->tractor_count = 11 + (int)(rfrac() * ((3 * cannonSmartness) + 1));
	IFSOUND(sound = -1;)
	break;
    case CW_TRANSPORTER:
	c->item[ITEM_TRANSPORTER]--;
	if ((int)Wrap_length(pl->pos.x - cpx, pl->pos.y - cpy)
	    < TRANSPORTER_DISTANCE) {
	    int item = -1;
	    long amount = 0;
	    Do_general_transporter(-1, cpx, cpy, target, &item, &amount);
	    if (item != -1)
		Cannon_add_item(ind, item, amount);
	    IFSOUND(sound = -1;)
	} else {
	    IFSOUND(sound = TRANSPORTER_FAIL_SOUND;)
	}
	break;
    case CW_GASJET:
	/* use emergency thrusts to make extra big jets */
	if ((rfrac() * (c->item[ITEM_EMERGENCY_THRUST] + 1)) >= 1) {
	    Make_debris(
		/* pos */	cpx, cpy,
		/* vel */	0, 0,
		/* id */	NO_ID,
		/* team */	c->team,
		/* type */	OBJ_SPARK,
		/* mass */	THRUST_MASS,
		/* status */	GRAVITY|FROMCANNON,
		/* color */	RED,
		/* radius */	8,
		/* number */	300, 700,
		/* dir */	dir - 4 * (4 - cannonSmartness),
				dir + 4 * (4 - cannonSmartness),
		/* speed */	0.1, speed * 4,
		/* life */	3, 20);
	    c->item[ITEM_EMERGENCY_THRUST]--;
	} else {
	    Make_debris(
		/* pos */	cpx, cpy,
		/* vel */	0, 0,
		/* id */	NO_ID,
		/* team */	c->team,
		/* type */	OBJ_SPARK,
		/* mass */	THRUST_MASS,
		/* status */	GRAVITY|FROMCANNON,
		/* color */	RED,
		/* radius */	8,
		/* number */	150, 350,
		/* dir */	dir - 3 * (4 - cannonSmartness),
				dir + 3 * (4 - cannonSmartness),
		/* speed */	0.1, speed * 2,
		/* life */	3, 20);
	}
	c->item[ITEM_FUEL]--;
	IFSOUND(sound = THRUST_SOUND;)
	break;
    case CW_SHOT:
    default:
	if (cannonFlak)
	    mods.warhead = CLUSTER;
	/* smarter cannons fire more accurately and
	   can therefore narrow their bullet streams */
	for (i = 0; i < (1 + 2 * c->item[ITEM_WIDEANGLE]); i++) {
	    int a_dir = dir
			+ (4 - cannonSmartness)
			* (-c->item[ITEM_WIDEANGLE] +  i);
	    a_dir = MOD2(a_dir, RES);
	    Fire_general_shot(-1, c->team, 1, cpx, cpy, OBJ_CANNON_SHOT,
			      a_dir, mods, -1);
	}
	/* I'm not sure cannons should use rearshots.
	   After all, they are restricted to 60 degrees when picking their
	   target. */
	for (i = 0; i < c->item[ITEM_REARSHOT]; i++) {
	    int a_dir = (int)(dir + (RES / 2)
			+ (4 - cannonSmartness)
			* (-((c->item[ITEM_REARSHOT] - 1) * 0.5) + i));
	    a_dir = MOD2(a_dir, RES);
	    Fire_general_shot(-1, c->team, 1, cpx, cpy, OBJ_CANNON_SHOT,
			      a_dir, mods, -1);
	}
    }

    /* finally, play sound effect */
    IFSOUND(if (sound != -1)
	            sound_play_sensors(cpx, cpy, sound);)
}

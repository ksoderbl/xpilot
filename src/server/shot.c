/* $Id: shot.c,v 4.9 2000/03/24 14:21:52 bert Exp $
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
#include <limits.h>
#else
#include <stdlib.h>
#include <string.h>
#endif

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
#include "cannon.h"
#include "objpos.h"
#include "netserver.h"
#include "error.h"

char shot_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: shot.c,v 4.9 2000/03/24 14:21:52 bert Exp $";
#endif

#define MISSILE_POWER_SPEED_FACT	0.25
#define MISSILE_POWER_TURNSPEED_FACT	0.75
#define MINI_TORPEDO_SPREAD_TIME	6
#define MINI_TORPEDO_SPREAD_SPEED	20
#define MINI_TORPEDO_SPREAD_ANGLE	90
#define MINI_MINE_SPREAD_TIME		18
#define MINI_MINE_SPREAD_SPEED		8
#define MINI_MISSILE_SPREAD_ANGLE	45

#if 0
extern unsigned SPACE_BLOCKS;

extern int Rate(int winner, int loser);
#endif

#define CONFUSED_UPDATE_GRANULARITY	10
#define CONFUSED_TIME			3


/***********************
 * Functions for shots.
 */

static object *objArray;

void Alloc_shots(int number)
{
    object		*x;
    int			i;

    x = (object *) calloc(number, sizeof(object));
    if (!x) {
	error("Not enough memory for shots.");
	exit(1);
    }

    objArray = x;
    for (i = 0; i < number; i++) {
	x->owner = -1;
	Obj[i] = x++;
    }
}


void Free_shots(void)
{
    if (objArray != NULL) {
	free(objArray);
	objArray = NULL;
    }
}


void Place_mine(int ind)
{
    player *pl = Players[ind];

    if (pl->item[ITEM_MINE] <= 0
	|| (BIT(pl->used, OBJ_SHIELD|OBJ_PHASING_DEVICE) && !shieldedMining))
	return;

    Place_general_mine(ind, pl->team, 0,
		       pl->pos.x, pl->pos.y, 0.0, 0.0, pl->mods);
}


void Place_moving_mine(int ind)
{
    player *pl = Players[ind];

    if (pl->item[ITEM_MINE] <= 0
	|| (BIT(pl->used, OBJ_SHIELD|OBJ_PHASING_DEVICE) && !shieldedMining))
	return;

    Place_general_mine(ind, pl->team, GRAVITY,
		       pl->pos.x, pl->pos.y, pl->vel.x, pl->vel.y, pl->mods);
}

void Place_general_mine(int ind, u_short team, long status, DFLOAT x, DFLOAT y,
			DFLOAT vx, DFLOAT vy, modifiers mods)
{
    char		msg[MSG_LEN];
    player		*pl = (ind == -1 ? NULL : Players[ind]);
    int			used, life;
    long		drain;
    DFLOAT		mass;
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

    if (pl && BIT(pl->status, KILLED)) {
	life = (int)(rfrac() * FPS);
    } else if (BIT(status, FROMCANNON)) {
	life = CANNON_SHOT_LIFE;
    } else {
	life = (mineLife ? mineLife : MINE_LIFETIME);
    }

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
	mass = (BIT(status, FROMCANNON) ? MINE_MASS * 0.6 : MINE_MASS);
	used = 1;
    }

    if (pl) {
	drain = ED_MINE;
	if (BIT(mods.warhead, CLUSTER)) {
	    drain += (long)(CLUSTER_MASS_DRAIN(mass));
	}
	if (pl->fuel.sum < -drain) {
	    sprintf(msg, "You need at least %ld fuel units to %s %s!",
		    (-drain) >> FUEL_SCALE_BITS,
		    (BIT(status, GRAVITY) ? "throw" : "drop"),
		    Describe_shot(OBJ_MINE, status, mods, 0));
	    Set_player_message (pl, msg);
	    return;
	}
	if (baseMineRange) {
	    for (i = 0; i < NumPlayers; i++) {
		if (i != ind
		    && !TEAM_IMMUNE(i, ind)
		    && !IS_TANK_IND(i)) {
		    int dx = (int)(x/BLOCK_SZ - World.base[Players[i]->home_base].pos.x);
		    int dy = (int)(y/BLOCK_SZ - World.base[Players[i]->home_base].pos.y);
		    if (sqr(dx) + sqr(dy) <= sqr(baseMineRange)) {
			Set_player_message(pl, "No base mining!");
			return;
		    }
		}
	    }
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
	mine->dir = 0;
	mine->id = (pl ? pl->id : -1);
	mine->team = team;
	mine->owner = mine->id;
	Object_position_init_pixels(mine, x, y);
	if (minis > 1) {
	    int		space = RES/minis;
	    int		dir;
	    DFLOAT	spread;

	    spread = (DFLOAT)((unsigned)mods.spread + 1);
	    /*
	     * Dir gives (S is ship upwards);
	     *
	     *			      o		    o   o
	     *	X2: o S	o	X3:   S		X4:   S
	     *			    o   o	    o   o
	     */
	    dir = (i * space) + space/2 + (minis-2)*(RES/2) + (pl?pl->dir:0);
	    dir += (int)((rfrac() - 0.5f) * space * 0.5f);
	    dir = MOD2(dir, RES);
	    mv.x = MINI_MINE_SPREAD_SPEED * tcos(dir) / spread;
	    mv.y = MINI_MINE_SPREAD_SPEED * tsin(dir) / spread;
	    /*
	     * This causes the added initial velocity to reduce to
	     * zero over the MINI_MINE_SPREAD_TIME.
	     */
	    mine->spread_left = MINI_MINE_SPREAD_TIME;
	    mine->acc.x = -mv.x / MINI_MINE_SPREAD_TIME;
	    mine->acc.y = -mv.y / MINI_MINE_SPREAD_TIME;
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
	mine->pl_range = (int)(MINE_RANGE / minis);
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
    DFLOAT		dist;
    DFLOAT		min_dist = World.hypotenuse + 1;

    if (BIT(pl->used, OBJ_PHASING_DEVICE))
	return;

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

void Make_treasure_ball(int treasure)
{
    object *ball;
    treasure_t *t = &(World.treasures[treasure]);
   DFLOAT	x = (t->pos.x + 0.5) * BLOCK_SZ,
		y = (t->pos.y * BLOCK_SZ) + 10;

    if (t->have) {
	xpprintf ("%s Failed Make_treasure_ball(treasure=%d):\n", showtime(), treasure);
	xpprintf ("\ttreasure: destroyed = %d, team = %d, have = %d\n",
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
    ball->acc.x = 0;
    ball->acc.y = 0;
    ball->dir = 0;
    Object_position_init_pixels(ball, x, y);
    ball->id = ball->owner = -1;
    ball->team = t->team;
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


/*
 * Describes shot of `type' which has `status' and `mods'.  If `hit' is
 * non-zero this description is part of a collision, otherwise its part
 * of a launch message.
 */
char *Describe_shot(int type, long status, modifiers mods, int hit)
{
    const char		*name, *howmany = "a ", *plural = "";
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
	name = "heatseeker";
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
    DFLOAT x,
	  y;

    if (pl->shots >= pl->shot_max || BIT(pl->used, OBJ_SHIELD|OBJ_PHASING_DEVICE))
	return;

    x = pl->pos.x + pl->ship->m_gun[pl->dir].x;
    y = pl->pos.y + pl->ship->m_gun[pl->dir].y;

    Fire_general_shot(ind, pl->team, 0, x, y, type, dir,
		      pl->shot_speed, pl->mods, -1);
}

void Fire_shot(int ind, int type, int dir)
{
    player *pl = Players[ind];

    if (pl->shots >= pl->shot_max || BIT(pl->used, OBJ_SHIELD|OBJ_PHASING_DEVICE))
	return;

    Fire_general_shot(ind, pl->team, 0, pl->pos.x, pl->pos.y,
		      type, dir, pl->shot_speed, pl->mods, -1);
}

void Fire_left_shot(int ind, int type, int dir, int gun)
{
    player *pl = Players[ind];
    DFLOAT x,
	  y;

    if (pl->shots >= pl->shot_max || BIT(pl->used, OBJ_SHIELD|OBJ_PHASING_DEVICE))
	return;

    x = pl->pos.x + pl->ship->l_gun[gun][pl->dir].x;
    y = pl->pos.y + pl->ship->l_gun[gun][pl->dir].y;

    Fire_general_shot(ind, pl->team, 0, x, y, type, dir,
		      pl->shot_speed, pl->mods, -1);

}

void Fire_right_shot(int ind, int type, int dir, int gun)
{
    player *pl = Players[ind];
    DFLOAT x,
	  y;

    if (pl->shots >= pl->shot_max || BIT(pl->used, OBJ_SHIELD|OBJ_PHASING_DEVICE))
	return;

    x = pl->pos.x + pl->ship->r_gun[gun][pl->dir].x;
    y = pl->pos.y + pl->ship->r_gun[gun][pl->dir].y;

    Fire_general_shot(ind, pl->team, 0, x, y, type, dir,
		      pl->shot_speed, pl->mods, -1);

}

void Fire_left_rshot(int ind, int type, int dir, int gun)
{
    player *pl = Players[ind];
    DFLOAT x,
	  y;

    if (pl->shots >= pl->shot_max || BIT(pl->used, OBJ_SHIELD|OBJ_PHASING_DEVICE))
	return;

    x = pl->pos.x + pl->ship->l_rgun[gun][pl->dir].x;
    y = pl->pos.y + pl->ship->l_rgun[gun][pl->dir].y;

    Fire_general_shot(ind, pl->team, 0, x, y, type, dir,
		      pl->shot_speed, pl->mods, -1);

}

void Fire_right_rshot(int ind, int type, int dir, int gun)
{
    player *pl = Players[ind];
    DFLOAT x,
	  y;

    if (pl->shots >= pl->shot_max || BIT(pl->used, OBJ_SHIELD|OBJ_PHASING_DEVICE))
	return;

    x = pl->pos.x + pl->ship->r_rgun[gun][pl->dir].x;
    y = pl->pos.y + pl->ship->r_rgun[gun][pl->dir].y;

    Fire_general_shot(ind, pl->team, 0, x, y, type, dir,
		      pl->shot_speed, pl->mods, -1);

}

void Fire_general_shot(int ind, u_short team, bool cannon, DFLOAT x, DFLOAT y,
		       int type, int dir, DFLOAT speed, modifiers mods,
		       int target)
{
    char		msg[MSG_LEN];
    player		*pl = (ind == -1 ? NULL : Players[ind]);
    int			used, life, fuse = 0,
			lock = 0,
			status = GRAVITY,
			i, ldir, minis,
			pl_range,
			pl_radius,
			rack_no = 0,
			racks_left = 0,
			r,
			on_this_rack = 0,
			side = 0,
			fired = 0;
    long		drain;
    DFLOAT		mass,
			turnspeed = 0,
			max_speed = SPEED_LIMIT,
			angle,
			spread;
    vector		mv;
    position		shotpos;

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
	if (cannon) {
	    mass = CANNON_SHOT_MASS;
	    life = CANNON_SHOT_LIFE;
	    SET_BIT(status, FROMCANNON);
	} else {
	    mass = ShotsMass;
	    life = ShotsLife;
	}
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
	    Add_fuel(&(pl->fuel), (long)(ED_SHOT));
	    sound_play_sensors(pl->pos.x, pl->pos.y, FIRE_SHOT_SOUND);
	    pl->shots++;
	}
	if (!ShotsGravity) {
	    CLR_BIT(status, GRAVITY);
	}
	break;

    case OBJ_SMART_SHOT:
    case OBJ_HEAT_SHOT:
	if ((type == OBJ_HEAT_SHOT) ? !allowHeatSeekers : !allowSmartMissiles) {
	    if (allowTorpedoes) {
		type = OBJ_TORPEDO;
	    } else {
		return;
	    }
	}
	/* FALLTHROUGH */
    case OBJ_TORPEDO:
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
	    pl_range = (type == OBJ_TORPEDO) ? (int)NUKE_RANGE : MISSILE_RANGE;
	} else {
	    mass = MISSILE_MASS;
	    used = 1;
	    pl_range = (type == OBJ_TORPEDO) ? (int)TORPEDO_RANGE : MISSILE_RANGE;
	}
	pl_range /= mods.mini + 1;
	pl_radius = MISSILE_LEN;

	drain = used * ED_SMART_SHOT;
	if (BIT(mods.warhead, CLUSTER)) {
	    if (pl)
		drain += (long)(CLUSTER_MASS_DRAIN(mass));
	}

	if (pl && BIT(pl->status, KILLED)) {
	    life = (int)(rfrac() * FPS);
	} else if (!cannon) {
	    life = (missileLife ? missileLife : MISSILE_LIFETIME);
	}

	switch (type) {
	case OBJ_HEAT_SHOT:
#ifndef HEAT_LOCK
	    lock = -1;
#else  /* HEAT_LOCK */
	    if (pl == NULL) {
		lock = target;
	    } else {
		if (!BIT(pl->lock.tagged, LOCK_PLAYER)
		|| ((pl->lock.distance > pl->sensor_range)
		    && BIT(World.rules->mode, LIMITED_VISIBILITY))) {
		    lock = -1;
		} else {
		    lock = pl->lock.pl_id;
		}
	    }
#endif /* HEAT_LOCK */
	    if (pl) {
		sound_play_sensors(pl->pos.x, pl->pos.y, FIRE_HEAT_SHOT_SOUND);
	    }
	    max_speed = SMART_SHOT_MAX_SPEED * HEAT_SPEED_FACT;
	    turnspeed = SMART_TURNSPEED * HEAT_SPEED_FACT;
	    speed *= HEAT_SPEED_FACT;
	    break;

	case OBJ_SMART_SHOT:
	    if (pl == NULL) {
		lock = target;
	    } else {
		if (!BIT(pl->lock.tagged, LOCK_PLAYER)
		|| ((pl->lock.distance > pl->sensor_range)
		    && BIT(World.rules->mode, LIMITED_VISIBILITY))
		|| !pl->visibility[GetInd[pl->lock.pl_id]].canSee)
		    return;
		lock = pl->lock.pl_id;
	    }
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
    spread = (DFLOAT)((unsigned)mods.spread + 1);
    /*
     * Calculate the maximum time it would take to cross one ships width,
     * don't fuse the shot/missile/torpedo for the owner only until that
     * time passes.  This is a hack to stop various odd missile and shot
     * mounting points killing the player when they're firing.
     */
    fuse += (int)((2.0 * (DFLOAT)SHIP_SZ) / speed + 1.0);

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
	shot->team	= team;
	shot->owner	= -1;
	shot->color	= (pl ? pl->color : WHITE);

	shotpos.x	= x;
	shotpos.y	= y;
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
	    shotpos.x += pl->ship->m_rack[rack_no][pl->dir].x;
	    shotpos.y += pl->ship->m_rack[rack_no][pl->dir].y;
	    side = (int)(pl->ship->m_rack[rack_no][0].y);
	}
	shotpos.x = WRAP_XPIXEL(shotpos.x);
	shotpos.y = WRAP_YPIXEL(shotpos.y);
	if (shotpos.x < 0 || shotpos.x >= World.width
	    || shotpos.y < 0 || shotpos.y >= World.height) {
	    NumObjs--;
	    continue;
	}
	Object_position_init_pixels(shot, shotpos.x, shotpos.y);

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
		angle = (DFLOAT)(on_this_rack - 1 - 2 * r);
		angle /= (3.0 * (DFLOAT)(on_this_rack - 1));
	    }
	    angle += (DFLOAT)(2 * side) / (DFLOAT)(3 * SHIP_SZ);
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
	    shot->acc.x = -mv.x / MINI_TORPEDO_SPREAD_TIME;
	    shot->acc.y = -mv.y / MINI_TORPEDO_SPREAD_TIME;
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

    if (frame_loops < pl->shot_time + fireRepeatRate) {
	return;
    }
    pl->shot_time = frame_loops;

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
    DFLOAT		modv, speed_modv, life_modv, num_modv;
    DFLOAT		mass;

    switch (shot->type) {

    case OBJ_SPARK:
    case OBJ_DEBRIS:
    case OBJ_WRECKAGE:
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
	    if (!anaColDet){
		Make_debris(
		    /* pos.x, pos.y   */ shot->prevpos.x, shot->prevpos.y,
		    /* vel.x, vel.y   */ shot->vel.x, shot->vel.y,
		    /* owner id       */ shot->id,
		    /* owner team	  */ shot->team,
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
	    } else {
		Make_debris(
		    shot->prevpos.x, shot->prevpos.y, shot->vel.x, shot->vel.y,
		    shot->id, shot->team, OBJ_DEBRIS, DEBRIS_MASS, GRAVITY,
		    RED, 8, 10, 20, 0, RES-1, 10, 50, 10, 2*(FPS+15));
	    }

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
	if (BIT(shot->status, FROMCANNON)) {
	    status |= FROMCANNON;
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
	    intensity = (int)CLUSTER_MASS_SHOTS(shot->mass);
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
	    num_modv = num_modv / ((DFLOAT)(unsigned)shot->mods.mini + 1.0f);
	}

	if (BIT(shot->mods.nuclear, NUCLEAR)) {
	    if (shot->type == OBJ_MINE) {
		intensity = (int)(intensity * (NUKE_MINE_EXPL_MULT * shot->mass / MINE_MASS));
	    } else {
		intensity = (int)(intensity * (NUKE_SMART_EXPL_MULT * shot->mass / MISSILE_MASS));
	    }
	    intensity = (int)(intensity * (shot->mods.mini + 1) / SHOT_MULT(shot));
	}

	if (BIT(shot->mods.warhead, IMPLOSION)) {
	    /*intensity >>= 1;*/
	    mass = -mass;
	}

	if (BIT(shot->type, OBJ_TORPEDO|OBJ_HEAT_SHOT|OBJ_SMART_SHOT)) {
	    intensity /= (1 + shot->mods.power);
	}

	Make_debris(
	    /* pos.x, pos.y   */ shot->prevpos.x, shot->prevpos.y,
	    /* vel.x, vel.y   */ shot->vel.x, shot->vel.y,
	    /* owner id       */ shot->id,
	    /* owner team     */ shot->team,
	    /* kind           */ type,
	    /* mass           */ mass,
	    /* status         */ status,
	    /* color          */ color,
	    /* radius         */ 6,
	    /* min,max debris */ (int)(0.20f * intensity * num_modv),
				 (int)(0.30f * intensity * num_modv),
	    /* min,max dir    */ 0, RES-1,
#ifdef DRAINFACTOR
	    /* min,max speed  */ ShotsSpeed * speed_modv,
				 ShotsSpeed * 5 * speed_modv,
#else
	    /* min,max speed  */ 20 * speed_modv,
				 (intensity >> 2) * speed_modv,
#endif
	    /* min,max life   */ (int)(8 * life_modv),
				 (int)((intensity >> 1) * life_modv)
	    );
	break;

    case OBJ_SHOT:
	if (shot->id == -1
	    || BIT(shot->status, FROMCANNON)
	    || BIT(shot->mods.warhead, CLUSTER))
	    break;
	pl = Players[GetInd[shot->id]];
	if (shot->type == OBJ_SHOT) {
	    if (--pl->shots <= 0) {
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
	xpprintf("%s Delete_shot(): Unkown shot type %d.\n", showtime(), shot->type);
	break;
    }

    Obj[ind] = Obj[--NumObjs];
    Obj[NumObjs] = shot;

    if (addMine | addHeat) {
	CLEAR_MODS(mods);
	if (BIT(World.rules->mode, ALLOW_CLUSTERS) && (rfrac() <= 0.333f)) {
	    SET_BIT(mods.warhead, CLUSTER);
	}
	if (BIT(World.rules->mode, ALLOW_MODIFIERS) && (rfrac() <= 0.333f)) {
	    SET_BIT(mods.warhead, IMPLOSION);
	}
	if (BIT(World.rules->mode, ALLOW_MODIFIERS)) {
	    mods.velocity = (int)(rfrac() * (MODS_VELOCITY_MAX + 1));
	}
	if (BIT(World.rules->mode, ALLOW_MODIFIERS)) {
	    mods.power = (int)(rfrac() * (MODS_POWER_MAX + 1));
	}
	if (addMine) {
	    Place_general_mine (-1, TEAM_NOT_SET, ((rfrac() < 0.5f) ? GRAVITY : 0),
				shot->pos.x, shot->pos.y, 0.0, 0.0, mods);
	}
	else if (addHeat) {
	    Fire_general_shot (-1, TEAM_NOT_SET, 0, shot->pos.x, shot->pos.y,
			       OBJ_HEAT_SHOT, (int)(rfrac() * RES), 1.0f,
			       mods, -1);
	}
    }
    else if (addBall) {
	Make_treasure_ball(shot->treasure);
    }
}

void Fire_laser(int ind)
{
    player	*pl = Players[ind];
    DFLOAT	x, y;
    
    if (pl->item[ITEM_LASER] > pl->num_pulses
	&& pl->velocity < PULSE_SPEED - PULSE_SAMPLE_DISTANCE) {
	if (pl->fuel.sum <= -ED_LASER) {
	    CLR_BIT(pl->used, OBJ_LASER);
	} else {
	    x = pl->pos.x + pl->ship->m_gun[pl->dir].x + pl->vel.x;
	    y = pl->pos.y + pl->ship->m_gun[pl->dir].y + pl->vel.y;
	    x = WRAP_XPIXEL(x);
	    y = WRAP_YPIXEL(y);
	    if (x >= 0 && x < World.width && y >= 0 && y < World.height) {
		Fire_general_laser(ind, pl->team, x, y, pl->dir, pl->mods);
	    }
	}
    }
}

void Fire_general_laser(int ind, u_short team, DFLOAT x, DFLOAT y,
			int dir, modifiers mods)
{
    player		*pl = ((ind == -1) ? NULL : Players[ind]);
    pulse_t		*pulse;
    int			life;

    if (pl) {
	Add_fuel(&(pl->fuel), (long)ED_LASER);
	sound_play_sensors(x, y, FIRE_LASER_SOUND);
	life = (int)PULSE_LIFE(pl->item[ITEM_LASER]);
    } else {
	life = (int)PULSE_LIFE(CANNON_PULSES);
    }

    if (NumPulses >= MAX_TOTAL_PULSES) {
	return;
    }
    Pulses[NumPulses] = (pulse_t *)malloc(sizeof(pulse_t));
    if (Pulses[NumPulses] == NULL) {
	return;
    }

    pulse = Pulses[NumPulses];
    pulse->id = (pl ? pl->id : -1);
    pulse->team = team;
    pulse->dir = dir;
    pulse->len = PULSE_LENGTH;
    pulse->life = life;
    pulse->mods = mods;
    pulse->refl = false;
    pulse->pos.x = x - PULSE_SPEED * tcos(dir);
    pulse->pos.y = y - PULSE_SPEED * tsin(dir);
    NumPulses++;
    if (pl)
	pl->num_pulses++;
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
    const DFLOAT		k = 10.0,
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
    DFLOAT		length, force, ratio, accell, cosine, pl_damping, ball_damping;
    const DFLOAT		k = 1500.0, b = 2.0;
    const DFLOAT		max_spring_ratio = 0.30;

    /* compute the normalized vector between the ball and the player */
    D.x = WRAP_DX(pl->pos.x - ball->pos.x);
    D.y = WRAP_DY(pl->pos.y - ball->pos.y);
    length = VECTOR_LENGTH(D);
    if (length > 0.0) {
	D.x /= length;
	D.y /= length;
    }
    else
	D.x = D.y = 0.0;

    /* compute the ratio for the spring action */
    ratio = (BALL_STRING_LENGTH - length) / (DFLOAT) BALL_STRING_LENGTH;

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
    DFLOAT	range = 0.0;
    DFLOAT	acc;
    DFLOAT	x_dif = 0.0;
    DFLOAT	y_dif = 0.0;
    DFLOAT	shot_speed;

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
	    range = Wrap_length(pl->pos.x + pl->ship->engine[pl->dir].x
				- shot->pos.x,
				pl->pos.y + pl->ship->engine[pl->dir].y
				- shot->pos.y);
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
		DFLOAT l;
		int i;

		range = HEAT_RANGE * (shot->count/HEAT_CLOSE_TIMEOUT);
		for (i=0; i<NumPlayers; i++) {
		    player *p = Players[i];

		    if (!BIT(p->status, THRUSTING))
			continue;

		    l = Wrap_length(p->pos.x + p->ship->engine[p->dir].x
				    - shot->pos.x,
				    p->pos.y + p->ship->engine[p->dir].y
				    - shot->pos.y);
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
	x_dif = (int)(rfrac() * 4 * shot->count);
	y_dif = (int)(rfrac() * 4 * shot->count);

    } else {

	if (BIT(shot->status, CONFUSED)
	    && (!(frame_loops % CONFUSED_UPDATE_GRANULARITY)
		|| shot->count == CONFUSED_TIME)) {

	    if (shot->count) {
		shot->info = Players[(int)(rfrac() * NumPlayers)]->id;
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
		if ((int)(rfrac() * 100) <= ((int)(range/2)+50))
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
    theta = (int)Wrap_findDir(pl->pos.x + x_dif - shot->pos.x,
						 pl->pos.y + y_dif - shot->pos.y);

    {
	DFLOAT x, y, vx, vy;
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
	    xi = (int)((x += vx) / BLOCK_SZ);
	    yi = (int)((y += vy) / BLOCK_SZ);
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
	xi = OBJ_X_IN_BLOCKS(shot);
	yi = OBJ_Y_IN_BLOCKS(shot);

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
		    && ((j == -1 && (rfrac() < 0.5f)) || j == 0 || j == 1))) {
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
	    theta = (int)Wrap_findDir((yi + sur[i].dy) * BLOCK_SZ
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
	shot->dir += (u_byte)(((-angle < shot->turnspeed) ? -angle : shot->turnspeed));
    else
	shot->dir -= (u_byte)(((angle < shot->turnspeed) ? angle : shot->turnspeed));

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



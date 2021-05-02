/* $Id: ship.c,v 4.6 2000/02/21 20:23:44 bert Exp $
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
#include <math.h>
#include <limits.h>
#else
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "saudio.h"
#include "error.h"
#include "objpos.h"
#include "netserver.h"

char ship_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: ship.c,v 4.6 2000/02/21 20:23:44 bert Exp $";
#endif


/******************************
 * Functions for ship movement.
 */

void Thrust(int ind)
{
    player		*pl = Players[ind];
    const int		min_dir = (int)(pl->dir + RES/2 - RES*0.2 - 1);
    const int		max_dir = (int)(pl->dir + RES/2 + RES*0.2 + 1);
    const DFLOAT	max_speed = 1 + (pl->power * 0.14);
    const int		max_life = 3 + (int)(pl->power * 0.35);
    static int		keep_rand;
    int			this_rand = (((keep_rand >>= 2)
					? (keep_rand)
					: (keep_rand = rand())) & 0x03);
    int			tot_sparks = (int)((pl->power * 0.15) + this_rand + 1);
    DFLOAT		x = pl->pos.x + pl->ship->engine[pl->dir].x;
    DFLOAT		y = pl->pos.y + pl->ship->engine[pl->dir].y;
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
	/* owner team	  */ pl->team,
	/* kind           */ OBJ_SPARK,
	/* mass           */ THRUST_MASS,
	/* status         */ GRAVITY | OWNERIMMUNE,
	/* color          */ RED,
	/* radius         */ 8,
	/* min,max debris */ tot_sparks-alt_sparks, tot_sparks-alt_sparks,
	/* min,max dir    */ min_dir, max_dir,
	/* min,max speed  */ 1.0, max_speed,
	/* min,max life   */ 3, max_life
	);

    Make_debris(
	/* pos.x, pos.y   */ x, y,
	/* vel.x, vel.y   */ pl->vel.x, pl->vel.y,
	/* owner id       */ pl->id,
	/* owner team	  */ pl->team,
	/* kind           */ OBJ_SPARK,
	/* mass           */ THRUST_MASS * ALT_SPARK_MASS_FACT,
	/* status         */ GRAVITY | OWNERIMMUNE,
	/* color          */ BLUE,
	/* radius         */ 8,
	/* min,max debris */ alt_sparks, alt_sparks,
	/* min,max dir    */ min_dir, max_dir,
	/* min,max speed  */ 1.0, max_speed,
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
	/* owner team	  */ pl->team,
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
#if 0
/* old code, not used anymore. */
    ship->vel.x -= ((tcos(shot->dir) * ABS(shot->vel.x-ship->vel.x) *
	shot->mass) / ship->mass);
    ship->vel.y -= ((tsin(shot->dir) * ABS(shot->vel.y-ship->vel.y) *
	shot->mass) / ship->mass);
#else
/* new code thanks to Uoti Urpala. */
    ship->vel.x -= (((shot->vel.x - ship->vel.x) *
	shot->mass) / ship->mass);
    ship->vel.y -= (((shot->vel.y - ship->vel.y) *
	shot->mass) / ship->mass);
#endif
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

/* Calculates the effect of a collision between two objects */
void Delta_mv(object *ship, object *obj)
{
    DFLOAT	vx, vy, m;

    m = ship->mass + ABS(obj->mass);
    vx = (ship->vel.x * ship->mass + obj->vel.x * obj->mass) / m;
    vy = (ship->vel.y * ship->mass + obj->vel.y * obj->mass) / m;
    if (ship->type == OBJ_PLAYER
	&& obj->id != -1
	&& BIT(obj->status, COLLISIONSHOVE)) {
	player *pl = (player *)ship;
	player *pusher = Players[GetInd[obj->id]];
	if (pusher != pl) {
	    Record_shove(pl, pusher, frame_loops);
	}
    }
    ship->vel.x = vx;
    ship->vel.y = vy;
    obj->vel.x = vx;
    obj->vel.y = vy;
}


void Obj_repel(object *obj1, object *obj2, int repel_dist)
{
    DFLOAT		xd, yd,
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
	    Record_shove(pl, pusher, frame_loops);
	}
    }

    if (obj2->type == OBJ_PLAYER && obj1->id != -1) {
	player *pl = (player *)obj2;
	player *pusher = Players[GetInd[obj1->id]];
	if (pusher != pl) {
	    Record_shove(pl, pusher, frame_loops);
	}
    }

    obj1->vel.x += dvx1;
    obj1->vel.y += dvy1;

    obj2->vel.x += dvx2;
    obj2->vel.y += dvy2;
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

    if (BIT(pl->used, OBJ_PHASING_DEVICE))
	return;

    /* Return, if no more players or no tanks */
    if (pl->fuel.num_tanks == 0
	|| NumPseudoPlayers == MAX_PSEUDO_PLAYERS
	|| peek_ID() == 0) {
	return;
    }

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
    Player_position_init_pixels(dummy, pl->pos.x, pl->pos.y);
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

    request_ID();
    NumPlayers++;
    NumPseudoPlayers++;
    updateScores = true;

    sound_play_sensors(pl->pos.x, pl->pos.y, TANK_DETACH_SOUND);

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


void Make_wreckage(
    /* pos.x, pos.y     */ DFLOAT x,            DFLOAT y,
    /* vel.x, vel.y     */ DFLOAT velx,         DFLOAT vely,
    /* owner id         */ int    id,
    /* owner team	*/ u_short team,
    /* min,max mass     */ DFLOAT min_mass,     DFLOAT max_mass,
    /* total mass       */ DFLOAT total_mass,
    /* status           */ long   status,
    /* color            */ int    color,
    /* max wreckage     */ int    max_wreckage,
    /* min,max dir      */ int    min_dir,      int    max_dir,
    /* min,max speed    */ DFLOAT min_speed,    DFLOAT max_speed,
    /* min,max life     */ int    min_life,     int    max_life
)
{
    object		*wreckage;
    int			i, life, size;
    modifiers		mods;
    DFLOAT		mass, sum_mass = 0.0;

    if (BIT(World.rules->mode, WRAP_PLAY)) {
	if (x < 0) x += World.width;
	else if (x >= World.width) x -= World.width;
	if (y < 0) y += World.height;
	else if (y >= World.height) y -= World.height;
    }
    if (x < 0 || x >= World.width || y < 0 || y >= World.height) {
	return;
    }
    if (max_life < min_life)
	max_life = min_life;
    if (ShotsLife >= FPS) {
	if (min_life > ShotsLife) {
	    min_life = ShotsLife;
	    max_life = ShotsLife;
	}
    }

    if (min_speed * max_life > World.hypotenuse)
	min_speed = World.hypotenuse / max_life;
    if (max_speed * min_life > World.hypotenuse)
	max_speed = World.hypotenuse / min_life;
    if (max_speed < min_speed)
	max_speed = min_speed;

    if (max_wreckage > MAX_TOTAL_SHOTS - NumObjs) {
	max_wreckage = MAX_TOTAL_SHOTS - NumObjs;
    }

    CLEAR_MODS(mods);

    for (i = 0; i < max_wreckage && sum_mass < total_mass; i++, NumObjs++) {
	DFLOAT		speed;
	int		dir, radius;

	wreckage = Obj[NumObjs];
	wreckage->color = color;
	wreckage->id = id;
	wreckage->team = team;
	wreckage->type = OBJ_WRECKAGE;

	/* Position */
	Object_position_init_pixels(wreckage, x, y);

	/* Direction */
	dir = MOD2(min_dir + (int)(rfrac() * MOD2(max_dir - min_dir, RES)), RES);
	wreckage->dir = dir;

	/* Velocity and acceleration */
	speed = min_speed + rfrac() * (max_speed - min_speed);
	wreckage->vel.x = velx + tcos(dir) * speed;
	wreckage->vel.y = vely + tsin(dir) * speed;
	wreckage->acc.x = 0;
	wreckage->acc.y = 0;

	/* Mass */
	mass = min_mass + rfrac() * (max_mass - min_mass);
	if ( sum_mass + mass > total_mass )
	    mass = total_mass - sum_mass;
	wreckage->mass = mass;
	sum_mass += mass;
	if ( mass < min_mass ) {
	    NumObjs--;
	    break;
	}

	/* Lifespan  */
	life = (int)(min_life + rfrac() * (max_life - min_life) + 1);
	if (life * speed > World.hypotenuse) {
	    life = (long)(World.hypotenuse / speed);
	}
	wreckage->life = life;
	wreckage->fuselife = wreckage->life;

	/* Wreckage type, rotation, and size */
	wreckage->turnspeed = 0.02 + rfrac() * 0.35;
	wreckage->rotation = (int)(rfrac() * RES);
	size = (int) ( 256.0 * 1.5 * mass / total_mass );
	if ( size > 255 )
	    size = 255;
	wreckage->size = size;
	wreckage->info = rand();

	radius = wreckage->size * 16 / 256;
	if ( radius < 8 ) radius = 8;

	wreckage->spread_left = 0;
	wreckage->pl_range = radius;
	wreckage->pl_radius = radius;
	wreckage->status = status;
	wreckage->mods = mods;
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
    /* reduce debris since we also create wreckage objects */
    min_debris >>= 1;
    max_debris >>= 1;

    Make_debris(
	/* pos.x, pos.y   */ pl->pos.x, pl->pos.y,
	/* vel.x, vel.y   */ pl->vel.x, pl->vel.y,
	/* owner id       */ pl->id,
	/* owner team	  */ pl->team,
	/* kind           */ OBJ_DEBRIS,
	/* mass           */ 3.5,
	/* status         */ GRAVITY,
	/* color          */ RED,
	/* radius         */ 8,
	/* min,max debris */ min_debris, max_debris,
	/* min,max dir    */ 0, RES-1,
	/* min,max speed  */ 20.0, 20 + (((int)(pl->mass))>>1),
	/* min,max life   */ 5, (int)(5 + (pl->mass * 1.5))
	);

    if ( !BIT(pl->status, KILLED) )
	return;
    Make_wreckage(
	/* pos.x, pos.y     */ pl->pos.x, pl->pos.y,
	/* vel.x, vel.y     */ pl->vel.x, pl->vel.y,
	/* owner id         */ pl->id,
	/* owner team	    */ pl->team,
	/* min,max mass     */ MAX(pl->mass/8.0, 0.33), pl->mass,
	/* total mass       */ 2.0 * pl->mass,
	/* status           */ GRAVITY,
	/* color            */ WHITE,
	/* max wreckage     */ 10,
	/* min,max dir      */ 0, RES-1,
	/* min,max speed    */ 10.0, 10 + (((int)(pl->mass))>>1),
	/* min,max life     */ 5, (int)(5 + (pl->mass * 1.5))
	);

}


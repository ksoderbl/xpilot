/* $Id: update.c,v 3.37 1994/03/30 17:07:31 bert Exp $
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

#define SERVER
#include <stdlib.h>
#include "global.h"
#include "map.h"
#include "score.h"
#include "draw.h"
#include "robot.h"
#include "bit.h"
#include "saudio.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: update.c,v 3.37 1994/03/30 17:07:31 bert Exp $";
#endif


#define update_object_speed(OBJ)	{				\
    int x=(int)((OBJ)->pos.x/BLOCK_SZ), y=(int)((OBJ)->pos.y/BLOCK_SZ);	\
    if (BIT((OBJ)->status, GRAVITY)) {					\
	(OBJ)->vel.x += (OBJ)->acc.x + World.gravity[x][y].x;		\
	(OBJ)->vel.y += (OBJ)->acc.y + World.gravity[x][y].y;		\
    } else {								\
	(OBJ)->vel.x += (OBJ)->acc.x;					\
	(OBJ)->vel.y += (OBJ)->acc.y;					\
    }									\
}


static char msg[MSG_LEN];


static void Transport_to_home(int ind)
{
    /*
     * Transport a corpse from the place where it died back to its homebase.
     * During the first part of the distance we give it a positive constant
     * acceleration G, during the second part we make this a negative one -G.
     * This results in a visually pleasing take off and landing.
     */
    player		*pl = Players[ind];
    float		bx, by, dx, dy,	t, m;
    const int		T = RECOVERY_DELAY;

    bx = World.base[pl->home_base].pos.x * BLOCK_SZ + BLOCK_SZ/2;
    by = World.base[pl->home_base].pos.y * BLOCK_SZ + BLOCK_SZ/2;
    dx = WRAP_DX(bx - pl->pos.x);
    dy = WRAP_DY(by - pl->pos.y);
    t = pl->count + 0.5f;
    if (2 * t <= T) {
	m = 2 / t;
    } else {
	t = T - t;
	m = (4 * t) / (T * T - 2 * t * t);
    }
    pl->vel.x = dx * m;
    pl->vel.y = dy * m;
}

/*
 * Turn emergency thrust on or off.
 */
void Emergency_thrust (int ind, int on)
{
    player	*pl = Players[ind];
    const int	emergency_thrust_time = 4 * FPS;

    if (on) {
	if (pl->emergency_thrust_left <= 0) {
	    pl->emergency_thrust_left = emergency_thrust_time;
	    pl->emergency_thrust_max = emergency_thrust_time;
	    pl->emergency_thrusts--;
	}
	SET_BIT(pl->used, OBJ_EMERGENCY_THRUST);
	sound_play_sensors(pl->pos.x, pl->pos.y, EMERGENCY_THRUST_ON_SOUND);
    } else {
	CLR_BIT(pl->used, OBJ_EMERGENCY_THRUST);
	if (pl->emergency_thrust_left <= 0) {
	    /*
	     * If we have no emergency thrust left we also turn off autopilot
	     * as that meant we were doing an emergency brake.
	     */
	    if (BIT(pl->used, OBJ_AUTOPILOT))
		Autopilot(ind, 0);
	    if (pl->emergency_thrusts <= 0)
		CLR_BIT(pl->have, OBJ_EMERGENCY_THRUST);
	}
	sound_play_sensors(pl->pos.x, pl->pos.y, EMERGENCY_THRUST_OFF_SOUND);
    }
}

/*
 * Turn autopilot on or off.  This always clears the thrusting bit.  During
 * automatic pilot mode any changes to the current power, turnacc, turnspeed
 * and turnresistance settings will be temporary.
 */
void Autopilot (int ind, int on)
{
    player	*pl = Players[ind];

    CLR_BIT(pl->status, THRUSTING);
    if (on) {
	pl->auto_power_s = pl->power;
	pl->auto_turnacc_s = pl->turnacc;
	pl->auto_turnspeed_s = pl->turnspeed;
	pl->auto_turnresistance_s = pl->turnresistance;
	SET_BIT(pl->used, OBJ_AUTOPILOT);
	pl->power = (MIN_PLAYER_POWER+MAX_PLAYER_POWER)/2.0;
	pl->turnspeed = (MIN_PLAYER_TURNSPEED+MAX_PLAYER_TURNSPEED)/2.0;
	pl->turnresistance = 0.2;
	sound_play_sensors(pl->pos.x, pl->pos.y, AUTOPILOT_ON_SOUND);
    } else {
	pl->power = pl->auto_power_s;
	pl->turnacc = pl->auto_turnacc_s;
	pl->turnspeed = pl->auto_turnspeed_s;
	pl->turnresistance = pl->auto_turnresistance_s;
	CLR_BIT(pl->used, OBJ_AUTOPILOT);
	sound_play_sensors(pl->pos.x, pl->pos.y, AUTOPILOT_OFF_SOUND);
    }
}

/*
 * Automatic pilot will try to hold the ship steady, turn to face away
 * from direction of travel, if so then turn on thrust which will
 * cause the ship to come to a rest within a short period of time.
 * This code is fairly self contained.
 */
static void do_Autopilot (player *pl)
{
    int		vad;	/* Velocity Away Delta */
    int		dir;
    int		afterburners;
    int		ix, iy;
    float	gx, gy;
    float	acc, vel;
    float	delta;
    float	turnspeed, power;
    const float	emergency_thrust_settings_delta = 150.0 / FPS;
    const float	auto_pilot_settings_delta = 15.0 / FPS;
    const float	auto_pilot_turn_factor = 2.5;
    const float	auto_pilot_dead_velocity = 0.5;

    /*
     * Having more autopilot items or using emergency thrust causes a much
     * quicker deceleration to occur than during normal flight.  Having
     * no autopilot items will cause minimum delta to occur, this is because
     * the autopilot code is used by the pause code.
     */
    delta = auto_pilot_settings_delta;
    if (pl->autopilots)
	delta *= pl->autopilots;

    if (BIT(pl->used, OBJ_EMERGENCY_THRUST)) {
	afterburners = MAX_AFTERBURNER;
	if (delta < emergency_thrust_settings_delta)
	    delta = emergency_thrust_settings_delta;
    } else {
	afterburners = pl->afterburners;
    }

    ix = pl->pos.x/BLOCK_SZ;
    iy = pl->pos.y/BLOCK_SZ;
    gx = World.gravity[ix][iy].x;
    gy = World.gravity[ix][iy].y;

    /*
     * Due to rounding errors if the velocity is very small we were probably
     * on target to stop last time round, so we actually absolutely stop.
     * This enables the ship to orient away from gravity and set up the
     * thrust to conteract it.
     */
    if ((vel = LENGTH(pl->vel.x, pl->vel.y)) < auto_pilot_dead_velocity) {
	pl->vel.x = pl->vel.y = vel = 0.0;
	pl->pos = pl->prevpos;
    }
	
    /*
     * Calculate power needed to change instantaneously to stopped.  We
     * must include gravity here for next time round the update loop.
     */
    acc = LENGTH(gx, gy) + vel;
    power = acc * pl->mass;
    if (afterburners)
	power /= AFTER_BURN_POWER_FACTOR(afterburners);
    
    /*
     * Calculate direction change needed to reduce velocity to zero.
     */
    if (vel == 0.0) {
	vad = findDir(-gx, -gy);
    } else {
	vad = findDir(-pl->vel.x, -pl->vel.y);
    }
    vad = MOD2(vad - pl->dir, RES);
    if (vad > RES/2) {
	vad = RES - vad;
	dir = -1;
    } else {
	dir = 1;
    }

    /*
     * Calculate turnspeed needed to change direction instataneously by
     * above direction change.
     */
    turnspeed = ((float)vad) / pl->turnresistance - pl->turnvel;
    if (turnspeed < 0) {
	turnspeed = -turnspeed;
	dir = -dir;
    }

    /*
     * Change the turnspeed setting towards the perfect value, and limit
     * to the maximum only (limiting to the minimum causes oscillation).
     */
    if (turnspeed < pl->turnspeed) {
	pl->turnspeed -= delta;
	if (turnspeed > pl->turnspeed)
	    pl->turnspeed = turnspeed;
    } else if (turnspeed > pl->turnspeed) {
	pl->turnspeed += delta;
	if (turnspeed < pl->turnspeed)
	    pl->turnspeed = turnspeed;
    }
    if (pl->turnspeed > MAX_PLAYER_TURNSPEED)
	pl->turnspeed = MAX_PLAYER_TURNSPEED;

    /*
     * Decide if its wise to turn this time.
     */
    if (pl->turnspeed > (turnspeed*auto_pilot_turn_factor)) {
	pl->turnacc = 0.0;
	pl->turnvel = 0.0;
    } else {
	pl->turnacc = dir * pl->turnspeed;
    }

    /*
     * Change the power setting towards the perfect value, and limit
     * to the maximum only (limiting to the minimum causes oscillation).
     */
    if (power < pl->power) {
	pl->power -= delta;
	if (power > pl->power)
	    pl->power = power;
    } else if (power > pl->power) {
	pl->power += delta;
	if (power < pl->power)
	    pl->power = power;
    }
    if (pl->power > MAX_PLAYER_POWER)
	pl->power = MAX_PLAYER_POWER;

    /*
     * Don't thrust if the direction will not be absolutely correct and hasn't
     * been very close last time.  The latter clause was added such that
     * when a fine direction adjustment is needed, but the turnspeed is too
     * high at the moment, it gets the ship slowing down even though it
     * will impart some sideways velocity.
     */
    if (pl->turnspeed != turnspeed && vad > RES/32) {
	CLR_BIT(pl->status, THRUSTING);
	return;
    }

    /*
     * Only thrust if the power setting is correct or less than correct,
     * we don't want to over thrust.
     */
    if (pl->power > power) {
	CLR_BIT(pl->status, THRUSTING);
    } else {
	SET_BIT(pl->status, THRUSTING);
    }
}

/*
 * Do tractor beam attraction between two players, where `pl' is doing
 * the tractor beam and `to' is the target.
 */
void do_Tractor_beam (player *pl)
{
    player	*to;
    float	maxdist, maxforce, percent;
    float	xd, yd, force, mass;
    long	cost;
    int		theta;

    maxdist = TRACTOR_MAX_RANGE(pl);

    if (BIT(pl->lock.tagged, (LOCK_PLAYER|LOCK_VISIBLE)) != (LOCK_PLAYER
							     |LOCK_VISIBLE)
	|| pl->lock.distance >= maxdist) {
	pl->tractor = NULL;
	return;
    }

    maxforce = TRACTOR_MAX_FORCE(pl);
    percent = TRACTOR_PERCENT(pl, maxdist);
    cost = TRACTOR_COST(percent);
    force = TRACTOR_FORCE(pl, percent, maxforce);

    if (pl->fuel.sum < -cost) {
	pl->tractor = NULL;
	CLR_BIT(pl->used, OBJ_TRACTOR_BEAM);
	return;
    }

    sound_play_sensors(pl->pos.x, pl->pos.y,
		       force < 0 ? TRACTOR_BEAM_SOUND : PRESSOR_BEAM_SOUND);

    to = pl->tractor = Players[GetInd[pl->lock.pl_id]];

    Add_fuel(&(pl->fuel), cost);

    xd = WRAP_DX(pl->pos.x - to->pos.x);
    yd = WRAP_DY(pl->pos.y - to->pos.y);

    theta = findDir(xd, yd);
    mass = pl->mass + to->mass;

    pl->vel.x += tcos(theta) * force / pl->mass;
    pl->vel.y += tsin(theta) * force / pl->mass;
    to->vel.x -= tcos(theta) * force / to->mass;
    to->vel.y -= tsin(theta) * force / to->mass;
}


/********** **********
 * Updating objects and the like.
 */
void Update_objects(void)
{
    int i, j, ecm, necm;
    player *pl;
    object *obj;

    /*
     * Update robots.
     */
    Update_robots();

    /*
     * Autorepeat fire, must unfortunately be done here, not in
     * the player loop below, because of collisions between the shots
     * and the auto-firing player that would otherwise occur.
     */
    if (fireRepeatRate > 0) {
	for (i = 0; i < NumPlayers; i++) {
	    pl = Players[i];
	    if (BIT(pl->used, OBJ_FIRE)
		&& loops - pl->shot_time >= fireRepeatRate) {
		Fire_normal_shots(i);
	    }
	}
    }

    /*
     * Special items.
     */
    for (i=0; i<NUM_ITEMS; i++)
	if (World.items[i].num < World.items[i].max
	    && World.items[i].chance > 0
	    && rand()%World.items[i].chance == 0) {

	    Place_item(i, 0);
	}

    /*
     * Let the fuel stations regenerate some fuel.
     */
    if (NumPlayers > 0) {
	int fuel = NumPlayers * STATION_REGENERATION;
	int frames_per_update = MAX_STATION_FUEL / (fuel * BLOCK_SZ);
	for (i=0; i<World.NumFuels; i++) {
	    if (World.fuel[i].fuel == MAX_STATION_FUEL) {
		continue;
	    }
	    if ((World.fuel[i].fuel += fuel) >= MAX_STATION_FUEL) {
		World.fuel[i].fuel = MAX_STATION_FUEL;
	    }
	    else if (World.fuel[i].last_change + frames_per_update > loops) {
		/*
		 * We don't send fuelstation info to the clients every frame
		 * if it wouldn't change their display.
		 */
		continue;
	    }
	    World.fuel[i].conn_mask = 0;
	    World.fuel[i].last_change = loops;
	}
    }

    /*
     * Update shots.
     */
    for (i=0; i<NumObjs; i++) {
	obj = Obj[i];

	update_object_speed(obj);
	Move_object(i);

	if (BIT(Obj[i]->type, OBJ_MINE))
	    Move_mine(i);

	else if (BIT(obj->type, OBJ_SMART_SHOT|OBJ_HEAT_SHOT|OBJ_TORPEDO))
	    Move_smart_shot(i);

	else if (BIT(obj->type, OBJ_BALL)) {
	    if (obj->id != -1)
		Move_ball(i);
	}
    }

    /*
     * Updating cannons, maybe a little bit of fireworks too?
     */
    for (i=0; i<World.NumCannons; i++) {
	if (World.cannon[i].dead_time > 0) {
	    if (!--World.cannon[i].dead_time) {
		World.block[World.cannon[i].pos.x][World.cannon[i].pos.y]
		    = CANNON;
		World.cannon[i].conn_mask = 0;
		World.cannon[i].last_change = loops;
	    }
	    continue;
	}
	if (World.cannon[i].active) {
	    if (rand()%16 == 0)
		Cannon_fire(i);
	}
	World.cannon[i].active = false;
    }
    
    /*
     * Update targets
     */
    for (i = 0; i < World.NumTargets; i++) {
	if (World.targets[i].dead_time > 0) {
	    if (!--World.targets[i].dead_time) {
		World.block[World.targets[i].pos.x][World.targets[i].pos.y]
		    = TARGET;
		World.targets[i].conn_mask = 0;
		World.targets[i].last_change = loops;
	    }
	    continue;
	}
	else if (World.targets[i].damage == TARGET_DAMAGE) {
	    continue;
	}
	World.targets[i].damage += TARGET_REPAIR_PER_FRAME;
	if (World.targets[i].damage >= TARGET_DAMAGE) {
	    World.targets[i].damage = TARGET_DAMAGE;
	}
	else if (World.targets[i].last_change + TARGET_UPDATE_DELAY < loops) {
	    /*
	     * We don't send target info to the clients every frame
	     * if the latest repair wouldn't change their display.
	     */
	    continue;
	}
	World.targets[i].conn_mask = 0;
	World.targets[i].last_change = loops;
    }

    /* * * * * *
     *
     * Player loop. Computes miscellaneous updates.
     *
     */
    for (i=0; i<NumPlayers; i++) {
#ifdef TURN_FUEL
        long tf = 0;
#endif

	pl = Players[i];

	/* Limits. */
	LIMIT(pl->power, MIN_PLAYER_POWER, MAX_PLAYER_POWER);
	LIMIT(pl->turnspeed, MIN_PLAYER_TURNSPEED, MAX_PLAYER_TURNSPEED);
	LIMIT(pl->turnresistance, MIN_PLAYER_TURNRESISTANCE,
				  MAX_PLAYER_TURNRESISTANCE);

	if (pl->count > 0) {
	    pl->count--;
	    if (!BIT(pl->status, PLAYING)) {
		Transport_to_home(i);
		Move_player(i);
		continue;
	    }
	}

	if (pl->count == 0) {
	    pl->count = -1;

	    if (!BIT(pl->status, PLAYING)) {
		SET_BIT(pl->status, PLAYING);
		Go_home(i);
	    }
	    if (BIT(pl->status, SELF_DESTRUCT)) {
		sprintf(msg, "%s has comitted suicide.", pl->name);
		Set_message(msg);
		Throw_items(pl);
		Kill_player(i);
		updateScores = true;
		pl->check = 0;
		pl->round = 0;
		pl->time  = 0;
	    }
	}

	if (BIT(pl->status, PLAYING|GAME_OVER|PAUSE) != PLAYING)
	    continue;

	if (pl->shield_time > 0) {
	    if (--pl->shield_time == 0) {
		CLR_BIT(pl->used, OBJ_SHIELD);
	    }
	    if (BIT(pl->used, OBJ_SHIELD) == 0) {
		CLR_BIT(pl->have, OBJ_SHIELD);
		pl->shield_time = 0;
	    }
	}

	if (BIT(pl->used, OBJ_EMERGENCY_THRUST)) {
	    if (pl->fuel.sum > 0
		&& BIT(pl->status, THRUSTING)
		&& --pl->emergency_thrust_left <= 0)
		Emergency_thrust(i, 0);
	}

	/*
	 * Only do autopilot code if switched on and player is not
	 * damaged (ie. can see).
	 */
	if (   (BIT(pl->used, OBJ_AUTOPILOT))
	    || (BIT(pl->status, HOVERPAUSE) && !pl->damaged)) {
		do_Autopilot(pl);
	}

	/*
	 * Compute turn
	 */
	pl->turnvel	+= pl->turnacc;
	pl->turnvel	*= pl->turnresistance;
        
#ifdef TURN_FUEL
        tf = pl->oldturnvel-pl->turnvel;
        tf = TURN_FUEL(tf);
        if (pl->fuel.sum <= tf) {
            tf = 0;
            pl->turnacc = 0.0;
            pl->turnvel = pl->oldturnvel;
        } else {
            Add_fuel(&(pl->fuel),-tf);
            pl->oldturnvel = pl->turnvel;
        }
#endif


	pl->float_dir	+= pl->turnvel;
        
	if (pl->float_dir < 0)
	    pl->float_dir += RES;
	if (pl->float_dir >= RES)
	    pl->float_dir -= RES;

	Turn_player(i);


	/*
	 * Compute energy drainage
	 */
	if (BIT(pl->used, OBJ_SHIELD))
            Add_fuel(&(pl->fuel), ED_SHIELD);

	if (BIT(pl->used, OBJ_CLOAKING_DEVICE))
            Add_fuel(&(pl->fuel), ED_CLOAKING_DEVICE);

#define UPDATE_RATE 100

	for (j = 0; j < NumPlayers; j++) {
	    if (pl->forceVisible)
		Players[j]->visibility[i].canSee = 1;

	    if (i == j || !BIT(Players[j]->used, OBJ_CLOAKING_DEVICE))
		pl->visibility[j].canSee = 1;
	    else if (pl->updateVisibility
		     || Players[j]->updateVisibility
		     || rand() % UPDATE_RATE
		     < ABS(loops - pl->visibility[j].lastChange)) {

		pl->visibility[j].lastChange = loops;
		pl->visibility[j].canSee
		    = rand() % (pl->sensors + 1)
			> (rand() % (Players[j]->cloaks + 1));
	    }
	}

	if (BIT(pl->used, OBJ_REFUEL)) {
	    if ((Wrap_length((pl->pos.x-World.fuel[pl->fs].pos.x),
		             (pl->pos.y-World.fuel[pl->fs].pos.y)) > 90.0)
		|| (pl->fuel.sum >= pl->fuel.max)) {
		CLR_BIT(pl->used, OBJ_REFUEL);
	    } else {
                int i = pl->fuel.num_tanks;
                int ct = pl->fuel.current;
                
                do {
		    if (World.fuel[pl->fs].fuel > REFUEL_RATE) {
		        World.fuel[pl->fs].fuel -= REFUEL_RATE;
			World.fuel[pl->fs].conn_mask = 0;
			World.fuel[pl->fs].last_change = loops;
		        Add_fuel(&(pl->fuel), REFUEL_RATE);
		    } else {
		        Add_fuel(&(pl->fuel), World.fuel[pl->fs].fuel);
		        World.fuel[pl->fs].fuel = 0;
		        World.fuel[pl->fs].conn_mask = 0;
		        World.fuel[pl->fs].last_change = loops;
		        CLR_BIT(pl->used, OBJ_REFUEL);
                        break;
		    }
                    if (pl->fuel.current == pl->fuel.num_tanks)
                        pl->fuel.current = 0;
                    else
                        pl->fuel.current += 1;
                } while (i--);
                pl->fuel.current = ct;
            }
	}

	/* target repair */
	if (BIT(pl->used, OBJ_REPAIR)) {
	    target_t *targ = &World.targets[pl->repair_target];
	    float x = targ->pos.x*BLOCK_SZ+BLOCK_SZ/2;
	    float y = targ->pos.y*BLOCK_SZ+BLOCK_SZ/2;
	    if (Wrap_length(pl->pos.x - x, pl->pos.y - y) > 90.0
		|| targ->damage >= TARGET_DAMAGE
		|| targ->dead_time > 0) {
		CLR_BIT(pl->used, OBJ_REPAIR);
	    } else {
		int i = pl->fuel.num_tanks;
		int ct = pl->fuel.current;

		do {
		    if (pl->fuel.tank[pl->fuel.current] > REFUEL_RATE) {
			targ->damage += TARGET_FUEL_REPAIR_PER_FRAME;
			targ->conn_mask = 0;
			targ->last_change = loops;
			Add_fuel(&(pl->fuel), -REFUEL_RATE);
			if (targ->damage > TARGET_DAMAGE) {
			    targ->damage = TARGET_DAMAGE;
			    break;
			}
		    } else {
			CLR_BIT(pl->used, OBJ_REPAIR);
		    }
		    if (pl->fuel.current == pl->fuel.num_tanks)
			pl->fuel.current = 0;
		    else
			pl->fuel.current += 1;
		} while (i--);
		pl->fuel.current = ct;
	    }
	}

	if (pl->fuel.sum <= 0) {
	    CLR_BIT(pl->used, OBJ_SHIELD|OBJ_CLOAKING_DEVICE);
	    CLR_BIT(pl->status, THRUSTING);
	}
	if (pl->fuel.sum > (pl->fuel.max-REFUEL_RATE))
	    CLR_BIT(pl->used, OBJ_REFUEL);

	/*
	 * Update acceleration vector etc.
	 */
	if (BIT(pl->status, THRUSTING)) {
            float power = pl->power;
            float f = pl->power * 0.0008;	/* 1/(FUEL_SCALE*MIN_POWER) */
            int a = (BIT(pl->used, OBJ_EMERGENCY_THRUST) ? MAX_AFTERBURNER
		     : pl->afterburners);

            if (a) {
                power = AFTER_BURN_POWER(power, a);
                f = AFTER_BURN_FUEL(f, a);
            }
	    pl->acc.x = power * tcos(pl->dir) / pl->mass;
	    pl->acc.y = power * tsin(pl->dir) / pl->mass;
            Add_fuel(&(pl->fuel), -f * FUEL_SCALE_FACT); /* Decrement fuel */
	} else {
	    pl->acc.x = pl->acc.y = 0.0;
	}

	pl->mass = pl->emptymass + FUEL_MASS(pl->fuel.sum);

	if (BIT(pl->status, WARPING)) {
	    position w;
	    int wx, wy, proximity,
	    	nearestFront, nearestRear,
	    	proxFront, proxRear;

	    if (World.wormHoles[pl->wormHoleHit].countdown
		&& World.wormHoles[pl->wormHoleHit].lastplayer != i)
		j = World.wormHoles[pl->wormHoleHit].lastdest;
	    else if (rand() % 100 < 10)
		do
		    j = rand() % World.NumWormholes;
		while (World.wormHoles[j].type == WORM_IN
		       || pl->wormHoleHit == j);
	    else {
		nearestFront = nearestRear = -1;
		proxFront = proxRear = 10000000;
		
		for (j = 0; j < World.NumWormholes; j++) {
		    if (j == pl->wormHoleHit
			|| World.wormHoles[j].type == WORM_IN)
			continue;
		    
		    wx = (World.wormHoles[j].pos.x -
			  World.wormHoles[pl->wormHoleHit].pos.x) * BLOCK_SZ;
		    wy = (World.wormHoles[j].pos.y - 
			  World.wormHoles[pl->wormHoleHit].pos.y) * BLOCK_SZ;
		    wx = WRAP_DX(wx);
		    wy = WRAP_DX(wy);
		    
		    proximity = pl->vel.y * wx + pl->vel.x * wy;
		    proximity = ABS(proximity);
		    
		    if (pl->vel.x * wx + pl->vel.y * wy < 0) {
			if (proximity < proxRear) {
			    nearestRear = j;
			    proxRear = proximity;
			}
		    } else if (proximity < proxFront) {
			nearestFront = j;
			proxFront = proximity;
		    }
		}
		
#define RANDOM_REAR_WORM
#ifndef RANDOM_REAR_WORM
		j = nearestFront < 0 ? nearestRear : nearestFront;
#else /* RANDOM_REAR_WORM */
		if (nearestFront >= 0)
		    j = nearestFront;
		else
		    do
			j = rand() % World.NumWormholes;
		    while (World.wormHoles[j].type == WORM_IN
			   || j == pl->wormHoleHit);
#endif /* RANDOM_REAR_WORM */
	    }

            sound_play_sensors(pl->pos.x, pl->pos.y, WORM_HOLE_SOUND);

	    w.x = World.wormHoles[j].pos.x * BLOCK_SZ + BLOCK_SZ / 2;
	    w.y = World.wormHoles[j].pos.y * BLOCK_SZ + BLOCK_SZ / 2;

	    /*
	     * Don't connect to balls while warping.
	     */
	    if (BIT(pl->used, OBJ_CONNECTOR))
		pl->ball = NULL;

	    if (BIT(pl->have, OBJ_BALL)) {
		object	*b;
		int k;

		/*
		 * Take every ball associated with player through worm hole.
		 * NB. the connector can cross a wall boundary this is
		 * allowed, so long as the ball itself doesn't collide.
		 */
		for (k = 0; k < NumObjs; k++) {
		    if (!BIT(Obj[k]->type, OBJ_BALL) || Obj[k]->id != pl->id)
			continue;
		    b = Obj[k];

		    b->pos.x = w.x + b->pos.x - pl->pos.x;
		    b->pos.y = w.y + b->pos.y - pl->pos.y;
		    b->vel.x /= WORM_BRAKE_FACTOR;
		    b->vel.y /= WORM_BRAKE_FACTOR;
		    b->prevpos = b->pos;
		}
	    }

	    pl->wormHoleDest = j;
	    pl->pos = w;
	    pl->vel.x /= WORM_BRAKE_FACTOR;
	    pl->vel.y /= WORM_BRAKE_FACTOR;
	    pl->prevpos = pl->pos;
	    pl->forceVisible += 15;

	    if (j != pl->wormHoleHit) {
		World.wormHoles[pl->wormHoleHit].lastplayer = i;
		World.wormHoles[pl->wormHoleHit].lastdest = j;
		World.wormHoles[pl->wormHoleHit].countdown = WORMCOUNT;
	    }

	    CLR_BIT(pl->status, WARPING);
	    SET_BIT(pl->status, WARPED);

            sound_play_sensors(pl->pos.x, pl->pos.y, WORM_HOLE_SOUND);
	}

	/*
	 * Update ECM blasts
	 */
	for (necm = pl->ecmInfo.count, ecm = 0; ecm < necm; ecm++) {
	    if ((pl->ecmInfo.size[ecm] >>= 1) == 0) {
		necm--;
		pl->ecmInfo.count--;
		pl->ecmInfo.pos[ecm] = pl->ecmInfo.pos[necm];
		pl->ecmInfo.size[ecm] = pl->ecmInfo.size[necm];
	    }
	}
	if (BIT(pl->used, OBJ_ECM)) {
	    if (necm < MAX_PLAYER_ECMS) {
		pl->ecmInfo.pos[necm].x = pl->pos.x;
		pl->ecmInfo.pos[necm].y = pl->pos.y;
		pl->ecmInfo.size[necm] = ECM_DISTANCE;
		pl->ecmInfo.count++;
	    }
	    CLR_BIT(pl->used, OBJ_ECM);
	}

	if (pl->transInfo.count)
	    pl->transInfo.count--;

	if (!BIT(pl->status, PAUSE)) {
	    update_object_speed(pl);	    /* New position */
	    Move_player(i);
	}

	if (BIT(pl->status, THRUSTING))
	    Thrust(i);
#ifdef TURN_FUEL
	if (tf)
            Turn_thrust(i, TURN_SPARKS(tf));
#endif

	/*
	 * Updating time player has used. Only used when timing
	 */
	if (pl->round != 0)
	    pl->time++;

	if (pl->round == 4) {
	    if ((pl->time < pl->best_run) || (pl->best_run == 0)) 
		pl->best_run = pl->time;
	    Players[i]->last_lap_time = Players[i]->time 
		- Players[i]->last_lap;
	    Kill_player(i);
	    pl->round = 0;
	    pl->check = 0;
	    pl->time = 0;
	}

	if (BIT(pl->lock.tagged, LOCK_PLAYER)) {
	    pl->lock.distance = Wrap_length(pl->pos.x -
				     Players[GetInd[pl->lock.pl_id]]->pos.x,
				     pl->pos.y -
				     Players[GetInd[pl->lock.pl_id]]->pos.y);
	    pl->sensor_range = MAX(pl->fuel.sum * ENERGY_RANGE_FACTOR,
				   VISIBILITY_DISTANCE);
	}

	pl->used &= pl->have;
    }

    for (i=0; i<World.NumWormholes; i++)
       if (World.wormHoles[i].countdown)
	   --World.wormHoles[i].countdown;
	    
    for (i = 0; i < NumPlayers; i++) {
	player *pl = Players[i];

	pl->updateVisibility = 0;

	if (pl->forceVisible) {
	    pl->forceVisible--;

	    if (!pl->forceVisible)
		pl->updateVisibility = 1;
	}

	if (BIT(pl->used, OBJ_TRACTOR_BEAM))
	    do_Tractor_beam (pl);
	else
	    pl->tractor = NULL;
    }

    /*
     * Checking for collision, updating score etc. (see collision.c)
     */
    Check_collision();


    /*
     * Update tanks, Kill players that ought to be killed.
     */
    for (i=NumPlayers-1; i>=0; i--) {
        player *pl = Players[i];
        
	if (BIT(pl->status, PLAYING|PAUSE|GAME_OVER|KILLED) == PLAYING)
	    Update_tanks(&(pl->fuel));
	if (BIT(pl->status, KILLED)) {
	    Throw_items(pl);
            if (pl->robot_mode != RM_OBJECT) {
	        Kill_player(i);
            } else {
                NumPseudoPlayers--;
                Explode(i);
                Delete_player(i);
		updateScores = true;
            }
        }
    }

    /*
     * Kill shots that ought to be dead.
     */
    for (i=NumObjs-1; i>=0; i--)
	if (--(Obj[i]->life) <= 0)
	    Delete_shot(i);

    /*
     * Compute general game status, do we have a winner?
     * (not called after Game_Over() )
     */
    if (gameDuration >= 0.0) {
	Compute_game_status();
    }

    /*
     * Now update labels if need be.
     */
    if (updateScores && loops % UPDATE_SCORE_DELAY == 0)
	Update_score_table();
}

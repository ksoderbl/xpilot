/* $Id: event.c,v 3.43 1994/04/14 08:13:15 bert Exp $
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
#include "score.h"
#include "map.h"
#include "robot.h"
#include "keys.h"
#include "saudio.h"
#include "bit.h"
#include "netserver.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: event.c,v 3.43 1994/04/14 08:13:15 bert Exp $";
#endif

#define SWAP(_a, _b)	    {float _tmp = _a; _a = _b; _b = _tmp;}

/*
 * Globals.
 */
static char		msg[MSG_LEN];



static void Refuel(int ind)
{
    player *pl = Players[ind];
    int i;
    float l, dist = 1e9;


    if (!BIT(pl->have, OBJ_REFUEL))
	return;

    CLR_BIT(pl->used, OBJ_REFUEL);
    for (i=0; i<World.NumFuels; i++) {
	l = Wrap_length(pl->pos.x - World.fuel[i].pos.x, 
			pl->pos.y - World.fuel[i].pos.y);
	if (BIT(pl->used, OBJ_REFUEL) == 0
	    || l < dist) {
	    SET_BIT(pl->used, OBJ_REFUEL);
	    pl->fs = i;
	    dist = l;
	}
    }
}


static void Repair(int ind)
{
    player *pl = Players[ind];
    int i;
    float l, dist = 1e9;
    float x, y;
    target_t *targ = World.targets;

    if (!BIT(pl->have, OBJ_REPAIR))
	return;

    CLR_BIT(pl->used, OBJ_REPAIR);
    for (i = 0; i < World.NumTargets; i++, targ++) {
	if (targ->team == pl->team
	    && targ->dead_time <= 0) {
	    x = targ->pos.x*BLOCK_SZ + BLOCK_SZ/2;
	    y = targ->pos.y*BLOCK_SZ + BLOCK_SZ/2;
	    l = Wrap_length(pl->pos.x - x, pl->pos.y - y);
	    if (BIT(pl->used, OBJ_REPAIR) == 0 || l < dist) {
		SET_BIT(pl->used, OBJ_REPAIR);
		pl->repair_target = i;
		dist = l;
	    }
	}
    }
}

int Player_lock_closest(int ind, int next)
{
    player *pl = Players[ind];
    int lock, i, newpl;
    float dist, best, l;

    if (!next)
	CLR_BIT(pl->lock.tagged, LOCK_PLAYER);
	
    if (BIT(pl->lock.tagged, LOCK_PLAYER)) {
	lock = GetInd[pl->lock.pl_id];
	dist = Wrap_length(Players[lock]->pos.x - pl->pos.x,
			   Players[lock]->pos.y - pl->pos.y);
    } else {
	lock = -1;
	dist = 0.0;
    }
    newpl = -1;
    best = FLT_MAX;
    for (i = 0; i < NumPlayers; i++) {
	if (lock == i || i == ind || TEAM(ind, i) ||
	    BIT(Players[i]->status, PLAYING|GAME_OVER) != PLAYING)
	    continue;
	l = Wrap_length(Players[i]->pos.x - pl->pos.x,
			Players[i]->pos.y - pl->pos.y);
	if (l >= dist && l < best) {
	    best = l;
	    newpl = i;
	}
    }
    if (newpl == -1)
	return 0;
    SET_BIT(pl->lock.tagged, LOCK_PLAYER);
    pl->lock.pl_id = Players[newpl]->id;
    return 1;
}
	

void Pause_player(int ind, int onoff)
{
    player		*pl = Players[ind];

    if (onoff) {
	if (!BIT(pl->status, PAUSE)) { /* Turn pause mode on */
	    pl->count = 10*FPS;
	    pl->updateVisibility = 1;
	    CLR_BIT(pl->status, SELF_DESTRUCT|PLAYING);
	    SET_BIT(pl->status, PAUSE);
	    pl->mychar = 'P';
	    updateScores = true;
	}
    } else {
	if (pl->count <= 0) {
	    CLR_BIT(pl->status, PAUSE);
	    updateScores = true;
	    if (!BIT(pl->mode, LIMITED_LIVES)) {
		pl->mychar = ' ';
		Go_home(ind);
		SET_BIT(pl->status, PLAYING);
	    } else {
		pl->life = 0;
		pl->mychar = 'W';
		SET_BIT(pl->status, GAME_OVER);
	    }
	}
    }
}


int Handle_keyboard(int ind)
{
    player  	*pl = Players[ind];
    int	    	i, j, k, key, pressed, xi, yi;
    float  	minv;


    for (key = 0; key < NUM_KEYS; key++) {
	if (pl->last_keyv[key / BITV_SIZE] == pl->prev_keyv[key / BITV_SIZE]) {
	    key |= (BITV_SIZE - 1);	/* Skip to next keyv element */
	    continue;
	}
	while (BITV_ISSET(pl->last_keyv, key)
	       == BITV_ISSET(pl->prev_keyv, key)) {
	    if (++key >= NUM_KEYS) {
		break;
	    }
	}
	if (key >= NUM_KEYS) {
	    break;
	}
	pressed = BITV_ISSET(pl->last_keyv, key) != 0;
	BITV_TOGGLE(pl->prev_keyv, key);

	/*
	 * Allow these functions while you're 'dead'.
	 */
	if (BIT(pl->status, PLAYING|GAME_OVER|PAUSE|HOVERPAUSE) != PLAYING) {
	    switch (key) {
	    case KEY_PAUSE:
	    case KEY_LOCK_NEXT:
	    case KEY_LOCK_PREV:
	    case KEY_LOCK_CLOSE:
	    case KEY_LOCK_NEXT_CLOSE:
	    case KEY_ID_MODE:
	    case KEY_TOGGLE_NUCLEAR:
	    case KEY_TOGGLE_CLUSTER:
	    case KEY_TOGGLE_IMPLOSION:
	    case KEY_TOGGLE_VELOCITY:
	    case KEY_TOGGLE_MINI:
	    case KEY_TOGGLE_SPREAD:
	    case KEY_TOGGLE_POWER:
	    case KEY_TOGGLE_LASER:
	    case KEY_TOGGLE_COMPASS:
	    case KEY_TOGGLE_OWNED_ITEMS:
	    case KEY_TOGGLE_MESSAGES:
	    case KEY_CLEAR_MODIFIERS:
	    case KEY_LOAD_MODIFIERS_1:
	    case KEY_LOAD_MODIFIERS_2:
	    case KEY_LOAD_MODIFIERS_3:
	    case KEY_LOAD_MODIFIERS_4:
	    case KEY_SWAP_SETTINGS:
	    case KEY_INCREASE_POWER:
	    case KEY_DECREASE_POWER:
	    case KEY_INCREASE_TURNSPEED:
	    case KEY_DECREASE_TURNSPEED:
	    case KEY_TANK_NEXT:
	    case KEY_TANK_PREV:
	    case KEY_TURN_LEFT:		/* Needed so that we don't get */
	    case KEY_TURN_RIGHT:	/* out-of-sync with the turnacc */
		break;
	    default:
		continue;
	    }
	}

	if (pressed) { /* --- KEYPRESS --- */
	    switch (key) {

	    case KEY_TANK_NEXT:
	    case KEY_TANK_PREV:
		if (pl->fuel.num_tanks) {
		    pl->fuel.current += (key==KEY_TANK_NEXT) ? 1 : -1;
		    if (pl->fuel.current < 0)
			pl->fuel.current = pl->fuel.num_tanks;
		    else if (pl->fuel.current > pl->fuel.num_tanks)
			pl->fuel.current = 0;
		}
		break;

	    case KEY_TANK_DETACH:
		Tank_handle_detach(pl);
		break;

	    case KEY_LOCK_NEXT:
	    case KEY_LOCK_PREV:
		j = i = GetInd[pl->lock.pl_id];
		do {
		    if (key == KEY_LOCK_PREV)
			i--;
		    else
			i++;
		    i = mod(i, NumPlayers);
		    if (i == j)
			break;
		} while (i == ind || BIT(Players[i]->status, GAME_OVER));
		pl->lock.pl_id = Players[i]->id;
		if (i == ind)
		    CLR_BIT(pl->lock.tagged, LOCK_PLAYER);
		else
		    SET_BIT(pl->lock.tagged, LOCK_PLAYER);
		break;

	    case KEY_TOGGLE_COMPASS:
		if (!BIT(pl->have, OBJ_COMPASS))
		    break;
		TOGGLE_BIT(pl->used, OBJ_COMPASS);
		if (BIT(pl->used, OBJ_COMPASS) == 0) {
		    break;
		}
		/*
		 * Verify if the lock has ever been initialized at all
		 * and if the lock is still valid.
		 */
		if (BIT(pl->lock.tagged, LOCK_PLAYER)
		    && NumPlayers > 1
		    && (k = pl->lock.pl_id) > 0
		    && k < Id
		    && (i = GetInd[k]) > 0
		    && i < NumPlayers
		    && Players[i]->id == k
		    && i != ind) {
		    break;
		}
		(void) Player_lock_closest(ind, 0);
		break;

	    case KEY_LOCK_NEXT_CLOSE:
		if (Player_lock_closest(ind, 1))
		    break;
	    case KEY_LOCK_CLOSE:
		(void) Player_lock_closest(ind, 0);
		break;

	    case KEY_CHANGE_HOME:
		xi = (int)pl->pos.x / BLOCK_SZ;
		yi = (int)pl->pos.y / BLOCK_SZ;
		if (World.block[xi][yi] == BASE) {
		    msg[0] = '\0';
		    for (i=0; i<World.NumBases; i++) {
			if (World.base[i].pos.x == xi
			    && World.base[i].pos.y == yi) {

			    if (i == pl->home_base) {
				break;
			    }
			    if (World.base[i].team != TEAM_NOT_SET
				&& World.base[i].team != pl->team)
				break;
			    pl->home_base = i;
			    sprintf(msg, "%s has changed home base.",
				    pl->name);
			    break;
			}
		    }
		    for (i=0; i<NumPlayers; i++)
			if (i != ind
			    && Players[i]->robot_mode != RM_OBJECT
			    && pl->home_base == Players[i]->home_base) {
			    Pick_startpos(i);
			    sprintf(msg, "%s has taken over %s's home base.",
				    pl->name, Players[i]->name);
			}
		    if (msg[0]) {
			sound_play_all(CHANGE_HOME_SOUND);
			Set_message(msg);
		    }
		    for (i = 0; i < NumPlayers; i++) {
			if (Players[i]->conn != NOT_CONNECTED) {
			    Send_base(Players[i]->conn,
				      pl->id, 
				      pl->home_base);
			}
		    }
		}
		break;

	    case KEY_SHIELD:
		if (BIT(pl->have, OBJ_SHIELD)) {
		    SET_BIT(pl->used, OBJ_SHIELD);
		    CLR_BIT(pl->used, OBJ_LASER);	/* don't remove! */
		}
		break;

	    case KEY_DROP_BALL:
		sound_play_sensors(pl->pos.x, pl->pos.y, DROP_BALL_SOUND);
		Detach_ball(ind, -1);
		break;

	    case KEY_FIRE_SHOT:
		if (!BIT(pl->used, OBJ_SHIELD|OBJ_FIRE)
		    && BIT(pl->have, OBJ_FIRE)) {
		    Fire_normal_shots(ind);
		}
		SET_BIT(pl->used, OBJ_FIRE);
		break;

	    case KEY_FIRE_MISSILE:
		if (pl->missiles > 0)
		    Fire_shot(ind, OBJ_SMART_SHOT, pl->dir);
		break;

	    case KEY_FIRE_HEAT:
		if (pl->missiles > 0)
		    Fire_shot(ind, OBJ_HEAT_SHOT, pl->dir);
		break;

	    case KEY_FIRE_TORPEDO:
		if (pl->missiles > 0)
		    Fire_shot(ind, OBJ_TORPEDO, pl->dir);

		break;

	    case KEY_FIRE_LASER:
		if (pl->lasers > 0 && BIT(pl->used, OBJ_SHIELD) == 0) {
		    SET_BIT(pl->used, OBJ_LASER);
		}
		break;

	    case KEY_TOGGLE_NUCLEAR:
		if (BIT(World.rules->mode, ALLOW_NUKES)) {
		    switch (pl->mods.nuclear) {
		    case NUCLEAR:
			pl->mods.nuclear |= FULLNUCLEAR;
			break;
		    case 0:
			pl->mods.nuclear = NUCLEAR;
			break;
		    default:
			pl->mods.nuclear = 0;
			break;
		    }
		}
		break;

	    case KEY_TOGGLE_CLUSTER:
		if (BIT(World.rules->mode, ALLOW_CLUSTERS)) {
		    TOGGLE_BIT(pl->mods.warhead, CLUSTER);
		}
		break;

	    case KEY_TOGGLE_IMPLOSION:
		if (BIT(World.rules->mode, ALLOW_MODIFIERS)) {
		    TOGGLE_BIT(pl->mods.warhead, IMPLOSION);
		}
		break;

	    case KEY_TOGGLE_VELOCITY:
		if (BIT(World.rules->mode, ALLOW_MODIFIERS)) {
		    /* NB. These may be bit fields, dont modify this code */
		    if (pl->mods.velocity == MODS_VELOCITY_MAX)
			pl->mods.velocity = 0;
		    else
			pl->mods.velocity++;
		}
		break;

	    case KEY_TOGGLE_MINI:
		if (BIT(World.rules->mode, ALLOW_MODIFIERS)) {
		    /* NB. These may be bit fields, dont modify this code */
		    if (pl->mods.mini == MODS_MINI_MAX)
			pl->mods.mini = 0;
		    else
			pl->mods.mini++;
		}
		break;

	    case KEY_TOGGLE_SPREAD:
		if (BIT(World.rules->mode, ALLOW_MODIFIERS)) {
		    /* NB. These may be bit fields, dont modify this code */
		    if (pl->mods.spread == MODS_SPREAD_MAX)
			pl->mods.spread = 0;
		    else
			pl->mods.spread++;
		}
		break;

	    case KEY_TOGGLE_LASER:
		if (BIT(World.rules->mode, ALLOW_LASER_MODIFIERS)) {
		    /* NB. These may be bit fields, dont modify this code */
		    if (pl->mods.laser == MODS_LASER_MAX)
			pl->mods.laser = 0;
		    else
			pl->mods.laser++;
		}
		break;

	    case KEY_TOGGLE_POWER:
		if (BIT(World.rules->mode, ALLOW_MODIFIERS)) {
		    /* NB. These may be bit fields, dont modify this code */
		    if (pl->mods.power == MODS_POWER_MAX)
			pl->mods.power = 0;
		    else
			pl->mods.power++;
		}
		break;

	    case KEY_CLEAR_MODIFIERS:
		CLEAR_MODS(pl->mods);
		break;

	    case KEY_LOAD_MODIFIERS_1:
	    case KEY_LOAD_MODIFIERS_2:
	    case KEY_LOAD_MODIFIERS_3:
	    case KEY_LOAD_MODIFIERS_4:
		pl->mods = pl->modbank[key - KEY_LOAD_MODIFIERS_1];
		break;

	    case KEY_TOGGLE_AUTOPILOT:
		if (BIT(pl->have, OBJ_AUTOPILOT))
		    Autopilot(ind, !BIT(pl->used, OBJ_AUTOPILOT));
		break;

	    case KEY_EMERGENCY_THRUST:
		if (BIT(pl->have, OBJ_EMERGENCY_THRUST))
		    Emergency_thrust(ind,!BIT(pl->used, OBJ_EMERGENCY_THRUST));
		break;

	    case KEY_DROP_MINE:
		Place_mine(ind);
		break;

	    case KEY_DETACH_MINE:
		Place_moving_mine(ind);
		break;

	    case KEY_TURN_LEFT:
	    case KEY_TURN_RIGHT:
		if (BIT(pl->used, OBJ_AUTOPILOT))
		    Autopilot(ind, 0);
		pl->turnacc = 0;
		if (BITV_ISSET(pl->last_keyv, KEY_TURN_LEFT)) {
		    pl->turnacc += pl->turnspeed;
		}
		if (BITV_ISSET(pl->last_keyv, KEY_TURN_RIGHT)) {
		    pl->turnacc -= pl->turnspeed;
		}
		break;

	    case KEY_SELF_DESTRUCT:
		TOGGLE_BIT(pl->status, SELF_DESTRUCT);
		if (BIT(pl->status, SELF_DESTRUCT))
		    pl->count = 150;
		break;

	    case KEY_PAUSE:
		xi = (int)pl->pos.x / BLOCK_SZ;
		yi = (int)pl->pos.y / BLOCK_SZ;
		j = World.base[pl->home_base].pos.x;
		k = World.base[pl->home_base].pos.y;
		if (j == xi && k == yi) {
		    minv = 1.0f;
		    i = PAUSE;
		} else {
		    /*
		     * Hover pause doesn't work within two squares of the
		     * players home base, they would want the better pause.
		     */
		    if (ABS(j - xi) <= 2 && ABS(k - yi) <= 2)
			break;
		    minv = 5.0f;
		    i = HOVERPAUSE;
		}
		minv += LENGTH(World.gravity[xi][yi].x,
			       World.gravity[xi][yi].y);
		if (pl->velocity > minv)
		    break;

		switch (i) {
		case PAUSE:
		    if (BIT(pl->status, HOVERPAUSE))
			break;

		    if (BIT(pl->used, OBJ_AUTOPILOT))
			Autopilot(ind, 0);

		    /* toggle pause mode */
		    Pause_player(ind, !BIT(pl->status, PAUSE));
		    if (BIT(pl->status, PLAYING)) {
			BITV_SET(pl->last_keyv, key);
			BITV_SET(pl->prev_keyv, key);
			return 1;
		    }
		    break;

		case HOVERPAUSE:
		    if (BIT(pl->status, PAUSE))
			break;

		    if (!BIT(pl->status, HOVERPAUSE)) {
			/*
			 * Turn hover pause on, together with shields.
			 */
			pl->count = 5*FPS;
			CLR_BIT(pl->status, SELF_DESTRUCT);
			SET_BIT(pl->status, HOVERPAUSE);

			if (BIT(pl->used, OBJ_EMERGENCY_THRUST))
			    Emergency_thrust(ind, 0);

			if (!BIT(pl->used, OBJ_AUTOPILOT))
			    Autopilot(ind, 1);

			/*
			 * Don't allow firing while paused. Similar
			 * reasons exist for refueling, connector and
			 * tractor beams.  Other items are allowed (esp.
			 * cloaking).
			 */
			CLR_BIT(pl->used, OBJ_FIRE|OBJ_CONNECTOR
				|OBJ_REFUEL|OBJ_TRACTOR_BEAM|OBJ_LASER);
			if (BIT(pl->have, OBJ_SHIELD))
			    SET_BIT(pl->used, OBJ_SHIELD);
		    } else if (pl->count <= 0) {
			Autopilot(ind, 0);
			CLR_BIT(pl->status, HOVERPAUSE);
			CLR_BIT(pl->used, OBJ_SHIELD);
		    }
		    break;
		}
		break;
		
	    case KEY_SWAP_SETTINGS:
		if (   BIT(pl->status, HOVERPAUSE)
		    || BIT(pl->used, OBJ_AUTOPILOT))
		    break;
		if (pl->turnacc == 0.0) {
		    SWAP(pl->power, pl->power_s);
		    SWAP(pl->turnspeed, pl->turnspeed_s);
		    SWAP(pl->turnresistance, pl->turnresistance_s);
		}
		break;

	    case KEY_REFUEL:
		Refuel(ind);
		break;

	    case KEY_REPAIR:
		Repair(ind);
		break;

	    case KEY_CONNECTOR:
		if (BIT(pl->have, OBJ_CONNECTOR))
		    SET_BIT(pl->used, OBJ_CONNECTOR);
		break;

	    case KEY_PRESSOR_BEAM:
		if (BIT(pl->have, OBJ_TRACTOR_BEAM)) {
		    pl->tractor_pressor = 1;
		    SET_BIT(pl->used, OBJ_TRACTOR_BEAM);
		}
		break;

	    case KEY_TRACTOR_BEAM:
		if (BIT(pl->have, OBJ_TRACTOR_BEAM)) {
		    pl->tractor_pressor = 0;
		    SET_BIT(pl->used, OBJ_TRACTOR_BEAM);
		}
		break;

	    case KEY_INCREASE_POWER:
		if (   BIT(pl->status, HOVERPAUSE)
		    || BIT(pl->used, OBJ_AUTOPILOT))
		    break;
		pl->power *= 1.10;
		pl->power = MIN(pl->power, MAX_PLAYER_POWER);
		break;

	    case KEY_DECREASE_POWER:
		if (   BIT(pl->status, HOVERPAUSE)
		    || BIT(pl->used, OBJ_AUTOPILOT))
		    break;
		pl->power *= 0.90;
		pl->power = MAX(pl->power, MIN_PLAYER_POWER);
		break;

	    case KEY_INCREASE_TURNSPEED:
		if (   BIT(pl->status, HOVERPAUSE)
		    || BIT(pl->used, OBJ_AUTOPILOT))
		    break;
		if (pl->turnacc == 0.0)
		    pl->turnspeed *= 1.05;
		pl->turnspeed = MIN(pl->turnspeed, MAX_PLAYER_TURNSPEED);
		break;

	    case KEY_DECREASE_TURNSPEED:
		if (   BIT(pl->status, HOVERPAUSE)
		    || BIT(pl->used, OBJ_AUTOPILOT))
		    break;
		if (pl->turnacc == 0.0)
		    pl->turnspeed *= 0.95;
		pl->turnspeed = MAX(pl->turnspeed, MIN_PLAYER_TURNSPEED);
		break;

	    case KEY_THRUST:
		if (BIT(pl->used, OBJ_AUTOPILOT))
		    Autopilot(ind, 0);
		SET_BIT(pl->status, THRUSTING);
		break;

	    case KEY_CLOAK:
		if (pl->cloaks > 0) {
		    sound_play_player(pl, CLOAK_SOUND);
		    pl->updateVisibility = 1;
		    TOGGLE_BIT(pl->used, OBJ_CLOAKING_DEVICE);
		}
		break;

	    case KEY_ECM:
		Fire_ecm(ind);
		break;

	    case KEY_TRANSPORTER:
		if (pl->transporters > 0 && pl->fuel.sum > -ED_TRANSPORTER) {
		    do_transporter(pl);
		    pl->transporters--;
		    Add_fuel(&(pl->fuel), ED_TRANSPORTER);
		    }
		break;

	    default:
		break;
	    }
	} else {
	    /* --- KEYRELEASE --- */
	    switch (key) {
	    case KEY_TURN_LEFT:
	    case KEY_TURN_RIGHT:
		if (BIT(pl->used, OBJ_AUTOPILOT))
		    Autopilot(ind, 0);
		pl->turnacc = 0;
		if (BITV_ISSET(pl->last_keyv, KEY_TURN_LEFT)) {
		    pl->turnacc += pl->turnspeed;
		}
		if (BITV_ISSET(pl->last_keyv, KEY_TURN_RIGHT)) {
		    pl->turnacc -= pl->turnspeed;
		}
		break;

	    case KEY_REFUEL:
		CLR_BIT(pl->used, OBJ_REFUEL);
		break;

	    case KEY_REPAIR:
		CLR_BIT(pl->used, OBJ_REPAIR);
		break;

	    case KEY_CONNECTOR:
		CLR_BIT(pl->used, OBJ_CONNECTOR);
		break;

	    case KEY_TRACTOR_BEAM:
	    case KEY_PRESSOR_BEAM:
		CLR_BIT(pl->used, OBJ_TRACTOR_BEAM);
		break;

	    case KEY_SHIELD:
		CLR_BIT(pl->used, OBJ_SHIELD|OBJ_LASER);
		break;

	    case KEY_FIRE_SHOT:
		CLR_BIT(pl->used, OBJ_FIRE);
		break;

	    case KEY_FIRE_LASER:
		CLR_BIT(pl->used, OBJ_LASER);
		break;

	    case KEY_THRUST:
		if (BIT(pl->used, OBJ_AUTOPILOT))
		    Autopilot(ind, 0);
		CLR_BIT(pl->status, THRUSTING);
		break;

	    default:
		break;
	    }
	}
    }
    memcpy(pl->prev_keyv, pl->last_keyv, sizeof(pl->last_keyv));
    pl->key_changed = 0;

    return 1;
}

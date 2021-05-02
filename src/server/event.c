/* $Id: event.c,v 5.4 2001/06/02 21:02:42 bertg Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#ifdef _WINDOWS
# include "NT/winServer.h"
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "score.h"
#include "map.h"
#include "saudio.h"
#include "bit.h"
#include "netserver.h"

char event_version[] = VERSION;


#define SWAP(_a, _b)	    {DFLOAT _tmp = _a; _a = _b; _b = _tmp;}

/*
 * Globals.
 */
static char		msg[MSG_LEN];



static void Refuel(int ind)
{
    player *pl = Players[ind];
    int i;
    DFLOAT l, dist = 1e9;


    if (!BIT(pl->have, HAS_REFUEL))
	return;

    CLR_BIT(pl->used, HAS_REFUEL);
    for (i=0; i<World.NumFuels; i++) {
	if (World.block[World.fuel[i].blk_pos.x]
		       [World.fuel[i].blk_pos.y] == FUEL) {
	    l = Wrap_length(pl->pos.x - World.fuel[i].pix_pos.x,
			    pl->pos.y - World.fuel[i].pix_pos.y);
	    if (BIT(pl->used, HAS_REFUEL) == 0
		|| l < dist) {
		SET_BIT(pl->used, HAS_REFUEL);
		pl->fs = i;
		dist = l;
	    }
	}
    }
}


static void Repair(int ind)
{
    player *pl = Players[ind];
    int i;
    DFLOAT l, dist = 1e9;
    DFLOAT x, y;
    target_t *targ = World.targets;

    if (!BIT(pl->have, HAS_REPAIR))
	return;

    CLR_BIT(pl->used, HAS_REPAIR);
    for (i = 0; i < World.NumTargets; i++, targ++) {
	if (targ->team == pl->team
	    && targ->dead_time <= 0) {
	    x = targ->pos.x*BLOCK_SZ + BLOCK_SZ/2;
	    y = targ->pos.y*BLOCK_SZ + BLOCK_SZ/2;
	    l = Wrap_length(pl->pos.x - x, pl->pos.y - y);
	    if (BIT(pl->used, HAS_REPAIR) == 0 || l < dist) {
		SET_BIT(pl->used, HAS_REPAIR);
		pl->repair_target = i;
		dist = l;
	    }
	}
    }
}

bool team_dead(int team)
{
    int i;
    bool alive = false;

    for (i = 0; i < NumPlayers; i++) {
	if (Players[i]->team == team &&
	    BIT(Players[i]->status, PLAYING|GAME_OVER) == PLAYING) {
	    alive = true;
	    break;
	}
    }
    return (!alive);
}

/*
 * Return true if a lock is allowed.
 */
static bool Player_lock_allowed(int ind, int lock)
{
    player		*pl = Players[ind];

    /* we can never lock on ourselves, nor on -1. */
    if (ind == lock || lock == -1) {
	return false;
    }

    /* if we are actively playing then we can lock since we are not viewing. */
    if (BIT(pl->status, PLAYING|PAUSE|GAME_OVER) == PLAYING) {
	return true;
    }

    /* if there is no team play then we can always lock on anyone. */
    if (!BIT(World.rules->mode, TEAM_PLAY)) {
	return true;
    }

    /* we can always lock on players from our own team. */
    if (TEAM(ind, lock)) {
	return true;
    }

    /* if lockOtherTeam is true then we can always lock on other teams. */
    if (lockOtherTeam) {
	return true;
    }

    /* if our own team is dead then we can lock on anyone. */
    if (team_dead(pl->team)) {
	return true;
    }

    /* can't find any reason why this lock should be allowed. */
    return false;
}

/*
 * Sven Mascheck:
 * If all _opponents are paused, then even LOCK_NEXT (ot LOCK_PREV)
 * might not lock_next (or lock_prev), as Player_lock_closest() might
 * be called  [ "event.c" line 466 ] :
 * This happens when the player is not locked on any one anymore -
 * and this happens if he tried to lock_closest before (if all
 * opponents are paused).
 * Player_lock_closest() is called with (ind, 0) and that means that
 * the lock is cleared in _any case_ with the current code - that could
 * be done without calling Player_lock_closest().
 * (btw, code in Player_lock_closest() looks like 'evolutionary code :)
 * I am not sure where to fix that locking problem
 * ( in "case KEY_LOCK_NEXT" or in Player_lock_closest() ).
 * I tried to find a solution but now i am bit screwed up..  :)
 */
int Player_lock_closest(int ind, int next)
{
    player *pl = Players[ind];
    int lock, i, newpl;
    DFLOAT dist, best, l;

    if (!next) {
	CLR_BIT(pl->lock.tagged, LOCK_PLAYER);
    }

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
	if (i == lock
	    || (BIT(Players[i]->status, PLAYING|PAUSE|GAME_OVER) != PLAYING)
	    || !Player_lock_allowed(ind, i)
	    || TEAM(ind,i)) {
	    continue;
	}
	l = Wrap_length(Players[i]->pos.x - pl->pos.x,
			Players[i]->pos.y - pl->pos.y);
	if (l >= dist && l < best) {
	    best = l;
	    newpl = i;
	}
    }
    if (newpl == -1) {
	return 0;
    }

    SET_BIT(pl->lock.tagged, LOCK_PLAYER);
    pl->lock.pl_id = Players[newpl]->id;

    return 1;
}


void Pause_player(int ind, int onoff)
{
    player		*pl = Players[ind];
    int			i;

    if (onoff != 0 && !BIT(pl->status, PAUSE)) { /* Turn pause mode on */
	pl->count = 10*FPS;
	pl->updateVisibility = 1;
	CLR_BIT(pl->status, SELF_DESTRUCT|PLAYING);
	SET_BIT(pl->status, PAUSE);
	pl->mychar = 'P';
	updateScores = true;
	if (BIT(pl->have, HAS_BALL))
	    Detach_ball(ind, -1);
    }
    else if (onoff == 0 && BIT(pl->status, PAUSE)) { /* Turn pause mode off */
	if (pl->count <= 0) {
	    bool toolate = false;

	    CLR_BIT(pl->status, PAUSE);
	    updateScores = true;
	    if (BIT(World.rules->mode, LIMITED_LIVES)) {
		for (i = 0; i < NumPlayers; i++) {
		    /* If a non-team member has lost a life,
		     * then it's too late to join. */
		    if (i == ind) {
			continue;
		    }
		    if (Players[i]->life < World.rules->lives && !TEAM(ind, i)) {
			toolate = true;
			break;
		    }
		}
	    }
	    if (toolate) {
		pl->life = 0;
		pl->mychar = 'W';
		SET_BIT(pl->status, GAME_OVER);
	    } else {
		pl->mychar = ' ';
		Go_home(ind);
		SET_BIT(pl->status, PLAYING);
		if (BIT(World.rules->mode, LIMITED_LIVES)) {
		    pl->life = World.rules->lives;
		}
	    }
	    if (BIT(World.rules->mode, TIMING)) {
		pl->round = 0;
		pl->check = 0;
		pl->time = 0;
		pl->best_lap = 0;
		pl->last_lap = 0;
		pl->last_lap_time = 0;
	    }
	}
    }
}


int Handle_keyboard(int ind)
{
    player  	*pl = Players[ind];
    int	    	i, j, k, key, pressed, xi, yi;
    DFLOAT  	minv;


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
	if (key != KEY_SHIELD)	/* would interfere with auto-idle-pause.. */
	    pl->frame_last_busy = frame_loops;	/* due to client auto-shield */

	/*
	 * Allow these functions before a round has started.
	 */
	if (round_delay > 0 && pressed) {
	    switch (key) {
	    case KEY_LOCK_NEXT:
	    case KEY_LOCK_PREV:
	    case KEY_LOCK_CLOSE:
	    case KEY_TOGGLE_NUCLEAR:
	    case KEY_PAUSE:
	    case KEY_TANK_NEXT:
	    case KEY_TANK_PREV:
	    case KEY_TOGGLE_VELOCITY:
	    case KEY_TOGGLE_CLUSTER:
	    case KEY_SWAP_SETTINGS:
	    case KEY_INCREASE_POWER:
	    case KEY_DECREASE_POWER:
	    case KEY_INCREASE_TURNSPEED:
	    case KEY_DECREASE_TURNSPEED:
	    case KEY_LOCK_NEXT_CLOSE:
	    case KEY_TOGGLE_COMPASS:
	    case KEY_TOGGLE_MINI:
	    case KEY_TOGGLE_SPREAD:
	    case KEY_TOGGLE_POWER:
	    case KEY_TOGGLE_LASER:
	    case KEY_CLEAR_MODIFIERS:
	    case KEY_LOAD_MODIFIERS_1:
	    case KEY_LOAD_MODIFIERS_2:
	    case KEY_LOAD_MODIFIERS_3:
	    case KEY_LOAD_MODIFIERS_4:
	    case KEY_SELECT_ITEM:
	    case KEY_TOGGLE_IMPLOSION:
	    case KEY_REPROGRAM:
	    case KEY_LOAD_LOCK_1:
	    case KEY_LOAD_LOCK_2:
	    case KEY_LOAD_LOCK_3:
	    case KEY_LOAD_LOCK_4:
		break;
	    default:
		continue;
	    }
	}

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
	    case KEY_TOGGLE_NUCLEAR:
	    case KEY_TOGGLE_CLUSTER:
	    case KEY_TOGGLE_IMPLOSION:
	    case KEY_TOGGLE_VELOCITY:
	    case KEY_TOGGLE_MINI:
	    case KEY_TOGGLE_SPREAD:
	    case KEY_TOGGLE_POWER:
	    case KEY_TOGGLE_LASER:
	    case KEY_TOGGLE_COMPASS:
	    case KEY_CLEAR_MODIFIERS:
	    case KEY_LOAD_MODIFIERS_1:
	    case KEY_LOAD_MODIFIERS_2:
	    case KEY_LOAD_MODIFIERS_3:
	    case KEY_LOAD_MODIFIERS_4:
	    case KEY_LOAD_LOCK_1:
	    case KEY_LOAD_LOCK_2:
	    case KEY_LOAD_LOCK_3:
	    case KEY_LOAD_LOCK_4:
	    case KEY_REPROGRAM:
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

	/* allow these functions while you're phased */
	if (BIT(pl->used, HAS_PHASING_DEVICE) && pressed) {
	    switch (key) {
	    case KEY_LOCK_NEXT:
	    case KEY_LOCK_PREV:
	    case KEY_LOCK_CLOSE:
	    case KEY_SHIELD:
	    case KEY_TOGGLE_NUCLEAR:
	    case KEY_TURN_LEFT:
	    case KEY_TURN_RIGHT:
	    case KEY_SELF_DESTRUCT:
	    case KEY_LOSE_ITEM:
	    case KEY_PAUSE:
	    case KEY_TANK_NEXT:
	    case KEY_TANK_PREV:
	    case KEY_TOGGLE_VELOCITY:
	    case KEY_TOGGLE_CLUSTER:
	    case KEY_SWAP_SETTINGS:
	    case KEY_INCREASE_POWER:
	    case KEY_DECREASE_POWER:
	    case KEY_INCREASE_TURNSPEED:
	    case KEY_DECREASE_TURNSPEED:
	    case KEY_THRUST:
	    case KEY_CLOAK:
	    case KEY_DROP_BALL:
	    case KEY_TALK:
	    case KEY_LOCK_NEXT_CLOSE:
	    case KEY_TOGGLE_COMPASS:
	    case KEY_TOGGLE_MINI:
	    case KEY_TOGGLE_SPREAD:
	    case KEY_TOGGLE_POWER:
	    case KEY_TOGGLE_LASER:
	    case KEY_EMERGENCY_THRUST:
	    case KEY_CLEAR_MODIFIERS:
	    case KEY_LOAD_MODIFIERS_1:
	    case KEY_LOAD_MODIFIERS_2:
	    case KEY_LOAD_MODIFIERS_3:
	    case KEY_LOAD_MODIFIERS_4:
	    case KEY_SELECT_ITEM:
	    case KEY_PHASING:
	    case KEY_TOGGLE_IMPLOSION:
	    case KEY_REPROGRAM:
	    case KEY_LOAD_LOCK_1:
	    case KEY_LOAD_LOCK_2:
	    case KEY_LOAD_LOCK_3:
	    case KEY_LOAD_LOCK_4:
	    case KEY_EMERGENCY_SHIELD:
	    case KEY_TOGGLE_AUTOPILOT:
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
		if (!BIT(pl->lock.tagged, LOCK_PLAYER)
		    || j < 0 || j >= NumPlayers) {
		    /* better jump to KEY_LOCK_CLOSE... */
		    Player_lock_closest(ind, 0);
		    break;
		}
		do {
		    if (key == KEY_LOCK_PREV) {
			if (--i < 0) {
			    i = NumPlayers - 1;
			}
		    } else {
			if (++i >= NumPlayers) {
			    i = 0;
			}
		    }
		    if (i == j)
			break;
		} while (i == ind
			 || BIT(Players[i]->status, GAME_OVER|PAUSE)
			 || !Player_lock_allowed(ind, i));
		if (i == ind) {
		    CLR_BIT(pl->lock.tagged, LOCK_PLAYER);
		}
		else {
		    pl->lock.pl_id = Players[i]->id;
		    SET_BIT(pl->lock.tagged, LOCK_PLAYER);
		}
		break;

	    case KEY_TOGGLE_COMPASS:
		if (!BIT(pl->have, HAS_COMPASS))
		    break;
		TOGGLE_BIT(pl->used, HAS_COMPASS);
		if (BIT(pl->used, HAS_COMPASS) == 0) {
		    break;
		}
		/*
		 * Verify if the lock has ever been initialized at all
		 * and if the lock is still valid.
		 */
		if (BIT(pl->lock.tagged, LOCK_PLAYER)
		    && NumPlayers > 1
		    && (k = pl->lock.pl_id) > 0
		    && (i = GetInd[k]) > 0
		    && i < NumPlayers
		    && Players[i]->id == k
		    && i != ind) {
		    break;
		}
		Player_lock_closest(ind, 0);
		break;

	    case KEY_LOCK_NEXT_CLOSE:
		if (!Player_lock_closest(ind, 1)) {
		    Player_lock_closest(ind, 0);
		}
		break;

	    case KEY_LOCK_CLOSE:
		Player_lock_closest(ind, 0);
		break;

	    case KEY_CHANGE_HOME:
		xi = OBJ_X_IN_BLOCKS(pl);
		yi = OBJ_Y_IN_BLOCKS(pl);
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
			    && !IS_TANK_IND(i)
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
		if (BIT(pl->have, HAS_SHIELD)) {
		    SET_BIT(pl->used, HAS_SHIELD);
		    CLR_BIT(pl->used, HAS_LASER);	/* don't remove! */
		}
		break;

	    case KEY_DROP_BALL:
		Detach_ball(ind, -1);
		break;

	    case KEY_FIRE_SHOT:
		if (!BIT(pl->used, HAS_SHIELD|HAS_SHOT)
		    && BIT(pl->have, HAS_SHOT)) {
		    SET_BIT(pl->used, HAS_SHOT);
		    Fire_normal_shots(ind);
		}
		break;

	    case KEY_FIRE_MISSILE:
		if (pl->item[ITEM_MISSILE] > 0)
		    Fire_shot(ind, OBJ_SMART_SHOT, pl->dir);
		break;

	    case KEY_FIRE_HEAT:
		if (pl->item[ITEM_MISSILE] > 0)
		    Fire_shot(ind, OBJ_HEAT_SHOT, pl->dir);
		break;

	    case KEY_FIRE_TORPEDO:
		if (pl->item[ITEM_MISSILE] > 0)
		    Fire_shot(ind, OBJ_TORPEDO, pl->dir);
		break;

	    case KEY_FIRE_LASER:
		if (pl->item[ITEM_LASER] > 0 && BIT(pl->used, HAS_SHIELD) == 0) {
		    SET_BIT(pl->used, HAS_LASER);
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

	    case KEY_REPROGRAM:
		SET_BIT(pl->status, REPROGRAM);
		break;

	    case KEY_LOAD_MODIFIERS_1:
	    case KEY_LOAD_MODIFIERS_2:
	    case KEY_LOAD_MODIFIERS_3:
	    case KEY_LOAD_MODIFIERS_4: {
		modifiers *m = &(pl->modbank[key - KEY_LOAD_MODIFIERS_1]);

		if (BIT(pl->status, REPROGRAM))
		    *m = pl->mods;
		else
		    pl->mods = *m;
		break;
	    }

	    case KEY_LOAD_LOCK_1:
	    case KEY_LOAD_LOCK_2:
	    case KEY_LOAD_LOCK_3:
	    case KEY_LOAD_LOCK_4: {
		int *l = &(pl->lockbank[key - KEY_LOAD_LOCK_1]);

		if (BIT(pl->status, REPROGRAM)) {
		    if (BIT(pl->lock.tagged, LOCK_PLAYER)) {
			*l = pl->lock.pl_id;
		    }
		} else {
		    if (*l != NOT_CONNECTED
			    && Player_lock_allowed(ind, GetInd[*l])) {
			pl->lock.pl_id = *l;
			SET_BIT(pl->lock.tagged, LOCK_PLAYER);
		    }
		}
		break;
	    }

	    case KEY_TOGGLE_AUTOPILOT:
		if (BIT(pl->have, HAS_AUTOPILOT))
		    Autopilot(ind, !BIT(pl->used, HAS_AUTOPILOT));
		break;

	    case KEY_EMERGENCY_THRUST:
		if (BIT(pl->have, HAS_EMERGENCY_THRUST))
		    Emergency_thrust(ind,!BIT(pl->used, HAS_EMERGENCY_THRUST));
		break;

	    case KEY_EMERGENCY_SHIELD:
		if (BIT(pl->have, HAS_EMERGENCY_SHIELD))
		    Emergency_shield(ind,!BIT(pl->used, HAS_EMERGENCY_SHIELD));
		break;

	    case KEY_DROP_MINE:
		Place_mine(ind);
		break;

	    case KEY_DETACH_MINE:
		Place_moving_mine(ind);
		break;

	    case KEY_DETONATE_MINES:
		Detonate_mines(ind);
		break;

	    case KEY_TURN_LEFT:
	    case KEY_TURN_RIGHT:
		if (BIT(pl->used, HAS_AUTOPILOT))
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
		if (BIT(pl->status, PAUSE)) {
		    i = PAUSE;
		}
		else if (BIT(pl->status, HOVERPAUSE)) {
		    i = HOVERPAUSE;
		}
		else {
		    xi = OBJ_X_IN_BLOCKS(pl);
		    yi = OBJ_Y_IN_BLOCKS(pl);
		    j = World.base[pl->home_base].pos.x;
		    k = World.base[pl->home_base].pos.y;
		    if (j == xi && k == yi) {
			minv = 3.0f;
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
		    minv += VECTOR_LENGTH(World.gravity[xi][yi]);
		    if (pl->velocity > minv)
			break;
		}

		switch (i) {
		case PAUSE:
		    if (BIT(pl->status, HOVERPAUSE))
			break;

		    if (BIT(pl->used, HAS_AUTOPILOT))
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

			if (BIT(pl->used, HAS_EMERGENCY_THRUST))
			    Emergency_thrust(ind, 0);

			if (BIT(pl->used, HAS_EMERGENCY_SHIELD))
			    Emergency_shield(ind, 0);

			if (!BIT(pl->used, HAS_AUTOPILOT))
			    Autopilot(ind, 1);

			if (BIT(pl->used, HAS_PHASING_DEVICE))
			    Phasing(ind, 0);

			/*
			 * Don't allow firing while paused. Similar
			 * reasons exist for refueling, connector and
			 * tractor beams.  Other items are allowed (esp.
			 * cloaking).
			 */
			pl->used &= ~USED_KILL;
			if (BIT(pl->have, HAS_SHIELD))
			    SET_BIT(pl->used, HAS_SHIELD);
		    } else if (pl->count <= 0) {
			Autopilot(ind, 0);
			CLR_BIT(pl->status, HOVERPAUSE);
			if (!BIT(pl->have, HAS_SHIELD)) {
			    CLR_BIT(pl->used, HAS_SHIELD);
			}
		    }
		    break;
		}
		break;

	    case KEY_SWAP_SETTINGS:
		if (   BIT(pl->status, HOVERPAUSE)
		    || BIT(pl->used, HAS_AUTOPILOT))
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
		if (BIT(pl->have, HAS_CONNECTOR))
		    SET_BIT(pl->used, HAS_CONNECTOR);
		break;

	    case KEY_PRESSOR_BEAM:
		if (BIT(pl->have, HAS_TRACTOR_BEAM)) {
		    pl->tractor_is_pressor = true;
		    SET_BIT(pl->used, HAS_TRACTOR_BEAM);
		}
		break;

	    case KEY_TRACTOR_BEAM:
		if (BIT(pl->have, HAS_TRACTOR_BEAM)) {
		    pl->tractor_is_pressor = false;
		    SET_BIT(pl->used, HAS_TRACTOR_BEAM);
		}
		break;

	    case KEY_INCREASE_POWER:
		if (   BIT(pl->status, HOVERPAUSE)
		    || BIT(pl->used, HAS_AUTOPILOT))
		    break;
		pl->power *= 1.10;
		pl->power = MIN(pl->power, MAX_PLAYER_POWER);
		break;

	    case KEY_DECREASE_POWER:
		if (   BIT(pl->status, HOVERPAUSE)
		    || BIT(pl->used, HAS_AUTOPILOT))
		    break;
		pl->power *= 0.90;
		pl->power = MAX(pl->power, MIN_PLAYER_POWER);
		break;

	    case KEY_INCREASE_TURNSPEED:
		if (   BIT(pl->status, HOVERPAUSE)
		    || BIT(pl->used, HAS_AUTOPILOT))
		    break;
		if (pl->turnacc == 0.0)
		    pl->turnspeed *= 1.05;
		pl->turnspeed = MIN(pl->turnspeed, MAX_PLAYER_TURNSPEED);
		break;

	    case KEY_DECREASE_TURNSPEED:
		if (   BIT(pl->status, HOVERPAUSE)
		    || BIT(pl->used, HAS_AUTOPILOT))
		    break;
		if (pl->turnacc == 0.0)
		    pl->turnspeed *= 0.95;
		pl->turnspeed = MAX(pl->turnspeed, MIN_PLAYER_TURNSPEED);
		break;

	    case KEY_THRUST:
		if (BIT(pl->used, HAS_AUTOPILOT))
		    Autopilot(ind, 0);
		SET_BIT(pl->status, THRUSTING);
		break;

	    case KEY_CLOAK:
		if (pl->item[ITEM_CLOAK] > 0) {
		    sound_play_player(pl, CLOAK_SOUND);
		    pl->updateVisibility = 1;
		    TOGGLE_BIT(pl->used, HAS_CLOAKING_DEVICE);
		}
		break;

	    case KEY_ECM:
		Fire_ecm(ind);
		break;

	    case KEY_TRANSPORTER:
		Do_transporter(ind);
		break;

	    case KEY_DEFLECTOR:
		if (pl->item[ITEM_DEFLECTOR] > 0) {
		    TOGGLE_BIT(pl->used, HAS_DEFLECTOR);
		    sound_play_player(pl, DEFLECTOR_SOUND);
		}
		break;

	    case KEY_HYPERJUMP:
		if (pl->item[ITEM_HYPERJUMP] > 0 && pl->fuel.sum > -ED_HYPERJUMP) {
		    pl->item[ITEM_HYPERJUMP]--;
		    Add_fuel(&(pl->fuel), ED_HYPERJUMP);
		    do_hyperjump(pl);
		}
		break;

	    case KEY_PHASING:
		if (BIT(pl->have, HAS_PHASING_DEVICE)) {
		    Phasing(ind, !BIT(pl->used, HAS_PHASING_DEVICE));
		}
		break;

	    case KEY_SELECT_ITEM:
		for (i = 0; i < NUM_ITEMS; i++) {
		    if (++pl->lose_item >= NUM_ITEMS) {
			pl->lose_item = 0;
		    }
		    if (BIT(1U << pl->lose_item, ITEM_BIT_FUEL | ITEM_BIT_TANK)) {
			/* can't lose fuel or tanks. */
			continue;
		    }
		    if (pl->item[pl->lose_item] > 0) {
			pl->lose_item_state = 2;	/* 2: key down; 1: key up */
			break;
		    }
		}
		break;

	    case KEY_LOSE_ITEM:
		do_lose_item(ind);
		break;

	    default:
		break;
	    }
	} else {
	    /* --- KEYRELEASE --- */
	    switch (key) {
	    case KEY_TURN_LEFT:
	    case KEY_TURN_RIGHT:
		if (BIT(pl->used, HAS_AUTOPILOT))
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
		CLR_BIT(pl->used, HAS_REFUEL);
		break;

	    case KEY_REPAIR:
		CLR_BIT(pl->used, HAS_REPAIR);
		break;

	    case KEY_CONNECTOR:
		CLR_BIT(pl->used, HAS_CONNECTOR);
		break;

	    case KEY_TRACTOR_BEAM:
	    case KEY_PRESSOR_BEAM:
		CLR_BIT(pl->used, HAS_TRACTOR_BEAM);
		break;

	    case KEY_SHIELD:
		if (BIT(pl->used, HAS_SHIELD) && round_delay == 0) {
		    CLR_BIT(pl->used, HAS_SHIELD|HAS_LASER);
		    /*
		     * Insert the default fireRepeatRate between lowering
		     * shields and firing in order to prevent macros
		     * and hacked clients.
		     */
		    pl->shot_time = frame_loops;
		}
		break;

	    case KEY_FIRE_SHOT:
		CLR_BIT(pl->used, HAS_SHOT);
		break;

	    case KEY_FIRE_LASER:
		CLR_BIT(pl->used, HAS_LASER);
		break;

	    case KEY_THRUST:
		if (BIT(pl->used, HAS_AUTOPILOT))
		    Autopilot(ind, 0);
		CLR_BIT(pl->status, THRUSTING);
		break;

	    case KEY_REPROGRAM:
		CLR_BIT(pl->status, REPROGRAM);
		break;

	    case KEY_SELECT_ITEM:
		pl->lose_item_state = 1;
		break;

	    default:
		break;
	    }
	}
    }
    memcpy(pl->prev_keyv, pl->last_keyv, sizeof(pl->last_keyv));

    return 1;
}

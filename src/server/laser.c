/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
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
#include <errno.h>
#include <math.h>
#include <limits.h>

#define SERVER
#include "xpconfig.h"
#include "serverconst.h"
#include "list.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "score.h"
#include "saudio.h"
#include "error.h"
#include "portability.h"
#include "objpos.h"
#include "asteroid.h"
#include "commonproto.h"


/*
 * Type to hold info about a player
 * which might be hit by a laser pulse.
 */
typedef struct victim {
    int			ind;		/* player index */
    position		pos;		/* current player position */
    DFLOAT		prev_dist;	/* distance at previous sample */
} victim_t;


/*
 * Type to hold info about all players
 * which may be hit by a laser pulse.
 */
typedef struct vicbuf {
    int			num_vic;	/* number of victims. */
    int			max_vic;	/* max number */
    victim_t		*vic_ptr;	/* pointer to buffer for victims */
} vicbuf_t;


/*
 * Destroy one laser pulse.
 */
static void Laser_pulse_destroy_one(int pulse_index)
{
    int			ind;
    player		*pl;
    pulse_t		*pulse_ptr;

    pulse_ptr = Pulses[pulse_index];
    if (pulse_ptr->id != NO_ID) {
	ind = GetInd[pulse_ptr->id];
	pl = Players[ind];
	pl->num_pulses--;
    }

    free(pulse_ptr);

    if (--NumPulses > pulse_index) {
	Pulses[pulse_index] = Pulses[NumPulses];
    }
}


/*
 * Destroy all laser pulses.
 */
static void Laser_pulse_destroy_all(void)
{
    int			p;

    for (p = NumPulses - 1; p >= 0; --p) {
	Laser_pulse_destroy_one(p);
    }
}


/*
 * Loop over all players and put the
 * ones which are close the pulse midpoint
 * in a vicbuf structure.
 */
static void Laser_pulse_find_victims(
	vicbuf_t *vicbuf, 
	pulse_t *pulse,
	DFLOAT midx,
	DFLOAT midy)
{
    int		i;
    player	*vic;
    DFLOAT	dist;

    vicbuf->num_vic = 0;
    for (i = 0; i < NumPlayers; i++) {
	vic = Players[i];
	if (BIT(vic->status, PLAYING|GAME_OVER|KILLED|PAUSE)
	    != PLAYING) {
	    continue;
	}
	if (BIT(vic->used, HAS_PHASING_DEVICE)) {
	    continue;
	}
	if (vic->id == pulse->id
	    && selfImmunity) {
	    continue;
	}
	if (selfImmunity &&
	    IS_TANK_PTR(vic) &&
	    vic->lock.pl_id == pulse->id) {
	    continue;
	}
	if (Team_immune(vic->id, pulse->id)) {
	    continue;
	}
	/* special case for cannon pulses */
	if (pulse->id == NO_ID &&
	    teamImmunity &&
	    BIT(World.rules->mode, TEAM_PLAY) &&
	    pulse->team == vic->team) {
	    continue;
	}
	if (vic->id == pulse->id && !pulse->refl) {
	    continue;
	}
	dist = Wrap_length(vic->pos.x - midx, vic->pos.y - midy);
	if (dist > pulse->len / 2 + SHIP_SZ) {
	    continue;
	}
	if (vicbuf->max_vic == 0) {
	    size_t victim_bufsize = NumPlayers * sizeof(victim_t);
	    vicbuf->vic_ptr = (victim_t *) malloc(victim_bufsize);
	    if (vicbuf->vic_ptr == NULL) {
		break;
	    }
	    vicbuf->max_vic = NumPlayers;
	}
	vicbuf->vic_ptr[vicbuf->num_vic].ind = i;
	vicbuf->vic_ptr[vicbuf->num_vic].pos.x = vic->pos.x;
	vicbuf->vic_ptr[vicbuf->num_vic].pos.y = vic->pos.y;
	vicbuf->vic_ptr[vicbuf->num_vic].prev_dist = 1e10;
	vicbuf->num_vic++;
    }
}


/*
 * Do what needs to be done when a laser pulse
 * actually hits a player.
 * If the pulse was reflected by a mirror
 * then set "refl" to true.
 */
static void Laser_pulse_hits_player(
	pulse_t *pulse,
	object *obj,
	DFLOAT x,
	DFLOAT y,
	victim_t *victim,
	bool *refl)
{
    player		*pl;
    player		*vicpl;
    int			ind;
    DFLOAT		sc;
    char		msg[MSG_LEN];

    if (pulse->id != NO_ID) {
	ind = GetInd[pulse->id];
	pl = Players[ind];
    } else {
	ind = -1;
	pl = NULL;
    }

    vicpl = Players[victim->ind];
    vicpl->forceVisible++;
    if (BIT(vicpl->have, HAS_MIRROR)
	&& (rfrac() * (2 * vicpl->item[ITEM_MIRROR])) >= 1) {
	pulse->pos.x = x - tcos(pulse->dir) * 0.5
			    * PULSE_SAMPLE_DISTANCE;
	pulse->pos.y = y - tsin(pulse->dir) * 0.5
			    * PULSE_SAMPLE_DISTANCE;
	pulse->dir = (int)Wrap_findDir(vicpl->pos.x - pulse->pos.x,
				  vicpl->pos.y - pulse->pos.y)
		     * 2 - RES / 2 - pulse->dir;
	pulse->dir = MOD2(pulse->dir, RES);
	pulse->life += vicpl->item[ITEM_MIRROR];
	pulse->len = PULSE_LENGTH;
	pulse->refl = true;
	*refl = true;
	return;
    }

    sound_play_sensors(vicpl->pos.x, vicpl->pos.y,
		       PLAYER_EAT_LASER_SOUND);
    if (BIT(vicpl->used, (HAS_SHIELD|HAS_EMERGENCY_SHIELD))
	== (HAS_SHIELD|HAS_EMERGENCY_SHIELD))
	return;
    if (!BIT(obj->type, KILLING_SHOTS))
	return;
    if (BIT(pulse->mods.laser, STUN)
	|| (laserIsStunGun == true
	    && allowLaserModifiers == false)) {
	if (BIT(vicpl->used, HAS_SHIELD|HAS_LASER|HAS_SHOT)
	    || BIT(vicpl->status, THRUSTING)) {
	    if (pl) {
		sprintf(msg,
		    "%s got paralysed by %s's stun laser.",
		    vicpl->name, pl->name);
		if (vicpl->id == pl->id)
		    strcat(msg, " How strange!");
	    } else {
		sprintf(msg,
		    "%s got paralysed by a stun laser.",
		    vicpl->name);
	    }
	    Set_message(msg);
	    CLR_BIT(vicpl->used,
		    HAS_SHIELD|HAS_LASER|OBJ_SHOT);
	    CLR_BIT(vicpl->status, THRUSTING);
	    vicpl->stunned += 5;
	}
    } else if (BIT(pulse->mods.laser, BLIND)) {
	vicpl->damaged += (FPS + 6);
	vicpl->forceVisible += (FPS + 6);
	if (pl)
	    Record_shove(vicpl, pl, frame_loops + FPS + 6);
    } else {
	Add_fuel(&(vicpl->fuel), (long)ED_LASER_HIT);
	if (!BIT(vicpl->used, HAS_SHIELD)
	    && !BIT(vicpl->have, HAS_ARMOR)) {
	    SET_BIT(vicpl->status, KILLED);
	    if (pl) {
		sprintf(msg,
		    "%s got roasted alive by %s's laser.",
		    vicpl->name, pl->name);
		if (vicpl->id == pl->id) {
		    sc = Rate(0, pl->score)
			   * laserKillScoreMult
			   * selfKillScoreMult;
		    SCORE(victim->ind, -sc,
			  OBJ_X_IN_BLOCKS(vicpl),
			  OBJ_Y_IN_BLOCKS(vicpl),
			  vicpl->name);
		    strcat(msg, " How strange!");
		} else {
		    sc = Rate(pl->score,
					 vicpl->score)
			 * laserKillScoreMult;
		    Score_players(ind, sc, vicpl->name,
				  victim->ind, -sc,
				  pl->name);
		}
	    } else {
		sc = Rate(CANNON_SCORE, vicpl->score) / 4;
		SCORE(victim->ind, -sc,
		      OBJ_X_IN_BLOCKS(vicpl),
		      OBJ_Y_IN_BLOCKS(vicpl),
		      "Cannon");
		sprintf(msg,
		    "%s got roasted alive by cannonfire.",
		    vicpl->name);
	    }
	    sound_play_sensors(vicpl->pos.x, vicpl->pos.y,
			       PLAYER_ROASTED_SOUND);
	    Set_message(msg);
	    if (pl && pl->id != vicpl->id) {
		pl->kills++;
		Robot_war(victim->ind, ind);
	    }
	}
	if (!BIT(vicpl->used, HAS_SHIELD)
	    && BIT(vicpl->have, HAS_ARMOR)) {
	    Player_hit_armor(victim->ind);
	}
    }
}


/*
 * Check a given pulse position against a list of players.
 * Do what needs to be done when on any pulse hits player event.
 * Return the number of hits.
 * When the pulse was reflected then "refl" will have been set to true.
 */
static int Laser_pulse_check_player_hits(
		    pulse_t *pulse,
		    object *obj,
		    DFLOAT x,
		    DFLOAT y,
		    vicbuf_t *vicbuf,
		    bool *refl)
{
    int			j;
    int			hits = 0;
    /* int			ind; */
    DFLOAT		dist;
    /* player		*pl; */
    victim_t		*victim;

    /*
    if (pulse->id != NO_ID) {
	ind = GetInd[pulse->id];
	pl = Players[ind];
    } else {
	ind = -1;
	pl = NULL;
    }
    */

    for (j = vicbuf->num_vic - 1; j >= 0; --j) {
	victim = &(vicbuf->vic_ptr[j]);
	dist = Wrap_length(x - victim->pos.x,
			   y - victim->pos.y);
	if (dist <= SHIP_SZ) {
	    Laser_pulse_hits_player(
			pulse,
			obj,
			x, y,
			victim,
			refl);
	    hits++;
	    /* stop at the first hit. */
	    break;
	}
	else if (dist >= victim->prev_dist) {
	    /* remove victim by copying the last victim over it */
	    vicbuf->vic_ptr[j] = vicbuf->vic_ptr[--vicbuf->num_vic];
	} else {
	    /* remember shortest distance from pulse to player */
	    vicbuf->vic_ptr[j].prev_dist = dist;
	}
    }

    return hits;
}


static list_t Laser_pulse_get_object_list(
	list_t input_obj_list,
	pulse_t *pulse,
	DFLOAT midx,
	DFLOAT midy)
{
    list_t		output_obj_list;
    list_t		ast_list;
    DFLOAT		dx, dy;
    int			range;
    list_iter_t		iter;
    object		*ast;

    if (input_obj_list != NULL) {
	List_clear(input_obj_list);
    }
    output_obj_list = input_obj_list;

    ast_list = Asteroid_get_list();
    if (ast_list != NULL) {
	if (output_obj_list == NULL) {
	    output_obj_list = List_new();
	}
	if (output_obj_list != NULL) {
	    /* fill list with interesting objects
	     * which are close to our pulse. */
	    for (iter = List_begin(ast_list);
		 iter != List_end(ast_list);
		 LI_FORWARD(iter))
	    {
		ast = (object *) LI_DATA(iter);
		dx = midx - ast->pos.x;
		dy = midy - ast->pos.y;
		dx = WRAP_DX(dx);
		dy = WRAP_DY(dy);
		range = ast->pl_radius + pulse->len / 2;
		if (sqr(dx) + sqr(dy) < sqr(range)) {
		    List_push_back(output_obj_list, ast);
		}
	    }
	}
    }

    return output_obj_list;
}


/*
 * For all existing laser pulse check
 * if they collide with ships or asteroids.
 */
void Laser_pulse_collision(void)
{
    int				ind, i;
    int				p;
    int				max, hits;
    bool			refl;
    vicbuf_t			vicbuf;
    DFLOAT			x, y, x1, x2, y1, y2;
    DFLOAT			dx, dy;
    DFLOAT			midx, midy;
    /* player			*pl; */
    pulse_t			*pulse;
    object			*obj = NULL, *ast = NULL;
    list_t			obj_list = NULL;
    list_iter_t			iter;

    /*
     * Allocate one object with which we will
     * do pulse wall bounce checking.
     */
    if ((obj = Object_allocate()) == NULL) {
	/* overload.  we can't do bounce checking. */
	Laser_pulse_destroy_all();
	return;
    }

    /* init vicbuf */
    vicbuf.num_vic = 0;
    vicbuf.max_vic = 0;
    vicbuf.vic_ptr = NULL;

    for (p = NumPulses - 1; p >= 0; --p) {
	pulse = Pulses[p];

	/* check for end of pulse life */
	if (--pulse->life < 0 || pulse->len < PULSE_LENGTH) {
	    Laser_pulse_destroy_one(p);
	    continue;
	}

	if (pulse->id != NO_ID) {
	    ind = GetInd[pulse->id];
	    /* pl = Players[ind]; */
	} else {
	    ind = -1;
	    /* pl = NULL; */
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
		pulse->len = (int)(pulse->len * (0 - x1) / (x2 - x1));
		x2 = x1 + tcos(pulse->dir) * pulse->len;
	    }
	    if (x2 >= World.width) {
		pulse->len = (int)(pulse->len * (World.width - 1 - x1)
		    / (x2 - x1));
		x2 = x1 + tcos(pulse->dir) * pulse->len;
	    }
	    y2 = y1 + tsin(pulse->dir) * pulse->len;
	    if (y2 < 0) {
		pulse->len = (int)(pulse->len * (0 - y1) / (y2 - y1));
		x2 = x1 + tcos(pulse->dir) * pulse->len;
		y2 = y1 + tsin(pulse->dir) * pulse->len;
	    }
	    if (y2 > World.height) {
		pulse->len = (int)(pulse->len * (World.height - 1 - y1)
		    / (y2 - y1));
		x2 = x1 + tcos(pulse->dir) * pulse->len;
		y2 = y1 + tsin(pulse->dir) * pulse->len;
	    }
	    if (pulse->len <= 0) {
		pulse->len = 0;
		continue;
	    }
	}

	/* calculate delta x and y for pulse start and end position. */
	dx = x2 - x1;
	dy = y2 - y1;
	dx = WRAP_DX(dx);
	dy = WRAP_DY(dy);

	/* max is the highest absolute delta length of either x or y. */
	max = (int)MAX(ABS(dx), ABS(dy));
	if (max == 0) {
	    continue;
	}

	/* calculate the midpoint of the new laser pulse position. */
	midx = x1 + (dx * 0.5);
	midy = y1 + (dy * 0.5);
	midx = WRAP_XPIXEL(midx);
	midy = WRAP_YPIXEL(midy);

	if (round_delay == 0) {
	    /* assemble a shortlist of players which might get hit. */
	    Laser_pulse_find_victims(&vicbuf, pulse, midx, midy);
	}

	obj_list = Laser_pulse_get_object_list(
				obj_list,
				pulse,
				midx, midy);

	obj->type = OBJ_PULSE;
	obj->life = 1;
	obj->id = pulse->id;
	obj->team = pulse->team;
	obj->count = 0;
	obj->status = 0;
	if (pulse->id == NO_ID) {
	    obj->status = FROMCANNON;
	}
	Object_position_init_pixels(obj, x1, y1);

	refl = false;

	for (i = hits = 0; i <= max; i += PULSE_SAMPLE_DISTANCE) {
	    x = x1 + (i * dx) / max;
	    y = y1 + (i * dy) / max;
	    obj->vel.x = (x - CLICK_TO_FLOAT(obj->pos.cx));
	    obj->vel.y = (y - CLICK_TO_FLOAT(obj->pos.cy));
	    /* changed from = x - obj->pos.x to make lasers disappear
	       less frequently when wrapping. There's still a small
	       chance of it happening though. */
	    Move_object(obj);
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

	    /* check for collision with objects. */
	    if (obj_list != NULL) {
		for (iter = List_begin(obj_list);
		     iter != List_end(obj_list);
		     LI_FORWARD(iter))
		{
		    DFLOAT adx, ady;
		    ast = (object *)LI_DATA(iter);
		    adx = x - ast->pos.x;
		    ady = y - ast->pos.y;
		    adx = WRAP_DX(adx);
		    ady = WRAP_DY(ady);
		    if (sqr(adx) + sqr(ady) <= sqr(ast->pl_radius)) {
			obj->life = 0;
			ast->life += ASTEROID_FUEL_HIT(ED_LASER_HIT,
						       WIRE_PTR(ast)->size);
			if (ast->life < 0)
			    ast->life = 0;
			if (ast->life == 0
			    && ind != -1
			    && asteroidPoints > 0
			    && Players[ind]->score <= asteroidMaxScore) {
			    SCORE(ind, asteroidPoints,
				  OBJ_X_IN_BLOCKS(ast), OBJ_Y_IN_BLOCKS(ast),
				  "");
			}
			break;
		    }
		}
	    }

	    if (obj->life == 0) {
		/* pulse hit asteroid */
		continue;
	    }

	    if (round_delay > 0) {
		/* at round delay no hits are possible */
		continue;
	    }

	    hits = Laser_pulse_check_player_hits(
			    pulse, obj,
			    x, y,
			    &vicbuf,
			    &refl);

	    if (hits > 0) {
		break;
	    }
	}

	if (i < max && refl == false) {
	    pulse->len = (pulse->len * i) / max;
	}
    }
    if (vicbuf.max_vic > 0 && vicbuf.vic_ptr != NULL) {
	free(vicbuf.vic_ptr);
    }

    obj->type = OBJ_DEBRIS;
    obj->life = 0;
    Cell_add_object(obj);

    if (obj_list != NULL) {
	List_delete(obj_list);
    }
}


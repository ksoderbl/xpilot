/* $Id: robotdef.c,v 3.7 1996/10/22 20:30:59 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gÿsbers         <bert@xpilot.org>
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
/* Robot code originally submitted by Maurice Abraham. */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "score.h"
#include "bit.h"
#include "saudio.h"
#include "netserver.h"
#include "pack.h"
#include "robot.h"
#include "error.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: robotdef.c,v 3.7 1996/10/22 20:30:59 bert Exp $";
#endif

/*
 * Flags for the default robots being in different modes (or moods).
 */
#define RM_ROBOT_IDLE         	(1 << 2)
#define RM_EVADE_LEFT         	(1 << 3)
#define RM_EVADE_RIGHT          (1 << 4)
#define RM_ROBOT_CLIMB          (1 << 5)
#define RM_HARVEST            	(1 << 6)
#define RM_ATTACK             	(1 << 7)
#define RM_TAKE_OFF           	(1 << 8)
#define RM_CANNON_KILL		(1 << 9)
#define RM_REFUEL		(1 << 10)
#define RM_NAVIGATE		(1 << 11)

/*
 * Map objects a robot can fly through without damage.
 */
extern unsigned SPACE_BLOCKS;
#define EMPTY_SPACE(s)	BIT(1 << (s), SPACE_BLOCKS)

/*
 * Bitmask of object types the robot puts up shield for.
 */
extern long KILLING_SHOTS;

/*
 * Prototypes for methods of the default robot type.
 */
static void Robot_default_create(int ind, char *str);
static void Robot_default_go_home(int ind);
static void Robot_default_play(int ind);
static void Robot_default_set_war(int ind, int victim_id);
static int Robot_default_war_on_player(int ind);
static void Robot_default_message(int ind, char *str);
static void Robot_default_destroy(int ind);

static robot_type_t robot_default_type = {
    "default",
    Robot_default_create,
    Robot_default_go_home,
    Robot_default_play,
    Robot_default_set_war,
    Robot_default_war_on_player,
    Robot_default_message,
    Robot_default_destroy
};

/*
 * The only thing we export from this file.
 * A function to initialize the robot type structure
 * with our name and the pointers to our action routines.
 *
 * Return 0 if all is OK, anything else will ignore this
 * robot type forever.
 */
int Robot_default_setup(robot_type_t *type_ptr)
{
    /* Not much to do for the default robot except init the type structure. */

    *type_ptr = robot_default_type;

    return 0;
}

/*
 * Private functions.
 */
static bool Check_robot_navigate(int ind, bool * num_evade);
static bool Check_robot_evade(int ind, int mine_i, int ship_i);
static bool Check_robot_target(int ind, int item_x, int item_y, int new_mode,
			       int attack_level);


/*
 * Function to cast from player structure to robot data structure.
 * This isolates casts (aka. type violations) to a few places.
 */
static robot_default_data_t *Robot_default_get_data(player *pl)
{
    return (robot_default_data_t *)pl->robot_data_ptr->private_data;
}

/*
 * A default robot is created.
 */
static void Robot_default_create(int ind, char *str)
{
    player			*pl = Players[ind];
    robot_default_data_t	*my_data;

    if (!(my_data = (robot_default_data_t *)malloc(sizeof(*my_data)))) {
	error("no mem for default robot");
	End_game();
    }

    my_data->robot_mode      = RM_TAKE_OFF;
    my_data->robot_count     = 0;
    my_data->robot_lock      = LOCK_NONE;
    my_data->robot_lock_id   = 0;

    if (str != NULL
	&& *str != '\0'
	&& sscanf(str, " %d %d", &my_data->attack, &my_data->attack) != 2) {
	if (str && *str) {
	    printf("invalid parameters for default robot: \"%s\"\n", str);
	}
	my_data->attack = 5 + (int)(rfrac() * 90 + 0.5);
	my_data->defense = 100 - my_data->attack;
    }
    /*
     * some parameters which may be changed to be dependent upon
     * the `attack' and `defense' settings of this robot.
     */
    my_data->robot_normal_speed = 5.0;	/* was 3 */
    my_data->robot_attack_speed = 15.0;	/* was 12 */
    my_data->robot_max_speed = 25.0;	/* was 20 */

    pl->robot_data_ptr->private_data = (void *)my_data;
}

/*
 * A default robot is placed on its homebase.
 */
static void Robot_default_go_home(int ind)
{
    player			*pl = Players[ind];
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    my_data->robot_mode      = RM_TAKE_OFF;
}

/*
 * A default robot is declaring war (or resetting war).
 */
static void Robot_default_set_war(int ind, int victim_id)
{
    player			*pl = Players[ind];
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    if (victim_id == -1) {
	CLR_BIT(my_data->robot_lock, LOCK_PLAYER);
    } else {
	my_data->robot_lock_id = victim_id;
	SET_BIT(my_data->robot_lock, LOCK_PLAYER);
    }
}

/*
 * Return the id of the player a default robot has war against (or -1).
 */
static int Robot_default_war_on_player(int ind)
{
    player			*pl = Players[ind];
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    if (BIT(my_data->robot_lock, LOCK_PLAYER)) {
	return my_data->robot_lock_id;
    } else {
	return -1;
    }
}

/*
 * A default robot receives a message.
 */
static void Robot_default_message(int ind, char *message)
{
#if 0
    player			*pl = Players[ind];
    robot_default_data_t	*my_data = Robot_default_get_data(pl);
    int				len;
    char			*ptr;
    char			sender_name[MAX_NAME_LEN];
    char			msg[MSG_LEN];

    /*
     * Extract the message body and the sender name from the message string.
     */

    ptr = strrchr(message, ']');/* get end of message */
    if (!ptr) {
	return;		/* Make sure to ignore server messages, oops! */
    }
    while (*--ptr != '[');	/* skip back over receiver name */
    while (*--ptr != '[');	/* skip back over sender name */
    strncpy(sender_name, ptr + 1, sizeof sender_name);	/* copy sender */
    len = ptr - message;	/* find read message length */
    if (message[len] == ' ') {	/* ignore the readability space */
	len--;
    }
    strncpy(msg, message, len);	/* copy real message */
    msg[len] = '\0';		/* make it a string */
    if ((ptr = strchr(sender_name, ']')) != NULL) {
	*ptr = '\0';	/* remove the ']' separator */
    }
    printf("%s got message \"%s\" from \"%s\"\n", pl->name, msg, sender_name);
#endif
}

/*
 * A default robot is destroyed.
 */
static void Robot_default_destroy(int ind)
{
    player			*pl = Players[ind];

    free(pl->robot_data_ptr->private_data);
    pl->robot_data_ptr->private_data = NULL;
}


static bool Check_robot_navigate(int ind, bool * num_evade)
{
    int         i, j, k;
    player     *pl;
    int         area_val[10][10];
    int         calc_val[10][10];
    int         locn_block;
    long        dx, dy;
    int         di, dj;
    bool        found_wall;
    bool        found_grav;
    bool	target_passable;
    bool        near_wall;
    int         best_val;
    int         best_i, best_j;
    float       best_vx, best_vy;
    int         best_dir;
    int         delta_dir;

    pl = Players[ind];

    if (pl->velocity > 2.0 || ABS(pl->vel.x) > 1.5)
	return false;

    for (i = 0; i < 10; i++) {
	for (j = 0; j < 10; j++) {
	    area_val[i][j] = 0;
	}
    }

    found_wall = false;
    found_grav = false;

    for (i = 0; i < 10; i += 2) {
	for (j = 0; j < 10; j += 2) {

	    dx = (long)(pl->pos.x / BLOCK_SZ) + (i / 2) - 2;
	    dy = (long)(pl->pos.y / BLOCK_SZ) + (j / 2) - 2;

	    if (BIT(World.rules->mode, WRAP_PLAY)) {
		if (dx < 0) dx += World.x;
		else if (dx >= World.x) dx -= World.x;
		if (dy < 0) dy += World.y;
		else if (dy >= World.y) dy -= World.y;
	    }
	    if (dx < 0 || dx >= World.x || dy < 0 || dy >= World.y)
		locn_block = FILLED;
	    else
		locn_block = World.block[dx][dy];

	    switch (locn_block) {

	    case SPACE:
	    case BASE:
	    case CHECK:
	    case WORMHOLE:
	    case ITEM_CONCENTRATOR:
		area_val[i][j] = 1;
		area_val[i + 1][j] = 1;
		area_val[i + 1][j + 1] = 1;
		area_val[i][j + 1] = 1;
		break;

	    case REC_LU:
		area_val[i + 1][j] = 1;
		found_wall = true;
		break;

	    case REC_LD:
		area_val[i + 1][j + 1] = 1;
		found_wall = true;
		break;

	    case REC_RU:
		area_val[i][j] = 1;
		found_wall = true;
		break;

	    case REC_RD:
		area_val[i][j + 1] = 1;
		found_wall = true;
		break;

	    case POS_GRAV:
	    case NEG_GRAV:
	    case CWISE_GRAV:
	    case ACWISE_GRAV:
	    case UP_GRAV:
	    case DOWN_GRAV:
	    case RIGHT_GRAV:
	    case LEFT_GRAV:
		found_grav = true;
		break;

	    case TARGET:
		if (!targetTeamCollision) {
		    target_passable = false;
		    for (k = 0; ; k++) {
			if (World.targets[k].pos.x == dx
			    && World.targets[k].pos.y == dy) {
			    if (pl->team == World.targets[k].team) {
				target_passable = true;
			    }
			    break;
			}
		    }
		    if (target_passable) {
			area_val[i][j] = 1;
			area_val[i + 1][j] = 1;
			area_val[i + 1][j + 1] = 1;
			area_val[i][j + 1] = 1;
			break;
		    }
		}
		/* FALLTHROUGH */

	    default:
		found_wall = true;
		break;
	    }
	}
    }

    if (found_grav || !found_wall)
	return false;

    /* iterate twice for weighting, central 6x6 square should be accurate */

    for (k = 0; k < 2; k++) {
	for (i = 0; i < 10; i++) {
	    for (j = 0; j < 10; j++) {

		calc_val[i][j] = 0;
		if (area_val[i][j] == 0)
		    continue;

		if (i <= 0 || i >= 9 || j <= 0 || j >= 9)
		    continue;

		calc_val[i][j] += 2 * area_val[i - 1][j];
		calc_val[i][j] += 2 * area_val[i][j + 1];
		calc_val[i][j] += 2 * area_val[i + 1][j];
		calc_val[i][j] += 2 * area_val[i][j - 1];

		calc_val[i][j] += area_val[i - 1][j - 1];
		calc_val[i][j] += area_val[i - 1][j + 1];
		calc_val[i][j] += area_val[i + 1][j - 1];
		calc_val[i][j] += area_val[i + 1][j + 1];
	    }
	}

	for (i = 0; i < 10; i++) {
	    for (j = 0; j < 10; j++) {
		area_val[i][j] = calc_val[i][j];
	    }
	}
    }

    /* now focus in to local 3x3 square */

    dx = (int)pl->pos.x;
    dy = (int)pl->pos.y;

    dx = dx - (dx / BLOCK_SZ * BLOCK_SZ);
    dy = dy - (dy / BLOCK_SZ * BLOCK_SZ);

    di = 3;
    dj = 3;

    if (dx > BLOCK_SZ / 2) {
	di++;
	dx -= BLOCK_SZ / 2;
    }
    if (dy > BLOCK_SZ / 2) {
	dj++;
	dy -= BLOCK_SZ / 2;
    }
    for (i = 0; i < 3; i++) {
	for (j = 0; j < 3; j++) {
	    area_val[i][j] = area_val[di + i][dj + j];
	}
    }

    *num_evade = true;

    if (ABS(pl->vel.x) < 0.5) {

	best_i = 1;
	best_j = (pl->vel.y > 0 ? 2 : 0);

    } else if (ABS(pl->vel.y) < 0.5) {

	best_i = (pl->vel.x > 0 ? 2 : 0);
	best_j = 1;

    } else {

	best_i = (pl->vel.x > 0 ? 2 : 0);
	best_j = (pl->vel.y > 0 ? 2 : 0);
    }

    best_val = area_val[best_i][best_j];
    near_wall = false;

    for (j = 2; j >= 0; j--) {
	for (i = 0; i <= 2; i++) {

	    if (i == 1 && j == 1)
		continue;

	    if (area_val[i][j] == 0) {
		near_wall = true;
		if (i == 1 && (j == 0 || j == 2)) {
		    best_i = 1;
		    best_j = (2 - j);
		    best_val = 99999;
		}
		continue;
	    }
	    if (area_val[i][j] > best_val) {
		best_i = i;
		best_j = j;
		best_val = area_val[i][j];
	    }
	}
    }

    if (area_val[1][1] >= best_val)
	return false;

    if (!near_wall) {
	if (BIT(pl->used, OBJ_REFUEL)) {
	    /* refueling, so hang around */
	    best_i = 1;
	    best_j = 1;
	    best_val = area_val[1][1];
	} else {
	    return false;
	}
    }
    if (best_j == 1) {
	if (dy < BLOCK_SZ / 6)
	    best_j = 2;
	if (dy > BLOCK_SZ / 3)
	    best_j = 0;
    }
    pl->turnspeed = MAX_PLAYER_TURNSPEED;
    pl->power = pl->mass / 2;
    LIMIT(pl->power, MIN_PLAYER_POWER, MAX_PLAYER_POWER);

    best_vx = (best_i - 1) * 0.75;
    best_vy = (best_j - 1) * 1.25;

    if (pl->vel.x > best_vx + 0.75)
	best_dir = 3 * RES / 8;
    else if (pl->vel.x < best_vx - 0.75)
	best_dir = RES / 8;
    else if (pl->vel.x > best_vx + 0.25)
	best_dir = 5 * RES / 16;
    else if (pl->vel.x < best_vx - 0.25)
	best_dir = 3 * RES / 16;
    else
	best_dir = RES / 4;

    delta_dir = best_dir - pl->dir;
    delta_dir = MOD2(delta_dir, RES);

    if (delta_dir > RES / 8 && delta_dir < 7 * RES / 8) {
	pl->turnacc = (delta_dir < RES / 2 ?
		       pl->turnspeed : (-pl->turnspeed));
    } else if (delta_dir > RES / 64 && delta_dir < 63 * RES / 64) {
	pl->turnspeed = MIN_PLAYER_TURNSPEED;
	pl->turnacc = (delta_dir < RES / 2 ?
		       pl->turnspeed : (-pl->turnspeed));
    } else {
	pl->turnacc = 0;
    }

    if (pl->vel.y > best_vy + 0.25) {
	CLR_BIT(pl->status, THRUSTING);
    } else if (pl->vel.y < best_vy - 0.25) {
	SET_BIT(pl->status, THRUSTING);
    }
    return true;
}

static int Find_target_team (int x, int y)
{
    target_t	*targ = World.targets;
    int		t;

    for (t = 0; t < World.NumTargets; t++, targ++)
	if (targ->pos.x == x && targ->pos.y == y)
	    break;
    return targ->team;
}

#define TARGET_PASSABLE_HACK
#ifdef TARGET_PASSABLE_HACK
/*
 * Hack: If we've got targets which are crossable by a team member
 * we assume that all targets in the vicinity have the same team
 * assigned to them to save time.  This is usually true, make sure that
 * targets are always more than twelve blocks apart.  If this isn't true
 * the robot may well evade or hit targets when not supposed to, but
 * its a small price to pay.
 */
static int target_team;

/*
 * Call once per scan of locality
 */
#define TARGET_INIT()	(target_team = TEAM_NOT_SET)

#define TARGET_PASSABLE(dx,dy) \
    (!targetTeamCollision && \
	pl->team == (target_team == TEAM_NOT_SET \
	    ? target_team = Find_target_team(dx,dy) \
	    : target_team))

/*
 * Use REALLY_EMPTY_SPACE() over EMPTY_SPACE() for above HACK
 */
#define REALLY_EMPTY_SPACE(type,dx,dy) \
    (EMPTY_SPACE(type) || (type == TARGET && TARGET_PASSABLE(dx,dy)))

#else
#define TARGET_INIT()
#define REALLY_EMPTY_SPACE(type,dx,dy) EMPTY_SPACE(type)
#endif

static bool Check_robot_evade(int ind, int mine_i, int ship_i)
{
    int				i;
    player			*pl = Players[ind];
    object			*shot;
    player			*ship;
    long			stop_dist;
    bool			evade;
    bool			left_ok, right_ok;
    int				safe_width;
    int				travel_dir;
    int				delta_dir;
    int				aux_dir;
    int				px[3], py[3];
    long			dist;
    int				locn_block;
    vector			*gravity;
    int				gravity_dir;
    long			dx, dy;
    float			velocity;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    safe_width = 3 * SHIP_SZ / 2;
    /* Prevent overflow. */
    velocity = (pl->velocity <= SPEED_LIMIT) ? pl->velocity : SPEED_LIMIT;
    stop_dist =
	(RES * velocity) / (MAX_PLAYER_TURNSPEED * pl->turnresistance)
	+ (velocity * velocity * pl->mass) / (2 * MAX_PLAYER_POWER)
	+ safe_width;
    /*
     * Limit the look ahead.  For very high speeds the current code
     * is ineffective and much too inefficient.
     */
    if (stop_dist > 10 * BLOCK_SZ) {
	stop_dist = 10 * BLOCK_SZ;
    }
    evade = false;

    if (pl->velocity <= 0.2) {
	vector	*grav = &World.gravity
	    [(int)pl->pos.x / BLOCK_SZ][(int)pl->pos.y / BLOCK_SZ];
	travel_dir = findDir(grav->x, grav->y);
    } else {
	travel_dir = findDir(pl->vel.x, pl->vel.y);
    }

    aux_dir = MOD2(travel_dir + RES / 4, RES);
    px[0] = pl->pos.x;		/* ship center x */
    py[0] = pl->pos.y;		/* ship center y */
    px[1] = px[0] + safe_width * tcos(aux_dir);	/* ship left side x */
    py[1] = py[0] + safe_width * tsin(aux_dir);	/* ship left side y */
    px[2] = 2 * px[0] - px[1];	/* ship right side x */
    py[2] = 2 * py[0] - py[1];	/* ship right side y */

    left_ok = true;
    right_ok = true;

    TARGET_INIT();

    for (dist = 0; dist < stop_dist + BLOCK_SZ / 2; dist += BLOCK_SZ / 2) {
	for (i = 0; i < 3; i++) {
	    dx = (px[i] + dist * tcos(travel_dir)) / BLOCK_SZ;
	    dy = (py[i] + dist * tsin(travel_dir)) / BLOCK_SZ;

	    if (BIT(World.rules->mode, WRAP_PLAY)) {
		if (dx < 0) dx += World.x;
		else if (dx >= World.x) dx -= World.x;
		if (dy < 0) dy += World.y;
		else if (dy >= World.y) dy -= World.y;
	    }
	    if (dx < 0 || dx >= World.x || dy < 0 || dy >= World.y) {
		evade = true;
		if (i == 1)
		    left_ok = false;
		if (i == 2)
		    right_ok = false;
		continue;
	    }
	    locn_block = World.block[dx][dy];
	    if (!REALLY_EMPTY_SPACE(locn_block,dx,dy)) {
		evade = true;
		if (i == 1)
		    left_ok = false;
		if (i == 2)
		    right_ok = false;
		continue;
	    }
	    /* Watch out for strong gravity */
	    gravity = &World.gravity[dx][dy];
	    if (sqr(gravity->x) + sqr(gravity->y) >= 1.0) {
		gravity_dir = findDir(gravity->x - pl->pos.x,
				      gravity->y - pl->pos.y);
		if (MOD2(gravity_dir - travel_dir, RES) <= RES / 4 ||
		    MOD2(gravity_dir - travel_dir, RES) >= 3 * RES / 4) {
		    evade = true;
		    if (i == 1)
			left_ok = false;
		    if (i == 2)
			right_ok = false;
		    continue;
		}
	    }
	}
    }

    if (mine_i >= 0) {
	shot = Obj[mine_i];
	aux_dir = Wrap_findDir(shot->pos.x + shot->vel.x - pl->pos.x,
			       shot->pos.y + shot->vel.y - pl->pos.y);
	delta_dir = MOD2(aux_dir - travel_dir, RES);
	if (delta_dir < RES / 4) {
	    left_ok = false;
	    evade = true;
	}
	if (delta_dir > RES * 3 / 4) {
	    right_ok = false;
	    evade = true;
	}
    }
    if (ship_i >= 0) {
	ship = Players[ship_i];
	aux_dir = Wrap_findDir(ship->pos.x - pl->pos.x + ship->vel.x * 2,
			       ship->pos.y - pl->pos.y + ship->vel.y * 2);
	delta_dir = MOD2(aux_dir - travel_dir, RES);
	if (delta_dir < RES / 4) {
	    left_ok = false;
	    evade = true;
	}
	if (delta_dir > RES * 3 / 4) {
	    right_ok = false;
	    evade = true;
	}
    }
    if (pl->velocity > my_data->robot_max_speed)
	evade = true;

    if (!evade)
	return false;

    delta_dir = 0;
    while (!left_ok && !right_ok && delta_dir < 7 * RES / 8) {
	delta_dir += RES / 16;

	left_ok = true;
	aux_dir = MOD2(travel_dir + delta_dir, RES);
	for (dist = 0; dist < stop_dist + BLOCK_SZ / 2; dist += BLOCK_SZ / 2) {
	    dx = (px[0] + dist * tcos(aux_dir)) / BLOCK_SZ;
	    dy = (py[0] + dist * tsin(aux_dir)) / BLOCK_SZ;

	    if (BIT(World.rules->mode, WRAP_PLAY)) {
		if (dx < 0) dx += World.x;
		else if (dx >= World.x) dx -= World.x;
		if (dy < 0) dy += World.y;
		else if (dy >= World.y) dy -= World.y;
	    }
	    if (dx < 0 || dx >= World.x || dy < 0 || dy >= World.y) {
		left_ok = false;
		continue;
	    }
	    locn_block = World.block[dx][dy];
	    if (!REALLY_EMPTY_SPACE(locn_block,dx,dy)) {
		left_ok = false;
		continue;
	    }
	    /* watch out for strong gravity */
	    gravity = &World.gravity[dx][dy];
	    if (sqr(gravity->x) + sqr(gravity->y) >= 1.0) {
		gravity_dir = findDir(gravity->x - pl->pos.x,
				      gravity->y - pl->pos.y);
		if (MOD2(gravity_dir - travel_dir, RES) <= RES / 4 ||
		    MOD2(gravity_dir - travel_dir, RES) >= 3 * RES / 4) {

		    left_ok = false;
		    continue;
		}
	    }
	}

	right_ok = true;
	aux_dir = MOD2(travel_dir - delta_dir, RES);
	for (dist = 0; dist < stop_dist + BLOCK_SZ / 2; dist += BLOCK_SZ / 2) {
	    dx = (px[0] + dist * tcos(aux_dir)) / BLOCK_SZ;
	    dy = (py[0] + dist * tsin(aux_dir)) / BLOCK_SZ;

	    if (BIT(World.rules->mode, WRAP_PLAY)) {
		if (dx < 0) dx += World.x;
		else if (dx >= World.x) dx -= World.x;
		if (dy < 0) dy += World.y;
		else if (dy >= World.y) dy -= World.y;
	    }
	    if (dx < 0 || dx >= World.x || dy < 0 || dy >= World.y) {
		right_ok = false;
		continue;
	    }
	    locn_block = World.block[dx][dy];
	    if (!REALLY_EMPTY_SPACE(locn_block,dx,dy)) {
		right_ok = false;
		continue;
	    }
	    /* watch out for strong gravity */
	    gravity = &World.gravity[dx][dy];
	    if (sqr(gravity->x) + sqr(gravity->y) >= 1.0) {
		gravity_dir = findDir(gravity->x - pl->pos.x,
				      gravity->y - pl->pos.y);
		if (MOD2(gravity_dir - travel_dir, RES) <= RES / 4 ||
		    MOD2(gravity_dir - travel_dir, RES) >= 3 * RES / 4) {

		    right_ok = false;
		    continue;
		}
	    }
	}
    }

    pl->turnspeed = MAX_PLAYER_TURNSPEED;
    pl->power = MAX_PLAYER_POWER;

    delta_dir = MOD2(pl->dir - travel_dir, RES);

    if (my_data->robot_mode != RM_EVADE_LEFT
	&& my_data->robot_mode != RM_EVADE_RIGHT) {
	if (left_ok && !right_ok)
	    my_data->robot_mode = RM_EVADE_LEFT;
	else if (right_ok && !left_ok)
	    my_data->robot_mode = RM_EVADE_RIGHT;
	else
	    my_data->robot_mode = (delta_dir < RES / 2 ?
			      RM_EVADE_LEFT : RM_EVADE_RIGHT);
    }
    if (delta_dir < 3 * RES / 8 || delta_dir > 5 * RES / 8) {
	pl->turnacc = (my_data->robot_mode == RM_EVADE_LEFT ?
		       pl->turnspeed : (-pl->turnspeed));
	CLR_BIT(pl->status, THRUSTING);
    } else {
	pl->turnacc = 0;
	SET_BIT(pl->status, THRUSTING);
	my_data->robot_mode = (delta_dir < RES/2 ? RM_EVADE_LEFT : RM_EVADE_RIGHT);
    }

    return true;
}

static void Robot_check_new_modifiers(player *pl, modifiers mods)
{
    if (!BIT(World.rules->mode, ALLOW_NUKES))
	mods.nuclear = 0;
    if (!BIT(World.rules->mode, ALLOW_CLUSTERS))
	CLR_BIT(mods.warhead, CLUSTER);
    if (!BIT(World.rules->mode, ALLOW_MODIFIERS)) {
	mods.velocity =
	mods.mini =
	mods.spread =
	mods.power = 0;
	CLR_BIT(mods.warhead, IMPLOSION);
    }
    if (!BIT(World.rules->mode, ALLOW_LASER_MODIFIERS))
	mods.laser = 0;
    pl->mods = mods;
}

static void Choose_weapon_modifier(player *pl, int weapon_type)
{
    int				stock, min;
    modifiers			mods;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    CLEAR_MODS(mods);

    switch (weapon_type) {
    case OBJ_TRACTOR_BEAM:
	Robot_check_new_modifiers(pl, mods);
	return;

    case OBJ_LASER:
	/*
	 * Robots choose non-damage laser settings occasionally.
	 */
	if ((my_data->robot_count % 4) == 0)
	    mods.laser = rand() % (MODS_LASER_MAX+1);
	Robot_check_new_modifiers(pl, mods);
	return;

    case OBJ_SHOT:
	/*
	 * Robots usually use wide beam shots, however they may narrow
	 * the beam occasionally.
	 */
	mods.spread = 0;
	if ((my_data->robot_count % 8) == 0)
	    mods.spread = rand() % (MODS_SPREAD_MAX+1);
	Robot_check_new_modifiers(pl, mods);
	return;

    case OBJ_MINE:
	stock = pl->item[ITEM_MINE];
	min = nukeMinMines;
	break;

    case OBJ_SMART_SHOT:
    case OBJ_HEAT_SHOT:
    case OBJ_TORPEDO:
	stock = pl->item[ITEM_MISSILE];
	min = nukeMinSmarts;
	if ((my_data->robot_count % 8) == 0)
	    mods.power = rand() % (MODS_POWER_MAX+1);
	break;

    default:
	return;
    }

    if (stock >= min) {
	/*
	 * More aggressive robots will choose to use nuclear weapons, this
	 * means you can safely approach wimpy robots... perhaps.
	 */
	if ((my_data->robot_count % 100) <= my_data->attack) {
	    SET_BIT(mods.nuclear, NUCLEAR);
	    if (stock > min && (stock < (2 * min)
				|| (my_data->robot_count % 2) == 0))
		    SET_BIT(mods.nuclear, FULLNUCLEAR);
	}
    }

    if (pl->fuel.sum > pl->fuel.l3) {
	if ((my_data->robot_count % 2) == 0) {
	    if ((my_data->robot_count % 8) == 0)
		mods.velocity = (rand() % MODS_VELOCITY_MAX) + 1;
	    SET_BIT(mods.warhead, CLUSTER);
	}
    }
    else if ((my_data->robot_count % 4) == 0) {
	SET_BIT(mods.warhead, IMPLOSION);
    }

    /*
     * Robot may change to use mini device setting occasionally.
     */
    if ((my_data->robot_count % 32) == 0) {
	mods.mini = rand() % (MODS_MINI_MAX+1);
	mods.spread = rand() % (MODS_SPREAD_MAX+1);
    }

    Robot_check_new_modifiers(pl, mods);
}

static bool Check_robot_target(int ind,
			       int item_x, int item_y,
			       int new_mode, int attack_level)
{
    player			*pl = Players[ind],
				*ship;
    long			item_dist;
    int				item_dir;
    int				travel_dir;
    int				delta_dir;
    long			dx, dy;
    long			dist;
    int				locn_block;
    bool			clear_path;
    bool			slowing;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);


    dx = item_x - pl->pos.x, dx = WRAP_DX(dx);
    dy = item_y - pl->pos.y, dy = WRAP_DY(dy);

    item_dist = LENGTH(dy, dx);

    if (dx == 0 && dy == 0) {
	vector	*grav = &World.gravity
	    [(int)pl->pos.x / BLOCK_SZ][(int)pl->pos.y / BLOCK_SZ];
	item_dir = findDir(grav->x, grav->y);
	item_dir = MOD2(item_dir + RES/2, RES);
    } else {
	item_dir = findDir(dx, dy);
    }

    if (new_mode == RM_REFUEL || new_mode == RM_CANNON_KILL)
	item_dist -= 2 * BLOCK_SZ;

    clear_path = true;

    TARGET_INIT();

    for (dist = 0; clear_path && dist < item_dist; dist += BLOCK_SZ / 2) {

	dx = (pl->pos.x + dist * tcos(item_dir)) / BLOCK_SZ;
	dy = (pl->pos.y + dist * tsin(item_dir)) / BLOCK_SZ;

	if (BIT(World.rules->mode, WRAP_PLAY)) {
	    if (dx < 0) dx += World.x;
	    else if (dx >= World.x) dx -= World.x;
	    if (dy < 0) dy += World.y;
	    else if (dy >= World.y) dy -= World.y;
	}
	if (dx < 0 || dx >= World.x || dy < 0 || dy >= World.y) {
	    clear_path = false;
	    continue;
	}
	locn_block = World.block[dx][dy];

	if (!REALLY_EMPTY_SPACE(locn_block,dx,dy) && locn_block != CANNON) {
	    clear_path = false;
	    continue;
	}
    }

    if (!clear_path)
	return false;

    if (pl->velocity <= 0.2) {
	vector	*grav = &World.gravity
	    [(int)pl->pos.x / BLOCK_SZ][(int)pl->pos.y / BLOCK_SZ];
	travel_dir = findDir(grav->x, grav->y);
    } else {
	travel_dir = findDir(pl->vel.x, pl->vel.y);
    }

    pl->turnspeed = MAX_PLAYER_TURNSPEED / 2;
    pl->power = MAX_PLAYER_POWER / 2;

    delta_dir = MOD2(item_dir - travel_dir, RES);
    if (delta_dir >= RES/4 && delta_dir <= 3*RES/4) {

	if (new_mode == RM_HARVEST) {	/* reverse direction of travel */
	    item_dir = MOD2(travel_dir + RES / 2, RES);
	}
	pl->turnspeed = MAX_PLAYER_TURNSPEED;
	slowing = true;

	if (pl->item[ITEM_MINE] && item_dist < 8 * BLOCK_SZ) {
	    Choose_weapon_modifier(pl, OBJ_MINE);
	    Place_moving_mine(ind);
	    new_mode = (rand() & 1) ? RM_EVADE_RIGHT : RM_EVADE_LEFT;
	}
    } else if (new_mode == RM_CANNON_KILL && item_dist <= 0) {

	/* too close, to move away */
	pl->turnspeed = MAX_PLAYER_TURNSPEED;
	item_dir = MOD2(item_dir + RES / 2, RES);
	slowing = true;
    } else {

	slowing = false;
    }

    delta_dir = MOD2(item_dir - pl->dir, RES);

    if (delta_dir > RES / 8 && delta_dir < 7 * RES / 8) {
	pl->turnacc = (delta_dir < RES / 2 ?
		       pl->turnspeed : (-pl->turnspeed));
    } else if ((delta_dir > RES / 16 && delta_dir < 15 * RES / 8)
	       || (my_data->robot_count % 8) == 0) {

	pl->turnspeed = MAX_PLAYER_TURNSPEED / 2;
	pl->turnacc = (delta_dir < RES / 2
		       ? pl->turnspeed : (-pl->turnspeed));
    } else if (delta_dir > RES / 64 && delta_dir < 63 * RES / 64) {

	pl->turnspeed = MAX_PLAYER_TURNSPEED / 2;
	pl->turnacc = (delta_dir < RES / 2 ?
		       pl->turnspeed : (-pl->turnspeed));
    } else {
	pl->turnacc = 0.0;
    }

    if (slowing || BIT(pl->used, OBJ_SHIELD)) {

	SET_BIT(pl->status, THRUSTING);

    } else if (item_dist < 0) {

	CLR_BIT(pl->status, THRUSTING);

    } else if (item_dist < 3*BLOCK_SZ) {

	if (pl->velocity < my_data->robot_normal_speed / 2)
	    SET_BIT(pl->status, THRUSTING);
	if (pl->velocity > my_data->robot_normal_speed)
	    CLR_BIT(pl->status, THRUSTING);

    } else if (new_mode != RM_ATTACK) {

	if (pl->velocity < 2*my_data->robot_normal_speed)
	    SET_BIT(pl->status, THRUSTING);
	if (pl->velocity > 3*my_data->robot_normal_speed)
	    CLR_BIT(pl->status, THRUSTING);

    } else {

	if (pl->velocity < my_data->robot_attack_speed / 2)
	    SET_BIT(pl->status, THRUSTING);
	if (pl->velocity > my_data->robot_attack_speed)
	    CLR_BIT(pl->status, THRUSTING);
    }

    if (new_mode == RM_ATTACK) {
	if (pl->item[ITEM_ECM] > 0 && item_dist < ECM_DISTANCE / 4) {
	    Fire_ecm(ind);
	}
	else if (pl->item[ITEM_TRANSPORTER] > 0
		 && item_dist < TRANSPORTER_DISTANCE
		 && pl->fuel.sum > -ED_TRANSPORTER) {
	    do_transporter(pl);
	    pl->item[ITEM_TRANSPORTER]--;
	    Add_fuel(&(pl->fuel), ED_TRANSPORTER);
	}
	else if (pl->item[ITEM_LASER] > pl->num_pulses
		 && -ED_LASER < pl->fuel.sum - pl->fuel.l3) {
	    if (BIT(my_data->robot_lock, LOCK_PLAYER)
		&& BIT(Players[GetInd[my_data->robot_lock_id]]->status,
		       PLAYING|PAUSE|GAME_OVER) == PLAYING) {
		ship = Players[GetInd[my_data->robot_lock_id]];
	    }
	    else if (pl->shot_max <= 0 && BIT(pl->lock.tagged, LOCK_PLAYER)) {
		ship = Players[GetInd[pl->lock.pl_id]];
	    }
	    else {
		ship = NULL;
	    }
	    if (ship
		&& BIT(ship->status, PLAYING|PAUSE|GAME_OVER) == PLAYING) {

		float	x1, y1, x3, y3, x4, y4, x5, y5;
		float	ship_dist, dir3, dir4, dir5;

		x1 = pl->pos.x + pl->vel.x + pl->ship->m_gun[pl->dir].x;
		y1 = pl->pos.y + pl->vel.y + pl->ship->m_gun[pl->dir].y;
		x3 = ship->pos.x + ship->vel.x;
		y3 = ship->pos.y + ship->vel.y;

		ship_dist = Wrap_length(x3 - x1, y3 - y1);

		if (ship_dist < PULSE_SPEED*PULSE_LIFE(pl->item[ITEM_LASER]) + SHIP_SZ) {
		    dir3 = Wrap_findDir(x3 - x1, y3 - y1);
		    x4 = x3 + tcos(MOD2((int)(dir3 - RES/4), RES)) * SHIP_SZ;
		    y4 = y3 + tsin(MOD2((int)(dir3 - RES/4), RES)) * SHIP_SZ;
		    x5 = x3 + tcos(MOD2((int)(dir3 + RES/4), RES)) * SHIP_SZ;
		    y5 = y3 + tsin(MOD2((int)(dir3 + RES/4), RES)) * SHIP_SZ;
		    dir4 = Wrap_findDir(x4 - x1, y4 - y1);
		    dir5 = Wrap_findDir(x5 - x1, y5 - y1);
		    if ((dir4 > dir5)
			? (pl->dir >= dir4 || pl->dir <= dir5)
			: (pl->dir >= dir4 && pl->dir <= dir5)) {
			SET_BIT(pl->used, OBJ_LASER);
		    }
		}
	    }
	}
	else if (BIT(pl->have, OBJ_TRACTOR_BEAM)) {
	    CLR_BIT(pl->used, OBJ_TRACTOR_BEAM);
	    pl->tractor_pressor = 0;

	    if (BIT(pl->lock.tagged, LOCK_PLAYER)
		&& pl->fuel.sum > pl->fuel.l3
		&& pl->lock.distance < TRACTOR_MAX_RANGE(pl)) {

		float xvd, yvd, vel;
		long dir;
		int away;

		ship = Players[GetInd[pl->lock.pl_id]];
		xvd = ship->vel.x - pl->vel.x;
		yvd = ship->vel.y - pl->vel.y;
		vel = LENGTH(xvd, yvd);
		dir = (long)(findDir(pl->pos.x - ship->pos.x,
				     pl->pos.y - ship->pos.y)
			     - findDir(xvd, yvd));
		dir = MOD2(dir, RES);
		away = (dir >= RES/4 && dir <= 3*RES/4);

		/*
		 * vel  - The relative velocity of ship to us.
		 * away - Heading away from us?
		 */
		if (pl->velocity <= my_data->robot_normal_speed) {
		    if (pl->lock.distance < (SHIP_SZ * 4)
			|| (!away && vel > my_data->robot_attack_speed)) {
			SET_BIT(pl->used, OBJ_TRACTOR_BEAM);
			pl->tractor_pressor = 1;
		    } else if (away
			       && vel < my_data->robot_max_speed
			       && vel > my_data->robot_normal_speed) {
			SET_BIT(pl->used, OBJ_TRACTOR_BEAM);
		    }
		}
		if (BIT(pl->used, OBJ_TRACTOR_BEAM))
		    SET_BIT(pl->lock.tagged, LOCK_VISIBLE);
	    }
	}
	if (BIT(pl->used, OBJ_LASER)) {
	    pl->turnacc = 0.0;
	    Choose_weapon_modifier(pl, OBJ_LASER);
	}
	else if ((my_data->robot_count % 10) == 0 && pl->item[ITEM_MISSILE] > 0) {
	    int type;

	    switch (my_data->robot_count % 5) {
	    case 0: case 1: case 2:	type = OBJ_SMART_SHOT; break;
	    case 3:			type = OBJ_HEAT_SHOT; break;
	    default:			type = OBJ_TORPEDO; break;
	    }
	    Choose_weapon_modifier(pl, type);
	    Fire_shot(ind, type, pl->dir);
	}
	else if ((my_data->robot_count % 2) == 0
		   && item_dist < VISIBILITY_DISTANCE
		   && (BIT(my_data->robot_lock, LOCK_PLAYER) /* robot has target */
		       || (my_data->robot_count
			   % (110 - my_data->attack))
		       < 15+attack_level)) {
	    if (pl->item[ITEM_MISSILE] > 0 && (rand() & 63) == 0) {
		Choose_weapon_modifier(pl, OBJ_SMART_SHOT);
		Fire_shot(ind, OBJ_SMART_SHOT, pl->dir);
	    } else {
		Choose_weapon_modifier(pl, OBJ_SHOT);
		Fire_normal_shots(ind);
	    }
	}
	if ((my_data->robot_count % 32) == 0) {
	    if (pl->fuel.sum > pl->fuel.l3) {
		Choose_weapon_modifier(pl, OBJ_MINE);
		Place_mine(ind);
	    } else if (pl->fuel.sum < pl->fuel.l2) {
		Place_mine(ind);
		CLR_BIT(pl->used, OBJ_CLOAKING_DEVICE);
	    }
	}
    }
    if (new_mode == RM_CANNON_KILL && !slowing) {
	if ((my_data->robot_count % 2) == 0 && item_dist < VISIBILITY_DISTANCE) {
	    Choose_weapon_modifier(pl, OBJ_SHOT);
	    Fire_normal_shots(ind);
	}
    }
    my_data->robot_mode = new_mode;
    return true;
}


static bool Check_robot_hunt(int ind)
{
    player			*pl = Players[ind];
    player			*ship;
    int				ship_dir;
    int				travel_dir;
    int				delta_dir;
    int				adj_dir;
    int				toofast, tooslow;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    if (!BIT(my_data->robot_lock, LOCK_PLAYER)
	|| my_data->robot_lock_id == pl->id)
	return false;
    if (pl->fuel.sum < MAX_PLAYER_FUEL/2)
	return false;

    ship = Players[GetInd[my_data->robot_lock_id]];

    ship_dir = Wrap_findDir(ship->pos.x - pl->pos.x, ship->pos.y - pl->pos.y);

    if (pl->velocity <= 0.2) {
	vector	*grav = &World.gravity
	    [(int)pl->pos.x / BLOCK_SZ][(int)pl->pos.y / BLOCK_SZ];
	travel_dir = findDir(grav->x, grav->y);
    } else {
	travel_dir = findDir(pl->vel.x, pl->vel.y);
    }

    delta_dir = MOD2(ship_dir - travel_dir, RES);
    tooslow = (pl->velocity < my_data->robot_attack_speed/2);
    toofast = (pl->velocity > my_data->robot_attack_speed);

    if (!tooslow && !toofast
	&& (delta_dir <= RES/16 || delta_dir >= 15*RES/16)) {

	pl->turnacc = 0;
	CLR_BIT(pl->status, THRUSTING);
	my_data->robot_mode = RM_ROBOT_IDLE;
	return true;
    }

    adj_dir = (delta_dir<RES/2 ? RES/4 : (-RES/4));

    if (tooslow) adj_dir = adj_dir/2;	/* point forwards more */
    if (toofast) adj_dir = 3*adj_dir/2;	/* point backwards more */

    adj_dir = MOD2(travel_dir + adj_dir, RES);
    delta_dir = MOD2(adj_dir - pl->dir, RES);

    if (delta_dir>=RES/16 && delta_dir<=15*RES/16) {
	pl->turnspeed = MAX_PLAYER_TURNSPEED/4;
	pl->turnacc = (delta_dir<RES/2 ? pl->turnspeed : (-pl->turnspeed));
    }

    if (delta_dir<RES/8 || delta_dir>7*RES/8) {
	SET_BIT(pl->status, THRUSTING);
    } else {
	CLR_BIT(pl->status, THRUSTING);
    }

    my_data->robot_mode = RM_ROBOT_IDLE;
    return true;
}


static void Robot_default_play(int ind)
{
    player			*pl = Players[ind],
				*ship;
    object			*shot;
    float			distance, mine_dist, item_dist, ship_dist,
				enemy_dist, cannon_dist, fuel_dist, target_dist,
				fx, fy, speed, x_speed, y_speed;
    int				j, mine_i, item_i, ship_i,
				enemy_i, cannon_i, fuel_i, target_i, x, y;
    long			dx, dy;
    long			killing_shots;
    bool			harvest_checked;
    bool			fuel_checked;
    bool			evade_checked;
    int				attack_level;
    int				shoot_time;
    int				shield_range;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);


    killing_shots = KILLING_SHOTS;
    if (treasureCollisionMayKill) {
	killing_shots |= OBJ_BALL;
    }

    if (my_data->robot_count <= 0)
	my_data->robot_count = 1000 + rand() % 32;

    my_data->robot_count--;

    CLR_BIT(pl->used, OBJ_SHIELD | OBJ_CLOAKING_DEVICE | OBJ_LASER);
    if (BIT(pl->have, OBJ_EMERGENCY_SHIELD)
	&& !BIT(pl->used, OBJ_EMERGENCY_SHIELD)) {
	Emergency_shield(ind, 1);
    }
    harvest_checked = false;
    fuel_checked = false;
    evade_checked = false;

    mine_i = -1;
    mine_dist = SHIP_SZ + 200;
    item_i = -1;
    item_dist = VISIBILITY_DISTANCE;

    if (BIT(pl->have, OBJ_CLOAKING_DEVICE) && pl->fuel.sum > pl->fuel.l3)
	SET_BIT(pl->used, OBJ_CLOAKING_DEVICE);

    for (j = 0; j < NumObjs; j++) {

	shot = Obj[j];

	/* Get rid of the most common object types first for speed. */
	if (BIT(shot->type, OBJ_DEBRIS|OBJ_SPARK)) {
	    continue;
	}
	if (BIT(shot->type, OBJ_ITEM)) {
	    if ((dx = shot->pos.x - pl->pos.x,
		    dx = WRAP_DX(dx),
		    ABS(dx)) < item_dist
		&& (dy = shot->pos.y - pl->pos.y,
		    dy = WRAP_DY(dy),
		    ABS(dy)) < item_dist
		&& (distance = LENGTH(dx, dy)) < item_dist) {
		item_i = j;
		item_dist = distance;
	    }
	    continue;
	}
	if (BIT(shot->type, OBJ_SMART_SHOT|OBJ_HEAT_SHOT|OBJ_MINE)) {
	    fx = shot->pos.x - pl->pos.x;
	    fy = shot->pos.y - pl->pos.y;
	    if ((dx = fx, dx = WRAP_DX(dx), ABS(dx)) < mine_dist
		&& (dy = fy, dy = WRAP_DY(dy), ABS(dy)) < mine_dist
		&& (distance = LENGTH(dx, dy)) < mine_dist) {
		mine_i = j;
		mine_dist = distance;
	    }
	    if ((dx = fx + (shot->vel.x - pl->vel.x) * ROB_LOOK_AH,
		    dx = WRAP_DX(dx), ABS(dx)) < mine_dist
		&& (dy = fy + (shot->vel.y - pl->vel.y) * ROB_LOOK_AH,
		    dy = WRAP_DY(dy), ABS(dy)) < mine_dist
		&& (distance = LENGTH(dx, dy)) < mine_dist) {
		mine_i = j;
		mine_dist = distance;
	    }
	}

	/*
	 * The only thing left to do regarding objects is to check if
	 * this robot needs to put up shields to protect against objects.
	 */
	if (!BIT(shot->type, killing_shots)) {
	    continue;
	}

	/*
	 * Any shot of team members excluding self are passive.
	 */
	if (shot->id >= 0
	    && TEAM_IMMUNE(GetInd[shot->id], ind)
	    && shot->id != pl->id) {
	    continue;
	}

	shield_range = 20 + SHIP_SZ + shot->pl_range;

	if ((dx = shot->pos.x + shot->vel.x - pl->pos.x + pl->vel.x,
		dx = WRAP_DX(dx),
		ABS(dx)) < shield_range
	    && (dy = shot->pos.y + shot->vel.y - pl->pos.y + pl->vel.y,
		dy = WRAP_DY(dy),
		ABS(dy)) < shield_range
	    && sqr(dx) + sqr(dy) <= sqr(shield_range)
	    && (my_data->robot_count % 100 < ((my_data->defense + 700) / 8)
		|| shot->id == -1
		|| shot->id == pl->id
	     /* || Players[GetInd[shot->id]]->score > 50	*/
		)) {
	    SET_BIT(pl->used, OBJ_SHIELD);
	    SET_BIT(pl->status, THRUSTING);

	    if (BIT(shot->type, OBJ_SMART_SHOT)) {
		if (mine_dist < ECM_DISTANCE / 4)
		    Fire_ecm(ind);
	    }
	    if (BIT(shot->type, OBJ_MINE)) {
		if (mine_dist < ECM_DISTANCE / 2)
		    Fire_ecm(ind);
	    }
	}
    }


    /*
     * Test if others are firing lasers at us.
     * Maybe move this into the player loop.
     */
    if (BIT(pl->used, OBJ_SHIELD) == 0
	&& BIT(pl->have, OBJ_SHIELD) != 0) {
	for (j = 0; j < NumPlayers; j++) {
	    ship = Players[j];
	    if (j == ind
		|| BIT(ship->status, PLAYING|PAUSE|GAME_OVER) != PLAYING)
		continue;
	    if (ship->num_pulses > 0) {
		distance = Wrap_length(pl->pos.x - ship->pos.x,
				       pl->pos.y - ship->pos.y);
		if (PULSE_SPEED*PULSE_LIFE(ship->item[ITEM_LASER]) + 2*SHIP_SZ
		    >= distance) {
		    int delta_dir = Wrap_findDir(pl->pos.x - ship->pos.x,
						 pl->pos.y - ship->pos.y),
			ship_dir = ship->dir + (ship->turnvel
			    + ship->turnacc) * ship->turnresistance;
		    if ((delta_dir - ship_dir < 0)
			? (ship_dir - delta_dir < RES/8
			    || delta_dir + RES - ship_dir < RES/8)
			: (delta_dir - ship_dir < RES/8
			    || ship_dir + RES - delta_dir < RES/8)) {
			SET_BIT(pl->used, OBJ_SHIELD);
			break;
		    }
		}
	    }
	}
    }

    /* Note: Only take time to navigate if not being shot at */
    if (!(BIT(pl->used, OBJ_SHIELD) && SET_BIT(pl->status, THRUSTING))
	&& Check_robot_navigate(ind, &evade_checked)) {
	if (playerShielding == 0
	    && playerStartsShielded != 0
	    && BIT(pl->have, OBJ_SHIELD)) {
	    SET_BIT(pl->used, OBJ_SHIELD);
	}
	return;
    }

    ship_i = -1;
    ship_dist = (pl->fuel.sum >= pl->fuel.l1 ? SHIP_SZ * 6 : 0);
    enemy_i = -1;
    if (pl->fuel.sum >= pl->fuel.l3)
	enemy_dist = (BIT(World.rules->mode, LIMITED_VISIBILITY) ?
		      MAX(pl->fuel.sum * ENERGY_RANGE_FACTOR,
			  VISIBILITY_DISTANCE)
		      : ((float)World.hypotenuse));
    else
	enemy_dist = VISIBILITY_DISTANCE;

    if (BIT(pl->used, OBJ_SHIELD))
	ship_dist = 0;

    for (j = 0; j < NumPlayers; j++) {
	ship = Players[j];
	if (j == ind
	    || BIT(ship->status, PLAYING|GAME_OVER|PAUSE) != PLAYING
	    || TEAM_IMMUNE(ind, j))
	    continue;

	dx = ship->pos.x - pl->pos.x, dx = WRAP_DX(dx);
	dy = ship->pos.y - pl->pos.y, dy = WRAP_DY(dy);
	distance = LENGTH(dx, dy);

	if (distance < ship_dist) {
	    ship_i = j;
	    ship_dist = distance;
	}

	if (BIT(my_data->robot_lock, LOCK_PLAYER)
	    && BIT(Players[GetInd[my_data->robot_lock_id]]->status,
		   PLAYING|PAUSE|GAME_OVER) == PLAYING) {
	    /* ignore all players unless target */
	    if (my_data->robot_lock_id == ship->id
		&& distance < enemy_dist) {

		enemy_i = j;
		enemy_dist = distance;
	    }
	} else {
	    if (IS_HUMAN_PTR(ship)
		&& distance < enemy_dist) {
		enemy_i    = j;
		enemy_dist = distance;
	    }
	}
    }

    if (ship_dist <= 3*SHIP_SZ && BIT(pl->have, OBJ_SHIELD))
	SET_BIT(pl->used, OBJ_SHIELD);

    if (BIT(my_data->robot_lock, LOCK_PLAYER)
	&& ship_i != -1
	&& my_data->robot_lock_id == Players[ship_i]->id)
	ship_i = -1; /* don't avoid target */

    if (BIT(pl->lock.tagged, LOCK_PLAYER)) {
	ship = Players[GetInd[pl->lock.pl_id]];
	if (BIT(ship->status, PLAYING|PAUSE|GAME_OVER) != PLAYING
	    || (BIT(my_data->robot_lock, LOCK_PLAYER)
		&& my_data->robot_lock_id != pl->lock.pl_id
		&& BIT(Players[GetInd[my_data->robot_lock_id]]->status,
		       PLAYING|PAUSE|GAME_OVER) == PLAYING)
	    || pl->lock.distance > 2 * VISIBILITY_DISTANCE
	    || (!(pl->visibility[GetInd[pl->lock.pl_id]].canSee)
		&& (my_data->robot_count % 25) == 0)) {
	    /* unset the player lock */
	    CLR_BIT(pl->lock.tagged, LOCK_PLAYER);
	    pl->lock.pl_id = 1;
	    pl->lock.pos.x = pl->pos.x;
	    pl->lock.pos.y = pl->pos.y;
	    pl->lock.distance = 0;
	}
    }
    if (enemy_i >= 0) {
	ship = Players[enemy_i];
	if (!BIT(pl->lock.tagged, LOCK_PLAYER)
	    || enemy_dist < 3*pl->lock.distance/4) {
	    pl->lock.pl_id = ship->id;
	    SET_BIT(pl->lock.tagged, LOCK_PLAYER);
	    pl->lock.pos.x = ship->pos.x;
	    pl->lock.pos.y = ship->pos.y;
	    pl->lock.distance = enemy_dist;
	    pl->sensor_range = VISIBILITY_DISTANCE;
	}
    }
    if (!evade_checked) {
	if (Check_robot_evade(ind, mine_i, ship_i)) {
	    if (playerShielding == 0
		&& playerStartsShielded != 0
		&& BIT(pl->have, OBJ_SHIELD)) {
		SET_BIT(pl->used, OBJ_SHIELD);
	    }
	    else if (maxShieldedWallBounceSpeed >
		    maxUnshieldedWallBounceSpeed
		&& maxShieldedWallBounceAngle >=
		    maxUnshieldedWallBounceAngle
		&& BIT(pl->have, OBJ_SHIELD)) {
		SET_BIT(pl->used, OBJ_SHIELD);
	    }
	    return;
	}
    }
    if (item_i >= 0 && enemy_dist > /* 2* */ item_dist
	/* && enemy_dist > 12*BLOCK_SZ */ ) {

	harvest_checked = true;
	dx = Obj[item_i]->pos.x;
	dy = Obj[item_i]->pos.y + SHIP_SZ / 2;

	if (Check_robot_target(ind, dx, dy, RM_HARVEST, 0)) {
	    return;
	}
    }
    if (BIT(pl->lock.tagged, LOCK_PLAYER)) {

	ship = Players[GetInd[pl->lock.pl_id]];
	shoot_time = pl->lock.distance / (pl->shot_speed + 1);
	dx = ship->pos.x + ship->vel.x * shoot_time;
	dy = ship->pos.y + ship->vel.y * shoot_time + SHIP_SZ / 2;
	attack_level = MAX(ship->score / 8, 0);

	if (Check_robot_target(ind, dx, dy, RM_ATTACK, attack_level)) {
	    return;
	}
    }
    if (item_i >= 0 && !harvest_checked) {

	dx = Obj[item_i]->pos.x;
	dy = Obj[item_i]->pos.y + SHIP_SZ / 2;

	if (Check_robot_target(ind, dx, dy, RM_HARVEST, 0)) {
	    return;
	}
    }

    if (Check_robot_hunt(ind)) {
	if (playerShielding == 0
	    && playerStartsShielded != 0
	    && BIT(pl->have, OBJ_SHIELD)) {
	    SET_BIT(pl->used, OBJ_SHIELD);
	}
	return;
    }

    cannon_i = -1;
    cannon_dist = VISIBILITY_DISTANCE;
    fuel_i = -1;
    fuel_dist = VISIBILITY_DISTANCE;
    target_i = -1;
    target_dist = VISIBILITY_DISTANCE;

    for (j = 0; j < World.NumCannons; j++) {

	if (World.cannon[j].dead_time > 0)
	    continue;

	if ((dx = World.cannon[j].pos.x*BLOCK_SZ + BLOCK_SZ/2 - pl->pos.x,
		dx = WRAP_DX(dx), ABS(dx)) < cannon_dist
	    && (dy = World.cannon[j].pos.y*BLOCK_SZ+BLOCK_SZ/2-pl->pos.y,
		dy = WRAP_DY(dy), ABS(dy)) < cannon_dist
	    && (distance = LENGTH(dx, dy)) < cannon_dist) {
	    cannon_i = j;
	    cannon_dist = distance;
	}
    }

    for (j = 0; j < World.NumFuels; j++) {

	if (World.fuel[j].fuel < 100 * FUEL_SCALE_FACT
	    || pl->fuel.sum >= MAX_PLAYER_FUEL - 200 * FUEL_SCALE_FACT)
	    continue;

	if ((dx = World.fuel[j].pix_pos.x - pl->pos.x,
		dx = WRAP_DX(dx), ABS(dx)) < fuel_dist
	    && (dy = World.fuel[j].pix_pos.y - pl->pos.y,
		dy = WRAP_DY(dy), ABS(dy)) < fuel_dist
	    && (distance = LENGTH(dx, dy)) < fuel_dist) {
	    if (World.block[World.fuel[j].blk_pos.x]
			   [World.fuel[j].blk_pos.y] == FUEL) {
		fuel_i = j;
		fuel_dist = distance;
	    }
	}
    }

    for (j = 0; j < World.NumTargets; j++) {

	/* Ignore dead or owned targets */
	if (World.targets[j].dead_time > 0
	    || pl->team == World.targets[j].team)
	    continue;

	if ((dx = World.targets[j].pos.x*BLOCK_SZ + BLOCK_SZ/2 - pl->pos.x,
		dx = WRAP_DX(dx), ABS(dx)) < target_dist
	    && (dy = World.targets[j].pos.y*BLOCK_SZ+BLOCK_SZ/2-pl->pos.y,
		dy = WRAP_DY(dy), ABS(dy)) < target_dist
	    && (distance = LENGTH(dx, dy)) < target_dist) {
	    target_i = j;
	    target_dist = distance;
	}
    }

    if (fuel_i >= 0 && cannon_dist > fuel_dist) {

	fuel_checked = true;
	dx = World.fuel[fuel_i].pix_pos.x;
	dy = World.fuel[fuel_i].pix_pos.y + SHIP_SZ / 2;

	SET_BIT(pl->used, OBJ_REFUEL);
	pl->fs = fuel_i;

	if (Check_robot_target(ind, dx, dy, RM_REFUEL, 0)) {
	    return;
	}
    }
    if (cannon_i >= 0) {

	dx = World.cannon[cannon_i].pos.x * BLOCK_SZ + BLOCK_SZ / 2;
	dy = World.cannon[cannon_i].pos.y * BLOCK_SZ + BLOCK_SZ / 2;

	if (Check_robot_target(ind, dx, dy, RM_CANNON_KILL, 0)) {
	    return;
	}
    }
    if (target_i >= 0) {
	dx = World.targets[target_i].pos.x * BLOCK_SZ + BLOCK_SZ / 2;
	dy = World.targets[target_i].pos.y * BLOCK_SZ + BLOCK_SZ / 2;

	if (Check_robot_target(ind, dx, dy, RM_CANNON_KILL, 0)) {
	    return;
	}
    }

    if (fuel_i >= 0 && !fuel_checked) {

	dx = World.fuel[fuel_i].pix_pos.x;
	dy = World.fuel[fuel_i].pix_pos.y + SHIP_SZ / 2;

	SET_BIT(pl->used, OBJ_REFUEL);
	pl->fs = fuel_i;

	if (Check_robot_target(ind, dx, dy, RM_REFUEL, 0))
	    return;
    }
    if (pl->fuel.sum < DEFAULT_PLAYER_FUEL)
	Add_fuel(&(pl->fuel), (int)(FUEL_SCALE_FACT * 0.02));
    if (pl->fuel.sum < MIN_PLAYER_FUEL)
	Add_fuel(&(pl->fuel), pl->fuel.sum - MIN_PLAYER_FUEL);

    if (playerShielding == 0
	&& playerStartsShielded != 0
	&& BIT(pl->have, OBJ_SHIELD)) {
	SET_BIT(pl->used, OBJ_SHIELD);
    }

    x = pl->pos.x/BLOCK_SZ;
    y = pl->pos.y/BLOCK_SZ;
    LIMIT(x, 0, World.x);
    LIMIT(y, 0, World.y);
    x_speed = pl->vel.x - 2 * World.gravity[x][y].x;
    y_speed = pl->vel.y - 2 * World.gravity[x][y].y;

    if (y_speed < (-my_data->robot_normal_speed) || (my_data->robot_count % 64) < 32) {

	my_data->robot_mode = RM_ROBOT_CLIMB;
	pl->turnspeed = MAX_PLAYER_TURNSPEED / 2;
	pl->power = MAX_PLAYER_POWER / 2;
	if (ABS(pl->dir - RES / 4) > RES / 16) {
	    pl->turnacc = (pl->dir < RES / 4
			   || pl->dir >= 3 * RES / 4
			   ? pl->turnspeed : (-pl->turnspeed));
	} else {
	    pl->turnacc = 0.0;
	}
	if (y_speed < my_data->robot_normal_speed / 2
	    && pl->velocity < my_data->robot_attack_speed)
	    SET_BIT(pl->status, THRUSTING);
	else if (y_speed > my_data->robot_normal_speed)
	    CLR_BIT(pl->status, THRUSTING);
	return;
    }
    /* must be idle */
    my_data->robot_mode = RM_ROBOT_IDLE;
    pl->turnspeed = MAX_PLAYER_TURNSPEED / 2;
    pl->turnacc = 0;
    pl->power = MAX_PLAYER_POWER / 2;
    CLR_BIT(pl->status, THRUSTING);
    speed = LENGTH(x_speed, y_speed);
    if (speed < my_data->robot_normal_speed / 2)
	SET_BIT(pl->status, THRUSTING);
    else if (speed > my_data->robot_normal_speed)
	CLR_BIT(pl->status, THRUSTING);
}


/* $Id: object.h,v 4.1 1998/04/16 17:41:42 bert Exp $
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

#ifndef	OBJECT_H
#define	OBJECT_H

#ifndef KEYS_H
/* need NUM_KEYS */
#include "keys.h"
#endif
#ifndef BIT_H
/* need BITV_DECL */
#include "bit.h"
#endif
#ifndef DRAW_H
/* need wireobj */
#include "draw.h"
#endif
#ifndef ITEM_H
/* need NUM_ITEMS */
#include "item.h"
#endif
#ifndef CLICK_H
/* need CLICK */
#include "click.h"
#endif

#ifdef	_WINDOWS
#include "NT/winNet.h"
#endif

/*
 * Different types of objects, including player.
 * Robots and tanks are players but have an additional bit.
 * Smart missile, heatseeker and torpedoe can be merged into missile.
 * ECM doesn't really need an object type.
 * Lasers and pulses can be merged.
 */
#define OBJ_PLAYER		(1U<<0)
#define OBJ_DEBRIS		(1U<<1)
#define OBJ_SPARK		(1U<<2)
#define OBJ_EMERGENCY_THRUST	(1U<<3)
#define OBJ_AUTOPILOT		(1U<<4)
#define OBJ_TRACTOR_BEAM	(1U<<5)
#define OBJ_LASER		(1U<<6)
#define OBJ_BALL		(1U<<7)
#define OBJ_SHOT		(1U<<8)
#define OBJ_SMART_SHOT		(1U<<9)
#define OBJ_CLOAKING_DEVICE	(1U<<10)
#define OBJ_SHIELD		(1U<<11)
#define OBJ_REFUEL		(1U<<12)
#define OBJ_REPAIR		(1U<<13)
#define OBJ_COMPASS		(1U<<14)
#define OBJ_MINE		(1U<<15)
#define OBJ_ECM			(1U<<16)
#define OBJ_TORPEDO		(1U<<17)
#define OBJ_HEAT_SHOT		(1U<<18)
#define OBJ_AFTERBURNER		(1U<<19)
#define OBJ_CONNECTOR		(1U<<20)
#define OBJ_PULSE		(1U<<21)
#define OBJ_EMERGENCY_SHIELD	(1U<<22)
#define OBJ_DEFLECTOR		(1U<<23)
#define OBJ_PHASING_DEVICE	(1U<<24)
#define OBJ_ITEM		(1U<<25)
#define OBJ_WRECKAGE		(1U<<26)

/*
 * Some object types are overloaded.
 */
#define OBJ_EXT_TANK		(1U<<1)
#define OBJ_EXT_ROBOT		(1U<<2)

#define IS_TANK_IND(ind)	IS_TANK_PTR(Players[ind])
#define IS_ROBOT_IND(ind)	IS_ROBOT_PTR(Players[ind])
#define IS_HUMAN_IND(ind)	IS_HUMAN_PTR(Players[ind])
#define IS_TANK_PTR(pl)		(BIT((pl)->type_ext,OBJ_EXT_TANK)==OBJ_EXT_TANK)
#define IS_ROBOT_PTR(pl)	(BIT((pl)->type_ext,OBJ_EXT_ROBOT)==OBJ_EXT_ROBOT)
#define IS_HUMAN_PTR(pl)	(!BIT((pl)->type_ext,OBJ_EXT_TANK|OBJ_EXT_ROBOT))

/*
 * Weapons modifiers.
 */
typedef struct {
    unsigned int	nuclear	:2;	/* N  modifier */
    unsigned int	warhead	:2;	/* CI modifier */
    unsigned int	velocity:2;	/* V# modifier */
    unsigned int	mini	:2;	/* X# modifier */
    unsigned int	spread	:2;	/* Z# modifier */
    unsigned int	power	:2;	/* B# modifier */
    unsigned int	laser	:2;	/* LS LB modifier */
    unsigned int	spare	:2;	/* spare */
} modifiers;

#define CLEAR_MODS(mods)	memset(&(mods), 0, sizeof(modifiers))

#define MODS_NUCLEAR_MAX	2	/* - N FN */
#define NUCLEAR			(1U<<0)
#define FULLNUCLEAR		(1U<<1)

#define MODS_WARHEAD_MAX	3	/* - C I CI */
#define CLUSTER			(1U<<0)
#define IMPLOSION		(1U<<1)

#define MODS_VELOCITY_MAX	3	/* - V1 V2 V3 */
#define MODS_MINI_MAX		3	/* - X2 X3 X4 */
#define MODS_SPREAD_MAX		3	/* - Z1 Z2 Z3 */
#define MODS_POWER_MAX		3	/* - B1 B2 B3 */

#define MODS_LASER_MAX		2	/* - LS LB */
#define STUN			(1U<<0)
#define BLIND			(1U<<1)

#define LOCK_NONE		0x00	/* No lock */
#define LOCK_PLAYER		0x01	/* Locked on player */
#define LOCK_VISIBLE		0x02	/* Lock information was on HUD */
					/* computed just before frame shown */
					/* and client input checked */
#define LOCKBANK_MAX		4	/* Maximum number of locks in bank */

#define NOT_CONNECTED		(-1)

/*
 * Object position is non-modifiable, except at one place.
 *
 * NB: position in pixels used to be a float.
 */
typedef const struct _objposition objposition;
struct _objposition {
    int		cx, cy;			/* object position in clicks. */
    int		x, y;			/* object position in pixels. */
    int		bx, by;			/* object position in blocks. */
};
#define OBJ_X_IN_CLICKS(obj)	((obj)->pos.cx)
#define OBJ_Y_IN_CLICKS(obj)	((obj)->pos.cy)
#define OBJ_X_IN_PIXELS(obj)	((obj)->pos.x)
#define OBJ_Y_IN_PIXELS(obj)	((obj)->pos.y)
#define OBJ_X_IN_BLOCKS(obj)	((obj)->pos.bx)
#define OBJ_Y_IN_BLOCKS(obj)	((obj)->pos.by)

typedef struct _object object;
struct _object {
    byte	color;			/* Color of object */
    u_byte	dir;			/* Direction of acceleration */
    int		id;			/* For shots => id of player */
    objposition	pos;			/* World coordinates */
    ipos	prevpos;		/* Object's previous position... */
    vector	vel;
    vector	acc;
    DFLOAT	max_speed;
    DFLOAT	mass;
    int		type;
    long	info;			/* Miscellaneous info */
    long	life;			/* No of ticks left to live */
    int		count;			/* Misc timings */
    long	status;
    modifiers	mods;			/* Modifiers to this object */

    /* up to here all object types (including players!) should be the same. */

    DFLOAT	turnspeed;		/* for missiles only */
    long	fuselife;		/* Ticks left when considered fused */

    object	*cell_list;		/* linked list for cell lookup */

    int 	owner;			/* Who's object is this ? */
					/* (spare for id)*/
    int		treasure;		/* Which treasure does ball belong */
    int		new_info;		/* smart re-lock id after confusion */
    DFLOAT	length;			/* Distance between ball and player */
    DFLOAT	ecm_range;		/* Range from last ecm center */
    int		spread_left;		/* how much spread time left */
    int		pl_range;		/* distance to player for collision. */
    int		pl_radius;		/* distance to player for hit. */

    u_byte	size;			/* Size of object (wreckage) */
    u_byte	rotation;		/* Rotation direction */

#ifdef __cplusplus
		_object() {}
#endif
};


/*
 * Fuel structure, used by player
 */
typedef struct {
    long	sum;			/* Sum of fuel in all tanks */
    long	max;			/* How much fuel can you take? */
    int		current;		/* Number of currently used tank */
    int		num_tanks;		/* Number of tanks */
    long	tank[1 + MAX_TANKS];	/* main fixed tank + extra tanks. */
    long	l1;			/* Fuel critical level */
    long	l2;			/* Fuel warning level */
    long	l3;			/* Fuel notify level */
} pl_fuel_t;

struct _visibility {
    int		canSee;
    long	lastChange;
};

#define MAX_PLAYER_ECMS		8	/* Maximum simultaneous per player */
typedef struct _ecm_info ecm_info;
struct _ecm_info {
    int		count;
    int 	size[MAX_PLAYER_ECMS];
    position 	pos[MAX_PLAYER_ECMS];
};

/*
 * Structure holding the info for one pulse of a laser.
 */
typedef struct {
    position	pos;
    int		dir;
    int		len;
    int		life;
    modifiers	mods;
} pulse_t;

/*
 * Transporter info, this is per-player
 */
typedef struct _trans_info trans_info;
struct _trans_info {
    int 	count,
		pl_id;
};

/*
 * Shove-information.
 *
 * This is for keeping a record of the last N times the player was shoved,
 * for assigning wall-smash-blame, where N=MAX_RECORDED_SHOVES.
 */
#define MAX_RECORDED_SHOVES 4

typedef struct {
    int		pusher_id;
    int		time;
} shove_t;

struct robot_data;

/* IMPORTANT
 *
 * This is the player structure, the first part MUST be similar to object_t,
 * this makes it possible to use the same basic operations on both of them
 * (mainly used in update.c).
 */
typedef struct player player;
struct player {
    byte	color;			/* Color of object */
    u_byte	dir;			/* Direction of acceleration */
    int		id;			/* Unique id of object */
    objposition	pos;			/* World coordinates */
    ipos	prevpos;		/* Previous position... */
    vector	vel;			/* Velocity of object */
    vector	acc;			/* Acceleration constant */
    DFLOAT	max_speed;		/* Maximum speed of object */
    DFLOAT	mass;			/* Mass of object (incl. cargo) */
    int		type;			/* Type of object */
    long	info;			/* Miscellaneous info */
    int		life;			/* Zero is dead. One is alive */
    int		count;			/* Miscellaneous timings */
    long	status;			/** Status, currently **/
    modifiers	mods;			/* Modifiers in effect */

    /* up to here the player type should be the same as an object. */

    int		type_ext;		/* extended type info (tank, robot) */

    DFLOAT	turnspeed;		/* How fast player acc-turns */
    DFLOAT	velocity;		/* Absolute speed */

    int		kills;			/* Number of kills this round */
    int		deaths;			/* Number of deaths this round */

    long	used;			/** Items you use **/
    long	have;			/** Items you have **/

    int		shield_time;		/* Shields if no playerShielding */
    pl_fuel_t	fuel;			/* ship tanks and the stored fuel */
    DFLOAT	emptymass;		/* Mass of empty ship */
    DFLOAT	float_dir;		/* Direction, in float var */
    DFLOAT	turnresistance;		/* How much is lost in % */
    DFLOAT	turnvel;		/* Current velocity of turn (right) */
#ifdef TURN_FUEL
    DFLOAT	oldturnvel;		/* Last velocity of turn (right) */
#endif
    DFLOAT	turnacc;		/* Current acceleration of turn */
    long	mode;			/* Player mode, currently */
    long	score;			/* Current score of player */
    long	prev_score;		/* Last score that has been updated */
    int		prev_life;		/* Last life that has been updated */
    wireobj	*ship;			/* wire model of ship shape */
    DFLOAT	power;			/* Force of thrust */
    DFLOAT	power_s;		/* Saved power fiks */
    DFLOAT	turnspeed_s;		/* Saved turnspeed */
    DFLOAT	turnresistance_s;	/* Saved (see above) */
    DFLOAT	sensor_range;		/* Range of sensors (radar) */
    int		shots;			/* Number of active shots by player */
    int		missile_rack;		/* Next missile rack to be active */

    int		num_pulses;		/* Number of laser pulses in the air. */
    int		max_pulses;		/* Max. number of laser pulses. */
    pulse_t	*pulses;		/* Info on laser pulses. */

    int		emergency_thrust_left;	/* how much emergency thrust left */
    int		emergency_thrust_max;	/* maximum time left */
    int		emergency_shield_left;	/* how much emergency shield left */
    int		emergency_shield_max;	/* maximum time left */
    int		phasing_left;		/* how much time left */
    int		phasing_max;		/* maximum time left */

    int		item[NUM_ITEMS];	/* for each item type how many */
    int		lose_item;		/* which item to drop */
    int		lose_item_state;	/* lose item key state, 2=up,1=down */

    DFLOAT	auto_power_s;		/* autopilot saves of current */
    DFLOAT	auto_turnacc_s;		/* power, turnacc, turnspeed and */
    DFLOAT	auto_turnspeed_s;	/* turnresistance settings. Restored */
    DFLOAT	auto_turnresistance_s;	/* when autopilot turned off */
    modifiers	modbank[NUM_MODBANKS];	/* useful modifier settings */
    int		tractor_pressor;	/* non-zero if tractor is pressor */
    player	*tractor;		/* target of tractor beam */
    int		shot_max;		/* Maximum number of shots active */
    int		shot_life;		/* Number of ticks shot will live */
    DFLOAT	shot_speed;		/* Speed of shots fired by player */
    DFLOAT	shot_mass;		/* Mass of shots fired by player */
    long	shot_time;		/* Time of last shot fired by player */
    int		repair_target;		/* Repairing this target */
    int		fs;			/* Connected to fuel station fs */
    int		check;			/* Next check point to pass */
    int		prev_check;		/* Previous check point for score */
    int		time;			/* The time a player has used */
    int		round;			/* Number of rounds player have done */
    int		prev_round;		/* Previous rounds value for score */
    int		best_lap;		/* Players best lap time */
    int		last_lap;		/* Time on last pass */
    int		last_lap_time;		/* What was your last pass? */
    int		last_check_dir;		/* player dir at last checkpoint */
    long	last_wall_touch;	/* last time player touched a wall */

    int		home_base;		/* Num of home base */
    struct {
	int	    tagged;		/* Flag, what is tagged? */
	int	    pl_id;		/* Tagging player id */
	DFLOAT	    distance;		/* Distance to object */
    } lock;
    int		lockbank[LOCKBANK_MAX]; /* Saved player locks */

    char	mychar;			/* Special char for player */
    char	prev_mychar;		/* Special char for player */
    char	name[MAX_CHARS];	/* Nick-name of player */
    char	realname[MAX_CHARS];	/* Real name of player */
    char	hostname[MAX_CHARS];	/* Hostname of client player uses */
    u_short	team;			/* What team is the player on? */
    u_short	pseudo_team;		/* Which team is used for my tanks */
					/* (detaching!) */
    object	*ball;

    /*
     * Pointer to robot private data (dynamically allocated).
     * Only used in robot code.
     */
    struct robot_data	*robot_data_ptr;

    /*
     * A record of who's been pushing me (a circular buffer).
     */
    shove_t     shove_record[MAX_RECORDED_SHOVES];
    int         shove_next;

    struct _visibility *visibility;

    int		updateVisibility, forceVisible, damaged;
    int		wormDrawCount, wormHoleHit, wormHoleDest;

    ecm_info	ecmInfo;		/* list of active ecms */
    trans_info  transInfo;		/* list of active transporters */

    int		conn;			/* connection index, -1 if robot */
    unsigned	version;		/* XPilot version number of client */

    BITV_DECL(last_keyv, NUM_KEYS);	/* Keyboard state */
    BITV_DECL(prev_keyv, NUM_KEYS);	/* Keyboard state */

    long	frame_last_busy;	/* When player touched keyboard. */

    void	*audio;			/* audio private data */

    int		player_fps;		/* FPS that this player can do */
    int		player_round;		/* Divisor for player FPS calculation */
    int		player_count;		/* Player's current frame count */

    int		isowner;		/* If player started this server. */

#ifdef __cplusplus
		player() {}
#endif
};

#endif

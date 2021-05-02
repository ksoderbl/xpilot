/* $Id: object.h,v 5.17 2001/05/24 11:26:50 bertg Exp $
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
/* need shipobj */
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

#ifdef _WINDOWS
#include "NT/winNet.h"
#endif

/*
 * Different types of objects, including player.
 * Robots and tanks are players but have an additional type_ext field.
 * Smart missile, heatseeker and torpedoe can be merged into missile.
 * ECM doesn't really need an object type.
 */
#define OBJ_PLAYER		(1U<<0)
#define OBJ_DEBRIS		(1U<<1)
#define OBJ_SPARK		(1U<<2)
#define OBJ_BALL		(1U<<3)
#define OBJ_SHOT		(1U<<4)
#define OBJ_SMART_SHOT		(1U<<5)
#define OBJ_MINE		(1U<<6)
#define OBJ_TORPEDO		(1U<<7)
#define OBJ_HEAT_SHOT		(1U<<8)
#define OBJ_PULSE		(1U<<9)
#define OBJ_ITEM		(1U<<10)
#define OBJ_WRECKAGE		(1U<<11)
#define OBJ_ASTEROID		(1U<<12)


/*
 * Some object types are overloaded.
 * These bits are set in the player->type_ext field.
 */
#define OBJ_EXT_TANK		(1U<<1)
#define OBJ_EXT_ROBOT		(1U<<2)

/* macro's to query the type of player. */
#define IS_TANK_IND(ind)	IS_TANK_PTR(Players[ind])
#define IS_ROBOT_IND(ind)	IS_ROBOT_PTR(Players[ind])
#define IS_HUMAN_IND(ind)	IS_HUMAN_PTR(Players[ind])
#define IS_TANK_PTR(pl)		(BIT((pl)->type_ext,OBJ_EXT_TANK)==OBJ_EXT_TANK)
#define IS_ROBOT_PTR(pl)	(BIT((pl)->type_ext,OBJ_EXT_ROBOT)==OBJ_EXT_ROBOT)
#define IS_HUMAN_PTR(pl)	(!BIT((pl)->type_ext,OBJ_EXT_TANK|OBJ_EXT_ROBOT))


/*
 * Different types of attributes a player can have.
 * These are the bits of the player->have and player->used fields.
 */
#define HAS_EMERGENCY_THRUST	(1U<<30)
#define HAS_AUTOPILOT		(1U<<29)
#define HAS_TRACTOR_BEAM	(1U<<28)
#define HAS_LASER		(1U<<27)
#define HAS_CLOAKING_DEVICE	(1U<<26)
#define HAS_SHIELD		(1U<<25)
#define HAS_REFUEL		(1U<<24)
#define HAS_REPAIR		(1U<<23)
#define HAS_COMPASS		(1U<<22)
#define HAS_AFTERBURNER		(1U<<21)
#define HAS_CONNECTOR		(1U<<20)
#define HAS_EMERGENCY_SHIELD	(1U<<19)
#define HAS_DEFLECTOR		(1U<<18)
#define HAS_PHASING_DEVICE	(1U<<17)
#define HAS_MIRROR		(1U<<16)
#define HAS_ARMOR		(1U<<15)
#define HAS_SHOT		(1U<<4)
#define HAS_BALL		(1U<<3)


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
    unsigned int	spare	:2;	/* padding for alignment */
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


/*
 * Node within a Cell list.
 */
typedef struct _cell_node cell_node;
struct _cell_node {
    cell_node		*next;
    cell_node		*prev;
};


#define OBJECT_BASE	\
    short		id;		/* For shots => id of player */	\
    unsigned short	team;		/* Team of player or cannon */	\
    objposition		pos;		/* World coordinates */		\
    ipos		prevpos;	/* previous position */		\
    vector		vel;		/* speed in x,y */		\
    vector		acc;		/* acceleration in x,y */	\
    DFLOAT		mass;		/* mass in unigrams */		\
    long		life;		/* No of ticks left to live */	\
    long		status;		/* gravity, etc. */		\
    int			type;		/* one bit of OBJ_XXX */	\
    int			count;		/* Misc timings */		\
    modifiers		mods;		/* Modifiers to this object */	\
    u_byte		color;		/* Color of object */		\
    u_byte		missile_dir;	/* missile direction */	\
/* up to here all object types are the same as all player types. */

#define OBJECT_EXTEND	\
    cell_node		cell;		/* node in cell linked list */	\
    long		info;		/* Miscellaneous info */	\
    long		fuselife;	/* fuse duration ticks */	\
    int			pl_range;	/* distance for collision */	\
    int			pl_radius;	/* distance for hit */		\
/* up to here all object types are the same. */


/*
 * Generic object
 */
typedef struct _object object;
struct _object {

    OBJECT_BASE

    OBJECT_EXTEND

#ifdef __cplusplus
			_object() {}
#endif

#define OBJ_IND(ind)	(Obj[(ind)])
#define OBJ_PTR(ptr)	((object *)(ptr))
};


/*
 * Mine object
 */
typedef struct _mineobject mineobject;
struct _mineobject {

    OBJECT_BASE

    OBJECT_EXTEND

    int 		owner;		/* Who's object is this ? */
    DFLOAT		ecm_range;	/* Range from last ecm center */
    int			spread_left;	/* how much spread time left */

#ifdef __cplusplus
			_mineobject() {}
#endif

#define MINE_IND(ind)	((mineobject *)Obj[(ind)])
#define MINE_PTR(ptr)	((mineobject *)(ptr))
};


#define MISSILE_EXTEND		\
    DFLOAT		max_speed;	/* speed limitation */		\
    DFLOAT		turnspeed;	/* how fast to turn */
/* up to here all missiles types are the same. */

/*
 * Generic missile object
 */
typedef struct _missileobject missileobject;
struct _missileobject {

    OBJECT_BASE

    OBJECT_EXTEND

    MISSILE_EXTEND

#ifdef __cplusplus
			_missileobject() {}
#endif

#define MISSILE_IND(ind)	((missileobject *)Obj[(ind)])
#define MISSILE_PTR(ptr)	((missileobject *)(ptr))
};


/*
 * Smart missile is a generic missile with extras.
 */
typedef struct _smartobject smartobject;
struct _smartobject {

    OBJECT_BASE

    OBJECT_EXTEND

    MISSILE_EXTEND

    int			new_info;	/* smart re-lock id */
    DFLOAT		ecm_range;	/* Range from last ecm center */

#ifdef __cplusplus
			_smartobject() {}
#endif

#define SMART_IND(ind)	((smartobject *)Obj[(ind)])
#define SMART_PTR(ptr)	((smartobject *)(ptr))
};


/*
 * Torpedo is a generic missile with extras
 */
typedef struct _torpobject torpobject;
struct _torpobject {

    OBJECT_BASE

    OBJECT_EXTEND

    MISSILE_EXTEND

    int			spread_left;	/* how much spread time left */

#ifdef __cplusplus
			_torpobject() {}
#endif

#define TORP_IND(ind)	((torpobject *)Obj[(ind)])
#define TORP_PTR(ptr)	((torpobject *)(ptr))
};


/*
 * The ball object.
 */
typedef struct _ballobject ballobject;
struct _ballobject {

    OBJECT_BASE

    OBJECT_EXTEND

    int 		owner;		/* Who's object is this ? */
    int			treasure;	/* treasure for ball */	
    DFLOAT		length;		/* distance ball to player */

#ifdef __cplusplus
			_ballobject() {}
#endif

#define BALL_IND(ind)	((ballobject *)Obj[(ind)])
#define BALL_PTR(obj)	((ballobject *)(obj))
};


/*
 * Object with a wireframe representation.
 */
typedef struct _wireobject wireobject;
struct _wireobject {

    OBJECT_BASE

    OBJECT_EXTEND

    DFLOAT		turnspeed;	/* how fast to turn */

    u_byte		size;		/* Size of object (wreckage) */
    u_byte		rotation;	/* Rotation direction */

#ifdef __cplusplus
			_wireobject() {}
#endif

#define WIRE_IND(ind)	((wireobject *)Obj[(ind)])
#define WIRE_PTR(obj)	((wireobject *)(obj))
};


/*
 * Any object type should be part of this union.
 */
typedef union _anyobject anyobject;
union _anyobject {
    object		obj;
    ballobject		ball;
    mineobject		mine;
    missileobject	missile;
    smartobject		smart;
    torpobject		torp;
    wireobject		wireobj;
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
typedef struct {
    int		size;
    position	pos;
    int		id;
} ecm_t;

/*
 * Structure holding the info for one pulse of a laser.
 */
typedef struct {
    position		pos;
    int			dir;
    int			len;
    int			life;
    int			id;
    unsigned short	team;
    modifiers		mods;
    bool		refl;
} pulse_t;

/*
 * Transporter info.
 */
typedef struct {
    position	pos;
    int		target;
    int		id;
    int		count;
} trans_t;

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

    OBJECT_BASE

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
    DFLOAT	oldturnvel;		/* Last velocity of turn (right) */
    DFLOAT	turnacc;		/* Current acceleration of turn */
    long	score;			/* Current score of player */
    long	prev_score;		/* Last score that has been updated */
    int		prev_life;		/* Last life that has been updated */
    shipobj	*ship;			/* wire model of ship shape */
    DFLOAT	power;			/* Force of thrust */
    DFLOAT	power_s;		/* Saved power fiks */
    DFLOAT	turnspeed_s;		/* Saved turnspeed */
    DFLOAT	turnresistance_s;	/* Saved (see above) */
    DFLOAT	sensor_range;		/* Range of sensors (radar) */
    int		shots;			/* Number of active shots by player */
    int		missile_rack;		/* Next missile rack to be active */

    int		num_pulses;		/* Number of laser pulses in the air. */

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
					/* power, turnspeed and */
    DFLOAT	auto_turnspeed_s;	/* turnresistance settings. Restored */
    DFLOAT	auto_turnresistance_s;	/* when autopilot turned off */
    modifiers	modbank[NUM_MODBANKS];	/* useful modifier settings */
    bool	tractor_is_pressor;	/* on if tractor is pressor */
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

    u_byte	dir;			/* Direction of acceleration */
    u_byte	unused1;		/* padding for alignment */
    char	mychar;			/* Special char for player */
    char	prev_mychar;		/* Special char for player */
    char	name[MAX_CHARS];	/* Nick-name of player */
    char	realname[MAX_CHARS];	/* Real name of player */
    char	hostname[MAX_CHARS];	/* Hostname of client player uses */
    unsigned short	pseudo_team;	/* Which team for detaching tanks */
    ballobject	*ball;

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
    int		stunned;

    int		ecmcount;		/* number of active ecms */

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
    int		isoperator;		/* If player has operator privileges. */

#ifdef __cplusplus
		player() {}
#endif
};

#endif

/* $Id: object.h,v 3.38 1994/05/23 19:15:49 bert Exp $
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

#ifndef	OBJECT_H
#define	OBJECT_H

#include "const.h"
#include "types.h"
#include "bit.h"
#include "keys.h"
#include "rules.h"
#include "draw.h"


#define OBJ_PLAYER		(1UL<<0)	/* Types of objects */
#define OBJ_DEBRIS		(1UL<<1)
#define OBJ_SPARK		(1UL<<2)
#define OBJ_EMERGENCY_THRUST	(1UL<<3)	/* Was OBJ_NUKE */
#define OBJ_AUTOPILOT		(1UL<<4)	/* Was OBJ_CANNON_DEBRIS */
#define OBJ_TRACTOR_BEAM	(1UL<<5)	/* Was OBJ_CANNON_SHOT */
#define OBJ_LASER		(1UL<<6)
#define OBJ_BALL		(1UL<<7)
#define OBJ_SHOT		(1UL<<8)	/* Misc. objects */
#define OBJ_SMART_SHOT		(1UL<<9)
#define OBJ_ROCKET_PACK		(1UL<<10)
#define OBJ_CLOAKING_DEVICE	(1UL<<11)
#define OBJ_ENERGY_PACK		(1UL<<12)
#define OBJ_WIDEANGLE_SHOT	(1UL<<13)
#define OBJ_SHIELD		(1UL<<14)
#define OBJ_REFUEL		(1UL<<15)
#define OBJ_REPAIR		(1UL<<16)
#define OBJ_COMPASS		(1UL<<17)
#define OBJ_BACK_SHOT		(1UL<<18)
#define OBJ_MINE		(1UL<<19)
#define OBJ_MINE_PACK		(1UL<<20)
#define OBJ_SENSOR_PACK		(1UL<<21)
#define OBJ_TANK		(1UL<<22)
#define OBJ_ECM			(1UL<<23)
#define OBJ_TORPEDO		(1UL<<24)
#define OBJ_HEAT_SHOT		(1UL<<25)
#define OBJ_AFTERBURNER		(1UL<<26)
#define OBJ_CONNECTOR		(1UL<<27)
#define OBJ_TRANSPORTER         (1UL<<28)
#define OBJ_FIRE		(1UL<<29)
#define OBJ_PULSE		(1UL<<30)
#define OBJ_EMERGENCY_SHIELD	(1UL<<31)

#define OBJ_ALL_ITEMS	\
	OBJ_AFTERBURNER|\
	OBJ_AUTOPILOT|\
	OBJ_BACK_SHOT|\
	OBJ_CLOAKING_DEVICE|\
	OBJ_ECM|\
	OBJ_EMERGENCY_THRUST|\
	OBJ_ENERGY_PACK|\
	OBJ_LASER|\
	OBJ_MINE_PACK|\
	OBJ_ROCKET_PACK|\
	OBJ_SENSOR_PACK|\
	OBJ_TANK|\
	OBJ_TRACTOR_BEAM|\
	OBJ_TRANSPORTER|\
	OBJ_WIDEANGLE_SHOT|\
	OBJ_EMERGENCY_SHIELD

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
#define NUCLEAR			(1L<<0)
#define FULLNUCLEAR		(1L<<1)

#define MODS_WARHEAD_MAX	3	/* - C I CI */
#define CLUSTER			(1L<<0)
#define IMPLOSION		(1L<<1)

#define MODS_VELOCITY_MAX	3	/* - V1 V2 V3 */
#define MODS_MINI_MAX		3	/* - X2 X3 X4 */
#define MODS_SPREAD_MAX		3	/* - Z1 Z2 Z3 */
#define MODS_POWER_MAX		3	/* - B1 B2 B3 */

#define MODS_LASER_MAX		2	/* - LS LB */
#define STUN			(1L<<0)
#define BLIND			(1L<<1)

#define LOCK_NONE		0x00	/* No lock */
#define LOCK_PLAYER		0x01	/* Locked on player */
#define LOCK_VISIBLE		0x02	/* Lock information was on HUD */
					/* computed just before frame shown */
					/* and client input checked */
#define LOCKBANK_MAX		4	/* Maximum number of locks in bank */

#define NOT_CONNECTED		(-1)

typedef struct _object object;
struct _object {
    byte	color;			/* Color of object */		
    int		id;			/* For shots => id of player */
    position	prevpos;		/* Object's previous position... */
    position	pos;			/* World coordinates */
    vector	vel;
    vector	acc;
    int		dir;
    float	max_speed;
    float	mass;
    int		type;
    long	info;			/* Miscellaneous info */
    int		life;			/* No of ticks left to live */
    int		count;			/* Misc timings */
    long	status;
    modifiers	mods;			/* Modifiers to this object */

    float	turnspeed;		/* for missiles only */

    object	*cell_list;		/* linked list for cell lookup */

    int 	owner;			/* Who's object is this ? */
                                        /* (spare for id)*/
    int		treasure;		/* Which treasure does ball belong */
    int		new_info;
    float	length;			/* Distance between ball and player */
    float	ecm_range;		/* Range from last ecm center */
    int		spread_left;		/* how much spread time left */
    int		pl_range;		/* distance to player for collision. */
    int		pl_radius;		/* distance to player for hit. */
};


/*
 * Fuel structure, used by player
 */
typedef struct {
    long	sum;			/* Sum of fuel in all tanks */
    long	max;			/* How much fuel can you take? */
    int		current;		/* Number of currently used tank */
    int		num_tanks;		/* Number of tanks */
    long	tank[MAX_TANKS];
    long	l1;			/* Fuel critical level */
    long	l2;			/* Fuel warning level */
    long	l3;			/* Fuel notify level */
} pl_fuel_t;

struct _visibility {
    int		canSee;
    long	lastChange;
};

#define MAX_PLAYER_ECMS	16		/* Maximum simultaneous per player */
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

/* IMPORTANT
 *
 * This is the player structure, the first part MUST be similar to object_t,
 * this makes it possible to use the same basic operations on both of them
 * (mainly used in update.c).
 */
typedef struct player player;
struct player {
    byte	color;			/* Color of object */		
    int		id;			/* Unique id of object */
    position	prevpos;		/* Previous position... */
    position	pos;			/* World coordinates */
    vector	vel;			/* Velocity of object */
    vector	acc;			/* Acceleration constant */
    int		dir;			/* Direction of acceleration */
    float	max_speed;		/* Maximum speed of object */
    float	mass;			/* Mass of object (incl. cargo) */
    int		type;			/* Type of object */
    long	info;			/* Miscellaneous info */
    int		life;			/* Zero is dead. One is alive */
    int		count;			/* Miscellaneous timings */
    long	status;			/** Status, currently **/
    modifiers	mods;			/* Modifiers in effect */

    float	turnspeed;		/* How fast player acc-turns */
    float	velocity;		/* Absolute speed */

    long	used;			/** Items you use **/
    long	have;			/** Items you have **/

    int		shield_time;		/* Shields if no playerShielding */
    pl_fuel_t	fuel;			/* ship tanks and the stored fuel */
    float	emptymass;		/* Mass of empty ship */
    float	float_dir;		/* Direction, in float var */
    float	turnresistance;		/* How much is lost in % */
    float	turnvel;		/* Current velocity of turn (right) */
#ifdef TURN_FUEL
    float	oldturnvel;		/* Last velocity of turn (right) */
#endif
    float	turnacc;		/* Current acceleration of turn */
    long	mode;			/* Player mode, currently */
    long	score;			/* Current score of player */
    long	prev_score;		/* Last score that has been updated */
    int		prev_life;		/* Last life that has been updated */
    wireobj	*ship;			/* wire model of ship shape */
    float	power;			/* Force of thrust */
    float	power_s;		/* Saved power fiks */
    float	turnspeed_s;		/* Saved turnspeed */
    float	turnresistance_s;	/* Saved (see above) */
    float	sensor_range;		/* Range of sensors (radar) */
    int		shots;			/* Number of active shots by player */

    int		afterburners;		/* Number of afterburners */
    int		extra_shots;		/* Number of extra shots / 2 */
    int		back_shots;		/* Number of rear shots */
    int		mines;			/* Number of mines. */
    int		cloaks;			/* Number of cloaks. */
    int		sensors;		/* Number of sensors */
    int		missiles;		/* Number of missiles. */
    int		lasers;			/* Number of laser items. */
    int		num_pulses;		/* Number of laser pulses in the air. */
    int		max_pulses;		/* Max. number of laser pulses. */
    pulse_t	*pulses;		/* Info on laser pulses. */
    int		ecms;			/* Number of ecms. */
    int		transporters;		/* Number of transporters */
    int		autopilots;		/* Number of autopilots */
    int		emergency_thrusts;	/* Number of emergency thrusts */
    int		emergency_shields;	/* Number of emergency shields */
    int		tractor_beams;		/* Number of tractor beams */

    int		emergency_thrust_left;	/* how much emergency thrust left */
    int		emergency_thrust_max;	/* maximum time left */
    int		emergency_shield_left;	/* how much emergency shield left */
    int		emergency_shield_max;	/* maximum time left */
    float	auto_power_s;		/* autopilot saves of current */
    float	auto_turnacc_s;		/* power, turnacc, turnspeed and */
    float	auto_turnspeed_s;	/* turnresistance settings. Restored */
    float	auto_turnresistance_s;	/* when autopilot turned off */
    modifiers	modbank[NUM_MODBANKS];	/* useful modifier settings */
    int		tractor_pressor;	/* non-zero if tractor is pressor */
    player	*tractor;		/* target of tractor beam */
    int		shot_max;		/* Maximum number of shots active */
    int		shot_life;		/* Number of ticks shot will live */
    float	shot_speed;		/* Speed of shots fired by player */
    float	shot_mass;		/* Mass of shots fired by player */
    long	shot_time;		/* Time of last shot fired by player */
    int		repair_target;		/* Repairing this target */
    int		fs;			/* Connected to fuel station fs */
    int		check;			/* Next check point to pass */
    int		time;			/* The time a player has used */
    int		round;			/* Number of rounds player have done */
    int		best_lap;		/* Players best lap time */
    int		best_run;		/* Best race time */
    int		last_lap;		/* Time on last pass */
    int		last_lap_time;		/* What was your last pass? */
    int		last_time;		/* What was the time? */

    int		home_base;		/* Num of home base */
    struct {
	int	    tagged;		/* Flag, what is tagged? */
	int	    pl_id;		/* Tagging player id */
	position    pos;		/* Position of locked object */
	float	    distance;		/* Distance to object */
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
     * Robot variables
     */
    int		robot_mode;		/* For players->RM_NOT_ROBOT */
    long	robot_count;		/* Misc timings, minimizes rand()use */
    int		robot_ind;		/* Index in the robot array */
    int		robot_lock;
    int		robot_lock_id;




    struct _visibility *visibility;

    int updateVisibility, forceVisible, damaged;
    int wormDrawCount, wormHoleHit, wormHoleDest;

    ecm_info	ecmInfo;		/* list of active ecms */
    trans_info  transInfo;		/* list of active transporters */

    int	rplay_fd;			/* rplay UDP socket fd */

    int conn;				/* connection index, -1 if robot */
    unsigned version;			/* XPilot version number of client */

    BITV_DECL(last_keyv, NUM_KEYS);	/* Keyboard state */
    BITV_DECL(prev_keyv, NUM_KEYS);	/* Keyboard state */
    int key_changed;

    void *audio;			/* audio private data */
};

#endif

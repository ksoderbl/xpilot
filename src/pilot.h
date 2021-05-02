/* pilot.h,v 1.3 1992/05/11 15:31:27 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef PILOT_H
#define PILOT_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include "config.h"
#include "dbuff.h"
#include "default.h"
#include "bit.h"
#include <limits.h>

/*
 * DBL_MAX and RAND_MAX is ANSI C standard, but some architectures (BSD) use
 * MAXDOUBLE and INT_MAX instead.
 */
#ifndef DBL_MAX
#   include <values.h>
#   define  DBL_MAX	MAXDOUBLE
#   define  RAND_MAX	INT_MAX
#endif

#define PI		M_PI
#define MOD(x, y)	(((x)>=(y) || (x)<0) ? ((x)>=(y)?(x)-(y):(x)+(y)) :(x))
#define TABLE_SIZE	RESOLUTION
#define tsin(x)		(tbl_sin[MOD(x, TABLE_SIZE)])
#define tcos(x)		(tbl_sin[MOD((x)+TABLE_SIZE/4, TABLE_SIZE)])

extern long get_ind[];

#define TEAM(i, j)	(BIT(Players[i]->status, PAUSE) ||		\
				BIT(Players[j]->status, PAUSE) ?	\
			true : (BIT(World.rules->mode, TEAM_PLAY) ?	\
			((Players[i]->team == Players[j]->team) &&	\
			Players[i]->team!=0) : false))

#define ABS(x)			( (x)<0 ? -(x) : (x) )
#ifndef MAX
#define MIN(x, y)		( (x)>(y) ? (y) : (x) )
#define MAX(x, y)		( (x)>(y) ? (x) : (y) )
#endif
#define sqr(x)			( (x)*(x) )
#define LENGTH(x, y)		( sqrt(sqr(x) + sqr(y)) )
#define LIMIT(val, lo, hi)	val = val>hi ? hi : (val<lo ? lo : val)

#define SHUTDOWN_DELAY		700
#define CANNON_DEAD_TIME	900

#define MAX_PLAYERS		MAX_START_POS	    /* Miscellaneous limits */
#define MAX_MSGS		8
#define MAX_CHARS		80
#define MAX_SCROLL_LEN		4096
#define MAX_STATUS_CHARS	200
#define MIN_PAUSE		800
#define MAX_SHOTS		64
#define MAX_TOTAL_SHOTS		16384
#define MAX_STATION_FUEL	500.0
#define MAX_ID			2048		    /* Should suffice :) */
#define STATION_REGENERATION	0.06
#define MAX_PLAYER_FUEL		2600.0
#define MIN_PLAYER_FUEL		350.0
#define MAX_PLAYER_TURNSPEED	64.0
#define MIN_PLAYER_TURNSPEED	4.0
#define MAX_PLAYER_POWER	55.0
#define MIN_PLAYER_POWER	5.0
#define REFUEL_RATE		5.0
#define RACE_PLAYER_FUEL	500.0
#define DEFAULT_PLAYER_FUEL	1000.0
#define RESOLUTION		128
#define GRAVS_POWER		2.7
#define SMART_SHOT_ACC		0.6
#define SPEED_LIMIT		65.0
#define SMART_SHOT_MAX_SPEED	22.0
#define ENERGY_RANGE_FACTOR	2.5
#define SHOTS_ANGLE		(RESOLUTION/40)
#define WARNING_DISTANCE	800.0
#define VISIBILITY_DISTANCE	1000.0

#define WORLD_SPACE		35

#define OBJ_PLAYER		(1L<<0)		/* Types of objects */
#define OBJ_DEBRIS		(1L<<1)
#define OBJ_SPARK		(1L<<2)
#define OBJ_SHOT		(1L<<8)		/* Misc. objects */
#define OBJ_SMART_SHOT		(1L<<9)
#define OBJ_SMART_SHOT_PACK	(1L<<10)
#define OBJ_CLOAKING_DEVICE	(1L<<11)
#define OBJ_ENERGY_PACK		(1L<<12)
#define OBJ_WIDEANGLE_SHOT	(1L<<13)
#define OBJ_TRAINER		(1L<<14)
#define OBJ_SHIELD		(1L<<15)
#define OBJ_REFUEL		(1L<<16)
#define OBJ_COMPASS		(1L<<17)
#define OBJ_REAR_SHOT		(1L<<18)
#define OBJ_MINE		(1L<<19)
#define OBJ_MINE_PACK		(1L<<20)
#define OBJ_CANNON_DEBRIS	(1L<<30)	/* Cannon objects */
#define OBJ_CANNON_SHOT		(1L<<31)

#define SHIP_WIDTH		14  /* Size (pixels) of radius for legal HIT! */

#define PLAYING			(1L<<0)	    /* Possible player status bits */
#define WAITING_SHOTS		(1L<<1)
#define KILLED			(1L<<2)
#define THRUSTING		(1L<<3)
#define SELF_DESTRUCT		(1L<<4)
#define GRAVITY			(1L<<5)
#define SHOT_GRAVITY		(1L<<6)
#define ID_MODE			(1L<<8)
#define LOOSE_MASS		(1L<<9)
#define PAUSE			(1L<<10)
#define GAME_OVER		(1L<<11)
#define INACTIVE		(1L<<12)    /* Accept keyboard input etc... */
#define FUEL_GAUGE		(1L<<13)
#define VELOCITY_GAUGE		(1L<<14)
#define POWER_GAUGE		(1L<<15)
#define INVISIBLE		(1L<<16)
#define IN_USE			(1L<<31)    /* This player entry is in use. */

#define LOCK_NONE		1
#define LOCK_PLAYER		2

#define MSG_LEN			256



/* * * * * *
 * Types that are used by the main / startup-program.
*/

typedef struct { float x, y; }	vector;
typedef vector			position;
typedef struct { int x, y; }	ipos;

typedef struct {			/* Defines wire-obj, i.e. ship */
    position *pts;
    int ant_points;
} wireobj;

typedef struct {
    int		color;			/* Color of object */		
    int		id;			/* For shots => id of player */
    position	pos;			/* World coordinates */
    vector	vel;
    vector	acc;
    int		dir;
    double	max_speed;
    double	velocity;
    double	turnspeed;
    double	mass;
    int		type;
    long	info;			/* Miscellaneous info */
    int		life;			/* No of ticks left to live */
    int		count;			/* Misc timings */
    long	status;
} object;

typedef struct {
    int		color;			/* Color of object */		
    int		id;			/* Unique id of object */
    position	pos;			/* World coordinates */
    vector	vel;			/* Velocity of object */
    vector	acc;			/* Acceleration constant */
    int		dir;			/* Direction of acceleration */
    double	max_speed;		/* Maximum speed of object */
    double	velocity;		/* Absolute speed */
    double	turnspeed;		/* How fast player acc-turns */
    double	mass;			/* Mass of object (incl. cargo) */
    int		type;			/* Type of object */
    long	info;			/* Miscellaneous info */
    int		life;			/* Zero is dead. One is alive */
    int		count;			/* Miscellaneous timings */
    long	status;			/** Status, currently **/

    long	used;			/** Items you use **/
    long	have;			/** Items you have **/

    long	control_count;		/* Display control for how long? */
    long	fuel_count;		/* Display fuel for how long? */
    double	fuel;			/* How long can you stay in the air? */
    double	max_fuel;		/* How much fuel can you take */
    double	emptymass;		/* Mass of empty ship */
    double	double_dir;		/* Direction, in double var */
    double	turnresistance;		/* How much is lost in % */
    double	turnvel;		/* Current velocity of turn (right) */
    double	turnacc;		/* Current acceleration of turn */
    long	mode;			/* Player mode, currently */
    long	instruments;		/* Instruments on screen (bitmask) */
    long	score;			/* Current score of player */
    double	power;			/* Force of thrust */
    double	power_s;		/* Saved power fiks */
    double	turnspeed_s;		/* Saved turnspeed */
    double	turnresistance_s;	/* Saved (see above) */
    double	sensor_range;		/* Range of sensors (radar) */
    int		shots;			/* Number of active shots by player */
    int		extra_shots;		/* Number of extra shots / 2 */
    int		mines;			/* Number of mines. */
    int		missiles;		/* Number of missiles. */
    int		shot_max;		/* Maximum number of shots active */
    int		shot_life;		/* Number of ticks shot will live */
    double	shot_speed;		/* Speed of shots fired by player */
    double	shot_mass;		/* Mass of shots fired by player */
    int		fs;			/* Connected to fuel station fs */
    int		check;			/* Next check point to pass */
    int		time;			/* The time aa player has used */
    int		round;			/* Number of rounds player have done */
    int		best_lap;		/* Players best lap time */
    int		best_run;		/* Best race time */
    int		last_lap;		/* Time on last pass */
    int		last_lap_time;		/* What was your last pass? */
    int		last_time;		/* What was the time? */
    position	world;			/* Lower left hand corner is this */
					/* world coordinate */

    int		home_base;		/* No of home base */
    struct {
	int	    tagged;		/* Flag, what is tagged? */
	int	    pl_id;		/* Tagging player id */
	position    pos;		/* Position of locked object */
	double	    distance;		/* Distance to object */
	char	    *name;		/* Name of locked object */
    } lock;

    char	mychar;			/* Special char for player */
    char	lblstr[MAX_CHARS];	/* Label string for player */
    char	name[MAX_CHARS];	/* Nick-name of player */
    char	realname[MAX_CHARS];	/* Real name of player */
    char	display[MAX_CHARS];	/* Display of player (string) */
    Display	*disp;			/* Display of player (pointer) */
    bool	info_press;		/* Player pressed the info button? */ 
    bool	help_press;		/* Player pressed the help button? */
    int		help_page;		/* Which page is the player on? */
    int		team;			/* What team is the player on? */

    /* Miscellaneous graphic variables */
    int		name_length;		/* Length of name in pixels */
    GC		gc;
    GC		gcr;
    GC		gcb;
    GC		gcp;
    GC		gctxt;
    Window	top;
    Window	draw;
    Window	radar;
    long	dpl_1[2], dpl_2[2];
    Window	quit;
    Window	help_b;
    Window	info_b;
    Window	help_w;
    Window	info_w;
    Window	info_close;
    Window	help_close;
    Window	help_next;
    Window	help_prev;
    Window	players;
    XColor	colors[4];

    dbuff_state_t   *dbuf_state;

#ifdef SCROLL
    char	scroll[MAX_SCROLL_LEN]; /* Scrolling string */
    int		scroll_len;		/* Scrolling string length */
#endif

    def_t	def;
} player;

typedef struct {
    int max_ant;
    char name[80];
    char host[80];
} server;

#endif

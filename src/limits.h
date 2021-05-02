/* limits.h,v 1.4 1992/06/28 05:38:16 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef LIMITS_H
#define	LIMITS_H

#include <limits.h>

/*
 * DBL_MAX and RAND_MAX is ANSI C standard, but some architectures (BSD) use
 * MAXDOUBLE and INT_MAX instead.
 */
#ifndef	DBL_MAX
#   include <values.h>
#   define  DBL_MAX	MAXDOUBLE
#endif
#ifndef	RAND_MAX
#   define  RAND_MAX	INT_MAX
#endif

/* Not everyone has PI (or M_PI defined). */
#ifndef	M_PI
#define PI		3.14159265358979323846
#else
#define	PI		M_PI
#endif

#define MOD(x, y)	(((x)>=(y) || (x)<0) ? ((x)>=(y)?(x)-(y):(x)+(y)) :(x))
#define TABLE_SIZE	RES
#define tsin(x)		(tbl_sin[MOD(x, TABLE_SIZE)])
#define tcos(x)		(tbl_sin[MOD((x)+TABLE_SIZE/4, TABLE_SIZE)])

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

#define CANNON_DEAD_TIME	900
#define	RECOVERY_DELAY		128

#define MAX_BASES		64
#define MAX_PLAYERS		MAX_BASES	    /* Miscellaneous limits */
#define MAX_MSGS		8
#define MAX_CHARS		80
#define	MAX_KEY_DEFS		100
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
#define RES		128
#define GRAVS_POWER		2.7
#define SMART_SHOT_ACC		0.6
#define SPEED_LIMIT		65.0
#define SMART_SHOT_MAX_SPEED	22.0
#define ENERGY_RANGE_FACTOR	2.5
#define SHOTS_ANGLE		(RES/40)
#define WARNING_DISTANCE	800.0
#define VISIBILITY_DISTANCE	1000.0
#define MAX_KEY_DEFS		100

#define BLOCK_SZ		35

#define SHIP_SZ		14  /* Size (pixels) of radius for legal HIT! */

#define MSG_LEN			256

#endif

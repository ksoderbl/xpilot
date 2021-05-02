/* $Id: const.h,v 3.10 1993/08/02 12:54:56 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bj�rn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert G�sbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef LIMITS_H
#define	LIMITS_H

#include <limits.h>
#include <math.h>

/*
 * FLT_MAX and RAND_MAX is ANSI C standard, but some systems (BSD) use
 * MAXFLOAT and INT_MAX instead.
 */
#ifndef	FLT_MAX
#   if defined(__sgi)
#       include <float.h>	/* FLT_MAX for SGI Personal Iris */
#   else
#	if defined(__sun__)
#           include <values.h>	/* MAXFLOAT for suns */
#	endif
#	define  FLT_MAX	MAXFLOAT
#   endif
#endif
#ifndef	RAND_MAX
#   define  RAND_MAX	INT_MAX
#endif

/* Not everyone has PI (or M_PI defined). */
#ifndef	M_PI
#   define PI		3.14159265358979323846
#else
#   define	PI		M_PI
#endif

/* Not everyone has LINE_MAX either, *sigh* */
#ifndef LINE_MAX
#   define LINE_MAX 2048
#endif

#define RES		        128

#define BLOCK_SZ		35

#define TABLE_SIZE	RES
#define tsin(x)		(tbl_sin[MOD2(x, TABLE_SIZE)])
#define tcos(x)		(tbl_sin[MOD2((x)+TABLE_SIZE/4, TABLE_SIZE)])

#define NELEM(a)	(sizeof(a) / sizeof((a)[0]))

#define ABS(x)			( (x)<0 ? -(x) : (x) )
#ifndef MAX
#   define MIN(x, y)		( (x)>(y) ? (y) : (x) )
#   define MAX(x, y)		( (x)>(y) ? (x) : (y) )
#endif
#define sqr(x)			( (x)*(x) )
#define LENGTH(x, y)		( hypot( (double) (x), (double) (y) ) )
#define LIMIT(val, lo, hi)	( val = val>hi ? hi : (val<lo ? lo : val) )

/*
 * Two macros for edge wrap of differences in position.
 * If the absolute value of a difference is bigger than
 * half the map size then it is wrapped.
 */
#define WRAP_DX(dx)	\
	(BIT(World.rules->mode, WRAP_PLAY) \
	    ? ((dx) < - (World.x * BLOCK_SZ >> 1) \
		? (dx) + World.x * BLOCK_SZ \
		: ((dx) > (World.x * BLOCK_SZ >> 1) \
		    ? (dx) - World.x * BLOCK_SZ \
		    : (dx))) \
	    : (dx))

#define WRAP_DY(dy)	\
	(BIT(World.rules->mode, WRAP_PLAY) \
	    ? ((dy) < - (World.y * BLOCK_SZ >> 1) \
		? (dy) + World.y * BLOCK_SZ \
		: ((dy) > (World.y * BLOCK_SZ >> 1) \
		    ? (dy) - World.y * BLOCK_SZ \
		    : (dy))) \
	    : (dy))

#ifndef MOD2
#  define MOD2(x, m)		( (x) & ((m) - 1) )
#endif	/* MOD2 */

#define PSEUDO_TEAM(i,j)\
	(Players[(i)]->pseudo_team == Players[(j)]->pseudo_team)
#define TEAM(i, j)		(					\
	BIT(Players[i]->status, PAUSE) || BIT(Players[j]->status, PAUSE)\
	? true								\
	: (BIT(World.rules->mode, TEAM_PLAY)				\
	   ? (Players[i]->team != TEAM_NOT_SET				\
	      && Players[i]->team == Players[j]->team)			\
	   : false))

#define CANNON_DEAD_TIME	900

#define	RECOVERY_DELAY		(FPS*3)

#define MAX_ID			4096		    /* Should suffice :) */
#define MAX_PSEUDO_PLAYERS      16

#define PLAYER_ITEM_RATE        5

#define MAX_MSGS		8
#define MAX_SCROLL_LEN		4096
#define MAX_CHARS		80
#define MSG_LEN			256

#define FONT_LEN		256

#define MAX_STATUS_CHARS	200

#define MIN_PAUSE		800

#define MAX_TOTAL_SHOTS		16384
#define MAX_SHOTS		64
#define SHOTS_ANGLE		(RES/40)

#define SPEED_LIMIT		65.0
#define MAX_PLAYER_TURNSPEED	64.0
#define MIN_PLAYER_TURNSPEED	4.0
#define MAX_PLAYER_POWER	55.0
#define MIN_PLAYER_POWER	5.0

#define FUEL_SCALE_BITS         8
#define FUEL_SCALE_FACT         (1<<FUEL_SCALE_BITS)
#define FUEL_MASS(f)            (f*0.005/FUEL_SCALE_FACT)
#define MAX_STATION_FUEL	(500<<FUEL_SCALE_BITS)
#define START_STATION_FUEL	(20<<FUEL_SCALE_BITS)
#define STATION_REGENERATION	(0.06*FUEL_SCALE_FACT)
#define MAX_PLAYER_FUEL		(2600<<FUEL_SCALE_BITS)
#define MIN_PLAYER_FUEL		(350<<FUEL_SCALE_BITS)
#define REFUEL_RATE		(5<<FUEL_SCALE_BITS)
#define ENERGY_PACK_FUEL        ((500+(rand()&511))<<FUEL_SCALE_BITS)
#define RACE_PLAYER_FUEL	(500<<FUEL_SCALE_BITS)
#define DEFAULT_PLAYER_FUEL	(1000<<FUEL_SCALE_BITS)
#define FUEL_NOTIFY             (16*FPS)

#define TARGET_DEAD_TIME	(FPS * 60)
#define TARGET_DAMAGE		(250<<FUEL_SCALE_BITS)
#define TARGET_REPAIR_PER_FRAME	(TARGET_DAMAGE / (FPS * 60 * 10))
#define TARGET_UPDATE_DELAY	(TARGET_DAMAGE / (TARGET_REPAIR_PER_FRAME \
				    * BLOCK_SZ))

#define LG2_MAX_AFTERBURNER    4
#define ALT_SPARK_MASS_FACT     4.2
#define ALT_FUEL_FACT           3
#define MAX_AFTERBURNER        ((1<<LG2_MAX_AFTERBURNER)-1)
#define AFTER_BURN_SPARKS(s,n)  (((s)*(n))>>LG2_MAX_AFTERBURNER)
#define AFTER_BURN_POWER(p,n)   \
 ((p)*(1.0+(n)*((ALT_SPARK_MASS_FACT-1.0)/(MAX_AFTERBURNER+1.0))))
#define AFTER_BURN_FUEL(f,n)    \
 (((f)*((MAX_AFTERBURNER+1)+(n)*(ALT_FUEL_FACT-1)))/(MAX_AFTERBURNER+1.0))

#ifdef	TURN_THRUST
#  define TURN_FUEL(acc)          (0.005*FUEL_SCALE_FACT*ABS(acc))
#  define TURN_SPARKS(tf)         (5+((tf)>>((FUEL_SCALE_BITS)-6)))
#endif

#define THRUST_MASS             0.7

#define MAX_TANKS               8
#define TANK_MASS               (ShipMass/10)
#define TANK_CAP(n)             (!(n)?MAX_PLAYER_FUEL:(MAX_PLAYER_FUEL/3))
#define TANK_FUEL(n)            ((TANK_CAP(n)*(5+(rand()&3)))/32)
#define TANK_REFILL_LIMIT       (MIN_PLAYER_FUEL/8)
#define TANK_THRUST_FACT        0.7
#define TANK_NOTHRUST_TIME      (HEAT_CLOSE_TIMEOUT/2+2)
#define TANK_THRUST_TIME        (TANK_NOTHRUST_TIME/2+1)

#define GRAVS_POWER		2.7

#define SHIP_SZ		        14  /* Size (pixels) of radius for legal HIT! */
#define VISIBILITY_DISTANCE	1000.0
#define WARNING_DISTANCE	(VISIBILITY_DISTANCE*0.8)

#define ECM_DISTANCE		(VISIBILITY_DISTANCE*0.2)
#define ECM_MIS_FACT		1.5

#define TRANSPORTER_DISTANCE	ECM_DISTANCE

#define MINE_RANGE              (VISIBILITY_DISTANCE*0.1)
#define MINE_MASS               30.0
#define MINE_LIFETIME           (5000+(rand()&255))
#define MINE_SPEED_FACT         1.3

#define MISSILE_LIFETIME        (rand()%(64*FPS-1)+(128*FPS))
#define MISSILE_MASS            5.0
#define MISSILE_RANGE           4
#define SMART_SHOT_ACC		0.6
#define SMART_SHOT_DECFACT	3
#define SMART_SHOT_MIN_SPEED	(SMART_SHOT_ACC*8)
#define SMART_TURNSPEED         2.6
#define SMART_SHOT_MAX_SPEED	22.0
#define SMART_SHOT_LOOK_AH      4
#define TORPEDO_SPEED_TIME      (2*FPS)
#define TORPEDO_ACC             (SMART_SHOT_MAX_SPEED/TORPEDO_SPEED_TIME)
#define TORPEDO_RANGE           (MINE_RANGE*0.45)

#define NUKE_SPEED_TIME		(2*FPS)
#define NUKE_ACC		(5*SMART_SHOT_MAX_SPEED/TORPEDO_SPEED_TIME)
#define NUKE_RANGE		(MINE_RANGE*1.5)
#define NUKE_MIN_SMART		7
#define NUKE_MASS_MULT		1
#define NUKE_EXPLOSION_MULT	20

#define HEAT_RANGE              (VISIBILITY_DISTANCE/2)
#define HEAT_SPEED_FACT         1.7
#define HEAT_CLOSE_TIMEOUT      (2*FPS)
#define HEAT_CLOSE_RANGE        HEAT_RANGE
#define HEAT_CLOSE_ERROR        0
#define HEAT_MID_TIMEOUT        (4*FPS)
#define HEAT_MID_RANGE          (2*HEAT_RANGE)
#define HEAT_MID_ERROR          8
#define HEAT_WIDE_TIMEOUT       (8*FPS)
#define HEAT_WIDE_ERROR         16

#define WALL_RETURN_TIME        32
#define WARN_TIME               2

#define BALL_STRING_LENGTH	120

#define TEAM_NOT_SET		0xffff

#define DEBRIS_MASS             4.5
#define DEBRIS_SPEED(intensity) ((rand()%(1+(intensity>>2)))|20)
#define DEBRIS_LIFE(intensity)  ((rand()%(1+intensity>>1))|8)
#define DEBRIS_TYPES		(NUM_COLORS * 3 * 3)

#define PL_DEBRIS_MASS          3.5
#define PL_DEBRIS_SPEED(mass)   DEBRIS_SPEED(((int)mass)<<1)
#define PL_DEBRIS_LIFE(mass)    (4+(rand()%(int)(1+mass*1.5)))

#define ENERGY_RANGE_FACTOR	(2.5/FUEL_SCALE_FACT)

#define WORM_BRAKE_FACTOR	1
#define WORMCOUNT		64

#define ROB_LOOK_AH		2

#ifdef __GNUC__
#define	INLINE	inline
#else
#define INLINE
#endif /* __GNUC__ */

#if defined(__sun__)
#  define srand(s)	srandom(s)
#  define rand()	random()
#endif /* __sun__ */

#if defined(ultrix) || defined(AIX)
/* STDRUP_OBJ should be uncomented in Makefile also */
extern char* strdup(const char*);
#endif

#endif

/* $Id: object.h,v 1.11 1992/08/27 00:26:01 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	OBJECT_H
#define	OBJECT_H

#include "limits.h"
#include "types.h"
#include "dbuff.h"


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
#define OBJ_SENSOR_PACK 	(1L<<21)
#define OBJ_TANK		(1L<<22)
#define OBJ_ECM 		(1L<<23)
#define OBJ_DUST                (1L<<24)
#define OBJ_TORPEDO             (1L<<25)
#define OBJ_HEAT_SHOT           (1L<<26)
#define OBJ_AFTER_BURNER        (1L<<27)
#define OBJ_CANNON_DEBRIS	(1L<<30)	/* Cannon objects */
#define OBJ_CANNON_SHOT		(1L<<31)

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
#define WARPING			(1L<<16)
#define WARPED			(1L<<17)
#define CONFUSED 		(1L<<18)

#define LOCK_NONE		1
#define LOCK_PLAYER		2

typedef struct {			/* Defines wire-obj, i.e. ship */
    position	*pts;
    int		num_points;
} wireobj;

typedef struct {
    byte	color;			/* Color of object */		
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
    int		new_info;
} object;


/*
 * The following enum type defines the possible actions as a result of
 * a keypress.
 */
typedef enum {
    KEY_DUMMY,
    KEY_LOCK_NEXT,
    KEY_LOCK_PREV,
    KEY_LOCK_CLOSE,
    KEY_CHANGE_HOME,
    KEY_SHIELD,
    KEY_FIRE_SHOT,
    KEY_FIRE_MISSILE,
    KEY_FIRE_TORPEDO,
    KEY_FIRE_HEAT,
    KEY_DROP_MINE,
    KEY_DETACH_MINE,
    KEY_TURN_LEFT,
    KEY_TURN_RIGHT,
    KEY_SELF_DESTRUCT,
    KEY_ID_MODE,
    KEY_PAUSE,
    KEY_TANK_DETACH,
    KEY_TANK_NEXT,
    KEY_TANK_PREV,
    KEY_TOGGLE_VELOCITY,
    KEY_TOGGLE_COMPASS,
    KEY_SWAP_SETTINGS,
    KEY_REFUEL,
    KEY_INCREASE_POWER,
    KEY_DECREASE_POWER,
    KEY_INCREASE_TURNSPEED,
    KEY_DECREASE_TURNSPEED,
    KEY_THRUST,
    KEY_CLOAK,
    KEY_SLOWDOWN,
    KEY_SPEEDUP,
    KEY_ECM
} keys_t;



/*
 * This is the player structure, the first part is similar to object_t, this
 * makes it possible to use the same basic operations on both of them (mainly
 * used in update.c).
 */
    typedef struct {
        long    sum;                    /* sum of fuel in all tanks */
        long    max;                    /* how much fuel can you take? */
        int     current;                /* number of currently used tank */
        int     no_tanks;               /* number of tanks */
        long    tank[MAX_TANKS];
        long	count;			/* Display fuel for how long? */
        long	l1;			/* Fuel critical level */
        long	l2;			/* Fuel warning level */
        long	l3;			/* Fuel notify level */
    } pl_fuel_t;
typedef struct {
    byte	color;			/* Color of object */		
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
    pl_fuel_t   fuel;                   /* ship tanks and the stored fuel */
    int         after_burners;          /* number of after burners, powerfull*/
                                        /* and efficient engine              */
    double	emptymass;		/* Mass of empty ship */
    double	double_dir;		/* Direction, in double var */
    double	turnresistance;		/* How much is lost in % */
    double	turnvel;		/* Current velocity of turn (right) */
#ifdef TURN_FUEL
    double	oldturnvel;		/* Last velocity of turn (right) */
#endif
    double	turnacc;		/* Current acceleration of turn */
    long	mode;			/* Player mode, currently */
    long	instruments;		/* Instruments on screen (bitmask) */
#define SHOW_HUD_INSTRUMENTS	(1<<0)		    
#define SHOW_HUD_VERTICAL	(1<<1)
#define SHOW_HUD_HORIZONTAL	(1<<2)
#define SHOW_FUEL_METER		(1<<3)
#define SHOW_FUEL_GAUGE		(1<<4)
#define SHOW_TURNSPEED_METER	(1<<5)
#define SHOW_POWER_METER	(1<<6)
    long	score;			/* Current score of player */
    double	power;			/* Force of thrust */
    double	power_s;		/* Saved power fiks */
    double	turnspeed_s;		/* Saved turnspeed */
    double      hud_move_fact;          /* scale the hud-movement (speed) */
    double      ptr_move_fact;          /* scale thes peed pointer length */
    double	turnresistance_s;	/* Saved (see above) */
    double	sensor_range;		/* Range of sensors (radar) */
    int		shots;			/* Number of active shots by player */
    int		extra_shots;		/* Number of extra shots / 2 */
    int		rear_shots;		/* Number of rear shots */
    int		mines;			/* Number of mines. */
    int		cloaks;			/* Number of cloaks. */
    int		sensors;		/* Number of sensors */
    int		missiles;		/* Number of missiles. */
    int		ecms;                   /* Number of ecms. */
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
    } lock;

    char	mychar;			/* Special char for player */
    char	lblstr[MAX_CHARS];	/* Label string for player */
    char	name[MAX_CHARS];	/* Nick-name of player */
    char	realname[MAX_CHARS];	/* Real name of player */
    Display	*disp;			/* Display of player (pointer) */
    int         map_point_distance;     /* distance of points in the background? */
    bool	info_press;		/* Player pressed the info button? */ 
    bool	help_press;		/* Player pressed the help button? */
    int		help_page;		/* Which page is the player on? */
    u_byte	team;			/* What team is the player on? */
    unsigned int pseudo_team;           /* which team is used for my tanks (detaching!) */

    /*
     * Robot variables
     */
    u_byte	robot_mode;		/* For players -> RM_NOT_ROBOT */
    long	robot_count;		/* Misc timings, minimizes rand() use */
    int		robot_ind;		/* Index in the robot array */
    int		robot_lock;
    int		robot_lock_id;
     
    /* Miscellaneous graphic variables */
    u_byte	disp_type;		/* Display type */
#define	DT_NONE			0
#define	DT_HAVE_COLOR	       	1
#define	DT_HAVE_PLANES		2
#define	DT_IS_DISPLAY		4
    int		name_length;		/* Length of name in pixels */
    GC		gc;			/* GC for the drawing area */
    GC		gcr;			/* GC for the radar */
    GC		gcb;			/* GC for the buttons */
    GC		gcp;			/* GC for the player list */
    GC		gctxt;			/* GC for the info/help text */
    Window	top;			/* Top-level window (topshell) */
    Window	draw;			/* Main play window */
    Pixmap	p_draw;			/* Saved pixmap for the drawing */
    					/* area (monochromes use this) */
    Window	radar;			/* Radar window */
    Window	players;		/* Player list window */
    Pixmap	p_radar, s_radar;	/* Pixmaps for the radar (implements */
					/* the planes hack on the radar for */
    					/* monochromes) */
    long	dpl_1[2], dpl_2[2];	/* Used by radar hack */
    Window	quit_b;			/* Quit button */
    Window	help_b;			/* Help window button */
    Window	info_b;			/* Info button */
    Window	help_w;			/* Help window */
    Window	info_w;			/* Info window */
    Window	info_close_b;		/* Info window's close button */
    Window	help_close_b;		/* Help window's close button */
    Window	help_next_b;		/* Help window's next button */
    Window	help_prev_b;		/* Help window's prevsious button */
    XColor	colors[4];		/* Colors */
    Colormap	colormap;		/* Private colormap */

    dbuff_state_t   *dbuf_state;        /* Holds current dbuff state */

#ifdef SCROLL
    char	scroll[MAX_SCROLL_LEN]; /* Scrolling string */
    int		scroll_len;		/* Scrolling string length */
#endif

    struct {
	int		canSee;
	unsigned long	lastChange;
    } visibility[MAX_PLAYERS];

    int updateVisibility, forceVisible, damaged;
    int wormDrawCount, wormHoleHit;

    struct
    {
	int size;
	position pos;
    } ecmInfo;

    struct _keyDefs {
        KeySym	keysym;			/* Keysym-to-action array */
        keys_t	key;
    } keyDefs[MAX_KEY_DEFS];
} player;

#endif

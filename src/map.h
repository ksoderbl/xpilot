/* $Id: map.h,v 1.9 1992/08/27 00:25:59 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	MAP_H
#define	MAP_H

#include "types.h"
#include "rules.h"
#include "limits.h"

/*
 * Default map name, set to your own favourite -
 * or set it to "N/A", in which case the computer
 * will generate a random map. (Not very good)
 */
#define DEFAULT_MAP		"continent"

#define MAX_CHECKS		10
#define CRASH_BLASTERS          32

#define SPACE			0
#define BASE			1
#define FILLED			2
#define FILLED_NO_DRAW		3
#define REC_LU			4
#define REC_LD			5
#define REC_RU			6
#define REC_RD			7
#define FUEL			8
#define CANNON			9
#define CHECK			10
#define SPECIAL			11
#define POS_GRAV		20
#define NEG_GRAV		21
#define CWISE_GRAV		22
#define ACWISE_GRAV		23
#define WORMHOLE		24
#define BLASTER                 25

#ifndef MAP_DIR
#define MAP_DIR			(LIBDIR "maps/")
#endif

#define DIR_RIGHT		0
#define DIR_UP			(RES/4)
#define DIR_LEFT		(RES/2)
#define DIR_DOWN		(3*RES/4)

#define ITEM_SMART_SHOT_PACK	0
#define ITEM_CLOAKING_DEVICE	1
#define ITEM_ENERGY_PACK	2
#define ITEM_WIDEANGLE_SHOT	3
#define ITEM_REAR_SHOT		4
#define ITEM_MINE_PACK		5
#define ITEM_SENSOR_PACK	6
#define ITEM_TANK		7
#define ITEM_ECM		8
#define ITEM_AFTER_BURNER       9
#define NUM_ITEMS               10

typedef struct {
    position	pos;
    long	fuel;
} fuel_t; 

typedef struct {
    ipos	pos;
    double	force;
} grav_t;

typedef struct {
    ipos	pos;
    u_byte      orig;
    int         fuel;
    int         chance;
    int         add_size;
    int         base_size;
    int         life;
    int         speed_fact;
    int         runtime;
} blaster_t;

typedef struct {
    ipos	pos;
    int		dir;
    int		dead_time;
    bool	active;
} cannon_t;

typedef struct {
    int max;		/* Max on world at a given time */
    int num;		/* Number active right now */
    int chance;		/* Chance for the item to appear on this world */
    int ref_chance;     /* given by default or map */
} item_t;

typedef enum { WORM_NORMAL, WORM_IN, WORM_OUT } wormType;

typedef struct {
    ipos pos;
    int lastdest, countdown;
    wormType type;
} wormhole_t;

typedef struct {
    int		x, y;				    /* Size of world */
    rules_t	*rules;
    char	name[MAX_CHARS];
    char	author[MAX_CHARS];

    u_byte	**block;
    vector	**gravity;

    item_t	items[NUM_ITEMS];

    int		NumBases;
    ipos	base[MAX_BASES];
    int         NumBlasters;
    blaster_t   *blaster;
    int		NumFuels;
    fuel_t	*fuel;
    int		NumGravs;
    grav_t	*grav;
    int		NumCannons;
    cannon_t	*cannon;
    int		NumChecks;
    ipos	check[MAX_CHECKS];
    int		NumWormholes;
    wormhole_t	*wormHoles;
} World_map;

#endif

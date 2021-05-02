/* map.h,v 1.3 1992/05/11 15:31:20 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include "rules.h"
#include "pilot.h"

/*
 * Default map name, set to your own favourite -
 * or set it to "N/A", in which case the computer
 * will generate a random map. (Not very good)
*/
#define DEFAULT_MAP		"continent"

#define MAX_START_POS		64
#define MAX_FUEL_STATIONS	128
#define MAX_GRAVITY_SENTERS	512
#define MAX_CANNONS		512
#define MAX_CHECK		10

#define SPACE			0
#define PORT			1
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

#ifndef MAP_DIR
#define MAP_DIR			(LIBDIR "maps/")
#endif

#define RIGHT			0
#define UP			(RESOLUTION/4)
#define LEFT			(RESOLUTION/2)
#define DOWN			(3*RESOLUTION/4)

#define ITEM_SMART_SHOT_PACK	0
#define ITEM_CLOAKING_DEVICE	1
#define ITEM_ENERGY_PACK	2
#define ITEM_WIDEANGLE_SHOT	3
#define ITEM_REAR_SHOT		4
#define ITEM_MINE_PACK		5
#define NO_OF_ITEMS		6
#define ITEM_SHOTS
#define ITEM_TRAINER
#define ITEM_SHIELD
#define ITEM_AUTOFIRE

typedef struct {
    position pos;
    double left;
} fuel_station; 

typedef struct {
    ipos pos;
    double force;
} grav;

typedef struct {
    ipos pos;
    int dir;
    int dead_time;
    bool active;
} cannontype;

typedef struct {
    int max;		    /* Max on world at a given time */
    int ant;		    /* Number active right now */
    int chance;		    /* Chance for the item to appear on this world */
} items_t;


typedef struct {
    int x, y;				    /* Size of world */
    rules_t *rules;
    char name[MAX_CHARS];
    char author[MAX_CHARS];

    unsigned char **type;
    vector **gravity;

    items_t items[NO_OF_ITEMS];

    int Ant_start;
    ipos Start_points[MAX_START_POS];
    int Ant_fuel;
    fuel_station fuel[MAX_FUEL_STATIONS];
    int Ant_gravs;
    grav gravs[MAX_GRAVITY_SENTERS];
    int Ant_cannon;
    cannontype cannon[MAX_CANNONS];
    int Ant_check;
    ipos check[MAX_CHECK];
} World_map;

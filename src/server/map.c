/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WINDOWS
# include <sys/file.h>
#endif

#define SERVER
#include "xpconfig.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "bit.h"
#include "error.h"
#include "commonproto.h"

#define GRAV_RANGE  10



/*
 * Globals.
 */
World_map World;


static void Init_map(void);
static void Alloc_map(void);
static void Generate_random_map(void);

static void Find_base_order(void);


#ifdef DEBUG
static void Print_map(void)			/* Debugging only. */
{
    int x, y;

    for (y=World.y-1; y>=0; y--) {
	for (x=0; x<World.x; x++)
	    switch (World.block[x][y]) {
	    case SPACE:
		putchar(' ');
		break;
	    case BASE:
		putchar('_');
		break;
	    default:
		putchar('X');
		break;
	    }
	putchar('\n');
    }
}
#endif


static void Init_map(void)
{
    World.x		= 256;
    World.y		= 256;
    World.diagonal	= (int) LENGTH(World.x, World.y);
    World.width		= World.x * BLOCK_SZ;
    World.height	= World.y * BLOCK_SZ;
    World.hypotenuse	= (int) LENGTH(World.width, World.height);
    World.NumFuels	= 0;
    World.NumBases	= 0;
    World.NumGravs	= 0;
    World.NumCannons	= 0;
    World.NumWormholes	= 0;
    World.NumTreasures	= 0;
    World.NumTargets	= 0;
    World.NumItemConcentrators	= 0;
    World.NumAsteroidConcs	= 0;
}


void Free_map(void)
{
    if (World.block) {
	free(World.block);
	World.block = NULL;
    }
    if (World.itemID) {
	free(World.itemID);
	World.itemID = NULL;
    }
    if (World.gravity) {
	free(World.gravity);
	World.gravity = NULL;
    }
    if (World.grav) {
	free(World.grav);
	World.grav = NULL;
    }
    if (World.base) {
	free(World.base);
	World.base = NULL;
    }
    if (World.cannon) {
	free(World.cannon);
	World.cannon = NULL;
    }
    if (World.fuel) {
	free(World.fuel);
	World.fuel = NULL;
    }
    if (World.wormHoles) {
	free(World.wormHoles);
	World.wormHoles = NULL;
    }
    if (World.itemConcentrators) {
	free(World.itemConcentrators);
	World.itemConcentrators = NULL;
    }
    if (World.asteroidConcs) {
	free(World.asteroidConcs);
	World.asteroidConcs = NULL;
    }
}


static void Alloc_map(void)
{
    int x;

    if (World.block || World.gravity)
	Free_map();

    World.block =
	(unsigned char **)malloc(sizeof(unsigned char *)*World.x
				 + World.x*sizeof(unsigned char)*World.y);
    World.itemID =
	(unsigned short **)malloc(sizeof(unsigned short *)*World.x
				 + World.x*sizeof(unsigned short)*World.y);
    World.gravity =
	(vector **)malloc(sizeof(vector *)*World.x
			  + World.x*sizeof(vector)*World.y);
    World.grav = NULL;
    World.base = NULL;
    World.fuel = NULL;
    World.cannon = NULL;
    World.wormHoles = NULL;
    World.itemConcentrators = NULL;
    World.asteroidConcs = NULL;
    if (World.block == NULL || World.itemID == NULL || World.gravity == NULL) {
	Free_map();
	error("Couldn't allocate memory for map (%d bytes)",
	      World.x * (World.y * (sizeof(unsigned char) + sizeof(vector))
			 + sizeof(vector*)
			 + sizeof(unsigned char*)));
	exit(-1);
    } else {
	unsigned char *map_line;
	unsigned char **map_pointer;
	unsigned short *item_line;
	unsigned short **item_pointer;
	vector *grav_line;
	vector **grav_pointer;

	map_pointer = World.block;
	map_line = (unsigned char*) ((unsigned char**)map_pointer + World.x);
	item_pointer = World.itemID;
	item_line = (unsigned short*) ((unsigned short**)item_pointer + World.x);
	grav_pointer = World.gravity;
	grav_line = (vector*) ((vector**)grav_pointer + World.x);

	for (x=0; x<World.x; x++) {
	    *map_pointer = map_line;
	    map_pointer += 1;
	    map_line += World.y;
	    *item_pointer = item_line;
	    item_pointer += 1;
	    item_line += World.y;
	    *grav_pointer = grav_line;
	    grav_pointer += 1;
	    grav_line += World.y;
	}
    }
}


static void Map_extra_error(int line_num)
{
#ifndef SILENT
    static int prev_line_num, error_count;
    const int max_error = 5;

    if (line_num > prev_line_num) {
	prev_line_num = line_num;
	if (++error_count <= max_error) {
	    xpprintf("Map file contains extranous characters on line %d\n",
		     line_num);
	}
	else if (error_count - max_error == 1) {
	    xpprintf("And so on...\n");
	}
    }
#endif
}


static void Map_missing_error(int line_num)
{
#ifndef SILENT
    static int prev_line_num, error_count;
    const int max_error = 5;

    if (line_num > prev_line_num) {
	prev_line_num = line_num;
	if (++error_count <= max_error) {
	    xpprintf("Not enough map data on map data line %d\n", line_num);
	}
	else if (error_count - max_error == 1) {
	    xpprintf("And so on...\n");
	}
    }
#endif
}


bool Grok_map(void)
{
    int i, x, y, c;
    char *s;

    Init_map();

    if (mapWidth <= 0 || mapWidth > MAX_MAP_SIZE ||
	mapHeight <= 0 || mapHeight > MAX_MAP_SIZE) {
	errno = 0;
	error("mapWidth or mapHeight exceeds map size limit [1, %d]",
		MAX_MAP_SIZE);
	free(mapData);
	mapData = NULL;
    } else {
	World.x = mapWidth;
	World.y = mapHeight;
    }
    if (extraBorder) {
	World.x += 2;
	World.y += 2;
    }
    World.diagonal = (int) LENGTH(World.x, World.y);
    World.width = World.x * BLOCK_SZ;
    World.height = World.y * BLOCK_SZ;
    World.hypotenuse = (int) LENGTH(World.width, World.height);
    strlcpy(World.name, mapName, sizeof(World.name));
    strlcpy(World.author, mapAuthor, sizeof(World.author));

    if (!mapData) {
	errno = 0;
	error("Generating random map");
	Generate_random_map();
	if (!mapData) {
	    return FALSE;
	}
    }

    Alloc_map();

    x = -1;
    y = World.y - 1;

    Set_world_rules();
    Set_world_items();
    Set_world_asteroids();

    if (BIT(World.rules->mode, TEAM_PLAY|TIMING) == (TEAM_PLAY|TIMING)) {
	error("Cannot teamplay while in race mode -- ignoring teamplay");
	CLR_BIT(World.rules->mode, TEAM_PLAY);
    }

    s = mapData;
    while (y >= 0) {

	x++;

	if (extraBorder && (x == 0 || x == World.x - 1
	    || y == 0 || y == World.y - 1)) {
	    if (x >= World.x) {
		x = -1;
		y--;
		continue;
	    } else {
		/* make extra border of solid rock */
		c = 'x';
	    }
	}
	else {
	    c = *s;
	    if (c == '\0' || c == EOF) {
		if (x < World.x) {
		    /* not enough map data on this line */
		    Map_missing_error(World.y - y);
		    c = ' ';
		} else {
		    c = '\n';
		}
	    } else {
		if (c == '\n' && x < World.x) {
		    /* not enough map data on this line */
		    Map_missing_error(World.y - y);
		    c = ' ';
		} else {
		    s++;
		}
	    }
	}
	if (x >= World.x || c == '\n') {
	    y--; x = -1;
	    if (c != '\n') {			/* Get rest of line */
		Map_extra_error(World.y - y);
		while (c != '\n' && c != EOF) {
		    c = *s++;
		}
	    }
	    continue;
	}

	switch (World.block[x][y] = c) {
	case 'r':
	case 'd':
	case 'f':
	case 'c':
	    World.NumCannons++;
	    break;
	case '*':
	case '^':
	    World.NumTreasures++;
	    break;
	case '#':
	    World.NumFuels++;
	    break;
	case '!':
	    World.NumTargets++;
	    break;
	case '%':
	    World.NumItemConcentrators++;
	    break;
	case '&':
	    World.NumAsteroidConcs++;
	    break;
	case '_':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	    World.NumBases++;
	    break;
	case '+':
	case '-':
	case '>':
	case '<':
        case 'i':
        case 'm':
        case 'j':
        case 'k':
	    World.NumGravs++;
	    break;
	case '@':
	case '(':
	case ')':
	    World.NumWormholes++;
	    break;
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	case 'Y': case 'Z':
	    if (BIT(World.rules->mode, TIMING))
		World.NumChecks++;
	    break;
	default:
	    break;
	}
    }

    free(mapData);
    mapData = NULL;

    /*
     * Get space for special objects.
     */
    if (World.NumCannons > 0
	&& (World.cannon = (cannon_t *)
	    malloc(World.NumCannons * sizeof(cannon_t))) == NULL) {
	error("Out of memory - cannons");
	exit(-1);
    }
    if (World.NumFuels > 0
	&& (World.fuel = (fuel_t *)
	    malloc(World.NumFuels * sizeof(fuel_t))) == NULL) {
	error("Out of memory - fuel depots");
	exit(-1);
    }
    if (World.NumGravs > 0
	&& (World.grav = (grav_t *)
	    malloc(World.NumGravs * sizeof(grav_t))) == NULL) {
	error("Out of memory - gravs");
	exit(-1);
    }
    if (World.NumWormholes > 0
	&& (World.wormHoles = (wormhole_t *)
	    malloc(World.NumWormholes * sizeof(wormhole_t))) == NULL) {
	error("Out of memory - wormholes");
	exit(-1);
    }
    if (World.NumTreasures > 0
	&& (World.treasures = (treasure_t *)
	    malloc(World.NumTreasures * sizeof(treasure_t))) == NULL) {
	error("Out of memory - treasures");
	exit(-1);
    }
    if (World.NumTargets > 0
	&& (World.targets = (target_t *)
	    malloc(World.NumTargets * sizeof(target_t))) == NULL) {
	error("Out of memory - targets");
	exit(-1);
    }
    if (World.NumItemConcentrators > 0
	&& (World.itemConcentrators = (item_concentrator_t *)
	    malloc(World.NumItemConcentrators * sizeof(item_concentrator_t))) == NULL) {
	error("Out of memory - item concentrators");
	exit(-1);
    }
    if (World.NumAsteroidConcs > 0
	&& (World.asteroidConcs = (asteroid_concentrator_t *)
	    malloc(World.NumAsteroidConcs * sizeof(asteroid_concentrator_t))) == NULL) {
	error("Out of memory - asteroid concentrators");
	exit(-1);
    }
    if (World.NumBases > 0) {
	if ((World.base = (base_t *)
	    malloc(World.NumBases * sizeof(base_t))) == NULL) {
	    error("Out of memory - bases");
	    exit(-1);
	}
    } else {
	error("WARNING: map has no bases!");
    }

    /*
     * Now reset all counters since we will recount everything
     * and reuse these counters while inserting the objects
     * into structures.
     */
    World.NumCannons = 0;
    World.NumFuels = 0;
    World.NumGravs = 0;
    World.NumWormholes = 0;
    World.NumTreasures = 0;
    World.NumTargets = 0;
    World.NumBases = 0;
    World.NumItemConcentrators = 0;
    World.NumAsteroidConcs = 0;

    for (i = 0; i < MAX_TEAMS; i++) {
	World.teams[i].NumMembers = 0;
	World.teams[i].NumRobots = 0;
	World.teams[i].NumBases = 0;
	World.teams[i].NumTreasures = 0;
	World.teams[i].NumEmptyTreasures = 0;
	World.teams[i].TreasuresDestroyed = 0;
	World.teams[i].TreasuresLeft = 0;
	World.teams[i].score = 0;
	World.teams[i].prev_score = 0;
    }

    /*
     * Change read tags to internal data, create objects
     */
    {
	int	worm_in = 0,
		worm_out = 0,
		worm_norm = 0;

	for (x=0; x<World.x; x++) {
	    u_byte *line = World.block[x];
	    unsigned short *itemID = World.itemID[x];

	    for (y=0; y<World.y; y++) {
		char c = line[y];

		itemID[y] = (unsigned short) -1;

		switch (c) {
		case ' ':
		case '.':
		default:
		    line[y] = SPACE;
		    break;

		case 'x':
		    line[y] = FILLED;
		    break;
		case 's':
		    line[y] = REC_LU;
		    break;
		case 'a':
		    line[y] = REC_RU;
		    break;
		case 'w':
		    line[y] = REC_LD;
		    break;
		case 'q':
		    line[y] = REC_RD;
		    break;

		case 'r':
		    line[y] = CANNON;
		    itemID[y] = World.NumCannons;
		    World.cannon[World.NumCannons].dir = DIR_UP;
		    World.cannon[World.NumCannons].blk_pos.x = x;
		    World.cannon[World.NumCannons].blk_pos.y = y;
		    World.cannon[World.NumCannons].pix_pos.x =
						(x + 0.5) * BLOCK_SZ;
		    World.cannon[World.NumCannons].pix_pos.y =
						(y + 0.333) * BLOCK_SZ;
		    World.cannon[World.NumCannons].dead_time = 0;
		    World.cannon[World.NumCannons].conn_mask = (unsigned)-1;
		    World.cannon[World.NumCannons].team = TEAM_NOT_SET;
		    Cannon_init(World.NumCannons);
		    World.NumCannons++;
		    break;
		case 'd':
		    line[y] = CANNON;
		    itemID[y] = World.NumCannons;
		    World.cannon[World.NumCannons].dir = DIR_LEFT;
		    World.cannon[World.NumCannons].blk_pos.x = x;
		    World.cannon[World.NumCannons].blk_pos.y = y;
		    World.cannon[World.NumCannons].pix_pos.x =
						(x + 0.667) * BLOCK_SZ;
		    World.cannon[World.NumCannons].pix_pos.y =
						(y + 0.5) * BLOCK_SZ;
		    World.cannon[World.NumCannons].dead_time = 0;
		    World.cannon[World.NumCannons].conn_mask = (unsigned)-1;
		    World.cannon[World.NumCannons].team = TEAM_NOT_SET;
		    Cannon_init(World.NumCannons);
		    World.NumCannons++;
		    break;
		case 'f':
		    line[y] = CANNON;
		    itemID[y] = World.NumCannons;
		    World.cannon[World.NumCannons].dir = DIR_RIGHT;
		    World.cannon[World.NumCannons].blk_pos.x = x;
		    World.cannon[World.NumCannons].blk_pos.y = y;
		    World.cannon[World.NumCannons].pix_pos.x =
						(x + 0.333) * BLOCK_SZ;
		    World.cannon[World.NumCannons].pix_pos.y =
						(y + 0.5) * BLOCK_SZ;
		    World.cannon[World.NumCannons].dead_time = 0;
		    World.cannon[World.NumCannons].conn_mask = (unsigned)-1;
		    World.cannon[World.NumCannons].team = TEAM_NOT_SET;
		    Cannon_init(World.NumCannons);
		    World.NumCannons++;
		    break;
		case 'c':
		    line[y] = CANNON;
		    itemID[y] = World.NumCannons;
		    World.cannon[World.NumCannons].dir = DIR_DOWN;
		    World.cannon[World.NumCannons].blk_pos.x = x;
		    World.cannon[World.NumCannons].blk_pos.y = y;
		    World.cannon[World.NumCannons].pix_pos.x =
						(x + 0.5) * BLOCK_SZ;
		    World.cannon[World.NumCannons].pix_pos.y =
						(y + 0.667) * BLOCK_SZ;
		    World.cannon[World.NumCannons].dead_time = 0;
		    World.cannon[World.NumCannons].conn_mask = (unsigned)-1;
		    World.cannon[World.NumCannons].team = TEAM_NOT_SET;
		    Cannon_init(World.NumCannons);
		    World.NumCannons++;
		    break;

		case '#':
		    line[y] = FUEL;
		    itemID[y] = World.NumFuels;
		    World.fuel[World.NumFuels].blk_pos.x = x;
		    World.fuel[World.NumFuels].blk_pos.y = y;
		    World.fuel[World.NumFuels].pix_pos.x = (x+0.5f)*BLOCK_SZ;
		    World.fuel[World.NumFuels].pix_pos.y = (y+0.5f)*BLOCK_SZ;
		    World.fuel[World.NumFuels].fuel = START_STATION_FUEL;
		    World.fuel[World.NumFuels].conn_mask = (unsigned)-1;
		    World.fuel[World.NumFuels].last_change = frame_loops;
		    World.fuel[World.NumFuels].team = TEAM_NOT_SET;
		    World.NumFuels++;
		    break;

		case '*':
		case '^':
		    line[y] = TREASURE;
		    itemID[y] = World.NumTreasures;
		    World.treasures[World.NumTreasures].pos.x = x;
		    World.treasures[World.NumTreasures].pos.y = y;
		    World.treasures[World.NumTreasures].have = false;
		    World.treasures[World.NumTreasures].destroyed = 0;
		    World.treasures[World.NumTreasures].empty = (c == '^');
		    /*
		     * Determining which team it belongs to is done later,
		     * in Find_closest_team().
		     */
		    World.treasures[World.NumTreasures].team = 0;
		    World.NumTreasures++;
		    break;
		case '!':
		    line[y] = TARGET;
		    itemID[y] = World.NumTargets;
		    World.targets[World.NumTargets].pos.x = x;
		    World.targets[World.NumTargets].pos.y = y;
		    /*
		     * Determining which team it belongs to is done later,
		     * in Find_closest_team().
		     */
		    World.targets[World.NumTargets].team = 0;
		    World.targets[World.NumTargets].dead_time = 0;
		    World.targets[World.NumTargets].damage = TARGET_DAMAGE;
		    World.targets[World.NumTargets].conn_mask = (unsigned)-1;
		    World.targets[World.NumTargets].update_mask = 0;
		    World.targets[World.NumTargets].last_change = frame_loops;
		    World.NumTargets++;
		    break;
		case '%':
		    line[y] = ITEM_CONCENTRATOR;
		    itemID[y] = World.NumItemConcentrators;
		    World.itemConcentrators[World.NumItemConcentrators].pos.x = x;
		    World.itemConcentrators[World.NumItemConcentrators].pos.y = y;
		    World.NumItemConcentrators++;
		    break;
		case '&':
		    line[y] = ASTEROID_CONCENTRATOR;
		    itemID[y] = World.NumAsteroidConcs;
		    World.asteroidConcs[World.NumAsteroidConcs].pos.x = x;
		    World.asteroidConcs[World.NumAsteroidConcs].pos.y = y;
		    World.NumAsteroidConcs++;
		    break;
		case '$':
		    line[y] = BASE_ATTRACTOR;
		    break;
		case '_':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		    line[y] = BASE;
		    itemID[y] = World.NumBases;
		    World.base[World.NumBases].pos.x = x;
		    World.base[World.NumBases].pos.y = y;
		    /*
		     * The direction of the base should be so that it points
		     * up with respect to the gravity in the region.  This
		     * is fixed in Find_base_dir() when the gravity has
		     * been computed.
		     */
		    World.base[World.NumBases].dir = DIR_UP;
		    if (BIT(World.rules->mode, TEAM_PLAY)) {
			if (c >= '0' && c <= '9') {
			    World.base[World.NumBases].team = c - '0';
			} else {
			    World.base[World.NumBases].team = 0;
			}
			World.teams[World.base[World.NumBases].team].NumBases++;
			if (World.teams[World.base[World.NumBases].team].NumBases == 1)
			    World.NumTeamBases++;
		    } else {
			World.base[World.NumBases].team = TEAM_NOT_SET;
		    }
		    World.NumBases++;
		    break;

		case '+':
		    line[y] = POS_GRAV;
		    itemID[y] = World.NumGravs;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = -GRAVS_POWER;
		    World.NumGravs++;
		    break;
		case '-':
		    line[y] = NEG_GRAV;
		    itemID[y] = World.NumGravs;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = GRAVS_POWER;
		    World.NumGravs++;
		    break;
		case '>':
		    line[y]= CWISE_GRAV;
		    itemID[y] = World.NumGravs;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = GRAVS_POWER;
		    World.NumGravs++;
		    break;
		case '<':
		    line[y] = ACWISE_GRAV;
		    itemID[y] = World.NumGravs;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = -GRAVS_POWER;
		    World.NumGravs++;
		    break;
	        case 'i':
		    line[y] = UP_GRAV;
		    itemID[y] = World.NumGravs;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = GRAVS_POWER;
		    World.NumGravs++;
		    break;
	        case 'm':
		    line[y] = DOWN_GRAV;
		    itemID[y] = World.NumGravs;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = -GRAVS_POWER;
		    World.NumGravs++;
		    break;
	        case 'k':
		    line[y] = RIGHT_GRAV;
		    itemID[y] = World.NumGravs;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = GRAVS_POWER;
		    World.NumGravs++;
		    break;
                case 'j':
		    line[y] = LEFT_GRAV;
		    itemID[y] = World.NumGravs;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = -GRAVS_POWER;
		    World.NumGravs++;
		    break;

		case '@':
		case '(':
		case ')':
		    World.wormHoles[World.NumWormholes].pos.x = x;
		    World.wormHoles[World.NumWormholes].pos.y = y;
		    World.wormHoles[World.NumWormholes].countdown = 0;
		    World.wormHoles[World.NumWormholes].lastdest = -1;
		    World.wormHoles[World.NumWormholes].temporary = 0;
		    World.wormHoles[World.NumWormholes].lastblock = SPACE;
		    World.wormHoles[World.NumWormholes].lastID = -1;
		    if (c == '@') {
			World.wormHoles[World.NumWormholes].type = WORM_NORMAL;
			worm_norm++;
		    } else if (c == '(') {
			World.wormHoles[World.NumWormholes].type = WORM_IN;
			worm_in++;
		    } else {
			World.wormHoles[World.NumWormholes].type = WORM_OUT;
			worm_out++;
		    }
		    line[y] = WORMHOLE;
		    itemID[y] = World.NumWormholes;
		    World.NumWormholes++;
		    break;

		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
		case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
		case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z':
		    if (BIT(World.rules->mode, TIMING)) {
			World.check[c-'A'].x = x;
			World.check[c-'A'].y = y;
			line[y] = CHECK;
		    } else {
			line[y] = SPACE;
		    }
		    break;

		case 'z':
		    line[y] = FRICTION;
		    break;

		case 'b':
		    line[y] = DECOR_FILLED;
		    break;
		case 'h':
		    line[y] = DECOR_LU;
		    break;
		case 'g':
		    line[y] = DECOR_RU;
		    break;
		case 'y':
		    line[y] = DECOR_LD;
		    break;
		case 't':
		    line[y] = DECOR_RD;
		    break;
		}
	    }
	}

	/*
	 * Verify that the wormholes are consistent, i.e. that if
	 * we have no 'out' wormholes, make sure that we don't have
	 * any 'in' wormholes, and (less critical) if we have no 'in'
	 * wormholes, make sure that we don't have any 'out' wormholes.
	 */
	if ((worm_norm) ? (worm_norm + worm_out < 2)
	    : (worm_in) ? (worm_out < 1)
	    : (worm_out > 0)) {

	    int i;

	    xpprintf("Inconsistent use of wormholes, removing them.\n");
	    for (i = 0; i < World.NumWormholes; i++)
		{
			World.block
				[World.wormHoles[i].pos.x]
				[World.wormHoles[i].pos.y] = SPACE;
			World.itemID
				[World.wormHoles[i].pos.x]
				[World.wormHoles[i].pos.y] = (unsigned short) -1;
		}
	    World.NumWormholes = 0;
	}

	if (!wormTime) {
	    for (i = 0; i < World.NumWormholes; i++) {
		int j = (int)(rfrac() * World.NumWormholes);
		while (World.wormHoles[j].type == WORM_IN)
		    j = (int)(rfrac() * World.NumWormholes);
		World.wormHoles[i].lastdest = j;
	    }
	}

	if (BIT(World.rules->mode, TIMING) && World.NumChecks == 0) {
	    xpprintf("No checkpoints found while race mode (timing) was set.\n");
	    xpprintf("Turning off race mode.\n");
	    CLR_BIT(World.rules->mode, TIMING);
	}

	/*
	 * Determine which team a treasure belongs to.
	 */
	if (BIT(World.rules->mode, TEAM_PLAY)) {
	    unsigned short team = TEAM_NOT_SET;
	    for (i = 0; i < World.NumTreasures; i++) {
		team = Find_closest_team(World.treasures[i].pos.x,
					 World.treasures[i].pos.y);
		World.treasures[i].team = team;
		if (team == TEAM_NOT_SET) {
		    error("Couldn't find a matching team for the treasure.");
		} else {
		    World.teams[team].NumTreasures++;
		    if (!World.treasures[i].empty) {
			World.teams[team].TreasuresLeft++;
		    } else {
			World.teams[team].NumEmptyTreasures++;
		    }
		}
	    }
	    for (i = 0; i < World.NumTargets; i++) {
		team = Find_closest_team(World.targets[i].pos.x,
					 World.targets[i].pos.y);
		if (team == TEAM_NOT_SET) {
		    error("Couldn't find a matching team for the target.");
		}
		World.targets[i].team = team;
	    }
	    if (teamCannons) {
		for (i = 0; i < World.NumCannons; i++) {
		    team = Find_closest_team(World.cannon[i].blk_pos.x,
					     World.cannon[i].blk_pos.y);
		    if (team == TEAM_NOT_SET) {
			error("Couldn't find a matching team for the cannon.");
		    }
		    World.cannon[i].team = team;
		}
	    }
	    for (i = 0; i < World.NumFuels; i++) {
		team = Find_closest_team(World.fuel[i].blk_pos.x,
					 World.fuel[i].blk_pos.y);
		if (team == TEAM_NOT_SET) {
		    error("Couldn't find a matching team for fuelstation.");
		}
		World.fuel[i].team = team;
	    }
	}
    }

    if (maxRobots == -1) {
	maxRobots = World.NumBases;
    }
    if (minRobots == -1) {
	minRobots = maxRobots;
    }
    if (BIT(World.rules->mode, TIMING)) {
	Find_base_order();
    }

#ifndef	SILENT
    xpprintf("World....: %s\nBases....: %d\nMapsize..: %dx%d\nTeam play: %s\n",
	   World.name, World.NumBases, World.x, World.y,
	   BIT(World.rules->mode, TEAM_PLAY) ? "on" : "off");
#endif

    D( Print_map(); )

    return TRUE;
}


/*
 * Use wildmap to generate a random map.
 */
static void Generate_random_map(void)
{
    int		width, height;

    edgeWrap = TRUE;
    width = World.x;
    height = World.y;

    Wildmap(width, height, World.name, World.author, &mapData, &width, &height);

    World.x = width;
    World.y = height;
    World.diagonal = (int) LENGTH(World.x, World.y);
    World.width = World.x * BLOCK_SZ;
    World.height = World.y * BLOCK_SZ;
    World.hypotenuse = (int) LENGTH(World.width, World.height);
}


/*
 * Find the correct direction of the base, according to the gravity in
 * the base region.
 *
 * If a base attractor is adjacent to a base then the base will point
 * to the attractor.
 */
void Find_base_direction(void)
{
    int	i;

    for (i = 0; i < World.NumBases; i++) {
	int	x = World.base[i].pos.x,
		y = World.base[i].pos.y,
		dir,
		att;
	double	dx = World.gravity[x][y].x,
		dy = World.gravity[x][y].y;

	if (dx == 0.0 && dy == 0.0) {	/* Undefined direction? */
	    dir = DIR_UP;	/* Should be set to direction of gravity! */
	} else {
	    dir = (int)findDir(-dx, -dy);
	    dir = ((dir + RES/8) / (RES/4)) * (RES/4);	/* round it */
	    dir = MOD2(dir, RES);
	}
	att = -1;
	/*BASES SNAP TO UPWARDS ATTRACTOR FIRST*/
        if (y == World.y - 1 && World.block[x][0] == BASE_ATTRACTOR && BIT(World.rules->mode, WRAP_PLAY)) {  /*check wrapped*/
	    if (att == -1 || dir == DIR_UP) {
		att = DIR_UP;
	    }
	}
	if (y < World.y - 1 && World.block[x][y + 1] == BASE_ATTRACTOR) {
	    if (att == -1 || dir == DIR_UP) {
		att = DIR_UP;
	    }
	}
	/*THEN DOWNWARDS ATTRACTORS*/
        if (y == 0 && World.block[x][World.y-1] == BASE_ATTRACTOR && BIT(World.rules->mode, WRAP_PLAY)) { /*check wrapped*/
	    if (att == -1 || dir == DIR_DOWN) {
		att = DIR_DOWN;
	    }
	}
	if (y > 0 && World.block[x][y - 1] == BASE_ATTRACTOR) {
	    if (att == -1 || dir == DIR_DOWN) {
		att = DIR_DOWN;
	    }
	}
	/*THEN RIGHTWARDS ATTRACTORS*/
	if (x == World.x - 1 && World.block[0][y] == BASE_ATTRACTOR && BIT(World.rules->mode, WRAP_PLAY)) { /*check wrapped*/
	    if (att == -1 || dir == DIR_RIGHT) {
		att = DIR_RIGHT;
	    }
	}
	if (x < World.x - 1 && World.block[x + 1][y] == BASE_ATTRACTOR) {
	    if (att == -1 || dir == DIR_RIGHT) {
		att = DIR_RIGHT;
	    }
	}
	/*THEN LEFTWARDS ATTRACTORS*/
	if (x == 0 && World.block[World.x-1][y] == BASE_ATTRACTOR && BIT(World.rules->mode, WRAP_PLAY)) { /*check wrapped*/
	    if (att == -1 || dir == DIR_LEFT) {
		att = DIR_LEFT;
	    }
	}
	if (x > 0 && World.block[x - 1][y] == BASE_ATTRACTOR) {
	    if (att == -1 || dir == DIR_LEFT) {
		att = DIR_LEFT;
	    }
	}
	if (att != -1) {
	    dir = att;
	}
	World.base[i].dir = dir;
    }
    for (i = 0; i < World.x; i++) {
	int j;
	for (j = 0; j < World.y; j++) {
	    if (World.block[i][j] == BASE_ATTRACTOR) {
		World.block[i][j] = SPACE;
	    }
	}
    }
}


/*
 * Return the team that is closest to this position.
 */
unsigned short Find_closest_team(int posx, int posy)
{
    unsigned short team = TEAM_NOT_SET;
    int i;
    DFLOAT closest = FLT_MAX, l;

    for (i = 0; i < World.NumBases; i++) {
	if (World.base[i].team == TEAM_NOT_SET)
	    continue;

	l = Wrap_length((posx - World.base[i].pos.x)*BLOCK_SZ,
			(posy - World.base[i].pos.y)*BLOCK_SZ);

	if (l < closest) {
	    team = World.base[i].team;
	    closest = l;
	}
    }

    return team;
}


/*
 * Determine the order in which players are placed
 * on starting positions after race mode reset.
 */
static void Find_base_order(void)
{
    int			i, j, k, n;
    DFLOAT		cx, cy, dist;

    if (!BIT(World.rules->mode, TIMING)) {
	World.baseorder = NULL;
	return;
    }
    if ((n = World.NumBases) <= 0) {
	error("Cannot support race mode in a map without bases");
	exit(-1);
    }

    if ((World.baseorder = (baseorder_t *)
	    malloc(n * sizeof(baseorder_t))) == NULL) {
	error("Out of memory - baseorder");
	exit(-1);
    }

    cx = World.check[0].x * BLOCK_SZ;
    cy = World.check[0].y * BLOCK_SZ;
    for (i = 0; i < n; i++) {
	dist = Wrap_length(World.base[i].pos.x * BLOCK_SZ - cx,
			   World.base[i].pos.y * BLOCK_SZ - cy);
	for (j = 0; j < i; j++) {
	    if (World.baseorder[j].dist > dist) {
		break;
	    }
	}
	for (k = i - 1; k >= j; k--) {
	    World.baseorder[k + 1] = World.baseorder[k];
	}
	World.baseorder[j].base_idx = i;
	World.baseorder[j].dist = dist;
    }
}


DFLOAT Wrap_findDir(DFLOAT dx, DFLOAT dy)
{
    dx = WRAP_DX(dx);
    dy = WRAP_DY(dy);
    return findDir(dx, dy);
}


DFLOAT Wrap_length(DFLOAT dx, DFLOAT dy)
{
    dx = WRAP_DX(dx);
    dy = WRAP_DY(dy);
    return LENGTH(dx, dy);
}


static void Compute_global_gravity(void)
{
    int			xi, yi, dx, dy;
    DFLOAT		xforce, yforce, strength;
    double		theta;
    vector		*grav;


    if (gravityPointSource == false) {
	theta = (gravityAngle * PI) / 180.0;
	xforce = cos(theta) * Gravity;
	yforce = sin(theta) * Gravity;
	for (xi=0; xi<World.x; xi++) {
	    grav = World.gravity[xi];

	    for (yi=0; yi<World.y; yi++, grav++) {
		grav->x = xforce;
		grav->y = yforce;
	    }
	}
    } else {
	for (xi=0; xi<World.x; xi++) {
	    grav = World.gravity[xi];
	    dx = (xi - gravityPoint.x) * BLOCK_SZ;
	    dx = WRAP_DX(dx);

	    for (yi=0; yi<World.y; yi++, grav++) {
		dy = (yi - gravityPoint.y) * BLOCK_SZ;
		dy = WRAP_DX(dy);

		if (dx == 0 && dy == 0) {
		    grav->x = 0.0;
		    grav->y = 0.0;
		    continue;
		}
		strength = Gravity / LENGTH(dx, dy);
		if (gravityClockwise) {
		    grav->x =  dy * strength;
		    grav->y = -dx * strength;
		}
		else if (gravityAnticlockwise) {
		    grav->x = -dy * strength;
		    grav->y =  dx * strength;
		}
		else {
		    grav->x =  dx * strength;
		    grav->y =  dy * strength;
		}
	    }
	}
    }
}


static void Compute_grav_tab(vector grav_tab[GRAV_RANGE+1][GRAV_RANGE+1])
{
    int			x, y;
    double		strength;

    grav_tab[0][0].x = grav_tab[0][0].y = 0;
    for (x = 0; x < GRAV_RANGE+1; x++) {
	for (y = (x == 0); y < GRAV_RANGE+1; y++) {
	    strength = pow((double)(sqr(x) + sqr(y)), -1.5);
	    grav_tab[x][y].x = x * strength;
	    grav_tab[x][y].y = y * strength;
	}
    }
}


static void Compute_local_gravity(void)
{
    int			xi, yi, g, gx, gy, ax, ay, dx, dy, gtype;
    int			first_xi, last_xi, first_yi, last_yi, mod_xi, mod_yi;
    int			min_xi, max_xi, min_yi, max_yi;
    DFLOAT		force, fx, fy;
    vector		*v, *grav, *tab, grav_tab[GRAV_RANGE+1][GRAV_RANGE+1];


    Compute_grav_tab(grav_tab);

    min_xi = 0;
    max_xi = World.x - 1;
    min_yi = 0;
    max_yi = World.y - 1;
    if (BIT(World.rules->mode, WRAP_PLAY)) {
	min_xi -= MIN(GRAV_RANGE, World.x);
	max_xi += MIN(GRAV_RANGE, World.x);
	min_yi -= MIN(GRAV_RANGE, World.y);
	max_yi += MIN(GRAV_RANGE, World.y);
    }
    for (g=0; g<World.NumGravs; g++) {
	gx = World.grav[g].pos.x;
	gy = World.grav[g].pos.y;
	force = World.grav[g].force;

	if ((first_xi = gx - GRAV_RANGE) < min_xi) {
	    first_xi = min_xi;
	}
	if ((last_xi = gx + GRAV_RANGE) > max_xi) {
	    last_xi = max_xi;
	}
	if ((first_yi = gy - GRAV_RANGE) < min_yi) {
	    first_yi = min_yi;
	}
	if ((last_yi = gy + GRAV_RANGE) > max_yi) {
	    last_yi = max_yi;
	}
	gtype = World.block[gx][gy];
	mod_xi = (first_xi < 0) ? (first_xi + World.x) : first_xi;
	dx = gx - first_xi;
	fx = force;
	for (xi = first_xi; xi <= last_xi; xi++, dx--) {
	    if (dx < 0) {
		fx = -force;
		ax = -dx;
	    } else {
		ax = dx;
	    }
	    mod_yi = (first_yi < 0) ? (first_yi + World.y) : first_yi;
	    dy = gy - first_yi;
	    grav = &World.gravity[mod_xi][mod_yi];
	    tab = grav_tab[ax];
	    fy = force;
	    for (yi = first_yi; yi <= last_yi; yi++, dy--) {
		if (dx || dy) {
		    if (dy < 0) {
			fy = -force;
			ay = -dy;
		    } else {
			ay = dy;
		    }
		    v = &tab[ay];
		    if (gtype == CWISE_GRAV || gtype == ACWISE_GRAV) {
			grav->x -= fy * v->y;
			grav->y += fx * v->x;
		    } else if (gtype == UP_GRAV || gtype == DOWN_GRAV) {
			grav->y += force * v->x;
		    } else if (gtype == RIGHT_GRAV || gtype == LEFT_GRAV) {
			grav->x += force * v->y;
		    } else {
			grav->x += fx * v->x;
			grav->y += fy * v->y;
		    }
		}
		else {
		    if (gtype == UP_GRAV || gtype == DOWN_GRAV) {
			grav->y += force;
		    }
		    else if (gtype == LEFT_GRAV || gtype == RIGHT_GRAV) {
			grav->x += force;
		    }
		}
		mod_yi++;
		grav++;
		if (mod_yi >= World.y) {
		    mod_yi = 0;
		    grav = World.gravity[mod_xi];
		}
	    }
	    if (++mod_xi >= World.x) {
		mod_xi = 0;
	    }
	}
    }
    /*
     * We may want to free the World.gravity memory here
     * as it is not used anywhere else.
     * e.g.: free(World.gravity);
     *       World.gravity = NULL;
     *       World.NumGravs = 0;
     * Some of the more modern maps have quite a few gravity symbols.
     */
}


void Compute_gravity(void)
{
    Compute_global_gravity();
    Compute_local_gravity();
}


void add_temp_wormholes(int xin, int yin, int xout, int yout)
{
    wormhole_t inhole, outhole, *wwhtemp;

    if ((wwhtemp = (wormhole_t *)realloc(World.wormHoles,
					 (World.NumWormholes + 2)
					 * sizeof(wormhole_t)))
	== NULL) {
	error("No memory for temporary wormholes.");
	return;
    }
    World.wormHoles = wwhtemp;

    inhole.pos.x = xin;
    inhole.pos.y = yin;
    outhole.pos.x = xout;
    outhole.pos.y = yout;
    inhole.countdown = outhole.countdown = wormTime;
    inhole.lastdest = World.NumWormholes + 1;
    inhole.temporary = outhole.temporary = 1;
    inhole.type = WORM_IN;
    outhole.type = WORM_OUT;
    inhole.lastblock = World.block[xin][yin];
    outhole.lastblock = World.block[xout][yout];
    inhole.lastID = World.itemID[xin][yin];
    outhole.lastID = World.itemID[xout][yout];
    World.wormHoles[World.NumWormholes] = inhole;
    World.wormHoles[World.NumWormholes + 1] = outhole;
    World.block[xin][yin] = World.block[xout][yout] = WORMHOLE;
    World.itemID[xin][yin] = World.NumWormholes;
    World.itemID[xout][yout] = World.NumWormholes + 1;
    World.NumWormholes += 2;
}

void remove_temp_wormhole(int ind)
{
    wormhole_t hole;

    hole = World.wormHoles[ind];
    World.block[hole.pos.x][hole.pos.y] = hole.lastblock;
    World.itemID[hole.pos.x][hole.pos.y] = hole.lastID;
    World.NumWormholes--;
    if (ind != World.NumWormholes) {
	World.wormHoles[ind] = World.wormHoles[World.NumWormholes];
    }
    World.wormHoles = (wormhole_t *)realloc(World.wormHoles,
					    World.NumWormholes
					    * sizeof(wormhole_t));
}


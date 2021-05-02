/* $Id: map.c,v 3.14 1993/10/02 00:36:14 bjoerns Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-93 by
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "global.h"
#include "map.h"
#include "bit.h"

#define GRAV_RANGE  10

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: map.c,v 3.14 1993/10/02 00:36:14 bjoerns Exp $";
#endif


/*
 * Globals.
 */
World_map World;


u_short Find_closest_team(int posx, int posy);


/*
 * Sets as many blocks as possible to FILLED_NO_DRAW.
 * You won't notice any difference. :)
 */
void Optimize_map(void)
{
    int x, y, type;


    for (x=1; x<(World.x-1); x++) {
        u_byte *line	= World.block[x];
        u_byte *n_line	= World.block[x+1];
        u_byte *p_line	= World.block[x-1];

	for (y=1; y<(World.y-1); y++)
	    if (line[y] == FILLED) {
		type = line[y-1];
		if ((type != FILLED) && (type != REC_LU) && (type != REC_RU))
		    continue;

		type = p_line[y];
		if ((type != FILLED) && (type != REC_RD) && (type != REC_RU))
		    continue;

		type = n_line[y];
		if ((type != FILLED) && (type != REC_LD) && (type != REC_LU))
		    continue;

		type = line[y+1];
		if ((type != REC_LD) && (type != REC_RD) && (type != FILLED)) {
                    y++;
		    continue;
                }

		line[y++] = FILLED_NO_DRAW;
	    }
    }
}


void Print_map(void)			/* Debugging only. */
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


void Init_map(void)
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
    World.NumBases	= 0;
    World.NumWormholes	= 0;
    World.NumTreasures  = 0;
    World.NumTargets    = 0;
}


void Free_map(void)
{
    if (World.block) free(World.block);
    if (World.gravity) free(World.gravity);
    if (World.grav) free(World.grav);
    if (World.base) free(World.base);
    if (World.cannon) free(World.cannon);
    if (World.fuel) free(World.fuel);
    if (World.wormHoles) free(World.wormHoles);
}


void Alloc_map(void)
{
    int x;

    if (World.block || World.gravity)
	Free_map();

    World.block =
        (unsigned char **)malloc(sizeof(unsigned char *)*World.x
				 + World.x*sizeof(unsigned char)*World.y);
    World.gravity = 
        (vector **)malloc(sizeof(vector *)*World.x
			  + World.x*sizeof(vector)*World.y);
    World.grav = 0;
    World.base = 0;
    World.fuel = 0;
    World.cannon = 0;
    World.wormHoles = 0;
    if (World.block == NULL || World.gravity == NULL) {
	Free_map();
	error("Couldn't allocate memory for map (%d bytes)",
	      World.x * (World.y * (sizeof(unsigned char) + sizeof(vector))
			 + sizeof(vector*)
			 + sizeof(unsigned char*)));
	exit(-1);
    } else {
        unsigned char *map_line;
        unsigned char **map_pointer;
        vector *grav_line;
        vector **grav_pointer;

        map_pointer = World.block;
        map_line = (unsigned char*) ((unsigned char**)map_pointer + World.x);
        grav_pointer = World.gravity;
        grav_line = (vector*) ((vector**)grav_pointer + World.x);

	for (x=0; x<World.x; x++) {
            *map_pointer = map_line;
            map_pointer += 1;
            map_line += World.y;
            *grav_pointer = grav_line;
            grav_pointer += 1;
            grav_line += World.y;
	}
    }
}


static void Map_error(int line_num)
{
#ifndef SILENT
    static int prev_line_num, error_count;
    const int max_error = 5;

    if (line_num > prev_line_num) {
	prev_line_num = line_num;
	if (++error_count <= max_error) {
	    printf("Not enough map data on map data line %d\n", line_num);
	}
	else if (error_count - max_error == 1) {
	    printf("And so on...\n");
	}
    }
#endif
}


void Grok_map(void)
{
    int i, x, y, c;
    bool done_line = false;
    char *s;

    Init_map();
    
    World.x = mapWidth;
    World.y = mapHeight;
    if (extraBorder) {
	World.x += 2;
	World.y += 2;
    }
    World.diagonal = (int) LENGTH(World.x, World.y);
    World.width = World.x * BLOCK_SZ;
    World.height = World.y * BLOCK_SZ;
    World.hypotenuse = (int) LENGTH(World.width, World.height);
    strncpy(World.name, mapName, sizeof(World.name) - 1);
    World.name[sizeof(World.name) - 1] = '\0';
    strncpy(World.author, mapAuthor, sizeof(World.author) - 1);
    World.author[sizeof(World.author) - 1] = '\0';
    
    Alloc_map();
    
    x = -1;
    y = World.y - 1;
    
    Set_world_rules();

    if (!mapData) {
	errno = 0;
	error("Generating random map");
	Generate_random_map();
	if (!mapData) {
	    return;
	}
    }

    s = mapData;
    while (y >= 0) {
	    
	x++;
	done_line = false;

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
		    Map_error(World.y - y);
		    c = ' ';
		} else {
		    c = '\n';
		}
	    } else {
		if (c == '\n' && x < World.x) {
		    /* not enough map data on this line */
		    Map_error(World.y - y);
		    c = ' ';
		} else {
		    s++;
		}
	    }
	}
	if (x >= World.x || c == '\n') {
	    y--; x = -1;
	    done_line = true;
	    if (c != '\n') {			/* Get rest of line */
		error("Map file contains extranous characters");
		while (c != '\n' && c != EOF)	/* from file. */
		    fputc(c = *s++, stderr);
	    }
	}
	if (done_line)
	    continue;

	switch (World.block[x][y] = c) {
	case 'r':
	case 'd':
	case 'f':
	case 'c':
	    World.NumCannons++;
	    break;
	case '*':
	    if (BIT(World.rules->mode, TEAM_PLAY))
		World.NumTreasures++;
	    else
		World.block[x][y] = ' ';
	    break;
	case '#': 
	    World.NumFuels++;
	    break;
	case '!':
	    if (BIT(World.rules->mode, TEAM_PLAY))
		World.NumTargets++;
	    else
		World.block[x][y] = ' ';
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

    free (mapData);
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
    if (World.NumBases > 0) {
	if ((World.base = (base_t *)
	    malloc(World.NumBases * sizeof(base_t))) == NULL) {
	    error("Out of memory - bases");
	    exit(-1);
	}
    } else {
	error("Warning, map has no bases");
    }
    World.NumCannons = 0;		/* Now reset all counters since */
    World.NumFuels = 0;			/* we will recount everything */
    World.NumGravs = 0;			/* (and reuse these counters) */
    World.NumWormholes = 0;		/* while inserting the objects */
    World.NumTreasures = 0;		/* into structures. */
    World.NumTargets = 0;
    World.NumBases = 0;
    for (i=0; i<MAX_TEAMS; i++)
	World.teams[i].NumMembers = World.teams[i].NumBases = 0;

    /*
     * Change read tags to internal data, create objects
     */
    {
	int	worm_in = 0,
		worm_out = 0,
		worm_norm = 0;
    
        for (x=0; x<World.x; x++) {
            u_byte *line = World.block[x];
    
            for (y=0; y<World.y; y++) {
		char c = line[y];
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
		    World.cannon[World.NumCannons].dir = DIR_UP;
		    World.cannon[World.NumCannons].pos.x = x;
		    World.cannon[World.NumCannons].pos.y = y;
		    World.cannon[World.NumCannons].dead_time = 0;
		    World.cannon[World.NumCannons].conn_mask = (unsigned)-1;
		    World.cannon[World.NumCannons].last_change = loops;
		    World.cannon[World.NumCannons].active = false;
		    World.NumCannons++;
		    break;
		case 'd':
		    line[y] = CANNON;
		    World.cannon[World.NumCannons].dir = DIR_LEFT;
		    World.cannon[World.NumCannons].pos.x = x;
		    World.cannon[World.NumCannons].pos.y = y;
		    World.cannon[World.NumCannons].dead_time = 0;
		    World.cannon[World.NumCannons].conn_mask = (unsigned)-1;
		    World.cannon[World.NumCannons].last_change = loops;
		    World.cannon[World.NumCannons].active = false;
		    World.NumCannons++;
		    break;
		case 'f':
		    line[y] = CANNON;
		    World.cannon[World.NumCannons].dir = DIR_RIGHT;
		    World.cannon[World.NumCannons].pos.x = x;
		    World.cannon[World.NumCannons].pos.y = y;
		    World.cannon[World.NumCannons].dead_time = 0;
		    World.cannon[World.NumCannons].conn_mask = (unsigned)-1;
		    World.cannon[World.NumCannons].last_change = loops;
		    World.cannon[World.NumCannons].active = false;
		    World.NumCannons++;
		    break;
		case 'c':
		    line[y] = CANNON;
		    World.cannon[World.NumCannons].dir = DIR_DOWN;
		    World.cannon[World.NumCannons].pos.x = x;
		    World.cannon[World.NumCannons].pos.y = y;
		    World.cannon[World.NumCannons].dead_time = 0;
		    World.cannon[World.NumCannons].conn_mask = (unsigned)-1;
		    World.cannon[World.NumCannons].last_change = loops;
		    World.cannon[World.NumCannons].active = false;
		    World.NumCannons++;
		    break;

		case '#':
		    line[y] = FUEL;
		    World.fuel[World.NumFuels].pos.x = x*BLOCK_SZ+BLOCK_SZ/2;
		    World.fuel[World.NumFuels].pos.y = y*BLOCK_SZ+BLOCK_SZ/2;
		    World.fuel[World.NumFuels].fuel = START_STATION_FUEL;
		    World.fuel[World.NumFuels].conn_mask = (unsigned)-1;
		    World.fuel[World.NumFuels].last_change = loops;
		    World.NumFuels++;
		    break;

		case '*':
		    line[y] = TREASURE;
		    World.treasures[World.NumTreasures].pos.x = x;
		    World.treasures[World.NumTreasures].pos.y = y;
		    World.treasures[World.NumTreasures].have = true;
		    World.treasures[World.NumTreasures].count = 0;
		    /*
		     * Determining which team it belongs to is done later,
		     * in Find_closest_team().
		     */
		    World.NumTreasures++;
		    break;
		case '!':
		    line[y] = TARGET;
		    World.targets[World.NumTargets].pos.x = x;
		    World.targets[World.NumTargets].pos.y = y;
		    World.targets[World.NumTargets].pos.y = y;
		    /*
		     * Determining which team it belongs to is done later,
		     * in Find_closest_team().
		     */
		    World.targets[World.NumTargets].dead_time = 0;
		    World.targets[World.NumTargets].damage = TARGET_DAMAGE;
		    World.targets[World.NumTargets].conn_mask = (unsigned)-1;
		    World.targets[World.NumTargets].last_change = loops;
		    World.NumTargets++;
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
			    World.teams[c - '0'].NumBases++;
			} else {
			    World.base[World.NumBases].team = 0;
			}
		    } else {
			World.base[World.NumBases].team = TEAM_NOT_SET;
		    }
		    World.NumBases++;
		    break;

		case '+':
		    line[y] = POS_GRAV;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = -GRAVS_POWER;
		    World.NumGravs++;
		    break;
		case '-':
		    line[y] = NEG_GRAV;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = GRAVS_POWER;
		    World.NumGravs++;
		    break;
		case '>':
		    line[y]= CWISE_GRAV;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = GRAVS_POWER;
		    World.NumGravs++;
		    break;
		case '<':
		    line[y] = ACWISE_GRAV;
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
		    } else
			line[y] = SPACE;
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
        if (worm_out == 0
	    && (worm_norm == 1 || (worm_norm == 0 && worm_in > 0))) {
            int i;
            
            error("Inconsistent use of wormholes, removing them");
            for (i=0; i<World.NumWormholes; i++)
                World.block
		    [World.wormHoles[i].pos.x]
		    [World.wormHoles[i].pos.y] = SPACE;
            World.NumWormholes = 0;
        }

	/*
	 * Determine which team a treasure belongs to.
	 * NOTE: Should check so that all teams have one, and only one,
	 * treasure.
	 */
	for (i=0; i<World.NumTreasures; i++) {
	    u_short team = Find_closest_team(World.treasures[i].pos.x,
					     World.treasures[i].pos.y);
	    if (team == TEAM_NOT_SET) {
		error("Couldn't find a matching team for the treasure.");
		World.treasures[i].have = false;
	    }
	    World.treasures[i].team = team;
	}
	for (i=0; i<World.NumTargets; i++) {
	    u_short team = Find_closest_team(World.targets[i].pos.x,
					     World.targets[i].pos.y);
	    if (team == TEAM_NOT_SET) {
		error("Couldn't find a matching team for the target.");
	    }
	    World.targets[i].team = team;
	}	
    }

    Optimize_map();

    if (WantedNumRobots == -1) {
	if (BIT(World.rules->mode, TEAM_PLAY)) {
	    WantedNumRobots = World.teams[0].NumBases;
	} else {
	    WantedNumRobots = World.NumBases;
	}
    }
    if (BIT(World.rules->mode, TIMING))
	WantedNumRobots = 0;

#ifndef	SILENT
    printf("World....: %s\nBases....: %d\nMapsize..: %dx%d\nTeam play: %s\n",
	   World.name, World.NumBases, World.x, World.y,
	   BIT(World.rules->mode, TEAM_PLAY) ? "on" : "off");
#endif

    D( Print_map() );
}


/*
 * Stupid random map routine, only to be used if the game can't open any
 * map file. :)
 */
void Generate_random_map(void)
{
    int			x,
			y,
			i,
			size,
			num_bases = 30,
			num_fuels = (World.x * World.y) / (1000 + rand()%1000),
			num_cannons = (World.x * World.y) / (100 + rand()%100),
			num_blocks = (World.x * World.y) / (25 + rand()%25),
			num_gravs = (World.x * World.y) / (2500 + rand()%2500),
			num_worms = (World.x * World.y) / (2500 + rand()%2500);
    

    strcpy(World.name, "Random Land");
    strcpy(World.author, "The Computer");

    size = (World.x + 1) * World.y + 1;
    if ((mapData = (char *)malloc(size)) == NULL) {
	error("Can't allocate map");
	return;
    }
    memset(mapData, ' ', size);

    while (--num_blocks >= 0) {
	switch (rand()%5) {
	case 0: i = 'a'; break;
	case 1: i = 'w'; break;
	case 2: i = 'q'; break;
	case 3: i = 's'; break;
	default: i = 'x'; break;
	}
	mapData[(rand() % World.x) + (rand() % World.y) * (World.x + 1)] = i;
    }
    while (--num_cannons >= 0) {
	switch (rand()%4) {
	case 0: i = 'd'; break;
	case 1: i = 'f'; break;
	case 2: i = 'r'; break;
	default: i = 'c'; break;
	}
	mapData[(rand() % World.x) + (rand() % World.y) * (World.x + 1)] = i;
    }
    while (--num_gravs >= 0) {
	switch (rand()%4) {
	case 0: i = '+'; break;
	case 1: i = '-'; break;
	case 2: i = '<'; break;
	default: i = '>'; break;
	}
	mapData[(rand() % World.x) + (rand() % World.y) * (World.x + 1)] = i;
    }
    while (--num_fuels >= 0) {
	mapData[(rand() % World.x) + (rand() % World.y) * (World.x + 1)] = '#';
    }
    while (--num_worms >= 0) {
	switch (rand()%3) {
	case 0: i = '('; break;
	case 1: i = ')'; break;
	default: i = '@'; break;
	}
	mapData[(rand() % World.x) + (rand() % World.y) * (World.x + 1)] = i;
    }
    while (--num_bases >= 0) {
	i = '0' + num_bases % 10;
	mapData[(rand() % World.x) + (rand() % World.y) * (World.x + 1)] = i;
    }
    for (i='A'; i<='Z'; i++) {
        x = rand() % (World.x-2) + 1;
        y = rand() % (World.y-2) + 1;
        mapData[x + y * (World.x + 1)] = i;
    }
    for (y = 0; y < World.y; y++) {
	mapData[(y + 1) * (World.x + 1) - 1] = '\n';
    }

    mapData[size - 1] = '\0';
}


/*
 * Find the correct direction of the base, according to the gravity in
 * the base region.
 */
void Find_base_direction(void)
{
    int	i;

    for (i=0; i<World.NumBases; i++) {
	int	x = World.base[i].pos.x,
		y = World.base[i].pos.y,
		dir;
	double	dx = World.gravity[x][y].x,
	    	dy = World.gravity[x][y].y;

	if (dx == 0.0 && dy == 0.0) {	/* Undefined direction? */
	    dir = DIR_UP;	/* Should be set to direction of gravity! */
	} else {
	    dir = (int)findDir(-dx, -dy);
	    dir = ((dir + RES/8) / (RES/4)) * (RES/4);	/* round it */
	    dir = MOD2(dir, RES);
	}
	World.base[i].dir = dir;
    }
}

/*
 * Return the team that is closest to this position.
 */
u_short Find_closest_team(int posx, int posy)
{
    u_short team = TEAM_NOT_SET;
    int i;
    float closest = FLT_MAX, l;

    for (i=0; i<World.NumBases; i++) {
        if (World.base[i].team == TEAM_NOT_SET)
            continue;
        
        l = Wrap_length(posx - World.base[i].pos.x,
			posy - World.base[i].pos.y);
	
        if (l < closest) {
            team = World.base[i].team;
            closest = l;
        }	
    }	

    return team;
}

float Wrap_findDir(float dx, float dy)
{
    dx = WRAP_DX(dx);
    dy = WRAP_DY(dy);
    return findDir(dx, dy);
}


float Wrap_length(float dx, float dy)
{
    dx = WRAP_DX(dx);
    dy = WRAP_DY(dy);
    return LENGTH(dx, dy);
}


void Compute_gravity(void)
{
    int xi, yi, g, gx, gy, dx, dy;
    int first_xi, last_xi, first_yi, last_yi, mod_xi, mod_yi, wrap;
    float dist2, xforce, yforce;
    double theta, clock;


    wrap = (BIT(World.rules->mode, WRAP_PLAY) != 0);

    if (gravityPointSource == false) {
	theta = (gravityAngle * PI) / 180.0;
	xforce = sin(theta) * Gravity;
	yforce = cos(theta) * Gravity;
	for (xi=0; xi<World.x; xi++) {
	    vector *line = World.gravity[xi];

	    for (yi=0; yi<World.y; yi++) {
		line[yi].y = xforce;
		line[yi].x = yforce;
	    }
	}
    } else {
	if (gravityClockwise) {
	    clock = -PI / 2;
	}
	else if (gravityAnticlockwise) {
	    clock = PI / 2;
	}
	else {
	    clock = 0.0;
	}
	for (xi=0; xi<World.x; xi++) {
	    vector *line = World.gravity[xi];

	    for (yi=0; yi<World.y; yi++) {
		dx = (xi - gravityPoint.x) * BLOCK_SZ;
		dy = (yi - gravityPoint.x) * BLOCK_SZ;
		dx = WRAP_DX(dx);
		dy = WRAP_DX(dy);

		if (dx == 0 && dy == 0) {
		    line[yi].y = 0.0;
		    line[yi].x = 0.0;
		    continue;
		}

		theta = atan2((double)dy, (double)dx) + clock;

		line[yi].x = cos(theta) * Gravity;
		line[yi].y = sin(theta) * Gravity;
	    }
	}
    }

    for (g=0; g<World.NumGravs; g++) {
	gx = World.grav[g].pos.x;
	gy = World.grav[g].pos.y;

	if (gx - GRAV_RANGE >= 0 || wrap) {
	    first_xi = gx - GRAV_RANGE;
	} else {
	    first_xi = 0;
	}
	if (gx + GRAV_RANGE < World.x || wrap) {
	    last_xi = gx + GRAV_RANGE;
	} else {
	    last_xi = World.x - 1;
	}
	if (gy - GRAV_RANGE >= 0 || wrap) {
	    first_yi = gy - GRAV_RANGE;
	} else {
	    first_yi = 0;
	}
	if (gy + GRAV_RANGE < World.y || wrap) {
	    last_yi = gy + GRAV_RANGE;
	} else {
	    last_yi = World.y - 1;
	}
	for (xi = first_xi; xi <= last_xi; xi++) {
	    vector *line = World.gravity[mod_xi = mod(xi, World.x)];

	    for (yi = first_yi; yi <= last_yi; yi++) {

		mod_yi = (wrap) ? mod(yi, World.y) : yi;

		dx = gx - xi;
		dy = gy - yi;

		if (dy == 0 && dx == 0)		/* In a grav? */
		    continue;

		theta = atan2((double)dy, (double)dx);

		if (World.block[gx][gy] == CWISE_GRAV
		    || World.block[gx][gy] == ACWISE_GRAV)
		    theta += PI/2.0;

		dist2 = sqr(dx) + sqr(dy);
		line[mod_yi].x += cos(theta) * World.grav[g].force / dist2;
		line[mod_yi].y += sin(theta) * World.grav[g].force / dist2;
	    }
        }
    }
}

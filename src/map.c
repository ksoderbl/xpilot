/* $Id: map.c,v 1.13 1993/04/01 18:17:37 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "global.h"
#include "map.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: map.c,v 1.13 1993/04/01 18:17:37 bjoerns Exp $";
#endif


/*
 * Globals.
 */
World_map World;


u_short Find_closest_team(int indx);


/*
 * Sets as many blocks as possible to FILLED_NO_DRAW.
 * You won't notice any difference. :)
 */
void Optimize_map()
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
    World.x		= 300;
    World.y		= 300;
    World.NumFuels	= 0;
    World.NumBases	= 0;
    World.NumGravs	= 0;
    World.NumCannons	= 0;
    World.NumBases	= 0;
    World.NumWormholes	= 0;
}


void Free_map(void)
{
    int x;

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


void Grok_map()
{
    int i, x, y, c;
    bool done_line = false;
    char *s;

    Init_map();
    
    World.x = mapWidth;
    World.y = mapHeight;
    strcpy(World.name, mapName);
    strcpy(World.author, mapAuthor);
    
    Alloc_map();
    
    x = -1;
    y = World.y - 1;
    
    Set_world_rules();

    if (!mapData) {
	Generate_random_map();
    } else {
	s = mapData;
	while ((c = *s++) && (y >= 0)) {
		
	    x++;
	    done_line = false;

	    if (x >= World.x || c == '\n') {
		x=-1; y--;
		done_line = true;
		if (c != '\n')				/* Get rest of line */
		    while (c != '\n' && c != EOF)	/* from file. */
			putchar(c = *s++);
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
    }

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
		    World.NumCannons++;
		    break;
		case 'd':
		    line[y] = CANNON;
		    World.cannon[World.NumCannons].dir = DIR_LEFT;
		    World.cannon[World.NumCannons].pos.x = x;
		    World.cannon[World.NumCannons].pos.y = y;
		    World.cannon[World.NumCannons].dead_time = 0;
		    World.NumCannons++;
		    break;
		case 'f':
		    line[y] = CANNON;
		    World.cannon[World.NumCannons].dir = DIR_RIGHT;
		    World.cannon[World.NumCannons].pos.x = x;
		    World.cannon[World.NumCannons].pos.y = y;
		    World.cannon[World.NumCannons].dead_time = 0;
		    World.NumCannons++;
		    break;
		case 'c':
		    line[y] = CANNON;
		    World.cannon[World.NumCannons].dir = DIR_DOWN;
		    World.cannon[World.NumCannons].pos.x = x;
		    World.cannon[World.NumCannons].pos.y = y;
		    World.cannon[World.NumCannons].dead_time = 0;
		    World.NumCannons++;
		    break;

		case '#':
		    line[y] = FUEL;
		    World.fuel[World.NumFuels].pos.x = x*BLOCK_SZ+BLOCK_SZ/2;
		    World.fuel[World.NumFuels].pos.y = y*BLOCK_SZ+BLOCK_SZ/2;
		    World.fuel[World.NumFuels].fuel = START_STATION_FUEL;
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
			c = WORM_NORMAL;
			worm_norm++;
		    } else if (c == '(') {
			c = WORM_IN;
			worm_in++;
		    } else {
			c = WORM_OUT;
			worm_out++;
		    }
		    World.wormHoles[World.NumWormholes].type = c;
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
	    u_short team = Find_closest_team(i);
	    if (team == TEAM_NOT_SET) {
		error("Couldn't find a matching team for the treasure.");
		World.treasures[i].have = false;
	    }
	    World.treasures[i].team = team;
	}
    }

    /* Calculating the world's diagonal length */

    World.diagonal = sqrt(World.x*World.x + World.y*World.y);

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

    D( Print_map(); )
}


/*
 * Stupid random map routine, only to be used if the game can't open any
 * map file. :)
 */
void Generate_random_map(void)
{
    int x, y, i = 0;
    

    Init_map();
    Alloc_map();

#ifndef	SILENT
    puts("Creating random map.");
#endif

    strcpy(World.name, "Random Land");
    strcpy(World.author, "The Computer");

    for (x=0; x<World.x; x++) {
        u_byte *line=World.block[x];

        for (y=World.y-1; y >= 0 ; y--) {
	    if ((y==World.y-1) || (y==0) || (x==0) || (x==World.x-1))
		line[y] = 'x';
	    else if (((rand()%20)==0) && ((x==1)||(x==World.x-2))) {
		if (x==1)
		    line[y] = 's';
		else
		    line[y] = 'a';
	    } else
		switch (rand()%11137) {
		case 0:
		case 1:
		    line[y] = '#';
                    World.NumFuels++;
		    break;
		case 10:
		case 11:
		    line[y] = 'x';
		    break;
		case 20:
		    line[y] = 's';
		    break;
		case 30:
		    line[y] = 'a';
		    break;
		case 40:
		    if (rand()%2)
			line[y] = 'w';
		    else
			line[y] = 'q';
		    break;
		case 50:
	            line[y] = '-';
                    World.NumGravs++;
		    break;
                case 51:
	            line[y] = '-';
                    World.NumGravs++;
		    break;
                case 52:
	            line[y] = '<';
                    World.NumGravs++;
		    break;
                case 53:
	            line[y] = '>';
                    World.NumGravs++;
		    break;
                case 60:
	            line[y] = '(';
                    World.NumWormholes++;
		    break;
                case 61:
	            line[y] = ')';
                    World.NumWormholes++;
		    break;
                case 62:
	            line[y] = '@';
                    World.NumWormholes++;
		    break;
                case 80:
                    line[y] = 'r';
                    World.NumCannons++;
                    break;
                case 81:
                    line[y] = 'd';
                    World.NumCannons++;
                    break;
                case 82:
                    line[y] = 'f';
                    World.NumCannons++;
                    break;
                case 83:
                    line[y] = 'c';
                    World.NumCannons++;
                    break;
		default:
		    line[y] = ' ';
		    break;
		}
	}
    }

    for (x=1; x<World.x-1; x++) {
        u_byte *line = World.block[x];

        for (y=1; y<World.y-1; y++) {
	    i = line[y-1];
	    if ((i == 'x' || i == 's' || i == 'a' || i == '#')
		&& (rand()%27) == 0) {

		line[y] = '_';
	    }
	}
    }
    
    for (i='A'; i<='Z'; i++) {
        x = rand() % (World.x-2) + 1;
        y = rand() % (World.y-2) + 1;
        World.block[x][y] = i;
    }
}


/*
 * Find the correct direction of the base, according to the gravity in
 * the base region.
 */
void Find_base_direction(void)
{
    int	i, x, y, dir;

    for (i=0; i<World.NumBases; i++) {
	x = World.base[i].pos.x;
	y = World.base[i].pos.y;
	dir = findDir(-World.gravity[x][y].x, -World.gravity[x][y].y);

	dir = ((dir + RES/8) / (RES/4)) * (RES/4);
	World.base[i].dir = dir;
    }
}


/*
 * Find the team that is closest to this treasure and assign the treasure
 * to this team.
 */
u_short Find_closest_team(int indx)
{
    int i;
    float closest = FLT_MAX, l;
    treasure_t	*t = &World.treasures[indx];

    t->team = TEAM_NOT_SET;
    for (i=0; i<World.NumBases; i++) {
	if (World.base[i].team == TEAM_NOT_SET)
	    continue;
	
	l = LENGTH(t->pos.x - World.base[i].pos.x,
		   t->pos.y - World.base[i].pos.y);

	if (l < closest) {
	    t->team = World.base[i].team;
	    closest = l;
	}
    }

    return t->team;
}

/* map.c,v 1.12 1992/06/28 05:38:17 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "global.h"
#include "map.h"

#ifndef	lint
static char sourceid[] =
    "@(#)map.c,v 1.12 1992/06/28 05:38:17 bjoerns Exp";
#endif


/*
 * Globals.
 */
World_map World;



/*
 * Sets as many blocks as possible to FILLED_NO_DRAW.  You won't notice the
 * difference. :)
 */
void Optimize_map()
{
    int x, y, type;


    for (x=1; x<(World.x-1); x++)
	for (y=1; y<(World.y-1); y++)
	    if (World.block[x][y] == FILLED) {
		type = World.block[x][y-1];
		if ((type != FILLED) && (type != REC_LU) && (type != REC_RU))
		    continue;

		type = World.block[x][y+1];
		if ((type != FILLED) && (type != REC_LD) && (type != REC_RD))
		    continue;

		type = World.block[x-1][y];
		if ((type != FILLED) && (type != REC_RD) && (type != REC_RU))
		    continue;

		type = World.block[x+1][y];
		if ((type != FILLED) && (type != REC_LD) && (type != REC_LU))
		    continue;

		World.block[x][y] = FILLED_NO_DRAW;
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
    World.NumWormholes	= 0;
}



void Free_map(void)
{
    int x;


    if (World.block) {
	for (x=0; x<World.x; x++) {
	    if (World.block[x])
		free(World.block[x]);
	    else
		break;
	}
	free(World.block);
    }
    if (World.gravity) {
	for (x=0; x<World.x; x++) {
	    if (World.gravity[x])
		free(World.gravity[x]);
	    else
		break;
	}
	free(World.gravity);
    }
}



void Alloc_map(void)
{
    int x;


    if (World.block || World.gravity)
	Free_map();

    World.block = (unsigned char **) malloc(sizeof(unsigned char *)*World.x);
    World.gravity = (vector **) malloc(sizeof(vector *)*World.x);

    if (World.block == NULL || World.gravity == NULL) {
    out_of_mem:
	Free_map();
	error("Couldn't allocate memory for map (%d bytes)",
	      (sizeof(unsigned char)+sizeof(vector))*World.y*World.x +
	      (sizeof(unsigned char *)+sizeof(vector *))*World.x);
	exit(-1);
    } else {
	for (x=0; x<World.x; x++) {
	    World.block[x] = (unsigned char *)
				malloc(sizeof(unsigned char)*World.y);
	    World.gravity[x] = (vector *) malloc(sizeof(vector)*World.y);

	    if (World.block[x] == NULL || World.gravity[x] == NULL)
		goto out_of_mem;
	}
    }
}


    
void Load_map(char *map)
{
    FILE *fd;
    int x, y, c;
    char file[256], str[256];
    bool done_line = false;


    /*
     * Initialize map file, append .map suffix if neccessary.
     */
    Init_map();
    strcpy(file, map);
    if (strstr(file, ".map") == NULL)
	strcat(file, ".map");

    /*
     * Check for presence of map file.	If not found, try in MAPDIR.
     */
    if ((fd=fopen(file, "r")) == NULL) {
	strcpy(str, file);
	sprintf(file, "%s%s", MAPDIR, str);
	fd=fopen(file, "r");
    }


    /*
     * Read data from file.
     */
    if (fd != NULL) {

	fscanf(fd, "%dx%d\n", &(World.x), &(World.y));	/* Get dimensions, */

	/*
	 * Allocate memory for map.
	 */
	Alloc_map();

	fgets(World.name, MAX_CHARS, fd);	    /* Temporary use of */
	Set_world_rules(atoi(World.name));	    /* World.name */

	fgets(World.name, MAX_CHARS, fd);	    /* Get name */
	World.name[strlen(World.name)-1]='\0';	    /* Strip newline */
	fgets(World.author, MAX_CHARS, fd);	    /* Get Author */
	World.author[strlen(World.author)-1]='\0';  /* Strip newline */

	x = -1;
	y = World.y-1;

	while (((c=getc(fd)) != EOF) && (y>=0)) {

	    x++;
	    done_line=false;

	    if (x>=World.x || c=='\n') {
		x=-1;
		y--;
		done_line=true;
		if (c!='\n')			    /* Get rest of line */
		    while (c!='\n' && c!=EOF)	    /* from file. */
			putchar(c=getc(fd));
	    }
	    if (done_line)
		continue;

	    switch (c) {
	    case 'x': 
		World.block[x][y] = FILLED;
		break;
	    case ' ':
		World.block[x][y] = SPACE;
		break;
	    case 's': 
		World.block[x][y] = REC_LU;
		break;
	    case 'a': 
		World.block[x][y] = REC_RU;
		break;
	    case 'w': 
		World.block[x][y] = REC_LD;
		break;
	    case 'q': 
		World.block[x][y] = REC_RD;
		break;
	    case 'r':
		World.block[x][y] = CANNON;
		World.cannon[World.NumCannons].dir = DIR_UP;
		World.cannon[World.NumCannons].pos.x = x;
		World.cannon[World.NumCannons].pos.y = y;
		World.cannon[World.NumCannons].dead_time = 0;
		World.NumCannons++;
		break;
	    case 'd':
		World.block[x][y] = CANNON;
		World.cannon[World.NumCannons].dir = DIR_LEFT;
		World.cannon[World.NumCannons].pos.x = x;
		World.cannon[World.NumCannons].pos.y = y;
		World.cannon[World.NumCannons].dead_time = 0;
		World.NumCannons++;
		break;
	    case 'f':
		World.block[x][y] = CANNON;
		World.cannon[World.NumCannons].dir = DIR_RIGHT;
		World.cannon[World.NumCannons].pos.x = x;
		World.cannon[World.NumCannons].pos.y = y;
		World.cannon[World.NumCannons].dead_time = 0;
		World.NumCannons++;
		break;
	    case 'c':
		World.block[x][y] = CANNON;
		World.cannon[World.NumCannons].dir = DIR_DOWN;
		World.cannon[World.NumCannons].pos.x = x;
		World.cannon[World.NumCannons].pos.y = y;
		World.cannon[World.NumCannons].dead_time = 0;
		World.NumCannons++;
		break;
	    case 'F': 
		World.block[x][y] = FUEL;
		World.fuel[World.NumFuels].pos.x = x*BLOCK_SZ+BLOCK_SZ/2;
		World.fuel[World.NumFuels].pos.y = y*BLOCK_SZ+BLOCK_SZ/2;
		World.fuel[World.NumFuels].left = 20.0;
		World.NumFuels++;
		break;
	    case '*': 
		World.block[x][y] = BASE;
		World.base[World.NumBases].x = x;
		World.base[World.NumBases].y = y;
		World.NumBases++;
		break;
	    case '+':
		World.block[x][y] = POS_GRAV;
		World.grav[World.NumGravs].pos.x = x;
		World.grav[World.NumGravs].pos.y = y;
		World.grav[World.NumGravs].force = -GRAVS_POWER;
		World.NumGravs++;
		break;
	    case '-':
		World.block[x][y] = NEG_GRAV;
		World.grav[World.NumGravs].pos.x = x;
		World.grav[World.NumGravs].pos.y = y;
		World.grav[World.NumGravs].force = GRAVS_POWER;
		World.NumGravs++;
		break;
	    case '>':
		World.block[x][y] = CWISE_GRAV;
		World.grav[World.NumGravs].pos.x = x;
		World.grav[World.NumGravs].pos.y = y;
		World.grav[World.NumGravs].force = GRAVS_POWER;
		World.NumGravs++;
		break;
	    case '<':
		World.block[x][y] = ACWISE_GRAV;
		World.grav[World.NumGravs].pos.x = x;
		World.grav[World.NumGravs].pos.y = y;
		World.grav[World.NumGravs].force = -GRAVS_POWER;
		World.NumGravs++;
		break;
	    case 'W':
		World.block[x][y] = WORMHOLE;
		World.wormhole[World.NumWormholes].x = x;
		World.wormhole[World.NumWormholes].y = y;
		World.NumWormholes++;
		break;
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
		if (BIT(World.rules->mode, TIMING)) {
		    World.check[c-'0'].x = x;
		    World.check[c-'0'].y = y;
		    World.NumChecks++;
		    World.block[x][y] = CHECK;
		} else World.block[x][y] = SPACE;
		break;

	    default:
		World.block[x][y] = SPACE;
		break;
	    }
	}

#ifndef	SILENT
	printf("World can take a maximum of %d players.\n",
	       World.NumBases);
#endif

	fclose(fd);

    } else {
	error(file);
	Generate_random_map();
    }

    if (World.NumWormholes == 1) {
	error("You're not allowed to have only 1 wormhole, removing it");
	World.NumWormholes = 0;
	for (x=0; x<World.x; x++)
	    for (y=0; y<World.y; y++)
		if (World.block[x][y] == WORMHOLE)
		    World.block[x][y] = SPACE;
    }

    Optimize_map();

    if (WantedNumRobots == -1)
	WantedNumRobots = (World.NumBases / 2);
    if (BIT(World.rules->mode, TIMING))
	WantedNumRobots = 0;

D(  Print_map();    )
}



/*
 * Stupid routine that generates a random map if the game can't open any
 * map file.  This should be the last choice.
 */
void Generate_random_map(void)
{
    int x, y, i=0;
    

    Init_map();
    Alloc_map();

#ifndef	SILENT
    puts("Creating random map.");
#endif

    Set_world_rules(0);
    strcpy(World.name, "Random map");
    strcpy(World.author, "The Computer");

    for (y=World.y-1; y >= 0 ; y--)
	for (x=0; x<World.x; x++) {
	    if ((y==World.y-1) || (y==0) || (x==0) || (x==World.x-1)) 
		World.block[x][y] = FILLED;
	    else if (((rand()%20)==0) && ((x==1)||(x==World.x-2))) {
		if (x==1)
		    World.block[x][y] = REC_LU;
		else
		    World.block[x][y] = REC_RU;
	    }
	    else
		switch (rand()%11137) {
		case 0:
		case 1:
		    World.block[x][y] = FUEL;
		    World.fuel[World.NumFuels].pos.x =
			x*BLOCK_SZ+BLOCK_SZ/2;
		    World.fuel[World.NumFuels].pos.y =
			y*BLOCK_SZ+BLOCK_SZ/2;
		    World.fuel[World.NumFuels].left = 20.0;
		    World.NumFuels++;
		    break;
		case 10:
		case 11:
		    World.block[x][y] = FILLED;
		    break;
		case 20:
		    World.block[x][y] = REC_LU;
		    break;
		case 30:
		    World.block[x][y] = REC_RU;
		    break;
		case 40:
		    if (rand()%2)
			World.block[x][y] = REC_LD;
		    else
			World.block[x][y] = REC_RD;
		    break;
		case 50:
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.NumGravs++;
		    if (rand()%2) {
			World.grav[World.NumGravs].force = -GRAVS_POWER;
			World.block[x][y] = NEG_GRAV;
		    } else {
			World.grav[World.NumGravs].force = GRAVS_POWER;
			World.block[x][y] = POS_GRAV;
		    }
		    break;
		default:
		    World.block[x][y] = SPACE;
		    break;
		}
	}

    for (x=1; x<World.x-1; x++)
	for (y=World.y-1; y>1; y--) {
	    i = World.block[x][y-1];
	    if ((i == FILLED || i == REC_LU || i == REC_RU || i == FUEL)
		&& (rand()%27) == 0) {

		World.block[x][y] = BASE;
		World.base[World.NumBases].x = x;
		World.base[World.NumBases].y = y;
		World.NumBases++;
	    }
	}
}

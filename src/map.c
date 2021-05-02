/* map.c,v 1.3 1992/05/11 15:31:18 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include "pilot.h"
#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

extern void Set_world_rules(int);

extern wireobj ships[];

void Print_map(void);
void Generate_random_map(void);

World_map World;



void Optimize_map()
{
    int x, y, type;


    for (x=1; x<(World.x-1); x++)
	for (y=1; y<(World.y-1); y++)
	    if (World.type[x][y]==FILLED) {
		type=World.type[x][y-1];
		if ((type!=FILLED) && (type!=REC_LU) && (type!=REC_RU))
		    continue;

		type=World.type[x][y+1];
		if ((type!=FILLED) && (type!=REC_LD) && (type!=REC_RD))
		    continue;

		type=World.type[x-1][y];
		if ((type!=FILLED) && (type!=REC_RD) && (type!=REC_RU))
		    continue;

		type=World.type[x+1][y];
		if ((type!=FILLED) && (type!=REC_LD) && (type!=REC_LU))
		    continue;

		World.type[x][y]=FILLED_NO_DRAW;
	    }
}



void Print_map(void)			/* Debugging only. */
{
    int x, y;

    for (y=World.y-1; y>=0; y--) {
	for (x=0; x<World.x; x++)
	    switch (World.type[x][y]) {
	    case SPACE:
		putchar(' ');
		break;
	    case PORT:
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
    World.x = 300; World.y = 300;
    World.Ant_fuel=0;
    World.Ant_start=0;
    World.Ant_gravs=0;
    World.Ant_cannon=0;
}



void Free_map(void)
{
    int x;


    if (World.type) {
	for (x=0; x<World.x; x++) {
	    if (World.type[x])
		free(World.type[x]);
	    else
		break;
	}
	free(World.type);
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


    if (World.type || World.gravity)
	Free_map();

    World.type = malloc(sizeof(unsigned char *)*World.x);
    World.gravity = malloc(sizeof(vector *)*World.x);

    if (World.type==NULL || World.gravity==NULL) {
    no_more_mem:
	Free_map();
	fprintf(stderr, "XPilots: Couldn't allocate memory for "
		"map (%d bytes).\n",
		(sizeof(unsigned char)+sizeof(vector))*World.y*World.x +
		(sizeof(unsigned char *)+sizeof(vector *))*World.x);
	exit (-1);
    } else {
	for (x=0; x<World.x; x++) {
	    World.type[x] = malloc(sizeof(unsigned char)*World.y);
	    World.gravity[x] = malloc(sizeof(vector)*World.y);

	    if (World.type[x]==NULL || World.gravity[x]==NULL)
		goto no_more_mem;
	}
    }
}


    
void Load_map(char *map)
{
    FILE *fd;
    int x, y, c;
    char file[256], str[256];
    bool done_line = False;


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
		World.type[x][y] = FILLED;
		break;
	    case ' ':
		World.type[x][y] = SPACE;
		break;
	    case 's': 
		World.type[x][y] = REC_LU;
		break;
	    case 'a': 
		World.type[x][y] = REC_RU;
		break;
	    case 'w': 
		World.type[x][y] = REC_LD;
		break;
	    case 'q': 
		World.type[x][y] = REC_RD;
		break;
	    case 'r':
		World.type[x][y] = CANNON;
		World.cannon[World.Ant_cannon].dir = UP;
		World.cannon[World.Ant_cannon].pos.x = x;
		World.cannon[World.Ant_cannon].pos.y = y;
		World.cannon[World.Ant_cannon].dead_time = 0;
		World.Ant_cannon++;
		break;
	    case 'd':
		World.type[x][y] = CANNON;
		World.cannon[World.Ant_cannon].dir = LEFT;
		World.cannon[World.Ant_cannon].pos.x = x;
		World.cannon[World.Ant_cannon].pos.y = y;
		World.cannon[World.Ant_cannon].dead_time = 0;
		World.Ant_cannon++;
		break;
	    case 'f':
		World.type[x][y] = CANNON;
		World.cannon[World.Ant_cannon].dir = RIGHT;
		World.cannon[World.Ant_cannon].pos.x = x;
		World.cannon[World.Ant_cannon].pos.y = y;
		World.cannon[World.Ant_cannon].dead_time = 0;
		World.Ant_cannon++;
		break;
	    case 'c':
		World.type[x][y] = CANNON;
		World.cannon[World.Ant_cannon].dir = DOWN;
		World.cannon[World.Ant_cannon].pos.x = x;
		World.cannon[World.Ant_cannon].pos.y = y;
		World.cannon[World.Ant_cannon].dead_time = 0;
		World.Ant_cannon++;
		break;
	    case 'F': 
		World.type[x][y] = FUEL;
		World.fuel[World.Ant_fuel].pos.x = x*WORLD_SPACE+WORLD_SPACE/2;
		World.fuel[World.Ant_fuel].pos.y = y*WORLD_SPACE+WORLD_SPACE/2;
		World.fuel[World.Ant_fuel].left = 20.0;
		World.Ant_fuel++;
		break;
	    case '*': 
		World.type[x][y] = PORT;
		World.Start_points[World.Ant_start].x = x;
		World.Start_points[World.Ant_start].y = y;
		World.Ant_start++;
		break;
	    case '+':
		World.type[x][y] = POS_GRAV;
		World.gravs[World.Ant_gravs].pos.x = x;
		World.gravs[World.Ant_gravs].pos.y = y;
		World.gravs[World.Ant_gravs].force = -GRAVS_POWER;
		World.Ant_gravs++;
		break;
	    case '-':
		World.type[x][y] = NEG_GRAV;
		World.gravs[World.Ant_gravs].pos.x = x;
		World.gravs[World.Ant_gravs].pos.y = y;
		World.gravs[World.Ant_gravs].force = GRAVS_POWER;
		World.Ant_gravs++;
		break;
	    case '>':
		World.type[x][y] = CWISE_GRAV;
		World.gravs[World.Ant_gravs].pos.x = x;
		World.gravs[World.Ant_gravs].pos.y = y;
		World.gravs[World.Ant_gravs].force = GRAVS_POWER;
		World.Ant_gravs++;
		break;
	    case '<':
		World.type[x][y] = ACWISE_GRAV;
		World.gravs[World.Ant_gravs].pos.x = x;
		World.gravs[World.Ant_gravs].pos.y = y;
		World.gravs[World.Ant_gravs].force = -GRAVS_POWER;
		World.Ant_gravs++;
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
		    World.Ant_check++;
		    World.type[x][y] = CHECK;
		} else World.type[x][y] = SPACE;
		break;

	    default:
		World.type[x][y] = SPACE;
		break;
	    }
	}

	printf("World can take a maximum of %d players.\n", World.Ant_start);

	fclose(fd);

    } else {
	perror("Map file");
	Generate_random_map();
    }

    Optimize_map();

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

    printf("Creating random map.\n");

    Set_world_rules(0);
    strcpy(World.name, "Random map");
    strcpy(World.author, "The Computer");

    for (y=World.y-1; y >= 0 ; y--)
	for (x=0; x<World.x; x++) {
	    if ((y==World.y-1) || (y==0) || (x==0) || (x==World.x-1)) 
		World.type[x][y] = FILLED;
	    else if (((rand()%20)==0) && ((x==1)||(x==World.x-2))) {
		if (x==1)
		    World.type[x][y] = REC_LU;
		else
		    World.type[x][y] = REC_RU;
	    }
	    else
		switch (rand()%11137) {
		case 0:
		case 1:
		    World.type[x][y] = FUEL;
		    World.fuel[World.Ant_fuel].pos.x =
			x*WORLD_SPACE+WORLD_SPACE/2;
		    World.fuel[World.Ant_fuel].pos.y =
			y*WORLD_SPACE+WORLD_SPACE/2;
		    World.fuel[World.Ant_fuel].left = 20.0;
		    World.Ant_fuel++;
		    break;
		case 10:
		case 11:
		    World.type[x][y] = FILLED;
		    break;
		case 20:
		    World.type[x][y] = REC_LU;
		    break;
		case 30:
		    World.type[x][y] = REC_RU;
		    break;
		case 40:
		    if (rand()%2)
			World.type[x][y] = REC_LD;
		    else
			World.type[x][y] = REC_RD;
		    break;
		case 50:
		    World.gravs[World.Ant_gravs].pos.x = x;
		    World.gravs[World.Ant_gravs].pos.y = y;
		    World.Ant_gravs++;
		    if (rand()%2) {
			World.gravs[World.Ant_gravs].force = -GRAVS_POWER;
			World.type[x][y] = NEG_GRAV;
		    } else {
			World.gravs[World.Ant_gravs].force = GRAVS_POWER;
			World.type[x][y] = POS_GRAV;
		    }
		    break;
		default:
		    World.type[x][y] = SPACE;
		    break;
		}
	}

    for (x=1; x<World.x-1; x++)
	for (y=World.y-1; y>1; y--) {
	    i = World.type[x][y-1];
	    if (((i==FILLED) || (i==REC_LU) || (i==REC_RU) || (i==FUEL)) &&
		((rand()%27)==0)) {
		World.type[x][y] = PORT;
		World.Start_points[World.Ant_start].x = x;
		World.Start_points[World.Ant_start].y = y;
		World.Ant_start++;
	    }
	}
}

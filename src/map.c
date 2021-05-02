/* $Id: map.c,v 1.16 1992/08/27 00:25:58 bjoerns Exp $
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
#include <sys/file.h>
#include <sys/stat.h>

#include "global.h"
#include "map.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: map.c,v 1.16 1992/08/27 00:25:58 bjoerns Exp $";
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


    for (x=1; x<(World.x-1); x++) {
        u_byte *line=World.block[x];
        u_byte *n_line=World.block[x+1];
        u_byte *p_line=World.block[x-1];

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
    World.NumBlasters	= 0;
    World.NumWormholes	= 0;
}



void Free_map(void)
{
    int x;


    if (World.block) free(World.block);
    if (World.gravity) free(World.gravity);
    if (World.grav) free(World.grav);
    if (World.blaster) free(World.blaster);
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
        (unsigned char **) malloc(  sizeof(unsigned char *)*World.x
                                  + World.x*sizeof(unsigned char)*World.y
                                 );
    World.gravity = 
        (vector **) malloc(  sizeof(vector *)*World.x
                           + World.x*sizeof(vector)*World.y
                          );
    World.grav=0;
    World.blaster=0;
    World.cannon=0;
    World.fuel=0;
    World.wormHoles=0;
    if (World.block == NULL || World.gravity == NULL) {
	Free_map();
	error("Couldn't allocate memory for map (%d bytes)",
                World.x
              * (  World.y*(sizeof(unsigned char)+sizeof(vector))
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

static int crash_blasters_free=CRASH_BLASTERS;

void add_blaster(int x,int y)
{
    u_byte *t;
    
    if (x<0 || y<0 || x>=World.x || y>=World.y) return;
    t = &(World.block[x][y]);
    if (*t==BLASTER || crash_blasters_free) {
        blaster_t *b = &(World.blaster[World.NumBlasters++]);
        
        b->pos.x = x;
        b->pos.y = y;
        if ((b->orig = *t)==BLASTER) {
            b->chance = DUST_CHANCE;
            b->add_size = DUST_ADD;
            b->base_size = DUST_BASE;
            b->speed_fact = DUST_SPEED_FACT;
            b->life = DUST_LIFE_FACT;
        } else {
            b->chance = W_DUST_CHANCE;
            b->add_size = W_DUST_ADD;
            b->base_size = W_DUST_BASE;
            b->speed_fact = W_DUST_SPEED_FACT;
            b->life = W_DUST_LIFE_FACT;
            crash_blasters_free--;
            if (x>0 && World.block[x-1][y]==FILLED_NO_DRAW)
                 World.block[x-1][y]=FILLED;
            if (x<World.x-1 && World.block[x+1][y]==FILLED_NO_DRAW)
                World.block[x+1][y]=FILLED;
            if (y>0 && World.block[x][y-1]==FILLED_NO_DRAW)
                World.block[x][y-1]=FILLED;
            if (y<World.y-1 && World.block[x][y+1]==FILLED_NO_DRAW)
                World.block[x][y+1]=FILLED;
            if (b->orig==FUEL) {
                int i,j;
                double xf = x*BLOCK_SZ+BLOCK_SZ/2;
                double yf = y*BLOCK_SZ+BLOCK_SZ/2;

                for (i=0;i<World.NumFuels;i++)
                    if (World.fuel[i].pos.x==xf && World.fuel[i].pos.y==yf) {
                        b->fuel=i;
                        World.fuel[i].fuel=0;
                        for (j=0;j<NumPlayers;j++)
                            if (   BIT(Players[j]->used, OBJ_REFUEL)
                                && Players[j]->fs==i)
                                CLR_BIT(Players[j]->used,OBJ_REFUEL);
                        break;
                    }
            }
        }
        *t = SPACE;
        b->runtime = 0;
   }
}

void remove_crash_blaster(int i)
{
    blaster_t *b = &(World.blaster[i]);

    if (b->orig!=BLASTER) {
        if ((World.block[b->pos.x][b->pos.y]=b->orig)==FUEL)
            World.fuel[b->fuel].fuel=0;
        *b = World.blaster[--World.NumBlasters];
        crash_blasters_free++;
    }
}


void Load_map(char *map)
{
    FILE *fd;
    int x, y, c;
    char file[256], str[256];
    bool done_line = false;
    int popen_flag = 0;

    /*
     * Initialize map file, append .map suffix if neccessary.
     */
    Init_map();
    strcpy(file, map);
    if (strstr(file, ".map") == NULL)
	strcat(file, ".map");

    /*
     * Check for presence of map file.	If not found, try in MAPDIR.
     * Also, try reading compressed map files (wiml@u.washington.edu)
     */
    fd = fopen(file, "r");
#ifdef COMPRESSED_MAPS
    if(!fd) {
      strcpy(str, file);
      strcat(str,  ".Z");
      if(!access(str, R_OK)) {
	popen_flag = 1;
	strcpy(file, str);
	sprintf(str, ZCAT_FORMAT, file);
	fd = popen(str, "r");
      }
    }
#endif
    if(!fd) {
      strcpy(str, MAPDIR);
      strcat(str, file);
      if(fd = fopen(str, "r")) strcpy(file, str);
#ifdef COMPRESSED_MAPS
      if(!fd) {
	strcat(str, ".Z");
	if(!access(str, R_OK)) {
	  popen_flag = 1;
	  strcpy(file, str);
	  sprintf(str, ZCAT_FORMAT, file);
	  fd = popen(str, "r");
	}
      }
#endif
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

	    switch (World.block[x][y]=c) {
	    case '@':
		World.NumBlasters++;
		break;
	    case 'r':
		World.NumCannons++;
		break;
	    case 'd':
		World.NumCannons++;
		break;
	    case 'f':
		World.NumCannons++;
		break;
	    case 'c':
		World.NumCannons++;
		break;
	    case 'F': 
		World.NumFuels++;
		break;
	    case '*': 
		World.NumBases++;
		break;
	    case '+':
		World.NumGravs++;
		break;
	    case '-':
		World.NumGravs++;
		break;
	    case '>':
		World.NumGravs++;
		break;
	    case '<':
		World.NumGravs++;
		break;
	    case 'W':
	    case 'I':
	    case 'O':
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
		if (BIT(World.rules->mode, TIMING))
		    World.NumChecks++;
		break;
	    default:
		break;
	    }
	}

	if(popen_flag)
	  pclose(fd);
	else
	  fclose(fd);

    } else {
	error(file);
	Generate_random_map();
    }

    /* get space for speciel objects. */
      /* check number of bases */
        if (World.NumBases>MAX_BASES) {
            fprintf(stderr,
                    "To many bases, used %d, maximum is %d\n",
                    World.NumBases,MAX_BASES);
          map_read_error:
            exit(1);
        } else
            World.NumBases=0;
      /* get space for blasters */
        if ( !(World.blaster=
                 (blaster_t*)malloc( (World.NumBlasters+CRASH_BLASTERS)
                                    *sizeof(blaster_t)))
        ) {
          no_obj_memory:
            fprintf(stderr,"no space left for objects.\n");
            goto map_read_error;
        } else
            World.NumBlasters=0;
      /* get space for cannons */
        if (   World.NumCannons
            && !(World.cannon=
                   (cannon_t*)malloc(World.NumCannons*sizeof(cannon_t)))
        ) {
            goto no_obj_memory;
        } else
            World.NumCannons=0;
      /* get space for fuel */
        if (   World.NumFuels
            && !(World.fuel=
                   (fuel_t*)malloc(World.NumFuels*sizeof(fuel_t)))
        ) {
            goto no_obj_memory;
        } else
            World.NumFuels=0;
      /* get space for gravs */
        if (   World.NumGravs
            && !(World.grav=
                   (grav_t*)malloc(World.NumGravs*sizeof(grav_t)))
        ) {
            goto no_obj_memory;
        } else
            World.NumGravs=0;
      /* get space for wormholes */
        if (   World.NumWormholes
            && !(World.wormHoles=
                   (wormhole_t*)malloc(World.NumWormholes*sizeof(wormhole_t)))
        ) {
            goto no_obj_memory;
        } else
            World.NumWormholes=0;
    
    /* change read tags to internal data, create objects */
    {   int worm_in=0;
        int worm_out=0;
        int worm_norm=0;
    
        for (x=0;x<World.x;x++) {
            u_byte *line=World.block[x];
    
            for (y=0;y<World.y;y++)
                switch (line[y]) {
                case 'x':
                    line[y] = FILLED;
                    break;
                    case ' ':
                    line[y] = SPACE;
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
                case '@':
                    line[y] = BLASTER;
                    add_blaster(x,y);
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
                case 'F':
                    line[y] = FUEL;
                    World.fuel[World.NumFuels].pos.x = x*BLOCK_SZ+BLOCK_SZ/2;
                    World.fuel[World.NumFuels].pos.y = y*BLOCK_SZ+BLOCK_SZ/2;
                    World.fuel[World.NumFuels].fuel = START_STATION_FUEL;
                    World.NumFuels++;
                    break;
                case '*':
                    line[y] = BASE;
                    World.base[World.NumBases].x = x;
                    World.base[World.NumBases].y = y;
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
                case 'W':
                case 'I':
                case 'O': {
                    u_byte c=line[y];
    
                    World.wormHoles[World.NumWormholes].pos.x = x;
                    World.wormHoles[World.NumWormholes].pos.y = y;
                    World.wormHoles[World.NumWormholes].countdown = 0;
                    if (c=='W') {
                        c=WORM_NORMAL;
                        worm_norm++;
                    } else if (c=='I') {
                        c=WORM_IN;
                        worm_in++;
                    } else {
                        c=WORM_OUT;
                        worm_out++;
                    }
                    World.wormHoles[World.NumWormholes].type = c;
                    line[y] = WORMHOLE;
                    World.NumWormholes++;
                    break;
                }
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
                        line[y] = CHECK;
                    } else
                        line[y] = SPACE;
                    break;
    
            default:
                line[y] = SPACE;
                break;
                    
            }
        }
    
        if (!worm_out && (worm_norm==1 || (!worm_norm && worm_in))) {
            int i;
            
            error("Inconsistent use of wormholes, removing them");
            for (i=0;i<World.NumWormholes;i++)
                World.block[World.wormHoles[i].pos.x]
                           [World.wormHoles[i].pos.y]=SPACE;
            World.NumWormholes = 0;
        }
    }

    Optimize_map();

    if (WantedNumRobots == -1)
	WantedNumRobots = (World.NumBases / 2);
    if (BIT(World.rules->mode, TIMING))
	WantedNumRobots = 0;

#ifndef	SILENT
	printf("World: %s\nMapsize: %dx%d\nBases: %d\n",
               World.name,
               World.x,
               World.y,
	       World.NumBases);
#endif

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
	    }
	    else
		switch (rand()%11137) {
		case 0:
		case 1:
		    line[y] = 'F';
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
	            line[y] = 'I';
                    World.NumWormholes++;
		    break;
                case 61:
	            line[y] = 'O';
                    World.NumWormholes++;
		    break;
                case 62:
	            line[y] = 'W';
                    World.NumWormholes++;
		    break;
                case 70:
	            line[y] = '@';
                    World.NumBlasters++;
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
        u_byte *line=World.block[x];

        for (y=1; y<World.y-1; y++) {
	    i = line[y-1];
	    if ((i == 'x' || i == 's' || i == 'a' || i == 'F')
		&& (rand()%27) == 0) {

		line[y] = '*';
	    }
	}
    }
    
    for (i=0;i<10;i++) {
        x=rand()%(World.x-2)+1;
        y=rand()%(World.y-2)+1;
        World.block[x][y]="0123456789"[i];
    }
}

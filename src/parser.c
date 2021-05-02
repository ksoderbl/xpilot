/* parser.c,v 1.3 1992/05/11 15:31:24 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <stdlib.h>
#include "map.h"
#include "pilot.h"

double		Gravity = -0.14;	/* Power of gravity */
double		Ship_Mass = 20.0;	/* Default mass of ship */
double		Shots_Mass = 0.1;	/* Default mass of shots */
double		Shots_Speed = 21.0;	/* Default speed of shots */
int		Shots_Life = 90;	/* Default number of ticks */
					/* each shot will live */
int		Shots_Max = 256;	/* Max shots pr. player */
bool		Shots_Gravity = true;	/* Shots affected by gravity */
bool		Loose_Mass = false;	/* Loose mass when firering */

extern void Load_map(char *);



void Parser(int argc, char *argv[])
{
    int i;
    bool map_loaded = false;


    for (i=1; i<argc; i++) {

	if (strcmp("-map", argv[i]) == 0) {
	    Load_map(argv[++i]);
	    map_loaded = true;
	    continue;
	}

	if (strcmp("-gravity", argv[i]) == 0) {
	    printf("Gravity is now %lf m/s^2.\n", 
		   Gravity=atof(argv[++i]));
	    continue;
	}

	if (strcmp("-shots", argv[i]) == 0) {
	    printf("Max shots from players is now %d.\n",
		   Shots_Max=atoi(argv[++i]));
	    continue;
	}

	if (strcmp("-shotmass", argv[i]) == 0) {
	    printf("Shotmass is now %lf.\n",
		   Shots_Mass=atof(argv[++i]));
	    continue;
	}

	if (strcmp("-shipmass", argv[i]) == 0) {
	    printf("Shipmass is now %lf.\n",
		   Ship_Mass=atof(argv[++i]));
	    continue;
	}

	if (strcmp("-shotlife", argv[i]) == 0) {
	    printf("Shots now live for %d ticks.\n",
		   Shots_Life=atoi(argv[++i]));
	    continue;
	}

	if (strcmp("-shotspeed", argv[i]) == 0) {
	    printf("Shotspeed is now %lf.\n",
		   Shots_Speed=atof(argv[++i]));
	    continue;
	}

	if (strcmp("-shotgravity", argv[i]) == 0) {
	    printf("Shotgravity is now off.\n");
	    Shots_Gravity=false;
	    continue;
	}

	if (strcmp("+shotgravity", argv[i]) == 0) {
	    printf("Shotgravity is now on.\n");
	    Shots_Gravity=true;
	    continue;
	}

	if (strcmp("-loosemass", argv[i]) == 0) {
	    printf("Player will loose mass when firering.\n");
	    Loose_Mass=true;
	    continue;
	}

	if (strcmp("+loosemass", argv[i]) == 0) {
	    printf("Player will not loose mass when firering.\n");
	    Loose_Mass=false;
	    continue;
	}

	if (strcmp("-noshield", argv[i]) == 0) {
	    printf("Ship's shield is now disabled.\n");
	    continue;
	}

	printf("Unkown option \"%s\".\n",
	       argv[i]);
    }

    if (!map_loaded) {
	warn("Map not specified, trying to open standard map.\n");
	Load_map(DEFAULT_MAP);
    }
}

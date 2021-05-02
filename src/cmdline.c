/* cmdline.c,v 1.5 1992/06/28 05:38:05 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <stdlib.h>
#include "global.h"
#include "robot.h"
#include "map.h"

#ifndef	lint
static char sourceid[] =
    "@(#)cmdline.c,v 1.5 1992/06/28 05:38:05 bjoerns Exp";
#endif

double		Gravity = -0.14;	/* Power of gravity */
double		ShipMass = 20.0;	/* Default mass of ship */
double		ShotsMass = 0.1;	/* Default mass of shots */
double		ShotsSpeed = 21.0;	/* Default speed of shots */
int		ShotsLife = 90;		/* Default number of ticks */
					/* each shot will live */
int		WantedNumRobots = -1;	/* How many robots should enter */
					/* the game? */
int		ShotsMax = 256;		/* Max shots pr. player */
bool		ShotsGravity = true;	/* Shots affected by gravity */
bool		LooseMass = false;	/* Loose mass when firering */



void Parser(int argc, char *argv[])
{
    int i;
    bool map_loaded = false;


    for (i=1; i<argc; i++) {

	if (strncmp("-help", argv[i], 2) == 0) {
	    printf("Usage:	%s [-options ..]\n\n"
		   "Where options include:\n"
		   "	-help			print out this message\n"
		   "	-version		print out current version\n"
		   "	-map <file>		use alternative map\n"
		   "	-gravity <real>		sets gravity power\n"
		   "	-shots <int>		new maximum shot limit\n"
		   "	-shotspeed <real>	default speed of shots\n"
		   "	-shotmass <real>	mass of players' shots\n"
		   "	-shotlife <ticks>	how long shots should last\n"
		   "	-shipmass <real>	mass of players' ship\n"
		   "	-robots <int>		number of robots\n",
		   argv[0]);
	    exit(0);
	}

	if (strncmp("-version", argv[i], 2) == 0) {
	    puts(TITLE);
	    exit(0);
	}

	if (strcmp("-robots", argv[i]) == 0) {
	    WantedNumRobots = atoi(argv[++i]);
	    if (WantedNumRobots < 0)
		WantedNumRobots = INT_MAX;
	    continue;
	}

	if (strcmp("-map", argv[i]) == 0) {
	    Load_map(argv[++i]);
	    map_loaded = true;
	    continue;
	}

	if (strcmp("-gravity", argv[i]) == 0) {
	    Gravity = atof(argv[++i]);
	    continue;
	}

	if (strcmp("-shots", argv[i]) == 0) {
	    ShotsMax = atoi(argv[++i]);
	    continue;
	}

	if (strcmp("-shotmass", argv[i]) == 0) {
	    ShotsMass = atof(argv[++i]);
	    continue;
	}

	if (strcmp("-shipmass", argv[i]) == 0) {
	    ShipMass = atof(argv[++i]);
	    continue;
	}

	if (strcmp("-shotlife", argv[i]) == 0) {
	    ShotsLife = atoi(argv[++i]);
	    continue;
	}

	if (strcmp("-shotspeed", argv[i]) == 0) {
	    ShotsSpeed = atof(argv[++i]);
	    continue;
	}

	printf("Unkown option \"%s\".\n", argv[i]);
    }

    if (!map_loaded) {
#ifndef SILENT
	printf("Map not specified, trying to open " DEFAULT_MAP ".\n");
#endif
	Load_map(DEFAULT_MAP);
    }
}

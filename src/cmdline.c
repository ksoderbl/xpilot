/* $Id: cmdline.c,v 1.17 1993/04/18 17:10:59 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 *
 *	Options parsing code contributed by Ted Lemon <mellon@ncd.com>
 */

#include <stdlib.h>
#include "global.h"
#include "robot.h"
#include "map.h"
#include "defaults.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: cmdline.c,v 1.17 1993/04/18 17:10:59 bjoerns Exp $";
#endif

float		Gravity;		/* Power of gravity */
float		ShipMass;		/* Default mass of ship */
float		ShotsMass;		/* Default mass of shots */
float		ShotsSpeed;		/* Default speed of shots */
int		ShotsLife;		/* Default number of ticks */
					/* each shot will live */
int		WantedNumRobots;	/* How many robots should enter */
					/* the game? */
int		ShotsMax;		/* Max shots pr. player */
bool		ShotsGravity;		/* Shots affected by gravity */
bool		LooseMass;		/* Loose mass when firering */

bool		RawMode;		/* Let robots live even if there */
					/* are no players logged in */
bool		NoQuit;			/* Don't quit even if there are */
					/* no human players playing */
char		*mapFileName;		/* Name of mapfile... */
char		*mapData;		/* Raw map data... */
int		mapWidth;		/* Width of the universe */
int		mapHeight;		/* Height of the universe */
char		*mapName;		/* Name of the universe */
char		*mapAuthor;		/* Name of the creator */

bool		crashWithPlayer;	/* Can a player hit another player? */
bool		playerKillings;		/* Can players kill each other? */
bool		playerShielding;	/* Can players use shields? */
bool		limitedVisibility;	/* Is visibility limited? */
bool		limitedLives;		/* Are lives limited? */
int		worldLives;		/* If so, what's the max? */
bool		teamPlay;		/* Are teams allowed? */
bool		onePlayerOnly;		/* Can there be multiple players? */
bool		timing;			/* Is this a race? */
bool		edgeWrap;		/* Do objects wrap when they cross
					   the edge of the Universe? */
bool		extraBorder;		/* Give map an extra border? */
ipos		gravityPoint;		/* Where does gravity originate? */
float		gravityAngle;		/* If gravity is along a uniform line,
					   at what angle is that line? */
bool		gravityPointSource;	/* Is gravity a point source? */
bool		gravityClockwise;	/* If so, is it clockwise? */
bool		gravityAnticlockwise;	/* If not clockwise, anticlockwise? */
char		*defaultsFileName;	/* Name of defaults file... */

int 		MovingItemsRand;	/* Probability for moving items */
int		ThrowItemOnKillRand;	/* Probability for players items to */
					/* drop upon beeing killed */

float 		movingItemProb;		/* Probability for moving items */
float		dropItemOnKillProb;	/* Probability for players items to */
					/* drop when player is killed */
float 		itemEnergyPackProb;
float 		itemTankProb;
float		itemECMProb;
float		itemMineProb;
float 		itemMissileProb;
float		itemCloakProb;
float		itemSensorProb;
float		itemWideangleProb;
float		itemRearshotProb;
float		itemAfterburnerProb;
float		maxItemDensity;
bool		allowNukes;

static optionDesc options[] = {
  { "help", "help", "Print out this message", "0", NULL, valInt },
  { "version", "version", "Print version information", "0", NULL, valInt },
  { "gravity", "gravity", "Gravity strength", "-0.14", &Gravity, valReal },
  { "shipMass", "shipmass", "Mass of fighters", "20.0", &ShipMass, valReal },
  { "shotMass", "shotmass", "Mass of bullets", "0.1", &ShotsMass, valReal },
  { "shotSpeed", "shotspeed", "Maximum speed of bullets", "21.0",
	&ShotsSpeed, valReal },
  { "shotLife", "shotlife", "Life of bullets in ticks",
	"90", &ShotsLife, valInt },
  { "maxRobots", "robots", "How many robots do you want?", 
	"4", &WantedNumRobots, valInt },
  { "maxPlayerShots", "shots", "Maximum bullets present at one time",
	"256", &ShotsMax, valInt },
  { "shotsGravity", "shotsGravity", "Are bullets afflicted by gravity",
	"true", &ShotsGravity, valBool },
  { "idleRun", "rawMode",
	"The robots keep playing even if all human players quit",
	"false", &RawMode, valBool },
  { "noQuit", "noquit", 
	"The server keeps running even if all players have left",
	"false", &NoQuit, valBool },
  { "mapWidth", "mapWidth", "Width of world", "100", &mapWidth, valInt },
  { "mapHeight", "mapHeight", "Height of world", "100", &mapHeight, valInt },
  { "mapFileName", "map", "Filename of map to use", DEFAULT_MAP, 
	&mapFileName, valString },
  { "mapName", "mapName", "Name of the map",
	"unknown", &mapName, valString },
  { "mapAuthor", "mapAuthor", "Name of map's author",
	"anonymous", &mapAuthor, valString },
  { "mapData", "mapData", "The map's topology",
	"", &mapData, valString },
  { "crashWithPlayer", "crashWithPlayer", 
	"Should crashes between players be allowed?",
	"yes", &crashWithPlayer, valBool },
  { "playerKillings", "playerKillings",
	"????????", "yes", &playerKillings, valBool },
  { "playerShielding", "playerShielding", "Should we allow shields",
	"yes", &playerShielding, valBool },
  { "limitedVisibility", "limitedVisibility",
	"Should the players have a limited visibility?",
      "no", &limitedVisibility, valBool },
  { "limitedLives", "limitedLives", "Limited lives",
      "no", &limitedLives, valBool },
  { "worldLives", "lives",
	"Number of lives each player has (no sense without limitedLives)",
	"0", &worldLives, valInt },
  { "teamPlay", "teamMode", "Should we allow team play?",
	"no", &teamPlay, valBool },
  { "onePlayerOnly", "onePlayerOnly", "One player modus",
	"no", &onePlayerOnly, valBool },
  { "timing", "timing", "Race mode", "no", &timing, valBool },
  { "edgeWrap", "edgeWrap", "Wrap around edges", "no", &edgeWrap, valBool },
  { "extraBorder", "extraBorder", "Give map an extra border of solid rock",
	  "no", &extraBorder, valBool },
  { "gravityPoint", "gravityPoint", "?????", "0,0", &gravityPoint, valIPos },
  { "gravityAngle", "gravityAngle", "?????", "0", &gravityAngle, valReal },
  { "gravityPointSource", "gravityPointSource", "?????",
      "false", &gravityPointSource, valBool },
  { "gravityClockwise", "gravityClockwise", "??????",
      "false", &gravityClockwise, valBool },
  { "gravityAnticlockwise", "gravityAnticlockwise", "??????",
      "false", &gravityAnticlockwise, valBool },
  { "defaultsFileName", "defaults",
	"Filename of defaults file to read on startup",
	"", &defaultsFileName, valString },
  { "framesPerSecond", "FPS", 
	"Number of frames per second the server should strive for",
	"18", &framesPerSecond, valInt },
  { "allowNukes", "nukes", "Should nukes be allowed",
	"False", &allowNukes, valBool },
  { "movingItemProb", "movingItemProb",
	"Probability for an item to appear as moving",
	"0.2", &movingItemProb, valReal },
  { "dropItemOnKillProb", "dropItemOnKillProb",
	"Probability for dropping an item (each item) when you are killed",
	"0.5", &dropItemOnKillProb, valReal },
  { "itemEnergyPackProb", "itemEnergyPackProb",
	"Probability for an energy pack to appear",
	"0", &itemEnergyPackProb, valReal },
  { "itemTankProb", "itemTankProb",
	"Probability for an extra tank to appear",
	"0", &itemTankProb, valReal },
  { "itemECMProb", "itemECMProb",
	"Probability for an ECM item to appear",
	"0", &itemECMProb, valReal },
  { "itemMineProb", "itemMineProb",
	"Probability for a mine item to appear",
	"0", &itemMineProb, valReal },
  { "itemMissileProb", "itemMissileProb",
	"Probability for a missile item to appear",
	"0", &itemMissileProb, valReal },
  { "itemCloakProb", "itemCloakProb",
	"Probability for a cloak item to appear",
	"0", &itemCloakProb, valReal },
  { "itemSensorProb", "itemSensorProb",
	"Probability for a sensor item to appear",
	"0", &itemSensorProb, valReal },
  { "itemWideangleProb", "itemWideangleProb",
	"Probability for a wideangle item to appear",
	"0", &itemWideangleProb, valReal },
  { "itemRearshotProb", "itemRearshotProb",
	"Probability for a rearshot item to appear",
	"0", &itemRearshotProb, valReal },
  { "itemAfterburnerProb", "itemAfterburnerProb",
	"Probability for an afterburner item to appear",
	"0", &itemAfterburnerProb, valReal },
  { "maxItemDensity", "maxItemDensity",
	"Maximum density [0.0-1.0] for items (max items per block)",
	"0.00012", &maxItemDensity, valReal },
};
  

void Parser(int argc, char *argv[])
{
    int i, j;
    char *fname;   


    for (i=1; i<argc; i++) {
	if (strncmp("-help", argv[i], 2) == 0) {
	    printf("Usage:\t%s [ options ]\n\nWhere options include:\n",
		   argv[0]);
	    for(j=0; j<NELEM(options); j++) {
		printf("    %s%s",
		       options[j].type == valBool ? "-/+" : "-",
		       options[j].commandLineOption);
		if (strcmp(options[j].commandLineOption, options[j].name))
		    printf(" or %s", options[j].name);
		printf(" %s\n\t%s\n",
		       options[j].type == valInt ? "<integer>" : 
		       options[j].type == valReal ? "<real>" :
		       options[j].type == valString ? "<string>" :
		       options[j].type == valIPos ? "<position>" :
		       "", options[j].helpLine);
	    }
	    printf("\n    The probabilities are in the range [0.0-1.0] "
		   "and they refer to the\n    probability that an event "
		   "will occur in a block in second.\n"
		   "    Boolean options are turned off by using +<option>.\n");
	    printf("\n    Please refer to the manual pages, xpilots(6) "
		   "and xpilot(6),\n    for more specific help.\n");
	    exit(0);
	}
	
	if (strncmp("-version", argv[i], 2) == 0) {
	    puts(TITLE);
	    exit(0);
	}

	if (argv[i][0] == '-' || argv[i][0] == '+') {
	    for (j = 0; j < NELEM(options); j++) {
		if (!strcasecmp(options[j].commandLineOption, argv[i] + 1)
		    || !strcasecmp(options[j].name, argv[i] + 1)) {
		    if (options[j].type == valBool) {
			if (argv[i][0] == '-')
			    addOption(options[j].name, "true", 0, NULL);
			else
			    addOption(options[j].name, "false", 0, NULL);
		    } else {
			if (i + 1 == argc)
			    error("Option '%s' needs an argument",
				  options[j].commandLineOption);
			else
			    addOption(options[j].name,
				      argv[++i], 1, (void *)0);
		    }
		    break;
		}
	    }
	    continue;
	}
	error("Unknown option '%s'", argv[i]);
    }
    
    if (!(fname = getOption("mapData"))) {
	if (!(fname = getOption("mapFileName"))) {
#ifndef SILENT
	    printf("Map not specified, trying to open " DEFAULT_MAP "\n");
#endif
	    if (!parseDefaultsFile(DEFAULT_MAP)) {
		error("Unable to read " DEFAULT_MAP);
	    }
	} else {
	    if (!parseDefaultsFile(fname)) {
		error("Unable to read %s, trying to open " DEFAULT_MAP, fname);
		parseDefaultsFile(DEFAULT_MAP);
	    }
	}
    }
    
    if (fname = getOption("defaultsFileName"))
	parseDefaultsFile(fname);
    else
	parseDefaultsFile(DEFAULTS_FILE_NAME);
    
    for (j = 0; j < NELEM(options); j++)
	addOption(options[j].name, options[j].defaultValue, 0, &options[j]);
    parseOptions();
    Grok_map();
}

/* $Id: cmdline.c,v 3.15 1993/08/02 12:54:52 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
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
    "@(#)$Id: cmdline.c,v 3.15 1993/08/02 12:54:52 bjoerns Exp $";
#endif

float		Gravity;		/* Power of gravity */
float		ShipMass;		/* Default mass of ship */
float		ShotsMass;		/* Default mass of shots */
float		ShotsSpeed;		/* Default speed of shots */
int		ShotsLife;		/* Default number of ticks */
					/* each shot will live */
int		WantedNumRobots;	/* How many robots should enter */
					/* the game? */
int		robotsLeave;		/* Do robots leave at all? */
int		robotLeaveLife;		/* Max life per robot (0=off)*/
int		robotLeaveScore;	/* Min score for robot to live (0=off)*/
int		robotLeaveRatio;	/* Min ratio for robot to live (0=off)*/
int		ShotsMax;		/* Max shots pr. player */
bool		ShotsGravity;		/* Shots affected by gravity */
bool		LooseMass;		/* Loose mass when firering */
int		fireRepeatRate;		/* Frames per autorepeat fire (0=off) */

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
int		contactPort;		/* Contact port number */

bool		crashWithPlayer;	/* Can a player hit another player? */
bool		playerKillings;		/* Can players kill each other? */
bool		playerShielding;	/* Can players use shields? */
bool		playerStartsShielded;	/* Players start with shields up? */
bool		limitedVisibility;	/* Is visibility limited? */
bool		limitedLives;		/* Are lives limited? */
int		worldLives;		/* If so, what's the max? */
bool		teamPlay;		/* Are teams allowed? */
bool		onePlayerOnly;		/* Can there be multiple players? */
bool		timing;			/* Is this a race? */
bool		edgeWrap;		/* Do objects wrap when they cross
					   the edge of the Universe? */
bool		edgeBounce;		/* Do objects bounce when they hit
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
float		destroyItemInCollisionProb;
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
float		itemTransporterProb;
float		maxItemDensity;
bool		allowNukes;
bool		playersOnRadar;		/* Are players visible on radar? */
bool		missilesOnRadar;	/* Are missiles visible on radar? */
bool		targetKillTeam;		/* if your target explodes, you die */

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
  { "fireRepeatRate", "fireRepeat",
	"Number of frames per automatic fire (0=off)",
	"2", &fireRepeatRate, valInt },
  { "maxRobots", "robots", "How many robots do you want?", 
	"4", &WantedNumRobots, valInt },
  { "robotsLeave", "robotsleave", "Do robots leave the game?", 
	"true", &robotsLeave, valBool },
  { "robotLeaveLife", "robotleavelife", "Max life per robot (0=off)", 
	"50", &robotLeaveLife, valInt },
  { "robotLeaveScore", "robotleavescore",
	"Min score for robot to play (0=off)", 
	"-90", &robotLeaveScore, valInt },
  { "robotLeaveRatio", "robotleaveratio",
	"Min ratio for robot to play (0=off)", 
	"-5", &robotLeaveRatio, valInt },
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
  { "contactPort", "port", "Contact port number",
	"15345", &contactPort, valInt },
  { "mapData", "mapData", "The map's topology",
	"", &mapData, valString },
  { "allowPlayerCrashes", "crash", 
	"Should crashes between players be allowed?",
	"yes", &crashWithPlayer, valBool },
  { "allowPlayerKilling", "killings",
	"Should players be allowed to kill one other",
	"yes", &playerKillings, valBool },
  { "allowShields", "shields", "Should we allow shields",
	"yes", &playerShielding, valBool },
  { "playerStartsShielded", "playerStartShielded",
	"Players start with shields up",
	"yes", &playerStartsShielded, valBool },
  { "targetKillTeam", "targetKillTeam",
	"Do team members die when target explodes?", "no",
	&targetKillTeam, valBool },
  { "limitedVisibility", "limitedVisibility",
	"Should the players have a limited visibility?",
	"no", &limitedVisibility, valBool },
  { "limitedLives", "limitedLives",
	"Should players have limited lives?",
	"no", &limitedLives, valBool },
  { "worldLives", "lives",
	"Number of lives each player has (no sense without limitedLives)",
	"0", &worldLives, valInt },
  { "teamPlay", "teams", "Should we allow team play?",
	"no", &teamPlay, valBool },
  { "onePlayerOnly", "onePlayerOnly", "One player modus",
	"no", &onePlayerOnly, valBool },
  { "timing", "race", "Race mode", "no", &timing, valBool },
  { "edgeWrap", "edgeWrap", "Wrap around edges", "no", &edgeWrap, valBool },
  { "edgeBounce", "edgeBounce", "Objects bounce when they hit the edge",
	"yes", &edgeBounce, valBool },
  { "extraBorder", "extraBorder", "Give map an extra border of solid rock",
	"no", &extraBorder, valBool },
  { "gravityPoint", "gravityPoint",
	"If the gravity is a point source, where does that gravity originate?",
	"0,0", &gravityPoint, valIPos },
  { "gravityAngle", "gravityAngle",
	"If gravity is along a uniform line, at what angle is that line?",
	"90", &gravityAngle, valReal },
  { "gravityPointSource", "gravityPointSource",
	"Is gravity originating from a single point?",
	"false", &gravityPointSource, valBool },
  { "gravityClockwise", "gravityClockwise",
	"If the gravity is a point source, is it clockwise?",
	"false", &gravityClockwise, valBool },
  { "gravityAnticlockwise", "gravityAnticlockwise",
	"If the gravity is a point source, is it anticlockwise?",
	"false", &gravityAnticlockwise, valBool },
  { "defaultsFileName", "defaults",
	"Filename of defaults file to read on startup",
	"", &defaultsFileName, valString },
  { "framesPerSecond", "FPS", 
	"Number of frames per second the server should strive for",
	"18", &framesPerSecond, valInt },
  { "allowNukes", "nukes", "Should nukes be allowed",
	"False", &allowNukes, valBool },
  { "playersOnRadar", "playersRadar", "Are players visible on the radar",
	"True", &playersOnRadar, valBool },
  { "missilesOnRadar", "missilesRadar", "Are missiles visible on the radar",
	"True", &missilesOnRadar, valBool },
  { "movingItemProb", "movingItemProb",
	"Probability for an item to appear as moving",
	"0.2", &movingItemProb, valReal },
  { "dropItemOnKillProb", "dropItemOnKillProb",
	"Probability for dropping an item (each item) when you are killed",
	"0.5", &dropItemOnKillProb, valReal },
  { "destroyItemInCollisionProb", "destroyItemInCollisionProb",
	"Probability for items (some items) to be destryed in an collision",
	"0.0", &destroyItemInCollisionProb, valReal },
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
  { "itemTransporterProb", "itemTransporterProb",
	"Probability for a transporter item to appear",
	"0", &itemTransporterProb, valReal },
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
		       options[j].name);
		if (strcmp(options[j].commandLineOption, options[j].name))
		    printf(" or %s", options[j].commandLineOption);
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
			if (i + 1 == argc) {
			    errno = 0;
			    error("Option '%s' needs an argument",
				  options[j].commandLineOption);
			} else
			    addOption(options[j].name,
				      argv[++i], 1, (void *)0);
		    }
		    break;
		}
	    }
	    continue;
	}
	errno = 0;
	error("Unknown option '%s'", argv[i]);
    }

    /*
     * Read map file if map data not found yet
     */
    if (!(fname = getOption("mapData"))) {
	if (!(fname = getOption("mapFileName"))) {
#ifndef SILENT
	    printf("Map not specified, trying to open " DEFAULT_MAP "\n");
#endif
	    if (!parseDefaultsFile(DEFAULT_MAP))
		error("Unable to read " DEFAULT_MAP);
	} else {
	    if (!parseDefaultsFile(fname)) {
		error("Unable to read %s, trying to open " DEFAULT_MAP, fname);
		if (!parseDefaultsFile(DEFAULT_MAP))
		    error("Unable to read " DEFAULT_MAP);
	    }
	}
    }
    
    /*
     * Read local defaults file
     */
    if (fname = getOption("defaultsFileName"))
	parseDefaultsFile(fname);
    else
	parseDefaultsFile(DEFAULTS_FILE_NAME);

    for (j = 0; j < NELEM(options); j++)
	addOption(options[j].name, options[j].defaultValue, 0, &options[j]);
    parseOptions();
    Grok_map();
}

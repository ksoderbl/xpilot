/* $Id: cmdline.c,v 3.58 1994/05/23 19:04:27 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-94 by
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
/* Options parsing code contributed by Ted Lemon <mellon@ncd.com> */

#define SERVER
#include <stdlib.h>
#include "global.h"
#include "robot.h"
#include "map.h"
#include "defaults.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: cmdline.c,v 3.58 1994/05/23 19:04:27 bert Exp $";
#endif

float		Gravity;		/* Power of gravity */
float		ShipMass;		/* Default mass of ship */
float		ShotsMass;		/* Default mass of shots */
float		ShotsSpeed;		/* Default speed of shots */
int		ShotsLife;		/* Default number of ticks */
					/* each shot will live */
int		WantedNumRobots;	/* How many robots should enter */
					/* the game? */
int		robotsTalk;		/* Do robots talk? */
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
char		*motd;			/* File name for motd */

bool		crashWithPlayer;	/* Can players overrun other players? */
bool		bounceWithPlayer;	/* Can players bounce other players? */
bool		playerKillings;		/* Can players kill each other? */
bool		playerShielding;	/* Can players use shields? */
bool		playerStartsShielded;	/* Players start with shields up? */
bool		shotsWallBounce;	/* Do shots bounce off walls? */
bool		minesWallBounce;	/* Do mines bounce off walls? */
bool		itemsWallBounce;	/* Do items bounce off walls? */
bool		missilesWallBounce;	/* Do missiles bounce off walls? */
bool		sparksWallBounce;	/* Do sparks bounce off walls? */
bool		debrisWallBounce;	/* Do sparks bounce off walls? */
bool		ballsWallBounce;	/* Do balls bounce off walls? */
bool		ecmsReprogramMines;	/* Do emcs reprogram mines? */
float		maxObjectWallBounceSpeed;	/* max object bounce speed */
float		maxShieldedWallBounceSpeed;	/* max shielded bounce speed */
float		maxUnshieldedWallBounceSpeed; /* max unshielded bounce speed */
float		maxShieldedWallBounceAngle;	/* max angle for landing */
float		maxUnshieldedWallBounceAngle;	/* max angle for landing */
float		playerWallBrakeFactor;	/* wall lowers speed if less than 1 */
float		objectWallBrakeFactor;	/* wall lowers speed if less than 1 */
float		objectWallBounceLifeFactor;	/* reduce object life */
float		wallBounceFuelDrainMult;/* Wall bouncing fuel drain factor */
float		wallBounceDestroyItemProb;/* Wall bouncing item destroy prob */
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
char		*scoreTableFileName;	/* Name of score table file */

int 		MovingItemsRand;	/* Probability for moving items */
int		ThrowItemOnKillRand;	/* Probability for players items to */
					/* drop upon beeing killed */
int		DetonateItemOnKillRand;	/* Probaility for remaining items to */
					/* detonate on being killed */
int		nukeMinSmarts;		/* minimum smarts for a nuke */
int		nukeMinMines;		/* minimum number of mines for nuke */
float		nukeClusterDamage;	/* multiplier for damage from nuke */
					/* cluster debris, reduces number */
					/* of particles by similar amount */
int		mineFuseTime;		/* Length of time mine is fused */

float 		movingItemProb;		/* Probability for moving items */
float		dropItemOnKillProb;	/* Probability for players items to */
					/* drop when player is killed */
float		detonateItemOnKillProb;	/* Probaility for remaining items to */
					/* detonate when player is killed */
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
float		itemLaserProb;
float		itemEmergencyThrustProb;
float		itemTractorBeamProb;
float		itemAutopilotProb;
float		itemEmergencyShieldProb;
float		itemProbMult;
float		maxItemDensity;
int		initialFuel;
int 		initialTanks;
int		initialECMs;
int		initialMines;
int 		initialMissiles;
int		initialCloaks;
int		initialSensors;
int		initialWideangles;
int		initialRearshots;
int		initialAfterburners;
int		initialTransporters;
int		initialLasers;
int		initialEmergencyThrusts;
int		initialTractorBeams;
int		initialAutopilots;
int		initialEmergencyShields;

bool		allowNukes;
bool		allowClusters;
bool		allowModifiers;
bool		allowLaserModifiers;
bool		allowShipShapes;

bool		playersOnRadar;		/* Are players visible on radar? */
bool		missilesOnRadar;	/* Are missiles visible on radar? */
bool		minesOnRadar;		/* Are mines visible on radar? */
bool		nukesOnRadar;		/* Are nuke weapons radar visible? */
bool		distinguishMissiles;	/* Smarts, heats & torps look diff.? */
int		maxMissilesPerPack;	/* Number of missiles per item. */
bool		identifyMines;		/* Mines have names displayed? */
bool		shieldedItemPickup;	/* Pickup items with shields up? */
bool		shieldedMining;		/* Detach mines with shields up? */
bool		laserIsStunGun;		/* Is the laser a stun gun? */
bool		reportToMetaServer;	/* Send status to meta-server? */
float		gameDuration;		/* total duration of game in minutes */

float		playerMinimumStartFuel;	/* Minimum starting fuel */

bool		teamAssign;		/* Assign player to team if not set? */
bool		teamImmunity;		/* Is team immune from player action */
  
bool		targetKillTeam;		/* if your target explodes, you die? */
bool		targetTeamCollision;	/* Does team collide with target? */
bool		treasureKillTeam;	/* die if treasure is destroyed? */
bool		treasureCollisionDestroys;
bool		treasureCollisionMayKill;

static optionDesc options[] = {
  { "help", "help", "Print out this message", "0", NULL, valVoid },
  { "version", "version", "Print version information", "0", NULL, valVoid },
  { "dump", "dump",
        "Print all options with their default values in defaultsfile format.",
        "0", NULL, valVoid },
  { "gravity", "gravity", "Gravity strength", "-0.14", &Gravity, valReal },
  { "shipMass", "shipmass", "Mass of fighters", "20.0", &ShipMass, valReal },
  { "shotMass", "shotmass", "Mass of bullets", "0.1", &ShotsMass, valReal },
  { "shotSpeed", "shotspeed", "Maximum speed of bullets", "21.0",
	&ShotsSpeed, valReal },
  { "shotLife", "shotlife", "Life of bullets in ticks",
	"60", &ShotsLife, valInt },
  { "fireRepeatRate", "fireRepeat",
	"Number of frames per automatic fire (0=off)",
	"2", &fireRepeatRate, valInt },
  { "maxRobots", "robots", "How many robots do you want?", 
	"4", &WantedNumRobots, valInt },
  { "robotsTalk", "robotsTalk", "Do robots talk when they die, kill, etc.?", 
	"false", &robotsTalk, valBool },
  { "robotsLeave", "robotsLeave", "Do robots leave the game?", 
	"true", &robotsLeave, valBool },
  { "robotLeaveLife", "robotLeaveLife", "Max life per robot (0=off)", 
	"50", &robotLeaveLife, valInt },
  { "robotLeaveScore", "robotLeaveScore",
	"Min score for robot to play (0=off)", 
	"-90", &robotLeaveScore, valInt },
  { "robotLeaveRatio", "robotLeaveRatio",
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
  { "motd", "motd", "File name for server-motd",
	SERVERMOTDFILE, &motd, valString },
  { "mapData", "mapData", "The map's topology",
	NULL, &mapData, valString },
  { "allowPlayerCrashes", "allowPlayerCrashes", 
	"Can players overrun other players?",
	"yes", &crashWithPlayer, valBool },
  { "allowPlayerBounces", "allowPlayerBounces", 
	"Can players bounce with other players?",
	"yes", &bounceWithPlayer, valBool },
  { "allowPlayerKilling", "killings",
	"Should players be allowed to kill one other?",
	"yes", &playerKillings, valBool },
  { "allowShields", "shields", "Should we allow shields?",
	"yes", &playerShielding, valBool },
  { "playerStartsShielded", "playerStartShielded",
	"Players start with shields up",
	"yes", &playerStartsShielded, valBool },
  { "shotsWallBounce", "shotsWallBounce",
	"Do shots bounce off walls?",
	"no", &shotsWallBounce, valBool },
  { "ballsWallBounce", "ballsWallBounce",
	"Do balls bounce off walls?",
	"yes", &ballsWallBounce, valBool },
  { "minesWallBounce", "minesWallBounce",
	"Do mines bounce off walls?",
	"no", &minesWallBounce, valBool },
  { "itemsWallBounce", "itemsWallBounce",
	"Do items bounce off walls?",
	"yes", &itemsWallBounce, valBool },
  { "missilesWallBounce", "missilesWallBounce",
	"Do missiles bounce off walls?",
	"no", &missilesWallBounce, valBool },
  { "sparksWallBounce", "sparksWallBounce",
	"Do thrust spark particles bounce off walls to give reactive thrust?",
	"no", &sparksWallBounce, valBool },
  { "debrisWallBounce", "debrisWallBounce",
	"Do explosion debris particles bounce off walls?",
	"no", &debrisWallBounce, valBool },
  { "maxObjectWallBounceSpeed", "maxObjectBounceSpeed",
	"The maximum allowed speed for objects to bounce off walls",
	"40", &maxObjectWallBounceSpeed, valReal },
  { "maxShieldedWallBounceSpeed", "maxShieldedBounceSpeed",
	"The maximum allowed speed for a shielded player to bounce off walls",
	"50", &maxShieldedWallBounceSpeed, valReal },
  { "maxUnshieldedWallBounceSpeed", "maxUnshieldedBounceSpeed",
	"Maximum allowed speed for an unshielded player to bounce off walls",
	"20", &maxUnshieldedWallBounceSpeed, valReal },
  { "maxShieldedPlayerWallBounceAngle", "maxShieldedBounceAngle",
	"Maximum allowed angle for a shielded player to bounce off walls",
	"90", &maxShieldedWallBounceAngle, valReal },
  { "maxUnshieldedPlayerWallBounceAngle", "maxUnshieldedBounceAngle",
	"Maximum allowed angle for an unshielded player to bounce off walls",
	"30", &maxUnshieldedWallBounceAngle, valReal },
  { "playerWallBounceBrakeFactor", "playerWallBrake",
	"Factor to slow down players when they hit the wall (between 0 and 1)",
	"0.89", &playerWallBrakeFactor, valReal },
  { "objectWallBounceBrakeFactor", "objectWallBrake",
	"Factor to slow down objects when they hit the wall (between 0 and 1)",
	"0.95", &objectWallBrakeFactor, valReal },
  { "objectWallBounceLifeFactor", "objectWallBounceLifeFactor",
	"Factor to reduce the life of objects after bouncing (between 0 and 1)",
	"0.80", &objectWallBounceLifeFactor, valReal },
  { "wallBounceFuelDrainMult", "wallBounceDrain",
	"Multiplication factor for player wall bounce fuel cost.",
	"1.0", &wallBounceFuelDrainMult, valReal },
  { "wallBounceDestroyItemProb", "wallBounceDestroyItemProb",
	"Player wall bounce item damage probability.",
	"0.0", &wallBounceDestroyItemProb, valReal },
  { "reportToMetaServer", "reportMeta",
	"Keep the meta server informed about our game?", "yes",
	&reportToMetaServer, valBool },
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
  { "teamAssign", "teamAssign",
	"Should player be assigned to first non-empty team if team isn't set?",
	"yes", &teamAssign, valBool },
  { "teamImmunity", "teamImmunity",
	"Should other team members be immune to various shots, thrust etc.?",
	"yes", &teamImmunity, valBool },
  { "emcsReprogramMines", "ecmsReprogramMines",
	"Do ECMs reprogram mines?",
	"yes", &ecmsReprogramMines, valBool },

  { "playerMinimumStartFuel", "playerMinStartFuel",
	"Minimum amount of fuel a player will have after being killed",
	"400.0", &playerMinimumStartFuel, valReal },

  { "targetKillTeam", "targetKillTeam",
	"Do team members die when their last target explodes?",
	"no", &targetKillTeam, valBool },
  { "targetTeamCollision", "targetCollision",
	"Do team members collide with their own target or not",
	"yes", &targetTeamCollision, valBool },

  { "treasureKillTeam", "treasureKillTeam",
	"Do team members die when treasure is destroyed?", "no",
	&treasureKillTeam, valBool },
  { "treasureCollisionDestroys", "treasureCollisionDestroy",
	"Destroy balls on collisions",
	"yes", &treasureCollisionDestroys, valBool },
  { "treasureCollisionMayKill", "treasureUnshieldedCollisionKills",
	"Does unshielded collision kill player",
	"no", &treasureCollisionMayKill, valBool },

  { "onePlayerOnly", "onePlayerOnly", "One player modus",
	"no", &onePlayerOnly, valBool },
  { "timing", "race", "Race mode", "no", &timing, valBool },
  { "edgeWrap", "edgeWrap", "Wrap around edges", "no", &edgeWrap, valBool },
  { "edgeBounce", "edgeBounce",
	"Players and bullets bounce when they hit the edge",
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
  { "scoreTableFileName", "scoretable",
	"Filename for the score table to be dumped to",
	"", &scoreTableFileName, valString },
  { "framesPerSecond", "FPS", 
	"Number of frames per second the server should strive for",
	"18", &framesPerSecond, valInt },
  { "allowNukes", "nukes", "Should nuclear weapons be allowed?",
	"False", &allowNukes, valBool },
  { "allowClusters", "clusters", "Should cluster weapons be allowed?",
	"False", &allowClusters, valBool },
  { "allowModifiers", "modifiers", "Should the weapon modifiers be allowed?",
	"False", &allowModifiers, valBool },
  { "allowLaserModifiers", "lasermodifiers",
	"Can lasers be modified to be a different weapon?",
	"False", &allowLaserModifiers, valBool },
  { "allowShipShapes", "ShipShapes",
	"Are players allowed to define their own ship shape?",
	"True", &allowShipShapes, valBool },
  { "playersOnRadar", "playersRadar",
	"Are players visible on the radar",
	"True", &playersOnRadar, valBool },
  { "missilesOnRadar", "missilesRadar",
	"Are missiles visible on the radar",
	"True", &missilesOnRadar, valBool },
  { "minesOnRadar", "minesRadar",
	"Are mines visible on the radar",
	"False", &minesOnRadar, valBool },
  { "nukesOnRadar", "nukesRadar",
	"Are nukes visible or highlighted on radar",
	"True", &nukesOnRadar, valBool },
  { "distinguishMissiles", "distinguishMissiles",
	"Are different types of missiles distinguished (by length)",
	"True", &distinguishMissiles, valBool },
  { "maxMissilesPerPack", "maxMissilesPerPack",
	"Number of missiles gotten by picking up one missile item.",
	"4", &maxMissilesPerPack, valInt },
  { "identifyMines", "identifyMines",
	"Are mine owner's names displayed",
	"True", &identifyMines, valBool },
  { "shieldedItemPickup", "shieldItem",
	"Can items be picked up while shields are up?",
	"False", &shieldedItemPickup, valBool },
  { "shieldedMining", "shieldMine",
	"Can mines be thrown and placed while shields are up?",
	"False", &shieldedMining, valBool },
  { "laserIsStunGun", "stunGun",
	"Is the laser weapon a stun gun weapon?",
	"False", &laserIsStunGun, valBool },
  { "nukeMinSmarts", "nukeMinSmarts",
	"Minimum number of smart missiles needed to make a nuclear variant",
	"7", &nukeMinSmarts, valInt },
  { "nukeMinMines", "nukeMinMines",
	"Minimum number of mines needed to make a nuclear variant",
	"4", &nukeMinMines, valInt },
  { "nukeClusterDamage", "nukeClusterDamage",
	"How much each cluster debris does damage wise from a nuke variant"
	"\n\tAlso reduces number of particles in the explosion",
	"1.0", &nukeClusterDamage, valReal },
  { "mineFuseTime", "mineFuseTime",
	"Time after which owned mines become deadly, zero means never",
	"0.0", &mineFuseTime, valSec },
  { "movingItemProb", "movingItemProb",
	"Probability for an item to appear as moving",
	"0.2", &movingItemProb, valReal },
  { "dropItemOnKillProb", "dropItemOnKillProb",
	"Probability for dropping an item (each item) when you are killed",
	"0.5", &dropItemOnKillProb, valReal },
  { "detonateItemOnKillProb", "detonateItemOnKillProb",
	"Probability for undropped items to detonate when you are killed",
	"0.5", &detonateItemOnKillProb, valReal },
  { "destroyItemInCollisionProb", "destroyItemInCollisionProb",
	"Probability for items (some items) to be destroyed in a collision",
	"0.0", &destroyItemInCollisionProb, valReal },
  { "itemProbMult", "itemProbFact",
	"Item Probability Multiplication Factor scales all item probabilities",
	"1.0", &itemProbMult, valReal },
  { "maxItemDensity", "maxItemDensity",
	"Maximum density [0.0-1.0] for items (max items per block)",
	"0.00012", &maxItemDensity, valReal },

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
  { "itemLaserProb", "itemLaserProb",
	"Probability for a Laser item to appear",
	"0", &itemLaserProb, valReal },
  { "itemEmergencyThrustProb", "itemEmergencyThrustProb",
	"Probability for an Emergency Thrust item to appear",
	"0", &itemEmergencyThrustProb, valReal },
  { "itemTractorBeamProb", "itemTractorBeamProb",
	"Probability for a Tractor Beam item to appear",
	"0", &itemTractorBeamProb, valReal },
  { "itemAutopilotProb", "itemAutopilotProb",
	"Probability for an Autopilot item to appear",
	"0", &itemAutopilotProb, valReal },
  { "itemEmergencyShieldProb", "itemEmergencyShieldProb",
	"Probability for an Emergency Shield item to appear",
	"0", &itemEmergencyShieldProb, valReal },

  { "initialFuel", "initialFuel",
        "How much fuel players start with",
        "1000", &initialFuel, valInt },
  { "initialTanks", "initialTanks",
        "How many tanks players start with",
        "0", &initialTanks, valInt },
  { "initialECMs", "initialECMs",
        "How many ECMs players start with",
        "0", &initialECMs, valInt },
  { "initialMines", "initialMines",
        "How many mines players start with",
        "0", &initialMines, valInt },
  { "initialMissiles", "initialMissiles",
        "How many missiles players start with",
        "0", &initialMissiles, valInt },
  { "initialCloaks", "initialCloaks",
        "How many cloaks players start with",
        "0", &initialCloaks, valInt },
  { "initialSensors", "initialSensors",
        "How many sensors players start with",
        "0", &initialSensors, valInt },
  { "initialWideangles", "initialWideangles",
        "How many wideangles players start with",
        "0", &initialWideangles, valInt },
  { "initialRearshots", "initialRearshots",
        "How many rearshots players start with",
        "0", &initialRearshots, valInt },
  { "initialAfterburners", "initialAfterburners",
        "How many afterburners players start with",
        "0", &initialAfterburners, valInt },
  { "initialTransporters", "initialTransporters",
        "How many transporters players start with",
        "0", &initialTransporters, valInt },
  { "initialLasers", "initialLasers",
        "How many lasers players start with",
        "0", &initialLasers, valInt },
  { "initialEmergencyThrusts", "initialEmergencyThrusts",
        "How many emergency thrusts players start with",
        "0", &initialEmergencyThrusts, valInt },
  { "initialTractorBeams", "initialTractorBeams",
        "How many tractor/pressor beams players start with",
        "0", &initialTractorBeams, valInt },
  { "initialAutopilots", "initialAutopilots",
        "How many autopilots players start with",
        "0", &initialAutopilots, valInt },
  { "initialEmergencyShields", "initialEmergencyShields",
        "How many emergency shields players start with",
        "0", &initialEmergencyShields, valInt },

  { "gameDuration", "time",
	"Duration of game in minutes (aka. pizza mode)",
	"0.0", &gameDuration, valReal },
};
  

static void Parse_help(char *progname)
{
    int			j;

    printf("Usage:\t%s [ options ]\n\nWhere options include:\n",
	   progname);
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
	       options[j].type == valSec ? "<seconds>" :
	       options[j].type == valPerSec ? "<per-second>" :
	       "", options[j].helpLine);
    }
    printf("\n    The probabilities are in the range [0.0-1.0] "
	   "and they refer to the\n    probability that an event "
	   "will occur in a block in second.\n"
	   "    Boolean options are turned off by using +<option>.\n");
    printf("\n    Please refer to the manual pages, xpilots(6) "
	   "and xpilot(6),\n    for more specific help.\n");
}

static void Parse_dump(char *progname)
{
    int			j;

    printf("\n");
    printf("# %s option dump\n", progname);
    printf("# \n");
    printf("# LIBDIR = %s\n", LIBDIR);
    printf("# DEFAULTS_FILE_NAME = %s\n",
	   DEFAULTS_FILE_NAME);
    printf("# MAPDIR = %s\n", MAPDIR);
    printf("# DEFAULT_MAP = %s\n", DEFAULT_MAP);
    printf("# \n");
    for (j = 0; j < NELEM(options); j++) {
	if (options[j].type != valVoid) {
	    int len = strlen(options[j].name);
	    printf("%s:%*s%s\n", options[j].name,
		   (len < 40) ? (40 - len) : 1, "",
		   (options[j].defaultValue != NULL)
		       ? options[j].defaultValue
		       : "");
	}
    }
    printf("\n");
}

int Parse_list(int *index, char *buf)
{
    int			i = *index;

    if (i < 0 || i >= NELEM(options)) {
	return -1;
    }
    if (options[i].defaultValue == NULL) {
	return 0;
    }
    switch (options[i].type) {
    case valInt:
    case valSec:
	sprintf(buf, "%s:%d", options[i].name,
		*(int *)options[i].variable);
	break;
    case valReal:
    case valPerSec:
	sprintf(buf, "%s:%g", options[i].name,
		*(float *)options[i].variable);
	break;
    case valBool:
	sprintf(buf, "%s:%s", options[i].name,
	        *(bool *)options[i].variable ? "yes" : "no");
	break;
    case valIPos:
	sprintf(buf, "%s:%d,%d", options[i].name,
		((ipos *)options[i].variable)->x,
		((ipos *)options[i].variable)->y);
	break;
    case valString:
	sprintf(buf, "%s:%s", options[i].name,
		*(char **)options[i].variable);
	break;
    default:
	return 0;
    }
    return 1;
}

void Parser(int argc, char *argv[])
{
    int			i, j;
    char		*fname;


    for (i=1; i<argc; i++) {
	if (strncmp("-help", argv[i], 2) == 0) {
	    Parse_help(*argv);
	    exit(0);
	}
	if (strcmp("-dump", argv[i]) == 0) {
	    Parse_dump(*argv);
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
			    addOption(options[j].name, "true", 1, NULL);
			else
			    addOption(options[j].name, "false", 1, NULL);
		    } else {
			if (i + 1 == argc) {
			    errno = 0;
			    error("Option '%s' needs an argument",
				  options[j].commandLineOption);
			} else
			    addOption(options[j].name,
				      argv[++i], 1, NULL);
		    }
		    break;
		}
	    }
	    if (j < NELEM(options)) {
		continue;
	    }
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

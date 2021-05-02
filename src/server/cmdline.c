/* $Id: cmdline.c,v 5.28 2001/06/24 20:12:05 bertg Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef _WINDOWS
# include "NT/winServer.h"
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "defaults.h"
#include "error.h"
#include "portability.h"
#include "checknames.h"
#include "commonproto.h"

char cmdline_version[] = VERSION;


DFLOAT		Gravity;		/* Power of gravity */
DFLOAT		ShipMass;		/* Default mass of ship */
DFLOAT		ballMass;		/* Default mass of balls */
DFLOAT		ShotsMass;		/* Default mass of shots */
DFLOAT		ShotsSpeed;		/* Default speed of shots */
int		ShotsLife;		/* Default number of ticks */
					/* each shot will live */
int		maxRobots;		/* How many robots should enter */
int		minRobots;		/* the game? */
char		*robotFile;		/* Filename for robot parameters */
int		robotsTalk;		/* Do robots talk? */
int		robotsLeave;		/* Do robots leave at all? */
int		robotLeaveLife;		/* Max life per robot (0=off)*/
int		robotLeaveScore;	/* Min score for robot to live (0=off)*/
int		robotLeaveRatio;	/* Min ratio for robot to live (0=off)*/
int		robotTeam;		/* Team for robots */
bool		restrictRobots;		/* Restrict robots to robotTeam? */
bool		reserveRobotTeam;	/* Allow only robots in robotTeam? */
int		ShotsMax;		/* Max shots pr. player */
bool		ShotsGravity;		/* Shots affected by gravity */
int		fireRepeatRate;		/* Frames per autorepeat fire (0=off) */

bool		RawMode;		/* Let robots live even if there */
					/* are no players logged in */
bool		NoQuit;			/* Don't quit even if there are */
					/* no human players playing */
bool		logRobots;		/* log robots coming and going */
char		*mapFileName;		/* Name of mapfile... */
char		*mapData;		/* Raw map data... */
int		mapWidth;		/* Width of the universe */
int		mapHeight;		/* Height of the universe */
char		*mapName;		/* Name of the universe */
char		*mapAuthor;		/* Name of the creator */
int		contactPort;		/* Contact port number */
char		*serverHost;		/* Host name (for multihomed hosts) */

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
bool		asteroidsWallBounce;	/* Do asteroids bounce off walls? */
bool		cloakedExhaust;		/* Generate exhaust when cloaked? */
bool		cloakedShield;		/* Allowed to use shields when cloaked? */
bool		ecmsReprogramMines;	/* Do ecms reprogram mines? */
bool		ecmsReprogramRobots;	/* Do ecms reprogram robots? */
DFLOAT		maxObjectWallBounceSpeed;	/* max object bounce speed */
DFLOAT		maxShieldedWallBounceSpeed;	/* max shielded bounce speed */
DFLOAT		maxUnshieldedWallBounceSpeed; /* max unshielded bounce speed */
DFLOAT		maxShieldedWallBounceAngle;	/* max angle for landing */
DFLOAT		maxUnshieldedWallBounceAngle;	/* max angle for landing */
DFLOAT		playerWallBrakeFactor;	/* wall lowers speed if less than 1 */
DFLOAT		objectWallBrakeFactor;	/* wall lowers speed if less than 1 */
DFLOAT		objectWallBounceLifeFactor;	/* reduce object life */
DFLOAT		wallBounceFuelDrainMult;/* Wall bouncing fuel drain factor */
DFLOAT		wallBounceDestroyItemProb;/* Wall bouncing item destroy prob */
bool		limitedVisibility;	/* Is visibility limited? */
DFLOAT		minVisibilityDistance;	/* Minimum visibility when starting */
DFLOAT		maxVisibilityDistance;	/* Maximum visibility */
bool		limitedLives;		/* Are lives limited? */
int		worldLives;		/* If so, what's the max? */
bool		endOfRoundReset;	/* Reset the world when round ends? */
int		resetOnHuman;		/* Last human to reset round for */
bool		teamPlay;		/* Are teams allowed? */
bool		teamFuel;		/* Do fuelstations belong to teams? */
bool		teamCannons;		/* Do cannons belong to teams? */
int		cannonSmartness;	/* Accuracy of cannonfire */
bool		cannonsUseItems;	/* Do cannons use items? */
int		cannonDeadTime;		/* How long do cannons stay dead? */
bool		keepShots;		/* Keep shots when player leaves? */
bool		timing;			/* Is this a race? */
bool		edgeWrap;		/* Do objects wrap when they cross
					   the edge of the Universe? */
bool		edgeBounce;		/* Do objects bounce when they hit
					   the edge of the Universe? */
bool		extraBorder;		/* Give map an extra border? */
ipos		gravityPoint;		/* Where does gravity originate? */
DFLOAT		gravityAngle;		/* If gravity is along a uniform line,
					   at what angle is that line? */
bool		gravityPointSource;	/* Is gravity a point source? */
bool		gravityClockwise;	/* If so, is it clockwise? */
bool		gravityAnticlockwise;	/* If not clockwise, anticlockwise? */
bool		gravityVisible;		/* Is gravity visible? */
bool		wormholeVisible;	/* Are wormholes visible? */
bool		itemConcentratorVisible;/* Are itemconcentrators visible? */
int		wormTime;
char		*defaultsFileName;	/* Name of defaults file... */
char		*passwordFileName;	/* Name of password file... */
char		*motdFileName;		/* Name of motd file */
char		*scoreTableFileName;	/* Name of score table file */

int		nukeMinSmarts;		/* minimum smarts for a nuke */
int		nukeMinMines;		/* minimum number of mines for nuke */
DFLOAT		nukeClusterDamage;	/* multiplier for damage from nuke */
					/* cluster debris, reduces number */
					/* of particles by similar amount */
int		mineFuseTime;		/* Length of time mine is fused */
int		mineLife;		/* lifetime of mines */
DFLOAT		minMineSpeed;		/* minimum speed of mines */
int		missileLife;		/* lifetime of missiles */
int		baseMineRange;		/* Distance from base mines may be used */

DFLOAT		shotKillScoreMult;
DFLOAT		torpedoKillScoreMult;
DFLOAT		smartKillScoreMult;
DFLOAT		heatKillScoreMult;
DFLOAT		clusterKillScoreMult;
DFLOAT		laserKillScoreMult;
DFLOAT		tankKillScoreMult;
DFLOAT		runoverKillScoreMult;
DFLOAT		ballKillScoreMult;
DFLOAT		explosionKillScoreMult;
DFLOAT		shoveKillScoreMult;
DFLOAT		crashScoreMult;
DFLOAT		mineScoreMult;
bool		asteroidScoring;

DFLOAT 		movingItemProb;		/* Probability for moving items */
DFLOAT		randomItemProb;		/* Probability for random-appearing items */
DFLOAT		dropItemOnKillProb;	/* Probability for players items to */
					/* drop when player is killed */
DFLOAT		detonateItemOnKillProb;	/* Probaility for remaining items to */
					/* detonate when player is killed */
DFLOAT		destroyItemInCollisionProb;
DFLOAT		rogueHeatProb;          /* prob. that unclaimed rocketpack */
DFLOAT		rogueMineProb;          /* or minepack will "activate" */
DFLOAT		itemProbMult;
DFLOAT		cannonItemProbMult;
DFLOAT		maxItemDensity;
DFLOAT		maxAsteroidDensity;
int		itemConcentratorRadius;
DFLOAT		itemConcentratorProb;

bool		allowSmartMissiles;
bool		allowHeatSeekers;
bool		allowTorpedoes;
bool		allowNukes;
bool		allowClusters;
bool		allowModifiers;
bool		allowLaserModifiers;
bool		allowShipShapes;

bool		playersOnRadar;		/* Are players visible on radar? */
bool		missilesOnRadar;	/* Are missiles visible on radar? */
bool		minesOnRadar;		/* Are mines visible on radar? */
bool		nukesOnRadar;		/* Are nuke weapons radar visible? */
bool		treasuresOnRadar;	/* Are treasure balls radar visible? */
bool		asteroidsOnRadar;	/* Are asteroids radar visible? */
bool		distinguishMissiles;	/* Smarts, heats & torps look diff.? */
int		maxMissilesPerPack;	/* Number of missiles per item. */
int		maxMinesPerPack;	/* Number of mines per item. */
bool		identifyMines;		/* Mines have names displayed? */
bool		shieldedItemPickup;	/* Pickup items with shields up? */
bool		shieldedMining;		/* Detach mines with shields up? */
bool		laserIsStunGun;		/* Is the laser a stun gun? */
bool		reportToMetaServer;	/* Send status to meta-server? */
bool		searchDomainForXPilot;	/* Do a DNS lookup for XPilot.domain? */
char		*denyHosts;		/* Computers which are denied service */
DFLOAT		gameDuration;		/* total duration of game in minutes */
bool		allowViewing;		/* Are players allowed to watch others? */

bool		teamAssign;		/* Assign player to team if not set? */
bool		teamImmunity;		/* Is team immune from player action */

bool		targetKillTeam;		/* if your target explodes, you die? */
bool		targetTeamCollision;	/* Does team collide with target? */
bool		targetSync;		/* all targets reappear together */
int		targetDeadTime;		/* How long do targgets stay dead? */
bool		treasureKillTeam;	/* die if treasure is destroyed? */
bool		captureTheFlag;		/* must treasure be safe to cash balls? */
bool		treasureCollisionDestroys;
bool		treasureCollisionMayKill;
bool		wreckageCollisionMayKill;
bool		asteroidCollisionMayKill;

DFLOAT		ballConnectorSpringConstant;
DFLOAT		ballConnectorDamping;
DFLOAT		maxBallConnectorRatio;
DFLOAT		ballConnectorLength;

DFLOAT		friction;		/* friction only affects ships */
DFLOAT		blockFriction;		/* friction in friction blocks */
bool		blockFrictionVisible;	/* if yes, friction blocks are decor; */
					/* if no, friction blocks are space */
int		coriolis;		/* angle velocity turns each frame */
DFLOAT		checkpointRadius;      	/* in blocks */
int		raceLaps;		/* how many laps per race */
bool		lockOtherTeam;		/* lock ply from other teams when dead? */
bool		loseItemDestroys; 	/* destroy item on loseItem? */
bool		useWreckage;		/* destroyed ships leave wreckage? */

int		maxOffensiveItems;	/* how many offensive and defensive */
int		maxDefensiveItems;	/* items can player carry */

int		roundDelaySeconds;	/* delay before start of each round */
int		maxRoundTime;		/* max. duration of each round */
int		roundsToPlay;		/* # of rounds to play. */
int		roundsPlayed;		/* # of rounds played sofar. */

int		maxVisibleObject;	/* how many objects a player can see */
bool		pLockServer;		/* Is server swappable out of memory?  */
bool		ignore20MaxFPS;		/* ignore client maxFPS request if 20 */
int		timerResolution;	/* OS timer resolution (times/sec) */
char		*password;		/* password for operator status */
int		clientPortStart;	/* First UDP port for clients */
int		clientPortEnd;		/* Last one (these are for firewalls) */

char		*robotRealName;		/* Real name for robot */
char		*robotHostName;		/* Host name for robot */

char		*tankRealName;		/* Real name for tank */
char		*tankHostName;		/* Host name for tank */
int		tankScoreDecrement;	/* Amount by which the tank's score */
					/* is decreased from the player's */

bool		turnThrust;		/* Does turning use fuel and shoot sparks? */
bool		selfImmunity;		/* Are players immune to their own weapons? */

char		*defaultShipShape;	/* What ship shape is used for players */
					/* who do not define their own? */
char		*tankShipShape;		/* What ship shape is used for tanks? */


/*
** Two functions which can be used if an option
** does not have its own function which should
** be called after the option value has been
** changed during runtime.  The tuner_none
** function should be specified when an option
** cannot be changed at all during runtime.
** The tuner_dummy can be specified if it
** is OK to modify the option during runtime
** and no follow up action is needed.
*/
void tuner_none(void)  {}
void tuner_dummy(void) {}


static void Tune_robot_real_name(void) { Fix_real_name(robotRealName); }
static void Tune_robot_host_name(void) { Fix_host_name(robotHostName); }
static void Tune_tank_real_name(void)  { Fix_real_name(tankRealName); }
static void Tune_tank_host_name(void)  { Fix_host_name(tankHostName); }


static option_desc options[] = {
    {
	"help",
	"help",
	"0",
	NULL,
	valVoid,
	tuner_none,
	"Print out this help message.\n",
	OPT_NONE
    },
    {
	"version",
	"version",
	"0",
	NULL,
	valVoid,
	tuner_none,
	"Print version information.\n",
	OPT_NONE
    },
    {
	"dump",
	"dump",
	"0",
	NULL,
	valVoid,
	tuner_none,
	"Print all options with their default values in defaultsfile format.\n",
	OPT_NONE
    },
    {
	"gravity",
	"gravity",
	"-0.14",
	&Gravity,
	valReal,
	tuner_none,
	"Gravity strength.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"shipMass",
	"shipMass",
	"20.0",
	&ShipMass,
	valReal,
	tuner_none,
	"Mass of fighters.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"ballMass",
	"ballMass",
	"50.0",
	&ballMass,
	valReal,
	tuner_none,
	"Mass of balls.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"shotMass",
	"shotMass",
	"0.1",
	&ShotsMass,
	valReal,
	tuner_none,
	"Mass of bullets.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"shotSpeed",
	"shotSpeed",
	"21.0",
	&ShotsSpeed,
	valReal,
	tuner_none,
	"Maximum speed of bullets.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"shotLife",
	"shotLife",
	"60",
	&ShotsLife,
	valInt,
	tuner_none,
	"Life of bullets in ticks.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"fireRepeatRate",
	"fireRepeat",
	"2",
	&fireRepeatRate,
	valInt,
	tuner_dummy,
	"Number of frames per automatic fire (0=off).\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxRobots",
	"robots",
	"4",
	&maxRobots,
	valInt,
	tuner_none,
	"The maximum number of robots wanted.\n"
	"Adds robots if there are less than maxRobots players.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"minRobots",
	"minRobots",
	"-1",
	&minRobots,
	valInt,
	tuner_none,
	"The minimum number of robots wanted.\n"
	"At least minRobots robots will be in the game, if there is room.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"robotFile",
	"robotFile",
	NULL,
	&robotFile,
	valString,
	tuner_none,
	"The file containing parameters for robot details.\n",
	OPT_COMMAND | OPT_DEFAULTS
    },
    {
	"robotsTalk",
	"robotsTalk",
	"false",
	&robotsTalk,
	valBool,
	tuner_dummy,
	"Do robots talk when they kill, die etc.?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"robotsLeave",
	"robotsLeave",
	"true",
	&robotsLeave,
	valBool,
	tuner_dummy,
	"Do robots leave the game?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"robotLeaveLife",
	"robotLeaveLife",
	"50",
	&robotLeaveLife,
	valInt,
	tuner_dummy,
	"Max life per robot (0=off).\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"robotLeaveScore",
	"robotLeaveScore",
	"-90",
	&robotLeaveScore,
	valInt,
	tuner_dummy,
	"Min score for robot to play (0=off).\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"robotLeaveRatio",
	"robotLeaveRatio",
	"-5",
	&robotLeaveRatio,
	valInt,
	tuner_dummy,
	"Min ratio for robot to play (0=off).\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"robotTeam",
	"robotTeam",
	"0",
	&robotTeam,
	valInt,
	tuner_dummy,
	"Team to use for robots.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"restrictRobots",
	"restrictRobots",
	"true",
	&restrictRobots,
	valBool,
	tuner_dummy,
	"Are robots restricted to their own team?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"reserveRobotTeam",
	"reserveRobotTeam",
	"true",
	&reserveRobotTeam,
	valBool,
	tuner_dummy,
	"Is the robot team only for robots?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"robotRealName",
	"robotRealName",
	"robot",
	&robotRealName,
	valString,
	Tune_robot_real_name,
	"What is the robots' apparent real name?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"robotHostName",
	"robotHostName",
	"xpilot.org",
	&robotHostName,
	valString,
	Tune_robot_host_name,
	"What is the robots' apparent host name?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"tankRealName",
	"tankRealName",
	"tank",
	&tankRealName,
	valString,
	Tune_tank_real_name,
	"What is the tanks' apparent real name?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"tankHostName",
	"tankHostName",
	"tanks.org",
	&tankHostName,
	valString,
	Tune_tank_host_name,
	"What is the tanks' apparent host name?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"tankScoreDecrement",
	"tankDecrement",
	"500",
	&tankScoreDecrement,
	valInt,
	tuner_dummy,
	"How much lower is the tank's score than the player's?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"turnThrust",
	"turnFuel",
	"false",
	&turnThrust,
	valBool,
	tuner_dummy,
	"Does turning use fuel and create sparks?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"selfImmunity",
	"selfImmunity",
	"false",
	&selfImmunity,
	valBool,
	tuner_dummy,
	"Are players immune to their own weapons?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"defaultShipShape",
	"defaultShipShape",
	"(NM:Default)(AU:Unknown)(SH: 15,0 -9,8 -9,-8)(MG: 15,0)(LG: 15,0)"
	"(RG: 15,0)(EN: -9,0)(LR: -9,8)(RR: -9,-8)(LL: -9,8)(RL: -9,-8)"
	"(MR: 15,0)",
	&defaultShipShape,
	valString,
	tuner_none,
	"What is the default ship shape for people who do not have a ship\n"
	"shape defined?",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"tankShipShape",
	"tankShipShape",
	"(NM:fueltank)(AU:John E. Norlin)"
	"(SH: 15,0 14,-5 9,-8 -5,-8 -3,-8 -3,0 "
	"2,0 2,2 -3,2 -3,6 5,6 5,8 -5,8 -5,-8 "
	"-9,-8 -14,-5 -15,0 -14,5 -9,8 9,8 14,5)"
	"(EN: -15,0)(MG: 15,0)",
	&tankShipShape,
	valString,
	tuner_none,
	"What is the ship shape used for tanks?",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxPlayerShots",
	"shots",
	"256",
	&ShotsMax,
	valInt,
	tuner_none,
	"Maximum allowed bullets per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"shotsGravity",
	"shotsGravity",
	"true",
	&ShotsGravity,
	valBool,
	tuner_dummy,
	"Are bullets afflicted by gravity.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"idleRun",
	"rawMode",
	"false",
	&RawMode,
	valBool,
	tuner_dummy,
	"Do robots keep on playing even if all human players quit?\n",
	OPT_COMMAND | OPT_DEFAULTS | OPT_VISIBLE
    },
    {
	"noQuit",
	"noQuit",
#ifdef _WINDOWS
	"true",
#else
	"false",
#endif
	&NoQuit,
	valBool,
	tuner_dummy,
	"Does the server wait for new human players to show up\n"
	"after all players have left.\n",
	OPT_COMMAND | OPT_DEFAULTS | OPT_VISIBLE
    },
    {
	"logRobots",
	"logRobots",
	"true",
	&logRobots,
	valBool,
	tuner_dummy,
	"Log the comings and goings of robots.\n",
	OPT_COMMAND | OPT_DEFAULTS | OPT_VISIBLE
    },
    {
	"mapWidth",
	"mapWidth",
	"100",
	&mapWidth,
	valInt,
	tuner_none,
	"Width of the world in blocks.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"mapHeight",
	"mapHeight",
	"100",
	&mapHeight,
	valInt,
	tuner_none,
	"Height of the world in blocks.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"mapFileName",
	"map",
	NULL,
	&mapFileName,
	valString,
	tuner_none,
	"The filename of the map to use.\n"
	"Or \"wild\" if you want a map by The Wild Map Generator.\n",
	OPT_COMMAND | OPT_DEFAULTS
    },
    {
	"mapName",
	"mapName",
	"unknown",
	&mapName,
	valString,
	tuner_none,
	"The title of the map.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"mapAuthor",
	"mapAuthor",
	"anonymous",
	&mapAuthor,
	valString,
	tuner_none,
	"The name of the map author.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"contactPort",
	"port",
	"15345",
	&contactPort,
	valInt,
	tuner_none,
	"The server contact port number.\n",
	OPT_COMMAND | OPT_DEFAULTS | OPT_VISIBLE
    },
    {
	"serverHost",
	"serverHost",
	NULL,
	&serverHost,
	valString,
	tuner_none,
	"The server's fully qualified domain name (for multihomed hosts).\n",
	OPT_COMMAND | OPT_DEFAULTS | OPT_VISIBLE
    },
    {
	"mapData",
	"mapData",
	NULL,
	&mapData,
	valString,
	tuner_none,
	"The map's topology.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"allowPlayerCrashes",
	"allowPlayerCrashes",
	"yes",
	&crashWithPlayer,
	valBool,
	Set_world_rules,
	"Can players overrun other players?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"allowPlayerBounces",
	"allowPlayerBounces",
	"yes",
	&bounceWithPlayer,
	valBool,
	Set_world_rules,
	"Can players bounce with other players?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"allowPlayerKilling",
	"killings",
	"yes",
	&playerKillings,
	valBool,
	tuner_none,
	"Should players be allowed to kill one other?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"allowShields",
	"shields",
	"yes",
	&playerShielding,
	valBool,
	tuner_none,
	"Are shields allowed?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"playerStartsShielded",
	"playerStartShielded",
	"yes",
	&playerStartsShielded,
	valBool,
	tuner_none,
	"Do players start with shields up?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"shotsWallBounce",
	"shotsWallBounce",
	"no",
	&shotsWallBounce,
	valBool,
	Move_init,
	"Do shots bounce off walls?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"ballsWallBounce",
	"ballsWallBounce",
	"yes",
	&ballsWallBounce,
	valBool,
	Move_init,
	"Do balls bounce off walls?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"minesWallBounce",
	"minesWallBounce",
	"no",
	&minesWallBounce,
	valBool,
	Move_init,
	"Do mines bounce off walls?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemsWallBounce",
	"itemsWallBounce",
	"yes",
	&itemsWallBounce,
	valBool,
	Move_init,
	"Do items bounce off walls?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"missilesWallBounce",
	"missilesWallBounce",
	"no",
	&missilesWallBounce,
	valBool,
	Move_init,
	"Do missiles bounce off walls?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"sparksWallBounce",
	"sparksWallBounce",
	"no",
	&sparksWallBounce,
	valBool,
	Move_init,
	"Do thrust spark particles bounce off walls to give reactive thrust?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"debrisWallBounce",
	"debrisWallBounce",
	"no",
	&debrisWallBounce,
	valBool,
	Move_init,
	"Do explosion debris particles bounce off walls?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"asteroidsWallBounce",
	"asteroidsWallBounce",
	"yes",
	&asteroidsWallBounce,
	valBool,
	Move_init,
	"Do asteroids bounce off walls?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"cloakedExhaust",
	"cloakedExhaust",
	"yes",
	&cloakedExhaust,
	valBool,
	tuner_none,
	"Do engines of cloaked ships generate exhaust?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"cloakedShield",
	"cloakedShield",
	"yes",
	&cloakedShield,
	valBool,
	tuner_none,
	"Can players use shields when cloaked?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxObjectWallBounceSpeed",
	"maxObjectBounceSpeed",
	"40",
	&maxObjectWallBounceSpeed,
	valReal,
	Move_init,
	"The maximum allowed speed for objects to bounce off walls.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxShieldedWallBounceSpeed",
	"maxShieldedBounceSpeed",
	"50",
	&maxShieldedWallBounceSpeed,
	valReal,
	Move_init,
	"The maximum allowed speed for a shielded player to bounce off walls.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxUnshieldedWallBounceSpeed",
	"maxUnshieldedBounceSpeed",
	"20",
	&maxUnshieldedWallBounceSpeed,
	valReal,
	Move_init,
	"Maximum allowed speed for an unshielded player to bounce off walls.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxShieldedPlayerWallBounceAngle",
	"maxShieldedBounceAngle",
	"90",
	&maxShieldedWallBounceAngle,
	valReal,
	Move_init,
	"Maximum allowed angle for a shielded player to bounce off walls.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxUnshieldedPlayerWallBounceAngle",
	"maxUnshieldedBounceAngle",
	"30",
	&maxUnshieldedWallBounceAngle,
	valReal,
	Move_init,
	"Maximum allowed angle for an unshielded player to bounce off walls.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"playerWallBounceBrakeFactor",
	"playerWallBrake",
	"0.89",
	&playerWallBrakeFactor,
	valReal,
	Move_init,
	"Factor to slow down players when they hit the wall (between 0 and 1).\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"objectWallBounceBrakeFactor",
	"objectWallBrake",
	"0.95",
	&objectWallBrakeFactor,
	valReal,
	Move_init,
	"Factor to slow down objects when they hit the wall (between 0 and 1).\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"objectWallBounceLifeFactor",
	"objectWallBounceLifeFactor",
	"0.80",
	&objectWallBounceLifeFactor,
	valReal,
	Move_init,
	"Factor to reduce the life of objects after bouncing (between 0 and 1).\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"wallBounceFuelDrainMult",
	"wallBounceDrain",
	"1.0",
	&wallBounceFuelDrainMult,
	valReal,
	Move_init,
	"Multiplication factor for player wall bounce fuel cost.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"wallBounceDestroyItemProb",
	"wallBounceDestroyItemProb",
	"0.0",
	&wallBounceDestroyItemProb,
	valReal,
	Move_init,
	"The probability for each item a player owns to get destroyed\n"
	"when the player bounces against a wall.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"reportToMetaServer",
	"reportMeta",
	"yes",
	&reportToMetaServer,
	valBool,
	tuner_none,
	"Keep the meta server informed about our game?\n",
	OPT_COMMAND | OPT_DEFAULTS | OPT_VISIBLE
    },
    {
	"searchDomainForXPilot",
	"searchDomainForXPilot",
	"no",
	&searchDomainForXPilot,
	valBool,
	tuner_none,
	"Search the local domain for the existence of xpilot.domain?\n",
	OPT_COMMAND | OPT_DEFAULTS | OPT_VISIBLE
    },
    {
	"denyHosts",
	"denyHosts",
	"",
	&denyHosts,
	valString,
	Set_deny_hosts,
	"List of network addresses of computers which are denied service.\n"
	"Each address may optionally be followed by a slash and a network mask.\n",
	OPT_COMMAND | OPT_DEFAULTS | OPT_VISIBLE
    },
    {
	"limitedVisibility",
	"limitedVisibility",
	"no",
	&limitedVisibility,
	valBool,
	Set_world_rules,
	"Should the players have a limited visibility?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"minVisibilityDistance",
	"minVisibility",
	"0.0",
	&minVisibilityDistance,
	valReal,
	tuner_none,
	"Minimum block distance for limited visibility, 0 for default.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxVisibilityDistance",
	"maxVisibility",
	"0.0",
	&maxVisibilityDistance,
	valReal,
	tuner_none,
	"Maximum block distance for limited visibility, 0 for default.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"limitedLives",
	"limitedLives",
	"no",
	&limitedLives,
	valBool,
	tuner_none,
	"Should players have limited lives?\n"
	"See also worldLives.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"worldLives",
	"lives",
	"0",
	&worldLives,
	valInt,
	tuner_none,
	"Number of lives each player has (no sense without limitedLives).\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"reset",
	"reset",
	"yes",
	&endOfRoundReset,
	valBool,
	tuner_dummy,
	"Does the world reset when the end of round is reached?\n"
	"When true all mines, missiles, shots and explosions will be\n"
	"removed from the world and all players including the winner(s)\n"
	"will be transported back to their homebases.\n"
	"This option is only effective when limitedLives is turned on.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"resetOnHuman",
	"humanReset",
	"0",
	&resetOnHuman,
	valInt,
	tuner_dummy,
	"Normally, new players have to wait until a round is finished\n"
	"before they can start playing. With this option, the first N\n"
	"human players to enter will cause the round to be restarted.\n"
	"In other words, if this option is set to 0, nothing special\n"
	"happens and you have to wait as usual until the round ends (if\n"
	"there are rounds at all, otherwise this option doesn't do\n"
	"anything). If it is set to 1, the round is ended when the first\n"
	"human player enters. This is useful if the robots have already\n"
	"started a round and you don't want to wait for them to finish.\n"
	"If it is set to 2, this also happens for the second human player.\n"
	"This is useful when you got bored waiting for another player to\n"
	"show up and have started playing against the robots. When someone\n"
	"finally joins you, they won't have to wait for you to finish the\n"
	"round before they can play too. For higher numbers it works the\n"
	"same. So this option gives the last human player for whom the\n"
	"round is restarted. Anyone who enters after that does have to\n"
	"wait until the round is over.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"teamPlay",
	"teams",
	"no",
	&teamPlay,
	valBool,
	tuner_none,
	"Is the map a team play map?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"teamFuel",
	"teamFuel",
	"no",
	&teamFuel,
	valBool,
	tuner_none,
	"Are fuelstations only available to team members?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"teamCannons",
	"teamCannons",
	"no",
	&teamCannons,
	valBool,
	tuner_none,
	"Do cannons choose sides in teamPlay?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"cannonSmartness",
	"cannonSmartness",
	"1",
	&cannonSmartness,
	valInt,
	tuner_none,
	"0: dumb (straight ahead),\n"
	"1: default (random direction),\n"
	"2: good (small error),\n"
	"3: accurate (aims at predicted player position).\n"
	"Also influences use of weapons if cannonsUseItems is on.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"cannonsUseItems",
	"cannonsPickupItems",
	"no",
	&cannonsUseItems,
	valBool,
	tuner_none,
	"Do cannons use items?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"cannonDeadTime",
	"cannonDeadTime",
	"72",
	&cannonDeadTime,
	valSec,
	tuner_dummy,
	"How many seconds do cannons stay dead?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"keepShots",
	"keepShots",
	"no",
	&keepShots,
	valBool,
	tuner_none,
	"Do shots, mines and missiles remain after their owner leaves?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"teamAssign",
	"teamAssign",
	"yes",
	&teamAssign,
	valBool,
	tuner_dummy,
	"If players have not specified which team they like to join\n"
	"should the server choose a team for them automatically?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"teamImmunity",
	"teamImmunity",
	"yes",
	&teamImmunity,
	valBool,
	tuner_dummy,
	"Should other team members be immune to various shots thrust etc.?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"ecmsReprogramMines",
	"ecmsReprogramMines",
	"yes",
	&ecmsReprogramMines,
	valBool,
	tuner_dummy,
	"Is it possible to reprogram mines with ECMs?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"ecmsReprogramRobots",
	"ecmsReprogramRobots",
	"yes",
	&ecmsReprogramRobots,
	valBool,
	tuner_dummy,
	"Are robots reprogrammed by ECMs instead of blinded?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"targetKillTeam",
	"targetKillTeam",
	"no",
	&targetKillTeam,
	valBool,
	tuner_dummy,
	"Do team members die when their last target explodes?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"targetTeamCollision",
	"targetCollision",
	"yes",
	&targetTeamCollision,
	valBool,
	tuner_dummy,
	"Do team members collide with their own target or not.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"targetSync",
	"targetSync",
	"no",
	&targetSync,
	valBool,
	tuner_dummy,
	"Do all the targets of a team reappear/repair at the same time?",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"targetDeadTime",
	"targetDeadTime",
	"60",
	&targetDeadTime,
	valSec,
	tuner_dummy,
	"How man seconds do targets stay destroyed?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"treasureKillTeam",
	"treasureKillTeam",
	"no",
	&treasureKillTeam,
	valBool,
	tuner_dummy,
	"Do team members die when their treasure is destroyed?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"captureTheFlag",
	"ctf",
	"no",
	&captureTheFlag,
	valBool,
	tuner_dummy,
	"Does a team's treasure have to be safe before enemy balls can be\n"
	"cashed?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"treasureCollisionDestroys",
	"treasureCollisionDestroy",
	"yes",
	&treasureCollisionDestroys,
	valBool,
	tuner_dummy,
	"Are balls destroyed when a player touches it?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"ballConnectorSpringConstant",
	"ballConnectorSpringConstant",
	"1500.0",
	&ballConnectorSpringConstant,
	valReal,
	tuner_dummy,
	"What is the spring constant for connectors?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"ballConnectorDamping",
	"ballConnectorDamping",
	"2.0",
	&ballConnectorDamping,
	valReal,
	tuner_dummy,
	"What is the damping force on connectors?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxBallConnectorRatio",
	"maxBallConnectorRatio",
	"0.30",
	&maxBallConnectorRatio,
	valReal,
	tuner_dummy,
	"How much longer or shorter can a connecter get before it breaks?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"ballConnectorLength",
	"ballConnectorLength",
	"120",
	&ballConnectorLength,
	valReal,
	tuner_dummy,
	"How long is a normal connector string?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"treasureCollisionMayKill",
	"treasureUnshieldedCollisionKills",
	"no",
	&treasureCollisionMayKill,
	valBool,
	tuner_dummy,
	"Does a ball kill a player when the player touches it unshielded?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"wreckageCollisionMayKill",
	"wreckageUnshieldedCollisionKills",
	"no",
	&wreckageCollisionMayKill,
	valBool,
	tuner_dummy,
	"Can ships be destroyed when hit by wreckage?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"asteroidCollisionMayKill",
	"asteroidUnshieldedCollisionKills",
	"yes",
	&asteroidCollisionMayKill,
	valBool,
	tuner_dummy,
	"Can ships be destroyed when hit by an asteroid?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {   "ignore20MaxFPS",
	"ignore20MaxFPS",
	"true",
	&ignore20MaxFPS,
	valBool,
	tuner_dummy,
	"Ignore client maxFPS request if it is 20 (the default setting).\n",
	OPT_COMMAND | OPT_DEFAULTS | OPT_VISIBLE
    },
    {
	"timing",
	"race",
	"no",
	&timing,
	valBool,
	tuner_none,
	"Is the map a race mode map?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"edgeWrap",
	"edgeWrap",
	"no",
	&edgeWrap,
	valBool,
	tuner_none,
	"Wrap around edges.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"edgeBounce",
	"edgeBounce",
	"yes",
	&edgeBounce,
	valBool,
	tuner_dummy,
	"Players and bullets bounce when they hit the (non-wrapping) edge.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"extraBorder",
	"extraBorder",
	"no",
	&extraBorder,
	valBool,
	tuner_none,
	"Give map an extra border of solid rock.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"gravityPoint",
	"gravityPoint",
	"0,0",
	&gravityPoint,
	valIPos,
	tuner_none,
	"If the gravity is a point source where does that gravity originate?\n"
	"Specify the point int the form: x,y.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"gravityAngle",
	"gravityAngle",
	"90",
	&gravityAngle,
	valReal,
	tuner_none,
	"If gravity is along a uniform line, at what angle is that line?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"gravityPointSource",
	"gravityPointSource",
	"false",
	&gravityPointSource,
	valBool,
	tuner_none,
	"Is gravity originating from a single point?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"gravityClockwise",
	"gravityClockwise",
	"false",
	&gravityClockwise,
	valBool,
	tuner_none,
	"If the gravity is a point source, is it clockwise?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"gravityAnticlockwise",
	"gravityAnticlockwise",
	"false",
	&gravityAnticlockwise,
	valBool,
	tuner_none,
	"If the gravity is a point source, is it anticlockwise?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"gravityVisible",
	"gravityVisible",
	"true",
	&gravityVisible,
	valBool,
	tuner_none,
	"Are gravity mapsymbols visible to players?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"wormholeVisible",
	"wormholeVisible",
	"true",
	&wormholeVisible,
	valBool,
	tuner_none,
	"Are wormhole mapsymbols visible to players?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemConcentratorVisible",
	"itemConcentratorVisible",
	"true",
	&itemConcentratorVisible,
	valBool,
	tuner_none,
	"Are itemconcentrator mapsymbols visible to players?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"wormTime",
	"wormTime",
	"0",
	&wormTime,
	valSec,
	tuner_none,
	"Number of seconds wormholes will remain stable.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"defaultsFileName",
	"defaults",
	NULL,
	&defaultsFileName,
	valString,
	tuner_none,
	"The filename of the defaults file to read on startup.\n",
	OPT_COMMAND,
    },
    {
	"passwordFileName",
	"passwordFileName",
	NULL,
	&passwordFileName,
	valString,
	tuner_none,
	"The filename of the password file to read on startup.\n",
	OPT_COMMAND | OPT_DEFAULTS,
    },
    {
	"motdFileName",
	"motd",
	NULL,
	&motdFileName,
	valString,
	tuner_none,
	"The filename of the MOTD file to show to clients when they join.\n",
	OPT_COMMAND | OPT_DEFAULTS,
    },
    {
	"scoreTableFileName",
	"scoretable",
	NULL,
	&scoreTableFileName,
	valString,
	tuner_none,
	"The filename for the score table to be dumped to.\n",
	OPT_COMMAND | OPT_DEFAULTS
    },
    {
	"framesPerSecond",
	"FPS",
	"14",
	&framesPerSecond,
	valInt,
	tuner_none,
	"The number of frames per second the server should strive for.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"allowSmartMissiles",
	"allowSmarts",
	"True",
	&allowSmartMissiles,
	valBool,
	tuner_dummy,
	"Should smart missiles be allowed?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"allowHeatSeekers",
	"allowHeats",
	"True",
	&allowHeatSeekers,
	valBool,
	tuner_dummy,
	"Should heatseekers be allowed?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"allowTorpedoes",
	"allowTorps",
	"True",
	&allowTorpedoes,
	valBool,
	tuner_dummy,
	"Should torpedoes be allowed?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"allowNukes",
	"nukes",
	"False",
	&allowNukes,
	valBool,
	tuner_none,
	"Should nuclear weapons be allowed?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"allowClusters",
	"clusters",
	"False",
	&allowClusters,
	valBool,
	tuner_none,
	"Should cluster weapons be allowed?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"allowModifiers",
	"modifiers",
	"False",
	&allowModifiers,
	valBool,
	tuner_none,
	"Should the weapon modifiers be allowed?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"allowLaserModifiers",
	"lasermodifiers",
	"False",
	&allowLaserModifiers,
	valBool,
	tuner_none,
	"Can lasers be modified to be a different weapon?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"allowShipShapes",
	"ShipShapes",
	"True",
	&allowShipShapes,
	valBool,
	tuner_none,
	"Are players allowed to define their own ship shape?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"playersOnRadar",
	"playersRadar",
	"True",
	&playersOnRadar,
	valBool,
	tuner_dummy,
	"Are players visible on the radar.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"missilesOnRadar",
	"missilesRadar",
	"True",
	&missilesOnRadar,
	valBool,
	tuner_dummy,
	"Are missiles visible on the radar.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"minesOnRadar",
	"minesRadar",
	"False",
	&minesOnRadar,
	valBool,
	tuner_dummy,
	"Are mines visible on the radar.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"nukesOnRadar",
	"nukesRadar",
	"True",
	&nukesOnRadar,
	valBool,
	tuner_dummy,
	"Are nukes visible or highlighted on the radar.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"treasuresOnRadar",
	"treasuresRadar",
	"False",
	&treasuresOnRadar,
	valBool,
	tuner_dummy,
	"Are treasure balls visible or highlighted on the radar.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"asteroidsOnRadar",
	"asteroidsRadar",
	"False",
	&asteroidsOnRadar,
	valBool,
	tuner_dummy,
	"Are asteroids visible on the radar.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"distinguishMissiles",
	"distinguishMissiles",
	"True",
	&distinguishMissiles,
	valBool,
	tuner_dummy,
	"Are different types of missiles distinguished (by length).\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxMissilesPerPack",
	"maxMissilesPerPack",
	"4",
	&maxMissilesPerPack,
	valInt,
	Tune_item_packs,
	"The number of missiles gotten by picking up one missile item.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxMinesPerPack",
	"maxMinesPerPack",
	"2",
	&maxMinesPerPack,
	valInt,
	Tune_item_packs,
	"The number of mines gotten by picking up one mine item.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"identifyMines",
	"identifyMines",
	"True",
	&identifyMines,
	valBool,
	tuner_dummy,
	"Are mine owner's names displayed.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"shieldedItemPickup",
	"shieldItem",
	"False",
	&shieldedItemPickup,
	valBool,
	tuner_dummy,
	"Can items be picked up while shields are up?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"shieldedMining",
	"shieldMine",
	"False",
	&shieldedMining,
	valBool,
	tuner_dummy,
	"Can mines be thrown and placed while shields are up?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"laserIsStunGun",
	"stunGun",
	"False",
	&laserIsStunGun,
	valBool,
	tuner_dummy,
	"Is the laser weapon a stun gun weapon?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"nukeMinSmarts",
	"nukeMinSmarts",
	"7",
	&nukeMinSmarts,
	valInt,
	tuner_dummy,
	"The minimum number of missiles needed to fire one nuclear missile.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"nukeMinMines",
	"nukeMinMines",
	"4",
	&nukeMinMines,
	valInt,
	tuner_dummy,
	"The minimum number of mines needed to make a nuclear mine.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"nukeClusterDamage",
	"nukeClusterDamage",
	"1.0",
	&nukeClusterDamage,
	valReal,
	tuner_dummy,
	"How much each cluster debris does damage wise from a nuke mine.\n"
	"This helps to reduce the number of particles caused by nuclear mine\n"
	"explosions, which improves server response time for such explosions.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"mineFuseTime",
	"mineFuseTime",
	"0.0",
	&mineFuseTime,
	valSec,
	tuner_dummy,
	"Number of seconds after which owned mines become deadly, zero means never.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"mineLife",
	"mineLife",
	"0",
	&mineLife,
	valInt,
	tuner_none,
	"Life of mines in ticks, zero means use default.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"minMineSpeed",
	"minMineSpeed",
	"0",
	&minMineSpeed,
	valReal,
	tuner_dummy,
	"Minimum speed of mines.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"missileLife",
	"missileLife",
	"0",
	&missileLife,
	valInt,
	tuner_none,
	"Life of missiles in ticks, zero means use default.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"baseMineRange",
	"baseMineRange",
	"0",
	&baseMineRange,
	valInt,
	tuner_dummy,
	"Range within which mines/bombs are not allowed.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"shotKillScoreMult",
	"shotKillScoreMult",
	"1.0",
	&shotKillScoreMult,
	valReal,
	tuner_none,
	"Multiplication factor to scale score for shot kills.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
        "torpedoKillScoreMult",
        "torpedoKillScoreMult",
        "1.0",
        &torpedoKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for torpedo kills.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
        "smartKillScoreMult",
        "smartKillScoreMult",
        "1.0",
        &smartKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for smart missile kills.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
        "heatKillScoreMult",
        "heatKillScoreMult",
        "1.0",
        &heatKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for heatseeker kills.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
        "clusterKillScoreMult",
        "clusterKillScoreMult",
        "1.0",
        &clusterKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for cluster debris kills.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
        "laserKillScoreMult",
        "laserKillScoreMult",
        "1.0",
        &laserKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for laser kills.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
        "tankKillScoreMult",
        "tankKillScoreMult",
        "0.44",
        &tankKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for tank kills.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
        "runoverKillScoreMult",
        "runoverKillScoreMult",
        "0.33",
        &runoverKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for player runovers.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
        "ballKillScoreMult",
        "ballKillScoreMult",
        "1.0",
        &ballKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for ball kills.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
        "explosionKillScoreMult",
        "explosionKillScoreMult",
        "0.33",
        &explosionKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for explosion kills.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
        "shoveKillScoreMult",
        "shoveKillScoreMult",
        "0.5",
        &shoveKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for shove kills.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
        "crashScoreMult",
        "crashScoreMult",
        "0.33",
        &crashScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for player crashes.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
        "mineScoreMult",
        "mineScoreMult",
        "0.17",
        &mineScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for mine hits.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"asteroidScoring",
	"asteroidScoring",
	"yes",
	&asteroidScoring,
	valBool,
	tuner_none,
	"Are points awarded for shooting asteroids?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"movingItemProb",
	"movingItemProb",
	"0.2",
	&movingItemProb,
	valReal,
	Set_misc_item_limits,
	"Probability for an item to appear as moving.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"randomItemProb",
	"randomItemProb",
	"0.0",
	&randomItemProb,
	valReal,
	Set_misc_item_limits,
	"Probability for an item to appear random.\n"
	"Random items change their appearance every frame, so players\n"
	"cannot tell what item they have until they get it.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"dropItemOnKillProb",
	"dropItemOnKillProb",
	"0.5",
	&dropItemOnKillProb,
	valReal,
	Set_misc_item_limits,
	"Probability for dropping an item (each item) when you are killed.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"detonateItemOnKillProb",
	"detonateItemOnKillProb",
	"0.5",
	&detonateItemOnKillProb,
	valReal,
	Set_misc_item_limits,
	"Probability for undropped items to detonate when you are killed.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"destroyItemInCollisionProb",
	"destroyItemInCollisionProb",
	"0.0",
	&destroyItemInCollisionProb,
	valReal,
	Set_misc_item_limits,
	"Probability for items (some items) to be destroyed in a collision.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemProbMult",
	"itemProbFact",
	"1.0",
	&itemProbMult,
	valReal,
	Tune_item_probs,
	"Item Probability Multiplication Factor scales all item probabilities.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"cannonItemProbMult",
	"cannonItemProbMult",
	"1.0",
	&cannonItemProbMult,
	valReal,
	tuner_none,
	"Average number of items a cannon gets per minute.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxItemDensity",
	"maxItemDensity",
	"0.00012",
	&maxItemDensity,
	valReal,
	Tune_item_probs,
	"Maximum density [0.0-1.0] for items (max items per block).\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"asteroidProb",
	"asteroidProb",
	"5e-7",
	&World.asteroids.prob,
	valReal,
	Tune_asteroid_prob,
	"Probability for an asteroid to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxAsteroidDensity",
	"maxAsteroidDensity",
	"0",
	&maxAsteroidDensity,
	valReal,
	Tune_asteroid_prob,
	"Maximum density [0.0-1.0] for asteroids (max asteroids per block.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemConcentratorRadius",
	"itemConcentratorRange",
	"10",
	&itemConcentratorRadius,
	valInt,
	Set_misc_item_limits,
	"The maximum distance from an item concentator for items to appear in.\n"
	"Sensible values are in the range 1 to 20.\n"
	"If no item concentators are defined in a map then items can popup anywhere.\n"
	"If any are any then items popup in the vicinity of an item concentrator\n"
	"with probability itemConcentratorProb and anywhere the remainder of the time.\n"
	"An item concentrator is drawn on screen as three rotating triangles.\n"
	"The map symbol is the percentage symbol '%'.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemConcentratorProb",
	"itemConcentratorProb",
	"1.0",
	&itemConcentratorProb,
	valReal,
	Set_misc_item_limits,
	"The probability, if any item concentrators are present, that they will be\n"
	"used.  This proportion of items will be placed near item concentrators,\n"
	"within itemConcentratorRadius.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"rogueHeatProb",
	"rogueHeatProb",
	"1.0",
	&rogueHeatProb,
	valReal,
	tuner_dummy,
	"Probability that unclaimed missile packs will go rogue.",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"rogueMineProb",
	"rogueMineProb",
	"1.0",
	&rogueMineProb,
	valReal,
	tuner_dummy,
	"Probability that unclaimed mine items will activate.",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemEnergyPackProb",
	"itemEnergyPackProb",
	"1e-9",
	&World.items[ITEM_FUEL].prob,
	valReal,
	Tune_item_probs,
	"Probability for an energy pack to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemTankProb",
	"itemTankProb",
	"1e-9",
	&World.items[ITEM_TANK].prob,
	valReal,
	Tune_item_probs,
	"Probability for an extra tank to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemECMProb",
	"itemECMProb",
	"1e-9",
	&World.items[ITEM_ECM].prob,
	valReal,
	Tune_item_probs,
	"Probability for an ECM item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemArmorProb",
	"itemArmorProb",
	"1e-9",
	&World.items[ITEM_ARMOR].prob,
	valReal,
	Tune_item_probs,
	"Probability for an armor item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemMineProb",
	"itemMineProb",
	"1e-9",
	&World.items[ITEM_MINE].prob,
	valReal,
	Tune_item_probs,
	"Probability for a mine item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemMissileProb",
	"itemMissileProb",
	"1e-9",
	&World.items[ITEM_MISSILE].prob,
	valReal,
	Tune_item_probs,
	"Probability for a missile item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemCloakProb",
	"itemCloakProb",
	"1e-9",
	&World.items[ITEM_CLOAK].prob,
	valReal,
	Tune_item_probs,
	"Probability for a cloak item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemSensorProb",
	"itemSensorProb",
	"1e-9",
	&World.items[ITEM_SENSOR].prob,
	valReal,
	Tune_item_probs,
	"Probability for a sensor item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemWideangleProb",
	"itemWideangleProb",
	"1e-9",
	&World.items[ITEM_WIDEANGLE].prob,
	valReal,
	Tune_item_probs,
	"Probability for a wideangle item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemRearshotProb",
	"itemRearshotProb",
	"1e-9",
	&World.items[ITEM_REARSHOT].prob,
	valReal,
	Tune_item_probs,
	"Probability for a rearshot item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemAfterburnerProb",
	"itemAfterburnerProb",
	"1e-9",
	&World.items[ITEM_AFTERBURNER].prob,
	valReal,
	Tune_item_probs,
	"Probability for an afterburner item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemTransporterProb",
	"itemTransporterProb",
	"1e-9",
	&World.items[ITEM_TRANSPORTER].prob,
	valReal,
	Tune_item_probs,
	"Probability for a transporter item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemMirrorProb",
	"itemMirrorProb",
	"1e-9",
	&World.items[ITEM_MIRROR].prob,
	valReal,
	Tune_item_probs,
	"Probability for a mirror item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemDeflectorProb",
	"itemDeflectorProb",
	"1e-9",
	&World.items[ITEM_DEFLECTOR].prob,
	valReal,
	Tune_item_probs,
	"Probability for a deflector item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemHyperJumpProb",
	"itemHyperJumpProb",
	"1e-9",
	&World.items[ITEM_HYPERJUMP].prob,
	valReal,
	Tune_item_probs,
	"Probability for a hyperjump item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemPhasingProb",
	"itemPhasingProb",
	"1e-9",
	&World.items[ITEM_PHASING].prob,
	valReal,
	Tune_item_probs,
	"Probability for a phasing item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemLaserProb",
	"itemLaserProb",
	"1e-9",
	&World.items[ITEM_LASER].prob,
	valReal,
	Tune_item_probs,
	"Probability for a Laser item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemEmergencyThrustProb",
	"itemEmergencyThrustProb",
	"1e-9",
	&World.items[ITEM_EMERGENCY_THRUST].prob,
	valReal,
	Tune_item_probs,
	"Probability for an Emergency Thrust item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemTractorBeamProb",
	"itemTractorBeamProb",
	"1e-9",
	&World.items[ITEM_TRACTOR_BEAM].prob,
	valReal,
	Tune_item_probs,
	"Probability for a Tractor Beam item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemAutopilotProb",
	"itemAutopilotProb",
	"1e-9",
	&World.items[ITEM_AUTOPILOT].prob,
	valReal,
	Tune_item_probs,
	"Probability for an Autopilot item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"itemEmergencyShieldProb",
	"itemEmergencyShieldProb",
	"1e-9",
	&World.items[ITEM_EMERGENCY_SHIELD].prob,
	valReal,
	Tune_item_probs,
	"Probability for an Emergency Shield item to appear.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialFuel",
	"initialFuel",
	"1000",
	&World.items[ITEM_FUEL].initial,
	valInt,
	Set_initial_resources,
	"How much fuel players start with, or the minimum after being killed.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialTanks",
	"initialTanks",
	"0",
	&World.items[ITEM_TANK].initial,
	valInt,
	Set_initial_resources,
	"How many tanks players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialECMs",
	"initialECMs",
	"0",
	&World.items[ITEM_ECM].initial,
	valInt,
	Set_initial_resources,
	"How many ECMs players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialArmor",
	"initialArmors",
	"0",
	&World.items[ITEM_ARMOR].initial,
	valInt,
	Set_initial_resources,
	"How much armor players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialMines",
	"initialMines",
	"0",
	&World.items[ITEM_MINE].initial,
	valInt,
	Set_initial_resources,
	"How many mines players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialMissiles",
	"initialMissiles",
	"0",
	&World.items[ITEM_MISSILE].initial,
	valInt,
	Set_initial_resources,
	"How many missiles players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialCloaks",
	"initialCloaks",
	"0",
	&World.items[ITEM_CLOAK].initial,
	valInt,
	Set_initial_resources,
	"How many cloaks players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialSensors",
	"initialSensors",
	"0",
	&World.items[ITEM_SENSOR].initial,
	valInt,
	Set_initial_resources,
	"How many sensors players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialWideangles",
	"initialWideangles",
	"0",
	&World.items[ITEM_WIDEANGLE].initial,
	valInt,
	Set_initial_resources,
	"How many wideangles players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialRearshots",
	"initialRearshots",
	"0",
	&World.items[ITEM_REARSHOT].initial,
	valInt,
	Set_initial_resources,
	"How many rearshots players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialAfterburners",
	"initialAfterburners",
	"0",
	&World.items[ITEM_AFTERBURNER].initial,
	valInt,
	Set_initial_resources,
	"How many afterburners players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialTransporters",
	"initialTransporters",
	"0",
	&World.items[ITEM_TRANSPORTER].initial,
	valInt,
	Set_initial_resources,
	"How many transporters players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialMirrors",
	"initialMirrors",
	"0",
	&World.items[ITEM_MIRROR].initial,
	valInt,
	Set_initial_resources,
	"How many mirrors players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialDeflectors",
	"initialDeflectors",
	"0",
	&World.items[ITEM_DEFLECTOR].initial,
	valInt,
	Set_initial_resources,
	"How many deflectors players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialHyperJumps",
	"initialHyperJumps",
	"0",
	&World.items[ITEM_HYPERJUMP].initial,
	valInt,
	Set_initial_resources,
	"How many hyperjumps players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialPhasings",
	"initialPhasings",
	"0",
	&World.items[ITEM_PHASING].initial,
	valInt,
	Set_initial_resources,
	"How many phasing devices players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialLasers",
	"initialLasers",
	"0",
	&World.items[ITEM_LASER].initial,
	valInt,
	Set_initial_resources,
	"How many lasers players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialEmergencyThrusts",
	"initialEmergencyThrusts",
	"0",
	&World.items[ITEM_EMERGENCY_THRUST].initial,
	valInt,
	Set_initial_resources,
	"How many emergency thrusts players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialTractorBeams",
	"initialTractorBeams",
	"0",
	&World.items[ITEM_TRACTOR_BEAM].initial,
	valInt,
	Set_initial_resources,
	"How many tractor/pressor beams players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialAutopilots",
	"initialAutopilots",
	"0",
	&World.items[ITEM_AUTOPILOT].initial,
	valInt,
	Set_initial_resources,
	"How many autopilots players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"initialEmergencyShields",
	"initialEmergencyShields",
	"0",
	&World.items[ITEM_EMERGENCY_SHIELD].initial,
	valInt,
	Set_initial_resources,
	"How many emergency shields players start with.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxFuel",
	"maxFuel",
	"10000",
	&World.items[ITEM_FUEL].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the amount of fuel per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxTanks",
	"maxTanks",
	"8",
	&World.items[ITEM_TANK].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of tanks per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxECMs",
	"maxECMs",
	"10",
	&World.items[ITEM_ECM].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of ECMs per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxMines",
	"maxMines",
	"10",
	&World.items[ITEM_MINE].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of mines per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxMissiles",
	"maxMissiles",
	"10",
	&World.items[ITEM_MISSILE].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of missiles per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxCloaks",
	"maxCloaks",
	"10",
	&World.items[ITEM_CLOAK].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of cloaks per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxSensors",
	"maxSensors",
	"10",
	&World.items[ITEM_SENSOR].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of sensors per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxWideangles",
	"maxWideangles",
	"10",
	&World.items[ITEM_WIDEANGLE].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of wides per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxRearshots",
	"maxRearshots",
	"10",
	&World.items[ITEM_REARSHOT].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of rearshots per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxAfterburners",
	"maxAfterburners",
	"10",
	&World.items[ITEM_AFTERBURNER].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of afterburners per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxTransporters",
	"maxTransporters",
	"10",
	&World.items[ITEM_TRANSPORTER].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of transporters per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxArmor",
	"maxArmors",
	"10",
	&World.items[ITEM_ARMOR].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the amount of armor per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxMirrors",
	"maxMirrors",
	"10",
	&World.items[ITEM_MIRROR].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of mirrors per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxDeflectors",
	"maxDeflectors",
	"10",
	&World.items[ITEM_DEFLECTOR].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of deflectors per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxPhasings",
	"maxPhasings",
	"10",
	&World.items[ITEM_PHASING].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of phasing devices per players.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxHyperJumps",
	"maxHyperJumps",
	"10",
	&World.items[ITEM_HYPERJUMP].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of hyperjumps per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxEmergencyThrusts",
	"maxEmergencyThrusts",
	"10",
	&World.items[ITEM_EMERGENCY_THRUST].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of emergency thrusts per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxLasers",
	"maxLasers",
	"5",
	&World.items[ITEM_LASER].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of lasers per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxTractorBeams",
	"maxTractorBeams",
	"4",
	&World.items[ITEM_TRACTOR_BEAM].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of tractorbeams per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxAutopilots",
	"maxAutopilots",
	"10",
	&World.items[ITEM_AUTOPILOT].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of autopilots per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxEmergencyShields",
	"maxEmergencyShields",
	"10",
	&World.items[ITEM_EMERGENCY_SHIELD].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of emergency shields per player.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"gameDuration",
	"time",
	"0.0",
	&gameDuration,
	valReal,
	tuner_none,
	"The duration of the game in minutes (aka. pizza mode).\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"allowViewing",
	"allowViewing",
	"false",
	&allowViewing,
	valBool,
	tuner_dummy,
	"Are players allowed to watch any other player while paused, waiting or dead?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"friction",
	"friction",
	"0.0",
	&friction,
	valReal,
	tuner_dummy,
	"Fraction of velocity ship loses each frame.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"blockFriction",
	"blockFriction",
	"0.0",
	&blockFriction,
	valReal,
	tuner_dummy,
	"Fraction of velocity ship loses each frame when it is in friction blocks.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"blockFrictionVisible",
	"blockFrictionVisible",
	"true",
	&blockFrictionVisible,
	valBool,
	tuner_none,
	"Are friction blocks visible?\n"
	"If true, friction blocks show up as decor; if false, they don't show up at all.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"coriolis",
	"coriolis",
	"0",
	&coriolis,
	valInt,
	tuner_dummy,
	"The clockwise angle (in degrees) a ship's velocity turns each frame.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"checkpointRadius",
	"checkpointRadius",
	"6.0",
	&checkpointRadius,
	valReal,
	tuner_dummy,
	"How close you have to be to a checkpoint to register - in blocks.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"raceLaps",
	"raceLaps",
	"3",
	&raceLaps,
	valInt,
	tuner_none,
	"How many laps a race is run over.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"lockOtherTeam",
	"lockOtherTeam",
	"true",
	&lockOtherTeam,
	valBool,
	tuner_none,
	"Can you lock on players from other teams when you're dead.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"loseItemDestroys",
	"loseItemDestroys",
	"false",
	&loseItemDestroys,
	valBool,
	tuner_none,
	"Destroy item that player drops. Otherwise drop it.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"useWreckage",
	"useWreckage",
	"true",
	&useWreckage,
	valBool,
	tuner_dummy,
	"Do destroyed ships leave wreckage?\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxOffensiveItems",
	"maxOffensiveItems",
	"100",
	&maxOffensiveItems,
	valInt,
	tuner_dummy,
	"How many offensive items a player can carry.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxDefensiveItems",
	"maxDefensiveItems",
	"100",
	&maxDefensiveItems,
	valInt,
	tuner_dummy,
	"How many defensive items a player can carry.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"roundDelay",
	"roundDelaySeconds",
	"0",
	&roundDelaySeconds,
	valInt,
	tuner_dummy,
	"Delay before start of each round, in seconds.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxRoundTime",
	"maxRoundTime",
	"0",
	&maxRoundTime,
	valInt,
	tuner_dummy,
	"The maximum duration of each round, in seconds.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"roundsToPlay",
	"roundsToPlay",
	"0",
	&roundsToPlay,
	valInt,
	tuner_dummy,
	"The number of rounds to play.  Unlimited if 0.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"maxVisibleObject",
	"maxVisibleObjects",
	"1000",
	&maxVisibleObject,
	valInt,
	tuner_dummy,
	"What is the maximum number of objects a player can see.\n",
	OPT_ORIGIN_ANY | OPT_VISIBLE
    },
    {
	"pLockServer",
	"pLockServer",
#ifdef PLOCKSERVER
	"true",
#else
	"false",
#endif
	&pLockServer,
	valBool,
	tuner_plock,
	"Whether the server is prevented from being swapped out of memory.\n",
	OPT_COMMAND | OPT_DEFAULTS | OPT_VISIBLE
    },
    {
	"timerResolution",
	"timerResolution",
	"0",
	&timerResolution,
	valInt,
	tuner_none,
	"If set to nonzero xpilots will requests signals from the OS at\n"
	"1/timerResolution second intervals.  The server will then compute\n"
	"a new frame FPS times out of every timerResolution signals.\n",
	OPT_COMMAND | OPT_DEFAULTS | OPT_VISIBLE
    },
    {
	"password",
	"password",
	NULL,
	&password,
	valString,
	tuner_dummy,
	"The password needed to obtain operator privileges.\n"
        "If specified on the command line, on many systems other\n"
	"users will be able to see the password.  Therefore, using\n"
	"the password file instead is recommended.",
	OPT_COMMAND | OPT_DEFAULTS | OPT_PASSWORD
    },
    {
	"clientPortStart",
	"clientPortStart",
	"0",
	&clientPortStart,
	valInt,
	tuner_dummy,
	"Use UDP ports clientPortStart - clientPortEnd (for firewalls)\n",
	OPT_COMMAND | OPT_DEFAULTS | OPT_VISIBLE
    },
    {
	"clientPortEnd",
	"clientPortEnd",
	"0",
	&clientPortEnd,
	valInt,
	tuner_dummy,
	"Use UDP ports clientPortStart - clientPortEnd (for firewalls)\n",
	OPT_COMMAND | OPT_DEFAULTS | OPT_VISIBLE
    },

};


static bool options_inited = FALSE;


option_desc* Get_option_descs(int *count_ptr)
{
    if (options_inited != TRUE) {
	dumpcore("options not initialized");
    }

    *count_ptr = NELEM(options);
    return &options[0];
}


static void Init_default_options(void)
{
    option_desc*	desc;

    if ((desc = Find_option_by_name("mapFileName")) == NULL) {
	dumpcore("Could not find map file option");
    }
    desc->defaultValue = Conf_default_map();

    if ((desc = Find_option_by_name("motdFileName")) == NULL) {
	dumpcore("Could not find motd file option");
    }
    desc->defaultValue = Conf_servermotdfile();

    if ((desc = Find_option_by_name("robotFile")) == NULL) {
	dumpcore("Could not find robot file option");
    }
    desc->defaultValue = Conf_robotfile();

    if ((desc = Find_option_by_name("defaultsFileName")) == NULL) {
	dumpcore("Could not find defaults file option");
    }
    desc->defaultValue = Conf_defaults_file_name();

    if ((desc = Find_option_by_name("passwordFileName")) == NULL) {
	dumpcore("Could not find password file option");
    }
    desc->defaultValue = Conf_password_file_name();
}


bool Init_options(void)
{
    int			i;
    int			option_count = NELEM(options);

    if (options_inited != FALSE) {
	dumpcore("Can't init options twice.");
    }

    Init_default_options();

    for (i = 0; i < option_count; i++) {
	if (Option_add_desc(&options[i]) == FALSE) {
	    return FALSE;
	}
    }

    options_inited = TRUE;

    return TRUE;
}


void Free_options(void)
{
    int			i;
    int			option_count = NELEM(options);

    if (options_inited == TRUE) {
	options_inited = FALSE;
	for (i = 0; i < option_count; i++) {
	    if (options[i].type == valString) {
		char **str_ptr = (char **)options[i].variable;
		char *str = *str_ptr;
		if (str != NULL && str != options[i].defaultValue) {
		    free(str);
		    *str_ptr = NULL;
		}
	    }
	}
    }
}


option_desc* Find_option_by_name(const char* name)
{
    int			j;
    int			option_count = NELEM(options);

    for (j = 0; j < option_count; j++) {
	if (!strcasecmp(options[j].commandLineOption, name)
	    || !strcasecmp(options[j].name, name))
	{
	    return(&options[j]);
	}
    }
    return NULL;
}


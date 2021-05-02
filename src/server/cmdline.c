/* $Id: cmdline.c,v 4.22 1999/11/07 11:57:30 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
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

#ifdef	_WINDOWS
#include "NT/winServer.h"
#else
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

char cmdline_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: cmdline.c,v 4.22 1999/11/07 11:57:30 bert Exp $";
#endif

DFLOAT		Gravity;		/* Power of gravity */
DFLOAT		ShipMass;		/* Default mass of ship */
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
char		*mapFileName;		/* Name of mapfile... */
char		*mapData;		/* Raw map data... */
int		mapWidth;		/* Width of the universe */
int		mapHeight;		/* Height of the universe */
char		*mapName;		/* Name of the universe */
char		*mapAuthor;		/* Name of the creator */
int		contactPort;		/* Contact port number */

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
int		resetOnHuman;		/* Restart when human enters? */
bool		teamPlay;		/* Are teams allowed? */
bool		teamFuel;		/* Do fuelstations belong to teams? */
bool		teamCannons;		/* Do cannons belong to teams? */
int		cannonSmartness;	/* Accuracy of cannonfire */
bool		cannonsUseItems;	/* Do cannons use items? */
bool		keepShots;		/* Keep shots when player leaves? */
bool		onePlayerOnly;		/* Can there be multiple players? */
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
char		*scoreTableFileName;	/* Name of score table file */

int		nukeMinSmarts;		/* minimum smarts for a nuke */
int		nukeMinMines;		/* minimum number of mines for nuke */
DFLOAT		nukeClusterDamage;	/* multiplier for damage from nuke */
					/* cluster debris, reduces number */
					/* of particles by similar amount */
int		mineFuseTime;		/* Length of time mine is fused */
int		mineLife;		/* lifetime of mines */
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

DFLOAT 		movingItemProb;		/* Probability for moving items */
DFLOAT		dropItemOnKillProb;	/* Probability for players items to */
					/* drop when player is killed */
DFLOAT		detonateItemOnKillProb;	/* Probaility for remaining items to */
					/* detonate when player is killed */
DFLOAT		destroyItemInCollisionProb;
DFLOAT           rogueHeatProb;          /* prob. that unclaimed rocketpack */
DFLOAT           rogueMineProb;          /* or minepack will "activate" */
DFLOAT		itemProbMult;
DFLOAT		cannonItemProbMult;
DFLOAT		maxItemDensity;
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
bool		distinguishMissiles;	/* Smarts, heats & torps look diff.? */
int		maxMissilesPerPack;	/* Number of missiles per item. */
int		maxMinesPerPack;	/* Number of mines per item. */
bool		identifyMines;		/* Mines have names displayed? */
bool		shieldedItemPickup;	/* Pickup items with shields up? */
bool		shieldedMining;		/* Detach mines with shields up? */
bool		laserIsStunGun;		/* Is the laser a stun gun? */
bool		reportToMetaServer;	/* Send status to meta-server? */
char		*denyHosts;		/* Computers which are denied service */
DFLOAT		gameDuration;		/* total duration of game in minutes */
bool		allowViewing;		/* Are players allowed to watch others? */

bool		teamAssign;		/* Assign player to team if not set? */
bool		teamImmunity;		/* Is team immune from player action */

bool		targetKillTeam;		/* if your target explodes, you die? */
bool		targetTeamCollision;	/* Does team collide with target? */
bool		targetSync;		/* all targets reappear together */
bool		treasureKillTeam;	/* die if treasure is destroyed? */
bool		treasureCollisionDestroys;
bool		treasureCollisionMayKill;
bool		wreckageCollisionMayKill;

DFLOAT		friction;		/* friction only affects ships */
DFLOAT		checkpointRadius;      	/* in blocks */
int		raceLaps;		/* how many laps per race */
bool		lockOtherTeam;		/* lock ply from other teams when dead? */
bool		loseItemDestroys; 	/* destroy or drop when player */
					/* uses loseItem */

int		maxOffensiveItems;	/* how many offensive and defensive */
int		maxDefensiveItems;	/* items can player carry */

int		roundDelay;		/* delay before start of each round */
int		maxRoundTime;		/* max. duration of each round */

bool		anaColDet;		/* use Analytical Collision Detection?  */

bool		pLockServer;		/* Is server swappable out of memory?  */

bool		ignore20MaxFPS;		/* ignore client maxFPS request if it is 20 */

extern char	conf_default_map_string[];	/* from common/config.c */
extern char	conf_robotfile_string[];	/* from common/config.c */

#define	MAP(_x)		_x
/* #define	MAP(_x)	 */

static void tuner_none(void)  {}
static void tuner_dummy(void) {}

static optionDesc options[] = {
    {
	"help",
	"help",
	"0",
	NULL,
	valVoid,
	tuner_none,
	"Print out this help message.\n",
	MAP(NULL)
    },
    {
	"version",
	"version",
	"0",
	NULL,
	valVoid,
	tuner_none,
	"Print version information.\n",
	MAP(NULL)
    },
    {
	"dump",
	"dump",
	"0",
	NULL,
	valVoid,
	tuner_none,
	"Print all options with their default values in defaultsfile format.\n",
	MAP(NULL)
    },
    {
	"gravity",
	"gravity",
	"-0.14",
	&Gravity,
	valReal,
	tuner_none,
	"Gravity strength.\n",
	MAP("World")
    },
    {
	"shipMass",
	"shipMass",
	"20.0",
	&ShipMass,
	valReal,
	tuner_none,
	"Mass of fighters.\n",
	MAP(NULL)
    },
    {
	"shotMass",
	"shotMass",
	"0.1",
	&ShotsMass,
	valReal,
	tuner_none,
	"Mass of bullets.\n",
	MAP(NULL)
    },
    {
	"shotSpeed",
	"shotSpeed",
	"21.0",
	&ShotsSpeed,
	valReal,
	tuner_none,
	"Maximum speed of bullets.\n",
	MAP(NULL)
    },
    {
	"shotLife",
	"shotLife",
	"60",
	&ShotsLife,
	valInt,
	tuner_none,
	"Life of bullets in ticks.\n",
	MAP(NULL)
    },
    {
	"fireRepeatRate",
	"fireRepeat",
	"2",
	&fireRepeatRate,
	valInt,
	tuner_dummy,
	"Number of frames per automatic fire (0=off).\n",
	MAP(NULL)
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
	MAP("Robots")
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
	MAP("Robots")
    },
    {
	"robotFile",
	"robotFile",
	conf_robotfile_string,
	&robotFile,
	valString,
	tuner_none,
	"The file containing parameters for robot details.\n",
	MAP("Robots")
    },
    {
	"robotsTalk",
	"robotsTalk",
	"false",
	&robotsTalk,
	valBool,
	tuner_dummy,
	"Do robots talk when they kill, die etc.?\n",
	MAP("Robots")
    },
    {
	"robotsLeave",
	"robotsLeave",
	"true",
	&robotsLeave,
	valBool,
	tuner_dummy,
	"Do robots leave the game?\n",
	MAP("Robots")
    },
    {
	"robotLeaveLife",
	"robotLeaveLife",
	"50",
	&robotLeaveLife,
	valInt,
	tuner_dummy,
	"Max life per robot (0=off).\n",
	MAP("Robots")
    },
    {
	"robotLeaveScore",
	"robotLeaveScore",
	"-90",
	&robotLeaveScore,
	valInt,
	tuner_dummy,
	"Min score for robot to play (0=off).\n",
	MAP("Robots")
    },
    {
	"robotLeaveRatio",
	"robotLeaveRatio",
	"-5",
	&robotLeaveRatio,
	valInt,
	tuner_dummy,
	"Min ratio for robot to play (0=off).\n",
	MAP("Robots")
    },
    {
	"robotTeam",
	"robotTeam",
	"0",
	&robotTeam,
	valInt,
	tuner_dummy,
	"Team to use for robots.\n",
	MAP("Robots")
    },
    {
	"restrictRobots",
	"restrictRobots",
	"true",
	&restrictRobots,
	valBool,
	tuner_dummy,
	"Are robots restricted to their own team?\n",
	MAP("Robots")
    },
    {
	"reserveRobotTeam",
	"reserveRobotTeam",
	"true",
	&reserveRobotTeam,
	valBool,
	tuner_dummy,
	"Is the robot team only for robots?\n",
	MAP("Robots")
    },
    {
	"maxPlayerShots",
	"shots",
	"256",
	&ShotsMax,
	valInt,
	tuner_none,
	"Maximum allowed bullets per player.\n",
	MAP(NULL)
    },
    {
	"shotsGravity",
	"shotsGravity",
	"true",
	&ShotsGravity,
	valBool,
	tuner_dummy,
	"Are bullets afflicted by gravity.\n",
	MAP(NULL)
    },
    {
	"idleRun",
	"rawMode",
	"false",
	&RawMode,
	valBool,
	tuner_dummy,
	"Do robots keep on playing even if all human players quit?\n",
	MAP(NULL)
    },
    {
	"noQuit",
	"noQuit",
#ifdef	_WINDOWS
	"true",
#else
	"false",
#endif
	&NoQuit,
	valBool,
	tuner_dummy,
	"Does the server wait for new human players to show up\n"
	"after all players have left.\n",
	MAP(NULL)
    },
    {
	"mapWidth",
	"mapWidth",
	"100",
	&mapWidth,
	valInt,
	tuner_none,
	"Width of the world in blocks.\n",
	MAP(NULL)
    },
    {
	"mapHeight",
	"mapHeight",
	"100",
	&mapHeight,
	valInt,
	tuner_none,
	"Height of the world in blocks.\n",
	MAP(NULL)
    },
    {
	"mapFileName",
	"map",
	conf_default_map_string,
	&mapFileName,
	valString,
	tuner_none,
	"The filename of the map to use.\n",
	MAP(NULL)
    },
    {
	"mapName",
	"mapName",
	"unknown",
	&mapName,
	valString,
	tuner_none,
	"The title of the map.\n",
	MAP(NULL)
    },
    {
	"mapAuthor",
	"mapAuthor",
	"anonymous",
	&mapAuthor,
	valString,
	tuner_none,
	"The name of the map author.\n",
	MAP(NULL)
    },
    {
	"contactPort",
	"port",
	"15345",
	&contactPort,
	valInt,
	tuner_none,
	"The server contact port number.\n",
	MAP("General")
    },
    {
	"mapData",
	"mapData",
	NULL,
	&mapData,
	valString,
	tuner_none,
	"The map's topology.\n",
	MAP(NULL)
    },
    {
	"allowPlayerCrashes",
	"allowPlayerCrashes",
	"yes",
	&crashWithPlayer,
	valBool,
	Set_world_rules,
	"Can players overrun other players?\n",
	MAP(NULL)
    },
    {
	"allowPlayerBounces",
	"allowPlayerBounces",
	"yes",
	&bounceWithPlayer,
	valBool,
	Set_world_rules,
	"Can players bounce with other players?\n",
	MAP(NULL)
    },
    {
	"allowPlayerKilling",
	"killings",
	"yes",
	&playerKillings,
	valBool,
	tuner_none,
	"Should players be allowed to kill one other?\n",
	MAP(NULL)
    },
    {
	"allowShields",
	"shields",
	"yes",
	&playerShielding,
	valBool,
	tuner_none,
	"Are shields allowed?\n",
	MAP(NULL)
    },
    {
	"playerStartsShielded",
	"playerStartShielded",
	"yes",
	&playerStartsShielded,
	valBool,
	tuner_none,
	"Do players start with shields up?\n",
	MAP(NULL)
    },
    {
	"shotsWallBounce",
	"shotsWallBounce",
	"no",
	&shotsWallBounce,
	valBool,
	Move_init,
	"Do shots bounce off walls?\n",
	MAP(NULL)
    },
    {
	"ballsWallBounce",
	"ballsWallBounce",
	"yes",
	&ballsWallBounce,
	valBool,
	Move_init,
	"Do balls bounce off walls?\n",
	MAP(NULL)
    },
    {
	"minesWallBounce",
	"minesWallBounce",
	"no",
	&minesWallBounce,
	valBool,
	Move_init,
	"Do mines bounce off walls?\n",
	MAP(NULL)
    },
    {
	"itemsWallBounce",
	"itemsWallBounce",
	"yes",
	&itemsWallBounce,
	valBool,
	Move_init,
	"Do items bounce off walls?\n",
	MAP(NULL)
    },
    {
	"missilesWallBounce",
	"missilesWallBounce",
	"no",
	&missilesWallBounce,
	valBool,
	Move_init,
	"Do missiles bounce off walls?\n",
	MAP(NULL)
    },
    {
	"sparksWallBounce",
	"sparksWallBounce",
	"no",
	&sparksWallBounce,
	valBool,
	Move_init,
	"Do thrust spark particles bounce off walls to give reactive thrust?\n",
	MAP(NULL)
    },
    {
	"debrisWallBounce",
	"debrisWallBounce",
	"no",
	&debrisWallBounce,
	valBool,
	Move_init,
	"Do explosion debris particles bounce off walls?\n",
	MAP(NULL)
    },
    {
	"cloakedExhaust",
	"cloakedExhaust",
	"yes",
	&cloakedExhaust,
	valBool,
	tuner_none,
	"Do engines of cloaked ships generate exhaust?\n",
	MAP(NULL)
    },
    {
	"cloakedShield",
	"cloakedShield",
	"yes",
	&cloakedShield,
	valBool,
	tuner_none,
	"Can players use shields when cloaked?\n",
	MAP(NULL)
    },
    {
	"maxObjectWallBounceSpeed",
	"maxObjectBounceSpeed",
	"40",
	&maxObjectWallBounceSpeed,
	valReal,
	Move_init,
	"The maximum allowed speed for objects to bounce off walls.\n",
	MAP(NULL)
    },
    {
	"maxShieldedWallBounceSpeed",
	"maxShieldedBounceSpeed",
	"50",
	&maxShieldedWallBounceSpeed,
	valReal,
	Move_init,
	"The maximum allowed speed for a shielded player to bounce off walls.\n",
	MAP(NULL)
    },
    {
	"maxUnshieldedWallBounceSpeed",
	"maxUnshieldedBounceSpeed",
	"20",
	&maxUnshieldedWallBounceSpeed,
	valReal,
	Move_init,
	"Maximum allowed speed for an unshielded player to bounce off walls.\n",
	MAP(NULL)
    },
    {
	"maxShieldedPlayerWallBounceAngle",
	"maxShieldedBounceAngle",
	"90",
	&maxShieldedWallBounceAngle,
	valReal,
	Move_init,
	"Maximum allowed angle for a shielded player to bounce off walls.\n",
	MAP(NULL)
    },
    {
	"maxUnshieldedPlayerWallBounceAngle",
	"maxUnshieldedBounceAngle",
	"30",
	&maxUnshieldedWallBounceAngle,
	valReal,
	Move_init,
	"Maximum allowed angle for an unshielded player to bounce off walls.\n",
	MAP(NULL)
    },
    {
	"playerWallBounceBrakeFactor",
	"playerWallBrake",
	"0.89",
	&playerWallBrakeFactor,
	valReal,
	Move_init,
	"Factor to slow down players when they hit the wall (between 0 and 1).\n",
	MAP(NULL)
    },
    {
	"objectWallBounceBrakeFactor",
	"objectWallBrake",
	"0.95",
	&objectWallBrakeFactor,
	valReal,
	Move_init,
	"Factor to slow down objects when they hit the wall (between 0 and 1).\n",
	MAP(NULL)
    },
    {
	"objectWallBounceLifeFactor",
	"objectWallBounceLifeFactor",
	"0.80",
	&objectWallBounceLifeFactor,
	valReal,
	Move_init,
	"Factor to reduce the life of objects after bouncing (between 0 and 1).\n",
	MAP(NULL)
    },
    {
	"wallBounceFuelDrainMult",
	"wallBounceDrain",
	"1.0",
	&wallBounceFuelDrainMult,
	valReal,
	Move_init,
	"Multiplication factor for player wall bounce fuel cost.\n",
	MAP(NULL)
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
	MAP(NULL)
    },
    {
	"reportToMetaServer",
	"reportMeta",
	"yes",
	&reportToMetaServer,
	valBool,
	tuner_none,
	"Keep the meta server informed about our game?\n",
	MAP(NULL)
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
	MAP(NULL)
    },
    {
	"limitedVisibility",
	"limitedVisibility",
	"no",
	&limitedVisibility,
	valBool,
	Set_world_rules,
	"Should the players have a limited visibility?\n",
	MAP(NULL)
    },
    {
	"minVisibilityDistance",
	"minVisibility",
	"0.0",
	&minVisibilityDistance,
	valReal,
	tuner_none,
	"Minimum block distance for limited visibility, 0 for default.\n",
	MAP(NULL)
    },
    {
	"maxVisibilityDistance",
	"maxVisibility",
	"0.0",
	&maxVisibilityDistance,
	valReal,
	tuner_none,
	"Maximum block distance for limited visibility, 0 for default.\n",
	MAP(NULL)
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
	MAP(NULL)
    },
    {
	"worldLives",
	"lives",
	"0",
	&worldLives,
	valInt,
	tuner_none,
	"Number of lives each player has (no sense without limitedLives).\n",
	MAP(NULL)
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
	MAP(NULL)
    },
    {
	"resetOnHuman",
	"humanReset",
	"no",
	&resetOnHuman,
	valInt,
	tuner_dummy,
	"Is the game restarted when there are less than resetOnHuman\n"
	"humans present and a new human logs in?\n",
	MAP(NULL)
    },
    {
	"teamPlay",
	"teams",
	"no",
	&teamPlay,
	valBool,
	tuner_none,
	"Is the map a team play map?\n",
	MAP(NULL)
    },
    {
	"teamFuel",
	"teamFuel",
	"no",
	&teamFuel,
	valBool,
	tuner_none,
	"Are fuelstations only available to team members?\n",
	MAP(NULL)
    },
    {
	"teamCannons",
	"teamCannons",
	"no",
	&teamCannons,
	valBool,
	tuner_none,
	"Do cannons choose sides in teamPlay?\n",
	MAP(NULL)
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
	MAP(NULL)
    },
    {
	"cannonsUseItems",
	"cannonsPickupItems",
	"no",
	&cannonsUseItems,
	valBool,
	tuner_none,
	"Do cannons use items?\n",
	MAP(NULL)
    },
    {
	"keepShots",
	"keepShots",
	"no",
	&keepShots,
	valBool,
	tuner_none,
	"Do shots, mines and missiles remain after their owner leaves?\n",
	MAP(NULL)
    },
    {
	"teamAssign",
	"teamAssign",
	"yes",
	&teamAssign,
	valBool,
	tuner_dummy,
	"If a player has not specified which team he likes to join\n"
	"should the server choose a team for him automatically?\n",
	MAP(NULL)
    },
    {
	"teamImmunity",
	"teamImmunity",
	"yes",
	&teamImmunity,
	valBool,
	tuner_dummy,
	"Should other team members be immune to various shots thrust etc.?\n",
	MAP(NULL)
    },
    {
	"ecmsReprogramMines",
	"ecmsReprogramMines",
	"yes",
	&ecmsReprogramMines,
	valBool,
	tuner_dummy,
	"Is it possible to reprogram mines with ECMs?\n",
	MAP(NULL)
    },
    {
	"ecmsReprogramRobots",
	"ecmsReprogramRobots",
	"yes",
	&ecmsReprogramRobots,
	valBool,
	tuner_dummy,
	"Are robots reprogrammed by ECMs instead of blinded?\n",
	MAP(NULL)
    },
    {
	"targetKillTeam",
	"targetKillTeam",
	"no",
	&targetKillTeam,
	valBool,
	tuner_dummy,
	"Do team members die when their last target explodes?\n",
	MAP(NULL)
    },
    {
	"targetTeamCollision",
	"targetCollision",
	"yes",
	&targetTeamCollision,
	valBool,
	tuner_dummy,
	"Do team members collide with their own target or not.\n",
	MAP(NULL)
    },
    {
	"targetSync",
	"targetSync",
	"no",
	&targetSync,
	valBool,
	tuner_dummy,
	"Do all the targets of a team reappear/repair at the same time?",
	MAP(NULL)
    },
    {
	"treasureKillTeam",
	"treasureKillTeam",
	"no",
	&treasureKillTeam,
	valBool,
	tuner_dummy,
	"Do team members die when their treasure is destroyed?\n",
	MAP(NULL)
    },
    {
	"treasureCollisionDestroys",
	"treasureCollisionDestroy",
	"yes",
	&treasureCollisionDestroys,
	valBool,
	tuner_dummy,
	"Are balls destroyed when a player touches it?\n",
	MAP(NULL)
    },
    {
	"treasureCollisionMayKill",
	"treasureUnshieldedCollisionKills",
	"no",
	&treasureCollisionMayKill,
	valBool,
	tuner_dummy,
	"Does a ball kill a player when the player touches it unshielded?\n",
	MAP(NULL)
    },
    {
	"wreckageCollisionMayKill",
	"wreckageUnshieldedCollisionKills",
	"no",
	&wreckageCollisionMayKill,
	valBool,
	tuner_dummy,
	"Can ships be destroyed when hit by wreckage?\n",
	MAP(NULL)
    },
    {   "ignore20MaxFPS",
	"ignore20MaxFPS",
	"true",
	&ignore20MaxFPS,
	valBool,
	tuner_dummy,
	"Ignore client maxFPS request if it is 20 (the default setting).\n",
	MAP(NULL)
    },
    {
	"onePlayerOnly",
	"onePlayerOnly",
	"no",
	&onePlayerOnly,
	valBool,
	tuner_none,
	"One player modus.\n",
	MAP(NULL)
    },
    {
	"timing",
	"race",
	"no",
	&timing,
	valBool,
	tuner_none,
	"Is the map a race mode map?\n",
	MAP(NULL)
    },
    {
	"edgeWrap",
	"edgeWrap",
	"no",
	&edgeWrap,
	valBool,
	tuner_none,
	"Wrap around edges.\n",
	MAP(NULL)
    },
    {
	"edgeBounce",
	"edgeBounce",
	"yes",
	&edgeBounce,
	valBool,
	tuner_dummy,
	"Players and bullets bounce when they hit the (non-wrapping) edge.\n",
	MAP(NULL)
    },
    {
	"extraBorder",
	"extraBorder",
	"no",
	&extraBorder,
	valBool,
	tuner_none,
	"Give map an extra border of solid rock.\n",
	MAP(NULL)
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
	MAP(NULL)
    },
    {
	"gravityAngle",
	"gravityAngle",
	"90",
	&gravityAngle,
	valReal,
	tuner_none,
	"If gravity is along a uniform line, at what angle is that line?\n",
	MAP(NULL)
    },
    {
	"gravityPointSource",
	"gravityPointSource",
	"false",
	&gravityPointSource,
	valBool,
	tuner_none,
	"Is gravity originating from a single point?\n",
	MAP(NULL)
    },
    {
	"gravityClockwise",
	"gravityClockwise",
	"false",
	&gravityClockwise,
	valBool,
	tuner_none,
	"If the gravity is a point source, is it clockwise?\n",
	MAP(NULL)
    },
    {
	"gravityAnticlockwise",
	"gravityAnticlockwise",
	"false",
	&gravityAnticlockwise,
	valBool,
	tuner_none,
	"If the gravity is a point source, is it anticlockwise?\n",
	MAP(NULL)
    },
    {
	"gravityVisible",
	"gravityVisible",
	"true",
	&gravityVisible,
	valBool,
	tuner_none,
	"Are gravity mapsymbols visible to players?\n",
	MAP(NULL)
    },
    {
	"wormholeVisible",
	"wormholeVisible",
	"true",
	&wormholeVisible,
	valBool,
	tuner_none,
	"Are wormhole mapsymbols visible to players?\n",
	MAP(NULL)
    },
    {
	"itemConcentratorVisible",
	"itemConcentratorVisible",
	"true",
	&itemConcentratorVisible,
	valBool,
	tuner_none,
	"Are itemconcentrator mapsymbols visible to players?\n",
	MAP(NULL)
    },
    {
	"wormTime",
	"wormTime",
	"0",
	&wormTime,
	valSec,
	tuner_none,
	"Number of seconds wormholes will remain stable.\n",
	MAP(NULL)
    },
    {
	"defaultsFileName",
	"defaults",
	"",
	&defaultsFileName,
	valString,
	tuner_none,
	"The filename of the defaults file to read on startup.\n",
	MAP(NULL)
    },
    {
	"scoreTableFileName",
	"scoretable",
	"",
	&scoreTableFileName,
	valString,
	tuner_none,
	"The filename for the score table to be dumped to.\n",
	MAP(NULL)
    },
    {
	"framesPerSecond",
	"FPS",
	"14",
	&framesPerSecond,
	valInt,
	tuner_none,
	"The number of frames per second the server should strive for.\n",
	MAP(NULL)
    },
    {
	"allowSmartMissiles",
	"allowSmarts",
	"True",
	&allowSmartMissiles,
	valBool,
	tuner_dummy,
	"Should smart missiles be allowed?\n",
	MAP(NULL)
    },
    {
	"allowHeatSeekers",
	"allowHeats",
	"True",
	&allowHeatSeekers,
	valBool,
	tuner_dummy,
	"Should heatseekers be allowed?\n",
	MAP(NULL)
    },
    {
	"allowTorpedoes",
	"allowTorps",
	"True",
	&allowTorpedoes,
	valBool,
	tuner_dummy,
	"Should torpedoes be allowed?\n",
	MAP(NULL)
    },
    {
	"allowNukes",
	"nukes",
	"False",
	&allowNukes,
	valBool,
	tuner_none,
	"Should nuclear weapons be allowed?\n",
	MAP(NULL)
    },
    {
	"allowClusters",
	"clusters",
	"False",
	&allowClusters,
	valBool,
	tuner_none,
	"Should cluster weapons be allowed?\n",
	MAP(NULL)
    },
    {
	"allowModifiers",
	"modifiers",
	"False",
	&allowModifiers,
	valBool,
	tuner_none,
	"Should the weapon modifiers be allowed?\n",
	MAP(NULL)
    },
    {
	"allowLaserModifiers",
	"lasermodifiers",
	"False",
	&allowLaserModifiers,
	valBool,
	tuner_none,
	"Can lasers be modified to be a different weapon?\n",
	MAP(NULL)
    },
    {
	"allowShipShapes",
	"ShipShapes",
	"True",
	&allowShipShapes,
	valBool,
	tuner_none,
	"Are players allowed to define their own ship shape?\n",
	MAP(NULL)
    },
    {
	"playersOnRadar",
	"playersRadar",
	"True",
	&playersOnRadar,
	valBool,
	tuner_dummy,
	"Are players visible on the radar.\n",
	MAP(NULL)
    },
    {
	"missilesOnRadar",
	"missilesRadar",
	"True",
	&missilesOnRadar,
	valBool,
	tuner_dummy,
	"Are missiles visible on the radar.\n",
	MAP(NULL)
    },
    {
	"minesOnRadar",
	"minesRadar",
	"False",
	&minesOnRadar,
	valBool,
	tuner_dummy,
	"Are mines visible on the radar.\n",
	MAP(NULL)
    },
    {
	"nukesOnRadar",
	"nukesRadar",
	"True",
	&nukesOnRadar,
	valBool,
	tuner_dummy,
	"Are nukes visible or highlighted on the radar.\n",
	MAP(NULL)
    },
    {
	"treasuresOnRadar",
	"treasuresRadar",
	"False",
	&treasuresOnRadar,
	valBool,
	tuner_dummy,
	"Are treasure balls visible or highlighted on the radar.\n",
	MAP(NULL)
    },
    {
	"distinguishMissiles",
	"distinguishMissiles",
	"True",
	&distinguishMissiles,
	valBool,
	tuner_dummy,
	"Are different types of missiles distinguished (by length).\n",
	MAP(NULL)
    },
    {
	"maxMissilesPerPack",
	"maxMissilesPerPack",
	"4",
	&maxMissilesPerPack,
	valInt,
	tuner_none,
	"The number of missiles gotten by picking up one missile item.\n",
	MAP(NULL)
    },
    {
	"maxMinesPerPack",
	"maxMinesPerPack",
	"2",
	&maxMinesPerPack,
	valInt,
	tuner_none,
	"The number of mines gotten by picking up one mine item.\n",
	MAP(NULL)
    },
    {
	"identifyMines",
	"identifyMines",
	"True",
	&identifyMines,
	valBool,
	tuner_dummy,
	"Are mine owner's names displayed.\n",
	MAP(NULL)
    },
    {
	"shieldedItemPickup",
	"shieldItem",
	"False",
	&shieldedItemPickup,
	valBool,
	tuner_dummy,
	"Can items be picked up while shields are up?\n",
	MAP(NULL)
    },
    {
	"shieldedMining",
	"shieldMine",
	"False",
	&shieldedMining,
	valBool,
	tuner_dummy,
	"Can mines be thrown and placed while shields are up?\n",
	MAP(NULL)
    },
    {
	"laserIsStunGun",
	"stunGun",
	"False",
	&laserIsStunGun,
	valBool,
	tuner_dummy,
	"Is the laser weapon a stun gun weapon?\n",
	MAP(NULL)
    },
    {
	"nukeMinSmarts",
	"nukeMinSmarts",
	"7",
	&nukeMinSmarts,
	valInt,
	tuner_dummy,
	"The minimum number of missiles needed to fire one nuclear missile.\n",
	MAP(NULL)
    },
    {
	"nukeMinMines",
	"nukeMinMines",
	"4",
	&nukeMinMines,
	valInt,
	tuner_dummy,
	"The minimum number of mines needed to make a nuclear mine.\n",
	MAP(NULL)
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
	MAP(NULL)
    },
    {
	"mineFuseTime",
	"mineFuseTime",
	"0.0",
	&mineFuseTime,
	valSec,
	tuner_dummy,
	"Time after which owned mines become deadly, zero means never.\n",
	MAP(NULL)
    },
    {
	"mineLife",
	"mineLife",
	"0",
	&mineLife,
	valInt,
	tuner_none,
	"Life of mines in ticks, zero means use default.\n",
	MAP(NULL)
    },
    {
	"missileLife",
	"missileLife",
	"0",
	&missileLife,
	valInt,
	tuner_none,
	"Life of missiles in ticks, zero means use default.\n",
	MAP(NULL)
    },
    {
	"baseMineRange",
	"baseMineRange",
	"0",
	&baseMineRange,
	valInt,
	tuner_dummy,
	"Range within which mines/bombs are not allowed.\n",
	MAP(NULL)
    },
    {
	"shotKillScoreMult",
	"shotKillScoreMult",
	"1.0",
	&shotKillScoreMult,
	valReal,
	tuner_none,
	"Multiplication factor to scale score for shot kills.\n",
	MAP(NULL)
    },
    {
        "torpedoKillScoreMult",
        "torpedoKillScoreMult",
        "1.0",
        &torpedoKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for torpedo kills.\n",
		MAP(NULL)
    },
    {
        "smartKillScoreMult",
        "smartKillScoreMult",
        "1.0",
        &smartKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for smart missile kills.\n",
		MAP(NULL)
    },
    {
        "heatKillScoreMult",
        "heatKillScoreMult",
        "1.0",
        &heatKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for heatseeker kills.\n",
		MAP(NULL)
    },
    {
        "clusterKillScoreMult",
        "clusterKillScoreMult",
        "1.0",
        &clusterKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for cluster debris kills.\n",
		MAP(NULL)
    },
    {
        "laserKillScoreMult",
        "laserKillScoreMult",
        "1.0",
        &laserKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for laser kills.\n",
		MAP(NULL)
    },
    {
        "tankKillScoreMult",
        "tankKillScoreMult",
        "0.44",
        &tankKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for tank kills.\n",
		MAP(NULL)
    },
    {
        "runoverKillScoreMult",
        "runoverKillScoreMult",
        "0.33",
        &runoverKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for player runovers.\n",
		MAP(NULL)
    },
    {
        "ballKillScoreMult",
        "ballKillScoreMult",
        "1.0",
        &ballKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for ball kills.\n",
		MAP(NULL)
    },
    {
        "explosionKillScoreMult",
        "explosionKillScoreMult",
        "0.33",
        &explosionKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for explosion kills.\n",
		MAP(NULL)
    },
    {
        "shoveKillScoreMult",
        "shoveKillScoreMult",
        "0.5",
        &shoveKillScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for shove kills.\n",
		MAP(NULL)
    },
    {
        "crashScoreMult",
        "crashScoreMult",
        "0.33",
        &crashScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for player crashes.\n",
		MAP(NULL)
    },
    {
        "mineScoreMult",
        "mineScoreMult",
        "0.17",
        &mineScoreMult,
        valReal,
        tuner_none,
        "Multiplication factor to scale score for mine hits.\n",
		MAP(NULL)
    },
    {
	"movingItemProb",
	"movingItemProb",
	"0.2",
	&movingItemProb,
	valReal,
	Set_misc_item_limits,
	"Probability for an item to appear as moving.\n",
	MAP(NULL)
    },
    {
	"dropItemOnKillProb",
	"dropItemOnKillProb",
	"0.5",
	&dropItemOnKillProb,
	valReal,
	Set_misc_item_limits,
	"Probability for dropping an item (each item) when you are killed.\n",
	MAP(NULL)
    },
    {
	"detonateItemOnKillProb",
	"detonateItemOnKillProb",
	"0.5",
	&detonateItemOnKillProb,
	valReal,
	Set_misc_item_limits,
	"Probability for undropped items to detonate when you are killed.\n",
	MAP(NULL)
    },
    {
	"destroyItemInCollisionProb",
	"destroyItemInCollisionProb",
	"0.0",
	&destroyItemInCollisionProb,
	valReal,
	Set_misc_item_limits,
	"Probability for items (some items) to be destroyed in a collision.\n",
	MAP(NULL)
    },
    {
	"itemProbMult",
	"itemProbFact",
	"1.0",
	&itemProbMult,
	valReal,
	Tune_item_probs,
	"Item Probability Multiplication Factor scales all item probabilities.\n",
	MAP(NULL)
    },
    {
	"cannonItemProbMult",
	"cannonItemProbMult",
	"1.0",
	&cannonItemProbMult,
	valReal,
	tuner_none,
	"Average number of items a cannon gets per minute.\n",
	MAP(NULL)
    },
    {
	"maxItemDensity",
	"maxItemDensity",
	"0.00012",
	&maxItemDensity,
	valReal,
	Tune_item_probs,
	"Maximum density [0.0-1.0] for items (max items per block).\n",
	MAP(NULL)
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
	MAP(NULL)
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
	MAP(NULL)
    },
    {
	"rogueHeatProb",
	"rogueHeatProb",
	"1.0",
	&rogueHeatProb,
	valReal,
	tuner_dummy,
	"Probability that unclaimed missile packs will go rogue.",
	MAP(NULL)
    },
    {
	"rogueMineProb",
	"rogueMineProb",
	"1.0",
	&rogueMineProb,
	valReal,
	tuner_dummy,
	"Probability that unclaimed mine items will activate.",
	MAP(NULL)
    },
    {
	"itemEnergyPackProb",
	"itemEnergyPackProb",
	"1e-9",
	&World.items[ITEM_FUEL].prob,
	valReal,
	Tune_item_probs,
	"Probability for an energy pack to appear.\n",
	MAP(NULL)
    },
    {
	"itemTankProb",
	"itemTankProb",
	"1e-9",
	&World.items[ITEM_TANK].prob,
	valReal,
	Tune_item_probs,
	"Probability for an extra tank to appear.\n",
	MAP(NULL)
    },
    {
	"itemECMProb",
	"itemECMProb",
	"1e-9",
	&World.items[ITEM_ECM].prob,
	valReal,
	Tune_item_probs,
	"Probability for an ECM item to appear.\n",
	MAP(NULL)
    },
    {
	"itemArmorProb",
	"itemArmorProb",
	"1e-9",
	&World.items[ITEM_ARMOR].prob,
	valReal,
	Tune_item_probs,
	"Probability for an armor item to appear.\n",
	MAP(NULL)
    },
    {
	"itemMineProb",
	"itemMineProb",
	"1e-9",
	&World.items[ITEM_MINE].prob,
	valReal,
	Tune_item_probs,
	"Probability for a mine item to appear.\n",
	MAP(NULL)
    },
    {
	"itemMissileProb",
	"itemMissileProb",
	"1e-9",
	&World.items[ITEM_MISSILE].prob,
	valReal,
	Tune_item_probs,
	"Probability for a missile item to appear.\n",
	MAP(NULL)
    },
    {
	"itemCloakProb",
	"itemCloakProb",
	"1e-9",
	&World.items[ITEM_CLOAK].prob,
	valReal,
	Tune_item_probs,
	"Probability for a cloak item to appear.\n",
	MAP(NULL)
    },
    {
	"itemSensorProb",
	"itemSensorProb",
	"1e-9",
	&World.items[ITEM_SENSOR].prob,
	valReal,
	Tune_item_probs,
	"Probability for a sensor item to appear.\n",
	MAP(NULL)
    },
    {
	"itemWideangleProb",
	"itemWideangleProb",
	"1e-9",
	&World.items[ITEM_WIDEANGLE].prob,
	valReal,
	Tune_item_probs,
	"Probability for a wideangle item to appear.\n",
	MAP(NULL)
    },
    {
	"itemRearshotProb",
	"itemRearshotProb",
	"1e-9",
	&World.items[ITEM_REARSHOT].prob,
	valReal,
	Tune_item_probs,
	"Probability for a rearshot item to appear.\n",
	MAP(NULL)
    },
    {
	"itemAfterburnerProb",
	"itemAfterburnerProb",
	"1e-9",
	&World.items[ITEM_AFTERBURNER].prob,
	valReal,
	Tune_item_probs,
	"Probability for an afterburner item to appear.\n",
	MAP(NULL)
    },
    {
	"itemTransporterProb",
	"itemTransporterProb",
	"1e-9",
	&World.items[ITEM_TRANSPORTER].prob,
	valReal,
	Tune_item_probs,
	"Probability for a transporter item to appear.\n",
	MAP(NULL)
    },
    {
	"itemMirrorProb",
	"itemMirrorProb",
	"1e-9",
	&World.items[ITEM_MIRROR].prob,
	valReal,
	Tune_item_probs,
	"Probability for a mirror item to appear.\n",
	MAP(NULL)
    },
    {
	"itemDeflectorProb",
	"itemDeflectorProb",
	"1e-9",
	&World.items[ITEM_DEFLECTOR].prob,
	valReal,
	Tune_item_probs,
	"Probability for a deflector item to appear.\n",
	MAP(NULL)
    },
    {
	"itemHyperJumpProb",
	"itemHyperJumpProb",
	"1e-9",
	&World.items[ITEM_HYPERJUMP].prob,
	valReal,
	Tune_item_probs,
	"Probability for a hyperjump item to appear.\n",
	MAP(NULL)
    },
    {
	"itemPhasingProb",
	"itemPhasingProb",
	"1e-9",
	&World.items[ITEM_PHASING].prob,
	valReal,
	Tune_item_probs,
	"Probability for a phasing item to appear.\n",
	MAP(NULL)
    },
    {
	"itemLaserProb",
	"itemLaserProb",
	"1e-9",
	&World.items[ITEM_LASER].prob,
	valReal,
	Tune_item_probs,
	"Probability for a Laser item to appear.\n",
	MAP(NULL)
    },
    {
	"itemEmergencyThrustProb",
	"itemEmergencyThrustProb",
	"1e-9",
	&World.items[ITEM_EMERGENCY_THRUST].prob,
	valReal,
	Tune_item_probs,
	"Probability for an Emergency Thrust item to appear.\n",
	MAP(NULL)
    },
    {
	"itemTractorBeamProb",
	"itemTractorBeamProb",
	"1e-9",
	&World.items[ITEM_TRACTOR_BEAM].prob,
	valReal,
	Tune_item_probs,
	"Probability for a Tractor Beam item to appear.\n",
	MAP(NULL)
    },
    {
	"itemAutopilotProb",
	"itemAutopilotProb",
	"1e-9",
	&World.items[ITEM_AUTOPILOT].prob,
	valReal,
	Tune_item_probs,
	"Probability for an Autopilot item to appear.\n",
	MAP(NULL)
    },
    {
	"itemEmergencyShieldProb",
	"itemEmergencyShieldProb",
	"1e-9",
	&World.items[ITEM_EMERGENCY_SHIELD].prob,
	valReal,
	Tune_item_probs,
	"Probability for an Emergency Shield item to appear.\n",
	MAP(NULL)
    },
    {
	"initialFuel",
	"initialFuel",
	"1000",
	&World.items[ITEM_FUEL].initial,
	valInt,
	Set_initial_resources,
	"How much fuel players start with, or the minimum after being killed.\n",
	MAP(NULL)
    },
    {
	"initialTanks",
	"initialTanks",
	"0",
	&World.items[ITEM_TANK].initial,
	valInt,
	Set_initial_resources,
	"How many tanks players start with.\n",
	MAP(NULL)
    },
    {
	"initialECMs",
	"initialECMs",
	"0",
	&World.items[ITEM_ECM].initial,
	valInt,
	Set_initial_resources,
	"How many ECMs players start with.\n",
	MAP(NULL)
    },
    {
	"initialArmor",
	"initialArmors",
	"0",
	&World.items[ITEM_ARMOR].initial,
	valInt,
	Set_initial_resources,
	"How much armor players start with.\n",
	MAP(NULL)
    },
    {
	"initialMines",
	"initialMines",
	"0",
	&World.items[ITEM_MINE].initial,
	valInt,
	Set_initial_resources,
	"How many mines players start with.\n",
	MAP(NULL)
    },
    {
	"initialMissiles",
	"initialMissiles",
	"0",
	&World.items[ITEM_MISSILE].initial,
	valInt,
	Set_initial_resources,
	"How many missiles players start with.\n",
	MAP(NULL)
    },
    {
	"initialCloaks",
	"initialCloaks",
	"0",
	&World.items[ITEM_CLOAK].initial,
	valInt,
	Set_initial_resources,
	"How many cloaks players start with.\n",
	MAP(NULL)
    },
    {
	"initialSensors",
	"initialSensors",
	"0",
	&World.items[ITEM_SENSOR].initial,
	valInt,
	Set_initial_resources,
	"How many sensors players start with.\n",
	MAP(NULL)
    },
    {
	"initialWideangles",
	"initialWideangles",
	"0",
	&World.items[ITEM_WIDEANGLE].initial,
	valInt,
	Set_initial_resources,
	"How many wideangles players start with.\n",
	MAP(NULL)
    },
    {
	"initialRearshots",
	"initialRearshots",
	"0",
	&World.items[ITEM_REARSHOT].initial,
	valInt,
	Set_initial_resources,
	"How many rearshots players start with.\n",
	MAP(NULL)
    },
    {
	"initialAfterburners",
	"initialAfterburners",
	"0",
	&World.items[ITEM_AFTERBURNER].initial,
	valInt,
	Set_initial_resources,
	"How many afterburners players start with.\n",
	MAP(NULL)
    },
    {
	"initialTransporters",
	"initialTransporters",
	"0",
	&World.items[ITEM_TRANSPORTER].initial,
	valInt,
	Set_initial_resources,
	"How many transporters players start with.\n",
	MAP(NULL)
    },
    {
	"initialMirrors",
	"initialMirrors",
	"0",
	&World.items[ITEM_MIRROR].initial,
	valInt,
	Set_initial_resources,
	"How many mirrors players start with.\n",
	MAP(NULL)
    },
    {
	"maxArmor",
	"maxArmors",
	"10",
	&World.items[ITEM_ARMOR].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the amount of armor per player.\n",
	MAP(NULL)
    },
    {
	"initialDeflectors",
	"initialDeflectors",
	"0",
	&World.items[ITEM_DEFLECTOR].initial,
	valInt,
	Set_initial_resources,
	"How many deflectors players start with.\n",
	MAP(NULL)
    },
    {
	"initialHyperJumps",
	"initialHyperJumps",
	"0",
	&World.items[ITEM_HYPERJUMP].initial,
	valInt,
	Set_initial_resources,
	"How many hyperjumps players start with.\n",
	MAP(NULL)
    },
    {
	"initialPhasings",
	"initialPhasings",
	"0",
	&World.items[ITEM_PHASING].initial,
	valInt,
	Set_initial_resources,
	"How many phasing devices players start with.\n",
	MAP(NULL)
    },
    {
	"initialLasers",
	"initialLasers",
	"0",
	&World.items[ITEM_LASER].initial,
	valInt,
	Set_initial_resources,
	"How many lasers players start with.\n",
	MAP(NULL)
    },
    {
	"initialEmergencyThrusts",
	"initialEmergencyThrusts",
	"0",
	&World.items[ITEM_EMERGENCY_THRUST].initial,
	valInt,
	Set_initial_resources,
	"How many emergency thrusts players start with.\n",
	MAP(NULL)
    },
    {
	"initialTractorBeams",
	"initialTractorBeams",
	"0",
	&World.items[ITEM_TRACTOR_BEAM].initial,
	valInt,
	Set_initial_resources,
	"How many tractor/pressor beams players start with.\n",
	MAP(NULL)
    },
    {
	"initialAutopilots",
	"initialAutopilots",
	"0",
	&World.items[ITEM_AUTOPILOT].initial,
	valInt,
	Set_initial_resources,
	"How many autopilots players start with.\n",
	MAP(NULL)
    },
    {
	"initialEmergencyShields",
	"initialEmergencyShields",
	"0",
	&World.items[ITEM_EMERGENCY_SHIELD].initial,
	valInt,
	Set_initial_resources,
	"How many emergency shields players start with.\n",
	MAP(NULL)
    },
    {
	"maxFuel",
	"maxFuel",
	"10000",
	&World.items[ITEM_FUEL].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the amount of fuel per player.\n",
	MAP(NULL)
    },
    {
	"maxTanks",
	"maxTanks",
	"8",
	&World.items[ITEM_TANK].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of tanks per player.\n",
	MAP(NULL)
    },
    {
	"maxECMs",
	"maxECMs",
	"10",
	&World.items[ITEM_ECM].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of ECMs per player.\n",
	MAP(NULL)
    },
    {
	"maxMines",
	"maxMines",
	"10",
	&World.items[ITEM_MINE].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of mines per player.\n",
	MAP(NULL)
    },
    {
	"maxMissiles",
	"maxMissiles",
	"10",
	&World.items[ITEM_MISSILE].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of missiles per player.\n",
	MAP(NULL)
    },
    {
	"maxCloaks",
	"maxCloaks",
	"10",
	&World.items[ITEM_CLOAK].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of cloaks per player.\n",
	MAP(NULL)
    },
    {
	"maxSensors",
	"maxSensors",
	"10",
	&World.items[ITEM_SENSOR].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of sensors per player.\n",
	MAP(NULL)
    },
    {
	"maxWideangles",
	"maxWideangles",
	"10",
	&World.items[ITEM_WIDEANGLE].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of wides per player.\n",
	MAP(NULL)
    },
    {
	"maxRearshots",
	"maxRearshots",
	"10",
	&World.items[ITEM_REARSHOT].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of rearshots per player.\n",
	MAP(NULL)
    },
    {
	"maxAfterburners",
	"maxAfterburners",
	"10",
	&World.items[ITEM_AFTERBURNER].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of afterburners per player.\n",
	MAP(NULL)
    },
    {
	"maxTransporters",
	"maxTransporters",
	"10",
	&World.items[ITEM_TRANSPORTER].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of transporters per player.\n",
	MAP(NULL)
    },
    {
	"maxMirrors",
	"maxMirrors",
	"10",
	&World.items[ITEM_MIRROR].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of mirrors per player.\n",
	MAP(NULL)
    },
    {
	"maxDeflectors",
	"maxDeflectors",
	"10",
	&World.items[ITEM_DEFLECTOR].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of deflectors per player.\n",
	MAP(NULL)
    },
    {
	"maxPhasings",
	"maxPhasings",
	"10",
	&World.items[ITEM_PHASING].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of phasing devices per players.\n",
	MAP(NULL)
    },
    {
	"maxHyperJumps",
	"maxHyperJumps",
	"10",
	&World.items[ITEM_HYPERJUMP].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of hyperjumps per player.\n",
	MAP(NULL)
    },
    {
	"maxEmergencyThrusts",
	"maxEmergencyThrusts",
	"10",
	&World.items[ITEM_EMERGENCY_THRUST].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of emergency thrusts per player.\n",
	MAP(NULL)
    },
    {
	"maxLasers",
	"maxLasers",
	"5",
	&World.items[ITEM_LASER].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of lasers per player.\n",
	MAP(NULL)
    },
    {
	"maxTractorBeams",
	"maxTractorBeams",
	"4",
	&World.items[ITEM_TRACTOR_BEAM].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of tractorbeams per player.\n",
	MAP(NULL)
    },
    {
	"maxAutopilots",
	"maxAutopilots",
	"10",
	&World.items[ITEM_AUTOPILOT].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of autopilots per player.\n",
	MAP(NULL)
    },
    {
	"maxEmergencyShields",
	"maxEmergencyShields",
	"10",
	&World.items[ITEM_EMERGENCY_SHIELD].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of emergency shields per player.\n",
	MAP(NULL)
    },
    {
	"gameDuration",
	"time",
	"0.0",
	&gameDuration,
	valReal,
	tuner_none,
	"The duration of the game in minutes (aka. pizza mode).\n",
	MAP(NULL)
    },
    {
	"allowViewing",
	"allowViewing",
	"false",
	&allowViewing,
	valBool,
	tuner_dummy,
	"Are players allowed to watch any other player while paused, waiting or dead?\n",
	MAP(NULL)
    },
    {
	"friction",
	"friction",
	"0.0",
	&friction,
	valReal,
	tuner_dummy,
	"Fraction of velocity ship loses each frame.\n",
	MAP(NULL)
    },
    {
	"checkpointRadius",
	"checkpointRadius",
	"6.0",
	&checkpointRadius,
	valReal,
	tuner_dummy,
	"How close you have to be to a checkpoint to register - in blocks.\n",
	MAP(NULL)
    },
    {
	"raceLaps",
	"raceLaps",
	"3",
	&raceLaps,
	valInt,
	tuner_none,
	"How many laps a race is run over.\n",
	MAP(NULL)
    },
    {
	"lockOtherTeam",
	"lockOtherTeam",
	"true",
	&lockOtherTeam,
	valBool,
	tuner_none,
	"Can you lock on players from other teams when you're dead.\n",
	MAP(NULL)
    },
    {
	"loseItemDestroys",
	"loseItemDestroys",
	"false",
	&loseItemDestroys,
	valBool,
	tuner_none,
	"Destroy item that player drops. Otherwise drop it.\n",
	MAP(NULL)
    },
    {
	"maxOffensiveItems",
	"maxOffensiveItems",
	"100",
	&maxOffensiveItems,
	valInt,
	tuner_dummy,
	"How many offensive items a player can carry.\n",
	MAP(NULL)
    },
    {
	"maxDefensiveItems",
	"maxDefensiveItems",
	"100",
	&maxDefensiveItems,
	valInt,
	tuner_dummy,
	"How many defensive items a player can carry.\n",
	MAP(NULL)
    },
    {
	"roundDelay",
	"roundDelay",
	"0",
	&roundDelay,
	valInt,
	tuner_dummy,
	"Delay before start of each round, in seconds.\n",
	MAP(NULL)
    },
    {
	"maxRoundTime",
	"maxRoundTime",
	"0",
	&maxRoundTime,
	valInt,
	tuner_dummy,
	"The maximum duration of each round, in seconds.\n",
	MAP(NULL)
    },
    {
	"analyticalCollisionDetection",
	"ACD",
	"true",
	&anaColDet,
	valBool,
	tuner_dummy,
	"Use Rakk's Analytical Collision Detection.\n",
	MAP(NULL)
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
	MAP("General")
    },

};


static void Parse_help(char *progname)
{
    int			j;
    const char		*str;

    xpprintf("Usage: %s [ options ]\n"
	   "Where options include:\n"
	   "\n",
	   progname);
    for (j = 0; j < NELEM(options); j++) {
	xpprintf("    %s%s",
	       options[j].type == valBool ? "-/+" : "-",
	       options[j].name);
	if (strcasecmp(options[j].commandLineOption, options[j].name))
	    xpprintf(" or %s", options[j].commandLineOption);
	xpprintf(" %s\n",
	       options[j].type == valInt ? "<integer>" :
	       options[j].type == valReal ? "<real>" :
	       options[j].type == valString ? "<string>" :
	       options[j].type == valIPos ? "<position>" :
	       options[j].type == valSec ? "<seconds>" :
	       options[j].type == valPerSec ? "<per-second>" :
	       "");
	for (str = options[j].helpLine; *str; str++) {
	    if (str == options[j].helpLine || str[-1] == '\n') {
		putchar('\t');
	    }
	    putchar(*str);
	}
	if (str > options[j].helpLine && str[-1] != '\n') {
	    putchar('\n');
	}
	putchar('\n');
    }
    xpprintf(
"    \n"
"    The probabilities are in the range [0.0-1.0] and they refer to the\n"
"    probability that an event will occur in a block per second.\n"
"    Boolean options are turned off by using +<option>.\n"
"    \n"
"    Please refer to the manual pages, xpilots(6) and xpilot(6),\n"
"    for more specific help.\n"
	  );
}

static void Parse_dump_mapedit(char *progname)
{
    int			j;

    xpprintf("# mapeditor value table\n");
    xpprintf("# Generated for xpilot version %s\n", TITLE);
    for (j = 0; j < NELEM(options); j++) {
	if (options[j].type != valVoid) {
	    if (options[j].mapperPos) {
		xpprintf("$$\n");
		xpprintf("%s\n%s\n%s\n%d\n%s\n",
			options[j].name,
			options[j].mapperPos,
			options[j].commandLineOption,
			options[j].type,
			options[j].defaultValue
			);
	    }
	}
    }
    xpprintf("\n");
}

static void Parse_dump_man(char *progname)
{
    int			j;

    for (j = 0; j < NELEM(options); j++) {
	if (options[j].type != valVoid) {
	    int len = strlen(options[j].name);
	    xpprintf("%s:%*s%s\n", options[j].name,
		   (len < 40) ? (40 - len) : 1, "",
		   (options[j].defaultValue != NULL)
		       ? options[j].defaultValue
		       : "");
	}
    }
    xpprintf("\n");
}

static void Parse_dump_windows(char *progname)
{
    int			j;

    for (j = 0; j < NELEM(options); j++) {
	if (options[j].type != valVoid) {
	    int len = strlen(options[j].name);
	    xpprintf("%s:%*s%s\n", options[j].name,
		   (len < 40) ? (40 - len) : 1, "",
		   (options[j].defaultValue != NULL)
		       ? options[j].defaultValue
		       : "");
	}
    }
    xpprintf("\n");
}

static void Parse_dump(char *progname)
{
    int			j;

    xpprintf("\n");
    xpprintf("# %s option dump\n", progname);
    xpprintf("# \n");
    xpprintf("# LIBDIR = %s\n", Conf_libdir());
    xpprintf("# DEFAULTS_FILE_NAME = %s\n", Conf_defaults_file_name());
    xpprintf("# MAPDIR = %s\n", Conf_mapdir());
    xpprintf("# DEFAULT_MAP = %s\n", Conf_default_map());
    xpprintf("# SERVERMOTDFILE = %s\n", Conf_servermotdfile());
    xpprintf("# \n");
    for (j = 0; j < NELEM(options); j++) {
	if (options[j].type != valVoid) {
	    int len = strlen(options[j].name);
	    xpprintf("%s:%*s%s\n", options[j].name,
		   (len < 40) ? (40 - len) : 1, "",
		   (options[j].defaultValue != NULL)
		       ? options[j].defaultValue
		       : "");
	}
    }
    xpprintf("\n");
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
		*(DFLOAT *)options[i].variable);
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

int Parser(int argc, char **argv)
{
    int			i, j;
    char		*fname;


    for (i=1; i<argc; i++) {
	if (strcmp("-help", argv[i]) == 0
	    || strcmp("-h", argv[i]) == 0) {
	    Parse_help(*argv);
		return(FALSE);
	}
	if (strcmp("-dump", argv[i]) == 0) {
	    Parse_dump(*argv);
		return(FALSE);
	}
	if (strcmp("-dumpMapedit", argv[i]) == 0) {
	    Parse_dump_mapedit(*argv);
		return(FALSE);
	}
	if (strcmp("-dumpMan", argv[i]) == 0) {
	    Parse_dump_man(*argv);
		return(FALSE);
	}
	if (strcmp("-dumpWindows", argv[i]) == 0) {
	    Parse_dump_windows(*argv);
		return(FALSE);
	}
	if (strcmp("-version", argv[i]) == 0 || strcmp("-v", argv[i]) == 0) {
	    puts(TITLE);
		return(FALSE);
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
		    } else if (options[j].type == valVoid) {
				break;
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
     * Read map file if map data not found yet.
     */
    if (!(fname = getOption("mapData"))) {
	if ((fname = getOption("mapFileName")) != NULL) {
	    if (!parseMapFile(fname)) {
		xpprintf("Unable to read %s, trying to open %s\n", fname, Conf_default_map());
		if (!parseMapFile(Conf_default_map()))
		    xpprintf("Unable to read %s\n", Conf_default_map());
	    }
	} else {
	    xpprintf("Map not specified, trying to open %s\n", Conf_default_map());
	    if (!parseMapFile(Conf_default_map()))
		xpprintf("Unable to read %s\n", Conf_default_map());
	}
    }

    /*
     * Read local defaults file
     */
    if ((fname = getOption("defaultsFileName")) != NULL)
	parseDefaultsFile(fname);
    else
	parseDefaultsFile(Conf_defaults_file_name());

    for (j = 0; j < NELEM(options); j++)
	addOption(options[j].name, options[j].defaultValue, 0, &options[j]);
    parseOptions();
    Grok_map();
    return(TRUE);
}


/*
 * Some options can be modified during the game.
 * Options which can be modified have a so called tuner function,
 * which checks the validity of the new option value, and possibly
 * does something extra depending upon the option in question.
 * Options which don't need such a tuner function set it to `tuner_dummy'.
 * Options which cannot be modified have the tuner set to `tuner_none'.
 */
int Tune_option(char *opt, char *val)
{
    int			ival;
    DFLOAT		fval;
    int			j;

    for (j = 0; j < NELEM(options); j++) {
	if (strcasecmp(options[j].commandLineOption, opt)
	    && strcasecmp(options[j].name, opt)) {
	    continue;
	}
	if (options[j].tuner == tuner_none) {
	    return -1;	/* Operation undefined */
	}
	switch (options[j].type) {
	case valInt:
	    if (sscanf(val, "%d", &ival) != 1) {
		return 0;
	    }
	    *(int *)options[j].variable = ival;
	    (*options[j].tuner)();
	    return 1;
	case valBool:
	    if (ON(val)) {
		*(bool *)options[j].variable = true;
	    }
	    else if (OFF(val)) {
		*(bool *)options[j].variable = false;
	    }
	    else {
		return 0;
	    }
	    (*options[j].tuner)();
	    return 1;
	case valReal:
	    if (sscanf(val, "%f", &fval) != 1) {
		return 0;
	    }
	    *(DFLOAT *)options[j].variable = fval;
	    (*options[j].tuner)();
	    return 1;
	case valString:
	    /* this is dubious and results in unfreed memory: */
	    {
		char *s = strdup(val);
		if (!s) {
		    return 0;
		}
		*(char **)options[j].variable = s;
		(*options[j].tuner)();
		return 1;
	    }
	default:
	    return -1;	/* Operation undefined */
	}
    }
    return -2;	/* Can't find variable */
}

optionDesc*	findOption(const char* name)
{
	int	j;
    for (j = 0; j < NELEM(options); j++) 
	{
		if (!strcasecmp(options[j].commandLineOption, name)
			|| !strcasecmp(options[j].name, name))
		{
			return(&options[j]);
		}
	}
	return(NULL);
}


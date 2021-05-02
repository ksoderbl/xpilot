/* $Id: cmdline.c,v 3.99 1998/01/23 13:02:52 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-97 by
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
#include "../contrib/NT/xpilots/winServer.h"
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

char cmdline_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: cmdline.c,v 3.99 1998/01/23 13:02:52 bert Exp $";
#endif

DFLOAT		Gravity;		/* Power of gravity */
DFLOAT		ShipMass;		/* Default mass of ship */
DFLOAT		ShotsMass;		/* Default mass of shots */
DFLOAT		ShotsSpeed;		/* Default speed of shots */
int		ShotsLife;		/* Default number of ticks */
					/* each shot will live */
int		WantedNumRobots;	/* How many robots should enter */
					/* the game? */
char		*robotFile;		/* Filename for robot parameters */
int		robotsTalk;		/* Do robots talk? */
int		robotsLeave;		/* Do robots leave at all? */
int		robotLeaveLife;		/* Max life per robot (0=off)*/
int		robotLeaveScore;	/* Min score for robot to live (0=off)*/
int		robotLeaveRatio;	/* Min ratio for robot to live (0=off)*/
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
bool		teamPlay;		/* Are teams allowed? */
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

DFLOAT 		movingItemProb;		/* Probability for moving items */
DFLOAT		dropItemOnKillProb;	/* Probability for players items to */
					/* drop when player is killed */
DFLOAT		detonateItemOnKillProb;	/* Probaility for remaining items to */
					/* detonate when player is killed */
DFLOAT		destroyItemInCollisionProb;
DFLOAT           rogueHeatProb;          /* prob. that unclaimed rocketpack */
DFLOAT           rogueMineProb;          /* or minepack will "activate" */
DFLOAT		itemProbMult;
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

bool		teamAssign;		/* Assign player to team if not set? */
bool		teamImmunity;		/* Is team immune from player action */

bool		targetKillTeam;		/* if your target explodes, you die? */
bool		targetTeamCollision;	/* Does team collide with target? */
bool		targetSync;		/* all targets reappear together */
bool		treasureKillTeam;	/* die if treasure is destroyed? */
bool		treasureCollisionDestroys;
bool		treasureCollisionMayKill;

DFLOAT		friction;		/* friction only affects ships */
DFLOAT		checkpointRadius;      	/* in blocks */
int		raceLaps;		/* how many laps per race */
bool		lockOtherTeam;		/* lock ply from other teams when dead? */
bool		loseItemDestroys; 	/* destroy or drop when player */
					/* uses loseItem */

int		maxOffensiveItems;	/* how many offensive and defensive */
int		maxDefensiveItems;	/* items can player carry */

bool		pLockServer;		/* Is server swappable out of memory?  */


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
	"Print out this help message.\n"
    },
    {
	"version",
	"version",
	"0",
	NULL,
	valVoid,
	tuner_none,
	"Print version information.\n"
    },
    {
	"dump",
	"dump",
	"0",
	NULL,
	valVoid,
	tuner_none,
	"Print all options with their default values in defaultsfile format.\n"
    },
    {
	"gravity",
	"gravity",
	"-0.14",
	&Gravity,
	valReal,
	tuner_none,
	"Gravity strength.\n"
    },
    {
	"shipMass",
	"shipMass",
	"20.0",
	&ShipMass,
	valReal,
	tuner_none,
	"Mass of fighters.\n"
    },
    {
	"shotMass",
	"shotMass",
	"0.1",
	&ShotsMass,
	valReal,
	tuner_none,
	"Mass of bullets.\n"
    },
    {
	"shotSpeed",
	"shotSpeed",
	"21.0",
	&ShotsSpeed,
	valReal,
	tuner_none,
	"Maximum speed of bullets.\n"
    },
    {
	"shotLife",
	"shotLife",
	"60",
	&ShotsLife,
	valInt,
	tuner_none,
	"Life of bullets in ticks.\n"
    },
    {
	"fireRepeatRate",
	"fireRepeat",
	"2",
	&fireRepeatRate,
	valInt,
	tuner_dummy,
	"Number of frames per automatic fire (0=off).\n"
    },
    {
	"maxRobots",
	"robots",
	"4",
	&WantedNumRobots,
	valInt,
	tuner_none,
	"How many robots do you want?\n"
    },
    {
	"robotFile",
	"robotFile",
	ROBOTFILE,
	&robotFile,
	valString,
	tuner_none,
	"The file containing parameters for robot details.\n"
    },
    {
	"robotsTalk",
	"robotsTalk",
	"false",
	&robotsTalk,
	valBool,
	tuner_dummy,
	"Do robots talk when they kill, die etc.?\n"
    },
    {
	"robotsLeave",
	"robotsLeave",
	"true",
	&robotsLeave,
	valBool,
	tuner_dummy,
	"Do robots leave the game?\n"
    },
    {
	"robotLeaveLife",
	"robotLeaveLife",
	"50",
	&robotLeaveLife,
	valInt,
	tuner_dummy,
	"Max life per robot (0=off).\n"
    },
    {
	"robotLeaveScore",
	"robotLeaveScore",
	"-90",
	&robotLeaveScore,
	valInt,
	tuner_dummy,
	"Min score for robot to play (0=off).\n"
    },
    {
	"robotLeaveRatio",
	"robotLeaveRatio",
	"-5",
	&robotLeaveRatio,
	valInt,
	tuner_dummy,
	"Min ratio for robot to play (0=off).\n"
    },
    {
	"maxPlayerShots",
	"shots",
	"256",
	&ShotsMax,
	valInt,
	tuner_none,
	"Maximum allowed bullets per player.\n"
    },
    {
	"shotsGravity",
	"shotsGravity",
	"true",
	&ShotsGravity,
	valBool,
	tuner_dummy,
	"Are bullets afflicted by gravity.\n"
    },
    {
	"idleRun",
	"rawMode",
	"false",
	&RawMode,
	valBool,
	tuner_dummy,
	"Do robots keep on playing even if all human players quit?\n"
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
	"after all players have left.\n"
    },
    {
	"mapWidth",
	"mapWidth",
	"100",
	&mapWidth,
	valInt,
	tuner_none,
	"Width of the world in blocks.\n"
    },
    {
	"mapHeight",
	"mapHeight",
	"100",
	&mapHeight,
	valInt,
	tuner_none,
	"Height of the world in blocks.\n"
    },
    {
	"mapFileName",
	"map",
	DEFAULT_MAP,
	&mapFileName,
	valString,
	tuner_none,
	"The filename of the map to use.\n"
    },
    {
	"mapName",
	"mapName",
	"unknown",
	&mapName,
	valString,
	tuner_none,
	"The title of the map.\n"
    },
    {
	"mapAuthor",
	"mapAuthor",
	"anonymous",
	&mapAuthor,
	valString,
	tuner_none,
	"The name of the map author.\n"
    },
    {
	"contactPort",
	"port",
	"15345",
	&contactPort,
	valInt,
	tuner_none,
	"The server contact port number.\n"
    },
    {
	"mapData",
	"mapData",
	NULL,
	&mapData,
	valString,
	tuner_none,
	"The map's topology.\n"
    },
    {
	"allowPlayerCrashes",
	"allowPlayerCrashes",
	"yes",
	&crashWithPlayer,
	valBool,
	Set_world_rules,
	"Can players overrun other players?\n"
    },
    {
	"allowPlayerBounces",
	"allowPlayerBounces",
	"yes",
	&bounceWithPlayer,
	valBool,
	Set_world_rules,
	"Can players bounce with other players?\n"
    },
    {
	"allowPlayerKilling",
	"killings",
	"yes",
	&playerKillings,
	valBool,
	tuner_none,
	"Should players be allowed to kill one other?\n"
    },
    {
	"allowShields",
	"shields",
	"yes",
	&playerShielding,
	valBool,
	tuner_none,
	"Are shields allowed?\n"
    },
    {
	"playerStartsShielded",
	"playerStartShielded",
	"yes",
	&playerStartsShielded,
	valBool,
	tuner_none,
	"Do players start with shields up?\n"
    },
    {
	"shotsWallBounce",
	"shotsWallBounce",
	"no",
	&shotsWallBounce,
	valBool,
	Move_init,
	"Do shots bounce off walls?\n"
    },
    {
	"ballsWallBounce",
	"ballsWallBounce",
	"yes",
	&ballsWallBounce,
	valBool,
	Move_init,
	"Do balls bounce off walls?\n"
    },
    {
	"minesWallBounce",
	"minesWallBounce",
	"no",
	&minesWallBounce,
	valBool,
	Move_init,
	"Do mines bounce off walls?\n"
    },
    {
	"itemsWallBounce",
	"itemsWallBounce",
	"yes",
	&itemsWallBounce,
	valBool,
	Move_init,
	"Do items bounce off walls?\n"
    },
    {
	"missilesWallBounce",
	"missilesWallBounce",
	"no",
	&missilesWallBounce,
	valBool,
	Move_init,
	"Do missiles bounce off walls?\n"
    },
    {
	"sparksWallBounce",
	"sparksWallBounce",
	"no",
	&sparksWallBounce,
	valBool,
	Move_init,
	"Do thrust spark particles bounce off walls to give reactive thrust?\n"
    },
    {
	"debrisWallBounce",
	"debrisWallBounce",
	"no",
	&debrisWallBounce,
	valBool,
	Move_init,
	"Do explosion debris particles bounce off walls?\n"
    },
    {
	"cloakedExhaust",
	"cloakedExhaust",
	"yes",
	&cloakedExhaust,
	valBool,
	tuner_none,
	"Do engines of cloaked ships generate exhaust?\n"
    },
    {
	"cloakedShield",
	"cloakedShield",
	"yes",
	&cloakedShield,
	valBool,
	tuner_none,
	"Can players use shields when cloaked?\n"
    },
    {
	"maxObjectWallBounceSpeed",
	"maxObjectBounceSpeed",
	"40",
	&maxObjectWallBounceSpeed,
	valReal,
	Move_init,
	"The maximum allowed speed for objects to bounce off walls.\n"
    },
    {
	"maxShieldedWallBounceSpeed",
	"maxShieldedBounceSpeed",
	"50",
	&maxShieldedWallBounceSpeed,
	valReal,
	Move_init,
	"The maximum allowed speed for a shielded player to bounce off walls.\n"
    },
    {
	"maxUnshieldedWallBounceSpeed",
	"maxUnshieldedBounceSpeed",
	"20",
	&maxUnshieldedWallBounceSpeed,
	valReal,
	Move_init,
	"Maximum allowed speed for an unshielded player to bounce off walls.\n"
    },
    {
	"maxShieldedPlayerWallBounceAngle",
	"maxShieldedBounceAngle",
	"90",
	&maxShieldedWallBounceAngle,
	valReal,
	Move_init,
	"Maximum allowed angle for a shielded player to bounce off walls.\n"
    },
    {
	"maxUnshieldedPlayerWallBounceAngle",
	"maxUnshieldedBounceAngle",
	"30",
	&maxUnshieldedWallBounceAngle,
	valReal,
	Move_init,
	"Maximum allowed angle for an unshielded player to bounce off walls.\n"
    },
    {
	"playerWallBounceBrakeFactor",
	"playerWallBrake",
	"0.89",
	&playerWallBrakeFactor,
	valReal,
	Move_init,
	"Factor to slow down players when they hit the wall (between 0 and 1).\n"
    },
    {
	"objectWallBounceBrakeFactor",
	"objectWallBrake",
	"0.95",
	&objectWallBrakeFactor,
	valReal,
	Move_init,
	"Factor to slow down objects when they hit the wall (between 0 and 1).\n"
    },
    {
	"objectWallBounceLifeFactor",
	"objectWallBounceLifeFactor",
	"0.80",
	&objectWallBounceLifeFactor,
	valReal,
	Move_init,
	"Factor to reduce the life of objects after bouncing (between 0 and 1).\n"
    },
    {
	"wallBounceFuelDrainMult",
	"wallBounceDrain",
	"1.0",
	&wallBounceFuelDrainMult,
	valReal,
	Move_init,
	"Multiplication factor for player wall bounce fuel cost.\n"
    },
    {
	"wallBounceDestroyItemProb",
	"wallBounceDestroyItemProb",
	"0.0",
	&wallBounceDestroyItemProb,
	valReal,
	Move_init,
	"The probability for each item a player owns to get destroyed\n"
	"when the player bounces against a wall.\n"
    },
    {
	"reportToMetaServer",
	"reportMeta",
	"yes",
	&reportToMetaServer,
	valBool,
	tuner_none,
	"Keep the meta server informed about our game?\n"
    },
    {
	"denyHosts",
	"denyHosts",
	"",
	&denyHosts,
	valString,
	Set_deny_hosts,
	"List of network addresses of computers which are denied service.\n"
	"Each address may optionally be followed by a slash and a network mask.\n"
    },
    {
	"limitedVisibility",
	"limitedVisibility",
	"no",
	&limitedVisibility,
	valBool,
	Set_world_rules,
	"Should the players have a limited visibility?\n"
    },
    {
	"minVisibilityDistance",
	"minVisibility",
	"0.0",
	&minVisibilityDistance,
	valReal,
	tuner_none,
	"Minimum block distance for limited visibility, 0 for default.\n"
    },
    {
	"maxVisibilityDistance",
	"maxVisibility",
	"0.0",
	&maxVisibilityDistance,
	valReal,
	tuner_none,
	"Maximum block distance for limited visibility, 0 for default.\n"
    },
    {
	"limitedLives",
	"limitedLives",
	"no",
	&limitedLives,
	valBool,
	tuner_none,
	"Should players have limited lives?\n"
	"See also worldLives.\n"
    },
    {
	"worldLives",
	"lives",
	"0",
	&worldLives,
	valInt,
	tuner_none,
	"Number of lives each player has (no sense without limitedLives).\n"
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
	"This option is only effective when limitedLives is turned on.\n"
    },
    {
	"teamPlay",
	"teams",
	"no",
	&teamPlay,
	valBool,
	tuner_none,
	"Is the map a team play map?\n"
    },
    {
	"teamAssign",
	"teamAssign",
	"yes",
	&teamAssign,
	valBool,
	tuner_dummy,
	"If a player has not specified which team he likes to join\n"
	"should the server choose a team for him automatically?\n"
    },
    {
	"teamImmunity",
	"teamImmunity",
	"yes",
	&teamImmunity,
	valBool,
	tuner_dummy,
	"Should other team members be immune to various shots thrust etc.?\n"
    },
    {
	"ecmsReprogramMines",
	"ecmsReprogramMines",
	"yes",
	&ecmsReprogramMines,
	valBool,
	tuner_dummy,
	"Is it possible to reprogram mines with ECMs?\n"
    },
    {
	"ecmsReprogramRobots",
	"ecmsReprogramRobots",
	"yes",
	&ecmsReprogramRobots,
	valBool,
	tuner_dummy,
	"Are robots reprogrammed by ECMs instead of blinded?\n"
    },
    {
	"targetKillTeam",
	"targetKillTeam",
	"no",
	&targetKillTeam,
	valBool,
	tuner_dummy,
	"Do team members die when their last target explodes?\n"
    },
    {
	"targetTeamCollision",
	"targetCollision",
	"yes",
	&targetTeamCollision,
	valBool,
	tuner_dummy,
	"Do team members collide with their own target or not.\n"
    },
    {
	"targetSync",
	"targetSync",
	"no",
	&targetSync,
	valBool,
	tuner_dummy,
	"Do all the targets of a team reappear/repair at the same time?"
    },
    {
	"treasureKillTeam",
	"treasureKillTeam",
	"no",
	&treasureKillTeam,
	valBool,
	tuner_dummy,
	"Do team members die when their treasure is destroyed?\n"
    },
    {
	"treasureCollisionDestroys",
	"treasureCollisionDestroy",
	"yes",
	&treasureCollisionDestroys,
	valBool,
	tuner_dummy,
	"Are balls destroyed when a player touches it?\n"
    },
    {
	"treasureCollisionMayKill",
	"treasureUnshieldedCollisionKills",
	"no",
	&treasureCollisionMayKill,
	valBool,
	tuner_dummy,
	"Does a ball kill a player when the player touches it unshielded?\n"
    },
    {
	"onePlayerOnly",
	"onePlayerOnly",
	"no",
	&onePlayerOnly,
	valBool,
	tuner_none,
	"One player modus.\n"
    },
    {
	"timing",
	"race",
	"no",
	&timing,
	valBool,
	tuner_none,
	"Is the map a race mode map?\n"
    },
    {
	"edgeWrap",
	"edgeWrap",
	"no",
	&edgeWrap,
	valBool,
	tuner_none,
	"Wrap around edges.\n"
    },
    {
	"edgeBounce",
	"edgeBounce",
	"yes",
	&edgeBounce,
	valBool,
	tuner_dummy,
	"Players and bullets bounce when they hit the (non-wrapping) edge.\n"
    },
    {
	"extraBorder",
	"extraBorder",
	"no",
	&extraBorder,
	valBool,
	tuner_none,
	"Give map an extra border of solid rock.\n"
    },
    {
	"gravityPoint",
	"gravityPoint",
	"0,0",
	&gravityPoint,
	valIPos,
	tuner_none,
	"If the gravity is a point source where does that gravity originate?\n"
	"Specify the point int the form: x,y.\n"
    },
    {
	"gravityAngle",
	"gravityAngle",
	"90",
	&gravityAngle,
	valReal,
	tuner_none,
	"If gravity is along a uniform line, at what angle is that line?\n"
    },
    {
	"gravityPointSource",
	"gravityPointSource",
	"false",
	&gravityPointSource,
	valBool,
	tuner_none,
	"Is gravity originating from a single point?\n"
    },
    {
	"gravityClockwise",
	"gravityClockwise",
	"false",
	&gravityClockwise,
	valBool,
	tuner_none,
	"If the gravity is a point source, is it clockwise?\n"
    },
    {
	"gravityAnticlockwise",
	"gravityAnticlockwise",
	"false",
	&gravityAnticlockwise,
	valBool,
	tuner_none,
	"If the gravity is a point source, is it anticlockwise?\n"
    },
    {
	"gravityVisible",
	"gravityVisible",
	"true",
	&gravityVisible,
	valBool,
	tuner_none,
	"Are gravity mapsymbols visible to players?\n"
    },
    {
	"wormholeVisible",
	"wormholeVisible",
	"true",
	&wormholeVisible,
	valBool,
	tuner_none,
	"Are wormhole mapsymbols visible to players?\n"
    },
    {
	"itemConcentratorVisible",
	"itemConcentratorVisible",
	"true",
	&itemConcentratorVisible,
	valBool,
	tuner_none,
	"Are itemconcentrator mapsymbols visible to players?\n"
    },
    {
	"defaultsFileName",
	"defaults",
	"",
	&defaultsFileName,
	valString,
	tuner_none,
	"The filename of the defaults file to read on startup.\n"
    },
    {
	"scoreTableFileName",
	"scoretable",
	"",
	&scoreTableFileName,
	valString,
	tuner_none,
	"The filename for the score table to be dumped to.\n"
    },
    {
	"framesPerSecond",
	"FPS",
	"14",
	&framesPerSecond,
	valInt,
	tuner_none,
	"The number of frames per second the server should strive for.\n"
    },
    {
	"allowSmartMissiles",
	"allowSmarts",
	"True",
	&allowSmartMissiles,
	valBool,
	tuner_dummy,
	"Should smart missiles be allowed?\n"
    },
    {
	"allowHeatSeekers",
	"allowHeats",
	"True",
	&allowHeatSeekers,
	valBool,
	tuner_dummy,
	"Should heatseekers be allowed?\n"
    },
    {
	"allowTorpedoes",
	"allowTorps",
	"True",
	&allowTorpedoes,
	valBool,
	tuner_dummy,
	"Should torpedoes be allowed?\n"
    },
    {
	"allowNukes",
	"nukes",
	"False",
	&allowNukes,
	valBool,
	tuner_none,
	"Should nuclear weapons be allowed?\n"
    },
    {
	"allowClusters",
	"clusters",
	"False",
	&allowClusters,
	valBool,
	tuner_none,
	"Should cluster weapons be allowed?\n"
    },
    {
	"allowModifiers",
	"modifiers",
	"False",
	&allowModifiers,
	valBool,
	tuner_none,
	"Should the weapon modifiers be allowed?\n"
    },
    {
	"allowLaserModifiers",
	"lasermodifiers",
	"False",
	&allowLaserModifiers,
	valBool,
	tuner_none,
	"Can lasers be modified to be a different weapon?\n"
    },
    {
	"allowShipShapes",
	"ShipShapes",
	"True",
	&allowShipShapes,
	valBool,
	tuner_none,
	"Are players allowed to define their own ship shape?\n"
    },
    {
	"playersOnRadar",
	"playersRadar",
	"True",
	&playersOnRadar,
	valBool,
	tuner_dummy,
	"Are players visible on the radar.\n"
    },
    {
	"missilesOnRadar",
	"missilesRadar",
	"True",
	&missilesOnRadar,
	valBool,
	tuner_dummy,
	"Are missiles visible on the radar.\n"
    },
    {
	"minesOnRadar",
	"minesRadar",
	"False",
	&minesOnRadar,
	valBool,
	tuner_dummy,
	"Are mines visible on the radar.\n"
    },
    {
	"nukesOnRadar",
	"nukesRadar",
	"True",
	&nukesOnRadar,
	valBool,
	tuner_dummy,
	"Are nukes visible or highlighted on the radar.\n"
    },
    {
	"treasuresOnRadar",
	"treasuresRadar",
	"False",
	&treasuresOnRadar,
	valBool,
	tuner_dummy,
	"Are treasure balls visible or highlighted on the radar.\n"
    },
    {
	"distinguishMissiles",
	"distinguishMissiles",
	"True",
	&distinguishMissiles,
	valBool,
	tuner_dummy,
	"Are different types of missiles distinguished (by length).\n"
    },
    {
	"maxMissilesPerPack",
	"maxMissilesPerPack",
	"4",
	&maxMissilesPerPack,
	valInt,
	tuner_none,
	"The number of missiles gotten by picking up one missile item.\n"
    },
    {
	"maxMinesPerPack",
	"maxMinesPerPack",
	"2",
	&maxMinesPerPack,
	valInt,
	tuner_none,
	"The number of mines gotten by picking up one mine item.\n"
    },
    {
	"identifyMines",
	"identifyMines",
	"True",
	&identifyMines,
	valBool,
	tuner_dummy,
	"Are mine owner's names displayed.\n"
    },
    {
	"shieldedItemPickup",
	"shieldItem",
	"False",
	&shieldedItemPickup,
	valBool,
	tuner_dummy,
	"Can items be picked up while shields are up?\n"
    },
    {
	"shieldedMining",
	"shieldMine",
	"False",
	&shieldedMining,
	valBool,
	tuner_dummy,
	"Can mines be thrown and placed while shields are up?\n"
    },
    {
	"laserIsStunGun",
	"stunGun",
	"False",
	&laserIsStunGun,
	valBool,
	tuner_dummy,
	"Is the laser weapon a stun gun weapon?\n"
    },
    {
	"nukeMinSmarts",
	"nukeMinSmarts",
	"7",
	&nukeMinSmarts,
	valInt,
	tuner_dummy,
	"The minimum number of missiles needed to fire one nuclear missile.\n"
    },
    {
	"nukeMinMines",
	"nukeMinMines",
	"4",
	&nukeMinMines,
	valInt,
	tuner_dummy,
	"The minimum number of mines needed to make a nuclear mine.\n"
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
	"explosions, which improves server response time for such explosions.\n"
    },
    {
	"mineFuseTime",
	"mineFuseTime",
	"0.0",
	&mineFuseTime,
	valSec,
	tuner_dummy,
	"Time after which owned mines become deadly, zero means never.\n"
    },
    {
	"mineLife",
	"mineLife",
	"0",
	&mineLife,
	valInt,
	tuner_none,
	"Life of mines in ticks, zero means use default.\n"
    },
    {
	"missileLife",
	"missileLife",
	"0",
	&missileLife,
	valInt,
	tuner_none,
	"Life of missiles in ticks, zero means use default.\n"
    },
    {
	"movingItemProb",
	"movingItemProb",
	"0.2",
	&movingItemProb,
	valReal,
	Set_misc_item_limits,
	"Probability for an item to appear as moving.\n"
    },
    {
	"dropItemOnKillProb",
	"dropItemOnKillProb",
	"0.5",
	&dropItemOnKillProb,
	valReal,
	Set_misc_item_limits,
	"Probability for dropping an item (each item) when you are killed.\n"
    },
    {
	"detonateItemOnKillProb",
	"detonateItemOnKillProb",
	"0.5",
	&detonateItemOnKillProb,
	valReal,
	Set_misc_item_limits,
	"Probability for undropped items to detonate when you are killed.\n"
    },
    {
	"destroyItemInCollisionProb",
	"destroyItemInCollisionProb",
	"0.0",
	&destroyItemInCollisionProb,
	valReal,
	Set_misc_item_limits,
	"Probability for items (some items) to be destroyed in a collision.\n"
    },
    {
	"itemProbMult",
	"itemProbFact",
	"1.0",
	&itemProbMult,
	valReal,
	Tune_item_probs,
	"Item Probability Multiplication Factor scales all item probabilities.\n"
    },
    {
	"maxItemDensity",
	"maxItemDensity",
	"0.00012",
	&maxItemDensity,
	valReal,
	Tune_item_probs,
	"Maximum density [0.0-1.0] for items (max items per block).\n"
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
	"The map symbol is the percentage symbol '%'.\n"
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
	"within itemConcentratorRadius.\n"
    },
    {
	"rogueHeatProb",
	"rogueHeatProb",
	"1.0",
	&rogueHeatProb,
	valReal,
	tuner_dummy,
	"Probability that unclaimed missile packs will go rogue."
    },
    {
	"rogueMineProb",
	"rogueMineProb",
	"1.0",
	&rogueMineProb,
	valReal,
	tuner_dummy,
	"Probability that unclaimed mine items will activate."
    },
    {
	"itemEnergyPackProb",
	"itemEnergyPackProb",
	"1e-9",
	&World.items[ITEM_FUEL].prob,
	valReal,
	Tune_item_probs,
	"Probability for an energy pack to appear.\n"
    },
    {
	"itemTankProb",
	"itemTankProb",
	"1e-9",
	&World.items[ITEM_TANK].prob,
	valReal,
	Tune_item_probs,
	"Probability for an extra tank to appear.\n"
    },
    {
	"itemECMProb",
	"itemECMProb",
	"1e-9",
	&World.items[ITEM_ECM].prob,
	valReal,
	Tune_item_probs,
	"Probability for an ECM item to appear.\n"
    },
    {
	"itemMineProb",
	"itemMineProb",
	"1e-9",
	&World.items[ITEM_MINE].prob,
	valReal,
	Tune_item_probs,
	"Probability for a mine item to appear.\n"
    },
    {
	"itemMissileProb",
	"itemMissileProb",
	"1e-9",
	&World.items[ITEM_MISSILE].prob,
	valReal,
	Tune_item_probs,
	"Probability for a missile item to appear.\n"
    },
    {
	"itemCloakProb",
	"itemCloakProb",
	"1e-9",
	&World.items[ITEM_CLOAK].prob,
	valReal,
	Tune_item_probs,
	"Probability for a cloak item to appear.\n"
    },
    {
	"itemSensorProb",
	"itemSensorProb",
	"1e-9",
	&World.items[ITEM_SENSOR].prob,
	valReal,
	Tune_item_probs,
	"Probability for a sensor item to appear.\n"
    },
    {
	"itemWideangleProb",
	"itemWideangleProb",
	"1e-9",
	&World.items[ITEM_WIDEANGLE].prob,
	valReal,
	Tune_item_probs,
	"Probability for a wideangle item to appear.\n"
    },
    {
	"itemRearshotProb",
	"itemRearshotProb",
	"1e-9",
	&World.items[ITEM_REARSHOT].prob,
	valReal,
	Tune_item_probs,
	"Probability for a rearshot item to appear.\n"
    },
    {
	"itemAfterburnerProb",
	"itemAfterburnerProb",
	"1e-9",
	&World.items[ITEM_AFTERBURNER].prob,
	valReal,
	Tune_item_probs,
	"Probability for an afterburner item to appear.\n"
    },
    {
	"itemTransporterProb",
	"itemTransporterProb",
	"1e-9",
	&World.items[ITEM_TRANSPORTER].prob,
	valReal,
	Tune_item_probs,
	"Probability for a transporter item to appear.\n"
    },
    {
	"itemLaserProb",
	"itemLaserProb",
	"1e-9",
	&World.items[ITEM_LASER].prob,
	valReal,
	Tune_item_probs,
	"Probability for a Laser item to appear.\n"
    },
    {
	"itemEmergencyThrustProb",
	"itemEmergencyThrustProb",
	"1e-9",
	&World.items[ITEM_EMERGENCY_THRUST].prob,
	valReal,
	Tune_item_probs,
	"Probability for an Emergency Thrust item to appear.\n"
    },
    {
	"itemTractorBeamProb",
	"itemTractorBeamProb",
	"1e-9",
	&World.items[ITEM_TRACTOR_BEAM].prob,
	valReal,
	Tune_item_probs,
	"Probability for a Tractor Beam item to appear.\n"
    },
    {
	"itemAutopilotProb",
	"itemAutopilotProb",
	"1e-9",
	&World.items[ITEM_AUTOPILOT].prob,
	valReal,
	Tune_item_probs,
	"Probability for an Autopilot item to appear.\n"
    },
    {
	"itemEmergencyShieldProb",
	"itemEmergencyShieldProb",
	"1e-9",
	&World.items[ITEM_EMERGENCY_SHIELD].prob,
	valReal,
	Tune_item_probs,
	"Probability for an Emergency Shield item to appear.\n"
    },
    {
	"initialFuel",
	"initialFuel",
	"1000",
	&World.items[ITEM_FUEL].initial,
	valInt,
	Set_initial_resources,
	"How much fuel players start with, or the minimum after being killed.\n"
    },
    {
	"initialTanks",
	"initialTanks",
	"0",
	&World.items[ITEM_TANK].initial,
	valInt,
	Set_initial_resources,
	"How many tanks players start with.\n"
    },
    {
	"initialECMs",
	"initialECMs",
	"0",
	&World.items[ITEM_ECM].initial,
	valInt,
	Set_initial_resources,
	"How many ECMs players start with.\n"
    },
    {
	"initialMines",
	"initialMines",
	"0",
	&World.items[ITEM_MINE].initial,
	valInt,
	Set_initial_resources,
	"How many mines players start with.\n"
    },
    {
	"initialMissiles",
	"initialMissiles",
	"0",
	&World.items[ITEM_MISSILE].initial,
	valInt,
	Set_initial_resources,
	"How many missiles players start with.\n"
    },
    {
	"initialCloaks",
	"initialCloaks",
	"0",
	&World.items[ITEM_CLOAK].initial,
	valInt,
	Set_initial_resources,
	"How many cloaks players start with.\n"
    },
    {
	"initialSensors",
	"initialSensors",
	"0",
	&World.items[ITEM_SENSOR].initial,
	valInt,
	Set_initial_resources,
	"How many sensors players start with.\n"
    },
    {
	"initialWideangles",
	"initialWideangles",
	"0",
	&World.items[ITEM_WIDEANGLE].initial,
	valInt,
	Set_initial_resources,
	"How many wideangles players start with.\n"
    },
    {
	"initialRearshots",
	"initialRearshots",
	"0",
	&World.items[ITEM_REARSHOT].initial,
	valInt,
	Set_initial_resources,
	"How many rearshots players start with.\n"
    },
    {
	"initialAfterburners",
	"initialAfterburners",
	"0",
	&World.items[ITEM_AFTERBURNER].initial,
	valInt,
	Set_initial_resources,
	"How many afterburners players start with.\n"
    },
    {
	"initialTransporters",
	"initialTransporters",
	"0",
	&World.items[ITEM_TRANSPORTER].initial,
	valInt,
	Set_initial_resources,
	"How many transporters players start with.\n"
    },
    {
	"initialLasers",
	"initialLasers",
	"0",
	&World.items[ITEM_LASER].initial,
	valInt,
	Set_initial_resources,
	"How many lasers players start with.\n"
    },
    {
	"initialEmergencyThrusts",
	"initialEmergencyThrusts",
	"0",
	&World.items[ITEM_EMERGENCY_THRUST].initial,
	valInt,
	Set_initial_resources,
	"How many emergency thrusts players start with.\n"
    },
    {
	"initialTractorBeams",
	"initialTractorBeams",
	"0",
	&World.items[ITEM_TRACTOR_BEAM].initial,
	valInt,
	Set_initial_resources,
	"How many tractor/pressor beams players start with.\n"
    },
    {
	"initialAutopilots",
	"initialAutopilots",
	"0",
	&World.items[ITEM_AUTOPILOT].initial,
	valInt,
	Set_initial_resources,
	"How many autopilots players start with.\n"
    },
    {
	"initialEmergencyShields",
	"initialEmergencyShields",
	"0",
	&World.items[ITEM_EMERGENCY_SHIELD].initial,
	valInt,
	Set_initial_resources,
	"How many emergency shields players start with.\n"
    },
    {
	"maxFuel",
	"maxFuel",
	"10000",
	&World.items[ITEM_FUEL].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the amount of fuel per player.\n"
    },
    {
	"maxTanks",
	"maxTanks",
	"8",
	&World.items[ITEM_TANK].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of tanks per player.\n"
    },
    {
	"maxECMs",
	"maxECMs",
	"10",
	&World.items[ITEM_ECM].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of ECMs per player.\n"
    },
    {
	"maxMines",
	"maxMines",
	"10",
	&World.items[ITEM_MINE].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of mines per player.\n"
    },
    {
	"maxMissiles",
	"maxMissiles",
	"10",
	&World.items[ITEM_MISSILE].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of missiles per player.\n"
    },
    {
	"maxCloaks",
	"maxCloaks",
	"10",
	&World.items[ITEM_CLOAK].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of cloaks per player.\n"
    },
    {
	"maxSensors",
	"maxSensors",
	"10",
	&World.items[ITEM_SENSOR].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of sensors per player.\n"
    },
    {
	"maxWideangles",
	"maxWideangles",
	"10",
	&World.items[ITEM_WIDEANGLE].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of wides per player.\n"
    },
    {
	"maxRearshots",
	"maxRearshots",
	"10",
	&World.items[ITEM_REARSHOT].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of rearshots per player.\n"
    },
    {
	"maxAfterburners",
	"maxAfterburners",
	"10",
	&World.items[ITEM_AFTERBURNER].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of afterburners per player.\n"
    },
    {
	"maxTransporters",
	"maxTransporters",
	"10",
	&World.items[ITEM_TRANSPORTER].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of transporters per player.\n"
    },
    {
	"maxEmergencyThrusts",
	"maxEmergencyThrusts",
	"10",
	&World.items[ITEM_EMERGENCY_THRUST].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of emergency thrusts per player.\n"
    },
    {
	"maxLasers",
	"maxLasers",
	"5",
	&World.items[ITEM_LASER].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of lasers per player.\n"
    },
    {
	"maxTractorBeams",
	"maxTractorBeams",
	"4",
	&World.items[ITEM_TRACTOR_BEAM].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of tractorbeams per player.\n"
    },
    {
	"maxAutopilots",
	"maxAutopilots",
	"10",
	&World.items[ITEM_AUTOPILOT].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of autopilots per player.\n"
    },
    {
	"maxEmergencyShields",
	"maxEmergencyShields",
	"10",
	&World.items[ITEM_EMERGENCY_SHIELD].limit,
	valInt,
	Set_initial_resources,
	"Upper limit on the number of emergency shields per player.\n"
    },
    {
	"gameDuration",
	"time",
	"0.0",
	&gameDuration,
	valReal,
	tuner_none,
	"The duration of the game in minutes (aka. pizza mode).\n"
    },
    {
	"friction",
	"friction",
	"0.0",
	&friction,
	valReal,
	tuner_dummy,
	"Fraction of velocity ship loses each frame.\n"
    },
    {
	"checkpointRadius",
	"checkpointRadius",
	"6.0",
	&checkpointRadius,
	valReal,
	tuner_dummy,
	"How close you have to be to a checkpoint to register - in blocks.\n"
    },
    {
	"raceLaps",
	"raceLaps",
	"3",
	&raceLaps,
	valInt,
	tuner_none,
	"How many laps a race is run over.\n"
    },
    {
	"lockOtherTeam",
	"lockOtherTeam",
	"true",
	&lockOtherTeam,
	valBool,
	tuner_none,
	"Can you lock on players from other teams when you're dead.\n"
    },
    {
	"loseItemDestroys",
	"loseItemDestroys",
	"false",
	&loseItemDestroys,
	valBool,
	tuner_none,
	"Destroy item that player drops. Otherwise drop it.\n"
    },
    {
	"maxOffensiveItems",
	"maxOffensiveItems",
	"100",
	&maxOffensiveItems,
	valInt,
	tuner_dummy,
	"How many offensive items a player can carry.\n"
    },
    {
	"maxDefensiveItems",
	"maxDefensiveItems",
	"100",
	&maxDefensiveItems,
	valInt,
	tuner_dummy,
	"How many defensive items a player can carry.\n"
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
	"Whether the server is prevented from being swapped out of memory.\n"
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

static void Parse_dump(char *progname)
{
    int			j;

    xpprintf("\n");
    xpprintf("# %s option dump\n", progname);
    xpprintf("# \n");
    xpprintf("# LIBDIR = %s\n", LIBDIR);
    xpprintf("# DEFAULTS_FILE_NAME = %s\n", DEFAULTS_FILE_NAME);
    xpprintf("# MAPDIR = %s\n", MAPDIR);
    xpprintf("# DEFAULT_MAP = %s\n", DEFAULT_MAP);
    xpprintf("# SERVERMOTDFILE = %s\n", SERVERMOTDFILE);
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

void Parser(int argc, char *argv[])
{
    int			i, j;
    char		*fname;


    for (i=1; i<argc; i++) {
	if (strncmp("-help", argv[i], 2) == 0) {
	    Parse_help(*argv);
#ifndef	_WINDOWS
	    exit(0);
#endif
	}
	if (strcmp("-dump", argv[i]) == 0) {
	    Parse_dump(*argv);
#ifndef	_WINDOWS
	    exit(0);
#endif
	}
	if (strcmp("-version", argv[i]) == 0 || strcmp("-v", argv[i]) == 0) {
	    puts(TITLE);
#ifndef	_WINDOWS
	    exit(0);
#endif
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
     * Read map file if map data not found yet
     */
    if (!(fname = getOption("mapData"))) {
	if (!(fname = getOption("mapFileName"))) {
#ifndef SILENT
	    xpprintf("Map not specified, trying to open " DEFAULT_MAP "\n");
#endif
	    if (!parseDefaultsFile(DEFAULT_MAP))
		xpprintf("Unable to read " DEFAULT_MAP);
	} else {
	    if (!parseDefaultsFile(fname)) {
		xpprintf("Unable to read %s, trying to open " DEFAULT_MAP, fname);
		if (!parseDefaultsFile(DEFAULT_MAP))
		    xpprintf("Unable to read " DEFAULT_MAP);
	    }
	}
    }

    /*
     * Read local defaults file
     */
    if ((fname = getOption("defaultsFileName")) != NULL)
	parseDefaultsFile(fname);
    else
	parseDefaultsFile(DEFAULTS_FILE_NAME);

    for (j = 0; j < NELEM(options); j++)
	addOption(options[j].name, options[j].defaultValue, 0, &options[j]);
    parseOptions();
    Grok_map();
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


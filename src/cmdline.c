/* $Id: cmdline.c,v 3.69 1994/09/17 00:56:29 bert Exp $
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
#include <stdio.h>

#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "robot.h"
#include "defaults.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: cmdline.c,v 3.69 1994/09/17 00:56:29 bert Exp $";
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
float		minVisibilityDistance;	/* Minimum visibility when starting */
float		maxVisibilityDistance;	/* Maximum visibility */
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
float           rogueHeatProb;          /* prob. that unclaimed rocketpack */
float           rogueMineProb;          /* or minepack will "activate" */
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
int		itemConcentratorRadius;
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
bool		treasuresOnRadar;	/* Are treasure balls radar visible? */
bool		distinguishMissiles;	/* Smarts, heats & torps look diff.? */
int		maxMissilesPerPack;	/* Number of missiles per item. */
bool		identifyMines;		/* Mines have names displayed? */
bool		shieldedItemPickup;	/* Pickup items with shields up? */
bool		shieldedMining;		/* Detach mines with shields up? */
bool		laserIsStunGun;		/* Is the laser a stun gun? */
bool		reportToMetaServer;	/* Send status to meta-server? */
float		gameDuration;		/* total duration of game in minutes */

bool		teamAssign;		/* Assign player to team if not set? */
bool		teamImmunity;		/* Is team immune from player action */

bool		targetKillTeam;		/* if your target explodes, you die? */
bool		targetTeamCollision;	/* Does team collide with target? */
bool		targetSync;		/* all targets reappear together */
bool		treasureKillTeam;	/* die if treasure is destroyed? */
bool		treasureCollisionDestroys;
bool		treasureCollisionMayKill;

float		friction;		/* friction only affects ships */
float		checkpointRadius;      	/* in blocks */
int		raceLaps;		/* how many laps per race */

static optionDesc options[] = {
    {
	"help",
	"help",
	"0",
	NULL,
	valVoid,
	"Print out this help message.\n"
    },
    {
	"version",
	"version",
	"0",
	NULL,
	valVoid,
	"Print version information.\n"
    },
    {
	"dump",
	"dump",
	"0",
	NULL,
	valVoid,
	"Print all options with their default values in defaultsfile format.\n"
    },
    {
	"gravity",
	"gravity",
	"-0.14",
	&Gravity,
	valReal,
	"Gravity strength.\n"
    },
    {
	"shipMass",
	"shipMass",
	"20.0",
	&ShipMass,
	valReal,
	"Mass of fighters.\n"
    },
    {
	"shotMass",
	"shotMass",
	"0.1",
	&ShotsMass,
	valReal,
	"Mass of bullets.\n"
    },
    {
	"shotSpeed",
	"shotSpeed",
	"21.0",
	&ShotsSpeed,
	valReal,
	"Maximum speed of bullets.\n"
    },
    {
	"shotLife",
	"shotLife",
	"60",
	&ShotsLife,
	valInt,
	"Life of bullets in ticks.\n"
    },
    {
	"fireRepeatRate",
	"fireRepeat",
	"2",
	&fireRepeatRate,
	valInt,
	"Number of frames per automatic fire (0=off).\n"
    },
    {
	"maxRobots",
	"robots",
	"4",
	&WantedNumRobots,
	valInt,
	"How many robots do you want?\n"
    },
    {
	"robotsTalk",
	"robotsTalk",
	"false",
	&robotsTalk,
	valBool,
	"Do robots talk when they kill, die etc.?\n"
    },
    {
	"robotsLeave",
	"robotsLeave",
	"true",
	&robotsLeave,
	valBool,
	"Do robots leave the game?\n"
    },
    {
	"robotLeaveLife",
	"robotLeaveLife",
	"50",
	&robotLeaveLife,
	valInt,
	"Max life per robot (0=off).\n"
    },
    {
	"robotLeaveScore",
	"robotLeaveScore",
	"-90",
	&robotLeaveScore,
	valInt,
	"Min score for robot to play (0=off).\n"
    },
    {
	"robotLeaveRatio",
	"robotLeaveRatio",
	"-5",
	&robotLeaveRatio,
	valInt,
	"Min ratio for robot to play (0=off).\n"
    },
    {
	"maxPlayerShots",
	"shots",
	"256",
	&ShotsMax,
	valInt,
	"Maximum allowed bullets per player.\n"
    },
    {
	"shotsGravity",
	"shotsGravity",
	"true",
	&ShotsGravity,
	valBool,
	"Are bullets afflicted by gravity.\n"
    },
    {
	"idleRun",
	"rawMode",
	"false",
	&RawMode,
	valBool,
	"Do robots keep on playing even if all human players quit?\n"
    },
    {
	"noQuit",
	"noQuit",
	"false",
	&NoQuit,
	valBool,
	"Does the server wait for new human players to show up\n"
	"after all players have left.\n"
    },
    {
	"mapWidth",
	"mapWidth",
	"100",
	&mapWidth,
	valInt,
	"Width of the world in blocks.\n"
    },
    {
	"mapHeight",
	"mapHeight",
	"100",
	&mapHeight,
	valInt,
	"Height of the world in blocks.\n"
    },
    {
	"mapFileName",
	"map",
	DEFAULT_MAP,
	&mapFileName,
	valString,
	"The filename of the map to use.\n"
    },
    {
	"mapName",
	"mapName",
	"unknown",
	&mapName,
	valString,
	"The title of the map.\n"
    },
    {
	"mapAuthor",
	"mapAuthor",
	"anonymous",
	&mapAuthor,
	valString,
	"The name of the map author.\n"
    },
    {
	"contactPort",
	"port",
	"15345",
	&contactPort,
	valInt,
	"The server contact port number.\n"
    },
    {
	"motd",
	"motd",
	SERVERMOTDFILE,
	&motd,
	valString,
	"The filename for the server-motd.\n"
    },
    {
	"mapData",
	"mapData",
	NULL,
	&mapData,
	valString,
	"The map's topology.\n"
    },
    {
	"allowPlayerCrashes",
	"allowPlayerCrashes",
	"yes",
	&crashWithPlayer,
	valBool,
	"Can players overrun other players?\n"
    },
    {
	"allowPlayerBounces",
	"allowPlayerBounces",
	"yes",
	&bounceWithPlayer,
	valBool,
	"Can players bounce with other players?\n"
    },
    {
	"allowPlayerKilling",
	"killings",
	"yes",
	&playerKillings,
	valBool,
	"Should players be allowed to kill one other?\n"
    },
    {
	"allowShields",
	"shields",
	"yes",
	&playerShielding,
	valBool,
	"Are shields allowed?\n"
    },
    {
	"playerStartsShielded",
	"playerStartShielded",
	"yes",
	&playerStartsShielded,
	valBool,
	"Do players start with shields up?\n"
    },
    {
	"shotsWallBounce",
	"shotsWallBounce",
	"no",
	&shotsWallBounce,
	valBool,
	"Do shots bounce off walls?\n"
    },
    {
	"ballsWallBounce",
	"ballsWallBounce",
	"yes",
	&ballsWallBounce,
	valBool,
	"Do balls bounce off walls?\n"
    },
    {
	"minesWallBounce",
	"minesWallBounce",
	"no",
	&minesWallBounce,
	valBool,
	"Do mines bounce off walls?\n"
    },
    {
	"itemsWallBounce",
	"itemsWallBounce",
	"yes",
	&itemsWallBounce,
	valBool,
	"Do items bounce off walls?\n"
    },
    {
	"missilesWallBounce",
	"missilesWallBounce",
	"no",
	&missilesWallBounce,
	valBool,
	"Do missiles bounce off walls?\n"
    },
    {
	"sparksWallBounce",
	"sparksWallBounce",
	"no",
	&sparksWallBounce,
	valBool,
	"Do thrust spark particles bounce off walls to give reactive thrust?\n"
    },
    {
	"debrisWallBounce",
	"debrisWallBounce",
	"no",
	&debrisWallBounce,
	valBool,
	"Do explosion debris particles bounce off walls?\n"
    },
    {
	"maxObjectWallBounceSpeed",
	"maxObjectBounceSpeed",
	"40",
	&maxObjectWallBounceSpeed,
	valReal,
	"The maximum allowed speed for objects to bounce off walls.\n"
    },
    {
	"maxShieldedWallBounceSpeed",
	"maxShieldedBounceSpeed",
	"50",
	&maxShieldedWallBounceSpeed,
	valReal,
	"The maximum allowed speed for a shielded player to bounce off walls.\n"
    },
    {
	"maxUnshieldedWallBounceSpeed",
	"maxUnshieldedBounceSpeed",
	"20",
	&maxUnshieldedWallBounceSpeed,
	valReal,
	"Maximum allowed speed for an unshielded player to bounce off walls.\n"
    },
    {
	"maxShieldedPlayerWallBounceAngle",
	"maxShieldedBounceAngle",
	"90",
	&maxShieldedWallBounceAngle,
	valReal,
	"Maximum allowed angle for a shielded player to bounce off walls.\n"
    },
    {
	"maxUnshieldedPlayerWallBounceAngle",
	"maxUnshieldedBounceAngle",
	"30",
	&maxUnshieldedWallBounceAngle,
	valReal,
	"Maximum allowed angle for an unshielded player to bounce off walls.\n"
    },
    {
	"playerWallBounceBrakeFactor",
	"playerWallBrake",
	"0.89",
	&playerWallBrakeFactor,
	valReal,
	"Factor to slow down players when they hit the wall (between 0 and 1).\n"
    },
    {
	"objectWallBounceBrakeFactor",
	"objectWallBrake",
	"0.95",
	&objectWallBrakeFactor,
	valReal,
	"Factor to slow down objects when they hit the wall (between 0 and 1).\n"
    },
    {
	"objectWallBounceLifeFactor",
	"objectWallBounceLifeFactor",
	"0.80",
	&objectWallBounceLifeFactor,
	valReal,
	"Factor to reduce the life of objects after bouncing (between 0 and 1).\n"
    },
    {
	"wallBounceFuelDrainMult",
	"wallBounceDrain",
	"1.0",
	&wallBounceFuelDrainMult,
	valReal,
	"Multiplication factor for player wall bounce fuel cost.\n"
    },
    {
	"wallBounceDestroyItemProb",
	"wallBounceDestroyItemProb",
	"0.0",
	&wallBounceDestroyItemProb,
	valReal,
	"The probability for each item a player owns to get destroyed\n"
	"when the player bounces against a wall.\n"
    },
    {
	"reportToMetaServer",
	"reportMeta",
	"yes",
	&reportToMetaServer,
	valBool,
	"Keep the meta server informed about our game?\n"
    },
    {
	"limitedVisibility",
	"limitedVisibility",
	"no",
	&limitedVisibility,
	valBool,
	"Should the players have a limited visibility?\n"
    },
    {
	"minVisibilityDistance",
	"minVisibility",
	"0.0",
	&minVisibilityDistance,
	valReal,
	"Minimum block distance for limited visibility, 0 for default.\n"
    },
    {
	"maxVisibilityDistance",
	"maxVisibility",
	"0.0",
	&maxVisibilityDistance,
	valReal,
	"Maximum block distance for limited visibility, 0 for default.\n"
    },
    {
	"limitedLives",
	"limitedLives",
	"no",
	&limitedLives,
	valBool,
	"Should players have limited lives?\n"
	"See also worldLives.\n"
    },
    {
	"worldLives",
	"lives",
	"0",
	&worldLives,
	valInt,
	"Number of lives each player has (no sense without limitedLives).\n"
    },
    {
	"reset",
	"reset",
	"yes",
	&endOfRoundReset,
	valBool,
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
	"Is the map a team play map?\n"
    },
    {
	"teamAssign",
	"teamAssign",
	"yes",
	&teamAssign,
	valBool,
	"If a player has not specified which team he likes to join\n"
	"should the server choose a team for him automatically?\n"
    },
    {
	"teamImmunity",
	"teamImmunity",
	"yes",
	&teamImmunity,
	valBool,
	"Should other team members be immune to various shots thrust etc.?\n"
    },
    {
	"emcsReprogramMines",
	"ecmsReprogramMines",
	"yes",
	&ecmsReprogramMines,
	valBool,
	"Is it possible to reprogram mines with ECMs?\n"
    },
    {
	"targetKillTeam",
	"targetKillTeam",
	"no",
	&targetKillTeam,
	valBool,
	"Do team members die when their last target explodes?\n"
    },
    {
	"targetTeamCollision",
	"targetCollision",
	"yes",
	&targetTeamCollision,
	valBool,
	"Do team members collide with their own target or not.\n"
    },
    {
	"targetSync",
	"targetSync",
	"no",
	&targetSync,
	valBool,
	"Do all the targets of a team reappear/repair at the same time?"
    },
    {
	"treasureKillTeam",
	"treasureKillTeam",
	"no",
	&treasureKillTeam,
	valBool,
	"Do team members die when their treasure is destroyed?\n"
    },
    {
	"treasureCollisionDestroys",
	"treasureCollisionDestroy",
	"yes",
	&treasureCollisionDestroys,
	valBool,
	"Are balls destroyed when a player touches it?\n"
    },
    {
	"treasureCollisionMayKill",
	"treasureUnshieldedCollisionKills",
	"no",
	&treasureCollisionMayKill,
	valBool,
	"Does a ball kill a player when the player touches it unshielded?\n"
    },
    {
	"onePlayerOnly",
	"onePlayerOnly",
	"no",
	&onePlayerOnly,
	valBool,
	"One player modus.\n"
    },
    {
	"timing",
	"race",
	"no",
	&timing,
	valBool,
	"Is the map a race mode map?\n"
    },
    {
	"edgeWrap",
	"edgeWrap",
	"no",
	&edgeWrap,
	valBool,
	"Wrap around edges.\n"
    },
    {
	"edgeBounce",
	"edgeBounce",
	"yes",
	&edgeBounce,
	valBool,
	"Players and bullets bounce when they hit the (non-wrapping) edge.\n"
    },
    {
	"extraBorder",
	"extraBorder",
	"no",
	&extraBorder,
	valBool,
	"Give map an extra border of solid rock.\n"
    },
    {
	"gravityPoint",
	"gravityPoint",
	"0,0",
	&gravityPoint,
	valIPos,
	"If the gravity is a point source where does that gravity originate?\n"
	"Specify the point int the form: x,y.\n"
    },
    {
	"gravityAngle",
	"gravityAngle",
	"90",
	&gravityAngle,
	valReal,
	"If gravity is along a uniform line, at what angle is that line?\n"
    },
    {
	"gravityPointSource",
	"gravityPointSource",
	"false",
	&gravityPointSource,
	valBool,
	"Is gravity originating from a single point?\n"
    },
    {
	"gravityClockwise",
	"gravityClockwise",
	"false",
	&gravityClockwise,
	valBool,
	"If the gravity is a point source, is it clockwise?\n"
    },
    {
	"gravityAnticlockwise",
	"gravityAnticlockwise",
	"false",
	&gravityAnticlockwise,
	valBool,
	"If the gravity is a point source, is it anticlockwise?\n"
    },
    {
	"defaultsFileName",
	"defaults",
	"",
	&defaultsFileName,
	valString,
	"The filename of the defaults file to read on startup.\n"
    },
    {
	"scoreTableFileName",
	"scoretable",
	"",
	&scoreTableFileName,
	valString,
	"The filename for the score table to be dumped to.\n"
    },
    {
	"framesPerSecond",
	"FPS",
	"18",
	&framesPerSecond,
	valInt,
	"The number of frames per second the server should strive for.\n"
    },
    {
	"allowNukes",
	"nukes",
	"False",
	&allowNukes,
	valBool,
	"Should nuclear weapons be allowed?\n"
    },
    {
	"allowClusters",
	"clusters",
	"False",
	&allowClusters,
	valBool,
	"Should cluster weapons be allowed?\n"
    },
    {
	"allowModifiers",
	"modifiers",
	"False",
	&allowModifiers,
	valBool,
	"Should the weapon modifiers be allowed?\n"
    },
    {
	"allowLaserModifiers",
	"lasermodifiers",
	"False",
	&allowLaserModifiers,
	valBool,
	"Can lasers be modified to be a different weapon?\n"
    },
    {
	"allowShipShapes",
	"ShipShapes",
	"True",
	&allowShipShapes,
	valBool,
	"Are players allowed to define their own ship shape?\n"
    },
    {
	"playersOnRadar",
	"playersRadar",
	"True",
	&playersOnRadar,
	valBool,
	"Are players visible on the radar.\n"
    },
    {
	"missilesOnRadar",
	"missilesRadar",
	"True",
	&missilesOnRadar,
	valBool,
	"Are missiles visible on the radar.\n"
    },
    {
	"minesOnRadar",
	"minesRadar",
	"False",
	&minesOnRadar,
	valBool,
	"Are mines visible on the radar.\n"
    },
    {
	"nukesOnRadar",
	"nukesRadar",
	"True",
	&nukesOnRadar,
	valBool,
	"Are nukes visible or highlighted on the radar.\n"
    },
    {
	"treasuresOnRadar",
	"treasuresRadar",
	"False",
	&treasuresOnRadar,
	valBool,
	"Are treasure balls visible or highlighted on the radar.\n"
    },
    {
	"distinguishMissiles",
	"distinguishMissiles",
	"True",
	&distinguishMissiles,
	valBool,
	"Are different types of missiles distinguished (by length).\n"
    },
    {
	"maxMissilesPerPack",
	"maxMissilesPerPack",
	"4",
	&maxMissilesPerPack,
	valInt,
	"The number of missiles gotten by picking up one missile item.\n"
    },
    {
	"identifyMines",
	"identifyMines",
	"True",
	&identifyMines,
	valBool,
	"Are mine owner's names displayed.\n"
    },
    {
	"shieldedItemPickup",
	"shieldItem",
	"False",
	&shieldedItemPickup,
	valBool,
	"Can items be picked up while shields are up?\n"
    },
    {
	"shieldedMining",
	"shieldMine",
	"False",
	&shieldedMining,
	valBool,
	"Can mines be thrown and placed while shields are up?\n"
    },
    {
	"laserIsStunGun",
	"stunGun",
	"False",
	&laserIsStunGun,
	valBool,
	"Is the laser weapon a stun gun weapon?\n"
    },
    {
	"nukeMinSmarts",
	"nukeMinSmarts",
	"7",
	&nukeMinSmarts,
	valInt,
	"The minimum number of missiles needed to fire one nuclear missile.\n"
    },
    {
	"nukeMinMines",
	"nukeMinMines",
	"4",
	&nukeMinMines,
	valInt,
	"The minimum number of mines needed to make a nuclear mine.\n"
    },
    {
	"nukeClusterDamage",
	"nukeClusterDamage",
	"1.0",
	&nukeClusterDamage,
	valReal,
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
	"Time after which owned mines become deadly, zero means never.\n"
    },
    {
	"movingItemProb",
	"movingItemProb",
	"0.2",
	&movingItemProb,
	valReal,
	"Probability for an item to appear as moving.\n"
    },
    {
	"dropItemOnKillProb",
	"dropItemOnKillProb",
	"0.5",
	&dropItemOnKillProb,
	valReal,
	"Probability for dropping an item (each item) when you are killed.\n"
    },
    {
	"detonateItemOnKillProb",
	"detonateItemOnKillProb",
	"0.5",
	&detonateItemOnKillProb,
	valReal,
	"Probability for undropped items to detonate when you are killed.\n"
    },
    {
	"destroyItemInCollisionProb",
	"destroyItemInCollisionProb",
	"0.0",
	&destroyItemInCollisionProb,
	valReal,
	"Probability for items (some items) to be destroyed in a collision.\n"
    },
    {
	"itemProbMult",
	"itemProbFact",
	"1.0",
	&itemProbMult,
	valReal,
	"Item Probability Multiplication Factor scales all item probabilities.\n"
    },
    {
	"maxItemDensity",
	"maxItemDensity",
	"0.00012",
	&maxItemDensity,
	valReal,
	"Maximum density [0.0-1.0] for items (max items per block).\n"
    },
    {
	"itemConcentratorRadius",
	"itemConcentratorRange",
	"10",
	&itemConcentratorRadius,
	valInt,
	"The maximum distance from an item concentator for items to appear in.\n"
	"Sensible values are in the range 1 to 20.\n"
	"If no item concentators are defined in a map then items can popup anywhere.\n"
	"Otherwise items always popup in the vicinity of an item concentrator.\n"
	"An item concentrator is drawn on screen as three rotating triangles.\n"
	"The map symbol is the percentage symbol '%'.\n"
    },
    {
	"rogueHeatProb",
	"rogueHeatProb",
	"1.0",
	&rogueHeatProb,
	valReal,
	"Probability that unclaimed missile packs will go rogue."
    },
    {
	"rogueMineProb",
	"rogueMineProb",
	"1.0",
	&rogueMineProb,
	valReal,
	"Probability that unclaimed mine items will activate."
    },
    {
	"itemEnergyPackProb",
	"itemEnergyPackProb",
	"0",
	&itemEnergyPackProb,
	valReal,
	"Probability for an energy pack to appear.\n"
    },
    {
	"itemTankProb",
	"itemTankProb",
	"0",
	&itemTankProb,
	valReal,
	"Probability for an extra tank to appear.\n"
    },
    {
	"itemECMProb",
	"itemECMProb",
	"0",
	&itemECMProb,
	valReal,
	"Probability for an ECM item to appear.\n"
    },
    {
	"itemMineProb",
	"itemMineProb",
	"0",
	&itemMineProb,
	valReal,
	"Probability for a mine item to appear.\n"
    },
    {
	"itemMissileProb",
	"itemMissileProb",
	"0",
	&itemMissileProb,
	valReal,
	"Probability for a missile item to appear.\n"
    },
    {
	"itemCloakProb",
	"itemCloakProb",
	"0",
	&itemCloakProb,
	valReal,
	"Probability for a cloak item to appear.\n"
    },
    {
	"itemSensorProb",
	"itemSensorProb",
	"0",
	&itemSensorProb,
	valReal,
	"Probability for a sensor item to appear.\n"
    },
    {
	"itemWideangleProb",
	"itemWideangleProb",
	"0",
	&itemWideangleProb,
	valReal,
	"Probability for a wideangle item to appear.\n"
    },
    {
	"itemRearshotProb",
	"itemRearshotProb",
	"0",
	&itemRearshotProb,
	valReal,
	"Probability for a rearshot item to appear.\n"
    },
    {
	"itemAfterburnerProb",
	"itemAfterburnerProb",
	"0",
	&itemAfterburnerProb,
	valReal,
	"Probability for an afterburner item to appear.\n"
    },
    {
	"itemTransporterProb",
	"itemTransporterProb",
	"0",
	&itemTransporterProb,
	valReal,
	"Probability for a transporter item to appear.\n"
    },
    {
	"itemLaserProb",
	"itemLaserProb",
	"0",
	&itemLaserProb,
	valReal,
	"Probability for a Laser item to appear.\n"
    },
    {
	"itemEmergencyThrustProb",
	"itemEmergencyThrustProb",
	"0",
	&itemEmergencyThrustProb,
	valReal,
	"Probability for an Emergency Thrust item to appear.\n"
    },
    {
	"itemTractorBeamProb",
	"itemTractorBeamProb",
	"0",
	&itemTractorBeamProb,
	valReal,
	"Probability for a Tractor Beam item to appear.\n"
    },
    {
	"itemAutopilotProb",
	"itemAutopilotProb",
	"0",
	&itemAutopilotProb,
	valReal,
	"Probability for an Autopilot item to appear.\n"
    },
    {
	"itemEmergencyShieldProb",
	"itemEmergencyShieldProb",
	"0",
	&itemEmergencyShieldProb,
	valReal,
	"Probability for an Emergency Shield item to appear.\n"
    },
    {
	"initialFuel",
	"initialFuel",
	"1000",
	&initialFuel,
	valInt,
	"How much fuel players start with, or the minimum after being killed.\n"
    },
    {
	"initialTanks",
	"initialTanks",
	"0",
	&initialTanks,
	valInt,
	"How many tanks players start with.\n"
    },
    {
	"initialECMs",
	"initialECMs",
	"0",
	&initialECMs,
	valInt,
	"How many ECMs players start with.\n"
    },
    {
	"initialMines",
	"initialMines",
	"0",
	&initialMines,
	valInt,
	"How many mines players start with.\n"
    },
    {
	"initialMissiles",
	"initialMissiles",
	"0",
	&initialMissiles,
	valInt,
	"How many missiles players start with.\n"
    },
    {
	"initialCloaks",
	"initialCloaks",
	"0",
	&initialCloaks,
	valInt,
	"How many cloaks players start with.\n"
    },
    {
	"initialSensors",
	"initialSensors",
	"0",
	&initialSensors,
	valInt,
	"How many sensors players start with.\n"
    },
    {
	"initialWideangles",
	"initialWideangles",
	"0",
	&initialWideangles,
	valInt,
	"How many wideangles players start with.\n"
    },
    {
	"initialRearshots",
	"initialRearshots",
	"0",
	&initialRearshots,
	valInt,
	"How many rearshots players start with.\n"
    },
    {
	"initialAfterburners",
	"initialAfterburners",
	"0",
	&initialAfterburners,
	valInt,
	"How many afterburners players start with.\n"
    },
    {
	"initialTransporters",
	"initialTransporters",
	"0",
	&initialTransporters,
	valInt,
	"How many transporters players start with.\n"
    },
    {
	"initialLasers",
	"initialLasers",
	"0",
	&initialLasers,
	valInt,
	"How many lasers players start with.\n"
    },
    {
	"initialEmergencyThrusts",
	"initialEmergencyThrusts",
	"0",
	&initialEmergencyThrusts,
	valInt,
	"How many emergency thrusts players start with.\n"
    },
    {
	"initialTractorBeams",
	"initialTractorBeams",
	"0",
	&initialTractorBeams,
	valInt,
	"How many tractor/pressor beams players start with.\n"
    },
    {
	"initialAutopilots",
	"initialAutopilots",
	"0",
	&initialAutopilots,
	valInt,
	"How many autopilots players start with.\n"
    },
    {
	"initialEmergencyShields",
	"initialEmergencyShields",
	"0",
	&initialEmergencyShields,
	valInt,
	"How many emergency shields players start with.\n"
    },
    {
	"gameDuration",
	"time",
	"0.0",
	&gameDuration,
	valReal,
	"The duration of the game in minutes (aka. pizza mode).\n"
    },
    {
	"friction",
	"friction",
	"0.0",
	&friction,
	valReal,
	"Fraction of velocity ship loses each frame.\n"
    },
    {
	"checkpointRadius",
	"checkpointRadius",
	"6.0",
	&checkpointRadius,
	valReal,
	"How close you have to be to a checkpoint to register - in blocks.\n"
    },
    {
	"raceLaps",
	"raceLaps",
	"3",
	&raceLaps,
	valInt,
	"How many laps a race is run over.\n"
    },
};


static void Parse_help(char *progname)
{
    int			j;
    char		*str;

    printf("Usage: %s [ options ]\n"
	   "Where options include:\n"
	   "\n",
	   progname);
    for (j = 0; j < NELEM(options); j++) {
	printf("    %s%s",
	       options[j].type == valBool ? "-/+" : "-",
	       options[j].name);
	if (strcasecmp(options[j].commandLineOption, options[j].name))
	    printf(" or %s", options[j].commandLineOption);
	printf(" %s\n",
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
    printf(
"    \n"
"    The probabilities are in the range [0.0-1.0] and they refer to the\n"
"    probability that an event will occur in a block in second.\n"
"    Boolean options are turned off by using +<option>.\n"
"    \n"
"    Please refer to the manual pages, xpilots(6) and xpilot(6),\n"
"    for more specific help.\n"
	  );
}

static void Parse_dump(char *progname)
{
    int			j;

    printf("\n");
    printf("# %s option dump\n", progname);
    printf("# \n");
    printf("# LIBDIR = %s\n", LIBDIR);
    printf("# DEFAULTS_FILE_NAME = %s\n", DEFAULTS_FILE_NAME);
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
	if (strcmp("-version", argv[i]) == 0 || strcmp("-v", argv[i]) == 0) {
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
    if ((fname = getOption("defaultsFileName")) != NULL)
	parseDefaultsFile(fname);
    else
	parseDefaultsFile(DEFAULTS_FILE_NAME);

    for (j = 0; j < NELEM(options); j++)
	addOption(options[j].name, options[j].defaultValue, 0, &options[j]);
    parseOptions();
    Grok_map();
}

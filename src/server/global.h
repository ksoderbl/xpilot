/* $Id: global.h,v 5.34 2002/08/21 14:22:30 bertg Exp $
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

#ifndef	GLOBAL_H
#define	GLOBAL_H

#ifndef OBJECT_H
/* need player */
#include "object.h"
#endif

#ifndef MAP_H
/* need World_map */
#include "map.h"
#endif

#ifndef LIST_H_INCLUDED
/* need list_t */
#include "list.h"
#endif

#ifndef MAX
#define MAX(a,b)  ((a) > (b) ? (a) : (b))
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

#define	STR80	(80)

typedef struct {
    char	owner[STR80];
    char	host[STR80];
} server_t;

/*
 * Global data.
 */
#define FPS		framesPerSecond
#define NumObjs		(ObjCount + 0)

extern player		**Players;
extern object		*Obj[];
extern pulse_t		*Pulses[];
extern ecm_t		*Ecms[];
extern trans_t		*Transporters[];
extern long		frame_loops;
extern int		NumPlayers;
extern int		NumPseudoPlayers;
extern int		NumQueuedPlayers;
extern int		ObjCount;
extern int		NumPulses;
extern int		NumEcms;
extern int		NumTransporters;
extern int		NumAlliances;
extern int		NumRobots, maxRobots, minRobots;
extern int		login_in_progress;

extern char		*robotFile;
extern int		robotsTalk, robotsLeave, robotLeaveLife;
extern int		robotLeaveScore, robotLeaveRatio;
extern int		robotTeam;
extern bool		restrictRobots, reserveRobotTeam;
extern World_map	World;
extern server_t		Server;
extern list_t		expandList;
extern DFLOAT		ShotsMass, ShipMass, ShotsSpeed, Gravity;
extern DFLOAT		ballMass;
extern DFLOAT		minItemMass;
extern int		ShotsMax, ShotsLife;
extern bool		ShotsGravity;
extern bool		shotHitFuelDrainUsesKineticEnergy;
extern int		fireRepeatRate;
extern long		DEF_BITS, KILL_BITS, DEF_HAVE, DEF_USED, USED_KILL;
extern int		GetInd[];
extern int		ShutdownServer, ShutdownDelay;
extern bool		RawMode;
extern bool		NoQuit;
extern bool		logRobots;
extern int		framesPerSecond;
extern long		main_loops;
extern char		*mapFileName;
extern int		mapRule;
extern char		*mapData;
extern int		mapWidth;
extern int		mapHeight;
extern char		*mapName;
extern char		*mapAuthor;
extern int 		contactPort;
extern char		*serverHost;
extern char		*serverAddr;
extern bool		crashWithPlayer;
extern bool		bounceWithPlayer;
extern bool		playerKillings;
extern bool		playerShielding;
extern bool		playerStartsShielded;
extern bool		shotsWallBounce;
extern bool		ballsWallBounce;
extern bool		ballCollisions;
extern bool		ballSparkCollisions;
extern bool		minesWallBounce;
extern bool		itemsWallBounce;
extern bool		missilesWallBounce;
extern bool		sparksWallBounce;
extern bool		debrisWallBounce;
extern bool		asteroidsWallBounce;
extern bool		cloakedExhaust;
extern bool		cloakedShield;
extern bool		ecmsReprogramMines;
extern bool		ecmsReprogramRobots;
extern DFLOAT		maxObjectWallBounceSpeed;
extern DFLOAT		maxShieldedWallBounceSpeed;
extern DFLOAT		maxUnshieldedWallBounceSpeed;
extern DFLOAT		maxShieldedWallBounceAngle;
extern DFLOAT		maxUnshieldedWallBounceAngle;
extern DFLOAT		playerWallBrakeFactor;
extern DFLOAT		objectWallBrakeFactor;
extern DFLOAT		objectWallBounceLifeFactor;
extern DFLOAT		wallBounceFuelDrainMult;
extern DFLOAT		wallBounceDestroyItemProb;

extern bool		limitedVisibility;
extern DFLOAT		minVisibilityDistance;
extern DFLOAT		maxVisibilityDistance;
extern bool		limitedLives;
extern int		worldLives;
extern bool		endOfRoundReset;
extern int		resetOnHuman;
extern bool		allowAlliances;
extern bool		announceAlliances;
extern bool		teamPlay;
extern bool		teamFuel;
extern bool		teamCannons;
extern int		cannonSmartness;
extern bool		cannonsUseItems;
extern bool		cannonsDefend;
extern bool		cannonFlak;
extern int		cannonDeadTime;
extern bool		keepShots;
extern bool		teamAssign;
extern bool		teamImmunity;
extern bool		teamShareScore;
extern bool		timing;
extern bool		ballrace;
extern bool		ballrace_connect;
extern bool		edgeWrap;
extern bool		edgeBounce;
extern bool		extraBorder;
extern ipos		gravityPoint;
extern DFLOAT		gravityAngle;
extern bool		gravityPointSource;
extern bool		gravityClockwise;
extern bool		gravityAnticlockwise;
extern bool		gravityVisible;
extern bool		wormholeVisible;
extern bool		itemConcentratorVisible;
extern bool		asteroidConcentratorVisible;
extern int		wormTime;
extern int		nukeMinSmarts;
extern int		nukeMinMines;
extern DFLOAT		nukeClusterDamage;
extern int		mineFuseTime;
extern int		mineLife;
extern DFLOAT		minMineSpeed;
extern int		missileLife;
extern int		baseMineRange;
extern int		mineShotDetonateDistance;

extern DFLOAT		shotKillScoreMult;
extern DFLOAT		torpedoKillScoreMult;
extern DFLOAT		smartKillScoreMult;
extern DFLOAT		heatKillScoreMult;
extern DFLOAT		clusterKillScoreMult;
extern DFLOAT		laserKillScoreMult;
extern DFLOAT		tankKillScoreMult;
extern DFLOAT		runoverKillScoreMult;
extern DFLOAT		ballKillScoreMult;
extern DFLOAT		explosionKillScoreMult;
extern DFLOAT		shoveKillScoreMult;
extern DFLOAT		crashScoreMult;
extern DFLOAT		mineScoreMult;
extern DFLOAT		selfKillScoreMult;
extern DFLOAT		selfDestructScoreMult;
extern DFLOAT		unownedKillScoreMult;
extern DFLOAT		asteroidPoints;
extern DFLOAT		cannonPoints;
extern DFLOAT		asteroidMaxScore;
extern DFLOAT		cannonMaxScore;

extern DFLOAT		destroyItemInCollisionProb;
extern bool		updateScores;
extern bool 		allowSmartMissiles;
extern bool 		allowHeatSeekers;
extern bool 		allowTorpedoes;
extern bool 		allowNukes;
extern bool		allowClusters;
extern bool		allowModifiers;
extern bool		allowLaserModifiers;
extern bool		allowShipShapes;

extern bool		shieldedItemPickup;
extern bool		shieldedMining;
extern bool		laserIsStunGun;
extern bool		targetKillTeam;
extern bool		targetSync;
extern int		targetDeadTime;
extern bool		reportToMetaServer;
extern bool		searchDomainForXPilot;
extern char		*denyHosts;
extern int		maxClientsPerIP;

extern bool		playersOnRadar;
extern bool		missilesOnRadar;
extern bool		minesOnRadar;
extern bool		nukesOnRadar;
extern bool		treasuresOnRadar;
extern bool		asteroidsOnRadar;
extern bool 		identifyMines;
extern bool		distinguishMissiles;
extern int		maxMissilesPerPack;
extern int		maxMinesPerPack;
extern bool		targetTeamCollision;
extern bool		treasureKillTeam;
extern bool		captureTheFlag;
extern bool		treasureCollisionDestroys;
extern bool		treasureCollisionMayKill;
extern bool		wreckageCollisionMayKill;
extern bool		asteroidCollisionMayKill;

extern DFLOAT		ballConnectorSpringConstant;
extern DFLOAT		ballConnectorDamping;
extern DFLOAT		maxBallConnectorRatio;
extern DFLOAT		ballConnectorLength;
extern bool		connectorIsString;

extern DFLOAT 		dropItemOnKillProb;
extern DFLOAT		detonateItemOnKillProb;
extern DFLOAT 		movingItemProb;
extern DFLOAT		randomItemProb;
extern DFLOAT            rogueHeatProb;
extern DFLOAT            rogueMineProb;
extern DFLOAT		itemProbMult;
extern DFLOAT		cannonItemProbMult;
extern DFLOAT		asteroidItemProb;
extern int		asteroidMaxItems;
extern DFLOAT		maxItemDensity;
extern DFLOAT		maxAsteroidDensity;
extern int		itemConcentratorRadius;
extern DFLOAT		itemConcentratorProb;
extern int		asteroidConcentratorRadius;
extern DFLOAT		asteroidConcentratorProb;
extern DFLOAT		gameDuration;
extern bool		allowViewing;
extern int		game_lock;

extern char		*motdFileName;
extern char	       	*scoreTableFileName;
extern char		*adminMessageFileName;
extern int		adminMessageFileSizeLimit;

extern DFLOAT		friction;
extern DFLOAT		blockFriction;
extern bool		blockFrictionVisible;
extern int		coriolis;
extern DFLOAT		checkpointRadius;
extern int		raceLaps;
extern bool		lockOtherTeam;
extern bool 		loseItemDestroys;
extern int		maxOffensiveItems;
extern int		maxDefensiveItems;

extern int		maxVisibleObject;
extern bool		pLockServer;
extern int		timerResolution;

extern int		roundDelaySeconds;
extern int		round_delay;
extern int		round_delay_send;
extern int		maxRoundTime;
extern int		roundtime;
extern int		roundsToPlay;
extern int		roundsPlayed;

extern bool		useWreckage;
extern bool		ignore20MaxFPS;
extern char		*password;

extern char		*robotRealName;
extern char		*robotHostName;

extern char		*tankRealName;
extern char		*tankHostName;
extern int		tankScoreDecrement;

extern bool		turnThrust;
extern bool		selfImmunity;

extern char		*defaultShipShape;
extern char		*tankShipShape;

extern int		clientPortStart;
extern int		clientPortEnd;

extern int		maxPauseTime;

extern long		KILLING_SHOTS;
extern unsigned		SPACE_BLOCKS;

#endif /* GLOBAL_H */


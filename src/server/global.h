/* $Id: global.h,v 4.10 1999/10/10 18:44:49 bert Exp $
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

#ifndef MAX
#define MAX(a,b)  ((a) > (b) ? (a) : (b))
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

#define	STR80	(80)

typedef struct {
    char name[STR80];
    char host[STR80];
} server;

/*
 * Global data.
 */
#ifdef SERVER
#define FPS			framesPerSecond
extern player		**Players;
extern object		*Obj[];
extern pulse_t		*Pulses[];
extern ecm_t		*Ecms[];
extern trans_t		*Transporters[];
extern long		frame_loops;
extern int		NumPlayers;
extern int		NumPseudoPlayers;
extern int		NumQueuedPlayers;
extern int		NumObjs;
extern int		NumPulses;
extern int		NumEcms;
extern int		NumTransporters;
extern int		NumRobots, maxRobots, minRobots;
extern int		login_in_progress;

extern char		*robotFile;
extern int		robotsTalk, robotsLeave, robotLeaveLife;
extern int		robotLeaveScore, robotLeaveRatio;
extern int		robotTeam;
extern bool		restrictRobots, reserveRobotTeam;
extern World_map	World;
extern server		Server;
extern DFLOAT		ShotsMass, ShipMass, ShotsSpeed, Gravity;
extern int		ShotsMax, ShotsLife;
extern bool		ShotsGravity;
extern int		fireRepeatRate;
extern long		DEF_BITS, KILL_BITS, DEF_HAVE, DEF_USED, USED_KILL;
extern int		GetInd[];
#endif
extern DFLOAT		tbl_sin[];
extern DFLOAT		tbl_cos[];
#ifdef SERVER
extern int		ShutdownServer, ShutdownDelay;
extern bool		RawMode;
extern bool		NoQuit;
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
extern bool		crashWithPlayer;
extern bool		bounceWithPlayer;
extern bool		playerKillings;
extern bool		playerShielding;
extern bool		playerStartsShielded;
extern bool		shotsWallBounce;
extern bool		ballsWallBounce;
extern bool		minesWallBounce;
extern bool		itemsWallBounce;
extern bool		missilesWallBounce;
extern bool		sparksWallBounce;
extern bool		debrisWallBounce;
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
extern bool		teamPlay;
extern bool		teamFuel;
extern bool		teamCannons;
extern int		cannonSmartness;
extern bool		cannonsUseItems;
extern bool		keepShots;
extern bool		teamAssign;
extern bool		teamImmunity;
extern bool		onePlayerOnly;
extern bool		timing;
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
extern int		wormTime;
extern int		nukeMinSmarts;
extern int		nukeMinMines;
extern DFLOAT		nukeClusterDamage;
extern int		mineFuseTime;
extern int		mineLife;
extern int		missileLife;
extern int		baseMineRange;

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
extern bool		reportToMetaServer;
extern char		*denyHosts;

extern bool		playersOnRadar;
extern bool		missilesOnRadar;
extern bool		minesOnRadar;
extern bool		nukesOnRadar;
extern bool		treasuresOnRadar;
extern bool 		identifyMines;
extern bool		distinguishMissiles;
extern int		maxMissilesPerPack;
extern int		maxMinesPerPack;
extern bool		targetTeamCollision;
extern bool		treasureKillTeam;
extern bool		treasureCollisionDestroys;
extern bool		treasureCollisionMayKill;
extern bool		wreckageCollisionMayKill;

extern DFLOAT 		dropItemOnKillProb;
extern DFLOAT		detonateItemOnKillProb;
extern DFLOAT 		movingItemProb;
extern DFLOAT            rogueHeatProb;
extern DFLOAT            rogueMineProb;
extern DFLOAT		itemProbMult;
extern DFLOAT		cannonItemProbMult;
extern DFLOAT		maxItemDensity;
extern int		itemConcentratorRadius;
extern DFLOAT		itemConcentratorProb;
extern DFLOAT		gameDuration;
extern bool		allowViewing;
extern time_t		gameOverTime;

extern char	       	*scoreTableFileName;

extern DFLOAT		friction;
extern DFLOAT		checkpointRadius;
extern int		raceLaps;
extern bool		lockOtherTeam;
extern bool 		loseItemDestroys;
extern int		maxOffensiveItems;
extern int		maxDefensiveItems;

extern bool		anaColDet;

extern bool		pLockServer;

extern int		roundDelay;
extern int		rdelay;
extern int		rdelaySend;
extern int		maxRoundTime;
extern int		roundtime;

extern bool		useWreckage;
extern bool		ignore20MaxFPS;

#endif

#endif /* GLOBAL_H */


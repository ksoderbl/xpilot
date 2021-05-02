/* $Id: global.h,v 3.49 1994/03/30 16:56:23 bert Exp $
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

#ifndef	GLOBAL_H
#define	GLOBAL_H

#include <stdio.h>
/*#include <stdlib.h> */
#include <math.h>
#include <errno.h>
#include "config.h"
#include "types.h"
#include "rules.h"
#include "object.h"
#include "map.h"
#include "draw.h"
#include "bit.h"
#include "version.h"
#include "proto.h"


#ifndef MAX
#define MAX(a,b)  ((a) > (b) ? (a) : (b))
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

typedef struct {
    char name[80];
    char host[80];
} server;


/*
 * Global data.
 */
#ifdef SERVER
#define FPS			framesPerSecond
extern player		**Players;
extern object		*Obj[];
extern long		loops;
extern long		Id;
extern int		NumPlayers;
extern int		NumPseudoPlayers;
extern int		NumObjs;
extern int		NumRobots, WantedNumRobots;
extern int		robotsTalk, robotsLeave, robotLeaveLife;
extern int		robotLeaveScore, robotLeaveRatio;
extern World_map	World;
extern server		Server;
extern float		ShotsMass, ShipMass, ShotsSpeed, Gravity;
extern int		ShotsMax, ShotsLife;
extern bool		ShotsGravity;
extern int		fireRepeatRate;
extern long		DEF_BITS, KILL_BITS, DEF_HAVE, DEF_USED, USED_KILL;
extern long		GetInd[];
#endif
extern float		tbl_sin[];
#ifdef SERVER
extern int		ShutdownServer, ShutdownDelay;
extern bool		RawMode;
extern bool		NoQuit;
extern int		framesPerSecond;
extern char		*mapFileName;
extern int		mapRule;
extern char		*mapData;
extern int		mapWidth;
extern int		mapHeight;
extern char		*mapName;
extern char		*mapAuthor;
extern int 		contactPort;
extern char		*motd;
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
extern bool		ecmsReprogramMines;
extern float		maxObjectWallBounceSpeed;
extern float		maxShieldedWallBounceSpeed;
extern float		maxUnshieldedWallBounceSpeed;
extern float		maxShieldedWallBounceAngle;
extern float		maxUnshieldedWallBounceAngle;
extern float		playerWallBrakeFactor;
extern float		objectWallBrakeFactor;
extern float		objectWallBounceLifeFactor;
extern float		wallBounceFuelDrainMult;
extern float		wallBounceDestroyItemProb;

extern float		playerMinimumStartFuel;

extern bool		limitedVisibility;
extern bool		limitedLives;
extern int		worldLives;
extern bool		teamPlay;
extern bool		teamAssign;
extern bool		teamImmunity;
extern bool		onePlayerOnly;
extern bool		timing;
extern bool		edgeWrap;
extern bool		edgeBounce;
extern bool		extraBorder;
extern ipos		gravityPoint;
extern float		gravityAngle;
extern bool		gravityPointSource;
extern bool		gravityClockwise;
extern bool		gravityAnticlockwise;
extern int		MovingItemsRand;
extern int 		ThrowItemOnKillRand;
extern int		nukeMinSmarts;
extern int		nukeMinMines;
extern float		nukeClusterDamage;
extern int		mineFuseTime;

extern float		destroyItemInCollisionProb;
extern bool		updateScores;
extern bool 		allowNukes;
extern bool		allowClusters;
extern bool		allowModifiers;
extern bool		allowLaserModifiers;
extern bool		allowShipShapes;

extern bool		shieldedItemPickup;
extern bool		shieldedMining;
extern bool		laserIsStunGun;
extern bool		targetKillTeam;
extern bool		reportToMetaServer;

extern bool		playersOnRadar;
extern bool		missilesOnRadar;
extern bool		minesOnRadar;
extern bool		nukesOnRadar;
extern bool 		identifyMines;
extern bool		distinguishMissiles;
extern bool		targetTeamCollision;
extern bool		treasureKillTeam;
extern bool		treasureCollisionDestroys;
extern bool		treasureCollisionMayKill;

extern float 		dropItemOnKillProb;
extern float 		movingItemProb;
extern float 		itemEnergyPackProb;
extern float 		itemTankProb;
extern float		itemECMProb;
extern float		itemMineProb;
extern float 		itemMissileProb;
extern float		itemCloakProb;
extern float		itemSensorProb;
extern float		itemWideangleProb;
extern float		itemRearshotProb;
extern float		itemAfterburnerProb;
extern float		itemTransporterProb;
extern float		itemLaserProb;
extern float		itemEmergencyThrustProb;
extern float		itemTractorBeamProb;
extern float		itemAutopilotProb;
extern float		itemProbMult;
extern float		maxItemDensity;
extern float		gameDuration;
extern time_t		gameOverTime;

extern int		initialFuel;
extern int		initialTanks;
extern int		initialECMs;
extern int		initialMines;
extern int		initialMissiles;
extern int		initialCloaks;
extern int		initialSensors;
extern int		initialWideangles;
extern int		initialRearshots;
extern int		initialAfterburners;
extern int		initialTransporters;
extern int		initialLasers;
extern int		initialEmergencyThrusts;
extern int		initialTractorBeams;
extern int		initialAutopilots;

extern char	       	*scoreTableFileName;
#endif

#endif /* GLOBAL_H */


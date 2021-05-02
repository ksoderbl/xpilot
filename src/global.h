/* $Id: global.h,v 3.13 1993/08/02 12:55:00 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	GLOBAL_H
#define	GLOBAL_H

#include <stdio.h>
#include <stdlib.h>
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


typedef struct {
    int max_num;
    char name[80];
    char host[80];
} server;


/*
 * Global data.
 */
extern player		**Players;
extern object		*Obj[];
extern wireobj		ships[];
extern long		loops;
extern long		Id;
extern int		NumPlayers;
extern int		NumPseudoPlayers;
extern int		NumObjs;
extern int		NumRobots, WantedNumRobots;
extern int		robotsLeave, robotLeaveLife;
extern int		robotLeaveScore, robotLeaveRatio;
extern World_map	World;
extern server		Server;
extern int		RadarHeight;
extern float		ShotsMass, ShipPower, ShipMass, ShotsSpeed, Gravity;
extern int		ShotsMax, ShotsLife;
extern bool		ShotsGravity;
extern int		fireRepeatRate;
extern long		DEF_BITS, KILL_BITS, DEF_HAVE, DEF_USED, USED_KILL;
extern long		GetInd[];
extern float		tbl_sin[];
extern int		Shutdown, ShutdownDelay;
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
extern bool		crashWithPlayer;
extern bool		playerKillings;
extern bool		playerShielding;
extern bool		playerStartsShielded;
extern bool		limitedVisibility;
extern bool		limitedLives;
extern int		worldLives;
extern bool		teamPlay;
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
extern float		destroyItemInCollisionProb;
extern bool		updateScores;
extern bool 		allowNukes;
extern bool		playersOnRadar;
extern bool		missilesOnRadar;
extern bool		targetKillTeam;
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
extern float		maxItemDensity;

#endif /* GLOBAL_H */

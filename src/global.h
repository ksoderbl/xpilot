/* $Id: global.h,v 1.10 1993/04/18 17:11:02 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	GLOBAL_H
#define	GLOBAL_H

#if defined(hpux)
#   pragma HP_ALIGN NATURAL
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <setjmp.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include "config.h"
#include "types.h"
#include "rules.h"
#include "object.h"
#include "map.h"
#include "draw.h"
#include "dbuff.h"
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
extern World_map	World;
extern message_t	*Msg[];
extern server		Server;
extern int		RadarHeight;
extern jmp_buf		SavedEnv;
extern float		ShotsMass, ShipPower, ShipMass, ShotsSpeed, Gravity;
extern int		ShotsMax, ShotsLife;
extern bool		ShotsGravity;
extern long		DEF_BITS, KILL_BITS, DEF_HAVE, DEF_USED, USED_KILL;
extern Atom		ProtocolAtom, KillAtom;
extern long		GetInd[];
extern float		tbl_sin[];
extern XColor		colors[];
extern int		Shutdown, ShutdownDelay;
extern int		RadarHeight;
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
extern bool		crashWithPlayer;
extern bool		playerKillings;
extern bool		playerShielding;
extern bool		limitedVisibility;
extern bool		limitedLives;
extern int		worldLives;
extern bool		teamPlay;
extern bool		onePlayerOnly;
extern bool		timing;
extern bool		edgeWrap;
extern bool		extraBorder;
extern ipos		gravityPoint;
extern float		gravityAngle;
extern bool		gravityPointSource;
extern bool		gravityClockwise;
extern bool		gravityAnticlockwise;
extern int		MovingItemsRand;
extern int 		ThrowItemOnKillRand;
extern bool		updateScores;
extern bool 		allowNukes;
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
extern float		maxItemDensity;

#if defined(__sun__)
#  define srand(s)	srandom(s)
#  define rand()	random()
#endif /* __sun__ */

#endif /* GLOBAL_H */

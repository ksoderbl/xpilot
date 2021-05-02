/*
 * XMapEdit, the XPilot Map Editor.  Copyright (C) 1993 by
 *
 *      Aaron Averill           <averila@oes.orst.edu>
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
 *
 * Modifications to XMapEdit
 * 1996:
 *      Robert Templeman        <mbcaprt@mphhpd.ph.man.ac.uk>
 * 1997:
 *      William Docter          <wad2@lehigh.edu>
 *
 * $Id: main.h,v 1.4 2000/03/13 18:20:04 bert Exp $
 */

#include                 <X11/Xlib.h>
#include                 <X11/Xutil.h>
#include                 <X11/Xos.h>
#include                 <X11/Xatom.h>
#include                 <X11/keysym.h>
#include                 <stdio.h>
#include                 <stdlib.h>
#include                 <string.h>
#include                 <ctype.h>
#include                 <time.h>
#include                 <memory.h>
#include                 <math.h>

#include                 "T_Toolkit.h"
#include                 "defaults.h"
#include                 "map.h"
#include                 "proto.h"
#include                 "tools.h"
#include                 "expose.h"
#include                 "file.h"
#include                 "round.h"
#include                 "help.h"
#include                 "grow.h"
#include                 "forms.h"

#define MAPWIDTH         0
#define MAPHEIGHT        1
#define MAPDATA          2
#define STRING           3
#define YESNO            4
#define FLOAT            5
#define POSFLOAT         6
#define INT              7
#define POSINT           8
#define COORD            9

typedef char    map_data_t[MAX_MAP_SIZE][MAX_MAP_SIZE];

typedef struct {
   max_str_t	mapName, mapAuthor, mapFileName;
   int                   width, height;
   char                  width_str[4], height_str[4];
   map_data_t            data;
   int                   view_x, view_y, view_zoom;
   int                   changed;
   char                  *comments;

   char		gravity[7];
   char		shipMass[7];
   char		shotMass[7];
   char		shotSpeed[7];
   char		shotLife[4];
   char		fireRepeatRate[7];
   char		minRobots[3];
   char		maxRobots[3];
   max_str_t	robotFile;
   int		robotsTalk;
   int		robotsLeave;
   char		robotLeaveLife[7];
   char		robotLeaveScore[7];
   char		robotLeaveRatio[7];
   char		robotTeam[3];
   int		restrictRobots;
   int		reserveRobotTeam;
   char		maxPlayerShots[4];
   int		shotsGravity;
   int		idleRun;
   int		noQuit;
   char		contactPort[7];
   int		allowPlayerCrashes;
   int		allowPlayerBounces;
   int		allowPlayerKilling;
   int		allowShields;
   int		playerStartsShielded;
   int		shotsWallBounce;
   int		ballsWallBounce;
   int		minesWallBounce;
   int		itemsWallBounce;
   int		missilesWallBounce;
   int		sparksWallBounce;
   int		debrisWallBounce;
   int		cloakedExhaust;
   int		cloakedShield;
   char		maxObjectWallBounceSpeed[20];
   char		maxShieldedWallBounceSpeed[20];
   char		maxUnshieldedWallBounceSpeed[20];
   char		maxShieldedPlayerWallBounceAngle[20];
   char		maxUnshieldedPlayerWallBounceAngle[20];
   char		playerWallBounceBrakeFactor[20];
   char		objectWallBounceBrakeFactor[20];
   char		objectWallBounceLifeFactor[20];
   char		wallBounceFuelDrainMult[20];
   char		wallBounceDestroyItemProb[20];
   int		reportToMetaServer;
   char		denyHosts[255];
   int		limitedVisibility;
   char		minVisibilityDistance[20];
   char		maxVisibilityDistance[20];
   int		limitedLives;
   char		worldLives[4];
   int		reset;
   int		resetOnHuman;
   int		teamPlay;
   int		teamCannons;
   int		teamFuel;
   char		cannonSmartness[3];
   int		cannonsUseItems;
   int		keepShots;
   int		teamAssign;
   int		teamImmunity;
   int		ecmsReprogramMines;
   int		ecmsReprogramRobots;
   int		targetKillTeam;
   int		targetTeamCollision;
   int		targetSync;
   int		treasureKillTeam;
   int		treasureCollisionDestroys;
   int		treasureCollisionMayKill;
   int		wreckageCollisionMayKill;
   int		ignore20MaxFPS;
   int		onePlayerOnly;
   int		timing;
   int		edgeWrap;
   int		edgeBounce;
   int		extraBorder;
   char		gravityPoint[8];
   char		gravityAngle[4];
   int		gravityPointSource;
   int		gravityClockwise;
   int		gravityAnticlockwise;
   int		gravityVisible;
   int		wormholeVisible;
   int		itemConcentratorVisible;
   char		wormTime[20];
   max_str_t	defaultsFileName;
   max_str_t	scoreTableFileName;
   char		framesPerSecond[20];
   int		allowSmartMissiles;
   int		allowHeatSeekers;
   int		allowTorpedoes;
   int		allowNukes;
   int		allowClusters;
   int		allowModifiers;
   int		allowLaserModifiers;
   int		allowShipShapes;
   int		playersOnRadar;
   int		missilesOnRadar;
   int		minesOnRadar;
   int		nukesOnRadar;
   int		treasuresOnRadar;
   int		distinguishMissiles;
   char		maxMissilesPerPack[20];
   char		maxMinesPerPack[20];
   int		identifyMines;
   int		shieldedItemPickup;
   int		shieldedMining;
   int		laserIsStunGun;
   char		nukeMinSmarts[20];
   char		nukeMinMines[20];
   char		nukeClusterDamage[20];
   char		mineFuseTime[20];
   char		mineLife[20];
   char		missileLife[20];
   char		baseMineRange[20];
   char		shotKillScoreMult[20];
   char		torpedoKillScoreMult[20];
   char		smartKillScoreMult[20];
   char		heatKillScoreMult[20];
   char		clusterKillScoreMult[20];
   char		laserKillScoreMult[20];
   char		tankKillScoreMult[20];
   char		runoverKillScoreMult[20];
   char		ballKillScoreMult[20];
   char		explosionKillScoreMult[20];
   char		shoveKillScoreMult[20];
   char		crashScoreMult[20];
   char		mineScoreMult[20];
   char		movingItemProb[20];
   char		dropItemOnKillProb[20];
   char		detonateItemOnKillProb[20];
   char		destroyItemInCollisionProb[20];
   char		itemProbMult[20];
   char		cannonItemProbMult[20];
   char		maxItemDensity[20];
   char		itemConcentratorRadius[20];
   char		itemConcentratorProb[20];
   char		rogueHeatProb[20];
   char		rogueMineProb[20];
   char		itemEnergyPackProb[20];
   char		itemTankProb[20];
   char		itemECMProb[20];
   char		itemArmorProb[20];
   char		itemMineProb[20];
   char		itemMissileProb[20];
   char		itemCloakProb[20];
   char		itemSensorProb[20];
   char		itemWideangleProb[20];
   char		itemRearshotProb[20];
   char		itemAfterburnerProb[20];
   char		itemTransporterProb[20];
   char		itemMirrorProb[20];
   char		itemDeflectorProb[20];
   char		itemHyperJumpProb[20];
   char		itemPhasingProb[20];
   char		itemLaserProb[20];
   char		itemEmergencyThrustProb[20];
   char		itemTractorBeamProb[20];
   char		itemAutopilotProb[20];
   char		itemEmergencyShieldProb[20];
   char		initialFuel[20];
   char		initialTanks[20];
   char		initialArmor[20];
   char		initialECMs[20];
   char		initialMines[20];
   char		initialMissiles[20];
   char		initialCloaks[20];
   char		initialSensors[20];
   char		initialWideangles[20];
   char		initialRearshots[20];
   char		initialAfterburners[20];
   char		initialTransporters[20];
   char		initialMirrors[20];
   char		maxArmor[20];
   char		initialDeflectors[20];
   char		initialHyperJumps[20];
   char		initialPhasings[20];
   char		initialLasers[20];
   char		initialEmergencyThrusts[20];
   char		initialTractorBeams[20];
   char		initialAutopilots[20];
   char		initialEmergencyShields[20];
   char		maxFuel[20];
   char		maxTanks[20];
   char		maxECMs[20];
   char		maxMines[20];
   char		maxMissiles[20];
   char		maxCloaks[20];
   char		maxSensors[20];
   char		maxWideangles[20];
   char		maxRearshots[20];
   char		maxAfterburners[20];
   char		maxTransporters[20];
   char		maxMirrors[20];
   char		maxDeflectors[20];
   char		maxPhasings[20];
   char		maxHyperJumps[20];
   char		maxEmergencyThrusts[20];
   char		maxLasers[20];
   char		maxTractorBeams[20];
   char		maxAutopilots[20];
   char		maxEmergencyShields[20];
   char		gameDuration[20];
   int		allowViewing;
   char		friction[20];
   char		checkpointRadius[20];
   char		raceLaps[20];
   int		lockOtherTeam;
   int		loseItemDestroys;
   char		maxOffensiveItems[20];
   char		maxDefensiveItems[20];
   char		roundDelay[20];
   char		maxRoundTime[20];
   int		analyticalCollisionDetection;
   int		pLockServer;


} map_t;

/* RTT */
typedef struct {
  char                   *name;
  char                   *value;
} charlie;
/* RTT */

extern char              *progname;

extern Window            mapwin, prefwin;
extern Pixmap            smlmap_pixmap;
extern int               mapwin_width, mapwin_height;

extern GC                Wall_GC, Decor_GC, Treasure_GC, Target_GC;
extern GC                Item_Conc_GC, Fuel_GC, Gravity_GC, Current_GC;
extern GC                Wormhole_GC, Base_GC, Cannon_GC;
extern GC                White_GC, Black_GC, xorgc;

extern int               drawicon,drawmode;
extern map_data_t        clipdata;
extern map_t             map;

typedef struct {
   char                  *name, *altname, *label;
   int                   length, type;
   char                  *charvar;
   int                   *intvar;
   int                   row,column,space;
} prefs_t;

extern int               numprefs;
extern prefs_t           prefs[222];












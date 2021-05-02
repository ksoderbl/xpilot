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
 * $Id: main.c,v 1.7 2000/05/20 14:46:17 bert Exp $
 */

#include                 "main.h"
#include                 "default_colors.h"

static char              *display_name = NULL;
char                     *progname;

Window                   mapwin, prefwin;
Pixmap                   smlmap_pixmap;
int                      mapwin_width, mapwin_height;
int                      geometry_width, geometry_height;

GC                       Wall_GC, Decor_GC, Treasure_GC, Target_GC;
GC                       Item_Conc_GC, Fuel_GC, Gravity_GC, Current_GC;
GC                       Wormhole_GC, Base_GC, Cannon_GC;
GC                       White_GC , Black_GC, xorgc;
char                     *fontname = "*-times-bold-r-*-*-14-*-*-*-*-*-*-*";

int                      drawicon,drawmode;
map_data_t               clipdata;
xpmap_t                    map;

int                      num_default_settings=225;
charlie                  default_settings[225] = {
	{ "gravity","-0.14" },
	{ "shipmass","20.0" },
	{ "shotmass","0.1" },
	{ "shotspeed","40" },
	{ "shotlife","20" },
	{ "firerepeatrate","2" },
  	{ "minrobots","-1" },
  	{ "maxrobots","6" },
  	{ "robotfile","\0" },
  	{ "robotstalk","no" },
  	{ "robotsleave","yes" },
  	{ "robotleavelife","50" },
  	{ "robotleavescore","-75" },
  	{ "robotleaveratio","-5" },
  	{ "robotteam","0" },
  	{ "restrictrobots","yes" },
	{ "reserverobotteam","yes" },
  	{ "maxplayershots","30" },
  	{ "shotsgravity","no" },
  	{ "idlerun","yes" },
  	{ "noquit","yes" },
  	{ "contactport","15345" },
  	{ "mapwidth","150" },
  	{ "mapheight","150" },
  	{ "mapname","<new map>" },
  	{ "mapauthor","<your name here>" },
  	{ "allowplayercrashes","yes" },
  	{ "allowplayerbounces","yes" },
  	{ "allowplayerkilling","yes" },
  	{ "allowshields","yes" },
  	{ "playerstartsshielded","yes" },
  	{ "shotswallbounce","no" },
  	{ "ballswallbounce","yes" },
  	{ "mineswallbounce","no" },
  	{ "itemswallbounce","yes" },
  	{ "missileswallbounce","no" },
  	{ "sparkswallbounce","no" },
  	{ "debriswallbounce","no" },
  	{ "cloakedexhaust","yes" },
  	{ "cloakedshield","yes" },
  	{ "maxobjectwallbouncespeed","1000" },
  	{ "maxshieldedwallbouncespeed","200" },
  	{ "maxunshieldedwallbouncespeed","100" },
  	{ "maxshieldedplayerwallbounceangle","180" },
  	{ "maxunshieldedplayerwallbounceangle","180" },
  	{ "playerwallbouncebrakefactor","0.4" },
  	{ "objectwallbouncebrakefactor","0.7" },
  	{ "objectwallbouncelifefactor","0.99" },
  	{ "wallbouncefueldrainmult","0.1" },
  	{ "wallbouncedestroyitemprob","0.0" },
  	{ "reporttometaserver","yes" },
  	{ "denyhosts","\0" },
  	{ "limitedvisibility","no" },
  	{ "minvisibilitydistance","0.0" },
  	{ "maxvisibilitydistance","0.0" },
  	{ "limitedlives","no" },
  	{ "worldlives","0" },
  	{ "reset","yes" },
  	{ "resetonhuman","no" },
  	{ "teamplay","no" },
  	{ "teamcannons","no" },
  	{ "teamfuel","no" },
  	{ "cannonsmartness","1" },
  	{ "cannonsuseitems","no" },
  	{ "keepshots","no" },
  	{ "teamassign","yes" },
  	{ "teamimmunity","yes" },
  	{ "ecmsreprogrammines","yes" },
  	{ "ecmsreprogramrobots","yes" },
  	{ "targetkillteam","yes" },
  	{ "targetteamcollision","no" },
  	{ "targetsync","no" },
  	{ "treasurekillteam","no" },
  	{ "treasurecollisiondestroys","yes" },
  	{ "treasurecollisionmaykill","no" },
  	{ "wreckagecollisionmaykill","no" },
  	{ "ignore20maxfps","yes" },
  	{ "oneplayeronly","no" },
  	{ "timing","no" },
  	{ "edgewrap","no" },
  	{ "edgebounce","yes" },
  	{ "extraborder","no" },
  	{ "gravitypoint","0,0" },
  	{ "gravityangle","90" },
  	{ "gravitypointsource","no" },
  	{ "gravityclockwise","no" },
  	{ "gravityanticlockwise","no" },
  	{ "gravityvisible","yes" },
  	{ "wormholevisible","yes" },
  	{ "itemconcentratorvisible","yes" },
  	{ "wormtime","0" },
  	{ "defaultsfilename","\0"},
  	{ "scoretablefilename","\0"},
  	{ "framespersecond","12" },
  	{ "allowsmartmissiles","yes" },
  	{ "allowheatseekers","yes" },
  	{ "allowtorpedoes","yes" },
  	{ "allownukes","yes" },
  	{ "allowclusters","yes" },
  	{ "allowmodifiers","yes" },
  	{ "allowlasermodifiers","yes" },
  	{ "allowshipshapes","yes" },
  	{ "playersonradar","yes" },
  	{ "missilesonradar","yes" },
  	{ "minesonradar","yes" },
  	{ "nukesonradar","yes" },
  	{ "treasuresonradar","yes" },
  	{ "distinguishmissiles","yes" },
  	{ "maxmissilesperpack","2" },
  	{ "maxminesperpack","2" },
  	{ "identifymines","yes" },
  	{ "shieldeditempickup","no" },
  	{ "shieldedmining","yes" },
  	{ "laserisstungun","no" },
  	{ "nukeminsmarts","4" },
  	{ "nukeminmines","3" },
  	{ "nukeclusterdamage","1.0" },
  	{ "minefusetime","0" },
  	{ "minelife","0" },
  	{ "missilelife","0" },
  	{ "baseminerange","0" },
  	{ "shotkillscoremult","1.0" },
  	{ "torpedokillscoremult","1.0" },
  	{ "smartkillscoremult","1.0" },
  	{ "heatkillscoremult","1.0" },
  	{ "clusterkillscoremult","1.0" },
  	{ "laserkillscoremult","1.0" },
  	{ "tankkillscoremult","0.44" },
  	{ "runoverkillscoremult","0.33" },
  	{ "ballkillscoremult","1.0" },
  	{ "explosionkillscoremult","0.33" },
  	{ "shovekillscoremult","0.5" },
  	{ "crashscoremult","0.33" },
  	{ "minescoremult","0.17" },
  	{ "movingitemprob","0.3" },
  	{ "dropitemonkillprob","0.8" },
  	{ "detonateitemonkillprob","0.1" },
  	{ "destroyitemincollisionprob","0.0" },
  	{ "itemprobmult","0.05" },
  	{ "cannonitemprobmult","1.0" },
  	{ "maxitemdensity","0.0001" },
  	{ "itemconcentratorradius","10" },
  	{ "itemconcentratorprob","1.0" },
  	{ "rogueheatprob","0" },
  	{ "roguemineprob","0" },
  	{ "itemenergypackprob","1e-7" },
  	{ "itemtankprob","1e-7" },
  	{ "itemecmprob","1e-7" },
  	{ "itemarmorprob","1e-7" },
  	{ "itemmineprob","1e-7" },
  	{ "itemmissileprob","1e-7" },
  	{ "itemcloakprob","1e-7" },
  	{ "itemsensorprob","1e-7" },
  	{ "itemwideangleprob","1e-7" },
  	{ "itemrearshotprob","1e-7" },
  	{ "itemafterburnerprob","1e-7" },
  	{ "itemtransporterprob","1e-7" },
  	{ "itemmirrorprob","1e-7" },
  	{ "itemdeflectorprob","1e-7" },
  	{ "itemhyperjumpprob","1e-7" },
  	{ "itemphasingprob","1e-7" },
  	{ "itemlaserprob","1e-7" },
  	{ "itememergencythrustprob","1e-7" },
  	{ "itemtractorbeamprob","1e-7" },
  	{ "itemautopilotprob","1e-7" },
  	{ "itememergencyshieldprob","1e-7" },
  	{ "initialfuel","2000" },
  	{ "initialtanks","0" },
  	{ "initialarmor","0" },
  	{ "initialecms","0" },
  	{ "initialmines","0" },
  	{ "initialmissiles","0" },
  	{ "initialcloaks","0" },
  	{ "initialsensors","0" },
  	{ "initialwideangles","0" },
  	{ "initialrearshots","0" },
  	{ "initialafterburners","0" },
  	{ "initialtransporters","0" },
  	{ "initialmirrors","0" },
  	{ "maxarmor","10" },
  	{ "initialdeflectors","0" },
  	{ "initialhyperjumps","0" },
  	{ "initialphasings","0" },
  	{ "initiallasers","0" },
  	{ "initialemergencythrusts","0" },
  	{ "initialtractorbeams","0" },
  	{ "initialautopilots","0" },
  	{ "initialemergencyshields","0" },
  	{ "maxfuel","10000" },
  	{ "maxtanks","8" },
  	{ "maxecms","10" },
  	{ "maxmines","10" },
  	{ "maxmissiles","10" },
  	{ "maxcloaks","10" },
  	{ "maxsensors","10" },
  	{ "maxwideangles","10" },
  	{ "maxrearshots","10" },
  	{ "maxafterburners","10" },
  	{ "maxtransporters","10" },
  	{ "maxdeflectors","10" },
  	{ "maxphasings","10" },
  	{ "maxhyperjumps","10" },
  	{ "maxemergencythrusts","10" },
  	{ "maxlasers","5" },
  	{ "maxtractorbeams","4" },
  	{ "maxautopilots","10" },
  	{ "maxemergencyshields","10" },
  	{ "maxmirrors","10" },
  	{ "gameduration","0.0" },
  	{ "allowviewing","no" },
  	{ "friction","0.0" },
  	{ "checkpointradius","6.0" },
  	{ "racelaps","3" },
  	{ "lockotherteam","yes" },
  	{ "loseitemdestroys","no" },
  	{ "usewreckage","yes" },
  	{ "maxoffensiveitems","50" },
  	{ "maxdefensiveitems","50" },
  	{ "rounddelay","0" },
  	{ "maxroundtime","0" },
  	{ "roundstoplay", "0" },
  	{ "analyticalcollisiondetection", "yes" },
  	{ "plockserver","no" },
  	{ "timerresolution", "0" },
  	{ "password", "\0" },
};

/* JLM Reorganized for new options */
int                      numprefs = 226;
prefs_t                  prefs[226] = {
{ "mapname"      ,"", "Name:"          ,255,STRING,map.mapName,0,0,0,0 },
{ "mapauthor"    ,"", "Author:"        ,255,STRING,map.mapAuthor,0,1,0,0 },
{ "mapwidth"     ,"", "Width:"     ,3,MAPWIDTH, map.width_str,0,3,0,0 },
{ "mapheight"    ,"", "Height:"    ,3,MAPHEIGHT,map.height_str,0,4,0,0 },
{ "defaultsfilename","defaults","Defaults File:",255,STRING,map.defaultsFileName,0,5,0,0},
{ "scoretablefilename","scoretable","Score File:",255,STRING,map.scoreTableFileName,0,6,0,0},
{ "extraborder"  ,"", "Extra Border?",0,YESNO,0,&map.extraBorder,7,0,0},
{ "edgewrap"     ,"", "Edge Wrap?"     ,0,YESNO   ,0,&map.edgeWrap,8,0,0},
{ "edgebounce"   ,"", "Edge Bounce?"   ,0,YESNO   ,0,&map.edgeBounce,9,0,0},
{ "gravity"      ,"", "Gravity:"       ,6,FLOAT   ,map.gravity,0,10,0,0},
{ "gravitypoint" ,"", "Gravity Point:" ,7,COORD   ,map.gravityPoint,0,11,0,0},
{ "gravityangle" ,"", "Gravity Angle:" ,3,POSINT  ,map.gravityAngle,0,12,0,0},
{ "gravitypointsource","","Point Source?",0,YESNO,0,&map.gravityPointSource,13,0,0},
{ "gravityclockwise","","Clockwise?"   ,0,YESNO,0,&map.gravityClockwise,14,0,0},
{ "gravityanticlockwise","","Anti-Clockwise?",0,YESNO,0,&map.gravityAnticlockwise,15,0,0},
{ "shotsgravity" ,"", "Shots Gravity?"  ,0,YESNO  ,0,&map.shotsGravity,16,0,0},
{ "limitedvisibility","","Limited Visi?",0,YESNO,0,&map.limitedVisibility,17,0,0},
{ "minvisibilitydistance","minvisibility","Min Visi Dist:",19,POSFLOAT,map.minVisibilityDistance,0,18,0,0},
{ "maxvisibilitydistance","maxvisibility","Max Visi Dist:",19,POSFLOAT,map.maxVisibilityDistance,0,19,0,0},
{ "shotswallbounce","","Shots Bounce?",0,YESNO,0,&map.shotsWallBounce,20,0,0},
{ "ballswallbounce","","Balls Bounce?",0,YESNO,0,&map.ballsWallBounce,21,0,0},
{ "mineswallbounce","","Mines Bounce?",0,YESNO,0,&map.minesWallBounce,22,0,0},
{ "itemswallbounce","","Items Bounce?",0,YESNO,0,&map.itemsWallBounce,23,0,0},
{ "missileswallbounce","","Missiles Bounce?",0,YESNO,0,&map.missilesWallBounce,24,0,0},
{ "sparkswallbounce","","Sparks Bounce?",0,YESNO,0,&map.sparksWallBounce,25,0,0},
{ "debriswallbounce","","Debris Bounce?",0,YESNO,0,&map.debrisWallBounce,26,0,0},
{ "playerwallbouncebrakefactor","playerwallbrake","Plyr Brake Fact:",19,POSFLOAT,map.playerWallBounceBrakeFactor,0,27,0,0},
{ "objectwallbouncebrakefactor","objectwallbrake","Obj Brake Fact:",19,POSFLOAT,map.objectWallBounceBrakeFactor,0,28,0,0},
{ "maxrobots"    ,"robots", "Max. Robots:"   ,2,POSINT  ,map.maxRobots,0,30,0,0},
{ "minrobots","","minrobots",19,FLOAT,map.minRobots,0,31,0,0},
{ "robotstalk","","Robots Talk?",0,YESNO,0,&map.robotsTalk,32,0,0},
{ "robotsleave","","Robots Leave?",0,YESNO,0,&map.robotsLeave,33,0,0},
{ "robotleavelife","","Robot Lve Life:",19,POSINT,map.robotLeaveLife,0,34,0,0},
{ "robotleavescore","","Robot Lve Score:",19,INT,map.robotLeaveScore,0,35,0,0},
{ "robotleaveratio","","Robot Lve Ratio:",19,POSFLOAT,map.robotLeaveRatio,0,36,0,0},
{ "robotfile","","Robot File:",255,STRING,map.robotFile,0,37,0,0},
{ "ecmsreprogramrobots","","EcmsReprgRbts",6,YESNO,0,&map.ecmsReprogramRobots,38,0,0},
{ "robotteam"    ,"", "Robot Team:"   ,2,POSINT  ,map.robotTeam,0,39,0,0},
{ "restrictrobots","","Restrict Robots?",0,YESNO,0,&map.restrictRobots,0,1,0},
{ "reserverobotteam","","Resrve Rob Team?",0,YESNO,0,&map.reserveRobotTeam,1,1,0},
{ "cannonsmartness","","Cannon Smrtnss",19,POSINT,map.cannonSmartness,0,2,1,0},
{ "cannonsuseitems","cannonspickupitems","CannonsUseItms",0,YESNO,0,&map.cannonsUseItems,3,1,0},
{ "cannonitemprobmult","","Can.ItemPrbMlt",19,POSFLOAT,map.cannonItemProbMult,0,4,1,0},
{ "allowshipshapes","shipshapes","Ship Shapes?",0,YESNO,0,&map.allowShipShapes,6,1,0},
{ "shipmass"     ,"", "Ship Mass:"     ,6,POSFLOAT,map.shipMass,0,7,1,0},
{ "shotmass"     ,"", "Shot Mass:"     ,6,POSFLOAT,map.shotMass,0,8,1,0},
{ "shotspeed"    ,"", "Shot Speed:"    ,6,FLOAT   ,map.shotSpeed,0,9,1,0},
{ "shotlife"     ,"", "Shot Life:"     ,3,POSINT  ,map.shotLife,0,10,1,0},
{ "firerepeatrate","firerepeat","FireRepeatRate:",19,POSINT,map.fireRepeatRate,0,11,1,0},
{ "maxplayershots","shots","Max. Shots:"    ,3,POSINT,map.maxPlayerShots,0,12,1,0},
{ "keepshots","","KeepShots",0,YESNO,0,&map.keepShots,13,1,0},
{ "maxoffensiveitems","","MaxOffenseItms:",6,POSINT,map.maxOffensiveItems,0,14,1,0},
{ "maxdefensiveitems","","MaxDefenseItms:",6,POSINT,map.maxDefensiveItems,0,15,1,0},
{ "loseitemdestroys","","Lose Item Dests?",0,YESNO,0,&map.loseItemDestroys,16,1,0},
{ "initialfuel","","Init. Fuel:",19,POSFLOAT,map.initialFuel,0,19,1,0},
{ "initialtanks","","Init. Tanks:",19,POSFLOAT,map.initialTanks,0,20,1,0},
{ "initialecms","","Init. ECMs:",19,POSFLOAT,map.initialECMs,0,21,1,0},
{ "initialmines","","Init. Mines:",19,POSFLOAT,map.initialMines,0,22,1,0},
{ "initialmissiles","","Init. Missiles:",19,POSFLOAT,map.initialMissiles,0,23,1,0},
{ "initialcloaks","","Init. Cloaks:",19,POSFLOAT,map.initialCloaks,0,24,1,0},
{ "initialsensors","","Init. Sensors:",19,POSFLOAT,map.initialSensors,0,25,1,0},
{ "initialwideangles","","Init. Wideshots:",19,POSFLOAT,map.initialWideangles,0,26,1,0},
{ "initialrearshots","","Init. Rearshots:",19,POSFLOAT,map.initialRearshots,0,27,1,0},
{ "initialafterburners","","Init. Aftrbrnrs:",19,POSFLOAT,map.initialAfterburners,0,28,1,0},
{ "initialtransporters","","Init. Trnsprtrs:",19,POSFLOAT,map.initialTransporters,0,29,1,0},
{ "initiallasers","","Init. Lasers:",19,POSFLOAT,map.initialLasers,0,30,1,0},
{ "initialemergencythrusts","","Init. EmerThrst:",19,POSFLOAT,map.initialEmergencyThrusts,0,31,1,0},
{ "initialtractorbeams","","Init. Tractors:",19,POSFLOAT,map.initialTractorBeams,0 ,32,1,0},
{ "initialautopilots","","Init. Autopilot:",19,POSFLOAT,map.initialAutopilots,0,33,1,0},
{ "initialemergencyshields","","Init. EmerShlds:",19,POSFLOAT,map.initialEmergencyShields,0,34,1,0},
{ "initialdeflectors","","Init Deflectors:",19,POSINT,map.initialDeflectors,0,35,1,0},
{ "initialhyperjumps","","Init. HyperJumps:",19,POSINT,map.initialHyperJumps,0,36,1,0},
{ "initialphasings","","Init. Phasings:",19,POSINT,map.initialPhasings,0,37,1,0},
{ "initialmirrors","","Init. Mirrors:",19,POSINT,map.initialMirrors,0,38,1,0},
{ "initialarmor","initialarmors","Init. Armor:",19,POSINT,map.initialArmor,0,39,1,0},
{ "maxfuel","","Max Fuel:",19,POSINT,map.maxFuel,0,2,2,0},
{ "maxtanks","","Max Tanks:",19,POSINT,map.maxTanks,0,3,2,0},
{ "maxecms","","Max ECMS:",19,POSINT,map.maxECMs,0,4,2,0},
{ "maxmines","","Max Mines:",19,POSINT,map.maxMines,0,5,2,0},
{ "maxmissiles","","Max Missiles:",19,POSINT,map.maxMissiles,0,6,2,0},
{ "maxcloaks","","Max Cloaks:",19,POSINT,map.maxCloaks,0,7,2,0},
{ "maxsensors","","Max Sensors:",19,POSINT,map.maxSensors,0,8,2,0},
{ "maxwideangles","","Max Wides:",19,POSINT,map.maxWideangles,0,9,2,0},
{ "maxrearshots","","Max Rearshots:",19,POSINT,map.maxRearshots,0,10,2,0},
{ "maxafterburners","","Max Afterbrnrs:",19,POSINT,map.maxAfterburners,0,11,2,0},
{ "maxtransporters","","Max Transprtrs:",19,POSINT,map.maxTransporters,0,12,2,0},
{ "maxemergencythrusts","","Max Emrg.Thrsts:",19,POSINT,map.maxEmergencyThrusts,0,13,2,0},
{ "maxlasers","","Max Lasers:",19,POSINT,map.maxLasers,0,14,2,0},
{ "maxtractorbeams","","Max TractorBms:",19,POSINT,map.maxTractorBeams,0,15,2,0},
{ "maxautopilots","","Max AutoPilots:",19,POSINT,map.maxAutopilots,0,16,2,0},
{ "maxemergencyshields","","Max Emrg.Shlds:",19,POSINT,map.maxEmergencyShields,0,17,2,0},
{ "maxdeflectors","","Max Deflectors:",19,POSINT,map.maxDeflectors,0,18,2,0},
{ "maxhyperjumps","","Max HyperJumps:",19,POSINT,map.maxDeflectors,0,19,2,0},
{ "maxphasings","","Max Phasings:",19,POSINT,map.maxPhasings,0,20,2,0},
{ "maxmirrors","","Max Mirrors:",19,POSINT,map.maxMirrors,0,21,2,0},
{ "maxarmor","maxarmors","Max Armor:",19,POSINT,map.maxArmor,0,22,2,0},
{ "allowshields","shields","Allow Shields?",0,YESNO,0,&map.allowShields,24,2,0},
{ "shieldeditempickup","shielditem","Shielded Pickup?",0,YESNO,0,&map.shieldedItemPickup,25,2,0},
{ "shieldedmining","shieldmine","Shield Mining?",0,YESNO,0,&map.shieldedMining,26,2,0},
{ "playerstartsshielded","playerstartshielded","Start Shielded?",0,YESNO,0,&map.playerStartsShielded,27,2,0},
{ "wallbouncefueldrainmult","wallbouncedrail","Fuel Drain Mult:",19,POSFLOAT,map.wallBounceFuelDrainMult,0,28,2,0},
{ "wallbouncedestroyitemprob","","BnceDestItmProb:",19,POSFLOAT,map.wallBounceDestroyItemProb,0,29,2,0},
{ "maxshieldedwallbouncespeed","maxshieldedbouncespeed","MxShldBnceSpd:",19,POSFLOAT,map.maxShieldedWallBounceSpeed,0,30,2,0},
{ "maxunshieldedwallbouncespeed","maxunshieldedbouncespeed","MxUnshBnceSpd:",19,POSFLOAT,map.maxUnshieldedWallBounceSpeed,0,31,2,0},
{ "maxshieldedplayerwallbounceangle","maxshieldedbounceangle","MxShldBnceAng:",19,POSFLOAT,map.maxShieldedPlayerWallBounceAngle,0,32,2,0},
{ "maxunshieldedplayerwallbounceangle","maxunshieldedbounceangle","MxUnshBnceAng:",19,POSFLOAT,map.maxUnshieldedPlayerWallBounceAngle,0,33,2,0},
{ "destroyitemincollisionprob","","DestItmCollProb:",19,POSFLOAT,map.destroyItemInCollisionProb,0,34,2,0},
{ "dropitemonkillprob","","DropItmKillProb:",19,POSFLOAT,map.dropItemOnKillProb,0,35,2,0},
{ "detonateitemonkillprob","","DestItmKillProb:",19,POSFLOAT,map.detonateItemOnKillProb,0,36,2,0},
{ "allownukes","nukes","Allow Nukes?",0,YESNO,0,&map.allowNukes,37,2,0},
{ "allowclusters","clusters","Allow Clusters?",0,YESNO,0,&map.allowClusters,38,2,0},
{ "nukeclusterdamage","","Nuke Clust Dam:",19,POSFLOAT,map.nukeClusterDamage,0,39,2,0},
{ "allowmodifiers","","Allow Mods?",0,YESNO,0,&map.allowModifiers,0,3,0},
{ "allowlasermodifiers","lasermodifiers","Laser Mods?",0,YESNO,0,&map.allowLaserModifiers,1,3,0},
{ "laserisstungun","stungun","Laser Stun Gun?",0,YESNO,0,&map.laserIsStunGun,2,3,0},
{ "allowplayerkilling","killings","Allow Killing?",0,YESNO,0,&map.allowPlayerKilling,3,3,0},
{ "allowplayercrashes","","Allow Crashes?",0,YESNO,0,&map.allowPlayerCrashes,4,3,0},
{ "allowplayerbounces","","Allow Bounces?",0,YESNO,0,&map.allowPlayerBounces,5,3,0},
{ "allowsmartmissiles","allowsmarts","Allow Smrts?",6,YESNO,0,&map.allowSmartMissiles,6,3,0},
{ "allowheatseekers","allowheats","Allow Heats?",6,YESNO,0,&map.allowHeatSeekers,7,3,0},
{ "allowtorpedoes","allowtorps","Allow Torps?",6,YESNO,0,&map.allowTorpedoes,8,3,0},
{ "ecmsreprogrammines","","EcmsRprgMines?",0,YESNO,0,&map.ecmsReprogramMines,9,3,0},
{ "missilesonradar","missilesradar","Miss on Radar?",0,YESNO,0,&map.missilesOnRadar,10,3,0},
{ "minesonradar","minesradar","Mines on Radar?",0,YESNO,0,&map.minesOnRadar,11,3,0},
{ "nukesonradar","nukesradar","Nukes on Radar?",0,YESNO,0,&map.nukesOnRadar,12,3,0},
{ "distinguishmissiles","","Distng Missiles?",0,YESNO,0,&map.distinguishMissiles,13,3,0},
{ "identifymines","","Identify Mines?",0,YESNO,0,&map.identifyMines,14,3,0},
{ "minefusetime","","Mine Fuse Time:",19,POSINT,map.mineFuseTime,0,15,3,0},
{ "minelife","","mine life",19,POSFLOAT,map.mineLife,0,16,3,0},
{ "maxminesperpack","","Mines/Pac",19,POSINT, map.maxMinesPerPack,0,17,3,0},
{ "baseminerange","","Base Mine Rng",19,POSINT,map.baseMineRange,0,18,3,0},
{ "missilelife","","Missile Life",19,POSFLOAT,map.missileLife,0,19,3,0},
{ "maxmissilesperpack","","Missiles/Pack:",6,INT,map.maxMissilesPerPack,0,20,3,0},
{ "nukeminsmarts","","Min Nuke Miss:",6,POSINT,map.nukeMinSmarts,0,21,3,0},
{ "nukeminmines","","Min Nuke Mines:",6,POSINT,map.nukeMinMines,0,22,3,0},
{ "movingitemprob","","Move Item Prob:",19,POSFLOAT,map.movingItemProb,0,23,3,0},
{ "maxobjectwallbouncespeed","maxobjectbouncespeed","MxObjBnceSpd:",19,POSFLOAT,map.maxObjectWallBounceSpeed,0,24,3,0},
{ "objectwallbouncelifefactor","","Obj Life Fact:",19,POSFLOAT,map.objectWallBounceLifeFactor,0,25,3,0},
{ "maxitemdensity","","MaxItemDensity:",19,POSFLOAT,map.maxItemDensity,0,26,3,0},
{ "itemconcentratorradius","itemconcentratorrange","Item Con Radius:",19,POSFLOAT,map.itemConcentratorRadius,0,27,3,0},
{ "itemconcentratorprob","","Item Con Prob:",19,POSFLOAT,map.itemConcentratorProb,0,28,3,0},
{ "itemprobmult","itemprobfact","Item Prob Mult:",19,POSFLOAT,map.itemProbMult,0,29,3,0},
{ "itemenergypackprob","","FuelPack Prob:",19,POSFLOAT,map.itemEnergyPackProb,0,30,3,0},
{ "itemtankprob","","Tank Prob:",19,POSFLOAT,map.itemTankProb,0,31,3,0},
{ "itemecmprob","","ECM Prob:",19,POSFLOAT,map.itemECMProb,0,32,3,0},
{ "itemmineprob","","Mine Prob:",19,POSFLOAT,map.itemMineProb,0,33,3,0},
{ "roguemineprob","","Rogue Mine Prob:",19,POSFLOAT,map.rogueMineProb,0,34,3,0},
{ "itemmissileprob","","Missile Prob:",19,POSFLOAT,map.itemMissileProb,0,35,3,0},
{ "rogueheatprob","","Rogue Heat Prob:",19,POSFLOAT,map.rogueHeatProb,0,36,3,0},
{ "itemcloakprob","","Cloak Prob:",19,POSFLOAT,map.itemCloakProb,0,37,3,0},
{ "itemsensorprob","","Sensor Prob:",19,POSFLOAT,map.itemSensorProb,0,38,3,0},
{ "itemwideangleprob","","Wideshot Prob:",19,POSFLOAT,map.itemWideangleProb,0,39,3,0},
{ "itemrearshotprob","","Rearshot Prob:",19,POSFLOAT,map.itemRearshotProb,0,0,4,0},
{ "itemafterburnerprob","","Aftrburner Prob:",19,POSFLOAT,map.itemAfterburnerProb,0,1,4,0},
{ "itemtransporterprob","","Trnsporter Prob:",19,POSFLOAT,map.itemTransporterProb,0,2,4,0},
{ "itemlaserprob","","Laser Prob:",19,POSFLOAT,map.itemLaserProb,0,3,4,0},
{ "itememergencythrustprob","","EmerThrst Prob:",19,POSFLOAT,map.itemEmergencyThrustProb,0,4,4,0},
{ "itemtractorbeamprob","","Tractor Prob:",19,POSFLOAT,map.itemTractorBeamProb,0,5,4,0},
{ "itemautopilotprob","","Autopilot Prob:",19,POSFLOAT,map.itemAutopilotProb,0,6,4,0},
{ "itememergencyshieldprob","","EmerShield Prob:",19,POSFLOAT,map.itemEmergencyShieldProb,0,7,4,0},
{ "itemdeflectorprob","","Deflector Prob:",19,POSFLOAT,map.itemDeflectorProb,0,8,4,0},
{ "itemhyperjumpprob","","HyperJump Prob:",19,POSFLOAT, map.itemHyperJumpProb,0,9,4,0},
{ "itemphasingprob","","Phasing Prob:",19,POSFLOAT,map.itemPhasingProb,0,10,4,0},
{ "itemmirrorprob","","ItemMirrorProb:",19,POSFLOAT,map.itemMirrorProb,0,11,4,0},
{ "itemarmorprob","","Armor Prob:",19,POSFLOAT,map.itemArmorProb,0,12,4,0},
{ "oneplayeronly","", "One Player?"    ,0,YESNO   ,0,&map.onePlayerOnly,13,4,0},
{ "limitedlives","","Limited Lives?",0,YESNO,0,&map.limitedLives,14,4,0},
{ "worldlives"   ,"lives", "Lives:"    ,3,POSINT  ,map.worldLives,0,15,4,0},
{ "playersonradar","playersradar","Plyrs on Radar?",0,YESNO,0,&map.playersOnRadar,16,4,0},
{ "teamplay"     ,"teams", "Team Play?"     ,0,YESNO   ,0,&map.teamPlay,18,4,0},
{ "teamassign","","Team Assign?",0,YESNO,0,&map.teamAssign,19,4,0},
{ "teamimmunity","","Team Immunity?",0,YESNO,0,&map.teamImmunity,20,4,0},
{ "teamcannons","","Team Cannons?",0,YESNO,0,&map.teamCannons,21,4,0},
{ "teamfuel","","Team Fuel?",0,YESNO,0,&map.teamFuel,22,4,0},
{ "reset","","World Reset?",0,YESNO,0,&map.reset,24,4,0},
{ "lockotherteam","","Lock Othr Team?",0,YESNO,0,&map.lockOtherTeam,25,4,0},
{ "targetkillteam","","Target Kill?",0,YESNO,0,&map.targetKillTeam,26,4,0},
{ "targetteamcollision","targetcollision","Target Collisn?",0,YESNO,0,&map.targetTeamCollision,27,4,0},
{ "targetsync","","Target Sync?",0,YESNO,0,&map.targetSync,28,4,0},
{ "treasurekillteam","","Tres Kill Team?",0,YESNO,0,&map.treasureKillTeam,29,4,0},
{ "treasurecollisiondestroys","treasurecollisiondestroy","Tres Col Dests?",0,YESNO,0,&map.treasureCollisionDestroys,30,4,0},
{ "treasurecollisionmaykill","treasureunshieldedcollisionkills","Tres Col Kills?",0,YESNO,0,&map.treasureCollisionMayKill,31,4,0},
{ "wreckagecollisionmaykill","wreckageunshieldedcollisionkills","Wreck Col Kills?",0,YESNO,0,&map.wreckageCollisionMayKill,32,4,0},
{ "treasuresonradar","treasuresradar","Tres on Radar?",0,YESNO,0,&map.treasuresOnRadar,33,4,0},
{ "timing"     ,"race", "Race-Timing?"      ,0,YESNO   ,0,&map.timing,34,4,0},
{ "racelaps","","Race Laps:",6,POSINT,map.raceLaps,0,35,4,0},
{ "checkpointradius","","Checkpoint Rad:",19,POSFLOAT,map.checkpointRadius,0,36,4,0},
{ "friction","","Friction:",19,POSFLOAT,map.friction,0,37,4,0},
{ "framespersecond","FPS","Frames/Second:",6,POSINT,map.framesPerSecond,0,38,4,0},
{ "timerresolution","","TimerResltn:",6,POSINT,map.timerResolution,0,39,4,0},
{ "ignore20maxfps","","Ignore FPS20",0,YESNO,0,&map.ignore20MaxFPS,0,5,0},
{ "reporttometaserver","reportmeta","Report to Meta?",0,YESNO,0,&map.reportToMetaServer,1,5,0},
{ "idlerun","rawmode","Idle Run?",0,YESNO,0,&map.idleRun,2,5,0},
{ "noquit","","No Quit?",0,YESNO,0,&map.noQuit,3,5,0},
{ "resetonhuman","humanreset","ResetOnHuman?",0,YESNO,0,&map.resetOnHuman,4,5,0},
{ "gameduration","","Game Duration:",6,POSINT,map.gameDuration,0,5,5,0},
{ "rounddelay","","Round Delay:",19,POSINT,map.roundDelay,0,6,5,0},
{ "maxroundtime","","MaxRoundTime:",19,POSINT,map.maxRoundTime,0,7,5,0},
{ "roundstoplay","","Rounds2Play:",19,POSINT,map.roundsToPlay,0,8,5,0},
{ "contactport","port","Contact Port:",19,INT,map.contactPort,0,9,5,0},
{ "denyhosts","","Deny Hosts:",255,STRING,map.denyHosts,0,10,5,0},
{ "plockserver","","pLockServer?",0,YESNO,0,&map.pLockServer,11,5,0},
{ "password","","Password:",255,STRING,map.password,0,12,5,0},
{ "allowviewing","","Allow Viewing?",0,YESNO,0,&map.allowViewing,13,5,0},
{ "analyticalcollisiondetection","","Analic.Col.Det",0,YESNO,0,&map.analyticalCollisionDetection,14,5,0},
{ "cloakedexhaust","","Cloaked Exhaust?",6,YESNO,0,&map.cloakedExhaust,15,5,0},
{ "cloakedshield","","Cloaked Shield?",6,YESNO,0,&map.cloakedShield,16,5,0},
{ "itemconcentratorvisible","","Item Conc Vis?",6,YESNO,0,&map.itemConcentratorVisible,17,5,0},
{ "gravityvisible","","Gravity Visible?",6,YESNO,0,&map.gravityVisible,18,5,0},
{ "wormholevisible","","Wormhole Visible?",6,YESNO,0,&map.wormholeVisible,19,5,0},
{ "wormtime","","Wormhole Time:",19,POSINT,map.wormTime,0,20,5,0},
{ "shotkillscoremult","","ShotKillScrMlt:",19,POSFLOAT,map.shotKillScoreMult,0,22,5,0},
{ "torpedokillscoremult","","TorpKillScrMlt:",19,POSFLOAT,map.torpedoKillScoreMult,0,23,5,0},
{ "smartkillscoremult","","SmrtKillScrMlt:",19,POSFLOAT,map.smartKillScoreMult,0,24,5,0},
{ "heatkillscoremult","","HeatKillScrMlt:",19,POSFLOAT,map.heatKillScoreMult,0,25,5,0},
{ "clusterkillscoremult","","ClstrKillScrMlt:",19,POSFLOAT,map.clusterKillScoreMult,0,26,5,0},
{ "laserkillscoremult","","LasrKillScrMlt:",19,POSFLOAT,map.laserKillScoreMult,0,27,5,0},
{ "tankkillscoremult","","TankKillScrMlt:",19,POSFLOAT,map.tankKillScoreMult,0,28,5,0},
{ "runoverkillscoremult","","RnvrKillScrMlt:",19,POSFLOAT,map.runoverKillScoreMult,0,29,5,0},
{ "ballkillscoremult","","BallKillScrMlt:",19,POSFLOAT,map.ballKillScoreMult,0,30,5,0},
{ "explosionkillscoremult","","ExplKillScrMlt:",19,POSFLOAT,map.explosionKillScoreMult,0,31,5,0},
{ "shovekillscoremult","","ShveKillScrMlt:",19,POSFLOAT,map.shoveKillScoreMult,0,32,5,0},
{ "crashscoremult","","CrashScrMlt:",19,POSFLOAT,map.crashScoreMult,0,33,5,0},
{ "minescoremult","","MineScrMlt:",19,POSFLOAT,map.mineScoreMult,0,34,5,0},
{ "usewreckage","","Use Wreckage?",6,YESNO,0,&map.useWreckage,36,5,0},
{ "mapdata","",NULL,0,MAPDATA,NULL,0,0,0,0 }
};

/***************************************************************************/
/* int main                                                                */
/* Arguments :                                                             */
/*   argc                                                                  */
/*   argv                                                                  */
/* Purpose :                                                               */
/***************************************************************************/

int main(int argc, char *argv[])
{
   SetDefaults(argc,argv);

   Setup_default_server_options();
   
   T_ConnectToServer(display_name);

   LoadMap(map.mapFileName);

   SizeMapwin();

   T_SetToolkitFont( fontname );

   T_GetGC(&White_GC, "white");
   T_GetGC(&Black_GC, "black");
   T_GetGC(&xorgc, "black");
   XSetFunction(display, xorgc, GXxor);
#ifndef MONO
   T_GetGC(&Wall_GC,      COLOR_WALL      );
   T_GetGC(&Decor_GC,     COLOR_DECOR     );
   T_GetGC(&Fuel_GC,      COLOR_FUEL      );
   T_GetGC(&Treasure_GC,  COLOR_TREASURE  );
   T_GetGC(&Target_GC,    COLOR_TARGET    );
   T_GetGC(&Item_Conc_GC, COLOR_ITEM_CONC );
   T_GetGC(&Gravity_GC,   COLOR_GRAVITY   );
   T_GetGC(&Current_GC,   COLOR_CURRENT   );
   T_GetGC(&Wormhole_GC,  COLOR_WORMHOLE  );
   T_GetGC(&Base_GC,      COLOR_BASE      );
   T_GetGC(&Cannon_GC,    COLOR_CANNON    );
#endif
      
   mapwin = T_MakeWindow(50, (int)((root_height-mapwin_height)/2),
        mapwin_width, mapwin_height, "white","black");
   T_SetWindowName(mapwin, "XMapEdit", "XMapEdit");
   XSelectInput( display, mapwin, ExposureMask | ButtonPressMask |
        KeyPressMask | StructureNotifyMask | ButtonReleaseMask |
        PointerMotionMask );
   BuildMapwinForm();

   smlmap_pixmap = XCreatePixmap(display,mapwin,TOOLSWIDTH,TOOLSWIDTH,
        DefaultDepth(display,screennum)); 

   prefwin = T_PopupCreate(PREF_X, PREF_Y, PREF_WIDTH, PREF_HEIGHT,
        "Preferences");
   BuildPrefsForm();

   helpwin = T_PopupCreate(HELP_X, HELP_Y,HELP_WIDTH,HELP_HEIGHT,"Help");

   ResetMap();

   XMapWindow(display, mapwin);

   MainEventLoop();

    return 0;
}




/* RTT allow setting of server options from the default_settings array above */
void   Setup_default_server_options()
{
  int i;
  
  for(i=0;i<num_default_settings;i++)
    AddOption(default_settings[i].name, default_settings[i].value);
  return;
}

    


/***************************************************************************/
/* SetDefaults                                                             */
/* Arguments :                                                             */
/*   argc                                                                  */
/*   argv                                                                  */
/* Purpose :                                                               */
/***************************************************************************/
void SetDefaults(int argc, char *argv[])
{
   int                   i,j;

   /* Set "hardcoded" defaults from defaults file */
   progname = argv[0];
   drawicon = 3;
   drawmode = 1;
   geometry_width = 800;
   geometry_height = 600;

   if (map.comments)
      free(map.comments);
   map.comments = (char *) NULL;
   map.mapName[0] = map.mapFileName[0] = (char ) NULL;
   /*   strcpy(map.author,"Captain America (mbcaprt@mphhpd.ph.man.ac.uk)\0");*/
   map.width = DEFAULT_WIDTH;
   sprintf(map.width_str,"%d",map.width);
   map.height = DEFAULT_HEIGHT;
   sprintf(map.height_str,"%d",map.height);
   for ( i=0; i<MAX_MAP_SIZE; i++)
      for ( j=0; j<MAX_MAP_SIZE; j++) {
         map.data[i][j] = ' ';
         clipdata[i][j] = ' ';
      }
   map.view_x = map.view_y = 0;
   map.view_zoom = DEFAULT_MAP_ZOOM;
   map.changed = 0;
   ParseArgs(argc, argv);
}

/***************************************************************************/
/* ParseArgs                                                               */
/* Arguments :                                                             */
/*   argc                                                                  */
/*   argv                                                                  */
/* Purpose :                                                               */
/***************************************************************************/
void ParseArgs(int argc, char *argv[])
{
   static char           *options[] = {
      "-map",
      "-zoom",
      "-font",
      "-display"
   };
   int                   NUMOPTIONS = 4;
   int                   index, option;

   for (index = 1; index < argc; index++) {
      for (option = 0; option < NUMOPTIONS; option++)
      if(!strcmp(argv[index], options[option]))
      break;

      switch(option) {

         case 0:
            strcpy(map.mapFileName, argv[++index] );
            break;

         case 1:
            map.view_zoom = atoi(argv[++index]);
            break;

         case 2:
            free(fontname);
            fontname = (char *) malloc(strlen(argv[++index])+1);
            strcpy(fontname,argv[index]);
            break;

         case 3:
            free(display_name);
            display_name = malloc(strlen(argv[++index])+1);
            strcpy(display_name,argv[index]);
            break;

      }
   }
}

/***************************************************************************/
/* ResetMap                                                                */
/* Arguments :                                                             */
/* Purpose :                                                               */
/***************************************************************************/
void ResetMap(void)
{
   map.view_x = map.view_y = 0;
   while ( ((mapwin_width - TOOLSWIDTH) > (map.width*map.view_zoom)) ||
        (mapwin_height > (map.height*map.view_zoom)) )
   map.view_zoom++;
   T_SetWindowSizeLimits(mapwin,TOOLSWIDTH+50,TOOLSHEIGHT,
        TOOLSWIDTH+map.width*map.view_zoom,map.height*map.view_zoom,0,0);
   SizeSmallMap();
   DrawTools();
   XFillRectangle(display,mapwin,Black_GC,TOOLSWIDTH,0,
        mapwin_width-TOOLSWIDTH, mapwin_height);
   DrawMap(TOOLSWIDTH,0,mapwin_width-TOOLSWIDTH,mapwin_height);
   T_FormScrollArea(mapwin,"draw_map_icon",T_SCROLL_UNBOUND,TOOLSWIDTH,0,
        mapwin_width-TOOLSWIDTH, mapwin_height,DrawMapIcon);
}

/***************************************************************************/
/* SizeMapwin                                                              */
/* Arguments :                                                             */
/* Purpose :                                                               */
/***************************************************************************/
void SizeMapwin(void)
{
   /* try for geometry settings */
   mapwin_width = geometry_width;
   mapwin_height = geometry_height;
   /* increase to size of tools if it's too small */
   if ( mapwin_width < (TOOLSWIDTH + map.view_zoom) )
   mapwin_width = TOOLSWIDTH + map.view_zoom;
   if ( mapwin_height < TOOLSHEIGHT )
   mapwin_height = TOOLSHEIGHT;
   /* if it's too big for map, zoom in*/
   while ( ((mapwin_width - TOOLSWIDTH) > (map.width*map.view_zoom)) ||
        (mapwin_height > (map.height*map.view_zoom)) )
   map.view_zoom++;
}

/***************************************************************************/
/* SizeSmallMap                                                            */
/* Arguments :                                                             */
/* Purpose :                                                               */
/***************************************************************************/
void SizeSmallMap(void)
{
   smlmap_width = smlmap_height = TOOLSWIDTH - 20;
   smlmap_xscale = (float)(map.width)/(float)(smlmap_width);
   smlmap_yscale = (float)(map.height)/(float)(smlmap_height);
   if ( map.width > map.height ) {
      smlmap_height = (int)(map.height/smlmap_xscale);
      smlmap_yscale = (float)(map.height)/(float)(smlmap_height);
      } else if ( map.height > map.width ) {
      smlmap_width = (int)(map.width/smlmap_yscale);
      smlmap_xscale = (float)(map.width)/(float)(smlmap_width);
   }
   smlmap_x = TOOLSWIDTH/2 - smlmap_width/2;
   smlmap_y = TOOLSWIDTH/2 - smlmap_height/2;
   T_FormScrollArea(mapwin,"move_view",T_SCROLL_UNBOUND,smlmap_x,
        smlmap_y+TOOLSHEIGHT-TOOLSWIDTH,smlmap_width, smlmap_height,
        MoveMapView);
}

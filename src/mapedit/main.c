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
 * $Id: main.c,v 1.4 1998/10/06 14:52:19 bert Exp $
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
map_t                    map;

/* RTT allow defaults on startup*/

int                      num_default_settings=143;
charlie                  default_settings[143] = {
  { "playerstartsshielded","yes" },
  { "shotswallbounce","no" },
  { "ballswallbounce","yes" },
  { "mineswallbounce","no" },
  { "itemswallbounce","yes" },
  { "missileswallbounce","no" },
  { "sparkswallbounce","no" },
  { "debriswallbounce","no" },
  { "limitedlives","yes" },
  { "mapwidth","100" },
  { "mapheight","100" },
  { "mapname","NewMap" },
  { "mapauthor","Your Name" },
  { "edgewrap","yes" },
  { "edgebounce","yes" },
  { "teamplay","no" },
  { "oneplayeronly","no" },
  { "timing","no" },
  { "maxrobots","6" },
  { "worldlives","3" },
  { "limitedvisibility","no" },
  { "shipmass","20" },
  { "gravityangle","90" },
  { "gravitypoint","0,0" },
  { "gravitypointsource","no" },
  { "gravityclockwise","no" },
  { "gravityanticlockwise","no" },
  { "shotmass","0.1" },
  { "shotspeed","21" },
  { "shotlife","60" },
  { "maxplayershots","30" },
  { "shotsgravity","yes" },
  { "firerepeatrate","2" },
  { "robotstalk","no" },
  { "robotsleave","yes" },
  { "robotleavelife","50" },
  { "robotleavescore","-75" },
  { "robotleaveratio","-5" },
  { "idlerun","no" },
  { "noquit","yes" },
  { "contactport","15345" },
  { "allowplayercrashes","no" },
  { "allowplayerbounces","yes" },
  { "allowplayerkilling","yes" },
  { "allowshields","no" },
  { "maxobjectwallbouncespeed","40" },
  { "maxshieldedwallbouncespeed","800" },
  { "maxunshieldedwallbouncespeed","800" },
  { "maxshieldedplayerwallbounceangle","180" },
  { "maxunshieldedplayerwallbounceangle","180" },
  { "playerwallbouncebrakefactor","0.6" },
  { "objectwallbouncebrakefactor","0.95" },
  { "objectwallbouncelifefactor","1" },
  { "wallbouncefueldrainmult","0.2" },
  { "wallbouncedestroyitemprob","0" },
  { "reporttometaserver","yes" },
  { "denyhosts","no" },
  { "minvisibilitydistance","35000" },
  { "maxvisibilitydistance","346465" },
  { "limitedlives","no" },
  { "reset","yes" },
  { "teamassign","yes" },
  { "teamimmunity","yes" },
  { "ecmsreprogrammines","yes" },
  { "targetkillteam","yes" },
  { "targetteamcollision","no" },
  { "targetsync","no" },
  { "treasurekillteam","no" },
  { "treasurecollisiondestroys","yes" },
  { "treasurecollisionmaykill","no" },
  { "friction","0" },
  { "extraborder","no" },
  { "framespersecond","14" },
  { "allownukes","yes" },
  { "allowclusters","yes" },
  { "allowmodifiers","yes" },
  { "allowlasermodifiers","yes" },
  { "allowshipshapes","yes" },
  { "playersonradar","yes" },
  { "missilesonradar","yes" },
  { "minesonradar","yes" },
  { "nukesonradar","yes" },
  { "treasuresonradar","no" },
  { "distinguishmissiles","yes" },
  { "maxmissilesperpack","2" },
  { "identifymines","yes" },
  { "shieldeditempickup","no" },
  { "shieldedmining","yes" },
  { "laserisstungun","no" },
  { "nukeminsmarts","3" },
  { "nukeminmines","3" },
  { "nukeclusterdamage","1" },
  { "minefusetime","13720000" },
  { "movingitemprob","0.3" },
  { "dropitemonkillprob","0.8" },
  { "detonateitemonkillprob","0" },
  { "destroyitemincollisionprob","0" },
  { "itemprobmult","0.0004" },
  { "maxitemdensity","0.03" },
  { "itemconcentratorradius","10" },
  { "rogueheatprob","0.01" },
  { "roguemineprob","0.02" },
  { "itemenergypackprob","1" },
  { "itemtankprob","1" },
  { "itemecmprob","4" },
  { "itemmineprob","1" },
  { "itemmissileprob","1" },
  { "itemcloakprob","1" },
  { "itemsensorprob","1" },
  { "itemwideangleprob","1" },
  { "itemrearshotprob","1" },
  { "itemafterburnerprob","1" },
  { "itemtransporterprob","1" },
  { "itemlaserprob","1" },
  { "itememergencythrustprob","0.2" },
  { "itemtractorbeamprob","0.7" },
  { "itemautopilotprob","1" },
  { "itememergencyshieldprob","1" },
  { "initialfuel","2000" },
  { "initialtanks","0" },
  { "initialecms","0" },
  { "initialmines","0" },
  { "initialmissiles","0" },
  { "initialcloaks","0" },
  { "initialsensors","0" },
  { "initialwideangles","0" },
  { "initialrearshots","0" },
  { "initialafterburners","3" },
  { "initialtransporters","0" },
  { "initiallasers","0" },
  { "initialemergencythrusts","0" },
  { "initialtractorbeams","0" },
  { "initialautopilots","1" },
  { "initialemergencyshields","0" },
  { "gameduration","0" },
  { "checkpointradius","6" },
  { "racelaps","3" },
  { "lockotherteam","yes" },
  { "loseitemdestroys","no" },
  { "maxoffensiveitems","20" },
  { "maxdefensiveitems","20" },
  { "gravity","-0.14" },
  { "plockserver","no" },
};

int                      numprefs = 151;
prefs_t                  prefs[151] = {
   { "mapwidth"     , "Map Width:"     ,3,MAPWIDTH, map.width_str,0,0,0,0 },
   { "mapheight"    , "Map Height:"    ,3,MAPHEIGHT,map.height_str,0,1,0,0 },
   { "mapname"      , "Name:"          ,255,STRING,map.name,0,2,0,0 },
   { "mapauthor"    , "Author:"        ,255,STRING,map.author,0,3,0,0 },
   { "edgebounce"   , "Edge Bounce:"   ,0,YESNO   ,0,&map.edgebounce,5,0,0},
   { "teamplay"     , "Team Play:"     ,0,YESNO   ,0,&map.teamplay,6,0,0},
   { "oneplayeronly", "One Player:"    ,0,YESNO   ,0,&map.oneplay,7,0,0},
   { "timing"       , "Timing:"        ,0,YESNO   ,0,&map.timing,8,0,0},
   { "maxrobots"    , "Max. Robots:"   ,2,POSINT  ,map.maxrobots,0, 9,0,0},
   { "worldlives"   , "Max. Lives:"    ,3,POSINT  ,map.maxlives,0, 10,0,0},
   { "limitedvisibility","Limit Visibility:",0,YESNO,0,&map.visibility,11,0,0},
   { "playershielding","Allow Shields:",0,YESNO   ,0,&map.shielding,12,0,0},
   { "shipmass"     , "Ship Mass:"     ,6,POSFLOAT,map.shipmass,0, 13,0,0},
   { "gravity"      , "Gravity:"       ,6,FLOAT   ,map.gravity,0, 14,0,0},
   { "gravityangle" , "Gravity Angle:" ,3,POSINT  ,map.gravityangle,0,15,0,0},
   { "gravitypoint" , "Gravity Point:" ,7,COORD   ,map.gravitypoint,0,16,0,0},
   { "gravitypointsource","Point Source:",0,YESNO,0,&map.ptsource,17,0,0},
   { "gravityclockwise","Clockwise:"   ,0,YESNO,0,&map.clockwise,18,0,0},
   { "gravityanticlockwise","Anti-Clockwise:",0,YESNO,0,&map.anitclock,19,0,0},
   { "shotmass"     , "Shot Mass:"     ,6,POSFLOAT,map.shotmass,0, 20,0,0},
   { "shotspeed"    , "Shot Speed:"    ,6,FLOAT   ,map.shotspeed,0, 21,0,0},
   { "shotlife"     , "Shot Life:"     ,3,POSINT  ,map.shotlife,0, 22,0,0},
   { "losemass"     , "Loose Mass:"    ,0,YESNO   ,0,&map.losemass, 23,0,0},
   { "maxplayershots","Max. Shots:"    ,3,POSINT,map.maxplayershots,0,24,0,0},
   { "shotsgravity" , "Shot Gravity:"  ,0,YESNO  ,0,&map.shotsgravity,25,0,0},
   { "crashwithplayer","Player Crashes:",0,YESNO,0,&map.playercrashes,26,0,0},
   { "playerkillings","Player Kills:"  ,0,YESNO ,0,&map.playerkills,27,0,0},
   { "firerepeatrate",	      			  "FireRepeatRate:",19,POSINT,			 map.fireRepeatRate,0	,28,0,0},
   { "robotfile",   	      			  "Robot File:",255,STRING,			 map.robotFile,0	,29,0,0},
   { "robotstalk",  	      			  "Robots Talk:",0,YESNO,				0,&map.robotsTalk	,30,0,0},
   { "robotsleave", 	      			  "Robots Leave:",0,YESNO,			0,&map.robotsLeave	,31,0,0},
   { "robotleavelife",	      			  "Robot LeaveLife:",19,POSINT,			 map.robotLeaveLife,0	,32,0,0},
   { "robotleavescore",	      			  "RobotLeaveScore",19,INT,			 map.robotLeaveScore,0	,33,0,0},
   { "robotleaveratio",	      			  "RobotLeaveRatio",19,POSFLOAT,			 map.robotLeaveRatio,0	,34,0,0},
   { "idlerun",	    	      			  "Idle Run",0,YESNO,				0,&map.idleRun	,35,0,0},
   { "noquit",	    	      			  "No Quit",0,YESNO,				0,&map.noQuit	,36,0,0},
   { "mapfilename", 	      			  "Map File Name",255,STRING,			 map.mapFileName,0	,37,0,0},
   { "contactport", 	      			  "Contact Port",19,INT,				 map.contactPort,0	,38,0,0},
   { "allowplayercrashes",    			  "AllowPl.Crashes",0,YESNO,			0,&map.allowPlayerCrashes ,39,0,0},
   { "allowplayerbounces",    			  "AllowPlBounces",0,YESNO,			0,&map.allowPlayerBounces ,0,1,0},
   { "allowplayerkilling",    			  "AllowPlKilling",0,YESNO,			0,&map.allowPlayerKilling ,1,1,0},
   { "allowshields",	      			  "AllowShields",0,YESNO,			0,&map.allowShields	,2,1,0},
   { "playerstartsshielded",  			  "P.StartsShielded",0,YESNO,		0,&map.playerStartsShielded ,3,1,0},
   { "shotswallbounce",	      			  "ShotsWallBnce",0,YESNO,			0,&map.shotsWallBounce	,4,1,0},
   { "ballswallbounce",	      			  "BallsWallBnce",0,YESNO,			0,&map.ballsWallBounce	,5,1,0},
   { "mineswallbounce",	      			  "MinesWallBnce",0,YESNO,			0,&map.minesWallBounce	,6,1,0},
   { "itemswallbounce",	      			  "ItemsWallBnce",0,YESNO,			0,&map.itemsWallBounce	,7,1,0},
   { "missileswallbounce",    			  "MissilesWallBnce",0,YESNO,			0,&map.missilesWallBounce ,8,1,0},
   { "sparkswallbounce",      			  "SparksWallBnce",0,YESNO,			0,&map.sparksWallBounce	,9,1,0},
   { "debriswallbounce",      			  "DebrisWallBnce",0,YESNO,			0,&map.debrisWallBounce	,10,1,0},
   { "maxobjectwallbouncespeed",		  "MxobjWBnceSpd",19,POSFLOAT,	 map.maxObjectWallBounceSpeed,0 ,11,1,0},
   { "maxshieldedwallbouncespeed",		  "MaxShldWBSpd",19,POSFLOAT,	 map.maxShieldedWallBounceSpeed,0 ,12,1,0},
   { "maxunshieldedwallbouncespeed",		  "MaxUhldWBSpd",19,POSFLOAT,	 map.maxUnshieldedWallBounceSpeed,0 ,13,1,0},
   { "maxshieldedplayerwallbounceangle",     "MxShldPWBAng",19,POSFLOAT,	 map.maxShieldedPlayerWallBounceAngle,0 ,14,1,0},
   { "maxunshieldedplayerwallbounceangle",   "MxUhldPWBAng",19,POSFLOAT,	 map.maxUnshieldedPlayerWallBounceAngle,0 ,15,1,0},
   { "playerwallbouncebrakefactor",		  "PlWBBrkFactor",19,POSFLOAT,	 map.playerWallBounceBrakeFactor,0 ,16,1,0},
   { "objectwallbouncebrakefactor",		  "ObWBBrkFactor",19,POSFLOAT,	 map.objectWallBounceBrakeFactor,0 ,17,1,0},
   { "objectwallbouncelifefactor",		  "ObWBLifeFactor",19,POSFLOAT,	 map.objectWallBounceLifeFactor,0 ,18,1,0},
   { "wallbouncefueldrainmult",			  "WBFuelDrnMult",19,POSFLOAT,	 map.wallBounceFuelDrainMult,0 ,19,1,0},
   { "wallbouncedestroyitemprob",		  "WBDestItProb",19,POSFLOAT,	 map.wallBounceDestroyItemProb,0 ,20,1,0},
   { "reporttometaserver",    			  "Rprt2MtaSrvr",0,YESNO,			0,&map.reportToMetaServer ,21,1,0},
   { "denyhosts",   	         		  "DenyHosts",0,YESNO,				0,&map.denyHosts	,22,1,0},
   { "minvisibilitydistance", 			  "Min.Visi.Dist",19,POSFLOAT,		 map.minVisibilityDistance,0 ,23,1,0},
   { "maxvisibilitydistance", 			  "Max.Visi.Dist",19,POSFLOAT,		 map.maxVisibilityDistance,0 ,24,1,0},
   { "limitedlives",	      			  "Limited Lives",0,YESNO,			0,&map.limitedLives	,25,1,0},
   { "reset",	    	      			  "Reset",0,YESNO,				0,&map.reset	,26,1,0},
   { "teamassign",  	      			  "TeamAssign",0,YESNO,				0,&map.teamAssign	,27,1,0},
   { "teamimmunity",	      			  "TeamImmune",0,YESNO,			0,&map.teamImmunity	,28,1,0},
   { "ecmsreprogrammines",    			  "EcmsReprMine",0,YESNO,			0,&map.ecmsReprogramMines ,29,1,0},
   { "targetkillteam",	      			  "Target KillT",0,YESNO,			0,&map.targetKillTeam	,30,1,0},
   { "targetteamcollision",   			  "Target TeamColsn",0,YESNO,		0,&map.targetTeamCollision ,31,1,0},
   { "targetsync",  	           		  "Target Sync",0,YESNO,				0,&map.targetSync	,32,1,0},
   { "treasurekillteam",      			  "Tres.KillTeam",0,YESNO,			0,&map.treasureKillTeam	,33,1,0},
   { "treasurecollisiondestroys",		  "Tres.CollDests",0,YESNO,		0,&map.treasureCollisionDestroys ,34,1,0},
   { "treasurecollisionmaykill",		  "Tres.CollKills",0,YESNO,		0,&map.treasureCollisionMayKill ,35,1,0},
   { "friction",    	      			  "Friction",19,POSFLOAT,			 map.friction,0	,36,1,0},
   { "edgewrap",    	      			  "Edge Wrap",0,YESNO,				0,&map.edgeWrap	,37,1,0},
   { "extraborder", 	      			  "Extra Border",0,YESNO,			0,&map.extraBorder	,38,1,0},
   { "defaultsfilename",      			  "Defs.File Name",255,STRING,		 map.defaultsFileName,0	,39,1,0},
   { "scoretablefilename",    			  "ScrsFileName",255,STRING,		 map.scoreTableFileName,0 ,0,2,0},
   { "framespersecond",	      			  "FPS",6,POSINT,			 map.framesPerSecond,0	,1,2,0},
   { "allownukes",  	      			  "Allow Nukes",0,YESNO,				0,&map.allowNukes	,2,2,0},
   { "allowclusters",	      			  "Allow Clusters",0,YESNO,			0,&map.allowClusters	,3,2,0},
   { "allowmodifiers",	      			  "Allow Mods",0,YESNO,			0,&map.allowModifiers	,4,2,0},
   { "allowlasermodifiers",   			  "Allow Lasermods",0,YESNO,		0,&map.allowLaserModifiers ,5,2,0},
   { "allowshipshapes",	      			  "Allow ShpShapes",0,YESNO,			0,&map.allowShipShapes	,6,2,0},
   { "playersonradar",	      			  "Players onRadar",0,YESNO,			0,&map.playersOnRadar	,7,2,0},
   { "missilesonradar",	      			  "Missiles on radar",0,YESNO,			0,&map.missilesOnRadar	,8,2,0},
   { "minesonradar",	      			  "Mines on radar",0,YESNO,			0,&map.minesOnRadar	,9,2,0},
   { "nukesonradar",	      			  "Nukes on radar",0,YESNO,			0,&map.nukesOnRadar	,10,2,0},
   { "treasuresonradar",      			  "TresOnRadar",0,YESNO,			0,&map.treasuresOnRadar	,11,2,0},
   { "distinguishmissiles",   			  "DstngsMis's",0,YESNO,		0,&map.distinguishMissiles ,12,2,0},
   { "maxmissilesperpack",    			  "MaxMis'sPPack",6,INT,			 map.maxMissilesPerPack,0 ,13,2,0},
   { "identifymines",	      			  "IdentifyMines",0,YESNO,			0,&map.identifyMines	,14,2,0},
   { "shieldeditempickup",    			  "ShieldPickup",0,YESNO,			0,&map.shieldedItemPickup ,15,2,0},
   { "shieldedmining",	      			  "ShieldMining",0,YESNO,			0,&map.shieldedMining	,16,2,0},
   { "laserisstungun",	      			  "LaserIsStun",0,YESNO,			0,&map.laserIsStunGun	,17,2,0},
   { "nukeminsmarts",	      			  "Nuke min smarts",6,POSINT,			 map.nukeMinSmarts,0	,18,2,0},
   { "nukeminmines",	      			  "Nuke min mines",6,POSINT,			 map.nukeMinMines,0	,19,2,0},
   { "nukeclusterdamage",     			  "NuClustDamage",19,POSFLOAT,		 map.nukeClusterDamage,0 ,20,2,0},
   { "minefusetime",	      			  "MineFuseTime",19,POSINT,			 map.mineFuseTime,0	,21,2,0},
   { "movingitemprob",	      			  "MovingItemProb",19,POSFLOAT,			 map.movingItemProb,0	,22,2,0},
   { "dropitemonkillprob",    			  "DrpItOnKilProb",19,POSFLOAT,		 map.dropItemOnKillProb,0 ,23,2,0},
   { "detonateitemonkillprob",			  "DtnteItemOKP",19,POSFLOAT,		 map.detonateItemOnKillProb,0 ,24,2,0},
   { "destroyitemincollisionprob",	  	  "DestItemInClsnP",19,POSFLOAT,	 map.destroyItemInCollisionProb,0 ,25,2,0},
   { "itemprobmult",	      			  "Item Prob Mult",19,POSFLOAT,			 map.itemProbMult,0	,26,2,0},
   { "maxitemdensity",	      			  "MaxItemDensity",19,POSFLOAT,			 map.maxItemDensity,0	,27,2,0},
   { "itemconcentratorradius",			  "ItemConcRad",19,POSFLOAT,		 map.itemConcentratorRadius,0 ,28,2,0},
   { "rogueheatprob",	      			  "RogueHeatProb",19,POSFLOAT,			 map.rogueHeatProb,0	,29,2,0},
   { "roguemineprob",	      			  "RogueMineProb",19,POSFLOAT,			 map.rogueMineProb,0	,30,2,0},
   { "itemenergypackprob",    			  "EnergyPackPr",19,POSFLOAT,		 map.itemEnergyPackProb,0 ,31,2,0},
   { "itemtankprob",	      			  "Tank Prob",19,POSFLOAT,			 map.itemTankProb,0	,32,2,0},
   { "itemecmprob", 	      			  "ECM Prob",19,POSFLOAT,			 map.itemECMProb,0	,33,2,0},
   { "itemmineprob",	      			  "Mine Prob",19,POSFLOAT,			 map.itemMineProb,0	,34,2,0},
   { "itemmissileprob",	      			  "Missile Prob",19,POSFLOAT,			 map.itemMissileProb,0	,35,2,0},
   { "itemcloakprob",	      			  "Cloak Prob",19,POSFLOAT,			 map.itemCloakProb,0	,36,2,0},
   { "itemsensorprob",	      			  "Sensor Prob",19,POSFLOAT,			 map.itemSensorProb,0	,37,2,0},
   { "itemwideangleprob",     			  "Wideangle Prob",19,POSFLOAT,		 map.itemWideangleProb,0 ,38,2,0},
   { "itemrearshotprob",      			  "Rearshot Prob",19,POSFLOAT,		 map.itemRearshotProb,0	,39,2,0},
   { "itemafterburnerprob",   			  "Aftrburner Prob",19,POSFLOAT,		 map.itemAfterburnerProb,0 ,0,3,0},
   { "itemtransporterprob",   			  "Trnsporter Prob",19,POSFLOAT,		 map.itemTransporterProb,0 ,1,3,0},
   { "itemlaserprob",	      			  "Laser Prob",19,POSFLOAT,			 map.itemLaserProb,0	,2,3,0},
   { "itememergencythrustprob",			  "EmrgnyThrstPr",19,POSFLOAT,		 map.itemEmergencyThrustProb,0 ,3,3,0},
   { "itemtractorbeamprob",   			  "TractorBeamPr",19,POSFLOAT,		 map.itemTractorBeamProb,0 ,4,3,0},
   { "itemautopilotprob",     			  "Auto pilot Prob",19,POSFLOAT,		 map.itemAutopilotProb,0 ,5,3,0},
   { "itememergencyshieldprob",			  "EmrgnyShdProb",19,POSFLOAT,		 map.itemEmergencyShieldProb,0 ,6,3,0},
   { "initialfuel", 	      			  "initial Fuel",19,POSFLOAT,			 map.initialFuel,0	,7,3,0},
   { "initialtanks",	      			  "initial Tanks",19,POSFLOAT,			 map.initialTanks,0	,8,3,0},
   { "initialecms", 	      			  "initial ECMs",19,POSFLOAT,			 map.initialECMs,0	,9,3,0},
   { "initialmines",  	      			  "initial mines",19,POSFLOAT,			 map.itialMines,0	,10,3,0},
   { "initialmissiles",	      			  "initial Missiles",19,POSFLOAT,			 map.initialMissiles,0	,11,3,0},
   { "initialcloaks",	      			  "initial Cloaks",19,POSFLOAT,			 map.initialCloaks,0	,12,3,0},
   { "initialsensors",	      			  "initial Sensors",19,POSFLOAT,			 map.initialSensors,0	,13,3,0},
   { "initialwideangles",     			  "initial Wideangs",19,POSFLOAT,		 map.initialWideangles,0 ,14,3,0},
   { "initialrearshots",      			  "initial Rearshots",19,POSFLOAT,		 map.initialRearshots,0	,15,3,0},
   { "initialafterburners",   			  "initial Afterbnrs",19,POSFLOAT,		 map.initialAfterburners,0 ,16,3,0},
   { "initialtransporters",   			  "initial Transprts",19,POSFLOAT,		 map.initialTransporters,0 ,17,3,0},
   { "initiallasers",	      			  "initial Lasers",19,POSFLOAT,			 map.initialLasers,0	,18,3,0},
   { "initialemergencythrusts",			  "initial EmrgyThrs",19,POSFLOAT,		 map.initialEmergencyThrusts,0 ,19,3,0},
   { "initialtractorbeams",   			  "initial TrctrBeam",19,POSFLOAT,		 map.initialTractorBeams,0 ,20,3,0},
   { "initialautopilots",     			  "initial Autopilot",19,POSFLOAT,		 map.initialAutopilots,0 ,21,3,0},
   { "initialemergencyshields",			  "initial EmrgyShld",19,POSFLOAT,		 map.initialEmergencyShields,0 ,22,3,0},
   { "gameduration",	      			  "GameDuration",6,POSINT,			 map.gameDuration,0	,23,3,0},
   { "checkpointradius",      			  "CheckpointtRad",19,POSFLOAT,		 map.checkpointRadius,0	,24,3,0},
   { "racelaps",    	      			  "Race Laps",6,POSINT,				 map.raceLaps,0	,25,3,0},
   { "lockotherteam",	      			  "lockOtherTeam",0,YESNO,			0,&map.lockOtherTeam	,26,3,0},
   { "loseitemdestroys",      			  "loseItemDests",0,YESNO,			0,&map.loseItemDestroys	,27,3,0},
   { "maxoffensiveitems",     			  "Max OffItems",6,POSINT,			 map.maxOffensiveItems,0 ,28,3,0},
   { "maxdefensiveitems",     			  "Max DefItems",6,POSINT,			 map.maxDefensiveItems,0 ,29,3,0},
   { "plockserver", 	      			  "pLockServer",0,YESNO,			0,&map.pLockServer	,30,3,0},
   { "mapdata"      , NULL             ,0,MAPDATA ,NULL     ,0,0,0,0 }
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

   LoadMap(map.filename);

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
   map.name[0] = map.filename[0] = (char ) NULL;
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
            strcpy(map.filename, argv[++index] );
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

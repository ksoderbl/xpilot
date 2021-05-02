/* $Id: default.c,v 3.21 1993/08/03 21:09:14 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <X11/Xos.h>
#include <X11/keysym.h>
#ifdef	__apollo
#    include <X11/ap_keysym.h>
#endif
#include <X11/Intrinsic.h>
#include <sys/types.h>
#include <sys/param.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "version.h"
#include "config.h"
#include "const.h"
#include "object.h"
#include "client.h"
#include "paint.h"
#include "draw.h"
#include "pack.h"
#include "bit.h"
#include "netclient.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: default.c,v 3.21 1993/08/03 21:09:14 bjoerns Exp $";
#endif

/*
 * Default fonts
 */
#define GAME_FONT		"-*-times-*-*-*-*-18-*-*-*-*-*-iso8859-1"
#define MESSAGE_FONT		"-*-times-*-*-*-*-14-*-*-*-*-*-iso8859-1"
#define SCORE_LIST_FONT		"-*-fixed-bold-*-*-*-15-*-*-*-c-*-iso8859-1"
#define BUTTON_FONT		"-*-*-bold-o-*-*-14-*-*-*-*-*-iso8859-1"
#define TEXT_FONT		"-*-*-bold-i-*-*-14-*-*-*-p-*-iso8859-1"

/*
 * Default colors.
 */
#define COLOR_BLACK		"#000000"
#define COLOR_WHITE		"#FFFFFF"
#define COLOR_BLUE		"#4E7CFF"
#define COLOR_RED		"#FF3A27"

static struct _keyResources
{
    char	*resource;
    char	*fallback;
    keys_t	key;
    char	*helpLine;
} keyResources[] = {
    { "keyTurnLeft",			"a",
	KEY_TURN_LEFT,			"Turn left (anti-clockwise)" },
    { "keyTurnRight",			"s",
	KEY_TURN_RIGHT,			"Turn right (clockwise)" },
    { "keyThrust",			"Shift_R Shift_L",
	KEY_THRUST,			"Thrust" },
    { "keyShield",			"space Meta_R",
	KEY_SHIELD,			"Raise shield" },
    { "keyFireShot",			"Return Linefeed",
	KEY_FIRE_SHOT,			"Fire shot" },
    { "keyFireMissile",			"backslash",
	KEY_FIRE_MISSILE,		"Fire smart missile" },
    { "keyFireTorpedo",			"quoteright",
	KEY_FIRE_TORPEDO,		"Fire unguided torpedo" },
    { "keyFireHeat",			"semicolon",
	KEY_FIRE_HEAT,			"Fire heat seaking missile" },
    { "keyFireNuke",			"n",
	KEY_FIRE_NUKE,			"Fire nuclear missile" },
    { "keyDropMine",			"Tab",
	KEY_DROP_MINE,			"Drop a mine (bomb)" },
    { "keyDetachMine",			"bracketright",
	KEY_DETACH_MINE,		"Detach a mine" },
    { "keyLockClose",			"Select Up Down",
	KEY_LOCK_CLOSE,			"Lock on closest player" },
    { "keyLockNext",			"Next Right",
	KEY_LOCK_NEXT,			"Lock on next player" },
    { "keyLockPrev",			"Prior Left",
	KEY_LOCK_PREV,			"Lock on previous player" },
    { "keyRefuel",			"f Control_L Control_R Caps_Lock",
	KEY_REFUEL,			"Refuel" },
    { "keyCloak",			"Delete BackSpace",
	KEY_CLOAK,			"Toggle cloakdevice" },
    { "keyEcm",				"bracketleft",
	KEY_ECM,			"Use ECM" },
    { "keySelfDestruct",		"q",
	KEY_SELF_DESTRUCT,		"Toggle self destruct" },
    { "keyIdMode",			"i",
	KEY_ID_MODE,			"Toggle ID mode" },
    { "keyPause",			"p Pause",
	KEY_PAUSE,			"Toggle pause mode" },
    { "keySwapSettings",		"Escape",
	KEY_SWAP_SETTINGS,		"Swap control settings" },
    { "keyChangeHome",			"Home h",
	KEY_CHANGE_HOME,		"Change home base" },
    { "keyConnector",			"Control_L",
	KEY_CONNECTOR,			"Use connector (pick up ball)" },
    { "keyDropBall",			"d",
	KEY_DROP_BALL,			"Drop a ball" },
    { "keyTankNext",			"e",
	KEY_TANK_NEXT,			"Shift to next tank" },
    { "keyTankPrev",			"w",
	KEY_TANK_PREV,			"Shift to previous tank" },
    { "keyTankDetach",			"r",
	KEY_TANK_DETACH,		"Detach tank" },
    { "keyIncreasePower",		"KP_Multiply",
	KEY_INCREASE_POWER,		"Increase power" },
    { "keyDecreasePower",		"KP_Divide",
	KEY_DECREASE_POWER,		"Decrease power" },
    { "keyIncreaseTurnspeed",		"KP_Add",
	KEY_INCREASE_TURNSPEED,		"Increase turnspeed" },
    { "keyDecreaseTurnspeed",		"KP_Subtract",
	KEY_DECREASE_TURNSPEED,		"Decrease turnspeed" },
    { "keyTransporter",			"t",
	KEY_TRANSPORTER,		"Use transporter" },
    { "keyTalk",			"m",
	KEY_TALK,			"Enable/disable talk window" },
    { "keyToggleVelocity",		"v",
	KEY_TOGGLE_VELOCITY,		"N/A" },
    { "keyToggleCompass",		"c",
	KEY_TOGGLE_COMPASS,		"N/A" }
};


static XrmOptionDescRec opts[] = {
    { "-help",				".help",
    	XrmoptionIsArg,				NULL },
    { "-version",			".version",
    	XrmoptionIsArg,				NULL },
    { "-name",				".name",
    	XrmoptionSepArg,			NULL },
    { "-join",				".join",
	XrmoptionIsArg,				NULL },
    { "-list",				".list",
	XrmoptionIsArg,				NULL },
    { "-team",				".team",
	XrmoptionSepArg,			NULL },
    { "-display",			".display",
    	XrmoptionSepArg,			NULL },
    { "-shutdown",			".shutdown",
    	XrmoptionSepArg,			NULL },
    { "-port",				".port",
    	XrmoptionSepArg,			NULL },
#ifdef SOUND
    { "-sounds",			".sounds",
    	XrmoptionSepArg,			NULL },
    { "-maxVolume",			".maxVolume",
    	XrmoptionSepArg,			NULL },
#endif
    { "-power",				".power",
    	XrmoptionSepArg,			NULL },
    { "-turnspeed",			".turnspeed",
    	XrmoptionSepArg,			NULL },
    { "-turnresistance",		".turnresistance",
    	XrmoptionSepArg,			NULL },
    { "-altPower",			".altPower",
    	XrmoptionSepArg,			NULL },
    { "-altTurnspeed",			".altTurnspeed",
    	XrmoptionSepArg,			NULL },
    { "-altTurnresistance",		".altTurnresistance",
    	XrmoptionSepArg,			NULL },
    { "-fuelNotify",			".fuelNotify",
    	XrmoptionSepArg,			NULL },
    { "-fuelWarning",			".fuelWarning",
    	XrmoptionSepArg,			NULL },
    { "-fuelCritical",			".fuelCritical",
    	XrmoptionSepArg,			NULL },
    { "-showHUD",			".showHUD",
    	XrmoptionSepArg,			NULL },
    { "-verticalHUDLine",		".verticalHUDLine",
    	XrmoptionSepArg,			NULL },
    { "-horizontalHUDLine",		".horizontalHUDLine",
    	XrmoptionSepArg,			NULL },
    { "-speedFactHUD",			".speedFactHUD",
    	XrmoptionSepArg,			NULL },
    { "-speedFactPTR",			".speedFactPTR",
    	XrmoptionSepArg,			NULL },
    { "-fuelMeter",			".fuelMeter",
    	XrmoptionSepArg,			NULL },
    { "-fuelGauge",			".fuelGauge",
    	XrmoptionSepArg,			NULL },
    { "-turnSpeedMeter",		".turnSpeedMeter",
    	XrmoptionSepArg,			NULL },
    { "-powerMeter",			".powerMeter",
    	XrmoptionSepArg,			NULL },
    { "-packetSizeMeter",		".packetSizeMeter",
    	XrmoptionSepArg,			NULL },
    { "-packetLossMeter",		".packetLossMeter",
    	XrmoptionSepArg,			NULL },
    { "-packetDropMeter",		".packetDropMeter",
    	XrmoptionSepArg,			NULL },
    { "-slidingRadar",			".slidingRadar",
    	XrmoptionSepArg,			NULL },
    { "-outlineWorld",			".outlineWorld",
    	XrmoptionSepArg,			NULL },
    { "-gameFont",			".gameFont",
    	XrmoptionSepArg,			NULL },
    { "-scoreListFont",			".scoreListFont",
    	XrmoptionSepArg,			NULL },
    { "-buttonFont",			".buttonFont",
    	XrmoptionSepArg,			NULL },
    { "-textFont",			".textFont",
    	XrmoptionSepArg,			NULL },
    { "-backGroundPointDist",		".backGroundPointDist",
	XrmoptionSepArg,			NULL },
    { "-receiveWindowSize",		".receiveWindowSize",
    	XrmoptionSepArg,			NULL },
    { "-black",				".black",
    	XrmoptionSepArg,			NULL },
    { "-white",				".white",
    	XrmoptionSepArg,			NULL },
    { "-blue",				".blue",
    	XrmoptionSepArg,			NULL },
    { "-red",				".red",
    	XrmoptionSepArg,			NULL },
    { "-keyTurnLeft",			".keyTurnLeft",
	XrmoptionSepArg,			NULL },
    { "-keyTurnRight",			".keyTurnRight",
	XrmoptionSepArg,			NULL },
    { "-keyThrust",			".keyThrust",
	XrmoptionSepArg,			NULL },
    { "-keyShield",			".keyShield",
	XrmoptionSepArg,			NULL },
    { "-keyFireShot",			".keyFireShot",
	XrmoptionSepArg,			NULL },
    { "-keyFireMissile",		".keyFireMissile",
	XrmoptionSepArg,			NULL },
    { "-keyFireTorpedo",		".keyFireTorpedo",
	XrmoptionSepArg,			NULL },
    { "-keyFireHeat",			".keyFireHeat",
	XrmoptionSepArg,			NULL },
    { "-keyFireNuke",			".keyFireNuke",
	XrmoptionSepArg,			NULL },
    { "-keyDropMine",			".keyDropMine",
	XrmoptionSepArg,			NULL },
    { "-keyDetachMine",			".keyDetachMine",
	XrmoptionSepArg,			NULL },
    { "-keyLockClose",			".keyLockClose",
	XrmoptionSepArg,			NULL },
    { "-keyLockNext",			".keyLockNext",
	XrmoptionSepArg,			NULL },
    { "-keyLockPrev",			".keyLockPrev",
	XrmoptionSepArg,			NULL },
    { "-keyRefuel",			".keyRefuel",
	XrmoptionSepArg,			NULL },
    { "-keyCloak",			".keyCloak",
	XrmoptionSepArg,			NULL },
    { "-keyEcm",			".keyEcm",
	XrmoptionSepArg,			NULL },
    { "-keySelfDestruct",		".keySelfDestruct",
	XrmoptionSepArg,			NULL },
    { "-keyIdMode",			".keyIdMode",
	XrmoptionSepArg,			NULL },
    { "-keyPause",			".keyPause",
	XrmoptionSepArg,			NULL },
    { "-keySwapSettings",		".keySwapSettings",
	XrmoptionSepArg,			NULL },
    { "-keyChangeHome",			".keyChangeHome",
	XrmoptionSepArg,			NULL },
    { "-keyConnector",			".keyConnector",
	XrmoptionSepArg,			NULL },
    { "-keyDropBall",			".keyDropBall",
	XrmoptionSepArg,			NULL },
    { "-keyTankNext",			".keyTankNext",
	XrmoptionSepArg,			NULL },
    { "-keyTankPrev",			".keyTankPrev",
	XrmoptionSepArg,			NULL },
    { "-keyTankDetach",			".keyTankDetach",
	XrmoptionSepArg,			NULL },
    { "-keyIncreasePower",		".keyIncreasePower",
	XrmoptionSepArg,			NULL },
    { "-keyDecreasePower",		".keyDecreasePower",
	XrmoptionSepArg,			NULL },
    { "-keyIncreaseTurnspeed",		".keyIncreaseTurnspeed",
	XrmoptionSepArg,			NULL },
    { "-keyDecreaseTurnspeed",		".keyDecreaseTurnspeed",
	XrmoptionSepArg,			NULL },
    { "-keyTransporter",		".keyTransporter",
	XrmoptionSepArg,			NULL },
    { "-keyTalk",			".keyTalk",
	XrmoptionSepArg,			NULL },
    { "-keyToggleVelocity",		".keyToggleVelocity",
	XrmoptionSepArg,			NULL },
    { "-keyToggleCompass",		".keyToggleCompass",
	XrmoptionSepArg,			NULL }
};


static int ON(char *optval)
{
    return (strncasecmp(optval, "true", 4) == 0
	    || strncasecmp(optval, "on", 2) == 0
	    || strncasecmp(optval, "yes", 3) == 0);
}


char* Get_keyhelpstring(keys_t key)
{
    int i=0;
    char* str = "Oops";

    for (i = 0; i < NELEM(keyResources); i++)
	if (keyResources[i].key == key) {
	    str = keyResources[i].helpLine;
	    break;
	}

    return str;
}


void Usage(void)
{
    int			i;

    printf("Usage: xpilot [-options ...] [server]\n");
    printf("Where options include:\n");
    for (i = 0; i < NELEM(opts); i++) {
	printf("\t%s%s\n", opts[i].option,
	       (opts[i].argKind == XrmoptionSepArg) ?  " <value>" : "");
    }
    printf("If no server is specified then the command will affect\n");
    printf("the servers on your local network\n");
    printf("Try: `telnet xpilot.cs.uit.no 4400' to see some remote servers\n");

    exit(1);
}


static int Get_resource(XrmDatabase db, char *myName, char *myClass,
			char *resource, char *fallback, char *result,
			unsigned size)
{
    int			i,
			len;
    char		str_name[80],
			str_class[80],
			*str_type[10];
    XrmValue		rmValue;

    sprintf(str_name, "%s.%s", myName, resource);
    sprintf(str_class, "%s.%c%s", myClass,
	    isupper(*resource) ? toupper(*resource) : *resource, resource + 1);

    if (XrmGetResource(db, str_name, str_class, str_type, &rmValue) == True) {
	if (rmValue.addr == NULL) {
	    len = 0;
	} else {
	    len = MIN(rmValue.size, size);
	    strncpy(result, rmValue.addr, len);
	}
	result[len] = '\0';
	for (i = 0; i < NELEM(opts); i++) {
	    if (opts[i].argKind == XrmoptionIsArg
		&& (strcmp(result, opts[i].option) == 0
		    || strcmp(result, opts[i].specifier) == 0)) {
		strncpy(result, "True", size);
		result[size - 1] = '\0';
		break;
	    }
	}
	return 1;
    }
    if (fallback != NULL) {
	strncpy(result, fallback, size - 1);
	result[size - 1] = '\0';
    } else {
	result[0] = '\0';
    }
    return 0;
}


static void Get_float_resource(XrmDatabase db, char *myName, char *myClass,
			       char *resource, float fallback, float *result)
{
    char		resValue[MAX_CHARS];

    if (Get_resource(db, myName, myClass, resource, NULL, resValue,
		     sizeof resValue) == 0
	|| sscanf(resValue, "%f", result) <= 0) {
	*result = fallback;
    }
}


static void Get_bool_resource(XrmDatabase db, char *myName, char *myClass,
			      char *resource, char *fallback, int *result)
{
    char		resValue[MAX_CHARS];

    Get_resource(db, myName, myClass, resource, fallback, resValue,
		 sizeof resValue);
    *result = (ON(resValue) != 0);
}


static void Get_int_resource(XrmDatabase db, char *myName, char *myClass,
			     char *resource, int fallback, int *result)
{
    char		resValue[MAX_CHARS];

    if (Get_resource(db, myName, myClass, resource, NULL, resValue,
		     sizeof resValue) == 0
	|| sscanf(resValue, "%d", result) <= 0) {
	*result = fallback;
    }
}


static void Get_file_defaults(XrmDatabase *rDBptr,
			       char *myName, char *myClass)
{
    int			len;
    char		*ptr,
			*lang = getenv("LANG"),
			*home = getenv("HOME"),
			path[MAXPATHLEN];
    XrmDatabase		tmpDB;

    sprintf(path, "%s%s", LIBDIR, myClass);
    *rDBptr = XrmGetFileDatabase(path);

    if (lang != NULL) {
	sprintf(path, "/usr/lib/X11/%s/app-defaults/%s", lang, myClass);
	if (access(path, 0) == -1) {
	    sprintf(path, "/usr/lib/X11/app-defaults/%s", myClass);
	}
    } else {
	sprintf(path, "/usr/lib/X11/%s", myClass);
    }
    tmpDB = XrmGetFileDatabase(path);
    XrmMergeDatabases(tmpDB, rDBptr);

    if ((ptr = getenv("XUSERFILESEARCHPATH")) != NULL) {
	sprintf(path, "%s/%s", ptr, myClass);
	tmpDB = XrmGetFileDatabase(path);
	XrmMergeDatabases(tmpDB, rDBptr);
    }
    else if ((ptr = getenv("XAPPLRESDIR")) != NULL) {
	if (lang != NULL) {
	    sprintf(path, "%s/%s/%s", ptr, lang, myClass);
	    if (access(path, 0) == -1) {
		sprintf(path, "%s/%s", ptr, myClass);
	    }
	} else {
	    sprintf(path, "%s/%s", ptr, myClass);
	}
	tmpDB = XrmGetFileDatabase(path);
	XrmMergeDatabases(tmpDB, rDBptr);
    }
    else if (home != NULL) {
	if (lang != NULL) {
	    sprintf(path, "%s/%s/%s", ptr, lang, myClass);
	    if (access(path, 0) == -1) {
		sprintf(path, "%s/%s", ptr, myClass);
	    }
	} else {
	    sprintf(path, "%s/%s", ptr, myClass);
	}
	tmpDB = XrmGetFileDatabase(path);
	XrmMergeDatabases(tmpDB, rDBptr);
    }

    if ((ptr = XResourceManagerString(dpy)) != NULL) {
	tmpDB = XrmGetStringDatabase(ptr);
	XrmMergeDatabases(tmpDB, rDBptr);
    }
    else if (home != NULL) {
	sprintf(path, "%s/.Xdefaults", home);
	tmpDB = XrmGetFileDatabase(path);
	XrmMergeDatabases(tmpDB, rDBptr);
    }

    if ((ptr = getenv("XENVIRONMENT")) != NULL) {
	tmpDB = XrmGetFileDatabase(ptr);
	XrmMergeDatabases(tmpDB, rDBptr);
    }
    else if (home != NULL) {
	sprintf(path, "%s/.Xdefaults-", home);
	len = strlen(path);
	gethostname(&path[len], sizeof path - len);
	path[sizeof path - 1] = '\0';
	tmpDB = XrmGetFileDatabase(path);
	XrmMergeDatabases(tmpDB, rDBptr);
    }

    if (home != NULL) {
	sprintf(path, "%s/.xpilotrc", home);
	tmpDB = XrmGetFileDatabase(path);
	XrmMergeDatabases(tmpDB, rDBptr);
    }
}


void Parse_options(int *argcp, char **argvp, char *realName, char *host,
		   int *port, int *my_team, int *list, int *join,
		   char *nickName, char *dispName, char *shut_msg)
{
    int			i,
			j,
			num;
    keys_t		key;
    KeySym		ks;
    char		*ptr,
			*str,
			*myName = "xpilot",
			*myClass = "XPilot",
			resValue[MAX_CHARS];
    XrmDatabase		argDB, rDB;

    XrmInitialize();

    argDB = 0;
    XrmParseCommand(&argDB, opts, NELEM(opts), myClass, argcp, argvp);

    /*
     * Check for bad arguments.
     */
    for (i = 1; i < *argcp; i++) {
	if (argvp[i][0] == '-') {
	    errno = 0;
	    error("Unknown option '%s'", argvp[i]);
	    Usage();
	}
    }

    if (Get_resource(argDB, myName, myClass, "help", NULL, resValue,
		     sizeof resValue) != 0) {
	Usage();
    }

    if (Get_resource(argDB, myName, myClass, "version", NULL, resValue,
		     sizeof resValue) != 0) {
	puts(TITLE);
	exit(0);
    }

    Get_resource(argDB, myName, myClass, "shutdown", "", shut_msg,
		 MAX_CHARS);

    if (Get_resource(argDB, myName, myClass, "display", NULL, dispName,
		     MAX_DISP_LEN) == 0
	|| dispName[0] == '\0') {
	if ((ptr = getenv("DISPLAY")) != NULL) {
	    strncpy(dispName, ptr, MAX_DISP_LEN);
	    dispName[MAX_DISP_LEN - 1] = '\0';
	} else {
	    strcpy(dispName, ":0.0");
	}
    }
#ifndef LINUX
    if (dispName[0] == '\0'
	|| strstr(dispName, "unix:0") != NULL
	|| strstr(dispName, "local:0") != NULL
	|| strstr(dispName, "localhost:0") != NULL
	|| strcmp(dispName, ":0.0") == 0
	|| strcmp(dispName, ":0") == 0) {
	sprintf(dispName, "%s:0", host);
    }
#endif
    if ((dpy = XOpenDisplay(dispName)) == NULL) {
	error("Can't open display '%s'", dispName);
	exit(1);
    }

    Get_file_defaults(&rDB, myName, myClass);

    XrmMergeDatabases(argDB, &rDB);

    Get_resource(rDB, myName, myClass, "name", realName, nickName,
		 MAX_NAME_LEN);
    CAP_LETTER(nickName[0]);
    if (nickName[0] < 'A' || nickName[0] > 'Z') {
	errno = 0;
	error("Your player name \"%s\" should start with an uppercase letter",
	    nickName);
	exit(1);
    }
    strncpy(realname, realName, sizeof(realname) - 1);
    strncpy(name, nickName, sizeof(name) - 1);

    Get_int_resource(rDB, myName, myClass, "team", TEAM_NOT_SET, my_team);
    if (*my_team < 0 || *my_team > 9) {
	*my_team = TEAM_NOT_SET;
    }
    team = *my_team;
    Get_int_resource(rDB, myName, myClass, "port", SERVER_PORT, port);
    Get_bool_resource(rDB, myName, myClass, "list", "False", list);
    Get_bool_resource(rDB, myName, myClass, "join", "False", join);

    Get_float_resource(rDB, myName, myClass, "power", 45.0, &power);
    Get_float_resource(rDB, myName, myClass, "turnSpeed", 35.0, &turnspeed);
    Get_float_resource(rDB, myName, myClass, "turnResistance", 0.12,
		       &turnresistance);
    Get_float_resource(rDB, myName, myClass, "altPower", 35.0, &power_s);
    Get_float_resource(rDB, myName, myClass, "altTurnspeed", 25.0,
		       &turnspeed_s);
    Get_float_resource(rDB, myName, myClass, "altTurnResistance", 0.12,
		       &turnresistance_s);

    Get_int_resource(rDB, myName, myClass, "backgroundPointDist", 8,
		     &map_point_distance);

    color_defaults[BLACK] = COLOR_BLACK;
    color_defaults[WHITE] = COLOR_WHITE;
    color_defaults[BLUE] = COLOR_BLUE;
    color_defaults[RED] = COLOR_RED;
    Get_resource(rDB, myName, myClass, "black", color_defaults[BLACK],
		 color_names[BLACK], sizeof(color_names[BLACK]));
    Get_resource(rDB, myName, myClass, "white", color_defaults[WHITE],
		 color_names[WHITE], sizeof(color_names[WHITE]));
    Get_resource(rDB, myName, myClass, "blue", color_defaults[BLUE],
		 color_names[BLUE], sizeof(color_names[BLUE]));
    Get_resource(rDB, myName, myClass, "red", color_defaults[RED],
		 color_names[RED], sizeof(color_names[RED]));

    instruments = 0;
    Get_bool_resource(rDB, myName, myClass, "showShipName", "True", &i);
    if (i) {
	SET_BIT(instruments, SHOW_SHIP_NAME);
    }
    Get_bool_resource(rDB, myName, myClass, "showHUD", "True", &i);
    if (i) {
	SET_BIT(instruments, SHOW_HUD_INSTRUMENTS);
    }
    Get_bool_resource(rDB, myName, myClass, "verticalHUDLine", "False", &i);
    if (i) {
	SET_BIT(instruments, SHOW_HUD_VERTICAL);
    }
    Get_bool_resource(rDB, myName, myClass, "horizontalHUDLine", "True", &i);
    if (i) {
	SET_BIT(instruments, SHOW_HUD_HORIZONTAL);
    }
    Get_bool_resource(rDB, myName, myClass, "fuelMeter", "True", &i);
    if (i) {
	SET_BIT(instruments, SHOW_FUEL_METER);
    }
    Get_bool_resource(rDB, myName, myClass, "fuelGauge", "False", &i);
    if (i) {
	SET_BIT(instruments, SHOW_FUEL_GAUGE);
    }
    Get_bool_resource(rDB, myName, myClass, "turnSpeedMeter", "False", &i);
    if (i) {
	SET_BIT(instruments, SHOW_TURNSPEED_METER);
    }
    Get_bool_resource(rDB, myName, myClass, "powerMeter", "False", &i);
    if (i) {
	SET_BIT(instruments, SHOW_POWER_METER);
    }
    Get_bool_resource(rDB, myName, myClass, "packetSizeMeter", "False", &i);
    if (i) {
	SET_BIT(instruments, SHOW_PACKET_SIZE_METER);
    }
    Get_bool_resource(rDB, myName, myClass, "packetLossMeter", "False", &i);
    if (i) {
	SET_BIT(instruments, SHOW_PACKET_LOSS_METER);
    }
    Get_bool_resource(rDB, myName, myClass, "packetDropMeter", "False", &i);
    if (i) {
	SET_BIT(instruments, SHOW_PACKET_DROP_METER);
    }
    Get_bool_resource(rDB, myName, myClass, "slidingRadar", "False", &i);
    if (i) {
	SET_BIT(instruments, SHOW_SLIDING_RADAR);
    }
    Get_bool_resource(rDB, myName, myClass, "outlineWorld", "False", &i);
    if (i) {
	SET_BIT(instruments, SHOW_OUTLINE_WORLD);
    }

    Get_float_resource(rDB, myName, myClass, "speedFactHUD", 0.0,
		       &hud_move_fact);
    Get_float_resource(rDB, myName, myClass, "speedFactPTR", 0.0,
		       &ptr_move_fact);
    Get_int_resource(rDB, myName, myClass, "fuelNotify", 500, &fuelLevel3);
    Get_int_resource(rDB, myName, myClass, "fuelWarning", 200, &fuelLevel2);
    Get_int_resource(rDB, myName, myClass, "fuelCritical", 100, &fuelLevel1);

    Get_resource(rDB, myName, myClass, "gameFont", GAME_FONT,
		 gameFontName, sizeof gameFontName);
    Get_resource(rDB, myName, myClass, "messageFont", MESSAGE_FONT,
		 messageFontName, sizeof messageFontName);
    Get_resource(rDB, myName, myClass, "scoreListFont", SCORE_LIST_FONT,
		 scoreListFontName, sizeof scoreListFontName);
    Get_resource(rDB, myName, myClass, "buttonFont", BUTTON_FONT,
		 buttonFontName, sizeof buttonFontName);
    Get_resource(rDB, myName, myClass, "textFont", TEXT_FONT,
		 textFontName, sizeof textFontName);

    Get_int_resource(rDB, myName, myClass, "receiveWindowSize",
		     DEF_RECEIVE_WINDOW_SIZE, &receive_window_size);

#ifdef SOUND
    Get_resource(rDB, myName, myClass, "sounds", SOUNDFILE, sounds,
		 sizeof sounds);

    Get_int_resource(rDB, myName, myClass, "maxVolume", 100, &maxVolume);
#endif


    /*
     * Key bindings
     */
    maxKeyDefs = 2 * NUM_KEYS;
    if ((keyDefs = (keydefs_t *)
	malloc(maxKeyDefs * sizeof(keydefs_t))) == NULL) {
	error("No memory for key bindings");
	exit(1);
    }
    num = 0;
    for (i = 0; i < NELEM(keyResources); i++) {
	key = keyResources[i].key;
	Get_resource(rDB, myName, myClass,
		     keyResources[i].resource, keyResources[i].fallback,
		     resValue, sizeof resValue);

	for (str = strtok(resValue, " \t\r\n");
	    str != NULL;
	    str = strtok(NULL, " \t\r\n")) {

	    if ((ks = XStringToKeysym(str)) == NoSymbol) {
		printf("\"%s\" is not a valid keysym.\n", str);
		continue;
	    }

	    for (j = 0; j < num; j++) {
		if (keyDefs[j].keysym == ks
		    && keyDefs[j].key == key) {
		    break;
		}
	    }
	    if (j < num) {
		continue;
	    }
	    if (num >= maxKeyDefs) {
		maxKeyDefs += NUM_KEYS;
		if ((keyDefs = (keydefs_t *)
		    realloc(keyDefs, maxKeyDefs * sizeof(keydefs_t)))
		    == NULL) {
		    error("No memory for key bindings");
		    exit(1);
		}
	    }

	    keyDefs[num].keysym = ks;
	    keyDefs[num].key = key;
	    num++;
	}
    }
    if (num < maxKeyDefs) {
	maxKeyDefs = num;
	if ((keyDefs = (keydefs_t *)
	    realloc(keyDefs, maxKeyDefs * sizeof(keydefs_t))) == NULL) {
	    error("No memory for key bindings");
	    exit(1);
	}
    }

#ifdef SOUND
    audioInit(dispName);
#endif /* SOUND */
}

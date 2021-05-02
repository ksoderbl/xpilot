/* $Id: default.c,v 1.6 1993/04/18 16:46:16 kenrsc Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <X11/keysym.h>
#include "global.h"
#include "pack.h"
#include "const.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: default.c,v 1.6 1993/04/18 16:46:16 kenrsc Exp $";
#endif


static int ON(char *optval)
{
    return (strcasecmp(optval, "true") == 0
	    || strcasecmp(optval, "on") == 0
	    || strcasecmp(optval, "yes") == 0);
}


static float atod(char *str)
{
    float tmp;

    sscanf(str, "%f", &tmp);
    return (tmp);
}


void Get_defaults(int ind)
{
    char		*str;
    int			i;
    player		*pl = Players[ind];
    static struct _keyResources
    {
	char    *resource;
	keys_t	key;
    } keyResources[] = {
	"keyLockNext", 		KEY_LOCK_NEXT,
	"keyLockPrev", 		KEY_LOCK_PREV,
	"keyLockClose", 	KEY_LOCK_CLOSE,
	"keyChangeHome",	KEY_CHANGE_HOME,
	"keyShield",		KEY_SHIELD,
	"keyFireShot",		KEY_FIRE_SHOT,
	"keyFireMissile",	KEY_FIRE_MISSILE,
        "keyFireTorpedo",       KEY_FIRE_TORPEDO,
        "keyFireHeat",          KEY_FIRE_HEAT,
	"keyFireNuke",          KEY_FIRE_NUKE,
	"keyDropMine",		KEY_DROP_MINE,
	"keyDetachMine",	KEY_DETACH_MINE,
	"keyTurnLeft",		KEY_TURN_LEFT,
	"keyTurnRight",		KEY_TURN_RIGHT,
	"keySelfDestruct",	KEY_SELF_DESTRUCT,
	"keyIdMode",		KEY_ID_MODE,
	"keyPause",		KEY_PAUSE,
	"keyToggleVelocity",	KEY_TOGGLE_VELOCITY,
	"keyToggleCompass",	KEY_TOGGLE_COMPASS,
	"keySwapSettings",	KEY_SWAP_SETTINGS,
	"keyRefuel",		KEY_REFUEL,
	"keyConnector",		KEY_CONNECTOR,
	"keyIncreasePower",	KEY_INCREASE_POWER,
	"keyDecreasePower",	KEY_DECREASE_POWER,
	"keyIncreaseTurnspeed",	KEY_INCREASE_TURNSPEED,
	"keyDecreaseTurnspeed",	KEY_DECREASE_TURNSPEED,
        "keyTankNext",          KEY_TANK_NEXT,
        "keyTankPrev",          KEY_TANK_PREV,
        "keyTankDetach",        KEY_TANK_DETACH,
	"keyThrust",		KEY_THRUST,
	"keyCloak",		KEY_CLOAK,
	"keyEcm",		KEY_ECM,
	"keyDropBall",		KEY_DROP_BALL,
    };
    static struct _keyDefs keyDefs[MAX_KEY_DEFS] =
    {
	XK_Left,		KEY_LOCK_PREV,
	XK_Right,		KEY_LOCK_NEXT,
	XK_Next,		KEY_LOCK_NEXT,
	XK_Prior,		KEY_LOCK_PREV,
	XK_Up,			KEY_LOCK_CLOSE,
	XK_Down,		KEY_LOCK_CLOSE,
	XK_Select,		KEY_LOCK_CLOSE,
	XK_Home,		KEY_CHANGE_HOME,
	XK_Meta_R,		KEY_SHIELD,
	XK_space,		KEY_SHIELD,
	XK_Return,		KEY_FIRE_SHOT,
	XK_backslash,		KEY_FIRE_MISSILE,
	XK_Linefeed,		KEY_ECM,
 	XK_bracketleft,		KEY_ECM,
        XK_quoteright,          KEY_FIRE_TORPEDO,
        XK_semicolon,           KEY_FIRE_HEAT,
	XK_n,                   KEY_FIRE_NUKE,
	XK_Tab,			KEY_DROP_MINE,
 	XK_bracketright,	KEY_DETACH_MINE,
	XK_a,			KEY_TURN_LEFT,
	XK_s,			KEY_TURN_RIGHT,
	XK_q,			KEY_SELF_DESTRUCT,
	XK_i,			KEY_ID_MODE,
	XK_p,			KEY_PAUSE,
	XK_v,			KEY_TOGGLE_VELOCITY,
	XK_c,			KEY_TOGGLE_COMPASS,
	XK_Escape,		KEY_SWAP_SETTINGS,
	XK_f,			KEY_REFUEL,
	XK_Control_L,		KEY_REFUEL,
	XK_Control_L,		KEY_CONNECTOR,
	XK_space,		KEY_CONNECTOR,
	XK_KP_Multiply,		KEY_INCREASE_POWER,
	XK_KP_Divide,		KEY_DECREASE_POWER,
	XK_KP_Add,		KEY_INCREASE_TURNSPEED,
	XK_KP_Subtract,		KEY_DECREASE_TURNSPEED,
        XK_w,                   KEY_TANK_PREV,
        XK_e,                   KEY_TANK_NEXT,
        XK_r,                   KEY_TANK_DETACH,
	XK_Shift_L,		KEY_THRUST,
	XK_Shift_R,		KEY_THRUST,
	XK_Delete,		KEY_CLOAK,
	XK_BackSpace,		KEY_CLOAK,
	XK_d,			KEY_DROP_BALL,
    };


    /*
     * Defaults.
     */
    pl->power			= 45.0;
    pl->turnspeed		= 30.0;
    pl->turnresistance		= 0.12;

    pl->power_s			= 35.0;
    pl->turnspeed_s		= 25.0;
    pl->turnresistance_s	= 0.12;
    pl->team			= TEAM_NOT_SET;
    pl->fuel.l3			= 500*FUEL_SCALE_FACT;
    pl->fuel.l2			= 200*FUEL_SCALE_FACT;
    pl->fuel.l1			= 100*FUEL_SCALE_FACT;
    pl->instruments		= SHOW_HUD_INSTRUMENTS | SHOW_HUD_HORIZONTAL;
    bcopy(keyDefs, pl->keyDefs, sizeof(keyDefs));

    /*
     * Name
     */
    if (pl->name[0] == '\0') {
	if ((pl->disp != NULL)
	    && (str = XGetDefault(pl->disp, "xpilot", "name")) != NULL)
	    strncpy(pl->name, str, MAX_NAME_LEN);
	else
	    strcpy(pl->name, pl->realname);
    }
    CAP_LETTER(pl->name[0]);


    /*
     * Control sensitivity.
     */
    str = XGetDefault(pl->disp, "xpilot", "power");
    if (str)
	pl->power = atod(str);
    str = XGetDefault(pl->disp, "xpilot", "turnSpeed");
    if (str)
	pl->turnspeed = atod(str);
    str = XGetDefault(pl->disp, "xpilot", "turnResistance");
    if (str)
	pl->turnresistance = atod(str);

    str = XGetDefault(pl->disp, "xpilot", "altPower");
    if (str)
	pl->power_s = atod(str);
    str = XGetDefault(pl->disp, "xpilot", "altTurnSpeed");
    if (str)
	pl->turnspeed_s = atod(str);
    str = XGetDefault(pl->disp, "xpilot", "altTurnResistance");
    if (str)
	pl->turnresistance_s = atod(str);


    /*
     * Misc. data, fuel limits on HUD.
     */
    str = XGetDefault(pl->disp, "xpilot", "team");
    if (str)
	pl->team = atoi(str);
    str = XGetDefault(pl->disp, "xpilot", "fuelNotify");
    if (str)
	pl->fuel.l3 = atod(str)*FUEL_SCALE_FACT;
    str = XGetDefault(pl->disp, "xpilot", "fuelWarning");
    if (str)
	pl->fuel.l2 = atod(str)*FUEL_SCALE_FACT;
    str = XGetDefault(pl->disp, "xpilot", "fuelCritical");
    if (str)
	pl->fuel.l1 = atod(str)*FUEL_SCALE_FACT;
    
    /*
     * Instruments.
     */

    /* HUD */
    str = XGetDefault(pl->disp, "xpilot", "showHUD");
    if (str) {
	if (ON(str)) {
	    SET_BIT(pl->instruments, SHOW_HUD_INSTRUMENTS);
	} else {
	    CLR_BIT(pl->instruments, SHOW_HUD_INSTRUMENTS);
	}
    }
    str = XGetDefault(pl->disp, "xpilot", "verticalHUDLine");
    if (str) {
	if (ON(str)) {
	    SET_BIT(pl->instruments, SHOW_HUD_VERTICAL);
	} else {
	    CLR_BIT(pl->instruments, SHOW_HUD_VERTICAL);
	}
    }
    str = XGetDefault(pl->disp, "xpilot", "horizontalHUDLine");
    if (str) {
	if (ON(str)) {
	    SET_BIT(pl->instruments, SHOW_HUD_HORIZONTAL);
	} else {
	    CLR_BIT(pl->instruments, SHOW_HUD_HORIZONTAL);
	}
    }
    str = XGetDefault(pl->disp, "xpilot", "speedFactHUD");
    pl->hud_move_fact = str?atod(str):0.0;
    str = XGetDefault(pl->disp, "xpilot", "speedFactPTR");
    pl->ptr_move_fact = str?atod(str):0.0;
    str = XGetDefault(pl->disp, "xpilot", "BackgroundPointDist");
    pl->map_point_distance = str ? atoi(str) : 8;


    /* FUEL */
    str = XGetDefault(pl->disp, "xpilot", "fuelMeter");
    if (str) {
	if (ON(str)) {
	    SET_BIT(pl->instruments, SHOW_FUEL_METER);
	} else {
	    CLR_BIT(pl->instruments, SHOW_FUEL_METER);
	}
    }
    str = XGetDefault(pl->disp, "xpilot", "fuelGauge");
    if (str) {
	if (ON(str)) {
	    SET_BIT(pl->instruments, SHOW_FUEL_GAUGE);
	} else {
	    CLR_BIT(pl->instruments, SHOW_FUEL_GAUGE);
	}
    }

    /* Misc. meters. */
    str = XGetDefault(pl->disp, "xpilot", "turnSpeedMeter");
    if (str) {
	if (ON(str)) {
	    SET_BIT(pl->instruments, SHOW_TURNSPEED_METER);
	} else {
	    CLR_BIT(pl->instruments, SHOW_TURNSPEED_METER);
	}
    }
    str = XGetDefault(pl->disp, "xpilot", "powerMeter");
    if (str) {
	if (ON(str)) {
	    SET_BIT(pl->instruments, SHOW_POWER_METER);
	} else {
	    CLR_BIT(pl->instruments, SHOW_POWER_METER);
	}
    }

    /* key bindings */
    for (i = 0; i < NELEM(keyResources); i++)
	if (str = XGetDefault(pl->disp, "xpilot", keyResources[i].resource))
	{
	    KeySym ks;
	    char *p, *p1;
	    int j;

	    if ((p1 = (char *)malloc(sizeof(char) * (strlen(str) + 1)))
		== NULL) {
		error("No memory for key bindings");
		continue;
	    }
	    strcpy(p1, str);
	    p = strtok(p1, " \t\n");

	    while (p) {
		if ((ks = XStringToKeysym(p)) == NoSymbol) {
		    printf("%s is not a valid keysym.\n", p);
		    break;
		}

		for (j = 0; j < MAX_KEY_DEFS; j++)
		    if (!pl->keyDefs[j].keysym || pl->keyDefs[j].keysym == ks) {
			pl->keyDefs[j].keysym = ks;
			pl->keyDefs[j].key = keyResources[i].key;
			break;
		    }

		if (j >= MAX_KEY_DEFS) {
		    printf("Too may key definitions.  Only %d allowed.\n",
			   MAX_KEY_DEFS);
		    i = NELEM(keyResources);
		    break;
		}

		p = strtok(NULL, " \t\n");
	    }

	    free(p1);
	}
}

/* default.c,v 1.5 1992/06/27 02:14:15 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <X11/keysym.h>
#include "global.h"
#include "pack.h"
#include "limits.h"

#ifndef	lint
static char sourceid[] =
    "@(#)default.c,v 1.5 1992/06/27 02:14:15 bjoerns Exp";
#endif

#define ON(x)	      ( (strcasecmp(x, "true")==0) || (strcasecmp(x, "on")==0) )
#define NELEM(a)	(sizeof(a) / sizeof((a)[0]))



static double atod(char *str)
{
    double tmp;

    sscanf(str, "%lf", &tmp);
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
	"keyDropMine",		KEY_DROP_MINE,
	"keyTurnLeft",		KEY_TURN_LEFT,
	"keyTurnRight",		KEY_TURN_RIGHT,
	"keySelfDestruct",	KEY_SELF_DESTRUCT,
	"keyIdMode",		KEY_ID_MODE,
	"keyPause",		KEY_PAUSE,
	"keyToggleVelocity",	KEY_TOGGLE_VELOCITY,
	"keyToggleCompass",	KEY_TOGGLE_COMPASS,
	"keySwapSettings",	KEY_SWAP_SETTINGS,
	"keyRefuel",		KEY_REFUEL,
	"keyIncreasePower",	KEY_INCREASE_POWER,
	"keyDecreasePower",	KEY_DECREASE_POWER,
	"keyIncreaseTurnspeed",	KEY_INCREASE_TURNSPEED,
	"keyDecreaseTurnspeed",	KEY_DECREASE_TURNSPEED,
	"keyThrust",		KEY_THRUST,
	"keyCloak",		KEY_CLOAK,
	"keySlowdown",		KEY_SLOWDOWN,
	"keySpeedup",		KEY_SPEEDUP
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
	XK_Linefeed,		KEY_FIRE_MISSILE,
	XK_Tab,			KEY_DROP_MINE,
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
	XK_KP_Multiply,		KEY_INCREASE_POWER,
	XK_KP_Divide,		KEY_DECREASE_POWER,
	XK_KP_Add,		KEY_INCREASE_TURNSPEED,
	XK_KP_Subtract,		KEY_DECREASE_TURNSPEED,
	XK_Shift_L,		KEY_THRUST,
	XK_Shift_R,		KEY_THRUST,
	XK_Delete,		KEY_CLOAK,
	XK_BackSpace,		KEY_CLOAK,
	XK_bracketright,	KEY_SPEEDUP,
	XK_bracketleft,		KEY_SLOWDOWN
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
    pl->team			= 0;
    pl->fuel3			= 500.0;
    pl->fuel2			= 200.0;
    pl->fuel1			= 100.0;
    pl->instruments		= SHOW_HUD_INSTRUMENTS |\
				  SHOW_HUD_HORIZONTAL;
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
	pl->fuel3 = atod(str);
    str = XGetDefault(pl->disp, "xpilot", "fuelWarning");
    if (str)
	pl->fuel2 = atod(str);
    str = XGetDefault(pl->disp, "xpilot", "fuelCritical");
    if (str)
	pl->fuel1 = atod(str);
    
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

	    p1 = strdup(str);
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

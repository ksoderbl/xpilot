/* $Id: default.c,v 3.80 1994/08/16 19:03:20 bert Exp $
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

#include "types.h"
#include <X11/Xos.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#ifdef	__apollo
#    include <X11/ap_keysym.h>
#endif
#include <sys/types.h>
#ifdef VMS
#include "strcasecmp.h"
#include <unixio.h>
#include <unixlib.h>
#else
#include <unistd.h>
#include <sys/param.h>
#endif
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "version.h"
#include "config.h"
#include "const.h"
#include "paint.h"
#include "pack.h"
#include "bit.h"
#include "keys.h"
#include "netclient.h"
#include "xinit.h"
#include "error.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: default.c,v 3.80 1994/08/16 19:03:20 bert Exp $";
#endif

#ifdef VMS
#define DISPLAY_ENV	"DECW$DISPLAY"
#define DISPLAY_DEF	"::0.0"
#define KEYBOARD_ENV	"DECW$KEYBOARD"
#else
#define DISPLAY_ENV	"DISPLAY"
#define DISPLAY_DEF	":0.0"
#define KEYBOARD_ENV	"KEYBOARD"
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN	1024
#endif

/*
 * Default fonts
 */
#define GAME_FONT	"-*-times-*-*-*-*-18-*-*-*-*-*-iso8859-1"
#define MESSAGE_FONT	"-*-times-*-*-*-*-14-*-*-*-*-*-iso8859-1"
#define SCORE_LIST_FONT	"-*-fixed-bold-*-*-*-15-*-*-*-c-*-iso8859-1"
#define BUTTON_FONT	"-*-*-bold-o-*-*-14-*-*-*-*-*-iso8859-1"
#define TEXT_FONT	"-*-*-bold-i-*-*-14-*-*-*-p-*-iso8859-1"
#define TALK_FONT	"-*-fixed-bold-*-*-*-15-*-*-*-c-*-iso8859-1"
#define KEY_LIST_FONT	"-*-fixed-medium-r-*-*-10-*-*-*-c-*-iso8859-1"
#define MOTD_FONT	"-*-courier-bold-r-*--14-*-*-*-*-*-iso8859-1"

static char		*myName = "xpilot";
static char		*myClass = "XPilot";


keys_t buttonDefs[MAX_POINTER_BUTTONS];

/*
 * Structure to store all the client options.
 * The most important field is the help field.
 * It is used to self-document the client to
 * the user when "xpilot -help" is issued.
 * Help lines can span multiple lines, but for
 * the key help window only the first line is used.
 */
struct option {
    char		*name;		/* option name */
    char		*noArg;		/* value for non-argument options */
    char		*fallback;	/* default value */
    keys_t		key;		/* key if not KEY_DUMMY */
    char		*help;		/* user help (multiline) */
} options[] = {
    {
	"help",
	"Yes",
	"",
	KEY_DUMMY,
	"Display this help message.\n"
    },
    {
	"version",
	"Yes",
	"",
	KEY_DUMMY,
	"Show the source code version.\n"
    },
    {
	"name",
	NULL,
	"",
	KEY_DUMMY,
	"Set the nickname.\n"
    },
    {
	"join",
	"Yes",
	"",
	KEY_DUMMY,
	"Join the game immediately, no questions asked.\n"
    },
    {
	"noLocalMotd",
	"Yes",
	"",
	KEY_DUMMY,
	"Do not display the local Message Of The Day.\n"
    },
    {
	"autoServerMotdPopup",
	NULL,
	"Yes",
	KEY_DUMMY,
	"Popup the MOTD of the server when available.\n"
    },
    {
	"list",
	"Yes",
	"",
	KEY_DUMMY,
	"List all servers running on the local network.\n"
    },
    {
	"team",
	NULL,
	TEAM_NOT_SET_STR,
	KEY_DUMMY,
	"Set the team to join.\n"
    },
    {
	"display",
	NULL,
	"",
	KEY_DUMMY,
	"Set the X display.\n"
    },
    {
	"keyboard",
	NULL,
	"",
	KEY_DUMMY,
	"Set the X keyboard input if you want keyboard input from\n"
	"another display.  The default is to use the keyboard input from\n"
	"the X display.\n"
    },
    {
	"geometry",
	NULL,
	"",
	KEY_DUMMY,
	"Set the window size and position in standard X geometry format.\n"
	"The maximum allowed window size is 1282x1024.\n"
    },
    {
	"shutdown",
	NULL,
	"",
	KEY_DUMMY,
	"Shutdown the server with a message.\n"
	"The message used is the first argument to this option.\n"
    },
    {
	"port",
	NULL,
	SERVER_PORT_STR,
	KEY_DUMMY,
	"Set the port number of the server.\n"
	"Almost all servers use the default port, which is the\n"
	"recommended policy.  You can find out about which port\n"
	"is used by a server by querying the XPilot Meta server.\n"
    },
    {
	"shipShape",
	NULL,
	"",
	KEY_DUMMY,
	"Define the ship shape to use.  Because the argument to this option\n"
	"is rather large (up to 500 bytes) the recommended way to set\n"
	"this option is in the .xpilotrc file in your home directory.\n"
	"The exact format is defined in the file README.ships in the XPilot\n"
	"distribution.  Note that there is a nifty tool called editss for\n"
	"easy ship creation.  See the XPilot FAQ for details.\n"
    },
    {
	"power",
	NULL,
	"45.0",
	KEY_DUMMY,
	"Set the engine power.\n"
	"Valid values are in the range 5-55.\n"
    },
    {
	"turnSpeed",
	NULL,
	"35.0",
	KEY_DUMMY,
	"Set the ship's turn speed.\n"
	"Valid values are in the range 4-64.\n"
	"See also turnResistance.\n"
    },
    {
	"turnResistance",
	NULL,
	"0.12",
	KEY_DUMMY,
	"Set the ship's turn resistance.\n"
	"This determines the speed at which a ship stops turning\n"
	"after a turn key has been released.\n"
	"Valid values are in the range 0.0-1.0.\n"
	"Useful values are in the range 0.05-0.15.\n"
	"See also turnSpeed.\n"
    },
    {
	"altPower",
	NULL,
	"35.0",
	KEY_DUMMY,
	"Set the alternate engine power.\n"
	"See also the keySwapSettings option.\n"
    },
    {
	"altTurnSpeed",
	NULL,
	"25.0",
	KEY_DUMMY,
	"Set the alternate ship's turn speed.\n"
	"See also the keySwapSettings option.\n"
    },
    {
	"altTurnResistance",
	NULL,
	"0.12",
	KEY_DUMMY,
	"Set the alternate ship's turn resistance.\n"
	"See also the keySwapSettings option.\n"
    },
    {
	"showHUD",
	NULL,
	"Yes",
	KEY_DUMMY,
	"Should the HUD be displayed or not.\n"
    },
    {
	"fuelNotify",
	NULL,
	"500",
	KEY_DUMMY,
	"The limit when the HUD fuel bar will become visible.\n"
    },
    {
	"fuelWarning",
	NULL,
	"200",
	KEY_DUMMY,
	"The limit when the HUD fuel bar will start flashing.\n"
    },
    {
	"fuelCritical",
	NULL,
	"100",
	KEY_DUMMY,
	"The limit when the HUD fuel bar will flash faster.\n"
    },
    {
	"verticalHUDLine",
	NULL,
	"No",
	KEY_DUMMY,
	"Should the vertical lines in the HUD be drawn.\n"
    },
    {
	"horizontalHUDLine",
	NULL,
	"Yes",
	KEY_DUMMY,
	"Should the horizontal lines in the HUD be drawn.\n"
    },
    {
	"speedFactHUD",
	NULL,
	"0.0",
	KEY_DUMMY,
	"Should the HUD be moved, to indicate the current velocity?\n"
    },
    {
	"speedFactPTR",
	NULL,
	"0.0",
	KEY_DUMMY,
	"Uses a red line to indicate the current velocity and direction.\n"
    },
    {
	"fuelMeter",
	NULL,
	"No",
	KEY_DUMMY,
	"Determines if the fuel meter should be visible.\n"
    },
    {
	"fuelGauge",
	NULL,
	"Yes",
	KEY_DUMMY,
	"Determines if the fuel gauge should be visible.\n"
    },
    {
	"turnSpeedMeter",
	NULL,
	"No",
	KEY_DUMMY,
	"Should the turn speed meter be visible at all times.\n"
    },
    {
	"powerMeter",
	NULL,
	"No",
	KEY_DUMMY,
	"Should the power meter be visible at all times.\n"
    },
    {
	"packetSizeMeter",
	NULL,
	"No",
	KEY_DUMMY,
	"Should the network packet size meter be visible at all times.\n"
	"Each bar is equavalent to 1024 bytes, for a maximum of 4096 bytes.\n"
    },
    {
	"packetLossMeter",
	NULL,
	"No",
	KEY_DUMMY,
	"Should the packet loss meter be visible.\n"
	"This gives the percentage of lossed frames due to network failure.\n"
    },
    {
	"packetDropMeter",
	NULL,
	"No",
	KEY_DUMMY,
	"Should the packet drop meter be visible.\n"
	"This gives the percentage of dropped frames due to display slowness.\n"
    },
    {
	"slidingRadar",
	NULL,
	"No",
	KEY_DUMMY,
	"If the game is in edgewrap mode then the radar will keep your\n"
	"position on the radar in the center and raw the rest of the radar\n"
	"around it.  Note that this requires a fast graphics system.\n"
    },
    {
	"outlineWorld",
	NULL,
	"No",
	KEY_DUMMY,
	"Draws only the outline of all the blue map constructs.\n"
    },
    {
	"markingLights",
	NULL,
	"Yes",
	KEY_DUMMY,
	"Should the fighters have marking lights, just like airplanes?\n"
    },
    {
	"sparkProb",
	NULL,
	"0.50",
	KEY_DUMMY,
	"The chance that sparks are drawn or not.\n"
	"This gives a sparkling effect.\n"
    },
    {
	"sparkSize",
	NULL,
	"2",
	KEY_DUMMY,
	"Size of sparks.\n"
    },
    {
	"charsPerSecond",
	NULL,
	"50",
	KEY_DUMMY,
	"Speed in which messages appear on screen in characters per second.\n"
    },
    {
	"clock",
	NULL,
	"No",
	KEY_DUMMY,
	"Should a clock be displayed in the top right of the score window.\n"
    },
    {
	"pointerControl",
	NULL,
	"No",
	KEY_DUMMY,
	"Enable mouse control.  This allows ship direction control by\n"
	"moving the mouse to the left for an anti-clockwise turn and\n"
	"moving the mouse to the right for a clockwise turn.\n"
	"Also see the pointerButton options for use of the mouse buttons.\n"
    },
    {
	"gameFont",
	NULL,
	GAME_FONT,
	KEY_DUMMY,
	"The font used on the HUD and for most other text.\n"
    },
    {
	"scoreListFont",
	NULL,
	SCORE_LIST_FONT,
	KEY_DUMMY,
	"The font used on the score list.\n"
	"This must be a non-proportional font.\n"
    },
    {
	"buttonFont",
	NULL,
	BUTTON_FONT,
	KEY_DUMMY,
	"The font used on all buttons.\n"
    },
    {
	"textFont",
	NULL,
	TEXT_FONT,
	KEY_DUMMY,
	"The font used in the help and about windows.\n"
    },
    {
	"talkFont",
	NULL,
	TALK_FONT,
	KEY_DUMMY,
	"The font used in the talk window.\n"
    },
	{
	"motdFont",
	NULL,
	MOTD_FONT,
	KEY_DUMMY,
	"The font used in the MOTD window and key list window.\n"
	"This must be a non-proportional font.\n"
    },
    {
	"messageFont",
	NULL,
	MESSAGE_FONT,
	KEY_DUMMY,
	"The font used for drawing messages.\n"
    },
    {
	"shotSize",
	NULL,
	"3",
	KEY_DUMMY,
	"The size of shots.\n"
    },
    {
	"teamShotSize",
	NULL,
	"2",
	KEY_DUMMY,
	"The size of team shots.  Note that team shots are drawn in blue.\n"
    },
    {
	"backgroundPointDist",
	NULL,
	"8",
	KEY_DUMMY,
	"The distance between points in the background measured in blocks.\n"
	"These are drawn in empty map regions to keep feeling for which\n"
	"direction the ship is moving to.\n"
    },
    {
	"backgroundPointSize",
	NULL,
	"2",
	KEY_DUMMY,
	"Specifies the size of the background points.  0 means no points.\n"
    },
    {
	"titleFlip",
	NULL,
	"Yes",
	KEY_DUMMY,
	"Should the title bar change or not.\n"
	"Some window managers like twm may have problems with\n"
	"flipping title bars.\n"
    },
    {
	"toggleShield",
	NULL,
	"No",
	KEY_DUMMY,
	"Are shields toggled by a keypress only?\n"
    },
    {
	"shieldDrawSolid",
	NULL,
	"Default",
	KEY_DUMMY,
	"Are shields drawn in a solid line.\n"
	"Not setting a value for this option will select the best value\n"
	"automatically for your particular display system.\n"
    },
    {
	"showShipName",
	NULL,
	"Yes",
	KEY_DUMMY,
	"Should all ships have the name of the player drawn below them.\n"
    },
    {
	"showMineName",
	NULL,
	"Yes",
	KEY_DUMMY,
	"Should the name of the owner of the mine be drawn below the mine.\n"
    },
    {
	"showMessages",
	NULL,
	"Yes",
	KEY_DUMMY,
	"Should messages appear on screen.\n"
    },
    {
	"showItems",
	NULL,
	"Yes",
	KEY_DUMMY,
	"Dunno what this does.  If you do please let me know.\n"
    },
    {
	"showItemsTime",
	NULL,
	"2.0",
	KEY_DUMMY,
	"Dunno what this does.  If you do please let me know.\n"
    },
    {
	"receiveWindowSize",
	NULL,
	"3",
	KEY_DUMMY,
	"Too complicated.  Keep it on 3.\n"
    },
    {
	"visual",
	NULL,
	"",
	KEY_DUMMY,
	"Specify which visual to use for allocating colors.\n"
	"To get a listing of all possible visuals on your dislay\n"
	"set the argument for this option to list.\n"
    },
    {
	"mono",
	"Yes",
	"",
	KEY_DUMMY,
	"Only use black and white (may be very slow).\n"
    },
    {
	"colorSwitch",
	NULL,
	"Yes",
	KEY_DUMMY,
	"Use color buffering or not.\n"
    },
    {
	"maxColors",
	NULL,
	"4",
	KEY_DUMMY,
	"The number of colors to use.  Valid values are 4, 8 and 16.\n"
    },
    {
	"black",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for black (better use color0 instead).\n"
    },
    {
	"white",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for white (better use color1 instead).\n"
    },
    {
	"blue",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for blue (better use color2 instead).\n"
    },
    {
	"red",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for red (better use color3 instead).\n"
    },
    {
	"color0",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for the first color.\n"
    },
    {
	"color1",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for the second color.\n"
    },
    {
	"color2",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for the third color.\n"
    },
    {
	"color3",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for the fourth color.\n"
    },
    {
	"color4",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for the fifth color.\n"
	"This is only used if maxColors is set to 8 or 16.\n"
    },
    {
	"color5",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for the sixth color.\n"
	"This is only used if maxColors is set to 8 or 16.\n"
    },
    {
	"color6",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for the seventh color.\n"
	"This is only used if maxColors is set to 8 or 16.\n"
    },
    {
	"color7",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for the eighth color.\n"
	"This is only used if maxColors is set to 8 or 16.\n"
    },
    {
	"color8",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for the nineth color.\n"
	"This is only used if maxColors is set to 16.\n"
    },
    {
	"color9",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for the tenth color.\n"
	"This is only used if maxColors is set to 16.\n"
    },
    {
	"color10",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for the eleventh color.\n"
	"This is only used if maxColors is set to 16.\n"
    },
    {
	"color11",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for the twelfth color.\n"
	"This is only used if maxColors is set to 16.\n"
    },
    {
	"color12",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for the thirteenth color.\n"
	"This is only used if maxColors is set to 16.\n"
    },
    {
	"color13",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for the fourteenth color.\n"
	"This is only used if maxColors is set to 16.\n"
    },
    {
	"color14",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for the fifteenth color.\n"
	"This is only used if maxColors is set to 16.\n"
    },
    {
	"color15",
	NULL,
	"",
	KEY_DUMMY,
	"The color value for the sixteenth color.\n"
	"This is only used if maxColors is set to 16.\n"
    },
    {
	"hudColor",
	NULL,
	"4",
	KEY_DUMMY,
	"Which color number to use for drawing the HUD.\n"
    },
    {
	"targetRadarColor",
	NULL,
	"4",
	KEY_DUMMY,
	"Which color number to use for drawing targets on the radar.\n"
	"Valid values are all even color numbers bigger than or equal to 2.\n"
    },
    {
	"modifierBank1",
	NULL,
	"",
	KEY_DUMMY,
	"The default weapon modifier values for the first modifier bank.\n"
    },
    {
	"modifierBank2",
	NULL,
	"",
	KEY_DUMMY,
	"The default weapon modifier values for the second modifier bank.\n"
    },
    {
	"modifierBank3",
	NULL,
	"",
	KEY_DUMMY,
	"The default weapon modifier values for the third modifier bank.\n"
    },
    {
	"modifierBank4",
	NULL,
	"",
	KEY_DUMMY,
	"The default weapon modifier values for the fourth modifier bank.\n"
    },
    {
	"keyTurnLeft",
	NULL,
	"a",
	KEY_TURN_LEFT,
	"Turn left (anti-clockwise).\n"
    },
    {
	"keyTurnRight",
	NULL,
	"s",
	KEY_TURN_RIGHT,
	"Turn right (clockwise).\n"
    },
    {
	"keyThrust",
	NULL,
	"Shift_R Shift_L",
	KEY_THRUST,
	"Thrust.\n"
    },
    {
	"keyShield",
	NULL,
	"space Caps_Lock",
	KEY_SHIELD,
	"Raise or toggle the shield.\n"
    },
    {
	"keyFireShot",
	NULL,
	"Return Linefeed",
	KEY_FIRE_SHOT,
	"Fire shot.\n"
	"Note that shields must be down to fire.\n"
    },
    {
	"keyFireMissile",
	NULL,
	"backslash",
	KEY_FIRE_MISSILE,
	"Fire smart missile.\n"
    },
    {
	"keyFireTorpedo",
	NULL,
	"quoteright",
	KEY_FIRE_TORPEDO,
	"Fire unguided torpedo.\n"
    },
    {
	"keyFireHeat",
	NULL,
	"semicolon",
	KEY_FIRE_HEAT,
	"Fire heat seaking missile.\n"
    },
    {
	"keyFireLaser",
	NULL,
	"slash",
	KEY_FIRE_LASER,
	"Activate laser beam.\n"
    },
    {
	"keyDropMine",
	NULL,
	"Tab",
	KEY_DROP_MINE,
	"Drop a stationary mine.\n"
    },
    {
	"keyDetachMine",
	NULL,
	"bracketright",
	KEY_DETACH_MINE,
	"Detach a moving mine.\n"
    },
    {
	"keyDetonateMines",
	NULL,
	"equal",
	KEY_DETONATE_MINES,
	"Detonate all the mines you have dropped or thrown.\n"
    },
    {
	"keyLockClose",
	NULL,
	"Select Up",
	KEY_LOCK_CLOSE,
	"Lock on closest player.\n"
    },
    {
	"keyLockNextClose",
	NULL,
	"Down",
	KEY_LOCK_NEXT_CLOSE,
	"Lock on next closest player.\n"
    },
    {
	"keyLockNext",
	NULL,
	"Next Right",
	KEY_LOCK_NEXT,
	"Lock on next player.\n"
    },
    {
	"keyLockPrev",
	NULL,
	"Prior Left",
	KEY_LOCK_PREV,
	"Lock on previous player.\n"
    },
    {
	"keyRefuel",
	NULL,
	"f Control_L Control_R",
	KEY_REFUEL,
	"Refuel.\n"
    },
    {
	"keyRepair",
	NULL,
	"f",
	KEY_REPAIR,
	"Repair target.\n"
    },
    {
	"keyCloak",
	NULL,
	"Delete BackSpace",
	KEY_CLOAK,
	"Toggle cloakdevice.\n"
    },
    {
	"keyEcm",
	NULL,
	"bracketleft",
	KEY_ECM,
	"Use ECM.\n"
    },
    {
	"keySelfDestruct",
	NULL,
	"q",
	KEY_SELF_DESTRUCT,
	"Toggle self destruct.\n"
    },
    {
	"keyIdMode",
	NULL,
	"u",
	KEY_ID_MODE,
	"Toggle User mode (show real names).\n"
    },
    {
	"keyPause",
	NULL,
	"p Pause",
	KEY_PAUSE,
	"Toggle pause mode.\n"
	"When the ship is stationary on its homebase.\n"
    },
    {
	"keySwapSettings",
	NULL,
	"Escape",
	KEY_SWAP_SETTINGS,
	"Swap control settings.\n"
	"These are the power, turn speed and turn resistance settings.\n"
    },
    {
	"keyChangeHome",
	NULL,
	"Home h",
	KEY_CHANGE_HOME,
	"Change home base.\n"
	"When the ship is stationary on a new homebase.\n"
    },
    {
	"keyConnector",
	NULL,
	"Control_L",
	KEY_CONNECTOR,
	"Connect to a ball.\n"
    },
    {
	"keyDropBall",
	NULL,
	"d",
	KEY_DROP_BALL,
	"Drop a ball.\n"
    },
    {
	"keyTankNext",
	NULL,
	"e",
	KEY_TANK_NEXT,
	"Use the next tank.\n"
    },
    {
	"keyTankPrev",
	NULL,
	"w",
	KEY_TANK_PREV,
	"Use the the previous tank.\n"
    },
    {
	"keyTankDetach",
	NULL,
	"r",
	KEY_TANK_DETACH,
	"Detach the current tank.\n"
    },
    {
	"keyIncreasePower",
	NULL,
	"KP_Multiply",
	KEY_INCREASE_POWER,
	"Increase engine power.\n"
    },
    {
	"keyDecreasePower",
	NULL,
	"KP_Divide",
	KEY_DECREASE_POWER,
	"Decrease engine power.\n"
    },
    {
	"keyIncreaseTurnspeed",
	NULL,
	"KP_Add",
	KEY_INCREASE_TURNSPEED,
	"Increase turnspeed.\n"
    },
    {
	"keyDecreaseTurnspeed",
	NULL,
	"KP_Subtract",
	KEY_DECREASE_TURNSPEED,
	"Decrease turnspeed.\n"
    },
    {
	"keyTransporter",
	NULL,
	"t",
	KEY_TRANSPORTER,
	"Use transporter to steal an item.\n"
    },
    {
	"keyTalk",
	NULL,
	"m",
	KEY_TALK,
	"Toggle talk window.\n"
    },
    {
	"keyToggleNuclear",
	NULL,
	"n",
	KEY_TOGGLE_NUCLEAR,
	"Toggle nuclear weapon modifier.\n"
    },
    {
	"keyToggleCluster",
	NULL,
	"c",
	KEY_TOGGLE_CLUSTER,
	"Toggle cluster weapon modifier.\n"
    },
    {
	"keyToggleImplosion",
	NULL,
	"i",
	KEY_TOGGLE_IMPLOSION,
	"Toggle implosion weapon modifier.\n"
    },
    {
	"keyToggleVelocity",
	NULL,
	"v",
	KEY_TOGGLE_VELOCITY,
	"Toggle explosion velocity weapon modifier.\n"
    },
    {
	"keyToggleMini",
	NULL,
	"x",
	KEY_TOGGLE_MINI,
	"Toggle mini weapon modifier.\n"
    },
    {
	"keyToggleSpread",
	NULL,
	"z",
	KEY_TOGGLE_SPREAD,
	"Toggle spread weapon modifier.\n"
    },
    {
	"keyTogglePower",
	NULL,
	"b",
	KEY_TOGGLE_POWER,
	"Toggle power weapon modifier.\n"
    },
    {
	"keyToggleCompass",
	NULL,
	"y",
	KEY_TOGGLE_COMPASS,
	"Toggle HUD/radar compass lock.\n"
    },
    {
	"keyToggleAutoPilot",
	NULL,
	"h",
	KEY_TOGGLE_AUTOPILOT,
	"Toggle automatic pilot mode.\n"
    },
    {
	"keyToggleLaser",
	NULL,
	"l",
	KEY_TOGGLE_LASER,
	"Toggle laser modifier.\n"
    },
    {
	"keyEmergencyThrust",
	NULL,
	"j",
	KEY_EMERGENCY_THRUST,
	"Pull emergency thrust handle.\n"
    },
    {
	"keyEmergencyShield",
	NULL,
	"g",
	KEY_EMERGENCY_SHIELD,
	"Toggle emergency shield power.\n"
    },
    {
	"keyTractorBeam",
	NULL,
	"comma",
	KEY_TRACTOR_BEAM,
	"Use tractor beam in attract mode.\n"
    },
    {
	"keyPressorBeam",
	NULL,
	"period",
	KEY_PRESSOR_BEAM,
	"Use tractor beam in repulse mode.\n"
    },
    {
	"keyClearModifiers",
	NULL,
	"k",
	KEY_CLEAR_MODIFIERS,
	"Clear current weapon modifiers.\n"
    },
    {
	"keyLoadModifiers1",
	NULL,
	"1",
	KEY_LOAD_MODIFIERS_1,
	"Load the weapon modifiers from bank 1.\n"
    },
    {
	"keyLoadModifiers2",
	NULL,
	"2",
	KEY_LOAD_MODIFIERS_2,
	"Load the weapon modifiers from bank 2.\n"
    },
    {
	"keyLoadModifiers3",
	NULL,
	"3",
	KEY_LOAD_MODIFIERS_3,
	"Load the weapon modifiers from bank 3.\n"
    },
    {
	"keyLoadModifiers4",
	NULL,
	"4",
	KEY_LOAD_MODIFIERS_4,
	"Load the weapon modifiers from bank 4.\n"
    },
    {
	"keyToggleOwnedItems",
	NULL,
	"o",
	KEY_TOGGLE_OWNED_ITEMS,
	"Toggle list of owned items on HUD.\n"
    },
    {
	"keyToggleMessages",
	NULL,
	"0",
	KEY_TOGGLE_MESSAGES,
	"Toggle showing of messages.\n"
    },
    {
	"keyReprogram",
	NULL,
	"quoteleft",
	KEY_REPROGRAM,
	"Reprogram modifier or lock bank.\n"
    },
    {
	"keyLoadLock1",
	NULL,
	"5",
	KEY_LOAD_LOCK_1,
	"Load player lock from bank 1.\n"
    },
    {
	"keyLoadLock2",
	NULL,
	"6",
	KEY_LOAD_LOCK_2,
	"Load player lock from bank 2.\n"
    },
    {
	"keyLoadLock3",
	NULL,
	"7",
	KEY_LOAD_LOCK_3,
	"Load player lock from bank 3.\n"
    },
    {
	"keyLoadLock4",
	NULL,
	"8",
	KEY_LOAD_LOCK_4,
	"Load player lock from bank 4.\n"
    },
    {
	"keyPointerControl",
	NULL,
	"KP_Enter",
	KEY_POINTER_CONTROL,
	"Toggle pointer control.\n"
    },
    {
	"pointerButton1",
	NULL,
	"keyFireShot",
	KEY_DUMMY,
	"The key to activate when pressing the first mouse button.\n"
    },
    {
	"pointerButton2",
	NULL,
	"keyThrust",
	KEY_DUMMY,
	"The key to activate when pressing the second mouse button.\n"
    },
    {
	"pointerButton3",
	NULL,
	"keyShield",
	KEY_DUMMY,
	"The key to activate when pressing the third mouse button.\n"
    },
    {
	"pointerButton4",
	NULL,
	"",
	KEY_DUMMY,
	"The key to activate when pressing the fourth mouse button.\n"
    },
    {
	"pointerButton5",
	NULL,
	"",
	KEY_DUMMY,
	"The key to activate when pressing the fifth mouse button.\n"
    },
#ifdef SOUND
    {
	"sounds",
	NULL,
	SOUNDFILE,
	KEY_DUMMY,
	"Specifies the sound file.\n"
    },
    {
	"maxVolume",
	NULL,
	"100",
	KEY_DUMMY,
	"Specifies the volume to play sounds with.\n"
    },
    {
	"audioServer",
	NULL,
	"",
	KEY_DUMMY,
	"Specifies the audio server to use.\n"
    },
#endif
};


static int ON(char *optval)
{
    return (strncasecmp(optval, "true", 4) == 0
	    || strncasecmp(optval, "on", 2) == 0
	    || strncasecmp(optval, "yes", 3) == 0);
}


char* Get_keyHelpString(keys_t key)
{
    int			i;
    char		*nl;
    static char		buf[MAX_CHARS];

    for (i = 0; i < NELEM(options); i++) {
	if (options[i].key == key) {
	    strncpy(buf, options[i].help, sizeof buf);
	    buf[sizeof buf - 1] = '\0';
	    if ((nl = strchr(buf, '\n')) != NULL) {
		*nl = '\0';
	    }
	    return buf;
	}
    }

    return NULL;
}


char* Get_keyResourceString(keys_t key)
{
    int			i;

    for (i = 0; i < NELEM(options); i++) {
	if (options[i].key == key) {
	    return options[i].name;
	}
    }

    return NULL;
}


static void Usage(void)
{
    int			i;

    printf(
"Usage: xpilot [-options ...] [server]\n"
"Where options include:\n"
"\n"
	  );
    for (i = 0; i < NELEM(options); i++) {
	printf("    -%s %s\n", options[i].name,
	       (options[i].noArg == NULL) ? "<value>" : "");
	if (options[i].help && options[i].help[0]) {
	    char *str;
	    printf("        ");
	    for (str = options[i].help; *str; str++) {
		putchar(*str);
		if (*str == '\n' && str[1]) {
		    printf("        ");
		}
	    }
	    if (str[-1] != '\n') {
		putchar('\n');
	    }
	}
	if (options[i].fallback && options[i].fallback[0]) {
	    printf("        The default value is: %s.\n", options[i].fallback);
	}
	printf("\n");
    }
    printf(
"Most of these options can also be set in the .xpilotrc file\n"
"in your home directory.\n"
"Each key option may have mutliple keys bound to it and\n"
"one key may be used by multiple key options.\n"
"If no server is specified then xpilot will search\n"
"for servers on your local network\n"
"For a listing of remote servers try: telnet xpilot.cs.uit.no 4400 \n"
	  );

    exit(1);
}


static int Find_resource(XrmDatabase db, char *resource,
			 char *result, unsigned size, int *index)
{
    int			i,
			len;
    char		str_name[80],
			str_class[80],
			*str_type[10];
    XrmValue		rmValue;

    for (i = 0;;) {
	if (!strcmp(resource, options[i].name)) {
	    *index = i;
	    break;
	}
	if (++i >= NELEM(options)) {
	    errno = 0;
	    error("BUG: Can't find option %s", resource);
	    exit(1);
	}
    }

    sprintf(str_name, "%s.%s", myName, resource);
    sprintf(str_class, "%s.%c%s", myClass,
	    isupper(*resource) ? toupper(*resource) : *resource, resource + 1);

    if (XrmGetResource(db, str_name, str_class, str_type, &rmValue) == True) {
	if (rmValue.addr == NULL) {
	    len = 0;
	} else {
	    len = MIN(rmValue.size, size - 1);
	    strncpy(result, rmValue.addr, len);
	}
	result[len] = '\0';
	return 1;
    }
    strncpy(result, options[*index].fallback, size);
    result[size - 1] = '\0';
    return 0;
}


static int Get_resource(XrmDatabase db,
			char *resource, char *result, unsigned size)
{
    int			index;

    return Find_resource(db, resource, result, size, &index);
}


static int Get_string_resource(XrmDatabase db,
			       char *resource, char *result, unsigned size)
{
    char		*src, *dst;
    int			index, val;

    val = Find_resource(db, resource, result, size, &index);
    src = dst = result;
    while ((*src & 0x7f) == *src && isgraph(*src) == 0 && *src != '\0') {
	src++;
    }
    while ((*src & 0x7f) != *src || isgraph(*src) != 0) {
	*dst++ = *src++;
    }
    *dst = '\0';

    return val;
}


static void Get_int_resource(XrmDatabase db,
			     char *resource, int *result)
{
    int			index;
    char		resValue[MAX_CHARS];

    Find_resource(db, resource, resValue, sizeof resValue, &index);
    if (sscanf(resValue, "%d", result) <= 0) {
	errno = 0;
	error("Bad value \"%s\" for option \"%s\", using default...",
	      resValue, resource);
	sscanf(options[index].fallback, "%d", result);
    }
}


static void Get_float_resource(XrmDatabase db,
			       char *resource, float *result)
{
    int			index;
    char		resValue[MAX_CHARS];

    Find_resource(db, resource, resValue, sizeof resValue, &index);
    if (sscanf(resValue, "%f", result) <= 0) {
	errno = 0;
	error("Bad value \"%s\" for option \"%s\", using default...",
	      resValue, resource);
	sscanf(options[index].fallback, "%f", result);
    }
}


static void Get_bool_resource(XrmDatabase db, char *resource, int *result)
{
    int			index;
    char		resValue[MAX_CHARS];

    Find_resource(db, resource, resValue, sizeof resValue, &index);
    *result = (ON(resValue) != 0);
}


static void Get_bit_resource(XrmDatabase db, char *resource,
			     long *mask, int bit)
{
    int			index;
    char		resValue[MAX_CHARS];

    Find_resource(db, resource, resValue, sizeof resValue, &index);
    if (ON(resValue)) {
	SET_BIT(*mask, bit);
    }
}


static void Get_file_defaults(XrmDatabase *rDBptr)
{
    int			len;
    char		*ptr,
			*lang = getenv("LANG"),
			*home = getenv("HOME"),
			path[MAXPATHLEN];
    XrmDatabase		tmpDB;

    sprintf(path, "%s%s", LIBDIR, myClass);
    *rDBptr = XrmGetFileDatabase(path);

#ifdef VMS
    /*
     * None of the paths generated will be valid VMS file names.
     */
    tmpDB = XrmGetFileDatabase("SYS$LOGIN:decw$xdefaults.dat");
    XrmMergeDatabases(tmpDB, rDBptr);
    tmpDB = XrmGetFileDatabase("DECW$USER_DEFAULTS:xpilot.dat");
    XrmMergeDatabases(tmpDB, rDBptr);
#else
    if (lang != NULL) {
	sprintf(path, "/usr/lib/X11/%s/app-defaults/%s", lang, myClass);
	if (access(path, 0) == -1) {
	    sprintf(path, "/usr/lib/X11/app-defaults/%s", myClass);
	}
    } else {
	sprintf(path, "/usr/lib/X11/app-defaults/%s", myClass);
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
	    sprintf(path, "%s/app-defaults/%s/%s", home, lang, myClass);
	    if (access(path, 0) == -1) {
		sprintf(path, "%s/app-defaults/%s", home, myClass);
	    }
	} else {
	    sprintf(path, "%s/app-defaults/%s", home, myClass);
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
#endif
}


void Parse_options(int *argcp, char **argvp, char *realName, int *port,
		   int *my_team, int *list, int *join, int *noLocalMotd,
		   char *nickName, char *dispName, char *shut_msg)
{
    int			i,
			j,
			firstKeyDef,
			size,
			num;
    keys_t		key;
    KeySym		ks;
    char		*ptr,
			*str,
			resValue[2*MSG_LEN];
    XrmDatabase		argDB = 0, rDB;
    XrmOptionDescRec	*xopt;

    XrmInitialize();

    /*
     * Construct a Xrm Option table from our options array.
     */
    size = sizeof(*xopt) * NELEM(options);
    for (i = 0; i < NELEM(options); i++) {
	size += 2 * (strlen(options[i].name) + 2);
    }
    if ((ptr = (char *)malloc(size)) == NULL) {
	error("No memory for options");
	exit(1);
    }
    xopt = (XrmOptionDescRec *)ptr;
    ptr += sizeof(*xopt) * NELEM(options);
    for (i = 0; i < NELEM(options); i++) {
	xopt[i].option = ptr;
	xopt[i].option[0] = '-';
	strcpy(&xopt[i].option[1], options[i].name);
	size = strlen(options[i].name) + 2;
	ptr += size;
	xopt[i].specifier = ptr;
	xopt[i].specifier[0] = '.';
	strcpy(&xopt[i].specifier[1], options[i].name);
	ptr += size;
	if (options[i].noArg) {
	    xopt[i].argKind = XrmoptionNoArg;
	    xopt[i].value = options[i].noArg;
	}
	else {
	    xopt[i].argKind = XrmoptionSepArg;
	    xopt[i].value = NULL;
	}
    }

    XrmParseCommand(&argDB, xopt, NELEM(options), myName, argcp, argvp);

    /*
     * Check for bad arguments.
     */
    for (i = 1; i < *argcp; i++) {
	if (argvp[i][0] == '-' || argvp[i][0] == '+') {
	    errno = 0;
	    error("Unknown or incomplete option '%s'", argvp[i]);
	    error("Type: %s -help to see a list of options", argvp[0]);
	    exit(1);
	}
	/* The rest of the arguments are hostnames of servers. */
    }

    if (Get_resource(argDB, "help", resValue, sizeof resValue) != 0) {
	Usage();
    }

    if (Get_resource(argDB, "version", resValue, sizeof resValue) != 0) {
	puts(TITLE);
	exit(0);
    }

    Get_resource(argDB, "shutdown", shut_msg, MAX_CHARS);

    if (Get_string_resource(argDB, "display", dispName, MAX_DISP_LEN) == 0
	|| dispName[0] == '\0') {
	if ((ptr = getenv(DISPLAY_ENV)) != NULL) {
	    strncpy(dispName, ptr, MAX_DISP_LEN);
	    dispName[MAX_DISP_LEN - 1] = '\0';
	} else {
	    strcpy(dispName, DISPLAY_DEF);
	}
    }
    if ((dpy = XOpenDisplay(dispName)) == NULL) {
	error("Can't open display '%s'", dispName);
	if (strcmp(dispName, "NO_X") == 0) {
	    /* user does not want X stuff.  experimental.  use at own risk. */
	    strcpy(nickName, realName);
	    *my_team = TEAM_NOT_SET;
	    Get_int_resource(argDB, "port", port);
	    Get_bool_resource(argDB, "list", list);
	    *join = false;
	    *noLocalMotd = true;
	    XrmDestroyDatabase(argDB);
	    free(xopt);
	    return;
	}
	exit(1);
    }

    if (Get_string_resource(argDB, "keyboard", resValue, MAX_DISP_LEN) == 0
	|| resValue[0] == '\0') {
	if ((ptr = getenv(KEYBOARD_ENV)) != NULL) {
	    strncpy(resValue, ptr, MAX_DISP_LEN);
	    resValue[MAX_DISP_LEN - 1] = '\0';
	}
    }
    if (resValue[0] == '\0') {
	kdpy = NULL;
    } else if ((kdpy = XOpenDisplay(resValue)) == NULL) {
	error("Can't open keyboard '%s'", resValue);
	exit(1);
    }

    Get_resource(argDB, "visual", visualName, sizeof visualName);
    if (strncasecmp(visualName, "list", 4) == 0) {
	List_visuals();
	exit(0);
    }

    Get_file_defaults(&rDB);

    XrmMergeDatabases(argDB, &rDB);

    Get_string_resource(rDB, "geometry", resValue, sizeof resValue);
    geometry = strdup(resValue);

    Get_resource(rDB, "name", nickName, MAX_NAME_LEN);
    if (!nickName[0]) {
	strcpy(nickName, realName);
    }
    CAP_LETTER(nickName[0]);
    if (nickName[0] < 'A' || nickName[0] > 'Z') {
	errno = 0;
	error("Your player name \"%s\" should start with an uppercase letter",
	    nickName);
	exit(1);
    }
    strncpy(realname, realName, sizeof(realname) - 1);
    strncpy(name, nickName, sizeof(name) - 1);

    Get_int_resource(rDB, "team", my_team);
    if (*my_team < 0 || *my_team > 9) {
	*my_team = TEAM_NOT_SET;
    }
    team = *my_team;

    Get_int_resource(rDB, "port", port);
    Get_bool_resource(rDB, "list", list);
    Get_bool_resource(rDB, "join", join);
    Get_bool_resource(rDB, "noLocalMotd", noLocalMotd);
    Get_bool_resource(rDB, "autoServerMotdPopup", &i);
    autoServerMotdPopup = (i != 0) ? true : false;

    Get_resource(rDB, "shipShape", resValue, sizeof resValue);
    shipShape = strdup(resValue);
    Validate_shape_str(shipShape);

    Get_float_resource(rDB, "power", &power);
    Get_float_resource(rDB, "turnSpeed", &turnspeed);
    Get_float_resource(rDB, "turnResistance", &turnresistance);
    Get_float_resource(rDB, "altPower", &power_s);
    Get_float_resource(rDB, "altTurnSpeed", &turnspeed_s);
    Get_float_resource(rDB, "altTurnResistance", &turnresistance_s);

    Get_float_resource(rDB, "sparkProb", &spark_prob);
    spark_rand = (int)(spark_prob * MAX_SPARK_RAND + 0.5f);
    Get_int_resource(rDB, "charsPerSecond", &charsPerSecond);
    Get_bool_resource(rDB, "markingLights", &i);
    markingLights = (i == false) ? false : true;

    Get_int_resource(rDB, "backgroundPointDist", &map_point_distance);
    Get_int_resource(rDB, "backgroundPointSize", &map_point_size);
    LIMIT(map_point_size, MIN_MAP_POINT_SIZE, MAX_MAP_POINT_SIZE);
    Get_int_resource(rDB, "sparkSize", &spark_size);
    LIMIT(spark_size, MIN_SPARK_SIZE, MAX_SPARK_SIZE);
    Get_int_resource(rDB, "shotSize", &shot_size);
    LIMIT(shot_size, MIN_SHOT_SIZE, MAX_SHOT_SIZE);
    Get_int_resource(rDB, "teamShotSize", &teamshot_size);
    LIMIT(teamshot_size, MIN_TEAMSHOT_SIZE, MAX_TEAMSHOT_SIZE);
    Get_bool_resource(rDB, "titleFlip", &titleFlip);
    /*
     * This is a special value; default or not defined means choose depending
     * on the display, otherwise its a boolean value.
     */
    Get_string_resource(rDB, "shieldDrawSolid", resValue, sizeof resValue);
    if (strncasecmp(resValue, "default", 7) == 0)
	shieldDrawMode = -1;
    else
	shieldDrawMode = ON(resValue);
    Get_bool_resource(rDB, "toggleShield", &toggle_shield);

    Get_resource(rDB, "modifierBank1", modBankStr[0], sizeof modBankStr[0]);
    Get_resource(rDB, "modifierBank2", modBankStr[1], sizeof modBankStr[1]);
    Get_resource(rDB, "modifierBank3", modBankStr[2], sizeof modBankStr[2]);
    Get_resource(rDB, "modifierBank4", modBankStr[3], sizeof modBankStr[3]);

    Get_resource(rDB, "visual", visualName, sizeof visualName);
    Get_bool_resource(rDB, "mono", &i);
    mono = (i != 0) ? true : false;
    Get_bool_resource(rDB, "colorSwitch", &i);
    colorSwitch = (i != 0) ? true : false;
    Get_int_resource(rDB, "maxColors", &maxColors);
    Get_string_resource(rDB, "black", color_names[0], sizeof(color_names[0]));
    Get_string_resource(rDB, "white", color_names[1], sizeof(color_names[1]));
    Get_string_resource(rDB, "blue", color_names[2], sizeof(color_names[2]));
    Get_string_resource(rDB, "red", color_names[3], sizeof(color_names[3]));
    for (i = 0; i < MAX_COLORS; i++) {
	char buf[16];
	sprintf(buf, "color%d", i);
	if (!Get_string_resource(rDB, buf, resValue, MAX_COLOR_LEN)) {
	    if (i < NUM_COLORS) {
		strcpy(resValue, color_names[i]);
	    }
	}
	strcpy(color_names[i], resValue);
    }
    Get_int_resource(rDB, "hudColor", &hudColor);
    Get_int_resource(rDB, "targetRadarColor", &targetRadarColor);

    instruments = 0;
    Get_bit_resource(rDB, "showShipName", &instruments, SHOW_SHIP_NAME);
    Get_bit_resource(rDB, "showMineName", &instruments, SHOW_MINE_NAME);
    Get_bit_resource(rDB, "showMessages", &instruments, SHOW_MESSAGES);
    Get_bit_resource(rDB, "showHUD", &instruments, SHOW_HUD_INSTRUMENTS);
    Get_bit_resource(rDB, "verticalHUDLine", &instruments, SHOW_HUD_VERTICAL);
    Get_bit_resource(rDB, "horizontalHUDLine", &instruments, SHOW_HUD_HORIZONTAL);
    Get_bit_resource(rDB, "fuelMeter", &instruments, SHOW_FUEL_METER);
    Get_bit_resource(rDB, "fuelGauge", &instruments, SHOW_FUEL_GAUGE);
    Get_bit_resource(rDB, "turnSpeedMeter", &instruments, SHOW_TURNSPEED_METER);
    Get_bit_resource(rDB, "powerMeter", &instruments, SHOW_POWER_METER);
    Get_bit_resource(rDB, "packetSizeMeter", &instruments, SHOW_PACKET_SIZE_METER);
    Get_bit_resource(rDB, "packetLossMeter", &instruments, SHOW_PACKET_LOSS_METER);
    Get_bit_resource(rDB, "packetDropMeter", &instruments, SHOW_PACKET_DROP_METER);
    Get_bit_resource(rDB, "slidingRadar", &instruments, SHOW_SLIDING_RADAR);
    Get_bit_resource(rDB, "outlineWorld", &instruments, SHOW_OUTLINE_WORLD);
    Get_bit_resource(rDB, "showItems", &instruments, SHOW_ITEMS);
    Get_bit_resource(rDB, "clock", &instruments, SHOW_CLOCK);

    Get_bool_resource(rDB, "pointerControl", &initialPointerControl);
    Get_float_resource(rDB, "showItemsTime", &showItemsTime);
    LIMIT(showItemsTime, MIN_SHOW_ITEMS_TIME, MAX_SHOW_ITEMS_TIME);

    Get_float_resource(rDB, "speedFactHUD", &hud_move_fact);
    Get_float_resource(rDB, "speedFactPTR", &ptr_move_fact);
    Get_int_resource(rDB, "fuelNotify", &fuelLevel3);
    Get_int_resource(rDB, "fuelWarning", &fuelLevel2);
    Get_int_resource(rDB, "fuelCritical", &fuelLevel1);

    Get_resource(rDB, "gameFont", gameFontName, sizeof gameFontName);
    Get_resource(rDB, "messageFont", messageFontName, sizeof messageFontName);
    Get_resource(rDB, "scoreListFont", scoreListFontName, sizeof scoreListFontName);
    Get_resource(rDB, "buttonFont", buttonFontName, sizeof buttonFontName);
    Get_resource(rDB, "textFont", textFontName, sizeof textFontName);
    Get_resource(rDB, "talkFont", talkFontName, sizeof talkFontName);
    Get_resource(rDB, "motdFont", motdFontName, sizeof motdFontName);

    Get_int_resource(rDB, "receiveWindowSize", &receive_window_size);
    LIMIT(receive_window_size, MIN_RECEIVE_WINDOW_SIZE, MAX_RECEIVE_WINDOW_SIZE);

#ifdef SOUND
    Get_string_resource(rDB, "sounds", sounds, sizeof sounds);
    Get_int_resource(rDB, "maxVolume", &maxVolume);
    Get_resource(rDB, "audioServer", audioServer, sizeof audioServer);
#endif

    /*
     * Key bindings
     */
    maxKeyDefs = 2 * NUM_KEYS;
    if (!(keyDefs = (keydefs_t *)malloc(maxKeyDefs * sizeof(keydefs_t)))) {
	error("No memory for key bindings");
	exit(1);
    }
    num = 0;
    for (i = 0; i < NELEM(options); i++) {
	if ((key = options[i].key) == KEY_DUMMY) {
	    continue;
	}
	Get_resource(rDB, options[i].name, resValue, sizeof resValue);
	firstKeyDef = num;
	for (str = strtok(resValue, " \t\r\n");
	     str != NULL;
	     str = strtok(NULL, " \t\r\n")) {

	    if ((ks = XStringToKeysym(str)) == NoSymbol) {
		printf("Invalid keysym \"%s\" for key \"%s\".\n",
		       str, options[i].name);
		continue;
	    }

	    for (j = firstKeyDef; j < num; j++) {
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
		if (!(keyDefs = (keydefs_t *)
			realloc(keyDefs, maxKeyDefs * sizeof(keydefs_t)))) {
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
	if (!(keyDefs = (keydefs_t *)
		realloc(keyDefs, maxKeyDefs * sizeof(keydefs_t)))) {
	    error("No memory for key bindings");
	    exit(1);
	}
    }

    /*
     * Pointer button bindings
     */
    for (i = 0; i < MAX_POINTER_BUTTONS; i++) {
	sprintf(resValue, "pointerButton%d", i+1);
	Get_resource(rDB, resValue, resValue, sizeof resValue);
	ptr = resValue;
	if (*ptr != '\0') {
	    if (!strncasecmp(ptr, "key", 3))
		ptr += 3;
	    for (j = 0; j < NELEM(options); j++) {
		if (options[j].key != KEY_DUMMY) {
		    if (!strcasecmp(ptr, options[j].name + 3)) {
			buttonDefs[i] = options[j].key;
			break;
		    }
		}
	    }
	    if (j == NELEM(options)) {
		errno = 0;
		error("Unknown key \"%s\" for pointer button %d", resValue, i);
	    }
	}
    }

    XrmDestroyDatabase(rDB);

    free(xopt);

#ifdef SOUND
    audioInit(dispName);
#endif /* SOUND */
}


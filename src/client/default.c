/* $Id: default.c,v 5.11 2001/06/22 05:27:42 dik Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>

#ifndef _WINDOWS
# include <unistd.h>
# include <X11/Xos.h>
# include <X11/keysym.h>
# include <X11/Xlib.h>
# include <X11/Xresource.h>
# ifdef	__apollo
#  include <X11/ap_keysym.h>
# endif
# include <sys/param.h>
#endif

#ifdef _WINDOWS
# include "NT/winX.h"
# include "NT/winXXPilot.h"
# include "NT/winConfig.h"
#endif

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
#include "types.h"
#include "protoclient.h"
#include "audio.h"
#include "commonproto.h"
#include "portability.h"
#include "talk.h"
#include "default.h"
#include "checknames.h"


char default_version[] = VERSION;



#define DISPLAY_ENV	"DISPLAY"
#define DISPLAY_DEF	":0.0"
#define KEYBOARD_ENV	"KEYBOARD"

#ifndef PATH_MAX
#define PATH_MAX	1023
#endif

/*
 * Default fonts
 */
#define GAME_FONT	"-*-times-*-*-*--18-*-*-*-*-*-iso8859-1"
#define MESSAGE_FONT	"-*-times-*-*-*--14-*-*-*-*-*-iso8859-1"
#define SCORE_LIST_FONT	"-*-fixed-bold-*-*--15-*-*-*-c-*-iso8859-1"
#define BUTTON_FONT	"-*-*-bold-o-*--14-*-*-*-*-*-iso8859-1"
#define TEXT_FONT	"-*-*-bold-i-*--14-*-*-*-p-*-iso8859-1"
#define TALK_FONT	"-*-fixed-bold-*-*--15-*-*-*-c-*-iso8859-1"
#define KEY_LIST_FONT	"-*-fixed-medium-r-*--10-*-*-*-c-*-iso8859-1"
#define MOTD_FONT	"-*-courier-bold-r-*--14-*-*-*-*-*-iso8859-1"


char			myName[] = "xpilot";
char			myClass[] = "XPilot";


#ifdef SPARC_CMAP_HACK
char  frameBuffer[MAX_CHARS]; /* frame buffer */
#endif



extern char *talk_fast_msgs[];	/* talk macros */
char talk_fast_temp_buf[7];		/* can handle up to 999 fast msgs */
char *talk_fast_temp_buf_big;


static void Get_test_resources(XrmDatabase rDB);


keys_t buttonDefs[MAX_POINTER_BUTTONS];

/* from common/config.c */
extern char conf_ship_file_string[];
extern char conf_texturedir_string[];
extern char conf_soundfile_string[];

/*
 * Structure to store all the client options.
 * The most important field is the help field.
 * It is used to self-document the client to
 * the user when "xpilot -help" is issued.
 * Help lines can span multiple lines, but for
 * the key help window only the first line is used.
 */
option options[] = {
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
	"user",
	NULL,
	"",
	KEY_DUMMY,
	"Set the realname.\n"
    },
    {
	"host",
	NULL,
	"",
	KEY_DUMMY,
	"Set the hostname.\n"
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
#ifdef _WINDOWS
	"No",			/* temporary till i straighten out the motd woes. */
#else
	"Yes",
#endif
	KEY_DUMMY,
	"Automatically popup the MOTD of the server on startup.\n"
    },
    {
	"refreshMotd",
	NULL,
	"No",
	KEY_DUMMY,
	"Get a fresh copy of the server MOTD every time it is displayed.\n"
    },
    {
	"text",
	"Yes",
	"No",
	KEY_DUMMY,
	"Use the simple text interface to contact a server\n"
	"instead of the graphical user interface.\n"
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
	"ignoreWindowManager",
	NULL,
	"",
	KEY_DUMMY,
	"Ignore the window manager when opening the top level player window.\n"
	"This can be handy if you want to have your XPilot window on a preferred\n"
	"position without window manager borders.  Also sometimes window managers\n"
	"may interfere when switching colormaps.  This option may prevent that.\n"
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
	"The exact format is defined in the file doc/README.SHIPS in the XPilot\n"
	"distribution.  Note that there is a nifty Unix tool called editss for\n"
	"easy ship creation.  There is XPShipEditor for Windows\n"
	"and Ship Shaper for Java.  See the XPilot FAQ for details.\n"
	"See also the \"shipShapeFile\" option below.\n"
    },
    {
	"shipShapeFile",
	NULL,
	conf_ship_file_string,
	KEY_DUMMY,
	"An optional file where shipshapes can be stored.\n"
	"If this resource is defined and it refers to an existing file\n"
	"then shipshapes can be referenced to by their name.\n"
	"For instance if you define shipShapeFile to be\n"
	"/home/myself/.shipshapes and this file contains one or more\n"
	"shipshapes then you can select the shipshape by starting xpilot as:\n"
	"	xpilot -shipShape myshipshapename\n"
	"Where \"myshipshapename\" should be the \"name:\" or \"NM:\" of\n"
	"one of the shipshapes defined in /home/myself/.shipshapes.\n"
	"Each shipshape definition should be defined on only one line,\n"
	"where all characters up to the first left parenthesis don't matter.\n"
	/* shipshopshapshepshit getting nuts from all these shpshp-s. */
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
	"filledWorld",
	NULL,
	"No",
	KEY_DUMMY,
	"Draws the walls solid, filled with one color.\n"
	"Be warned that this option needs fast graphics.\n"
    },
    {
	"texturedWalls",
	NULL,
	"No",
	KEY_DUMMY,
	"Draws the walls filled with a texture pattern.\n"
	"See also the wallTextureFile option.\n"
	"Be warned that this needs a very fast graphics system.\n"
    },
    {
	"wallTextureFile",
	NULL,
	"",
	KEY_DUMMY,
	"Specify a XPM format pixmap file to load the wall texture from.\n"
    },
    {
	"texturePath",
	NULL,
	conf_texturedir_string,
	KEY_DUMMY,
	"Search path for texture files.\n"
	"This is a list of one or more directories separated by colons.\n"
    },
    {
	"texturedObjects",
	NULL,
	"No",
	KEY_DUMMY,
	"Whether to draw ships, shots and walls with textures.\n"
	"Be warned that this needs a very fast graphics system.\n"
	"You may also need to enable multibuffering or double-buffering.\n"
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
	"Valid values are in the range [0.0-1.0]\n"
    },
    {
	"sparkSize",
	NULL,
	"2",
	KEY_DUMMY,
	"Size of sparks in pixels.\n"
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
	"clockAMPM",
	NULL,
	"No",
	KEY_DUMMY,
	"12 or 24 hour format for clock display.\n",
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
	"maxMessages",
	NULL,
	"8",
	KEY_DUMMY,
	"The maximum number of messages to display.\n"
    },
    {
	"reverseScroll",
	NULL,
	"No",
	KEY_DUMMY,
	"Reverse scroll direction of messages.\n"
    },
#ifndef _WINDOWS
    {
	"selectionAndHistory",
	NULL,
	"Yes",
	KEY_DUMMY,
	"Provide cut&paste for the player messages and the talk window and\n"
	"a `history' for the talk window.\n"
    },
    {
	"maxLinesInHistory",
	NULL,
	"32",
	KEY_DUMMY,
	"Number of your messages saved in the `history' of the talk window.\n"
	"`history' is accessible with `keyTalkCursorUp/Down'.\n"
    },
#endif
    {
	"shotSize",
	NULL,
	"3",
	KEY_DUMMY,
	"The size of shots in pixels.\n"
    },
    {
	"teamShotSize",
	NULL,
	"2",
	KEY_DUMMY,
	"The size of team shots in pixels.\n"
	"Note that team shots are drawn in blue.\n"
    },
    {
	"showNastyShots",
	NULL,
	"No",
	KEY_DUMMY,
	"Use the new Nasty Looking Shots or the original rectangle shots,\n"
	"You will probably want to increase your shotSize if you use this.\n"
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
	"flipping title bars.  Hence this option to turn it off.\n"
    },
    {
	"toggleShield",
	NULL,
	"No",
	KEY_DUMMY,
	"Are shields toggled by a keypress only?\n"
    },
    {
	"autoShield", /* Don auto-shield hack */
	NULL,
	"Yes",
	KEY_DUMMY,
	"Are shields lowered automatically for weapon fire?\n"
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
	"Should owned items be displayed permanently on the HUD,\n"
	"or only when their amount has changed?\n"
    },
    {
	"showItemsTime",
	NULL,
	"2.0",
	KEY_DUMMY,
	"The time in seconds to display item information when\n"
	"it has changed and the showItems option is turned on.\n"
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
	"erase",
	"Yes",
	"",
	KEY_DUMMY,
	"Use Erase(-hack) for slow X.\n"
    },
    {
	"colorSwitch",
	NULL,
	"Yes",
	KEY_DUMMY,
	"Use color buffering or not.\n"
	"Usually color buffering is faster, especially on 8-bit PseudoColor displays.\n"
    },
    {
	"multibuffer",
	NULL,
	"No",
	KEY_DUMMY,
	"Use the X windows multibuffer extension if present.\n"
    },
    {
	"maxColors",
	NULL,
	"8",
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
	"hudLockColor",
	NULL,
	"4",
	KEY_DUMMY,
	"Which color number to use for drawing the lock on the HUD.\n"
    },
    {
	"wallColor",
	NULL,
	"2",
	KEY_DUMMY,
	"Which color number to use for drawing walls.\n"
    },
    {
	"wallRadarColor",
	NULL,
	"2",
	KEY_DUMMY,
	"Which color number to use for drawing walls on the radar.\n"
	"Valid values all even numbers smaller than maxColors.\n"
    },
    {
	"decorColor",
	NULL,
	"6",
	KEY_DUMMY,
	"Which color number to use for drawing decorations.\n"
    },
    {
	"showDecor",
	NULL,
	"Yes",
	KEY_DUMMY,
	"Should decorations be displayed on the screen and radar?\n"
    },
    {
	"decorRadarColor",
	NULL,
	"6",
	KEY_DUMMY,
	"Which color number to use for drawing decorations on the radar.\n"
	"Valid values are all even numbers smaller than maxColors.\n"
    },
    {   
	"oldMessagesColor",
	NULL,
	"1",
	KEY_DUMMY,
	"Which color number to use for drawing old messages.\n"
    },
    {
	"outlineDecor",
	NULL,
	"No",
	KEY_DUMMY,
	"Draws only the outline of the map decoration.\n"
    },
    {
	"filledDecor",
	NULL,
	"No",
	KEY_DUMMY,
	"Draws filled decorations.\n"
    },
    {
	"texturedDecor",
	NULL,
	"No",
	KEY_DUMMY,
	"Draws the map decoration filled with a texture pattern.\n"
	"See also the decorTextureFile and texturedWalls options.\n"
    },
    {
	"decorTextureFile",
	NULL,
	"",
	KEY_DUMMY,
	"Specify a XPM format pixmap file to load the decor texture from.\n"
    },
    {
	"texturedBalls",
	NULL,
	"No",
	KEY_DUMMY,
	"Draw the balls with a texture specified by the ballTextureFile option.\n"
    },
    {
	"ballTextureFile",
	NULL,
	"",
	KEY_DUMMY,
	"Specify a XPM format pixmap file to load the ball texture from.\n"
    },
    {
	"targetRadarColor",
	NULL,
	"4",
	KEY_DUMMY,
	"Which color number to use for drawing targets on the radar.\n"
	"Valid values are all even numbers smaller than maxColors.\n"
    },
    {
	"sparkColors",
	NULL,
	"5,6,7,3",
	KEY_DUMMY,
	"Which color numbers to use for spark and debris particles.\n"
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
	"Fire heatseeking missile.\n"
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
	"Detonate the mine you have dropped or thrown, which is closest to you.\n"
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
	"End",
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
	"Pause",
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
        "keySwapScaleFactor",
        NULL,
        "",
        KEY_SWAP_SCALEFACTOR,
        "Swap scalefactor settings.\n"
        "These are the scalefactor settings.\n"
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
	"keyDeflector",
	NULL,
	"o",
	KEY_DEFLECTOR,
	"Toggle deflector.\n"
    },
    {
	"keyHyperJump",
	NULL,
	"q",
	KEY_HYPERJUMP,
	"Teleport.\n"
    },
    {
	"keyPhasing",
	NULL,
	"p",
	KEY_PHASING,
	"Use phasing device.\n"
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
	"KP_7",
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
	"KP_8",
	KEY_TOGGLE_OWNED_ITEMS,
	"Toggle list of owned items on HUD.\n"
    },
    {
	"keyToggleMessages",
	NULL,
	"KP_9",
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
	"keyToggleRecord",
	NULL,
	"KP_5",
	KEY_TOGGLE_RECORD,
	"Toggle recording of session (see recordFile).\n"
    },
    {
	"keySelectItem",
	NULL,
	"KP_0 KP_Insert",
	KEY_SELECT_ITEM,
	"Select an item to lose.\n"
    },
    {
	"keyLoseItem",
	NULL,
	"KP_Delete KP_Decimal",
	KEY_LOSE_ITEM,
	"Lose the selected item.\n"
    },
#ifndef _WINDOWS
    {
	"keyPrintMessagesStdout",
	NULL,
	"Print",
	KEY_PRINT_MSGS_STDOUT,
	"Print the current messages to stdout.\n"
    },
    {
	"keyTalkCursorLeft",
	NULL,
	"Left",
	KEY_TALK_CURSOR_LEFT,
	"Move Cursor to the left in the talk window.\n"
    },
    {
	"keyTalkCursorRight",
	NULL,
	"Right",
	KEY_TALK_CURSOR_RIGHT,
	"Move Cursor to the right in the talk window.\n"
    },
    {
	"keyTalkCursorUp",
	NULL,
	"Up",
	KEY_TALK_CURSOR_UP,
	"Browsing in the history of the talk window.\n"
    },
    {
	"keyTalkCursorDown",
	NULL,
	"Down",
	KEY_TALK_CURSOR_DOWN,
	"Browsing in the history of the talk window.\n"
    },
#endif
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
    {
	"maxFPS",
	NULL,
	"20",
	KEY_DUMMY,
	"Set client's maximum FPS supported.\n"
    },
    {
	"recordFile",
	NULL,
	"",
	KEY_DUMMY,
	"An optional file where a recording of a game can be made.\n"
	"If this file is undefined then recording isn't possible.\n"
    },
    {
	"clientPortStart",
	NULL,
	"0",
	KEY_DUMMY,
	"Use UDP ports clientPortStart - clientPortEnd (for firewalls).\n"
    },
    {
	"clientPortEnd",
	NULL,
	"0",
	KEY_DUMMY,
	"Use UDP ports clientPortStart - clientPortEnd (for firewalls).\n"
    },
#ifdef _WINDOWS
    {
	"threadedDraw",
	NULL,
	"No",
	KEY_DUMMY,
	"Tell Windows to do the heavy BitBlt in another thread\n"
	},
    {
	"radarDivisor",
	NULL,
	"1",
	KEY_DUMMY,
	"Specifies how many frames between radar window updates.\n"
	},
#endif
#ifdef	WINDOWSCALING
    {
	"scaleFactor",
	NULL,
	"1.0",
	KEY_DUMMY,
	"Specifies scaling factor for the drawing window.\n"
    },
    {
        "altScaleFactor",
        NULL,
        "2.0",
        KEY_DUMMY,
        "Specifies alternative scaling factor for the drawing window.\n"
    },
#endif
#ifdef SOUND
    {
	"sounds",
	NULL,
	conf_soundfile_string,
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
#ifdef SPARC_CMAP_HACK
    {
	"frameBuffer",
	NULL,
	"/dev/fb",
	KEY_DUMMY,
	"Specifies the device name of the frame buffer.\n"
    },
#endif    
#ifdef DEVELOPMENT
    {
        "test",
        NULL,
        "",
        KEY_DUMMY,
        "Which development testing parameters to use?\n"
    },
#endif
/* talk macros: */
    {
	"keySendMsg1",
	NULL,
	"F1",
	KEY_MSG_1,
	"Sends the talkmessage stored in msg1.\n"
    },
    {
	"msg1",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 1.\n"
    },
    {
	"keySendMsg2",
	NULL,
	"F2",
	KEY_MSG_2,
	"Sends the talkmessage stored in msg2.\n"
    },
    {
	"msg2",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 2.\n"
    },
    {
	"keySendMsg3",
	NULL,
	"F3",
	KEY_MSG_3,
	"Sends the talkmessage stored in msg3.\n"
    },
    {
	"msg3",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 3.\n"
    },
    {
	"keySendMsg4",
	NULL,
	"F4",
	KEY_MSG_4,
	"Sends the talkmessage stored in msg4.\n"
    },
    {
	"msg4",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 4.\n"
    },
    {
	"keySendMsg5",
	NULL,
	"F5",
	KEY_MSG_5,
	"Sends the talkmessage stored in msg5.\n"
    },
    {
	"msg5",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 5.\n"
    },
    {
	"keySendMsg6",
	NULL,
	"F6",
	KEY_MSG_6,
	"Sends the talkmessage stored in msg6.\n"
    },
    {
	"msg6",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 6.\n"
    },
    {
	"keySendMsg7",
	NULL,
	"F7",
	KEY_MSG_7,
	"Sends the talkmessage stored in msg7.\n"
    },
    {
	"msg7",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 7.\n"
    },
    {
	"keySendMsg8",
	NULL,
	"F8",
	KEY_MSG_8,
	"Sends the talkmessage stored in msg8.\n"
    },
    {
	"msg8",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 8.\n"
    },
    {
	"keySendMsg9",
	NULL,
	"F9",
	KEY_MSG_9,
	"Sends the talkmessage stored in msg9.\n"
    },
    {
	"msg9",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 9.\n"
    },
    {
	"keySendMsg10",
	NULL,
	"F10",
	KEY_MSG_10,
	"Sends the talkmessage stored in msg10.\n"
    },
    {
	"msg10",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 10.\n"
    },
    {
	"keySendMsg11",
	NULL,
	"F11",
	KEY_MSG_11,
	"Sends the talkmessage stored in msg11.\n"
    },
    {
	"msg11",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 11.\n"
    },
    {
	"keySendMsg12",
	NULL,
	"F12",
	KEY_MSG_12,
	"Sends the talkmessage stored in msg12.\n"
    },
    {
	"msg12",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 12.\n"
    },
    {
	"keySendMsg13",
	NULL,
	"",
	KEY_MSG_13,
	"Sends the talkmessage stored in msg13.\n"
    },
    {
	"msg13",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 13.\n"
    },
    {
	"keySendMsg14",
	NULL,
	"",
	KEY_MSG_14,
	"Sends the talkmessage stored in msg14.\n"
    },
    {
	"msg14",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 14.\n"
    },
    {
	"keySendMsg15",
	NULL,
	"",
	KEY_MSG_15,
	"Sends the talkmessage stored in msg15.\n"
    },
    {
	"msg15",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 15.\n"
    },
    {
	"keySendMsg16",
	NULL,
	"",
	KEY_MSG_16,
	"Sends the talkmessage stored in msg16.\n"
    },
    {
	"msg16",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 16.\n"
    },
    {
	"keySendMsg17",
	NULL,
	"",
	KEY_MSG_17,
	"Sends the talkmessage stored in msg17.\n"
    },
    {
	"msg17",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 17.\n"
    },
    {
	"keySendMsg18",
	NULL,
	"",
	KEY_MSG_18,
	"Sends the talkmessage stored in msg18.\n"
    },
    {
	"msg18",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 18.\n"
    },
    {
	"keySendMsg19",
	NULL,
	"",
	KEY_MSG_19,
	"Sends the talkmessage stored in msg19.\n"
    },
    {
	"msg19",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 19.\n"
    },
    {
	"keySendMsg20",
	NULL,
	"",
	KEY_MSG_20,
	"Sends the talkmessage stored in msg20.\n"
    },
    {
	"msg20",
	NULL,
	"",
	KEY_DUMMY,
	"Talkmessage 20.\n"
    },
};

int optionsCount = NELEM(options);

unsigned String_hash(const char *s)
{
    unsigned		hash = 0;

    for (; *s; s++) {
	hash = (((hash >> 29) & 7) | (hash << 3)) ^ *s;
    }
    return hash;
}


char* Get_keyHelpString(keys_t key)
{
    int			i;
    char		*nl;
    static char		buf[MAX_CHARS];

    for (i = 0; i < NELEM(options); i++) {
	if (options[i].key == key) {
	    strlcpy(buf, options[i].help, sizeof buf);
	    if ((nl = strchr(buf, '\n')) != NULL) {
		*nl = '\0';
	    }
	    return buf;
	}
    }

    return NULL;
}


const char* Get_keyResourceString(keys_t key)
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
	    const char *str;
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
	    printf("        The default %s: %s.\n",
		   (options[i].key == KEY_DUMMY)
		       ? "value is"
		       : (strchr(options[i].fallback, ' ') == NULL)
			   ? "key is"
			   : "keys are",
		   options[i].fallback);
	}
	printf("\n");
    }
    printf(
"Most of these options can also be set in the .xpilotrc file\n"
"in your home directory.\n"
"Each key option may have multiple keys bound to it and\n"
"one key may be used by multiple key options.\n"
"If no server is specified then xpilot will search\n"
"for servers on your local network.\n"
"For a listing of remote servers try: telnet meta.xpilot.org 4400 \n"
	  );

    exit(1);
}


static int Find_resource(XrmDatabase db, const char *resource,
			 char *result, unsigned size, int *index)
{
#ifndef _WINDOWS
    int			i;
    int			len;
    char		str_name[80],
			str_class[80],
			*str_type[10];
    XrmValue		rmValue;
    unsigned		hash = String_hash(resource);

    for (i = 0;;) {
	if (hash == options[i].hash && !strcmp(resource, options[i].name)) {
	    *index = i;
	    break;
	}
	if (++i >= NELEM(options)) {
	    errno = 0;
	    error("BUG: Can't find option \"%s\"", resource);
	    exit(1);
	}
    }
    sprintf(str_name, "%s.%s", myName, resource);
    sprintf(str_class, "%s.%c%s", myClass,
	    islower(*resource) ? toupper(*resource) : *resource, resource + 1);

    if (XrmGetResource(db, str_name, str_class, str_type, &rmValue) == True) {
	if (rmValue.addr == NULL) {
	    len = 0;
	} else {
	    len = MIN(rmValue.size, size - 1);
	    memcpy(result, rmValue.addr, len);
	}
	result[len] = '\0';
	return 1;
    }
    strlcpy(result, options[*index].fallback, size);

    return 0;

#else	/* _WINDOWS */
    Config_get_resource(resource, result, size, index);

    return 1;
#endif
}


static int Get_resource(XrmDatabase db,
			const char *resource, char *result, unsigned size)
{
    int			index;

    return Find_resource(db, resource, result, size, &index);
}


static int Get_string_resource(XrmDatabase db,
			       const char *resource, char *result, unsigned size)
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
			     const char *resource, int *result)
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
			       const char *resource, DFLOAT *result)
{
    int			index;
    double		temp_result;
    char		resValue[MAX_CHARS];

    temp_result = 0.0;
    Find_resource(db, resource, resValue, sizeof resValue, &index);
    if (sscanf(resValue, "%lf", &temp_result) <= 0) {
	errno = 0;
	error("Bad value \"%s\" for option \"%s\", using default...",
	      resValue, resource);
	sscanf(options[index].fallback, "%lf", &temp_result);
    }
    *result = (DFLOAT) temp_result;
}


static void Get_bool_resource(XrmDatabase db, const char *resource, int *result)
{
    int			index;
    char		resValue[MAX_CHARS];

    Find_resource(db, resource, resValue, sizeof resValue, &index);
    *result = (ON(resValue) != 0);
}


static void Get_bit_resource(XrmDatabase db, const char *resource,
			     long *mask, int bit)
{
    int			index;
    char		resValue[MAX_CHARS];

    Find_resource(db, resource, resValue, sizeof resValue, &index);
    if (ON(resValue)) {
	SET_BIT(*mask, bit);
    }
}

static void Get_shipshape_resource(XrmDatabase db, char **ship_shape)
{
    char		resValue[MAX(2*MSG_LEN, PATH_MAX + 1)];

    Get_resource(db, "shipShape", resValue, sizeof resValue);
    *ship_shape = xp_strdup(resValue);
    if (*ship_shape && **ship_shape && !strchr(*ship_shape, '(' )) {
	/* so it must be the name of shipshape defined in the shipshapefile. */
	Get_resource(db, "shipShapeFile", resValue, sizeof resValue);
	if (resValue[0] != '\0') {
	    FILE *fp = fopen(resValue, "r");
	    if (!fp) {
		perror(resValue);
	    } else {
		char *ptr;
		char *str;
		char line[1024];
		while (fgets(line, sizeof line, fp)) {
		    if ((str = strstr(line, "(name:" )) != NULL
			|| (str = strstr(line, "(NM:" )) != NULL) {
			str = strchr(str, ':');
			while (*++str == ' ');
			if ((ptr = strchr(str, ')' )) != NULL) {
			    *ptr = '\0';
			}
			if (!strcmp(str, *ship_shape)) {
			    /* Gotcha */
			    free(*ship_shape);
			    if (ptr != NULL) {
				*ptr = ')';
			    }
			    *ship_shape = xp_strdup(line);
			    break;
			}
		    }
		}
		fclose(fp);
	    }
	}
    }
}


#ifndef _WINDOWS
void Get_xpilotrc_file(char *path, unsigned size)
{
    const char		*home = getenv("HOME");
    const char		*defaultFile = ".xpilotrc";
    const char		*optionalFile = getenv("XPILOTRC");

    if (optionalFile != NULL) {
	strlcpy(path, optionalFile, size);
    }
    else if (home != NULL) {
	strlcpy(path, home, size);
	strlcat(path, "/", size);
	strlcat(path, defaultFile, size);
    }
    else {
	strlcpy(path, "", size);
    }
}
#endif


#ifndef _WINDOWS
static void Get_file_defaults(XrmDatabase *rDBptr)
{
    int			len;
    char		*ptr,
			*lang = getenv("LANG"),
			*home = getenv("HOME"),
			path[PATH_MAX + 1];
    XrmDatabase		tmpDB;

    sprintf(path, "%s%s", Conf_libdir(), myClass);
    *rDBptr = XrmGetFileDatabase(path);

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

    Get_xpilotrc_file(path, sizeof(path));
    if (path[0] != '\0') {
	tmpDB = XrmGetFileDatabase(path);
	XrmMergeDatabases(tmpDB, rDBptr);
    }
}
#endif	/* _WINDOWS*/


void Parse_options(int *argcp, char **argvp, char *realName, int *port,
		   int *my_team, int *text, int *list,
		   int *join, int *noLocalMotd,
		   char *nickName, char *dispName, char *hostName,
		   char *shut_msg)
{
    char		*ptr;
    char		*str;
    int			i;
    int			j;
    int			num;
    int			firstKeyDef;
    keys_t		key;
    KeySym		ks;

    char		resValue[MAX(2*MSG_LEN, PATH_MAX + 1)];
    XrmDatabase		argDB = 0, rDB = 0;

#ifndef _WINDOWS
    XrmOptionDescRec	*xopt;
    int			size;


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
	options[i].hash = String_hash(options[i].name);
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
	    xopt[i].value = (char *)options[i].noArg;
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
	    strlcpy(dispName, ptr, MAX_DISP_LEN);
	} else {
	    strlcpy(dispName, DISPLAY_DEF, MAX_DISP_LEN);
	}
    }
    if ((dpy = XOpenDisplay(dispName)) == NULL) {
	error("Can't open display '%s'", dispName);
	if (strcmp(dispName, "NO_X") == 0) {
	    /* user does not want X stuff.  experimental.  use at own risk. */
	    strlcpy(nickName, realName, MAX_NAME_LEN);
	    *my_team = TEAM_NOT_SET;
	    Get_int_resource(argDB, "port", port);
	    Get_bool_resource(argDB, "list", list);
	    *text = true;
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
	    strlcpy(resValue, ptr, MAX_DISP_LEN);
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
    geometry = xp_strdup(resValue);
#endif

    if ((talk_fast_temp_buf_big = (char *)malloc(TALK_FAST_MSG_SIZE)) != NULL) {
        for (i = 0; i < TALK_FAST_NR_OF_MSGS; ++i) {
            sprintf (talk_fast_temp_buf, "msg%d", i + 1);
            Get_resource(rDB, talk_fast_temp_buf, talk_fast_temp_buf_big, TALK_FAST_MSG_SIZE);
            talk_fast_msgs[i] = xp_strdup (talk_fast_temp_buf_big);
        }
        free (talk_fast_temp_buf_big);
    }
    else {
	for (i = 0; i < TALK_FAST_NR_OF_MSGS; ++i) {
	    talk_fast_msgs[i] = NULL;
	}
    }

    Get_bool_resource(rDB, "ignoreWindowManager", &ignoreWindowManager);

    Get_resource(rDB, "user", resValue, MAX_NAME_LEN);
    if (resValue[0]) {
	strlcpy(realName, resValue, MAX_NAME_LEN);
    }
    Fix_real_name(realName);

    Get_resource(rDB, "host", resValue, MAX_HOST_LEN);
    if (resValue[0]) {
	strlcpy(hostName, resValue, MAX_HOST_LEN);
    }
    Fix_host_name(hostName);

    Get_resource(rDB, "name", nickName, MAX_NAME_LEN);
    if (!nickName[0]) {
	strlcpy(nickName, realName, MAX_NAME_LEN);
    }
    CAP_LETTER(nickName[0]);
    if (nickName[0] < 'A' || nickName[0] > 'Z') {
	errno = 0;
	error("Your player name \"%s\" should start with an uppercase letter",
	    nickName);
	exit(1);
    }
    /* strip trailing whitespace. */
    for (ptr = &nickName[strlen(nickName)]; ptr-- > nickName; ) {
	if (isascii(*ptr) && isspace(*ptr)) {
	    *ptr = '\0';
	} else {
	    break;
	}
    }
    strlcpy(realname, realName, sizeof(realname));
    strlcpy(name, nickName, sizeof(name));

    Get_int_resource(rDB, "team", my_team);

    IFWINDOWS( Config_get_name(name); )
    IFWINDOWS( Config_get_team(my_team); )

    if (*my_team < 0 || *my_team > 9) {
	*my_team = TEAM_NOT_SET;
    }
    team = *my_team;

    Get_int_resource(rDB, "port", port);
    Get_bool_resource(rDB, "text", text);
    Get_bool_resource(rDB, "list", list);
    Get_bool_resource(rDB, "join", join);
    Get_bool_resource(rDB, "noLocalMotd", noLocalMotd);
    Get_bool_resource(rDB, "autoServerMotdPopup", &i);
    autoServerMotdPopup = (i != 0) ? true : false;
    Get_bool_resource(rDB, "refreshMotd", &i);
    refreshMotd = (i != 0) ? true : false;

    Get_shipshape_resource(rDB, &shipShape);
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
    Get_bool_resource(rDB, "showNastyShots", &showNastyShots);
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
    Get_bool_resource(rDB, "autoShield", &auto_shield);

    Get_int_resource(rDB, "clientPortStart", &clientPortStart);
    Get_int_resource(rDB, "clientPortEnd", &clientPortEnd);


    Get_resource(rDB, "modifierBank1", modBankStr[0], sizeof modBankStr[0]);
    Get_resource(rDB, "modifierBank2", modBankStr[1], sizeof modBankStr[1]);
    Get_resource(rDB, "modifierBank3", modBankStr[2], sizeof modBankStr[2]);
    Get_resource(rDB, "modifierBank4", modBankStr[3], sizeof modBankStr[3]);

    Get_resource(rDB, "visual", visualName, sizeof visualName);
    Get_bool_resource(rDB, "mono", &i);
    mono = (i != 0) ? true : false;
    Get_bool_resource(rDB, "colorSwitch", &i);
    colorSwitch = (i != 0) ? true : false;
    Get_bool_resource(rDB, "multibuffer", &i);
    multibuffer = (i != 0) ? true : false;

    /* Windows already derived maxColors in InitWinX */
    IFNWINDOWS( Get_int_resource(rDB, "maxColors", &maxColors); )

    Get_string_resource(rDB, "black", color_names[0], sizeof(color_names[0]));
    Get_string_resource(rDB, "white", color_names[1], sizeof(color_names[1]));
    Get_string_resource(rDB, "blue", color_names[2], sizeof(color_names[2]));
    Get_string_resource(rDB, "red", color_names[3], sizeof(color_names[3]));
    for (i = 0; i < MAX_COLORS; i++) {
	char buf[16];
	sprintf(buf, "color%d", i);
	if (!Get_string_resource(rDB, buf, resValue, MAX_COLOR_LEN)) {
	    if (i < NUM_COLORS) {
		strlcpy(resValue, color_names[i], MAX_COLOR_LEN);
	    }
	}
	strlcpy(color_names[i], resValue, MAX_COLOR_LEN);
    }
    Get_int_resource(rDB, "hudColor", &hudColor);
    Get_int_resource(rDB, "hudLockColor", &hudLockColor);
    Get_int_resource(rDB, "wallColor", &wallColor);
    Get_int_resource(rDB, "wallRadarColor", &wallRadarColor);
    Get_int_resource(rDB, "decorColor", &decorColor);
    Get_int_resource(rDB, "decorRadarColor", &decorRadarColor);
    Get_int_resource(rDB, "targetRadarColor", &targetRadarColor);
    Get_int_resource(rDB, "oldMessagesColor", &oldMessagesColor);
    Get_resource(rDB, "sparkColors", sparkColors, MSG_LEN);

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
    Get_bit_resource(rDB, "showItems", &instruments, SHOW_ITEMS);
    Get_bit_resource(rDB, "clock", &instruments, SHOW_CLOCK);
    Get_bit_resource(rDB, "clockAMPM", &instruments, SHOW_CLOCK_AMPM_FORMAT);
    Get_bit_resource(rDB, "outlineWorld", &instruments, SHOW_OUTLINE_WORLD);
    Get_bit_resource(rDB, "filledWorld", &instruments, SHOW_FILLED_WORLD);
    Get_bit_resource(rDB, "texturedWalls", &instruments, SHOW_TEXTURED_WALLS);
    Get_bit_resource(rDB, "showDecor", &instruments, SHOW_DECOR);
    Get_bit_resource(rDB, "outlineDecor", &instruments, SHOW_OUTLINE_DECOR);
    Get_bit_resource(rDB, "filledDecor", &instruments, SHOW_FILLED_DECOR);
    Get_bit_resource(rDB, "texturedDecor", &instruments, SHOW_TEXTURED_DECOR);
    Get_bit_resource(rDB, "texturedBalls", &instruments, SHOW_TEXTURED_BALLS);
    Get_bit_resource(rDB, "reverseScroll", &instruments, SHOW_REVERSE_SCROLL);

    Get_bool_resource(rDB, "texturedObjects", &blockBitmaps);
    Get_bool_resource(rDB, "pointerControl", &initialPointerControl);
    Get_bool_resource(rDB, "erase", &useErase);
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

    Get_int_resource(rDB, "maxMessages", &maxMessages);
#ifndef _WINDOWS
    Get_bool_resource(rDB, "selectionAndHistory", &selectionAndHistory);
    Get_int_resource(rDB, "maxLinesInHistory", &maxLinesInHistory);
    LIMIT(maxLinesInHistory, 1, MAX_HIST_MSGS);
#endif

    Get_int_resource(rDB, "receiveWindowSize", &receive_window_size);
    LIMIT(receive_window_size, MIN_RECEIVE_WINDOW_SIZE, MAX_RECEIVE_WINDOW_SIZE);

    Get_resource(rDB, "recordFile", resValue, sizeof resValue);
    Record_init(resValue);
    Get_resource(rDB, "texturePath", resValue, sizeof resValue);
    texturePath = xp_strdup(resValue);
    Get_resource(rDB, "wallTextureFile", resValue, sizeof resValue);
    wallTextureFile = xp_strdup(resValue);
    Get_resource(rDB, "decorTextureFile", resValue, sizeof resValue);
    decorTextureFile = xp_strdup(resValue);
    Get_resource(rDB, "ballTextureFile", resValue, sizeof resValue);
    ballTextureFile = xp_strdup(resValue);

    Get_int_resource(rDB, "maxFPS", &maxFPS);
    oldMaxFPS = maxFPS;

    IFWINDOWS( Get_int_resource(rDB, "radarDivisor", &RadarDivisor); )
    IFWINDOWS( Get_bool_resource(rDB, "threadedDraw", &ThreadedDraw); )

#ifdef	WINDOWSCALING
    Get_float_resource(rDB, "scaleFactor", &scaleFactor);
    if (scaleFactor == 0.0) {
	scaleFactor = 1.0;
    }
    LIMIT(scaleFactor, MIN_SCALEFACTOR, MAX_SCALEFACTOR);
    Get_float_resource(rDB, "altScaleFactor", &scaleFactor_s);
    if (scaleFactor_s == 0.0) {
        scaleFactor_s = 2.0;
    }
    LIMIT(scaleFactor_s, MIN_SCALEFACTOR, MAX_SCALEFACTOR);
#endif

#ifdef SOUND
    Get_string_resource(rDB, "sounds", sounds, sizeof sounds);
    Get_int_resource(rDB, "maxVolume", &maxVolume);
    Get_resource(rDB, "audioServer", audioServer, sizeof audioServer);
#endif

#ifdef SPARC_CMAP_HACK
    Get_string_resource(rDB, "frameBuffer", frameBuffer, sizeof frameBuffer);
#endif

    Get_test_resources(rDB);

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

	    /* insertion sort. */
	    for (j = num; j > 0; j--) {
		if (ks >= keyDefs[j - 1].keysym) {
		    break;
		}
		keyDefs[j] = keyDefs[j - 1];
	    }
	    keyDefs[j].keysym = ks;
	    keyDefs[j].key = key;
	    num++;
	    if (!key) {
		printf("bug key 0\n");
		exit(1);
	    }
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

#ifndef _WINDOWS
    XrmDestroyDatabase(rDB);

    free(xopt);
#endif

#ifdef SOUND
    audioInit(dispName);
#endif /* SOUND */
}

void	defaultCleanup(void)
{
    IFWINDOWS( Get_xpilotini_file(-1); )

    if (keyDefs) {
	free(keyDefs);
	keyDefs = NULL;
    }
    if (texturePath) {
	free(texturePath);
	texturePath = NULL;
    }
    if (wallTextureFile) {
	free(wallTextureFile);
	wallTextureFile = NULL;
    }
    if (decorTextureFile) {
	free(decorTextureFile);
	decorTextureFile = NULL;
    }
    if (ballTextureFile) {
	free(ballTextureFile);
	ballTextureFile = NULL;
    }
    if (shipShape) {
	free(shipShape);
	shipShape = NULL;
    }

#ifdef SOUND
    audioCleanup();
#endif /* SOUND */
}


#ifdef DEVELOPMENT
static int X_error_handler(Display *display, XErrorEvent *xev)
{
    char		buf[1024];

    fflush(stdout);
    fprintf(stderr, "X error\n");
    XGetErrorText(display, xev->error_code, buf, sizeof buf);
    buf[sizeof(buf) - 1] = '\0';
    fprintf(stderr, "%s\n", buf);
    fflush(stderr);
    *(double *) -3 = 2.10;	/*core dump*/
    exit(1);
    return 0;
}

static void X_after(Display *display)
{
    static int		n;

    if (n < 1000) {
	printf("_X_ %4d\n", n++);
    }
}

static void Get_test_resources(XrmDatabase rDB)
{
    char	*s;
    char testBuffer[256];

    Get_string_resource(rDB, "test", testBuffer, sizeof testBuffer);

    for (s = strtok(testBuffer, ":"); s != NULL; s = strtok(NULL, ":")) {
	if (!strncasecmp(s, "xsync", 3)) {
	    XSynchronize(dpy, True);
	    XSetErrorHandler(X_error_handler);
	}
	else if (!strncasecmp(s, "xdebug", 4)) {
	    XSetErrorHandler(X_error_handler);
	}
	else if (!strncasecmp(s, "after", 5)) {
	    XSetAfterFunction(dpy, (int (*)(
#if NeedNestedPrototypes
					    Display *
#endif
					    )) X_after);
	}
	else if (!strncasecmp(s, "color", 3)) {
	    Colors_debug();
	}
	else {
	    printf("typo %s\n", s);
	    exit(1);
	}
    }
}
#else
static void Get_test_resources(XrmDatabase rDB)
{
}
#endif


/* $Id: winConfig.c,v 5.2 2001/06/22 05:00:57 dik Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
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
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "version.h"
#include "config.h"
#include "const.h"
#include "error.h"
#include "commonproto.h"
#include "../paint.h"
#include "keys.h"
#include "../default.h"
#include "winConfig.h"
#include "winClient.h"
#include "winXXPilot.h"
#ifndef	lint
static char sourceid[] =
    "@(#)$Id: winConfig.c,v 5.2 2001/06/22 05:00:57 dik Exp $";
#endif


extern  char    **Argv;
extern  int     Argc;

const char*	winHelpFile;


void Config_get_name(char *name)
{
    if (*name == '\0') {
	/* Windows may have no default name */
	strcpy(name, "NoName");
    }
}


void Config_get_team(int *my_team)
{
    if (Argc > 2) {
	/* oh, it's those fixed pos variables again! */
	if (Argv[2] && !strncmp(Argv[2], "-team", 5)) {
	    /* i've really got to fix that... */
	    *my_team = Argv[2][6] & 0x0f;
	}
    }
}


void Config_save_resource(FILE *fp, const char *resource, char *value)
{
    WritePrivateProfileString("Settings",
			      resource,
			      value,
			      Get_xpilotini_file(1));
}


/* save our window's position */
void Config_save_window_positions(void)
{
    WINDOWPLACEMENT	wp;
    Window w;
    RECT rect;
    char	s[50];
    w = WinXGetParent(top);
    WinXGetWindowRect(w, &rect);
    WinXGetWindowPlacement(w, &wp);
    if (wp.showCmd != SW_SHOWMINIMIZED)
    {
	extern	const char* s_WindowMet;
	extern	const char* s_L;
	extern	const char* s_T;
	extern	const char* s_R;
	extern	const char* s_B;
	itoa(rect.left, s, 10);
	WritePrivateProfileString(s_WindowMet, s_L,
				  itoa(rect.left, s, 10),
				  Get_xpilotini_file(1));
	WritePrivateProfileString(s_WindowMet, s_T,
				  itoa(rect.top, s, 10),
				  Get_xpilotini_file(1));
	WritePrivateProfileString(s_WindowMet, s_R,
				  itoa(rect.right, s, 10),
				  Get_xpilotini_file(1));
	WritePrivateProfileString(s_WindowMet, s_B,
				  itoa(rect.bottom, s, 10),
				  Get_xpilotini_file(1));
    }
}


void Config_get_profile_string(
	const char* key,
	const char* def,
	char* result,
	int size)
{
    int		i;

    for (i = 0; i < 3; i++)
    {
	GetPrivateProfileString("Settings", key, "", result, size, 
				Get_xpilotini_file(i));
	if (result[0] != '\0')
	    return;
    }
    strncpy(result, def, size);
}


void Config_get_resource(
	const char *resource,
	char *result,
	unsigned size,
	int* index)
{
    unsigned		hash = String_hash(resource);
	int	i;

    for (i = 0;;)
	{
		if (!strcmp(resource, options[i].name))
		{
			*index = i;
			break;
		}
		if (++i >= optionsCount)
		{
			errno = 0;
			error("BUG: Can't find option \"%s\"", resource);
			exit(1);
		}
    }
    Config_get_profile_string(resource, options[i].fallback, result, size);

#if 0
    GetPrivateProfileString("Settings", resource, "", result, size, 
			    Get_xpilotini_file(1));
    if (result[0] == '\0') {
	GetPrivateProfileString("Settings", resource, "", result, size, 
				Get_xpilotini_file(2));
	if (result[0] == '\0') {
	    strncpy(result, options[*index].fallback, size);
	}
    }
#endif
}


/* Get the name for XPilot.ini .  Hopefully, this will be the fully qualified
path to where XPilot.exe started from.  Right now, we guess that is the location
of the default help file.
the parameter level is used to determine which ini file to use.  This allows us
to cascade through a series of default ini files.
level = -1 = free private memory.
level =  0 = current directory, XPilot.host.ini  (currently returns defini)
level =  1 = current directory, XPilot.ini
level =  2 = Windows directory, XPilot.ini
*/
char *Get_xpilotini_file(int level)
{
    static char *xpini = NULL;
    static char *winxpini = NULL;
    static char *defini = "XPilot.ini";

    char *end;
    int size;

    switch (level) {
    case 0:
    case 1:
	if (xpini)
	    return (xpini);
	if (!winHelpFile)	/* do we have the help default to build from? */
	    return (defini);	/* no, return generic name (wherever that is) */

	end = strrchr(winHelpFile, '\\');
	if (!end)
	    return (defini);	/* no backslash? bail out */
	size = (end - winHelpFile) + 13;
	xpini = malloc(size + 1);
	memset(xpini, 0, size);
	strncpy(xpini, winHelpFile, end - winHelpFile);
	strcat(xpini, "\\");
	strcat(xpini, defini);
	return (xpini);
    case 2:
	if (winxpini)
	    return (winxpini);
	size = GetWindowsDirectory(winxpini, 0);	/* returns size needed */
	winxpini = malloc(size + 15);
	GetWindowsDirectory(winxpini, size + 13);
	if (winxpini[strlen(winxpini)] != '\\')
	    strcat(winxpini, "\\");
	strcat(winxpini, defini);
	return (winxpini);
    case -1:
	if (xpini) {
	    free(xpini);
	    xpini = NULL;
	}
	if (winxpini) {
	    free(winxpini);
	    winxpini = NULL;
	}
	return (NULL);
    }
    error("BUG: bad level in Get_xpilotini_file");
    return (defini);
}

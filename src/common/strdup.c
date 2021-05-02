/* $Id: strdup.c,v 5.3 2001/05/27 12:27:20 bertg Exp $
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
#include <string.h>

#include "error.h"
#include "commonproto.h"

char *xp_strdup(const char *old_string)
{
    char	*new_string;
    size_t	string_length;

    string_length = strlen(old_string);
    new_string = (char *)malloc(string_length + 1);
    if (new_string) {
	memcpy(new_string, old_string, string_length + 1);
    }

    return new_string;
}

char *xp_safe_strdup(const char *old_string)
{
    char	*new_string;

    new_string = xp_strdup(old_string);
    if (new_string == NULL) {
	fatal("Not enough memory.");
    }

    return new_string;
}


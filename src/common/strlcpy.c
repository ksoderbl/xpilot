/* $Id: strlcpy.c,v 5.5 2001/05/07 20:06:09 bertg Exp $
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

#include <stdlib.h>
#include "commonproto.h"


/*
    NAME
	strlcpy
    ARGS
	char *dest
	const char *src
	size_t size
    DESC
	Copy src to dest.
	Dest may hold at most size - 1 characters
	and will always be NUL terminated,
	except if size equals zero.
	Return strlen(src).
	There was not enough room in dest if the
	return value is bigger or equal than size.
*/
size_t strlcpy(char *dest, const char *src, size_t size)
{
    char		*d = dest;
    const char		*s = src;
    char		*maxd = dest + (size - 1);

    if (size > 0) {
	while (*s && d < maxd) {
	    *d = *s;
	    s++;
	    d++;
	}
	*d = '\0';
    }
    while (*s) {
	s++;
    }
    return (s - src);
}


/*
    NAME
	strlcat
    ARGS
	char *dest
	const char *src
	size_t size
    DESC
	Append src to dest.
	Dest may hold at most size - 1 characters
	and will always be NUL terminated,
	except if size equals zero.
	Return strlen(src) + strlen(dest).
	There was not enough room in dest if the
	return value is bigger or equal than size.
*/
size_t strlcat(char *dest, const char *src, size_t size)
{
    char		*d = dest;
    const char		*s = src;
    char		*maxd = dest + (size - 1);
    size_t		dlen = 0;

    if (size > 0) {
	while (*d && d < maxd) {
	    d++;
	}
	dlen = (d - dest);
	while (*s && d < maxd) {
	    *d = *s;
	    s++;
	    d++;
	}
	*d = '\0';
    }
    while (*s) {
	s++;
    }
    return dlen + (s - src);
}


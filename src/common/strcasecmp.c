/* $Id: strcasecmp.c,v 5.2 2001/06/02 21:02:25 bertg Exp $
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
#include <ctype.h>


/* prototypes to shut up strict compiler. */
int strcasecmp(const char *str1, const char *str2);
int strncasecmp(const char *str1, const char *str2, size_t n);


/*
 * By Ian Malcom Brown.
 * Changes by BG: prototypes with const,
 * moved the ++ expressions out of the macro.
 * Only test for the null byte in one string.
 */
int strcasecmp(const char *str1, const char *str2)
{
    int	c1, c2;


    do {
	c1 = *str1++;
	c2 = *str2++;
	c1 = tolower(c1);
	c2 = tolower(c2);
    } while (c1 == c2 && c1 != 0);

    return (c1 - c2);
}

/*
 * By Bert Gijsbers, derived from Ian Malcom Brown's strcasecmp().
 */
int strncasecmp(const char *str1, const char *str2, size_t n)
{
    int	c1, c2;

    do {
	if (n-- <= 0) {
	    return 0;
	}
	c1 = *str1++;
	c2 = *str2++;
	c1 = tolower(c1);
	c2 = tolower(c2);
    } while (c1 == c2 && c1 != 0);

    return (c1 - c2);
}

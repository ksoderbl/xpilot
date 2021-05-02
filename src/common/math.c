/* $Id: math.c,v 4.2 1998/04/16 17:40:43 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
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
#include <math.h>

#include "version.h"
#include "config.h"
#include "const.h"
#include "error.h"

char math_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: math.c,v 4.2 1998/04/16 17:40:43 bert Exp $";
#endif


DFLOAT		tbl_sin[TABLE_SIZE];
DFLOAT		tbl_cos[TABLE_SIZE];

int ON(char *optval);
int OFF(char *optval);
int f2i(DFLOAT f);
DFLOAT findDir(DFLOAT x, DFLOAT y);
void Make_table(void);

int ON(char *optval)
{
    return (strncasecmp(optval, "true", 4) == 0
	    || strncasecmp(optval, "on", 2) == 0
	    || strncasecmp(optval, "yes", 3) == 0);
}


int OFF(char *optval)
{
    return (strncasecmp(optval, "false", 5) == 0
	    || strncasecmp(optval, "off", 3) == 0
	    || strncasecmp(optval, "no", 2) == 0);
}


int mod(int x, int y)
{
    if (x >= y || x < 0)
	x = x - y*(x/y);

    if (x < 0)
	x += y;

    return x;
}


int f2i(DFLOAT f)
{
    return (f < 0) ? -(int)(0.5f - f) : (int)(f + 0.5f);
}

DFLOAT findDir(DFLOAT x, DFLOAT y)
{
    DFLOAT angle;

    if (x != 0.0 || y != 0.0)
	angle = atan2(y, x) / (2 * PI);
    else
	angle = 0.0;

    if (angle < 0)
	angle++;
    return angle * RES;
}


DFLOAT rfrac(void)
{
    /*
     * Return a pseudo-random value in the range { 0 <= x < 1 }.
     * Assume all RAND_MAXs are at least 32767 and divide by 32768.
     */
    return (DFLOAT)((double)(rand() & 0x7FFF) * 0.000030517578125);
}

void Make_table(void)
{
    int i;

    for (i = 0; i < TABLE_SIZE; i++) {
	tbl_sin[i] = sin(i * (2.0 * PI / TABLE_SIZE));
	tbl_cos[i] = cos(i * (2.0 * PI / TABLE_SIZE));
    }
}


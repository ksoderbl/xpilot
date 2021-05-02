/* $Id: math.c,v 3.9 1994/03/30 16:56:52 bert Exp $
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

/*
 * Actually used by SERVER and CLIENT but define SERVER
 * so to avoid X11 dependencies for the server
 */
#define SERVER
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "config.h"
#include "types.h"
#include "const.h"
#include "draw.h"
#include "error.h"


#ifndef	lint
static char sourceid[] =
    "@(#)$Id: math.c,v 3.9 1994/03/30 16:56:52 bert Exp $";
#endif


static int	debugShapeParsing = 0;
static int	verboseShapeParsing;
static int	shapeLimits;

float		tbl_sin[TABLE_SIZE];


int mod(int x, int y)
{
    if (x >= y || x < 0)
	x = x - y*(x/y);

    if (x < 0)
	x += y;

    return x;
}


float findDir(float x, float y)
{
    float angle;

    if (x != 0.0 || y != 0.0)
	angle = atan2(y, x) / (2 * PI);
    else
	angle = 0.0;

    if (angle < 0)
	angle++;
    return angle * RES;
}


void Make_table(void)
{
    int i;

    for (i=0; i<TABLE_SIZE; i++)
	tbl_sin[i] = sin((2*PI) * ((float)i/TABLE_SIZE));
}


static void Rotate_ship(wireobj *w)
{
    int			i, j;

    /* take the initial shape (pts[0]) and rotate it */
    for (j = 1; j < RES; j++) {
	for (i = 0; i < w->num_points; i++) {
	    w->pts[j][i].x = tcos(j) * w->pts[0][i].x
			   - tsin(j) * w->pts[0][i].y;
	    w->pts[j][i].y = tsin(j) * w->pts[0][i].x
			   + tcos(j) * w->pts[0][i].y;
	}
    }
}

wireobj *Default_ship(void)
{
    static wireobj	def_ship;
    static position	ship_pts[RES][3];

    if (!def_ship.num_points) {
	int			i;

	Make_table();
	def_ship.num_points = 3;
	def_ship.pt1 = 1;
	def_ship.pt2 = 2;
	for (i = 0; i < RES; i++) {
	    def_ship.pts[i] = &ship_pts[i][0];
	}
	def_ship.pts[0][0].x = 15;
	def_ship.pts[0][0].y =  0;
	def_ship.pts[0][1].x = -9;
	def_ship.pts[0][1].y =  8;
	def_ship.pts[0][2].x = -9;
	def_ship.pts[0][2].y = -8;
	Rotate_ship(&def_ship);
    }
    return &def_ship;
}

static int shape2wire(char *str, wireobj *w)
{
    int			i,
			x,
			y;
    const int		min_pts = 3,
			max_pts = 16;

    if (debugShapeParsing) {
	printf("parsing shape: %s\n", str);
    }
    if (sscanf(str, "(%d,%d,%d)", &w->num_points, &w->pt1, &w->pt2) != 3
	|| w->num_points < min_pts
	|| w->num_points > max_pts
	|| w->pt1 < 0
	|| w->pt1 >= w->num_points
	|| w->pt2 < 0
	|| w->pt2 >= w->num_points
	) {
	if (verboseShapeParsing) {
	    printf("Bad ship shape: header description incorrect %s\n", str);
	}
	return -1;
    }
    w->pts[0] = (position *) malloc(w->num_points * sizeof(position) * RES);
    if (!w->pts[0]) {
	error("No memory for ship shape %s", str);
	return -1;
    }
    for (i = 1; i < RES; i++) {
	w->pts[i] = &w->pts[0][i * w->num_points];
    }
    for (i = 0; i < w->num_points; i++) {
	str = strchr(str + 1, '(');
	if (!str) {
	    if (verboseShapeParsing) {
		printf("Bad ship shape: only %d points defined, %d expected\n",
		       i, w->num_points);
	    }
	    return -1;
	}
	if (sscanf(str, "(%d,%d)", &x, &y) != 2) {
	    if (verboseShapeParsing) {
		printf("Bad ship shape: format error in point %d\n", i);
	    }
	    return -1;
	}
	w->pts[0][i].x = x;
	w->pts[0][i].y = y;
    }
    if (shapeLimits) {
	const int	isLow = -8, isHi = 8, isLeft = 8, isRight = -8,
			minLow = 1, minHi = 1, minLeft = 1, minRight = 1,
			horMax = 15, verMax = 15,
			minCount = 3;
	int		low = 0, hi = 0, left = 0, right = 0,
			count = 0, change, max = 0;

	for (i = 0; i < w->num_points; i++) {
	    x = w->pts[0][i].x;
	    y = w->pts[0][i].y;
	    change = 0;
	    if (y >= isLeft) change++, left++;
	    if (y <= isRight) change++, right++;
	    if (x <= isLow) change++, low++;
	    if (x >= isHi) change++, hi++;
	    if (change) count++;
	    if (abs(y) > horMax) max++;
	    if (abs(x) > verMax) max++;
	}
	if (low < minLow
	    || hi < minHi
	    || left < minLeft
	    || right < minRight
	    || count < minCount) {
	    if (verboseShapeParsing) {
		printf("Ship shape does not meet size requirements (%d,%d,%d,%d,%d)\n",
		       low, hi, left, right, count);
	    }
	    return -1;
	}
	if (max != 0) {
	    if (verboseShapeParsing) {
		printf("Ship shape exceeds size maxima.\n");
	    }
	    return -1;
	}
    }

    Rotate_ship(w);

    return 0;
}

static wireobj *do_parse_shape(char *str)
{
    wireobj		*w;

    if (!str || !*str) {
	return Default_ship();
    }
    if (!(w = (wireobj *)malloc(sizeof(*w)))) {
	error("No mem for ship shape");
	return Default_ship();
    }
    w->pts[0] = NULL;
    if (shape2wire(str, w) != 0) {
	if (w->pts[0] != NULL) {
	    free(w->pts[0]);
	    w->pts[0] = NULL;
	}
	free(w);
	return Default_ship();
    }
    return w;
}

void Free_ship_shape(wireobj *w)
{
    if (w != Default_ship() && w != NULL) {
	if (w->pts[0] != NULL) {
	    free(w->pts[0]);
	    w->pts[0] = NULL;
	}
	free(w);
    }
}

wireobj *Parse_shape_str(char *str)
{
    verboseShapeParsing = debugShapeParsing;
    shapeLimits = 1;
    return do_parse_shape(str);
}

wireobj *Convert_shape_str(char *str)
{
    verboseShapeParsing = debugShapeParsing;
    shapeLimits = debugShapeParsing;
    return do_parse_shape(str);
}

int Validate_shape_str(char *str)
{
    wireobj		*w;

    verboseShapeParsing = 1;
    shapeLimits = 1;
    w = do_parse_shape(str);
    Free_ship_shape(w);
    return (w != Default_ship());
}

/* $Id: shipshape.c,v 5.4 2001/05/25 02:47:49 dik Exp $
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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

#if defined(_WINDOWS)
	/* need this for printf wrappers. */
#	ifdef	_XPILOTNTSERVER_
#		include "../server/NT/winServer.h"
#	elif !defined(_XPMONNT_)
#		include "NT/winX.h"
#		include "../client/NT/winClient.h"
#	endif
#endif

#include "version.h"
#include "xpconfig.h"
#include "const.h"
#include "draw.h"
#include "error.h"

char shipshape_version[] = VERSION;



static int	debugShapeParsing = 0;
static int	verboseShapeParsing;
static int	shapeLimits;

static int	Get_shape_keyword(char *keyw);
extern void	Make_table(void);

void Rotate_point(position pt[RES])
{
    int			i;

    for (i = 1; i < RES; i++) {
	pt[i].x = tcos(i) * pt[0].x - tsin(i) * pt[0].y;
	pt[i].y = tsin(i) * pt[0].x + tcos(i) * pt[0].y;
    }
}

static void Rotate_ship(shipobj *w)
{
    int			i;

    for (i = 0; i < w->num_points; i++) {
	Rotate_point(&w->pts[i][0]);
    }
    Rotate_point(&w->engine[0]);
    Rotate_point(&w->m_gun[0]);
    for (i = 0; i < w->num_l_gun; i++) {
	Rotate_point(&w->l_gun[i][0]);
    }
    for (i = 0; i < w->num_r_gun; i++) {
	Rotate_point(&w->r_gun[i][0]);
    }
    for (i = 0; i < w->num_l_rgun; i++) {
	Rotate_point(&w->l_rgun[i][0]);
    }
    for (i = 0; i < w->num_r_rgun; i++) {
	Rotate_point(&w->r_rgun[i][0]);
    }
    for (i = 0; i < w->num_l_light; i++) {
	Rotate_point(&w->l_light[i][0]);
    }
    for (i = 0; i < w->num_r_light; i++) {
	Rotate_point(&w->r_light[i][0]);
    }
    for (i = 0; i < w->num_m_rack; i++) {
	Rotate_point(&w->m_rack[i][0]);
    }
}

/*
 * Return a pointer to a default ship.
 * This function should always succeed,
 * therefore no malloc()ed memory is used.
 */
shipobj *Default_ship(void)
{
    static shipobj	sh;
    static position	pts[6][RES];

    if (!sh.num_points) {
	sh.num_points = 3;
	sh.pts[0] = &pts[0][0];
	sh.pts[0][0].x = 15;
	sh.pts[0][0].y = 0;
	sh.pts[1] = &pts[1][0];
	sh.pts[1][0].x = -9;
	sh.pts[1][0].y = 8;
	sh.pts[2] = &pts[2][0];
	sh.pts[2][0].x = -9;
	sh.pts[2][0].y = -8;

	sh.engine[0].x = -9;
	sh.engine[0].y = 0;

	sh.m_gun[0].x = 15;
	sh.m_gun[0].y = 0;

	sh.num_l_light = 1;
	sh.l_light[0] = &pts[3][0];
	sh.l_light[0][0].x = -9;
	sh.l_light[0][0].y = 8;

	sh.num_r_light = 1;
	sh.r_light[0] = &pts[4][0];
	sh.r_light[0][0].x = -9;
	sh.r_light[0][0].y = -8;

	sh.num_m_rack = 1;
	sh.m_rack[0] = &pts[5][0];
	sh.m_rack[0][0].x = 15;
	sh.m_rack[0][0].y = 0;

	sh.num_l_gun = sh.num_r_gun = sh.num_l_rgun = sh.num_r_rgun = 0;

	Make_table();

	Rotate_ship(&sh);
    }

    return &sh;
}

static int shape2wire(char *ship_shape_str, shipobj *w)
{
/*
 * Macros to simplify limit-checking for ship points.
 * Until XPilot goes C++.
 */
#define GRID_PT(x,y)	grid.pt[(x)+15][(y)+15]
#define GRID_ADD(x,y)	(GRID_PT(x, y) = 2, \
			 grid.chk[grid.todo][0] = (x) + 15, \
			 grid.chk[grid.todo][1] = (y) + 15, \
			 grid.todo++)
#define GRID_GET(x,y)	((x) = (int)grid.chk[grid.done][0] - 15, \
			 (y) = (int)grid.chk[grid.done][1] - 15, \
			 grid.done++)
#define GRID_CHK(x,y)	(GRID_PT(x, y) == 2)
#define GRID_READY()	(grid.done >= grid.todo)
#define GRID_RESET()	(memset(grid.pt, 0, sizeof grid.pt), \
			 grid.done = 0, \
			 grid.todo = 0)

    struct grid_t {
	int		todo, done;
	unsigned char	pt[32][32];
	unsigned char	chk[32*32][2];
    } grid;

    int 		i, j, x, y, dx, dy,
			inx, iny, max,
			ofNum, ofLeft, ofRight,		/* old format */
			shape_version = 0;
    ipos 		pt[MAX_SHIP_PTS],
			engine,
			m_gun,
			l_light[MAX_LIGHT_PTS],
			r_light[MAX_LIGHT_PTS],
			l_gun[MAX_GUN_PTS],
			r_gun[MAX_GUN_PTS],
			l_rgun[MAX_GUN_PTS],
			r_rgun[MAX_GUN_PTS],
			m_rack[MAX_RACK_PTS];
    bool		mainGunSet = false,
			engineSet = false;
    char		*str,
			*teststr;
    char 		keyw[20],
			buf[MSG_LEN];

    w->num_points = 0;
    w->num_l_gun = 0;
    w->num_r_gun = 0;
    w->num_l_rgun = 0;
    w->num_r_rgun = 0;
    w->num_l_light = 0;
    w->num_r_light = 0;
    w->num_m_rack  = 0;
#ifdef	_NAMEDSHIPS
    w->name = NULL;
    w->author = NULL;
#endif

    if (debugShapeParsing) {
	xpprintf("parsing shape: %s\n", ship_shape_str);
    }

    for (str = ship_shape_str; (str = strchr(str, '(' )) != NULL; ) {

	str++;

	if (shape_version == 0) {
	    if (isdigit(*str)) {
		shape_version = 0x3100;
		if (verboseShapeParsing) {
		    xpprintf("ship shape is in old format\n");
		}
		break;
	    }
	    else {
		shape_version = 0x3200;
	    }
	}

	for (i = 0; (keyw[i] = str[i]) != '\0'; i++) {
	    if (i == sizeof(keyw) - 1) {
		keyw[i] = '\0';
		break;
	    }
	    if (keyw[i] == ':') {
		keyw[i + 1] = '\0';
		break;
	    }
	}
	if (str[i] != ':') {
	    if (verboseShapeParsing) {
		xpprintf("Missing colon in ship shape: %s\n", keyw);
	    }
	    continue;
	}
	for (teststr = &buf[++i]; (buf[i] = str[i]) != '\0'; i++) {
	    if (buf[i] == ')' ) {
		buf[i++] = '\0';
		break;
	    }
	}
	str += i;

	switch (Get_shape_keyword(keyw)) {

	case 0:		/* Keyword is 'shape' */
	    while (teststr) {
		while (*teststr == ' ') teststr++;
		if (sscanf(teststr, "%d,%d", &inx, &iny) != 2) {
		    if (verboseShapeParsing) {
			xpprintf("Missing ship shape coordinate in: \"%s\"\n",
			       teststr);
		    }
		    break;
		}
		if (w->num_points >= MAX_SHIP_PTS) {
		    if (verboseShapeParsing) {
			xpprintf("Too many ship shape coordinates\n");
		    }
		}
		else {
		    pt[w->num_points].x = inx;
		    pt[w->num_points].y = iny;
		    w->num_points++;
		    if (debugShapeParsing) {
			xpprintf("ship point at %d,%d\n", inx, iny);
		    }
		}
		teststr = strchr(teststr, ' ');
	    }
	    break;

	case 1:		/* Keyword is 'mainGun' */
	    if (mainGunSet) {
		if (verboseShapeParsing) {
		    xpprintf("Ship shape keyword \"%s\" multiple defined\n",
			   keyw);
		}
		break;
	    }
	    while (*teststr == ' ') teststr++;
	    if (sscanf(teststr, "%d,%d", &inx, &iny) != 2) {
		if (verboseShapeParsing) {
		    xpprintf("Missing main gun coordinate in: \"%s\"\n",
			   teststr);
		}
	    }
	    else {
		m_gun.x = inx;
		m_gun.y = iny;
		mainGunSet = true;
		if (debugShapeParsing) {
		    xpprintf("main gun at %d,%d\n", inx, iny);
		}
	    }
	    break;

	case 2:		/* Keyword is 'leftGun' */
	    while (teststr) {
		while (*teststr == ' ') teststr++;
		if (sscanf(teststr, "%d,%d", &inx, &iny) != 2) {
		    if (verboseShapeParsing) {
			xpprintf("Missing left gun coordinate in: \"%s\"\n",
			       teststr);
		    }
		    break;
		}
		if (w->num_l_gun >= MAX_GUN_PTS) {
		    if (verboseShapeParsing) {
			xpprintf("Too many left gun coordinates\n");
		    }
		}
		else {
		    l_gun[w->num_l_gun].x = inx;
		    l_gun[w->num_l_gun].y = iny;
		    w->num_l_gun++;
		    if (debugShapeParsing) {
			xpprintf("left gun at %d,%d\n", inx, iny);
		    }
		}
		teststr = strchr(teststr, ' ');
	    }
	    break;

	case 3:		/* Keyword is 'rightGun' */
	    while (teststr) {
		while (*teststr == ' ') teststr++;
		if (sscanf(teststr, "%d,%d" ,&inx, &iny) != 2) {
		    if (verboseShapeParsing) {
			xpprintf("Missing right gun coordinate in: \"%s\"\n",
			       teststr);
		    }
		    break;
		}
		if (w->num_r_gun >= MAX_GUN_PTS) {
		    if (verboseShapeParsing) {
			xpprintf("Too many right gun coordinates\n");
		    }
		}
		else {
		    r_gun[w->num_r_gun].x = inx;
		    r_gun[w->num_r_gun].y = iny;
		    w->num_r_gun++;
		    if (debugShapeParsing) {
			xpprintf("right gun at %d,%d\n", inx, iny);
		    }
		}
		teststr = strchr(teststr, ' ');
	    }
	    break;

	case 4:		/* Keyword is 'leftLight' */
	    while (teststr) {
		while (*teststr == ' ') teststr++;
		if (sscanf(teststr, "%d,%d", &inx, &iny) != 2) {
		    if (verboseShapeParsing) {
			xpprintf("Missing left light coordinate in: \"%s\"\n",
			       teststr);
		    }
		    break;
		}
		if (w->num_l_light >= MAX_LIGHT_PTS) {
		    if (verboseShapeParsing) {
			xpprintf("Too many left light coordinates\n");
		    }
		}
		else {
		    l_light[w->num_l_light].x = inx;
		    l_light[w->num_l_light].y = iny;
		    w->num_l_light++;
		    if (debugShapeParsing) {
			xpprintf("left light at %d,%d\n", inx, iny);
		    }
		}
		teststr = strchr(teststr, ' ');
	    }
	    break;

	case 5:		/* Keyword is 'rightLight' */
	    while (teststr) {
		while (*teststr == ' ') teststr++;
		if (sscanf(teststr, "%d,%d", &inx, &iny) != 2) {
		    if (verboseShapeParsing) {
			xpprintf("Missing right light coordinate in: \"%s\"\n",
			       teststr);
		    }
		    break;
		}
		if (w->num_r_light >= MAX_LIGHT_PTS) {
		    if (verboseShapeParsing) {
			xpprintf("Too many right light coordinates\n");
		    }
		}
		else {
		    r_light[w->num_r_light].x = inx;
		    r_light[w->num_r_light].y = iny;
		    w->num_r_light++;
		    if (debugShapeParsing) {
			xpprintf("right light at %d,%d\n", inx, iny);
		    }
		}
		teststr = strchr(teststr, ' ');
	    }
	    break;

	case 6:		/* Keyword is 'engine' */
	    if (engineSet) {
		if (verboseShapeParsing) {
		    xpprintf("Ship shape keyword \"%s\" multiple defined\n",
			   keyw);
		}
		break;
	    }
	    while (*teststr == ' ') teststr++;
	    if (sscanf(teststr, "%d,%d", &inx, &iny) != 2) {
		if (verboseShapeParsing) {
		    xpprintf("Missing engine coordinate in: \"%s\"\n",
			   teststr);
		}
	    }
	    else {
		engine.x = inx;
		engine.y = iny;
		engineSet = true;
		if (debugShapeParsing) {
		    xpprintf("engine at %d,%d\n", inx, iny);
		}
	    }
	    break;

	case 7:		/* Keyword is 'missileRack' */
	    while (teststr) {
		while (*teststr == ' ') teststr++;
		if (sscanf(teststr, "%d,%d", &inx, &iny) != 2) {
		    if (verboseShapeParsing) {
			xpprintf("Missing missile rack coordinate in: \"%s\"\n",
			       teststr);
		    }
		    break;
		}
		if (w->num_m_rack >= MAX_RACK_PTS) {
		    if (verboseShapeParsing) {
			xpprintf("Too many missile rack coordinates\n");
		    }
		}
		else {
		    m_rack[w->num_m_rack].x = inx;
		    m_rack[w->num_m_rack].y = iny;
		    w->num_m_rack++;
		    if (debugShapeParsing) {
			xpprintf("missile rack at %d,%d\n", inx, iny);
		    }
		}
		teststr = strchr(teststr, ' ');
	    }
	    break;

	case 8:		/* Keyword is 'name' */
#ifdef	_NAMEDSHIPS
	    w->name = xp_strdup(teststr);
	    /* w->name[strlen(w->name)-1] = '\0'; */
#endif
	    break;

	case 9:		/* Keyword is 'author' */
#ifdef	_NAMEDSHIPS
	    w->author = xp_strdup(teststr);
	    /* w->author[strlen(w->author)-1] = '\0'; */
#endif
	    break;

	case 10:		/* Keyword is 'leftRearGun' */
	    while (teststr) {
		while (*teststr == ' ') teststr++;
		if (sscanf(teststr, "%d,%d", &inx, &iny) != 2) {
		    if (verboseShapeParsing) {
			xpprintf("Missing left rear gun coordinate in: \"%s\"\n",
			       teststr);
		    }
		    break;
		}
		if (w->num_l_rgun >= MAX_GUN_PTS) {
		    if (verboseShapeParsing) {
			xpprintf("Too many left rear gun coordinates\n");
		    }
		}
		else {
		    l_rgun[w->num_l_rgun].x = inx;
		    l_rgun[w->num_l_rgun].y = iny;
		    w->num_l_rgun++;
		    if (debugShapeParsing) {
			xpprintf("left rear gun at %d,%d\n", inx, iny);
		    }
		}
		teststr = strchr(teststr, ' ');
	    }
	    break;

	case 11:		/* Keyword is 'rightRearGun' */
	    while (teststr) {
		while (*teststr == ' ') teststr++;
		if (sscanf(teststr, "%d,%d" ,&inx, &iny) != 2) {
		    if (verboseShapeParsing) {
			xpprintf("Missing right rear gun coordinate in: \"%s\"\n",
			       teststr);
		    }
		    break;
		}
		if (w->num_r_rgun >= MAX_GUN_PTS) {
		    if (verboseShapeParsing) {
			xpprintf("Too many right rear gun coordinates\n");
		    }
		}
		else {
		    r_rgun[w->num_r_rgun].x = inx;
		    r_rgun[w->num_r_rgun].y = iny;
		    w->num_r_rgun++;
		    if (debugShapeParsing) {
			xpprintf("right rear gun at %d,%d\n", inx, iny);
		    }
		}
		teststr = strchr(teststr, ' ');
	    }
	    break;

	default:
	    if (verboseShapeParsing) {
		xpprintf("Invalid ship shape keyword: \"%s\"\n", keyw);
	    }
	    /* the good thing about this format is that we can just ignore
	     * this.  it is likely to be a new extension we don't know
	     * about yet. */
	    break;
	}
    }

    if (shape_version == 0x3100) {
	str = ship_shape_str;

	if (sscanf(str, "(%d,%d,%d)", &ofNum, &ofLeft, &ofRight) != 3
	    || ofNum < MIN_SHIP_PTS
	    || ofNum > MAX_SHIP_PTS
	    || ofLeft < 0
	    || ofLeft >= ofNum
	    || ofRight < 0
	    || ofRight >= ofNum
	    ) {
	    if (verboseShapeParsing) {
		xpprintf("Invalid ship shape header: \"%s\"\n", str);
	    }
	    return -1;
	}

	for (i = 0; i < ofNum; i++) {
	    str = strchr(str + 1, '(');
	    if (!str) {
		if (verboseShapeParsing) {
		    xpprintf("Bad ship shape: "
			   "only %d points defined, %d expected\n",
			   i, ofNum);
		}
		return -1;
	    }
	    if (sscanf(str, "(%d,%d)", &inx, &iny) != 2) {
		if (verboseShapeParsing) {
		    xpprintf("Bad ship shape: format error in point %d\n",
			   i);
		}
		return -1;
	    }
	    pt[i].x = inx;
	    pt[i].y = iny;
	}

	w->num_points = ofNum;

	m_gun = pt[0];
	mainGunSet = true;

	l_light[0] = pt[ofLeft];
	w->num_l_light = 1;

	r_light[0] = pt[ofRight];
	w->num_r_light = 1;

	engine.x = (pt[ofLeft].x + pt[ofRight].x) / 2;
	engine.y = (pt[ofLeft].y + pt[ofRight].y) / 2;
	engineSet = true;
    }

    /* Check for some things being set, and give them defaults if not */

    if (w->num_points < 3) {
	if (verboseShapeParsing) {
	    xpprintf("not enough ship points defined\n");
	}
	return -1;
    }
    if (!mainGunSet) {		/* No main gun set, put at foremost point */
	max = 0;
	for (i = 1; i < w->num_points; i++) {
	    if (pt[i].x > pt[max].x
		|| (pt[i].x == pt[max].x
		    && ABS(pt[i].y) < ABS(pt[max].y))) {
		max = i;
	    }
	}
	m_gun = pt[max];
	mainGunSet = true;
    }
    if (!w->num_l_light) {	/* No left light set, put at leftmost point */
	max = 0;
	for (i = 1; i < w->num_points; i++) {
	    if (pt[i].y > pt[max].y
		|| (pt[i].y == pt[max].y
		    && pt[i].x <= pt[max].x)) {
		max = i;
	    }
	}
	l_light[0] = pt[max];
	w->num_l_light = 1;
    }
    if (!w->num_r_light) {	/* No right light set, put at rightmost point */
	max = 0;
	for (i = 1; i < w->num_points; i++) {
	    if (pt[i].y < pt[max].y
		|| (pt[i].y == pt[max].y
		    && pt[i].x <= pt[max].x)) {
		max = i;
	    }
	}
	r_light[0] = pt[max];
	w->num_r_light = 1;
    }
    if (!engineSet) {		/* No engine position, put at rear of ship */
	max = 0;
	for (i = 1; i < w->num_points; i++) {
	    if (pt[i].x < pt[max].x) {
		max = i;
	    }
	}
	engine.x = pt[max].x;
	engine.y = 0;		/* this may lay outside of ship. */
	engineSet = true;
    }
    if (!w->num_m_rack) {	/* No missile racks, put at main gun position*/
	m_rack[0] = m_gun;
	w->num_m_rack = 1;
    }

    if (shapeLimits) {
	const int	isLow = -8, isHi = 8, isLeft = 8, isRight = -8,
			minLow = 1, minHi = 1, minLeft = 1, minRight = 1,
			horMax = 15, verMax = 15, horMin = -15, verMin = -15,
			minCount = 3, minSize = 22 + 16;
	int		low = 0, hi = 0, left = 0, right = 0,
			count = 0, change, max = 0,
			lowest = 0, highest = 0, leftmost = 0, rightmost = 0;
	int		invalid = 0;
	const int	checkWidthAgainstLongestAxis = 1;

	for (i = 0; i < w->num_points; i++) {
	    x = pt[i].x;
	    y = pt[i].y;
	    change = 0;
	    if (y >= isLeft) {
		change++, left++;
		if (y > leftmost) leftmost = y;
	    }
	    if (y <= isRight) {
		change++, right++;
		if (y < rightmost) rightmost = y;
	    }
	    if (x <= isLow) {
		change++, low++;
		if (x < lowest) lowest = x;
	    }
	    if (x >= isHi) {
		change++, hi++;
		if (x > highest) highest = x;
	    }
	    if (change) count++;
	    if (y > horMax || y < horMin) max++;
	    if (x > verMax || x < verMin) max++;
	}
	if (low < minLow
	    || hi < minHi
	    || left < minLeft
	    || right < minRight
	    || count < minCount) {
	    if (verboseShapeParsing) {
		xpprintf("Ship shape does not meet size requirements (%d,%d,%d,%d,%d)\n",
		       low, hi, left, right, count);
	    }
	    return -1;
	}
	if (max != 0) {
	    if (verboseShapeParsing) {
		xpprintf("Ship shape exceeds size maxima.\n");
	    }
	    return -1;
	}
	if (leftmost - rightmost + highest - lowest < minSize) {
	    if (verboseShapeParsing) {
		xpprintf("Ship shape is not big enough.\n"
		       "The ship's width and height added together should\n"
		       "be at least %d.\n", minSize);
	    }
	    return -1;
	}

	if (checkWidthAgainstLongestAxis) {
	    /*
	     * For making sure the ship is the right width!
	     */
	    int pair[2] = { 0, 0 };
	    int dist = 0, tmpDist = 0;
	    double vec[2], width, dTmp;
	    const int minWidth = 12;

	    /*
	     * Loop over all the points and find the two furthest apart
	     */
	    for (i = 0; i < w->num_points; i++) {
		for (j = i + 1; j < w->num_points; j++) {
		    /*
		     * Compare the points if they are not the same ones.
		     * Get this distance -- doesn't matter about sqrting
		     * it since only size is important.
		     */
		    if ((tmpDist = ((pt[i].x - pt[j].x) * (pt[i].x - pt[j].x) +
				    (pt[i].y - pt[j].y) * (pt[i].y - pt[j].y)))
			> dist) {
			/*
			 * Set new separation thingy.
			 */
			dist = tmpDist;
			pair[0] = i;
			pair[1] = j;
		    }
		}
	    }

	    /*
	     * Now we know the vector that is _|_ to the one above
	     * is simply found by (x,y) -> (y,-x) => dot-prod = 0
	     */
	    vec[0] = (double)(pt[pair[1]].y - pt[pair[0]].y);
	    vec[1] = (double)(pt[pair[0]].x - pt[pair[1]].x);

	    /*
	     * Normalise
	     */
	    dTmp = LENGTH(vec[0], vec[1]);
	    vec[0] /= dTmp;
	    vec[1] /= dTmp;

	    /*
	     * Now check the width _|_ to the ship main line.
	     */
	    for (i = 0, width = dTmp = 0.0; i < w->num_points; i++) {
		for (j = i + 1; j < w->num_points; j++) {
		    /*
		     * Check the line if the points are not the same ones
		     */
		    if ((width = fabs(vec[0] * (double)(pt[i].x - pt[j].x) +
				      vec[1] * (double)(pt[i].y - pt[j].y)))
			> dTmp) {
			dTmp = width;
		    }
		}
	    }

	    /*
	     * And make sure it is nice and far away
	     */
	    if (((int)dTmp) < minWidth) {
		if (verboseShapeParsing) {
		    printf("Ship shape is not big enough.\n"
			   "The ship's width should be at least %d.\n"
			   "Player's is %d\n", minWidth, (int)dTmp);
		}
		return -1;
	    }
	}

	/*
	 * Check that none of the special points are outside the
	 * shape defined by the normal points.
	 * First the shape is drawn on a grid.  Then all grid points
	 * on the outside of the shape are marked.  Thusly for each
	 * special point can be determined if it is outside the shape.
	 */
	GRID_RESET();

	/* Draw the ship outline first. */
	for (i = 0; i < w->num_points; i++) {
	    j = i + 1;
	    if (j == w->num_points) j = 0;

	    GRID_PT(pt[i].x, pt[i].y) = 1;

	    dx = pt[j].x - pt[i].x;
	    dy = pt[j].y - pt[i].y;
	    if (ABS(dx) >= ABS(dy)) {
		if (dx > 0) {
		    for (x = pt[i].x + 1; x < pt[j].x; x++) {
			y = pt[i].y + (dy * (x - pt[i].x)) / dx;
			GRID_PT(x, y) = 1;
		    }
		}
		else {
		    for (x = pt[j].x + 1; x < pt[i].x; x++) {
			y = pt[j].y + (dy * (x - pt[j].x)) / dx;
			GRID_PT(x, y) = 1;
		    }
		}
	    }
	    else {
		if (dy > 0) {
		    for (y = pt[i].y + 1; y < pt[j].y; y++) {
			x = pt[i].x + (dx * (y - pt[i].y)) / dy;
			GRID_PT(x, y) = 1;
		    }
		}
		else {
		    for (y = pt[j].y + 1; y < pt[i].y; y++) {
			x = pt[j].x + (dx * (y - pt[j].y)) / dy;
			GRID_PT(x, y) = 1;
		    }
		}
	    }
	}

	/* Check the borders of the grid for blank points. */
	for (y = -15; y <= 15; y++) {
	    for (x = -15; x <= 15; x += (y == -15 || y == 15) ? 1 : 2*15) {
		if (GRID_PT(x, y) == 0) {
		    GRID_ADD(x, y);
		}
	    }
	}

	/* Check from the borders of the grid to the centre. */
	while (!GRID_READY()) {
	    GRID_GET(x, y);
	    if (x <  15 && GRID_PT(x + 1, y) == 0) GRID_ADD(x + 1, y);
	    if (x > -15 && GRID_PT(x - 1, y) == 0) GRID_ADD(x - 1, y);
	    if (y <  15 && GRID_PT(x, y + 1) == 0) GRID_ADD(x, y + 1);
	    if (y > -15 && GRID_PT(x, y - 1) == 0) GRID_ADD(x, y - 1);
	}

	/*
	 * Note that for the engine, old format shapes may well have the
	 * engine position outside the ship, so this check not used for those.
	 */

	if (GRID_CHK(m_gun.x, m_gun.y)) {
	    if (verboseShapeParsing) {
		xpprintf("Main gun outside ship\n");
	    }
	    invalid++;
	}
	for (i = 0; i < w->num_l_gun; i++) {
	    if (GRID_CHK(l_gun[i].x, l_gun[i].y)) {
		if (verboseShapeParsing) {
		    xpprintf("Left gun %d outside ship\n", i);
		}
		invalid++;
	    }
	}
	for (i = 0; i < w->num_r_gun; i++) {
	    if (GRID_CHK(r_gun[i].x, r_gun[i].y)) {
		if (verboseShapeParsing) {
		    xpprintf("Right gun %d outside ship\n", i);
		}
		invalid++;
	    }
	}
	for (i = 0; i < w->num_l_rgun; i++) {
	    if (GRID_CHK(l_rgun[i].x, l_rgun[i].y)) {
		if (verboseShapeParsing) {
		    xpprintf("Left rear gun %d outside ship\n", i);
		}
		invalid++;
	    }
	}
	for (i = 0; i < w->num_r_rgun; i++) {
	    if (GRID_CHK(r_rgun[i].x, r_rgun[i].y)) {
		if (verboseShapeParsing) {
		    xpprintf("Right rear gun %d outside ship\n", i);
		}
		invalid++;
	    }
	}
	for (i = 0; i < w->num_m_rack; i++) {
	    if (GRID_CHK(m_rack[i].x, m_rack[i].y)) {
		if (verboseShapeParsing) {
		    xpprintf("Missile rack %d outside ship\n", i);
		}
		invalid++;
	    }
	}
	for (i = 0; i < w->num_l_light; i++) {
	    if (GRID_CHK(l_light[i].x, l_light[i].y)) {
		if (verboseShapeParsing) {
		    xpprintf("Left light %d outside ship\n", i);
		}
		invalid++;
	    }
	}
	for (i = 0; i < w->num_r_light; i++) {
	    if (GRID_CHK(r_light[i].x, r_light[i].y)) {
		if (verboseShapeParsing) {
		    xpprintf("Right light %d outside ship\n", i);
		}
		invalid++;
	    }
	}
	if (GRID_CHK(engine.x, engine.y)) {
	    if (verboseShapeParsing) {
		xpprintf("Engine outside of ship\n");
	    }
	    invalid++;
	    /* this could happen in case of an old format ship shape. */
	    if (shape_version == 0x3100 && invalid == 1) {
		/* move engine until it is legal. */
		for (x = -15, y = 0; x <= 15; x++) {
		    if (!GRID_CHK(x, y)) {
			engine.x = x;
			engine.y = y;
			invalid--;
			break;
		    }
		}
	    }
	}

	if (debugShapeParsing) {
	    for (i = -15; i <= 15; i++) {
		for (j = -15; j <= 15; j++) {
		    switch (GRID_PT(j, i)) {
		    case 0: putchar(' '); break;
		    case 1: putchar('*'); break;
		    case 2: putchar('.'); break;
		    default: putchar('?'); break;
		    }
		}
		putchar('\n');
	    }
	}

	if (invalid != 0) {
	    return -1;
	}
    }

    i = sizeof(position) * RES;
    if (!(w->pts[0] = (position*)malloc(w->num_points * i))
	|| (w->num_l_gun
	    && !(w->l_gun[0] = (position*)malloc(w->num_l_gun * i)))
	|| (w->num_r_gun
	    && !(w->r_gun[0] = (position*)malloc(w->num_r_gun * i)))
	|| (w->num_l_rgun
	    && !(w->l_rgun[0] = (position*)malloc(w->num_l_rgun * i)))
	|| (w->num_r_rgun
	    && !(w->r_rgun[0] = (position*)malloc(w->num_r_rgun * i)))
	|| (w->num_l_light
	    && !(w->l_light[0] = (position*)malloc(w->num_l_light * i)))
	|| (w->num_r_light
	    && !(w->r_light[0] = (position*)malloc(w->num_r_light * i)))
	|| (w->num_m_rack
	    && !(w->m_rack[0] = (position*)malloc(w->num_m_rack * i)))
	) {
	error("Not enough memory for ship shape");
	if (w->pts[0]) {
	    free(w->pts[0]);
	    if (w->l_gun[0]) {
		free(w->l_gun[0]);
		if (w->r_gun[0]) {
		    free(w->r_gun[0]);
		    if (w->l_rgun[0]) {
			free(w->l_rgun[0]);
			if (w->r_rgun[0]) {
			    free(w->r_rgun[0]);
			    if (w->l_light[0]) {
				free(w->l_light[0]);
				if (w->r_light[0]) {
				    free(w->r_light[0]);
				    if (w->m_rack[0]) {
					free(w->m_rack[0]);
				    }
				}
			    }
			}
		    }
		}
	    }
	}
	return -1;
    }

    for (i = 1; i < w->num_points; i++) {
	w->pts[i] = &w->pts[i - 1][RES];
    }
    for (i = 1; i < w->num_l_gun; i++) {
	w->l_gun[i] = &w->l_gun[i - 1][RES];
    }
    for (i = 1; i < w->num_r_gun; i++) {
	w->r_gun[i] = &w->r_gun[i - 1][RES];
    }
    for (i = 1; i < w->num_l_rgun; i++) {
	w->l_rgun[i] = &w->l_rgun[i - 1][RES];
    }
    for (i = 1; i < w->num_r_rgun; i++) {
	w->r_rgun[i] = &w->r_rgun[i - 1][RES];
    }
    for (i = 1; i < w->num_l_light; i++) {
	w->l_light[i] = &w->l_light[i - 1][RES];
    }
    for (i = 1; i < w->num_r_light; i++) {
	w->r_light[i] = &w->r_light[i - 1][RES];
    }
    for (i = 1; i < w->num_m_rack; i++) {
	w->m_rack[i] = &w->m_rack[i - 1][RES];
    }

    for (i = 0; i < w->num_points; i++) {
	w->pts[i][0].x = pt[i].x;
	w->pts[i][0].y = pt[i].y;
    }
    if (engineSet) {
	w->engine[0].x = engine.x;
	w->engine[0].y = engine.y;
    }
    if (mainGunSet) {
	w->m_gun[0].x = m_gun.x;
	w->m_gun[0].y = m_gun.y;
    }
    for (i = 0; i < w->num_l_gun; i++) {
	w->l_gun[i][0].x = l_gun[i].x;
	w->l_gun[i][0].y = l_gun[i].y;
    }
    for (i = 0; i < w->num_r_gun; i++) {
	w->r_gun[i][0].x = r_gun[i].x;
	w->r_gun[i][0].y = r_gun[i].y;
    }
    for (i = 0; i < w->num_l_rgun; i++) {
	w->l_rgun[i][0].x = l_rgun[i].x;
	w->l_rgun[i][0].y = l_rgun[i].y;
    }
    for (i = 0; i < w->num_r_rgun; i++) {
	w->r_rgun[i][0].x = r_rgun[i].x;
	w->r_rgun[i][0].y = r_rgun[i].y;
    }
    for (i = 0; i < w->num_l_light; i++) {
	w->l_light[i][0].x = l_light[i].x;
	w->l_light[i][0].y = l_light[i].y;
    }
    for (i = 0; i < w->num_r_light; i++) {
	w->r_light[i][0].x = r_light[i].x;
	w->r_light[i][0].y = r_light[i].y;
    }
    for (i = 0; i < w->num_m_rack; i++) {
	w->m_rack[i][0].x = m_rack[i].x;
	w->m_rack[i][0].y = m_rack[i].y;
    }
    Rotate_ship(w);

    return 0;
}

static shipobj *do_parse_shape(char *str)
{
    shipobj		*w;

    if (!str || !*str) {
	if (debugShapeParsing) {
	    xpprintf("shape str not set\n");
	}
	return Default_ship();
    }
    if (!(w = (shipobj *)malloc(sizeof(*w)))) {
	error("No mem for ship shape");
	return Default_ship();
    }
    if (shape2wire(str, w) != 0) {
	free(w);
	if (debugShapeParsing) {
	    xpprintf("shape2wire failed\n");
	}
	return Default_ship();
    }
    if (debugShapeParsing) {
	xpprintf("shape2wire succeeded\n");
    }

    return(w);
}

void Free_ship_shape(shipobj *w)
{
    if (w != NULL && w != Default_ship()) {
	if (w->num_points > 0 && w->pts[0]) free(w->pts[0]);
	if (w->num_l_gun > 0 && w->l_gun[0]) free(w->l_gun[0]);
	if (w->num_r_gun > 0 && w->r_gun[0]) free(w->r_gun[0]);
	if (w->num_l_rgun > 0 && w->l_rgun[0]) free(w->l_rgun[0]);
	if (w->num_r_rgun > 0 && w->r_rgun[0]) free(w->r_rgun[0]);
	if (w->num_l_light > 0 && w->l_light[0]) free(w->l_light[0]);
	if (w->num_r_light > 0 && w->r_light[0]) free(w->r_light[0]);
	if (w->num_m_rack > 0 && w->m_rack[0]) free(w->m_rack[0]);
#ifdef	_NAMEDSHIPS
	if (w->name) free(w->name);
	if (w->author) free(w->author);
#endif
	free(w);
    }
}

shipobj *Parse_shape_str(char *str)
{
    verboseShapeParsing = debugShapeParsing;
    shapeLimits = 1;
    return do_parse_shape(str);
}

shipobj *Convert_shape_str(char *str)
{
    verboseShapeParsing = debugShapeParsing;
    shapeLimits = debugShapeParsing;
    return do_parse_shape(str);
}

int Validate_shape_str(char *str)
{
    shipobj		*w;

    verboseShapeParsing = 1;
    shapeLimits = 1;
    w = do_parse_shape(str);
    Free_ship_shape(w);
    return (w && w != Default_ship());
}

void Convert_ship_2_string(shipobj *w, char *buf, char *ext,
			   unsigned shape_version)
{
    char		tmp[MSG_LEN];
    int			i,
			buflen,
			extlen,
			tmplen,
			ll,
			rl;

    ext[extlen = 0] = '\0';

    if (shape_version >= 0x3200) {
	strcpy(buf, "(SH:");
	buflen = strlen(&buf[0]);
	for (i = 0; i < w->num_points && i < MAX_SHIP_PTS; i++) {
	    sprintf(&buf[buflen], " %d,%d",
		    (int)w->pts[i][0].x, (int)w->pts[i][0].y);
	    buflen += strlen(&buf[buflen]);
	}
	sprintf(&buf[buflen], ")(EN: %d,%d)(MG: %d,%d)",
		(int)w->engine[0].x, (int)w->engine[0].y,
		(int)w->m_gun[0].x, (int)w->m_gun[0].y);
	buflen += strlen(&buf[buflen]);

	/*
	 * If the calculations are correct then only from here on
	 * there is danger for overflowing the MSG_LEN size
	 * of the buffer.  Therefore first copy a new pair of
	 * parentheses into a temporary buffer and when the closing
	 * parenthesis is reached then determine if there is enough
	 * room in the main buffer or else copy it into the extended
	 * buffer.  This scheme allows cooperation with versions which
	 * didn't had the extended buffer yet for which the extended
	 * buffer will simply be discarded.
	 */
	if (w->num_l_gun > 0) {
	    strcpy(&tmp[0], "(LG:");
	    tmplen = strlen(&tmp[0]);
	    for (i = 0; i < w->num_l_gun && i < MAX_GUN_PTS; i++) {
		sprintf(&tmp[tmplen], " %d,%d",
			(int)w->l_gun[i][0].x, (int)w->l_gun[i][0].y);
		tmplen += strlen(&tmp[tmplen]);
	    }
	    strcpy(&tmp[tmplen], ")");
	    tmplen++;
	    if (buflen + tmplen < MSG_LEN) {
		strcpy(&buf[buflen], tmp);
		buflen += tmplen;
	    }
	    else if (extlen + tmplen < MSG_LEN) {
		strcpy(&ext[extlen], tmp);
		extlen += tmplen;
	    }
	}
	if (w->num_r_gun > 0) {
	    strcpy(&tmp[0], "(RG:");
	    tmplen = strlen(&tmp[0]);
	    for (i = 0; i < w->num_r_gun && i < MAX_GUN_PTS; i++) {
		sprintf(&tmp[tmplen], " %d,%d",
			(int)w->r_gun[i][0].x, (int)w->r_gun[i][0].y);
		tmplen += strlen(&tmp[tmplen]);
	    }
	    strcpy(&tmp[tmplen], ")");
	    tmplen++;
	    if (buflen + tmplen < MSG_LEN) {
		strcpy(&buf[buflen], tmp);
		buflen += tmplen;
	    }
	    else if (extlen + tmplen < MSG_LEN) {
		strcpy(&ext[extlen], tmp);
		extlen += tmplen;
	    }
	}
	if (w->num_l_rgun > 0) {
	    strcpy(&tmp[0], "(LR:");
	    tmplen = strlen(&tmp[0]);
	    for (i = 0; i < w->num_l_rgun && i < MAX_GUN_PTS; i++) {
		sprintf(&tmp[tmplen], " %d,%d",
			(int)w->l_rgun[i][0].x, (int)w->l_rgun[i][0].y);
		tmplen += strlen(&tmp[tmplen]);
	    }
	    strcpy(&tmp[tmplen], ")");
	    tmplen++;
	    if (buflen + tmplen < MSG_LEN) {
		strcpy(&buf[buflen], tmp);
		buflen += tmplen;
	    }
	    else if (extlen + tmplen < MSG_LEN) {
		strcpy(&ext[extlen], tmp);
		extlen += tmplen;
	    }
	}
	if (w->num_r_rgun > 0) {
	    strcpy(&tmp[0], "(RR:");
	    tmplen = strlen(&tmp[0]);
	    for (i = 0; i < w->num_r_rgun && i < MAX_GUN_PTS; i++) {
		sprintf(&tmp[tmplen], " %d,%d",
			(int)w->r_rgun[i][0].x, (int)w->r_rgun[i][0].y);
		tmplen += strlen(&tmp[tmplen]);
	    }
	    strcpy(&tmp[tmplen], ")");
	    tmplen++;
	    if (buflen + tmplen < MSG_LEN) {
		strcpy(&buf[buflen], tmp);
		buflen += tmplen;
	    }
	    else if (extlen + tmplen < MSG_LEN) {
		strcpy(&ext[extlen], tmp);
		extlen += tmplen;
	    }
	}
	if (w->num_l_light > 0) {
	    strcpy(&tmp[0], "(LL:");
	    tmplen = strlen(&tmp[0]);
	    for (i = 0; i < w->num_l_light && i < MAX_LIGHT_PTS; i++) {
		sprintf(&tmp[tmplen], " %d,%d",
			(int)w->l_light[i][0].x, (int)w->l_light[i][0].y);
		tmplen += strlen(&tmp[tmplen]);
	    }
	    strcpy(&tmp[tmplen], ")");
	    tmplen++;
	    if (buflen + tmplen < MSG_LEN) {
		strcpy(&buf[buflen], tmp);
		buflen += tmplen;
	    }
	    else if (extlen + tmplen < MSG_LEN) {
		strcpy(&ext[extlen], tmp);
		extlen += tmplen;
	    }
	}
	if (w->num_r_light > 0) {
	    strcpy(&tmp[0], "(RL:");
	    tmplen = strlen(&tmp[0]);
	    for (i = 0; i < w->num_r_light && i < MAX_LIGHT_PTS; i++) {
		sprintf(&tmp[tmplen], " %d,%d",
			(int)w->r_light[i][0].x, (int)w->r_light[i][0].y);
		tmplen += strlen(&tmp[tmplen]);
	    }
	    strcpy(&tmp[tmplen], ")");
	    tmplen++;
	    if (buflen + tmplen < MSG_LEN) {
		strcpy(&buf[buflen], tmp);
		buflen += tmplen;
	    }
	    else if (extlen + tmplen < MSG_LEN) {
		strcpy(&ext[extlen], tmp);
		extlen += tmplen;
	    }
	}
	if (w->num_m_rack > 0) {
	    strcpy(&tmp[0], "(MR:");
	    tmplen = strlen(&tmp[0]);
	    for (i = 0; i < w->num_m_rack && i < MAX_RACK_PTS; i++) {
		sprintf(&tmp[tmplen], " %d,%d",
			(int)w->m_rack[i][0].x, (int)w->m_rack[i][0].y);
		tmplen += strlen(&tmp[tmplen]);
	    }
	    strcpy(&tmp[tmplen], ")");
	    tmplen++;
	    if (buflen + tmplen < MSG_LEN) {
		strcpy(&buf[buflen], tmp);
		buflen += tmplen;
	    }
	    else if (extlen + tmplen < MSG_LEN) {
		strcpy(&ext[extlen], tmp);
		extlen += tmplen;
	    }
	}
    }
    else {
	/* 3.1 version had 16 points maximum.  just ignore the excess. */
	int num_points = MIN(w->num_points, 16);
#if 0
	if (num_points < w->num_points) {
	    printf("Truncating ship to 16 points for old 3.1 server\n");
	}
#endif
	if (shape_version != 0x3100) {
	    errno = 0;
	    error("Unknown ship shape version: %x", shape_version);
	}

	for (i = 1, ll = rl = 0; i < num_points; i++) {
	    if (w->pts[i][0].y > w->pts[ll][0].y
		|| (w->pts[i][0].y == w->pts[ll][0].y
		    && w->pts[i][0].x < w->pts[ll][0].x)) {
		ll = i;
	    }
	    if (w->pts[i][0].y < w->pts[rl][0].y
		|| (w->pts[i][0].y == w->pts[rl][0].y
		    && w->pts[i][0].x < w->pts[rl][0].x)) {
		rl = i;
	    }
	}
	sprintf(buf, "(%d,%d,%d)", num_points, ll, rl);
	buflen = strlen(buf);
	for (i = 0; i < num_points; i++) {
	    sprintf(&buf[buflen], "(%d,%d)",
		    (int)w->pts[i][0].x, (int)w->pts[i][0].y);
	    buflen += strlen(&buf[buflen]);
	}
    }
    if (buflen >= MSG_LEN || extlen >= MSG_LEN) {
	errno = 0;
	error("BUG: convert ship: buffer overflow (%d,%d)", buflen, extlen);
    }
    if (debugShapeParsing) {
	xpprintf("ship 2 str: %s %s\n", buf, ext);
    }
}

static int Get_shape_keyword(char *keyw)
{
#define NUM_SHAPE_KEYS	12

    static char		shape_keys[NUM_SHAPE_KEYS][16] = {
			    "shape:",
			    "mainGun:",
			    "leftGun:",
			    "rightGun:",
			    "leftLight:",
			    "rightLight:",
			    "engine:",
			    "missileRack:",
			    "name:",
			    "author:",
			    "leftRearGun:",
			    "rightRearGun:",
			};
    static char		abbrev_keys[NUM_SHAPE_KEYS][4] = {
			    "SH:",
			    "MG:",
			    "LG:",
			    "RG:",
			    "LL:",
			    "RL:",
			    "EN:",
			    "MR:",
			    "NM:",
			    "AU:",
			    "LR:",
			    "RR:",
			};
    int			i;

    /* non-abbreviated keywords start with a lower case letter. */
    if (islower(*keyw)) {
	for (i = 0; i < NUM_SHAPE_KEYS; i++) {
	    if (!strcmp(keyw, shape_keys[i])) {
		break;
	    }
	}
    }
    /* abbreviated keywords start with an upper case letter. */
    else if (isupper(*keyw)) {
	for (i = 0; i < NUM_SHAPE_KEYS; i++) {
	    if (!strcmp(keyw, abbrev_keys[i])) {
		break;
	    }
	}
    }
    /* dunno what this is. */
    else {
	i = -1;
    }
    return(i);
}

void Calculate_shield_radius(shipobj *w)
{
    int			i;
    int			radius2, max_radius = 0;

    for (i = 0; i < w->num_points; i++) {
	radius2 = (int)(sqr(w->pts[i][0].x) + sqr(w->pts[i][0].y));
	if (radius2 > max_radius) {
	    max_radius = radius2;
	}
    }
    max_radius = (int)(2.0 * sqrt(max_radius));
    w->shield_radius = (max_radius + 2 <= 34)
			? 34
			: (max_radius + 2 - (max_radius & 1));
}


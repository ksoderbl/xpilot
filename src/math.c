/* $Id: math.c,v 1.6 1993/04/14 16:50:00 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include "global.h"


#define GRAV_RANGE  10
#define PTS_IN_SHIP 3

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: math.c,v 1.6 1993/04/14 16:50:00 bjoerns Exp $";
#endif

float  	tbl_sin[TABLE_SIZE];
wireobj		ships[RES];


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


float Wrap_findDir(float dx, float dy)
{
    dx = WRAP_DX(dx);
    dy = WRAP_DY(dy);
    return findDir(dx, dy);
}


float Wrap_length(float dx, float dy)
{
    dx = WRAP_DX(dx);
    dy = WRAP_DY(dy);
    return LENGTH(dx, dy);
}


void Make_table(void)
{
    int i;

    for (i=0; i<TABLE_SIZE; i++)
	tbl_sin[i] = sin((2*PI) * ((float)i/TABLE_SIZE));
}


void Make_ships(void)
{
    int i, z;


    ships[0].pts=(position *)malloc(PTS_IN_SHIP*sizeof(position));
    ships[0].pts[0].x = 15; ships[0].pts[0].y =  0;
    ships[0].pts[1].x = -9; ships[0].pts[1].y =  8;
    ships[0].pts[2].x = -9; ships[0].pts[2].y = -8;
    ships[0].num_points=PTS_IN_SHIP;

    for (i=1; i<RES; i++) {
	ships[i].pts=(position *)malloc(PTS_IN_SHIP*sizeof(position));

	for (z=0; z<PTS_IN_SHIP; z++) {
	    ships[i].pts[z].x = tcos(i)*ships[0].pts[z].x
		- tsin(i)*ships[0].pts[z].y;
	    ships[i].pts[z].y = tsin(i)*ships[0].pts[z].x
		+ tcos(i)*ships[0].pts[z].y;
	}
    }
}


void Free_ships(void)
{
    int dir;

    for (dir=0; dir<RES; dir++)
	free(ships[dir].pts);
}


void Compute_gravity(void)
{
    int xi, yi, g, gx, gy;
    int first_xi, last_xi, first_yi, last_yi, mod_xi, mod_yi, wrap;
    float theta, dist2, dx, dy;


    for (xi=0; xi<World.x; xi++) {
        vector *line = World.gravity[xi];

	for (yi=0; yi<World.y; yi++) {
	    line[yi].y = Gravity;
	    line[yi].x = 0.0;
	}
    }

    wrap = (BIT(World.rules->mode, WRAP_PLAY) != 0);

    for (g=0; g<World.NumGravs; g++) {
	gx = World.grav[g].pos.x;
	gy = World.grav[g].pos.y;

	if (gx - GRAV_RANGE >= 0 || wrap) {
	    first_xi = gx - GRAV_RANGE;
	} else {
	    first_xi = 0;
	}
	if (gx + GRAV_RANGE < World.x || wrap) {
	    last_xi = gx + GRAV_RANGE;
	} else {
	    last_xi = World.x - 1;
	}
	if (gy - GRAV_RANGE >= 0 || wrap) {
	    first_yi = gy - GRAV_RANGE;
	} else {
	    first_yi = 0;
	}
	if (gy + GRAV_RANGE < World.y || wrap) {
	    last_yi = gy + GRAV_RANGE;
	} else {
	    last_yi = World.y - 1;
	}
	for (xi = first_xi; xi <= last_xi; xi++) {
	    vector *line = World.gravity[mod_xi = mod(xi, World.x)];

	    for (yi = first_yi; yi <= last_yi; yi++) {

		mod_yi = (wrap) ? mod(yi, World.y) : yi;

		dx = gx - xi;
		dy = gy - yi;

		if ((dy == 0) && (dx == 0))	/* In a grav? */
		    continue;

		if (dx != 0.0) {
		    theta = atan2(dy, dx);
		} else
		    if (dy > 0)
			theta = PI/2.0;
		    else
			theta = 3.0 * PI/2.0;

		if (World.block[gx][gy] == CWISE_GRAV
		    || World.block[gx][gy] == ACWISE_GRAV)
		    theta += PI/2.0;

		dist2 = sqr(dx) + sqr(dy);
		line[mod_yi].x += cos(theta) * World.grav[g].force / dist2;
		line[mod_yi].y += sin(theta) * World.grav[g].force / dist2;
	    }
        }
    }
}

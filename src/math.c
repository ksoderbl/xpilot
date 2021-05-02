/* $Id: math.c,v 3.2 1993/06/28 20:53:46 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include "global.h"


#define PTS_IN_SHIP 3

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: math.c,v 3.2 1993/06/28 20:53:46 bjoerns Exp $";
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

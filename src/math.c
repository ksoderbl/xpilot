/* math.c,v 1.3 1992/05/11 15:31:21 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include "pilot.h"
#include "map.h"

#define GRAV_RANGE  10
#define PTS_IN_SHIP 3

double		    tbl_sin[TABLE_SIZE];
wireobj		    ships[RESOLUTION];

extern World_map    World;
extern double	    Gravity;



void Make_table(void)
{
    int i;

    for (i=0; i<TABLE_SIZE; i++)
	tbl_sin[i] = sin((2*PI) * ((double)i/TABLE_SIZE));
}


void Make_ships(void)
{
    int i, z;


    ships[0].pts=(position *)malloc(PTS_IN_SHIP*sizeof(position));
    ships[0].pts[0].x=15; ships[0].pts[0].y= 0;
    ships[0].pts[1].x=-9; ships[0].pts[1].y= 8;
    ships[0].pts[2].x=-9; ships[0].pts[2].y=-8;
    ships[0].ant_points=PTS_IN_SHIP;

    for (i=1; i<RESOLUTION; i++) {
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


    for (dir=0; dir<RESOLUTION; dir++)
	free(ships[dir].pts);
}


void Compute_gravity(void)
{
    int xi, yi, g, gx, gy;
    double theta, avst, dx, dy;


    for (xi=0; xi<World.x; xi++)
	for (yi=0; yi<World.y; yi++) {
	    World.gravity[xi][yi].y = Gravity;
	    World.gravity[xi][yi].x = 0.0;
	}

    for (g=0; g<World.Ant_gravs; g++) {
	gx = World.gravs[g].pos.x;
	gy = World.gravs[g].pos.y;

	for (xi = (gx>GRAV_RANGE) ? gx-GRAV_RANGE : 0;
	     (xi<gx+GRAV_RANGE) && (xi<World.x); xi++)
	    for (yi = (gy>GRAV_RANGE) ? gy-GRAV_RANGE : 0;
		 (yi<gy+GRAV_RANGE) && (yi<World.y); yi++) {
		dx = (double)(gx - xi);
		dy = (double)(gy - yi);

		if ((dy == 0) && (dx == 0))	/* In a grav? */
		    continue;

		avst = (double)LENGTH(dx, dy);
		if (dx != 0.0) {
		    theta = atan2(dy, dx);
		} else
		    if (dy > 0)
			theta = PI/2.0;
		    else
			theta = 3.0*PI/2.0;

		if (World.type[gx][gy] == CWISE_GRAV ||
		    World.type[gx][gy] == ACWISE_GRAV)
		    theta += PI/2.0;

		World.gravity[xi][yi].x += cos(theta)*World.gravs[g].force/
		    sqr(avst);
		World.gravity[xi][yi].y += sin(theta)*World.gravs[g].force/
		    sqr(avst);
	    }
    }
}

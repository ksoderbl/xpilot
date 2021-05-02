/* $Id: gfx3d.c,v 5.0 2001/04/07 20:00:58 dik Exp $
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

#include "gfx3d.h"
#include "const.h"

int Angle_add(int x, int dx)
{
    x += dx;
    if ( x > RES) 
	x -= RES;
    if ( x < 0) 
	x += RES;
    return x;
}

void Rotate_point3d(point3d *sv, point3d *v,
		  int angle_x, int angle_y, int angle_z) 
{


    double tempx, tempy, tempz, newx, newy, newz;
    double xSin, xCos, ySin, yCos, zSin, zCos;

    newx=sv->x;
    newy=sv->y;
    newz=sv->z;


    xSin = tsin(angle_x);
    xCos = tcos(angle_x);

    ySin = tsin(angle_y);
    yCos = tcos(angle_y);

    zSin = tsin(angle_z);
    zCos = tcos(angle_z);

/* rotate coord around z axis */
    tempx = newx*zCos - newy*zSin;
    tempy = newx*zSin + newy*zCos;
    newx = tempx;
    newy = tempy;

  /* then y axis */
    tempx=newx*yCos - newz*ySin;
    tempz=newx*ySin + newz*yCos;
    newx=tempx;
    newz=tempz;

  /* then x axis */
    tempy=newy*xCos - newz*xSin;
    tempz=newy*xSin + newz*xCos;

    v->x=(int)tempx;
    v->y=(int)tempy;
    v->z=(int)tempz;
}

void Project_point3d(point3d *v,int zOffset) 
{
    v->x = v->x*70/(v->z+zOffset);
    v->y = -(v->y*70/(v->z+zOffset));
}


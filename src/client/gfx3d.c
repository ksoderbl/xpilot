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


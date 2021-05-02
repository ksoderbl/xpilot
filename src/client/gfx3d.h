#ifndef GFX3D_H
#define GFX3D_H

#define RADIANS		(3.14159265 / 180.0)

typedef struct {
    int		x;
    int		y;
    int		z;
} point3d;

typedef struct {
    int		from;
    int		to;
} line3d;

int Angle_add(int x, int dx);

void Rotate_point3d(point3d *sv, point3d *v,
		    int angle_x, int angle_y, int angle_z);

void Project_point3d(point3d *v,int zOffset);

#endif

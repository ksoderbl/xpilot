/* $Id: gfx2d.h,v 5.0 2001/04/07 20:00:58 dik Exp $
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
#ifndef GFX2D_H
#define GFX2D_H

typedef unsigned int RGB_COLOR;

#define RGB24(r, g, b)  ((((b)&255) << 16) | (((g)&255) << 8) | ((r)&255))

#define RED_VALUE(col) ((col) &255)
#define GREEN_VALUE(col) (((col) >> 8) &255)
#define BLUE_VALUE(col) (((col) >>16) &255)

/*
    Purpose: bounding box for one image or a set of images.
    The xmin and ymin elements give the lowest coordinate which
    has a non-black color value.  The xmax and ymax elements
    give the highest coordinate which has a non-black color.
    The number of pixels covered by one box is given by:
	(xmax + 1 - xmin, ymax + 1 - ymin).
*/
typedef struct {
    int		xmin, ymin;
    int		xmax, ymax;
} bbox_t;


/*
    Purpose: A device/os independent structure to do keep 24bit images in.
    an instance of xp_picture_t can contain more than 1 image, 
    This feature is  useful for structural identical bitmaps (example: items), 
    and rotated images. When dealing with rotated images, the first image
    in the xp_picture_t structure is used as texture for the transformation of the 
    others.
*/

typedef struct {
    int		width, height;
    int		images;
    RGB_COLOR	**data;

    bbox_t	*bbox;
} xp_picture_t;

int Picture_init(xp_picture_t *picture, int height, int width, int images);
int Picture_load( xp_picture_t *picture, const char *path);
void Picture_rotate(xp_picture_t *picture);

void Picture_set_pixel(xp_picture_t *picture, int image, int x, int y, 
		       RGB_COLOR color);
RGB_COLOR Picture_get_rotated_pixel(const xp_picture_t *picture, 
				    int x, int y, int image);
RGB_COLOR Picture_get_pixel(const xp_picture_t *picture, int image,
			    int x, int y);
RGB_COLOR Picture_get_pixel_area(const xp_picture_t *picture, int image, 
				 double x1, double y1, double dx, double dy);
void Picture_get_bounding_box(xp_picture_t *picture);

#endif

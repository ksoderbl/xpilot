/* $Id: blockbitmaps.h,v 5.1 2002/02/10 19:29:39 bertg Exp $
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

#ifndef	BLOCKBITMAPS_H
#define	BLOCKBITMAPS_H

#include "gfx2d.h"

#define BM_HOLDER_FRIEND 0
#define BM_HOLDER_ENEMY  1
#define BM_BALL		 2
#define BM_SHIP_SELF	 3
#define BM_SHIP_FRIEND	 4
#define BM_SHIP_ENEMY	 5
#define BM_BULLET	 6
#define BM_BULLET_OWN	 7
#define BM_BASE_DOWN	 8
#define BM_BASE_LEFT	 9
#define BM_BASE_UP	10
#define BM_BASE_RIGHT	11
#define BM_FUELCELL	12
#define BM_FUEL		13
#define BM_ALL_ITEMS	14
#define BM_CANNON_DOWN  15
#define BM_CANNON_LEFT  16
#define BM_CANNON_UP	17
#define BM_CANNON_RIGHT 18
#define BM_SPARKS	19
#define BM_PAUSED	20
#define BM_WALL_TOP	21
#define BM_WALL_LEFT	22
#define BM_WALL_BOTTOM	23
#define BM_WALL_RIGHT	24
#define BM_WALL_LU	25
#define BM_WALL_RU	26
#define BM_WALL_LD	27
#define BM_WALL_RD	28

#define BM_WALL_FILLED  29
#define BM_WALL_UR	30
#define BM_WALL_UL	31

#define BM_SCORE_BG	32
#define BM_LOGO		33
#define BM_REFUEL	34
#define BM_WORMHOLE	35
#define BM_MINE_TEAM    36
#define BM_MINE_OTHER	37
#define BM_CONCENTRATOR 38
#define BM_PLUSGRAVITY  39
#define BM_MINUSGRAVITY 40
#define BM_CHECKPOINT	41
#define BM_METER	42
#define BM_ASTEROIDCONC	43

#define NUM_BITMAPS 44

#define BG_IMAGE_HEIGHT 442  
#define LOGO_HEIGHT     223

#define RADAR_TEXTURE_SIZE 32


typedef struct {
    Pixmap		bitmap;
    int			scale_width, scale_height;    

    Pixmap		mask;
    bbox_t		bbox;
} xp_bitmap_t;

/* XXX need comment about purpose of this structure. */
typedef struct {
    const char		*filename;
    int			rotations;

    int			width, height;
    xp_bitmap_t		*bitmaps;
    xp_picture_t	picture;
} xp_pixmap_t;

extern xp_pixmap_t xp_pixmaps[];

void PaintBitmap(Drawable d, int type, int x, int y, int width, int height,
		 int number);
void PaintFuelSlice(Drawable d, int type, int x, int y, int width, int height, 
		    int image, int size); 
void PaintMeter(Drawable d, int type, int x, int y, int width, int height, 
		int size);

int Block_bitmap_images(int type);
int Block_bitmaps_create(void);

void Block_bitmap_create_begin(Drawable d, 
			       xp_pixmap_t *xp_pixmap, int image,
			       int width, int height);

void Block_bitmap_create_end(Drawable d);

void Block_bitmap_set_pixel(xp_pixmap_t *xp_pixmap, int image, int x, int y, 
			    RGB_COLOR color);
void Block_bitmap_paint(Drawable d, int type, int x, int y, 
			int width, int height,
		 int number);

void Cache_ships(Drawable d);

void Block_bitmap_create(Display* dpy, Drawable d, 
			 xp_pixmap_t *xp_pixmap, int number,
			 int width, int height);

void Block_bitmap_paint_fuel_slice(Drawable d, int type, int x, int y, 
				   int width, int height, 
				   int image, int size);

void Block_bitmap_paint_meter(Drawable d, int type, int x, int y, 
			      int width, int height, 
			      int size);

#endif

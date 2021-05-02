/* $Id: blockbitmaps.c,v 5.3 2001/06/03 17:21:06 bertg Exp $
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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#ifndef _WINDOWS
# include <X11/Xlib.h>
#endif

#ifdef _WINDOWS
# include "../common/NT/winX.h"
# include "NT/winbitmap.h"
# include "NT/winClient.h"
#endif 

#include "blockbitmaps.h"
#include "gfx2d.h"

#include "version.h"
#include "error.h"
#include "const.h"
#include "paint.h"
#include "portability.h"


char blockbitmaps_version[] = VERSION;


int scaled_bitmaps = 0;
extern xp_picture_t radar_colors;


void Block_bitmap_create(Display* dpy, Drawable d, xp_pixmap_t *xp_pixmap, int number,
		   int width, int height);
static void Block_bitmap_picture_copy(xp_pixmap_t *xp_pixmap, int image);
static void Block_bitmap_picture_scale(xp_pixmap_t *xp_pixmap, int image);

/* 
    i got a terrible feeling that the right thing is to add all of 
    these to options in default.c? 
    (2 options for all? (one for filename, one for on/off), and those who can be 
    rotated might need a value that specifies the resolution. 
    (the 3 ships alone really chews memory : 128*32*32*4*3 bytes of space (~3072K))
    
    This means a number of options explosion, but is there any alternative?
*/



xp_pixmap_t xp_pixmaps[] = {
    { "holder1.ppm"	    , 1 , 35, 35},
    { "holder2.ppm"	    , 1 , 35, 35},
    { "ball.ppm"	    , 1 , 21, 21},
    { "ship_red.ppm"	    , 128, 32, 32},
    { "ship_blue.ppm"	    , 128, 32, 32},
    { "ship_red2.ppm"	    , 128, 32, 32},
    { "bullet.ppm"	    , -8 ,  8,  8},
    { "bullet_blue.ppm"    ,  -8 ,  8,  8},
    { "base_down.ppm"	    , 1 , 35, 35},
    { "base_left.ppm"	    , 1 , 35, 35},
    { "base_up.ppm"	    , 1 , 35, 35},
    { "base_right.ppm"	    , 1 , 35, 35},
    { "fuelcell.ppm"	    , 1 , 35, 35},
    { "fuel2.ppm"	    , -16, 29, 29},
    { "allitems.ppm"	    , -30, 16, 16},
    { "cannon_down.ppm"    , 1 , 35, 35},
    { "cannon_left.ppm"    , 1 , 35, 35},
    { "cannon_up.ppm"	    , 1 , 35, 35},
    { "cannon_right.ppm"   , 1 , 35, 35},
    { "sparks.ppm"	    , -8, 2 , 2},
    { "paused.ppm"	    , -2, 35 , 35},
    { "wall_top.ppm"	    , 1, 35 , 35},
    { "wall_left.ppm"	    , 1, 35 , 35},
    { "wall_bottom.ppm"    , 1, 35 , 35},
    { "wall_right.ppm"	    , 1, 35 , 35},
    { "wall_ul.ppm"	    , 1, 35 , 35},
    { "wall_ur.ppm"	    , 1, 35 , 35},
    { "wall_dl.ppm"	    , 1, 35 , 35},
    { "wall_dr.ppm"	    , 1, 35 , 35},
    { "wall_fi.ppm"	    , 1, 35 , 35},
    { "wall_url.ppm"	    , 1, 35 , 35},
    { "wall_ull.ppm"	    , 1, 35 , 35},
    { "clouds.ppm"	    , 1, 256, BG_IMAGE_HEIGHT},
    { "logo.ppm"	    , 1, 256, LOGO_HEIGHT},
    { "refuel.ppm"	    , -4, 8, 8},
    { "wormhole.ppm"	    , 8, 35, 35},
    { "mine_team.ppm"	    , 1, 21, 15},
    { "mine_other.ppm"	    , 1, 21, 15},
    { "concentrator.ppm"    , 32, 35, 35},
    { "plus.ppm"	    , 1, 35, 35},
    { "minus.ppm"	    , 1, 35, 35},
    { "checkpoint.ppm"	    , -2, 35, 35},
    { "meter.ppm"	    , -2, 200, 11},
};


/*
    XXX
*/
int Block_bitmap_images(int type)
{
    return xp_pixmaps[type].picture.images;
}


/* Purpose: initialize the block bitmaps, currently i call it after
   item bitmaps has been created.
   i hacked the rotations member, it's really #images, but it can also
   be negative.
   if rotations > 0 then rotate it (resolution = #images)
   else it's a handdrawn animation (#images = -rotations) or similar images
   collected in same ppm for convenience for editing purposes (items).

   return 0 on success.
   return -1 on error.
*/
int Block_bitmaps_create(void)
{
    int		i, j, images;
    static int	block_bitmaps_loaded = 0;
    
    if (block_bitmaps_loaded) {
	return (block_bitmaps_loaded == 2) ? 0 : -1;
    }

    block_bitmaps_loaded = 1;

    for (i = 0 ; i < NUM_BITMAPS; i++) {
	IFWINDOWS( Progress("Loading image: %s", xp_pixmaps[i].filename); )

	images = (xp_pixmaps[i].rotations > 0 ) ? 
			    xp_pixmaps[i].rotations : 
			    -xp_pixmaps[i].rotations;

	xp_pixmaps[i].bitmaps = malloc(images * sizeof(xp_bitmap_t));
	if (!xp_pixmaps[i].bitmaps) {
	    error("Not enough memory.");
	    break;
	}
	for (j = 0; j < images; j++) {
	    xp_pixmaps[i].bitmaps[j].scale_height = -1;
	    xp_pixmaps[i].bitmaps[j].scale_width = -1;
	    xp_pixmaps[i].bitmaps[j].bitmap = None;
	    xp_pixmaps[i].bitmaps[j].mask = None;
	}

	if (Picture_init(&xp_pixmaps[i].picture,
			  xp_pixmaps[i].height, 
			  xp_pixmaps[i].width,
			  images) == -1) {
	    break;
	}
	if (Picture_load(&xp_pixmaps[i].picture,
			  xp_pixmaps[i].filename) == -1) {
	    break;
	}
    
	if (xp_pixmaps[i].rotations > 1) {
	    Picture_rotate(&xp_pixmaps[i].picture);
	}
	Picture_get_bounding_box(&xp_pixmaps[i].picture);
    }

    if (i == NUM_BITMAPS) {
	block_bitmaps_loaded = 2;
    }

    return (block_bitmaps_loaded == 2) ? 0 : -1;
}


/*
    Purpose: Draw a bitmap/pixmap, name should be changed(Block_bitmap_paint??)
    unix implementation probably need extra parameters?
    I rather only change name and parameters once, 
    as i call it all over the place in guimap.c and guiobjects.c.
    There is a scale check, if the bitmap is not correct size then it'll
    scale before drawing.
*/

void PaintBitmap(Drawable d, int type, int x, int y, int width, int height,
		 int number)
{
    if (!xp_pixmaps[type].bitmaps)
		return;
	if (xp_pixmaps[type].bitmaps[number].scale_height != height) {
	Block_bitmap_create(dpy, d, &xp_pixmaps[type], number, width, height);
	scaled_bitmaps++;
    }
    Block_bitmap_paint(d, type, x, y, width, height, number);
}

/*
    Purpose: Works almost like PaintBitmap,
    except that it paint a slice of a fuel image
    (size is height of the slice that should be draw)
*/
void PaintFuelSlice(Drawable d, int type,
		    int x, int y,
		    int width, int height,
		    int image, int size)
{
    if (xp_pixmaps[type].bitmaps[image].scale_height != height) {
	Block_bitmap_create(dpy, d, &xp_pixmaps[type], image, width, height);
	scaled_bitmaps++;
    }
    Block_bitmap_paint_fuel_slice(d, type, x, y, width, height, image, size);
}


/*
    Purpose: Paint a meter, size, is how much of the meter that is used, 
    first image is the empty meter, second is the filled meter
 */

void PaintMeter(Drawable d, int type, int x, int y, int width, int height, int size)
{
    if (xp_pixmaps[type].bitmaps[0].scale_height != height) {
	Block_bitmap_create(dpy, d, &xp_pixmaps[type], 0, width, height);
	scaled_bitmaps++;
    }
    if (xp_pixmaps[type].bitmaps[1].scale_height != height) {
	Block_bitmap_create(dpy, d, &xp_pixmaps[type], 1, width, height);
	scaled_bitmaps++;
    }
    Block_bitmap_paint_meter(d, type, x, y, width, height, size);
}


/*
   Purpose: to even out the scaling process,
   just the ships is forced created with this function,
   To reduce lag incidents due to massive scaling,
   in the beginning of a game
*/

int rotation_types[] = {BM_SHIP_SELF, BM_SHIP_FRIEND, BM_SHIP_ENEMY};

void Cache_ships(Drawable d)
{
    int i;
    static int type=0;
    static int number=0;
    return;
    if (scaled_bitmaps < 3) {
	for (i=0; i < 3 - scaled_bitmaps;i++) {
	    number++;
	    if (number == xp_pixmaps[rotation_types[type]].picture.images) {
		type++;
		number = 0;
		if (type == 3) {
		type = 0;
		}
	    }

	    if (xp_pixmaps[rotation_types[type]].bitmaps[number].scale_height
		!= WINSCALE(32))
	    {
    		Block_bitmap_create(dpy, d,
				    &xp_pixmaps[rotation_types[type]],
				    number, WINSCALE(32), WINSCALE(32));
	    }
	}	
	
    }
    scaled_bitmaps = 0;
}


/* Purpose: Take a device independent picture and create a 
    device/os dependent image. 
    This is only used in the scalefactor 1.0 special case.

    Actually this function could be killed, but it's very fast
    and it uses the intended original image. 
*/

static void Block_bitmap_picture_copy(xp_pixmap_t *xp_pixmap, int image)
{
    int		x, y;
    RGB_COLOR	color;

    for (y=0; y < xp_pixmap->height; y++)
	for (x=0; x < xp_pixmap->width; x++) {
	    color = Picture_get_pixel(&(xp_pixmap->picture), 
					    image, x, y);
	    Block_bitmap_set_pixel(xp_pixmap, image, x, y, color);
	}


    /* copy bounding box from original picture. */
    xp_pixmap->bitmaps[image].bbox = xp_pixmap->picture.bbox[image];
}


/*  Purpose: Take a device independent picture and create a
    scaled device/os dependent image. 
    This is for some of us the general case.
    The trick is for each pixel in the target image 
    to find the area it responds to in the original image, and then
    find an average of the colors in this area.
*/
    
static void Block_bitmap_picture_scale(xp_pixmap_t *xp_pixmap, int image)
{
    int		x, y;
    RGB_COLOR	color;
    double	x_scaled, y_scaled;
    double      dx_scaled, dy_scaled;
    double	orig_height, orig_width;
    int		height, width;
    
    orig_height = xp_pixmap->height; 
    orig_width = xp_pixmap->width;
    height = xp_pixmap->bitmaps[image].scale_height;
    width = xp_pixmap->bitmaps[image].scale_width;

    dx_scaled = orig_width  / width;
    dy_scaled = orig_height / height;
    y_scaled = 0;
    for (y = 0; y < height; y++) {

	x_scaled = 0;
	for (x=0; x < width; x++) {
	    color = Picture_get_pixel_area(&(xp_pixmap->picture), image,
					       x_scaled, y_scaled,
					       dx_scaled, dy_scaled);
	    Block_bitmap_set_pixel(xp_pixmap, image, x, y, color);
	    x_scaled += dx_scaled;
	}
	y_scaled += dy_scaled;
    }


    /* scale bounding box as well. */
    {
	bbox_t	*src = &xp_pixmap->picture.bbox[image];
	bbox_t	*dst = &xp_pixmap->bitmaps[image].bbox;

	dst->xmin = (int)((width * src->xmin) / orig_width);
	dst->ymin = (int)((height * src->ymin) / orig_height);
	dst->xmax = (int)(((width * src->xmax) + (orig_width - 1 )) / orig_width);
	dst->ymax = (int)(((height * src->ymax) + (orig_height - 1 )) / orig_height);
    }
}




/* XXX todo: make static */

/*
    Purpose: create a device/OS dependent bitmap.
    The windows version need to create and lock a device context.
    I got no clue what the unix version needs before and after drawing the
    picture to the pixmap. 
    (the windows version just need the Drawable as parameter, the unix version
    might need more)

  */

void Block_bitmap_create(Display* dpy, Drawable d,
			 xp_pixmap_t *xp_pixmap, 
			 int image, int width, int height)
{
    Block_bitmap_create_begin(d, xp_pixmap, image, width, height);

    xp_pixmap->bitmaps[image].scale_width = width;		
    xp_pixmap->bitmaps[image].scale_height = height;		

    if (height == xp_pixmap->height && width == xp_pixmap->width) {
	/* exactly same size as original */
	Block_bitmap_picture_copy(xp_pixmap, image);
    } else {
	Block_bitmap_picture_scale(xp_pixmap, image);
    }
    Block_bitmap_create_end(d);
}

#ifndef _WINDOWS
/*
 * Maybe move this part to a sperate file.
 */

#include "paintdata.h"

extern int		dispDepth;
extern unsigned long	(*RGB)(unsigned char r, unsigned char g, unsigned char b);

static GC		maskGC;

/*
    Purpose: to allocate and prepare a pixmap for drawing.
    this might be inlined in block_bitmap_create instead?
  */

void Block_bitmap_create_begin(Drawable d,
			       xp_pixmap_t * xp_pixmap,int image,
			       int width, int height)
{
    Drawable		pixmap;

    if (xp_pixmap->bitmaps[image].bitmap) {
	XFreePixmap(dpy, xp_pixmap->bitmaps[image].bitmap);
	xp_pixmap->bitmaps[image].bitmap = None;
    }
    if (xp_pixmap->bitmaps[image].mask) {
	XFreePixmap(dpy, xp_pixmap->bitmaps[image].mask);
	xp_pixmap->bitmaps[image].mask = None;
    }

    pixmap = XCreatePixmap(dpy, d,
			   width, height,
			   dispDepth);
    if (!pixmap) {
	error("Could not create pixmap");
	exit(1);
    }
    xp_pixmap->bitmaps[image].bitmap = pixmap;

    pixmap = XCreatePixmap(dpy, d,
			   width, height,
			   1);
    if (!pixmap) {
	error("Could not create mask pixmap");
	exit(1);
    }
    xp_pixmap->bitmaps[image].mask = pixmap;

    if (!maskGC) {
	XGCValues	xgc;
	unsigned long	values;

	xgc.line_width = 0;
	xgc.line_style = LineSolid;
	xgc.cap_style = CapButt;
	xgc.join_style = JoinMiter;
	xgc.graphics_exposures = False;
	values = 
	    GCLineWidth|GCLineStyle|GCCapStyle|GCJoinStyle|GCGraphicsExposures;
	maskGC = XCreateGC(dpy, pixmap, values, &xgc);
    }
}

/*
    Purpose: to deallocate resources needed during creation of a bitmap.
*/
void Block_bitmap_create_end(Drawable d)
{
}

/*
    Purpose: set 1 pixel in the device/OS dependent bitmap.
*/

void Block_bitmap_set_pixel(xp_pixmap_t * xp_pixmap,
			    int image, int x, int y,
			    RGB_COLOR color)
{
    unsigned long	pixel;
    unsigned char	r, g, b;

    r = RED_VALUE(color);
    g = GREEN_VALUE(color);
    b = BLUE_VALUE(color);
    pixel = (RGB)(r, g, b);
    SET_FG(pixel);
    XDrawPoint(dpy, xp_pixmap->bitmaps[image].bitmap, gc,
	       x, y);

    pixel = (color) ? 1 : 0;
    XSetForeground(dpy, maskGC, pixel);
    XDrawPoint(dpy, xp_pixmap->bitmaps[image].mask, maskGC,
	       x, y);
}

/*
    New fuelCell animation, Not pretty but it works :)
    Will require a new unix drawbitmap operation, as it works
    by drawing less than the height in the image specify.
*/
void Block_bitmap_paint_fuel_slice(Drawable d, int type, 
				   int x, int y, 
				   int width, int height, 
				   int image, int size)
{
    xp_pixmap_t	*pix = &xp_pixmaps[type];
    xp_bitmap_t	*bit = &pix->bitmaps[image];
    bbox_t	*box = &bit->bbox;

    /*
    assert(width >= box->xmax);
    assert(height >= box->ymax);
    */
    XCopyArea(dpy, xp_pixmaps[type].bitmaps[image].bitmap,
	      d, gc,
	      0 + box->xmin, 0 + box->ymin,
	      box->xmax + 1 - box->xmin,
	      size * (box->ymax + 1 - box->ymin) / bit->scale_width,
	      x + box->xmin, y + box->ymin);
}

/*
    Purpose: Paint a meter in a device/OS dependent fashion.
     I'm not certain this is correct implemented, it is _not_ tested.
     Note, there is no bounding box, for clearity. (image has to be fullsize)
*/

void Block_bitmap_paint_meter(Drawable d, int type, 
			      int x, int y, 
			      int width, int height, 
			      int size) 
{

    /*First draw the part of the meter that should be filled */
    XCopyArea(dpy, xp_pixmaps[type].bitmaps[1].bitmap, /* 1 = filled image */
	      d, gc,
	      0, 0,
	      size, xp_pixmaps[type].height,
	      x, y); 
    /*Then draw the part of the meter that should be empty */

    XCopyArea(dpy, xp_pixmaps[type].bitmaps[0].bitmap,	/* 0 = empty image */
	      d, gc,
	      size, 0,
	      xp_pixmaps[type].width - size, xp_pixmaps[type].height,
	      x + size, y); 
}

/*
    Purpose: Paint a bitmap in a device/OS dependent fashion.
*/

void Block_bitmap_paint(Drawable d, int type, int x, int y, int width,
			int height, int number)
{
    xp_pixmap_t		*pix = &xp_pixmaps[type];
    xp_bitmap_t		*bit = &pix->bitmaps[number];
    bbox_t		*box = &bit->bbox;

    XSetClipOrigin(dpy, gc, x, y);
    XSetClipMask(dpy, gc, bit->mask);
    XCopyArea(dpy, bit->bitmap,
	      d, gc,
	      0 + box->xmin, 0 + box->ymin,
	      box->xmax + 1 - box->xmin, box->ymax + 1 - box->ymin,
	      x + box->xmin, y + box->ymin);
    XSetClipMask(dpy, gc, None);
}


#endif

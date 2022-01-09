/* $Id: paintmap.c,v 5.3 2001/10/11 20:27:35 bertg Exp $
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
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>

#ifndef _WINDOWS
# include <unistd.h>
# include <X11/Xlib.h>
# include <X11/Xos.h>
#endif

#ifdef _WINDOWS
# include "NT/winX.h"
#endif

#include "version.h"
#include "xpconfig.h"
#include "const.h"
#include "error.h"
#include "bit.h"
#include "types.h"
#include "keys.h"
#include "rules.h"
#include "setup.h"
#include "texture.h"
#include "paint.h"
#include "paintdata.h"
#include "paintmacros.h"
#include "record.h"
#include "xinit.h"
#include "protoclient.h"
#include "guimap.h"


char paintmap_version[] = VERSION;


int	wallColor;		/* Color index for wall drawing */
int	decorColor;		/* Color index for decoration drawing */
char	*wallTextureFile;	/* Filename of wall texture */
char	*decorTextureFile;	/* Filename of decor texture */

extern setup_t		*Setup;



void Paint_vcannon(void)
{
    int	i;
    if (num_vcannon > 0) {
	for (i = 0; i < num_vcannon; i++) {
    	    Gui_paint_cannon(vcannon_ptr[i].x, vcannon_ptr[i].y, vcannon_ptr[i].type);
	}
	RELEASE(vcannon_ptr, num_vcannon, max_vcannon);
    }
}

void Paint_vfuel(void)
{
    int	i;
    if (num_vfuel > 0) {
	for (i = 0; i < num_vfuel; i++) {
	    Gui_paint_fuel(vfuel_ptr[i].x, vfuel_ptr[i].y, vfuel_ptr[i].fuel);
	}
	RELEASE(vfuel_ptr, num_vfuel, max_vfuel);
    }
}

void Paint_vbase(void)
{
    int	i;
    if (num_vbase > 0) {
	for (i = 0; i < num_vbase; i++) {
	    Gui_paint_base(vbase_ptr[i].x, vbase_ptr[i].y, vbase_ptr[i].xi, vbase_ptr[i].yi, vbase_ptr[i].type);
	}
	RELEASE(vbase_ptr, num_vbase, max_vbase);
    }
}

void Paint_vdecor(void)
{
    int	i;
    bool last, more_y;
    
    if (num_vdecor > 0) {
	for (i = 0; i < num_vdecor; i++) {
	    last = (i + 1 == num_vdecor);
	    more_y = (vdecor_ptr[i].yi != vdecor_ptr[i + 1].yi);
	    Gui_paint_decor(vdecor_ptr[i].x, vdecor_ptr[i].y, 
			    vdecor_ptr[i].xi, vdecor_ptr[i].yi,
			    vdecor_ptr[i].type, last, more_y);
	}
	RELEASE(vdecor_ptr, num_vdecor, max_vdecor);
    }
}

/*
 * Draw the current player view of the map in the large viewing area.
 * This includes drawing walls, fuelstations, targets and cannons.
 *
 * Walls can be drawn in three ways:
 *
 *  1) Like the original grid.
 *
 *  2) In outline mode (thanks to Tero Kivinen).
 *     Replace this with an explanation about how outline mode works.
 *
 *  3) In filled mode (thanks to Steven Singer).
 *     How does filled mode work?
 *     It's cunning.  It scans from left to right across an area 1 block deep.
 *     Say the map is :
 *     
 *     space       wall    space  w  s w
 *             /        |        / \  | |
 *            /         |        |  \ | |     <- Scanning this line
 *           /          |        |   \| |
 *     
 *     It starts from the left and determines if it's in wall or outside wall.
 *     If it is it sets tl and bl (top left and bottom left) to the left hand
 *     side of the window.
 *     It then examines each block in turn and decides if the block starts
 *     or ends a wall.  If it starts it sets tl and bl, it it ends it sets
 *     tr and br (top right and bottom right).
 *     When it finds a wall end it draws a filled polygon (bl, y) -
 *     (tl, y + BLOCK_SZ) - (tr, y + BLOCK_SZ) - (br, y) and reset the pointers
 *     to indicate outside of wall.
 *     Hence the line indicated above would be drawn with 3 filled polygons.
 *
 */
void Paint_world(void)
{
    int			xi, yi, xb, yb, xe, ye, fuel;
    int			rxb, ryb;
    int			x, y;
    int			type;
    int			dot;
    int			fill_top_left = -1,
			fill_top_right = -1,
			fill_bottom_left = -1,
			fill_bottom_right = -1;
    static int		wormDrawCount;
    unsigned char	*mapptr, *mapbase;
    static int		wallTileReady = 0;
    static Pixmap	wallTile = None;
    int			wallTileDoit = false;
    XPoint		points[5];

    if (BIT(instruments, SHOW_TEXTURED_WALLS)) {
	if (!wallTileReady) {
	    wallTile = Texture_wall();
	    wallTileReady = (wallTile == None) ? -1 : 1;
	}
	if (wallTileReady == 1) {
	    wallTileDoit = true;
	    XSetTile(dpy, gc, wallTile);
	    XSetTSOrigin(dpy, gc, -WINSCALE(realWorld.x), WINSCALE(realWorld.y));
	}
    }

    wormDrawCount = (wormDrawCount + 1) & 7;

    xb = ((world.x < 0) ? (world.x - (BLOCK_SZ - 1)) : world.x) / BLOCK_SZ;
    yb = ((world.y < 0) ? (world.y - (BLOCK_SZ - 1)) : world.y) / BLOCK_SZ;
    xe = (world.x + ext_view_width) / BLOCK_SZ;
    ye = (world.y + ext_view_height) / BLOCK_SZ;
    if (!BIT(Setup->mode, WRAP_PLAY)) {
	if (xb < 0)
	    xb = 0;
	if (yb < 0)
	    yb = 0;
	if (xe >= Setup->x)
	    xe = Setup->x - 1;
	if (ye >= Setup->y)
	    ye = Setup->y - 1;
	if (world.x <= 0) {
	    Gui_paint_border(0, 0, 0, Setup->height);
	}
	if (world.x + ext_view_width >= Setup->width) {
	    Gui_paint_border(Setup->width, 0, Setup->width, Setup->height);
	}
	if (world.y <= 0) {
	    Gui_paint_border(0, 0, Setup->width, 0);
	}
	if (world.y + ext_view_height >= Setup->height) {
	    Gui_paint_border(0, Setup->height, Setup->width, Setup->height);
	}
    }

    y = yb * BLOCK_SZ;
    yi = mod(yb, Setup->y);
    mapbase = Setup->map_data + yi;
    if (ext_view_width > MAX_VIEW_SIZE || ext_view_height > MAX_VIEW_SIZE) {
	Gui_paint_visible_border(world.x + ext_view_width/2 - MAX_VIEW_SIZE/2, 
				 world.y + ext_view_height/2 - MAX_VIEW_SIZE/2,
				 world.x + ext_view_width/2 + MAX_VIEW_SIZE/2, 
				 world.y + ext_view_height/2 + MAX_VIEW_SIZE/2);
    }

    for (ryb = yb; ryb <= ye; ryb++, yi++, y += BLOCK_SZ, mapbase++) {

	if (yi == Setup->y) {
	    if (!BIT(Setup->mode, WRAP_PLAY))
		break;
	    yi = 0;
	    mapbase = Setup->map_data;
	}

	x = xb * BLOCK_SZ;
	xi = mod(xb, Setup->x);
	mapptr = mapbase + xi * Setup->y;

	for (rxb = xb; rxb <= xe; rxb++, xi++, x += BLOCK_SZ,
	     mapptr += Setup->y) {

	    if (xi == Setup->x) {
		if (!BIT(Setup->mode, WRAP_PLAY))
		    break;
		xi = 0;
		mapptr = mapbase;
	    }

	    type = *mapptr;

	    if (!(type & BLUE_BIT)) {

		switch (type) {

		case SETUP_FILLED_NO_DRAW:
		    if (BIT(instruments, SHOW_FILLED_WORLD|SHOW_TEXTURED_WALLS)
			&& fill_top_left == -1) {
			fill_top_left = fill_bottom_left = x;
		    }
		    break;
		case SETUP_CHECK:
		    Gui_paint_setup_check(x, y, xi, yi);
		    break;

		case SETUP_ACWISE_GRAV:
		    Gui_paint_setup_acwise_grav(x, y);
		    break;

		case SETUP_CWISE_GRAV:
		    Gui_paint_setup_cwise_grav(x, y);
		    break;

		case SETUP_POS_GRAV:
		    Gui_paint_setup_pos_grav(x, y);
		    break;

		case SETUP_NEG_GRAV:
		    Gui_paint_setup_neg_grav(x, y);
		    break;

		case SETUP_UP_GRAV:
		    Gui_paint_setup_up_grav(x, y);
		    break;

	        case SETUP_DOWN_GRAV:
		    Gui_paint_setup_down_grav(x, y);
		    break;

		case SETUP_RIGHT_GRAV:
		    Gui_paint_setup_right_grav(x, y);
		    break;

		case SETUP_LEFT_GRAV:
		    Gui_paint_setup_left_grav(x, y);
		    break;

		case SETUP_WORM_IN:
		case SETUP_WORM_NORMAL:
		    Gui_paint_setup_worm(x, y, wormDrawCount);
		    break;

		case SETUP_ITEM_CONCENTRATOR:
		    Gui_paint_setup_item_concentrator(x, y);
		    break;

		case SETUP_ASTEROID_CONCENTRATOR:
		    Gui_paint_setup_asteroid_concentrator(x, y);
		    break;

		case SETUP_CANNON_UP:
		case SETUP_CANNON_DOWN:
		case SETUP_CANNON_RIGHT:
		case SETUP_CANNON_LEFT:
		    if (Cannon_dead_time_by_pos(xi, yi, &dot) <= 0) {
			Handle_vcannon(x, y, type);
			break;
		    }
		    if (dot == 0) {
			break;
		    }
		    /*FALLTHROUGH*/

		case SETUP_SPACE_DOT:
		case SETUP_DECOR_DOT_FILLED:
		case SETUP_DECOR_DOT_RU:
		case SETUP_DECOR_DOT_RD:
		case SETUP_DECOR_DOT_LU:
		case SETUP_DECOR_DOT_LD:
		    Gui_paint_decor_dot(x, y, map_point_size);
		    break;

		case SETUP_BASE_UP:
		case SETUP_BASE_RIGHT:
		case SETUP_BASE_DOWN:
		case SETUP_BASE_LEFT:
		    Handle_vbase(x, y, xi, yi, type);
		    break;

		case SETUP_DECOR_FILLED:
		case SETUP_DECOR_RD:
		case SETUP_DECOR_RU:
		case SETUP_DECOR_LD:
		case SETUP_DECOR_LU:
		    if (BIT(instruments, SHOW_DECOR))
			Handle_vdecor(x, y, xi, yi, type);
		    break;
		    
		case SETUP_TARGET+0:
		case SETUP_TARGET+1:
		case SETUP_TARGET+2:
		case SETUP_TARGET+3:
		case SETUP_TARGET+4:
		case SETUP_TARGET+5:
		case SETUP_TARGET+6:
		case SETUP_TARGET+7:
		case SETUP_TARGET+8:
		case SETUP_TARGET+9: 
		    {
			int damage, target, own;
    			
			if (Target_alive(xi, yi, &damage) != 0)
			    break;
			
			target = type - SETUP_TARGET;
			own = (self && (self->team == target));

			Gui_paint_setup_target(x, y, target, damage, own);

		    }
		    break;

		case SETUP_TREASURE+0:
		case SETUP_TREASURE+1:
		case SETUP_TREASURE+2:
		case SETUP_TREASURE+3:
		case SETUP_TREASURE+4:
		case SETUP_TREASURE+5:
		case SETUP_TREASURE+6:
		case SETUP_TREASURE+7:
		case SETUP_TREASURE+8:
		case SETUP_TREASURE+9:
		    {
			int	treasure;
			bool	own;
			
			treasure = type - SETUP_TREASURE;
			own = (self && self->team == treasure);
			
			Gui_paint_setup_treasure(x, y, treasure, own);
		    }
		    break;

		default:
		    break;
		}
	    }
	    else {
		if (!BIT(instruments, SHOW_FILLED_WORLD|SHOW_TEXTURED_WALLS)) {
		    Gui_paint_walls(x, y, type, xi, yi);

		    if ((type & BLUE_FUEL) == BLUE_FUEL) {
			fuel = Fuel_by_pos(xi, yi);
			Handle_vfuel(x, y, fuel);
		    }
		}
		else {
		    if ((type & BLUE_FUEL) == BLUE_FUEL) {
			fuel = Fuel_by_pos(xi, yi);
			Handle_vfuel(x, y, fuel);
		    }
		    else if (type & BLUE_OPEN) {
			if (type & BLUE_BELOW) {
			    fill_top_left = x + BLOCK_SZ;
			    fill_bottom_left = x;
			    fill_top_right = fill_bottom_right = -1;
			} else {
			    fill_top_right = x + BLOCK_SZ;
			    fill_bottom_right = x;
			}
		    }
		    else if (type & BLUE_CLOSED) {
			if (!(type & BLUE_BELOW)) {
			    fill_top_left = x;
			    fill_bottom_left = x + BLOCK_SZ;
			    fill_top_right = fill_bottom_right = -1;
			} else {
			    fill_top_right = x;
			    fill_bottom_right = x + BLOCK_SZ;
			}
		    }
		    if (type & BLUE_RIGHT) {
			fill_top_right = fill_bottom_right = x + BLOCK_SZ;
		    }
		    if (fill_top_left == -1) {
			fill_top_left = fill_bottom_left = x;
		    }
		    if (fill_top_right != -1) {
			points[0].x = WINSCALE(X(fill_bottom_left));
			points[0].y = WINSCALE(Y(y));
			points[1].x = WINSCALE(X(fill_top_left));
			points[1].y = WINSCALE(Y(y + BLOCK_SZ));
			points[2].x = WINSCALE(X(fill_top_right));
			points[2].y = WINSCALE(Y(y + BLOCK_SZ));
			points[3].x = WINSCALE(X(fill_bottom_right));
			points[3].y = WINSCALE(Y(y));
			points[4] = points[0];
			if (wallTileDoit) {
			    XSetFillStyle(dpy, gc, FillTiled);
			} else {
			    SET_FG(colors[wallColor].pixel);
			}
			rd.fillPolygon(dpy, p_draw, gc,
				       points, 5,
				       Convex, CoordModeOrigin);
			if (wallTileDoit) {
			    XSetFillStyle(dpy, gc, FillSolid);
			}
			if (useErase){
			    int left_x = MIN(fill_bottom_left, fill_top_left);
			    int right_x = MAX(fill_bottom_right, fill_top_right);
			    Erase_rectangle(WINSCALE(X(left_x)) - 1,
					    WINSCALE(Y(y + BLOCK_SZ)) - 1,
					    WINSCALE(right_x - left_x) + 4,
					    WINSCALE(BLOCK_SZ) +3);
			}
			fill_top_left =
			fill_top_right =
			fill_bottom_left =
			fill_bottom_right = -1;
		    }
		}
	    }
	}

	if (fill_top_left != -1) {
	    points[0].x = WINSCALE(X(fill_bottom_left));
	    points[0].y = WINSCALE(Y(y));
	    points[1].x = WINSCALE(X(fill_top_left));
	    points[1].y = WINSCALE(Y(y + BLOCK_SZ));
	    points[2].x = WINSCALE(X(x));
	    points[2].y = WINSCALE(Y(y + BLOCK_SZ));
	    points[3].x = WINSCALE(X(x));
	    points[3].y = WINSCALE(Y(y));
	    points[4] = points[0];
	    if (wallTileDoit) {
		XSetFillStyle(dpy, gc, FillTiled);
	    } else {
		SET_FG(colors[wallColor].pixel);
	    }
	    rd.fillPolygon(dpy, p_draw, gc,
			   points, 5,
			   Convex, CoordModeOrigin);
	    if (wallTileDoit) {
		XSetFillStyle(dpy, gc, FillSolid);
	    }
	    if (useErase){
		int left_x = MIN(fill_bottom_left, fill_top_left);
		Erase_rectangle(WINSCALE(X(left_x)) - 1,
				WINSCALE(Y(y + BLOCK_SZ)) - 1,
				WINSCALE(x - left_x) + 4,
				WINSCALE(BLOCK_SZ) + 3);
	    }
	    fill_top_left =
	    fill_top_right =
	    fill_bottom_left =
	    fill_bottom_right = -1;
	}
    }
}


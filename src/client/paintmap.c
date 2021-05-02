/* $Id: paintmap.c,v 4.3 1998/04/17 09:00:28 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
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

#ifdef	_WINDOWS
#include "NT/winX.h"
#else
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xos.h>
#endif

#include "version.h"
#include "config.h"
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
#include "record.h"
#include "xinit.h"
#include "protoclient.h"

char paintmap_version[] = VERSION;

#define X(co)  ((int) ((co) - world.x))
#define Y(co)  ((int) (world.y + view_height - (co)))

#define FIND_NAME_WIDTH(other)						\
    if ((other)->name_width == 0) {					\
	(other)->name_len = strlen((other)->name);			\
	(other)->name_width = 2 + XTextWidth(gameFont, (other)->name,	\
					 (other)->name_len);		\
    }

int	wallColor;		/* Color index for wall drawing */
int	decorColor;		/* Color index for decoration drawing */
char	*wallTextureFile;	/* Filename of wall texture */
char	*decorTextureFile;	/* Filename of decor texture */

extern setup_t		*Setup;

void Paint_vcannon(void)
{
    int			i, x, y, type;
    XPoint		points[5];

    if (num_vcannon > 0) {
	SET_FG(colors[WHITE].pixel);
	for (i = 0; i < num_vcannon; i++) {
	    type = vcannon_ptr[i].type;
	    x = vcannon_ptr[i].x;
	    y = vcannon_ptr[i].y;
	    switch (type) {
	    case SETUP_CANNON_UP:
		points[0].x = WINSCALE(X(x));
		points[0].y = WINSCALE(Y(y));
		points[1].x = WINSCALE(X(x+BLOCK_SZ));
		points[1].y = WINSCALE(Y(y));
		points[2].x = WINSCALE(X(x+BLOCK_SZ/2));
		points[2].y = WINSCALE(Y(y+BLOCK_SZ/3));
		break;
	    case SETUP_CANNON_DOWN:
		points[0].x = WINSCALE(X(x));
		points[0].y = WINSCALE(Y(y+BLOCK_SZ));
		points[1].x = WINSCALE(X(x+BLOCK_SZ));
		points[1].y = WINSCALE(Y(y+BLOCK_SZ));
		points[2].x = WINSCALE(X(x+BLOCK_SZ/2));
		points[2].y = WINSCALE(Y(y+2*BLOCK_SZ/3));
		break;
	    case SETUP_CANNON_RIGHT:
		points[0].x = WINSCALE(X(x));
		points[0].y = WINSCALE(Y(y));
		points[1].x = WINSCALE(X(x));
		points[1].y = WINSCALE(Y(y+BLOCK_SZ));
		points[2].x = WINSCALE(X(x+BLOCK_SZ/3));
		points[2].y = WINSCALE(Y(y+BLOCK_SZ/2));
		break;
	    case SETUP_CANNON_LEFT:
		points[0].x = WINSCALE(X(x+BLOCK_SZ));
		points[0].y = WINSCALE(Y(y));
		points[1].x = WINSCALE(X(x+BLOCK_SZ));
		points[1].y = WINSCALE(Y(y+BLOCK_SZ));
		points[2].x = WINSCALE(X(x+2*BLOCK_SZ/3));
		points[2].y = WINSCALE(Y(y+BLOCK_SZ/2));
		break;
	    default:
		errno = 0;
		error("Unknown cannon type %d", type);
		continue;
	    }
	    points[3] = points[0];
	    rd.drawLines(dpy, p_draw, gc, points, 4, 0);
	    Erase_points(0, points, 4);
	}
	RELEASE(vcannon_ptr, num_vcannon, max_vcannon);
    }
}

void Paint_vfuel(void)
{
#define FUEL_BORDER 2

    int			i, x, y, size, text_width;
    long		fuel;
    char		s[2];

    if (num_vfuel > 0) {
	SET_FG(colors[RED].pixel);
	for (i = 0; i < num_vfuel; i++) {
	    x = vfuel_ptr[i].x;
	    y = vfuel_ptr[i].y;
	    fuel = vfuel_ptr[i].fuel;
	    size = (BLOCK_SZ - 2*FUEL_BORDER) * fuel / MAX_STATION_FUEL;
#if ERASE
	    /* speedup for slow old cheap graphics cards like cg3. */
	    rd.drawLine(dpy, p_draw, gc,
		      WINSCALE(X(x + FUEL_BORDER)),
			  WINSCALE(Y(y + FUEL_BORDER + size)),
		      WINSCALE(X(x + FUEL_BORDER + (BLOCK_SZ - 2*FUEL_BORDER))),
		      WINSCALE(Y(y + FUEL_BORDER + size)));
#else
	    rd.fillRectangle(dpy, p_draw, gc,
			  WINSCALE(X(x + FUEL_BORDER)),
			  WINSCALE(Y(y + FUEL_BORDER + size)),
			  WINSCALE(BLOCK_SZ - 2*FUEL_BORDER + 1),
			  WINSCALE(size + 1));
#endif
	    Erase_rectangle(WINSCALE(X(x)) - 1,
			    WINSCALE(Y(y + BLOCK_SZ)) - 1,
			    WINSCALE(BLOCK_SZ) + 2,
			    WINSCALE(BLOCK_SZ) + 2);
	}
	/* Draw F in fuel cells */
	s[0] = 'F'; s[1] = '\0';
	XSetFunction(dpy, gc, GXxor);
	SET_FG(colors[BLACK].pixel ^ colors[RED].pixel);
	text_width = XTextWidth(gameFont, s, 1);
	for (i = 0; i < num_vfuel; i++) {
	    x = vfuel_ptr[i].x;
	    y = vfuel_ptr[i].y;
	    rd.drawString(dpy, p_draw, gc,
			WINSCALE(X(x + BLOCK_SZ/2 - text_width/2)),
			WINSCALE(Y(y + BLOCK_SZ/2 - gameFont->ascent/2)),
			s, 1);
	}
	XSetFunction(dpy, gc, GXcopy);
	RELEASE(vfuel_ptr, num_vfuel, max_vfuel);
    }
}

void Paint_vbase(void)
{
    const int	BORDER = 4;					/* in pixels */
    int		i, id, x, y, xi, yi, team, type, size;
    other_t	*other;
    char	s[3];

    if (num_vbase > 0) {
	SET_FG(colors[WHITE].pixel);
	for (i = 0; i < num_vbase; i++) {
	    x = vbase_ptr[i].x;
	    y = vbase_ptr[i].y;
	    xi = vbase_ptr[i].xi;
	    yi = vbase_ptr[i].yi;
	    type = vbase_ptr[i].type;
	    switch (type) {
	    case SETUP_BASE_UP:
		Segment_add(WHITE,
			    X(x), Y(y-1),
			    X(x+BLOCK_SZ), Y(y-1));
		y -= BORDER + gameFont->ascent;
		break;
	    case SETUP_BASE_DOWN:
		Segment_add(WHITE,
			    X(x), Y(y+BLOCK_SZ+1),
			    X(x+BLOCK_SZ), Y(y+BLOCK_SZ+1));
		y += BORDER + BLOCK_SZ;
		break;
	    case SETUP_BASE_LEFT:
		Segment_add(WHITE,
			    X(x+BLOCK_SZ+1), Y(y+BLOCK_SZ),
			    X(x+BLOCK_SZ+1), Y(y));
		x += BLOCK_SZ + BORDER;
		y += BLOCK_SZ/2 - gameFont->ascent/2;
		break;
	    case SETUP_BASE_RIGHT:
		Segment_add(WHITE,
			    X(x-1), Y(y+BLOCK_SZ),
			    X(x-1), Y(y));
		y += BLOCK_SZ/2 - gameFont->ascent/2;
		x -= BORDER;
		break;
	    default:
		errno = 0;
		error("Bad base dir.");
		continue;
	    }
	    if (Base_info_by_pos(xi, yi, &id, &team) == -1) {
		continue;
	    }
/* only draw base teams if ship naming is on, SKS 25/05/94
*/
	    if (BIT(Setup->mode, TEAM_PLAY) &&
		BIT(instruments, SHOW_SHIP_NAME)) {
		s[0] = '0' + team;
		s[1] = ' ';
		s[2] = '\0';
		size = XTextWidth(gameFont, s, 2);
		if (type == SETUP_BASE_RIGHT) {
		    x -= size;
		}
		rd.drawString(dpy, p_draw, gc,
			    WINSCALE(X(x)), WINSCALE(Y(y)),
			    s, 2);
		Erase_rectangle(WINSCALE(X(x)) - 1,
				 WINSCALE(Y(y)) - gameFont->ascent - 1,
				size + 2,
				gameFont->ascent + gameFont->descent + 2);
		if (type != SETUP_BASE_RIGHT) {
		    x += size;
		}
	    }
/* only draw base names if ship naming is on, SKS 25/05/94
*/
	    if ((other = Other_by_id(id)) != NULL &&
		BIT(instruments, SHOW_SHIP_NAME)) {
		FIND_NAME_WIDTH(other);
		if (type == SETUP_BASE_RIGHT) {
		    x -= other->name_width;
		}
		rd.drawString(dpy, p_draw, gc,
			    WINSCALE(X(x)), WINSCALE(Y(y)),
			    other->name, other->name_len);
		Erase_rectangle(WINSCALE(X(x)) - 1,
				WINSCALE(Y(y)) - gameFont->ascent - 1,
				other->name_width + 2,
				gameFont->ascent + gameFont->descent + 2 );
	    }
	}
	RELEASE(vbase_ptr, num_vbase, max_vbase);
    }
}

void Paint_vdecor(void)
{
    XPoint		points[5];

    if (num_vdecor > 0) {
	int			i, x, y, xi, yi, type, mask;
	int			fill_top_left = -1,
				fill_top_right = -1,
				fill_bottom_left = -1,
				fill_bottom_right = -1;
	static int		decorTileReady = 0;
	static Pixmap		decorTile = None;
	int			decorTileDoit = false;
	unsigned char		decor[256];

	SET_FG(colors[decorColor].pixel);

	memset(decor, 0, sizeof decor);
	decor[SETUP_DECOR_FILLED] = DECOR_UP | DECOR_LEFT | DECOR_DOWN | DECOR_RIGHT;
	decor[SETUP_DECOR_RU] = DECOR_UP | DECOR_RIGHT | DECOR_CLOSED;
	decor[SETUP_DECOR_RD] = DECOR_DOWN | DECOR_RIGHT | DECOR_OPEN | DECOR_BELOW;
	decor[SETUP_DECOR_LU] = DECOR_UP | DECOR_LEFT | DECOR_OPEN;
	decor[SETUP_DECOR_LD] = DECOR_LEFT | DECOR_DOWN | DECOR_CLOSED | DECOR_BELOW;

	if (BIT(instruments, SHOW_TEXTURED_DECOR)) {
	    if (!decorTileReady) {
		decorTile = Texture_decor();
		decorTileReady = (decorTile == None) ? -1 : 1;
	    }
	    if (decorTileReady == 1) {
		decorTileDoit = true;
		XSetTile(dpy, gc, decorTile);
		XSetTSOrigin(dpy, gc, -realWorld.x, realWorld.y);
		XSetFillStyle(dpy, gc, FillTiled);
	    }
	}

	for (i = 0; i < num_vdecor; i++) {

	    x = vdecor_ptr[i].x;
	    y = vdecor_ptr[i].y;
	    xi = vdecor_ptr[i].xi;
	    yi = vdecor_ptr[i].yi;
	    type = vdecor_ptr[i].type;
	    mask = decor[type];

	    if (!BIT(instruments, SHOW_FILLED_DECOR|SHOW_TEXTURED_DECOR)) {
		if (mask & DECOR_LEFT) {
		    if ((xi == 0)
			? (!BIT(Setup->mode, WRAP_PLAY) ||
			    !(decor[Setup->map_data[(Setup->x - 1) * Setup->y + yi]]
				& DECOR_RIGHT))
			: !(decor[Setup->map_data[(xi - 1) * Setup->y + yi]]
			    & DECOR_RIGHT)) {
			Segment_add(decorColor,
				    X(x),
				    Y(y),
				    X(x),
				    Y(y+BLOCK_SZ));
		    }
		}
		if (mask & DECOR_DOWN) {
		    if ((yi == 0)
			? (!BIT(Setup->mode, WRAP_PLAY) ||
			    !(decor[Setup->map_data[xi * Setup->y + Setup->y - 1]]
				& DECOR_UP))
			: !(decor[Setup->map_data[xi * Setup->y + (yi - 1)]]
			    & DECOR_UP)) {
			Segment_add(decorColor,
				    X(x),
				    Y(y),
				    X(x+BLOCK_SZ),
				    Y(y));
		    }
		}
		if (mask & DECOR_RIGHT) {
		    if (!BIT(instruments, SHOW_OUTLINE_DECOR)
			|| ((xi == Setup->x - 1)
			    ? (!BIT(Setup->mode, WRAP_PLAY)
			       || !(decor[Setup->map_data[yi]]
				    & DECOR_LEFT))
			    : !(decor[Setup->map_data[(xi + 1) * Setup->y + yi]]
				& DECOR_LEFT))) {
			Segment_add(decorColor,
				    X(x+BLOCK_SZ),
				    Y(y),
				    X(x+BLOCK_SZ),
				    Y(y+BLOCK_SZ));
		    }
		}
		if (mask & DECOR_UP) {
		    if (!BIT(instruments, SHOW_OUTLINE_DECOR)
			|| ((yi == Setup->y - 1)
			    ? (!BIT(Setup->mode, WRAP_PLAY)
			       || !(decor[Setup->map_data[xi * Setup->y]]
				    & DECOR_DOWN))
			    : !(decor[Setup->map_data[xi * Setup->y + (yi + 1)]]
				& DECOR_DOWN))) {
			Segment_add(decorColor,
				    X(x),
				    Y(y+BLOCK_SZ),
				    X(x+BLOCK_SZ),
				    Y(y+BLOCK_SZ));
		    }
		}
		if (mask & DECOR_OPEN) {
		    Segment_add(decorColor,
				X(x),
				Y(y),
				X(x+BLOCK_SZ),
				Y(y+BLOCK_SZ));
		}
		else if (mask & DECOR_CLOSED) {
		    Segment_add(decorColor,
				X(x),
				Y(y+BLOCK_SZ),
				X(x+BLOCK_SZ),
				Y(y));
		}
	    }
	    else {
		if (mask & DECOR_OPEN) {
		    if (mask & DECOR_BELOW) {
			fill_top_left = x + BLOCK_SZ;
			fill_bottom_left = x;
			fill_top_right = fill_bottom_right = -1;
		    } else {
			fill_top_right = x + BLOCK_SZ;
			fill_bottom_right = x;
		    }
		}
		else if (mask & DECOR_CLOSED) {
		    if (!(mask & DECOR_BELOW)) {
			fill_top_left = x;
			fill_bottom_left = x + BLOCK_SZ;
			fill_top_right = fill_bottom_right = -1;
		    } else {
			fill_top_right = x;
			fill_bottom_right = x + BLOCK_SZ;
		    }
		}
		if (mask & DECOR_RIGHT) {
		    fill_top_right = fill_bottom_right = x + BLOCK_SZ;
		}
		if (fill_top_left == -1) {
		    fill_top_left = fill_bottom_left = x;
		}
		if (fill_top_right == -1
		    && (i + 1 == num_vdecor || yi != vdecor_ptr[i + 1].yi)) {
		    fill_top_right = x + BLOCK_SZ;
		    fill_bottom_right = x + BLOCK_SZ;
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
		    rd.fillPolygon(dpy, p_draw, gc,
				   points, 5,
				   Convex, CoordModeOrigin);
#if ERASE
		    {
			int left_x = MIN(fill_bottom_left, fill_top_left);
			int right_x = MAX(fill_bottom_right, fill_top_right);
			Erase_rectangle(WINSCALE(X(left_x)) - 1,
					WINSCALE(Y(y + BLOCK_SZ)) - 1,
					WINSCALE(right_x - left_x) + 4,
					WINSCALE(BLOCK_SZ) + 3);
		    }
#endif
		    fill_top_left =
		    fill_top_right =
		    fill_bottom_left =
		    fill_bottom_right = -1;
		}
	    }
	}
	if (decorTileDoit) {
	    XSetFillStyle(dpy, gc, FillSolid);
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
    int			xi, yi, xb, yb, xe, ye, size, fuel, color;
    int			rxb, ryb;
    int			x, y;
    int			type;
    int			dot;
    int			fill_top_left = -1,
			fill_top_right = -1,
			fill_bottom_left = -1,
			fill_bottom_right = -1;
    char		s[2];
    static const int	INSIDE_BL = BLOCK_SZ - 2;
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
	    XSetTSOrigin(dpy, gc, -realWorld.x, realWorld.y);
	}
    }

    wormDrawCount = (wormDrawCount + 1) & 7;

    xb = ((world.x < 0) ? (world.x - (BLOCK_SZ - 1)) : world.x) / BLOCK_SZ;
    yb = ((world.y < 0) ? (world.y - (BLOCK_SZ - 1)) : world.y) / BLOCK_SZ;
    xe = (world.x + view_width) / BLOCK_SZ;
    ye = (world.y + view_height) / BLOCK_SZ;
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
	    Segment_add(wallColor,
			X(0), Y(0),
			X(0), Y(Setup->height));
	}
	if (world.x + view_width >= Setup->width) {
	    Segment_add(wallColor,
			X(Setup->width), Y(0),
			X(Setup->width), Y(Setup->height));
	}
	if (world.y <= 0) {
	    Segment_add(wallColor,
			X(0), Y(0),
			X(Setup->width), Y(0));
	}
	if (world.y + view_height >= Setup->height) {
	    Segment_add(wallColor,
			X(0), Y(Setup->height),
			X(Setup->width), Y(Setup->height));
	}
    }

    y = yb * BLOCK_SZ;
    yi = mod(yb, Setup->y);
    mapbase = Setup->map_data + yi;

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
			SET_FG(colors[BLUE].pixel);
		    points[0].x = WINSCALE(X(x+(BLOCK_SZ/2)));
		    points[0].y = WINSCALE(Y(y));
		    points[1].x = WINSCALE(X(x));
		    points[1].y = WINSCALE(Y(y+BLOCK_SZ/2));
		    points[2].x = WINSCALE(X(x+BLOCK_SZ/2));
		    points[2].y = WINSCALE(Y(y+BLOCK_SZ));
		    points[3].x = WINSCALE(X(x+BLOCK_SZ));
		    points[3].y = WINSCALE(Y(y+(BLOCK_SZ/2)));
		    points[4] = points[0];

		    if (Check_index_by_pos(xi, yi) == nextCheckPoint) {
			rd.fillPolygon(dpy, p_draw, gc,
				       points, 5,
				       Convex, CoordModeOrigin);
			Erase_rectangle(WINSCALE(X(x)),
					WINSCALE(Y(y+BLOCK_SZ)),
					WINSCALE(BLOCK_SZ),
					WINSCALE(BLOCK_SZ));
		    } else {
			rd.drawLines(dpy, p_draw, gc,
				   points, 5, 0);
			Erase_points(0, points, 5);
		    }
		    break;

		case SETUP_ACWISE_GRAV:
		    Arc_add(RED,
			    X(x+5), Y(y+BLOCK_SZ-5),
			    BLOCK_SZ-10, BLOCK_SZ-10, 64*150, 64*300);
		    Segment_add(RED,
				X(x+BLOCK_SZ/2),
				Y(y+BLOCK_SZ-5),
				X(x+BLOCK_SZ/2+4),
				Y(y+BLOCK_SZ-1));
		    Segment_add(RED,
				X(x+BLOCK_SZ/2),
				Y(y+BLOCK_SZ-5),
				X(x+BLOCK_SZ/2+4),
				Y(y+BLOCK_SZ-9));
		    break;

		case SETUP_CWISE_GRAV:
		    Arc_add(RED,
			    X(x+5), Y(y+BLOCK_SZ-5),
			    BLOCK_SZ-10, BLOCK_SZ-10, 64*90, 64*300);
		    Segment_add(RED,
				X(x+BLOCK_SZ/2),
				Y(y+BLOCK_SZ-5),
				X(x+BLOCK_SZ/2-4),
				Y(y+BLOCK_SZ-1));
		    Segment_add(RED,
				X(x+BLOCK_SZ/2),
				Y(y+BLOCK_SZ-5),
				X(x+BLOCK_SZ/2-4),
				Y(y+BLOCK_SZ-9));
		    break;

		case SETUP_POS_GRAV:
		    Arc_add(RED,
			    X(x+1), Y(y+BLOCK_SZ-1),
			    INSIDE_BL, INSIDE_BL, 0, 64*360);
		    Segment_add(RED,
			      X(x+BLOCK_SZ/2),
			      Y(y+5),
			      X(x+BLOCK_SZ/2),
			      Y(y+BLOCK_SZ-5));
		    Segment_add(RED,
				X(x+5),
				Y(y+BLOCK_SZ/2),
				X(x+BLOCK_SZ-5),
				Y(y+BLOCK_SZ/2));
		    break;

		case SETUP_NEG_GRAV:
		    Arc_add(RED,
			    X(x+1), Y(y+BLOCK_SZ-1),
			    INSIDE_BL, INSIDE_BL, 0, 64*360);
		    Segment_add(RED,
				X(x+5),
				Y(y+BLOCK_SZ/2),
				X(x+BLOCK_SZ-5),
				Y(y+BLOCK_SZ/2));
		    break;

		case SETUP_UP_GRAV:
		    Segment_add(RED,
				X(x+BLOCK_SZ/2),
				Y(y),
				X(x+BLOCK_SZ/2),
				Y(y+BLOCK_SZ));
		    Segment_add(RED,
				X(x+BLOCK_SZ/2),
				Y(y+BLOCK_SZ),
				X(x+BLOCK_SZ/2-10),
				Y(y+BLOCK_SZ-10));
		    Segment_add(RED,
				X(x+BLOCK_SZ/2),
				Y(y+BLOCK_SZ),
				X(x+BLOCK_SZ/2+10),
				Y(y+BLOCK_SZ-10));
		    break;
	        case SETUP_DOWN_GRAV:
		    Segment_add(RED,
				X(x+BLOCK_SZ/2),
				Y(y),
				X(x+BLOCK_SZ/2),
				Y(y+BLOCK_SZ));
		    Segment_add(RED,
				X(x+BLOCK_SZ/2),
				Y(y),
				X(x+BLOCK_SZ/2-10),
				Y(y+10));
		    Segment_add(RED,
				X(x+BLOCK_SZ/2),
				Y(y),
				X(x+BLOCK_SZ/2+10),
				Y(y+10));
		    break;
		case SETUP_RIGHT_GRAV:
		    Segment_add(RED,
				X(x),
				Y(y+BLOCK_SZ/2),
				X(x+BLOCK_SZ),
				Y(y+BLOCK_SZ/2));
		    Segment_add(RED,
				X(x+BLOCK_SZ),
				Y(y+BLOCK_SZ/2),
				X(x+BLOCK_SZ-10),
				Y(y+BLOCK_SZ/2+10));
		    Segment_add(RED,
				X(x+BLOCK_SZ),
				Y(y+BLOCK_SZ/2),
				X(x+BLOCK_SZ-10),
				Y(y+BLOCK_SZ/2-10));
		    break;
		case SETUP_LEFT_GRAV:
		    Segment_add(RED,
				X(x),
				Y(y+BLOCK_SZ/2),
				X(x+BLOCK_SZ),
				Y(y+BLOCK_SZ/2));
		    Segment_add(RED,
				X(x),
				Y(y+BLOCK_SZ/2),
				X(x+10),
				Y(y+BLOCK_SZ/2+10));
		    Segment_add(RED,
				X(x),
				Y(y+BLOCK_SZ/2),
				X(x+10),
				Y(y+BLOCK_SZ/2-10));
		    break;
		case SETUP_WORM_IN:
		case SETUP_WORM_NORMAL:
		    {
			static int wormOffset[8][3] = {
			    { 10, 10, 10 },
			    { 5, 10, 10 },
			    { 0, 10, 10 },
			    { 0, 5, 10 },
			    { 0, 0, 10 },
			    { 5, 0, 10 },
			    { 10, 0, 10 },
			    { 10, 5, 10 }
			};
#define _O	wormOffset[wormDrawCount]
#define ARC(_x, _y, _w)						\
	Arc_add(RED,						\
		X(x) + (_x),					\
		Y(y + BLOCK_SZ) + (_y),				\
		INSIDE_BL - (_w), INSIDE_BL - (_w), 0, 64 * 360)

			SET_FG(colors[RED].pixel);
			ARC(0, 0, 0);
			ARC(_O[0], _O[1], _O[2]);
			ARC(_O[0] * 2, _O[1] * 2, _O[2] * 2);
			break;
		    }

		case SETUP_ITEM_CONCENTRATOR:
		    {
			static struct concentrator_triangle {
			    int		radius;
			    int		displ;
			    int		dir_off;
			    int		rot_speed;
			    int		rot_dir;
			} tris[] = {
#if 0
			    { 14, 3, 0, 1, 0 },
			    { 11, 5, 3, 2, 0 },
			    {  7, 8, 5, 3, 0 },
#else
			    {  7, 3, 0, 3, 0 },
			    { 11, 5, 3, 3, 0 },
			    { 14, 8, 5, 3, 0 },
#endif
			};
			static unsigned	rot_dir;
			static long		concentratorloop;
			unsigned		rdir, tdir;
			int			i, cx, cy;
			XPoint		pts[4];

			SET_FG(colors[RED].pixel);
			if (concentratorloop != loops) {
			    concentratorloop = loops;
			    rot_dir += 5;
			    for (i = 0; i < NELEM(tris); i++) {
				tris[i].rot_dir += tris[i].rot_speed;
			    }
			}
			for (i = 0; i < NELEM(tris); i++) {
			    /* I'll bet you didn't know that floating point math
			       is faster than integer math on a pentium 
			       (and for some reason the UNIX way rounds off too much) */
			    rdir = MOD2(rot_dir + tris[i].dir_off, RES);
			    cx = (int)(X(x + BLOCK_SZ / 2)
				+ tris[i].displ * tcos(rdir));
			    cy = (int)(Y(y + BLOCK_SZ / 2)
				+ tris[i].displ * tsin(rdir));
			    tdir = MOD2(tris[i].rot_dir, RES);
			    pts[0].x = WINSCALE(cx + (int)(tris[i].radius * tcos(tdir)));
			    pts[0].y = WINSCALE(cy + (int)(tris[i].radius * tsin(tdir)));
			    pts[1].x = WINSCALE(cx + (int)(tris[i].radius
					  * tcos(MOD2(tdir + RES/3, RES))));
			    pts[1].y = WINSCALE(cy + (int)(tris[i].radius
					  * tsin(MOD2(tdir + RES/3, RES))));
			    pts[2].x = WINSCALE(cx + (int)(tris[i].radius
					  * tcos(MOD2(tdir + 2*RES/3, RES))));
			    pts[2].y = WINSCALE(cy + (int)(tris[i].radius
					  * tsin(MOD2(tdir + 2*RES/3, RES))));
			    /* Trace("DC: %d cx=%d/%d %d/%d %d/%d %d/%d %d/%d\n", 
				    i, cx, cy, pts[0].x, pts[0].y, 
				    pts[1].x, pts[1].y, pts[2].x, pts[2].y, pts[3].x, pts[3].y); */

			    pts[3] = pts[0];
			    rd.drawLines(dpy, p_draw, gc,
					 pts, NELEM(pts), CoordModeOrigin);
			    Erase_points(0, pts, NELEM(pts));
			}
		    }
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
		    Rectangle_add(BLUE,
				  X(x + BLOCK_SZ / 2) - (map_point_size >> 1),
				  Y(y + BLOCK_SZ / 2) - (map_point_size >> 1),
				  map_point_size, map_point_size);
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
		case SETUP_TARGET+9: {
		    int		a1,a2,b1,b2;
		    int		damage;

		    if (Target_alive(xi, yi, &damage) != 0)
			break;

		    if (self && self->team == type - SETUP_TARGET) {
			color = BLUE;
		    } else {
			color = RED;
		    }
		    SET_FG(colors[color].pixel);

		    a1 = X(x);
		    b1 = Y(y+BLOCK_SZ);
		    a2 = a1 + BLOCK_SZ;
		    b2 = b1 + BLOCK_SZ;
		    Segment_add(color, a1, b1, a1, b2);
		    Segment_add(color, a2, b1, a2, b2);
		    Segment_add(color, a1, b1, a2, b1);
		    Segment_add(color, a1, b2, a2, b2);

		    rd.drawRectangle(dpy, p_draw, gc,
				     WINSCALE(X(x+(BLOCK_SZ+2)/4)),
				     WINSCALE(Y(y+3*BLOCK_SZ/4)),
				     WINSCALE(BLOCK_SZ/2),
				     WINSCALE(BLOCK_SZ/2));
		    Erase_4point(WINSCALE(X(x+(BLOCK_SZ+2)/4)),
				 WINSCALE(Y(y+3*BLOCK_SZ/4)),
				 WINSCALE(BLOCK_SZ/2), WINSCALE(BLOCK_SZ/2));

		    if (BIT(Setup->mode, TEAM_PLAY)) {
			s[0] = '0' + type - SETUP_TARGET; s[1] = '\0';
			size = XTextWidth(gameFont, s, 1);
			rd.drawString(dpy, p_draw, gc,
				      WINSCALE(X(x + BLOCK_SZ/2 - size/2)),
				      WINSCALE(Y(y + BLOCK_SZ/2
					- gameFont->ascent/2)),
				      s, 1);
			Erase_rectangle(WINSCALE(X(x + BLOCK_SZ/2 - size/2))-1,
					WINSCALE(Y(y + BLOCK_SZ/2
						- gameFont->ascent/2))
						- gameFont->ascent - 1,
					size + 3,
					gameFont->ascent+ gameFont->descent+ 2);
		    }

		    if (damage != TARGET_DAMAGE) {
			size = (damage * BLOCK_SZ) / (TARGET_DAMAGE * 2);
			a1 = x + size;
			a2 = y + size;
			b1 = x + (BLOCK_SZ - size);
			b2 = y + (BLOCK_SZ - size);

			Segment_add(RED,
				    X(a1), Y(a2),
				    X(b1), Y(b2));

			Segment_add(RED,
				    X(a1), Y(b2),
				    X(b1), Y(a2));
		    }
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
		    if (self && self->team == type - SETUP_TREASURE) {
			color = BLUE;
		    } else {
			color = RED;
		    }
		    SET_FG(colors[color].pixel);
		    Segment_add(color,
				X(x),Y(y),
				X(x),Y(y + BLOCK_SZ/2));
		    Segment_add(color,
				X(x + BLOCK_SZ),Y(y),
				X(x + BLOCK_SZ),
				Y(y + BLOCK_SZ/2));
		    Segment_add(color,
				X(x),Y(y),
				X(x + BLOCK_SZ),Y(y));
		    Arc_add(color,
			    X(x),
			    Y(y + BLOCK_SZ),
			    BLOCK_SZ, BLOCK_SZ, 0, 64*180);
		    s[1] = '\0'; s[0] = '0' + type - SETUP_TREASURE;
		    rd.drawString(dpy, p_draw, gc,
				  WINSCALE(X(x+BLOCK_SZ/2)),
				  WINSCALE(Y(y+BLOCK_SZ/2)),
				  s, 1);
		    Erase_rectangle(WINSCALE(X(x+BLOCK_SZ/2))-1,
				    WINSCALE(Y(y+BLOCK_SZ/2))-gameFont->ascent,
				    XTextWidth(gameFont, s, 1) + 2,
				    gameFont->ascent + gameFont->descent);
		    break;

		default:
		    break;
		}
	    }
	    else {
		if (!BIT(instruments, SHOW_FILLED_WORLD|SHOW_TEXTURED_WALLS)) {
		    if (type & BLUE_LEFT) {
			Segment_add(wallColor,
				    X(x),
				    Y(y),
				    X(x),
				    Y(y+BLOCK_SZ));
		    }
		    if (type & BLUE_DOWN) {
			Segment_add(wallColor,
				    X(x),
				    Y(y),
				    X(x+BLOCK_SZ),
				    Y(y));
		    }
		    if (type & BLUE_RIGHT) {
			Segment_add(wallColor,
				    X(x+BLOCK_SZ),
				    Y(y),
				    X(x+BLOCK_SZ),
				    Y(y+BLOCK_SZ));
		    }
		    if (type & BLUE_UP) {
			Segment_add(wallColor,
				    X(x),
				    Y(y+BLOCK_SZ),
				    X(x+BLOCK_SZ),
				    Y(y+BLOCK_SZ));
		    }
		    if ((type & BLUE_FUEL) == BLUE_FUEL) {
			fuel = Fuel_by_pos(xi, yi);
			Handle_vfuel(x, y, fuel);
		    }
		    else if (type & BLUE_OPEN) {
			Segment_add(wallColor,
				    X(x),
				    Y(y),
				    X(x+BLOCK_SZ),
				    Y(y+BLOCK_SZ));
		    }
		    else if (type & BLUE_CLOSED) {
			Segment_add(wallColor,
				    X(x),
				    Y(y+BLOCK_SZ),
				    X(x+BLOCK_SZ),
				    Y(y));
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
#if ERASE
			{
			    int left_x = MIN(fill_bottom_left, fill_top_left);
			    int right_x = MAX(fill_bottom_right, fill_top_right);
			    Erase_rectangle(WINSCALE(X(left_x)) - 1,
					    WINSCALE(Y(y + BLOCK_SZ)) - 1,
					    WINSCALE(right_x - left_x) + 4,
					    WINSCALE(BLOCK_SZ) +3);
			}
#endif
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
#if ERASE
	    {
		int left_x = MIN(fill_bottom_left, fill_top_left);
		Erase_rectangle(WINSCALE(X(left_x)) - 1,
				WINSCALE(Y(y + BLOCK_SZ)) - 1,
				WINSCALE(x - left_x) + 4,
				WINSCALE(BLOCK_SZ) + 3);
	    }
#endif
	    fill_top_left =
	    fill_top_right =
	    fill_bottom_left =
	    fill_bottom_right = -1;
	}
    }
}


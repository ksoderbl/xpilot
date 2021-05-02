/* $Id: guimap.c,v 5.7 2002/02/10 19:29:39 bertg Exp $
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
# include "NT/winBitmap.h"
#endif

#include "gfx2d.h"
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
#include "paintmacros.h"
#include "record.h"
#include "xinit.h"
#include "protoclient.h"
#include "blockbitmaps.h"
#include "guimap.h"


char guimap_version[] = VERSION;


/* XXX better include a header. */
extern int	wallColor;		/* Color index for wall drawing */
extern int	decorColor;		/* Color index for decoration drawing */
extern char	*wallTextureFile;	/* Filename of wall texture */
extern char	*decorTextureFile;	/* Filename of decor texture */


extern setup_t	*Setup;


void Gui_paint_walls(int x, int y, int type, int xi, int yi)
{
    if (!blockBitmaps) {
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

	if (type & BLUE_LEFT) {
	    PaintBitmap(p_draw, BM_WALL_LEFT, WINSCALE(X(x - 1)),
			WINSCALE(Y(y + BLOCK_SZ)), WINSCALE(BLOCK_SZ),
			WINSCALE(BLOCK_SZ), 0);
	}
	if (type & BLUE_DOWN) {
	    PaintBitmap(p_draw, BM_WALL_BOTTOM, WINSCALE(X(x)),
			WINSCALE(Y(y + BLOCK_SZ - 1)), WINSCALE(BLOCK_SZ),
			WINSCALE(BLOCK_SZ), 0);
	}
	if (type & BLUE_RIGHT) {
	    PaintBitmap(p_draw, BM_WALL_RIGHT, WINSCALE(X(x + 1)),
			WINSCALE(Y(y + BLOCK_SZ)), WINSCALE(BLOCK_SZ),
			WINSCALE(BLOCK_SZ), 0);
	}
	if (type & BLUE_UP) {
	    PaintBitmap(p_draw, BM_WALL_TOP, WINSCALE(X(x)),
			WINSCALE(Y(y + BLOCK_SZ + 1)), WINSCALE(BLOCK_SZ),
			WINSCALE(BLOCK_SZ), 0);
	}
	if ((type & BLUE_FUEL) == BLUE_FUEL) {
	}

	else if (type & BLUE_OPEN) {
	    PaintBitmap(p_draw, BM_WALL_UR, WINSCALE(X(x)),
			WINSCALE(Y(y + BLOCK_SZ)), WINSCALE(BLOCK_SZ),
			WINSCALE(BLOCK_SZ), 0);
	}
	else if (type & BLUE_CLOSED) {
	    PaintBitmap(p_draw, BM_WALL_UL, WINSCALE(X(x)),
			WINSCALE(Y(y + BLOCK_SZ)), WINSCALE(BLOCK_SZ),
			WINSCALE(BLOCK_SZ), 0);
	}
    }
}


void Gui_paint_cannon(int x, int y, int type)
{
    if (!blockBitmaps) {
	XPoint		points[5];

	SET_FG(colors[WHITE].pixel);
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
	    return;
	}
	points[3] = points[0];
	rd.drawLines(dpy, p_draw, gc, points, 4, 0);
	Erase_points(0, points, 4);
    }
    else {
	switch (type) {
	case SETUP_CANNON_UP:
	    PaintBitmap(p_draw, BM_CANNON_DOWN, WINSCALE(X(x)),
			WINSCALE(Y(y + BLOCK_SZ)), WINSCALE(BLOCK_SZ),
			WINSCALE(BLOCK_SZ), 0);
	    break;
	case SETUP_CANNON_DOWN:
	    PaintBitmap(p_draw, BM_CANNON_UP, WINSCALE(X(x)),
			WINSCALE(Y(y + BLOCK_SZ - 1)), WINSCALE(BLOCK_SZ),
			WINSCALE(BLOCK_SZ), 0);
	    break;
	case SETUP_CANNON_LEFT:
	    PaintBitmap(p_draw, BM_CANNON_RIGHT, WINSCALE(X(x)),
			WINSCALE(Y(y + BLOCK_SZ)), WINSCALE(BLOCK_SZ),
			WINSCALE(BLOCK_SZ), 0);
	    break;
	case SETUP_CANNON_RIGHT:
	    PaintBitmap(p_draw, BM_CANNON_LEFT, WINSCALE(X(x - 1)),
			WINSCALE(Y(y + BLOCK_SZ)), WINSCALE(BLOCK_SZ),
			WINSCALE(BLOCK_SZ), 0);
	    break;
	default:
	    errno = 0;
	    error("Bad base dir.");
	    return;
	}
    }
}


void Gui_paint_fuel(int x, int y, int fuel)
{
	int fuel_images = Block_bitmap_images(BM_FUEL);

    if (!blockBitmaps || !fuel_images) {
#define FUEL_BORDER 2
	int			size;

	static char		s[2] = "F";
	static int		text_width = 0;
	static int		text_is_bigger;

#ifndef WINDOWSCALING
	if (!text_width)
#else
	static DFLOAT		lastScaleFactor;

	if (!text_width || lastScaleFactor != scaleFactor)
#endif
	{
#ifdef WINDOWSCALING

	    lastScaleFactor = scaleFactor;
#endif
	    text_width = XTextWidth(gameFont, s, 1);
	    text_is_bigger = text_width+4 > WINSCALE(BLOCK_SZ)+1 ||
			    (gameFont->ascent + gameFont->descent) > WINSCALE(BLOCK_SZ) + 2;
	}
	SET_FG(colors[RED].pixel);
	size = (BLOCK_SZ - 2*FUEL_BORDER) * fuel / MAX_STATION_FUEL;
	if (useErase) {
	/* speedup for slow old cheap graphics cards like cg3.
	    or Xterminals with slow connection */
	    rd.drawLine(dpy, p_draw, gc,
			WINSCALE(X(x + FUEL_BORDER)),
			WINSCALE(Y(y + FUEL_BORDER + size)),
			WINSCALE(X(x + FUEL_BORDER + (BLOCK_SZ - 2*FUEL_BORDER))),
			WINSCALE(Y(y + FUEL_BORDER + size)));
	} else {
	    rd.fillRectangle(dpy, p_draw, gc,
			     WINSCALE(X(x + FUEL_BORDER)),
			     WINSCALE(Y(y + FUEL_BORDER + size)),
			     WINSCALE(BLOCK_SZ - 2*FUEL_BORDER + 1),
			     WINSCALE(size + 1));
	}
	if (!text_is_bigger)
	    Erase_rectangle(WINSCALE(X(x)) - 1,
			    WINSCALE(Y(y + BLOCK_SZ)) - 1,
			    WINSCALE(BLOCK_SZ) + 2,
			    WINSCALE(BLOCK_SZ) + 2);

	/* Draw F in fuel cells */
	XSetFunction(dpy, gc, GXxor);
	SET_FG(colors[BLACK].pixel ^ colors[RED].pixel);
	x = WINSCALE(X(x + BLOCK_SZ/2)) - text_width/2,
	y = WINSCALE(Y(y + BLOCK_SZ/2)) + gameFont->ascent/2,
	rd.drawString(dpy, p_draw, gc, x, y, s, 1);
	XSetFunction(dpy, gc, GXcopy);

	if (text_is_bigger)
	    Erase_rectangle(x - 2, y - gameFont->ascent,
			    text_width + 4, gameFont->ascent + gameFont->descent);
    }
    else {
#define BITMAP_FUEL_BORDER 3

	int size;
	/* x + x * y will give a pseudo random number, 
	so different fuelcells will not be displayed with the same image-frame.*/
	int image = ( (loops + x + x * y) % (fuel_images * 2) ); 

	/* the animation is played from image 0-15 then back again from image 15-0 */

	if (image >= fuel_images) 
	    image = (2 * fuel_images - 1) - image;


	size = (BLOCK_SZ - 2 * BITMAP_FUEL_BORDER) * fuel / MAX_STATION_FUEL;

	PaintBitmap(p_draw, BM_FUELCELL, WINSCALE(X(x)), WINSCALE(Y(y + BLOCK_SZ)),
		    WINSCALE(BLOCK_SZ), WINSCALE(BLOCK_SZ), 0);

	PaintFuelSlice(p_draw, BM_FUEL,
		      WINSCALE(X(x + BITMAP_FUEL_BORDER)), 
		       WINSCALE(Y(y + size + BITMAP_FUEL_BORDER)), 
		       WINSCALE(BLOCK_SZ - 2 * BITMAP_FUEL_BORDER), 
		       WINSCALE(BLOCK_SZ - 2 * BITMAP_FUEL_BORDER), 
		       image, WINSCALE(size));

	Erase_rectangle(WINSCALE(X(x)) - 1,
			WINSCALE(Y(y + BLOCK_SZ)) - 1,
			WINSCALE(BLOCK_SZ) + 2, WINSCALE(BLOCK_SZ) + 2);

    }
}


void Gui_paint_base(int x, int y, int xi, int yi, int type)
{
    if (!blockBitmaps) {
	const int	BORDER = 4;					/* in pixels */
	int		id, team, size;
	other_t	*other;
	char	s[3];

	SET_FG(colors[WHITE].pixel);

	switch (type) {
	case SETUP_BASE_UP:
	    Segment_add(WHITE,
			X(x), Y(y-1),
			X(x+BLOCK_SZ), Y(y-1));
	    break;
	case SETUP_BASE_DOWN:
	    Segment_add(WHITE,
			X(x), Y(y+BLOCK_SZ+1),
			X(x+BLOCK_SZ), Y(y+BLOCK_SZ+1));
	    break;
	case SETUP_BASE_LEFT:
	    Segment_add(WHITE,
			X(x+BLOCK_SZ+1), Y(y+BLOCK_SZ),
			X(x+BLOCK_SZ+1), Y(y));
	    break;
	case SETUP_BASE_RIGHT:
	    Segment_add(WHITE,
			X(x-1), Y(y+BLOCK_SZ),
			X(x-1), Y(y));
	    break;
	default:
	    errno = 0;
	    error("Bad base dir.");
	    return;
	}
	/* only draw base teams if ship naming is on, SKS 25/05/94 */
	if (!BIT(instruments, SHOW_SHIP_NAME) ||
	    Base_info_by_pos(xi, yi, &id, &team) == -1)
	    return;

	/* operate in pixels from here out */
	x = WINSCALE(X(x));
	y = WINSCALE(Y(y));

	if ((other = Other_by_id(id)) != NULL) {
	    if (other->name_width == 0) {					
		other->name_len = strlen(other->name);
		other->name_width = 2 + XTextWidth(gameFont, other->name,	
						   other->name_len);
	    }
	}
	if (BIT(Setup->mode, TEAM_PLAY)) {
	    s[0] = '0' + team;
	    if (other) {
		s[1] = ' ';
		s[2] = '\0';
	    } else
		s[1] = '\0';
		size = XTextWidth(gameFont, s, other ? 2 : 1);
	} else
	    size = 0;

	switch (type) {
	case SETUP_BASE_UP:
	    y += BORDER + gameFont->ascent;
	    x += WINSCALE(BLOCK_SZ/2);
	    x -= size / 2 + (other ? other->name_width/2 : 0);
	    break;
	case SETUP_BASE_DOWN:
	    y -= WINSCALE(BLOCK_SZ) + BORDER + gameFont->descent;
	    x += WINSCALE(BLOCK_SZ/2);
	    x -= size / 2 + (other ? other->name_width/2 : 0);
	    break;
	case SETUP_BASE_LEFT:
	    x += WINSCALE(BLOCK_SZ) + BORDER;
	    y += -WINSCALE(BLOCK_SZ/2) + gameFont->ascent/2;
	    break;
	case SETUP_BASE_RIGHT:
	    x -= BORDER + (other ? other->name_width : 0) + size;
	    y += -WINSCALE(BLOCK_SZ/2) + gameFont->ascent/2;
	    break;
	}
	if (size) {
	    rd.drawString(dpy, p_draw, gc, x, y, s, other ? 2 : 1);
	     Erase_rectangle(x - 1, y - gameFont->ascent - 1,
			     size + 2,
			     gameFont->ascent + gameFont->descent+2);
	     x += size;
	}
	if (other) {
	     rd.drawString(dpy, p_draw, gc, x, y,
			   other->name, other->name_len);
	     Erase_rectangle(x - 1,
			     y - gameFont->ascent - 1,
			     other->name_width + 2,
			     gameFont->ascent+gameFont->descent + 2);
	}
    }
    else {
	const int BORDER = 4;	/* in pixels */
	int id, team, size;
	other_t *other;
	char s[3];

	SET_FG(colors[WHITE].pixel);

	switch (type) {
	case SETUP_BASE_UP:
	    PaintBitmap(p_draw, BM_BASE_DOWN, WINSCALE(X(x)),
			WINSCALE(Y(y + BLOCK_SZ)), WINSCALE(BLOCK_SZ),
			WINSCALE(BLOCK_SZ), 0);
	    break;
	case SETUP_BASE_DOWN:
	    PaintBitmap(p_draw, BM_BASE_UP, WINSCALE(X(x)),
			WINSCALE(Y(y + BLOCK_SZ - 1)), WINSCALE(BLOCK_SZ),
			WINSCALE(BLOCK_SZ), 0);
	    break;
	case SETUP_BASE_LEFT:
	    PaintBitmap(p_draw, BM_BASE_RIGHT, WINSCALE(X(x)),
			WINSCALE(Y(y + BLOCK_SZ)), WINSCALE(BLOCK_SZ),
			WINSCALE(BLOCK_SZ), 0);
	    break;
	case SETUP_BASE_RIGHT:
	    PaintBitmap(p_draw, BM_BASE_LEFT, WINSCALE(X(x - 1)),
			WINSCALE(Y(y + BLOCK_SZ)), WINSCALE(BLOCK_SZ),
			WINSCALE(BLOCK_SZ), 0);
	    break;
	default:
	    errno = 0;
	    error("Bad base dir.");
	    return;
	}
	/* only draw base teams if ship naming is on, SKS 25/05/94 */
	if (!BIT(instruments, SHOW_SHIP_NAME) ||
	    Base_info_by_pos(xi, yi, &id, &team) == -1)
	    return;

	/* operate in pixels from here out */
	x = WINSCALE(X(x));
	y = WINSCALE(Y(y));

	if ((other = Other_by_id(id)) != NULL) {
	    if (other->name_width == 0) {
		other->name_len = strlen(other->name);
		other->name_width = 2 + XTextWidth(gameFont, other->name,
						   other->name_len);
	    }
	}
	if (BIT(Setup->mode, TEAM_PLAY)) {
	    s[0] = '0' + team;
	    if (other) {
		s[1] = ' ';
		s[2] = '\0';
	    }
	    else
		s[1] = '\0';
	    size = XTextWidth(gameFont, s, other ? 2 : 1);
	}
	else
	    size = 0;

	switch (type) {
	case SETUP_BASE_UP:
	    y += BORDER + gameFont->ascent;
	    x += WINSCALE(BLOCK_SZ / 2);
	    x -= size / 2 + (other ? other->name_width / 2 : 0);
	    break;
	case SETUP_BASE_DOWN:
	    y -= WINSCALE(BLOCK_SZ) + BORDER + gameFont->descent;
	    x += WINSCALE(BLOCK_SZ / 2);
	    x -= size / 2 + (other ? other->name_width / 2 : 0);
	    break;
	case SETUP_BASE_LEFT:
	    x += WINSCALE(BLOCK_SZ) + BORDER;
	    y += -WINSCALE(BLOCK_SZ / 2) + gameFont->ascent / 2;
	    break;
	case SETUP_BASE_RIGHT:
	    x -= BORDER + (other ? other->name_width : 0) + size;
	    y += -WINSCALE(BLOCK_SZ / 2) + gameFont->ascent / 2;
	    break;
	}
	if (size) {
	    rd.drawString(dpy, p_draw, gc, x, y, s, other ? 2 : 1);
	    Erase_rectangle(x - 1, y - gameFont->ascent - 1,
			    size + 2, gameFont->ascent + gameFont->descent + 2);
	    x += size;
	}
	if (other) {
	    rd.drawString(dpy, p_draw, gc, x, y, other->name, other->name_len);
	    Erase_rectangle(x - 1,
			    y - gameFont->ascent - 1,
			    other->name_width + 2,
			    gameFont->ascent + gameFont->descent + 2);
	}
    }
}


void Gui_paint_decor(int x, int y, int xi, int yi, int type, bool last, bool more_y)
{
    XPoint		    points[5];

    int mask;
    int			    fill_top_left = -1,
			    fill_top_right = -1,
			    fill_bottom_left = -1,
			    fill_bottom_right = -1;
    static int		    decorTileReady = 0;
    static Pixmap	    decorTile = None;
    int			    decorTileDoit = false;
    static unsigned char    decor[256];    
    static int		    decorReady = 0;

    SET_FG(colors[decorColor].pixel);

    if (!decorReady) {
	memset(decor, 0, sizeof decor);
	decor[SETUP_DECOR_FILLED] = DECOR_UP | DECOR_LEFT | DECOR_DOWN | DECOR_RIGHT;
	decor[SETUP_DECOR_RU] = DECOR_UP | DECOR_RIGHT | DECOR_CLOSED;
	decor[SETUP_DECOR_RD] = DECOR_DOWN | DECOR_RIGHT | DECOR_OPEN | DECOR_BELOW;
	decor[SETUP_DECOR_LU] = DECOR_UP | DECOR_LEFT | DECOR_OPEN;
	decor[SETUP_DECOR_LD] = DECOR_LEFT | DECOR_DOWN | DECOR_CLOSED | DECOR_BELOW;
    }

    if (BIT(instruments, SHOW_TEXTURED_DECOR)) {
	if (!decorTileReady) {
	    decorTile = Texture_decor();
	    decorTileReady = (decorTile == None) ? -1 : 1;
	}
	if (decorTileReady == 1) {
	    decorTileDoit = true;
	    XSetTile(dpy, gc, decorTile);
	    XSetTSOrigin(dpy, gc, -WINSCALE(realWorld.x), WINSCALE(realWorld.y));
	    XSetFillStyle(dpy, gc, FillTiled);
	}
    }

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
	    && (last || more_y)) {
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
	    if (useErase){
		int left_x = MIN(fill_bottom_left, fill_top_left);
		int right_x = MAX(fill_bottom_right, fill_top_right);
		Erase_rectangle(WINSCALE(X(left_x)) - 1,
				WINSCALE(Y(y + BLOCK_SZ)) - 1,
				WINSCALE(right_x - left_x) + 4,
				WINSCALE(BLOCK_SZ) + 3);
	    }
	    fill_top_left =
	    fill_top_right =
	    fill_bottom_left =
	    fill_bottom_right = -1;
	}
    }
    if (decorTileDoit && last) {
        XSetFillStyle(dpy, gc, FillSolid);
    }
}


void Gui_paint_setup_check(int x, int y, int xi, int yi)
{
    XPoint points[5];
    if (!blockBitmaps) {
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
    } else {
	if (Check_index_by_pos(xi, yi) == nextCheckPoint) {
	    PaintBitmap(p_draw, BM_CHECKPOINT, WINSCALE(X(x)), WINSCALE(Y(y + BLOCK_SZ)),
		WINSCALE(BLOCK_SZ), WINSCALE(BLOCK_SZ), 1);
	
	} else {
	    PaintBitmap(p_draw, BM_CHECKPOINT, WINSCALE(X(x)), WINSCALE(Y(y + BLOCK_SZ)),
		WINSCALE(BLOCK_SZ), WINSCALE(BLOCK_SZ), 0);

	}
	Erase_rectangle(WINSCALE(X(x)),
			WINSCALE(Y(y+BLOCK_SZ)),
			WINSCALE(BLOCK_SZ),
			WINSCALE(BLOCK_SZ));
    }
}


void Gui_paint_border(int x, int y, int xi, int yi)
{
    Segment_add(wallColor,
		X(x), Y(y),
		X(xi), Y(yi));
}


void Gui_paint_visible_border(int x, int y, int xi, int yi)
{
    Segment_add(hudColor,
		X(x), Y(y),
		X(x), Y(yi));

    Segment_add(hudColor,
		X(xi), Y(y),
		X(xi), Y(yi));

    Segment_add(hudColor,
		X(x), Y(y),
		X(xi), Y(y));

    Segment_add(hudColor,
		X(x), Y(yi),
		X(xi), Y(yi));
}


void Gui_paint_setup_acwise_grav(int x, int y)
{
    Arc_add(RED, X(x+5), Y(y+BLOCK_SZ-5),
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
}


void Gui_paint_setup_cwise_grav(int x, int y)
{
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
}


void Gui_paint_setup_pos_grav(int x, int y)
{
    static const int	INSIDE_BL = BLOCK_SZ - 2;
    if (!blockBitmaps) {

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
    } else {
	PaintBitmap(p_draw, BM_PLUSGRAVITY, WINSCALE(X(x)), WINSCALE(Y(y + BLOCK_SZ)),
		    WINSCALE(BLOCK_SZ), WINSCALE(BLOCK_SZ), 0);

    }
}


void Gui_paint_setup_neg_grav(int x, int y)
{
    static const int	INSIDE_BL = BLOCK_SZ - 2;

    if (!blockBitmaps) {

	Arc_add(RED,
		X(x+1), Y(y+BLOCK_SZ-1),
		INSIDE_BL, INSIDE_BL, 0, 64*360);
	Segment_add(RED,
		    X(x+5),
		    Y(y+BLOCK_SZ/2),
		    X(x+BLOCK_SZ-5),
		    Y(y+BLOCK_SZ/2));
    } else {
	PaintBitmap(p_draw, BM_MINUSGRAVITY, WINSCALE(X(x)), WINSCALE(Y(y + BLOCK_SZ)),
		    WINSCALE(BLOCK_SZ), WINSCALE(BLOCK_SZ), 0);

    }
}



void Gui_paint_setup_up_grav(int x, int y)
{
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
}



void Gui_paint_setup_down_grav(int x, int y)
{
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
}



void Gui_paint_setup_right_grav(int x, int y)
{
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
}



void Gui_paint_setup_left_grav(int x, int y)
{
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
}


void Gui_paint_setup_worm(int x, int y, int wormDrawCount)
{
    if (!blockBitmaps) {
	static const int	INSIDE_BL = BLOCK_SZ - 2;
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

	SET_FG(colors[RED].pixel);
	Arc_add(RED, 
		X(x) + 0, Y(y + BLOCK_SZ) + 0, 
		INSIDE_BL - 0, INSIDE_BL - 0, 0, 64 * 360);

	Arc_add(RED, 
		X(x) + _O[0], Y(y + BLOCK_SZ) + _O[1], 
		INSIDE_BL - _O[2], INSIDE_BL - _O[2], 0, 64 * 360);

	Arc_add(RED, 
		X(x) + _O[0] * 2, Y(y + BLOCK_SZ) + _O[1] * 2, 
		INSIDE_BL - _O[2] * 2, INSIDE_BL - _O[2] * 2, 0, 64 * 360);
    }
    else {
	PaintBitmap(p_draw, BM_WORMHOLE, WINSCALE(X(x)), WINSCALE(Y(y + BLOCK_SZ)),
		    WINSCALE(BLOCK_SZ), WINSCALE(BLOCK_SZ), wormDrawCount);
    }
}


void Gui_paint_setup_item_concentrator(int x, int y)
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
    if (!blockBitmaps) {
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
    } else {
	PaintBitmap(p_draw, BM_CONCENTRATOR, WINSCALE(X(x)), WINSCALE(Y(y + BLOCK_SZ)),
		    WINSCALE(BLOCK_SZ), WINSCALE(BLOCK_SZ), (loops + (x + x * y)) % 32);

    }
}


void Gui_paint_setup_asteroid_concentrator(int x, int y) 
{
    static struct concentrator_square {
	int		size;
	int		displ;
	int		dir_off;
	int		rot_speed;
	int		rot_dir;
    } sqrs[] = {
#if 0
	{ 16, 3, 0, 1, 0 },
	{ 12, 5, 3, 2, 0 },
	{  8, 8, 5, 3, 0 },
#else
	{  8, 3, 0, 3, 0 },
	{ 12, 5, 3, 3, 0 },
	{ 16, 8, 5, 3, 0 },
#endif
    };
    static unsigned	rot_dir;
    static long		concentratorloop;
    unsigned		rdir, tdir;
    int			i, cx, cy;
    XPoint		pts[5];
    if (!blockBitmaps) {
	SET_FG(colors[RED].pixel);
	if (concentratorloop != loops) {
	    concentratorloop = loops;
	    rot_dir += 5;
	    for (i = 0; i < NELEM(sqrs); i++) {
		sqrs[i].rot_dir += sqrs[i].rot_speed;
	    }
	}
	for (i = 0; i < NELEM(sqrs); i++) {
	    /* I'll bet you didn't know that floating point math
	       is faster than integer math on a pentium 
	       (and for some reason the UNIX way rounds off too much) */
	    rdir = MOD2(rot_dir + sqrs[i].dir_off, RES);
	    cx = (int)(X(x + BLOCK_SZ / 2)
		+ sqrs[i].displ * tcos(rdir));
	    cy = (int)(Y(y + BLOCK_SZ / 2)
		+ sqrs[i].displ * tsin(rdir));
	    tdir = MOD2(sqrs[i].rot_dir, RES);
	    pts[0].x = WINSCALE(cx + (int)(sqrs[i].size * tcos(tdir)));
	    pts[0].y = WINSCALE(cy + (int)(sqrs[i].size * tsin(tdir)));
	    pts[1].x = WINSCALE(cx + (int)(sqrs[i].size * tcos(MOD2(tdir + RES/4, RES))));
	    pts[1].y = WINSCALE(cy + (int)(sqrs[i].size * tsin(MOD2(tdir + RES/4, RES))));
	    pts[2].x = WINSCALE(cx + (int)(sqrs[i].size * tcos(MOD2(tdir + 2*RES/4, RES))));
	    pts[2].y = WINSCALE(cy + (int)(sqrs[i].size * tsin(MOD2(tdir + 2*RES/4, RES))));
		pts[3].x = WINSCALE(cx + (int)(sqrs[i].size * tcos(MOD2(tdir + 3*RES/4, RES))));
	    pts[3].y = WINSCALE(cy + (int)(sqrs[i].size * tsin(MOD2(tdir + 3*RES/4, RES))));
	    /* Trace("DC: %d cx=%d/%d %d/%d %d/%d %d/%d %d/%d\n", 
		    i, cx, cy, pts[0].x, pts[0].y, 
		    pts[1].x, pts[1].y, pts[2].x, pts[2].y, pts[3].x, pts[3].y); */

	    pts[4] = pts[0];
	    rd.drawLines(dpy, p_draw, gc,
			 pts, NELEM(pts), CoordModeOrigin);
	    Erase_points(0, pts, NELEM(pts));
	}
    } else {
	PaintBitmap(p_draw, BM_ASTEROIDCONC, WINSCALE(X(x)), WINSCALE(Y(y + BLOCK_SZ)),
		    WINSCALE(BLOCK_SZ), WINSCALE(BLOCK_SZ), (loops + (x + x * y)) % 32);

    }
}


void Gui_paint_decor_dot(int x, int y, int size)
{
    Rectangle_add(BLUE, X(x + BLOCK_SZ / 2) - (map_point_size >> 1),
		  Y(y + BLOCK_SZ / 2) - (map_point_size >> 1),
		  size, size);
}


void Gui_paint_setup_target(int x, int y, int target, int damage, bool own)
{
    int	    size, a1, a2, b1, b2, color;
    char    s[2];

    color = own ? BLUE : RED;
	
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
	s[0] = '0' + target; s[1] = '\0';
	size = XTextWidth(gameFont, s, 1);
	rd.drawString(dpy, p_draw, gc,
		      WINSCALE(X(x + BLOCK_SZ/2)) - size/2,
		      WINSCALE(Y(y + BLOCK_SZ/2))
			+ gameFont->ascent/2,
		      s, 1);
	Erase_rectangle(WINSCALE(X(x + BLOCK_SZ/2)) - size/2-1,
			WINSCALE(Y(y + BLOCK_SZ/2))
				+ gameFont->ascent/2
				- gameFont->ascent - 1,
			size + 2,
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


void Gui_paint_setup_treasure(int x, int y, int treasure, bool own)
{
    if (!blockBitmaps) {
	char    s[2];
	int	    color;
	int	    size;
	color = own ? BLUE : RED;

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
	if (BIT(Setup->mode, TEAM_PLAY)) {
	    s[1] = '\0'; s[0] = '0' + treasure;
	    size = XTextWidth(gameFont, s, 1);
	    rd.drawString(dpy, p_draw, gc,
			  WINSCALE(X(x + BLOCK_SZ/2)) - size/2,
			  WINSCALE(Y(y + 2*BALL_RADIUS)), s, 1);
	    Erase_rectangle(WINSCALE(X(x + BLOCK_SZ/2)) - size/2 - 1,
			    WINSCALE(Y(y + 2*BALL_RADIUS))
			      - gameFont->ascent - 1,
			    size + 2,
			    gameFont->ascent+ gameFont->descent+ 2);
	}
    }
    else {
	char    s[2];
	int	    size;    
	
	int type;
	int	    color;

	type = own ? BM_HOLDER_FRIEND : BM_HOLDER_ENEMY;

	PaintBitmap(p_draw, type, WINSCALE(X(x)), WINSCALE(Y(y + BLOCK_SZ)),
		    WINSCALE(BLOCK_SZ), WINSCALE(BLOCK_SZ), 0);

	if (BIT(Setup->mode, TEAM_PLAY)) {
	    color = own ? BLUE : RED;
            SET_FG(colors[color].pixel);

	    s[1] = '\0'; s[0] = '0' + treasure;
	    size = XTextWidth(gameFont, s, 1);
	    rd.drawString(dpy, p_draw, gc,
			  WINSCALE(X(x + BLOCK_SZ/2)) - size/2,
			  WINSCALE(Y(y + BALL_RADIUS + 5)), s, 1);
	    Erase_rectangle(WINSCALE(X(x + BLOCK_SZ/2 )) - size/2 - 1,
			    WINSCALE(Y(y + BALL_RADIUS + 5))
			      - gameFont->ascent - 1,
			    size + 2,
			    gameFont->ascent+ gameFont->descent+ 2);
	}

    }
}


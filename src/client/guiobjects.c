/*
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
#include <time.h>
#include <limits.h>
#include <math.h>
#include <sys/types.h>

#ifndef _WINDOWS
# include <unistd.h>
# include <X11/Xlib.h>
# include <X11/Xos.h>
#endif

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
#include "portability.h"
#include "blockbitmaps.h"
#include "wreckshape.h"
#include "astershape.h"
#include "guiobjects.h"
#include "pack.h"
#include "commonproto.h"


extern setup_t		*Setup;


extern XGCValues	gcv;

int blockBitmapShips = 1;

void Gui_paint_ball(int x, int y)
{
    if (!blockBitmaps) {
	static Pixmap   ballTile = None;

	if (BIT(instruments, SHOW_TEXTURED_BALLS)) {
	    if (ballTile == None) {
		ballTile = Texture_ball();
		if (ballTile == None) {
		    CLR_BIT(instruments, SHOW_TEXTURED_BALLS);
		}
	    }
	    if (ballTile != None) {
		XSetTile(dpy, gc, ballTile);
		XSetFillStyle(dpy, gc, FillTiled);
	    }
	}
	else {
	    ballTile = None;
	}

	x = X(x);
	y = Y(y);
	if (ballTile != None) {
	    XSetTSOrigin(dpy, gc, WINSCALE(x - BALL_RADIUS), WINSCALE(y - BALL_RADIUS));
	    rd.fillArc(dpy, p_draw, gc,
		WINSCALE(x - BALL_RADIUS), WINSCALE(y - BALL_RADIUS),
		     WINSCALE(2*BALL_RADIUS), WINSCALE(2*BALL_RADIUS),
		     0, 64*360);
	    Erase_rectangle(WINSCALE(x - BALL_RADIUS),
			    WINSCALE(y - BALL_RADIUS),
			    WINSCALE(2*BALL_RADIUS),
			    WINSCALE(2*BALL_RADIUS));
	}
	else {
	    Arc_add(WHITE, x - BALL_RADIUS, y - BALL_RADIUS,
		    2*BALL_RADIUS, 2*BALL_RADIUS, 0, 64*360);
	}
	if (ballTile != None) {
	    XSetFillStyle(dpy, gc, FillSolid);
	}
    }
    else {

	x = X(x);
	y = Y(y);
	PaintBitmap(p_draw, BM_BALL, WINSCALE(x - BALL_RADIUS),
		    WINSCALE(y - BALL_RADIUS), WINSCALE(BALL_RADIUS * 2 + 1),
		    WINSCALE(BALL_RADIUS * 2 + 1), 0);
    }
}


void Gui_paint_ball_connecter(int x1, int y1, int x2, int y2)
{
    x2 = X(x2);
    y2 = Y(y2);
    x1 = X(x1);
    y1 = Y(y1);
    Segment_add(WHITE, x1, y1, x2, y2);
}

/* used by Paint_mine */
static void Gui_paint_mine_name(int x, int y, char *name) 
{
    int		name_len, name_width;

    if (!name) {
	return;
    }

    name_len = strlen(name);
    name_width = XTextWidth(gameFont, name, name_len);

    if (name != NULL) {
	rd.drawString(dpy, p_draw, gc,
		    WINSCALE(x) - name_width / 2,
		    WINSCALE(y + 4) + gameFont->ascent + 1,
		    name, name_len);
	Erase_rectangle(WINSCALE(x) - name_width / 2 - 1,
			WINSCALE(y + 4) - 3 + 1,
			name_width + 2,
			gameFont->ascent + gameFont->descent
			 + 6);
    }
}

void Gui_paint_mine(int x, int y, int teammine, char *name)
{
    if (!blockBitmaps) {
	static DFLOAT	lastScaleFactor;
	static XPoint	mine_points[21];
	static XPoint	world_mine_points[21] = {
	    { 0, 0 },
	    { 1, 0 },
	    { 0, -1 },
	    { 4, 0 },
	    { 0, -1 },
	    { 6, 0 },
	    { 0, 1 },
	    { 4, 0 },
	    { 0, 1 },
	    { 1, 0 },
	    { 0, 2 },
	    { -1, 0 },
	    { 0, 1 },
	    { -4, 0 },
	    { 0, 1 },
	    { -6, 0 },
	    { 0, -1 },
	    { -4, 0 },
	    { 0, -1 },
	    { -1, 0 },
	    { 0, -2 }
	};

	if (lastScaleFactor != scaleFactor) {
	    int			i;
	    lastScaleFactor = scaleFactor;
	    for (i = 1; i < 21; ++i) {
		mine_points[i].x = WINSCALE(world_mine_points[i].x);
		mine_points[i].y = WINSCALE(world_mine_points[i].y);
	    }
	}

	x = X(x);
	y = Y(y);
	mine_points[0].x = WINSCALE(x - 8);
	mine_points[0].y = WINSCALE(y - 1);
	if (teammine == 0) {
	    SET_FG(colors[BLUE].pixel);
	    rd.fillRectangle(dpy, p_draw, gc,
			WINSCALE(x - 7), WINSCALE(y - 2), 
			WINSCALE(15), WINSCALE(5));
	}

	SET_FG(colors[WHITE].pixel);
	rd.drawLines(dpy, p_draw, gc,
		   mine_points, 21, CoordModePrevious);
	Erase_rectangle( WINSCALE(x - 8) - 1, WINSCALE(y - 4) - 1, 
			WINSCALE(17)+2, WINSCALE(9)+2);

	if (name) {
	    Gui_paint_mine_name(x, y, name);
	}
    }
    else {
	x = X(x);
	y = Y(y);
	if (teammine == 0) {
	    SET_FG(colors[BLUE].pixel);
	    PaintBitmap(p_draw, BM_MINE_OTHER, WINSCALE(x - 10), WINSCALE(y - 7),
			WINSCALE(21), WINSCALE(15), 0);
	}
	else {
	    SET_FG(colors[WHITE].pixel);
	    PaintBitmap(p_draw, BM_MINE_TEAM, WINSCALE(x - 10), WINSCALE(y - 7),
			WINSCALE(21), WINSCALE(15), 0);
	}

	if (name) {
	    Gui_paint_mine_name(x, y, name);
	}
    }
}


void Gui_paint_spark(int color, int x, int y)
{
    color = spark_color[color];

    Rectangle_add(color, 
		x - spark_size/2,
		y - spark_size/2,
		spark_size, spark_size);

}


void Gui_paint_wreck(int x, int y, bool deadly, int wtype, int rot, int size) 
{
    int color, cnt, tx, ty;
    static XPoint points[NUM_WRECKAGE_POINTS+2];

    for (cnt = 0; cnt < NUM_WRECKAGE_POINTS; cnt++) {
	tx = (int)(wreckageShapes[wtype][cnt][rot].x * size) >> 8;
	ty = (int)(wreckageShapes[wtype][cnt][rot].y * size) >> 8;

	points[cnt].x = WINSCALE(X(x + tx));
	points[cnt].y = WINSCALE(Y(y + ty));

    }
    points[cnt++] = points[0];

    color = (deadly) ? WHITE: RED;
    SET_FG(colors[color].pixel);
    rd.drawLines(dpy, p_draw, gc, points, cnt, 0);
    Erase_points(0, points, cnt);

}


void Gui_paint_asteroid(int x, int y, int type, int rot, int size)
{
    int cnt, tx, ty;
    static XPoint points[NUM_ASTEROID_POINTS+2];

    for (cnt = 0; cnt < NUM_ASTEROID_POINTS; cnt++) {
	tx = (int)(asteroidShapes[type][cnt][rot].x * size * 1.4);
	ty = (int)(asteroidShapes[type][cnt][rot].y * size * 1.4);

	points[cnt].x = WINSCALE(X(x + tx));
	points[cnt].y = WINSCALE(Y(y + ty));

    }
    points[cnt++] = points[0];

    SET_FG(colors[WHITE].pixel);
    rd.drawLines(dpy, p_draw, gc, points, cnt, 0);
    Erase_points(0, points, cnt);

}


static void Gui_paint_nastyshot(int color, int x, int y, int size)
{
	int z = size;

    if (rfrac() < 0.5f) {
	Segment_add(color, 
		    x - z, y - z, 
		    x + z, y + z);
	Segment_add(color,
		    x + z, y - z,
		    x - z, y + z);
    } else {
	Segment_add(color,
		    x - z, y,
		    x  + z, y);
	Segment_add(color,
		    x, y - z,
		    x, y + z);
    }
}


void Gui_paint_fastshot(int color, int x, int y)
{
    if (!blockBitmaps) {
        int z = shot_size/2;

	if (showNastyShots) {
	    Gui_paint_nastyshot(color, x, y, z);
	} else {
	    Rectangle_add(color,
			  x - z,
			  y - z,
			  shot_size, shot_size);
	}
    }
    else {
	int s_size = (shot_size > 8) ? 8 : shot_size ;
	int z = s_size / 2;
	PaintBitmap(p_draw, BM_BULLET, WINSCALE(x ) - z, WINSCALE(y ) - z, 8, 8, s_size - 1);
    }
}

void Gui_paint_teamshot(int color, int x, int y)
{
    if (!blockBitmaps) {
	Gui_paint_nastyshot(color, x, y, shot_size/2);
    }
    else {
	int s_size = (teamshot_size > 8) ? 8 : shot_size ;
	int z = s_size / 2;
	PaintBitmap(p_draw, BM_BULLET_OWN, WINSCALE(x ) - z, WINSCALE(y ) - z, 8, 8, s_size - 1);
    }
}


void Gui_paint_missiles_begin(void)
{
#if defined(_WINDOWS) && !defined(PENS_OF_PLENTY)
    SET_FG(MISSILECOLOR);
#else
    SET_FG(colors[WHITE].pixel);
#endif
    XSetLineAttributes(dpy, gc, 4,
		       LineSolid, CapButt, JoinMiter);

}


void Gui_paint_missiles_end(void)
{
    XSetLineAttributes(dpy, gc, 0,
		       LineSolid, CapButt, JoinMiter);
}


void Gui_paint_missile(int x, int y, int len, int dir)
{
   int		x1, x2, y1, y2;

    x1 = X(x);
    y1 = Y(y);
    x2 = (int)(x1 - tcos(dir) * len);
    y2 = (int)(y1 + tsin(dir) * len);
    rd.drawLine(dpy, p_draw, gc, 
	    WINSCALE(x1), WINSCALE(y1), WINSCALE(x2), WINSCALE(y2));
    Erase_segment(4, WINSCALE(x1) , WINSCALE(y1),
		  WINSCALE(x2) , WINSCALE(y2));
}


void Gui_paint_lasers_begin(void)
{
    XSetLineAttributes(dpy, gc, 3,
			   LineSolid, CapButt, JoinMiter);
}


void Gui_paint_lasers_end(void)
{
    XSetLineAttributes(dpy, gc, 0,
			   LineSolid, CapButt, JoinMiter);
}


void Gui_paint_laser(int color, int x1, int y1, int len, int dir)
{
    int		x2, y2;

    x2 = (int)(x1 + len * tcos(dir));
    y2 = (int)(y1 + len * tsin(dir));
    if ((unsigned)(color) >= NUM_COLORS) {
	color = WHITE;
    }
#if !defined(_WINDOWS) || defined(PENS_OF_PLENTY)
    SET_FG(colors[color].pixel);
#else
    SET_FG((unsigned)(color == BLUE ? LASERTEAMCOLOR : LASERCOLOR));
#endif
    rd.drawLine(dpy, p_draw, gc,
	      WINSCALE(X(x1)), WINSCALE(Y(y1)),
	      WINSCALE(X(x2)), WINSCALE(Y(y2)));
    Erase_segment(3, WINSCALE(X(x1)), WINSCALE(Y(y1)),
		     WINSCALE(X(x2)), WINSCALE(Y(y2)) );

}


void Gui_paint_paused(int x, int y, int count)
{
    if (!blockBitmaps) {

	int		x0, y0;
	static int	pauseCharWidth = -1;

	const int half_pause_size = 3*BLOCK_SZ/7;

	if (pauseCharWidth < 0) {
	    pauseCharWidth = XTextWidth(gameFont, "P", 1);
	}
	SET_FG(colors[BLUE].pixel);
	x0 = X(x - half_pause_size);
	y0 = Y(y + half_pause_size);
	rd.fillRectangle(dpy, p_draw, gc,
		       WINSCALE(x0), WINSCALE(y0),
		       WINSCALE(2*half_pause_size+1), WINSCALE(2*half_pause_size+1));
	if (count <= 0 || loops % 10 >= 5) {
	    SET_FG(colors[mono?BLACK:WHITE].pixel);
	    rd.drawRectangle(dpy, p_draw, gc,
			   WINSCALE(x0 - 1),
			   WINSCALE(y0 - 1),
			   WINSCALE(2*(half_pause_size+1)),
			   WINSCALE(2*(half_pause_size+1)));
	    rd.drawString(dpy, p_draw, gc,
			WINSCALE(X(x)) - pauseCharWidth/2,
			WINSCALE(Y(y-1)) + gameFont->ascent/2,
			"P", 1);
	}
	Erase_rectangle(WINSCALE(x0 - 1) - 1, WINSCALE(y0 - 1) - 1,
			WINSCALE(2*half_pause_size+1)+3,
			WINSCALE(2*half_pause_size+1)+3);

    }
    else {
	PaintBitmap(p_draw, BM_PAUSED, WINSCALE(X(x - BLOCK_SZ / 2)),
		    WINSCALE(Y(y + BLOCK_SZ / 2)), WINSCALE(35), WINSCALE(35),
		    (count <= 0 || loops % 10 >= 5) ? 1 : 0);
    }
}


void Gui_paint_ecm(int x, int y, int size)
{
    Arc_add(WHITE,
	    X(x - size / 2),
	    Y(y + size / 2),
	    size, size, 0, 64 * 360);
}


void Gui_paint_refuel(int x0, int y0, int x1, int y1)
{
    if (!blockBitmaps) {

#ifdef _WINDOWS
	SET_FG(colors[WHITE].pixel+CLOAKCOLOROFS);	/* dashed line */
#endif
	rd.drawLine(dpy, p_draw, gc,
		    WINSCALE(X(x0)), WINSCALE(Y(y0)),
		    WINSCALE(X(x1)), WINSCALE(Y(y1)));
	Erase_segment(1, WINSCALE(X(x0)), WINSCALE(Y(y0)),
		      WINSCALE(X(x1)), WINSCALE(Y(y1)));
    }
    else {
	int size = WINSCALE(8);
	double dx, dy;
	int i;
	int fuel[16] = { 1, 2, 3, 3, 2, 1, 0, 1, 2, 3, 2, 1, 2, 3, 3, 2 };

	x0 = WINSCALE(X(x0));
	y0 = WINSCALE(Y(y0));
	x1 = WINSCALE(X(x1));
	y1 = WINSCALE(Y(y1));
	dx = (double)(x1 - x0) / 16;
	dy = (double)(y1 - y0) / 16;
	for (i = 0; i < 16; i++) {
	    PaintBitmap(p_draw, BM_REFUEL, (int)(x0 + (dx * i) - size / 2),
			(int)(y0 + (dy * i) - size / 2), size, size,
			fuel[(loops + 16 - i) % 16]);
	}
    }
}


void Gui_paint_connector(int x0, int y0, int x1, int y1, int tractor)
{
    if (tractor) {
#ifdef _WINDOWS
	SET_FG(colors[WHITE].pixel);
#endif
	rd.setDashes(dpy, gc, 0, cdashes, NUM_CDASHES);
    } else {
#ifdef _WINDOWS
	SET_FG(colors[WHITE].pixel+CLOAKCOLOROFS);	/* dashed line */
#endif
	rd.setDashes(dpy, gc, 0, dashes, NUM_DASHES);
    }
    rd.drawLine(dpy, p_draw, gc,
	      WINSCALE(X(x0)), WINSCALE(Y(y0)),
	      WINSCALE(X(x1)), WINSCALE(Y(y1)));
    Erase_segment(1, WINSCALE(X(x0)), WINSCALE(Y(y0)),
		  WINSCALE(X(x1)), WINSCALE(Y(y1)));
    if (tractor) {
	rd.setDashes(dpy, gc, 0, dashes, NUM_DASHES);
    }
}


void Gui_paint_transporter(int x0, int y0, int x1, int y1)
{
#ifdef _WINDOWS
    SET_FG(colors[WHITE].pixel+CLOAKCOLOROFS);	/* dashed line */
#endif

    rd.drawLine(dpy, p_draw, gc,
	      WINSCALE(X(x0)), WINSCALE(Y(y0)), 
		  WINSCALE(X(x1)), WINSCALE(Y(y1)));
    Erase_segment(1, WINSCALE(X(x0)), WINSCALE(Y(y0)),
		  WINSCALE(X(x1)), WINSCALE(Y(y1)));
}


void Gui_paint_all_connectors_begin()
{
    unsigned long	mask;

    SET_FG(colors[WHITE].pixel);
    if (gcv.line_style != LineOnOffDash) {
	gcv.line_style = LineOnOffDash;
	mask = GCLineStyle;
#ifndef NO_ROTATING_DASHES
	mask |= GCDashOffset;
#endif
	XChangeGC(dpy, gc, mask, &gcv);
    }

}


void Gui_paint_ships_begin()
{
    gcv.dash_offset = WINSCALE(DASHES_LENGTH - (loops % DASHES_LENGTH));
}


void Gui_paint_ships_end()
{
   unsigned long	mask;
   if (gcv.line_style != LineSolid) {
	gcv.line_style = LineSolid;
	mask = GCLineStyle;
	XChangeGC(dpy, gc, mask, &gcv);
    }
    gcv.dash_offset = 0;
}


static void Gui_paint_rounddelay(int x, int y)
{
    char s[12];
    int	 t, text_width;

    sprintf(s, "%d", roundDelay / FPS);
    t = strlen(s);
    SET_FG(colors[WHITE].pixel);
    text_width = XTextWidth(gameFont, s, t);
    rd.drawString(dpy, p_draw, gc,
		WINSCALE(X(x)) - text_width / 2,
		WINSCALE(Y(y)) + gameFont->ascent/2,
		s, t);
}


/*  Here starts the paint functions for ships  (MM) */
static void Gui_paint_ship_name(int x , int y, other_t *other)
{
    FIND_NAME_WIDTH(other);
    SET_FG(colors[WHITE].pixel);
    rd.drawString(dpy, p_draw, gc,
		WINSCALE(X(x)) - other->name_width / 2,
		WINSCALE(Y(y) + 16) + gameFont->ascent,
		other->name, other->name_len);
    Erase_rectangle(WINSCALE(X(x)) - other->name_width / 2 - 1,
		    WINSCALE(Y(y) + 16) + gameFont->ascent
		     - gameFont->ascent, other->name_width + 4,
		    gameFont->ascent + gameFont->descent + 5);
}


static int Gui_is_my_tank(other_t *other)
{
    char	tank_name[MAX_NAME_LEN];

    if (self == NULL
	|| other == NULL
	|| other->mychar != 'T'
	|| (BIT(Setup->mode, TEAM_PLAY)
	&& self->team != other->team)) {
	    return 0;
    }

    if (strlcpy(tank_name, self->name, MAX_NAME_LEN) < MAX_NAME_LEN) {
	strlcat(tank_name, "'s tank", MAX_NAME_LEN);
    }

    if (strcmp(tank_name, other->name)) {
	return 0;
    }

    return 1;
}


static int Gui_calculate_ship_color(int id, other_t *other)
{
    int ship_color = WHITE;

    if (useErase){
	/*
	 * Outline the locked ship in a different color,
	 * instead of mucking around with polygons.
	 */
	if (lock_id == id  && id != -1 && lock_dist != 0) {
	    ship_color = RED;
	}
    }

#ifndef NO_BLUE_TEAM
    if (BIT(Setup->mode, TEAM_PLAY)
	&& self != NULL
	&& self->id != id
	&& other != NULL
	&& self->team == other->team) {
	    ship_color = BLUE;
    }

    if (self != NULL
	&& self->id != id
	&& other != NULL
	&& self->alliance != ' '
	&& self->alliance == other->alliance) {
	    ship_color = BLUE;
    }

    if (Gui_is_my_tank(other)) {
	ship_color = BLUE;
    }
#endif
    if (roundDelay > 0 && ship_color == WHITE) {
	ship_color = RED;
    }
    return ship_color;
}


static void Gui_paint_marking_lights(int id, int x, int y, shipobj *ship, int dir)
{
    int lcnt;

    if (((loops + id) & 0xF) == 0) {
	for (lcnt = 0; lcnt < ship->num_l_light; lcnt++) {
	    Rectangle_add(RED,
			  X(x + ship->l_light[lcnt][dir].x) - 2,
			  Y(y + ship->l_light[lcnt][dir].y) - 2,
			  6, 6);
	    Segment_add(RED,
			X(x + ship->l_light[lcnt][dir].x)-8,
			Y(y + ship->l_light[lcnt][dir].y),
			X(x + ship->l_light[lcnt][dir].x)+8,
			Y(y + ship->l_light[lcnt][dir].y));
	    Segment_add(RED,
			X(x + ship->l_light[lcnt][dir].x),
			Y(y + ship->l_light[lcnt][dir].y)-8,
			X(x + ship->l_light[lcnt][dir].x),
			Y(y + ship->l_light[lcnt][dir].y)+8);
	}
    } else if (((loops + id) & 0xF) == 2) {
	for (lcnt = 0; lcnt < ship->num_r_light; lcnt++) {
	    int rightLightColor = maxColors > 4 ? 4 : BLUE;
	    Rectangle_add(rightLightColor,
			  X(x + ship->r_light[lcnt][dir].x)-2,
			  Y(y + ship->r_light[lcnt][dir].y)-2,
			  6, 6);
	    Segment_add(rightLightColor,
			X(x + ship->r_light[lcnt][dir].x)-8,
			Y(y + ship->r_light[lcnt][dir].y),
			X(x + ship->r_light[lcnt][dir].x)+8,
			Y(y + ship->r_light[lcnt][dir].y));
	    Segment_add(rightLightColor,
			X(x + ship->r_light[lcnt][dir].x),
			Y(y + ship->r_light[lcnt][dir].y)-8,
			X(x + ship->r_light[lcnt][dir].x),
			Y(y + ship->r_light[lcnt][dir].y)+8);
	}
    }
}


static void Gui_paint_shields_deflectors(int x, int y, int radius, int shield,
				  int deflector, int eshield, int ship_color)
{
    int		e_radius = radius + 4;
    int		half_radius = radius >> 1;
    int		half_e_radius = e_radius >> 1;
    int		scolor = -1;
    int		ecolor = -1;

    if (shield) 
	scolor = ship_color;
    if (deflector)
	ecolor = loops & 0x02 ? RED : BLUE;
    if (eshield && shield) {
	if (ecolor != -1) {
	    scolor = ecolor;
	    ecolor = ship_color;
	} else {
	    scolor = ecolor = ship_color;
	}
    }

    if (ecolor != -1) {		/* outer shield */
	    SET_FG(colors[ecolor].pixel);
	rd.drawArc(dpy, p_draw, gc, 
		   WINSCALE(X(x - half_e_radius)), 
		   WINSCALE(Y(y + half_e_radius)),
		   WINSCALE(e_radius), WINSCALE(e_radius),
		   0, 64 * 360);
	Erase_arc(WINSCALE(X(x - half_e_radius)),
		  WINSCALE(Y(y + half_e_radius)),
		  WINSCALE(e_radius), WINSCALE(e_radius),
		  0, 64 * 360);
    }
    if (scolor != -1) {
	    SET_FG(colors[scolor].pixel);
	    rd.drawArc(dpy, p_draw, gc, 
		       WINSCALE(X(x - half_radius)), 
		       WINSCALE(Y(y + half_radius)),
		       WINSCALE(radius), WINSCALE(radius),
		       0, 64 * 360);
	    Erase_arc(WINSCALE(X(x - half_radius)),
		      WINSCALE(Y(y + half_radius)),
		      WINSCALE(radius), WINSCALE(radius),
		      0, 64 * 360);
    }
}

static void Set_drawstyle_dashed(int ship_color, int cloak);

static void Gui_paint_ship_cloaked(int ship_color, XPoint *points, int point_count)
{
    Set_drawstyle_dashed(ship_color, 1);
    rd.drawLines(dpy, p_draw, gc, points, point_count, 0);
    Erase_points(1, points, point_count);
}

static void Gui_paint_ship_phased(int ship_color, XPoint *points, int point_count)
{
    Gui_paint_ship_cloaked(ship_color, points, point_count);
}

static void generic_paint_ship(int x, int y, int ang, int ship)
{
    PaintBitmap(p_draw, ship, WINSCALE(X(x) - 16), WINSCALE(Y(y) - 16),
		WINSCALE(32), WINSCALE(32), ang);
}


static void Gui_paint_ship_uncloaked(int id, XPoint *points,
			      int ship_color, int point_count)
{
    if (gcv.line_style != LineSolid) {
	gcv.line_style = LineSolid;
	XChangeGC(dpy, gc, GCLineStyle, &gcv);
    }
    SET_FG(colors[ship_color].pixel);
    rd.drawLines(dpy, p_draw, gc, points, point_count, 0);
    Erase_points(0, points, point_count);
    if (!useErase){
	if (lock_id == id
	    && id != -1
	    && lock_dist != 0) {

	    rd.fillPolygon(dpy, p_draw, gc,
			   points, point_count,
			   Complex, CoordModeOrigin);
	}
    }

}


static void Set_drawstyle_dashed(int ship_color, int cloak)
{
    int mask;
    if (gcv.line_style != LineOnOffDash) {
	gcv.line_style = LineOnOffDash;
	mask = GCLineStyle;
#ifndef NO_ROTATING_DASHES
	mask |= GCDashOffset;
#endif
	XChangeGC(dpy, gc, mask, &gcv);
    }
#if !defined(_WINDOWS) || defined(PENS_OF_PLENTY)
    SET_FG(colors[ship_color].pixel);
#else
    SET_FG(colors[ship_color].pixel+(cloak ? CLOAKCOLOROFS : 0));
#endif
}


static int set_shipshape(int world_x, int world_y,
		  int dir, shipobj *ship, XPoint *points)
{
    int			cnt;
    position		*ship_point_pos;
    XPoint		*xpts = points;
    int			window_x;
    int			window_y;

    for (cnt = 0; cnt < ship->num_points; cnt++) {
	ship_point_pos = &(ship->pts[cnt][dir]);
	window_x = X(world_x + ship_point_pos->x);
	window_y = Y(world_y + ship_point_pos->y);
	xpts->x = WINSCALE(window_x);
	xpts->y = WINSCALE(window_y);
	xpts++;
    }
    points[cnt++] = points[0];

    return cnt;
}


void Gui_paint_ship(int x, int y, int dir, int id, int cloak, int phased,
		    int shield, int deflector, int eshield)
{
    int			cnt, ship_color;
    other_t		*other;
    shipobj		*ship;
    XPoint		points[64];
    int			ship_shape;

    ship = Ship_by_id(id);
    other = Other_by_id(id);


    ship_color = WHITE;
    cnt = set_shipshape(x, y, dir, ship, points);


    /*
     * Determine if the name of the player should be drawn below
     * his/her ship.
     */
    if (BIT(instruments, SHOW_SHIP_NAME)
	&& self != NULL
	&& self->id != id
	&& other != NULL) {

	Gui_paint_ship_name(x , y, other);
    }

    if (roundDelay > 0 && roundDelay % FPS < FPS/2) {
	Gui_paint_rounddelay(x, y);
	return;
    }

    ship_color = Gui_calculate_ship_color(id, other);

    if (cloak == 0 && phased == 0) {
	if (!blockBitmaps || !blockBitmapShips) {
	    Gui_paint_ship_uncloaked(id, points, ship_color, cnt);
	}
	else {
	    if (ship_color == BLUE)
		ship_shape = BM_SHIP_FRIEND;
	    else if (self != NULL && self->id != id) 
		ship_shape = BM_SHIP_ENEMY;
	    else 
		ship_shape = BM_SHIP_SELF;

	    generic_paint_ship(x, y, dir, ship_shape);
	}

    }

    if (phased) {
	Gui_paint_ship_phased(ship_color, points, cnt);
    } else if (cloak) {
	Gui_paint_ship_cloaked(ship_color, points, cnt);
    }
    if (markingLights) {
        Gui_paint_marking_lights(id, x, y, ship, dir);
    }
    if (shield || deflector) {
        Set_drawstyle_dashed(ship_color, cloak);
	Gui_paint_shields_deflectors(x, y, ship->shield_radius, 
				    shield, deflector, 
				    eshield, ship_color);
    }	
}


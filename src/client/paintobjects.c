/* $Id: paintobjects.c,v 4.11 1998/09/09 00:22:16 dick Exp $
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
#include "NT/winClient.h"
#else
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>

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
#include "portability.h"

char paintobjects_version[] = VERSION;

#define X(co)  ((int) ((co) - world.x))
#define Y(co)  ((int) (world.y + view_height - (co)))

extern setup_t		*Setup;

u_byte	debris_colors;		/* Number of debris intensities from server */
bool	markingLights;
char	*ballTextureFile;	/* Filename of ball texture */


#define NUM_WRECKAGE_SHAPES 3
#define NUM_WRECKAGE_POINTS 12

static int wreckageRawShapes[NUM_WRECKAGE_SHAPES][NUM_WRECKAGE_POINTS][2] = {
    { {-9, 6}, {-2, 8}, { 5, 2}, { 9, 3}, {10, 0}, { 5,-1},
      { 3, 0}, {-2,-9}, {-5,-6}, {-3,-2}, {-7,-1}, {-5, 2} },
    { {-8,-9}, {-9,-3}, {-7, 3}, {-1, 7}, { 8, 9}, { 9, 6},
      { 2, 5}, {-2, 2}, { 4,-1}, { 2,-5}, { 0,-2}, {-5,-2} },
    { {-9,-2}, {-7, 2}, {-2,-3}, { 2,-3}, { 0, 1}, { 1,10},
      { 4, 9}, { 4, 2}, { 7,-2}, { 7,-5}, { 2,-8}, {-4,-7} },
};

static position *wreckageShapes[NUM_WRECKAGE_SHAPES][NUM_WRECKAGE_POINTS];


extern XGCValues	gcv;

#define FIND_NAME_WIDTH(other)						\
    if ((other)->name_width == 0) {					\
	(other)->name_len = strlen((other)->name);			\
	(other)->name_width = 2 + XTextWidth(gameFont, (other)->name,	\
					 (other)->name_len);		\
    }


static int wrap(int *xp, int *yp)
{
    int			x = *xp, y = *yp;

    if (x < world.x || x > world.x + view_width) {
	if (x < realWorld.x || x > realWorld.x + view_width) {
	    return 0;
	}
	*xp += world.x - realWorld.x;
    }
    if (y < world.y || y > world.y + view_height) {
	if (y < realWorld.y || y > realWorld.y + view_height) {
	    return 0;
	}
	*yp += world.y - realWorld.y;
    }
    return 1;
}


/*db960828 added color parameter cause Windows needs to blt a different
         bitmap based on the color. Unix ignores this parameter*/
void Paint_item_symbol(u_byte type, Drawable d, GC mygc, int x, int y, int color)
{
#ifdef	_WINDOWS
    rd.paintItemSymbol(type, d, mygc, x, y, color);
#else
    gcv.stipple = itemBitmaps[type];
    gcv.fill_style = FillStippled;
    gcv.ts_x_origin = x;
    gcv.ts_y_origin = y;
    XChangeGC(dpy, mygc,
	      GCStipple|GCFillStyle|GCTileStipXOrigin|GCTileStipYOrigin,
	      &gcv);
    rd.paintItemSymbol(type, d, mygc, x, y, color);
    XFillRectangle(dpy, d, mygc, x, y, ITEM_SIZE, ITEM_SIZE);
    gcv.fill_style = FillSolid;
    XChangeGC(dpy, mygc, GCFillStyle, &gcv);
#endif
}


void Paint_item(u_byte type, Drawable d, GC mygc, int x, int y)
{
    const int		SIZE = ITEM_TRIANGLE_SIZE;
    XPoint		points[5];

#ifndef NO_ITEM_TRIANGLES
    points[0].x = x - SIZE;
    points[0].y = y - SIZE;
    points[1].x = x;
    points[1].y = y + SIZE;
    points[2].x = x + SIZE;
    points[2].y = y - SIZE;
    points[3] = points[0];
    SET_FG(colors[BLUE].pixel);
    rd.drawLines(dpy, d, mygc, points, 4, CoordModeOrigin);
#endif

    SET_FG(colors[RED].pixel);
#if 0
    str[0] = itemtype_ptr[i].type + '0';
    str[1] = '\0';
    rd.drawString(dpy, d, mygc,
		x - XTextWidth(gameFont, str, 1)/2,
		y + SIZE - 1,
		str, 1);
#endif
    Paint_item_symbol(type, d, mygc, 
		x - ITEM_SIZE/2, 
		y - SIZE + 2, ITEM_PLAYFIELD);
}


void Paint_shots(void)
{
    int		color, i, j, t_, id, x, y, xs, ys, x1, x2, y1, y2, len, dir;
    int		x_areas, y_areas, areas, max_;

    if (num_itemtype > 0) {
	SET_FG(colors[RED].pixel);
	for (i = 0; i < num_itemtype; i++) {
	    x = itemtype_ptr[i].x;
	    y = itemtype_ptr[i].y;
	    if (wrap(&x, &y)) {
		Paint_item((u_byte)itemtype_ptr[i].type, p_draw, gc, 
			WINSCALE(X(x)), WINSCALE(Y(y)));
		Erase_rectangle(WINSCALE(X(x)) - ITEM_TRIANGLE_SIZE,
				WINSCALE(Y(y)) - ITEM_TRIANGLE_SIZE,
				2*ITEM_TRIANGLE_SIZE + 1,
				2*ITEM_TRIANGLE_SIZE + 1);
	    }
	}
	RELEASE(itemtype_ptr, num_itemtype, max_itemtype);
    }

    if (num_ball > 0) {
	static Pixmap		ballTile = None;

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
	else
		ballTile = None;

	for (i = 0; i < num_ball; i++) {
	    x = ball_ptr[i].x;
	    y = ball_ptr[i].y;
	    id = ball_ptr[i].id;
	    if (wrap(&x, &y)) {
		x = X(x);
		y = Y(y);
		if (ballTile != None) {
		    XSetTSOrigin(dpy, gc, WINSCALE(x - BALL_RADIUS), WINSCALE(y - BALL_RADIUS));
		    rd.fillArc(dpy, p_draw, gc,
			WINSCALE(x - BALL_RADIUS), WINSCALE(y - BALL_RADIUS),
			     2*BALL_RADIUS, 2*BALL_RADIUS, 0, 64*360);
		    Erase_rectangle(WINSCALE(x - BALL_RADIUS),
				    WINSCALE(y - BALL_RADIUS),
				    2*BALL_RADIUS,
				    2*BALL_RADIUS);
		}
		else {
		    Arc_add(WHITE, x - BALL_RADIUS, y - BALL_RADIUS,
			    2*BALL_RADIUS, 2*BALL_RADIUS, 0, 64*360);
		}
		if (ball_ptr[i].id == -1) {
		    continue;
		}
		for (j = 0; j < num_ship; j++) {
		    if (ship_ptr[j].id == id) {
			break;
		    }
		}
		if (j >= num_ship) {
		    continue;
		}
		xs = ship_ptr[j].x;
		ys = ship_ptr[j].y;
		if (wrap(&xs, &ys)) {
		    xs = X(xs);
		    ys = Y(ys);
		    Segment_add(WHITE, x, y, xs, ys);
		}
	    }
	}
	if (ballTile != None) {
	    XSetFillStyle(dpy, gc, FillSolid);
	}
	RELEASE(ball_ptr, num_ball, max_ball);
    }

    if (num_mine > 0) {
	static XPoint mine_points[21] = {
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

	for (i = 0; i < num_mine; i++) {
	    x = mine_ptr[i].x;
	    y = mine_ptr[i].y;
	    if (wrap(&x, &y)) {
		x = X(x);
		y = Y(y);
		mine_points[0].x = WINSCALE(x - 8);
		mine_points[0].y = WINSCALE(y - 1);
		if (mine_ptr[i].teammine == 0) {
			SET_FG(colors[BLUE].pixel);
			rd.fillRectangle(dpy, p_draw, gc,
				WINSCALE(x - 7), WINSCALE(y - 2), 
				(15), (5));
		}
		SET_FG(colors[WHITE].pixel);
		rd.drawLines(dpy, p_draw, gc,
			   mine_points, 21, CoordModePrevious);
		Erase_rectangle( WINSCALE(x - 8) - 1, WINSCALE(y - 2) - 3, 
				(19), (9));

		/*
		 * Determine if the name of the player who is safe
		 * from the mine should be drawn.
		 * Mines unsafe to all players have the name "Expired"
		 * We do not know who is safe for mines sent with id==0
		 */
		if (BIT(instruments, SHOW_MINE_NAME) && mine_ptr[i].id!=0) {
		    other_t *other;
		    char *name;
		    int name_width, name_len;
		    if (mine_ptr[i].id == EXPIRED_MINE_ID) {
			static char expired_name[] = "Expired";
			static int expired_name_width = 0;
			static int expired_name_len = 0;
			if (expired_name_len == 0) {
			    expired_name_len = strlen(expired_name);
			    expired_name_width = XTextWidth(gameFont,
							    expired_name,
							    expired_name_len);
			}
			name = expired_name;
			name_len = expired_name_len;
			name_width = expired_name_width;
		    } else if ((other=Other_by_id(mine_ptr[i].id))!=NULL) {
			FIND_NAME_WIDTH(other);
			name = other->name;
			name_len = other->name_len;
			name_width = other->name_width;
		    } else {
			static char unknown_name[] = "Not of this world!";
			static int unknown_name_width = 0;
			static int unknown_name_len = 0;
			if (unknown_name_len == 0) {
			    unknown_name_len = strlen(unknown_name);
			    unknown_name_width = XTextWidth(gameFont,
							    unknown_name,
							    unknown_name_len);
			}
			name = unknown_name;
			name_len = unknown_name_len;
			name_width = unknown_name_width;
		    }
		    if (name!=NULL) {
			rd.drawString(dpy, p_draw, gc,
				    WINSCALE(x - name_width / 2),
				    WINSCALE(y) + gameFont->ascent + 4,
				    name, name_len);
			Erase_rectangle(WINSCALE(x - name_width / 2) - 1,
					WINSCALE(y) + 4 - 3,
					name_width + 2,
					gameFont->ascent + gameFont->descent
					 + 6);
		    }
		}
	    }
	}
	RELEASE(mine_ptr, num_mine, max_mine);
    }

    x_areas = (view_width + 255) >> 8;
    y_areas = (view_height + 255) >> 8;
    areas = x_areas * y_areas;
    max_ = areas * (debris_colors >= 3 ? debris_colors : 4);

#define BASE_X(i)	((i % x_areas) << 8)
#define BASE_Y(i)	(view_height - 1 - (((i / x_areas) % y_areas) << 8))
#define COLOR(i)	(i / areas)
#if 0
/* before "sparkColors" option: */
#define DEBRIS_COLOR(color) \
	((debris_colors > 4) ?				\
	 (5 + (((color & 1) << 2) | (color >> 1))) :	\
	 ((debris_colors >= 3) ?			\
	  (5 + color) : (color)))
#else
/* adjusted for "sparkColors" option: */
#define DEBRIS_COLOR(color) \
	((debris_colors > 4) ?				\
	 ((((color & 1) << 2) | (color >> 1))) :	\
	  (color))
#endif

    for (i = 0; i < max_; i++) {
	if (num_debris[i] > 0) {
	    x = BASE_X(i);
	    y = BASE_Y(i);
	    color = COLOR(i);
	    color = DEBRIS_COLOR(color);
	    color = spark_color[color];
	    for (j = 0; j < num_debris[i]; j++) {
		Rectangle_add(color,
			      x + debris_ptr[i][j].x - spark_size/2,
			      y - debris_ptr[i][j].y - spark_size/2,
			      spark_size, spark_size);
	    }
	    RELEASE(debris_ptr[i], num_debris[i], max_debris[i]);
	}
    }

    if ( num_wreckage > 0 ) {
	int wtype, size, cnt, rot, tx, ty;
	XPoint points[NUM_WRECKAGE_POINTS+2];

	for  (i = 0; i < num_wreckage; i++) {
	    color = WHITE;

	    x = wreckage_ptr[i].x;
	    y = wreckage_ptr[i].y;
	    if (wrap(&x, &y)) {
		wtype = wreckage_ptr[i].wrecktype % NUM_WRECKAGE_SHAPES;
		rot = wreckage_ptr[i].rotation;
		size = wreckage_ptr[i].size;

		for (cnt = 0; cnt < NUM_WRECKAGE_POINTS; cnt++) {
		    tx = (int)wreckageShapes[wtype][cnt][rot].x;
		    ty = (int)wreckageShapes[wtype][cnt][rot].y;
		    
		    tx = tx * size / 256;
		    ty = ty * size / 256;

		    points[cnt].x = WINSCALE(X(x + tx));
		    points[cnt].y = WINSCALE(Y(y + ty));

		}
		points[cnt++] = points[0];

		SET_FG(colors[color].pixel);
		rd.drawLines(dpy, p_draw, gc, points, cnt, 0);
		Erase_points(0, points, cnt);
	    }
	
	}
	RELEASE(wreckage_ptr, num_wreckage, max_wreckage);
    }

    /*
     * Draw fastshots
     */
    for (i = 0; i < max_; i++) {
	t_ = i + DEBRIS_TYPES;

	if (num_fastshot[i] > 0) {
	    int z = shot_size/2;

		x = BASE_X(i);
	    y = BASE_Y(i);
	    color = COLOR(i);
	    if (color != WHITE && color != BLUE) {
		color = WHITE;
	    }
	    for (j = 0; j < num_fastshot[i]; j++) {
		if (showNastyShots) {
			if (rfrac() < 0.5f) {
				Segment_add(color,
					x + fastshot_ptr[i][j].x - z,
					y - fastshot_ptr[i][j].y - z,
					x + fastshot_ptr[i][j].x + z,
					y - fastshot_ptr[i][j].y + z);
				Segment_add(color,
					x + fastshot_ptr[i][j].x + z,
					y - fastshot_ptr[i][j].y - z,
					x + fastshot_ptr[i][j].x - z,
					y - fastshot_ptr[i][j].y + z);
			} else {
				Segment_add(color,
					x + fastshot_ptr[i][j].x - z,
					y - fastshot_ptr[i][j].y,
					x + fastshot_ptr[i][j].x + z,
					y - fastshot_ptr[i][j].y);
				Segment_add(color,
					x + fastshot_ptr[i][j].x,
					y - fastshot_ptr[i][j].y - z,
					x + fastshot_ptr[i][j].x,
					y - fastshot_ptr[i][j].y + z);
			}
		} else {
			Rectangle_add(color,
						  x + fastshot_ptr[i][j].x - z,
						  y - fastshot_ptr[i][j].y - z,
						  shot_size, shot_size);
		}
	    }
	    RELEASE(fastshot_ptr[i], num_fastshot[i], max_fastshot[i]);
	}

	/*
	 * Teamshots are in range DEBRIS_TYPES to DEBRIS_TYPES*2-1 in fastshot.
	 */
	/* IFWINDOWS( Trace("t_=%d\n", t_); )*/
	if (num_fastshot[t_] > 0) {
	    int z = teamshot_size/2;

	    x = BASE_X(i);
	    y = BASE_Y(i);
	    color = COLOR(i);
	    for (j = 0; j < num_fastshot[t_]; j++) {
		if (rfrac() < 0.5f) {
		    Segment_add(color,
				x + fastshot_ptr[t_][j].x - z,
				y - fastshot_ptr[t_][j].y - z,
				x + fastshot_ptr[t_][j].x + z,
				y - fastshot_ptr[t_][j].y + z);
		    Segment_add(color,
				x + fastshot_ptr[t_][j].x + z,
				y - fastshot_ptr[t_][j].y - z,
				x + fastshot_ptr[t_][j].x - z,
				y - fastshot_ptr[t_][j].y + z);
		} else {
		    Segment_add(color,
				x + fastshot_ptr[t_][j].x - z,
				y - fastshot_ptr[t_][j].y,
				x + fastshot_ptr[t_][j].x + z,
				y - fastshot_ptr[t_][j].y);
		    Segment_add(color,
				x + fastshot_ptr[t_][j].x,
				y - fastshot_ptr[t_][j].y - z,
				x + fastshot_ptr[t_][j].x,
				y - fastshot_ptr[t_][j].y + z);
		}
	    }
	    RELEASE(fastshot_ptr[t_], num_fastshot[t_], max_fastshot[t_]);
	}
    }

    if (num_missile > 0) {
	int len;
#if defined(_WINDOWS) && !defined(PENS_OF_PLENTY)
	SET_FG(MISSILECOLOR);
#else
	SET_FG(colors[WHITE].pixel);
#endif
	XSetLineAttributes(dpy, gc, 4,
			   LineSolid, CapButt, JoinMiter);
	for (i = 0; i < num_missile; i++) {
	    x = missile_ptr[i].x;
	    y = missile_ptr[i].y;
	    len = MISSILE_LEN;
	    if (missile_ptr[i].len > 0) {
		len = missile_ptr[i].len;
	    }
	    if (wrap(&x, &y)) {
		x1 = X(x);
		y1 = Y(y);
		x2 = (int)(x1 - tcos(missile_ptr[i].dir) * len);
		y2 = (int)(y1 + tsin(missile_ptr[i].dir) * len);
		rd.drawLine(dpy, p_draw, gc, 
			WINSCALE(x1), WINSCALE(y1), WINSCALE(x2), WINSCALE(y2));
		Erase_segment(4, WINSCALE(x1) , WINSCALE(y1),
			      WINSCALE(x2) , WINSCALE(y2));
	    }
	}
	XSetLineAttributes(dpy, gc, 0,
			   LineSolid, CapButt, JoinMiter);
	RELEASE(missile_ptr, num_missile, max_missile);
    }

    if (num_laser > 0) {
	XSetLineAttributes(dpy, gc, 3,
			   LineSolid, CapButt, JoinMiter);
	for (i = 0; i < num_laser; i++) {
	    x1 = laser_ptr[i].x;
	    y1 = laser_ptr[i].y;
	    len = laser_ptr[i].len;
	    dir = laser_ptr[i].dir;
	    if (wrap(&x1, &y1)) {
		x2 = (int)(x1 + len * tcos(dir));
		y2 = (int)(y1 + len * tsin(dir));
		if ((unsigned)(color = laser_ptr[i].color) >= NUM_COLORS) {
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
	}
	XSetLineAttributes(dpy, gc, 0,
			   LineSolid, CapButt, JoinMiter);
	RELEASE(laser_ptr, num_laser, max_laser);
    }
}

void Paint_ships(void)
{
    int			i, x, y, x0, y0, x1, y1;
    int			cnt, dir, size, lcnt, ship_color;
    unsigned long	mask;
    other_t		*other;
    static int		pauseCharWidth = -1;
    wireobj		*ship;
    XPoint		points[64];

    gcv.dash_offset = DASHES_LENGTH - (loops % DASHES_LENGTH);
    if (num_paused > 0) {
	const int half_pause_size = 3*BLOCK_SZ/7;

	if (pauseCharWidth < 0) {
	    pauseCharWidth = XTextWidth(gameFont, "P", 1);
	}

	for (i = 0; i < num_paused; i++) {
	    x = paused_ptr[i].x;
	    y = paused_ptr[i].y;
	    if (wrap(&x, &y)) {
		SET_FG(colors[BLUE].pixel);
		x0 = X(x - half_pause_size);
		y0 = Y(y + half_pause_size);
		rd.fillRectangle(dpy, p_draw, gc,
			       WINSCALE(x0), WINSCALE(y0),
			       WINSCALE(2*half_pause_size+1), WINSCALE(2*half_pause_size+1));
		if (paused_ptr[i].count <= 0 || loops % 10 >= 5) {
		    SET_FG(colors[mono?BLACK:WHITE].pixel);
		    rd.drawRectangle(dpy, p_draw, gc,
				   WINSCALE(x0 - 1),
				   WINSCALE(y0 - 1),
				   WINSCALE(2*(half_pause_size+1)),
				   WINSCALE(2*(half_pause_size+1)));
		    rd.drawString(dpy, p_draw, gc,
				WINSCALE(X(x - pauseCharWidth/2)),
				WINSCALE(Y(y - gameFont->ascent/2)),
				"P", 1);
		}
		Erase_rectangle(WINSCALE(x0 - 1) - 1, WINSCALE(y0 - 1) - 1,
				WINSCALE(2*half_pause_size+1)+3,
				WINSCALE(2*half_pause_size+1)+3);
	    }
	}
	RELEASE(paused_ptr, num_paused, max_paused);
    }

    if (num_ecm > 0) {
	for (i = 0; i < num_ecm; i++) {
	    if ((size = ecm_ptr[i].size) > 0) {
		x = ecm_ptr[i].x;
		y = ecm_ptr[i].y;
		if (wrap(&x, &y)) {
		    Arc_add(WHITE,
			    X(x - size / 2),
			    Y(y + size / 2),
			    size, size, 0, 64 * 360);
		}
	    }
	}
	RELEASE(ecm_ptr, num_ecm, max_ecm);
    }

    if (num_ship > 0) {
	for (i = 0; i < num_ship; i++) {
	    x = ship_ptr[i].x;
	    y = ship_ptr[i].y;
	    if (!wrap(&x, &y)) {
		continue;
	    }

	    dir = ship_ptr[i].dir;
	    ship = Ship_by_id(ship_ptr[i].id);
	    for (cnt = 0; cnt < ship->num_points; cnt++) {
		points[cnt].x = WINSCALE(X(x + ship->pts[cnt][dir].x));
		points[cnt].y = WINSCALE(Y(y + ship->pts[cnt][dir].y));
	    }
	    points[cnt++] = points[0];

	    /*
	     * Determine if the name of the player should be drawn below
	     * his/her ship.
	     */
	    if (BIT(instruments, SHOW_SHIP_NAME)
		&& self != NULL
		&& self->id != ship_ptr[i].id
		&& (other = Other_by_id(ship_ptr[i].id)) != NULL) {
		FIND_NAME_WIDTH(other);
		SET_FG(colors[WHITE].pixel);
		rd.drawString(dpy, p_draw, gc,
			    WINSCALE(X(x - other->name_width / 2)),
			    WINSCALE(Y(y - gameFont->ascent - 15)),
			    other->name, other->name_len);
		Erase_rectangle(WINSCALE(X(x - other->name_width / 2)) - 1,
				WINSCALE(Y(y - gameFont->ascent - 15))
				 - gameFont->ascent, other->name_width + 4,
				gameFont->ascent + gameFont->descent + 5);
	    }

	    ship_color = WHITE;
	    if (roundDelay > 0 && roundDelay % FPS < FPS/2) {
		char s[12];
		int	 t, text_width;

		sprintf(s, "%d", roundDelay / FPS);
		t = strlen(s);
		SET_FG(colors[WHITE].pixel);
		text_width = XTextWidth(gameFont, s, t);
		rd.drawString(dpy, p_draw, gc,
			    WINSCALE(X(x - text_width / 2)),
			    WINSCALE(Y(y - gameFont->ascent/2)),
			    s, t);
	    }
	    else {
		if (useErase){
		    /*
		     * Outline the locked ship in a different color,
		     * instead of mucking around with polygons.
		     */
		    if (lock_id == ship_ptr[i].id
			&& ship_ptr[i].id != -1
					    && lock_dist != 0) {
					    ship_color = RED;
		    }
		}
#ifndef NO_BLUE_TEAM
		if (BIT(Setup->mode, TEAM_PLAY)
		    && self != NULL
		    && self->id != ship_ptr[i].id
		    && (other = Other_by_id(ship_ptr[i].id)) != NULL
		    && self->team == other->team) {
			ship_color = BLUE;
		}
#endif
		if (roundDelay > 0 && ship_color == WHITE) {
		    ship_color = RED;
		}

		if (ship_ptr[i].cloak == 0) {
		    if (gcv.line_style != LineSolid) {
			gcv.line_style = LineSolid;
			XChangeGC(dpy, gc, GCLineStyle, &gcv);
		    }
		    SET_FG(colors[ship_color].pixel);
		    rd.drawLines(dpy, p_draw, gc, points, cnt, 0);
		    Erase_points(0, points, cnt);
		    if (!useErase){
			if (lock_id == ship_ptr[i].id
			    && ship_ptr[i].id != -1
			    && lock_dist != 0) {
				rd.fillPolygon(dpy, p_draw, gc,
				    points, cnt,
				    Complex, CoordModeOrigin);
			}
		    }

		    if (markingLights) {
			if (((loops + ship_ptr[i].id) & 0xF) == 0) {
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
			} else if (((loops + ship_ptr[i].id) & 0xF) == 2) {
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
		}

		if (ship_ptr[i].shield || ship_ptr[i].cloak || ship_ptr[i].deflector) {
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
		    SET_FG(colors[ship_color].pixel+(ship_ptr[i].cloak ? CLOAKCOLOROFS : 0));
#endif
		    if (ship_ptr[i].cloak) {
			if (useErase){
			    int j;
			    for (j = 0; j < cnt - 1; j++) {
				rd.drawLine(dpy, p_draw, gc,
				      points[j].x, points[j].y,
				      points[j + 1].x, points[j + 1].y);
			    }
			    Erase_points(1, points, cnt);
			} else {
			    rd.drawLines(dpy, p_draw, gc, points, cnt, 0);
			}
		    }

		    /* draw deflectors and shields */
		    {
			int radius = ship->shield_radius;
			int e_radius = radius + 4;
			int half_radius = radius >> 1;
			int half_e_radius = e_radius >> 1;
			int scolor = -1;
			int ecolor = -1;

			IFWINDOWS(Trace("shield=%d deflector=%d eshield=%d\n",
			    ship_ptr[i].shield, ship_ptr[i].deflector, ship_ptr[i].eshield);)
			if (ship_ptr[i].shield) 
			    scolor = ship_color;
			if (ship_ptr[i].deflector)
			    ecolor = loops & 0x02 ? RED : BLUE;
			if (ship_ptr[i].eshield && ship_ptr[i].shield) {
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
		    }	/* draw deflectors and shields */
		}	/* .shield .cloak .deflector */
	    } /* !roundDelay */
	}
	RELEASE(ship_ptr, num_ship, max_ship);
    }

    if (num_refuel > 0 || num_connector > 0 || num_trans > 0) {
	SET_FG(colors[WHITE].pixel);
	if (gcv.line_style != LineOnOffDash) {
	    gcv.line_style = LineOnOffDash;
	    mask = GCLineStyle;
#ifndef NO_ROTATING_DASHES
	    mask |= GCDashOffset;
#endif
	    XChangeGC(dpy, gc, mask, &gcv);
	}
	if (num_refuel > 0) {
#ifdef _WINDOWS
	    SET_FG(colors[WHITE].pixel+CLOAKCOLOROFS);	/* dashed line */
#endif
	    for (i = 0; i < num_refuel; i++) {
		x0 = refuel_ptr[i].x0;
		y0 = refuel_ptr[i].y0;
		x1 = refuel_ptr[i].x1;
		y1 = refuel_ptr[i].y1;
		if (wrap(&x0, &y0)
		    && wrap(&x1, &y1)) {
		    rd.drawLine(dpy, p_draw, gc,
			      WINSCALE(X(x0)), WINSCALE(Y(y0)),
			      WINSCALE(X(x1)), WINSCALE(Y(y1)));
		    Erase_segment(1, WINSCALE(X(x0)), WINSCALE(Y(y0)),
				  WINSCALE(X(x1)), WINSCALE(Y(y1)));
		}
	    }
	    RELEASE(refuel_ptr, num_refuel, max_refuel);
	}
	if (num_connector > 0) {
	    int	cdashing = 0;

	    for (i = 0; i < num_connector; i++) {
		x0 = connector_ptr[i].x0;
		y0 = connector_ptr[i].y0;
		x1 = connector_ptr[i].x1;
		y1 = connector_ptr[i].y1;
#ifdef _WINDOWS
		SET_FG(colors[WHITE].pixel+CLOAKCOLOROFS);	/* dashed line */
#endif
		if (connector_ptr[i].tractor) {
		    if (!cdashing) {
#ifdef _WINDOWS
			SET_FG(colors[WHITE].pixel);
#endif
			rd.setDashes(dpy, gc, 0, cdashes, NUM_CDASHES);
			cdashing = 1;
		    }
		} else {
		    if (cdashing) {
#ifdef _WINDOWS
			SET_FG(colors[WHITE].pixel+CLOAKCOLOROFS);	/* dashed line */
#endif
			rd.setDashes(dpy, gc, 0, dashes, NUM_DASHES);
			cdashing = 0;
		    }
		}
		if (wrap(&x0, &y0)
		    && wrap(&x1, &y1)) {
		    rd.drawLine(dpy, p_draw, gc,
			      WINSCALE(X(x0)), WINSCALE(Y(y0)),
			      WINSCALE(X(x1)), WINSCALE(Y(y1)));
		    Erase_segment(1, WINSCALE(X(x0)), WINSCALE(Y(y0)),
				  WINSCALE(X(x1)), WINSCALE(Y(y1)));
		}
	    }
	    RELEASE(connector_ptr, num_connector, max_connector);
	    if (cdashing)
		rd.setDashes(dpy, gc, 0, dashes, NUM_DASHES);
	}
	if (num_trans > 0) {
#ifdef _WINDOWS
	    SET_FG(colors[WHITE].pixel+CLOAKCOLOROFS);	/* dashed line */
#endif
	    for (i = 0; i < num_trans; i++) {
		x0 = trans_ptr[i].x1;
		y0 = trans_ptr[i].y1;
		x1 = trans_ptr[i].x2;
		y1 = trans_ptr[i].y2;
		if (wrap(&x0, &y0) && wrap(&x1, &y1)) {
		    rd.drawLine(dpy, p_draw, gc,
			      WINSCALE(X(x0)), WINSCALE(Y(y0)), 
				  WINSCALE(X(x1)), WINSCALE(Y(y1)));
		    Erase_segment(1, WINSCALE(X(x0)), WINSCALE(Y(y0)),
				  WINSCALE(X(x1)), WINSCALE(Y(y1)));
		}
	    }
	    RELEASE(trans_ptr, num_trans, max_trans);
	}
    }

    if (gcv.line_style != LineSolid) {
	gcv.line_style = LineSolid;
	mask = GCLineStyle;
	XChangeGC(dpy, gc, mask, &gcv);
    }
    gcv.dash_offset = 0;
}

int Init_wreckage()
{
    int cnt, i, ms;
    int cleanup = 0;
    
    ms = sizeof(position) * RES;

    for ( cnt = 0; cnt < NUM_WRECKAGE_SHAPES; cnt++ ) {

	/*
	 * Try to allocate memory for precalculated wreckage shapes
	 */
	for ( i = 0; i < NUM_WRECKAGE_POINTS; i++ ) {
	    if ( !( wreckageShapes[cnt][i] = (position *) malloc( ms )) ) {
		break;
	    }
	}
	if ( i < NUM_WRECKAGE_POINTS ) {
	    error("Not enough memory for ship shape");
	    while ( i-- >= 0 )
		free( wreckageShapes[cnt][i] );
	    cleanup = 1;
	    break;
	}

	/*
	 * Rotate all points in wreckage-shape
	 */
	for ( i = 0; i < NUM_WRECKAGE_POINTS; i++ ) {
	    extern void Rotate_point(position pt[RES]);

	    wreckageShapes[cnt][i][0].x = wreckageRawShapes[cnt][i][0];
	    wreckageShapes[cnt][i][0].y = wreckageRawShapes[cnt][i][1];
	    Rotate_point( &wreckageShapes[cnt][i][0] );
	}
    }

    if ( cnt < NUM_WRECKAGE_SHAPES ) {
	while ( cnt-- >= 0 ) {
	    for ( i = 0; i < NUM_WRECKAGE_POINTS; i++ ) {
		free( wreckageShapes[cnt][i] );
	    }
	}
	return -1;
    }

    return 0;
}

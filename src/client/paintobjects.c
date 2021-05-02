/* $Id: paintobjects.c,v 4.27 2000/03/12 11:46:24 bert Exp $
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
#include "guiobjects.h"
#include "gfx3d.h"

char paintobjects_version[] = VERSION;

#define X(co)  ((int) ((co) - world.x))
#define Y(co)  ((int) (world.y + view_height - (co)))

#define COLOR(i)	(i / areas)
#define BASE_X(i)	(((i % x_areas) << 8) + view_x_offset)
#define BASE_Y(i)	((view_height - 1 - (((i / x_areas) % y_areas) << 8)) - view_y_offset)


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


position *wreckageShapes[NUM_WRECKAGE_SHAPES][NUM_WRECKAGE_POINTS];


u_byte	debris_colors;		/* Number of debris intensities from server */
bool	markingLights;
char	*ballTextureFile;	/* Filename of ball texture */


extern XGCValues	gcv;
extern setup_t		*Setup;


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


static void Paint_items(void)
{
    int		i, x, y;

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
}


static void Paint_balls(void)
{
    int		i, j, id, x, y, xs, ys;

    if (num_ball > 0) {

	for (i = 0; i < num_ball; i++) {
	    x = ball_ptr[i].x;
	    y = ball_ptr[i].y;
	    id = ball_ptr[i].id;

	    if (wrap(&x, &y)) {
		Gui_paint_ball(x, y);

		if (id == -1) {
		    continue;
		}

		for (j = 0; j < num_ship && ship_ptr[j].id != id; j++) {
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
		    Gui_paint_ball_connecter(x, y, xs, ys);
		}
	    }
	}
	RELEASE(ball_ptr, num_ball, max_ball);
    }
}


static void Paint_mines(void)
{
    int		i, x, y;
    char	*name = NULL;

    if (num_mine > 0) {

	for (i = 0; i < num_mine; i++) {
	    x = mine_ptr[i].x;
	    y = mine_ptr[i].y;
	    
	    if (wrap(&x, &y)) {

		/*
		 * Determine if the name of the player who is safe
		 * from the mine should be drawn.
		 * Mines unsafe to all players have the name "Expired"
		 * We do not know who is safe for mines sent with id==0
		 */
		if (BIT(instruments, SHOW_MINE_NAME) && mine_ptr[i].id!=0) {
		    other_t *other;
		    if (mine_ptr[i].id == EXPIRED_MINE_ID) {
			static char expired_name[] = "Expired";
			name = expired_name;
		    } else if ((other=Other_by_id(mine_ptr[i].id))!=NULL) {
			name = other->name;
		    } else {
			static char unknown_name[] = "Not of this world!";
			name = unknown_name;
		    }
		}
		Gui_paint_mine(x, y, mine_ptr[i].teammine, name);
	    }
	}
	RELEASE(mine_ptr, num_mine, max_mine);
    }
}


static void Paint_debris(int x_areas, int y_areas, int areas, int max_)
{
    int		color, i, j,  x, y;

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
	    for (j = 0; j < num_debris[i]; j++) {
		Gui_paint_spark(color, x + debris_ptr[i][j].x , y - debris_ptr[i][j].y);
	    }
	    RELEASE(debris_ptr[i], num_debris[i], max_debris[i]);
	}
    }
}


static void Paint_wreckages()
{
    int		i, x, y;
    int		wtype, size, rot;
    bool	deadly;

    if ( num_wreckage > 0 ) {

	for  (i = 0; i < num_wreckage; i++) {
	    x = wreckage_ptr[i].x;
	    y = wreckage_ptr[i].y;
	    if (wrap(&x, &y)) {
		deadly = (wreckage_ptr[i].wrecktype & 0x80);

		wtype = (wreckage_ptr[i].wrecktype & 0x7F) % NUM_WRECKAGE_SHAPES;
		rot = wreckage_ptr[i].rotation;
		size = wreckage_ptr[i].size;

		Gui_paint_wreck(x, y, deadly, wtype, rot, size);
	    }
	
	}
	RELEASE(wreckage_ptr, num_wreckage, max_wreckage);
    }
}
		    

static void Paint_missiles()
{
    int		i, x, y;
    int		len, dir;

    if (num_missile > 0) {
	Gui_paint_missiles_begin();

	for (i = 0; i < num_missile; i++) {
	    x = missile_ptr[i].x;
	    y = missile_ptr[i].y;
	    dir = missile_ptr[i].dir;
	    len = MISSILE_LEN;
	    if (missile_ptr[i].len > 0) {
		len = missile_ptr[i].len;
	    }

	    if (wrap(&x, &y)) {
		Gui_paint_missile(x, y, len, dir);
	    }
	}
	Gui_paint_missiles_end();
	RELEASE(missile_ptr, num_missile, max_missile);
    }
}


static void Paint_lasers(void)
{
    int		color, i, x1, y1, len, dir;

    if (num_laser > 0) {

	Gui_paint_lasers_begin();

	for (i = 0; i < num_laser; i++) {
	    x1 = laser_ptr[i].x;
	    y1 = laser_ptr[i].y;
	    len = laser_ptr[i].len;
	    dir = laser_ptr[i].dir;
	    color = laser_ptr[i].color;

	    if (wrap(&x1, &y1)) {
		Gui_paint_laser(color, x1, y1, len, dir);
	    }
	}
	Gui_paint_lasers_end();

	RELEASE(laser_ptr, num_laser, max_laser);
    }
}


static void Paint_fastshots(int i, int x_areas, int y_areas, int areas)
{
    int		x, y, j, color;

    if (num_fastshot[i] > 0) {

	x = BASE_X(i);
	y = BASE_Y(i);
	color = COLOR(i);
	if (color != WHITE && color != BLUE) {
	    color = WHITE;
	}
	for (j = 0; j < num_fastshot[i]; j++) {
	    Gui_paint_fastshot(color, x + fastshot_ptr[i][j].x , y - fastshot_ptr[i][j].y);
	}
	RELEASE(fastshot_ptr[i], num_fastshot[i], max_fastshot[i]);
    }
}


static void Paint_teamshots(int i, int t_, int x_areas, int y_areas, int areas)
{
    int		x, y, j, color;

    /*
     * Teamshots are in range DEBRIS_TYPES to DEBRIS_TYPES*2-1 in fastshot.
     */
    /* IFWINDOWS( Trace("t_=%d\n", t_); )*/
    if (num_fastshot[t_] > 0) {

	x = BASE_X(i);
	y = BASE_Y(i);
	color = COLOR(i);
	for (j = 0; j < num_fastshot[t_]; j++) {
	Gui_paint_teamshot(color, x + fastshot_ptr[t_][j].x, y - fastshot_ptr[t_][j].y);
	}
	RELEASE(fastshot_ptr[t_], num_fastshot[t_], max_fastshot[t_]);
    }
}


void Paint_shots(void)
{
    int		i, t_;
    int		x_areas, y_areas, areas, max_;

    Paint_items();
    Paint_balls();
    Paint_mines();

    x_areas = (real_view_width + 255) >> 8;
    y_areas = (real_view_height + 255) >> 8;
    areas = x_areas * y_areas;
    max_ = areas * (debris_colors >= 3 ? debris_colors : 4);

    Paint_debris(x_areas, y_areas, areas, max_);

    Paint_wreckages();

    for (i = 0; i < max_; i++) {
	t_ = i + DEBRIS_TYPES;
	Paint_fastshots(i, x_areas, y_areas, areas);
	Paint_teamshots(i, t_, x_areas, y_areas, areas);
    }

    Paint_missiles();
    Paint_lasers();
}


static void Paint_paused(void)
{
    int i, x, y;    

    if (num_paused > 0) {

	for (i = 0; i < num_paused; i++) {
	    x = paused_ptr[i].x;
	    y = paused_ptr[i].y;
	    if (wrap(&x, &y)) {
		Gui_paint_paused(x, y, paused_ptr[i].count);

	    }
	}
	RELEASE(paused_ptr, num_paused, max_paused);
    }
}


static void Paint_ecm(void)
{
    int	    i, x, y, size;

    if (num_ecm > 0) {

	for (i = 0; i < num_ecm; i++) {
	    if ((size = ecm_ptr[i].size) > 0) {
		x = ecm_ptr[i].x;
		y = ecm_ptr[i].y;
		if (wrap(&x, &y)) {
		    Gui_paint_ecm(x, y, size);
		}
	    }
	}
	RELEASE(ecm_ptr, num_ecm, max_ecm);
    }
}


static void Paint_all_ships(void)
{
    int	    i, x, y;

    if (num_ship > 0) {

	for (i = 0; i < num_ship; i++) {
	    x = ship_ptr[i].x;
	    y = ship_ptr[i].y;
	    if (!wrap(&x, &y)) {
		continue;
	    }

            /*
             * ship in the center? (svenska-hack)
             */
	    if ( abs(X(x)-view_width/2) <= 1
		&& abs(Y(y)-view_height/2) <= 1
		&& Other_by_id(ship_ptr[i].id) != NULL ) {
		  eyesId = ship_ptr[i].id;
	    }

	    Gui_paint_ship(x, y, 
			   ship_ptr[i].dir, ship_ptr[i].id,
			   ship_ptr[i].cloak, ship_ptr[i].shield,
			   ship_ptr[i].deflector, ship_ptr[i].eshield);


	}
	RELEASE(ship_ptr, num_ship, max_ship);
    }
}


static void Paint_refuel(void)
{
    int	    i, x0, y0, x1, y1;

    if (num_refuel > 0) {

	for (i = 0; i < num_refuel; i++) {
	    x0 = refuel_ptr[i].x0;
	    y0 = refuel_ptr[i].y0;
	    x1 = refuel_ptr[i].x1;
	    y1 = refuel_ptr[i].y1;
	    if (wrap(&x0, &y0) && wrap(&x1, &y1)) {
		Gui_paint_refuel(x0, y0, x1, y1);
	    }
	}
	RELEASE(refuel_ptr, num_refuel, max_refuel);
    }
}


static void Paint_connectors(void)
{
    int	    i, x0, y0, x1, y1;

    if (num_connector > 0) {

	for (i = 0; i < num_connector; i++) {
	    x0 = connector_ptr[i].x0;
	    y0 = connector_ptr[i].y0;
	    x1 = connector_ptr[i].x1;
	    y1 = connector_ptr[i].y1;
	    if (wrap(&x0, &y0) && wrap(&x1, &y1)) {
		Gui_paint_connector(x0, y0, x1, y1, connector_ptr[i].tractor);
	    }
	}
	RELEASE(connector_ptr, num_connector, max_connector);
    }
}


static void Paint_transporters(void)
{
    int	    i, x0, y0, x1, y1;

    if (num_trans > 0) {

	for (i = 0; i < num_trans; i++) {
	    x0 = trans_ptr[i].x1;
	    y0 = trans_ptr[i].y1;
	    x1 = trans_ptr[i].x2;
	    y1 = trans_ptr[i].y2;
	    if (wrap(&x0, &y0) && wrap(&x1, &y1)) {
		Gui_paint_transporter(x0, y0, x1, y1);
	    }
	}
	RELEASE(trans_ptr, num_trans, max_trans);
    }
}


static void Paint_all_connectors(void)
{

    if (num_refuel > 0 ||
	num_connector > 0 ||
	num_trans > 0) {

	Gui_paint_all_connectors_begin();
	Paint_refuel();
	Paint_connectors();
	Paint_transporters();
    }
}


void Paint_ships(void)
{
    Gui_paint_ships_begin();

    Paint_paused();
    Paint_ecm();
    Paint_all_ships();
    Paint_all_connectors();
    
    Gui_paint_ships_end();
 
}
 

int Init_wreckage()
{
    int		shp, i;
    size_t	point_size;
    size_t	total_size;
    char	*dynmem;
 
    /*
     * Allocate memory for all the wreckage points.
     */
    point_size = sizeof(position) * RES;
    total_size = point_size * NUM_WRECKAGE_POINTS * NUM_WRECKAGE_SHAPES;
    if ((dynmem = (char *) malloc(total_size)) == NULL) {
	error("Not enough memory for wreckage shapes");
	return -1;
    }

    /*
     * For each wreckage-shape rotate all points.
     */
    for ( shp = 0; shp < NUM_WRECKAGE_SHAPES; shp++ ) {
	for ( i = 0; i < NUM_WRECKAGE_POINTS; i++ ) {
	    extern void Rotate_point(position pt[RES]);

	    wreckageShapes[shp][i] = (position *) dynmem;
	    dynmem += point_size;
	    wreckageShapes[shp][i][0].x = wreckageRawShapes[shp][i][0];
	    wreckageShapes[shp][i][0].y = wreckageRawShapes[shp][i][1];
	    Rotate_point( &wreckageShapes[shp][i][0] );
	}
    }

    return 0;
}


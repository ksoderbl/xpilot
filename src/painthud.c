/* $Id: painthud.c,v 3.8 1998/01/08 19:28:49 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-97 by
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
#include "../contrib/NT/xpilot/winX.h"
#include "../contrib/NT/xpilot/winClient.h"
#include <math.h>
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

char painthud_version[] = VERSION;

#define X(co)  ((int) ((co) - world.x))
#define Y(co)  ((int) (world.y + view_height - (co)))

#define FIND_NAME_WIDTH(other)						\
    if ((other)->name_width == 0) {					\
	(other)->name_len = strlen((other)->name);			\
	(other)->name_width = 2 + XTextWidth(gameFont, (other)->name,	\
					 (other)->name_len);		\
    }

extern setup_t		*Setup;
extern int		RadarHeight;
extern score_object_t	score_objects[MAX_SCORE_OBJECTS];
extern int		score_object;
extern XGCValues	gcv;

int	hudColor;		/* Color index for HUD drawing */
int	hudLockColor;		/* Color index for lock on HUD drawing */
DFLOAT	charsPerTick = 0.0;	/* Output speed of messages */

message_t	*TalkMsg[MAX_MSGS], *GameMsg[MAX_MSGS];

/*
 * Draw a meter of some kind on screen.
 * When the x-offset is specified as a negative value then
 * the meter is drawn relative to the right side of the screen,
 * otherwise from the normal left side.
 */
static void Paint_meter(int xoff, int y, const char *title, int val, int max)
{
#define METER_WIDTH		200
#define METER_HEIGHT		8

    const int	mw1_4 = METER_WIDTH/4,
		mw2_4 = METER_WIDTH/2,
		mw3_4 = 3*METER_WIDTH/4,
		mw4_4 = METER_WIDTH,
		BORDER = 5;
    int		x, xstr;

    if (xoff >= 0) {
	x = xoff;
	xstr = x + METER_WIDTH + BORDER;
    } else {
	x = view_width - (METER_WIDTH - xoff);
	xstr = x - (BORDER + XTextWidth(gameFont, title, strlen(title)));
    }

    Rectangle_add(RED,
		  x+2, y+2,
		  (int)(((METER_WIDTH-3)*val)/(max?max:1)), WINSCALE(METER_HEIGHT-3));
    SET_FG(colors[WHITE].pixel);
    rd.drawRectangle(dpy, p_draw, gc,
		   WINSCALE(x), WINSCALE(y), WINSCALE(METER_WIDTH), WINSCALE(METER_HEIGHT));
    Erase_4point(x, y, METER_WIDTH, METER_HEIGHT);

    /* Paint scale levels(?) */
    Segment_add(WHITE, x,       y-4,	x,       y+METER_HEIGHT+4);
    Segment_add(WHITE, x+mw4_4, y-4,	x+mw4_4, y+METER_HEIGHT+4);
    Segment_add(WHITE, x+mw2_4, y-3,	x+mw2_4, y+METER_HEIGHT+3);
    Segment_add(WHITE, x+mw1_4, y-1,	x+mw1_4, y+METER_HEIGHT+1);
    Segment_add(WHITE, x+mw3_4, y-1,	x+mw3_4, y+METER_HEIGHT+1);

    rd.drawString(dpy, p_draw, gc,
		  WINSCALE(xstr), WINSCALE(y+(gameFont->ascent+METER_HEIGHT)/2),
		  title, strlen(title));
    Erase_rectangle(xstr,
		    y+(gameFont->ascent+METER_HEIGHT)/2 - gameFont->ascent,
		    XTextWidth(gameFont, title, strlen(title)) + 2,
		    gameFont->ascent + gameFont->descent);
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

void Paint_score_objects(void)
{
    int		i, x, y;

    for (i=0; i < MAX_SCORE_OBJECTS; i++) {
	score_object_t*	sobj = &score_objects[i];
	if (sobj->count > 0) {
	    if (sobj->count%3) {
		x = sobj->x * BLOCK_SZ + BLOCK_SZ/2 - sobj->msg_width/2;
		y = sobj->y * BLOCK_SZ + BLOCK_SZ/2 - gameFont->ascent/2;
		if (wrap(&x, &y)) {
		    SET_FG(colors[hudColor].pixel);
		    rd.drawString(dpy, p_draw, gc,
				WINSCALE(X(x)),
				WINSCALE(Y(y)),
				sobj->msg,
				sobj->msg_len);
		    Erase_rectangle(X(x) - 1, Y(y) - gameFont->ascent,
				    sobj->msg_width + 2,
				    gameFont->ascent + gameFont->descent);
		}
	    }
	    sobj->count++;
	    if (sobj->count > SCORE_OBJECT_COUNT) {
		sobj->count = 0;
		sobj->hud_msg_len = 0;
	    }
	}
    }
}


void Paint_meters(void)
{
    if (BIT(instruments, SHOW_FUEL_METER))
	Paint_meter(-10, 20, "Fuel", (int)fuelSum, (int)fuelMax);
    if (BIT(instruments, SHOW_POWER_METER) || control_count)
	Paint_meter(-10, 40, "Power", (int)power, (int)MAX_PLAYER_POWER);
    if (BIT(instruments, SHOW_TURNSPEED_METER) || control_count)
	Paint_meter(-10, 60, "Turnspeed",
		    (int)turnspeed, (int)MAX_PLAYER_TURNSPEED);
    if (control_count > 0)
	control_count--;
    if (BIT(instruments, SHOW_PACKET_SIZE_METER))
	Paint_meter(-10, 80, "Packet",
		   (packet_size >= 4096) ? 4096 : packet_size, 4096);
    if (BIT(instruments, SHOW_PACKET_LOSS_METER))
	Paint_meter(-10, 100, "Loss", packet_loss, FPS);
    if (BIT(instruments, SHOW_PACKET_DROP_METER))
	Paint_meter(-10, 120, "Drop", packet_drop, FPS);

    if (thrusttime >= 0 && thrusttimemax > 0)
	Paint_meter((view_width-300)/2 -32, 2*view_height/3,
		    "Thrust Left",
		    (thrusttime >= thrusttimemax ? thrusttimemax : thrusttime),
		    thrusttimemax);

    if (shieldtime >= 0 && shieldtimemax > 0)
	Paint_meter((view_width-300)/2 -32, 2*view_height/3 + 20,
		    "Shields Left",
		    (shieldtime >= shieldtimemax ? shieldtimemax : shieldtime),
		    shieldtimemax);

    if (destruct > 0)
	Paint_meter((view_width-300)/2 -32, 2*view_height/3 + 40,
		   "Self destructing", destruct, 150);

    if (shutdown_count >= 0)
	Paint_meter((view_width-300)/2 -32, 2*view_height/3 + 60,
		   "SHUTDOWN", shutdown_count, shutdown_delay);
}


static void Paint_lock(int hud_pos_x, int hud_pos_y)
{
    const int	BORDER = 2;
    int		x, y;
    int		i, dir = 96;
    int		hudShipColor = hudColor;
    other_t	*target;
    wireobj	*ship;
    char	str[50];
    static int	warningCount;
    static int	mapdiag = 0;
    XPoint	points[64];

    if (mapdiag == 0) {
	mapdiag = (int)LENGTH(Setup->x * BLOCK_SZ, Setup->y * BLOCK_SZ);
    }

    /*
     * Display direction arrow and miscellaneous target information.
     */
    if ((target = Other_by_id(lock_id)) == NULL) {
	return;
    }
    FIND_NAME_WIDTH(target);
    rd.drawString(dpy, p_draw, gc,
		WINSCALE(hud_pos_x - target->name_width / 2),
		WINSCALE(hud_pos_y - HUD_SIZE+HUD_OFFSET - gameFont->descent - BORDER),
		target->name, target->name_len);
    Erase_rectangle(hud_pos_x - target->name_width / 2 - 1,
		    hud_pos_y - HUD_SIZE+HUD_OFFSET - gameFont->descent
			- BORDER - gameFont->ascent,
		    target->name_width + 2,
		    gameFont->ascent + gameFont->descent);

    ship = Ship_by_id(lock_id);
    for (i = 0; i < ship->num_points; i++) {
	points[i].x = WINSCALE((int)(hud_pos_x + ship->pts[i][dir].x / 2 + 60));
	points[i].y = WINSCALE((int)(hud_pos_y + ship->pts[i][dir].y / 2 - 80));
    }
    points[i++] = points[0];
    SET_FG(colors[hudShipColor].pixel);
#if ERASE
    rd.drawLines(dpy, p_draw, gc, points, i, 0);
    Erase_points(0, points, i);
#else
    rd.fillPolygon(dpy, p_draw, gc,
		   points, i,
		   Complex, CoordModeOrigin);
#endif
    SET_FG(colors[hudColor].pixel);

    if (lock_dist != 0) {
	sprintf(str, "%03d", lock_dist / BLOCK_SZ);
	rd.drawString(dpy, p_draw, gc,
		    WINSCALE(hud_pos_x + HUD_SIZE - HUD_OFFSET + BORDER),
		    WINSCALE(hud_pos_y - HUD_SIZE+HUD_OFFSET
					 - gameFont->descent - BORDER),
		    str, 3);
	Erase_rectangle(hud_pos_x + HUD_SIZE - HUD_OFFSET + BORDER - 1,
			hud_pos_y - HUD_SIZE+HUD_OFFSET
			    - gameFont->descent - BORDER - gameFont->ascent,
			XTextWidth(gameFont, str, 3) + 2,
			gameFont->ascent + gameFont->descent);
	if (lock_dist > WARNING_DISTANCE || warningCount++ % 2 == 0) {
	    int size = MIN(mapdiag / lock_dist, 10);

	    if (size == 0) {
		size = 1;
	    }
	    if (self != NULL
		&& self->team == target->team
		&& BIT(Setup->mode, TEAM_PLAY)) {
		Arc_add(hudColor,
			(int)(hud_pos_x + HUD_SIZE * 0.6 * tcos(lock_dir)
			      - size * 0.5),
			(int)(hud_pos_y - HUD_SIZE * 0.6 * tsin(lock_dir)
			      - size * 0.5),
			size, size, 0, 64*360);
	    } else {
		SET_FG(colors[hudLockColor].pixel);
		x = (int)(hud_pos_x + HUD_SIZE * 0.6 * tcos(lock_dir)
			  - size * 0.5),
		y = (int)(hud_pos_y - HUD_SIZE * 0.6 * tsin(lock_dir)
			  - size * 0.5),
		rd.fillArc(dpy, p_draw, gc,
			 WINSCALE(x), WINSCALE(y),
			 WINSCALE(size), WINSCALE(size), 0, 64*360);
		Erase_rectangle(x, y, size, size);
		SET_FG(colors[hudColor].pixel);       
	    }
	}
    }
}


void Paint_HUD(void)
{
    const int		BORDER = 3;
    int			vert_pos, horiz_pos, size;
    char		str[50];
    int			hud_pos_x;
    int			hud_pos_y;
    int			i, j, maxWidth = -1,
			rect_x, rect_y, rect_width, rect_height;
    static int		vertSpacing = -1;
    static char		autopilot[] = "Autopilot";

    int modlen = 0;

    /*
     * Show speed pointer
     */
    if (ptr_move_fact != 0.0
	&& selfVisible != 0
	&& (vel.x != 0 || vel.y != 0)) {
	Segment_add(hudColor,
		    view_width / 2,
		    view_height / 2,
		    (int)(view_width / 2 - ptr_move_fact*vel.x),
		    (int)(view_height / 2 + ptr_move_fact*vel.y));
    }

    if (!BIT(instruments, SHOW_HUD_INSTRUMENTS)) {
	return;
    }

    /*
     * Display the HUD
     */
    SET_FG(colors[hudColor].pixel);

    hud_pos_x = (int)(view_width / 2 - hud_move_fact*vel.x);
    hud_pos_y = (int)(view_height / 2 + hud_move_fact*vel.y);

    /* HUD frame */
    gcv.line_style = LineOnOffDash;
    XChangeGC(dpy, gc, GCLineStyle | GCDashOffset, &gcv);

    if (BIT(instruments, SHOW_HUD_HORIZONTAL)) {
	rd.drawLine(dpy, p_draw, gc,
		  WINSCALE(hud_pos_x-HUD_SIZE), WINSCALE(hud_pos_y-HUD_SIZE+HUD_OFFSET),
		  WINSCALE(hud_pos_x+HUD_SIZE), WINSCALE(hud_pos_y-HUD_SIZE+HUD_OFFSET));
	Erase_segment(0,
		      hud_pos_x-HUD_SIZE, hud_pos_y-HUD_SIZE+HUD_OFFSET,
		      hud_pos_x+HUD_SIZE, hud_pos_y-HUD_SIZE+HUD_OFFSET);
	rd.drawLine(dpy, p_draw, gc,
		  WINSCALE(hud_pos_x-HUD_SIZE), WINSCALE(hud_pos_y+HUD_SIZE-HUD_OFFSET),
		  WINSCALE(hud_pos_x+HUD_SIZE), WINSCALE(hud_pos_y+HUD_SIZE-HUD_OFFSET));
	Erase_segment(0,
		      hud_pos_x-HUD_SIZE, hud_pos_y+HUD_SIZE-HUD_OFFSET,
		      hud_pos_x+HUD_SIZE, hud_pos_y+HUD_SIZE-HUD_OFFSET);
    }
    if (BIT(instruments, SHOW_HUD_VERTICAL)) {
	rd.drawLine(dpy, p_draw, gc,
		  WINSCALE(hud_pos_x-HUD_SIZE+HUD_OFFSET), WINSCALE(hud_pos_y-HUD_SIZE),
		  WINSCALE(hud_pos_x-HUD_SIZE+HUD_OFFSET), WINSCALE(hud_pos_y+HUD_SIZE));
	Erase_segment(0,
		      hud_pos_x-HUD_SIZE+HUD_OFFSET, hud_pos_y-HUD_SIZE,
		      hud_pos_x-HUD_SIZE+HUD_OFFSET, hud_pos_y+HUD_SIZE);
	rd.drawLine(dpy, p_draw, gc,
		  WINSCALE(hud_pos_x+HUD_SIZE-HUD_OFFSET), WINSCALE(hud_pos_y-HUD_SIZE),
		  WINSCALE(hud_pos_x+HUD_SIZE-HUD_OFFSET), WINSCALE(hud_pos_y+HUD_SIZE));
	Erase_segment(0,
		      hud_pos_x+HUD_SIZE-HUD_OFFSET, hud_pos_y-HUD_SIZE,
		      hud_pos_x+HUD_SIZE-HUD_OFFSET, hud_pos_y+HUD_SIZE);
    }
    gcv.line_style = LineSolid;
    XChangeGC(dpy, gc, GCLineStyle, &gcv);


    /* Special itemtypes */
    if (vertSpacing < 0)
	vertSpacing
	    = MAX(ITEM_SIZE, gameFont->ascent + gameFont->descent) + 1;
    vert_pos = hud_pos_y - HUD_SIZE+HUD_OFFSET + BORDER;
    horiz_pos = hud_pos_x - HUD_SIZE+HUD_OFFSET - BORDER;
    rect_width = 0;
    rect_height = 0;
    rect_x = horiz_pos;
    rect_y = vert_pos;

    for (i=0; i<NUM_ITEMS; i++) {
	int num = numItems[i];

	if (i == ITEM_FUEL)
	    continue;

	if (BIT(instruments, SHOW_ITEMS)) {
	    lastNumItems[i] = num;
	    if (num <= 0)
		num = -1;
	} else {
	    if (num != lastNumItems[i]) {
		numItemsTime[i] = (int)(showItemsTime * (float)FPS);
		lastNumItems[i] = num;
	    }
	    if (numItemsTime[i]-- <= 0) {
		numItemsTime[i] = 0;
		num = -1;
	    }
	}

	if (num >= 0) {
	    int len, width;

	    /* Paint item symbol */
	    Paint_item_symbol((u_byte)i, p_draw, gc, 
			WINSCALE(horiz_pos - ITEM_SIZE),
			WINSCALE(vert_pos), 
			ITEM_HUD);

	    if (i == lose_item) {
		if (lose_item_active != 0) {
		    if (lose_item_active < 0) {
			lose_item_active++;
		    }
		    rd.drawRectangle(dpy, p_draw, gc, 
				WINSCALE(horiz_pos-ITEM_SIZE-2),
				WINSCALE(vert_pos-2), ITEM_SIZE+2, ITEM_SIZE+2);
		}
	    }

	    /* Paint item count */
	    sprintf(str, "%d", num);
	    len = strlen(str);
	    width = XTextWidth(gameFont, str, len);
	    rd.drawString(dpy, p_draw, gc,
			WINSCALE(horiz_pos - ITEM_SIZE - BORDER - width),
			WINSCALE(vert_pos + ITEM_SIZE/2 + gameFont->ascent/2),
			str, len);

	    maxWidth = MAX(maxWidth, width + BORDER + ITEM_SIZE);
	    vert_pos += vertSpacing;

	    if (vert_pos+vertSpacing > hud_pos_y+HUD_SIZE-HUD_OFFSET-BORDER) {
		rect_width += maxWidth + 2*BORDER;
		rect_height = vert_pos - rect_y;
		horiz_pos -= maxWidth + 2*BORDER;
		vert_pos = hud_pos_y - HUD_SIZE+HUD_OFFSET + BORDER;
		maxWidth = -1;
	    }
	}
    }
    if (maxWidth != -1) {
	rect_width += maxWidth + BORDER;
    }
    if (rect_width > 0) {
	if (rect_height == 0) {
	    rect_height = vert_pos - rect_y;
	}
	rect_x -= rect_width;
	Erase_rectangle(rect_x, rect_y, rect_width, rect_height);
    }

    /* Fuel notify, HUD meter on */
    if (fuelCount || fuelSum < fuelLevel3) {
	sprintf(str, "%04d", (int)fuelSum);
	rd.drawString(dpy, p_draw, gc,
		    WINSCALE(hud_pos_x + HUD_SIZE-HUD_OFFSET+BORDER),
		    WINSCALE(hud_pos_y + HUD_SIZE-HUD_OFFSET+BORDER + gameFont->ascent),
		    str, strlen(str));
	Erase_rectangle(hud_pos_x + HUD_SIZE-HUD_OFFSET+BORDER - 1,
			hud_pos_y + HUD_SIZE-HUD_OFFSET+BORDER,
			XTextWidth(gameFont, str, strlen(str)) + 2,
			gameFont->ascent + gameFont->descent);
	if (numItems[ITEM_TANK]) {
	    if (fuelCurrent == 0)
		strcpy(str,"M ");
	    else
		sprintf(str, "T%d", fuelCurrent);
	    rd.drawString(dpy, p_draw, gc,
			WINSCALE(hud_pos_x + HUD_SIZE-HUD_OFFSET + BORDER),
			WINSCALE(hud_pos_y + HUD_SIZE-HUD_OFFSET + BORDER
			+ gameFont->descent + 2*gameFont->ascent),
			str, strlen(str));
	    Erase_rectangle(hud_pos_x + HUD_SIZE-HUD_OFFSET + BORDER - 1,
			    hud_pos_y + HUD_SIZE-HUD_OFFSET + BORDER
				+ gameFont->descent + gameFont->ascent,
			    XTextWidth(gameFont, str, strlen(str)) + 2,
			    gameFont->ascent + gameFont->descent);
	}
    }

    /* Update the lock display */
    Paint_lock(hud_pos_x, hud_pos_y);

    /* Draw last score on hud if it is an message attached to it */
    for (i=0, j=0; i < MAX_SCORE_OBJECTS; i++) {
	score_object_t*	sobj
	    = &score_objects[(i+score_object)%MAX_SCORE_OBJECTS];
	if (sobj->hud_msg_len > 0) {
	    rd.drawString(dpy, p_draw, gc,
			WINSCALE(hud_pos_x - sobj->hud_msg_width/2),
			WINSCALE(hud_pos_y + HUD_SIZE-HUD_OFFSET + BORDER
			+ gameFont->ascent + j * (gameFont->ascent + gameFont->descent)),
			sobj->hud_msg, sobj->hud_msg_len);
	    Erase_rectangle(hud_pos_x - sobj->hud_msg_width/2 - 1,
			    hud_pos_y + HUD_SIZE-HUD_OFFSET + BORDER
				+ j * (gameFont->ascent + gameFont->descent),
			    sobj->hud_msg_width + 2,
			    gameFont->ascent + gameFont->descent);
	    j++;
	}
    }

    if (time_left >= 0) {
	sprintf(str, "%3d:%02d", (int)(time_left / 60), (int)(time_left % 60));
	size = XTextWidth(gameFont, str, strlen(str));
	rd.drawString(dpy, p_draw, gc,
		    WINSCALE(hud_pos_x - HUD_SIZE+HUD_OFFSET - BORDER - size),
		    WINSCALE(hud_pos_y - HUD_SIZE+HUD_OFFSET - BORDER
			- gameFont->descent),
		    str, strlen(str));
	Erase_rectangle(hud_pos_x - HUD_SIZE+HUD_OFFSET - BORDER - size - 1,
			hud_pos_y - HUD_SIZE+HUD_OFFSET - BORDER,
			size + 2, gameFont->ascent + gameFont->descent);
    }

    /* Update the modifiers */
    modlen = strlen(mods);
    rd.drawString(dpy, p_draw, gc,
		WINSCALE(hud_pos_x - HUD_SIZE+HUD_OFFSET-BORDER
		    - XTextWidth(gameFont, mods, modlen)),
		WINSCALE(hud_pos_y + HUD_SIZE-HUD_OFFSET+BORDER
		    + gameFont->ascent),
		mods, strlen(mods));

    Erase_rectangle(hud_pos_x - HUD_SIZE+HUD_OFFSET-BORDER
			- XTextWidth(gameFont, mods, modlen),
		    hud_pos_y + HUD_SIZE-HUD_OFFSET+BORDER,
			XTextWidth(gameFont, mods, modlen) + 2,
		    gameFont->ascent + gameFont->descent);

    if (autopilotLight) {
	rd.drawString(dpy, p_draw, gc,
		    WINSCALE(hud_pos_x - XTextWidth(gameFont, autopilot,
					   sizeof(autopilot)-1)/2),
		    WINSCALE(hud_pos_y - HUD_SIZE+HUD_OFFSET - BORDER
					 - gameFont->descent * 2 - gameFont->ascent),
		    autopilot, sizeof(autopilot)-1);

	Erase_rectangle(hud_pos_x - XTextWidth(gameFont, autopilot,
						sizeof(autopilot)-1)/2,
			hud_pos_y - HUD_SIZE+HUD_OFFSET - BORDER
			    - gameFont->descent * 2 - gameFont->ascent * 2,
			XTextWidth(gameFont, autopilot,
				   sizeof(autopilot)-1) + 2,
			gameFont->ascent + gameFont->descent);
    }

    /* Fuel gauge, must be last */
    if (BIT(instruments, SHOW_FUEL_GAUGE) == 0
	|| !((fuelCount)
	   || (fuelSum < fuelLevel3
	      && ((fuelSum < fuelLevel1 && (loops%4) < 2)
		  || (fuelSum < fuelLevel2
		      && fuelSum > fuelLevel1
		      && (loops%8) < 4)
		  || (fuelSum > fuelLevel2)))))
	return;

    if (fuelCount > 0) {
	fuelCount--;
    }

/* XXX:   SET_FG(colors[BLUE].pixel);*/
    rd.drawRectangle(dpy, p_draw, gc,
		  WINSCALE(hud_pos_x + HUD_SIZE - HUD_OFFSET + FUEL_GAUGE_OFFSET - 1),
		  WINSCALE(hud_pos_y - HUD_SIZE + HUD_OFFSET + FUEL_GAUGE_OFFSET - 1),
		  WINSCALE(HUD_OFFSET - (2*FUEL_GAUGE_OFFSET) + 3),
		  WINSCALE(HUD_FUEL_GAUGE_SIZE + 3));
    Erase_4point(hud_pos_x + HUD_SIZE - HUD_OFFSET + FUEL_GAUGE_OFFSET - 1,
		 hud_pos_y - HUD_SIZE + HUD_OFFSET + FUEL_GAUGE_OFFSET - 1,
		 HUD_OFFSET - (2*FUEL_GAUGE_OFFSET) + 3,
		 HUD_FUEL_GAUGE_SIZE + 3);

    size = (HUD_FUEL_GAUGE_SIZE * fuelSum) / fuelMax;
    rd.fillRectangle(dpy, p_draw, gc,
		   WINSCALE(hud_pos_x + HUD_SIZE - HUD_OFFSET + FUEL_GAUGE_OFFSET + 1),
		   WINSCALE(hud_pos_y - HUD_SIZE + HUD_OFFSET + FUEL_GAUGE_OFFSET
					+ HUD_FUEL_GAUGE_SIZE - size + 1),
		   WINSCALE(HUD_OFFSET - (2*FUEL_GAUGE_OFFSET)),
		   WINSCALE(size));
    Erase_rectangle(hud_pos_x + HUD_SIZE - HUD_OFFSET + FUEL_GAUGE_OFFSET + 1,
		    hud_pos_y - HUD_SIZE + HUD_OFFSET + FUEL_GAUGE_OFFSET
			+ HUD_FUEL_GAUGE_SIZE - size + 1,
		    HUD_OFFSET - (2*FUEL_GAUGE_OFFSET), size);
}

void Paint_messages(void)
{
    int		i, x, y, top_y, bot_y, width, len;
    const int	BORDER = 10,
		SPACING = messageFont->ascent+messageFont->descent+1;
    message_t	*msg;

    if (charsPerTick <= 0.0)
	charsPerTick = (float)charsPerSecond / FPS;

    top_y = BORDER + messageFont->ascent;
    bot_y = WINSCALE(view_height) - messageFont->descent - BORDER;

    for (i = (BIT(instruments, SHOW_REVERSE_SCROLL) ? 2 * maxMessages : 0); (BIT(instruments, SHOW_REVERSE_SCROLL) ? i >= 0 : i < 2 * maxMessages); i+= (BIT(instruments, SHOW_REVERSE_SCROLL) ? -1 : 1)) {
	if (i < maxMessages) {
	    msg = TalkMsg[i];
	} else {
	    msg = GameMsg[i - maxMessages];
	}
	if (msg->len == 0)
	    continue;
	if (msg->life-- <= 0) {
	    msg->txt[0] = '\0';
	    msg->len = 0;
	    msg->life = 0;
	    continue;
	}
	if (i < maxMessages) {
	    x = BORDER;
	    y = top_y;
	    top_y += SPACING;
	} else {
	    if (!BIT(instruments, SHOW_MESSAGES)) {
		continue;
	    }
	    x = BORDER;
	    y = bot_y;
	    bot_y -= SPACING;
	}
	if (msg->life > MSG_FLASH)
	    XSetForeground(dpy, messageGC, colors[RED].pixel);
	else
	    XSetForeground(dpy, messageGC, colors[WHITE].pixel);
	len = (int)(charsPerTick * (MSG_DURATION - msg->life));
	len = MIN(msg->len, len);
	rd.drawString(dpy, p_draw, messageGC,
		      x, y,
		      msg->txt, len);
	if (len < msg->len) {
	    width = XTextWidth(messageFont, msg->txt, len);
	} else {
	    width = msg->pixelLen;
	}
	Erase_rectangle(x - 1, y - messageFont->ascent,
			width + 2,
			messageFont->ascent + messageFont->descent);
    }
}


void Add_message(char *message)
{
    int			i, len;
    message_t		*tmp, **msg_set;

    len = strlen(message);
    if (message[len - 1] == ']' || strncmp(message, " <", 2) == 0) {
	msg_set = TalkMsg;
    } else {
	msg_set = GameMsg;
    }
    tmp = msg_set[maxMessages - 1];
    for (i = maxMessages - 1; i > 0; i--) {
	msg_set[i] = msg_set[i - 1];
    }
    msg_set[0] = tmp;

    msg_set[0]->life = MSG_DURATION;
    strcpy(msg_set[0]->txt, message);
    msg_set[0]->len = len;

#ifdef DEVELOPMENT
    /* anti-censor hack */
    for (i = 0; i < len - 3; i++) {
	static char censor_text[] = "@&$*";
	static char rough_text[][5] = { "fuck", "shit", "damn" };
	static int rough_index = 0;
	if (msg_set[0]->txt[i] == censor_text[0]
	    && !strncmp(&msg_set[0]->txt[i], censor_text, 4)) {
	    if (++rough_index >= 3) {
		rough_index = 0;
	    }
	    strncpy(&msg_set[0]->txt[i], rough_text[rough_index], 4);
	}
    }
#endif

    msg_set[0]->pixelLen = XTextWidth(messageFont, msg_set[0]->txt, msg_set[0]->len);
}

void Paint_recording(void)
{
    int			w = -1;
    int			x, y;
    char		buf[32];
    int			mb, ck, len;
    long		size;

    if (!recording || (loops % 16) < 8)
	return;

    SET_FG(colors[RED].pixel);
    size = Record_size();
    mb = size >> 20;
    ck = (10 * (size - ((long)mb << 20))) >> 20;
    sprintf(buf, "REC %d.%d", mb, ck);
    len = strlen(buf);
    w = XTextWidth(gameFont, buf, len);
    x = view_width - 10 - w;
    y = 10 + gameFont->ascent;
    XDrawString(dpy, p_draw, gc, x, y, buf, len);
    Erase_rectangle(x-1, 10, w+2, gameFont->ascent + gameFont->descent);
}


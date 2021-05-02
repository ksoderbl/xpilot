/* $Id: record.h,v 5.0 2001/04/07 20:00:58 dik Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
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

#ifndef RECORD_H
#define RECORD_H

/*
 * Structure to call all user-interface drawing routines
 * which are modified versions when recording is active.
 */
struct recordable_drawing {
    void (*newFrame)(void);
    void (*endFrame)(void);
    int (*drawArc)(Display *display, Drawable drawable, GC gc,
		    int x, int y,
		    unsigned width, unsigned height,
		    int angle1, int angle2);
    int (*drawLines)(Display *display, Drawable drawable, GC gc,
		     XPoint *points, int npoints, int mode);
    int (*drawLine)(Display *display, Drawable drawable, GC gc,
		    int x1, int y1,
		    int x2, int y2);
    int (*drawRectangle)(Display *display, Drawable drawable, GC gc,
			 int x, int y,
			 unsigned int width, unsigned int height);
    int (*drawString)(Display *display, Drawable drawable, GC gc,
		      int x, int y,
		      const char *string, int length);
    int (*fillArc)(Display *display, Drawable drawable, GC gc,
		    int x, int y,
		    unsigned height, unsigned width,
		    int angle1, int angle2);
    int (*fillPolygon)(Display *display, Drawable drawable, GC gc,
			XPoint *points, int npoints,
			int shape, int mode);
    void (*paintItemSymbol)(unsigned char type, Drawable drawable, GC mygc,
			    int x, int y, int color);
    int (*fillRectangle)(Display *display, Drawable drawable, GC gc,
			  int x, int y,
			  unsigned width, unsigned height);
    int (*fillRectangles)(Display *display, Drawable drawable, GC gc,
			   XRectangle *rectangles, int nrectangles);
    int (*drawArcs)(Display *display, Drawable drawable, GC gc,
		     XArc *arcs, int narcs);
    int (*drawSegments)(Display *display, Drawable drawable, GC gc,
			 XSegment *segments, int nsegments);
    int (*setDashes)(Display *display, GC gc,
		     int dash_offset, const char *dash_list, int n);
};

extern struct recordable_drawing	rd;	/* external Drawing interface */

extern int		recording;	/* Are we recording or not. */


long Record_size(void);
void Record_toggle(void);
void Record_init(char *filename);
void Record_cleanup(void);

#endif

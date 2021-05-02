/* $Id: record.c,v 3.9 1995/09/17 15:13:06 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
 *
 *      Bjørn Stabell        (bjoerns@staff.cs.uit.no)
 *      Ken Ronny Schouten   (kenrsc@stud.cs.uit.no)
 *      Bert Gÿsbers         (bert@mc.bio.uva.nl)
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

#ifdef VMS
#include <unixio.h>
#include <unixlib.h>
#else
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "version.h"
#include "config.h"
#include "error.h"
#include "const.h"
#include "client.h"
#include "paint.h"
#include "setup.h"
#include "record.h"
#include "recordfmt.h"
#include "xpmread.h"

char record_version[] = VERSION;

/*
 * GC elements for line drawing operations.
 */
#define RSTROKEGC	(GCForeground | GCBackground | \
			 GCLineWidth | GCLineStyle | \
			 GCDashOffset | GCFunction)
/*
 * GC elements for polygon filling (except GCForeground).
 */
#define RTILEGC		(GCFillStyle | GCTile | \
			 GCTileStipXOrigin | GCTileStipYOrigin)


/*
 * Functions and variables for recording
 */
static char		*record_filename = NULL;/* Name of recordfile. */
static FILE		*recordFP = NULL;	/* File handle for writing
						 * recording frames to. */
int			recording = False;	/* Are we recording or not. */
static int		record_start = False;	/* Should we start recording
						 * at the next frame. */
static int		record_frame_count = 0;	/* How many recorded frames. */
static const char	*record_dashes;		/* Which dash list to use. */
static int		record_num_dashes;	/* How big is dashes list. */
static int		record_dash_dirty = 0;	/* Has dashes list changed? */

/*
 * Dummy functions for "recordable drawing" interface, when not recording.
 */
static void Dummy_newFrame(void) {}
static void Dummy_endFrame(void) {}
static void Dummy_paintItemSymbol(unsigned char type, Drawable drawable,
				  GC mygc, int x, int y) {}

/*
 * Miscellaneous recording functions.
 * These are only called when (recording == True).
 */
static void RWriteByte(unsigned char i)
{
    putc(i, recordFP);
}

static void RWriteShort(short i)
{
    putc(i, recordFP);
    i >>= 8;
    putc(i, recordFP);
}

static void RWriteUShort(unsigned short i)
{
    putc(i, recordFP);
    i >>= 8;
    putc(i, recordFP);
}

static void RWriteLong(long i)
{
    putc(i, recordFP);
    i >>= 8;
    putc(i, recordFP);
    i >>= 8;
    putc(i, recordFP);
    i >>= 8;
    putc(i, recordFP);
}

static void RWriteULong(unsigned long i)
{
    putc(i, recordFP);
    i >>= 8;
    putc(i, recordFP);
    i >>= 8;
    putc(i, recordFP);
    i >>= 8;
    putc(i, recordFP);
}

static void RWriteString(char *str)
{
    int				len = strlen(str);
    int				i;

    RWriteUShort(len);
    for (i = 0; i < len; i++) {
	putc(str[i], recordFP);
    }
}

/*
 * Output the XPilot Recording Header at the beginning
 * of the recording file.
 */
static void RWriteHeader(void)
{
    extern char			hostname[];
    time_t			t;
    char			buf[256];
    char			*ptr;
    int				i;

    rewind(recordFP);

    /* First write out magic 4 letter word */
    putc('X', recordFP);
    putc('P', recordFP);
    putc('R', recordFP);
    putc('C', recordFP);

    /* Write which version of the XPilot Record Protocol this is. */
    putc(RC_MAJORVERSION, recordFP);
    putc('.', recordFP);
    putc(RC_MINORVERSION, recordFP);
    putc('\n', recordFP);

    /* Write player's nick, login, host, server, FPS and the date. */
    RWriteString(name);
    RWriteString(realname);
    RWriteString(hostname);
    RWriteString(servername);
    RWriteByte(FPS);
    time(&t);
    strcpy(buf, ctime(&t));
    if ((ptr = strchr(buf, '\n')) != NULL) {
	*ptr = '\0';
    }
    RWriteString(buf);

    /* Write info about graphics setup. */
    putc(maxColors, recordFP);
    for (i = 0; i < maxColors; i++) {
	RWriteULong(colors[i].pixel);
	RWriteUShort(colors[i].red);
	RWriteUShort(colors[i].green);
	RWriteUShort(colors[i].blue);
    }
    RWriteString(gameFontName);
    RWriteString(messageFontName);

    RWriteUShort(view_width);
    RWriteUShort(view_height);

    record_dashes = dashes;
    record_num_dashes = NUM_DASHES;
    record_dash_dirty = True;
}

static int RGetPixelIndex(unsigned long pixel)
{
    int			i;

    for (i = 0; i < maxColors; i++) {
	if (pixel == colors[i].pixel) {
	    return i;
	}
    }
    for (i = 1; i < maxColors; i++) {
	if (pixel == (colors[BLACK].pixel ^ colors[i].pixel)) {
	    return i + maxColors;
	}
    }

    return WHITE;
}

static void RWriteTile(Pixmap tile)
{
    typedef struct tile_list {
	struct tile_list	*next;
	Pixmap			tile;
	unsigned char		tile_id;
    } tile_list_t;
    static tile_list_t		*list = NULL;
    tile_list_t			*lptr;
    static int			next_tile_id = 1;
    unsigned			x, y;
    int				i;
    XImage			*img;

    for (lptr = list; lptr != NULL; lptr = lptr->next) {
	if (lptr->tile == tile) {
	    /* tile already sent before. */
	    RWriteByte(RC_TILE);
	    RWriteByte(lptr->tile_id);
	    return;
	}
    }

    /* a first time tile. */

    if (!(lptr = (tile_list_t *)malloc(sizeof(tile_list_t)))) {
	error("Not enough memory");
	RWriteByte(RC_TILE);
	RWriteByte(0);
	return;
    }
    lptr->next = list;
    lptr->tile = tile;
    lptr->tile_id = next_tile_id;
    list = lptr;

    if (!(img = xpm_image_from_pixmap(tile))) {
	RWriteByte(RC_TILE);
	RWriteByte(0);
	lptr->tile_id = 0;
	return;
    }
    RWriteByte(RC_NEW_TILE);
    RWriteByte(lptr->tile_id);
    RWriteUShort(img->width);
    RWriteUShort(img->height);
    for (y = 0; y < img->height; y++) {
	for (x = 0; x < img->width; x++) {
	    unsigned long pixel = XGetPixel(img, x, y);
	    for (i = 0; i < maxColors - 1; i++) {
		if (pixel == colors[i].pixel) {
		    break;
		}
	    }
	    RWriteByte(i);
	}
    }

    XDestroyImage(img);

    next_tile_id++;
}

static void RWriteGC(GC gc, unsigned long req_mask)
{
    XGCValues			values;
    unsigned long		write_mask;
    static unsigned long	prev_mask;
    static XGCValues		prev_values;
    static int			prev_frame_count = -1;
    unsigned short		gc_mask;

    if (prev_frame_count != record_frame_count) {
	prev_frame_count = record_frame_count;
	write_mask = RSTROKEGC | RTILEGC;
	XGetGCValues(dpy, gc, write_mask, &values);
	if (values.fill_style != FillTiled) {
	    write_mask &= ~(GCTileStipXOrigin | GCTileStipYOrigin
			    | GCTile);
	}
	prev_mask = write_mask;
	prev_values = values;
    }
    else {
	write_mask = req_mask | GCFunction;
	XGetGCValues(dpy, gc, write_mask, &values);

	if ((write_mask & prev_mask & GCForeground) != 0) {
	    if (prev_values.foreground == values.foreground) {
		write_mask &= ~GCForeground;
	    } else {
		prev_values.foreground = values.foreground;
	    }
	}
	if ((write_mask & prev_mask & GCBackground) != 0) {
	    if (prev_values.background == values.background) {
		write_mask &= ~GCBackground;
	    } else {
		prev_values.background = values.background;
	    }
	}
	if ((write_mask & prev_mask & GCLineWidth) != 0) {
	    if (prev_values.line_width == values.line_width) {
		write_mask &= ~GCLineWidth;
	    } else {
		prev_values.line_width = values.line_width;
	    }
	}
	if ((write_mask & prev_mask & GCLineStyle) != 0) {
	    if (prev_values.line_style == values.line_style) {
		write_mask &= ~GCLineStyle;
	    } else {
		prev_values.line_style = values.line_style;
	    }
	}
	if ((write_mask & prev_mask & GCDashOffset) != 0) {
	    if (prev_values.dash_offset == values.dash_offset) {
		write_mask &= ~GCDashOffset;
	    } else {
		prev_values.dash_offset = values.dash_offset;
	    }
	}
	if ((write_mask & prev_mask & GCFunction) != 0) {
	    if (prev_values.function == values.function) {
		write_mask &= ~GCFunction;
	    } else {
		prev_values.function = values.function;
	    }
	}
	if ((write_mask & prev_mask & GCFillStyle) != 0) {
	    if (prev_values.fill_style == values.fill_style) {
		write_mask &= ~GCFillStyle;
	    } else {
		prev_values.fill_style = values.fill_style;
	    }
	    /*
	     * We only update some values if they
	     * are going to be used.
	     * e.g., no use for tile origins and tiles
	     * if fill style is not tiled.
	     */
	    if (values.fill_style == FillTiled) {
		if ((write_mask & prev_mask & GCTileStipXOrigin) != 0) {
		    if (prev_values.ts_x_origin == values.ts_x_origin) {
			write_mask &= ~GCTileStipXOrigin;
		    } else {
			prev_values.ts_x_origin = values.ts_x_origin;
		    }
		}
		if ((write_mask & prev_mask & GCTileStipYOrigin) != 0) {
		    if (prev_values.ts_y_origin == values.ts_y_origin) {
			write_mask &= ~GCTileStipYOrigin;
		    } else {
			prev_values.ts_y_origin = values.ts_y_origin;
		    }
		}
		if ((write_mask & prev_mask & GCTile) != 0) {
		    if (prev_values.tile == values.tile) {
			write_mask &= ~GCTile;
		    } else {
			prev_values.tile = values.tile;
		    }
		}
	    }
	    else {
		write_mask &= ~(GCTileStipXOrigin | GCTileStipYOrigin
				| GCTile);
	    }
	}

	if (!write_mask && !record_dash_dirty) {
	    putc(RC_NOGC, recordFP);
	    return;
	}

	prev_mask |= write_mask;
    }

    putc(RC_GC, recordFP);

    gc_mask = 0;
    if (write_mask & GCForeground)
	gc_mask |= RC_GC_FG;
    if (write_mask & GCBackground)
	gc_mask |= RC_GC_BG;
    if (write_mask & GCLineWidth)
	gc_mask |= RC_GC_LW;
    if (write_mask & GCLineStyle)
	gc_mask |= RC_GC_LS;
    if (write_mask & GCDashOffset)
	gc_mask |= RC_GC_DO;
    if (write_mask & GCFunction)
	gc_mask |= RC_GC_FU;
    if (record_dash_dirty) {
	gc_mask |= RC_GC_DA;
	if ((write_mask & GCDashOffset) == 0) {
	    write_mask |= GCDashOffset;
	    values.dash_offset = prev_values.dash_offset;
	    gc_mask |= RC_GC_DO;
	}
    }
    if (write_mask & RTILEGC) {
	gc_mask |= RC_GC_B2;
	if (write_mask & GCFillStyle)
	    gc_mask |= RC_GC_FS;
	if (write_mask & GCTileStipXOrigin)
	    gc_mask |= RC_GC_XO;
	if (write_mask & GCTileStipYOrigin)
	    gc_mask |= RC_GC_YO;
	if (write_mask & GCTile)
	    gc_mask |= RC_GC_TI;
    }

    RWriteByte(gc_mask);
    if (gc_mask & RC_GC_B2) {
	RWriteByte(gc_mask >> 8);
    }

    if (write_mask & GCForeground)
	RWriteByte(RGetPixelIndex(values.foreground));
    if (write_mask & GCBackground)
	RWriteByte(RGetPixelIndex(values.background));
    if (write_mask & GCLineWidth)
	RWriteByte(values.line_width);
    if (write_mask & GCLineStyle)
	RWriteByte(values.line_style);
    if (write_mask & GCDashOffset)
	RWriteByte(values.dash_offset);
    if (write_mask & GCFunction)
	RWriteByte(values.function);
    if (record_dash_dirty) {
	int i;
	RWriteByte(record_num_dashes);
	for (i = 0; i < record_num_dashes; i++) {
	    RWriteByte(record_dashes[i]);
	}
    }
    if (write_mask & RTILEGC) {
	if (write_mask & GCFillStyle)
	    RWriteByte(values.fill_style);
	if (write_mask & GCTileStipXOrigin)
	    RWriteLong(values.ts_x_origin);
	if (write_mask & GCTileStipYOrigin)
	    RWriteLong(values.ts_y_origin);
	if (write_mask & GCTile) {
	    RWriteTile(values.tile);
	}
    }
}

static void RNewFrame(void)
{
    static int		before;

    if (!before++) {
	RWriteHeader();
    }

    recording = True;

    putc(RC_NEWFRAME, recordFP);
    RWriteUShort(view_width);
    RWriteUShort(view_height);
}

static void REndFrame(void)
{
    if (damaged) {
	XGCValues			values;

	XGetGCValues(dpy, gc, GCForeground, &values);

	RWriteByte(RC_DAMAGED);
	if ((damaged & 1) != 0) {
	    XSetForeground(dpy, gc, colors[BLUE].pixel);
	} else {
	    XSetForeground(dpy, gc, colors[BLACK].pixel);
	}
	RWriteGC(gc, GCForeground | RTILEGC);
	RWriteByte(damaged);

	XSetForeground(dpy, gc, values.foreground);
    }

    putc(RC_ENDFRAME, recordFP);

    fflush(recordFP);

    recording = False;

    record_frame_count++;	/* Number of frames written sofar. */
}

static int RDrawArc(Display *display, Drawable drawable, GC gc,
		    int x, int y,
		    unsigned width, unsigned height,
		    int angle1, int angle2)
{
    XDrawArc(display, drawable, gc, x, y, width, height, angle1, angle2);
    if (drawable == p_draw) {
	putc(RC_DRAWARC, recordFP);
	RWriteGC(gc, RSTROKEGC | RTILEGC);
	RWriteShort(x);
	RWriteShort(y);
	RWriteByte(width);
	RWriteByte(height);
	RWriteShort(angle1);
	RWriteShort(angle2);
    }
    return 0;
}

static int RDrawLines(Display *display, Drawable drawable, GC gc,
		      XPoint *points, int npoints, int mode)
{
    XDrawLines(display, drawable, gc, points, npoints, mode);
    if (drawable == p_draw) {
	int i;
	XPoint *xp = points;

	putc(RC_DRAWLINES, recordFP);
	RWriteGC(gc, RSTROKEGC | RTILEGC);
	RWriteUShort(npoints);
	for (i = 0; i < npoints; i++, xp++) {
	    RWriteShort(xp->x);
	    RWriteShort(xp->y);
	}
	RWriteByte(mode);
    }
    return 0;
}

static int RDrawLine(Display *display, Drawable drawable, GC gc,
		     int x1, int y1,
		     int x2, int y2)
{
    XDrawLine(display, drawable, gc, x1, y1, x2, y2);
    if (drawable == p_draw) {
	putc(RC_DRAWLINE, recordFP);
	RWriteGC(gc, RSTROKEGC | RTILEGC);
	RWriteShort(x1);
	RWriteShort(y1);
	RWriteShort(x2);
	RWriteShort(y2);
    }
    return 0;
}

static int RDrawRectangle(Display *display, Drawable drawable, GC gc,
			  int x, int y,
			  unsigned width, unsigned height)
{
    XDrawRectangle(display, drawable, gc, x, y, width, height);
    if (drawable == p_draw) {
	putc(RC_DRAWRECTANGLE, recordFP);
	RWriteGC(gc, RSTROKEGC | RTILEGC);
	RWriteShort(x);
	RWriteShort(y);
	RWriteByte(width);
	RWriteByte(height);
    }
    return 0;
}

static int RDrawString(Display *display, Drawable drawable, GC gc,
		       int x, int y,
		       const char *string, int length)
{
    XDrawString(display, drawable, gc, x, y, string, length);
    if (drawable == p_draw) {
	int i;
	XGCValues values;

	putc(RC_DRAWSTRING, recordFP);
	RWriteGC(gc, GCForeground | RTILEGC);
	RWriteShort(x);
	RWriteShort(y);
	XGetGCValues(display, gc, GCFont, &values);
	RWriteByte((values.font == messageFont->fid) ? 1 : 0);
	RWriteUShort(length);
	for (i = 0; i < length; i++)
	    putc(string[i], recordFP);
    }
    return 0;
}
   
static int RFillArc(Display *display, Drawable drawable, GC gc,
		    int x, int y,
		    unsigned height, unsigned width,
		    int angle1, int angle2)
{
    XFillArc(display, drawable, gc, x, y, width, height, angle1, angle2);
    if (drawable == p_draw) {
	putc(RC_FILLARC, recordFP);
	RWriteGC(gc, GCForeground | RTILEGC);
	RWriteShort(x);
	RWriteShort(y);
	RWriteByte(width);
	RWriteByte(height);
	RWriteShort(angle1);
	RWriteShort(angle2);
    }
    return 0;
}

static int RFillPolygon(Display *display, Drawable drawable, GC gc,
		        XPoint *points, int npoints,
			int shape, int mode)
{
    XFillPolygon(display, drawable, gc, points, npoints, shape, mode);
    if (drawable == p_draw) {
	int i;
	XPoint *xp = points;

	putc(RC_FILLPOLYGON, recordFP);
	RWriteGC(gc, GCForeground | RTILEGC);
	RWriteUShort(npoints);
	for (i = 0; i < npoints; i++, xp++) {
	    RWriteShort(xp->x);
	    RWriteShort(xp->y);
	}
	RWriteByte(shape);
	RWriteByte(mode);
    }
    return 0;
}

static void RPaintItemSymbol(unsigned char type, Drawable drawable, GC mygc,
			     int x, int y)
{
    if (drawable == p_draw) {
	putc(RC_PAINTITEMSYMBOL, recordFP);
	RWriteGC(gc, GCForeground | GCBackground);
	putc(type, recordFP);
	RWriteShort(x);
	RWriteShort(y);
    }
}

static int RFillRectangle(Display *display, Drawable drawable, GC gc,
			  int x, int y,
			  unsigned width, unsigned height)
{
    XFillRectangle(display, drawable, gc, x, y, width, height);
    if (drawable == p_draw) {
	putc(RC_FILLRECTANGLE, recordFP);
	RWriteGC(gc, GCForeground | RTILEGC);
	RWriteShort(x);
	RWriteShort(y);
	RWriteByte(width);
	RWriteByte(height);
    }
    return 0;
}

static int RFillRectangles(Display *display, Drawable drawable, GC gc,
			   XRectangle *rectangles, int nrectangles)
{
    XFillRectangles(display, drawable, gc, rectangles, nrectangles);
    if (drawable == p_draw) {
	int i;

	putc(RC_FILLRECTANGLES, recordFP);
	RWriteGC(gc, GCForeground | RTILEGC);
	RWriteUShort(nrectangles);
	for (i = 0; i < nrectangles; i++) {
	    RWriteShort(rectangles[i].x);
	    RWriteShort(rectangles[i].y);
	    RWriteByte(rectangles[i].width);
	    RWriteByte(rectangles[i].height);
	}
    }
    return 0;
}

static int RDrawArcs(Display *display, Drawable drawable, GC gc,
		     XArc *arcs, int narcs)
{
    XDrawArcs(display, drawable, gc, arcs, narcs);
    if (drawable == p_draw) {
	int i;

	putc(RC_DRAWARCS, recordFP);
	RWriteGC(gc, RSTROKEGC | RTILEGC);
	RWriteUShort(narcs);
	for (i = 0; i < narcs; i++) {
	    RWriteShort(arcs[i].x);
	    RWriteShort(arcs[i].y);
	    RWriteByte(arcs[i].width);
	    RWriteByte(arcs[i].height);
	    RWriteShort(arcs[i].angle1);
	    RWriteShort(arcs[i].angle2);
	}
    }
    return 0;
}

static int RDrawSegments(Display *display, Drawable drawable, GC gc,
			 XSegment *segments, int nsegments)
{
    XDrawSegments(display, drawable, gc, segments, nsegments);
    if (drawable == p_draw) {
	int i;

	putc(RC_DRAWSEGMENTS, recordFP);
	RWriteGC(gc, RSTROKEGC | RTILEGC);
	RWriteUShort(nsegments);
	for (i = 0; i < nsegments; i++) {
	    RWriteShort(segments[i].x1);
	    RWriteShort(segments[i].y1);
	    RWriteShort(segments[i].x2);
	    RWriteShort(segments[i].y2);
	}
    }
    return 0;
}

static int RSetDashes(Display *display, GC gc,
		      int dash_offset, const char *dash_list, int n)
{
    XSetDashes(display, gc, dash_offset, dash_list, n);
    record_dashes = dash_list;	/* supposedly static memory */
    record_num_dashes = n;
    record_dash_dirty = True;
    return 0;
}


/*
 * The `_Xconst' trick from <X11/Xfuncproto.h> doesn't work
 * on Suns when not compiling under full ANSI mode.
 * So we force the prototypes to use `const' instead of `_Xconst'
 * by means of defining function types and casting with them.
 */
typedef int (*draw_string_proto_t)(Display *, Drawable, GC,
				   int, int, const char *, int);
typedef int (*set_dashes_proto_t)(Display *, GC, int, const char *, int);

/*
 * X windows drawing
 */
static struct recordable_drawing Xdrawing = {
    Dummy_newFrame,
    Dummy_endFrame,
    XDrawArc,
    XDrawLines,
    XDrawLine,
    XDrawRectangle,
    (draw_string_proto_t)XDrawString,
    XFillArc,
    XFillPolygon,
    Dummy_paintItemSymbol,
    XFillRectangle,
    XFillRectangles,
    XDrawArcs,
    XDrawSegments,
    (set_dashes_proto_t)XSetDashes,
};

/*
 * Recording + X windows drawing
 */
static struct recordable_drawing Rdrawing = {
    RNewFrame,
    REndFrame,
    RDrawArc,
    RDrawLines,
    RDrawLine,
    RDrawRectangle,
    RDrawString,
    RFillArc,
    RFillPolygon,
    RPaintItemSymbol,
    RFillRectangle,
    RFillRectangles,
    RDrawArcs,
    RDrawSegments,
    RSetDashes,
};

/*
 * Publicly accessible drawing routines.
 * This is either a copy of Xdrawing or of Rdrawing.
 */
struct recordable_drawing rd;

/*
 * Return the number of bytes written sofar to
 * the record file.  This way the user can monitor
 * that she ain't filling up all of her diskspace.
 */
long Record_size(void)
{
    return (recordFP != NULL) ? ftell(recordFP) : 0L;
}

/*
 * Toggle the recording of frames.
 * This only makes sense if there has been defined
 * a filename to write the recordings to.
 * When recording is turned on for the first time
 * then we have to open the file to write to.
 */
void Record_toggle(void)
{
    if (record_filename != NULL) {
	if (!record_start) {
	    record_start = True;
	    if (!recordFP) {
		if ((recordFP = fopen(record_filename, "w")) == NULL) {
		    perror("Unable to open record file");
		    free(record_filename);
		    record_filename = NULL;
		    record_start = False;
		} else {
		    setvbuf(recordFP, NULL, _IOFBF, (size_t)(8 * 1024));
		}
	    }
	} else {
	    record_start = False;
	}
	if (record_start) {
	    rd = Rdrawing;
	} else {
	    rd = Xdrawing;
	    recording = False;
	}
    }
}

/*
 * Inform the user how many frames have been
 * written and remind her to which file.
 */
void Record_cleanup(void)
{
    if (record_filename != NULL && record_frame_count > 0) {
	fflush(recordFP);
	printf("Recorded %d frames to %s\n",
	       record_frame_count, record_filename);
    }
}

/*
 * Store the name of the file where the user
 * wants recordings to be written to.
 */
void Record_init(char *filename)
{
    rd = Xdrawing;
    if (filename != NULL && filename[0] != '\0') {
	record_filename = strdup(filename);
    }
}


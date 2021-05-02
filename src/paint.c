/* $Id: paint.c,v 3.34 1993/08/03 21:09:18 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <X11/Intrinsic.h>
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "version.h"
#include "client.h"
#include "const.h"
#include "error.h"
#include "draw.h"
#include "item.h"
#include "xinit.h"
#include "setup.h"
#include "rules.h"
#include "bit.h"
#include "paint.h"
#include "net.h"
#include "netclient.h"

#define BLUE_LEFT	1
#define BLUE_UP		2
#define BLUE_RIGHT	3
#define BLUE_DOWN	4

#define X(co)  ((int) ((co) - world.x))
#define Y(co)  ((int) (FULL - (co) + world.y))

extern float  		tbl_sin[];
extern wireobj		ships[];
extern setup_t		*Setup;
extern int		RadarHeight;
extern score_object_t	score_objects[MAX_SCORE_OBJECTS];
extern int 		score_object;


/*
 * Globals.
 */
XFontStruct* gameFont;		/* The fonts used in the game */
XFontStruct* messageFont;
XFontStruct* scoreListFont;
XFontStruct* buttonFont;
XFontStruct* textFont;
char	gameFontName[FONT_LEN];	/* The fonts used in the game */
char	messageFontName[FONT_LEN];
char	scoreListFontName[FONT_LEN];
char	buttonFontName[FONT_LEN];
char	textFontName[FONT_LEN];
char	color_names[4][MAX_COLOR_LEN];
char	*color_defaults[4];

Display	*dpy;			/* Display of player (pointer) */
short	about_page;		/* Which page is the player on? */
u_short	team;			/* What team is the player on? */

u_byte	dpy_type;		/* Display type */

GC	gc;			/* GC for the game area */
GC	messageGC;		/* GC for messages in the game area */
GC	radarGC;		/* GC for the radar */
GC	buttonGC;		/* GC for the buttons */
GC	scoreListGC;		/* GC for the player list */
GC	textGC;			/* GC for the info/help text */

Window	top;			/* Top-level window (topshell) */
Window	draw;			/* Main play window */
Pixmap	p_draw;			/* Saved pixmap for the drawing */
					/* area (monochromes use this) */
Window	radar;			/* Radar window */
Window	players;		/* Player list window */
Pixmap	p_radar, s_radar;	/* Pixmaps for the radar (implements */
				/* the planes hack on the radar for */
				/* monochromes) */
long	dpl_1[2], dpl_2[2];	/* Used by radar hack */
Window	quit_b;			/* Quit button */
Window	help_b;			/* Help window button */
Window	about_b;		/* About button */
Window	help_w;			/* Help window */
Window	about_w;		/* About window */
Window	about_close_b;		/* About window's close button */
Window	about_next_b;		/* About window's next button */
Window	about_prev_b;		/* About window's previous button */
Window	help_close_b;		/* Help window's close button */
Window	talk_w;			/* Talk window */
XColor	colors[4];		/* Colors */
Colormap	colormap;	/* Private colormap */
bool	gotFocus;
bool	radar_exposed;
bool	players_exposed;

dbuff_state_t   *dbuf_state;	/* Holds current dbuff state */

int		maxKeyDefs;
keydefs_t	*keyDefs;

other_t	*self;			/* player info */

message_t		*Msg[MAX_MSGS];


/*
 * Local types and data
 */
typedef struct {
    short		x0, y0, x1, y1;
} refuel_t;

typedef struct {
    short		x0, y0, x1, y1;
} connector_t;

typedef struct {
    short		x, y, dir;
} smart_t;

typedef struct {
    short		x, y, id;
} ball_t;

typedef struct {
    short		x, y, id, dir;
    u_byte		shield, cloak;
} ship_t;

typedef struct {
    short		x, y;
} mine_t;

typedef struct {
    short		x, y, type;
} itemtype_t;

typedef struct {
    short		x, y, color;
} shot_t;

typedef struct {
    short		x, y, size;
} ecm_t;

typedef struct {
    short		x1, y1, x2, y2;
} trans_t;

typedef struct {
    short		x, y, count;
} paused_t;

typedef struct {
    short		x, y;
} radar_t;

typedef struct {
    short		x, y, type;
} vcannon_t;

typedef struct {
    short		x, y;
    long		fuel;
} vfuel_t;

typedef struct {
    short		x, y, xi, yi, type;
} vbase_t;

typedef struct {
    u_byte		x, y;
} debris_t;

static refuel_t		*refuel_ptr;
static int		 num_refuel, max_refuel;
static connector_t	*connector_ptr;
static int		 num_connector, max_connector;
static smart_t		*smart_ptr;
static int		 num_smart, max_smart;
static ball_t		*ball_ptr;
static int		 num_ball, max_ball;
static ship_t		*ship_ptr;
static int		 num_ship, max_ship;
static mine_t		*mine_ptr;
static int		 num_mine, max_mine;
static itemtype_t	*itemtype_ptr;
static int		 num_itemtype, max_itemtype;
static shot_t		*shot_ptr;
static int		 num_shot, max_shot;
static ecm_t		*ecm_ptr;
static int		 num_ecm, max_ecm;
static trans_t 		*trans_ptr;
static int		 num_trans, max_trans;
static paused_t		*paused_ptr;
static int		 num_paused, max_paused;
static radar_t		*radar_ptr;
static int		 num_radar, max_radar;
static vcannon_t	*vcannon_ptr;
static int		 num_vcannon, max_vcannon;
static vfuel_t		*vfuel_ptr;
static int		 num_vfuel, max_vfuel;
static vbase_t		*vbase_ptr;
static int		 num_vbase, max_vbase;
static debris_t		*debris_ptr[DEBRIS_TYPES];
static int		 num_debris[DEBRIS_TYPES],
			 max_debris[DEBRIS_TYPES];

#define HANDLE(P,N,M,T)							\
    if (N >= M && ((M <= 0)						\
	? (P = (void *) malloc((M = 1) * sizeof(*P)))			\
	: (P = (void *) realloc(P, (M += M) * sizeof(*P)))) == NULL) {	\
	error("No memory");						\
	N = M = 0;							\
	return -1;							\
    } else								\
    	(P[N++] = T)

#ifndef PAINT_FREE
# define PAINT_FREE	1
#endif
#if PAINT_FREE
# define RELEASE(P, N, M)	(free(P), (M) = 0, (N) = 0)
#else
# define RELEASE(P, N, M)	((N) = 0)
#endif

static long		loops = 0,
			start_loops,
			end_loops;

static XPoint		points[5];
static XGCValues	gcv;

static XPoint		diamond[] = {
    { 0, -DSIZE },
    { DSIZE, -DSIZE },
    { -DSIZE, -DSIZE },
    { -DSIZE, DSIZE },
    { DSIZE, DSIZE }
};

static XRectangle	*rect_ptr[NUM_COLORS];
static int		num_rect[NUM_COLORS], max_rect[NUM_COLORS];
static XArc		*arc_ptr[NUM_COLORS];
static int		num_arc[NUM_COLORS], max_arc[NUM_COLORS];
static XSegment		*seg_ptr[NUM_COLORS];
static int		num_seg[NUM_COLORS], max_seg[NUM_COLORS];

static Pixel		current_foreground;

#define RESET_FG()	(current_foreground = -1)
#define SET_FG(PIXEL)				\
    if ((PIXEL) == current_foreground) ;	\
    else XSetForeground(dpy, gc, current_foreground = (PIXEL))

#define FIND_NAME_WIDTH(other)						\
    if ((other)->name_width == 0) {					\
	(other)->name_len = strlen((other)->name);			\
	(other)->name_width = XTextWidth(gameFont, (other)->name,	\
					 (other)->name_len);		\
    }


static void Rectangle_start(void)
{
    int i;

    for (i = 0; i < NUM_COLORS; i++) {
	num_rect[i] = 0;
    }
}

static void Rectangle_end(void)
{
    int i;

    for (i = 0; i < NUM_COLORS; i++) {
	if (num_rect[i] > 0) {
	    SET_FG(colors[i].pixel);
	    XDrawRectangles(dpy, p_draw, gc,
		rect_ptr[i], num_rect[i]);
	    RELEASE(rect_ptr[i], num_rect[i], max_rect[i]);
	}
    }
}

static int Rectangle_add(int color, int x, int y, int width, int height)
{
    XRectangle		t;

    if (color < 0 || color >= NUM_COLORS) {
	errno = 0;
	error("Bad rectangle color %d", color);
	return -1;
    }
    t.x = x;
    t.y = y;
    t.width = width;
    t.height = height;
    HANDLE(rect_ptr[color], num_rect[color], max_rect[color], t);
    return 0;
}

static void Arc_start(void)
{
    int i;

    for (i = 0; i < NUM_COLORS; i++) {
	num_arc[i] = 0;
    }
}

static void Arc_end(void)
{
    int i;

    for (i = 0; i < NUM_COLORS; i++) {
	if (num_arc[i] > 0) {
	    SET_FG(colors[i].pixel);
	    XDrawArcs(dpy, p_draw, gc,
		arc_ptr[i], num_arc[i]);
	    RELEASE(arc_ptr[i], num_arc[i], max_arc[i]);
	}
    }
}

static int Arc_add(int color,
		   int x, int y,
		   int width, int height,
		   int angle1, int angle2)
{
    XArc t;

    if (color < 0 || color >= NUM_COLORS) {
	errno = 0;
	error("Bad arc color %d", color);
	return -1;
    }
    t.x = x;
    t.y = y;
    t.width = width;
    t.height = height;
    t.angle1 = angle1;
    t.angle2 = angle2;
    HANDLE(arc_ptr[color], num_arc[color], max_arc[color], t);
    return 0;
}

static void Segment_start(void)
{
    int i;

    for (i = 0; i < NUM_COLORS; i++) {
	num_seg[i] = 0;
    }
}

static void Segment_end(void)
{
    int i;

    for (i = 0; i < NUM_COLORS; i++) {
	if (num_seg[i] > 0) {
	    SET_FG(colors[i].pixel);
	    XDrawSegments(dpy, p_draw, gc,
		seg_ptr[i], num_seg[i]);
	    RELEASE(seg_ptr[i], num_seg[i], max_seg[i]);
	}
    }
}

static int Segment_add(int color, int x1, int y1, int x2, int y2)
{
    XSegment t;

    if (color < 0 || color >= NUM_COLORS) {
	errno = 0;
	error("Bad segment color %d", color);
	return -1;
    }
    t.x1 = x1;
    t.y1 = y1;
    t.x2 = x2;
    t.y2 = y2;
    HANDLE(seg_ptr[color], num_seg[color], max_seg[color], t);
    return 0;
}

#ifdef SCROLL
static char *scroll(char *string, int start, int length)
{
    static char str[MAX_SCROLL_LEN];
    int i;
    
    for (i=0; string[i+start] && i<length; i++)
	str[i] = string[i+start];
    str[length] = '\0';

    return (str);
}
#endif

#define METER_WIDTH		200
#define METER_HEIGHT		8

static void Paint_meter(int x, int y, char *title, int val, int max)
{
    const int	mw1_4 = METER_WIDTH/4,
    		mw2_4 = METER_WIDTH/2,
		mw3_4 = 3*METER_WIDTH/4,
		mw4_4 = METER_WIDTH,
    		BORDER = 5;

    SET_FG(colors[RED].pixel);
    XFillRectangle(dpy, p_draw, gc, x+2, y+2,
		   (int)(((METER_WIDTH-3)*val)/max), METER_HEIGHT-3);
    SET_FG(colors[WHITE].pixel);
    Rectangle_add(WHITE, x, y, METER_WIDTH, METER_HEIGHT);

    /* Paint scale levels(?) */
    Segment_add(WHITE, x, y-4,		x, y+METER_HEIGHT+4);
    Segment_add(WHITE, x+mw4_4, y-4,	x+mw4_4, y+METER_HEIGHT+4);
    Segment_add(WHITE, x+mw2_4, y-3,	x+mw2_4, y+METER_HEIGHT+3);
    Segment_add(WHITE, x+mw1_4, y-1,	x+mw1_4, y+METER_HEIGHT+1);
    Segment_add(WHITE, x+mw3_4, y-1,	x+mw3_4, y+METER_HEIGHT+1);
    XDrawString(dpy, p_draw, gc,
		x+METER_WIDTH+BORDER, y+(gameFont->ascent+METER_HEIGHT)/2,
		title, strlen(title));
}

static int wrap(int *x, int *y)
{
    int x1, y1;

    if (*x >= world.x
	&& *x <= world.x + FULL
	&& *y >= world.y
	&& *y <= world.y + FULL) {
	return 1;
    }
    if (BIT(Setup->mode, WRAP_PLAY) && wrappedWorld) {
	if ((wrappedWorld & 1) && *x > FULL) {
	    x1 = *x - Setup->x * BLOCK_SZ;
	} else {
	    x1 = *x;
	}
	if ((wrappedWorld & 2) && *y > FULL) {
	    y1 = *y - Setup->y * BLOCK_SZ;
	} else {
	    y1 = *y;
	}
	if (x1 >= realWorld.x
	    && x1 <= realWorld.x + FULL
	    && y1 >= realWorld.y
	    && y1 <= realWorld.y + FULL) {
	    
	    if ((wrappedWorld & 1) && *x == x1) {
		*x = x1 + Setup->x * BLOCK_SZ;
	    }
	    if ((wrappedWorld & 2) && *y == y1) {
		*y = y1 + Setup->y * BLOCK_SZ;
	    }
	    return 1;
	}
    }
#if 0
    errno = 0;
    error("Object (%d, %d) not in view (%d, %d)",
	  *x, *y, pos.x, pos.y);
#endif
    return 0;
}


void Paint_item_symbol(u_byte type, Drawable d, GC gc, int x, int y)
{
    gcv.stipple = itemBitmaps[type];
    gcv.fill_style = FillStippled;
    gcv.ts_x_origin = x;
    gcv.ts_y_origin = y;
    XChangeGC(dpy, gc,
	      GCStipple|GCFillStyle|GCTileStipXOrigin|GCTileStipYOrigin,
	      &gcv);
    XFillRectangle(dpy, d, gc, x, y, ITEM_SIZE, ITEM_SIZE);
    gcv.fill_style = FillSolid;
    XChangeGC(dpy, gc, GCFillStyle, &gcv);
}


void Paint_item(u_byte type, Drawable d, GC gc, int x, int y)
{
    const int		SIZE = 5*ITEM_SIZE/7 + 1;

    points[0].x = x - SIZE;
    points[0].y = y - SIZE;
    points[1].x = x;
    points[1].y = y + SIZE;
    points[2].x = x + SIZE;
    points[2].y = y - SIZE;
    points[3] = points[0];
    SET_FG(colors[BLUE].pixel);
    XDrawLines(dpy, d, gc, points, 4, CoordModeOrigin);

    SET_FG(colors[RED].pixel);
#if 0
    str[0] = itemtype_ptr[i].type + '0';
    str[1] = '\0';
    XDrawString(dpy, d, gc,
		x - XTextWidth(gameFont, str, 1)/2,
		y + SIZE - 1,
		str, 1);
#endif
    Paint_item_symbol(type, d, gc, x - ITEM_SIZE/2, y - SIZE + 2);
}


static void Paint_shots(void)
{
    int		color, i, j, id, x, y, xs, ys, size;
    char	*p, str[2];

    if (num_itemtype > 0) {
	SET_FG(colors[RED].pixel);
	for (i = 0; i < num_itemtype; i++) {
	    x = itemtype_ptr[i].x;
	    y = itemtype_ptr[i].y;
	    if (wrap(&x, &y))
		Paint_item(itemtype_ptr[i].type, p_draw, gc, X(x), Y(y));
	}
	RELEASE(itemtype_ptr, num_itemtype, max_itemtype);
    }

    if (num_shot > 0) {
	for (i = 0; i < num_shot; i++) {
	    x = shot_ptr[i].x;
	    y = shot_ptr[i].y;
	    if (wrap(&x, &y)) {
		Rectangle_add(shot_ptr[i].color, X(x), Y(y), 2, 2);
	    }
	}
	RELEASE(shot_ptr, num_shot, max_shot);
    }

    if (num_ball > 0) {
	for (i = 0; i < num_ball; i++) {
	    x = ball_ptr[i].x;
	    y = ball_ptr[i].y;
	    id = ball_ptr[i].id;
	    if (wrap(&x, &y)) {
		x = X(x);
		y = Y(y);
		Arc_add(WHITE, x - 10, y - 10, 20, 20, 0, 64*360);
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
		mine_points[0].x = x - 8;
		mine_points[0].y = y - 1;
		SET_FG(colors[BLUE].pixel);
		XFillRectangle(dpy, p_draw, gc,
			       x - 7, y - 2, 15, 5);
		SET_FG(colors[RED].pixel);
		XDrawLines(dpy, p_draw, gc,
			   mine_points, 21, CoordModePrevious);
		/* Arc_add(BLUE, x - 4, y - 4, 8, 8, 0, 64*360); */
	    }
	}
	RELEASE(mine_ptr, num_mine, max_mine);
    }

    size = BIT(dpy_type, DT_HAVE_COLOR) ? 2 : 1;
    for (i = 0; i < DEBRIS_TYPES; i++) {
	if (num_debris[i] > 0) {
	    x = ((i % 3) << 8);
	    y = (((i / 3) % 3) << 8);
	    color = i / 9;
	    y = FULL - 1 - y;
	    for (j = 0; j < num_debris[i]; j++) {
		Rectangle_add(color,
			      x + debris_ptr[i][j].x,
			      y - debris_ptr[i][j].y,
			      size, size);
	    }
	    RELEASE(debris_ptr[i], num_debris[i], max_debris[i]);
	}
    }

    if (num_smart > 0) {
	SET_FG(colors[WHITE].pixel);
	XSetLineAttributes(dpy, gc, 4,
			   LineSolid, CapButt, JoinMiter);
	for (i = 0; i < num_smart; i++) {
	    x = smart_ptr[i].x;
	    y = smart_ptr[i].y;
	    if (wrap(&x, &y)) {
		x = X(x);
		y = Y(y);
		XDrawLine(dpy, p_draw, gc,
			  x, y,
			  (int)(x - tcos(smart_ptr[i].dir) * SMART_SHOT_LEN),
			  (int)(y + tsin(smart_ptr[i].dir) * SMART_SHOT_LEN));
	    }
	}
	XSetLineAttributes(dpy, gc, 0,
			   LineSolid, CapButt, JoinMiter);
	RELEASE(smart_ptr, num_smart, max_smart);
    }
}

static void Paint_ships(void)
{
    int			i, x, y, dir, x0, y0, x1, y1, size;
    unsigned long	mask;
    other_t		*other;
    static int		pauseCharWidth = -1;

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
		XFillRectangle(dpy, p_draw, gc,
			       X(x - half_pause_size),
			       Y(y + half_pause_size),
			       2*half_pause_size+1, 2*half_pause_size+1);
		if (paused_ptr[i].count <= 0 || loops % 10 >= 5) {
		    SET_FG(colors[WHITE].pixel);
		    Rectangle_add(WHITE,
				  X(x - (half_pause_size + 1)),
				  Y(y + (half_pause_size + 1)),
				  2*(half_pause_size+1), 2*(half_pause_size+1));
		    XDrawString(dpy, p_draw, gc,
				X(x - pauseCharWidth/2),
				Y(y - gameFont->ascent/2),
				"P", 1);
		}
	    }
	    RELEASE(paused_ptr, num_paused, max_paused);
	}
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
	SET_FG(colors[WHITE].pixel);
	for (i = 0; i < num_ship; i++) {
	    x = ship_ptr[i].x;
	    y = ship_ptr[i].y;
	    if (wrap(&x, &y)) {
		dir = ship_ptr[i].dir;
		points[0].x = X(x + ships[dir].pts[0].x);
		points[0].y = Y(y + ships[dir].pts[0].y);
		points[1].x = X(x + ships[dir].pts[1].x);
		points[1].y = Y(y + ships[dir].pts[1].y);
		points[2].x = X(x + ships[dir].pts[2].x);
		points[2].y = Y(y + ships[dir].pts[2].y);
		points[3] = points[0];

		/*
		 * Determine if the name of the player should be drawn below
		 * his/her ship.
		 */
		if (BIT(instruments, SHOW_SHIP_NAME)
		    && self != NULL
		    && self->id != ship_ptr[i].id
		    && (other = Other_by_id(ship_ptr[i].id)) != NULL) {
		    FIND_NAME_WIDTH(other);
		    XDrawString(dpy, p_draw, gc,
				X(x - other->name_width / 2),
				Y(y - gameFont->ascent - 15),
				other->name, other->name_len);
		}

		if (ship_ptr[i].cloak == 0) {
		    if (gcv.line_style != LineSolid) {
			gcv.line_style = LineSolid;
			XChangeGC(dpy, gc, GCLineStyle, &gcv);
		    }
		    if (lock_id == ship_ptr[i].id
			&& ship_ptr[i].id != -1
			&& lock_dist != 0) {
			XFillPolygon(dpy, p_draw, gc, points, 4,
				     Convex, CoordModeOrigin);
		    } else {
			XDrawLines(dpy, p_draw, gc, points, 4, 0);
		    }
		}

		if (ship_ptr[i].shield || ship_ptr[i].cloak) {
		    if (gcv.line_style != LineOnOffDash) {
			gcv.line_style = LineOnOffDash;
			mask = GCLineStyle;
#ifndef NO_ROTATING_DASHES
			mask |= GCDashOffset;
#endif
			XChangeGC(dpy, gc, mask, &gcv);
		    }
		    if (ship_ptr[i].cloak) {
			XDrawLines(dpy, p_draw, gc, points, 4, 0);
		    }
		    if (ship_ptr[i].shield) {
			XDrawArc(dpy, p_draw, gc,
				 X(x - 17),
				 Y(y + 17),
				 34, 34, 0, 64 * 360);
		    }
		}
	    }
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
	    for (i = 0; i < num_refuel; i++) {
		x0 = refuel_ptr[i].x0;
		y0 = refuel_ptr[i].y0;
		x1 = refuel_ptr[i].x1;
		y1 = refuel_ptr[i].y1;
		if (wrap(&x0, &y0)
		    && wrap(&x1, &y1)) {
		    XDrawLine(dpy, p_draw, gc,
			      X(x0), Y(y0),
			      X(x1), Y(y1));
		}
	    }
	    RELEASE(refuel_ptr, num_refuel, max_refuel);
	}
	if (num_connector > 0) {
	    for (i = 0; i < num_connector; i++) {
		x0 = connector_ptr[i].x0;
		y0 = connector_ptr[i].y0;
		x1 = connector_ptr[i].x1;
		y1 = connector_ptr[i].y1;
		if (wrap(&x0, &y0)
		    && wrap(&x1, &y1)) {
		    XDrawLine(dpy, p_draw, gc,
			      X(x0), Y(y0),
			      X(x1), Y(y1));
		}
	    }
	    RELEASE(connector_ptr, num_connector, max_connector);
	}
	if (num_trans > 0) {
	    for (i = 0; i < num_trans; i++) {
		x0 = trans_ptr[i].x1;
		y0 = trans_ptr[i].y1;
		x1 = trans_ptr[i].x2;
		y1 = trans_ptr[i].y2;
		if (wrap(&x0, &y0) && wrap(&x1, &y1))
		    XDrawLine(dpy, p_draw, gc, 
			      X(x0), Y(y0), X(x1), Y(y1));
	    }
	    RELEASE(trans_ptr, num_trans, max_trans);
	}
    }

    if (gcv.line_style != LineSolid) {
	gcv.line_style = LineSolid;
	XChangeGC(dpy, gc, GCLineStyle, &gcv);
    }
}


static void Paint_score_objects(void)
{
    int		i, x, y;

    for (i=0; i < MAX_SCORE_OBJECTS; i++) {
	score_object_t*	sobj = &score_objects[i];
	if (sobj->count > 0) {
	    if (sobj->count%3) {
		x = sobj->x * BLOCK_SZ + BLOCK_SZ/2 - sobj->msg_width/2; 
		y = sobj->y * BLOCK_SZ + BLOCK_SZ/2 - gameFont->ascent/2;
		if (wrap(&x, &y)) {
		    SET_FG(colors[RED].pixel);
		    XDrawString(dpy, p_draw, gc,
				X(x), 
				Y(y),
				sobj->msg, 
				sobj->msg_len);
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


static void Paint_meters(void)
{
    if (BIT(instruments, SHOW_FUEL_METER))
	Paint_meter(10, 10, "Fuel", fuelSum, fuelMax);
    if (BIT(instruments, SHOW_POWER_METER) || control_count)
     	Paint_meter(10, 40, "Power", power, MAX_PLAYER_POWER);
    if (BIT(instruments, SHOW_TURNSPEED_METER) || control_count)
	Paint_meter(10, 60, "Turnspeed", turnspeed, MAX_PLAYER_TURNSPEED);
    if (control_count > 0)
	control_count--;
    if (BIT(instruments, SHOW_PACKET_SIZE_METER))
	Paint_meter(10, 80, "Packet",
		   (packet_size >= 4096) ? 4096 : packet_size, 4096);
    if (BIT(instruments, SHOW_PACKET_LOSS_METER))
	Paint_meter(10, 100, "Loss", packet_loss, FPS);
    if (BIT(instruments, SHOW_PACKET_DROP_METER))
	Paint_meter(10, 120, "Drop", packet_drop, FPS);

    if (destruct > 0)
	Paint_meter((FULL-300)/2 -32, 3*FULL/4,
		   "Self destructing", destruct, 150);
    if (shutdown_count >= 0)
	Paint_meter((FULL-300)/2 -32, 4*FULL/5,
		   "SHUTDOWN", shutdown_count, shutdown_delay);
}


static void Paint_lock(int hud_pos_x, int hud_pos_y)
{
    const int	BORDER = 2;
    other_t	*target;
    char	str[50];
    static int	warningCount;

    /*
     * Display direction arrow and miscellaneous target information.
     */
    if ((target = Other_by_id(lock_id)) == NULL) {
	return;
    }
    FIND_NAME_WIDTH(target);
    XDrawString(dpy, p_draw, gc,
		hud_pos_x - target->name_width / 2,
		hud_pos_y - HUD_SIZE+HUD_OFFSET - gameFont->descent - BORDER,
		target->name, target->name_len);
    if (lock_dist != 0) {
	sprintf(str, "%03d", lock_dist / BLOCK_SZ);
	XDrawString(dpy, p_draw, gc,
		    hud_pos_x + HUD_SIZE - HUD_OFFSET + BORDER,
		    hud_pos_y - HUD_SIZE+HUD_OFFSET
		    - gameFont->descent - BORDER,
		    str, 3);
	if (lock_dist > WARNING_DISTANCE || warningCount++ % 2 == 0) {
	    int size = 10000 / (800 + lock_dist);
	    if (size == 0) {
		size = 1;
	    }
	    if (self != NULL
		&& self->team == target->team
		&& BIT(Setup->mode, TEAM_PLAY))
		Arc_add(BLUE,
			(int)(hud_pos_x + HUD_SIZE * 0.6 * tcos(lock_dir)
			      - size * 0.5),
			(int)(hud_pos_y - HUD_SIZE * 0.6 * tsin(lock_dir)
			      - size * 0.5),
			size, size, 0, 64*360);
	    else
		XFillArc(dpy, p_draw, gc,
			 (int)(hud_pos_x + HUD_SIZE * 0.6 * tcos(lock_dir)
			       - size * 0.5),
			 (int)(hud_pos_y - HUD_SIZE * 0.6 * tsin(lock_dir)
			       - size * 0.5),
			 size, size, 0, 64*360);
	}
    }
}


static void Paint_HUD(void)
{
    const int BORDER = 3;
    int vert_pos, horiz_pos, size;
    char str[50];
    int hud_pos_x;
    int hud_pos_y;
    int	i, j, maxWidth = -1;
    static int vertSpacing = -1;
    
    /* 
     * Show speed pointer
     */
    if (ptr_move_fact != 0.0
	&& selfVisible != 0
	&& (vel.x != 0 || vel.y != 0)) {
        Segment_add(RED,
		    CENTER,
		    CENTER,
		    CENTER - ptr_move_fact*vel.x,
		    CENTER + ptr_move_fact*vel.y);
    }

    if (!BIT(instruments, SHOW_HUD_INSTRUMENTS)) {
	return;
    }

    /*
     * Display the HUD
     */
    SET_FG(colors[BLUE].pixel);

    hud_pos_x = CENTER - hud_move_fact*vel.x;
    hud_pos_y = CENTER + hud_move_fact*vel.y;

    /* HUD frame */
    gcv.line_style = LineOnOffDash;
    XChangeGC(dpy, gc, GCLineStyle | GCDashOffset, &gcv);
    
    if (BIT(instruments, SHOW_HUD_HORIZONTAL)) {
	XDrawLine(dpy, p_draw, gc,
		  hud_pos_x-HUD_SIZE, hud_pos_y-HUD_SIZE+HUD_OFFSET,
		  hud_pos_x+HUD_SIZE, hud_pos_y-HUD_SIZE+HUD_OFFSET);
	XDrawLine(dpy, p_draw, gc,
		  hud_pos_x-HUD_SIZE, hud_pos_y+HUD_SIZE-HUD_OFFSET,
		  hud_pos_x+HUD_SIZE, hud_pos_y+HUD_SIZE-HUD_OFFSET);
    }
    if (BIT(instruments, SHOW_HUD_VERTICAL)) {
	XDrawLine(dpy, p_draw, gc,
		  hud_pos_x-HUD_SIZE+HUD_OFFSET, hud_pos_y-HUD_SIZE, 
		  hud_pos_x-HUD_SIZE+HUD_OFFSET, hud_pos_y+HUD_SIZE);
	XDrawLine(dpy, p_draw, gc,
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

    for (i=0; i<NUM_ITEMS; i++) {
	int num;
	switch (i) {
	case ITEM_ROCKET_PACK:
	    num = numRockets;
	    break;
	case ITEM_CLOAKING_DEVICE:
	    num = numCloaks;
	    break;
	case ITEM_WIDEANGLE_SHOT:
	    num = numFrontShots;
	    break;
	case ITEM_BACK_SHOT:
	    num = numBackShots;
	    break;
	case ITEM_MINE_PACK:
	    num = numMines;
	    break;
	case ITEM_SENSOR_PACK:
	    num = numSensors;
	    break;
	case ITEM_TANK:
	    num = numTanks;
	    break;
	case ITEM_ECM:
	    num = numEcms;
	    break;
	case ITEM_AFTERBURNER:
	    num = numAfterburners;
	    break;
	case ITEM_TRANSPORTER:
	    num = numTransporters;
	    break;
	case ITEM_ENERGY_PACK:
	default:
	    num = 0;
	    break;
	}
	if (num > 0) {
	    int len, width;

	    /* Paint item symbol */
	    Paint_item_symbol(i, p_draw, gc, horiz_pos - ITEM_SIZE, vert_pos);

	    /* Paint item count */
	    sprintf(str, "%d", num);
	    len = strlen(str);
	    width = XTextWidth(gameFont, str, len);
	    XDrawString(dpy, p_draw, gc,
			horiz_pos - ITEM_SIZE - BORDER - width,
			vert_pos + ITEM_SIZE/2 + gameFont->ascent/2,
			str, len);

	    maxWidth = MAX(maxWidth, width + BORDER + ITEM_SIZE);
	    vert_pos += vertSpacing;

	    if (vert_pos+vertSpacing > hud_pos_y+HUD_SIZE-HUD_OFFSET-BORDER) {
		vert_pos = hud_pos_y - HUD_SIZE+HUD_OFFSET + BORDER;
		horiz_pos -= maxWidth + 2*BORDER;
		maxWidth = -1;
	    }
	}
    }

    /* Fuel notify, HUD meter on */
    if (fuelCount || fuelSum < fuelLevel3) {
 	sprintf(str, "%04d", fuelSum);
 	XDrawString(dpy, p_draw, gc,
		    hud_pos_x + HUD_SIZE-HUD_OFFSET+BORDER,
 		    hud_pos_y + HUD_SIZE-HUD_OFFSET+BORDER + gameFont->ascent,
		    str, strlen(str));
	if (numTanks) {
	    if (fuelCurrent == 0)
		strcpy(str,"M ");
	    else
		sprintf(str, "T%d", fuelCurrent);
 	    XDrawString(dpy, p_draw, gc,
			hud_pos_x + HUD_SIZE-HUD_OFFSET + BORDER,
 		        hud_pos_y + HUD_SIZE-HUD_OFFSET + BORDER
			+ gameFont->descent + 2*gameFont->ascent,
			str, strlen(str));
	}
    }

    /* Update the lock display */
    Paint_lock(hud_pos_x, hud_pos_y);

    /* Draw last score on hud if it is an message attached to it */
    for (i=0, j=0; i < MAX_SCORE_OBJECTS; i++) {
	score_object_t*	sobj
	    = &score_objects[(i+score_object)%MAX_SCORE_OBJECTS];
	if (sobj->hud_msg_len > 0) {
	    XDrawString(dpy, p_draw, gc,
			hud_pos_x - sobj->hud_msg_width/2,
			hud_pos_y + HUD_SIZE-HUD_OFFSET + BORDER
			+ gameFont->ascent
			+ j++ * (gameFont->ascent + gameFont->descent),
			sobj->hud_msg, sobj->hud_msg_len);
	}
    }

    /* Fuel gauge, must be last */
    if (!((fuelCount)
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

    Rectangle_add(BLUE,
		  hud_pos_x + HUD_SIZE - HUD_OFFSET + FUEL_GAUGE_OFFSET - 1,
		  hud_pos_y - HUD_SIZE + HUD_OFFSET + FUEL_GAUGE_OFFSET - 1,
		  HUD_OFFSET - (2*FUEL_GAUGE_OFFSET) + 3,
		  HUD_FUEL_GAUGE_SIZE + 3);

    size = (HUD_FUEL_GAUGE_SIZE * fuelSum) / fuelMax;
    XFillRectangle(dpy, p_draw, gc,
		   hud_pos_x + HUD_SIZE - HUD_OFFSET + FUEL_GAUGE_OFFSET + 1,
		   hud_pos_y - HUD_SIZE + HUD_OFFSET + FUEL_GAUGE_OFFSET
		   + HUD_FUEL_GAUGE_SIZE - size + 1,
		   HUD_OFFSET - (2*FUEL_GAUGE_OFFSET), size);
}

static void Paint_messages(void)
{
    int i, y;
    const int	BORDER = 10,
		SPACING = messageFont->ascent+messageFont->descent+1;
    
    for (y = FULL - messageFont->descent - BORDER, i = 0;
	 i < MAX_MSGS;
	 i++, y -= SPACING) {
	if (Msg[i]->life-- > MSG_FLASH)
	    XSetForeground(dpy, messageGC, colors[RED].pixel);
	else if (Msg[i]->life > 0)
	    XSetForeground(dpy, messageGC, colors[WHITE].pixel);
	else {
	    Msg[i]->txt[0] = '\0';
	    Msg[i]->len = 0;
	    continue;
	}
	XDrawString(dpy, p_draw, messageGC,
		    BORDER, y,
		    Msg[i]->txt, Msg[i]->len);
    }
}


void Add_message(char *message)
{
    int i;
    message_t *tmp;
    
    
    tmp = Msg[MAX_MSGS-1];
    for (i=MAX_MSGS-1; i>0; i--)
	Msg[i] = Msg[i-1];
    
    Msg[0] = tmp;
    
    Msg[0]->life = MSG_DURATION;
    strcpy(Msg[0]->txt, message);
    Msg[0]->len = strlen(message);
}


static void Paint_radar(void)
{
    int			i, x, y, x1, y1, xw, yw;
    const float		xf = 256.0f / (float)(Setup->x * BLOCK_SZ),
    			yf = (float)RadarHeight / (float)(Setup->y * BLOCK_SZ);

    if (radar_exposed == false) {
	return;
    }
    if (s_radar != p_radar) {
	/* Draw static radar onto radar */
	XCopyArea(dpy, s_radar, p_radar, gc,
		  0, 0, 256, RadarHeight, 0, 0);
    } else {
	/* Clear radar */
	XSetForeground(dpy, radarGC, colors[BLACK].pixel);
	XFillRectangle(dpy, p_radar,
		       radarGC, 0, 0, 256, RadarHeight);
    }
    
    XSetForeground(dpy, radarGC, colors[WHITE].pixel);

    /* Checkpoint */
    if (BIT(Setup->mode, TIMING)) {
	Check_pos_by_index(nextCheckPoint, &x, &y);
	x = (int)(x * BLOCK_SZ * xf + 0.5);
	y = RadarHeight - (int)(y * BLOCK_SZ * yf + 0.5) + DSIZE - 1;
	diamond[0].x = x;
	diamond[0].y = y;
	XDrawLines(dpy, p_radar, radarGC,
		   diamond, 5, CoordModePrevious);
    }
    if (selfVisible != 0 && loops % 16 < 13) {
	x = (int)(pos.x * xf + 0.5);
	y = RadarHeight - (int)(pos.y * yf + 0.5) - 1;
	x1 = (int)(x + 8 * tcos(heading));
	y1 = (int)(y - 8 * tsin(heading));
	XDrawLine(dpy, p_radar, radarGC,
		  x, y, x1, y1);
	if (BIT(Setup->mode, WRAP_PLAY)) {
	    xw = x1 - (x1 + 256) % 256;
	    yw = y1 - (y1 + RadarHeight) % RadarHeight;
	    if (xw != 0) {
		XDrawLine(dpy, p_radar, radarGC,
			  x - xw, y, x1 - xw, y1);
	    }
	    if (yw != 0) {
		XDrawLine(dpy, p_radar, radarGC,
			  x, y - yw, x1, y1 - yw);
		if (xw != 0) {
		    XDrawLine(dpy, p_radar, radarGC,
			      x - xw, y - yw, x1 - xw, y1 - yw);
		}
	    }
	}
    }
    for (i = 0; i<num_radar; i++) {
	x = (int)(radar_ptr[i].x * xf + 0.5);
	y = RadarHeight - (int)(radar_ptr[i].y * yf + 0.5) - 1;
	XFillRectangle(dpy, p_radar, radarGC,
		       x-1, y-1, 3, 3);
	if (BIT(Setup->mode, WRAP_PLAY)) {
	    xw = (x - 1 < 0) ? -256 : (x + 1 >= 256) ? 256 : 0;
	    yw = (y - 1 < 0) ? -RadarHeight
			     : (y + 1 >= RadarHeight) ? RadarHeight : 0;
	    if (xw != 0) {
		XFillRectangle(dpy, p_radar, radarGC,
			       x-1 - xw, y-1, 3, 3);
	    }
	    if (yw != 0) {
		XFillRectangle(dpy, p_radar, radarGC,
			       x-1, y-1 - yw, 3, 3);
		if (xw != 0) {
		    XFillRectangle(dpy, p_radar, radarGC,
				   x-1 - xw, y-1 - yw, 3, 3);
		}
	    }
	}
    }
}

static void Paint_vcannon(void)
{
    int			i, x, y, type;

    if (num_vcannon > 0) {
	SET_FG(colors[WHITE].pixel);
	for (i = 0; i < num_vcannon; i++) {
	    type = vcannon_ptr[i].type;
	    x = vcannon_ptr[i].x;
	    y = vcannon_ptr[i].y;
	    switch (type) {
	    case SETUP_CANNON_UP:
		points[0].x = X(x);
		points[0].y = Y(y);
		points[1].x = X(x+BLOCK_SZ);
		points[1].y = Y(y);
		points[2].x = X(x+BLOCK_SZ/2);
		points[2].y = Y(y+BLOCK_SZ/3);
		break;
	    case SETUP_CANNON_DOWN:
		points[0].x = X(x);
		points[0].y = Y(y+BLOCK_SZ);
		points[1].x = X(x+BLOCK_SZ);
		points[1].y = Y(y+BLOCK_SZ);
		points[2].x = X(x+BLOCK_SZ/2);
		points[2].y = Y(y+2*BLOCK_SZ/3);
		break;
	    case SETUP_CANNON_RIGHT:
		points[0].x = X(x);
		points[0].y = Y(y);
		points[1].x = X(x);
		points[1].y = Y(y+BLOCK_SZ);
		points[2].x = X(x+BLOCK_SZ/3);
		points[2].y = Y(y+BLOCK_SZ/2);
		break;
	    case SETUP_CANNON_LEFT:
		points[0].x = X(x+BLOCK_SZ);
		points[0].y = Y(y);
		points[1].x = X(x+BLOCK_SZ);
		points[1].y = Y(y+BLOCK_SZ);
		points[2].x = X(x+2*BLOCK_SZ/3);
		points[2].y = Y(y+BLOCK_SZ/2);
		break;
	    default:
		errno = 0;
		error("Unknown cannon type %d", type);
		continue;
	    }
	    points[3] = points[0];
	    XDrawLines(dpy, p_draw, gc, points, 4, 0);
	}
	RELEASE(vcannon_ptr, num_vcannon, max_vcannon);
    }
}

static void Paint_vfuel(void)
{
#define FUEL_BORDER 2

    int			i, x, y, size;
    long		fuel;
    char		s[2];

    if (num_vfuel > 0) {
	SET_FG(colors[RED].pixel);
	for (i = 0; i < num_vfuel; i++) {
	    x = vfuel_ptr[i].x;
	    y = vfuel_ptr[i].y;
	    fuel = vfuel_ptr[i].fuel;
	    size = (BLOCK_SZ - 2*FUEL_BORDER) * fuel / MAX_STATION_FUEL;
	    XFillRectangle(dpy, p_draw, gc,
			   X(x + FUEL_BORDER),
			   Y(y + FUEL_BORDER + size),
			   BLOCK_SZ - 2*FUEL_BORDER + 1,
			   size + 1);
	}
	/* Draw F in fuel cells */
	s[0] = 'F'; s[1] = '\0';
	XSetFunction(dpy, gc, GXxor);
	SET_FG(colors[BLACK].pixel ^ colors[RED].pixel);
	for (i = 0; i < num_vfuel; i++) {
	    x = vfuel_ptr[i].x;
	    y = vfuel_ptr[i].y;
	    XDrawString(dpy, p_draw, gc,
			X(x + BLOCK_SZ/2 - XTextWidth(gameFont, s, 1)/2),
			Y(y + BLOCK_SZ/2 - gameFont->ascent/2),
			s, 1);
	}
	XSetFunction(dpy, gc, GXcopy);
	RELEASE(vfuel_ptr, num_vfuel, max_vfuel);
    }
}

static void Paint_vbase(void)
{
    const int	BORDER = 4;					/* in pixels */
    int		i, id, x, y, xi, yi, type;
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
	    if (!BIT(Setup->mode, TEAM_PLAY)) {
		s[0]='\0';
	    } else {
		s[0] = '0' + Base_team_by_pos(xi, yi);
		s[1] = ' ';
		s[2] = '\0';
	    }
	    switch (type) {
	    case SETUP_BASE_UP:
		Segment_add(WHITE,
			    X(x), Y(y-1),
			    X(x+BLOCK_SZ), Y(y-1));
		y -= BORDER + gameFont->ascent;
		if (s[0]) {
		    XDrawString(dpy, p_draw, gc,
				X(x), Y(y),
				s, 2);
		    x += XTextWidth(gameFont, s, 2);
		}
		if ((id = Base_id_by_pos(xi, yi)) != -1
		    && (other = Other_by_id(id)) != NULL) {
		    FIND_NAME_WIDTH(other);
		    XDrawString(dpy, p_draw, gc,
				X(x), Y(y),
				other->name, other->name_len);
		}
		break;
	    case SETUP_BASE_DOWN:
		Segment_add(WHITE,
			    X(x), Y(y+BLOCK_SZ+1),
			    X(x+BLOCK_SZ), Y(y+BLOCK_SZ+1));
		y += BORDER + BLOCK_SZ;
		if (s[0]) {
		    XDrawString(dpy, p_draw, gc,
				X(x), Y(y),
				s, 2);
		    x += XTextWidth(gameFont, s, 2);
		}
		if ((id = Base_id_by_pos(xi, yi)) != -1
		    && (other = Other_by_id(id)) != NULL) {
		    FIND_NAME_WIDTH(other);
		    XDrawString(dpy, p_draw, gc,
				X(x), Y(y),
				other->name, other->name_len);
		}
		break;
	    case SETUP_BASE_LEFT:
		Segment_add(WHITE,
			    X(x+BLOCK_SZ+1), Y(y+BLOCK_SZ),
			    X(x+BLOCK_SZ+1), Y(y));
		x += BLOCK_SZ + BORDER;
		y += BLOCK_SZ/2 - gameFont->ascent/2;
		if (s[0]) {
		    XDrawString(dpy, p_draw, gc,
				X(x), Y(y),
				s, 2);
		    x += XTextWidth(gameFont, s, 2);
		}
		if ((id = Base_id_by_pos(xi, yi)) != -1
		    && (other = Other_by_id(id)) != NULL) {
		    FIND_NAME_WIDTH(other);
		    XDrawString(dpy, p_draw, gc,
				X(x), Y(y),
				other->name, other->name_len);
		}
		break;
	    case SETUP_BASE_RIGHT:
		Segment_add(WHITE,
			    X(x-1), Y(y+BLOCK_SZ),
			    X(x-1), Y(y));
    		y += BLOCK_SZ/2 - gameFont->ascent/2;
		x -= BORDER;
		if (s[0]) {
		    x -= XTextWidth(gameFont, s, 2);
		    s[1] = s[0];
		    s[0] = ' ';
		    XDrawString(dpy, p_draw, gc,
				X(x), Y(y),
				s, 2);
		}
		if ((id = Base_id_by_pos(xi, yi)) != -1
		    && (other = Other_by_id(id)) != NULL) {
		    FIND_NAME_WIDTH(other);
		    x -= other->name_width;
		    XDrawString(dpy, p_draw, gc,
				X(x), Y(y),
				other->name, other->name_len);
		}
		break;
	    default:
		errno = 0;
		error("Wrong direction of base.");
		continue;
	    }
	}
	RELEASE(vbase_ptr, num_vbase, max_vbase);
    }
}

static void Paint_world(void)
{
    int xi, yi, xb, yb, size, fuel, color;
    int rxb, ryb;
    int xdot;
    int ydot;
    int x, y;
    static const int WS_PR_SC=1+(float)FULL/BLOCK_SZ;
    static const int INSIDE_WS=BLOCK_SZ-2;
    static int wormDrawCount;
    int type;
    int sx = 0, sy = 0;
    char s[2];
    static bool blue_init = false;
    static unsigned char blue[256];


    if (blue_init == false) {
	memset(blue, 0, sizeof blue);
	blue[SETUP_FILLED] = BLUE_LEFT | BLUE_UP | BLUE_RIGHT | BLUE_DOWN;
	blue[SETUP_FILLED_NO_DRAW] = blue[SETUP_FILLED];
	blue[SETUP_FUEL] = blue[SETUP_FILLED];
	blue[SETUP_REC_RU] = BLUE_RIGHT | BLUE_UP;
	blue[SETUP_REC_RD] = BLUE_RIGHT | BLUE_DOWN;
	blue[SETUP_REC_LU] = BLUE_LEFT | BLUE_UP;
	blue[SETUP_REC_LD] = BLUE_LEFT | BLUE_DOWN;
	blue_init = true;
    }
    
    wormDrawCount = (wormDrawCount + 1) & 7;

    xb = (world.x/BLOCK_SZ);
    yb = (world.y/BLOCK_SZ);
    if (!BIT (Setup->mode, WRAP_PLAY)) {
	if (xb < 0)
	    sx = -xb;
	if (yb < 0)
	    sy = -yb;
	if (world.x < 0) {
	    Segment_add(BLUE,
			X(0), Y(0),
			X(0), Y(Setup->y * BLOCK_SZ));
	}
	if (world.x + FULL >= Setup->x * BLOCK_SZ) {
	    Segment_add(BLUE,
			X(Setup->x * BLOCK_SZ), Y(0),
			X(Setup->x * BLOCK_SZ), Y(Setup->y * BLOCK_SZ));
	}
	if (world.y < 0) {
	    Segment_add(BLUE,
			X(0), Y(0),
			X(Setup->x * BLOCK_SZ), Y(0));
	}
	if (world.y + FULL >= Setup->y * BLOCK_SZ) {
	    Segment_add(BLUE,
			X(0), Y(Setup->y * BLOCK_SZ),
			X(Setup->x * BLOCK_SZ), Y(Setup->y * BLOCK_SZ));
	}
    }

    for (rxb = sx; rxb <= WS_PR_SC; rxb++) {
	xi = xb + rxb;
	if (xi == Setup->x) {
	    if (!BIT(Setup->mode, WRAP_PLAY))
		break;
	    xdot = 1;
	} else
	    xdot = 0;
	if (xi >= Setup->x)
	    xi -= Setup->x;

	for (ryb = sy; ryb <= WS_PR_SC; ryb++) {
	    yi = ryb + yb;

	    if (yi == Setup->y) {
		if (!BIT(Setup->mode, WRAP_PLAY))
		    break;
		ydot = 1;
	    }
	    else
		ydot = 0;

	    if (yi >= Setup->y)
		yi -= Setup->y;

	    x = (xb + rxb) * BLOCK_SZ;
	    y = (yb + ryb) * BLOCK_SZ;

	    switch (type = Setup->map_data[xi * Setup->y + yi]) {
		
	    case SETUP_FUEL:
		fuel = Fuel_by_pos(xi, yi);
		Handle_vfuel(x, y, fuel);
		/*FALLTHROUGH*/

	    case SETUP_FILLED:
	    case SETUP_FILLED_NO_DRAW:
		if ((xi == 0)
		    ? (!BIT(Setup->mode, WRAP_PLAY) ||
			!(blue[Setup->map_data[(Setup->x - 1) * Setup->y + yi]]
			    & BLUE_RIGHT))
		    : !(blue[Setup->map_data[(xi - 1) * Setup->y + yi]]
			& BLUE_RIGHT))
		    Segment_add(BLUE,
				X(x),
				Y(y),
				X(x),
				Y(y+BLOCK_SZ));
		if ((yi == 0)
		    ? (!BIT(Setup->mode, WRAP_PLAY) ||
			!(blue[Setup->map_data[xi * Setup->y + Setup->y - 1]]
			    & BLUE_UP))
		    : !(blue[Setup->map_data[xi * Setup->y + (yi - 1)]]
			& BLUE_UP))
		    Segment_add(BLUE,
				X(x),
				Y(y),
				X(x+BLOCK_SZ),
				Y(y));
		if (!BIT(instruments, SHOW_OUTLINE_WORLD)
		    || ((xi == Setup->x - 1)
			? (!BIT(Setup->mode, WRAP_PLAY)
			   || !(blue[Setup->map_data[yi]]
				& BLUE_LEFT))
			: !(blue[Setup->map_data[(xi + 1) * Setup->y + yi]]
			    & BLUE_LEFT)))
		    Segment_add(BLUE,
				X(x+BLOCK_SZ),
				Y(y),
				X(x+BLOCK_SZ),
				Y(y+BLOCK_SZ));
		if (!BIT(instruments, SHOW_OUTLINE_WORLD)
		    || ((yi == Setup->y - 1)
			? (!BIT(Setup->mode, WRAP_PLAY)
			   || !(blue[Setup->map_data[xi * Setup->y]]
				& BLUE_DOWN))
			: !(blue[Setup->map_data[xi * Setup->y + (yi + 1)]]
			    & BLUE_DOWN)))
		    Segment_add(BLUE,
				X(x),
				Y(y+BLOCK_SZ),
				X(x+BLOCK_SZ),
				Y(y+BLOCK_SZ));
		break;
		
	    case SETUP_REC_LU:
		if (xi == 0
		    ? (!BIT(Setup->mode, WRAP_PLAY) ||
			!(blue[Setup->map_data[(Setup->x - 1) * Setup->y + yi]]
			    & BLUE_RIGHT))
		    : !(blue[Setup->map_data[(xi - 1) * Setup->y + yi]]
			& BLUE_RIGHT))
		    Segment_add(BLUE,
				X(x),
				Y(y),
				X(x),
				Y(y+BLOCK_SZ));
		if (!BIT(instruments, SHOW_OUTLINE_WORLD)
		    || ((yi == Setup->y - 1)
			? (!BIT(Setup->mode, WRAP_PLAY)
			   || !(blue[Setup->map_data[xi * Setup->y]]
				& BLUE_DOWN))
			: !(blue[Setup->map_data[xi * Setup->y + (yi + 1)]]
			    & BLUE_DOWN)))
		    Segment_add(BLUE,
				X(x),
				Y(y+BLOCK_SZ),
				X(x+BLOCK_SZ),
				Y(y+BLOCK_SZ));
		Segment_add(BLUE,
			    X(x),
			    Y(y),
			    X(x+BLOCK_SZ),
			    Y(y+BLOCK_SZ));
		break;
		
	    case SETUP_REC_RU:
		if (!BIT(instruments, SHOW_OUTLINE_WORLD)
		    || ((xi == Setup->x - 1)
			? (!BIT(Setup->mode, WRAP_PLAY)
			   || !(blue[Setup->map_data[yi]]
				& BLUE_LEFT))
			: !(blue[Setup->map_data[(xi + 1) * Setup->y + yi]]
			    & BLUE_LEFT)))
		    Segment_add(BLUE,
				X(x+BLOCK_SZ),
				Y(y),
				X(x+BLOCK_SZ),
				Y(y+BLOCK_SZ));
		if (!BIT(instruments, SHOW_OUTLINE_WORLD)
		    || ((yi == Setup->y - 1)
			? (!BIT(Setup->mode, WRAP_PLAY)
			   || !(blue[Setup->map_data[xi * Setup->y]]
				& BLUE_DOWN))
			: !(blue[Setup->map_data[xi * Setup->y + (yi + 1)]]
			    & BLUE_DOWN)))
		    Segment_add(BLUE,
				X(x),
				Y(y+BLOCK_SZ),
				X(x+BLOCK_SZ),
				Y(y+BLOCK_SZ));
		Segment_add(BLUE,
			    X(x),
			    Y(y+BLOCK_SZ),
			    X(x+BLOCK_SZ),
			    Y(y));
		break;
		
	    case SETUP_REC_LD:
		if ((xi == 0)
		    ? (!BIT(Setup->mode, WRAP_PLAY) ||
			!(blue[Setup->map_data[(Setup->x - 1) * Setup->y + yi]]
			    & BLUE_RIGHT))
		    : !(blue[Setup->map_data[(xi - 1) * Setup->y + yi]]
			& BLUE_RIGHT))
		    Segment_add(BLUE,
				X(x),
				Y(y),
				X(x),
				Y(y+BLOCK_SZ));
		if ((yi == 0)
		    ? (!BIT(Setup->mode, WRAP_PLAY) ||
			!(blue[Setup->map_data[xi * Setup->y + Setup->y - 1]]
			    & BLUE_UP))
		    : !(blue[Setup->map_data[xi * Setup->y + (yi - 1)]]
			& BLUE_UP))
		    Segment_add(BLUE,
				X(x),
				Y(y),
				X(x+BLOCK_SZ),
				Y(y));
		Segment_add(BLUE,
			    X(x),
			    Y(y+BLOCK_SZ),
			    X(x+BLOCK_SZ),
			    Y(y));
		break;
		
	    case SETUP_REC_RD:
		if (!BIT(instruments, SHOW_OUTLINE_WORLD)
		    || ((xi == Setup->x - 1)
			? (!BIT(Setup->mode, WRAP_PLAY)
			   || !(blue[Setup->map_data[yi]]
				& BLUE_LEFT))
			: !(blue[Setup->map_data[(xi + 1) * Setup->y + yi]]
			    & BLUE_LEFT)))
		    Segment_add(BLUE,
				X(x+BLOCK_SZ),
				Y(y),
				X(x+BLOCK_SZ),
				Y(y+BLOCK_SZ));
		if ((yi == 0)
		    ? (!BIT(Setup->mode, WRAP_PLAY) ||
			!(blue[Setup->map_data[xi * Setup->y + Setup->y - 1]]
			    & BLUE_UP))
		    : !(blue[Setup->map_data[xi * Setup->y + (yi - 1)]]
			& BLUE_UP))
		    Segment_add(BLUE,
				X(x),
				Y(y),
				X(x+BLOCK_SZ),
				Y(y));
		Segment_add(BLUE,
			    X(x),
			    Y(y),
			    X(x+BLOCK_SZ),
			    Y(y+BLOCK_SZ));
		break;
		
	    case SETUP_CHECK:
		SET_FG(colors[BLUE].pixel);
		points[0].x = X(x+(BLOCK_SZ/2));
		points[0].y = Y(y);
		points[1].x = X(x);
		points[1].y = Y(y+BLOCK_SZ/2);
		points[2].x = X(x+BLOCK_SZ/2);
		points[2].y = Y(y+BLOCK_SZ);
		points[3].x = X(x+BLOCK_SZ);
		points[3].y = Y(y+(BLOCK_SZ/2));
		points[4] = points[0];

		if (Check_index_by_pos(xi, yi) == nextCheckPoint) {
		    XFillPolygon(dpy, p_draw, gc,
				 points, 5, Convex, CoordModeOrigin);
		} else {
		    XDrawLines(dpy, p_draw, gc,
			       points, 5, 0); 
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
			INSIDE_WS, INSIDE_WS, 0, 64*360);
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
			INSIDE_WS, INSIDE_WS, 0, 64*360);
		Segment_add(RED,
			    X(x+5),
			    Y(y+BLOCK_SZ/2),
			    X(x+BLOCK_SZ-5),
			    Y(y+BLOCK_SZ/2));
		break;
		
	    case SETUP_WORM_IN:
	    case SETUP_WORM_NORMAL:
		{
		    static int wormOffset[8][3] = {
			10, 10, 10,
			5, 10, 10,
			0, 10, 10,
			0, 5, 10,
			0, 0, 10,
			5, 0, 10,
			10, 0, 10,
			10, 5, 10,
		    };
#define _O 	wormOffset[wormDrawCount]
#define ARC(_x, _y, _w)						\
    Arc_add(RED,						\
	    X(x) + (_x),					\
	    Y(y + BLOCK_SZ) + (_y),				\
	    INSIDE_WS - (_w), INSIDE_WS - (_w), 0, 64 * 360)

		    SET_FG(colors[RED].pixel);
		    ARC(0, 0, 0);
		    ARC(_O[0], _O[1], _O[2]);
		    ARC(_O[0] * 2, _O[1] * 2, _O[2] * 2);
		    break;
		}

	    case SETUP_CANNON_UP:
	    case SETUP_CANNON_DOWN:
	    case SETUP_CANNON_RIGHT:
	    case SETUP_CANNON_LEFT:
		if (Cannon_dead_time_by_pos(xi, yi) <= 0) {
		    Handle_vcannon(x, y, type);
		    break;
		}
		/*FALLTHROUGH*/

	    case SETUP_SPACE: {
                int bg_point_dist = map_point_distance;

		if (xdot || ydot
		    || (bg_point_dist != 0
			&& (xi%bg_point_dist) == 0
			&& (yi%bg_point_dist) == 0)) {
		    Rectangle_add(BLUE,
				  X((xb+rxb) * BLOCK_SZ + BLOCK_SZ / 2),
				  Y((yb+ryb) * BLOCK_SZ + BLOCK_SZ / 2),
				  1, 1);
		}

#if 0	/* def DRAW_GRAV_VECTORS */
		SET_FG(colors[RED].pixel);
		XDrawLine(dpy, p_draw, gc,
			  X(x+BLOCK_SZ/2),
			  Y(y+BLOCK_SZ/2),
			  X(x+BLOCK_SZ/2 + 50*World.gravity[xi][yi].x),
			  Y(y+BLOCK_SZ/2 + 50*World.gravity[xi][yi].y));
		SET_FG(colors[WHITE].pixel);
		XDrawPoint(dpy, p_draw, gc,
			   X(x+BLOCK_SZ/2),
			   Y(y+BLOCK_SZ/2));
#endif
		break;
            }

	    case SETUP_BASE_UP:
	    case SETUP_BASE_RIGHT:
	    case SETUP_BASE_DOWN:
	    case SETUP_BASE_LEFT:
		Handle_vbase(x, y, xi, yi, type);
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
		    int 	a1,a2,b1,b2;
		    int 	damage;
		    
		    if (Target_alive(xi, yi, &damage) != 0)
			break;

		    if (team == type - SETUP_TARGET) {
			color = BLUE;
		    } else {
			color = RED;
		    }
		    SET_FG(colors[color].pixel);

		    Rectangle_add(color,
				  X(x), Y(y+BLOCK_SZ),
				  BLOCK_SZ, BLOCK_SZ);

		    Rectangle_add(color,
				  X(x+(BLOCK_SZ+2)/4),
				  Y(y+3*BLOCK_SZ/4),
				  BLOCK_SZ/2, BLOCK_SZ/2);

		    s[0] = '0' + type - SETUP_TARGET; s[1] = '\0';
		    XDrawString(dpy, p_draw, gc,
				X(x + BLOCK_SZ/2
				  - XTextWidth(gameFont, s, 1)/2),
				Y(y + BLOCK_SZ/2 - gameFont->ascent/2),
				s, 1);

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
		if (team == type - SETUP_TREASURE) {
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
		XDrawString(dpy, p_draw, gc,
			    X(x+BLOCK_SZ/2),
			    Y(y+BLOCK_SZ/2), s, 1);
		break;

	    default:
		break;
	    }
	}
    }
}


void Paint_world_radar(void)
{
    int			i, xi, yi, xm, ym, xp, yp;
    float		xs, ys;
    int			npoint = 0, nsegment = 0, start, end;
    const int		max = 256;
    u_byte		visible[256];
    XSegment		segments[256];
    XPoint		points[256];

    if (s_radar == p_radar) 
	XSetPlaneMask(dpy, radarGC, 
		      AllPlanes&(~(dpl_1[0]|dpl_1[1])));
    if (s_radar != radar) {
	/* Clear radar */
	XSetForeground(dpy, radarGC, colors[BLACK].pixel);
	XFillRectangle(dpy, s_radar, radarGC, 0, 0, 256, RadarHeight);
    }
    XSetForeground(dpy, radarGC, colors[BLUE].pixel);

    memset(visible, 0, sizeof visible);
    visible[SETUP_FILLED] = 1;
    visible[SETUP_FILLED_NO_DRAW] = 1;
    visible[SETUP_REC_LU] = 1;
    visible[SETUP_REC_RU] = 1;
    visible[SETUP_REC_LD] = 1;
    visible[SETUP_REC_RD] = 1;
    visible[SETUP_FUEL] = 1;
    for (i = 0; i < 10; i++) {
	visible[SETUP_TARGET+i] = 1;
    }

    if (Setup->x >= 256) {
	xs = 256.0 / Setup->x;
	ys = (float)RadarHeight / Setup->y;
	for (xi=0; xi<Setup->x; xi++) {
	    start = end = -1;
	    xp = (int)((xi + 0.5) * xs);
	    for (yi=0; yi<Setup->y; yi++) {
		if (visible[Setup->map_data[xi * Setup->y + yi]] != 0) {
		    yp = (int)((yi + 0.5) * ys);
		    if (start == -1) {
			start = end = yp;
		    } else {
			end = yp;
		    }
		}
		if (start != -1
		    && (visible[Setup->map_data[xi * Setup->y + yi]] == 0
		    || yi == Setup->y - 1)) {
		    if (end > start) {
			segments[nsegment].x1 = xp;
			segments[nsegment].y1 = RadarHeight - 1 - start;
			segments[nsegment].x2 = xp;
			segments[nsegment].y2 = RadarHeight - 1 - end;
			nsegment++;
			if (nsegment >= max || yi == Setup->y - 1) {
			    XDrawSegments(dpy, s_radar, radarGC,
					  segments, nsegment);
			    nsegment = 0;
			}
		    } else {
			points[npoint].x = xp;
			points[npoint].y = RadarHeight - 1 - start;
			npoint++;
			if (npoint >= max || yi == Setup->y - 1) {
			    XDrawPoints(dpy, s_radar, radarGC,
					points, npoint, CoordModeOrigin);
			    npoint = 0;
			}
		    }
		    start = end = -1;
		}
	    }
	}
    } else {
	xs = Setup->x / 256.0;
	ys = Setup->y / (float)RadarHeight;
	for (xi=0; xi<256; xi++) {
	    xm = (int)(xi * xs + 0.5) * Setup->y;
	    start = end = -1;
	    xp = xi;
	    for (yi=0; yi<RadarHeight; yi++) {
		ym = (int)(yi * ys + 0.5);
		if (visible[Setup->map_data[xm + ym]] != 0) {
		    yp = yi;
		    if (start == -1) {
			start = end = yp;
		    } else {
			end = yp;
		    }
		}
		if (start != -1
		    && (visible[Setup->map_data[xm + ym]] == 0
		    || yi == RadarHeight - 1)) {
		    if (end > start) {
			segments[nsegment].x1 = xp;
			segments[nsegment].y1 = RadarHeight - 1 - start;
			segments[nsegment].x2 = xp;
			segments[nsegment].y2 = RadarHeight - 1 - end;
			nsegment++;
			if (nsegment >= max || yi == RadarHeight - 1) {
			    XDrawSegments(dpy, s_radar, radarGC,
					  segments, nsegment);
			    nsegment = 0;
			}
		    } else {
			points[npoint].x = xp;
			points[npoint].y = RadarHeight - 1 - start;
			npoint++;
			if (npoint >= max || yi == RadarHeight - 1) {
			    XDrawPoints(dpy, s_radar, radarGC,
					points, npoint, CoordModeOrigin);
			    npoint = 0;
			}
		    }
		    start = end = -1;
		}
	    }
	}
    }
    if (nsegment > 0) {
	XDrawSegments(dpy, s_radar, radarGC,
		      segments, nsegment);
    }
    if (npoint > 0) {
	XDrawPoints(dpy, s_radar, radarGC,
		    points, npoint, CoordModeOrigin);
    }

    if (s_radar == p_radar)
	XSetPlaneMask(dpy, radarGC, 
		      AllPlanes&(~(dpl_2[0]|dpl_2[1])));
}


void Paint_frame(void)
{
    static long scroll_i = 0;

    if (start_loops != end_loops) {
	errno = 0;
	error("Start neq. End");
    }
    loops = end_loops;

#ifdef SCROLL
    /*
     * Scroll a message in the window title.
     */
    if ((loops % SCROLL_DELAY) == 0) {
	if (++scroll_i >= LONG_MAX)
	    scroll_i = 0;
	XStoreName(dpy, top,
		   scroll(scroll,
			  scroll_i % scroll_len,
			  SCROLL_LEN));
    }
#else
    /*
     * Switch between two different window titles.
     */
    if ((loops % TITLE_DELAY) == 0) {
	scroll_i = !scroll_i;
	if (scroll_i)
	    XStoreName(dpy, top, COPYRIGHT);
	else
	    XStoreName(dpy, top, TITLE);
	
    }
#endif

    /*
     * Do we really need to draw all this if the player is damaged?
     */
    if (damaged <= 0) {
	Rectangle_start();
	Arc_start();
	Segment_start();

	Paint_world();

	Rectangle_end();
	Segment_end();
	Rectangle_start();
	Segment_start();

	Paint_vfuel();
	Paint_vcannon();
	Paint_vbase();
	Paint_shots();

	gcv.dash_offset = DASHES_LENGTH - (loops % DASHES_LENGTH);
	Paint_ships();
	gcv.dash_offset = 0;

	Paint_HUD();

	Rectangle_end();
	Segment_end();
	Rectangle_start();
	Segment_start();

	Paint_meters();

	Rectangle_end();
	Arc_end();
	Segment_end();

	Paint_messages();
	Paint_radar();
	Paint_score_objects();
    }

    /*
     * Now switch planes and clear the screen.
     */
    if (p_radar != radar && radar_exposed == true) {
	if (BIT(instruments, SHOW_SLIDING_RADAR) == 0) {
	    XCopyArea(dpy, p_radar, radar, gc,
		      0, 0, 256, RadarHeight, 0, 0);
	} else {
	    int x, y, w, h;
	    float xp, yp, xo, yo;

	    xp = (float) (pos.x * 256) / (Setup->x * BLOCK_SZ);
	    yp = (float) (pos.y * RadarHeight) / (Setup->y * BLOCK_SZ);
	    xo = (float) 256 / 2;
	    yo = (float) RadarHeight / 2;
	    if (xo <= xp) {
		x = (int) (xp - xo + 0.5);
	    } else {
		x = (int) (256 + xp - xo + 0.5);
	    }
	    if (yo <= yp) {
		y = (int) (yp - yo + 0.5);
	    } else {
		y = (int) (RadarHeight + yp - yo + 0.5);
	    }
	    y = RadarHeight - y - 1;
	    w = 256 - x;
	    h = RadarHeight - y;
	    XCopyArea(dpy, p_radar, radar, gc,
		      0, 0, x, y, w, h);
	    XCopyArea(dpy, p_radar, radar, gc,
		      x, 0, w, y, 0, h);
	    XCopyArea(dpy, p_radar, radar, gc,
		      0, y, x, h, w, 0);
	    XCopyArea(dpy, p_radar, radar, gc,
		      x, y, w, h, 0, 0);
	}
    }
    if (p_draw != draw)
	XCopyArea(dpy, p_draw, draw, gc,
		  0, 0, 768, 768, 0, 0);

    dbuff_switch(dbuf_state);

    if (BIT(dpy_type, DT_HAVE_PLANES)) {
	XSetPlaneMask(dpy, gc, dbuf_state->drawing_planes);
	XSetPlaneMask(dpy, messageGC, dbuf_state->drawing_planes);
    }

    if (damaged > 0) {
	XSetFunction(dpy, gc, GXxor);
	SET_FG(colors[BLACK].pixel ^ colors[BLUE].pixel);
	XFillRectangle(dpy, draw, gc, 0, 0, 768, 768);
	XSetFunction(dpy, gc, GXcopy);
    }
    else {
	SET_FG(colors[BLACK].pixel);
	XFillRectangle(dpy, p_draw, gc, 0, 0, 768, 768);
    }

    if (talk_mapped == true) {
	static bool toggle;
	static long last_toggled;

	if (loops > last_toggled + FPS / 2 || loops < last_toggled) {
	    toggle = (toggle == false) ? true : false;
	    last_toggled = loops;
	}
	Talk_cursor(toggle);
    }

    XFlush(dpy);
}


int Handle_start(long server_loops)
{
    int			i;

    start_loops = server_loops;

    num_refuel = 0;
    num_connector = 0;
    num_smart = 0;
    num_ball = 0;
    num_ship = 0;
    num_mine = 0;
    num_itemtype = 0;
    num_shot = 0;
    num_ecm = 0;
    num_trans = 0;
    num_paused = 0;
    num_radar = 0;
    num_vcannon = 0;
    num_vfuel = 0;
    num_vbase = 0;
    for (i = 0; i < DEBRIS_TYPES; i++) {
	num_debris[i] = 0;
    }

    damaged = 0;
    destruct = 0;
    shutdown_delay = 0;
    shutdown_count = -1;
    return 0;
}

int Handle_end(long server_loops)
{
    end_loops = server_loops;
    Paint_frame();
    return 0;
}

int Handle_self(int x, int y, int vx, int vy, int newHeading,
		float newPower, float newTurnspeed, float newTurnresistance,
		int newLockId, int newLockDist, int newLockBearing,
		int newNextCheckPoint,
		int newNumCloaks, int newNumSensors, int newNumMines,
		int newNumRockets, int newNumEcms, int newNumTransporters,
		int newExtraShots, int newRearShots,
		int newAfterBurners, int newNumTanks, int newCurrentTank,
		int newFuelSum, int newFuelMax, int newPacketSize)
{
    pos.x = x;
    pos.y = y;
    vel.x = vx;
    vel.y = vy;
    heading = newHeading;
    power = newPower;
    turnspeed = newTurnspeed;
    turnresistance = newTurnresistance;
    lock_id = newLockId;
    lock_dist = newLockDist;
    lock_dir = newLockBearing;
    nextCheckPoint = newNextCheckPoint;
    numCloaks = newNumCloaks;
    numSensors = newNumSensors;
    numMines = newNumMines;
    numRockets = newNumRockets;
    numEcms = newNumEcms;
    numTransporters = newNumTransporters;
    numFrontShots = newExtraShots;
    numBackShots = newRearShots;
    numAfterburners = newAfterBurners;
    numTanks = newNumTanks;
    fuelCurrent = newCurrentTank;
    if (newFuelSum > fuelSum && selfVisible != 0) {
	fuelCount = FUEL_NOTIFY;
    }
    fuelSum = newFuelSum;
    fuelMax = newFuelMax;
    selfVisible = 0;
    if (newPacketSize + 16 < packet_size) {
	packet_size -= 16;
    } else {
	packet_size = newPacketSize;
    }

    world.x = pos.x - CENTER;
    world.y = pos.y - CENTER;
    realWorld = world;
    wrappedWorld = 0;
    if (BIT(Setup->mode, WRAP_PLAY)) {
	if (world.x < 0) {
	    wrappedWorld |= 1;
	    world.x += Setup->x * BLOCK_SZ;
	} else if (world.x + FULL >= Setup->x * BLOCK_SZ) {
	    realWorld.x -= Setup->x * BLOCK_SZ;
	    wrappedWorld |= 1;
	}
	if (world.y < 0) {
	    wrappedWorld |= 2;
	    world.y += Setup->y * BLOCK_SZ;
	} else if (world.y + FULL >= Setup->y * BLOCK_SZ) {
	    realWorld.y -= Setup->y * BLOCK_SZ;
	    wrappedWorld |= 2;
	}
    }
    return 0;
}


int Handle_damaged(int dam)
{
    damaged = dam;
    return 0;
}


int Handle_destruct(int count)
{
    destruct = count;
    return 0;
}


int Handle_shutdown(int count, int delay)
{
    shutdown_count = count;
    shutdown_delay = delay;
    return 0;
}


int Handle_refuel(int x0, int y0, int x1, int y1)
{
    refuel_t	t;

    t.x0 = x0;
    t.x1 = x1;
    t.y0 = y0;
    t.y1 = y1;
    HANDLE(refuel_ptr, num_refuel, max_refuel, t);
    return 0;
}

int Handle_connector(int x0, int y0, int x1, int y1)
{
    connector_t	t;

    t.x0 = x0;
    t.x1 = x1;
    t.y0 = y0;
    t.y1 = y1;
    HANDLE(connector_ptr, num_connector, max_connector, t);
    return 0;
}

int Handle_smart(int x, int y, int dir)
{
    smart_t	t;

    t.x = x;
    t.y = y;
    t.dir = dir;
    HANDLE(smart_ptr, num_smart, max_smart, t);
    return 0;
}

int Handle_ball(int x, int y, int id)
{
    ball_t	t;

    t.x = x;
    t.y = y;
    t.id = id;
    HANDLE(ball_ptr, num_ball, max_ball, t);
    return 0;
}

int Handle_ship(int x, int y, int id, int dir, int shield, int cloak)
{
    ship_t	t;

    t.x = x;
    t.y = y;
    t.id = id;
    t.dir = dir;
    t.shield = shield;
    t.cloak = cloak;
    HANDLE(ship_ptr, num_ship, max_ship, t);

    if (self != NULL && self->id == id) {
	selfVisible = 1;
	return Handle_radar(x, y);
    }

    return 0;
}

int Handle_mine(int x, int y)
{
    mine_t	t;

    t.x = x;
    t.y = y;
    HANDLE(mine_ptr, num_mine, max_mine, t);
    return 0;
}

int Handle_item(int x, int y, int type)
{
    itemtype_t	t;

    t.x = x;
    t.y = y;
    t.type = type;
    HANDLE(itemtype_ptr, num_itemtype, max_itemtype, t);
    return 0;
}

int Handle_shot(int x, int y, int color)
{
    shot_t	t;

    t.x = x;
    t.y = y;
    t.color = color;
    HANDLE(shot_ptr, num_shot, max_shot, t);
    return 0;
}

int Handle_debris(int type, u_byte *p, int n)
{
#define num		(num_debris[type])
#define max		(max_debris[type])
#define ptr		(debris_ptr[type])

    if (n > max) {
	if (max == 0) {
	    ptr = (debris_t *) malloc (n * sizeof(*ptr));
	} else {
	    ptr = (debris_t *) realloc (ptr, n * sizeof(*ptr));
	}
	if (ptr == NULL) {
	    error("No memory for debris");
	    num = max = 0;
	    return -1;
	}
	max = n;
    }
    else if (n <= 0) {
	printf("debris %d < 0\n", n);
	return 0;
    }
    num = n;
    memcpy(ptr, p, n * sizeof(*ptr));
    return 0;

#undef num
#undef max
#undef ptr
}

int Handle_ecm(int x, int y, int size)
{
    ecm_t	t;

    t.x = x;
    t.y = y;
    t.size = size;
    HANDLE(ecm_ptr, num_ecm, max_ecm, t);
    return 0;
}

int Handle_trans(int x1, int y1, int x2, int y2)
{
    trans_t	t;

    t.x1 = x1;
    t.y1 = y1;
    t.x2 = x2;
    t.y2 = y2;
    HANDLE(trans_ptr, num_trans, max_trans, t);
    return 0;
}

int Handle_paused(int x, int y, int count)
{
    paused_t	t;

    t.x = x;
    t.y = y;
    t.count = count;
    HANDLE(paused_ptr, num_paused, max_paused, t);
    return 0;
}

int Handle_radar(int x, int y)
{
    radar_t	t;

    t.x = x;
    t.y = y;
    HANDLE(radar_ptr, num_radar, max_radar, t);
    return 0;
}

int Handle_message(char *msg)
{
    Add_message(msg);
    return 0;
}

int Handle_vcannon(int x, int y, int type)
{
    vcannon_t	t;

    t.x = x;
    t.y = y;
    t.type = type;
    HANDLE(vcannon_ptr, num_vcannon, max_vcannon, t);
    return 0;
}

int Handle_vfuel(int x, int y, long fuel)
{
    vfuel_t	t;

    t.x = x;
    t.y = y;
    t.fuel = fuel;
    HANDLE(vfuel_ptr, num_vfuel, max_vfuel, t);
    return 0;
}

int Handle_vbase(int x, int y, int xi, int yi, int type)
{
    vbase_t	t;

    t.x = x;
    t.y = y;
    t.xi = xi;
    t.yi = yi;
    t.type = type;
    HANDLE(vbase_ptr, num_vbase, max_vbase, t);
    return 0;
}

#define BORDER			6
#define SCORE_LIST_WINDOW_WIDTH	256

void Paint_score_start(void)
{
    static bool	first = true;
    static char	headingStr[MSG_LEN];
    static int thisLine;

    if (first) {
	thisLine = BORDER + scoreListFont->ascent;
	first = false;

	if (BIT(Setup->mode, TEAM_PLAY))
	    strcpy(headingStr, " TM ");
	else
	    strcpy(headingStr, "  ");
	strcat(headingStr, "SCORE ");
	if (BIT(Setup->mode, LIMITED_LIVES))
	    strcat(headingStr, "LIFE");
	strcat(headingStr, " NAME");
    }

    XClearWindow(dpy, players);
    ShadowDrawString(dpy, players, scoreListGC,
		     BORDER, thisLine,
		     headingStr,
		     colors[WHITE].pixel,
		     colors[BLACK].pixel);

    gcv.line_style = LineSolid;
    XChangeGC(dpy, scoreListGC, GCLineStyle, &gcv);
    XDrawLine(dpy, players, scoreListGC,
	      BORDER, thisLine,
	      SCORE_LIST_WINDOW_WIDTH - BORDER, thisLine);

    gcv.line_style = LineOnOffDash;
    XChangeGC(dpy, scoreListGC, GCLineStyle, &gcv);
}


void Paint_score_entry(int entry_num,
		       other_t* other,
		       bool best,
		       char* name)
{
    static char	teamStr[3], lifeStr[6], label[MSG_LEN];
    static int lineSpacing = -1, firstLine;
    int thisLine = firstLine + lineSpacing * entry_num;

    /*
     * First time we're here, set up miscellaneous strings for
     * efficiency and calculate some other constants.
     */
    if (lineSpacing == -1) {
	teamStr[0] = teamStr[2] = '\0';
	teamStr[1] = ' ';
	lifeStr[0] = '\0';

	lineSpacing
	    = scoreListFont->ascent + scoreListFont->descent + 3;
	firstLine
	    = 2*BORDER + scoreListFont->ascent + lineSpacing;
    }

    /*
     * Setup the status line
     */
    if (BIT(Setup->mode, TEAM_PLAY))
	teamStr[0] = other->team + '0';

    if (BIT(Setup->mode, LIMITED_LIVES))
	sprintf(lifeStr, " %3d", other->life);

    sprintf(label, "%c %s% 05d%s  %s",
	    other->mychar, teamStr, other->score, lifeStr, name);

    /*
     * Draw the line
     */
    ShadowDrawString(dpy, players, scoreListGC,
		     BORDER, thisLine,
		     label,
		     colors[WHITE].pixel,
		     colors[BLACK].pixel);

    /*
     * Underline the best player
     */
    if (best) {
	XDrawLine(dpy, players, scoreListGC,
		  BORDER, thisLine,
  		  SCORE_LIST_WINDOW_WIDTH - BORDER, thisLine);
    }
}

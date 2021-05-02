/* $Id: paint.c,v 3.152 1996/05/04 22:25:11 bert Exp $
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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <limits.h>

#include <X11/Xlib.h>
#include <X11/Xos.h>

#include "version.h"
#include "config.h"
#include "const.h"
#include "error.h"
#include "paint.h"
#include "xinit.h"
#include "setup.h"
#include "rules.h"
#include "bit.h"
#include "keys.h"
#include "net.h"
#include "netclient.h"
#include "dbuff.h"
#include "record.h"
#include "texture.h"

char paint_version[] = VERSION;

#ifndef ERASE
#define ERASE		0
#endif

#define ERASE_INITIALIZED	(1 << 0)

#define MAX_LINE_WIDTH	4	/* widest line drawn */

#define X(co)  ((int) ((co) - world.x))
#define Y(co)  ((int) (world.y + view_height - (co)))

extern float		tbl_sin[];
extern float		tbl_cos[];
extern setup_t		*Setup;
extern int		RadarHeight;
extern score_object_t	score_objects[MAX_SCORE_OBJECTS];
extern int		score_object;


/*
 * Globals.
 */
XFontStruct* gameFont;		/* The fonts used in the game */
XFontStruct* messageFont;
XFontStruct* scoreListFont;
XFontStruct* buttonFont;
XFontStruct* textFont;
XFontStruct* talkFont;
XFontStruct* motdFont;
char	gameFontName[FONT_LEN];	/* The fonts used in the game */
char	messageFontName[FONT_LEN];
char	scoreListFontName[FONT_LEN];
char	buttonFontName[FONT_LEN];
char	textFontName[FONT_LEN];
char	talkFontName[FONT_LEN];
char	motdFontName[FONT_LEN];

Display	*dpy;			/* Display of player (pointer) */
Display	*kdpy;			/* Keyboard display */
short	about_page;		/* Which page is the player on? */
u_short	team;			/* What team is the player on? */

GC	gc;			/* GC for the game area */
GC	messageGC;		/* GC for messages in the game area */
GC	radarGC;		/* GC for the radar */
GC	buttonGC;		/* GC for the buttons */
GC	scoreListGC;		/* GC for the player list */
GC	textGC;			/* GC for the info text */
GC	talkGC;			/* GC for the message window */
GC	motdGC;			/* GC for the motd text */

Window	top;			/* Top-level window (topshell) */
Window	draw;			/* Main play window */
Window	keyboard;		/* Keyboard window */
Pixmap	p_draw;			/* Saved pixmap for the drawing */
					/* area (monochromes use this) */
Window	radar;			/* Radar window */
Window	players;		/* Player list window */
Pixmap	p_radar, s_radar;	/* Pixmaps for the radar (implements */
				/* the planes hack on the radar for */
				/* monochromes) */
long	dpl_1[2], dpl_2[2];	/* Used by radar hack */
Window	about_w;		/* About window */
Window	about_close_b;		/* About window's close button */
Window	about_next_b;		/* About window's next button */
Window	about_prev_b;		/* About window's previous button */
Window	keys_close_b;		/* Help window's close button */
Window	talk_w;			/* Talk window */
XColor	colors[MAX_COLORS];	/* Colors */
Colormap	colormap;	/* Private colormap */
int	maxColors;		/* Max. number of colors to use */
int	hudColor;		/* Color index for HUD drawing */
int	hudLockColor;		/* Color index for lock on HUD drawing */
int	wallColor;		/* Color index for wall drawing */
int	wallRadarColor;		/* Color index for walls on radar. */
int	targetRadarColor;	/* Color index for targets on radar. */
int	decorColor;		/* Color index for decoration drawing */
int	decorRadarColor;	/* Color index for decorations on radar. */
bool	gotFocus;
int	radar_exposures;
bool	players_exposed;
short	view_width, view_height;	/* Visible area according to server */
u_byte	debris_colors;		/* Number of debris intensities from server */
u_byte	spark_rand;		/* Sparkling effect */
float	charsPerTick = 0.0;	/* Output speed of messages */
bool	markingLights;
int	titleFlip;		/* Do special title bar flipping? */
int	shieldDrawMode = -1;	/* Either LineOnOffDash or LineSolid */
char	modBankStr[NUM_MODBANKS][MAX_CHARS];	/* modifier banks */
char	*texturePath;		/* Path list of texture directories */
char	*wallTextureFile;	/* Filename of wall texture */
char	*decorTextureFile;	/* Filename of decor texture */
char	*ballTextureFile;	/* Filename of ball texture */

int	(*radarPlayerRectFN)	/* Function to draw player on radar */
	(Display *disp, Drawable d, GC gc,
	 int x, int y, unsigned width, unsigned height);

int		maxKeyDefs;
keydefs_t	*keyDefs;

other_t	*self;			/* player info */

message_t		*TalkMsg[MAX_MSGS], *GameMsg[MAX_MSGS];


static void Paint_clock(int redraw);


static int		eyesId;		/* Player we get frame updates for */


/*
 * Local types and data
 */
typedef struct {
    short		x0, y0, x1, y1;
} refuel_t;

typedef struct {
    short		x0, y0, x1, y1;
    u_byte		tractor;
} connector_t;

typedef struct {
    unsigned char	color, dir;
    short		x, y, len;
} laser_t;

typedef struct {
    short		x, y, dir;
    unsigned char	len;
} missile_t;

typedef struct {
    short		x, y, id;
} ball_t;

typedef struct {
    short		x, y, id, dir;
    u_byte		shield, cloak, eshield;
} ship_t;

typedef struct {
    short		x, y, teammine, id;
} mine_t;

typedef struct {
    short		x, y, type;
} itemtype_t;

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
    short		x, y, size;
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

typedef struct {
    short		x, y, xi, yi, type;
} vdecor_t;

#if ERASE
typedef struct {
    int			flags;
    XRectangle		*rect_ptr;
    int			num_rect,
			max_rect;
    XArc		*arc_ptr;
    int			num_arc,
			max_arc;
    XSegment		*seg_ptr[MAX_LINE_WIDTH + 1];
    int			num_seg[MAX_LINE_WIDTH + 1],
			max_seg[MAX_LINE_WIDTH + 1];
} erase_t;

static erase_t		erase[2],
			*erp;
#endif	/* ERASE */

static refuel_t		*refuel_ptr;
static int		 num_refuel, max_refuel;
static connector_t	*connector_ptr;
static int		 num_connector, max_connector;
static laser_t		*laser_ptr;
static int		 num_laser, max_laser;
static missile_t	*missile_ptr;
static int		 num_missile, max_missile;
static ball_t		*ball_ptr;
static int		 num_ball, max_ball;
static ship_t		*ship_ptr;
static int		 num_ship, max_ship;
static mine_t		*mine_ptr;
static int		 num_mine, max_mine;
static itemtype_t	*itemtype_ptr;
static int		 num_itemtype, max_itemtype;
static ecm_t		*ecm_ptr;
static int		 num_ecm, max_ecm;
static trans_t		*trans_ptr;
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
static debris_t		*fastshot_ptr[DEBRIS_TYPES * 2];
static int		 num_fastshot[DEBRIS_TYPES * 2],
			 max_fastshot[DEBRIS_TYPES * 2];
static vdecor_t		*vdecor_ptr;
static int		 num_vdecor, max_vdecor;

/*
 * Macro to add one new element of a given type to a dynamic array.
 * T is the type of the element.
 * P is the pointer to the array memory.
 * N is the current number of elements in the array.
 * M is the current size of the array.
 * V is the new element to add.
 * The goal is to keep the number of malloc/realloc calls low
 * while not wasting too much memory because of over-allocation.
 */
#define HANDLE(T,P,N,M,V)							\
    if (N >= M && ((M <= 0)						\
	? (P = (T *) malloc((M = 1) * sizeof(*P)))			\
	: (P = (T *) realloc(P, (M += M) * sizeof(*P)))) == NULL) {	\
	error("No memory");						\
	N = M = 0;							\
	return -1;							\
    } else								\
	(P[N++] = V)

#ifndef PAINT_FREE
# define PAINT_FREE	1
#endif
#if PAINT_FREE
# define RELEASE(P, N, M)	if (!(N)) ; else (free(P), (M) = 0, (N) = 0)
#else
# define RELEASE(P, N, M)	((N) = 0)
#endif

#if ERASE
/*
 * Macro to make room in a given dynamic array for new elements.
 * P is the pointer to the array memory.
 * N is the current number of elements in the array.
 * M is the current size of the array.
 * T is the type of the elements.
 * E is the number of new elements to store in the array.
 * The goal is to keep the number of malloc/realloc calls low
 * while not wasting too much memory because of over-allocation.
 */
#define EXPAND(P,N,M,T,E)						\
    if ((N) + (E) > (M)) {						\
	if ((M) <= 0) {							\
	    M = (E) + 2;						\
	    P = (T *) malloc((M) * sizeof(T));				\
	    N = 0;							\
	} else {							\
	    M = ((M) << 1) + (E);					\
	    P = (T *) realloc(P, (M) * sizeof(T));			\
	}								\
	if (P == NULL) {						\
	    error("No memory");						\
	    N = M = 0;							\
	    return;	/* ! */						\
	}								\
    }

#define UNEXPAND(P,N,M)							\
    if ((N) < ((M) >> 2)) {						\
	free(P);							\
	M = 0;								\
    }									\
    N = 0;
#endif	/* ERASE */

static long		loops = 0,
			start_loops,
			end_loops,
			time_left = -1;

static XPoint		points[255];
static XGCValues	gcv;

static XRectangle	*rect_ptr[MAX_COLORS];
static int		num_rect[MAX_COLORS], max_rect[MAX_COLORS];
static XArc		*arc_ptr[MAX_COLORS];
static int		num_arc[MAX_COLORS], max_arc[MAX_COLORS];
static XSegment		*seg_ptr[MAX_COLORS];
static int		num_seg[MAX_COLORS], max_seg[MAX_COLORS];

static unsigned long	current_foreground;

#define RESET_FG()	(current_foreground = -1)
#define SET_FG(PIXEL)				\
    if ((PIXEL) == current_foreground) ;	\
    else XSetForeground(dpy, gc, current_foreground = (PIXEL))

#define FIND_NAME_WIDTH(other)						\
    if ((other)->name_width == 0) {					\
	(other)->name_len = strlen((other)->name);			\
	(other)->name_width = 2 + XTextWidth(gameFont, (other)->name,	\
					 (other)->name_len);		\
    }

#if ERASE
static void Erase_start(void)
{
    int			i;

    if (damaged > 0) {
	error("BUG: Erase_start while damaged");
	return;
    }

    if (erase[0].flags == 0) {
	printf("ERASE is On!\n");
	erp = &erase[0];
    }
    if (BIT(erp->flags, ERASE_INITIALIZED) == 0) {
	SET_FG(colors[BLACK].pixel);
	XFillRectangle(dpy, p_draw, gc, 0, 0, draw_width, draw_height);
	SET_BIT(erp->flags, ERASE_INITIALIZED);
    }
    erp->num_rect = 0;
    erp->num_arc = 0;
    for (i = 0; i <= MAX_LINE_WIDTH; i++) {
	erp->num_seg[i] = 0;
    }
}

static void Erase_end(void)
{
    int			i,
			linewidth = false;

    if (damaged > 0) {
	error("BUG: Erase_end while damaged");
	return;
    }

    if (erp == &erase[0]) {
	erp = &erase[1];
    } else {
	erp = &erase[0];
    }

    SET_FG(colors[BLACK].pixel);

    if (erp->num_rect != 0) {
	XFillRectangles(dpy, p_draw, gc, erp->rect_ptr, erp->num_rect);
	UNEXPAND(erp->rect_ptr, erp->num_rect, erp->max_rect);
    }
    if (erp->num_arc != 0) {
	XDrawArcs(dpy, p_draw, gc, erp->arc_ptr, erp->num_arc);
	UNEXPAND(erp->arc_ptr, erp->num_arc, erp->max_arc);
    }
    for (i = 0; i <= MAX_LINE_WIDTH; i++) {
	if (erp->num_seg[i] != 0) {
	    XSetLineAttributes(dpy, gc, i,
			       LineSolid, CapProjecting, JoinMiter);
	    linewidth = true;
	    XDrawSegments(dpy, p_draw, gc, erp->seg_ptr[i], erp->num_seg[i]);
	    UNEXPAND(erp->seg_ptr[i], erp->num_seg[i], erp->max_seg[i]);
	}
    }
    if (linewidth == true) {
	XSetLineAttributes(dpy, gc, 0,
			   LineSolid, CapButt, JoinMiter);
    }
}

static void Erase_rectangle(int x, int y, int width, int height)
{
    XRectangle		*p;

    EXPAND(erp->rect_ptr, erp->num_rect, erp->max_rect, XRectangle, 1);
    p = &erp->rect_ptr[erp->num_rect++];
    p->x = x;
    p->y = y;
    p->width = width;
    p->height = height;
}

static void Erase_rectangles(XRectangle *rectp, int n)
{
    EXPAND(erp->rect_ptr, erp->num_rect, erp->max_rect, XRectangle, n);
    memcpy(&erp->rect_ptr[erp->num_rect], rectp, n * sizeof(XRectangle));
    erp->num_rect += n;
}

static void Erase_arc(int x, int y, int width, int height,
		      int angle1, int angle2)
{
    XArc		*p;

    EXPAND(erp->arc_ptr, erp->num_arc, erp->max_arc, XArc, 1);
    p = &erp->arc_ptr[erp->num_arc++];
    p->x = x;
    p->y = y;
    p->width = width;
    p->height = height;
    p->angle1 = angle1;
    p->angle2 = angle2;
}

static void Erase_arcs(XArc *arcp, int n)
{
    EXPAND(erp->arc_ptr, erp->num_arc, erp->max_arc, XArc, n);
    memcpy(&erp->arc_ptr[erp->num_arc], arcp, n * sizeof(XArc));
    erp->num_arc += n;
}

static void Erase_segment(int width, int x1, int y1, int x2, int y2)
{
    XSegment		*p;

    EXPAND(erp->seg_ptr[width], erp->num_seg[width], erp->max_seg[width],
	   XSegment, 1);
    p = &erp->seg_ptr[width][erp->num_seg[width]++];
    p->x1 = x1;
    p->y1 = y1;
    p->x2 = x2;
    p->y2 = y2;
}

static void Erase_segments(XSegment *segp, int n)
{
    EXPAND(erp->seg_ptr[0], erp->num_seg[0], erp->max_seg[0],
	   XSegment, n);
    memcpy(&erp->seg_ptr[0][erp->num_seg[0]], segp, n * sizeof(XSegment));
    erp->num_seg[0] += n;
}

static void Erase_points(int width, XPoint *pointp, int n)
{
    XSegment		*p;
    int			i;

    EXPAND(erp->seg_ptr[width], erp->num_seg[width], erp->max_seg[width],
	   XSegment, n - 1);
    p = &erp->seg_ptr[width][erp->num_seg[width]];
    for (i = 1; i < n; i++) {
	p->x1 = pointp->x;
	p->y1 = pointp->y;
	pointp++;
	p->x2 = pointp->x;
	p->y2 = pointp->y;
	p++;
    }
    erp->num_seg[width] += n - 1;
}

static void Erase_4point(int x, int y, int width, int height)
{
    XSegment		*p;

    EXPAND(erp->seg_ptr[0], erp->num_seg[0], erp->max_seg[0],
	   XSegment, 4);
    p = &erp->seg_ptr[0][erp->num_seg[0]];
    p->x1 = x;
    p->y1 = y;
    p->x2 = x + width;
    p->y2 = y;
    p++;
    p->x1 = x + width;
    p->y1 = y;
    p->x2 = x + width;
    p->y2 = y + height;
    p++;
    p->x1 = x + width;
    p->y1 = y + height;
    p->x2 = x;
    p->y2 = y + height;
    p++;
    p->x1 = x;
    p->y1 = y + height;
    p->x2 = x;
    p->y2 = y;
    p++;
    erp->num_seg[0] += 4;
}
#else	/* ERASE */
#define Erase_start()
#define Erase_end()
#define Erase_rectangle(A,B,C,D)
#define Erase_rectangles(A,B)
#define Erase_arc(A,B,C,D,E,F)
#define Erase_arcs(A,B)
#define Erase_segment(A,B,C,D,E)
#define Erase_segments(A,B)
#define Erase_points(A,B,C)
#define Erase_4point(A,B,C,D)
#endif	/* ERASE */

static void Rectangle_start(void)
{
    int i;

    for (i = 0; i < maxColors; i++) {
	num_rect[i] = 0;
    }
}

static void Rectangle_end(void)
{
    int i;

    for (i = 0; i < maxColors; i++) {
	if (num_rect[i] > 0) {
	    SET_FG(colors[i].pixel);
	    rd.fillRectangles(dpy, p_draw, gc, rect_ptr[i], num_rect[i]);
	    Erase_rectangles(rect_ptr[i], num_rect[i]);
	    RELEASE(rect_ptr[i], num_rect[i], max_rect[i]);
	}
    }
}

static int Rectangle_add(int color, int x, int y, int width, int height)
{
    XRectangle		t;

    t.x = x;
    t.y = y;
    t.width = width;
    t.height = height;
    HANDLE(XRectangle, rect_ptr[color], num_rect[color], max_rect[color], t);
    return 0;
}

static void Arc_start(void)
{
    int i;

    for (i = 0; i < maxColors; i++) {
	num_arc[i] = 0;
    }
}

static void Arc_end(void)
{
    int i;

    for (i = 0; i < maxColors; i++) {
	if (num_arc[i] > 0) {
	    SET_FG(colors[i].pixel);
	    rd.drawArcs(dpy, p_draw, gc,
		arc_ptr[i], num_arc[i]);
	    Erase_arcs(arc_ptr[i], num_arc[i]);
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

    t.x = x;
    t.y = y;
    t.width = width;
    t.height = height;
    t.angle1 = angle1;
    t.angle2 = angle2;
    HANDLE(XArc, arc_ptr[color], num_arc[color], max_arc[color], t);
    return 0;
}

static void Segment_start(void)
{
    int i;

    for (i = 0; i < maxColors; i++) {
	num_seg[i] = 0;
    }
}

static void Segment_end(void)
{
    int i;

    for (i = 0; i < maxColors; i++) {
	if (num_seg[i] > 0) {
	    SET_FG(colors[i].pixel);
	    rd.drawSegments(dpy, p_draw, gc,
		seg_ptr[i], num_seg[i]);
	    Erase_segments(seg_ptr[i], num_seg[i]);
	    RELEASE(seg_ptr[i], num_seg[i], max_seg[i]);
	}
    }
}

static int Segment_add(int color, int x1, int y1, int x2, int y2)
{
    XSegment t;

    t.x1 = x1;
    t.y1 = y1;
    t.x2 = x2;
    t.y2 = y2;
    HANDLE(XSegment, seg_ptr[color], num_seg[color], max_seg[color], t);
    return 0;
}

/*
 * Draw a meter of some kind on screen.
 * When the x-offset is specified as a negative value then
 * the meter is drawn relative to the right side of the screen,
 * otherwise from the normal left side.
 */
static void Paint_meter(int xoff, int y, char *title, int val, int max)
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
		  (int)(((METER_WIDTH-3)*val)/(max?max:1)), METER_HEIGHT-3);
    SET_FG(colors[WHITE].pixel);
    rd.drawRectangle(dpy, p_draw, gc,
		   x, y, METER_WIDTH, METER_HEIGHT);
    Erase_4point(x, y, METER_WIDTH, METER_HEIGHT);

    /* Paint scale levels(?) */
    Segment_add(WHITE, x,       y-4,	x,       y+METER_HEIGHT+4);
    Segment_add(WHITE, x+mw4_4, y-4,	x+mw4_4, y+METER_HEIGHT+4);
    Segment_add(WHITE, x+mw2_4, y-3,	x+mw2_4, y+METER_HEIGHT+3);
    Segment_add(WHITE, x+mw1_4, y-1,	x+mw1_4, y+METER_HEIGHT+1);
    Segment_add(WHITE, x+mw3_4, y-1,	x+mw3_4, y+METER_HEIGHT+1);

    rd.drawString(dpy, p_draw, gc,
		  xstr, y+(gameFont->ascent+METER_HEIGHT)/2,
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


void Game_over_action(u_byte stat)
{
    static u_byte old_stat = 0;

    if (BIT(old_stat, GAME_OVER) && !BIT(stat, GAME_OVER)
	&& !BIT(stat,PAUSE)) {
	XMapRaised(dpy, top);
    }
    if (BIT(old_stat, PLAYING|PAUSE|GAME_OVER) != PLAYING) {
	if (BIT(stat, PLAYING|PAUSE|GAME_OVER) == PLAYING) {
	    Reset_shields();
	}
    }

    old_stat = stat;
}

static void Paint_item_symbol(u_byte type, Drawable d, GC mygc, int x, int y)
{
    gcv.stipple = itemBitmaps[type];
    gcv.fill_style = FillStippled;
    gcv.ts_x_origin = x;
    gcv.ts_y_origin = y;
    XChangeGC(dpy, mygc,
	      GCStipple|GCFillStyle|GCTileStipXOrigin|GCTileStipYOrigin,
	      &gcv);
    rd.paintItemSymbol(type, d, mygc, x, y);
    XFillRectangle(dpy, d, mygc, x, y, ITEM_SIZE, ITEM_SIZE);
    gcv.fill_style = FillSolid;
    XChangeGC(dpy, mygc, GCFillStyle, &gcv);
}


void Paint_item(u_byte type, Drawable d, GC mygc, int x, int y)
{
    const int		SIZE = ITEM_TRIANGLE_SIZE;

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
    Paint_item_symbol(type, d, mygc, x - ITEM_SIZE/2, y - SIZE + 2);
}


static void Paint_shots(void)
{
    int		color, i, j, id, x, y, xs, ys, x1, x2, y1, y2, len, dir;
    int		x_areas, y_areas, areas, max;

    if (num_itemtype > 0) {
	SET_FG(colors[RED].pixel);
	for (i = 0; i < num_itemtype; i++) {
	    x = itemtype_ptr[i].x;
	    y = itemtype_ptr[i].y;
	    if (wrap(&x, &y)) {
		Paint_item(itemtype_ptr[i].type, p_draw, gc, X(x), Y(y));
		Erase_rectangle(X(x) - ITEM_TRIANGLE_SIZE,
				Y(y) - ITEM_TRIANGLE_SIZE,
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
	for (i = 0; i < num_ball; i++) {
	    x = ball_ptr[i].x;
	    y = ball_ptr[i].y;
	    id = ball_ptr[i].id;
	    if (wrap(&x, &y)) {
		x = X(x);
		y = Y(y);
		if (ballTile != None) {
		    XSetTSOrigin(dpy, gc, x - BALL_RADIUS, y - BALL_RADIUS);
		    XFillArc(dpy, p_draw, gc, x - BALL_RADIUS, y - BALL_RADIUS,
			     2*BALL_RADIUS, 2*BALL_RADIUS, 0, 64*360);
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
		mine_points[0].x = x - 8;
		mine_points[0].y = y - 1;
		if (mine_ptr[i].teammine == 0) {
			SET_FG(colors[BLUE].pixel);
			rd.fillRectangle(dpy, p_draw, gc, x - 7, y - 2, 15, 5);
		}
		SET_FG(colors[WHITE].pixel);
		rd.drawLines(dpy, p_draw, gc,
			   mine_points, 21, CoordModePrevious);
		Erase_rectangle(x - 8, y - 3, 17, 7);

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
			static char *expired_name = "Expired";
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
			static char *unknown_name = "Not of this world!";
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
				    x - name_width / 2,
				    y + gameFont->ascent + 4,
				    name, name_len);
			Erase_rectangle(x - name_width / 2 - 1, y + 4,
					name_width + 2,
					gameFont->ascent + gameFont->descent);
		    }
		}
	    }
	}
	RELEASE(mine_ptr, num_mine, max_mine);
    }

    x_areas = (view_width + 255) >> 8;
    y_areas = (view_height + 255) >> 8;
    areas = x_areas * y_areas;
    max = areas * (debris_colors >= 3 ? debris_colors : 4);

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

    for (i = 0; i < max; i++) {
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

    /*
     * Draw fastshots
     */
    for (i = 0; i < max; i++) {
	int t = i + DEBRIS_TYPES;

	if (num_fastshot[i] > 0) {
	    x = BASE_X(i);
	    y = BASE_Y(i);
	    color = COLOR(i);
	    if (color != WHITE && color != BLUE) {
		color = WHITE;
	    }
	    for (j = 0; j < num_fastshot[i]; j++) {
		Rectangle_add(color,
			      x + fastshot_ptr[i][j].x - shot_size/2,
			      y - fastshot_ptr[i][j].y - shot_size/2,
			      shot_size, shot_size);
	    }
	    RELEASE(fastshot_ptr[i], num_fastshot[i], max_fastshot[i]);
	}

	/*
	 * Teamshots are in range DEBRIS_TYPES to DEBRIS_TYPES*2-1 in fastshot.
	 */
	if (num_fastshot[t] > 0) {
	    x = BASE_X(i);
	    y = BASE_Y(i);
	    color = COLOR(i);
	    for (j = 0; j < num_fastshot[t]; j++) {
		Rectangle_add(color,
			      x + fastshot_ptr[t][j].x - teamshot_size/2,
			      y - fastshot_ptr[t][j].y - teamshot_size/2,
			      teamshot_size, teamshot_size);
	    }
	    RELEASE(fastshot_ptr[t], num_fastshot[t], max_fastshot[t]);
	}
    }

    if (num_missile > 0) {
	int len;
	SET_FG(colors[WHITE].pixel);
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
		rd.drawLine(dpy, p_draw, gc, x1, y1, x2, y2);
		Erase_segment(4, x1, y1, x2, y2);
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
		SET_FG(colors[color].pixel);
		rd.drawLine(dpy, p_draw, gc,
			  X(x1), Y(y1),
			  X(x2), Y(y2));
		Erase_segment(3, X(x1), Y(y1), X(x2), Y(y2));
	    }
	}
	XSetLineAttributes(dpy, gc, 0,
			   LineSolid, CapButt, JoinMiter);
	RELEASE(laser_ptr, num_laser, max_laser);
    }
}

static void Paint_ships(void)
{
    int			i, x, y, x0, y0, x1, y1;
    int			cnt, dir, size, lcnt, ship_color;
    unsigned long	mask;
    other_t		*other;
    static int		pauseCharWidth = -1;
    wireobj		*ship;

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
			       x0, y0,
			       2*half_pause_size+1, 2*half_pause_size+1);
		if (paused_ptr[i].count <= 0 || loops % 10 >= 5) {
		    SET_FG(colors[mono?BLACK:WHITE].pixel);
		    rd.drawRectangle(dpy, p_draw, gc,
				   x0 - 1,
				   y0 - 1,
				   2*(half_pause_size+1),
				   2*(half_pause_size+1));
		    rd.drawString(dpy, p_draw, gc,
				X(x - pauseCharWidth/2),
				Y(y - gameFont->ascent/2),
				"P", 1);
		}
		Erase_rectangle(x0 - 1, y0 - 1,
				2*half_pause_size+3, 2*half_pause_size+3);
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
	    if (wrap(&x, &y)) {
		dir = ship_ptr[i].dir;
		ship = Ship_by_id(ship_ptr[i].id);
		for (cnt = 0; cnt < ship->num_points; cnt++) {
		    points[cnt].x = X(x + ship->pts[cnt][dir].x);
		    points[cnt].y = Y(y + ship->pts[cnt][dir].y);
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
				X(x - other->name_width / 2),
				Y(y - gameFont->ascent - 15),
				other->name, other->name_len);
		    Erase_rectangle(X(x - other->name_width / 2) - 1,
				    Y(y - gameFont->ascent - 15)
					- gameFont->ascent,
				    other->name_width + 2,
				    gameFont->ascent + gameFont->descent);
		}

		ship_color = WHITE;
#if ERASE
/*
 * If ERASE is defined outline the locked ship in a different color,
 * instead of mucking around with polygons.
 */
		if (lock_id == ship_ptr[i].id
		    && ship_ptr[i].id != -1
		    && lock_dist != 0) {
		    ship_color = RED;
		}
#endif
#ifndef NO_BLUE_TEAM
		if (BIT(Setup->mode, TEAM_PLAY)
		    && self != NULL
		    && self->id != ship_ptr[i].id
		    && (other = Other_by_id(ship_ptr[i].id)) != NULL
		    && self->team == other->team) {
		    ship_color = BLUE;
		}
#endif

		if (ship_ptr[i].cloak == 0) {
		    if (gcv.line_style != LineSolid) {
			gcv.line_style = LineSolid;
			XChangeGC(dpy, gc, GCLineStyle, &gcv);
		    }
		    SET_FG(colors[ship_color].pixel);
		    rd.drawLines(dpy, p_draw, gc, points, cnt, 0);
		    Erase_points(0, points, cnt);
#if !ERASE
		    if (lock_id == ship_ptr[i].id
			&& ship_ptr[i].id != -1
			&& lock_dist != 0) {
			rd.fillPolygon(dpy, p_draw, gc,
				       points, cnt,
				       Complex, CoordModeOrigin);
		    }
#endif

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
				    Y(y + ship->r_light[lcnt][dir].y)-2, 6, 6);
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

		if (ship_ptr[i].shield || ship_ptr[i].cloak) {
		    if (gcv.line_style != LineOnOffDash) {
			gcv.line_style = LineOnOffDash;
			mask = GCLineStyle;
#ifndef NO_ROTATING_DASHES
			mask |= GCDashOffset;
#endif
			XChangeGC(dpy, gc, mask, &gcv);
		    }
		    SET_FG(colors[ship_color].pixel);
		    if (ship_ptr[i].cloak) {
#if ERASE
			int j;
			for (j = 0; j < cnt - 1; j++) {
			    rd.drawLine(dpy, p_draw, gc,
				      points[j].x, points[j].y,
				      points[j + 1].x, points[j + 1].y);
			}
			Erase_points(1, points, cnt);
#else
			rd.drawLines(dpy, p_draw, gc, points, cnt, 0);
#endif
		    }
		    if (ship_ptr[i].shield) {
			int radius = ship->shield_radius;
			int e_radius = radius + 4;
			int half_radius = radius >> 1;
			int half_e_radius = e_radius >> 1;

			rd.drawArc(dpy, p_draw, gc, X(x - half_radius), Y(y + half_radius),
				   radius, radius, 0, 64 * 360);
			Erase_arc(X(x - half_radius), Y(y + half_radius),
				  radius, radius, 0, 64 * 360);

			if (ship_ptr[i].eshield) {	/* Emergency Shield */
			    rd.drawArc(dpy, p_draw, gc, X(x - half_e_radius), Y(y + half_e_radius),
				       e_radius, e_radius, 0, 64 * 360);
			    Erase_arc(X(x - half_e_radius), Y(y + half_e_radius),
				      e_radius, e_radius, 0, 64 * 360);
			}
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
		    rd.drawLine(dpy, p_draw, gc,
			      X(x0), Y(y0),
			      X(x1), Y(y1));
		    Erase_segment(1, X(x0), Y(y0), X(x1), Y(y1));
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
		if (connector_ptr[i].tractor) {
		    if (!cdashing) {
			rd.setDashes(dpy, gc, 0, cdashes, NUM_CDASHES);
			cdashing = 1;
		    }
		} else {
		    if (cdashing) {
			rd.setDashes(dpy, gc, 0, dashes, NUM_DASHES);
			cdashing = 0;
		    }
		}
		if (wrap(&x0, &y0)
		    && wrap(&x1, &y1)) {
		    rd.drawLine(dpy, p_draw, gc,
			      X(x0), Y(y0),
			      X(x1), Y(y1));
		    Erase_segment(1, X(x0), Y(y0), X(x1), Y(y1));
		}
	    }
	    RELEASE(connector_ptr, num_connector, max_connector);
	    if (cdashing)
		rd.setDashes(dpy, gc, 0, dashes, NUM_DASHES);
	}
	if (num_trans > 0) {
	    for (i = 0; i < num_trans; i++) {
		x0 = trans_ptr[i].x1;
		y0 = trans_ptr[i].y1;
		x1 = trans_ptr[i].x2;
		y1 = trans_ptr[i].y2;
		if (wrap(&x0, &y0) && wrap(&x1, &y1)) {
		    rd.drawLine(dpy, p_draw, gc,
			      X(x0), Y(y0), X(x1), Y(y1));
		    Erase_segment(1, X(x0), Y(y0), X(x1), Y(y1));
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
		    SET_FG(colors[hudColor].pixel);
		    rd.drawString(dpy, p_draw, gc,
				X(x),
				Y(y),
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


static void Paint_meters(void)
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
		hud_pos_x - target->name_width / 2,
		hud_pos_y - HUD_SIZE+HUD_OFFSET - gameFont->descent - BORDER,
		target->name, target->name_len);
    Erase_rectangle(hud_pos_x - target->name_width / 2 - 1,
		    hud_pos_y - HUD_SIZE+HUD_OFFSET - gameFont->descent
			- BORDER - gameFont->ascent,
		    target->name_width + 2,
		    gameFont->ascent + gameFont->descent);

    ship = Ship_by_id(lock_id);
    for (i = 0; i < ship->num_points; i++) {
	points[i].x = hud_pos_x + ship->pts[i][dir].x / 2 + 60;
	points[i].y = hud_pos_y + ship->pts[i][dir].y / 2 - 80;
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
		    hud_pos_x + HUD_SIZE - HUD_OFFSET + BORDER,
		    hud_pos_y - HUD_SIZE+HUD_OFFSET
		    - gameFont->descent - BORDER,
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
			 x, y,
			 size, size, 0, 64*360);
		Erase_rectangle(x, y, size, size);
		SET_FG(colors[hudColor].pixel);       
	    }
	}
    }
}


static void Paint_HUD(void)
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
		  hud_pos_x-HUD_SIZE, hud_pos_y-HUD_SIZE+HUD_OFFSET,
		  hud_pos_x+HUD_SIZE, hud_pos_y-HUD_SIZE+HUD_OFFSET);
	Erase_segment(0,
		      hud_pos_x-HUD_SIZE, hud_pos_y-HUD_SIZE+HUD_OFFSET,
		      hud_pos_x+HUD_SIZE, hud_pos_y-HUD_SIZE+HUD_OFFSET);
	rd.drawLine(dpy, p_draw, gc,
		  hud_pos_x-HUD_SIZE, hud_pos_y+HUD_SIZE-HUD_OFFSET,
		  hud_pos_x+HUD_SIZE, hud_pos_y+HUD_SIZE-HUD_OFFSET);
	Erase_segment(0,
		      hud_pos_x-HUD_SIZE, hud_pos_y+HUD_SIZE-HUD_OFFSET,
		      hud_pos_x+HUD_SIZE, hud_pos_y+HUD_SIZE-HUD_OFFSET);
    }
    if (BIT(instruments, SHOW_HUD_VERTICAL)) {
	rd.drawLine(dpy, p_draw, gc,
		  hud_pos_x-HUD_SIZE+HUD_OFFSET, hud_pos_y-HUD_SIZE,
		  hud_pos_x-HUD_SIZE+HUD_OFFSET, hud_pos_y+HUD_SIZE);
	Erase_segment(0,
		      hud_pos_x-HUD_SIZE+HUD_OFFSET, hud_pos_y-HUD_SIZE,
		      hud_pos_x-HUD_SIZE+HUD_OFFSET, hud_pos_y+HUD_SIZE);
	rd.drawLine(dpy, p_draw, gc,
		  hud_pos_x+HUD_SIZE-HUD_OFFSET, hud_pos_y-HUD_SIZE,
		  hud_pos_x+HUD_SIZE-HUD_OFFSET, hud_pos_y+HUD_SIZE);
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
	    Paint_item_symbol(i, p_draw, gc, horiz_pos - ITEM_SIZE, vert_pos);

	    if (i == lose_item) {
		if (lose_item_active != 0) {
		    if (lose_item_active < 0) {
			lose_item_active++;
		    }
		    rd.drawRectangle(dpy, p_draw, gc, horiz_pos-ITEM_SIZE-2,
				     vert_pos-2, ITEM_SIZE+2, ITEM_SIZE+2);
		}
	    }

	    /* Paint item count */
	    sprintf(str, "%d", num);
	    len = strlen(str);
	    width = XTextWidth(gameFont, str, len);
	    rd.drawString(dpy, p_draw, gc,
			horiz_pos - ITEM_SIZE - BORDER - width,
			vert_pos + ITEM_SIZE/2 + gameFont->ascent/2,
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
		    hud_pos_x + HUD_SIZE-HUD_OFFSET+BORDER,
		    hud_pos_y + HUD_SIZE-HUD_OFFSET+BORDER + gameFont->ascent,
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
			hud_pos_x + HUD_SIZE-HUD_OFFSET + BORDER,
			hud_pos_y + HUD_SIZE-HUD_OFFSET + BORDER
			+ gameFont->descent + 2*gameFont->ascent,
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
			hud_pos_x - sobj->hud_msg_width/2,
			hud_pos_y + HUD_SIZE-HUD_OFFSET + BORDER
			+ gameFont->ascent
			+ j * (gameFont->ascent + gameFont->descent),
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
		    hud_pos_x - HUD_SIZE+HUD_OFFSET - BORDER - size,
		    hud_pos_y - HUD_SIZE+HUD_OFFSET - BORDER
			- gameFont->descent,
		    str, strlen(str));
	Erase_rectangle(hud_pos_x - HUD_SIZE+HUD_OFFSET - BORDER - size - 1,
			hud_pos_y - HUD_SIZE+HUD_OFFSET - BORDER,
			size + 2, gameFont->ascent + gameFont->descent);
    }

    /* Update the modifiers */
    modlen = strlen(mods);
    rd.drawString(dpy, p_draw, gc,
		hud_pos_x - HUD_SIZE+HUD_OFFSET-BORDER
		    - XTextWidth(gameFont, mods, modlen),
		hud_pos_y + HUD_SIZE-HUD_OFFSET+BORDER
		    + gameFont->ascent,
		mods, strlen(mods));

    Erase_rectangle(hud_pos_x - HUD_SIZE+HUD_OFFSET-BORDER
			- XTextWidth(gameFont, mods, modlen),
		    hud_pos_y + HUD_SIZE-HUD_OFFSET+BORDER,
			XTextWidth(gameFont, mods, modlen) + 2,
		    gameFont->ascent + gameFont->descent);

    if (autopilotLight) {
	rd.drawString(dpy, p_draw, gc,
		    hud_pos_x - XTextWidth(gameFont, autopilot,
					   sizeof(autopilot)-1)/2,
		    hud_pos_y - HUD_SIZE+HUD_OFFSET - BORDER
			- gameFont->descent * 2 - gameFont->ascent,
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
		  hud_pos_x + HUD_SIZE - HUD_OFFSET + FUEL_GAUGE_OFFSET - 1,
		  hud_pos_y - HUD_SIZE + HUD_OFFSET + FUEL_GAUGE_OFFSET - 1,
		  HUD_OFFSET - (2*FUEL_GAUGE_OFFSET) + 3,
		  HUD_FUEL_GAUGE_SIZE + 3);
    Erase_4point(hud_pos_x + HUD_SIZE - HUD_OFFSET + FUEL_GAUGE_OFFSET - 1,
		 hud_pos_y - HUD_SIZE + HUD_OFFSET + FUEL_GAUGE_OFFSET - 1,
		 HUD_OFFSET - (2*FUEL_GAUGE_OFFSET) + 3,
		 HUD_FUEL_GAUGE_SIZE + 3);

    size = (HUD_FUEL_GAUGE_SIZE * fuelSum) / fuelMax;
    rd.fillRectangle(dpy, p_draw, gc,
		   hud_pos_x + HUD_SIZE - HUD_OFFSET + FUEL_GAUGE_OFFSET + 1,
		   hud_pos_y - HUD_SIZE + HUD_OFFSET + FUEL_GAUGE_OFFSET
		   + HUD_FUEL_GAUGE_SIZE - size + 1,
		   HUD_OFFSET - (2*FUEL_GAUGE_OFFSET), size);
    Erase_rectangle(hud_pos_x + HUD_SIZE - HUD_OFFSET + FUEL_GAUGE_OFFSET + 1,
		    hud_pos_y - HUD_SIZE + HUD_OFFSET + FUEL_GAUGE_OFFSET
			+ HUD_FUEL_GAUGE_SIZE - size + 1,
		    HUD_OFFSET - (2*FUEL_GAUGE_OFFSET), size);
}

static void Paint_messages(void)
{
    int		i, x, y, top_y, bot_y, width, len;
    const int	BORDER = 10,
		SPACING = messageFont->ascent+messageFont->descent+1;
    message_t	*msg;

    if (charsPerTick <= 0.0)
	charsPerTick = (float)charsPerSecond / FPS;

    top_y = BORDER + messageFont->ascent;
    bot_y = view_height - messageFont->descent - BORDER;

    for (i = 0; i < 2 * MAX_MSGS; i++) {
	if (i < MAX_MSGS) {
	    msg = TalkMsg[i];
	} else {
	    msg = GameMsg[i - MAX_MSGS];
	}
	if (msg->len == 0)
	    continue;
	if (msg->life-- <= 0) {
	    msg->txt[0] = '\0';
	    msg->len = 0;
	    msg->life = 0;
	    continue;
	}
	if (i < MAX_MSGS) {
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
    tmp = msg_set[MAX_MSGS - 1];
    for (i = MAX_MSGS - 1; i > 0; i--) {
	msg_set[i] = msg_set[i - 1];
    }
    msg_set[0] = tmp;

    msg_set[0]->life = MSG_DURATION;
    strcpy(msg_set[0]->txt, message);
    msg_set[0]->len = len;
    msg_set[0]->pixelLen = XTextWidth(messageFont, msg_set[0]->txt, msg_set[0]->len);
}


static void Paint_radar(void)
{
    int			i, x, y, x1, y1, xw, yw;
    const float		xf = 256.0f / (float)Setup->width,
			yf = (float)RadarHeight / (float)Setup->height;

    if (radar_exposures == 0) {
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
	/* top */
	points[0].x = x;
	points[0].y = y;
	/* right */
	points[1].x = x + DSIZE;
	points[1].y = y - DSIZE;
	/* bottom */
	points[2].x = x;
	points[2].y = y - 2*DSIZE;
	/* left */
	points[3].x = x - DSIZE;
	points[3].y = y - DSIZE;
	/* top */
	points[4].x = x;
	points[4].y = y;
	XDrawLines(dpy, p_radar, radarGC,
		   points, 5, 0);
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
	int s;
	if ((s = radar_ptr[i].size) <= 0)
	    s = 1;
	x = (int)(radar_ptr[i].x * xf + 0.5) - s / 2;
	y = RadarHeight - (int)(radar_ptr[i].y * yf + 0.5) - 1 - s / 2;
	(*radarPlayerRectFN)(dpy, p_radar, radarGC, x, y, s, s);
	if (BIT(Setup->mode, WRAP_PLAY)) {
	    xw = (x < 0) ? -256 : (x + s >= 256) ? 256 : 0;
	    yw = (y < 0) ? -RadarHeight
			     : (y + s >= RadarHeight) ? RadarHeight : 0;
	    if (xw != 0) {
		(*radarPlayerRectFN)(dpy, p_radar, radarGC,
				     x - xw, y, s, s);
	    }
	    if (yw != 0) {
		(*radarPlayerRectFN)(dpy, p_radar, radarGC,
				     x, y - yw, s, s);

		if (xw != 0) {
		    (*radarPlayerRectFN)(dpy, p_radar, radarGC,
					 x - xw, y - yw, s, s);
		}
	    }
	}
    }
    if (num_radar)
	RELEASE(radar_ptr, num_radar, max_radar);
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
	    rd.drawLines(dpy, p_draw, gc, points, 4, 0);
	    Erase_points(0, points, 4);
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
#if ERASE
	    /* speedup for slow old cheap graphics cards like cg3. */
	    rd.drawLine(dpy, p_draw, gc,
		      X(x + FUEL_BORDER), Y(y + FUEL_BORDER + size),
		      X(x + FUEL_BORDER + (BLOCK_SZ - 2*FUEL_BORDER)),
		      Y(y + FUEL_BORDER + size));
#else
	    rd.fillRectangle(dpy, p_draw, gc,
			  X(x + FUEL_BORDER), Y(y + FUEL_BORDER + size),
			  BLOCK_SZ - 2*FUEL_BORDER + 1, size + 1);
#endif
	    Erase_rectangle(X(x + FUEL_BORDER),
			    Y(y - FUEL_BORDER + BLOCK_SZ),
			    BLOCK_SZ - 2*FUEL_BORDER + 1,
			    BLOCK_SZ - 2*FUEL_BORDER + 1);
	}
	/* Draw F in fuel cells */
	s[0] = 'F'; s[1] = '\0';
	XSetFunction(dpy, gc, GXxor);
	SET_FG(colors[BLACK].pixel ^ colors[RED].pixel);
	for (i = 0; i < num_vfuel; i++) {
	    x = vfuel_ptr[i].x;
	    y = vfuel_ptr[i].y;
	    rd.drawString(dpy, p_draw, gc,
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
			    X(x), Y(y),
			    s, 2);
		Erase_rectangle(X(x) - 1, Y(y) - gameFont->ascent,
				size + 2,
				gameFont->ascent + gameFont->descent);
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
			    X(x), Y(y),
			    other->name, other->name_len);
		Erase_rectangle(X(x) - 1, Y(y) - gameFont->ascent,
				other->name_width + 2,
				gameFont->ascent + gameFont->descent);
	    }
	}
	RELEASE(vbase_ptr, num_vbase, max_vbase);
    }
}

static void Paint_vdecor(void)
{
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
		    points[0].x = X(fill_bottom_left);
		    points[0].y = Y(y);
		    points[1].x = X(fill_top_left);
		    points[1].y = Y(y + BLOCK_SZ);
		    points[2].x = X(fill_top_right);
		    points[2].y = Y(y + BLOCK_SZ);
		    points[3].x = X(fill_bottom_right);
		    points[3].y = Y(y);
		    points[4] = points[0];
		    rd.fillPolygon(dpy, p_draw, gc,
				   points, 5,
				   Convex, CoordModeOrigin);
#if ERASE
		    {
			int left_x = MIN(fill_bottom_left, fill_top_left);
			int right_x = MAX(fill_bottom_right, fill_top_right);
			Erase_rectangle(X(left_x), Y(y + BLOCK_SZ),
					right_x - left_x + 1, BLOCK_SZ);
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
static void Paint_world(void)
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
			rd.fillPolygon(dpy, p_draw, gc,
				       points, 5,
				       Convex, CoordModeOrigin);
			Erase_rectangle(X(x), Y(y+BLOCK_SZ),
					BLOCK_SZ, BLOCK_SZ);
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
			    { 14, 3, 0, 1, 0 },
			    { 11, 5, 3, 2, 0 },
			    {  7, 8, 5, 3, 0 },
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
			    rdir = MOD2(rot_dir + tris[i].dir_off, RES);
			    cx = X(x + BLOCK_SZ / 2)
				+ tris[i].displ * tcos(rdir);
			    cy = Y(y + BLOCK_SZ / 2)
				+ tris[i].displ * tsin(rdir);
			    tdir = MOD2(tris[i].rot_dir, RES);
			    pts[0].x = cx + tris[i].radius * tcos(tdir);
			    pts[0].y = cy + tris[i].radius * tsin(tdir);
			    pts[1].x = cx + tris[i].radius
					  * tcos(MOD2(tdir + RES/3, RES));
			    pts[1].y = cy + tris[i].radius
					  * tsin(MOD2(tdir + RES/3, RES));
			    pts[2].x = cx + tris[i].radius
					  * tcos(MOD2(tdir + 2*RES/3, RES));
			    pts[2].y = cy + tris[i].radius
					  * tsin(MOD2(tdir + 2*RES/3, RES));
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
				     X(x+(BLOCK_SZ+2)/4),
				     Y(y+3*BLOCK_SZ/4),
				     BLOCK_SZ/2, BLOCK_SZ/2);
		    Erase_4point(X(x+(BLOCK_SZ+2)/4),
				 Y(y+3*BLOCK_SZ/4),
				 BLOCK_SZ/2, BLOCK_SZ/2);

		    if (BIT(Setup->mode, TEAM_PLAY)) {
			s[0] = '0' + type - SETUP_TARGET; s[1] = '\0';
			size = XTextWidth(gameFont, s, 1);
			rd.drawString(dpy, p_draw, gc,
				      X(x + BLOCK_SZ/2 - size/2),
				      Y(y + BLOCK_SZ/2 - gameFont->ascent/2),
				      s, 1);
			Erase_rectangle(X(x + BLOCK_SZ/2 - size/2) - 1,
					Y(y + BLOCK_SZ/2 - gameFont->ascent/2)
					    - gameFont->ascent,
					size + 2,
					gameFont->ascent + gameFont->descent);
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
				  X(x+BLOCK_SZ/2),
				  Y(y+BLOCK_SZ/2),
				  s, 1);
		    Erase_rectangle(X(x+BLOCK_SZ/2) - 1,
				    Y(y+BLOCK_SZ/2) - gameFont->ascent,
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
			points[0].x = X(fill_bottom_left);
			points[0].y = Y(y);
			points[1].x = X(fill_top_left);
			points[1].y = Y(y + BLOCK_SZ);
			points[2].x = X(fill_top_right);
			points[2].y = Y(y + BLOCK_SZ);
			points[3].x = X(fill_bottom_right);
			points[3].y = Y(y);
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
			    Erase_rectangle(X(left_x), Y(y + BLOCK_SZ),
					    right_x - left_x + 1, BLOCK_SZ);
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
	    points[0].x = X(fill_bottom_left);
	    points[0].y = Y(y);
	    points[1].x = X(fill_top_left);
	    points[1].y = Y(y + BLOCK_SZ);
	    points[2].x = X(x);
	    points[2].y = Y(y + BLOCK_SZ);
	    points[3].x = X(x);
	    points[3].y = Y(y);
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
		Erase_rectangle(X(left_x), Y(y + BLOCK_SZ),
				x - left_x + 1, BLOCK_SZ);
	    }
#endif
	    fill_top_left =
	    fill_top_right =
	    fill_bottom_left =
	    fill_bottom_right = -1;
	}
    }
}


void Paint_sliding_radar(void)
{
    if (BIT(Setup->mode, WRAP_PLAY) == 0) {
	return;
    }
    if (p_radar != s_radar) {
	return;
    }
    if (BIT(instruments, SHOW_SLIDING_RADAR) != 0) {
	if (s_radar != radar) {
	    return;
	}
	s_radar = XCreatePixmap(dpy, radar,
				256, RadarHeight,
				dispDepth);
				/* not anymore since 3.3.0:
				DefaultDepth(dpy, DefaultScreen(dpy))); */
	p_radar = s_radar;
	if (radar_exposures > 0) {
	    Paint_world_radar();
	}
    } else {
	if (s_radar == radar) {
	    return;
	}
	XFreePixmap(dpy, s_radar);
	s_radar = radar;
	p_radar = s_radar;
	if (radar_exposures > 0) {
	    Paint_world_radar();
	}
    }
}


void Paint_world_radar(void)
{
    int			i, xi, yi, xm, ym, xp, yp = 0, xmoff, xioff;
    int			type, vis, damage;
    float		xs, ys;
    int			npoint = 0, nsegment = 0, start, end, currColor, visibleColorChange;
    const int		max = 256;
    u_byte		visible[256], visibleColor[256];
    XSegment		segments[256];
    XPoint		points[256];

    radar_exposures = 2;

    if (s_radar == p_radar)
	XSetPlaneMask(dpy, radarGC,
		      AllPlanes&(~(dpl_1[0]|dpl_1[1])));
    if (s_radar != radar) {
	/* Clear radar */
	XSetForeground(dpy, radarGC, colors[BLACK].pixel);
	XFillRectangle(dpy, s_radar, radarGC, 0, 0, 256, RadarHeight);
    } else {
	XClearWindow(dpy, radar);
    }

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
    for (i = BLUE_BIT; i < sizeof visible; i++) {
	visible[i] = 1;
    }
    if (BIT(instruments, SHOW_DECOR)) {
	visible[SETUP_DECOR_FILLED] = 1;
	visible[SETUP_DECOR_LU] = 1;
	visible[SETUP_DECOR_RU] = 1;
	visible[SETUP_DECOR_LD] = 1;
	visible[SETUP_DECOR_RD] = 1;
    }

    memset(visibleColor, 0, sizeof visibleColor);
    visibleColor[SETUP_FILLED] = 
	visibleColor[SETUP_FILLED_NO_DRAW] =
	visibleColor[SETUP_REC_LU] =
	visibleColor[SETUP_REC_RU] =
	visibleColor[SETUP_REC_LD] =
	visibleColor[SETUP_REC_RD] =
	visibleColor[SETUP_FUEL] = wallRadarColor;
    for (i = 0; i < 10; i++) {
	visibleColor[SETUP_TARGET+i] = wallRadarColor;
    }
    for (i = BLUE_BIT; i < sizeof visible; i++) {
	visibleColor[i] = wallRadarColor;
    }
    if (BIT(instruments, SHOW_DECOR)) {
	visibleColor[SETUP_DECOR_FILLED] =
	    visibleColor[SETUP_DECOR_LU] = 
	    visibleColor[SETUP_DECOR_RU] = 
	    visibleColor[SETUP_DECOR_LD] = 
	    visibleColor[SETUP_DECOR_RD] = decorRadarColor;
    }

    /* The following code draws the map on the radar.  Segments and
       points arrays are use to build lists of things to be drawn.
       Normally the segments and points are drawn when the arrays are
       full, but now they are also drawn when the color changes.  The
       visibleColor array is used to determine the color to be used
       for the given visible block type.

       Another (and probably better) way to do this would be use
       different segments and points arrays for each visible color.  */
       
    if (Setup->x >= 256) {
	xs = (float)(256 - 1) / (Setup->x - 1);
	ys = (float)(RadarHeight - 1) / (Setup->y - 1);
	currColor = -1;
	for (xi = 0; xi < Setup->x; xi++) {
	    start = end = -1;
	    xp = (int)(xi * xs + 0.5);
	    xioff = xi * Setup->y;
	    for (yi = 0; yi < Setup->y; yi++) {
		visibleColorChange = 0;
		type = Setup->map_data[xioff + yi];
		if (type >= SETUP_TARGET && type < SETUP_TARGET+10)
		    vis = (Target_alive(xi, yi, &damage) == 0);
		else vis = visible[type];
		if (vis) {
		    yp = (int)(yi * ys + 0.5);
		    if (start == -1) {
			if ((nsegment > 0 || npoint > 0) && currColor != visibleColor[type]) {
			    if (nsegment > 0) {
				XDrawSegments(dpy, s_radar, radarGC,
					      segments, nsegment);
				nsegment = 0;
			    }
			    if (npoint > 0) {
				XDrawPoints(dpy, s_radar, radarGC,
					    points, npoint, CoordModeOrigin);
				npoint = 0;
			    }
			}
			start = end = yp;
			currColor = visibleColor[type];
			XSetForeground(dpy, radarGC, colors[currColor].pixel);
		    } else {
			end = yp;
			visibleColorChange = visibleColor[type] != currColor;
		    }
		}
		
		if (start != -1 && (!vis || yi == Setup->y - 1 || visibleColorChange)) {
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

		if (visibleColorChange) {
		    if (nsegment > 0) {
			XDrawSegments(dpy, s_radar, radarGC,
				      segments, nsegment);
			nsegment = 0;
		    }
		    if (npoint > 0) {
			XDrawPoints(dpy, s_radar, radarGC,
				    points, npoint, CoordModeOrigin);
			npoint = 0;
		    }
		    start = end = yp;
		    currColor = visibleColor[type];
		    XSetForeground(dpy, radarGC, colors[currColor].pixel);
		}
	    }
	}
    } else {
	xs = (float)(Setup->x - 1) / (256 - 1);
	ys = (float)(Setup->y - 1) / (RadarHeight - 1);
	currColor = -1;
	for (xi = 0; xi < 256; xi++) {
	    xm = (int)(xi * xs + 0.5);
	    xmoff = xm * Setup->y;
	    start = end = -1;
	    xp = xi;
	    for (yi = 0; yi < RadarHeight; yi++) {
		visibleColorChange = 0;
		ym = (int)(yi * ys + 0.5);
		type = Setup->map_data[xmoff + ym];
		vis = visible[type];
		if (type >= SETUP_TARGET && type < SETUP_TARGET+10)
		    vis = (Target_alive(xm, ym, &damage) == 0);
		if (vis) {
		    yp = yi;
		    if (start == -1) {
			if ((nsegment > 0 || npoint > 0) && currColor != visibleColor[type]) {
			    if (nsegment > 0) {
				XDrawSegments(dpy, s_radar, radarGC,
					      segments, nsegment);
				nsegment = 0;
			    }
			    if (npoint > 0) {
				XDrawPoints(dpy, s_radar, radarGC,
					    points, npoint, CoordModeOrigin);
				npoint = 0;
			    }
			}
			start = end = yp;
			currColor = visibleColor[type];
			XSetForeground(dpy, radarGC, colors[currColor].pixel);
		    } else {
			end = yp;
			visibleColorChange = visibleColor[type] != currColor;
		    }
		}
		
		if (start != -1 && (!vis || yi == RadarHeight - 1 || visibleColorChange)) {
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

		if (visibleColorChange) {
		    if (nsegment > 0) {
			XDrawSegments(dpy, s_radar, radarGC,
				      segments, nsegment);
			nsegment = 0;
		    }
		    if (npoint > 0) {
			XDrawPoints(dpy, s_radar, radarGC,
				    points, npoint, CoordModeOrigin);
			npoint = 0;
		    }
		    start = end = yp;
		    currColor = visibleColor[type];
		    XSetForeground(dpy, radarGC, colors[currColor].pixel);
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

    for (i = 0;; i++) {
	int dead_time, damage;
	if (Target_by_index(i, &xi, &yi, &dead_time, &damage) == -1) {
	    break;
	}
	if (dead_time) {
	    continue;
	}
	Paint_radar_block(xi, yi, targetRadarColor);
    }
}

/*
 * Try and draw an area of the radar which represents block position
 * `xi' `yi'.  If `draw' is zero the area is cleared.
 */
void Paint_radar_block(int xi, int yi, int color)
{
    float	xs, ys;
    int		xp, yp, xw, yw;

    if (s_radar == p_radar)
	XSetPlaneMask(dpy, radarGC, AllPlanes&(~(dpl_1[0]|dpl_1[1])));
    XSetForeground(dpy, radarGC, colors[color].pixel);

    if (Setup->x >= 256) {
	xs = (float)(256 - 1) / (Setup->x - 1);
	ys = (float)(RadarHeight - 1) / (Setup->y - 1);
	xp = (int)(xi * xs + 0.5);
	yp = RadarHeight - 1 - (int)(yi * ys + 0.5);
	XDrawPoint(dpy, s_radar, radarGC, xp, yp);
    } else {
	xs = (float)(Setup->x - 1) / (256 - 1);
	ys = (float)(Setup->y - 1) / (RadarHeight - 1);
	/*
	 * Calculate the min and max points on the radar that would show
	 * block position `xi' and `yi'.  Note `xp' is the minimum x coord
	 * for `xi',which is one more than the previous xi value would give,
	 * and `xw' is the maximum, which is then changed to a width value.
	 * Similarly for `yw' and `yp' (the roles are reversed because the
	 * radar is upside down).
	 */
	xp = (int)((xi - 0.5) / xs) + 1;
	xw = (int)((xi + 0.5) / xs);
	yw = (int)((yi - 0.5) / ys) + 1;
	yp = (int)((yi + 0.5) / ys);
	xw -= xp;
	yw = yp - yw;
	yp = RadarHeight - 1 - yp;
	XFillRectangle(dpy, s_radar, radarGC, xp, yp, xw+1, yw+1);
    }
    if (s_radar == p_radar)
	XSetPlaneMask(dpy, radarGC,
		      AllPlanes&(~(dpl_2[0]|dpl_2[1])));
}

void Paint_REC(void)
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

void Paint_frame(void)
{
    static long		scroll_i = 0;
    static int		prev_damaged = 0;
    static int		prev_prev_damaged = 0;

    if (start_loops != end_loops) {
	errno = 0;
	error("Start neq. End (%ld,%ld,%ld)", start_loops, end_loops, loops);
    }
    loops = end_loops;

    /*
     * Switch between two different window titles.
     */
    if (titleFlip && (loops % TITLE_DELAY) == 0) {
	scroll_i = !scroll_i;
	if (scroll_i)
	    XStoreName(dpy, top, COPYRIGHT);
	else
	    XStoreName(dpy, top, TITLE);

    }

    rd.newFrame();


    /*
     * Do we really need to draw all this if the player is damaged?
     */
    if (damaged <= 0) {
	if (prev_prev_damaged) {
	    SET_FG(colors[BLACK].pixel);
	    XFillRectangle(dpy, draw, gc, 0, 0, draw_width, draw_height);
	}

	Erase_start();

	Arc_start();

	Rectangle_start();
	Segment_start();
	Paint_world();
	Segment_end();
	Rectangle_end();

	Rectangle_start();
	Segment_start();

	Paint_vfuel();
	Paint_vdecor();
	Paint_vcannon();
	Paint_vbase();
	Paint_shots();

	Rectangle_end();
	Segment_end();

	Rectangle_start();
	Segment_start();

	Paint_ships();
	Paint_meters();
	Paint_HUD();

	Paint_REC();

	Rectangle_end();
	Segment_end();

	Arc_end();

	Paint_messages();
	Paint_radar();
	Paint_score_objects();
    }
    else {
	/* Damaged. */

	XSetFunction(dpy, gc, GXxor);
	SET_FG(colors[BLACK].pixel ^ colors[BLUE].pixel);
	XFillRectangle(dpy, draw, gc, 0, 0, draw_width, draw_height);
	XSetFunction(dpy, gc, GXcopy);
	SET_FG(colors[BLACK].pixel);
    }
    prev_prev_damaged = prev_damaged;
    prev_damaged = damaged;

    rd.endFrame();

    if (radar_exposures == 1) {
	Paint_world_radar();
    }

    /*
     * Now switch planes and clear the screen.
     */
    if (p_radar != radar && radar_exposures > 0) {
	if (BIT(instruments, SHOW_SLIDING_RADAR) == 0
	    || BIT(Setup->mode, WRAP_PLAY) == 0) {
	    XCopyArea(dpy, p_radar, radar, gc,
		      0, 0, 256, RadarHeight, 0, 0);
	} else {
	    int x, y, w, h;
	    float xp, yp, xo, yo;

	    xp = (float) (pos.x * 256) / Setup->width;
	    yp = (float) (pos.y * RadarHeight) / Setup->height;
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
    else if (radar_exposures > 2) {
	Paint_world_radar();
    }
    if (dbuf_state->type == PIXMAP_COPY) {
	XCopyArea(dpy, p_draw, draw, gc,
		  0, 0, view_width, view_height, 0, 0);
    }

    dbuff_switch(dbuf_state);

    if (dbuf_state->type == COLOR_SWITCH) {
	XSetPlaneMask(dpy, gc, dbuf_state->drawing_planes);
	XSetPlaneMask(dpy, messageGC, dbuf_state->drawing_planes);
    }

    if (!damaged) {
	/* Prepare invisible buffer for next frame by clearing. */
#if ERASE
	Erase_end();
#else
	SET_FG(colors[BLACK].pixel);
	XFillRectangle(dpy, p_draw, gc, 0, 0, draw_width, draw_height);
#endif
    }

    if (talk_mapped == true) {
	static bool toggle;
	static long last_toggled;

	if (loops >= last_toggled + FPS / 2 || loops < last_toggled) {
	    toggle = (toggle == false) ? true : false;
	    last_toggled = loops;
	}
	Talk_cursor(toggle);
    }
    Paint_clock(0);

    XFlush(dpy);
}


int Handle_start(long server_loops)
{
    int			i;

    start_loops = server_loops;

    num_refuel = 0;
    num_connector = 0;
    num_missile = 0;
    num_ball = 0;
    num_ship = 0;
    num_mine = 0;
    num_itemtype = 0;
    num_ecm = 0;
    num_trans = 0;
    num_paused = 0;
    num_radar = 0;
    num_vcannon = 0;
    num_vfuel = 0;
    num_vbase = 0;
    num_vdecor = 0;
    for (i = 0; i < DEBRIS_TYPES; i++) {
	num_debris[i] = 0;
    }

    damaged = 0;
    destruct = 0;
    shutdown_delay = 0;
    shutdown_count = -1;
    eyesId = (self != NULL) ? self->id : 0;
    thrusttime = -1;
    shieldtime = -1;
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
		int newNextCheckPoint, int newAutopilotLight,
		u_byte *newNumItems, int newCurrentTank,
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
    autopilotLight = newAutopilotLight;
    memcpy(numItems, newNumItems, NUM_ITEMS * sizeof(u_byte));
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

    world.x = pos.x - (view_width / 2);
    world.y = pos.y - (view_height / 2);
    realWorld = world;
    if (BIT(Setup->mode, WRAP_PLAY)) {
	if (world.x < 0 && world.x + view_width < Setup->width) {
	    world.x += Setup->width;
	}
	else if (world.x > 0 && world.x + view_width >= Setup->width) {
	    realWorld.x -= Setup->width;
	}
	if (world.y < 0 && world.y + view_height < Setup->height) {
	    world.y += Setup->height;
	}
	else if (world.y > 0 && world.y + view_height >= Setup->height) {
	    realWorld.y -= Setup->height;
	}
    }
    return 0;
}


int Handle_eyes(int id)
{
    eyesId = id;
    return 0;
}

int Handle_damaged(int dam)
{
    damaged = dam;
    return 0;
}

int Handle_modifiers(char *m)
{
    strncpy(mods, m, MAX_CHARS);
    mods[MAX_CHARS-1] = '\0';
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

int Handle_thrusttime(int count, int max)
{
    thrusttime = count;
    thrusttimemax = max;
    return 0;
}

int Handle_shieldtime(int count, int max)
{
    shieldtime = count;
    shieldtimemax = max;
    return 0;
}

int Handle_refuel(int x0, int y0, int x1, int y1)
{
    refuel_t	t;

    t.x0 = x0;
    t.x1 = x1;
    t.y0 = y0;
    t.y1 = y1;
    HANDLE(refuel_t, refuel_ptr, num_refuel, max_refuel, t);
    return 0;
}

int Handle_connector(int x0, int y0, int x1, int y1, int tractor)
{
    connector_t	t;

    t.x0 = x0;
    t.x1 = x1;
    t.y0 = y0;
    t.y1 = y1;
    t.tractor = tractor;
    HANDLE(connector_t, connector_ptr, num_connector, max_connector, t);
    return 0;
}

int Handle_laser(int color, int x, int y, int len, int dir)
{
    laser_t	t;

    t.color = color;
    t.x = x;
    t.y = y;
    t.len = len;
    t.dir = dir;
    HANDLE(laser_t, laser_ptr, num_laser, max_laser, t);
    return 0;
}

int Handle_missile(int x, int y, int len, int dir)
{
    missile_t	t;

    t.x = x;
    t.y = y;
    t.dir = dir;
    t.len = len;
    HANDLE(missile_t, missile_ptr, num_missile, max_missile, t);
    return 0;
}

int Handle_ball(int x, int y, int id)
{
    ball_t	t;

    t.x = x;
    t.y = y;
    t.id = id;
    HANDLE(ball_t, ball_ptr, num_ball, max_ball, t);
    return 0;
}

int Handle_ship(int x, int y, int id, int dir, int shield, int cloak, int eshield)
{
    ship_t	t;

    t.x = x;
    t.y = y;
    t.id = id;
    t.dir = dir;
    t.shield = shield;
    t.cloak = cloak;
    t.eshield = eshield;
    HANDLE(ship_t, ship_ptr, num_ship, max_ship, t);

    if (id == eyesId) {
	selfVisible = 1;
	return Handle_radar(x, y, 3);
    }

    return 0;
}

int Handle_mine(int x, int y, int teammine, int id)
{
    mine_t	t;

    t.x = x;
    t.y = y;
    t.teammine = teammine;
    t.id = id;
    HANDLE(mine_t, mine_ptr, num_mine, max_mine, t);
    return 0;
}

int Handle_item(int x, int y, int type)
{
    itemtype_t	t;

    t.x = x;
    t.y = y;
    t.type = type;
    HANDLE(itemtype_t, itemtype_ptr, num_itemtype, max_itemtype, t);
    return 0;
}

#define HANDLE_DEBRIS(_type, _p, _n) \
    if (_n > max) {						\
	if (max == 0) {						\
	    ptr = (debris_t *)malloc(n * sizeof(*ptr));		\
	} else {						\
	    ptr = (debris_t *)realloc(ptr, _n * sizeof(*ptr));	\
	}							\
	if (ptr == NULL) {					\
	    error("No memory for debris");			\
	    num = max = 0;					\
	    return -1;						\
	}							\
	max = _n;						\
    }								\
    else if (_n <= 0) {						\
	printf("debris %d < 0\n", _n);				\
	return 0;						\
    }								\
    num = _n;							\
    memcpy(ptr, _p, _n * sizeof(*ptr));				\
    return 0;


int Handle_fastshot(int type, u_byte *p, int n)
{
#define num		(num_fastshot[type])
#define max		(max_fastshot[type])
#define ptr		(fastshot_ptr[type])
    HANDLE_DEBRIS(type, p, n);
#undef num
#undef max
#undef ptr
}

int Handle_debris(int type, u_byte *p, int n)
{
#define num		(num_debris[type])
#define max		(max_debris[type])
#define ptr		(debris_ptr[type])
    HANDLE_DEBRIS(type, p, n);
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
    HANDLE(ecm_t, ecm_ptr, num_ecm, max_ecm, t);
    return 0;
}

int Handle_trans(int x1, int y1, int x2, int y2)
{
    trans_t	t;

    t.x1 = x1;
    t.y1 = y1;
    t.x2 = x2;
    t.y2 = y2;
    HANDLE(trans_t, trans_ptr, num_trans, max_trans, t);
    return 0;
}

int Handle_paused(int x, int y, int count)
{
    paused_t	t;

    t.x = x;
    t.y = y;
    t.count = count;
    HANDLE(paused_t, paused_ptr, num_paused, max_paused, t);
    return 0;
}

int Handle_radar(int x, int y, int size)
{
    radar_t	t;

    t.x = x;
    t.y = y;
    t.size = size;
    HANDLE(radar_t, radar_ptr, num_radar, max_radar, t);
    return 0;
}

int Handle_message(char *msg)
{
    Add_message(msg);
    return 0;
}

int Handle_time_left(long sec)
{
    if (sec >= 0 && sec < 10 && (time_left > sec || sec == 0)) {
	XBell(dpy, 0);
	XFlush(dpy);
    }
    time_left = (sec >= 0) ? sec : 0;
    return 0;
}

int Handle_vcannon(int x, int y, int type)
{
    vcannon_t	t;

    t.x = x;
    t.y = y;
    t.type = type;
    HANDLE(vcannon_t, vcannon_ptr, num_vcannon, max_vcannon, t);
    return 0;
}

int Handle_vfuel(int x, int y, long fuel)
{
    vfuel_t	t;

    t.x = x;
    t.y = y;
    t.fuel = fuel;
    HANDLE(vfuel_t, vfuel_ptr, num_vfuel, max_vfuel, t);
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
    HANDLE(vbase_t, vbase_ptr, num_vbase, max_vbase, t);
    return 0;
}

int Handle_vdecor(int x, int y, int xi, int yi, int type)
{
    vdecor_t	t;

    t.x = x;
    t.y = y;
    t.xi = xi;
    t.yi = yi;
    t.type = type;
    HANDLE(vdecor_t, vdecor_ptr, num_vdecor, max_vdecor, t);
    return 0;
}
		   
#define BORDER			6
#define SCORE_LIST_WINDOW_WIDTH	256

void Paint_score_start(void)
{
    static bool	first = true;
    char	headingStr[MSG_LEN];
    static int thisLine;

    if (first) {
	thisLine = BORDER + scoreListFont->ascent;
	first = false;
    }

    if (showRealName) {
	strcpy(headingStr, "NICK=USER@HOST");
    } else {
	strcpy(headingStr, "  ");
	if (BIT(Setup->mode, TIMING)) {
	    if (version >= 0x3261) {
		strcat(headingStr, "LAP ");
	    }
	}
	else if (BIT(Setup->mode, TEAM_PLAY)) {
	    strcpy(headingStr, " TM ");
	}
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

    Paint_clock(1);
}


void Paint_score_entry(int entry_num,
		       other_t* other,
		       bool best)
{
    static char		raceStr[8], teamStr[4], lifeStr[8], label[MSG_LEN];
    static int		lineSpacing = -1, firstLine;
    int			thisLine;

    /*
     * First time we're here, set up miscellaneous strings for
     * efficiency and calculate some other constants.
     */
    if (lineSpacing == -1) {
	memset(raceStr, '\0', sizeof raceStr);
	memset(teamStr, '\0', sizeof teamStr);
	memset(lifeStr, '\0', sizeof lifeStr);
	teamStr[1] = ' ';
	raceStr[2] = ' ';

	lineSpacing
	    = scoreListFont->ascent + scoreListFont->descent + 3;
	firstLine
	    = 2*BORDER + scoreListFont->ascent + lineSpacing;
    }
    thisLine = firstLine + lineSpacing * entry_num;

    /*
     * Setup the status line
     */
    if (showRealName) {
	sprintf(label, "%s=%s@%s", other->name, other->real, other->host);
    } else {
	other_t*	war = Other_by_id(other->war_id);

	if (BIT(Setup->mode, TIMING)) {
	    raceStr[0] = ' ';
	    raceStr[1] = ' ';
	    if (version >= 0x3261) {
		if ((other->mychar == ' ' || other->mychar == 'R')
		    && other->round + other->check > 0) {
		    if (other->round > 99) {
			sprintf(raceStr, "%3d", other->round);
		    }
		    else {
			sprintf(raceStr, "%d.%c",
				other->round, other->check + 'a');
		    }
		}
	    }
	}
	else if (BIT(Setup->mode, TEAM_PLAY)) {
	    teamStr[0] = other->team + '0';
	}

	if (BIT(Setup->mode, LIMITED_LIVES))
	    sprintf(lifeStr, " %3d", other->life);

	if (war) {
	    sprintf(label, "%c %s%s%5d%s  %s (%s)",
		    other->mychar, raceStr, teamStr, other->score, lifeStr,
		    other->name, war->name);
	} else {
	    sprintf(label, "%c %s%s%5d%s  %s",
		    other->mychar, raceStr, teamStr, other->score, lifeStr,
		    other->name);
	}
    }

    /*
     * Draw the line
     */
    if ((other->mychar == 'D'
	|| other->mychar == 'P'
	|| other->mychar == 'W')
	&& !mono) {
	XSetForeground(dpy, scoreListGC, colors[BLACK].pixel);
	XDrawString(dpy, players, scoreListGC,
		    BORDER, thisLine,
		    label, strlen(label));
    } else {
	ShadowDrawString(dpy, players, scoreListGC,
			 BORDER, thisLine,
			 label,
			 colors[WHITE].pixel,
			 colors[BLACK].pixel);
    }

    /*
     * Underline the best player
     */
    if (best) {
	XDrawLine(dpy, players, scoreListGC,
		  BORDER, thisLine,
		  SCORE_LIST_WINDOW_WIDTH - BORDER, thisLine);
    }
}


static void Paint_clock(int redraw)
{
    int			minute,
			hour,
			height = scoreListFont->ascent + scoreListFont->descent
				+ 3,
			border = 3;
    time_t		t;
    struct tm		*m;
    char		buf[16];
    static long		prev_loops;
    static int		width;

    if (BIT(instruments, SHOW_CLOCK) == 0) {
	if (width != 0) {
	    XSetForeground(dpy, scoreListGC, colors[windowColor].pixel);
	    XFillRectangle(dpy, players, scoreListGC,
			   256 - (width + 2 * border), 0,
			   width + 2 * border, height);
	    width = 0;
	}
	return;
    }
    if (redraw == 0
	&& loops > prev_loops
	&& loops - prev_loops < (FPS << 5)) {
	return;
    }
    prev_loops = loops;
    time(&t);
    m = localtime(&t);

    /* round seconds up to next minute. */
    minute = m->tm_min;
    hour = m->tm_hour;
    if (minute++ == 59) {
	minute = 0;
	if (hour++ == 23) {
	    hour = 0;
	}
    }
    if (!BIT(instruments, SHOW_CLOCK_AMPM_FORMAT)) {
	sprintf(buf, "%02d:%02d", hour, minute);
    } else {
	char tmpchar = 'A';
	/* strftime(buf, sizeof(buf), "%l:%M%p", m); */
	if (m->tm_hour > 12){
	    tmpchar = 'P';
	    m->tm_hour %= 12;
	}
	sprintf(buf, "%2d:%02d%cM", m->tm_hour, m->tm_min, tmpchar);
    }
    width = XTextWidth(scoreListFont, buf, strlen(buf));
    XSetForeground(dpy, scoreListGC, colors[windowColor].pixel);
    XFillRectangle(dpy, players, scoreListGC,
		   256 - (width + 2 * border), 0,
		   width + 2 * border, height);
    ShadowDrawString(dpy, players, scoreListGC,
		     256 - (width + border),
		     scoreListFont->ascent + 4,
		     buf,
		     colors[WHITE].pixel,
		     colors[BLACK].pixel);
}

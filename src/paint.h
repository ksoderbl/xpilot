/* $Id: paint.h,v 3.14 1993/08/02 12:55:26 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bj�rn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert G�sbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef PAINT_H
#define PAINT_H

#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include "types.h"
#include "dbuff.h"
#include "keys.h"
#include "client.h"

void Add_message(char *message);
int Handle_start(long server_loops);
int Handle_end(long server_loops);
int Handle_self(int x, int y, int vx, int vy, int dir,
    float power, float turnspeed, float turnresistance,
    int lock_id, int lock_dist, int lock_dir,
    int nextCheckPoint, int numCloaks, int numSensors, int numMines,
    int numRockets, int numEcms, int numTransporters, int numFrontShots, int numBackShots,
    int numAfterburners, int num_tanks, int currentTank,
    int fuel_sum, int fuel_max, int packet_size);
int Handle_damaged(int damaged);
int Handle_destruct(int count);
int Handle_shutdown(int count, int delay);
int Handle_refuel(int x0, int y0, int x1, int y1);
int Handle_connector(int x0, int y0, int x1, int y1);
int Handle_smart(int x, int y, int dir);
int Handle_ball(int x, int y, int id);
int Handle_ship(int x, int y, int id, int dir, int shield, int cloak);
int Handle_mine(int x, int y);
int Handle_item(int x, int y, int type);
int Handle_shot(int x, int y, int color);
int Handle_debris(int type, u_byte *p, int n);
int Handle_ecm(int x, int y, int size);
int Handle_trans(int x1, int y1, int x2, int y2);
int Handle_paused(int x, int y, int count);
int Handle_radar(int x, int y);
int Handle_vcannon(int x, int y, int type);
int Handle_vfuel(int x, int y, long fuel);
int Handle_vbase(int x, int y, int xi, int yi, int type);
int Handle_message(char *msg);
void Paint_item(u_byte type, Drawable d, GC gc, int x, int y);
void Paint_world_radar(void);

/*
 * Global objects.
 */

/* The fonts used in the game */
extern XFontStruct* gameFont;
extern XFontStruct* messageFont;
extern XFontStruct* scoreListFont;
extern XFontStruct* buttonFont;
extern XFontStruct* textFont;

/* The name of the fonts used in the game */
extern char gameFontName[FONT_LEN];
extern char messageFontName[FONT_LEN];
extern char scoreListFontName[FONT_LEN];
extern char buttonFontName[FONT_LEN];
extern char textFontName[FONT_LEN];

extern Display	*dpy;			/* Display of player (pointer) */
extern short	about_page;		/* Which page is the player on? */
extern u_short	team;			/* What team is the player on? */
extern bool	players_exposed;	/* Is score window exposed? */
extern bool	radar_exposed;		/* Is radar window exposed? */

#define MAX_COLOR_LEN		32

/* Display type */
#define	DT_NONE			0
#define	DT_HAVE_COLOR		1
#define	DT_HAVE_PLANES		2
#define	DT_IS_DISPLAY		4
extern u_byte	dpy_type;

extern GC	gc, messageGC, radarGC, buttonGC, scoreListGC, textGC;
extern Window	top, draw, radar, players;
extern Pixmap	p_draw, p_radar, s_radar;
extern Pixmap	itemBitmaps[];
extern long	dpl_1[2], dpl_2[2];	/* Used by radar hack */
extern Window	quit_b, about_b, help_b,
		about_w, about_close_b, about_next_b, about_prev_b,
		help_w, help_close_b, talk_w;
extern XColor	colors[4];		/* Colors */
extern Colormap	colormap;		/* Private colormap */
extern bool	gotFocus;
extern bool	talk_mapped;
extern char	color_names[4][MAX_COLOR_LEN];	/* User defined colors */
extern char	*color_defaults[4];		/* Default colors definitions */

extern int		maxKeyDefs;
extern keydefs_t	*keyDefs;
extern other_t*		self;		/* Player info */
extern dbuff_state_t*	dbuf_state;	/* Holds current dbuff state */

#endif
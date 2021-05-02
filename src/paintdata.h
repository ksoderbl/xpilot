/* $Id: paintdata.h,v 3.6 1997/11/27 20:09:25 bert Exp $
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

#ifndef PAINTTYPES_H
#define PAINTTYPES_H

/* need u_byte */
#ifndef	TYPES_H
#include "types.h"
#endif

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
#define STORE(T,P,N,M,V)						\
    if (N >= M && ((M <= 0)						\
	? (P = (T *) malloc((M = 1) * sizeof(*P)))			\
	: (P = (T *) realloc(P, (M += M) * sizeof(*P)))) == NULL) {	\
	error("No memory");						\
	N = M = 0;							\
	return -1;							\
    } else								\
	(P[N++] = V)

/*
 * Local types and data for painting.
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

extern refuel_t		*refuel_ptr;
extern int		 num_refuel, max_refuel;
extern connector_t	*connector_ptr;
extern int		 num_connector, max_connector;
extern laser_t		*laser_ptr;
extern int		 num_laser, max_laser;
extern missile_t	*missile_ptr;
extern int		 num_missile, max_missile;
extern ball_t		*ball_ptr;
extern int		 num_ball, max_ball;
extern ship_t		*ship_ptr;
extern int		 num_ship, max_ship;
extern mine_t		*mine_ptr;
extern int		 num_mine, max_mine;
extern itemtype_t	*itemtype_ptr;
extern int		 num_itemtype, max_itemtype;
extern ecm_t		*ecm_ptr;
extern int		 num_ecm, max_ecm;
extern trans_t		*trans_ptr;
extern int		 num_trans, max_trans;
extern paused_t		*paused_ptr;
extern int		 num_paused, max_paused;
extern radar_t		*radar_ptr;
extern int		 num_radar, max_radar;
extern vcannon_t	*vcannon_ptr;
extern int		 num_vcannon, max_vcannon;
extern vfuel_t		*vfuel_ptr;
extern int		 num_vfuel, max_vfuel;
extern vbase_t		*vbase_ptr;
extern int		 num_vbase, max_vbase;
extern debris_t		*debris_ptr[DEBRIS_TYPES];
extern int		 num_debris[DEBRIS_TYPES],
			 max_debris[DEBRIS_TYPES];
extern debris_t		*fastshot_ptr[DEBRIS_TYPES * 2];
extern int		 num_fastshot[DEBRIS_TYPES * 2],
			 max_fastshot[DEBRIS_TYPES * 2];
extern vdecor_t		*vdecor_ptr;
extern int		 num_vdecor, max_vdecor;

extern long		start_loops, end_loops;
extern long		time_left;

#define RESET_FG()	(current_foreground = -1)
#define SET_FG(PIXEL)				\
    if ((PIXEL) == current_foreground) ;	\
    else XSetForeground(dpy, gc, current_foreground = (PIXEL))

extern unsigned long	current_foreground;

#ifndef ERASE
#define ERASE		0
#endif

#define ERASE_INITIALIZED	(1 << 0)

#if ERASE

#define MAX_LINE_WIDTH	4

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

extern erase_t		erase[2],
			*erp;
#endif	/* ERASE */

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

#ifndef PAINT_FREE
# define PAINT_FREE	1
#endif
#if PAINT_FREE
# define RELEASE(P, N, M)	if (!(N)) ; else (free(P), (M) = 0, (N) = 0)
#else
# define RELEASE(P, N, M)	((N) = 0)
#endif

extern XRectangle	*rect_ptr[MAX_COLORS];
extern int		num_rect[MAX_COLORS], max_rect[MAX_COLORS];
extern XArc		*arc_ptr[MAX_COLORS];
extern int		num_arc[MAX_COLORS], max_arc[MAX_COLORS];
extern XSegment		*seg_ptr[MAX_COLORS];
extern int		num_seg[MAX_COLORS], max_seg[MAX_COLORS];

#if ERASE
extern void Erase_start(void);
extern void Erase_end(void);
extern void Erase_rectangle(int x, int y, int width, int height);
extern void Erase_rectangles(XRectangle *rectp, int n);
extern void Erase_arc(int x, int y, int width, int height,
		      int angle1, int angle2);
extern void Erase_arcs(XArc *arcp, int n);
extern void Erase_segment(int width, int x1, int y1, int x2, int y2);
extern void Erase_segments(XSegment *segp, int n);
extern void Erase_points(int width, XPoint *pointp, int n);
extern void Erase_4point(int x, int y, int width, int height);
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

extern void Rectangle_start(void);
extern void Rectangle_end(void);
extern int Rectangle_add(int color, int x, int y, int width, int height);
extern void Arc_start(void);
extern void Arc_end(void);
extern int Arc_add(int color,
		   int x, int y,
		   int width, int height,
		   int angle1, int angle2);
extern void Segment_start(void);
extern void Segment_end(void);
extern int Segment_add(int color, int x1, int y1, int x2, int y2);

#endif

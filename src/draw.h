/* $Id: draw.h,v 3.20 1994/04/12 13:43:10 bjoerns Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-94 by
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

#ifndef	DRAW_H
#define	DRAW_H

/*
 * Abstract (non-display system specific) drawing definitions.
 * 
 * This file should not contain any X window stuff as VMS
 * does not accept this in the server.  Rightly so. :-)
 */

#include "types.h"

/*
 * The server supports only 4 colors, except for spark/debris, which
 * may have 8 different colors.
 */
#define NUM_COLORS	    4

#define BLACK		    0
#define WHITE		    1
#define BLUE		    2
#define RED		    3

/*
 * The minimum and maximum playing window sizes supported by the server.
 */
#define MIN_VIEW_SIZE	    384
#define MAX_VIEW_SIZE	    1024
#define DEF_VIEW_SIZE	    768

/*
 * Spark rand limits.
 */
#define MIN_SPARK_RAND	    0		/* Not display spark */
#define MAX_SPARK_RAND	    0x80	/* Always display spark */
#define DEF_SPARK_RAND	    0x55	/* 66% */

#define DSIZE		    4	    /* Size of diamond (on radar) */

#define MSG_DURATION	    1024
#define MSG_FLASH	    892

#define SCROLL_DELAY	    10
#define SCROLL_LEN	    100

#define TITLE_DELAY	    500
#define	UPDATE_SCORE_DELAY  (FPS)

#define CLOAK_FAILURE	    130

typedef struct {			/* Defines wire-obj, i.e. ship */
    position	*pts[RES];		/* the shape rotated many ways */
    int		num_points;		/* total points in object */
    int		pt1, pt2;		/* which two points are the back */
} wireobj;

typedef unsigned long	Pixel;

extern wireobj *Default_ship(void);
extern void Free_ship_shape(wireobj *w);
extern wireobj *Parse_shape_str(char *str);
extern wireobj *Convert_shape_str(char *str);
extern int Validate_shape_str(char *str);

#endif

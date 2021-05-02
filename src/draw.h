/* $Id: draw.h,v 3.33 1995/12/04 14:47:13 bert Exp $
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

#ifndef	DRAW_H
#define	DRAW_H

#ifndef TYPES_H
/* need position */
#include "types.h"
#endif

/*
 * Abstract (non-display system specific) drawing definitions.
 *
 * This file should not contain any X window stuff as VMS
 * does not accept this in the server.  Rightly so. :-)
 */

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

#define TITLE_DELAY	    500
#define	UPDATE_SCORE_DELAY  (FPS)
#define CONTROL_DELAY	    100

/*
 * Please don't change any of these maxima.
 * It will create incompatibilities and frustration.
 */
#define MIN_SHIP_PTS	    3
#define MAX_SHIP_PTS	    24
#define MAX_GUN_PTS	    3
#define MAX_LIGHT_PTS	    3
#define MAX_RACK_PTS	    4

typedef struct {			/* Defines wire-obj, i.e. ship */
    position	*pts[MAX_SHIP_PTS];	/* the shape rotated many ways */
    int		num_points;		/* total points in object */
    position	engine[RES];		/* Engine position */
    position	m_gun[RES];		/* Main gun position */
    int		num_l_gun,
		num_r_gun,
		num_l_rgun,
		num_r_rgun;		/* number of additional cannons */
    position	*l_gun[MAX_GUN_PTS],	/* Additional cannon positions, left*/
		*r_gun[MAX_GUN_PTS],	/* Additional cannon positions, right*/
		*l_rgun[MAX_GUN_PTS],	/* Additional rear cannon positions, left*/
		*r_rgun[MAX_GUN_PTS];	/* Additional rear cannon positions, right*/
    int		num_l_light,		/* Number of lights */
		num_r_light;
    position	*l_light[MAX_LIGHT_PTS], /* Left and right light positions */
		*r_light[MAX_LIGHT_PTS];
    int		num_m_rack;		/* Number of missile racks */
    position	*m_rack[MAX_RACK_PTS];
    int		shield_radius;		/* Radius of shield used by client. */
} wireobj;

extern wireobj *Default_ship(void);
extern void Free_ship_shape(wireobj *w);
extern wireobj *Parse_shape_str(char *str);
extern wireobj *Convert_shape_str(char *str);
extern void Calculate_shield_radius(wireobj *w);
extern int Validate_shape_str(char *str);
extern void Convert_ship_2_string(wireobj *w, char *buf, char *ext,
				  unsigned shape_version);

extern float rfrac(void);
extern int mod(int x, int y);

#endif

/* $Id: draw.h,v 5.6 2003/09/16 21:02:32 bertg Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
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

#ifndef	DRAW_H
#define	DRAW_H

#ifndef TYPES_H
/* need position */
#include "types.h"
#include "const.h"
#endif

/*
 * Abstract (non-display system specific) drawing definitions.
 *
 * This file should not contain any X window stuff.
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
 * Windows deals in Pens, not Colors.  So each pen has to have all of its
 * attributes defined.
 */
#if defined(_WINDOWS) && !defined(PENS_OF_PLENTY)
#define	CLOAKCOLOROFS	15		/* colors 16 and 17 are dashed white/blue */
#define	MISSILECOLOR	18		/* wide white pen */
#define	LASERCOLOR		19		/* wide red pen */
#define	LASERTEAMCOLOR	20		/* wide blue pen */
#define	FUNKCOLORS		6		/* 6 funky colors here (15-20) */
#endif

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

#ifdef	_NAMEDSHIPS
    char*	name;
    char*	author;
#endif
} shipobj;

extern shipobj *Default_ship(void);
extern void Free_ship_shape(shipobj *w);
extern shipobj *Parse_shape_str(char *str);
extern shipobj *Convert_shape_str(char *str);
extern void Calculate_shield_radius(shipobj *w);
extern int Validate_shape_str(char *str);
extern void Convert_ship_2_string(shipobj *w, char *buf, char *ext,
				  unsigned shape_version);
void Rotate_point(position pt[RES]);

extern int mod(int x, int y);

#endif

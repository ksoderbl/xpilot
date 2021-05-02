/* $Id: click.h,v 1.7 1997/11/27 20:09:04 bert Exp $
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

#ifndef CLICK_H
#define CLICK_H

/*
 * The wall collision detection routines depend on repeatability
 * (getting the same result even after some "neutral" calculations)
 * and an exact determination whether a point is in space,
 * inside the wall (crash!) or on the edge.
 * This will be hard to achieve if only floating point would be used.
 * However, a resolution of a pixel is a bit rough and ugly.
 * Therefore a fixed point sub-pixel resolution is used called clicks.
 */
#define CLICK_SHIFT		6
#define CLICK			(1 << CLICK_SHIFT)
#define PIXEL_CLICKS		CLICK
#define BLOCK_CLICKS		(BLOCK_SZ << CLICK_SHIFT)
#define CLICK_TO_PIXEL(C)	((int)((C) >> CLICK_SHIFT))
#define CLICK_TO_BLOCK(C)	((int)((C) / (BLOCK_SZ << CLICK_SHIFT)))
#define CLICK_TO_FLOAT(C)	((DFLOAT)(C) * (1.0f / CLICK))
#define PIXEL_TO_CLICK(I)	((click_t)(I) << CLICK_SHIFT)
#define FLOAT_TO_CLICK(F)	((int)((F) * CLICK))

/*
 * Two acros for edge wrap of x and y coordinates measured in clicks.
 * Note that the correction needed should never be bigger than the size of the map.
 */
#define WRAP_XCLICK(x_)	\
	(BIT(World.rules->mode, WRAP_PLAY) \
	    ? ((x_) < 0 \
		? (x_) + mp.click_width \
		: ((x_) >= mp.click_width \
		    ? (x_) - mp.click_width \
		    : (x_))) \
	    : (x_))

#define WRAP_YCLICK(y_)	\
	(BIT(World.rules->mode, WRAP_PLAY) \
	    ? ((y_) < 0 \
		? (y_) + mp.click_height \
		: ((y_) >= mp.click_height \
		    ? (y_) - mp.click_height \
		    : (y_))) \
	    : (y_))

typedef int click_t;

typedef struct {
    click_t		x, y;
} clpos;

typedef struct {
    click_t		x, y;
} clvec;

#endif

/* $Id: walls.h,v 5.0 2001/04/07 20:01:00 dik Exp $
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

#ifndef WALLS_H
#define WALLS_H

#ifndef CLICK_H
#include "click.h"
#endif

extern char walls_version[];
extern long KILLING_SHOTS;
int Rate(int winner, int looser);

/*
 * Wall collision detection and bouncing.
 *
 * The wall collision detection routines depend on repeatability
 * (getting the same result even after some "neutral" calculations)
 * and an exact determination whether a point is in space,
 * inside the wall (crash!) or on the edge.
 * This will be hard to achieve if only floating point would be used.
 * However, a resolution of a pixel is a bit rough and ugly.
 * Therefore a fixed point sub-pixel resolution is used called clicks.
 */

#define FLOAT_TO_INT(F)		((F) < 0 ? -(int)(0.5f-(F)) : (int)((F)+0.5f))
#define DOUBLE_TO_INT(D)	((D) < 0 ? -(int)(0.5-(D)) : (int)((D)+0.5))

typedef enum {
    NotACrash = 0,
    CrashUniverse = 0x01,
    CrashWall = 0x02,
    CrashTarget = 0x04,
    CrashTreasure = 0x08,
    CrashCannon = 0x10,
    CrashUnknown = 0x20,
    CrashWormHole = 0x40,
    CrashWallSpeed = 0x80,
    CrashWallNoFuel = 0x100,
    CrashWallAngle = 0x200
} move_crash_t;

typedef enum {
    NotABounce = 0,
    BounceHorLo = 0x01,
    BounceHorHi = 0x02,
    BounceVerLo = 0x04,
    BounceVerHi = 0x08,
    BounceLeftDown = 0x10,
    BounceLeftUp = 0x20,
    BounceRightDown = 0x40,
    BounceRightUp = 0x80,
    BounceEdge = 0x0100
} move_bounce_t;

typedef struct {
    int			edge_wrap;
    int			edge_bounce;
    int			wall_bounce;
    int			cannon_crashes;
    int			target_crashes;
    int			treasure_crashes;
    int			wormhole_warps;
    int			phased;
    object		*obj;
    player		*pl;
} move_info_t;

typedef struct {
    const move_info_t	*mip;
    move_crash_t	crash;
    move_bounce_t	bounce;
    clpos		pos;
    vector		vel;
    clvec		todo;
    clvec		done;
    int			dir;
    int			cannon;
    int			wormhole;
    int			target;
    int			treasure;
} move_state_t;

struct move_parameters {
    click_t		click_width;		/* Map width in clicks */
    click_t		click_height;		/* Map width in clicks */

    int			max_shielded_angle;	/* max player bounce angle */
    int			max_unshielded_angle;	/* max player bounce angle */

    unsigned long	obj_bounce_mask;	/* which objects bounce? */
    unsigned long	obj_cannon_mask;	/* objects crash cannons? */
    unsigned long	obj_target_mask;	/* object target hit? */
    unsigned long	obj_treasure_mask;	/* objects treasure crash? */
};

#endif

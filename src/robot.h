/* $Id: robot.h,v 3.7 1994/02/07 13:20:34 bjoerns Exp $
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
/* Robot code submitted by Maurice Abraham. */

#ifndef ROBOT_H
#define ROBOT_H

/*
 * We should have these configurable for experimentation.
 */
#define	NORMAL_ROBOT_SPEED	3.0
#define	ATTACK_ROBOT_SPEED	12.0
#define MAX_ROBOT_SPEED		20.0

/*
 * Players are not robots.
 * Tanks are really objects.
 * The rest are robots in different modes.
 */
#define RM_NOT_ROBOT          	(1 << 0)
#define RM_OBJECT               (1 << 1)
#define RM_ROBOT_IDLE         	(1 << 2)
#define RM_EVADE_LEFT         	(1 << 3)
#define RM_EVADE_RIGHT          (1 << 4)
#define RM_ROBOT_CLIMB          (1 << 5)
#define RM_HARVEST            	(1 << 6)
#define RM_ATTACK             	(1 << 7)
#define RM_TAKE_OFF           	(1 << 8)
#define RM_CANNON_KILL		(1 << 9)
#define RM_REFUEL		(1 << 10)
#define RM_NAVIGATE		(1 << 11)

typedef struct {
    char	*name;
    int		attack,		/* Attack + defense ~ 100 */
    		defense;
    unsigned	used;		/* Number of times robot played in a game */
} robot_t;

#endif

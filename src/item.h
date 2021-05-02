/* $Id: item.h,v 3.15 1995/01/11 19:32:05 bert Exp $
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

#ifndef ITEM_H
#define ITEM_H

enum Item {
    ITEM_FUEL			= 0,
    ITEM_WIDEANGLE		= 1,
    ITEM_REARSHOT		= 2,
    ITEM_AFTERBURNER		= 3,
    ITEM_CLOAK			= 4,
    ITEM_SENSOR			= 5,
    ITEM_TRANSPORTER		= 6,
    ITEM_TANK			= 7,
    ITEM_MINE			= 8,
    ITEM_MISSILE		= 9,
    ITEM_ECM			= 10,
    ITEM_LASER			= 11,
    ITEM_EMERGENCY_THRUST	= 12,
    ITEM_TRACTOR_BEAM		= 13,
    ITEM_AUTOPILOT		= 14,
    ITEM_EMERGENCY_SHIELD	= 15,
    NUM_ITEMS			= 16
};

#define ITEM_BIT_FUEL			(1U << ITEM_FUEL)
#define ITEM_BIT_WIDEANGLE		(1U << ITEM_WIDEANGLE)
#define ITEM_BIT_REARSHOT		(1U << ITEM_REARSHOT)
#define ITEM_BIT_AFTERBURNER		(1U << ITEM_AFTERBURNER)
#define ITEM_BIT_CLOAK			(1U << ITEM_CLOAK)
#define ITEM_BIT_SENSOR			(1U << ITEM_SENSOR)
#define ITEM_BIT_TRANSPORTER		(1U << ITEM_TRANSPORTER)
#define ITEM_BIT_TANK			(1U << ITEM_TANK)
#define ITEM_BIT_MINE			(1U << ITEM_MINE)
#define ITEM_BIT_MISSILE		(1U << ITEM_MISSILE)
#define ITEM_BIT_ECM			(1U << ITEM_ECM)
#define ITEM_BIT_LASER			(1U << ITEM_LASER)
#define ITEM_BIT_EMERGENCY_THRUST	(1U << ITEM_EMERGENCY_THRUST)
#define ITEM_BIT_TRACTOR_BEAM		(1U << ITEM_TRACTOR_BEAM)
#define ITEM_BIT_AUTOPILOT		(1U << ITEM_AUTOPILOT)
#define ITEM_BIT_EMERGENCY_SHIELD	(1U << ITEM_EMERGENCY_SHIELD)

/* Each item is ITEM_SIZE x ITEM_SIZE */
#define ITEM_SIZE		16

#define ITEM_TRIANGLE_SIZE	(5*ITEM_SIZE/7 + 1)

#endif /* ITEM_H */

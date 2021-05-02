/* $Id: item.h,v 3.13 1994/08/15 08:16:44 bert Exp $
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

#ifndef ITEM_H
#define ITEM_H

enum Item {
    ITEM_ENERGY_PACK		= 0,
    ITEM_WIDEANGLE_SHOT		= 1,
    ITEM_BACK_SHOT		= 2,
    ITEM_AFTERBURNER		= 3,
    ITEM_CLOAKING_DEVICE	= 4,
    ITEM_SENSOR_PACK		= 5,
    ITEM_TRANSPORTER		= 6,
    ITEM_TANK			= 7,
    ITEM_MINE_PACK		= 8,
    ITEM_ROCKET_PACK		= 9,
    ITEM_ECM			= 10,
    ITEM_LASER			= 11,
    ITEM_EMERGENCY_THRUST	= 12,
    ITEM_TRACTOR_BEAM		= 13,
    ITEM_AUTOPILOT		= 14,
    ITEM_EMERGENCY_SHIELD	= 15,
    NUM_ITEMS			= 16
};

/* Each item is ITEM_SIZE x ITEM_SIZE */
#define ITEM_SIZE		16

#define ITEM_TRIANGLE_SIZE	(5*ITEM_SIZE/7 + 1)

#endif /* ITEM_H */

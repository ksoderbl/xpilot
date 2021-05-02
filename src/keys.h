/* $Id: keys.h,v 3.20 1994/05/23 19:10:32 bert Exp $
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

#ifndef KEYS_H
#define KEYS_H

#ifndef SERVER
#include <X11/X.h>

#define CONTROL_DELAY	100
#endif

/*
 * The following enum type defines the possible actions as a result of
 * a keypress or keyrelease.
 *
 * NB, if you add your own keys then please use the last ones first,
 * so that they don't clash with our future keys.
 */
typedef enum {
    KEY_DUMMY,							/* 0 */
    KEY_LOCK_NEXT,
    KEY_LOCK_PREV,
    KEY_LOCK_CLOSE,
    KEY_CHANGE_HOME,
    KEY_SHIELD,							/* 5 */
    KEY_FIRE_SHOT,
    KEY_FIRE_MISSILE,
    KEY_FIRE_TORPEDO,
    KEY_TOGGLE_NUCLEAR,		/* Was KEY_FIRE_NUKE */
    KEY_FIRE_HEAT,						/* 10 */
    KEY_DROP_MINE,
    KEY_DETACH_MINE,
    KEY_TURN_LEFT,
    KEY_TURN_RIGHT,
    KEY_SELF_DESTRUCT,
    KEY_ID_MODE,
    KEY_PAUSE,
    KEY_TANK_DETACH,
    KEY_TANK_NEXT,
    KEY_TANK_PREV,						/* 20 */
    KEY_TOGGLE_VELOCITY,
    KEY_TOGGLE_CLUSTER,		/* Was KEY_TOGGLE_COMPASS */
    KEY_SWAP_SETTINGS,
    KEY_REFUEL,
    KEY_CONNECTOR,
    KEY_INCREASE_POWER,
    KEY_DECREASE_POWER,
    KEY_INCREASE_TURNSPEED,
    KEY_DECREASE_TURNSPEED,
    KEY_THRUST,							/* 30 */
    KEY_CLOAK,
    KEY_ECM,
    KEY_DROP_BALL,
    KEY_TRANSPORTER,
    KEY_TALK,
    KEY_FIRE_LASER,
    KEY_LOCK_NEXT_CLOSE,
    KEY_TOGGLE_COMPASS,
    KEY_TOGGLE_MINI,
    KEY_TOGGLE_SPREAD,						/* 40 */
    KEY_TOGGLE_POWER,
    KEY_TOGGLE_AUTOPILOT,
    KEY_TOGGLE_LASER,
    KEY_EMERGENCY_THRUST,
    KEY_TRACTOR_BEAM,
    KEY_PRESSOR_BEAM,
    KEY_CLEAR_MODIFIERS,
    KEY_LOAD_MODIFIERS_1,
    KEY_LOAD_MODIFIERS_2,
    KEY_LOAD_MODIFIERS_3,					/* 50 */
    KEY_LOAD_MODIFIERS_4,
    KEY_TOGGLE_OWNED_ITEMS,
    KEY_TOGGLE_MESSAGES,
    KEY_REPAIR,
    KEY_TOGGLE_IMPLOSION,
    KEY_REPROGRAM,
    KEY_LOAD_LOCK_1,
    KEY_LOAD_LOCK_2,
    KEY_LOAD_LOCK_3,
    KEY_LOAD_LOCK_4,						/* 60 */
    KEY_EMERGENCY_SHIELD,
    KEY_UNUSED_62,
    KEY_UNUSED_63,
    NUM_KEYS		/* The number of different keys_t */
} keys_t;


#ifndef SERVER
typedef struct {
    KeySym	keysym;			/* Keysym-to-action array */
    keys_t	key;
} keydefs_t;
#endif

#endif

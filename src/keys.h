/* $Id: keys.h,v 3.5 1993/08/02 12:55:03 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef KEYS_H
#define KEYS_H

#include <X11/Intrinsic.h>

#define CONTROL_DELAY	100


/*
 * The following enum type defines the possible actions as a result of
 * a keypress or keyrelease.
 */
typedef enum {
    KEY_DUMMY,
    KEY_LOCK_NEXT,
    KEY_LOCK_PREV,
    KEY_LOCK_CLOSE,
    KEY_CHANGE_HOME,
    KEY_SHIELD,
    KEY_FIRE_SHOT,
    KEY_FIRE_MISSILE,
    KEY_FIRE_TORPEDO,
    KEY_FIRE_NUKE,
    KEY_FIRE_HEAT,
    KEY_DROP_MINE,
    KEY_DETACH_MINE,
    KEY_TURN_LEFT,
    KEY_TURN_RIGHT,
    KEY_SELF_DESTRUCT,
    KEY_ID_MODE,
    KEY_PAUSE,
    KEY_TANK_DETACH,
    KEY_TANK_NEXT,
    KEY_TANK_PREV,
    KEY_TOGGLE_VELOCITY,
    KEY_TOGGLE_COMPASS,
    KEY_SWAP_SETTINGS,
    KEY_REFUEL,
    KEY_CONNECTOR,
    KEY_INCREASE_POWER,
    KEY_DECREASE_POWER,
    KEY_INCREASE_TURNSPEED,
    KEY_DECREASE_TURNSPEED,
    KEY_THRUST,
    KEY_CLOAK,
    KEY_ECM,
    KEY_DROP_BALL,
    KEY_TRANSPORTER,
    KEY_TALK,
    NUM_KEYS		/* The number of different keys_t */
} keys_t;

typedef struct {
    KeySym	keysym;			/* Keysym-to-action array */
    keys_t	key;
} keydefs_t;

#endif

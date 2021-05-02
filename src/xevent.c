/* $Id: xevent.c,v 3.31 1994/05/23 19:26:01 bert Exp $
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

#include <stdlib.h>
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/keysym.h>
#ifdef	__apollo
#    include <X11/ap_keysym.h>
#endif
#include <errno.h>
#include "client.h"
#include "paint.h"
#include "xinit.h"
#include "draw.h"
#include "keys.h"
#include "packet.h"
#include "bit.h"
#include "setup.h"
#include "netclient.h"
#include "widget.h"
#include "error.h"

extern setup_t		*Setup;

static BITV_DECL(keyv, NUM_KEYS);


static keys_t Lookup_key(KeySym ks, bool reset)
{
    keys_t ret = KEY_DUMMY;
    static int i = 0;

    if (reset)
	i = 0;

    while (i < maxKeyDefs && keyDefs[i].key) {
	if (keyDefs[i].keysym == ks) {
	    ret = keyDefs[i].key;
	    i++;
	    break;
	} else {
	    i++;
	}
    }

    return (ret);
}

int Key_init(void)
{
    if (sizeof(keyv) != KEYBOARD_SIZE) {
	error ("%s, %d: keyv size %d, KEYBOARD_SIZE is %d",
	       __FILE__, __LINE__,
	       sizeof(keyv), KEYBOARD_SIZE);
	exit(1);
    }
    memset(keyv, 0, sizeof keyv);
    return 0;
}

int Key_update(void)
{
    return Send_keyboard(keyv);
}

static void Key_event(XEvent *event)
{
    KeySym  	ks;
    keys_t	key;
    bool	first_iteration = true, change = false;

    ks = XLookupKeysym(&event->xkey, 0);

    while ((key = Lookup_key(ks, first_iteration)) != KEY_DUMMY) {
	first_iteration = False;
	if (event->type == KeyPress) { /* --- KEYPRESS --- */
	    switch (key) {
	    case KEY_ID_MODE:
		showRealName = showRealName ? false : true;
		scoresChanged++;
		continue;	/* server doesn't need to know */

	    case KEY_SHIELD:
		if (toggle_shield) {
		    shields = !shields;
		    if (shields) {
			BITV_SET(keyv, key);
		    } else {
			BITV_CLR(keyv, key);
		    }
		    change = true;
		    continue;
		}
		break;

	    case KEY_REFUEL:
	    case KEY_REPAIR:
	    case KEY_TANK_NEXT:
	    case KEY_TANK_PREV:
		fuelCount = FUEL_NOTIFY;
		break;

	    case KEY_SWAP_SETTINGS:
	    case KEY_INCREASE_POWER:
	    case KEY_DECREASE_POWER:
	    case KEY_INCREASE_TURNSPEED:
	    case KEY_DECREASE_TURNSPEED:
		control_count = CONTROL_DELAY;
		break;

	    case KEY_TALK:
		Talk_map_window((talk_mapped == false) ? true : false);
		continue;	/* server doesn't need to know */

	    case KEY_TOGGLE_OWNED_ITEMS:
		TOGGLE_BIT(instruments, SHOW_ITEMS);
		continue;	/* server doesn't need to know */

	    case KEY_TOGGLE_MESSAGES:
		TOGGLE_BIT(instruments, SHOW_MESSAGES);
		continue;	/* server doesn't need to know */

	    default:
		break;
	    }
	    change = true;
	    BITV_SET(keyv, key);
	}
	else if (event->type == KeyRelease) { /* --- KEYRELEASE --- */
	    switch (key) {
	    case KEY_ID_MODE:
	    case KEY_TALK:
	    case KEY_TOGGLE_OWNED_ITEMS:
	    case KEY_TOGGLE_MESSAGES:
		continue;	/* server doesn't need to know */

	    case KEY_SHIELD:
		if (toggle_shield) {
		    continue;
		}
		break;

	    case KEY_REFUEL:
	    case KEY_REPAIR:
		fuelCount = FUEL_NOTIFY;
		break;

	    default:
		break;
	    }
	    change = true;
	    BITV_CLR(keyv, key);
	}
    }
    if (change == true) {
	Net_key_change();
    }
}

void Reset_shields(void)
{
    if (toggle_shield) {
	BITV_SET(keyv, KEY_SHIELD);
	Net_key_change();
	shields = 1;
    }
}

void Set_toggle_shield(int onoff)
{
    toggle_shield = onoff;
    if (toggle_shield) {
	shields = (BITV_ISSET(keyv, KEY_SHIELD) != 0);
    }
}

int xevent(int new_input)
{
    int			i, n, type;
    XEvent		event;
    XClientMessageEvent	*cmev;
    XConfigureEvent	*conf;
    static int		talk_key_repeat_count;
    static XEvent	talk_key_repeat_event;

#ifdef SOUND
    audioEvents();
#endif /* SOUND */

    switch (new_input) {
    case 0: type = QueuedAlready; break;
    case 1: type = QueuedAfterReading; break;
    case 2: type = QueuedAfterFlush; break;
    default:
	errno = 0;
	error("Bad input queue type (%d)", new_input);
	return -1;
    }
    n = XEventsQueued(dpy, type);
    for (i = 0; i < n; i++) {
	XNextEvent(dpy, &event);

	switch (event.type) {

	case ClientMessage:
	    cmev = (XClientMessageEvent *)&event;
	    if (cmev->message_type == ProtocolAtom
		&& cmev->data.l[0] == KillAtom) {
		errno = 0;
		error("WM Quit");
		return -1;
	    }
	    break;

	case KeyPress:
	    talk_key_repeat_count = 0;
	case KeyRelease:
	    if (event.xkey.window == top) {
		Key_event(&event);
	    }
	    else if (event.xkey.window == talk_w) {
		if (event.type == KeyPress) {
		    talk_key_repeat_count = 1;
		    talk_key_repeat_event = event;
		}
		else if (talk_key_repeat_count > 0
		    && event.xkey.keycode
			== talk_key_repeat_event.xkey.keycode) {
		    talk_key_repeat_count = 0;
		}
		Talk_event(&event);
	    }
	    /* else : here we can add widget.c key uses. */
	    break;

	case ButtonPress:
	    if (Widget_event(&event) != 0) {
		break;
	    }
	    Expose_button_window(BLACK, event.xbutton.window);
	    break;

	case MotionNotify:
	    Widget_event(&event);
	    break;

	case ButtonRelease:
	    if (Widget_event(&event) != 0) {
		extern int quitting;
		if (quitting == true) {
		    quitting = false;
		    errno = 0;
		    error("Quit");
		    return -1;
		}
		break;
	    }
	    Expose_button_window(RED, event.xbutton.window);
	    if (event.xbutton.window == about_close_b)
		About(about_close_b);
	    else if (event.xbutton.window == about_next_b)
		About(about_next_b);
	    else if (event.xbutton.window == about_prev_b)
		About(about_prev_b);
	    else if (event.xbutton.window == keys_close_b)
		Keys(keys_close_b);
	    break;

	case Expose:
	    if (event.xexpose.window == players) {
		if (event.xexpose.count == 0) {
		    players_exposed = true;
		    scoresChanged++;
		}
	    }
	    else if (event.xexpose.window == about_w) {
		if (event.xexpose.count == 0) {
		    Expose_about_window();
		}
	    }
	    else if (event.xexpose.window == keys_w) {
		if (event.xexpose.count == 0) {
		    Expose_keys_window();
		}
	    }
	    else  if (event.xexpose.window == radar) {
		if (event.xexpose.count == 0) {
		    radar_exposed = true;
		    Paint_world_radar();
		}
	    }
	    else if (event.xexpose.window == talk_w) {
		if (event.xexpose.count == 0) {
		    Talk_event(&event);
		}
	    }
	    else if (Widget_event(&event) == 0) {
		if (event.xexpose.count == 0) {
		    Expose_button_window(RED, event.xexpose.window);
		}
	    }
	    break;

	case EnterNotify:
	case LeaveNotify:
	    Widget_event(&event);
	    break;

	    /* Back in play */
	case FocusIn:
	    gotFocus = true;
	    XAutoRepeatOff(dpy);
	    break;

	    /* Probably not playing now */
	case FocusOut:
	case UnmapNotify:
	    gotFocus = false;
	    XAutoRepeatOn(dpy);
	    break;

	case MappingNotify:
	    XRefreshKeyboardMapping(&event.xmapping);
	    break;

	case ConfigureNotify:
	    conf = &event.xconfigure;
	    if (conf->window == top) {
		Resize(conf->window, conf->width, conf->height);
	    }
	    else {
		Widget_event(&event);
	    }
	    break;

	default:
	    break;
	}
    }
    if (talk_key_repeat_count > 0) {
	if (talk_key_repeat_count++ >= FPS
	    && talk_key_repeat_count % ((FPS + 3) / 4) == 0) {
	    Talk_event(&talk_key_repeat_event);
	}
    }

    if (kdpy) {
	n = XEventsQueued(kdpy, type);
	for (i = 0; i < n; i++) {
	    XNextEvent(kdpy, &event);
	    switch (event.type) {
	    case KeyPress:
	    case KeyRelease:
		Key_event(&event);
		break;

		/* Back in play */
	    case FocusIn:
		gotFocus = true;
		XAutoRepeatOff(kdpy);
		break;

		/* Probably not playing now */
	    case FocusOut:
	    case UnmapNotify:
		gotFocus = false;
		XAutoRepeatOn(kdpy);
		break;

	    case MappingNotify:
		XRefreshKeyboardMapping(&event.xmapping);
		break;
	    }
	}
    }
		
    return 0;
}


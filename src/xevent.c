/* $Id: xevent.c,v 3.51 1995/01/24 17:29:27 bert Exp $
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

#include <stdlib.h>
#include <errno.h>

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#ifdef	__apollo
#    include <X11/ap_keysym.h>
#endif

#include "version.h"
#include "config.h"
#include "const.h"
#include "paint.h"
#include "xinit.h"
#include "keys.h"
#include "packet.h"
#include "bit.h"
#include "setup.h"
#include "netclient.h"
#include "widget.h"
#include "error.h"
#include "record.h"

char xevent_version[] = VERSION;

extern setup_t		*Setup;

static BITV_DECL(keyv, NUM_KEYS);

int		initialPointerControl = false;
bool		pointerControl = false;
extern keys_t	buttonDefs[MAX_POINTER_BUTTONS];
extern Cursor	pointerControlCursor;



#if defined(JOYSTICK) && defined(__linux__)
/*
 * Joystick support for Linux 1.0 by Eckard Kopatzki (eko@isar.muc.de).
 * Needs joystick-0.7 by Art Smith, Jeff Tranter, Carlos Puchol.
 * Which in turn requires Linux 1.0 or higher.
 */
#include <linux/joystick.h>

#define JS_DEVICE	"/dev/js0"

/*
 * center position of the joystick in X and Y, resp.
 * thresholds which lead to the emulation of the key action
 */
#define JS_X0	630
#define JS_Y0	630
#define JS_DX	50
#define JS_DY	50

/*
 * Functions which are bound to the joystick actions.
 * These should be specified as defined in default.c.
 */
#define JS_LEFT		KEY_TURN_LEFT
#define JS_RIGHT	KEY_TURN_RIGHT
#define JS_UP		KEY_THRUST
#define JS_DOWN		KEY_SWAP_SETTINGS
#define JS_BUTTON0	KEY_FIRE_SHOT
#define JS_BUTTON1	KEY_SHIELD

static int Key_set(int key, int onoff)
{
    if (onoff) {
	if (!BITV_ISSET(keyv, key)) {
	    BITV_SET(keyv, key);
	    return true;
	}
    } else {
	if (BITV_ISSET(keyv, key)) {
	    BITV_CLR(keyv, key);
	    return true;
	}
    }
    return false;
}

static void Joystick_event(void)
{
    static int		js_fd = 0;
    static bool		js_avail = false;
    struct JS_DATA_TYPE	js;
    int			change = 0;

    if (!js_fd && !js_avail) {
	if ((js_fd = open(JS_DEVICE, O_RDONLY)) == -1) {
	    return;
	}
	js_avail = TRUE;
    }
    if (js_avail && read(js_fd, &js, JS_RETURN) == JS_RETURN) {
	change |= Key_set(JS_BUTTON0, (js.buttons & 1));
	change |= Key_set(JS_BUTTON1, (js.buttons & 2));
	change |= Key_set(JS_LEFT,    (js.x < JS_X0 - JS_DX));
	change |= Key_set(JS_RIGHT,   (js.x > JS_X0 + JS_DX));
	change |= Key_set(JS_UP,      (js.y < JS_Y0 - JS_DY));
	change |= Key_set(JS_DOWN,    (js.y > JS_Y0 + JS_DY));
	if (change) {
	    Net_key_change();
	}
    }
}
#endif


static keys_t Lookup_key(XEvent *event, KeySym ks, bool reset)
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
#ifdef DEVELOPMENT
    if (reset && ret == KEY_DUMMY) {
	XComposeStatus	compose;
	char		str[4];
	int		count;

	memset(str, 0, sizeof str);
	count = XLookupString(&event->xkey, str, 1, &ks, &compose);
	if (count == NoSymbol) {
	    printf("Unknown keysym: 0x%03x", ks);
	}
	else {
	    printf("No action bound to keysym 0x%03x", ks);
	    if (*str) {
		printf(", which is key \"%s\"", str);
	    }
	}
	printf("\n");
    }
#endif

    return (ret);
}

static void Pointer_control_set_state(int onoff)
{
    if (onoff) {
	pointerControl = true;
	XGrabPointer(dpy, draw, true, 0, GrabModeAsync,
		     GrabModeAsync, draw, pointerControlCursor, CurrentTime);
	XWarpPointer(dpy, None, draw,
		     0, 0, 0, 0,
		     draw_width/2, draw_height/2);
	XDefineCursor(dpy, draw, pointerControlCursor);
	XSelectInput(dpy, draw,
		     PointerMotionMask | ButtonPressMask | ButtonReleaseMask);
    } else {
	pointerControl = false;
	XUngrabPointer(dpy, CurrentTime);
	XDefineCursor(dpy, draw, None);
	XSelectInput(dpy, draw, 0);
    }
}

static void Talk_set_state(bool onoff)
{
    if (onoff) {
	/* Enable talking, disable pointer control if it is enabled. */
	if (pointerControl) {
	    initialPointerControl = true;
	    Pointer_control_set_state(false);
	}
	Talk_map_window(true);
    }
    else {
	/* Disable talking, enable pointer control if it was enabled. */
	Talk_map_window(false);
	if (initialPointerControl) {
	    initialPointerControl = false;
	    Pointer_control_set_state(true);
	}
    }
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

static bool Key_press(keys_t key)
{
    switch (key) {
    case KEY_ID_MODE:
	showRealName = showRealName ? false : true;
	scoresChanged++;
	return false;	/* server doesn't need to know */

    case KEY_SHIELD:
	if (toggle_shield) {
	    shields = !shields;
	    if (shields) {
		BITV_SET(keyv, key);
	    } else {
		BITV_CLR(keyv, key);
	    }
	    return true;
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
	Talk_set_state((talk_mapped == false) ? true : false);
	return false;	/* server doesn't need to know */

    case KEY_TOGGLE_OWNED_ITEMS:
	TOGGLE_BIT(instruments, SHOW_ITEMS);
	return false;	/* server doesn't need to know */

    case KEY_TOGGLE_MESSAGES:
	TOGGLE_BIT(instruments, SHOW_MESSAGES);
	return false;	/* server doesn't need to know */

    case KEY_POINTER_CONTROL:
	if (version < 0x3202) {
	    error("Cannot use pointer control below version 3.2.3");
	    return false;
	}
	Pointer_control_set_state(!pointerControl);
	return false;	/* server doesn't need to know */
    case KEY_TOGGLE_RECORD:
	Record_toggle();
	return false;	/* server doesn't need to know */
    default:
	break;
    }
    if (key < NUM_KEYS) {
	BITV_SET(keyv, key);
    }

    return true;
}

static bool Key_release(keys_t key)
{
    switch (key) {
    case KEY_ID_MODE:
    case KEY_TALK:
    case KEY_TOGGLE_OWNED_ITEMS:
    case KEY_TOGGLE_MESSAGES:
	return false;	/* server doesn't need to know */

    case KEY_SHIELD:
	if (toggle_shield) {
	    return false;
	}
	break;

    case KEY_REFUEL:
    case KEY_REPAIR:
	fuelCount = FUEL_NOTIFY;
	break;

    default:
	break;
    }
    if (key < NUM_KEYS) {
	BITV_CLR(keyv, key);
    }

    return true;
}

static void Key_event(XEvent *event)
{
    KeySym 		ks;
    keys_t		key;
    int			change = false;
    bool		(*key_do)(keys_t key);

    switch(event->type) {
    case KeyPress:
	key_do = Key_press;
	break;
    case KeyRelease:
	key_do = Key_release;
	break;
    default:
	return;
    }

    if ((ks = XLookupKeysym(&event->xkey, 0)) == NoSymbol) {
	return;
    }

    for (key = Lookup_key(event, ks, true);
	 key != KEY_DUMMY;
	 key = Lookup_key(event, ks, false)) {

	change |= (*key_do)(key);
    }
    if (change) {
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

static void Talk_event(XEvent *event)
{
    if (!Talk_do_event(event)) {
	Talk_set_state(false);
    }
}

int xevent(int new_input)
{
    static ipos		mouse;		/* position of mouse pointer. */
    int			movement = 0;	/* horizontal mouse movement. */
    int			i, n, type;
    ipos		delta;
    XEvent		event;
    XClientMessageEvent	*cmev;
    XConfigureEvent	*conf;
    static int		talk_key_repeat_count;
    static XEvent	talk_key_repeat_event;

#ifdef SOUND
    audioEvents();
#endif /* SOUND */

#ifdef JOYSTICK
    Joystick_event();
#endif /* JOYSTICK */

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
		printf("Quit\n");
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
		if (!talk_mapped)
		    talk_key_repeat_count = 0;
	    }
	    /* else : here we can add widget.c key uses. */
	    break;

	case ButtonPress:
	    if (event.xbutton.window == draw) {
		if (pointerControl
		    && !talk_mapped
		    && event.xbutton.button <= MAX_POINTER_BUTTONS) {
		    if (Key_press(buttonDefs[event.xbutton.button-1])) {
			Net_key_change();
		    }
		}
		break;
	    }
	    if (Widget_event(&event) != 0) {
		break;
	    }
	    Expose_button_window(BLACK, event.xbutton.window);
	    break;

	case MotionNotify:
	    if (event.xmotion.window == draw) {
		if (pointerControl) {
		    if (!talk_mapped) {
			if (!event.xmotion.send_event) {
			    movement += event.xmotion.x - mouse.x;
			}
		    }
		    mouse.x = event.xmotion.x;
		    mouse.y = event.xmotion.y;
		}
	    }
	    else {
		Widget_event(&event);
	    }
	    break;

	case ButtonRelease:
	    if (event.xbutton.window == draw) {
		if (pointerControl
		    && !talk_mapped
		    && event.xbutton.button <= MAX_POINTER_BUTTONS) {
		    if (Key_release(buttonDefs[event.xbutton.button-1])) {
			Net_key_change();
		    }
		}
		break;
	    }
	    if (Widget_event(&event) != 0) {
		extern int quitting;
		if (quitting == true) {
		    quitting = false;
		    printf("Quit\n");
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
	    else  if (event.xexpose.window == radar) {
		if (event.xexpose.count <= 1) {
		    radar_exposures = 1;
		} else {
		    radar_exposures++;
		}
	    }
	    else if (event.xexpose.window == talk_w) {
		if (event.xexpose.count == 0) {
		    Talk_event(&event);
		    if (!talk_mapped)
			talk_key_repeat_count = 0;
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
	    if (initialPointerControl && !talk_mapped) {
		initialPointerControl = false;
		Pointer_control_set_state(true);
	    }
	    gotFocus = true;
	    XAutoRepeatOff(dpy);
	    break;

	    /* Probably not playing now */
	case FocusOut:
	case UnmapNotify:
	    if (pointerControl) {
		initialPointerControl = true;
		Pointer_control_set_state(false);
	    }
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
	if (++talk_key_repeat_count >= FPS
	    && (talk_key_repeat_count - FPS) % ((FPS + 2) / 3) == 0) {
	    Talk_event(&talk_key_repeat_event);
	    if (!talk_mapped)
		talk_key_repeat_count = 0;
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

    if (pointerControl) {
	if (!talk_mapped) {
	    if (movement != 0) {
		Send_pointer_move(movement);
		delta.x = draw_width / 2 - mouse.x;
		delta.y = draw_height / 2 - mouse.y;
		if (ABS(delta.x) > 3 * draw_width / 8
		    || ABS(delta.y) > 3 * draw_height / 8) {

		    event.type = MotionNotify;
		    event.xmotion.display = dpy;
		    event.xmotion.window = draw;
		    event.xmotion.x = draw_width/2;
		    event.xmotion.y = draw_height/2;
		    XSendEvent(dpy, draw, False, PointerMotionMask, &event);
		    XWarpPointer(dpy, None, draw,
				 0, 0, 0, 0,
				 draw_width/2, draw_height/2);
		    XFlush(dpy);
		}
	    }
	}
    }

    return 0;
}


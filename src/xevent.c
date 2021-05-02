/* $Id: xevent.c,v 3.73 1998/01/28 08:50:08 bert Exp $
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

#include <stdlib.h>
#include <errno.h>

#ifndef	_WINDOWS
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#ifdef	__apollo
#    include <X11/ap_keysym.h>
#endif
#else
#include "../contrib/NT/xpilot/winX.h"
#include "../contrib/NT/xpilot/winAudio.h"
#include "../contrib/NT/xpilot/winClient.h"
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
#include "portability.h"

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
#define JS_DX	100
#define JS_DY	100

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
	js_avail = true;
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
	IFWINDOWS( Trace("Lookup_key: got key ks=%04X ret=%d\n", ks, ret); )

#ifdef DEVELOPMENT
    if (reset && ret == KEY_DUMMY) {
	static XComposeStatus	compose;
	char			str[4];
	int			count;

	memset(str, 0, sizeof str);
	count = XLookupString(&event->xkey, str, 1, &ks, &compose);
	if (count == NoSymbol) {
	    printf("Unknown keysym: 0x%03lx", ks);
	}
	else {
	    printf("No action bound to keysym 0x%03lx", ks);
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
#ifndef	_WINDOWS
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
#else
    {
	char* wintalkstr;
	wintalkstr = (char*)mfcDoTalkWindow();
	if (*wintalkstr)
	    Net_talk(wintalkstr);
    }
#endif
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
    BITV_SET(keyv, KEY_SHIELD);

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

    /* Don auto-shield hack */
    case KEY_FIRE_SHOT:
    case KEY_FIRE_LASER:
    case KEY_FIRE_MISSILE:
    case KEY_FIRE_TORPEDO:
    case KEY_FIRE_HEAT:
    case KEY_DROP_MINE:
    case KEY_DETACH_MINE:
	if (auto_shield && BITV_ISSET(keyv, KEY_SHIELD)) {
	    BITV_CLR(keyv, KEY_SHIELD);
	}
	break;

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
	else if (auto_shield) {
	    shields = 1;
#if 0
	    shields = 0;
	    BITV_CLR(keyv, key);
	    return true;
#endif
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
    case KEY_SELECT_ITEM:
    case KEY_LOSE_ITEM:
	if (version < 0x3400) {
	    static int before;
	    if (!before++) {
		errno = 0;
		error("Servers less than 3.4.0 dont know how to drop items");
	    }
	    return false;
	}
	if (lose_item_active == 1) {
	    lose_item_active = 2;
	} else {
	    lose_item_active = 1;
	}
        break;
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

    /* Don auto-shield hack */
    /* restore shields */
    case KEY_FIRE_SHOT:
    case KEY_FIRE_LASER:
    case KEY_FIRE_MISSILE:
    case KEY_FIRE_TORPEDO:
    case KEY_FIRE_HEAT:
    case KEY_DROP_MINE:
    case KEY_DETACH_MINE:
	if (auto_shield && shields && !BITV_ISSET(keyv, KEY_SHIELD)) {
	    /* Here We need to know if any other weapons are still on */
	    /*      before we turn shield back on   */
	    BITV_CLR(keyv, key);
	    if (!BITV_ISSET(keyv, KEY_FIRE_SHOT) &&
		!BITV_ISSET(keyv, KEY_FIRE_LASER) &&
		!BITV_ISSET(keyv, KEY_FIRE_MISSILE) &&
		!BITV_ISSET(keyv, KEY_FIRE_TORPEDO) &&
		!BITV_ISSET(keyv, KEY_FIRE_HEAT) &&
		!BITV_ISSET(keyv, KEY_DROP_MINE) &&
		!BITV_ISSET(keyv, KEY_DETACH_MINE)
	    ) {
		BITV_SET(keyv, KEY_SHIELD);
	    }
	}
	break;

    case KEY_SHIELD:
	if (toggle_shield) {
	    return false;
	}
	else if (auto_shield) {
	    shields = 0;
#if 0
	    shields = 1;
	    BITV_SET(keyv, key);
	    return true;
#endif
	}
	break;

    case KEY_REFUEL:
    case KEY_REPAIR:
	fuelCount = FUEL_NOTIFY;
	break;

    case KEY_SELECT_ITEM:
    case KEY_LOSE_ITEM:
	if (version < 0x3400) {
	    return false;
	}
	if (lose_item_active == 2) {
	    lose_item_active = 1;
	} else {
	    lose_item_active = -FPS;
	}
        break;

    default:
	break;
    }
    if (key < NUM_KEYS) {
	BITV_CLR(keyv, key);
    }

    return true;
}

void Key_event(XEvent *event)
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
    if (toggle_shield || auto_shield) {
	BITV_SET(keyv, KEY_SHIELD);
	shields = 1;
	if (auto_shield) {
	    if (BITV_ISSET(keyv, KEY_FIRE_SHOT) ||
		BITV_ISSET(keyv, KEY_FIRE_LASER) ||
		BITV_ISSET(keyv, KEY_FIRE_MISSILE) ||
		BITV_ISSET(keyv, KEY_FIRE_TORPEDO) ||
		BITV_ISSET(keyv, KEY_FIRE_HEAT) ||
		BITV_ISSET(keyv, KEY_DROP_MINE) ||
		BITV_ISSET(keyv, KEY_DETACH_MINE)) {
		BITV_CLR(keyv, KEY_SHIELD);
	    }
	}
	Net_key_change();
    }
}

void Set_auto_shield(int onoff)
{
    auto_shield = onoff;
}

void Set_toggle_shield(int onoff)
{
    toggle_shield = onoff;
    if (toggle_shield) {
	if (auto_shield) {
	    shields = 1;
	}
	else {
	    shields = (BITV_ISSET(keyv, KEY_SHIELD) != 0);
	}
    }
}

static void Talk_event(XEvent *event)
{
    if (!Talk_do_event(event)) {
	Talk_set_state(false);
    }
}

#ifndef	_WINDOWS
int xevent(int new_input)
#else
int xevent(XEvent event)
#endif
{
    static ipos		mouse;		/* position of mouse pointer. */
    int			movement = 0;	/* horizontal mouse movement. */
    ipos		delta;
#ifndef	_WINDOWS
    int			i, n, type;
    XEvent		event;
    XClientMessageEvent	*cmev;
    XConfigureEvent	*conf;
#endif
    static int		talk_key_repeat_count;
    static XEvent	talk_key_repeat_event;
#ifdef DEVELOPMENT
    static time_t	back_in_play_since;
#endif

#ifdef SOUND
    audioEvents();
#endif /* SOUND */

#ifdef JOYSTICK
    Joystick_event();
#endif /* JOYSTICK */

#ifndef	_WINDOWS
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
#endif

	switch (event.type) {

#ifndef	_WINDOWS
	case ClientMessage:
	    cmev = (XClientMessageEvent *)&event;
	    if (cmev->message_type == ProtocolAtom
		&& cmev->format == 32
		&& cmev->data.l[0] == KillAtom) {
		/*
		 * On HP-UX 10.20 with CDE strange things happen
		 * sometimes when closing xpilot via the window
		 * manager.  Keypresses may result in funny characters
		 * after the client exits.  The remedy to this seems
		 * to be to explicitly destroy the top window with
		 * XDestroyWindow when the window manager asks the
		 * client to quit and then wait for the resulting
		 * DestroyNotify event before closing the connection
		 * with the X server.
		 */
		XDestroyWindow(dpy, top);
		XSync(dpy, True);
		printf("Quit\n");
		return -1;
	    }
	    break;
#endif

	case KeyPress:
	    talk_key_repeat_count = 0;
	case KeyRelease:
#ifdef DEVELOPMENT
	    if (back_in_play_since) {
		time_t now = time(NULL);
		if (now - back_in_play_since > 0) {
		    back_in_play_since = 0;
		} else {
		    /* after popup ignore key events for 1 seconds. */
		    break;
		}
	    }
#endif
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

#ifndef	_WINDOWS
	    /* Back in play */
	case FocusIn:
#ifdef DEVELOPMENT
	    if (!gotFocus) {
		time(&back_in_play_since);
	    }
#endif
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
#endif

#ifndef	_WINDOWS
	case ConfigureNotify:
	    conf = &event.xconfigure;
	    if (conf->window == top) {
		Resize(conf->window, conf->width, conf->height);
	    }
	    else {
		Widget_event(&event);
	    }
	    break;
#endif

	default:
	    break;
	}
#ifndef	_WINDOWS
    }
#endif
    if (talk_key_repeat_count > 0) {
	if (++talk_key_repeat_count >= FPS
	    && (talk_key_repeat_count - FPS) % ((FPS + 2) / 3) == 0) {
	    Talk_event(&talk_key_repeat_event);
	    if (!talk_mapped)
		talk_key_repeat_count = 0;
	}
    }

#ifndef	_WINDOWS
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
#endif

    if (pointerControl) {
	if (!talk_mapped) {
	    if (movement != 0) {
		Send_pointer_move(movement);
		delta.x = draw_width / 2 - mouse.x;
		delta.y = draw_height / 2 - mouse.y;
		if (ABS(delta.x) > 3 * draw_width / 8
		    || ABS(delta.y) > 1 * draw_height / 8) {

#ifndef	_WINDOWS
		    event.type = MotionNotify;
		    event.xmotion.display = dpy;
		    event.xmotion.window = draw;
		    event.xmotion.x = draw_width/2;
		    event.xmotion.y = draw_height/2;
		    XSendEvent(dpy, draw, False, PointerMotionMask, &event);
#endif
		    XWarpPointer(dpy, None, draw,
				 0, 0, 0, 0,
				 draw_width/2, draw_height/2);
		    IFWINDOWS( Trace("Recovering mouse m=%d/%d delta=%d/%d\n",
		    			mouse.x, mouse.y, delta.x, delta.y); )
		    XFlush(dpy);
		}
	    }
	}
    }
    return 0;
}


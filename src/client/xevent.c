/* $Id: xevent.c,v 4.31 2001/03/20 20:05:54 bert Exp $
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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#ifndef _WINDOWS
# include <X11/Xlib.h>
# include <X11/Xos.h>
# include <X11/Xutil.h>
# include <X11/keysym.h>
# include <X11/Xatom.h>
# include <X11/Xmd.h>
# ifdef	__apollo
#  include <X11/ap_keysym.h>
# endif
#endif

#ifdef _WINDOWS
# include "NT/winX.h"
# include "NT/winAudio.h"
# include "NT/winClient.h"
# include "NT/winXKey.h"
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
#include "paintdata.h"
#include "talk.h"
#include "configure.h"
#include "xeventhandlers.h"

char xevent_version[] = VERSION;

extern char *talk_fast_msgs[];	/* talk macros */

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

    if (!draw) {
	return;
    }
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


keys_t Lookup_key(XEvent *event, KeySym ks, bool reset)
{
    keys_t ret = KEY_DUMMY;
    static int i = 0;

    if (reset) {
	/* binary search since keyDefs is sorted on keysym. */
	int lo = 0, hi = maxKeyDefs - 1;
	while (lo < hi) {
	    i = (lo + hi) >> 1;
	    if (ks > keyDefs[i].keysym) {
		lo = i + 1;
	    } else {
		hi = i;
	    }
	}
	if (lo == hi && ks == keyDefs[lo].keysym) {
	    while (lo > 0 && ks == keyDefs[lo - 1].keysym) {
		lo--;
	    }
	    i = lo;
	    ret = keyDefs[i].key;
	    i++;
	}
    }
    else {
	if (i < maxKeyDefs && ks == keyDefs[i].keysym) {
	    ret = keyDefs[i].key;
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

void Pointer_control_set_state(int onoff)
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
	if (!selectionAndHistory)
	    XSelectInput(dpy, draw, 0);
	else
	    XSelectInput(dpy, draw, ButtonPressMask | ButtonReleaseMask);
    }
    XFlush(dpy);
}

#ifndef _WINDOWS

static void Talk_set_state(bool onoff)
{

    if (onoff) {
	/* Enable talking, disable pointer control if it is enabled. */
	if (pointerControl) {
	    initialPointerControl = true;
	    Pointer_control_set_state(false);
	}
	if (selectionAndHistory) {
	    XSelectInput(dpy, draw, PointerMotionMask | ButtonPressMask
				    | ButtonReleaseMask);
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

#else

static void Talk_set_state(bool onoff)
{
	char* wintalkstr;

    if (pointerControl) {
	initialPointerControl = true;
	Pointer_control_set_state(false);
    }
	wintalkstr = (char*)mfcDoTalkWindow();
	if (*wintalkstr)
	    Net_talk(wintalkstr);

    if (initialPointerControl) {
	initialPointerControl = false;
	Pointer_control_set_state(true);
    }
}
#endif


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

bool Key_check_talk_macro(keys_t key)
{
    if (key >= KEY_MSG_1 && key < KEY_MSG_1 + TALK_FAST_NR_OF_MSGS) {
    /* talk macros */
	Talk_macro(talk_fast_msgs[key - KEY_MSG_1]);
    }
    return true;
}


bool Key_press_id_mode(keys_t key)
{
    showRealName = showRealName ? false : true;
    scoresChanged++;
    return false;	/* server doesn't need to know */
}

bool Key_press_autoshield_hack(keys_t key)
{
    if (auto_shield && BITV_ISSET(keyv, KEY_SHIELD)) {
	BITV_CLR(keyv, KEY_SHIELD);
    }
    return false;
}

bool Key_press_shield(keys_t key)
{
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
    return false;
}

bool Key_press_fuel(keys_t key)
{
    fuelCount = FUEL_NOTIFY;
    return false;
}

bool Key_press_swap_settings(keys_t key)
{
    DFLOAT _tmp;
#define SWAP(a, b) (_tmp = (a), (a) = (b), (b) = _tmp)
	
    SWAP(power, power_s);
    SWAP(turnspeed, turnspeed_s);
    SWAP(turnresistance, turnresistance_s);
    control_count = CONTROL_DELAY;
    Config_redraw();

    return true;
}

bool Key_press_swap_scalefactor(keys_t key)
{
#ifdef WINDOWSCALING
    DFLOAT tmp;
    tmp = scaleFactor;
    scaleFactor = scaleFactor_s;
    scaleFactor_s = tmp;

    Init_scale_array();
    Scale_dashes();
    Config_redraw();
#endif

    return false;
}

bool Key_press_increase_power(keys_t key)
{
    power = power * 1.10;
    power = MIN(power, MAX_PLAYER_POWER);
    Send_power(power);

    Config_redraw();
    control_count = CONTROL_DELAY;
    return false;	/* server doesn't see these keypresses anymore */

}

bool Key_press_decrease_power(keys_t key)
{
    power = power * 0.90;
    power = MAX(power, MIN_PLAYER_POWER);
    Send_power(power);

    Config_redraw();
    control_count = CONTROL_DELAY;
    return false;	/* server doesn't see these keypresses anymore */
}

bool Key_press_increase_turnspeed(keys_t key)
{
    turnspeed = turnspeed * 1.05;
    turnspeed = MIN(turnspeed, MAX_PLAYER_TURNSPEED);
    Send_turnspeed(turnspeed);

    Config_redraw();
    control_count = CONTROL_DELAY;
    return false;	/* server doesn't see these keypresses anymore */
}

bool Key_press_decrease_turnspeed(keys_t key)
{
    turnspeed = turnspeed * 0.95;
    turnspeed = MAX(turnspeed, MIN_PLAYER_TURNSPEED);
    Send_turnspeed(turnspeed);

    Config_redraw();
    control_count = CONTROL_DELAY;
    return false;	/* server doesn't see these keypresses anymore */
}

bool Key_press_talk(keys_t key)
{
    Talk_set_state((talk_mapped == false) ? true : false);
    return false;	/* server doesn't need to know */
}

bool Key_press_show_items(keys_t key)
{
    TOGGLE_BIT(instruments, SHOW_ITEMS);
    return false;	/* server doesn't need to know */
}

bool Key_press_show_messages(keys_t key)
{
    TOGGLE_BIT(instruments, SHOW_MESSAGES);
    return false;	/* server doesn't need to know */
}

bool Key_press_pointer_control(keys_t key)
{
    if (version < 0x3202) {
	error("Cannot use pointer control below version 3.2.3");
    } else  {
        Pointer_control_set_state(!pointerControl);
    }
    return false;	/* server doesn't need to know */
}

bool Key_press_toggle_record(keys_t key)
{
    Record_toggle();
    return false;	/* server doesn't need to know */
}

#ifndef _WINDOWS
bool Key_press_msgs_stdout(keys_t key)
{
    if (selectionAndHistory)
	Print_messages_to_stdout();
    return false;	/* server doesn't need to know */
}
#endif

bool Key_press_select_lose_item(keys_t key)
{
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
    return true;
}


bool Key_press(keys_t key)
{
    Key_check_talk_macro(key);

    switch (key) {
    case KEY_ID_MODE:
	return (Key_press_id_mode(key));

    case KEY_FIRE_SHOT:
    case KEY_FIRE_LASER:
    case KEY_FIRE_MISSILE:
    case KEY_FIRE_TORPEDO:
    case KEY_FIRE_HEAT:
    case KEY_DROP_MINE:
    case KEY_DETACH_MINE:
	Key_press_autoshield_hack(key);    
	break;

    case KEY_SHIELD:
	if (Key_press_shield(key))
	    return true; 
	break;

    case KEY_REFUEL:
    case KEY_REPAIR:
    case KEY_TANK_NEXT:
    case KEY_TANK_PREV:
	Key_press_fuel(key);
	break;

    case KEY_SWAP_SETTINGS:
	if (!Key_press_swap_settings(key))
	    return false;
	break;

    case KEY_SWAP_SCALEFACTOR:
	if (!Key_press_swap_scalefactor(key))
	    return false;
	break;

    case KEY_INCREASE_POWER:
	return Key_press_increase_power(key);

    case KEY_DECREASE_POWER:
	return Key_press_decrease_power(key);

    case KEY_INCREASE_TURNSPEED:
	return Key_press_increase_turnspeed(key);

    case KEY_DECREASE_TURNSPEED:
	return Key_press_decrease_turnspeed(key);

    case KEY_TALK:
	return Key_press_talk(key);

    case KEY_TOGGLE_OWNED_ITEMS:
	return Key_press_show_items(key);

    case KEY_TOGGLE_MESSAGES:
	return Key_press_show_messages(key);

    case KEY_POINTER_CONTROL:
	return Key_press_pointer_control(key);

    case KEY_TOGGLE_RECORD:
	return Key_press_toggle_record(key);
#ifndef _WINDOWS
    case KEY_PRINT_MSGS_STDOUT:
	return Key_press_msgs_stdout(key);
#endif
    case KEY_SELECT_ITEM:
    case KEY_LOSE_ITEM:
	if (!Key_press_select_lose_item(key)) 
	    return false;
    default:
	break;
    }

    if (key < NUM_KEYS) {
	BITV_SET(keyv, key);
    }

    return true;
}

bool Key_release(keys_t key)
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

void Talk_event(XEvent *event)
{
    if (!Talk_do_event(event)) {
	Talk_set_state(false);
    }
}


int	talk_key_repeat_count;
XEvent	talk_key_repeat_event;

void xevent_keyboard(int queued)
{
#ifndef _WINDOWS
    int			i, n;
    XEvent		event;
#endif

    if (talk_key_repeat_count > 0) {
	if (++talk_key_repeat_count >= FPS
	    && (talk_key_repeat_count - FPS) % ((FPS + 2) / 3) == 0) {
	    Talk_event(&talk_key_repeat_event);
	    if (!talk_mapped)
		talk_key_repeat_count = 0;
	}
    }

#ifndef _WINDOWS
    if (kdpy) {
	n = XEventsQueued(kdpy, queued);
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
}

ipos	delta;
ipos	mouse;		/* position of mouse pointer. */
int	movement;	/* horizontal mouse movement. */


void xevent_pointer()
{ 
#ifndef _WINDOWS
    XEvent		event;
#endif

    if (pointerControl) {
	if (!talk_mapped) {

#ifdef _WINDOWS
	    /* This is a HACK to fix mouse control under windows. */
	    {
		 POINT point;

		 GetCursorPos(&point);
		 movement = point.x - draw_width/2; 
		 XWarpPointer(dpy, None, draw,
			      0, 0, 0, 0,
			      draw_width/2, draw_height/2);
	    }
		/* fix end */
#endif 

	    if (movement != 0) {
		Send_pointer_move(movement);
		delta.x = draw_width / 2 - mouse.x;
		delta.y = draw_height / 2 - mouse.y;
		if (ABS(delta.x) > 3 * draw_width / 8
		    || ABS(delta.y) > 1 * draw_height / 8) {

#ifndef _WINDOWS
		    memset(&event, 0, sizeof(event));
		    event.type = MotionNotify;
		    event.xmotion.display = dpy;
		    event.xmotion.window = draw;
		    event.xmotion.x = draw_width/2;
		    event.xmotion.y = draw_height/2;
		    XSendEvent(dpy, draw, False, PointerMotionMask, &event);
		    XWarpPointer(dpy, None, draw,
				 0, 0, 0, 0,
				 draw_width/2, draw_height/2);
#endif
		    XFlush(dpy);
		}
	    }
	}
    }
}

#ifndef _WINDOWS
int xevent(int new_input)
#else
int xevent(XEvent event)
#endif
{
    int			queued = 0;
#ifndef _WINDOWS
    int			i, n;
    XEvent		event;
#endif

#ifdef SOUND
    audioEvents();
#endif /* SOUND */

#ifdef JOYSTICK
    Joystick_event();
#endif /* JOYSTICK */

    movement = 0;

#ifndef _WINDOWS
    switch (new_input) {
    case 0: queued = QueuedAlready; break;
    case 1: queued = QueuedAfterReading; break;
    case 2: queued = QueuedAfterFlush; break;
    default:
	errno = 0;
	error("Bad input queue type (%d)", new_input);
	return -1;
    }
    n = XEventsQueued(dpy, queued);
    for (i = 0; i < n; i++) {
	XNextEvent(dpy, &event);
#endif
	switch (event.type) {

#ifndef _WINDOWS
	    /*
	     * after requesting a selection we are notified that we
	     * can access it.
	     */
	case SelectionNotify:
	    SelectionNotify_event(&event);    
	    break;
	    /*
	     * we are requested to provide a selection.
	     */
	case SelectionRequest:
	    SelectionRequest_event(&event);
	    break;

	case SelectionClear:
	    if (selectionAndHistory)
		Clear_selection();
	    break;

	case MapNotify:
	    MapNotify_event(&event);
	    break;

	case ClientMessage:
	    if (ClientMessage_event(&event) == -1) {
		return -1;
	    }
	    break;

	    /* Back in play */
	case FocusIn:
	    FocusIn_event(&event);
	    break;

	    /* Probably not playing now */
	case FocusOut:
	case UnmapNotify:
	    UnmapNotify_event(&event);
	    break;
	    
	case MappingNotify:
	    XRefreshKeyboardMapping(&event.xmapping);
	    break;


	case ConfigureNotify:
	    ConfigureNotify_event(&event);
	    break;
#endif

	case KeyPress:
	    talk_key_repeat_count = 0;
	    /* FALLTHROUGH */
	case KeyRelease:
	    KeyChanged_event(&event);
	    break;

	case ButtonPress:
	    ButtonPress_event(&event);
	    break;

	case MotionNotify:
	    MotionNotify_event(&event);
	    break;

	case ButtonRelease:
	    if (ButtonRelease_event(&event) == -1) 
	        return -1;
	    break;

	case Expose:
	    Expose_event(&event);
	    break;

	case EnterNotify:
	case LeaveNotify:
	    Widget_event(&event);
	    break;

	default:
	    break;
	}
#ifndef _WINDOWS
    }
#endif

    xevent_keyboard(queued);	
    xevent_pointer();
    return 0;
}



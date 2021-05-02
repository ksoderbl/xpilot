/* $Id: xevent.c,v 3.12 1993/08/02 12:55:43 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/keysym.h>
#ifdef	__apollo
#    include <X11/ap_keysym.h>
#endif
#include <errno.h>
#include "client.h"
#include "xinit.h"
#include "draw.h"
#include "paint.h"
#include "keys.h"
#include "bit.h"
#include "setup.h"
#include "netclient.h"

extern int		scoresChanged;
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
    memset(keyv, 0, sizeof keyv);
    return 0;
}

int Key_update(void)
{
    return Send_keyboard(keyv);
}

void Key_event(XEvent *event)
{
    KeySym  	ks;
    keys_t	key;
    bool	first_iteration = True, change = false;

    ks = XLookupKeysym(&event->xkey, 0);

    while ((key = Lookup_key(ks, first_iteration)) != KEY_DUMMY) {
	first_iteration = False;
	if (event->type == KeyPress) { /* --- KEYPRESS --- */
	    switch (key) {
	    case KEY_ID_MODE:
		TOGGLE_BIT(instruments, SHOW_SHIP_NAME);
		continue;	/* server doesn't need to know */

	    case KEY_REFUEL:
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
		continue;	/* server doesn't need to know */

	    case KEY_REFUEL:
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

int xevent(int new_input)
{
    int			i, n, type;
    XEvent		event;
    XClientMessageEvent	*cmev;

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
	case KeyRelease:
	    if (event.xkey.window == talk_w) {
		Talk_event(&event);
	    } else {
		Key_event(&event);
	    }
	    break;

	case ButtonPress:
	    Expose_button_window(BLACK, event.xbutton.window);
	    break;

	case ButtonRelease:
	    Expose_button_window(RED, event.xbutton.window);
	    if (event.xbutton.window == quit_b) {
		errno = 0;
		error("Quit");
		return -1;
	    }
	    else if (event.xbutton.window == about_b)
		About(about_b);
	    else if (event.xbutton.window == help_b)
		Help(help_b);
	    else if (event.xbutton.window == about_close_b)
		About(about_close_b);
	    else if (event.xbutton.window == about_next_b)
		About(about_next_b);
	    else if (event.xbutton.window == about_prev_b)
		About(about_prev_b);
	    else if (event.xbutton.window == help_close_b)
		Help(help_close_b);
	    break;

	case Expose:
	    if (event.xexpose.count > 0)	/* We don't want any */
		break;				/* subarea exposures */

	    if (event.xexpose.window == players) {
		players_exposed = true;
		scoresChanged++;
	    }
	    else if (event.xexpose.window == about_w)
		Expose_about_window();
	    else if (event.xexpose.window == help_w)
		Expose_help_window();
	    else  if (event.xexpose.window == radar) {
		radar_exposed = true;
		Paint_world_radar();
	    }
	    else if (event.xexpose.window == talk_w)
		Talk_event(&event);
	    else
		Expose_button_window(RED, event.xexpose.window);
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

	default:
	    break;
	}
    }

    return 0;
}


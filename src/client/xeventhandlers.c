/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
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

#include "xpconfig.h"
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


extern int	talk_key_repeat_count;  /* xevent.c */
extern XEvent	talk_key_repeat_event;  /* xevent.c */

extern void Talk_event(XEvent *event);  /* xevent.c */
extern void Key_event(XEvent *event);   /* xevent.c */
extern bool Key_press(keys_t key);	/* xevent.c */
extern bool Key_release(keys_t key);	/* xevent.c */
extern void Pointer_control_set_state(bool valueOn); /* xevent.c */


extern int	initialPointerControl;
extern bool	pointerControl;
extern keys_t	buttonDefs[MAX_POINTER_BUTTONS][MAX_BUTTON_DEFS+1];
extern ipos	mouse;		/* position of mouse pointer. */
extern int	movement;	/* horizontal mouse movement. */

#ifndef _WINDOWS
/* avoid trouble with Atoms and 64 bit archs */
typedef CARD32  Atom32;
#endif

/* selection in talk or draw window */
extern selection_t selection;

/* store message in history, when it is sent? */
extern bool save_talk_str;

#ifdef DEVELOPMENT
time_t	back_in_play_since;
#endif

#ifndef _WINDOWS
/*
 * code for the following three functions and the selectionEvents
 * happily and with benediction taken from the terminal emulator
 * `rxvt-2.6Pre2' (GNU) maintained by Geoff Wing <gcw@pobox.com>.
 * (modified)
*/
static void Selection_paste(Window win, unsigned prop, int Delete)
{
    long            nread;
    unsigned long   bytes_after, nitems;
    unsigned char  *data;
    Atom            actual_type;
    int             actual_fmt;
 
    if (prop == None)
        return;

    for (nread = 0, bytes_after = 1; bytes_after > 0; nread += nitems) {
        if ((XGetWindowProperty(dpy, win, prop, (nread / 4), MAX_CHARS / 4,
                                Delete, AnyPropertyType, &actual_type,
                                &actual_fmt, &nitems, &bytes_after,
                                &data) != Success)) {
            XFree(data);
            return;
        }
        if (Talk_paste((char*)data, nitems, False) == 0) {
	   /* talk window doesn't accept text anymore */
	    return;
	} else {
	    save_talk_str = true;
	}
        XFree(data);
    }
}

/*
 * we want a selection
 */
static void Selection_request(void)
{
    Atom prop;
 
    if (selection.txt != NULL) {
	/* internal selection */
	if (Talk_paste(selection.txt, selection.len, False) > 0)
	    save_talk_str = true;
    } 
    else if (XGetSelectionOwner(dpy, XA_PRIMARY) == None) {
	Selection_paste(DefaultRootWindow(dpy), XA_CUT_BUFFER0, False);
    } 
    else {
	prop = XInternAtom(dpy, "VT_SELECTION", False);
	XConvertSelection(dpy, XA_PRIMARY, XA_STRING, prop, talk_w,
			    CurrentTime);
	/* the selectionNotify event `will do the rest' */
    }
}

/*
 * our selection is requested
 */
static void Selection_send(const XSelectionRequestEvent *rq)
{
    XEvent          ev;
    Atom32          target_list[2];
    static Atom     xa_targets = None;
 
    if (xa_targets == None)
        xa_targets = XInternAtom(dpy, "TARGETS", False);
 
    ev.xselection.type = SelectionNotify;
    ev.xselection.property = None;
    ev.xselection.display = rq->display;
    ev.xselection.requestor = rq->requestor;
    ev.xselection.selection = rq->selection;
    ev.xselection.target = rq->target;
    ev.xselection.time = rq->time;
 
    if (rq->target == xa_targets) {
        target_list[0] = (Atom32) xa_targets;
        target_list[1] = (Atom32) XA_STRING;
        XChangeProperty(dpy, rq->requestor, rq->property, rq->target,
                        (8 * sizeof(target_list[0])), PropModeReplace,
                        (unsigned char *)target_list,
                        (sizeof(target_list) / sizeof(target_list[0])));
        ev.xselection.property = rq->property;
    } 
    else if (rq->target == XA_STRING) {
	XChangeProperty(dpy, rq->requestor, rq->property,
			rq->target, 8, PropModeReplace,
			(unsigned char *) selection.txt, selection.len);
	ev.xselection.property = rq->property;
    }
    XSendEvent(dpy, rq->requestor, False, 0, &ev);
}

void SelectionNotify_event(XEvent *event)
{
    if (selectionAndHistory)

    Selection_paste(event->xselection.requestor,
		    event->xselection.property, True);
}

void SelectionRequest_event(XEvent *event)
{
    Selection_send(&(event->xselectionrequest));
}

void MapNotify_event(XEvent *event)
{
    if (ignoreWindowManager == 1) {
        XSetInputFocus(dpy, top, RevertToParent, CurrentTime); 
        ignoreWindowManager = 2;
    }
}


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
int ClientMessage_event(XEvent *event)
{
    XClientMessageEvent	*cmev;
    cmev = (XClientMessageEvent *)event;
    if (cmev->message_type == ProtocolAtom
	&& cmev->format == 32
	&& cmev->data.l[0] == KillAtom) {
        XDestroyWindow(dpy, top);
	XSync(dpy, True);
	printf("Quit\n");
	return -1;
    }
    return 0;
}

void FocusIn_event(XEvent *event)
{
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
}

void UnmapNotify_event(XEvent *event)
{
    if (pointerControl) {
        initialPointerControl = true;
        Pointer_control_set_state(false);
    }
    gotFocus = false;
    XAutoRepeatOn(dpy);
}

void ConfigureNotify_event(XEvent *event)
{
    XConfigureEvent	*conf;

    conf = &(event->xconfigure);
    if (conf->window == top) {
	Resize(conf->window, conf->width, conf->height);
    } 
    else {
        Widget_event(event);
    }
}
#endif

void KeyChanged_event(XEvent *event)
{
#ifdef DEVELOPMENT
    if (back_in_play_since) {
        time_t now = time(NULL);
	if (now - back_in_play_since > 0) {
	    back_in_play_since = 0;
	} else {
	    /* after popup ignore key events for 1 seconds. */
	    return;
	}
    }
#endif
    if (event->xkey.window == top) {
        Key_event(event);
    } 
    else if (event->xkey.window == talk_w) {
        if (event->type == KeyPress) {
	    talk_key_repeat_count = 1;
	    talk_key_repeat_event = *event;
	} 
	else if (talk_key_repeat_count > 0
		   && event->xkey.keycode
		   == talk_key_repeat_event.xkey.keycode) {
	    talk_key_repeat_count = 0;
	}
	Talk_event(event);
	if (!talk_mapped)
	    talk_key_repeat_count = 0;
    }
	/* else : here we can add widget.c key uses. */
}

void ButtonPress_event(XEvent *event)
{
    if (event->xbutton.window == draw
	|| event->xbutton.window == talk_w) {
        if (pointerControl
	    && !talk_mapped
	    && event->xbutton.button <= MAX_POINTER_BUTTONS) {
	    int i;
	    for (i = 0; i < NUM_BUTTON_DEFS(event->xbutton.button-1); ++i) {
	    	if (Key_press(buttonDefs[event->xbutton.button-1][i])) {
		    Net_key_change();
		}
	    }
	} 
#ifndef _WINDOWS
	else if (selectionAndHistory) {
	    switch (event->xbutton.button) {
	    case Button1:
	        if (!talk_mapped) {
		  /* start cutting from the talk messages */
		  Talk_cut_from_messages(&(event->xbutton));
		} 
		else {
		    /* start cutting from ... */
		    if (event->xbutton.window == draw) {
		        /* ...the talk messages */
		        Talk_cut_from_messages(&(event->xbutton));
		    } 
		    else {
		        /* ...the talk window */
		        Talk_window_cut(&(event->xbutton));
		    } 
		}
		break;

	    case Button2:
	        if (talk_mapped) {
		    if (event->xbutton.window == talk_w) {
		        Talk_place_cursor(&(event->xbutton), false);
		    }
		    Selection_request();
		}
		break;

	    default:
	        break;
	    } /* switch */
	      /* end of selectionAndHistory */
	}
#endif /* not _WINDOWS */
	return;
    }
    if (Widget_event(event) != 0) {
        return;
    }
    Expose_button_window(BLACK, event->xbutton.window);
}

void MotionNotify_event(XEvent *event)
{
    if (event->xmotion.window == draw) {
        if (pointerControl) {
	    if (!talk_mapped) {
	        if (!event->xmotion.send_event) {
		    movement += event->xmotion.x - mouse.x;
		}
	    }
	    mouse.x = event->xmotion.x;
	    mouse.y = event->xmotion.y;
	}
    } 
    else {
        Widget_event(event);
    }
}

int ButtonRelease_event(XEvent *event)
{
    if (event->xbutton.window == draw
	|| event->xbutton.window == talk_w) {
      
        if (pointerControl
	    && !talk_mapped
	    && event->xbutton.button <= MAX_POINTER_BUTTONS) {
	    int i;
	    for (i = 0; i < NUM_BUTTON_DEFS(event->xbutton.button-1); ++i) {
	    	if (Key_release(buttonDefs[event->xbutton.button-1][i])) {
		    Net_key_change();
		}
	    }
	} 
#ifndef _WINDOWS
	else if (!selectionAndHistory) {
	    return 0;
	}
	if (!talk_mapped && event->xbutton.button == 1) {
		/*
		 * finish a cut from the talk messages
		 */
	    Talk_cut_from_messages(&(event->xbutton));
	} 
	else if (talk_mapped && event->xbutton.button == 1){
		/*
		 * finish a cut from ...
		 */
	    if (event->xbutton.window == draw
		&& selection.draw.state == SEL_PENDING) {
	        Talk_cut_from_messages(&(event->xbutton));
	    } 
	    else if (selection.talk.state == SEL_PENDING) {
	        Talk_window_cut(&(event->xbutton));
	    }
	}
#endif /* not _WINDOWS */
	return 0;
    }
    if (Widget_event(event) != 0) {
	if (quitting == true) {
	    quitting = false;
	    printf("Quit\n");
	    return -1;
	}
	return 0;
    }
    Expose_button_window(RED, event->xbutton.window);
    if (event->xbutton.window == about_close_b)
	About(about_close_b);
    else if (event->xbutton.window == about_next_b)
	About(about_next_b);
    else if (event->xbutton.window == about_prev_b)
	About(about_prev_b);
    return 0;
}

void Expose_event(XEvent *event)
{
    if (event->xexpose.window == players) {
        if (event->xexpose.count == 0) {
	    players_exposed = true;
	    scoresChanged++;
	}
    } 
    else if (event->xexpose.window == about_w) {
	  if (event->xexpose.count == 0) {
	      Expose_about_window();
	  }
    } 
    else if (event->xexpose.window == radar) {
	  if (event->xexpose.count <= 1) {
	      radar_exposures = 1;
	  } 
	  else {
	      radar_exposures++;
	  }
    }
    else if (event->xexpose.window == talk_w) {
	if (event->xexpose.count == 0) {
	    Talk_event(event);
	    if (!talk_mapped)
	      talk_key_repeat_count = 0;
	}
    }
    else if (Widget_event(event) == 0) {
	if (event->xexpose.count == 0) {
	    Expose_button_window(RED, event->xexpose.window);
	}
    }
}

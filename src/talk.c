/* $Id: talk.c,v 3.1 1996/10/13 19:30:51 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gÿsbers         <bert@xpilot.org>
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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#ifndef	__apollo
#    include <string.h>
#endif
#include <errno.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "version.h"
#include "config.h"
#include "const.h"
#include "paint.h"
#include "xinit.h"
#include "error.h"
#include "netclient.h"
#include "protoclient.h"

char talk_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: talk.c,v 3.1 1996/10/13 19:30:51 bert Exp $";
#endif

/* Information window dimensions */
#define TALK_TEXT_HEIGHT	(textFont->ascent + textFont->descent)
#define TALK_OUTSIDE_BORDER	2
#define TALK_INSIDE_BORDER	3
#define TALK_WINDOW_HEIGHT	(TALK_TEXT_HEIGHT + 2 * TALK_INSIDE_BORDER)
#define TALK_WINDOW_X		(50 - TALK_OUTSIDE_BORDER)
#define TALK_WINDOW_Y		(draw_height*3/4 - TALK_WINDOW_HEIGHT/2)
#define TALK_WINDOW_WIDTH	(draw_width \
				    - 2*(TALK_WINDOW_X + TALK_OUTSIDE_BORDER))

#define CTRL(c)			((c) & 0x1F)

/*
 * Globals.
 */
bool			talk_mapped;

static bool		talk_created;
static char		talk_str[MAX_CHARS];
static struct {
    bool		visible;
    short		offset;
    short		point;
} talk_cursor;


static void Talk_create_window(void)
{
    /*
     * Create talk window.
     */
    talk_w
	= XCreateSimpleWindow(dpy, draw,
			      TALK_WINDOW_X, TALK_WINDOW_Y,
			      TALK_WINDOW_WIDTH, TALK_WINDOW_HEIGHT,
			      TALK_OUTSIDE_BORDER, colors[WHITE].pixel,
			      colors[BLACK].pixel);

    XSelectInput(dpy, talk_w, KeyPressMask | KeyReleaseMask | ExposureMask);
}


void Talk_cursor(bool visible)
{
    if (talk_mapped == false || visible == talk_cursor.visible) {
	return;
    }
    if (visible == false) {
	XSetForeground(dpy, talkGC, colors[BLACK].pixel);
	XDrawString(dpy, talk_w, talkGC,
		    talk_cursor.offset + TALK_INSIDE_BORDER,
		    talkFont->ascent + TALK_INSIDE_BORDER,
		    "_", 1);
	XSetForeground(dpy, talkGC, colors[WHITE].pixel);
	if (talk_cursor.point < strlen(talk_str)) {
	    XDrawString(dpy, talk_w, talkGC,
			talk_cursor.offset + TALK_INSIDE_BORDER,
			talkFont->ascent + TALK_INSIDE_BORDER,
			&talk_str[talk_cursor.point], 1);
	}
	talk_cursor.visible = false;
    } else {
	talk_cursor.offset = XTextWidth(talkFont, talk_str, talk_cursor.point);
	XDrawString(dpy, talk_w, talkGC,
		    talk_cursor.offset + TALK_INSIDE_BORDER,
		    talkFont->ascent + TALK_INSIDE_BORDER,
		    "_", 1);
	talk_cursor.visible = true;
    }
}


void Talk_map_window(bool map)
{
    static Window	root;
    static int		root_x, root_y;

    if (map == true) {
	Window child;
	int win_x, win_y;
	unsigned int keys_buttons;

	if (talk_created == false) {
	    Talk_create_window();
	    talk_created = true;
	}
	XMapWindow(dpy, talk_w);
	talk_mapped = true;

	XQueryPointer(dpy, DefaultRootWindow(dpy),
		      &root, &child, &root_x, &root_y, &win_x, &win_y,
		      &keys_buttons);
	XWarpPointer(dpy, None, talk_w,
		     0, 0, 0, 0,
		     TALK_WINDOW_WIDTH - 2,
		     TALK_WINDOW_HEIGHT - 2);
	XFlush(dpy);	/* warp pointer ASAP. */
    }
    else if (talk_created == true) {
	XUnmapWindow(dpy, talk_w);
	XWarpPointer(dpy, None, root, 0, 0, 0, 0, root_x, root_y);
	talk_mapped = false;
    }
    talk_cursor.visible = false;
}


int Talk_do_event(XEvent *event)
{
    char		ch;
    bool		cursor_visible = talk_cursor.visible;
    int			count, oldlen, newlen, onewidth, oldwidth;
    KeySym		keysym;
    char		new_str[MAX_CHARS];
    bool		result = true;

    /* according to Ralf Berger <gotan@physik.rwth-aachen.de>
     * compose should be static:
     * the value of 'compose' has to be preserved across calls to
     * 'XLookupString' (the man page also mentioned that a portable program 
     * should pass NULL but i don't know if that's specific to dec-alpha ?).
     * To fix this declare 'compose' static (can't hurt anyway).
     */
    static XComposeStatus	compose;

    switch (event->type) {

    case Expose:
	XClearWindow(dpy, talk_w);
	XDrawString(dpy, talk_w, talkGC,
		    TALK_INSIDE_BORDER, talkFont->ascent + TALK_INSIDE_BORDER,
		    talk_str, strlen(talk_str));
	if (cursor_visible == true) {
	    talk_cursor.visible = false;
	    Talk_cursor(cursor_visible);
	}
	break;

    case KeyRelease:
	/*
	 * Nothing to do.
	 * We may want to make some kind of key repeat ourselves.
	 * Some day...
	 */
	break;

    case KeyPress:
	onewidth = XTextWidth(talkFont, talk_str, 1);
	count = XLookupString(&event->xkey, &ch, 1, &keysym, &compose);
	if (count == NoSymbol) {
	    break;
	}

	switch (ch) {
	case '\0':
	    /*
	     * ?  Ignore.
	     */
	    break;

	case '\r':
	case '\n':
	    /*
	     * Return.  Send the talk message to the server if there is text.
	     */
	    if (talk_str[0] != '\0') {
		Net_talk(talk_str);
		talk_cursor.point = 0;
		talk_str[0] = '\0';
	    }
	    result = false;
	    break;

	case '\033':
	    /*
	     * Escape.  Cancel talking.
	     */
	    talk_str[0] = '\0';
	    talk_cursor.point = 0;
	    result = false;
	    break;

	case CTRL('A'):
	    /*
	     * Put cursor at start of line.
	     */
	    Talk_cursor(false);
	    talk_cursor.point = 0;
	    Talk_cursor(true);
	    break;

	case CTRL('E'):
	    /*
	     * Put cursor at end of line.
	     */
	    Talk_cursor(false);
	    talk_cursor.point = strlen(talk_str);
	    Talk_cursor(true);
	    break;

	case CTRL('B'):
	    /*
	     * Put cursor one character back.
	     */
	    if (talk_cursor.point > 0) {
		Talk_cursor(false);
		talk_cursor.point--;
		Talk_cursor(true);
	    }
	    break;

	case CTRL('F'):
	    /*
	     * Put cursor one character forward.
	     */
	    if (talk_cursor.point < strlen(talk_str)) {
		Talk_cursor(false);
		talk_cursor.point++;
		Talk_cursor(true);
	    }
	    break;

	case '\b':
	case '\177':
	case CTRL('D'):
	case CTRL('W'):
	case CTRL('U'):
	case CTRL('K'):
	    /*
	     * Erase characters.
	     */
	    Talk_cursor(false);

	    strcpy(new_str, talk_str);
	    oldlen = strlen(talk_str);
	    newlen = oldlen;

	    /*
	     * Calculate text changes first without drawing.
	     */
	    if (ch == CTRL('W')) {
		/*
		 * Word erase.
		 * Erase whitespace first and then one word.
		 */
		while (newlen > 0 && talk_str[newlen - 1] == ' ') {
		    newlen--;
		}
		while (newlen > 0 && talk_str[newlen - 1] != ' ') {
		    newlen--;
		}
	    }
	    else if (ch == CTRL('U')) {
		/*
		 * Erase everything.
		 */
		newlen = 0;
	    }
	    else if (ch == CTRL('K')) {
		/*
		 * Clear rest of the line.
		 */
		newlen = talk_cursor.point;
	    }
	    else if (oldlen > 0) {
		/*
		 * Erase one character.
		 */
		newlen--;
		if (ch != CTRL('D') || talk_cursor.point >= newlen) {
		    if (talk_cursor.point > 0) {
			talk_cursor.point--;
		    }
		}
		strcpy(&new_str[talk_cursor.point],
		       &talk_str[talk_cursor.point + 1]);
	    }

	    new_str[newlen] = '\0';
	    if (talk_cursor.point > newlen) {
		talk_cursor.point = newlen;
	    }

	    /*
	     * Now reflect the text changes onto the screen.
	     */
	    if (newlen < oldlen) {
		XSetForeground(dpy, talkGC, colors[BLACK].pixel);
		XDrawString(dpy, talk_w, talkGC,
			    talk_cursor.point * onewidth + TALK_INSIDE_BORDER,
			    talkFont->ascent + TALK_INSIDE_BORDER,
			    &talk_str[talk_cursor.point],
			    oldlen - talk_cursor.point);
		XSetForeground(dpy, talkGC, colors[WHITE].pixel);
	    }
	    if (talk_cursor.point < newlen) {
		XDrawString(dpy, talk_w, talkGC,
			    talk_cursor.point * onewidth + TALK_INSIDE_BORDER,
			    talkFont->ascent + TALK_INSIDE_BORDER,
			    &new_str[talk_cursor.point],
			    newlen - talk_cursor.point);
	    }
	    Talk_cursor(cursor_visible);

	    strcpy(talk_str, new_str);

	    break;

	default:
	    if ((ch & 0x7F) == ch && !isprint(ch)) {
		/*
		 * Unknown special character.
		 */
		break;
	    }

	    oldlen = strlen(talk_str);
	    oldwidth = XTextWidth(talkFont, talk_str, oldlen);
	    if (oldlen >= MAX_CHARS - 2
		|| oldwidth >= TALK_WINDOW_WIDTH - 2*TALK_INSIDE_BORDER - 5) {
		/*
		 * No more space for new text.
		 */
		XBell(dpy, 100);
		break;
	    }

	    /*
	     * Enter new text.
	     */
	    strcpy(new_str, talk_str);
	    strcpy(&new_str[talk_cursor.point + 1],
		   &talk_str[talk_cursor.point]);
	    new_str[talk_cursor.point] = ch;
	    newlen = oldlen + 1;

	    /*
	     * Reflect text changes onto screen.
	     */
	    Talk_cursor(false);
	    if (talk_cursor.point < oldlen) {
		/*
		 * Erase old text from cursor to end of line.
		 */
		XSetForeground(dpy, talkGC, colors[BLACK].pixel);
		XDrawString(dpy, talk_w, talkGC,
			    talk_cursor.point * onewidth + TALK_INSIDE_BORDER,
			    talkFont->ascent + TALK_INSIDE_BORDER,
			    &talk_str[talk_cursor.point],
			    oldlen - talk_cursor.point);
		XSetForeground(dpy, talkGC, colors[WHITE].pixel);
	    }
	    XDrawString(dpy, talk_w, talkGC,
			talk_cursor.point * onewidth + TALK_INSIDE_BORDER,
			talkFont->ascent + TALK_INSIDE_BORDER,
			&new_str[talk_cursor.point],
			newlen - talk_cursor.point);
	    talk_cursor.point++;
	    Talk_cursor(cursor_visible);

	    strcpy(talk_str, new_str);

	    break;
	}
	XFlush(dpy);	/* needed in case we don't get frames to draw soon. */

	/*
	 * End of KeyPress.
	 */
	break;

    default:
	break;
    }

    return result;	/* keep on talking if true, no more talking if false */
}

void Talk_resize(void)
{
    if (talk_created) {
	XMoveResizeWindow(dpy, talk_w,
			  TALK_WINDOW_X, TALK_WINDOW_Y,
			  TALK_WINDOW_WIDTH, TALK_WINDOW_HEIGHT);
    }
}


/* $Id: xeventhandlers.h,v 5.0 2001/04/07 20:00:58 dik Exp $
 *
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

#ifndef XEVENTHANDLERS_H
#define XEVENTHANDLERS_H

void SelectionNotify_event(XEvent *event);
void SelectionRequest_event(XEvent *event);
void MapNotify_event(XEvent *event);
int ClientMessage_event(XEvent *event);
void FocusIn_event(XEvent *event);
void UnmapNotify_event(XEvent *event);
void ConfigureNotify_event(XEvent *event);
void Expose_event(XEvent *event);
void KeyChanged_event(XEvent *event);
void ButtonPress_event(XEvent *xevent);
void MotionNotify_event(XEvent *event);
int ButtonRelease_event(XEvent *event);
#endif

/* $Id: xinit.h,v 3.9 1993/08/02 12:55:47 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	XINIT_H
#define	XINIT_H

#include <X11/Intrinsic.h>
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>

#include "client.h"

extern Atom		ProtocolAtom, KillAtom;

/*
 * Prototypes for xinit.c
 */
extern int Parse_colors(Colormap cmap);
extern int Init_window(void);
extern int Alloc_msgs(int number);
extern void Free_msgs(void);
extern void Expose_info_window(void);
extern void Expose_help_window(void);
extern void Expose_button_window(int color, Window w);
extern void Info(Window w);
extern void Help(Window w);
extern void Talk_cursor(bool visible);
extern void Talk_map_window(bool map);
extern void Talk_event(XEvent *event);
extern void Quit(void);
extern int FatalError(Display *dpy);
extern void Draw_score_table(void);

extern int DrawShadowText(Display*, Window w, GC gc,
			  int x_border, int start_y,
			  char *str, Pixel fg, Pixel bg);
extern void ShadowDrawString(Display*, Window w, GC gc, int x,
			   int start_y, char *str, Pixel fg, Pixel bg);

#endif


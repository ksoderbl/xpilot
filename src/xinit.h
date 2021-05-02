/* $Id: xinit.h,v 3.26 1994/04/12 13:43:18 bjoerns Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-94 by
 *
 *      Bj�rn Stabell        (bjoerns@staff.cs.uit.no)
 *      Ken Ronny Schouten   (kenrsc@stud.cs.uit.no)
 *      Bert G�sbers         (bert@mc.bio.uva.nl)
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

#ifndef	XINIT_H
#define	XINIT_H

#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>

#include "client.h"

#define MAX_VISUAL_NAME	12

#define MIN_TOP_WIDTH	(640 + 2)
#define MAX_TOP_WIDTH	(1280 + 2)
#define DEF_TOP_WIDTH	(1024 + 2)
#define MIN_TOP_HEIGHT	480
#define MAX_TOP_HEIGHT	1024
#define DEF_TOP_HEIGHT	768

#define HavePlanes(d) (DisplayPlanes(d, DefaultScreen(d)) > 2)
#define HaveColor(d)							\
    (DefaultVisual(d, DefaultScreen(d))->class == PseudoColor		\
     || DefaultVisual(d, DefaultScreen(d))->class == GrayScale)

extern Atom		ProtocolAtom, KillAtom;
extern int		buttonColor, windowColor, borderColor;
extern int		ButtonHeight;
extern char		visualName[MAX_VISUAL_NAME];
extern Visual		*visual;
extern int		dispDepth;
extern bool		mono;
extern bool		colorSwitch;
extern char		color_names[MAX_COLORS][MAX_COLOR_LEN];
extern int		top_width, top_height;
extern int		draw_width, draw_height;
extern char		*geometry;
extern bool		autoServerMotdPopup;

/*
 * Prototypes for xinit.c
 */
extern int Parse_colors(Colormap cmap);
extern void List_visuals(void);
extern int Init_window(void);
extern int Alloc_msgs(int number);
extern void Free_msgs(void);
extern void Expose_info_window(void);
extern void Expose_keys_window(void);
extern void Expose_button_window(int color, Window w);
extern void Info(Window w);
extern void Keys(Window w);
extern void Talk_cursor(bool visible);
extern void Talk_map_window(bool map);
extern void Talk_event(XEvent *event);
extern void Quit(void);
extern int FatalError(Display *dpy);
extern void Draw_score_table(void);
extern void Resize(Window w, int width, int height);

extern int DrawShadowText(Display*, Window w, GC gc,
			  int x_border, int start_y,
			  char *str, Pixel fg, Pixel bg);
extern void ShadowDrawString(Display*, Window w, GC gc, int x,
			   int start_y, char *str, Pixel fg, Pixel bg);
void About(Window w);
void Expose_about_window(void);
int Handle_motd(long off, char *buf, int len, long filesize);
int Startup_server_motd(void);

#endif


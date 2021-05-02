/* $Id: widget.c,v 3.20 1995/01/24 17:27:40 bert Exp $
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

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include "version.h"
#include "config.h"
#include "const.h"
#include "paint.h"
#include "xinit.h"
#include "bit.h"
#include "error.h"
#include "widget.h"

char widget_version[] = VERSION;

static void Widget_resize_viewer(XEvent *event, int ind);

static widget_t		*widgets;
static int		num_widgets, max_widgets;

static void Widget_window_gravity(Window w, int gravity)
{
    unsigned long		mask;
    XSetWindowAttributes	attr;

    mask = CWWinGravity;
    attr.win_gravity = gravity;
    XChangeWindowAttributes(dpy, w, mask, &attr);
}

static void Widget_bit_gravity(Window w, int gravity)
{
    unsigned long		mask;
    XSetWindowAttributes	attr;

    mask = CWBitGravity;
    attr.bit_gravity = gravity;
    XChangeWindowAttributes(dpy, w, mask, &attr);
}

static int Widget_validate(int widget_desc)
{
    if (widget_desc <= NO_WIDGET || widget_desc >= num_widgets) {
	return NO_WIDGET;
    }
    if (widgets[widget_desc].type == WIDGET_DUMMY) {
	return NO_WIDGET;
    }
    return widget_desc;
}

static widget_t *Widget_pointer(int widget_desc)
{
    if (Widget_validate(widget_desc) == NO_WIDGET) {
	return NULL;
    }
    return &widgets[widget_desc];
}

Window Widget_window(int widget_desc)
{
    widget_t		*widget;

    if ((widget = Widget_pointer(widget_desc)) == NULL) {
	return 0;
    }
    return widget->window;
}

static void Widget_destroy_viewer(widget_t *w)
{
    widget_viewer_t		*v = (widget_viewer_t *)w->sub;

    if (v->num_lines > 0 && v->line != NULL) {
	free(v->line);
    }
    v->num_lines = 0;
    v->line = NULL;
}

void Widget_destroy(int widget_desc)
{
    int			i;
    widget_t		*w,
			*parent;
    widget_form_t	*form;

    if ((w = Widget_pointer(widget_desc)) != NULL) {
	w->type = WIDGET_DUMMY;
	w->name = NULL;
	if (w->sub != NULL) {
	    if (w->type == WIDGET_FORM) {
		form = (widget_form_t *) w->sub;
		if (form->children != NULL) {
		    for (i = 0; i < form->num_children; i++) {
			Widget_destroy(form->children[i]);
		    }
		    free(form->children);
		    form->children = NULL;
		    form->num_children = 0;
		}
	    }
	    else if (w->type == WIDGET_VIEWER) {
		Widget_destroy_viewer(w);
	    }
	    free(w->sub);
	    w->sub = NULL;
	}
	if (w->window != 0) {
	    XDestroyWindow(dpy, w->window);
	    w->window = 0;
	}
	if (w->parent_desc != NO_WIDGET) {
	    if ((parent = Widget_pointer(w->parent_desc)) != NULL
		&& parent->type == WIDGET_FORM) {
		form = (widget_form_t *) parent->sub;
		for (i = 0; i < form->num_children; i++) {
		    if (form->children[i] == widget_desc) {
			form->children[i] = NO_WIDGET;
		    }
		}
	    }
	    w->parent_desc = NO_WIDGET;
	}
    }
}

static widget_t *Widget_new(int *descp)
{
    int			i;

    if (widgets != NULL) {
	if (max_widgets > 0) {
	    if (num_widgets < max_widgets) {
		if (descp != NULL) {
		    *descp = num_widgets;
		}
		return &widgets[num_widgets++];
	    }
	    for (i = 1; i < num_widgets; i++) {
		if (widgets[i].type == WIDGET_DUMMY) {
		    if (descp != NULL) {
			*descp = i;
		    }
		    return &widgets[i];
		}
	    }
	}
    }
    if (widgets == NULL || max_widgets <= 0) {
	num_widgets = 0;
	max_widgets = 10;
	widgets = (widget_t *) malloc(max_widgets * sizeof(widget_t));
    } else {
	max_widgets += 10;
	widgets = (widget_t *) realloc(widgets,
				       max_widgets * sizeof(widget_t));
    }
    if (widgets == NULL) {
	num_widgets = max_widgets = 0;
	error("No memory for widgets");
	return NULL;
    }
    else if (num_widgets == 0) {
	/*
	 * The first widget is a dummy.
	 */
	widgets[num_widgets].type = WIDGET_DUMMY;
	widgets[num_widgets].parent_desc = NO_WIDGET;
	widgets[num_widgets].window = 0;
	widgets[num_widgets].width = 0;
	widgets[num_widgets].height = 0;
	widgets[num_widgets].sub = NULL;
	num_widgets++;
    }
    if (descp != NULL) {
	*descp = num_widgets;
    }
    return &widgets[num_widgets++];
}

static int Widget_create(widget_type_t type, char *name, Window window,
			 int width, int height, void *sub)
{
    int			desc;
    widget_t		*widget;

    if ((widget = Widget_new(&desc)) != NULL) {
	widget->type = type;
	widget->name = name;
	widget->parent_desc = NO_WIDGET;
	widget->window = window;
	widget->width = width;
	widget->height = height;
	widget->sub = sub;
    } else {
	if (sub != NULL) {
	    free(sub);
	}
	XDestroyWindow(dpy, window);
    }

    return desc;
}

static int Widget_add_child(int parent_desc, int child_desc)
{
    int			i;
    widget_t		*parent,
			*child;
    widget_form_t	*form;
    const int		incr = 1;

    if ((parent = Widget_pointer(parent_desc)) == NULL
	|| (child = Widget_pointer(child_desc)) == NULL) {
	errno = 0;
	error("Can't add child widget to parent");
	return NO_WIDGET;
    }
    if (parent->type != WIDGET_FORM || parent->sub == NULL) {
	errno = 0;
	error("Not a form widget");
	return NO_WIDGET;
    }
    if (child->parent_desc != NO_WIDGET) {
	errno = 0;
	error("Widget parent non-zero");
	child->parent_desc = NO_WIDGET;
    }
    form = (widget_form_t *) parent->sub;
    for (i = 0; i < form->num_children; i++) {
	if (widgets[form->children[i]].type == WIDGET_DUMMY) {
	    form->children[i] = child_desc;
	    child->parent_desc = parent_desc;
	    return child_desc;
	}
    }
    if (form->num_children == 0) {
	form->children = (int *) malloc((form->num_children + incr)
					* sizeof(*form->children));
    } else {
	form->children = (int *) realloc(form->children,
					 (form->num_children + incr)
					 * sizeof(*form->children));
    }
    if (form->children == NULL) {
	form->num_children = 0;
	error("No memory for form children");
	return NO_WIDGET;
    }
    else {
	for (i = 1; i < incr; i++) {
	    form->children[form->num_children + i] = 0;
	}
	form->children[form->num_children] = child_desc;
	child->parent_desc = parent_desc;
	form->num_children += incr;
    }
    return child_desc;
}

int Widget_resize(int widget_desc, int width, int height)
{
    widget_t			*widget;

    if ((widget = Widget_pointer(widget_desc)) == NULL) {
#ifndef SILENT
	printf("no widget pointer for resize (%d)\n", widget_desc);
#endif
	return NO_WIDGET;
    }
    XResizeWindow(dpy, widget->window, width, height);
    widget->width = width;
    widget->height = height;
    return widget_desc;
}

static void Widget_draw_button(widget_t *widget, bool inverse, char *label)
{
    int			x, y;
    unsigned long	fg, bg;

    if (mono) {
	if (inverse) {
	    fg = colors[BLACK].pixel;
	    bg = colors[WHITE].pixel;
	} else {
	    fg = colors[WHITE].pixel;
	    bg = colors[BLACK].pixel;
	}
    } else {
	if (inverse) {
	    fg = colors[RED].pixel;
	    bg = colors[BLACK].pixel;
	} else {
	    fg = colors[BLACK].pixel;
	    bg = colors[RED].pixel;
	}
    }
    XSetForeground(dpy, buttonGC, bg);
    XFillRectangle(dpy, widget->window, buttonGC,
		   0, 0, widget->width, widget->height);
    XSetForeground(dpy, buttonGC, colors[buttonColor].pixel);

    if (mono) {
	if (inverse) {
	    fg = colors[BLACK].pixel;
	    bg = colors[WHITE].pixel;
	} else {
	    fg = colors[WHITE].pixel;
	    bg = colors[BLACK].pixel;
	}
    } else {
	if (inverse) {
	    fg = colors[WHITE].pixel;
	    bg = colors[BLACK].pixel;
	} else {
	    fg = colors[WHITE].pixel;
	    bg = colors[BLACK].pixel;
	}
    }
    y = (widget->height - (buttonFont->ascent + buttonFont->descent)) / 2;
    x = (widget->width - XTextWidth(buttonFont, label, strlen(label))) / 2;
    ShadowDrawString(dpy, widget->window, buttonGC,
		     x, buttonFont->ascent + y,
		     label, fg, bg);
}

static void Widget_draw_input(widget_t *widget, char *str)
{
    XClearWindow(dpy, widget->window);
    XDrawString(dpy, widget->window, textGC,
		(widget->width
		 - XTextWidth(textFont, str, strlen(str))) / 2,
		textFont->ascent + (widget->height
		 - (textFont->ascent + textFont->descent)) / 2,
		str, strlen(str));
}

static void Widget_draw_arrow(widget_t *widget)
{
    int			bg,
			fg,
			left,
			right,
			top,
			bottom;
    XPoint		pts[4];
    widget_arrow_t	*arroww;

    arroww = (widget_arrow_t *) widget->sub;

    left = widget->width / 4;
    right = widget->width - left;
    top = widget->height / 4;
    bottom = widget->height - top;
    if (widget->type == WIDGET_BUTTON_ARROW_RIGHT) {
	int tmp = left; left = right; right = tmp;
    }
    if (arroww->pressed == true && arroww->inside == true) {
	fg = BLACK;
	bg = RED;
    } else {
	fg = RED;
	bg = BLACK;
    }
    XSetForeground(dpy, buttonGC, colors[bg].pixel);
    XFillRectangle(dpy, widget->window, buttonGC,
		   0, 0,
		   widget->width, widget->height);
    XSetForeground(dpy, buttonGC, colors[fg].pixel);
    pts[0].x = left;
    pts[0].y = widget->height / 2;
    pts[1].x = right;
    pts[1].y = top;
    pts[2].x = right;
    pts[2].y = bottom;
    pts[3].x = pts[0].x;
    pts[3].y = pts[0].y;
    XFillPolygon(dpy, widget->window, buttonGC, pts, 4,
		 Convex, CoordModeOrigin);
    XSetForeground(dpy, buttonGC, colors[WHITE].pixel);
}

static void Widget_draw_slider(widget_t *widget)
{
    int			i,
			unit,
			block_size,
			block_max_size,
			block_offset,
			block_start;
    XPoint		pts[4];
    widget_slider_t	*sliderw = (widget_slider_t *) widget->sub;
    widget_t		*viewer_widget = Widget_pointer(sliderw->viewer_desc);
    widget_viewer_t	*viewerw = (widget_viewer_t *)viewer_widget->sub;

    XSetForeground(dpy, buttonGC, colors[RED].pixel);
    if (widget->type == WIDGET_SLIDER_HORI) {
	unit = widget->height;
	pts[0].x = unit / 4;
	pts[0].y = unit / 2;
	pts[1].x = unit / 4 * 3;
	pts[1].y = unit / 4;
	pts[2].x = unit / 4 * 3;
	pts[2].y = unit / 4 * 3;
	pts[3] = pts[0];
	XFillPolygon(dpy, widget->window, buttonGC, pts, 4,
		     Convex, CoordModeOrigin);
	for (i = 0; i < 4; i++) {
	    pts[i].x = widget->width - pts[i].x;
	}
	XFillPolygon(dpy, widget->window, buttonGC, pts, 4,
		     Convex, CoordModeOrigin);
	block_offset = unit;
	block_max_size = widget->width - 2 * block_offset;
	block_size = block_max_size * viewer_widget->width
				    / viewerw->real_width;
	block_start = block_max_size * viewerw->visible_x
				    / viewerw->real_width;
	XFillRectangle(dpy, widget->window, buttonGC,
		       block_offset + block_start, unit / 4,
		       block_size, unit / 2);
	XSetForeground(dpy, buttonGC, colors[BLACK].pixel);
	XFillRectangle(dpy, widget->window, buttonGC,
		       block_offset, unit / 4,
		       block_start, unit / 2);
	XFillRectangle(dpy, widget->window, buttonGC,
		       block_offset + block_start + block_size, unit / 4,
		       block_max_size - (block_start + block_size), unit / 2);
    }
    else if (widget->type == WIDGET_SLIDER_VERT) {
	unit = widget->width;
	pts[0].x = unit / 2;
	pts[0].y = unit / 4;
	pts[1].x = unit / 4;
	pts[1].y = unit / 4 * 3;
	pts[2].x = unit / 4 * 3;
	pts[2].y = unit / 4 * 3;
	pts[3] = pts[0];
	XFillPolygon(dpy, widget->window, buttonGC, pts, 4,
		     Convex, CoordModeOrigin);
	for (i = 0; i < 4; i++) {
	    pts[i].y = widget->height - pts[i].y;
	}
	XFillPolygon(dpy, widget->window, buttonGC, pts, 4,
		     Convex, CoordModeOrigin);
	block_offset = unit;
	block_max_size = widget->height - 2 * block_offset;
	block_size = block_max_size * viewer_widget->height
				    / viewerw->real_height;
	block_start = block_max_size * viewerw->visible_y
				    / viewerw->real_height;
	XFillRectangle(dpy, widget->window, buttonGC,
		       unit / 4, block_offset + block_start,
		       unit / 2, block_size);
	XSetForeground(dpy, buttonGC, colors[BLACK].pixel);
	XFillRectangle(dpy, widget->window, buttonGC,
		       unit / 4, block_offset,
		       unit / 2, block_start);
	XFillRectangle(dpy, widget->window, buttonGC,
		       unit / 4, block_offset + block_start + block_size,
		       unit / 2, block_max_size - (block_start + block_size));
    }
    XSetForeground(dpy, buttonGC, colors[WHITE].pixel);
}

static void Widget_viewer_draw_lines(widget_t *widget, int start, int end)
{
    widget_viewer_t	*viewerw = (widget_viewer_t *)widget->sub;
    int			text_height = viewerw->font->ascent
				    + viewerw->font->descent,
			i,
			x = 20,
			y = 20 + viewerw->font->ascent + start * text_height;

    for (i = start; i < end; i++) {
	if (!mono) {
	    XSetForeground(dpy, motdGC, colors[BLACK].pixel);
	    XDrawString(dpy, widget->window, motdGC,
			x+1, y+1,
			(char *) viewerw->line[i].txt, viewerw->line[i].len);
	    XSetForeground(dpy, motdGC, colors[WHITE].pixel);
	    XDrawString(dpy, widget->window, motdGC,
			x-1, y-1,
			(char *) viewerw->line[i].txt, viewerw->line[i].len);
	}
	else {
	    XDrawString(dpy, widget->window, motdGC,
			x, y,
			(char *) viewerw->line[i].txt, viewerw->line[i].len);
	}
	y += text_height;
    }
    XSetForeground(dpy, motdGC, colors[WHITE].pixel);
}

static void Widget_draw_viewer(widget_t *widget, XExposeEvent *expose)
{
    widget_viewer_t	*viewerw = (widget_viewer_t *)widget->sub;
    int			start,
			end,
			y0,
			y1,
			text_height = viewerw->font->ascent
				    + viewerw->font->descent;

    y0 = viewerw->visible_y;
    y1 = viewerw->real_height;
    if (expose != NULL) {
	y0 = MAX(y0, expose->y);
	y1 = MIN(y1, expose->y + expose->height);
    }
    else {
	y1 = MIN(y1, viewerw->visible_y + widget->height);
    }
    start = (y0 - 20) / text_height;
    end = (y1 - 20) / text_height + 1;
    if (start < 0) {
	start = 0;
    }
    if (end > viewerw->num_lines) {
	end = viewerw->num_lines;
    }
    if (start < end) {
	Widget_viewer_draw_lines(widget, start, end);
    }
}

static void Widget_draw_expose(int widget_desc, XExposeEvent *expose)
{
    widget_t			*widget;
    widget_label_t		*labelw;
    widget_bool_t		*boolw;
    widget_menu_t		*menuw;
    widget_entry_t		*entryw;
    widget_activate_t		*activw;
    widget_int_t		*intw;
    widget_float_t		*floatw;
    char			buf[16];

    if ((widget = Widget_pointer(widget_desc)) == NULL) {
	errno = 0;
	error("Widget draw invalid");
	return;
    }

    switch (widget->type) {

    case WIDGET_FORM:
	if (expose && expose->count > 0)
	    break;
	XClearWindow(dpy, widget->window);
	break;

    case WIDGET_LABEL:
	if (expose && expose->count > 0)
	    break;
	labelw = (widget_label_t *) widget->sub;
	ShadowDrawString(dpy, widget->window, textGC,
			 labelw->x_offset, textFont->ascent + labelw->y_offset,
			 labelw->str, colors[WHITE].pixel, colors[BLACK].pixel);
	break;

    case WIDGET_BUTTON_BOOL:
	if (expose && expose->count > 0)
	    break;
	boolw = (widget_bool_t *) widget->sub;
	Widget_draw_button(widget,
			   (boolw->pressed && boolw->inside) ? true : false,
			   (boolw->state == true) ? "Yes" : "No");
	break;

    case WIDGET_BUTTON_MENU:
	if (expose && expose->count > 0)
	    break;
	menuw = (widget_menu_t *) widget->sub;
	Widget_draw_button(widget, false, menuw->str);
	break;

    case WIDGET_BUTTON_ACTIVATE:
	if (expose && expose->count > 0)
	    break;
	activw = (widget_activate_t *) widget->sub;
	Widget_draw_button(widget,
			   (activw->pressed && activw->inside) ? true : false,
			   activw->str);
	break;

    case WIDGET_BUTTON_ENTRY:
	if (expose && expose->count > 0)
	    break;
	entryw = (widget_entry_t *) widget->sub;
	Widget_draw_button(widget, entryw->inside, entryw->str);
	break;

    case WIDGET_INPUT_INT:
	if (expose && expose->count > 0)
	    break;
	intw = (widget_int_t *) widget->sub;
	sprintf(buf, "%d", *intw->val);
	Widget_draw_input(widget, buf);
	break;

    case WIDGET_INPUT_FLOAT:
	if (expose && expose->count > 0)
	    break;
	floatw = (widget_float_t *) widget->sub;
	if (floatw->max <= 1.0) {
	    sprintf(buf, "%.2f", *floatw->val);
	}
	else if (floatw->max <= 10.0) {
	    sprintf(buf, "%.1f", *floatw->val);
	} else {
	    sprintf(buf, "%d", (int) *floatw->val);
	}
	Widget_draw_input(widget, buf);
	break;

    case WIDGET_BUTTON_ARROW_LEFT:
    case WIDGET_BUTTON_ARROW_RIGHT:
	if (expose && expose->count > 0)
	    break;
	Widget_draw_arrow(widget);
	break;

    case WIDGET_SLIDER_HORI:
    case WIDGET_SLIDER_VERT:
	if (expose && expose->count > 0)
	    break;
	Widget_draw_slider(widget);
	break;

    case WIDGET_VIEWER:
	Widget_draw_viewer(widget, expose);
	break;

#if 1 && !defined(SILENT)
    default:
	printf("Widget_draw: default %d\n", widget->type);
	break;
#endif
    }
}

void Widget_draw(int widget_desc)
{
    Widget_draw_expose(widget_desc, NULL);
}

static void Widget_button_slider(XEvent *event, widget_t *widget, bool pressed)
{
    int			unit,
			new_x,
			new_y,
			block_size,
			block_max_size,
			block_start,
			block_new_start,
			block_offset;
    widget_slider_t	*sliderw = (widget_slider_t *) widget->sub;
    widget_t		*viewer_widget;
    widget_viewer_t	*viewerw;

    viewer_widget = Widget_pointer(sliderw->viewer_desc);
    viewerw = (widget_viewer_t *)viewer_widget->sub;

    if (sliderw->pressed == false && pressed == false) {
	errno = 0;
	error("Slider widget not pressed");
	return;
    }
    sliderw->pressed = pressed;
    if (!pressed) {
	return;
    }

    if (widget->type == WIDGET_SLIDER_VERT) {
	unit = widget->width;
	block_offset = unit;
	block_max_size = widget->height - 2 * block_offset;
	block_size = block_max_size * viewer_widget->height
				    / viewerw->real_height;
	block_start = block_offset + block_max_size * viewerw->visible_y
						/ viewerw->real_height;
	block_new_start = block_start;
	if (event->xbutton.y < unit) {
	    block_new_start = block_start - block_size;
	}
	else if (event->xbutton.y >= block_offset + block_max_size) {
	    block_new_start = block_start + block_size;
	}
	else {
	    block_new_start = event->xbutton.y - block_size / 2;
	}
	if (block_new_start < block_offset) {
	    block_new_start = block_offset;
	}
	else if (block_new_start + block_size > block_offset + block_max_size) {
	    block_new_start = block_offset + block_max_size - block_size;
	}
	if (block_new_start != block_start) {
	    new_y = viewerw->real_height * (block_new_start - block_offset)
		    / block_max_size;
	    viewerw->visible_y = new_y;
	    XMoveWindow(dpy, viewer_widget->window,
			- viewerw->visible_x, - viewerw->visible_y);
	    Widget_draw_slider(widget);
	}
    }
    else if (widget->type == WIDGET_SLIDER_HORI) {
	unit = widget->height;
	block_offset = unit;
	block_max_size = widget->width - 2 * block_offset;
	block_size = block_max_size * viewer_widget->width
				    / viewerw->real_width;
	block_start = block_offset + block_max_size * viewerw->visible_x
						/ viewerw->real_width;
	block_new_start = block_start;
	if (event->xbutton.x < unit) {
	    block_new_start = block_start - block_size;
	}
	else if (event->xbutton.x >= block_offset + block_max_size) {
	    block_new_start = block_start + block_size;
	}
	else {
	    block_new_start = event->xbutton.x - block_size / 2;
	}
	if (block_new_start < block_offset) {
	    block_new_start = block_offset;
	}
	else if (block_new_start + block_size > block_offset + block_max_size) {
	    block_new_start = block_offset + block_max_size - block_size;
	}
	if (block_new_start != block_start) {
	    new_x = viewerw->real_width * (block_new_start - block_offset)
		    / block_max_size;
	    viewerw->visible_x = new_x;
	    XMoveWindow(dpy, viewer_widget->window,
			- viewerw->visible_x, - viewerw->visible_y);
	    Widget_draw_slider(widget);
	}
    }
}

struct widget_check_event {
    Window		window;
    int			found;
};

static Bool Widget_check_motion(Display *d, XEvent *e, char *p)
{
    struct widget_check_event	*cm = (struct widget_check_event *) p;

    if (e->xany.window == cm->window) {
	if (e->type == MotionNotify) {
	    cm->found++;
	}
    }
    return False;
}

static void Widget_button_motion(XEvent *event, int widget_desc)
{
    widget_t			*widget;
    struct widget_check_event	cm;
    XEvent			dumb;

    if ((widget = Widget_pointer(widget_desc)) == NULL) {
	errno = 0;
	error("Widget button motion invalid");
	return;
    }

    switch (widget->type) {

    case WIDGET_SLIDER_HORI:
    case WIDGET_SLIDER_VERT:
	cm.window = widget->window;
	cm.found = 0;
	XCheckIfEvent(dpy, &dumb, Widget_check_motion, (char *)&cm);
	if (cm.found > 0) {
	    break;
	}
	Widget_button_slider(event, widget, true);
	break;

#if 1 && !defined(SILENT)
    default:
	printf("Widget_button_motion: default %d\n", widget->type);
	break;
#endif
    }
}

static void Widget_button(XEvent *event, int widget_desc, bool pressed)
{
    widget_t			*widget,
				*entry_widget,
				*pulldown_widget,
				*sub_widget;
    widget_bool_t		*boolw;
    widget_menu_t		*menuw;
    widget_form_t		*pullw;
    widget_activate_t		*activw;
    widget_int_t		*intw;
    widget_float_t		*floatw;
    widget_arrow_t		*arroww;
    widget_entry_t		*entryw;
    int				i,
				ival,
				sub_widget_desc;
    float			fval,
				delta,
				fmin,
				offset,
				newoffset;

    if ((widget = Widget_pointer(widget_desc)) == NULL) {
	errno = 0;
	error("Widget button invalid");
	return;
    }
    switch (widget->type) {
    case WIDGET_BUTTON_BOOL:
	boolw = (widget_bool_t *) widget->sub;
	if (boolw->pressed == false && pressed == false) {
	    errno = 0;
	    error("Bool widget not pressed");
	    break;
	}
	boolw->pressed = pressed;
	if (boolw->inside == false) {
	    break;
	}
	if (pressed == false) {
	    boolw->state = (boolw->state == true) ? false : true;
	}
	Widget_draw(widget_desc);
	if (pressed == false) {
	    if (boolw->callback) {
		if ((*boolw->callback)(widget_desc,
				   boolw->user_data, &boolw->state) == 1) {
		    Widget_draw(widget_desc);
		}
	    }
	}
	break;
    case WIDGET_BUTTON_MENU:
	menuw = (widget_menu_t *) widget->sub;
	if (menuw->pressed == false && pressed == false) {
	    errno = 0;
	    error("Menu widget not pressed");
	    break;
	}
	menuw->pressed = pressed;
	Widget_draw(widget_desc);
	if ((pulldown_widget = Widget_pointer(menuw->pulldown_desc)) == NULL) {
	    errno = 0;
	    error("No pulldown widget");
	    break;
	}
	if (pulldown_widget->type != WIDGET_FORM) {
	    errno = 0;
	    error("Pulldown not a form");
	    break;
	}
	if (pressed == true) {
	    XMoveWindow(dpy, pulldown_widget->window,
			event->xbutton.x_root - event->xbutton.x - 1,
			event->xbutton.y_root - event->xbutton.y
			    + widget->height);
	    XMapRaised(dpy, pulldown_widget->window);
	    XFlush(dpy);
	} else {
	    XUnmapWindow(dpy, pulldown_widget->window);
	    pullw = (widget_form_t *) pulldown_widget->sub;
	    for (i = 0; i < pullw->num_children; i++) {
		if ((entry_widget = Widget_pointer(pullw->children[i]))
		    == NULL) {
		    continue;
		}
		entryw = (widget_entry_t *) entry_widget->sub;
		if (entryw->inside == false) {
		    continue;
		}
		entryw->inside = false;
		if (entryw->callback) {
		    (*entryw->callback)(pullw->children[i],
					entryw->user_data,
					&entryw->str);
		}
	    }
	}
	break;
    case WIDGET_BUTTON_ACTIVATE:
	activw = (widget_activate_t *) widget->sub;
	if (activw->pressed == false && pressed == false) {
	    errno = 0;
	    error("Activate widget not pressed");
	    break;
	}
	activw->pressed = pressed;
	if (activw->inside == false) {
	    break;
	}
	Widget_draw(widget_desc);
	if (pressed == false
	    && activw->callback) {
	    if ((*activw->callback)(widget_desc,
				    activw->user_data,
				    &activw->str) == 1) {
		Widget_draw(widget_desc);
	    }
	}
	break;
    case WIDGET_BUTTON_ARROW_LEFT:
    case WIDGET_BUTTON_ARROW_RIGHT:
	arroww = (widget_arrow_t *) widget->sub;
	if (arroww->pressed == false && pressed == false) {
	    errno = 0;
	    error("Arrow widget not pressed");
	    break;
	}
	arroww->pressed = pressed;
	if (arroww->inside == false) {
	    break;
	}
	Widget_draw(widget_desc);
	if (pressed == false
	    && (sub_widget_desc = arroww->widget_desc) != NO_WIDGET
	    && (sub_widget = Widget_pointer(sub_widget_desc)) != NULL) {
	    switch (sub_widget->type) {
	    case WIDGET_INPUT_INT:
		intw = (widget_int_t *) sub_widget->sub;
		ival = *intw->val;
		LIMIT(ival, intw->min, intw->max);
		if (widget->type == WIDGET_BUTTON_ARROW_RIGHT) {
		    ival = (int)(*intw->val * 1.05 + 0.5);
		    if (ival == *intw->val) {
			ival++;
		    }
		} else {
		    ival = (int)(*intw->val * 0.95);
		    if (ival == *intw->val) {
			ival--;
		    }
		}
		LIMIT(ival, intw->min, intw->max);
		if (ival != *intw->val) {
		    *intw->val = ival;
		    Widget_draw(sub_widget_desc);
		    if (intw->callback) {
			if ((*intw->callback)(sub_widget_desc,
					  intw->user_data, intw->val) == 1) {
			    Widget_draw(sub_widget_desc);
			}
		    }
		}
		break;
	    case WIDGET_INPUT_FLOAT:
		floatw = (widget_float_t *) sub_widget->sub;
		fval = *floatw->val;
		LIMIT(fval, floatw->min, floatw->max);
		delta = floatw->max - floatw->min;
		if (fval >= 0) {
		    if (floatw->min < 0) {
			fmin = 0;
		    } else {
			fmin = floatw->min;
		    }
		    offset = fval - fmin;
		} else {
		    if (floatw->max > 0) {
			fmin = 0;
		    } else {
			fmin = floatw->max;
		    }
		    offset = -fval + fmin;
		}
		if ((widget->type == WIDGET_BUTTON_ARROW_RIGHT)
		    == (fval >= 0)) {
		    newoffset = (float)(offset * 1.05);
		    if (newoffset - offset < delta / 100.0) {
			newoffset = offset + delta / 100.0;
		    }
		} else {
		    newoffset = (float)(offset * 0.95);
		    if (newoffset - offset > -delta / 100.0) {
			newoffset = offset - delta / 100.0;
		    }
		    if (newoffset < 0 && offset > 0) {
			newoffset = 0;
		    }
		}
		if (fval >= 0) {
		    fval = fmin + newoffset;
		} else {
		    fval = fmin - newoffset;
		}
		LIMIT(fval, floatw->min, floatw->max);
		if (fval != *floatw->val) {
		    *floatw->val = fval;
		    Widget_draw(sub_widget_desc);
		    if (floatw->callback) {
			if ((*floatw->callback)(sub_widget_desc,
						floatw->user_data,
						floatw->val) == 1) {
			    Widget_draw(sub_widget_desc);
			}
		    }
		}
		break;
	    default:
		/*NOTREACHED*/
		break;
	    }
	}
	break;
    case WIDGET_SLIDER_HORI:
    case WIDGET_SLIDER_VERT:
	Widget_button_slider(event, widget, pressed);
	break;
#if 1 && !defined(SILENT)
    default:
	printf("Widget_button: default %d\n", widget->type);
	break;
#endif
    }
}

static void Widget_inside(XEvent *event, int widget_desc, bool inside)
{
    widget_t		*widget;
    widget_entry_t	*entryw;
    widget_bool_t	*boolw;
    widget_activate_t	*activw;
    widget_arrow_t	*arroww;

    if ((widget = Widget_pointer(widget_desc)) == NULL) {
	errno = 0;
	error("Widget inside invalid");
	return;
    }
    switch (widget->type) {
    case WIDGET_BUTTON_MENU:
	/* not used. */
	break;
    case WIDGET_BUTTON_ENTRY:
	entryw = (widget_entry_t *) widget->sub;
	entryw->inside = inside;
	Widget_draw(widget_desc);
	break;
    case WIDGET_BUTTON_BOOL:
	boolw = (widget_bool_t *) widget->sub;
	boolw->inside = inside;
	if (boolw->pressed == true) {
	    Widget_draw(widget_desc);
	}
	break;
    case WIDGET_BUTTON_ACTIVATE:
	activw = (widget_activate_t *) widget->sub;
	activw->inside = inside;
	if (activw->pressed == true) {
	    Widget_draw(widget_desc);
	}
	break;
    case WIDGET_BUTTON_ARROW_RIGHT:
    case WIDGET_BUTTON_ARROW_LEFT:
	arroww = (widget_arrow_t *) widget->sub;
	arroww->inside = inside;
	if (arroww->pressed == true) {
	    Widget_draw(widget_desc);
	}
	break;
#if 1 && !defined(SILENT)
    default:
	printf("Widget_inside: default %d\n", widget->type);
	break;
#endif
    }
}

int Widget_event(XEvent *event)
{
    int			i,
			count;
    widget_t		*widget;
    widget_bool_t	*boolw;
    widget_activate_t	*activw;
    widget_menu_t	*menuw;
    widget_arrow_t	*arroww;
    widget_slider_t	*sliderw;

    if (event->type == ButtonRelease) {
	if (event->xbutton.button == Button1) {
	    count = 0;
	    for (i = 1; i < num_widgets; i++) {
		if ((widget = Widget_pointer(i)) == NULL) {
		    continue;
		}
		switch (widget->type) {
		case WIDGET_BUTTON_BOOL:
		    boolw = (widget_bool_t *) widget->sub;
		    if (boolw->pressed == true) {
			count++;
			Widget_button(event, i, false);
		    }
		    break;
		case WIDGET_BUTTON_ACTIVATE:
		    activw = (widget_activate_t *) widget->sub;
		    if (activw->pressed == true) {
			count++;
			Widget_button(event, i, false);
		    }
		    break;
		case WIDGET_BUTTON_MENU:
		    menuw = (widget_menu_t *) widget->sub;
		    if (menuw->pressed == true) {
			count++;
			Widget_button(event, i, false);
		    }
		    break;
		case WIDGET_BUTTON_ARROW_RIGHT:
		case WIDGET_BUTTON_ARROW_LEFT:
		    arroww = (widget_arrow_t *) widget->sub;
		    if (arroww->pressed == true) {
			count++;
			Widget_button(event, i, false);
		    }
		    break;
		case WIDGET_SLIDER_HORI:
		case WIDGET_SLIDER_VERT:
		    sliderw = (widget_slider_t *) widget->sub;
		    if (sliderw->pressed == true) {
			count++;
			Widget_button(event, i, false);
		    }
		    break;
		default:
		    /*NOTREACHED*/
		    break;
		}
	    }
	    if (count > 0) {
		return 1;
	    }
	}
    } else {
	for (i = 1; i < num_widgets; i++) {
	    if (widgets[i].type == WIDGET_DUMMY) {
		continue;
	    }
	    if (widgets[i].window == event->xany.window) {
		switch (event->type) {
		case Expose:
		    Widget_draw_expose(i, &event->xexpose);
		    break;
		case ButtonPress:
		    if (event->xbutton.button == Button1) {
			Widget_button(event, i, true);
		    }
		    break;
		case MotionNotify:
		    Widget_button_motion(event, i);
		    break;
		case EnterNotify:
		    Widget_inside(event, i, true);
		    break;
		case LeaveNotify:
		    Widget_inside(event, i, false);
		    break;
		case ConfigureNotify:
		    if (widgets[i].name != NULL
			&& strncmp(widgets[i].name, "popup", 5) == 0) {
			if (strcmp(widgets[i].name, "popup_viewer") == 0) {
			    Widget_resize_viewer(event, i);
			}
		    }
		    break;
		}
		return 1;
	    }
	}
    }
    return 0;
}

static int Widget_form_window(Window window, int parent_desc,
			      int width, int height)
{
    int			widget_desc;
    widget_t		*parent_widget;
    widget_form_t	*formw;

    if (parent_desc != NO_WIDGET) {
	if ((parent_widget = Widget_pointer(parent_desc)) == NULL) {
	    errno = 0;
	    error("Widget_form_window: Invalid parent widget");
	    XDestroyWindow(dpy, window);
	    return NO_WIDGET;
	}
    }
    if ((formw = (widget_form_t *) malloc(sizeof(*formw))) == NULL) {
	error("No memory for form widget");
	XDestroyWindow(dpy, window);
	return NO_WIDGET;
    }
    formw->children = NULL;
    formw->num_children = 0;
    widget_desc = Widget_create(WIDGET_FORM, "form", window,
				width, height, formw);
    if (widget_desc == NO_WIDGET) {
	XDestroyWindow(dpy, window);
	return NO_WIDGET;
    }
    if (parent_desc != NO_WIDGET) {
	if (Widget_add_child(parent_desc, widget_desc) == NO_WIDGET) {
	    Widget_destroy(widget_desc);
	    return NO_WIDGET;
	}
    }
    return widget_desc;
}

int Widget_create_form(int parent_desc, Window parent_window,
		       int x, int y, int width, int height,
		       int border)
{
    Window		window;
    widget_t		*parent_widget;

    if (parent_desc != NO_WIDGET) {
	if ((parent_widget = Widget_pointer(parent_desc)) == NULL) {
	    errno = 0;
	    error("Widget_create_form: Invalid parent widget");
	    return NO_WIDGET;
	}
	parent_window = parent_widget->window;
    }
    window =
	XCreateSimpleWindow(dpy, parent_window,
			    x, y, width, height,
			    border, colors[borderColor].pixel,
			    colors[windowColor].pixel);
    XSelectInput(dpy, window, 0);
    return Widget_form_window(window, parent_desc, width, height);
}

int Widget_create_activate(int parent_desc,
			   int x, int y, int width, int height,
			   int border, char *str,
			   int (*callback)(int, void *, char **),
			   void *user_data)
{
    int			widget_desc;
    Window		window;
    widget_t		*parent_widget;
    widget_activate_t	*activw;

    if ((parent_widget = Widget_pointer(parent_desc)) == NULL
	|| parent_widget->type != WIDGET_FORM) {
	errno = 0;
	error("Widget_create_activate: Invalid parent widget");
	return NO_WIDGET;
    }
    if ((activw = (widget_activate_t *) malloc(sizeof(*activw))) == NULL) {
	error("No memory for activate widget");
	return NO_WIDGET;
    }
    activw->pressed = false;
    activw->inside = false;
    activw->str = str;
    activw->callback = callback;
    activw->user_data = user_data;
    window =
	XCreateSimpleWindow(dpy, parent_widget->window,
			    x, y, width, height,
			    border, colors[borderColor].pixel,
			    colors[buttonColor].pixel);
    XSelectInput(dpy, window,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask
		 | OwnerGrabButtonMask | EnterWindowMask | LeaveWindowMask);
    widget_desc = Widget_create(WIDGET_BUTTON_ACTIVATE, "activate", window,
				width, height, activw);
    if (widget_desc == NO_WIDGET) {
	return NO_WIDGET;
    }
    if (Widget_add_child(parent_desc, widget_desc) == NO_WIDGET) {
	Widget_destroy(widget_desc);
	return NO_WIDGET;
    }
    return widget_desc;
}

int Widget_create_bool(int parent_desc,
		       int x, int y, int width, int height,
		       int border, bool val,
		       int (*callback)(int, void *, bool *),
		       void *user_data)
{
    int			widget_desc;
    Window		window;
    widget_t		*parent_widget;
    widget_bool_t	*boolw;

    if ((parent_widget = Widget_pointer(parent_desc)) == NULL
	|| parent_widget->type != WIDGET_FORM) {
	errno = 0;
	error("Widget_create_bool: Invalid parent widget");
	return NO_WIDGET;
    }
    if ((boolw = (widget_bool_t *) malloc(sizeof(*boolw))) == NULL) {
	error("No memory for bool widget");
	return NO_WIDGET;
    }
    boolw->pressed = false;
    boolw->inside = false;
    boolw->state = val;
    boolw->callback = callback;
    boolw->user_data = user_data;
    window =
	XCreateSimpleWindow(dpy, parent_widget->window,
			    x, y, width, height,
			    border, colors[borderColor].pixel,
			    colors[buttonColor].pixel);
    XSelectInput(dpy, window,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask
		 | OwnerGrabButtonMask | EnterWindowMask | LeaveWindowMask);
    widget_desc = Widget_create(WIDGET_BUTTON_BOOL, "bool", window,
				width, height, boolw);
    if (widget_desc == NO_WIDGET) {
	return NO_WIDGET;
    }
    if (Widget_add_child(parent_desc, widget_desc) == NO_WIDGET) {
	Widget_destroy(widget_desc);
	return NO_WIDGET;
    }
    return widget_desc;
}

int Widget_add_pulldown_entry(int menu_desc, char *str,
			      int (*callback)(int, void *, char **),
			      void *user_data)
{
    int				entry_desc,
				pulldown_desc,
				width,
				height,
				pull_width,
				pull_height;
    widget_t			*menu_widget,
				*pulldown_widget;
    widget_menu_t		*menuw;
    widget_form_t		*pullw;
    widget_entry_t		*entryw;
    Window			window;
    XSetWindowAttributes	sattr;
    unsigned long		mask;

    if ((menu_widget = Widget_pointer(menu_desc)) == NULL
	|| menu_widget->type != WIDGET_BUTTON_MENU) {
	errno = 0;
	error("Widget_add_pulldown_entry: Invalid menu");
	return NO_WIDGET;
    }
    menuw = (widget_menu_t *) menu_widget->sub;
    pulldown_desc = menuw->pulldown_desc;
    if (pulldown_desc == NO_WIDGET) {
	mask = 0;
	sattr.background_pixel = colors[windowColor].pixel;
	mask |= CWBackPixel;
	sattr.border_pixel = colors[borderColor].pixel;
	mask |= CWBorderPixel;
	sattr.bit_gravity = NorthWestGravity;
	mask |= CWBitGravity;
	sattr.save_under = True;
	mask |= CWSaveUnder;
	sattr.event_mask = NoEventMask;
	mask |= CWEventMask;
	sattr.override_redirect = True;
	mask |= CWOverrideRedirect;
	if (colormap != 0) {
	    sattr.colormap = colormap;
	    mask |= CWColormap;
	}
	pull_width = menu_widget->width + 2;
	pull_height = menu_widget->height + 2;
	window = XCreateWindow(dpy,
			       DefaultRootWindow(dpy),
			       0, 0,
			       pull_width, pull_height,
			       0, dispDepth,
			       InputOutput, visual,
			       mask, &sattr);
	pulldown_desc
	    = Widget_form_window(window, NO_WIDGET,
				 pull_width, pull_height);
	if ((pulldown_widget = Widget_pointer(pulldown_desc)) == NULL) {
	    errno = 0;
	    error("Can't create pulldown");
	    return NO_WIDGET;
	}
	menuw->pulldown_desc = pulldown_desc;
    }
    else if ((pulldown_widget = Widget_pointer(pulldown_desc)) == NULL) {
	errno = 0;
	error("Not a pulldown");
	return NO_WIDGET;
    }
    pullw = (widget_form_t *) pulldown_widget->sub;

    if ((entryw = (widget_entry_t *) malloc(sizeof(*entryw))) == NULL) {
	error("No memory for entry widget");
	return NO_WIDGET;
    }
    entryw->inside = false;
    entryw->str = str;
    entryw->callback = callback;
    entryw->user_data = user_data;

    height = menu_widget->height;
    width = XTextWidth(buttonFont, str, strlen(str))
	+ (height - (buttonFont->ascent + buttonFont->descent));
    if (width < pulldown_widget->width - 2) {
	width = pulldown_widget->width - 2;
    }
    pull_height = (pullw->num_children + 1) * (menu_widget->height + 1) + 1;
    pull_width = pulldown_widget->width;
    if (pull_width < width + 2) {
	pull_width = width + 2;
    }
    Widget_resize(pulldown_desc, pull_width, pull_height);
    window =
	XCreateSimpleWindow(dpy, pulldown_widget->window,
			    0, pullw->num_children * (menu_widget->height + 1),
			    width, height,
			    1, colors[borderColor].pixel,
			    colors[buttonColor].pixel);
    XSelectInput(dpy, window,
		 ExposureMask | EnterWindowMask | LeaveWindowMask);
    entry_desc = Widget_create(WIDGET_BUTTON_ENTRY, "entry", window,
			       width, height, entryw);
    if (entry_desc == NO_WIDGET) {
	return NO_WIDGET;
    }
    if (Widget_add_child(pulldown_desc, entry_desc) == NO_WIDGET) {
	errno = 0;
	error("Can't create pulldown entry");
	Widget_destroy(entry_desc);
	return NO_WIDGET;
    }
    Widget_map(entry_desc);
    return menu_desc;
}

int Widget_create_menu(int parent_desc,
		       int x, int y, int width, int height,
		       int border, char *str)
{
    int			widget_desc;
    Window		window;
    widget_t		*parent_widget;
    widget_menu_t	*menuw;

    if ((parent_widget = Widget_pointer(parent_desc)) == NULL
	|| parent_widget->type != WIDGET_FORM) {
	errno = 0;
	error("Widget_create_menu: Invalid parent widget");
	return NO_WIDGET;
    }
    if ((menuw = (widget_menu_t *) malloc(sizeof(*menuw))) == NULL) {
	error("No memory for menu widget");
	return NO_WIDGET;
    }
    menuw->pressed = false;
    menuw->str = str;
    menuw->pulldown_desc = NO_WIDGET;
    window =
	XCreateSimpleWindow(dpy, parent_widget->window,
			    x, y, width, height,
			    border, colors[borderColor].pixel,
			    colors[buttonColor].pixel);
    XSelectInput(dpy, window,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask
		 | OwnerGrabButtonMask | EnterWindowMask | LeaveWindowMask);
    widget_desc = Widget_create(WIDGET_BUTTON_MENU, "menu", window,
				width, height, menuw);
    if (widget_desc == NO_WIDGET) {
	return NO_WIDGET;
    }
    if (Widget_add_child(parent_desc, widget_desc) == NO_WIDGET) {
	Widget_destroy(widget_desc);
	return NO_WIDGET;
    }
    return widget_desc;
}

int Widget_create_int(int parent_desc,
		      int x, int y, int width, int height,
		      int border, int *val, int min, int max,
		      int (*callback)(int, void *, int *),
		      void *user_data)
{
    int			widget_desc;
    Window		window;
    widget_t		*parent_widget;
    widget_int_t	*intw;

    if ((parent_widget = Widget_pointer(parent_desc)) == NULL
	|| parent_widget->type != WIDGET_FORM) {
	errno = 0;
	error("Widget_create_int: Invalid parent widget");
	return NO_WIDGET;
    }
    if ((intw = (widget_int_t *) malloc(sizeof(*intw))) == NULL) {
	error("No memory for int widget");
	return NO_WIDGET;
    }
    intw->val = val;
    intw->min = min;
    intw->max = max;
    intw->callback = callback;
    intw->user_data = user_data;
    window =
	XCreateSimpleWindow(dpy, parent_widget->window,
			    x, y, width, height,
			    border, colors[borderColor].pixel,
			    colors[BLACK].pixel);
    XSelectInput(dpy, window, ExposureMask);
    widget_desc = Widget_create(WIDGET_INPUT_INT, "input_int", window,
				width, height, intw);
    if (widget_desc == NO_WIDGET) {
	return NO_WIDGET;
    }
    if (Widget_add_child(parent_desc, widget_desc) == NO_WIDGET) {
	Widget_destroy(widget_desc);
	return NO_WIDGET;
    }
    return widget_desc;
}

int Widget_create_float(int parent_desc,
			int x, int y, int width, int height,
			int border, float *val, float min, float max,
			int (*callback)(int, void *, float *),
			void *user_data)
{
    int			widget_desc;
    Window		window;
    widget_t		*parent_widget;
    widget_float_t	*floatw;

    if ((parent_widget = Widget_pointer(parent_desc)) == NULL
	|| parent_widget->type != WIDGET_FORM) {
	errno = 0;
	error("Widget_create_float: Invalid parent widget");
	return NO_WIDGET;
    }
    if ((floatw = (widget_float_t *) malloc(sizeof(*floatw))) == NULL) {
	error("No memory for float widget");
	return NO_WIDGET;
    }
    floatw->val = val;
    floatw->min = min;
    floatw->max = max;
    floatw->callback = callback;
    floatw->user_data = user_data;
    window =
	XCreateSimpleWindow(dpy, parent_widget->window,
			    x, y, width, height,
			    border, colors[borderColor].pixel,
			    colors[BLACK].pixel);
    XSelectInput(dpy, window, ExposureMask);
    widget_desc = Widget_create(WIDGET_INPUT_FLOAT, "input_float", window,
				width, height, floatw);
    if (widget_desc == NO_WIDGET) {
	return NO_WIDGET;
    }
    if (Widget_add_child(parent_desc, widget_desc) == NO_WIDGET) {
	Widget_destroy(widget_desc);
	return NO_WIDGET;
    }
    return widget_desc;
}

int Widget_create_label(int parent_desc,
			int x, int y,
			int width, int height,
			int border, char *str)
{
    int			widget_desc;
    Window		window;
    widget_t		*parent_widget;
    widget_label_t	*labelw;

    if ((parent_widget = Widget_pointer(parent_desc)) == NULL
	|| parent_widget->type != WIDGET_FORM) {
	errno = 0;
	error("Widget_create_label: Invalid parent widget");
	return NO_WIDGET;
    }
    if ((labelw = (widget_label_t *) malloc(sizeof(*labelw))) == NULL) {
	error("No memory for label widget");
	return NO_WIDGET;
    }
    labelw->str = str;
    labelw->x_offset = (width - XTextWidth(textFont, str, strlen(str))) / 2;
    labelw->y_offset = (height - (textFont->ascent + textFont->descent)) / 2;
    window =
	XCreateSimpleWindow(dpy, parent_widget->window,
			    x, y, width, height,
			    border, colors[borderColor].pixel,
			    colors[windowColor].pixel);
    XSelectInput(dpy, window, ExposureMask);
    widget_desc = Widget_create(WIDGET_LABEL, "label", window,
				width, height, labelw);
    if (widget_desc == NO_WIDGET) {
	return NO_WIDGET;
    }
    if (Widget_add_child(parent_desc, widget_desc) == NO_WIDGET) {
	Widget_destroy(widget_desc);
	return NO_WIDGET;
    }
    return widget_desc;
}

static int Widget_create_arrow(widget_type_t type, int parent_desc,
			       int x, int y,
			       int width, int height,
			       int border, int related_desc)
{
    int			widget_desc;
    Window		window;
    widget_t		*parent_widget;
    widget_arrow_t	*arroww;

    if ((parent_widget = Widget_pointer(parent_desc)) == NULL
	|| parent_widget->type != WIDGET_FORM) {
	errno = 0;
	error("Widget_create_arrow: Invalid parent widget");
	return NO_WIDGET;
    }
    if ((arroww = (widget_arrow_t *) malloc(sizeof(*arroww))) == NULL) {
	error("No memory for arrow widget");
	return NO_WIDGET;
    }
    arroww->pressed = false;
    arroww->inside = false;
    arroww->widget_desc = related_desc;
    window =
	XCreateSimpleWindow(dpy, parent_widget->window,
			    x, y, width, height,
			    border, colors[borderColor].pixel,
			    colors[BLACK].pixel);
    XSelectInput(dpy, window,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask
		 | OwnerGrabButtonMask | EnterWindowMask | LeaveWindowMask);
    widget_desc = Widget_create(type, "arrow", window,
				width, height, arroww);
    if (widget_desc == NO_WIDGET) {
	return NO_WIDGET;
    }
    if (Widget_add_child(parent_desc, widget_desc) == NO_WIDGET) {
	Widget_destroy(widget_desc);
	return NO_WIDGET;
    }
    return widget_desc;
}

int Widget_create_arrow_right(int parent_desc, int x, int y,
			      int width, int height,
			      int border, int related_desc)
{
    return Widget_create_arrow(WIDGET_BUTTON_ARROW_RIGHT, parent_desc, x, y,
			       width, height,
			       border, related_desc);
}

int Widget_create_arrow_left(int parent_desc, int x, int y,
			     int width, int height,
			     int border, int related_desc)
{
    return Widget_create_arrow(WIDGET_BUTTON_ARROW_LEFT, parent_desc, x, y,
			       width, height,
			       border, related_desc);
}

int Widget_create_popup(int width, int height, int border,
			char *window_name, char *icon_name)
{
    int				disp_width = DisplayWidth(dpy, 0),
				disp_height = DisplayHeight(dpy, 0),
				x, y,
				popup_desc;
    widget_t			*widget;
    Window			window;
    XSetWindowAttributes	sattr;
    unsigned long		mask;

    x = (disp_width > width) ? (disp_width - width) / 2 : 0;
    y = (disp_height > height) ? (disp_height - height) / 2 : 0;
    mask = 0;
    sattr.background_pixel = colors[windowColor].pixel;
    mask |= CWBackPixel;
    sattr.border_pixel = colors[borderColor].pixel;
    mask |= CWBorderPixel;
    sattr.bit_gravity = NorthWestGravity;
    mask |= CWBitGravity;
    sattr.event_mask = StructureNotifyMask;
    mask |= CWEventMask;
    if (colormap != 0) {
	sattr.colormap = colormap;
	mask |= CWColormap;
    }
    window = XCreateWindow(dpy,
			   DefaultRootWindow(dpy),
			   x, y,
			   width, height,
			   0, dispDepth,
			   InputOutput, visual,
			   mask, &sattr);
    popup_desc
	= Widget_form_window(window, NO_WIDGET,
			     width, height);
    if ((widget = Widget_pointer(popup_desc)) == NULL) {
	return NO_WIDGET;
    }
    widget->name = "popup";
    XStoreName(dpy, widget->window, window_name);
    XSetIconName(dpy, widget->window, icon_name);
    XSetTransientForHint(dpy, widget->window, top);
    return popup_desc;
}

int Widget_create_confirm(char *confirm_str,
			  int (*callback)(int, void *, char **))
{
    static char		button_str[] = "CLOSE";
    const int		label_height = textFont->ascent + textFont->descent,
			button_height = 3 * (buttonFont->ascent
				+ buttonFont->descent) / 2,
			label_width = 10 + XTextWidth(textFont, confirm_str,
						      strlen(confirm_str)),
			button_width = 2 * button_height
				+ XTextWidth(buttonFont, button_str,
					     strlen(button_str)),
			label_space = 2 * label_height,
			button_space = button_height,
			popup_height = 2 * label_space + label_height
				+ 2 * button_space + button_height,
			popup_width = MAX(label_width + 2*label_space,
					  button_width + 2*button_space);
    int			popup_desc,
			label_desc,
			button_desc;

    popup_desc = Widget_create_popup(popup_width, popup_height, 1,
				"Confirm", "Confirm");
    if (popup_desc == NO_WIDGET) {
	return NO_WIDGET;
    }
    label_desc = Widget_create_label(popup_desc,
				     (popup_width - label_width) / 2,
				     label_space,
				     label_width, label_height,
				     0, confirm_str);
    if (label_desc == NO_WIDGET) {
	Widget_destroy(popup_desc);
	return NO_WIDGET;
    }
    button_desc = Widget_create_activate(popup_desc,
					 (popup_width - button_width) / 2,
					 popup_height - button_height
						- button_space,
					 button_width, button_height,
					 0, button_str,
					 callback,
					 (void *)popup_desc);
    if (button_desc == NO_WIDGET) {
	Widget_destroy(popup_desc);
	Widget_destroy(label_desc);
	return NO_WIDGET;
    }
    Widget_map_sub(popup_desc);
    return popup_desc;
}

int Widget_backing_store(int widget_desc, int mode)
{
    XSetWindowAttributes	sattr;
    widget_t			*widget;

    if ((widget = Widget_pointer(widget_desc)) == NULL) {
	return NO_WIDGET;
    }
    sattr.backing_store = mode;
    XChangeWindowAttributes(dpy, widget->window, CWBackingStore, &sattr);
    return widget_desc;
}

int Widget_map_sub(int widget_desc)
{
    widget_t		*widget;

    if ((widget = Widget_pointer(widget_desc)) != NULL) {
	XMapSubwindows(dpy, widget->window);
    } else {
	errno = 0;
	error("Widget_map_sub: Invalid widget");
	return NO_WIDGET;
    }
    return widget_desc;
}

int Widget_map(int widget_desc)
{
    widget_t		*widget;

    if ((widget = Widget_pointer(widget_desc)) != NULL) {
	XMapWindow(dpy, widget->window);
    } else {
	errno = 0;
	error("Widget_map: Invalid widget %d", widget_desc);
	return NO_WIDGET;
    }
    return widget_desc;
}

int Widget_unmap(int widget_desc)
{
    widget_t		*widget;

    if ((widget = Widget_pointer(widget_desc)) != NULL) {
	XUnmapWindow(dpy, widget->window);
    } else {
	errno = 0;
	error("Widget_unmap: Invalid widget");
	return NO_WIDGET;
    }
    return widget_desc;
}

int Widget_raise(int widget_desc)
{
    widget_t		*widget;

    if ((widget = Widget_pointer(widget_desc)) != NULL) {
	XMapRaised(dpy, widget->window);
    } else {
	errno = 0;
	error("Widget_raise: Invalid widget");
	return NO_WIDGET;
    }
    return widget_desc;
}

static int Widget_create_slider(int parent_desc, widget_type_t slider_type,
				int x, int y, int width, int height,
				int border, int viewer_desc)
{
    int			widget_desc;
    Window		window;
    widget_t		*parent_widget;
    widget_slider_t	*sliderw;

    if ((parent_widget = Widget_pointer(parent_desc)) == NULL
	|| parent_widget->type != WIDGET_FORM) {
	errno = 0;
	error("Widget_create_slider: Invalid parent widget");
	return NO_WIDGET;
    }
    if ((sliderw = (widget_slider_t *) malloc(sizeof(*sliderw))) == NULL) {
	error("No memory for slider widget");
	return NO_WIDGET;
    }
    sliderw->pressed = false;
    sliderw->viewer_desc = viewer_desc;
    window =
	XCreateSimpleWindow(dpy, parent_widget->window,
			    x, y, width, height,
			    border, colors[borderColor].pixel,
			    colors[BLACK].pixel);
    XSelectInput(dpy, window,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask
		 | ButtonMotionMask | OwnerGrabButtonMask);
    widget_desc = Widget_create(slider_type, "slider", window,
				width, height, sliderw);
    if (widget_desc == NO_WIDGET) {
	return NO_WIDGET;
    }
    if (Widget_add_child(parent_desc, widget_desc) == NO_WIDGET) {
	Widget_destroy(widget_desc);
	return NO_WIDGET;
    }

    return widget_desc;
}

static int Widget_viewer_close_callback(int widget_desc, void *data,
					char **strptr)
{
    Widget_unmap((int) data);
    return 0;
}

static int Widget_viewer_calculate_view(int viewer_desc)
{
    widget_t		*widget = Widget_pointer(viewer_desc);
    widget_viewer_t	*viewerw = (widget_viewer_t *)widget->sub;
    int			new_x,
			new_y,
			new_width,
			new_height;

    new_width = 2 * 20 + viewerw->max_width;
    new_height = 2 * 20 + viewerw->num_lines
			* (viewerw->font->ascent + viewerw->font->descent);
    if (new_width < widget->width) {
	new_width = widget->width;
    }
    if (new_height < widget->height) {
	new_height = widget->height;
    }
    new_x = (viewerw->real_width <= widget->width)
	? 0
	: viewerw->visible_x * (new_width - widget->width)
				/ (viewerw->real_width - widget->width);
    if (new_x > 0 && -new_x + viewerw->real_width < widget->width) {
	new_x = viewerw->real_width - widget->width;
    }
    new_y = (viewerw->real_height <= widget->height)
	? 0
	: viewerw->visible_y * (new_height - widget->height)
				/ (viewerw->real_height - widget->height);
    if (new_y > 0 && -new_y + viewerw->real_height < widget->height) {
	new_y = viewerw->real_height - widget->height;
    }
    XResizeWindow(dpy, widget->window, new_width, new_height);
    XMoveWindow(dpy, widget->window, new_x, new_y);
    viewerw->visible_x = new_x;
    viewerw->visible_y = new_y;
    viewerw->real_width = new_width;
    viewerw->real_height = new_height;
    XFlush(dpy);

    return 0;
}

static int Widget_viewer_calculate_text(int viewer_desc)
{
    int			i,
			count;
    widget_t		*w = Widget_pointer(viewer_desc);
    widget_viewer_t	*viewerw = (widget_viewer_t *)w->sub;
    viewer_line_t	*line;

    if (viewerw->num_lines > 0 && viewerw->line != NULL) {
	free(viewerw->line);
    }
    viewerw->line = NULL;
    viewerw->num_lines = 0;
    viewerw->max_width = 0;

    for (i = count = 0; i < viewerw->len; i++) {
	if (viewerw->buf[i] == '\n') {
	    count++;
	}
    }
    if (viewerw->len > 0 && viewerw->buf[viewerw->len - 1] != '\n') {
	count++;
    }
    if (!count) {
	return 0;
    }
    viewerw->line = (viewer_line_t *) malloc(count * sizeof(viewer_line_t));
    if (!viewerw->line) {
	error("No mem for viewer text");
	return -1;
    }
    viewerw->num_lines = count;
    line = viewerw->line;
    for (count = i = 0; count < viewerw->num_lines; count++, i++) {
	line[count].txt = &viewerw->buf[i];
	while (viewerw->buf[i] != '\n') {
	    if (++i >= viewerw->len) {
		break;
	    }
	}
	line[count].len = &viewerw->buf[i] - line[count].txt;
	line[count].txt_width = XTextWidth(viewerw->font,
					   (char *) line[count].txt,
					   line[count].len);
	if (line[count].txt_width > viewerw->max_width) {
	    viewerw->max_width = line[count].txt_width;
	}
    }
    Widget_viewer_calculate_view(viewer_desc);

    return 0;
}

static void Widget_resize_viewer(XEvent *event, int ind)
{
    XConfigureEvent	*conf = &event->xconfigure;
    int			width = conf->width,
			height = conf->height;
    widget_t		*popup = Widget_pointer(ind);
    widget_form_t	*formw = (widget_form_t *)popup->sub;
    int			viewer_desc = formw->children[0];
    widget_t		*viewer_widget = Widget_pointer(viewer_desc);
    widget_viewer_t	*viewer_sub = (widget_viewer_t *)viewer_widget->sub;
    XFontStruct		*font = viewer_sub->font;
    const int
			close_width = 2*8 + XTextWidth(font, "CLOSE", 5),
			close_height = 6 + font->ascent + font->descent,
			close_x_offset = (width - close_width) / 2,
			close_y_offset = 5,
			close_spaced_height = close_height + 2*close_y_offset,
			hori_slider_height = 20,
			vert_slider_width = 20,
			hori_slider_width = width - vert_slider_width,
			vert_slider_height = height - close_spaced_height
				    - hori_slider_height,
			viewer_width = width - vert_slider_width,
			viewer_height = height - close_spaced_height,
			hori_slider_x = 0,
			hori_slider_y = height - hori_slider_height
						- close_spaced_height,
			vert_slider_x = width - vert_slider_width,
			vert_slider_y = 0;

    if (width == popup->width && height == popup->height) {
	return;
    }
    XResizeWindow(dpy, viewer_sub->overlay, viewer_width, viewer_height);
    Widget_resize(formw->children[1], hori_slider_width, hori_slider_height);
    XMoveWindow(dpy, Widget_pointer(formw->children[1])->window,
		hori_slider_x, hori_slider_y);
    Widget_resize(formw->children[2], vert_slider_width, vert_slider_height);
    XMoveWindow(dpy, Widget_pointer(formw->children[2])->window,
		vert_slider_x, vert_slider_y);
    Widget_resize(formw->children[3], close_width, close_height);
    XMoveWindow(dpy, Widget_pointer(formw->children[3])->window,
		close_x_offset, viewer_height + close_y_offset);
    popup->width = width;
    popup->height = height;
    viewer_widget->width = viewer_width;
    viewer_widget->height = viewer_height;
    viewer_sub->visible_x = 0;
    viewer_sub->visible_y = 0;
    Widget_viewer_calculate_view(viewer_desc);
}

int Widget_create_viewer(const char *buf, int len,
			 int width, int height, int border,
			 char *window_name, char *icon_name,
			 XFontStruct *font)
{
    const int
			close_width = 2*8 + XTextWidth(font, "CLOSE", 5),
			close_height = 6 + font->ascent + font->descent,
			close_x_offset = (width - close_width) / 2,
			close_y_offset = 5,
			close_spaced_height = close_height + 2*close_y_offset,
			hori_slider_height = 20,
			vert_slider_width = 20,
			hori_slider_width = width - vert_slider_width,
			vert_slider_height = height - close_spaced_height
				    - hori_slider_height,
			viewer_width = width - vert_slider_width,
			viewer_height = height - close_spaced_height,
			hori_slider_x = 0,
			hori_slider_y = height - hori_slider_height
						- close_spaced_height,
			vert_slider_x = width - vert_slider_width,
			vert_slider_y = 0;
    int			popup_desc,
			viewer_desc,
			hori_slider_desc,
			vert_slider_desc,
			close_button_desc;
    widget_t		*popup_widget;
    widget_viewer_t	*viewerw;
    Window		window;


    popup_desc = Widget_create_popup(width, height, border,
				     window_name, icon_name);
    if (popup_desc == NO_WIDGET
	|| (popup_widget = Widget_pointer(popup_desc)) == NULL
	|| popup_widget->type != WIDGET_FORM) {
	errno = 0;
	error("Widget_create_viewer: No popup");
	return NO_WIDGET;
    }
    popup_widget->name = "popup_viewer";

    if ((viewerw = (widget_viewer_t *) malloc(sizeof(*viewerw))) == NULL) {
	error("No mem for viewer");
	Widget_destroy(popup_desc);
	return NO_WIDGET;
    }
    viewerw->buf = buf;
    viewerw->len = len;
    viewerw->vert_slider_desc = NO_WIDGET;
    viewerw->hori_slider_desc = NO_WIDGET;
    viewerw->close_button_desc = NO_WIDGET;
    viewerw->visible_x = 0;
    viewerw->visible_y = 0;
    viewerw->real_width = width;
    viewerw->real_height = height;
    viewerw->max_width = 0;
    viewerw->num_lines = 0;
    viewerw->line = NULL;
    viewerw->font = font;
    viewerw->overlay =
	XCreateSimpleWindow(dpy, popup_widget->window,
			    0, 0, viewer_width, viewer_height,
			    0, colors[borderColor].pixel,
			    colors[windowColor].pixel);
    XSelectInput(dpy, viewerw->overlay, 0);
    Widget_bit_gravity(viewerw->overlay, NorthWestGravity);
    window =
	XCreateSimpleWindow(dpy, viewerw->overlay,
			    0, 0, viewer_width, viewer_height,
			    0, colors[borderColor].pixel,
			    colors[windowColor].pixel);
    XSelectInput(dpy, window, ExposureMask);
    Widget_bit_gravity(window, NorthWestGravity);
    Widget_window_gravity(window, NorthWestGravity);
    XMapWindow(dpy, window);
    viewer_desc = Widget_create(WIDGET_VIEWER, "viewer", window,
				viewer_width, viewer_height,
				viewerw);
    if (viewer_desc == NO_WIDGET) {
	Widget_destroy(popup_desc);
	return NO_WIDGET;
    }
    if (Widget_add_child(popup_desc, viewer_desc) == NO_WIDGET) {
	Widget_destroy(viewer_desc);
	Widget_destroy(popup_desc);
	return NO_WIDGET;
    }
    hori_slider_desc = Widget_create_slider(popup_desc,
					    WIDGET_SLIDER_HORI,
					    hori_slider_x,
					    hori_slider_y,
					    hori_slider_width,
					    hori_slider_height,
					    0,
					    viewer_desc);
    if (hori_slider_desc == NO_WIDGET) {
	Widget_destroy(popup_desc);
	return NO_WIDGET;
    }
    Widget_window_gravity(Widget_pointer(hori_slider_desc)->window,
			  SouthWestGravity);
    vert_slider_desc = Widget_create_slider(popup_desc,
					    WIDGET_SLIDER_VERT,
					    vert_slider_x,
					    vert_slider_y,
					    vert_slider_width,
					    vert_slider_height,
					    0,
					    viewer_desc);
    if (vert_slider_desc == NO_WIDGET) {
	Widget_destroy(popup_desc);
	return NO_WIDGET;
    }
    Widget_window_gravity(Widget_pointer(vert_slider_desc)->window,
			  NorthEastGravity);
    close_button_desc = Widget_create_activate(popup_desc,
					       close_x_offset,
					       viewer_height + close_y_offset,
					       close_width,
					       close_height,
					       0,
					       "CLOSE",
					       Widget_viewer_close_callback,
					       (void *)popup_desc);
    if (close_button_desc == NO_WIDGET) {
	Widget_destroy(popup_desc);
	return NO_WIDGET;
    }
    Widget_window_gravity(Widget_pointer(close_button_desc)->window,
			  SouthGravity);

    viewerw->vert_slider_desc = vert_slider_desc;
    viewerw->hori_slider_desc = hori_slider_desc;
    viewerw->close_button_desc = close_button_desc;

    Widget_map_sub(popup_desc);
    Widget_map(popup_desc);
    XFlush(dpy);

    Widget_viewer_calculate_text(viewer_desc);

    return popup_desc;
}

int Widget_update_viewer(int popup_desc, const char *buf, int len)
{
    widget_t		*popup = Widget_pointer(popup_desc);
    widget_form_t	*formw;
    int			viewer_desc;
    widget_t		*viewer_widget;
    widget_viewer_t	*viewer_sub;
    int			text_height,
			first_visible,
			last_visible,
			start,
			end;

    if (!popup || popup->type != WIDGET_FORM || !popup->name
	|| strcmp(popup->name, "popup_viewer")) {
	errno = 0;
	error("Widget_update_viewer: not a popup viewer");
	return -1;
    }
    formw = (widget_form_t *)popup->sub;
    viewer_desc = formw->children[0];
    viewer_widget = Widget_pointer(viewer_desc);
    viewer_sub = (widget_viewer_t *) viewer_widget->sub;
    viewer_sub->buf = buf;
    viewer_sub->len = len;
    text_height = viewer_sub->font->ascent + viewer_sub->font->descent;
    first_visible = (viewer_sub->visible_y - 20) / text_height;
    last_visible = (viewer_sub->visible_y + viewer_widget->height - 20)
		    / text_height;
    start = MAX(0, viewer_sub->num_lines - 1);
    Widget_viewer_calculate_text(viewer_desc);
    end = viewer_sub->num_lines;
    start = MAX(first_visible, start);
    end = MIN(last_visible + 1, end);
    if (start < end) {
	Widget_viewer_draw_lines(viewer_widget, start, end);
    }
    Widget_draw(formw->children[1]);
    Widget_draw(formw->children[2]);
    return 0;
}

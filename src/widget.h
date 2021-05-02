/* $Id: widget.h,v 3.13 1996/10/12 08:37:15 bert Exp $
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

#ifndef WIDGET_H
#define WIDGET_H

#define NO_WIDGET		0	/* Not a widget descriptor */

typedef enum widget_type {
    WIDGET_DUMMY,
    WIDGET_FORM,
    WIDGET_LABEL,
    WIDGET_BUTTON_BOOL,
    WIDGET_BUTTON_ACTIVATE,
    WIDGET_BUTTON_MENU,
    WIDGET_BUTTON_ENTRY,
    WIDGET_BUTTON_ARROW_LEFT,
    WIDGET_BUTTON_ARROW_RIGHT,
    WIDGET_INPUT_INT,
    WIDGET_INPUT_FLOAT,
    WIDGET_INPUT_STRING,
    WIDGET_VIEWER,
    WIDGET_SLIDER_HORI,
    WIDGET_SLIDER_VERT,
    NUM_WIDGET_TYPES
} widget_type_t;

typedef struct widget {
    widget_type_t		type;		/* Widget sub type */
    const char			*name;		/* Widget name */
    int				parent_desc;	/* Widget parent if non-zero */
    Window			window;		/* X drawing window */
    int				width,		/* Window width */
				height,		/* Window height */
				border;		/* Window border */
    void			*sub;		/* Widget sub info */
} widget_t;

typedef struct widget_form {
    int				*children;	/* Children widgets */
    int				num_children;	/* Number of children */
} widget_form_t;

typedef struct widget_label {
    const char			*str;		/* Label string */
    int				x_offset,	/* String horizontal offset */
				y_offset;	/* String vertical offset */
} widget_label_t;

typedef struct widget_bool {
    bool			pressed;	/* If button press active */
    bool			inside;		/* If pointer inside window */
    bool			state;		/* True or false */
    int				(*callback)(int, void *, bool *);
    void			*user_data;
} widget_bool_t;

typedef struct widget_menu {
    bool			pressed;	/* If button press active */
    const char			*str;		/* Label string */
    int				pulldown_desc;	/* Pulldown widget descriptor */
} widget_menu_t;

typedef struct widget_entry {
    bool			inside;		/* If pointer inside window */
    const char			*str;		/* Label string */
    int				(*callback)(int, void *, const char **);
    void			*user_data;
} widget_entry_t;

typedef struct widget_activate {
    bool			pressed;	/* If button press active */
    bool			inside;		/* If pointer inside window */
    const char			*str;		/* Label string */
    int				(*callback)(int, void *, const char **);
    void			*user_data;
} widget_activate_t;

typedef struct widget_arrow {
    bool			pressed;	/* pressed or not */
    bool			inside;		/* If pointer inside window */
    int				widget_desc;	/* Related input widget */
} widget_arrow_t;

typedef struct widget_int {
    int				*val,		/* Integer pointer */
				min,		/* Minimum value */
				max;		/* Maximum value */
    int				(*callback)(int, void *, int *);
    void			*user_data;
} widget_int_t;

typedef struct widget_float {
    float			*val,		/* Float pointer */
				min,		/* Minimum value */
				max;		/* Maximum value */
    int				(*callback)(int, void *, float *);
    void			*user_data;
} widget_float_t;

typedef struct widget_string {
    const char			*str;		/* Current input string */
} widget_string_t;

typedef struct view_line {
    const char			*txt;
    int				len;
    int				txt_width;
} viewer_line_t;

typedef struct widget_viewer {
    Window			overlay;
    const char			*buf;
    int				len,
				vert_slider_desc,
				hori_slider_desc,
				save_button_desc,
				close_button_desc,
				visible_x,
				visible_y,
				real_width,
				real_height,
				max_width,
				num_lines;
    viewer_line_t		*line;
    XFontStruct			*font;
} widget_viewer_t;

typedef struct widget_slider {
    bool			pressed;	/* pressed or not */
    bool			inside;		/* If pointer inside window */
    int				viewer_desc;
} widget_slider_t;

void Widget_destroy(int widget_desc);
Window Widget_window(int widget_desc);
void Widget_draw(int widget_desc);
int Widget_event(XEvent *event);
int Widget_create_form(int parent_desc, Window parent_window,
		       int x, int y, int width, int height,
		       int border);
int Widget_create_activate(int parent_desc,
			   int x, int y, int width, int height,
			   int border,
			   const char *str,
			   int (*callback)(int, void *, const char **),
			   void *user_data);
int Widget_create_bool(int parent_desc,
		       int x, int y, int width, int height,
		       int border,
		       bool val, int (*callback)(int, void *, bool *),
		       void *user_data);
int Widget_add_pulldown_entry(int menu_desc, const char *str,
			      int (*callback)(int, void *, const char **),
			      void *user_data);
int Widget_create_menu(int parent_desc,
		       int x, int y, int width, int height,
		       int border, const char *str);
int Widget_create_int(int parent_desc,
		      int x, int y, int width, int height,
		      int border, int *val, int min, int max,
		      int (*callback)(int, void *, int *),
		      void *user_data);
int Widget_create_float(int parent_desc,
			int x, int y, int width, int height,
			int border, float *val, float min, float max,
			int (*callback)(int, void *, float *),
			void *user_data);
int Widget_create_label(int parent_desc,
			int x, int y,
			int width, int height,
			int border, const char *str);
int Widget_create_arrow_right(int parent_desc, int x, int y,
			      int width, int height,
			      int border,
			      int related_desc);
int Widget_create_arrow_left(int parent_desc, int x, int y,
			     int width, int height,
			     int border, int related_desc);
int Widget_create_popup(int width, int height, int border,
			const char *window_name, const char *icon_name);
int Widget_create_confirm(const char *confirm_str,
			  int (*callback)(int, void *, const char **));
int Widget_map_sub(int widget_desc);
int Widget_map(int widget_desc);
int Widget_raise(int widget_desc);
int Widget_unmap(int widget_desc);
int Widget_resize(int widget_desc, int width, int height);
int Widget_create_viewer(const char *buf, int len,
			 int width, int height, int border,
			 const char *window_name, const char *icon_name,
			 XFontStruct *font);
int Widget_update_viewer(int popup_desc, const char *buf, int len);

#endif

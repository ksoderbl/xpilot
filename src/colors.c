/* $Id: colors.c,v 3.2 1996/10/13 19:30:45 bert Exp $
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
#include <X11/Xos.h>
#include <X11/Xutil.h>

#include "version.h"
#include "config.h"
#include "const.h"
#include "types.h"
#include "paint.h"
#include "xinit.h"
#include "error.h"
#include "dbuff.h"
#include "protoclient.h"

char colors_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: colors.c,v 3.2 1996/10/13 19:30:45 bert Exp $";
#endif

/* Kludge for visuals under C++ */
#if defined(__cplusplus)
#define class c_class
#endif

#define MAX_VISUAL_CLASS	6

char			visualName[MAX_VISUAL_NAME];
Visual			*visual;
int			dispDepth;
bool			mono;
bool			colorSwitch;
bool			multibuffer;

/*
 * Visual names.
 */
static struct visual_class_name {
    int		visual_class;
    const char	*visual_name;
} visual_class_names[MAX_VISUAL_CLASS] = {
    { StaticGray,	"StaticGray"  },
    { GrayScale,	"GrayScale"   },
    { StaticColor,	"StaticColor" },
    { PseudoColor,	"PseudoColor" },
    { TrueColor,	"TrueColor"   },
    { DirectColor,	"DirectColor" }
};

/*
 * Default colors.
 */
char			color_names[MAX_COLORS][MAX_COLOR_LEN];
static const char	*color_defaults[MAX_COLORS] = {
    "#000000", "#FFFFFF", "#4E7CFF", "#FF3A27",
    "#33BB44", "#992200", "#BB7700", "#EE9900",
    "#770000", "#CC4400", "#DD8800", "#FFBB11",
    "#9f9f9f", "#5f5f5f", "#dfdfdf", "#202020"
};
static const char	*gray_defaults[MAX_COLORS] = {
    "#000000", "#FFFFFF", "#AAAAAA", "#CCCCCC",
    "#BBBBBB", "#888888", "#AAAAAA", "#CCCCCC",
    "#777777", "#999999", "#BBBBBB", "#DDDDDD",
    "#9f9f9f", "#5f5f5f", "#dfdfdf", "#202020"
};


/*
 * Parse the user configurable color definitions.
 */
static int Parse_colors(Colormap cmap)
{
    int			i;
    const char		**def;

    /*
     * Get the color definitions.
     */
    if (mono == true) {
	colors[0].red = colors[0].green = colors[0].blue = 0;
	colors[0].flags = DoRed | DoGreen | DoBlue;
	colors[1].red = colors[1].green = colors[1].blue = 0xFFFF;
	colors[1].flags = DoRed | DoGreen | DoBlue;
	colors[3] = colors[2] = colors[1];
	return 0;
    }
    if (visual->class == StaticGray || visual->class == GrayScale) {
	def = &gray_defaults[0];
    } else {
	def = &color_defaults[0];
    }
    for (i = 0; i < maxColors; i++) {
	if (color_names[i][0] != '\0') {
	    if (XParseColor(dpy, cmap, color_names[i], &colors[i])) {
		continue;
	    }
	    printf("Can't parse color %d \"%s\"\n", i, color_names[i]);
	}
	if (def[i] != NULL && def[i][0] != '\0') {
	    if (XParseColor(dpy, cmap, def[i], &colors[i])) {
		continue;
	    }
	    printf("Can't parse default color %d \"%s\"\n", i, def[i]);
	}
	if (i < NUM_COLORS) {
	    return -1;
	} else {
	    colors[i] = colors[i % NUM_COLORS];
	}
    }
    return 0;
}


/*
 * If we have a private colormap and color switching is on then
 * copy the first few colors from the default colormap into it
 * to prevent ugly color effects on the rest of the screen.
 */
static void Fill_colormap(void)
{
    int			i,
			cells_needed,
			max_fill;
    unsigned long	pixels[256];
    XColor		mycolors[256];

    if (colormap == 0 || colorSwitch != true) {
	return;
    }
    cells_needed = (maxColors == 16) ? 256
	: (maxColors == 8) ? 64
	: 16;
    max_fill = MAX(256, visual->map_entries) - cells_needed;
    if (max_fill <= 0) {
	return;
    }
    if (XAllocColorCells(dpy, colormap, False, NULL, 0, pixels, max_fill)
	== False) {
	errno = 0;
	error("Can't pre-alloc color cells");
	return;
    }
    /* Check for misunderstanding of X colormap stuff. */
    for (i = 0; i < max_fill; i++) {
	if (pixels[i] != i) {
#ifdef DEVELOPMENT
	    errno = 0;
	    error("Can't pre-fill color map, got %d'th pixel %lu",
		  i, pixels[i]);
#endif
	    XFreeColors(dpy, colormap, pixels, max_fill, 0);
	    return;
	}
    }
    for (i = 0; i < max_fill; i++) {
	mycolors[i].pixel = pixels[i];
    }
    XQueryColors(dpy, DefaultColormap(dpy, DefaultScreen(dpy)),
		 mycolors, max_fill);
    XStoreColors(dpy, colormap, mycolors, max_fill);
}


/*
 * Create a private colormap.
 */
static void Get_colormap(void)
{
    printf("Creating a private colormap\n");
    colormap = XCreateColormap(dpy, DefaultRootWindow(dpy),
			       visual, AllocNone);
}


/*
 * Convert a visual class to its name.
 */
static const char *Visual_class_name(int visual_class)
{
    int			i;

    for (i = 0; i < MAX_VISUAL_CLASS; i++) {
	if (visual_class_names[i].visual_class == visual_class) {
	    return visual_class_names[i].visual_name;
	}
    }
    return "UnknownVisual";
}


/*
 * List the available visuals for the default screen.
 */
void List_visuals(void)
{
    int				i,
				num;
    XVisualInfo			*vinfo_ptr,
				my_vinfo;
    long			mask;

    num = 0;
    mask = 0;
    my_vinfo.screen = DefaultScreen(dpy);
    mask |= VisualScreenMask;
    vinfo_ptr = XGetVisualInfo(dpy, mask, &my_vinfo, &num);
    printf("Listing all visuals:\n");
    for (i = 0; i < num; i++) {
	printf("Visual class    %12s\n",
	       Visual_class_name(vinfo_ptr[i].class));
	printf("    id              0x%02x\n", (unsigned)vinfo_ptr[i].visualid);
	printf("    screen          %8d\n", vinfo_ptr[i].screen);
	printf("    depth           %8d\n", vinfo_ptr[i].depth);
	printf("    red_mask        0x%06x\n", (unsigned)vinfo_ptr[i].red_mask);
	printf("    green_mask      0x%06x\n", (unsigned)vinfo_ptr[i].green_mask);
	printf("    blue_mask       0x%06x\n", (unsigned)vinfo_ptr[i].blue_mask);
	printf("    colormap_size   %8d\n", vinfo_ptr[i].colormap_size);
	printf("    bits_per_rgb    %8d\n", vinfo_ptr[i].bits_per_rgb);
    }
    XFree((void *) vinfo_ptr);
}


/*
 * Support all available visuals.
 */
static void Choose_visual(void)
{
    int				i,
				num,
				best_size,
				cmap_size,
				visual_id,
				visual_class;
    XVisualInfo			*vinfo_ptr,
				my_vinfo,
				*best_vinfo;
    long			mask;

    visual_id = -1;
    visual_class = -1;
    if (visualName[0] != '\0') {
	if (strncmp(visualName, "0x", 2) == 0) {
	    if (sscanf(visualName, "%x", &visual_id) < 1) {
		errno = 0;
		error("Bad visual id \"%s\", using default\n", visualName);
		visual_id = -1;
	    }
	} else {
	    for (i = 0; i < MAX_VISUAL_CLASS; i++) {
		if (strncasecmp(visualName, visual_class_names[i].visual_name,
				strlen(visual_class_names[i].visual_name))
				== 0) {
		    visual_class = visual_class_names[i].visual_class;
		    break;
		}
	    }
	    if (visual_class == -1) {
		errno = 0;
		error("Unknown visual class named \"%s\", using default\n",
		    visualName);
	    }
	}
    }
    if (visual_class < 0 && visual_id < 0) {
	visual = DefaultVisual(dpy, DefaultScreen(dpy));
	if (visual->class == TrueColor || visual->class == DirectColor) {
	    visual_class = PseudoColor;
	    strcpy(visualName, "PseudoColor");
	}
    }
    if (visual_class >= 0 || visual_id >= 0) {
	mask = 0;
	my_vinfo.screen = DefaultScreen(dpy);
	mask |= VisualScreenMask;
	if (visual_class >= 0) {
	    my_vinfo.class = visual_class;
	    mask |= VisualClassMask;
	}
	if (visual_id >= 0) {
	    my_vinfo.visualid = visual_id;
	    mask |= VisualIDMask;
	}
	num = 0;
	if ((vinfo_ptr = XGetVisualInfo(dpy, mask, &my_vinfo, &num)) == NULL
	    || num <= 0) {
	    errno = 0;
	    error("No visuals available with class name \"%s\", using default",
		visualName);
	    visual_class = -1;
	}
	else {
	    best_vinfo = vinfo_ptr;
	    for (i = 1; i < num; i++) {
		best_size = best_vinfo->colormap_size;
		cmap_size = vinfo_ptr[i].colormap_size;
		if (cmap_size > best_size) {
		    if (best_size < 256) {
			best_vinfo = &vinfo_ptr[i];
		    }
		}
		else if (cmap_size >= 256) {
		    best_vinfo = &vinfo_ptr[i];
		}
	    }
	    visual = best_vinfo->visual;
	    visual_class = best_vinfo->class;
	    dispDepth = best_vinfo->depth;
	    XFree((void *) vinfo_ptr);
	    printf("Using visual %s with depth %d and %d colors\n",
		   Visual_class_name(visual->class), dispDepth,
		   visual->map_entries);
	    Get_colormap();
	}
    }
    if (visual_class < 0) {
	visual = DefaultVisual(dpy, DefaultScreen(dpy));
	dispDepth = DefaultDepth(dpy, DefaultScreen(dpy));
	colormap = 0;
    }
}


/*
 * Setup color and double buffering resources.
 * It returns 0 if the initialization was successful,
 * or -1 if it couldn't initialize the double buffering routine.
 */
int Colors_init(void)
{
    int				i, p, num_planes;

    colormap = 0;

    Choose_visual();

    /*
     * Get misc. display info.
     */
    if (visual->class == StaticGray ||
	visual->class == StaticColor ||
	visual->class == TrueColor) {
	colorSwitch = false;
    }
    if (colorSwitch == true) {
	maxColors = (maxColors >= 16 && visual->map_entries >= 256) ? 16
	    : (maxColors >= 8 && visual->map_entries >= 64) ? 8
	    : 4;
	if (visual->map_entries < 16) {
	    colorSwitch = false;
	}
    } else {
	maxColors = (maxColors >= 16 && visual->map_entries >= 16) ? 16
	    : (maxColors >= 8 && visual->map_entries >= 8) ? 8
	    : 4;
    }
    if (visual->map_entries < 4) {
	mono = true;
    }
    if (mono == true) {
	/* Color switching doesn't work for mono, needs at least 4 planes. */
	colorSwitch = false;
	maxColors = 4;
    }
    num_planes = (mono == true) ? 1
	: (maxColors == 16) ? 4
	: (maxColors == 8) ? 3
	: 2;

    if (Parse_colors(DefaultColormap(dpy, DefaultScreen(dpy))) == -1) {
	printf("Color parsing failed\n");
	return -1;
    }

    if (colormap != 0) {
	Fill_colormap();
    }

    /*
     * Initialize the double buffering routine.
     */
    dbuf_state = NULL;
    if (multibuffer) {
	dbuf_state = start_dbuff(dpy,
				 (colormap != 0)
				     ? colormap
				     : DefaultColormap(dpy,
						       DefaultScreen(dpy)),
				 MULTIBUFFER,
				 num_planes,
				 colors);
    }
    if (dbuf_state == NULL) {
	multibuffer = false;
	dbuf_state = start_dbuff(dpy,
				 (colormap != 0)
				     ? colormap
				     : DefaultColormap(dpy,
						       DefaultScreen(dpy)),
				 ((colorSwitch) ? COLOR_SWITCH : PIXMAP_COPY),
				 num_planes,
				 colors);
    }
    if (dbuf_state == NULL && colormap == 0) {

	/*
	 * Create a private colormap if we can't allocate enough colors.
	 */
	Get_colormap();
	Fill_colormap();

	/*
	 * Try to initialize the double buffering again.
	 */
	dbuf_state = start_dbuff(dpy, colormap,
				 ((colorSwitch) ? COLOR_SWITCH : PIXMAP_COPY),
				 num_planes,
				 colors);
    }

    if (dbuf_state == NULL) {
	/* Can't setup double buffering */
	errno = 0;
	error("Can't setup colors with visual %s and %d colormap entries",
	      Visual_class_name(visual->class), visual->map_entries);
	return -1;
    }

    for (i = maxColors; i < MAX_COLORS; i++) {
	colors[i] = colors[i % maxColors];
    }

    /*
     * A little hack that enables us to draw on both sets of double buffering
     * planes at once.
     */
    for (p=0; p<2; p++) {
	dpl_1[p] = dpl_2[p] = 0;

	for (i=0; i<32; i++)
	    if (!((1<<i)&dbuf_state->masks[p]))
		if (dpl_1[p])
		    dpl_2[p] |= 1<<i;
		else
		    dpl_1[p] |= 1<<i;
    }

    return 0;
}

void Colors_cleanup(void)
{
    if (dbuf_state) {
	end_dbuff(dbuf_state);
	dbuf_state = NULL;
    }
    if (colormap) {
	XFreeColormap(dpy, colormap);
	colormap = 0;
    }
}


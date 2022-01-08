/* $Id: colors.c,v 5.1 2001/05/08 11:35:29 bertg Exp $
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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#ifndef _WINDOWS
# include <unistd.h>
# include <X11/Xlib.h>
# include <X11/Xos.h>
# include <X11/Xutil.h>
#endif

#ifdef _WINDOWS
# include "NT/winX.h"
#endif

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


/* Kludge for visuals under C++ */
#if defined(__cplusplus)
#define class c_class
#endif


/*
 * The number of X11 visuals.
 */
#define MAX_VISUAL_CLASS	6


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

char		visualName[MAX_VISUAL_NAME];
Visual		*visual;
int		dispDepth;
bool		mono;
bool		colorSwitch;
bool		multibuffer;
bool		blockBitmaps;		/* Whether to draw everything as bitmaps. */

#ifndef _WINDOWS

/*
 * Dimensions of color cubes in decreasing
 * total number of colors used.
 */
static struct rgb_cube_size {
    unsigned char	r, g, b;
} rgb_cube_sizes[] = {
    { 6, 6, 5 },	/* 180 */
    { 5, 6, 5 },	/* 150 */
    { 6, 6, 4 },	/* 144 */
    { 5, 5, 5 },	/* 125 */
    { 5, 6, 4 },	/* 120 */
    { 6, 6, 3 },	/* 108 */
    { 5, 5, 4 },	/* 100 */
    { 5, 6, 3 },	/* 90 */
    { 4, 5, 4 },	/* 80 */
    { 5, 5, 3 },	/* 75 */
    { 4, 4, 4 },	/* 64 */
    { 4, 5, 3 },	/* 60 */
    { 4, 4, 3 },	/* 48 */
};

unsigned long		(*RGB)(u_byte r, u_byte g, u_byte b);
static unsigned long	RGB_PC(u_byte r, u_byte g, u_byte b);
static unsigned long	RGB_TC(u_byte r, u_byte g, u_byte b);

/*
 * Visual names.
 */
static struct Visual_class_name {
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
 * Structure to hold pixel information 
 * for a color cube for PseudoColor visuals.
 */
struct Color_cube {
    int				reds;
    int				greens;
    int				blues;
    int				mustfree;
    unsigned long		pixels[256];
};
static struct Color_cube	*color_cube;

/*
 * Structure to hold pixel information
 * for a true color visual.
 */
struct True_color {
    unsigned long		red_bits[256];
    unsigned long		green_bits[256];
    unsigned long		blue_bits[256];
};
static struct True_color	*true_color;

static void Colors_init_radar_hack(void);
static int  Colors_init_color_cube(void);
static int  Colors_init_true_color(void);

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
	printf("    id                  0x%02x\n", (unsigned)vinfo_ptr[i].visualid);
	printf("    screen          %8d\n", vinfo_ptr[i].screen);
	printf("    depth           %8d\n", vinfo_ptr[i].depth);
	printf("    red_mask        0x%06x\n", (unsigned)vinfo_ptr[i].red_mask);
	printf("    green_mask      0x%06x\n", (unsigned)vinfo_ptr[i].green_mask);
	printf("    blue_mask       0x%06x\n", (unsigned)vinfo_ptr[i].blue_mask);
	printf("    colormap_size   %8d\n", vinfo_ptr[i].colormap_size);
	printf("    bits_per_rgb    %8d\n", vinfo_ptr[i].bits_per_rgb);
    }
    XFree((void *) vinfo_ptr);

#ifdef DEVELOPMENT
    dbuff_list(dpy);
#endif
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
				using_default,
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
	using_default = true;
    } else {
	using_default = false;
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
	    if (using_default == false) {
		errno = 0;
		error("No visuals available with class name \"%s\", using default",
		    visualName);
	    }
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
	colors[2] = colors[1];
	colors[3] = colors[1];
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

    if (XAllocColorCells(dpy, colormap,
			 False, NULL,
			 0, pixels, max_fill) == False) {
	errno = 0;
	error("Can't pre-alloc color cells");
	return;
    }

    /* Check for misunderstanding of X colormap stuff. */
    for (i = 0; i < max_fill; i++) {
	if (i != (int) pixels[i]) {
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
 * Setup color and double buffering resources.
 * It returns 0 if the initialization was successful,
 * or -1 if it couldn't initialize the double buffering routine.
 */
int Colors_init(void)
{
    int				i, num_planes;

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
    if (visual->map_entries < 16) {
	colorSwitch = false;
	if (visual->map_entries < 4) {
	    mono = true;
	}
    }
    if (mono == true) {
	colorSwitch = false;
	maxColors = 4;
    }
    else if (colorSwitch == true) {
	maxColors = (maxColors >= 16 && visual->map_entries >= 256) ? 16
	    : (maxColors >= 8 && visual->map_entries >= 64) ? 8
	    : 4;
    }
    else {
	maxColors = (maxColors >= 16 && visual->map_entries >= 16) ? 16
	    : (maxColors >= 8 && visual->map_entries >= 8) ? 8
	    : 4;
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

	if (multibuffer) {
	    dbuf_state = start_dbuff(dpy, colormap,
				     MULTIBUFFER,
				     num_planes,
				     colors);
	}

	if (dbuf_state == NULL) {
	    dbuf_state = start_dbuff(dpy, colormap,
				     ((colorSwitch) ? COLOR_SWITCH : PIXMAP_COPY),
				     num_planes,
				     colors);
	}
    }

    if (dbuf_state == NULL) {
	/* Can't setup double buffering */
	errno = 0;
	error("Can't setup colors with visual %s and %d colormap entries",
	      Visual_class_name(visual->class), visual->map_entries);
	return -1;
    }

    switch (dbuf_state->type) {
    case COLOR_SWITCH:
	printf("Using color switching\n");
	break;

    case PIXMAP_COPY:
	printf("Using pixmap copying\n");
	break;

    case MULTIBUFFER:
#ifdef	DBE
	printf("Using double-buffering\n");
	break;
#else
#ifdef	MBX
	printf("Using multi-buffering\n");
	break;
#endif
#endif

    default:
	printf("Unknown dbuf state %d\n", dbuf_state->type);
	exit(1);
    }

    for (i = maxColors; i < MAX_COLORS; i++) {
	colors[i] = colors[i % maxColors];
    }

    Colors_init_radar_hack();

    Colors_init_block_bitmaps();

    return 0;
}


/*
 * A little hack that enables us to draw on both sets of double buffering
 * planes at once.
 */
static void Colors_init_radar_hack(void)
{
    int				i, p;

    for (p = 0; p < 2; p++) {
	int num = 0;

	dpl_1[p] = dpl_2[p] = 0;

	for (i = 0; i < 32; i++) {
	    if (!((1 << i) & dbuf_state->drawing_plane_masks[p])) {
	        num++;
		if (num == 1 || num == 3) {
		    dpl_1[p] |= 1<<i;   /* planes with moving radar objects */
		}
		else {
		    dpl_2[p] |= 1<<i;   /* constant map part of radar */
		}
	    }
	}
    }
}


/*
 * Setup color structures for use with drawing bitmaps.
 *
 * on error return -1,
 * on success return 0.
 */
static int Colors_init_block_bitmap_colors(void)
{
    int r = -1;

    switch (visual->class) {
    case PseudoColor:
	r = Colors_init_color_cube();
	break;

    case DirectColor:
	/*
	 * I don't really understand the difference between
	 * DirectColor and TrueColor.  Let's test if we can
	 * consider DirectColor to be similar to TrueColor.
	 */
	/*FALLTHROUGH*/

    case StaticColor:
    case TrueColor:
	r = Colors_init_true_color();
	break;

    case GrayScale:
    case StaticGray:
	/*
	 * Haven't implemented implemented bitmaps for gray colors yet.
	 */
	/*FALLTHROUGH*/

    default:
	printf("blockBitmaps not implemented for visual \"%s\"\n",
		Visual_class_name(visual->class));
	blockBitmaps = false;
	break;
    }

    return r;
}


/*
 * See if we can use block bitmaps.
 * If we can then setup the colors
 * and allocate the bitmaps.
 *
 * on error return -1,
 * on success return 0.
 */
int Colors_init_block_bitmaps(void)
{
    if (dbuf_state->type == COLOR_SWITCH) {
	if (blockBitmaps) {
	    printf("Can't do blockBitmaps if colorSwitch\n");
	    blockBitmaps = false;
	}
    }
    if (blockBitmaps) {
	if (Colors_init_block_bitmap_colors() == -1) {
	    blockBitmaps = false;
	}
    }
    if (blockBitmaps) {
	if (Block_bitmaps_create() == -1) {
	    /*
	    ** not sure if this is possible after
	    ** blockbitmap colors have been created.
	    */
	    blockBitmaps = false;
	}
    }

    return (blockBitmaps == true) ? 0 : -1;
}


/*
 * Calculate a pixel from a RGB triplet for a PseudoColor visual.
 */
static unsigned long RGB_PC(u_byte r, u_byte g, u_byte b)
{
    int			i;

    r = (r * color_cube->reds) >> 8;
    g = (g * color_cube->greens) >> 8;
    b = (b * color_cube->blues) >> 8;
    i = (((r * color_cube->greens) + g) * color_cube->blues) + b;

    return color_cube->pixels[i];
}


/*
 * Calculate a pixel from a RGB triplet for a TrueColor visual.
 */
static unsigned long RGB_TC(u_byte r, u_byte g, u_byte b)
{
    unsigned long	pixel = 0;

    pixel |= true_color->red_bits[r];
    pixel |= true_color->green_bits[g];
    pixel |= true_color->blue_bits[b];

    return pixel;
}


/*
 * Fill a color cube.
 *
 * Simple implementation for now.
 * Make it more ambitious wrt. read-only cells later.
 *
 * Two ways to allocate colors for a RGB cube.
 * One is to use the outer edges and sides for colors.
 * Another is to divide the cube in r*g*b sub-cubes and
 * choose the color in the centre of each sub-cube.
 * The latter option looks better because it will most
 * likely result in better color matches with on average
 * less color distance.
 */
static void Fill_color_cube(int reds, int greens, int blues,
			    XColor colors[256])
{
    int			i, r, g, b;

    i = 0;
    for (r = 0; r < reds; r++) {
	for (g = 0; g < greens; g++) {
	    for (b = 0; b < blues; b++, i++) {
		colors[i].pixel = color_cube->pixels[i];
		colors[i].flags = DoRed | DoGreen | DoBlue;
		colors[i].red   = (((r * 256) + 128) / reds) * 0x101;
		colors[i].green = (((g * 256) + 128) / greens) * 0x101;
		colors[i].blue  = (((b * 256) + 128) / blues) * 0x101;
	    }
	}
    }

    color_cube->reds = reds;
    color_cube->greens = greens;
    color_cube->blues = blues;
}


/*
 * Allocate a color cube.
 *
 * Simple implementation for now.
 * Make it more ambitious wrt. read-only cells later.
 */
static int Colors_init_color_cube(void)
{
    int			i, n, r, g, b;
    XColor		colors[256];

    if (color_cube != NULL) {
	error("Already a cube!\n");
	exit(1);
    }

    color_cube = (struct Color_cube *) calloc(1, sizeof(struct Color_cube));
    if (!color_cube) {
	error("Could not allocate memory for a color cube");
	return -1;
    }
    for (i = 0; i < NELEM(rgb_cube_sizes); i++) {

	r = rgb_cube_sizes[i].r;
	g = rgb_cube_sizes[i].g;
	b = rgb_cube_sizes[i].b;
	n = r * g * b;

	if (XAllocColorCells(dpy,
			     (colormap != 0)
				 ? colormap
				 : DefaultColormap(dpy,
						   DefaultScreen(dpy)),
			     False, NULL, 0,
			     &color_cube->pixels[0],
			     n) == False) {
	    /*printf("Could not alloc %d colors for RGB cube\n", n);*/
	    continue;
	}

	printf("Got %d colors for a %d*%d*%d RGB cube\n",
		n, r, g, b);

	color_cube->mustfree = 1;

	Fill_color_cube(r, g, b, &colors[0]);

	XStoreColors(dpy,
		     (colormap != 0)
			 ? colormap
			 : DefaultColormap(dpy,
					   DefaultScreen(dpy)),
		     colors,
		     n);

	RGB = RGB_PC;

	return 0;
    }

    printf("Could not alloc colors for RGB cube\n");

    return -1;
}


/*
 * Free our color cube.
 */
static void Colors_free_color_cube(void)
{
    if (color_cube) {
	if (color_cube->mustfree) {
	    XFreeColors(dpy,
			(colormap != 0)
			    ? colormap
			    : DefaultColormap(dpy,
					      DefaultScreen(dpy)),
			&color_cube->pixels[0],
			color_cube->reds * color_cube->greens * color_cube->blues,
			0);
	    color_cube->mustfree = 0;
	}
	free(color_cube);
	color_cube = NULL;
	RGB = NULL;
    }
}


/*
 * Allocate and initialize a true color structure.
 */
static int Colors_init_true_color(void)
{
    int			i, j, r, g, b;

    if ((visual->red_mask == 0) ||
	(visual->green_mask == 0) ||
	(visual->blue_mask == 0) ||
	((visual->red_mask &
	  visual->green_mask &
	  visual->blue_mask) != 0)) {

	printf("Your visual \"%s\" has weird characteristics:\n",
		Visual_class_name(visual->class));
	printf("\tred mask 0x%06lx, green mask 0x%06lx, blue mask 0x%06lx,\n",
		visual->red_mask, visual->green_mask, visual->blue_mask);
	printf("\toverlap mask 0x%06lx\n",
		visual->red_mask & visual->green_mask & visual->blue_mask);
	return -1;
    }

    if (true_color) {
	error("Already a True_color!");
	exit(1);
    }

    true_color = (struct True_color *) calloc(1, sizeof(struct True_color));
    if (!true_color) {
	error("Could not allocate memory for a true color structure");
	return -1;
    }

    r = 7;
    g = 7;
    b = 7;
    for (i = 31; i >= 0; --i) {
	if ((visual->red_mask & (1UL << i)) != 0) {
	    if (r >= 0) {
		for (j = 0; j < 256; j++) {
		    if (j & (1 << r)) {
			true_color->red_bits[j] |= (1UL << i);
		    }
		}
		r--;
	    }
	}
	if ((visual->green_mask & (1UL << i)) != 0) {
	    if (g >= 0) {
		for (j = 0; j < 256; j++) {
		    if (j & (1 << g)) {
			true_color->green_bits[j] |= (1UL << i);
		    }
		}
		g--;
	    }
	}
	if ((visual->blue_mask & (1UL << i)) != 0) {
	    if (b >= 0) {
		for (j = 0; j < 256; j++) {
		    if (j & (1 << b)) {
			true_color->blue_bits[j] |= (1UL << i);
		    }
		}
		b--;
	    }
	}
    }

    RGB = RGB_TC;

    return 0;
}


/*
 * Free a true color structure.
 */
static void Colors_free_true_color(void)
{
    if (true_color) {
	free(true_color);
	true_color = NULL;
	RGB = NULL;
    }
}


/*
 * Deallocate everything related to colors.
 */
void Colors_free_block_bitmaps(void)
{
    Colors_free_color_cube();
    Colors_free_true_color();

    if (blockBitmaps) {

	blockBitmaps = false;
    }
}


/*
 * Deallocate everything related to colors.
 */
void Colors_cleanup(void)
{
    Colors_free_block_bitmaps();

    if (dbuf_state) {
	end_dbuff(dbuf_state);
	dbuf_state = NULL;
    }
    if (colormap) {
	XFreeColormap(dpy, colormap);
	colormap = 0;
    }
}


#ifdef DEVELOPMENT
void Colors_debug(void)
{
    int			i, n, r, g, b;
    XColor		colors[256];
    FILE		*fp = fopen("rgb", "w");

    if (!color_cube) {
	static struct Color_cube cc;
	color_cube = &cc;
	for (i = 0; i < 256; i++) {
	    cc.pixels[i] = i;
	}
    }

    for (i = 0; i < NELEM(rgb_cube_sizes); i++) {

	r = rgb_cube_sizes[i].r;
	g = rgb_cube_sizes[i].g;
	b = rgb_cube_sizes[i].b;
	n = r * g * b;

	Fill_color_cube(r, g, b, colors);

	fprintf(fp, "\n\n  RGB  %d %d %d\n\n", r, g, b);
	i = 0;
	for (r = 0; r < color_cube->reds; r++) {
	    for (g = 0; g < color_cube->greens; g++) {
		for (b = 0; b < color_cube->blues; b++, i++) {
		    fprintf(fp, "color %4d    %04X  %04X  %04X\n",
			    i, colors[i].red, colors[i].green, colors[i].blue);
		}
	    }
	}
	fprintf(fp, "\nblack %3lu\nwhite %3lu\nred   %3lu\ngreen %3lu\nblue  %3lu\n",
		RGB_PC(0, 0, 0),
		RGB_PC(255, 255, 255),
		RGB_PC(255, 0, 0),
		RGB_PC(0, 255, 0),
		RGB_PC(0, 0, 255));
    }

    fclose(fp);

    exit(1);
}
#endif	/* DEVELOPMENT */


#endif	/* _WINDOWS */

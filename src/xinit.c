/* $Id: xinit.c,v 3.93 1996/04/27 18:39:08 bert Exp $
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
#ifndef	__apollo
#    include <string.h>
#endif
#include <limits.h>
#include <errno.h>

#include "version.h"
#include "config.h"
#include "const.h"
#include "rules.h"
#include "icon.h"
#include "paint.h"
#include "xinit.h"
#include "bit.h"
#include "keys.h"
#include "setup.h"
#include "widget.h"
#include "configure.h"
#include "error.h"
#include "netclient.h"
#include "dbuff.h"

char xinit_version[] = VERSION;

/*
 * Item structures
 */
#include "items/itemRocketPack.xbm"
#include "items/itemCloakingDevice.xbm"
#include "items/itemEnergyPack.xbm"
#include "items/itemWideangleShot.xbm"
#include "items/itemRearShot.xbm"
#include "items/itemMinePack.xbm"
#include "items/itemSensorPack.xbm"
#include "items/itemTank.xbm"
#include "items/itemEcm.xbm"
#include "items/itemAfterburner.xbm"
#include "items/itemTransporter.xbm"
#include "items/itemLaser.xbm"
#include "items/itemEmergencyThrust.xbm"
#include "items/itemTractorBeam.xbm"
#include "items/itemAutopilot.xbm"
#include "items/itemEmergencyShield.xbm"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: xinit.c,v 3.93 1996/04/27 18:39:08 bert Exp $";
#endif

/* Kludge for visuals under C++ */
#if defined(__cplusplus)
#define class c_class
#endif

/* How far away objects should be placed from each other etc... */
#define BORDER			10
#define BTN_BORDER		4

/* Information window dimensions */
#define ABOUT_WINDOW_WIDTH	600
#define ABOUT_WINDOW_HEIGHT	700
#define TALK_TEXT_HEIGHT	(textFont->ascent + textFont->descent)
#define TALK_OUTSIDE_BORDER	2
#define TALK_INSIDE_BORDER	3
#define TALK_WINDOW_HEIGHT	(TALK_TEXT_HEIGHT + 2 * TALK_INSIDE_BORDER)
#define TALK_WINDOW_X		(50 - TALK_OUTSIDE_BORDER)
#define TALK_WINDOW_Y		(draw_height*3/4 - TALK_WINDOW_HEIGHT/2)
#define TALK_WINDOW_WIDTH	(draw_width \
				    - 2*(TALK_WINDOW_X + TALK_OUTSIDE_BORDER))

#define CTRL(c)			((c) & 0x1F)

#define MAX_VISUAL_CLASS	6

extern message_t	*TalkMsg[], *GameMsg[];
extern int		RadarHeight;

/*
 * Globals.
 */
int			ButtonHeight;
Atom			ProtocolAtom, KillAtom;
bool			talk_mapped;
int			buttonColor, windowColor, borderColor;
int			quitting = false;
char			visualName[MAX_VISUAL_NAME];
Visual			*visual;
int			dispDepth;
bool			mono;
bool			colorSwitch;
bool			multibuffer;
int			top_width, top_height, top_x, top_y, top_posmask;
int			draw_width, draw_height;
char			*geometry;
bool			autoServerMotdPopup;
Cursor			pointerControlCursor;
char			sparkColors[MSG_LEN];
int			spark_color[MAX_COLORS];
int			num_spark_colors;

static message_t	*MsgBlock;
static bool		about_created;
static bool		talk_created;
static char		talk_str[MAX_CHARS];
static struct {
    bool		visible;
    short		offset;
    short		point;
} talk_cursor;


/*
 * NB!  Is dependent on the order of the items in item.h!
 */
static struct {
    char*	data;
    char*	keysText;
} itemBitmapData[NUM_ITEMS] = {
    {
	itemEnergyPack_bits,
	"Extra energy/fuel"
    },
    {
	itemWideangleShot_bits,
	"Extra front cannons"
    },
    {
	itemRearShot_bits,
	"Extra rear cannon"
    },
    {
	itemAfterburner_bits,
	"Afterburner; makes your engines more powerful"
    },
    {
	itemCloakingDevice_bits,
	"Cloaking device; "
	"makes you almost invisible, both on radar and on screen"
    },
    {
	itemSensorPack_bits,
	"Sensor; "
	"enables you to see cloaked opponents more easily"
    },
    {
	itemTransporter_bits,
	"Transporter; enables you to steal equipment from "
	"other players"
    },
    {
	itemTank_bits,
	"Tank; "
	"makes refueling quicker, increases maximum fuel "
	"capacity and can be jettisoned to confuse enemies"
    },
    {
	itemMinePack_bits,
	"Mine; "
	"can be dropped as a bomb or as a stationary mine"
    },
    {
	itemRocketPack_bits,
	"Rocket; can be utilized as smart missile, "
	"heat seaking missile, nuclear missile or just a "
	"plain unguided missile (torpedo)"
    },
    {
	itemEcm_bits,
	"ECM (Electronic Counter Measures); "
	"can be used to disturb electronic equipment, for instance "
	"can it be used to confuse smart missiles and reprogram "
	"robots to seak certain players"
    },
    {
	itemLaser_bits,
	"Laser; "
	"limited range laser beam, costs a lot of fuel, "
	"having more laser items increases the range of the laser, "
	"they can be irrepairably damaged by ECMs"
    },
    {
	itemEmergencyThrust_bits,
	"Emergency Thrust; "
	"gives emergency thrust capabilities for a limited period"
    },
    {   itemTractorBeam_bits,
	"Tractor Beam; "
	"gives mutual attractive force to currently locked on ship, "
	"this means the heavier your ship, the less likely you will move "
	"when being tractored or using a tractor"
    },
    {
	itemAutopilot_bits,
	"Autopilot; "
	"when on, the ship will turn and thrust against the "
	"direction of travel"
    },
    {
	itemEmergencyShield_bits,
	"EmergencyShield; "
	"gives emergency shield capabilities for a limited period"
    },
};
Pixmap	itemBitmaps[NUM_ITEMS];		/* Bitmaps for the items */

char dashes[NUM_DASHES] = { 8, 4 };
char cdashes[NUM_CDASHES] = { 3, 9 };

static void createAboutWindow(void);
static void createTalkWindow(void);
static int Quit_callback(int, void *, char **);
static int About_callback(int, void *, char **);
static int Keys_callback(int, void *, char **);
static int Config_callback(int, void *, char **);
static int Score_callback(int, void *, char **);
static int Player_callback(int, void *, char **);
static int Motd_callback(int, void *, char **);

/*
 * Visual names.
 */
static struct visual_class_name {
    int		visual_class;
    char	*visual_name;
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
char		color_names[MAX_COLORS][MAX_COLOR_LEN];
static char	*color_defaults[MAX_COLORS] = {
    "#000000", "#FFFFFF", "#4E7CFF", "#FF3A27",
    "#33BB44", "#992200", "#BB7700", "#EE9900",
    "#770000", "#CC4400", "#DD8800", "#FFBB11",
    "#9f9f9f", "#5f5f5f", "#dfdfdf", "#202020"
};
static char	*gray_defaults[MAX_COLORS] = {
    "#000000", "#FFFFFF", "#AAAAAA", "#CCCCCC",
    "#BBBBBB", "#888888", "#AAAAAA", "#CCCCCC",
    "#777777", "#999999", "#BBBBBB", "#DDDDDD",
    "#9f9f9f", "#5f5f5f", "#dfdfdf", "#202020"
};


/*
 * Set specified font for that GC.
 * Return font that is used for this GC, even if setting a new
 * font failed (return default font in that case).
 */
XFontStruct* Set_font(Display* dpy, GC gc, char* fontName, char *resName)
{
    XFontStruct*	font;

    if ((font = XLoadQueryFont(dpy, fontName)) == NULL) {
	error("Couldn't find font '%s' for %s, using default font",
	      fontName, resName);
	font = XQueryFont(dpy, XGContextFromGC(gc));
    } else
	XSetFont(dpy, gc, font->fid);

    return font;
}


/*
 * Parse the user configurable color definitions.
 */
int Parse_colors(Colormap cmap)
{
    int			i;
    char		**def;

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
static char *Visual_class_name(int visual_class)
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
static void Get_visual_info(void)
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
 * Convert a string of color numbers into an array
 * of "colors[]" indices stored by "spark_color[]".
 * Initialize "num_spark_colors".
 */
static void Init_spark_colors(void)
{
    char		buf[MSG_LEN];
    char		*src, *dst;
    unsigned		col;
    int			i;

    num_spark_colors = 0;
    /*
     * The sparkColors specification may contain
     * any possible separator.  Only look at numbers.
     */
    for (src = sparkColors; *src; src++) {
	if (isascii(*src) && isdigit(*src)) {
	    dst = &buf[0];
	    do {
		*dst++ = *src++;
	    } while (*src && isascii(*src) && isdigit(*src));
	    *dst = '\0';
	    src--;
	    if (sscanf(buf, "%u", &col) == 1) {
		if (col < maxColors) {
		    spark_color[num_spark_colors++] = col;
		}
	    }
	}
    }
    if (num_spark_colors == 0) {
	if (maxColors <= 8) {
	    /* 3 colors ranging from 5 up to 7 */
	    for (i = 5; i < maxColors; i++) {
		spark_color[num_spark_colors++] = i;
	    }
	}
	else {
	    /* 7 colors ranging from 5 till 11 */
	    for (i = 5; i < 12; i++) {
		spark_color[num_spark_colors++] = i;
	    }
	}
	/* default spark colors always include RED. */
	spark_color[num_spark_colors++] = RED;
    }
    for (i = num_spark_colors; i < MAX_COLORS; i++) {
	spark_color[i] = spark_color[num_spark_colors - 1];
    }
}


/*
 * Initialize miscellaneous window hints and properties.
 */
void Init_disp_prop(Display *d, Window win, int w, int h, int x, int y,
		   int flags)
{
    extern char		**Argv;
    extern int		Argc;
    XClassHint		xclh;
    XWMHints		xwmh;
    XSizeHints		xsh;
    char		msg[256];

    xwmh.flags	   = InputHint|StateHint|IconPixmapHint;
    xwmh.input	   = True;
    xwmh.initial_state = NormalState;
    xwmh.icon_pixmap   = XCreateBitmapFromData(d, win,
					       (char *)icon_bits,
					       icon_width, icon_height);

    xsh.flags = (flags|PMinSize|PMaxSize|PBaseSize|PResizeInc);
    xsh.width = w;
    xsh.base_width =
    xsh.min_width = MIN_TOP_WIDTH;
    xsh.max_width = MAX_TOP_WIDTH;
    xsh.width_inc = 1;
    xsh.height = h;
    xsh.base_height =
    xsh.min_height = MIN_TOP_HEIGHT;
    xsh.max_height = MAX_TOP_HEIGHT;
    xsh.height_inc = 1;
    xsh.x = x;
    xsh.y = y;

    xclh.res_name = NULL;	/* NULL: Automatically uses Argv[0], */
    xclh.res_class = "XPilot"; /* stripped of directory prefixes. */

    /*
     * Set the above properties.
     */
    XSetWMProperties(d, win, NULL, NULL, Argv, Argc,
		     &xsh, &xwmh, &xclh);

    /*
     * Now initialize icon and window title name.
     */
    if (titleFlip)
	sprintf(msg, "Successful connection to server at \"%s\".",
		servername);
    else
	sprintf(msg, "%s -- Server at \"%s\".", TITLE, servername);
    XStoreName(d, win, msg);

    sprintf(msg, "%s:%s", name, servername);
    XSetIconName(d, win, msg);

    if (d != dpy)
	    return;

    /*
     * Specify IO error handler and the WM_DELETE_WINDOW atom in
     * an attempt to catch 'nasty' quits.
     */
    ProtocolAtom = XInternAtom(d, "WM_PROTOCOLS", False);
    KillAtom = XInternAtom(d, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(d, win, &KillAtom, 1);
    XSetIOErrorHandler(FatalError);
}

/*
 * The following function initializes a toplevel window.
 * It returns 0 if the initialization was successful,
 * or -1 if it couldn't initialize the double buffering routine.
 */
int Init_top(void)
{
    int				i, p, values,
				num_planes,
				top_flags,
				top_x, top_y,
				x, y;
    unsigned			w, h;
    XGCValues			xgc;
    XSetWindowAttributes	sattr;
    unsigned long		mask;

    colormap = 0;

    Get_visual_info();

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

    if (shieldDrawMode == -1) {
	shieldDrawMode = 0;
	/*
	 * Default is solid for NCD X11 servers.  My NCD mono 19 inch
	 * terminal, vendor release 2002 suffers from terrible slowness
	 * when drawing dashed arcs with thick lines.
	 */
	if (strcmp (ServerVendor (dpy),
		    "DECWINDOWS (Compatibility String) "
		    "Network Computing Devices Inc.") == 0
	    && ProtocolVersion (dpy) == 11)
	    shieldDrawMode = 1;

#ifdef ERASE
	/*
	 * The NeWS X server doesn't orrectly erase shields.
	 */
	if (!strcmp(ServerVendor(dpy), "X11/NeWS - Sun Microsystems Inc."))
	    shieldDrawMode = 1;
#endif
    }

    shieldDrawMode = shieldDrawMode ? LineSolid : LineOnOffDash;
    radarPlayerRectFN = (mono ? XDrawRectangle : XFillRectangle);

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
    if (hudColor >= maxColors || hudColor <= 0) {
	hudColor = BLUE;
    }
    if (hudLockColor >= maxColors || hudLockColor <= 0) {
	hudLockColor = hudColor;
    }
    if (wallColor >= maxColors || wallColor <= 0) {
	wallColor = BLUE;
    }
    if (wallRadarColor >= maxColors
	|| (wallRadarColor & 1)) {
	wallRadarColor = BLUE;
    }
    if (targetRadarColor >= maxColors
	|| (targetRadarColor & 1)) {
	targetRadarColor = BLUE;
    }
    if (decorColor >= maxColors || decorColor <= 0) {
	decorColor = RED;
    }
    if (decorRadarColor >= maxColors
	|| (decorRadarColor & 1)) {
	decorRadarColor = 2;
    }

    /*
     * Get toplevel geometry.
     */
    top_flags = 0;
    if (geometry != NULL && geometry[0] != '\0') {
	mask = XParseGeometry(geometry, &x, &y, &w, &h);
    } else {
	mask = 0;
    }
    if ((mask & WidthValue) != 0) {
	top_width = w;
	top_flags |= USSize;
    } else {
	top_width = DEF_TOP_WIDTH;
	top_flags |= PSize;
    }
    LIMIT(top_width, MIN_TOP_WIDTH, MAX_TOP_WIDTH);
    if ((mask & HeightValue) != 0) {
	top_height = h;
	top_flags |= USSize;
    } else {
	top_height = DEF_TOP_HEIGHT;
	top_flags |= PSize;
    }
    LIMIT(top_height, MIN_TOP_HEIGHT, MAX_TOP_HEIGHT);
    if ((mask & XValue) != 0) {
	if ((mask & XNegative) != 0) {
	    top_x = DisplayWidth(dpy, DefaultScreen(dpy)) - top_width + x;
	} else {
	    top_x = x;
	}
	top_flags |= USPosition;
    } else {
	top_x = (DisplayWidth(dpy, DefaultScreen(dpy)) - top_width) /2;
	top_flags |= PPosition;
    }
    if ((mask & YValue) != 0) {
	if ((mask & YNegative) != 0) {
	    top_y = DisplayHeight(dpy, DefaultScreen(dpy)) - top_height + y;
	} else {
	    top_y = y;
	}
	top_flags |= USPosition;
    } else {
	top_y = (DisplayHeight(dpy, DefaultScreen(dpy)) - top_height) /2;
	top_flags |= PPosition;
    }
    if (geometry != NULL) {
	free(geometry);
	geometry = NULL;
    }

    /*
     * Create toplevel window (we need this first so that we can create GCs)
     */
    mask = 0;
    sattr.background_pixel = colors[WHITE].pixel;
    mask |= CWBackPixel;
    sattr.border_pixel = colors[WHITE].pixel;
    mask |= CWBorderPixel;
    if (colormap != 0) {
	sattr.colormap = colormap;
	mask |= CWColormap;
    }
    top = XCreateWindow(dpy,
			DefaultRootWindow(dpy),
			top_x, top_y,
			top_width, top_height,
			0, dispDepth,
			InputOutput, visual,
			mask, &sattr);
    XSelectInput(dpy, top,
		 KeyPressMask | KeyReleaseMask
		 | FocusChangeMask | StructureNotifyMask);
    Init_disp_prop(dpy, top, top_width, top_height, top_x, top_y, top_flags);
    if (kdpy) {
	int scr = DefaultScreen(kdpy);
	keyboard = XCreateSimpleWindow(kdpy,
				       DefaultRootWindow(kdpy),
				       top_x, top_y,
				       top_width, top_height,
				       0, 0, BlackPixel(dpy, scr));
	XSelectInput(kdpy, keyboard,
		     KeyPressMask | KeyReleaseMask | FocusChangeMask);
	Init_disp_prop(kdpy, keyboard, top_width, top_height,
		       top_x, top_y, top_flags);
    }

    /*
     * Create item bitmaps
     */
    for (i=0; i<NUM_ITEMS; i++)
	itemBitmaps[i]
	    = XCreateBitmapFromData(dpy, top,
				    (char *)itemBitmapData[i].data,
				    ITEM_SIZE, ITEM_SIZE);

    /*
     * Creates and initializes the graphic contexts.
     */
    xgc.line_width = 0;
    xgc.line_style = LineSolid;
    xgc.cap_style = CapButt;
    xgc.join_style = JoinMiter;		/* I think this is fastest, is it? */
    xgc.graphics_exposures = False;
    values
	= GCLineWidth|GCLineStyle|GCCapStyle|GCJoinStyle|GCGraphicsExposures;

    messageGC
	= XCreateGC(dpy, top, values, &xgc);
    radarGC
	= XCreateGC(dpy, top, values, &xgc);
    buttonGC
	= XCreateGC(dpy, top, values, &xgc);
    scoreListGC
	= XCreateGC(dpy, top, values, &xgc);
    textGC
	= XCreateGC(dpy, top, values, &xgc);
    talkGC
	= XCreateGC(dpy, top, values, &xgc);
    motdGC
	= XCreateGC(dpy, top, values, &xgc);
    gc
	= XCreateGC(dpy, top, values, &xgc);
    XSetBackground(dpy, gc, colors[BLACK].pixel);
    XSetDashes(dpy, gc, 0, dashes, NUM_DASHES);

    /*
     * Set fonts
     */
    gameFont
	= Set_font(dpy, gc, gameFontName, "gameFont");
    messageFont
	= Set_font(dpy, messageGC, messageFontName, "messageFont");
    scoreListFont
	= Set_font(dpy, scoreListGC, scoreListFontName, "scoreListFont");
    buttonFont
	= Set_font(dpy, buttonGC, buttonFontName, "buttonFont");
    textFont
	= Set_font(dpy, textGC, textFontName, "textFont");
    talkFont
	= Set_font(dpy, talkGC, talkFontName, "talkFont");
    motdFont
	= Set_font(dpy, motdGC, motdFontName, "motdFont");

    XSetState(dpy, gc,
	      WhitePixel(dpy, DefaultScreen(dpy)),
	      BlackPixel(dpy, DefaultScreen(dpy)),
	      GXcopy, AllPlanes);
    XSetState(dpy, radarGC,
	      WhitePixel(dpy, DefaultScreen(dpy)),
	      BlackPixel(dpy, DefaultScreen(dpy)),
	      GXcopy, AllPlanes);
    XSetState(dpy, messageGC,
	      WhitePixel(dpy, DefaultScreen(dpy)),
	      BlackPixel(dpy, DefaultScreen(dpy)),
	      GXcopy, AllPlanes);
    XSetState(dpy, buttonGC,
	      WhitePixel(dpy, DefaultScreen(dpy)),
	      BlackPixel(dpy, DefaultScreen(dpy)),
	      GXcopy, AllPlanes);
    XSetState(dpy, scoreListGC,
	      WhitePixel(dpy, DefaultScreen(dpy)),
	      BlackPixel(dpy, DefaultScreen(dpy)),
	      GXcopy, AllPlanes);

    if (dbuf_state->type == COLOR_SWITCH)
	XSetPlaneMask(dpy, gc, dbuf_state->drawing_planes);

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

    if (mono) {
	buttonColor = BLACK;
	windowColor = BLACK;
	borderColor = WHITE;
    } else {
	windowColor = BLUE;
	buttonColor = RED;
	borderColor = WHITE;
    }

    return 0;
}

/*
 * Creates the playing windows.
 * Returns 0 on success, -1 on error.
 */
int Init_windows(void)
{
    unsigned			w, h;
    int				button_form,
				menu_button;
    Pixmap			pix;
    GC				cursorGC;

    if (!top) {
	if (Init_top()) {
	    return -1;
	}
    }

    draw_width = top_width - (256 + 2);
    draw_height = top_height;
    draw = XCreateSimpleWindow(dpy, top, 258, 0,
			       draw_width, draw_height,
			       0, 0, colors[BLACK].pixel);
    radar = XCreateSimpleWindow(dpy, top, 0, 0,
				256, RadarHeight, 0, 0,
				colors[BLACK].pixel);

    /* Create buttons */
#define BUTTON_WIDTH	84
    ButtonHeight
	= buttonFont->ascent + buttonFont->descent + 2*BTN_BORDER;

    button_form
	= Widget_create_form(0, top,
			     0, RadarHeight,
			     256, ButtonHeight + 2,
			     0);
    Widget_create_activate(button_form,
			   0 + 0*BUTTON_WIDTH, 0,
			   BUTTON_WIDTH, ButtonHeight,
			   1, "QUIT",
			   Quit_callback, NULL);
    Widget_create_activate(button_form,
			   1 + 1*BUTTON_WIDTH, 0,
			   BUTTON_WIDTH, ButtonHeight,
			   1, "ABOUT",
			   About_callback, NULL);
    menu_button
	= Widget_create_menu(button_form,
			     2 + 2*BUTTON_WIDTH, 0,
			     BUTTON_WIDTH, ButtonHeight,
			     1, "MENU");
    Widget_add_pulldown_entry(menu_button,
			      "KEYS", Keys_callback, NULL);
    Widget_add_pulldown_entry(menu_button,
			      "CONFIG", Config_callback, NULL);
    Widget_add_pulldown_entry(menu_button,
			      "SCORE", Score_callback, NULL);
    Widget_add_pulldown_entry(menu_button,
			      "PLAYER", Player_callback, NULL);
    Widget_add_pulldown_entry(menu_button,
			      "MOTD", Motd_callback, NULL);
    Widget_map_sub(button_form);

    /* Create score list window */
    players
	= XCreateSimpleWindow(dpy, top, 0, RadarHeight + ButtonHeight + 2,
			      256, top_height
				  - (RadarHeight + ButtonHeight + 2),
			      0, 0,
			      colors[windowColor].pixel);

    /*
     * Selecting the events we can handle.
     */
    XSelectInput(dpy, radar, ExposureMask);
    XSelectInput(dpy, players, ExposureMask);
    XSelectInput(dpy, draw, 0);


    /*
     * Initialize misc. pixmaps if we're not color switching.
     * (This could be in dbuff_init completely IMHO, -- Metalite)
     */
    switch (dbuf_state->type) {

    case PIXMAP_COPY:
	p_radar = XCreatePixmap(dpy, radar, 256, RadarHeight, dispDepth);
	s_radar = XCreatePixmap(dpy, radar, 256, RadarHeight, dispDepth);
	p_draw  = XCreatePixmap(dpy, draw, draw_width, draw_height, dispDepth);
	break;

    case MULTIBUFFER:
	p_radar = XCreatePixmap(dpy, radar, 256, RadarHeight, dispDepth);
	s_radar = XCreatePixmap(dpy, radar, 256, RadarHeight, dispDepth);
	dbuff_init(dbuf_state);
	break;

    case COLOR_SWITCH:
	s_radar = radar;
	p_radar = s_radar;
	p_draw = draw;
	Paint_sliding_radar();
	break;
    }

    XAutoRepeatOff(dpy);	/* We don't want any autofire, yet! */
    if (kdpy)
	XAutoRepeatOff(kdpy);

    /*
     * Define a blank cursor for use with pointer control
     */
    XQueryBestCursor(dpy, draw, 1, 1, &w, &h);
    pix = XCreatePixmap(dpy, draw, w, h, 1);
    cursorGC = XCreateGC(dpy, pix, 0, NULL);
    XSetForeground(dpy, gc, 0);
    XFillRectangle(dpy, pix, cursorGC, 0, 0, w, h);
    XFreeGC(dpy, cursorGC);
    pointerControlCursor = XCreatePixmapCursor(dpy, pix, pix, &colors[BLACK],
					       &colors[BLACK], 0, 0);
    XFreePixmap(dpy, pix);

    /*
     * Maps the windows, makes the visible. Voila!
     */
    XMapSubwindows(dpy, top);
    XMapWindow(dpy, top);
    XSync(dpy, False);

    if (kdpy) {
	XMapWindow(kdpy, keyboard);
	XSync(kdpy, False);
    }

    Init_spark_colors();

    about_created = false;
    talk_created = false;
    talk_mapped = false;
    talk_str[0] = '\0';
    talk_cursor.point = 0;

    return 0;
}


int Alloc_msgs(void)
{
    message_t		*x;
    int			i;

    if ((x = (message_t *)malloc(2 * MAX_MSGS * sizeof(message_t))) == NULL) {
	error("No memory for messages");
	return -1;
    }
    MsgBlock = x;
    for (i = 0; i < 2 * MAX_MSGS; i++) {
	if (i < MAX_MSGS) {
	    TalkMsg[i] = x;
	} else {
	    GameMsg[i - MAX_MSGS] = x;
	}
	x->txt[0] = '\0';
	x->len = 0;
	x->life = 0;
	x++;
    }
    return 0;
}


void Free_msgs(void)
{
    free(MsgBlock);
}


void ShadowDrawString(Display* dpy, Window w, GC gc,
		      int x, int y, char* str,
		      unsigned long fg, unsigned long bg)
{
    if (HaveColor(dpy)) {
	XSetForeground(dpy, gc, bg);
	XDrawString(dpy, w, gc, x+1, y+1, str, strlen(str));
	x--; y--;
    }
    XSetForeground(dpy, gc, fg);
    XDrawString(dpy, w, gc, x, y, str, strlen(str));
}


/*
 * General text formatting routine which does wrap around
 * if necessary at whitespaces.  The function returns the
 * vertical position it ended at.
 */
int DrawShadowText(Display* dpy, Window w, GC gc,
		    int x_border, int y_start, char *str,
		    unsigned long fg, unsigned long bg)
{
    XFontStruct*	font = XQueryFont(dpy, XGContextFromGC(gc));
    int			y, x;
    XWindowAttributes	wattr;

    if (str==NULL || *str=='\0')
	return 0;

    /* Get width of window */
    XGetWindowAttributes(dpy, w, &wattr);

    /* Start position */
    x = x_border;
    y = y_start + font->ascent;

    do {
	char word[LINE_MAX];
	int wordLen, i;

	for (i=0; *str && !isspace(*str) && i < LINE_MAX-1; str++, i++)
	    word[i] = *str;
	word[i] = '\0';

	/* Word length in pixels */
	wordLen = XTextWidth(font, word, i);

	/* We need a linebreak? */
	if (x + wordLen > wattr.width - BORDER) {
	    x = x_border;
	    y += font->ascent + font->descent + 1;
	}

	/* Draw word and move cursor to point to after this word */
	ShadowDrawString(dpy, w, gc, x, y, word, fg, bg);
	x += wordLen;

	/* Handle whitespace */
	for (; isspace(*str); str++)
	    switch (*str) {
		/* New paragraph */
	    case '\n':
		x = x_border;
		y += font->ascent + font->descent + 1;
		break;

		/* Just a space */
	    default:
		x += XTextWidth(font, " ", 1);
		break;
	    }
    } while (*str != '\0');

    return y + font->descent + 1;
}


#define NUM_ABOUT_PAGES		4

/*
 * This variable tells us what item comes last on page 0.  If -1 it hasn't
 * been initialized yet (page 0 needs exposing to do this).  If it is
 * NUM_ITEMS-1 then there is no need to split to page and the NEXT and PREV
 * keys will automatically skip that page.
 */
static int itemsplit = -1;

void Expose_about_window(void)
{
    int	i, y, old_y, box_start, box_end, first, last;

    XClearWindow(dpy, about_w);

    switch (about_page) {
    case 0:
    case 1:
	if (about_page == 0) {
	    y = DrawShadowText(dpy, about_w, textGC,
			   BORDER, BORDER,
			   "BONUS ITEMS\n"
			   "\n"
			   "Scattered around the world you might find some "
			   "of these red triangle objects.  They are "
			   "well worth picking up since they either improve "
			   "on the equipment you have, or they give you "
			   "new equipment.  If a fighter explodes, some of "
			   "its equipment might be found among the debris.",
			   colors[WHITE].pixel, colors[BLACK].pixel);
	    first = 0;
	    last = (itemsplit == -1) ? (NUM_ITEMS-1) : itemsplit;
	} else {
	    y = DrawShadowText(dpy, about_w, textGC,
			   BORDER, BORDER,
			   "BONUS ITEMS CONTINUED\n",
			   colors[WHITE].pixel, colors[BLACK].pixel);
	    first = itemsplit+1;
	    last = (NUM_ITEMS-1);
	}

	y += BORDER;
	box_start = y;
	y += BORDER / 2;
	for (i = first; i <= last; i++) {

	    y += BORDER / 2;

	    /* Draw description text */
	    old_y = y;
	    y = DrawShadowText(dpy, about_w, textGC,
			       5*BORDER + 2*ITEM_SIZE, old_y,
			       itemBitmapData[i].keysText,
			       colors[WHITE].pixel, colors[BLACK].pixel);
	    if (y - old_y < 2 * ITEM_TRIANGLE_SIZE) {
		y = old_y + 2 * ITEM_TRIANGLE_SIZE;
	    }
	    box_end = y + BORDER / 2;
	    if (i == last) {
		box_end += BORDER / 2;
	    }

	    /* Paint the item on the left side */
	    XSetForeground(dpy, textGC, colors[BLACK].pixel);
	    XFillRectangle(dpy, about_w, textGC,
			   BORDER, box_start,
			   2*ITEM_SIZE+2*BORDER, box_end - box_start);
	    XSetForeground(dpy, textGC, colors[RED].pixel);
	    Paint_item(i, about_w, textGC, 2*BORDER + ITEM_SIZE,
		       old_y + ITEM_TRIANGLE_SIZE);
	    XSetForeground(dpy, textGC, colors[WHITE].pixel);

	    /*
	     * Check for items overlapping button window, if so then
	     * remove this item, set itemsplit to previous item and
	     * stop adding more items.
	     */
	    if (about_page == 0
		&& itemsplit == -1
		&& box_end >= (ABOUT_WINDOW_HEIGHT - BORDER * 2 - 4
			       - (2*BTN_BORDER + buttonFont->ascent
				  + buttonFont->descent))) {
		itemsplit = i-1;
		XSetForeground(dpy, textGC, colors[windowColor].pixel);
		XFillRectangle(dpy, about_w, textGC,
			       BORDER, box_start,
			       ABOUT_WINDOW_WIDTH, box_end - box_start);
		XSetForeground(dpy, textGC, colors[WHITE].pixel);
		break;
	    }

	    y = box_end;
	    box_start = box_end;

	}
	/*
	 * No page split, obviously font is small enough or not enough
	 * items.
	 */
	if (about_page == 0 && itemsplit == -1) {
	    itemsplit = NUM_ITEMS-1;
	}
	break;

    case 2:
	DrawShadowText(dpy, about_w, textGC,
	BORDER, BORDER,
	"GAME OBJECTIVE\n"
	"\n"
	"XPilot is a multi-player 2D space game.  "
	"Some features are borrowed from classics like the Atari coin-ups "
	"Asteriods and Gravitar, and the home-computer games "
	"Thrust (Commdore 64) and Gravity Force, but XPilot has many "
	"new features as well.\n"
	"\n"
	"The primary goal of the game is to collect points and increase "
	"your rating by destroying enemy fighters and cannons.  "
	"You are equipped with a machine gun when you start the game, "
	"but after a while you should have managed to collect some other "
	"fancy equipment.\n"
	"\n"
	"Another important task is to refuel your ship.  This is "
	"vital because your engine, radar, weapons and shields all "
	"require fuel.  Some even work better the more fuel you "
	"have aboard (mainly the radar).\n"
	"\n"
	"Optional modes include variations on this game play: "
	"you can play together in teams, you can disable shields "
	"(and all other equipment if you like), "
	"you can race against time and fellow players, and much much more.",
	colors[WHITE].pixel, colors[BLACK].pixel);
	break;

    case 3:
	DrawShadowText(dpy, about_w, textGC,
	BORDER, BORDER,
	"ABOUT XPILOT\n"
	"\n"
	"The game was conceived in its orignal form at the "
	"University of Tromsø (Norway) by Ken Ronny Schouten and "
	"Bjørn Stabell during the fall of 1991, but much of the game today "
	"is the result of hard efforts by Bert Gÿsbers of the "
	"molecular cytology lab at the University of Amsterdam (The Netherlands).  "
	"Bert joined the team in the spring of 1993.\n"
	"\n"
	"A large number of features have been contributed by XPilot fans from "
	"all around the world.  See the CREDITS file for details.\n"
	"\n"
	"For more information, "
	"read the XPilot FAQ (Frequently Asked Questions), "
	"and the on-line manual pages for xpilot(6) and xpilots(6).\n"
	"\n"
	"Bugs should be reported to <xpilot@cs.uit.no>.\n"
	"\n\n"
	"Good luck as a future xpilot,\n"
	"Bjørn Stabell, Ken Ronny Schouten & Bert Gÿsbers",
	colors[WHITE].pixel, colors[BLACK].pixel);
	break;

    default:
	error("Unkown page number %d\n", about_page);
	break;
    }
}


static void createAboutWindow(void)
{
    const int			windowWidth = ABOUT_WINDOW_WIDTH,
				buttonWindowHeight = 2*BTN_BORDER
				    + buttonFont->ascent + buttonFont->descent,
				windowHeight = ABOUT_WINDOW_HEIGHT;
    int				textWidth;
    XSetWindowAttributes	sattr;
    unsigned long		mask;


    /*
     * Create the window and initialize window name.
     */
    mask = 0;
    sattr.background_pixel = colors[windowColor].pixel;
    mask |= CWBackPixel;
    sattr.border_pixel = colors[borderColor].pixel;
    mask |= CWBorderPixel;
    if (colormap != 0) {
	sattr.colormap = colormap;
	mask |= CWColormap;
    }
    sattr.backing_store = Always;
    mask |= CWBackingStore;

    about_w
	= XCreateWindow(dpy,
			DefaultRootWindow(dpy),
			0, 0,
			windowWidth, windowHeight,
			2, dispDepth,
			InputOutput, visual,
			mask, &sattr);
    XStoreName(dpy, about_w, "XPilot - information");
    XSetIconName(dpy, about_w, "XPilot/info");
    XSetTransientForHint(dpy, about_w, top);

    textWidth = XTextWidth(buttonFont, "CLOSE", 5);
    about_close_b
	= XCreateSimpleWindow(dpy, about_w,
			      BORDER, (windowHeight - BORDER
				       - buttonWindowHeight - 4),
			      2*BTN_BORDER + textWidth,
			      buttonWindowHeight,
			      0, 0,
			      colors[buttonColor].pixel);

    /*
     * Create 'buttons' in the window.
     */
    textWidth = XTextWidth(buttonFont, "NEXT", 4);
    about_next_b
	= XCreateSimpleWindow(dpy, about_w,
			      windowWidth/2 - BTN_BORDER - textWidth/2,
			      windowHeight - BORDER - buttonWindowHeight - 4,
			      2*BTN_BORDER + textWidth, buttonWindowHeight,
			      0, 0,
			      colors[buttonColor].pixel);
    textWidth = XTextWidth(buttonFont, "PREV", 4);
    about_prev_b
	= XCreateSimpleWindow(dpy, about_w,
			      windowWidth - BORDER - 2*BTN_BORDER - textWidth,
			      windowHeight - BORDER - buttonWindowHeight - 4,
			      2*BTN_BORDER + textWidth, buttonWindowHeight,
			      0, 0,
			      colors[buttonColor].pixel);

    XSelectInput(dpy, about_close_b,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XSelectInput(dpy, about_next_b,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XSelectInput(dpy, about_prev_b,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XSelectInput(dpy, about_w, ExposureMask);

    Expose_about_window();

    XMapSubwindows(dpy, about_w);
}


void Expose_button_window(int color, Window w)
{
    if (w != about_close_b
	&& w != about_next_b
	&& w != about_prev_b) {
	return;
    }

    if (mono == false) {
	XWindowAttributes	wattr;			/* Get window height */
	XGetWindowAttributes(dpy, w, &wattr);	/* and width */

	XSetForeground(dpy, buttonGC, colors[color].pixel);
	XFillRectangle(dpy, w, buttonGC, 0, 0, wattr.width, wattr.height);
	XSetForeground(dpy, buttonGC, colors[WHITE].pixel);
    } else
	XClearWindow(dpy, w);

    if (w == about_close_b)
	ShadowDrawString(dpy, w, buttonGC,
			 BTN_BORDER, buttonFont->ascent + BTN_BORDER,
			 "CLOSE",
			 colors[WHITE].pixel, colors[BLACK].pixel);
    if (w == about_next_b)
	ShadowDrawString(dpy, w, buttonGC,
			 BTN_BORDER, buttonFont->ascent + BTN_BORDER,
			 "NEXT",
			 colors[WHITE].pixel, colors[BLACK].pixel);
    if (w == about_prev_b)
	ShadowDrawString(dpy, w, buttonGC,
			 BTN_BORDER, buttonFont->ascent + BTN_BORDER,
			 "PREV",
			 colors[WHITE].pixel, colors[BLACK].pixel);
}


void About(Window w)
{
    if (about_created == false) {
	createAboutWindow();
	about_created = true;
    }
    if (w == about_close_b) {
	about_page = 0;
	XUnmapWindow(dpy, about_w);
    } else if (w == about_next_b) {
	about_page++;
	if (about_page == 1 && itemsplit >= NUM_ITEMS-1)
	    about_page++;
	if (about_page >= NUM_ABOUT_PAGES)
	    about_page = 0;
	Expose_about_window();
    } else if (w == about_prev_b) {
	about_page--;
	if (about_page == 1 && itemsplit >= NUM_ITEMS-1)
	    about_page--;
	if (about_page <= -1)
	    about_page = NUM_ABOUT_PAGES-1;
	Expose_about_window();
    }
}


static int About_callback(int widget_desc, void *data, char **str)
{
    if (about_created == false) {
	createAboutWindow();
	about_created = true;
    }
    XMapWindow(dpy, about_w);
    return 0;
}


static int Keys_callback(int widget_desc, void *data, char **unused)
{
    static bool		keys_created = false;
    static int		keys_viewer;

    if (keys_created == false) {
	unsigned	bufsize = 1;
	char		*buf = (char *)calloc(1, 1),
			*end,
			*help,
			*str;
	int		i,
			len,
			maxkeylen = 0;
	extern char* Get_keyHelpString(keys_t key);

	for (i = 0; i < maxKeyDefs; i++) {
	    if ((str = XKeysymToString(keyDefs[i].keysym)) != NULL
		&& (len = strlen(str)) > maxkeylen) {
		maxkeylen = len;
	    }
	}
	for (i = 0; i < maxKeyDefs; i++) {
	    if (!(str = XKeysymToString(keyDefs[i].keysym))
		|| !(help = Get_keyHelpString(keyDefs[i].key))) {
		continue;
	    }
	    bufsize += strlen(str) + strlen(help) + maxkeylen + 4;
	    if (!(buf = (char *)realloc(buf, bufsize))) {
		error("No memory for key list");
		return 0;
	    }
	    end = &buf[strlen(buf)];
	    sprintf(end, "%-*s  %s\n", maxkeylen, str, help);
	}
	keys_viewer =
	    Widget_create_viewer(buf,
				 strlen(buf),
				 2*DisplayWidth(dpy, DefaultScreen(dpy))/3,
				 4*DisplayHeight(dpy, DefaultScreen(dpy))/5,
				 2,
				 "XPilot - key reference", "XPilot:keys",
				 motdFont);
	if (keys_viewer == NO_WIDGET) {
	    errno = 0;
	    error("Can't create key viewer");
	    return 0;
	}

	keys_created = true;
    }
    else if (keys_viewer != NO_WIDGET) {
	Widget_map(keys_viewer);
    }
    return 0;
}


static int Config_callback(int widget_desc, void *data, char **str)
{
    Config(true);
    return 0;
}


static int Score_callback(int widget_desc, void *data, char **str)
{
    Config(false);
    if (showRealName != false) {
	showRealName = false;
	scoresChanged = 1;
    }
    return 0;
}


static int Player_callback(int widget_desc, void *data, char **str)
{
    Config(false);
    if (showRealName != true) {
	showRealName = true;
	scoresChanged = 1;
    }
    return 0;
}


static int Quit_callback(int widget_desc, void *data, char **str)
{
    quitting = true;
    return 0;
}


static void createTalkWindow(void)
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
	    createTalkWindow();
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


void Resize(Window w, int width, int height)
{
    if (w != top) {
	return;
    }
    /* ignore illegal resizes */
    LIMIT(width, MIN_TOP_WIDTH, MAX_TOP_WIDTH);
    LIMIT(height, MIN_TOP_HEIGHT, MAX_TOP_HEIGHT);
    if (width == top_width && height == top_height) {
	return;
    }
    top_width = width;
    top_height = height;
    draw_width = top_width - 258;
    draw_height = top_height;
    Send_display();
    Net_flush();
    XResizeWindow(dpy, draw, draw_width, draw_height);
    if (dbuf_state->type == PIXMAP_COPY) {
	XFreePixmap(dpy, p_draw);
	p_draw = XCreatePixmap(dpy, draw, draw_width, draw_height, dispDepth);
    }
    XResizeWindow(dpy, players,
		  256, top_height - (RadarHeight + ButtonHeight + 2));
    if (talk_created) {
	XMoveResizeWindow(dpy, talk_w,
			  TALK_WINDOW_X, TALK_WINDOW_Y,
			  TALK_WINDOW_WIDTH, TALK_WINDOW_HEIGHT);
    }
    Config_resize();
}


/*
 * Cleanup player structure, close the display etc.
 */
void Quit(void)
{
    if (dpy != NULL) {
	XAutoRepeatOn(dpy);
	if (dbuf_state) {
	    end_dbuff(dbuf_state);
	    dbuf_state = NULL;
	}
	if (colormap) {
	    XFreeColormap(dpy, colormap);
	    colormap = 0;
	}
	XCloseDisplay(dpy);
	dpy = NULL;
	if (kdpy) {
	    XAutoRepeatOn(kdpy);
	    XCloseDisplay(kdpy);
	    kdpy = NULL;
	}
    }
}


int FatalError(Display *dpy)
{
    Net_cleanup();
    /*
     * Quit(&client);
     * It's already a fatal I/O error, nothing to cleanup.
     */
    exit(0);
    return(0);
}


#define MAX_MOTD_SIZE	(30*1024)

static char		*motd_buf;
static int		motd_size;
static int		motd_viewer = NO_WIDGET;
static int		motd_auto_popup;

static int Motd_callback(int widget_desc, void *data, char **str)
{
    if (motd_buf == NULL) {
	motd_auto_popup = 0;
	Net_ask_for_motd(0, MAX_MOTD_SIZE);
	Net_flush();
    }
    else if (motd_viewer != NO_WIDGET) {
	Widget_map(motd_viewer);
    }
    return 0;
}


int Handle_motd(long off, char *buf, int len, long filesize)
{
    int			i;
    static char		no_motd_msg[] = "\nServer has no MOTD\n\n";

    if (!motd_buf) {
	motd_size = MIN(filesize, MAX_MOTD_SIZE);
	i = MAX(motd_size, sizeof no_motd_msg) + 1;
	if (!(motd_buf = (char *) malloc(i))) {
	    error("No memory for MOTD");
	    return -1;
	}
	memset(motd_buf, ' ', motd_size);
	for (i = 39; i < motd_size; i += 40) {
	    motd_buf[i] = '\n';
	}
    }
    else if (filesize < motd_size) {
	motd_size = filesize;
	motd_buf[motd_size] = '\0';
    }
    if (off < motd_size && len > 0) {
	if (off + len > motd_size) {
	    len = motd_size - off;
	}
	memcpy(motd_buf + off, buf, len);
    }
    else if (len == 0 && off > 0) {
	return 0;
    }
    if (motd_size == 0) {
	if (motd_auto_popup) {
	    if (motd_buf != NULL) {
		free(motd_buf);
		motd_buf = NULL;
	    }
	    return 0;
	}
	strcpy(motd_buf, no_motd_msg);
	motd_size = strlen(motd_buf);
    }
    if (motd_viewer == NO_WIDGET) {
	char title[100];
	sprintf(title, "XPilot motd from %s", servername);
	motd_viewer =
	    Widget_create_viewer(motd_buf,
				 (off || len) ? (off + len) : strlen(motd_buf),
				 2*DisplayWidth(dpy, DefaultScreen(dpy))/3,
				 4*DisplayHeight(dpy, DefaultScreen(dpy))/8,
				 2,
				 title, "XPilot:motd",
				 motdFont);
	if (motd_viewer == NO_WIDGET) {
	    errno = 0;
	    error("Can't create MOTD viewer");
	}
    }
    else if (len > 0) {
	Widget_update_viewer(motd_viewer, motd_buf, off + len);
    }

    return 0;
}

int Startup_server_motd(void)
{
    if (autoServerMotdPopup) {
	motd_auto_popup = 1;
	return Net_ask_for_motd(0, MAX_MOTD_SIZE);
    }
    return 0;
}

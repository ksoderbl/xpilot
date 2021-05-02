/* $Id: xinit.c,v 3.25 1993/08/03 11:53:42 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <X11/Xproto.h>
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

#include "version.h"
#include "icon.h"
#include "client.h"
#include "paint.h"
#include "draw.h"
#include "xinit.h"
#include "bit.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: xinit.c,v 3.25 1993/08/03 11:53:42 bjoerns Exp $";
#endif

extern message_t	*Msg[];
extern int		RadarHeight;

/*
 * Globals.
 */
int	ButtonHeight;
Atom	ProtocolAtom, KillAtom;
bool	talk_mapped;


static XFontStruct	*font;
static message_t	*MsgBlock;
static char		*keyHelpList = NULL, *keyHelpDesc = NULL;
static int		buttonColor, windowColor;
static int		KeyDescOffset;
static bool		about_created;
static bool		help_created;
static bool		talk_created;
static char		talk_str[MAX_CHARS];
static struct {
    bool		visible;
    short		offset;
} talk_cursor;


/*
 * Item structures
 */
#include "item.h"

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
/* NB!  Is dependent on the order of the items in item.h */
static struct {
    char*	data;
    char*	helpText;
} itemBitmapData[NUM_ITEMS] = {
    {	itemEnergyPack_bits,
	    "Extra energy/fuel"					},
    {	itemWideangleShot_bits,
	    "Extra front cannons"				},
    {	itemRearShot_bits,
	    "Extra rear cannon"					},
    {	itemAfterburner_bits,
	    "Afterburner; makes your engines more efficient"	},
    {	itemCloakingDevice_bits,
	    "Cloaking device; "
	    "makes you invisible, both on radar and on screen"	},
    {	itemSensorPack_bits,
	    "Sensor; "
	    "enables you to see cloaked opponents more easily"	},
    {	itemTransporter_bits,
	    "Transporter; enables you to steal equipment from "
	    "other players"					},
    {	itemTank_bits,
	    "Tank; "
	    "makes refueling quicker, increases maximum fuel "
	    "capacity and can be jettisoned to confuse enemies"	},
    {	itemMinePack_bits,
	    "Mine; "
	    "can be dropped as a bomb or as a stationary mine"	},
    {	itemRocketPack_bits,
	    "Rocket; can be utilized as smart missile, "
	    "heat seaking missile, nuclear missile or just a "
	    "plain unguided missile (torpedo)"			},
    {	itemEcm_bits,
	    "ECM (Electronic Counter Measures); "
	    "can be used to disturb electronic equipment, for instance "
	    "can it be used to confuse smart missiles and reprogram "
	    "robots to seak certain players"			},
};
Pixmap	itemBitmaps[NUM_ITEMS];		/* Bitmaps for the items */
    

#ifndef NO_ROTATING_DASHES
char dashes[NUM_DASHES] = { 8, 4 };
#endif

/* How far away objects should be placed from each other etc... */
#define BORDER	10
#define BTN_BORDER 4

/* Information window dimensions */
#define ABOUT_WINDOW_WIDTH	600
#define ABOUT_WINDOW_HEIGHT	768
#define TALK_TEXT_HEIGHT	(textFont->ascent + textFont->descent)
#define TALK_OUTSIDE_BORDER	2
#define TALK_INSIDE_BORDER	3
#define TALK_WINDOW_HEIGHT	(TALK_TEXT_HEIGHT + 2 * TALK_INSIDE_BORDER)
#define TALK_WINDOW_X		(50 - TALK_OUTSIDE_BORDER)
#define TALK_WINDOW_Y		(768*3/4 - TALK_WINDOW_HEIGHT/2)
#define TALK_WINDOW_WIDTH	(768 - 2*(TALK_WINDOW_X + TALK_OUTSIDE_BORDER))

#define CTRL(c)			((c) & 0x1F)

static void createAboutWindow(void);
static void createHelpWindow(void);
static void createTalkWindow(void);


/*
 * Set specified font for that GC.
 * Return font that is used for this GC, even if setting a new
 * font failed (return default font in that case).
 */
XFontStruct* Set_font(Display* dpy, GC gc, char* fontName)
{
    XFontStruct*	font;

    if ((font = XLoadQueryFont(dpy, fontName)) == NULL) {
	error("Couldn't find font '%s', using default font", fontName);
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

    for (i = 0; i < 4; i++) {
	if (XParseColor(dpy, cmap, color_names[i], &colors[i]) == 0) {
	    printf("Can't parse color %d \"%s\"\n",
		   i, color_names[i]);
	    if (XParseColor(dpy, cmap, color_defaults[i], &colors[i]) == 0) {
		printf("Can't parse default color %d \"%s\"\n",
		       i, color_defaults[i]);
		return -1;
	    }
	}
    }
    return 0;
}


/*
 * The following function initializes a player window.
 * It returns 0 if the initialization was successful, 
 * or -1 if it couldn't initialize the double buffering routine.
 */
int Init_window(void)
{
    int			i, p, values;
    XGCValues		xgc;
    char		msg[256];


    /*
     * Get misc. display info.
     */
    dpy_type = DT_IS_DISPLAY;
    if (HavePlanes(dpy))
	SET_BIT(dpy_type, DT_HAVE_PLANES | DT_HAVE_COLOR);


    colormap = 0;

    if (Parse_colors(DefaultColormap(dpy, DefaultScreen(dpy))) == -1) {
	return -1;
    }

    /*
     * Initializes the double buffering routine.
     */
    dbuf_state = start_dbuff(dpy,
			     DefaultColormap(dpy,
					     DefaultScreen(dpy)),
			     BIT(dpy_type, DT_HAVE_COLOR)
			     ? COLOR_SWITCH : PIXMAP_COPY,
			     BIT(dpy_type, DT_HAVE_COLOR) ? 2 : 1,
			     colors);

    if (dbuf_state == NULL) {

	error("Short of colors, creating private cmap for '%s'", name);
	/*
	 * Create a private colormap if we can't allocate enough colors.
	 */
	colormap = XCreateColormap(dpy, DefaultRootWindow(dpy),
				   DefaultVisual(dpy,
						 DefaultScreen(dpy)),
				   AllocNone);
	/*
	 * Try to initialize the double buffering again.
	 */
	dbuf_state = start_dbuff(dpy, colormap,
				 BIT(dpy_type, DT_HAVE_COLOR)
				 ? COLOR_SWITCH : PIXMAP_COPY,
				 BIT(dpy_type, DT_HAVE_COLOR) ? 2 : 1,
				 colors);
    }

    if (dbuf_state == NULL) {
	/* Can't setup double buffering */
	return -1;
    }


    about_created = false;
    help_created = false;
    talk_created = false;
    talk_mapped = false;
    talk_str[0] = '\0';


    /*
     * Create toplevel window (we need this first so that we can create GCs)
     */
    top = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0,
			      1026, 768, 0, 0, colors[WHITE].pixel);
    if (colormap)
	XSetWindowColormap(dpy, top, colormap);


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
    gc
	= XCreateGC(dpy, top, values, &xgc);
    XSetBackground(dpy, gc, colors[BLACK].pixel);
    XSetDashes(dpy, gc, 0, dashes, NUM_DASHES);


    /*
     * Set fonts
     */
    gameFont
	= Set_font(dpy, gc, gameFontName);
    messageFont
	= Set_font(dpy, messageGC, messageFontName);
    scoreListFont
	= Set_font(dpy, scoreListGC, scoreListFontName);
    buttonFont
	= Set_font(dpy, buttonGC, buttonFontName);
    textFont
	= Set_font(dpy, textGC, textFontName);

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

    
    if (BIT(dpy_type, DT_HAVE_PLANES))
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
		    dpl_2[p] = 1<<i;
		else
		    dpl_1[p] = 1<<i;
    }


    /*
     * Creates the windows.
     */
    if (BIT(dpy_type, DT_HAVE_COLOR)) {
	windowColor = BLUE;
	buttonColor = RED;
    } else {
	buttonColor = BLACK;
	windowColor = BLACK;
    }

    draw = XCreateSimpleWindow(dpy, top, 258, 0,
			       768, 768, 0, 0, colors[BLACK].pixel);
    radar = XCreateSimpleWindow(dpy, top, 0, 0,
				256, RadarHeight, 0, 0,
				colors[BLACK].pixel);

    /* Create buttons */
#define BUTTON_WIDTH	84
    ButtonHeight
	= buttonFont->ascent + buttonFont->descent + 2*BTN_BORDER;

    quit_b
	= XCreateSimpleWindow(dpy, top,
			      1 + 0*BUTTON_WIDTH, RadarHeight+1,
			      BUTTON_WIDTH, ButtonHeight, 0, 0,
			      colors[buttonColor].pixel);
    about_b
	= XCreateSimpleWindow(dpy, top,
			      2 + 1*BUTTON_WIDTH, RadarHeight+1,
			      BUTTON_WIDTH, ButtonHeight, 0, 0,
			      colors[buttonColor].pixel);
    help_b
	= XCreateSimpleWindow(dpy, top,
			      3 + 2*BUTTON_WIDTH, RadarHeight+1,
			      BUTTON_WIDTH, ButtonHeight, 0, 0,
			      colors[buttonColor].pixel);

    /* Create score list window */
    players
	= XCreateSimpleWindow(dpy, top, 0, RadarHeight+ButtonHeight+2,
			      256, 768 - (RadarHeight - ButtonHeight - 2),
			      0, 0,
			      colors[windowColor].pixel);

    /*
     * Selecting events the we can handle.
     */
    XSelectInput(dpy, top,
		 KeyPressMask | KeyReleaseMask
		 | FocusChangeMask | StructureNotifyMask);
    XSelectInput(dpy, quit_b,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XSelectInput(dpy, about_b,
		 ButtonPressMask | ButtonReleaseMask| ExposureMask);
    XSelectInput(dpy, help_b,
		 ButtonPressMask | ButtonReleaseMask | ExposureMask);
    XSelectInput(dpy, radar, ExposureMask);
    XSelectInput(dpy, players, ExposureMask);
    XSelectInput(dpy, draw, 0);


    /*
     * Initialize miscellaneous window hints and properties.
     */
    {
	extern char	**Argv;
	extern int	Argc;
	XClassHint	xclh;
	XWMHints	xwmh;
	XSizeHints	xsh;

	xwmh.flags	   = InputHint|StateHint|IconPixmapHint; 
	xwmh.input	   = True;
	xwmh.initial_state = NormalState;
	xwmh.icon_pixmap   = XCreateBitmapFromData(dpy, top,
						   (char *)icon_bits,
						   icon_width, icon_height);

	xsh.flags = (PPosition|PSize|PMinSize|PMaxSize|PBaseSize);
	xsh.width = xsh.base_width = xsh.min_width = xsh.max_width = 1026;
	xsh.height = xsh.base_height = xsh.min_height = xsh.max_height = 768;
	xsh.x = (DisplayWidth(dpy, DefaultScreen(dpy)) - xsh.width) /2;
	xsh.y = (DisplayHeight(dpy, DefaultScreen(dpy)) - xsh.height) /2;

	xclh.res_name = NULL;	/* NULL: Automatically uses Argv[0], */
	xclh.res_class = "XPilot"; /* stripped of directory prefixes. */

	/*
	 * Set the above properties.
	 */
	XSetWMProperties(dpy, top, NULL, NULL, Argv, Argc,
			 &xsh, &xwmh, &xclh);

	/*
	 * Now initialize icon and window title name.
	 */
	sprintf(msg,
		"Successful connection to server at \"%s\".", servername);
	XStoreName(dpy, top, msg);

	sprintf(msg, "%s:%s", name, servername);
	XSetIconName(dpy, top, msg);

	/*
	 * Specify IO error handler and the WM_DELETE_WINDOW atom in
	 * an attempt to catch 'nasty' quits.
	 */
	ProtocolAtom = XInternAtom(dpy, "WM_PROTOCOLS", False);
	KillAtom = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(dpy, top, &KillAtom, 1);
	XSetIOErrorHandler(FatalError);
    }


    /*
     * Initialize misc. pixmaps if this is monochrome.
     */
    if (!BIT(dpy_type, DT_HAVE_PLANES)) {
	p_radar = XCreatePixmap(dpy, radar, 256, RadarHeight, 1);
	s_radar = XCreatePixmap(dpy, radar, 256, RadarHeight, 1);
	p_draw  = XCreatePixmap(dpy, draw, 768, 768, 1);
    }
    else {
	if (BIT(instruments, SHOW_SLIDING_RADAR) != 0) {
	    s_radar = XCreatePixmap(dpy, radar,
				    256, RadarHeight,
				    DefaultDepth(dpy, DefaultScreen(dpy)));
	} else {
	    s_radar = radar;
	}
	p_radar = s_radar;
	p_draw = draw;
    }

    XAutoRepeatOff(dpy);	/* We don't want any autofire, yet! */


    /*
     * Maps the windows, makes the visible. Voila!
     */
    XMapWindow(dpy, draw);
    XMapWindow(dpy, radar);
    XMapWindow(dpy, quit_b);
    XMapWindow(dpy, about_b);
    XMapWindow(dpy, help_b);
    XMapWindow(dpy, players);
    XMapWindow(dpy, top);


    XSync(dpy, False);

    return 0;
}


int Alloc_msgs(int number)
{
    message_t *x;
    int i;

    if ((x = (message_t *)malloc(number * sizeof(message_t))) == NULL) {
	error("No memory for messages");
	return -1;
    }
    MsgBlock = x;
    for (i=0; i<number; i++) {
	Msg[i]=x;
	x->txt[0] = '\0';
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
		      int x, int y, char* str, Pixel fg, Pixel bg)
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
		    int x_border, int y_start,
		    char *str, Pixel fg, Pixel bg)
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


#define NUM_ABOUT_PAGES	3

void Expose_about_window(void)
{
    XClearWindow(dpy, about_w);

    switch (about_page) {
    case 0: {
	int	i, y;
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
	y += ITEM_SIZE + BORDER;
	for (i=0; i<NUM_ITEMS; i++) {
	    int old_y = y;

	    /* Draw description text */
	    y = DrawShadowText(dpy, about_w, textGC,
			       5*BORDER + 2*ITEM_SIZE, y,
			       itemBitmapData[i].helpText,
			       colors[WHITE].pixel, colors[BLACK].pixel);

	    /* Paint the item on the left side */
	    XSetForeground(dpy, textGC, colors[BLACK].pixel);
	    XFillRectangle(dpy, about_w, textGC,
			   BORDER, old_y - (ITEM_SIZE + BORDER)/2,
			   2*ITEM_SIZE+2*BORDER, y-old_y+BORDER+ITEM_SIZE);
	    XSetForeground(dpy, textGC, colors[RED].pixel);
	    Paint_item(i, about_w, textGC, 2*BORDER + ITEM_SIZE, (y+old_y)/2);
	    XSetForeground(dpy, textGC, colors[WHITE].pixel);

	    y += ITEM_SIZE + BORDER;
	}
    }
	break;
	    
    case 1:
	DrawShadowText(dpy, about_w, textGC,
	BORDER, BORDER,
	"GAME OBJECTIVE\n"
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

    case 2:
	DrawShadowText(dpy, about_w, textGC,
	BORDER, BORDER,
	"ABOUT XPILOT\n"
	"\n"
	"XPilot is still not a finished product, so please apology for "
	"any bugs etc.  However, if you find any, we would greatly "
	"appreciate that you reported to us.\n"
	"\n"
	"New versions are continuously being developed, but at a random "
	"rate.  Currently, this isn't very fast at all, mainly due to the "
	"mandatory work in conjunction with our studies (really!!).\n"
	"\n"
	"For more info, read the man pages for xpilot(6) and xpilots(6).\n"
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
    XSetWindowAttributes	setAttr;


    /*
     * Create the window and initialize window name.
     */
    about_w
	= XCreateSimpleWindow(dpy, DefaultRootWindow(dpy),
			      0, 0,
			      windowWidth, windowHeight,
			      2, colors[WHITE].pixel, 
			      colors[windowColor].pixel);
    if (colormap)
	XSetWindowColormap(dpy, about_w, colormap);
    XStoreName(dpy, about_w, "XPilot - information");
    XSetIconName(dpy, about_w, "XPilot/info");

    /*
     * Enable backing store if possible.
     */
    setAttr.backing_store = Always;
    XChangeWindowAttributes(dpy, about_w, CWBackingStore, &setAttr);

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
}


static void createHelpWindow(void)
{
    const int			buttonWindowHeight = 2*BTN_BORDER
				    + buttonFont->ascent + buttonFont->descent;
    int				windowWidth = 0,
				windowHeight = 0,
				maxKeyNameLen = 0,
				maxKeyDescLen = 0;
    XSetWindowAttributes	setAttr;
    extern char* Get_keyhelpstring(keys_t);

    if (keyHelpList == NULL) {
	int	sizeList = 1, sizeDesc = 1, i = 0;

	/*
	 * Make sure the strings are empty and null terminated.
	 */
	keyHelpList = malloc(1);
	keyHelpList[0] = '\0';
	keyHelpDesc = malloc(1);
	keyHelpDesc[0] = '\0';

	/*
	 * Build the key help string, and while we're at it find
	 * the extent (physical :) of the text.
	 */
	for (i = 0; i < maxKeyDefs; i++) {
	    int len;
	    char* str;

	    /*
	     * Key name
	     */
	    str = XKeysymToString(keyDefs[i].keysym);
	    sizeList += strlen(str) + 1;
	    keyHelpList = realloc(keyHelpList, sizeList);
	    strcat(keyHelpList, str);
	    keyHelpList[sizeList-2] = '\n';
	    keyHelpList[sizeList-1] = '\0';

	    /* Store longest keysym name */
	    len = XTextWidth(textFont, str, strlen(str));
	    if (len > maxKeyNameLen)
		maxKeyNameLen = len;

	    /*
	     * Description of action invoked for this key
	     */
	    str = Get_keyhelpstring(keyDefs[i].key);
	    sizeDesc += strlen(str) + 1;
	    keyHelpDesc = realloc(keyHelpDesc, sizeDesc);
	    strcat(keyHelpDesc, str);
	    keyHelpDesc[sizeDesc-2] = '\n';
	    keyHelpDesc[sizeDesc-1] = '\0';

	    /* Store longest desc */
	    len = XTextWidth(textFont, str, strlen(str));
	    if (len > maxKeyDescLen)
		maxKeyDescLen = len;

	    windowHeight += textFont->ascent + textFont->descent + 1;
	}
    }

    /*
     * Now calculate window dimensions and the offset we need to
     * put the description (the x coordinate it should begin on).
     */
    windowHeight += buttonWindowHeight + 4 + 3*BORDER;
    windowWidth = 4*BORDER + maxKeyNameLen + maxKeyDescLen;
    KeyDescOffset = 3*BORDER + maxKeyNameLen;

    /*
     * Create main window.
     */
    help_w
	= XCreateSimpleWindow(dpy, DefaultRootWindow(dpy),
			      0, 0,
			      windowWidth, windowHeight,
			      2, colors[WHITE].pixel,
			      colors[windowColor].pixel);
    if (colormap)
	XSetWindowColormap(dpy, help_w, colormap);
    XStoreName(dpy, help_w, "XPilot - key reference");
    XSetIconName(dpy, help_w, "XPilot/keys");

    /*
     * Enable backing store if possible.
     */
    setAttr.backing_store = Always;
    XChangeWindowAttributes(dpy, help_w, CWBackingStore, &setAttr);

    /*
     * Create buttons.
     */
    help_close_b
	= XCreateSimpleWindow(dpy, help_w,
			      BORDER,
			      (windowHeight - BORDER - buttonWindowHeight - 4),
			      (XTextWidth(buttonFont,
					  "CLOSE", 5) + 2*BTN_BORDER),
			      buttonWindowHeight,
			      0, 0,
			      colors[buttonColor].pixel);
    XSelectInput(dpy, help_close_b,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XSelectInput(dpy, help_w, ExposureMask);

    Expose_help_window();
}

    
void Expose_help_window(void)
{
    XClearWindow(dpy, help_w);

    DrawShadowText(dpy, help_w, textGC,
		   BORDER, BORDER, keyHelpList,
		   colors[WHITE].pixel, colors[BLACK].pixel);
    DrawShadowText(dpy, help_w, textGC,
		   KeyDescOffset, BORDER, keyHelpDesc,
		   colors[WHITE].pixel, colors[BLACK].pixel);
}


void Expose_button_window(int color, Window w)
{
    if (BIT(dpy_type, DT_HAVE_COLOR)) {
	XWindowAttributes	wattr;			/* Get window height */
	XGetWindowAttributes(dpy, w, &wattr);	/* and width */

	XSetForeground(dpy, buttonGC, colors[color].pixel);
	XFillRectangle(dpy, w, buttonGC, 0, 0, wattr.width, wattr.height);
	XSetForeground(dpy, buttonGC, colors[WHITE].pixel);
    } else
	XClearWindow(dpy, w);

    if (w == quit_b)
	ShadowDrawString(dpy, w, buttonGC,
			 (BUTTON_WIDTH
			  - XTextWidth(buttonFont, "QUIT", 4)) / 2,
			 BTN_BORDER + buttonFont->ascent,
			 "QUIT",
			 colors[WHITE].pixel, colors[BLACK].pixel);

    if (w == about_b)
	ShadowDrawString(dpy, w, buttonGC,
			 (BUTTON_WIDTH
			  - XTextWidth(buttonFont, "ABOUT", 5)) / 2,
			 BTN_BORDER + buttonFont->ascent,
			 "ABOUT", 
			 colors[WHITE].pixel, colors[BLACK].pixel);
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

    if (w == help_b)
	ShadowDrawString(dpy, w, buttonGC,
			 (BUTTON_WIDTH
			  - XTextWidth(buttonFont, "KEYS", 4)) / 2,
			 BTN_BORDER + buttonFont->ascent,
			 "KEYS",
			 colors[WHITE].pixel, colors[BLACK].pixel);
    if (w == help_close_b)
	ShadowDrawString(dpy, w, buttonGC,
			 BTN_BORDER, buttonFont->ascent + BTN_BORDER,
			 "CLOSE",
			 colors[WHITE].pixel, colors[BLACK].pixel);
}


void About(Window w)
{
    if (about_created == false) {
	createAboutWindow();
	about_created = true;
    }
    if (w == about_b) {
	XMapWindow(dpy, about_w);
	XMapSubwindows(dpy, about_w);
    } else if (w == about_close_b) {
	about_page = 0;
	XUnmapSubwindows(dpy, about_w);
	XUnmapWindow(dpy, about_w);
    } else if (w == about_next_b) {
	about_page++;
	if (about_page == NUM_ABOUT_PAGES)
	    about_page = 0;
	Expose_about_window();
    } else if (w == about_prev_b) {
	about_page--;
	if (about_page == -1)
	    about_page = NUM_ABOUT_PAGES-1;
	Expose_about_window();
    }
}


void Help(Window w)
{
    if (help_created == false) {
	createHelpWindow();
	help_created = true;
    }
    if (w == help_b) {
	XMapWindow(dpy, help_w);
	XMapSubwindows(dpy, help_w);
    } else if (w == help_close_b) {
	XUnmapSubwindows(dpy, help_w);
	XUnmapWindow(dpy, help_w);
    }
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
	XSetForeground(dpy, textGC, colors[BLACK].pixel);
	XDrawString(dpy, talk_w, textGC,
		    talk_cursor.offset + TALK_INSIDE_BORDER,
		    textFont->ascent + TALK_INSIDE_BORDER,
		    "_", 1);
	XSetForeground(dpy, textGC, colors[WHITE].pixel);
	talk_cursor.visible = false;
    } else {
	talk_cursor.offset = XTextWidth(textFont, talk_str, strlen(talk_str));
	XDrawString(dpy, talk_w, textGC,
		    talk_cursor.offset + TALK_INSIDE_BORDER,
		    textFont->ascent + TALK_INSIDE_BORDER,
		    "_", 1);
	talk_cursor.visible = true;
    }
}


void Talk_map_window(bool map)
{
    if (map == true) {
	if (talk_created == false) {
	    createTalkWindow();
	    talk_created = true;
	}
	XMapWindow(dpy, talk_w);
	talk_mapped = true;
    }
    else if (talk_created == true) {
	XUnmapWindow(dpy, talk_w);
	talk_mapped = false;
    }
    talk_cursor.visible = false;
}


void Talk_event(XEvent *event)
{
    char		ch;
    bool		cursor_visible = talk_cursor.visible;
    int			i, count, oldlen, newlen, oldwidth, newwidth;
    KeySym		keysym;
    XComposeStatus	compose;

    switch (event->type) {

    case Expose:
	XClearWindow(dpy, talk_w);
	XDrawString(dpy, talk_w, textGC,
		    TALK_INSIDE_BORDER, textFont->ascent + TALK_INSIDE_BORDER,
		    talk_str, strlen(talk_str));
	if (cursor_visible == true) {
	    talk_cursor.visible = false;
	    Talk_cursor(cursor_visible);
	}
	break;

    case KeyRelease:
	break;

    case KeyPress:
	count = XLookupString(&event->xkey, &ch, 1, &keysym, &compose);
	if (count == NoSymbol || ch == '\0') {
	    /**/
	}
	else if (ch == '\r' || ch == '\n') {
	    if (talk_str[0] != '\0') {
		if (version < 0x3030) {
		    Add_message("<<Talking is not supported by this server>>");
		} else {
		    Net_talk(talk_str);
		}
		talk_str[0] = '\0';
	    }
	    Talk_map_window(false);
	}
	else if (ch == '\033') {
	    talk_str[0] = '\0';
	    Talk_map_window(false);
	}
	else if (ch == '\b' || ch == '\177'
		 || ch == CTRL('W') || ch == CTRL('U')) {
	    Talk_cursor(false);
	    oldlen = strlen(talk_str);
	    newlen = oldlen;
	    if (ch == CTRL('W')) {
		for (i = oldlen; i > 0 && talk_str[i - 1] == ' '; i--) {
		    newlen--;
		}
		for (; i > 0 && talk_str[i - 1] != ' '; i--) {
		    newlen--;
		}
	    }
	    else if (ch == CTRL('U')) {
		newlen = 0;
	    }
	    else if (oldlen > 0) {
		newlen = oldlen - 1;
	    }
	    newwidth = XTextWidth(textFont, talk_str, newlen);
	    XSetForeground(dpy, textGC, colors[BLACK].pixel);
	    XDrawString(dpy, talk_w, textGC,
			newwidth + TALK_INSIDE_BORDER,
			textFont->ascent + TALK_INSIDE_BORDER,
			&talk_str[newlen], oldlen - newlen);
	    XSetForeground(dpy, textGC, colors[WHITE].pixel);
	    talk_str[newlen] = '\0';
	    Talk_cursor(cursor_visible);
	}
	else if ((ch & 0x7F) != ch || isprint(ch)) {
	    oldlen = strlen(talk_str);
	    oldwidth = XTextWidth(textFont, talk_str, oldlen);
	    if (oldlen >= MAX_CHARS - 2
		|| oldwidth >= TALK_WINDOW_WIDTH - 2*TALK_INSIDE_BORDER - 5) {
		XBell(dpy, 100);
	    }
	    else {
		oldwidth = XTextWidth(textFont, talk_str, oldlen);
		talk_str[oldlen] = ch;
		talk_str[oldlen + 1] = '\0';
		Talk_cursor(false);
		XDrawString(dpy, talk_w, textGC,
			    oldwidth + TALK_INSIDE_BORDER,
			    textFont->ascent + TALK_INSIDE_BORDER,
			    &talk_str[oldlen], 1);
		Talk_cursor(cursor_visible);
	    }
	}
	break;

    default:
	break;
    }
}


/*
 * Cleanup player structure, close the display etc.
 */
void Quit(void)
{
    if (dpy != NULL) {
	XAutoRepeatOn(dpy);
	if (quit_b) {
	    Expose_button_window(RED, quit_b);
	    quit_b = 0;
	}
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
}

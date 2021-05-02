/* $Id: xinit.c,v 1.4 1993/03/23 17:54:18 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>

#include <stdio.h>
#ifndef	apollo
#    include <string.h>
#endif

#include "global.h"
#include "map.h"
#include "draw.h"
#include "version.h"
#include "icon.h"
#include "pack.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: xinit.c,v 1.4 1993/03/23 17:54:18 bjoerns Exp $";
#endif

/*
 * Globals.
 */
static char		msg[MSG_LEN];
static XFontStruct	*font;
static message_t	*MsgBlock;

Atom		ProtocolAtom, KillAtom;

#ifndef NO_ROTATING_DASHES
char dashes[NUM_DASHES] = { 8, 4 };
#endif


/*
 * The following function initializes a player window.  It returns SUCCESS if
 * the initialization was successful, E_DISPLAY if it couldn't open display,
 * or E_DBUFF if it couldn't initialize the double buffering routine.
 */
u_byte Init_window(int ind)
{
    int			i, p, button_color, window_color;
    XGCValues		xgc;
#ifdef	SCROLL
    char		title[256];
#endif
    player		*pl = Players[ind];
    static char		msg[256];


    /*
     * Get misc. display info.
     */
    pl->disp_type = DT_IS_DISPLAY;
    if (HavePlanes(pl->disp))
	SET_BIT(pl->disp_type, DT_HAVE_PLANES | DT_HAVE_COLOR);


    /*
     * Initializes colors.
     */
    pl->colors[BLACK].red	= 0; 
    pl->colors[BLACK].green	= 0; 
    pl->colors[BLACK].blue	= 0; 
    pl->colors[WHITE].red	= 65535;
    pl->colors[WHITE].green	= 65535;
    pl->colors[WHITE].blue	= 65535;
    pl->colors[BLUE].red	= 10000;
    pl->colors[BLUE].green	= 30000;
    pl->colors[BLUE].blue	= 65535;
    pl->colors[RED].red		= 65535;
    pl->colors[RED].green	= 15000;
    pl->colors[RED].blue	= 10000;
    pl->colormap		= NULL;


    /*
     * Initializes the double buffering routine.
     */
    pl->dbuf_state = start_dbuff(ind, pl->disp,
				 DefaultColormap(pl->disp,
						 DefaultScreen(pl->disp)),
				 BIT(pl->disp_type, DT_HAVE_COLOR) ? 2 : 1,
				 pl->colors);

    if (pl->dbuf_state == NULL) {

	error("Short of colors, creating private cmap for '%s'", pl->name);
	/*
	 * Create a private colormap if we can't allocate enough colors.
	 */
	pl->colormap = XCreateColormap(pl->disp, DefaultRootWindow(pl->disp),
				       DefaultVisual(pl->disp,
						     DefaultScreen(pl->disp)),
				       AllocNone);
	/*
	 * Try to initialize the double buffering again.
	 */
	pl->dbuf_state = start_dbuff(ind, pl->disp, pl->colormap,
				     BIT(pl->disp_type, DT_HAVE_COLOR) ? 2 : 1,
				     pl->colors);
    }

    if (pl->dbuf_state == NULL) {
	XCloseDisplay(pl->disp);
	return (E_DBUFF);
    }


    /*
     * Creates the windows.
     */
    if (BIT(pl->disp_type, DT_HAVE_COLOR)) {
	window_color = BLUE;
	button_color = RED;
    } else {
	button_color = BLACK;
	window_color = BLACK;
    }

    pl->top = XCreateSimpleWindow(pl->disp, DefaultRootWindow(pl->disp), 0, 0,
				  1026, 768, 0, 0, pl->colors[WHITE].pixel);
    if (pl->colormap)
	XSetWindowColormap(pl->disp, pl->top, pl->colormap);

    pl->draw = XCreateSimpleWindow(pl->disp, pl->top, 258, 0,
				   768, 768, 0, 0, pl->colors[BLACK].pixel);
    pl->radar = XCreateSimpleWindow(pl->disp, pl->top, 0, 0,
				    256, RadarHeight, 0, 0,
				    pl->colors[BLACK].pixel);
    pl->quit_b = XCreateSimpleWindow(pl->disp, pl->top, 0, RadarHeight+1,
				     85, 22, 0, 0,
				     pl->colors[button_color].pixel);
    pl->info_b = XCreateSimpleWindow(pl->disp, pl->top, 86, RadarHeight+1,
				     85, 22, 0, 0,
				     pl->colors[button_color].pixel);
    pl->help_b = XCreateSimpleWindow(pl->disp, pl->top, 172, RadarHeight+1,
				     84, 22, 0, 0,
				     pl->colors[button_color].pixel);
    pl->players = XCreateSimpleWindow(pl->disp, pl->top, 0, RadarHeight+24,
				      256, 746-RadarHeight, 0, 0,
				      pl->colors[window_color].pixel);
#define HH	600
    pl->help_w = XCreateSimpleWindow(pl->disp, pl->draw, (FULL-420)/2,
				     (FULL-HH)/2, 420, HH,
				     5, pl->colors[WHITE].pixel,
				     pl->colors[window_color].pixel);
    pl->help_close_b = XCreateSimpleWindow(pl->disp, pl->help_w,
					   4, HH-26, 85, 22, 0, 0,
					   pl->colors[button_color].pixel);
    pl->help_next_b = XCreateSimpleWindow(pl->disp, pl->help_w,
					  420/2-85/2, HH-26, 85, 22, 0, 0,
					  pl->colors[button_color].pixel);
    pl->help_prev_b = XCreateSimpleWindow(pl->disp, pl->help_w,
					  420-89, HH-26, 85, 22, 0, 0,
					  pl->colors[button_color].pixel);

    pl->info_w = XCreateSimpleWindow(pl->disp, pl->draw, (FULL-500)/2, 
				     (FULL-500)/2, 500, 500,
				     5, pl->colors[WHITE].pixel, 
				     pl->colors[window_color].pixel);
    pl->info_close_b = XCreateSimpleWindow(pl->disp, pl->info_w, 4, 500-26, 85,
					   22, 0, 0,
					   pl->colors[button_color].pixel);


    /*
     * Selecting events the we can handle.
     */
    XSelectInput(pl->disp, pl->top,
		 KeyPressMask | KeyReleaseMask
		 | FocusChangeMask | StructureNotifyMask);
    XSelectInput(pl->disp, pl->quit_b,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XSelectInput(pl->disp, pl->info_close_b,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XSelectInput(pl->disp, pl->help_close_b,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XSelectInput(pl->disp, pl->help_next_b,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XSelectInput(pl->disp, pl->help_prev_b,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XSelectInput(pl->disp, pl->info_b, ButtonPressMask | ExposureMask);
    XSelectInput(pl->disp, pl->help_b, ButtonPressMask | ExposureMask);
    XSelectInput(pl->disp, pl->info_w, ExposureMask);
    XSelectInput(pl->disp, pl->help_w, ExposureMask);
    XSelectInput(pl->disp, pl->radar, ExposureMask);
    XSelectInput(pl->disp, pl->players, ExposureMask);
    XSelectInput(pl->disp, pl->draw, 0);


    /*
     * Initialize miscellaneous window hints and properties.
     */
    {
	extern char	**Argv;
	extern int	Argc;
	XClassHint	xclh;
	XWMHints	xwmh;
	XSizeHints	xsh;

	xwmh.flags		= InputHint|StateHint|IconPixmapHint; 
	xwmh.input		= True;
	xwmh.initial_state	= NormalState;
	xwmh.icon_pixmap	= XCreateBitmapFromData(pl->disp, pl->top,
							icon_bits, icon_width,
							icon_height);

	xsh.flags = (PPosition|PSize|PMinSize|PMaxSize|PBaseSize);
	xsh.width = xsh.base_width = xsh.min_width = xsh.max_width = 1026;
	xsh.height = xsh.base_height = xsh.min_height = xsh.max_height = 768;
	xsh.x = (DisplayWidth(pl->disp, DefaultScreen(pl->disp))
		 - xsh.width) /2;
	xsh.y = (DisplayHeight(pl->disp, DefaultScreen(pl->disp))
		 - xsh.height) /2;

	xclh.res_name = NULL;		/* NULL: Automatically uses Argv[0], */
	xclh.res_class = "XPilot";	/* stripped of directory prefixes. */

	/*
	 * Set the above properties.
	 */
	XSetWMProperties(pl->disp, pl->top, NULL, NULL, Argv, Argc,
			 &xsh, &xwmh, &xclh);

	/*
	 * Now initialize icon and window title name.
	 */
	sprintf(msg,
		"Successful connection to server at \"%s\".", Server.host);
	XStoreName(pl->disp, pl->top, msg);

	sprintf(msg, "%s:%s", pl->name, Server.host);
	XSetIconName(pl->disp, pl->top, msg);

	/*
	 * Specify IO error handler and the WM_DELETE_WINDOW atom in
	 * an attempt to catch 'nasty' quits.
	 */
	ProtocolAtom = XInternAtom(pl->disp, "WM_PROTOCOLS", False);
	KillAtom = XInternAtom(pl->disp, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(pl->disp, pl->top, &KillAtom, 1);
	XSetIOErrorHandler(FatalError);
    }


    /*
     * Initialize misc. pixmaps if this is monochrome.
     */
    if (!BIT(pl->disp_type, DT_HAVE_PLANES)) {
	pl->p_radar = XCreatePixmap(pl->disp, pl->radar, 256, RadarHeight, 1);
	pl->s_radar = XCreatePixmap(pl->disp, pl->radar, 256, RadarHeight, 1);
	pl->p_draw  = XCreatePixmap(pl->disp, pl->draw, 768, 768, 1);
    }
    else {
	pl->p_radar = pl->radar;
	pl->s_radar = pl->radar;
	pl->p_draw = pl->draw;
    }

    /*
     * Following initializes the text that may be specified to scroll
     * across the window title.
     */
#ifdef SCROLL
    sprintf(pl->scroll, "						    "
	    "					"
	    TITLE "\"%s\" connected to server at \"%s\"..... "
	    "The game world is %s, dimensions: %d boxes wide and %d boxes "
	    "high. It was created, and made made available, by %s. You are "
	    "playing with rules bitfield $%04lx, huh? Well, don't let it "
	    "bother you. You will find %d cannons, %d gravs and %d starting "
	    "positions in this world. Not to mention %d energy stations, "
	    "which will hopefully satisfy your needs. If you are playing in "
	    "race mode, it might help to inform you that there are %d "
	    "checkpoints in each round............. "
	    "Controls: 'a' and 's' turn your "
	    "fighter left and right, 'shift' activates your splendid "
	    "(but aye so energy-consuming) engines. 'Return' fires your "
	    "cannon, and if you have first activated the compass/lock-"
	    "computer and selected a target (with either 'Select' (nearest), "
	    "'Next' or 'Prev') - you may fire smart missiles by pressing the "
	    "'\\' (BackSlash) key..... Other controls are: 'ctrl' or 'f' for "
	    "recharging your energy, obtained by maneuvering close to an "
	    "energy station and holding down the appropriate key. 'Space' "
	    "or 'Meta-R' activates your shield. So, All that's left is to "
	    "wish you good luck... :)	   Who knows, you might need it? :) "
	    "								  ",
	    pl->name, Server.host, World.name, World.x, World.y, World.author,
	    World.rules->mode, World.NumCannons, World.NumGravs,
	    World.NumBases, World.NumFuels, World.NumChecks);
    pl->scroll_len = strlen(pl->scroll);
#endif

    XAutoRepeatOff(pl->disp);	    /* We don't want any autofire, yet! */


    /*
     * Maps the windows, makes the visible. Voila!
     */
    XMapWindow(pl->disp, pl->top);
    XMapWindow(pl->disp, pl->draw);
    XMapWindow(pl->disp, pl->radar);
    XMapWindow(pl->disp, pl->quit_b);
    XMapWindow(pl->disp, pl->info_b);
    XMapWindow(pl->disp, pl->help_b);
    XMapWindow(pl->disp, pl->players);


    /*
     * Creates and initializes the graphic contexts.
     */
    pl->gc = XCreateGC(pl->disp, pl->draw, 0, &xgc);
    pl->gcr = XCreateGC(pl->disp, pl->radar, 0, &xgc);
    pl->gcb = XCreateGC(pl->disp, pl->quit_b, 0, &xgc);
    pl->gcp = XCreateGC(pl->disp, pl->players, 0, &xgc);
    pl->gctxt = XCreateGC(pl->disp, pl->info_w, 0, &xgc);

    XSetBackground(pl->disp, pl->gc, pl->colors[BLACK].pixel);
    XSetDashes(pl->disp, pl->gc, 0, dashes, NUM_DASHES);
    XSetLineAttributes(pl->disp, pl->gc, 0, LineSolid, CapButt, JoinBevel);
    pl->color = WHITE;

    if ((font = XLoadQueryFont(pl->disp,
	      "-*-*-medium-r-*-*-12-120-75-75-m-70-iso8859-1")) == NULL) {
	error("Could not load font. Will use default font");
    } else
	XSetFont(pl->disp, pl->gc, font->fid);

    if ((font = XLoadQueryFont(pl->disp,
	      "-*-*-bold-o-*-*-14-140-75-75-m-90-iso8859-1")) == NULL) {
	error("Could not load font. Will use default font....");
    } else
	XSetFont(pl->disp, pl->gcb, font->fid);

    if ((font = XLoadQueryFont(pl->disp,
	      "-*-*-bold-i-*-*-17-120-100-100-p-86-iso8859-1")) == NULL) {
	error("Could not load font. Will use default font....");
    } else
	XSetFont(pl->disp, pl->gctxt, font->fid);	

    if ((font = XLoadQueryFont(pl->disp,
	      "-*-*-bold-*-*-*-13-120-75-75-c-80-iso8859-1")) == NULL) {
	error("Could not load font. Will use default font....");
    } else
	XSetFont(pl->disp, pl->gcp, font->fid);

    XSetState(pl->disp, pl->gcr,
	      WhitePixel(pl->disp, DefaultScreen(pl->disp)),
	      BlackPixel(pl->disp, DefaultScreen(pl->disp)),
	      GXcopy, AllPlanes);
    XSetState(pl->disp, pl->gc,
	      WhitePixel(pl->disp, DefaultScreen(pl->disp)),
	      BlackPixel(pl->disp, DefaultScreen(pl->disp)),
	      GXcopy, AllPlanes);
    XSetState(pl->disp, pl->gcb,
	      WhitePixel(pl->disp, DefaultScreen(pl->disp)),
	      BlackPixel(pl->disp, DefaultScreen(pl->disp)),
	      GXcopy, AllPlanes);
    XSetState(pl->disp, pl->gcp,
	      WhitePixel(pl->disp, DefaultScreen(pl->disp)),
	      BlackPixel(pl->disp, DefaultScreen(pl->disp)),
	      GXcopy, AllPlanes);

    
    if (BIT(pl->disp_type, DT_HAVE_PLANES))
	XSetPlaneMask(pl->disp, pl->gc, pl->dbuf_state->drawing_planes);


    /*
     * Get length of name in pixels (with drawing area font) for future
     * use. (not correct if players use different fonts)
     */
    pl->name_length = XTextWidth(XQueryFont(pl->disp, XGContextFromGC(pl->gc)),
				 pl->name, strlen(pl->name));


    /*
     * A little hack that enables us to draw on both sets of double buffering
     * planes at once.
     */
    pl->dpl_1[0] = pl->dpl_2[0] = pl->dpl_1[1] = pl->dpl_2[0] = 0;

    for (p=0; p<2; p++) {
	pl->dpl_1[p] = pl->dpl_2[p] = 0;

	for (i=0; i<32; i++)
	    if (!((1<<i)&pl->dbuf_state->masks[p]))
		if (pl->dpl_1[p])
		    pl->dpl_2[p] = 1<<i;
		else
		    pl->dpl_1[p] = 1<<i;
    }

    return (SUCCESS);
}


void Alloc_msgs(int number)
{
    message_t *x=(message_t*)malloc(number*sizeof(message_t));
    int i;

    MsgBlock = x;
    for (i=0; i<number; i++) {
	Msg[i]=x;
	x->txt[0] = '\0';
	x->life = 0;
        x++;
    }
}


void Free_msgs(void)
{
    free(MsgBlock);
}


void DrawShadowText(int ind, Display *disp, Window w, GC gc, int x,
		    int start_y, char *str, Pixel fg, Pixel bg)
{
    char line[256];
    int i, y = start_y;


    if (str==NULL || *str=='\0')
	return;

    do {

	for (i=0; *str != '\0' && *str != '\n'; str++, i++)
	    line[i] = *str;
	line[i] = '\0';

	y += 20;

	ShadowDrawString(ind, disp, w, gc, x, y, line, fg, bg);

    } while (*(++str) != '\0');
}


void Expose_info_window(int ind)
{
    player *pl = Players[ind];


    DrawShadowText(ind, pl->disp, pl->info_w, pl->gctxt, 24, 14,
	   "\n\n"
	   "MISCELLANEOUS INFORMATION\n"
	   "\n\n"
	   "XPilot is still not a finished product, so please apology for\n"
	   "any bugs etc.  However, if you find any, we would greatly\n"
	   "appreciate that you reported to us.\n"
	   "\n"
	   "New versions are continuously being developed, but at a random\n"
	   "rate.  Currently, this isn't very fast at all, mainly due to the\n"
	   "mandatory work in conjunction with our studies (really!!).\n"
	   "\n"
	   "For more info, read the man pages for xpilot(6) and xpilots(6).\n"
	   "\n\n"
	   "Good luck as a future fighter-pilot,\n"
	   "Bjørn Stabell & Ken Ronny Schouten\n",
	   pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
}


void Expose_help_window(int ind)
{
    player *pl = Players[ind];


    if (BIT(pl->disp_type, DT_HAVE_COLOR)) {
	XSetForeground(pl->disp, pl->gctxt, pl->colors[BLUE].pixel);
	XFillRectangle(pl->disp, pl->help_w, pl->gctxt, 0, 0, 420, HH);
	XSetForeground(pl->disp, pl->gctxt, pl->colors[WHITE].pixel);
    } else
	XClearWindow(pl->disp, pl->help_w);
    
    switch (pl->help_page) {
    case 0:
	DrawShadowText(ind, pl->disp, pl->help_w, pl->gctxt, 15, 7,
		       "KEYS:\n"
		       "\n"
		       "A\n"
		       "S\n"
		       "F or CTRL\n"
		       "Space|META_R\n"
		       "SHIFT\n"
		       "Return\n"
		       "\n"
		       "HOME\n"
		       "SELECT|Up|Down\n"
		       "NEXT\n"
		       "PREV\n"
		       "'*'\'/'\n"
		       "'+'\'-'\n"
		       "Q\n"
		       "Tab\n"
		       "]\n"
		       "\\\n"
                       ";\n"
                       "'\n"
		       "[\n"
		       "BackSpace\n"
                       "W\n"
                       "E\n"
                       "R\n"
		       "I\n",
		       pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
	DrawShadowText(ind, pl->disp, pl->help_w, pl->gctxt, 180, 7,
		       "\n\n"
		       "Rotate left.\n"
		       "Rotate right.\n"
		       "Refueling.\n"
		       "Shield.\n"
		       "Thrust.\n"
		       "Fire.\n"
		       "\n"
		       "Change home base, if on new base.\n"
		       "Lock on nearest player, if in range.\n"
		       "Lock on next player.\n"
		       "Lock on previous player.\n"
		       "Increase/decrease engine power.\n"
		       "Increase/decrease rotation speed.\n"
		       "Self destruct.\n"
		       "Place mine.\n"
		       "Detach mine.\n"
		       "Fire smart missile.\n"
                       "Fire heat seeking missile.\n"
                       "Fire torpedo (missile without lock).\n"
		       "Trigger ECM.\n"
		       "Turn on/off cloak.\n"
                       "Switch to next tank.\n"
                       "Switch to previous tank.\n"
                       "Detach current tank.\n"
		       "Toggle id mode.\n",
		       pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
	break;

    case 1:
	DrawShadowText(ind, pl->disp, pl->help_w, pl->gctxt,20,7,
	       "GAME OBJECTIVE:\n"
	       "\n"
	       "The primary goal of the game is to collect points by\n"
	       "destroying friends and cannons with your guns.\n"
	       "\n"
	       "Another important task is to refuel your ship.	This is\n"
	       "vital because your engine, radar, guns and shields all\n"
	       "require fuel.  Some even work better the more fuel you\n"
	       "have aboard (mainly the radar).\n"
	       "\n"
	       "Scattered around the world you will find some stationary\n"
	       "objects resembling red triangles.  These are bonus items\n"
	       "that may help you in a multitude of ways; first, the most\n"
	       "normal item, is the fuel-packs labeled F, then there are\n"
	       "smart missiles labeled S, wide angle shots labeled W, and\n"
	       "lastly you may by lucky enough to find a cloaking device\n"
	       "labeled, you guessed it, C.\n"
	       "\n",
	       pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
	break;
    case HELP_PAGES:
    default:
	break;
    }
}


void Expose_button_window(int ind, int color, Window w)
{
    player *pl = Players[ind];


    if (BIT(pl->disp_type, DT_HAVE_COLOR)) {
	XSetForeground(pl->disp, pl->gcb, pl->colors[color].pixel);
	XFillRectangle(pl->disp, w, pl->gcb, 0, 0, 256, 22);
	XSetForeground(pl->disp, pl->gcb, pl->colors[WHITE].pixel);
    } else
	XClearWindow(pl->disp, w);

    if (w == pl->quit_b)
	ShadowDrawString(ind, pl->disp, w, pl->gcb, 24, 16, "QUIT", 
			 pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
    if (w == pl->info_b)
	ShadowDrawString(ind, pl->disp, w, pl->gcb, 24, 16, "INFO", 
			 pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
    if (w == pl->help_b)
	ShadowDrawString(ind, pl->disp, w, pl->gcb, 24, 16, "HELP", 
			 pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
    if (w == pl->info_close_b)
	ShadowDrawString(ind, pl->disp, w, pl->gcb, 20, 16, "CLOSE", 
			 pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
    if (w == pl->help_close_b)
	ShadowDrawString(ind, pl->disp, w, pl->gcb, 20, 16, "CLOSE", 
			 pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
    if (w == pl->help_next_b)
	ShadowDrawString(ind, pl->disp, w, pl->gcb, 24, 16, "NEXT", 
			 pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
    if (w == pl->help_prev_b)
	ShadowDrawString(ind, pl->disp, w, pl->gcb, 22, 16, "PREV",
			 pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
}


void Info(int ind, Window w)
{
    player *pl = Players[ind];

    
    if (pl->info_press && w != pl->info_close_b)
	return;
    else if (w == pl->info_b) {
	if (pl->help_press = true)
	    Help(ind, pl->help_close_b);

	pl->info_press = true;

	XMapWindow(pl->disp, pl->info_w);
	XMapSubwindows(pl->disp, pl->info_w);
    } else {
	pl->info_press = false;
	Expose_button_window(ind, RED, pl->info_b);
	XUnmapSubwindows(pl->disp, pl->info_w);
	XUnmapWindow(pl->disp, pl->info_w);
    }
}


void Help(int ind, Window w)
{
    player *pl = Players[ind];

    
    if (pl->help_press && w == pl->help_b) 
	return;
    else if (w == pl->help_b) {
	if (pl->info_press = true)
	    Info(ind, pl->info_close_b);

	pl->help_press = true;

	XMapWindow(pl->disp,pl->help_w);
	XMapSubwindows(pl->disp, pl->help_w);
    } else if (w == pl->help_close_b) {
	pl->help_press = false;
	pl->help_page = 0;
	Expose_button_window(ind, RED, pl->help_b);
	XUnmapSubwindows(pl->disp, pl->help_w);
	XUnmapWindow(pl->disp,pl->help_w);
    } else if (w == pl->help_next_b) {
	Expose_button_window(ind, RED, pl->help_next_b);
	pl->help_page++;
	if (pl->help_page == HELP_PAGES)
	    pl->help_page = 0;
	Expose_help_window(ind);
    } else if (w == pl->help_prev_b) {
	Expose_button_window(ind, RED, pl->help_prev_b);
	pl->help_page--;
	if (pl->help_page == -1)
	    pl->help_page = HELP_PAGES-1;
	Expose_help_window(ind);
    }
}


/*
 * Cleanup player structure, close the display etc.
 */
void Quit(int ind)
{
    player *pl = Players[ind];


    sprintf(msg, "%s left the game.", Players[ind]->name);
    Set_message(msg);

    if (pl->disp_type != DT_NONE) {
	Expose_button_window(ind, RED, Players[ind]->quit_b);

	XAutoRepeatOn(pl->disp);

	end_dbuff(ind, pl->dbuf_state);	    		/* Clean up */
	if (pl->colormap)
	    XFreeColormap(pl->disp, pl->colormap);
	XCloseDisplay(pl->disp);
    }

    Delete_player(ind);
    updateScores = true;
}


/*
 * This is quite rude, but it keeps the whole game from going down because
 * of just one user.
 */
int FatalError(Display *disp)
{
    int i;

#ifdef USE_SIGIO
    if(Check_new_players())
	longjmp(SavedEnv, 1);
#endif

    for (i=0; i<NumPlayers; i++)
	if (Players[i]->disp == disp)
	    break;

    if (i < NumPlayers) {	    /* Found the display. */
	printf("Player %s@%s did a nasty quit.\n",
	       Players[i]->name, DisplayString(Players[i]->disp));

	Quit(i);
	/*
	 * We got through Quit(), therefore there are some more players, just
	 * longjmp() back and continue... :)
	 */
	longjmp(SavedEnv, 1);

    } else {
	error("Fatal I/O error. Couldn't determine which player caused it");
    }
    return (0);
}



void Draw_score_table(void)
{
    int register i, ind;
    player *pl;


    for(ind=0; ind<NumPlayers; ind++) {
	pl = Players[ind];

	if (pl->disp_type == DT_NONE)
	    continue;

	XClearWindow(pl->disp, pl->players);
	
	for(i=0; i<NumPlayers; i++)
	    ShadowDrawString(ind, pl->disp, pl->players, pl->gcp,
                             1, 20+(20*i),
			     Players[i]->lblstr,
                             pl->colors[WHITE].pixel,
			     pl->colors[BLACK].pixel);
    }
}


/* board.c,v 1.3 1992/05/11 15:30:57 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include "pilot.h"
#include "map.h"
#include "draw.h"
#include "version.h"
#include "icon.h"
#include <X11/Shell.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#define FRAC(py)	    ((int)((py) * 1024.0/768.0))
#define ShadowDrawString(d, w, gc, x, y, str, fg, bg)	\
{							\
	XSetForeground(d,gc,bg);			\
	XDrawString(d,w,gc,x+1,y+1, str, strlen(str));	\
	XSetForeground(d,gc,fg);			\
	XDrawString(d,w,gc,x-1,y-1,str,strlen(str));	\
}

extern player	*Players[];
extern int	Antall;
extern World_map World;
extern message_t *Msg[];
extern server	Server;
extern int	Radar_height;
extern jmp_buf	Saved_env;

extern void Kill_player(int);
extern void Set_message(char *);
extern void Set_label_strings(void);
extern void End_game(void);

extern void dbuff_switch(dbuff_state_t *);
extern dbuff_state_t *start_dbuff(Display *, Colormap,
				  long, XColor *);
void	Quit(int);
int	FatalError(Display *);
void	Info(int, Window);
void	Help(int, Window);

static char msg[MSG_LEN];
static XFontStruct *font;
Atom	Protocol_atom, Kill_atom;

#ifndef NO_ROTATING_DASHES
char dashes[NO_OF_DASHES] = { 8, 4 };
#endif



bool Draw_board(int ind, char *display)
{
    int i, p;
    XGCValues xgc;
    player *pl=Players[ind];
    static Pixmap icon_xbm;
#ifdef	SCROLL
    char title[256];
#endif


    /*
     * Opens display.
     */
    if ((pl->disp=XOpenDisplay(display)) == NULL) {
	fprintf(stderr, "Couldn't open display to '%s'.\n", display);
	return (false);
    }

    /*
     * Initializes colors.
     */
    pl->colors[BLACK].red = 0; 
    pl->colors[BLACK].green = 0; 
    pl->colors[BLACK].blue = 0; 
    pl->colors[WHITE].red = 65535;
    pl->colors[WHITE].green = 65535;
    pl->colors[WHITE].blue = 65535;
    pl->colors[BLUE].red = 10000;
    pl->colors[BLUE].green = 30000;
    pl->colors[BLUE].blue = 65535;
    pl->colors[RED].red = 65535;
    pl->colors[RED].green = 15000;
    pl->colors[RED].blue = 10000;


    /*
     * Initializes the double buffering routine.
     */
    if (pl->dbuf_state=start_dbuff(pl->disp,
				   DefaultColormap(pl->disp,
						   DefaultScreen(pl->disp)),
				   2, pl->colors));
    if (pl->dbuf_state == NULL) {
	fprintf(stderr, "Couldn't initialize double buffering for '%s'.\n",
		display);
	XCloseDisplay(pl->disp);
	return (false);
    }


    /*
     * Creates the windows.
     */
    pl->top = XCreateSimpleWindow(pl->disp, RootWindow(pl->disp,0), 0, 0,
				  1026, 768, 0, 0, pl->colors[WHITE].pixel);
    pl->draw = XCreateSimpleWindow(pl->disp, pl->top, 258, 0,
				   768, 768, 0, 0, pl->colors[BLACK].pixel);
    pl->radar = XCreateSimpleWindow(pl->disp, pl->top, 0, 0,
				    256, Radar_height, 0, 0,
				    pl->colors[BLACK].pixel);
    pl->quit = XCreateSimpleWindow(pl->disp, pl->top, 0, Radar_height,
				   85, 22, 0, 0, pl->colors[RED].pixel);
    pl->info_b = XCreateSimpleWindow(pl->disp, pl->top, 86, Radar_height,
				     85, 22, 0, 0, pl->colors[RED].pixel);
    pl->help_b = XCreateSimpleWindow(pl->disp, pl->top, 172, Radar_height,
				     84, 22, 0, 0, pl->colors[RED].pixel);
    pl->players = XCreateSimpleWindow(pl->disp, pl->top, 0, Radar_height+22,
				      256, 746-Radar_height, 0, 0,
				      pl->colors[BLUE].pixel);

    pl->help_w = XCreateSimpleWindow(pl->disp, pl->draw, (FULL-420)/2,
				     (FULL-450)/2, 420, 450,
				     5, pl->colors[WHITE].pixel, 
				     pl->colors[BLUE].pixel);
    pl->help_close = XCreateSimpleWindow(pl->disp, pl->help_w, 4, 450-26,85,
					 22,0,0,pl->colors[RED].pixel);
    pl->help_next = XCreateSimpleWindow(pl->disp, pl->help_w, 420/2-85/2, 
					450-26,85,22,0,0,
					pl->colors[RED].pixel);
    pl->help_prev = XCreateSimpleWindow(pl->disp, pl->help_w, 420-89, 450-26,
					85,22,0,0,pl->colors[RED].pixel);

    pl->info_w = XCreateSimpleWindow(pl->disp, pl->draw, (FULL-500)/2, 
				     (FULL-500)/2, 500, 500,
				     5, pl->colors[WHITE].pixel, 
				     pl->colors[BLUE].pixel);
    pl->info_close = XCreateSimpleWindow(pl->disp, pl->info_w, 4, 500-26,85,
					 22,0,0,pl->colors[RED].pixel);

    /*
     * Maps the windows, makes the visible. Voila!
     */
    XMapWindow(pl->disp, pl->top);
    XMapWindow(pl->disp, pl->draw);
    XMapWindow(pl->disp, pl->radar);
    XMapWindow(pl->disp, pl->quit);
    XMapWindow(pl->disp, pl->info_b);
    XMapWindow(pl->disp, pl->help_b);
    XMapWindow(pl->disp, pl->players);

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
	    "wish you good luck... :)	   Who knows, you might neeed it? :) "
	    "								  ",
	    pl->name, Server.host, World.name, World.x, World.y, World.author,
	    World.rules->mode, World.Ant_cannon, World.Ant_gravs,
	    World.Ant_start, World.Ant_fuel, World.Ant_check);
    pl->scroll_len = strlen(pl->scroll);
#endif

    /*
     * Making bitmaps.
     */
    icon_xbm = XCreateBitmapFromData(pl->disp, pl->top,
				     icon_bits, icon_width, icon_height);

    /*
     * Prepare to catch the WM_DELETE_WINDOW 'event'.
     * (Should perhaps do a XGetWMProtocols first, but really not neccessary
     * for the moment.)	 Also add an error handler to catch the rest of those
     * nasty closes.
     */
    Protocol_atom = XInternAtom(pl->disp, "WM_PROTOCOLS", False);
    Kill_atom = XInternAtom(pl->disp, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(pl->disp, pl->top, &Kill_atom, 1);
    XSetIOErrorHandler(FatalError);


    /*
     * Setting up the window properties, icon etc.
     */
    XSetStandardProperties(pl->disp, pl->top, NULL, NULL, icon_xbm,
			   NULL, 0, NULL);  /* May specify argv and argc? */
    sprintf(msg, "You have successfully joined the server at \"%s\".",
	    Server.host);
    XStoreName(pl->disp, pl->top, msg);
    sprintf(msg, "%s:%s", pl->name, Server.host);
    XSetIconName(pl->disp, pl->top, msg);
    

    /*
     * Selecting events the we can handle.
     */
    XSelectInput(pl->disp, pl->top, KeyPressMask | KeyReleaseMask |
		 EnterWindowMask | ResizeRedirectMask |
		 EnterWindowMask | LeaveWindowMask);
    XSelectInput(pl->disp, pl->quit,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XSelectInput(pl->disp, pl->info_close,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XSelectInput(pl->disp, pl->help_close,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XSelectInput(pl->disp, pl->help_next,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XSelectInput(pl->disp, pl->help_prev,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XSelectInput(pl->disp, pl->info_b, ButtonPressMask | ExposureMask);
    XSelectInput(pl->disp, pl->help_b, ButtonPressMask | ExposureMask);
    XSelectInput(pl->disp, pl->info_w, ExposureMask);
    XSelectInput(pl->disp, pl->help_w, ExposureMask);
    XSelectInput(pl->disp, pl->radar, ExposureMask);
    XSelectInput(pl->disp, pl->players, ExposureMask);
    XSelectInput(pl->disp, pl->draw, 0);

    XAutoRepeatOff(pl->disp);	    /* We don't want any autofire, yet! */

 
    /*
     * Creates and initializes the graphic contexts.
     */
    pl->gc = XCreateGC(pl->disp, pl->draw, 0, &xgc);
    pl->gcr = XCreateGC(pl->disp, pl->radar, 0, &xgc);
    pl->gcb = XCreateGC(pl->disp, pl->quit, 0, &xgc);
    pl->gcp = XCreateGC(pl->disp, pl->players, 0, &xgc);
    pl->gctxt = XCreateGC(pl->disp, pl->info_w, 0, &xgc);

    XSetBackground(pl->disp, pl->gc, pl->colors[BLACK].pixel);
    XSetDashes(pl->disp, pl->gc, 0, dashes, NO_OF_DASHES);
    XSetLineAttributes(pl->disp, pl->gc, 0, LineSolid, CapButt, JoinBevel);
    pl->color = WHITE;

    if ((font = XLoadQueryFont(pl->disp,
	      "-*-*-medium-r-*-*-12-120-75-75-m-70-iso8859-1")) == NULL) {
	warn("Could not load font. Will use default font....\n");
    } else
	XSetFont(pl->disp, pl->gc, font->fid);

    if ((font = XLoadQueryFont(pl->disp,
	      "-*-*-bold-o-*-*-14-140-75-75-m-90-iso8859-1")) == NULL) {
	warn("Could not load font. Will use default font....\n");
    } else
	XSetFont(pl->disp, pl->gcb, font->fid);

    if ((font = XLoadQueryFont(pl->disp,
	      "-*-*-bold-i-*-*-17-120-100-100-p-86-iso8859-1")) == NULL) {
	warn("Could not load font. Will use default font....\n");
    } else
	XSetFont(pl->disp, pl->gctxt, font->fid);	

    if ((font = XLoadQueryFont(pl->disp,
	      "-*-*-bold-*-*-*-13-120-75-75-c-80-iso8859-1")) == NULL) {
	warn("Could not load font. Will use default font....\n");
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

    XSetPlaneMask(pl->disp, pl->gc,
		  pl->dbuf_state->drawing_planes);


    /*
     * Get length of name in pixels (with drawing area font) for future
     * use. (not nice if players use different fonts)
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

    return (true);
}



void Alloc_msgs(int number)
{
    int i;

    for (i=0; i<number; i++) {
	Msg[i] = (message_t *)malloc(sizeof(message_t));
	Msg[i]->txt[0] = '\0';
	Msg[i]->life = 0;
    }
}


void Free_msgs(int number)
{
    int i;

    for (i=0; i<number; i++)
	free(Msg[i]);
}


void DrawShadowText(Display *disp, Window w, GC gc, int x, int start_y,
		    char *str, Pixel fg, Pixel bg)
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

	ShadowDrawString(disp, w, gc, x, y, line, fg, bg);

    } while (*(++str) != '\0');
}


void Expose_info_window(int ind)
{
    player *pl = Players[ind];


    DrawShadowText(pl->disp, pl->info_w, pl->gctxt, 24, 14,
	   "\n\n"
	   "MISCELLANEOUS INFORMATION\n"
	   "\n\n"
	   "XPilot is still not a finished product, so please appology for\n"
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


    XSetForeground(pl->disp, pl->gctxt, pl->colors[BLUE].pixel);
    XFillRectangle(pl->disp, pl->help_w, pl->gctxt, 0, 0, 420, 450);
    XSetForeground(pl->disp, pl->gctxt, pl->colors[WHITE].pixel);
    
    switch (pl->help_page) {
    case 0:
	DrawShadowText(pl->disp, pl->help_w, pl->gctxt,15,7,
		       "KEYS:\n"
		       "\n"
		       "A\n"
		       "S\n"
		       "F or CTRL\n"
		       "Space or META_R\n"
		       "SHIFT\n"
		       "Return\n"
		       "\n"
		       "HOME\n"
		       "SELECT, Up or Down\n"
		       "NEXT\n"
		       "PREV\n"
		       "'*'\'/'\n"
		       "'+'\'-'\n"
		       "Q\n"
		       "\\\n"
		       "BackSpace\n"
		       "C\n"
		       "I\n",
		       pl->colors[WHITE].pixel,pl->colors[BLACK].pixel);
	DrawShadowText(pl->disp, pl->help_w, pl->gctxt,160,7,
		       "\n\n"
		       "Rotate left.\n"
		       "Rotate right.\n"
		       "Refuling.\n"
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
		       "Fire smart missile.\n"
		       "Turn on/off cloak.\n"
		       "Toggle lock computer.\n"
		       "Toggle id mode.\n",
		       pl->colors[WHITE].pixel,pl->colors[BLACK].pixel);
	break;

    case 1:
	DrawShadowText(pl->disp, pl->help_w, pl->gctxt,20,7,
	       "GAME OBJECTIVE:\n"
	       "\n"
	       "The primary goal of the game is to collect points by\n"
	       "destroying friends and cannons with your guns.\n"
	       "\n"
	       "Another important task is to refuel your ship.	This is\n"
	       "vital because both your engine, radar, guns and shields\n"
	       "are dependant on your ships fuel.  Some even work better\n"
	       "the more fuel you have aboard (mainly the radar).\n"
	       "\n"
	       "Scattered around the world you will find some stationary\n"
	       "objects resembeling red triangles.  These are bonus items\n"
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


    XSetForeground(pl->disp, pl->gcb, pl->colors[color].pixel);
    XFillRectangle(pl->disp, w, pl->gcb, 0, 0, 256, 22);
    XSetForeground(pl->disp, pl->gcb, pl->colors[WHITE].pixel);
    if (w == pl->quit)
	ShadowDrawString(pl->disp, w, pl->gcb, 24, 16, "QUIT", 
			 pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
    if (w == pl->info_b)
	ShadowDrawString(pl->disp, w, pl->gcb, 24, 16, "INFO", 
			 pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
    if (w == pl->help_b)
	ShadowDrawString(pl->disp, w, pl->gcb, 24, 16, "HELP", 
			 pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
    if (w == pl->info_close)
	ShadowDrawString(pl->disp, w, pl->gcb, 20, 16, "CLOSE", 
			 pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
    if (w == pl->help_close)
	ShadowDrawString(pl->disp, w, pl->gcb, 20, 16, "CLOSE", 
			 pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
    if (w == pl->help_next)
	ShadowDrawString(pl->disp, w, pl->gcb, 24, 16, "NEXT", 
			 pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
    if (w == pl->help_prev)
	ShadowDrawString(pl->disp, w, pl->gcb, 22, 16, "PREV",
			 pl->colors[WHITE].pixel, pl->colors[BLACK].pixel);
}


void Info(int ind, Window w)
{
    player *pl = Players[ind];

    
    if (pl->info_press && w != pl->info_close)
	return;
    else if (w == pl->info_b) {
	if (pl->help_press = true)
	    Help(ind, pl->help_close);

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
	    Info(ind, pl->info_close);

	pl->help_press = true;

	XMapWindow(pl->disp,pl->help_w);
	XMapSubwindows(pl->disp, pl->help_w);
    } else if (w == pl->help_close) {
	pl->help_press = false;
	pl->help_page = 0;
	Expose_button_window(ind, RED, pl->help_b);
	XUnmapSubwindows(pl->disp, pl->help_w);
	XUnmapWindow(pl->disp,pl->help_w);
    } else if (w == pl->help_next) {
	Expose_button_window(ind, RED, pl->help_next);
	pl->help_page++;
	if (pl->help_page == HELP_PAGES)
	    pl->help_page = 0;
	Expose_help_window(ind);
    } else if (w == pl->help_prev) {
	Expose_button_window(ind, RED, pl->help_prev);
	pl->help_page--;
	if (pl->help_page == -1)
	    pl->help_page = HELP_PAGES-1;
	Expose_help_window(ind);
    }
}


void Quit(int ind)
{
    player *pl = Players[ind];


    Expose_button_window(ind, RED, Players[ind]->quit);

    XAutoRepeatOn(pl->disp);

    sprintf(msg, "Player %s has left the game.", Players[ind]->name);
    Set_message(msg);

    end_dbuff(pl->dbuf_state);	    /* Clean up... */
    XCloseDisplay(pl->disp);
    Delete_player(ind);

    if (Antall > 0)
	Set_label_strings();
    else
	End_game();
}



/*
 * This is quite rude, but it keeps the whole game from going down because
 * of just one user.
 */
int FatalError(Display *disp)
{
    int i;


    for (i=0; i<Antall; i++)
	if (Players[i]->disp == disp)
	    break;

    if (i<Antall) {	    /* Found the display. */
	fprintf(stderr, "Player %s@%s did a nasty quit.\n",
		Players[i]->name, Players[i]->display);

	Quit(i);
	/*
	 * We got through Quit(), therefore there are some more players, just
	 * longjmp() back and continue... :)
	 */
	longjmp(Saved_env, 1);

    } else {
	fprintf(stderr, "Fatal I/O error, but couldn't determine which "
		"player caused it.\n");
    }
}



void Set_labels(void)
{
    int register i, x;
    player *pl;


    for(x=0;x<Antall;x++) {
	pl = Players[x];
	XSetPlaneMask(pl->disp, pl->gc,
		      pl->dbuf_state->drawing_planes);
	XSetForeground(pl->disp, pl->gcp, 
		       pl->colors[BLUE].pixel);
	
	XFillRectangle(pl->disp, pl->players,
		       pl->gcp, 0, 0, 256, 490);
	
	XSetForeground(pl->disp, pl->gcp, 
		       pl->colors[WHITE].pixel);
	
	for(i=0; i<Antall; i++)
	    ShadowDrawString(pl->disp,pl->players,pl->gcp,1,20 + (20*i),
			     Players[i]->lblstr ,pl->colors[WHITE].pixel,
			     pl->colors[BLACK].pixel);
    }
}

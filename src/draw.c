/* draw.c,v 1.3 1992/05/11 15:31:10 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <stdio.h>
#include "pilot.h"
#include "map.h"
#include "draw.h"
#include "version.h"
#include <math.h>

#define X(i, co)  ((int) ((co) - Players[i]->world.x))
#define Y(i, co)  ((int) (FULL-((co) - Players[i]->world.y)))

#define Draw_meter(_p, x, y, title, per)				\
{									\
    player *p=(_p);							\
    XSetForeground(p->disp, p->gc, p->colors[RED].pixel);		\
    XFillRectangle(p->disp, p->draw, p->gc, x+2, y+2,			\
		   (int)(298*per), 8);					\
    XSetForeground(p->disp, p->gc, p->colors[WHITE].pixel);		\
    XDrawRectangle(p->disp, p->draw, p->gc, x, y, 300, 10);		\
    XDrawLine(p->disp, p->draw, p->gc, x, y-4, x, y+14);		\
    XDrawLine(p->disp, p->draw, p->gc, x+300, y-3, x+300, y+13);	\
    XDrawLine(p->disp, p->draw, p->gc, x+150, y-3, x+150, y+13);	\
    XDrawLine(p->disp, p->draw, p->gc, x+75, y-1, x+75, y+11);		\
    XDrawLine(p->disp, p->draw, p->gc, x+225, y-1, x+225, y+11);	\
    XDrawString(p->disp, p->draw, p->gc, (x)+305, (y)+10,		\
		title, strlen(title));					\
}

#define UNKOWN_PLAYER	"* Unknown *"
#define NO_PLAYER	"** None **"

extern player *Players[];
extern int Antall;
extern int Shutdown;
extern wireobj ships[];
extern void Update_objects();
extern int Ant_Shots;
extern object *Shots[];
extern World_map World;
extern XColor  colors[];
extern double tbl_sin[];
extern int Radar_height;

void Draw_ships(int, int);
void Draw_shots(int, int);
void Draw_radar(int, int);
void Draw_world(int, int);
void Display_message(int, int);
void Display_compass(int, int);
void Draw_cannon(int, int);
void Draw_world_radar(int);
void Display_time(int, int);
void Display_fuel_gauge(int, int);
void Display_velocity_gauge(int, int);
void Display_power_gauge(int, int);
void Display_hud(int, int);
void Draw_meters(int, int);

static XPoint points[5];
static unsigned long loops=0;
XGCValues   gcv;

extern Pixmap tile_xbm;

message_t *Msg[MAX_MSGS];

XPoint	diamond[] = {
    { 0, -DSIZE },
    { DSIZE, -DSIZE },
    { -DSIZE, -DSIZE },
    { -DSIZE, DSIZE },
    { DSIZE, DSIZE }
};



#ifdef SCROLL
char *scroll(char *string, int start, int length)
{
    static char str[MAX_SCROLL_LEN];
    int i;
    
    
    for (i=0; string[i+start] && i<length; i++)
	str[i] = string[i+start];
    
    str[length]='\0';
    return (str);
}
#endif



void Draw_objects()
{
    static long scroll_i=0;
    int i, j;
    player *dr;
    
    
    if (++loops >= ULONG_MAX)	    /* Used for misc. timing purposes */
	loops=0;
    
#ifdef SCROLL
    if ((loops%SCROLL_DELAY) == 0)
	if (++scroll_i >= ULONG_MAX)
	    scroll_i=0;
#else
    if ((loops%TITLE_DELAY) == 0)
	scroll_i = !scroll_i;
#endif
    
    for(j=0; j<Antall; j++) {
	dr = Players[j];
	XSync(dr->disp, False);
	
	/*
	 * If status is GAME_OVER, the user may look through the other
	 * players 'eyes'.  This is done by using two indexes, one determining
	 * which data should be used (i, set below) and one determining where
	 * to draw it (j).
	 */
	if (BIT(dr->status, GAME_OVER) && (BIT(dr->status, PLAYING)) &&
	    (dr->lock.tagged==LOCK_PLAYER))
	    i = get_ind[dr->lock.pl_id];
	else
	    i = j;
	
#ifdef SCROLL
	/*
	 * Scroll a message in the window title.
	 */
	if ((loops%SCROLL_DELAY) == 0)
	    XStoreName(dr->disp, dr->top,
		       scroll(dr->scroll, scroll_i%dr->scroll_len, SCROLL_LEN));
#else
	/*
	 * Switch between two different window titles.
	 */
	if ((loops%TITLE_DELAY) == 0) {
	    if (scroll_i)
		XStoreName(dr->disp, dr->top, COPYRIGHT);
	    else
		XStoreName(dr->disp, dr->top, TITLE);
	    
	}
#endif
	
	gcv.dash_offset = 0;
	Display_hud(j, i);
	Draw_world(j, i);
	Draw_shots(j, i);
	gcv.dash_offset = DASHES_LENGTH-(loops%DASHES_LENGTH);
	Draw_ships(j, i);
	XSetForeground(dr->disp, dr->gc, dr->colors[WHITE].pixel);
	Draw_cannon(j, i);
	/*
	  old stuff, will be totally removed
	  if (BIT(Players[i]->used, OBJ_COMPASS))
	  Display_compass(j, i);
	  
	  if (BIT(Players[i]->status, VELOCITY_GAUGE))
	  Display_velocity_gauge(j, i);
	  
	  if (BIT(Players[i]->status, POWER_GAUGE))
	  Display_power_gauge(j, i);
	  
	  if (BIT(World.rules->mode, TIMING))
	  Display_time(j, i);
	  */
	if (BIT(Players[i]->instruments, SHOW_FUEL_GAUGE))
	    Display_fuel_gauge(j, i);
	
	Draw_meters(j, i);
	
	Display_message(j, i);
	
	Draw_radar(j, i);
	
	/*
	 * Now switch planes and clear the screen.
	 */
	dbuff_switch(dr->dbuf_state);
	XSetPlaneMask(dr->disp, dr->gc, dr->dbuf_state->drawing_planes);
	XSetForeground(dr->disp, dr->gc, dr->colors[BLACK].pixel);
	XFillRectangle(dr->disp, dr->draw, dr->gc, 0, 0, 768, 768);
	/*	XFlush(dr->disp);   */
    }
}


void Draw_ships(int draw, int data)
{
    int i;
    player *dr = Players[draw];
    player *pl = Players[data];
    player *pl_i;
    unsigned long mask=0;
    
    
    for(i=0; i<Antall; i++) {
	pl_i = Players[i];
	if (BIT(pl_i->status, PLAYING|PAUSE) &&
	    (!BIT(pl_i->status, GAME_OVER)) &&
	    (pl->world.x < pl_i->pos.x) &&
	    (pl->world.x+FULL > pl_i->pos.x) &&
	    (pl->world.y  < pl_i->pos.y) &&
	    (pl->world.y+FULL > pl_i->pos.y)) {
	    
	    if (BIT(pl_i->status, PAUSE)) {
		
		XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
		
		XFillRectangle(dr->disp, dr->draw, dr->gc,
			       X(data, pl_i->pos.x-10),
			       Y(data, pl_i->pos.y+13),
			       20, 26);
		
		XSetForeground(dr->disp, dr->gc, dr->colors[WHITE].pixel);
		
		if (pl_i->count<=0 || (loops%10)>=5) {
		    XDrawRectangle(dr->disp, dr->draw, dr->gc,
				   X(data, pl_i->pos.x-10),
				   Y(data, pl_i->pos.y+13),
				   20, 26);
		    
		    XDrawString(dr->disp, dr->draw, dr->gc,
				X(data, pl_i->pos.x-2),
				Y(data, pl_i->pos.y-4),
				"P", 1);
		}
		continue;
	    }
	    
	    if (BIT(pl_i->status, INVISIBLE) && i!=data && !TEAM(i, draw))
		continue;
	    
	    points[0].x = X(data, pl_i->pos.x + ships[pl_i->dir].pts[0].x);
	    points[0].y = Y(data, pl_i->pos.y + ships[pl_i->dir].pts[0].y);
	    points[1].x = X(data, pl_i->pos.x + ships[pl_i->dir].pts[1].x);
	    points[1].y = Y(data, pl_i->pos.y + ships[pl_i->dir].pts[1].y);
	    points[2].x = X(data, pl_i->pos.x + ships[pl_i->dir].pts[2].x);
	    points[2].y = Y(data, pl_i->pos.y + ships[pl_i->dir].pts[2].y);
	    points[3].x = X(data, pl_i->pos.x + ships[pl_i->dir].pts[0].x);
	    points[3].y = Y(data, pl_i->pos.y + ships[pl_i->dir].pts[0].y);
	    
	    XSetForeground(dr->disp, dr->gc, dr->colors[pl_i->color].pixel);
	    
	    /*
	     * Determine if you the name of the player should be drawn below
	     * his/her ship.
	     */
	    if ((BIT(pl->status, ID_MODE) && (data!=i)) ||
		((draw != data) && (i == data)))
		XDrawString(dr->disp, dr->draw, dr->gc,
			    X(data, pl_i->pos.x-pl_i->name_length/2),
			    Y(data,pl_i->pos.y-30),
			    pl_i->name, strlen(pl_i->name));
	    
	    if (BIT(pl_i->used, OBJ_REFUEL|OBJ_SHIELD) ||
		BIT(pl_i->status, INVISIBLE)) {
		
		mask = GCLineStyle;
		gcv.line_style = LineOnOffDash;
		
#ifndef NO_ROTATING_DASHES
		mask |= GCDashOffset;
#endif
		XChangeGC(dr->disp, dr->gc, mask, &gcv);
		
		if (BIT(pl_i->status, INVISIBLE))
		    XDrawLines(dr->disp, dr->draw, dr->gc, points, 4, 0);
		
		if (BIT(pl_i->used, OBJ_REFUEL)) {
		    XDrawLine(dr->disp, dr->draw, dr->gc,
			      X(data, World.fuel[pl_i->fs].pos.x),
			      Y(data, World.fuel[pl_i->fs].pos.y),
			      X(data, pl_i->pos.x),
			      Y(data, pl_i->pos.y));
		}
		
		if (BIT(pl_i->used, OBJ_SHIELD)) 
		    XDrawArc(dr->disp, dr->draw, dr->gc,
			     X(data,pl_i->pos.x - 17),
			     Y(data,pl_i->pos.y + 17),
			     34, 34, 0, 64 * 360);
	    } 
	    
	    gcv.line_style = LineSolid;
	    XChangeGC(dr->disp, dr->gc, GCLineStyle, &gcv);
	    
	    if (!BIT(pl_i->status, INVISIBLE)) {
		if (pl->lock.tagged==LOCK_PLAYER && get_ind[pl->lock.pl_id]==i)
		    XFillPolygon(dr->disp, dr->draw, dr->gc, points, 4,
				 Convex, CoordModeOrigin);
		else
		    XDrawLines(dr->disp, dr->draw, dr->gc, points, 4, 0);
	    }
	}
    }
}



void Draw_shots(int draw, int data)
{
    int i, x, y;
    char str[2];
    player *pl = Players[data];
    player *dr = Players[draw];
    object *shot;
    
    
    for(i=0; i<Ant_Shots; i++) {
	shot = Shots[i];
	
	if (shot->pos.x >= pl->world.x &&
	    shot->pos.x < pl->world.x+FULL &&
	    shot->pos.y >= pl->world.y &&
	    shot->pos.y < pl->world.y+FULL) {
	    
	    x = X(data, shot->pos.x);
	    y = Y(data, shot->pos.y);
	    
	    if ((shot->id!=-1) &&
		(TEAM(data, get_ind[shot->id])) && (shot->id != pl->id) &&
		(shot->type != OBJ_CANNON_DEBRIS)) 
		XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
	    else
		XSetForeground(dr->disp, dr->gc, dr->colors[shot->color].pixel);
	    
	    switch (shot->type) {
	    case OBJ_SHOT:  
	    case OBJ_SPARK:
	    case OBJ_DEBRIS:
	    case OBJ_CANNON_SHOT:
	    case OBJ_CANNON_DEBRIS:
		XDrawRectangle(dr->disp, dr->draw, dr->gc, x, y, 2, 2);
		break;
		
	    case OBJ_SMART_SHOT:
		XSetLineAttributes(dr->disp, dr->gc, 4,
				   LineSolid, CapButt, JoinMiter);
		
		XDrawLine(dr->disp, dr->draw, dr->gc,
			  x, y,
			  (int)(x-tcos(shot->dir)*SMART_SHOT_LEN),
			  (int)(y+tsin(shot->dir)*SMART_SHOT_LEN));
		
		XSetLineAttributes(dr->disp, dr->gc, 0,
				   LineSolid, CapButt, JoinMiter);
		break;
		
	    case OBJ_MINE:
		XDrawArc(dr->disp, dr->draw, dr->gc,
			 x, y, 8, 8, 0, 64 * 360);
		break;
	    case OBJ_WIDEANGLE_SHOT:
		str[0] = 'W';
		goto draw;
		break;
	    case OBJ_REAR_SHOT:
		str[0] = 'R';
		goto draw;
		break;
	    case OBJ_SMART_SHOT_PACK:
		str[0] = 'S';
		goto draw;
		break;
	    case OBJ_ENERGY_PACK:
		str[0] = 'F';
		goto draw;
		break;
	    case OBJ_MINE_PACK:
		str[0] = 'M';
		goto draw;
		break;
	    case OBJ_CLOAKING_DEVICE:
		str[0] = 'C';
	    draw:
		points[0].x = x-10;
		points[0].y = y+10;
		points[1].x = x;
		points[1].y = y-10;
		points[2].x = x+10;
		points[2].y = y+10;
		points[3].x = x-10;
		points[3].y = y+10;
		
		XDrawLines(dr->disp, dr->draw, dr->gc,
			   points, 4, CoordModeOrigin);
		str[1] = '\0';
		XDrawString(dr->disp, dr->draw, dr->gc, x-3, y+8, str, 1);
		break;
		
	    default:
		warn("Draw_shots: Shot type not defined.");
		break;
	    }
	}
    }
}


void Draw_meters(int draw, int data)
{
    player *dr=Players[draw], *pl=Players[data];
    
    
    if (BIT(pl->instruments, SHOW_FUEL_METER) ||
	(pl->fuel_count && !BIT(pl->instruments, SHOW_FUEL_GAUGE)))
	Draw_meter(dr, 10, 10, "Fuel", (pl->fuel/pl->max_fuel));
    if (BIT(pl->instruments, SHOW_POWER_METER) || pl->control_count)
	Draw_meter(dr, 10, 40, "Power", ((double)pl->power/MAX_PLAYER_POWER));
    if (BIT(pl->instruments, SHOW_TURNSPEED_METER) || pl->control_count)
	Draw_meter(dr, 10, 60, "Turnspeed",
		   ((double)pl->turnspeed/MAX_PLAYER_TURNSPEED));
    
    if (BIT(pl->status, SELF_DESTRUCT) && pl->count>0)
	Draw_meter(dr, (FULL-300)/2 -32, 3*FULL/4,
		   "Self destructing", pl->count/150.0);
    if (Shutdown != -1)
	Draw_meter(dr, (FULL-300)/2 -32, 4*FULL/5,
		   "SHUTDOWN", (double)Shutdown/(double)SHUTDOWN_DELAY);
    
    /*	Draw_meter(dr, 10, 30, "Shots", ((double)pl->shots/pl->shot_max));  */
}

void Display_hud(int draw, int data)
{
    player *dr=Players[draw], *pl=Players[data];
    player *target=Players[get_ind[dr->lock.pl_id]];
    int size, vert_pos;
    char str[20];
    double dx, dy, theta;
    
    
    XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
    
    if (!BIT(pl->instruments, SHOW_HUD_INSTRUMENTS))
	return;
    
    /*
     * HUD frame.
     */
    gcv.line_style = LineOnOffDash;
    XChangeGC(dr->disp, dr->gc, GCLineStyle | GCDashOffset, &gcv);
    
    if (BIT(pl->instruments, SHOW_HUD_HORIZONTAL)) {
	XDrawLine(dr->disp, dr->draw, dr->gc,
		  CENTER-HUD_SIZE, CENTER-HUD_SIZE+HUD_OFFSET,
		  CENTER+HUD_SIZE, CENTER-HUD_SIZE+HUD_OFFSET);
	XDrawLine(dr->disp, dr->draw, dr->gc,
		  CENTER-HUD_SIZE, CENTER+HUD_SIZE-HUD_OFFSET,
		  CENTER+HUD_SIZE, CENTER+HUD_SIZE-HUD_OFFSET);
    }
    if (BIT(pl->instruments, SHOW_HUD_VERTICAL)) {
	XDrawLine(dr->disp, dr->draw, dr->gc,
		  CENTER-HUD_SIZE+HUD_OFFSET, CENTER-HUD_SIZE, 
		  CENTER-HUD_SIZE+HUD_OFFSET, CENTER+HUD_SIZE);
	XDrawLine(dr->disp, dr->draw, dr->gc,
		  CENTER+HUD_SIZE-HUD_OFFSET, CENTER-HUD_SIZE,
		  CENTER+HUD_SIZE-HUD_OFFSET, CENTER+HUD_SIZE);
    }
    gcv.line_style = LineSolid;
    XChangeGC(dr->disp, dr->gc, GCLineStyle, &gcv);
    
    
    /*
     * Special items.
     */
    vert_pos=CENTER-HUD_SIZE+HUD_OFFSET;
    
    if (BIT(pl->have, OBJ_CLOAKING_DEVICE)) {
	vert_pos+=HUD_ITEMS_SPACE;
	str[0]='C';
	XDrawString(dr->disp, dr->draw, dr->gc,
		    CENTER-HUD_SIZE+5, vert_pos,
		    str, 1);
    }
    if (pl->extra_shots>0) {
	vert_pos+=HUD_ITEMS_SPACE;
	str[0]='W';
	XDrawString(dr->disp, dr->draw, dr->gc,
		    CENTER-HUD_SIZE+5, vert_pos,
		    str, 1);
    }
    if (BIT(pl->have, OBJ_REAR_SHOT)) {
	vert_pos+=HUD_ITEMS_SPACE;
	str[0]='R';
	XDrawString(dr->disp, dr->draw, dr->gc,
		    CENTER-HUD_SIZE+5, vert_pos,
		    str, 1);
    }
    
    
    /*
     * Missiles, mines and fuel display (count)
     */
    if (pl->mines > 0) {
	sprintf(str, "M%02d", pl->mines);
	XDrawString(dr->disp, dr->draw, dr->gc, CENTER-HUD_SIZE-4,
		    CENTER-HUD_SIZE+HUD_OFFSET/2+4, str, 4);
    }
    if (pl->missiles > 0) {
	sprintf(str, "S%02d", pl->missiles);
	XDrawString(dr->disp, dr->draw, dr->gc, CENTER-HUD_SIZE-4,
		    CENTER+HUD_SIZE-HUD_OFFSET/2+6, str, 4);
    }
    /* Fuel notify, HUD meter on */
    if (pl->fuel_count || pl->fuel<pl->def.fuel3) {
	sprintf(str, "%04.0f", pl->fuel);
	XDrawString(dr->disp, dr->draw, dr->gc, CENTER+HUD_SIZE-HUD_OFFSET+2,
		    CENTER+HUD_SIZE-HUD_OFFSET/2+6, str, 4);
    }
    
    
    /*
     * Display direction arrow and miscellaneous target information.
     */
    switch (pl->lock.tagged) {
    case LOCK_PLAYER:
	dx=(target->pos.x - pl->pos.x);
	dy=(target->pos.y - pl->pos.y);
	if ((BIT(World.rules->mode, LIMITED_VISIBILITY) &&
	     (pl->lock.distance > pl->sensor_range)) ||
	    (BIT(target->status, INVISIBLE) &&
	     !TEAM(data, get_ind[dr->lock.pl_id])) ||
	    (dy==0.0 && dx==0.0))
	    goto no_target;
	sprintf(str, "%03d", (int)(pl->lock.distance/WORLD_SPACE));
	XDrawString(dr->disp, dr->draw, dr->gc,
		    CENTER+HUD_SIZE-HUD_OFFSET+2,
		    CENTER-HUD_SIZE+HUD_OFFSET/2+3, str, 3);
	break;
	
    case LOCK_NONE:
	goto no_target;
	break;
	
    default:
	warn("Unkown pl->lock.tagged.\n");
	break;
    }
    
    theta=atan2(dy, dx);	/* Only works for LOCK_PLAYER */
    
    if ((pl->lock.distance>WARNING_DISTANCE) || ((loops%2)==0)) {
	size = 10000/(800+pl->lock.distance);
	if (TEAM(data, get_ind[pl->lock.pl_id]))
	    XDrawArc(dr->disp, dr->draw, dr->gc,
		     (int)(CENTER+HUD_SIZE*0.6*cos(theta)),
		     (int)(CENTER-HUD_SIZE*0.6*sin(theta)),
		     size, size, 0, 64*360);
	else
	    XFillArc(dr->disp, dr->draw, dr->gc,
		     (int)(CENTER+HUD_SIZE*0.6*cos(theta)),
		     (int)(CENTER-HUD_SIZE*0.6*sin(theta)),
		     size, size, 0, 64*360);
    }
    
    size=strlen(target->name);
    XDrawString(dr->disp, dr->draw, dr->gc,
		CENTER-target->name_length/2, CENTER-HUD_SIZE+HUD_OFFSET/2+3,
		target->name, size);
    
    
    /* Fuel gauge, must be last */
 no_target:
    
    if (!((pl->fuel_count) ||
	  (pl->fuel<pl->def.fuel3 &&
	   ((pl->fuel<pl->def.fuel1 && (loops%4)<2) ||
	    (pl->fuel<pl->def.fuel2 && pl->fuel>pl->def.fuel1 && (loops%8)<4) ||
	    (pl->fuel>pl->def.fuel2)))))
	return;
    
    XDrawRectangle(dr->disp, dr->draw, dr->gc,
		   CENTER+HUD_SIZE-HUD_OFFSET+FUEL_GAUGE_OFFSET-1,
		   CENTER-HUD_SIZE+HUD_OFFSET+FUEL_GAUGE_OFFSET-1,
		   HUD_OFFSET-(2*FUEL_GAUGE_OFFSET)+2,
		   HUD_FUEL_GAUGE_SIZE+2);
    
    size = pl->fuel/pl->max_fuel*HUD_FUEL_GAUGE_SIZE;
    XFillRectangle(dr->disp, dr->draw, dr->gc,
		   CENTER+HUD_SIZE-HUD_OFFSET+FUEL_GAUGE_OFFSET,
		   CENTER-HUD_SIZE+HUD_OFFSET+FUEL_GAUGE_OFFSET
		   + (HUD_FUEL_GAUGE_SIZE-size),
		   HUD_OFFSET-(2*FUEL_GAUGE_OFFSET), size);
    
    XSetFillStyle(dr->disp, dr->gc, FillSolid);		/* Reset fill style */
}


void Display_fuel_gauge(int draw, int data)
{
    int percent;
    player *pl = Players[data];
    player *dr = Players[draw];
    
    
    XSetForeground(dr->disp, dr->gc, dr->colors[WHITE].pixel);
    
    XDrawArc(dr->disp, dr->draw, dr->gc,
	     730 - 120, 36 - 30,
	     60, 60, 0, 64 * 360);
    
    XSetForeground(dr->disp, dr->gc, dr->colors[RED].pixel);
    
    percent = 64*(pl->fuel/pl->max_fuel)*360;
    XFillArc(dr->disp, dr->draw, dr->gc,
	     730 - 119, 36 - 29,
	     58, 58, 90*64 - percent,
	     (int)percent);
    
    XSetForeground(dr->disp, dr->gc, dr->colors[WHITE].pixel);
    
    XDrawLine(dr->disp, dr->draw, dr->gc,
	      730-120+30, 36, 730-120+30, 6);
}



void Display_velocity_gauge(int draw, int data)
{
    double theta;
    player *dr = Players[draw];
    player *pl = Players[data];
    
    
    XSetForeground(dr->disp, dr->gc, 
		   dr->colors[WHITE].pixel);
    
    XDrawArc(dr->disp, dr->draw, 
	     dr->gc,
	     730 - 210, 36 - 30,
	     60, 60, 0, 64 * 360);	
    
    XDrawLine(dr->disp, dr->draw,
	      dr->gc, 730-210+30, 36, 730-210+30, 6);
    
    
    XSetForeground(dr->disp, dr->gc, 
		   dr->colors[RED].pixel);
    
    theta = 5*PI/2 - ((pl->velocity/pl->max_speed)*2*PI);
    XDrawLine(dr->disp, dr->draw, dr->gc, 730-210+30, 36,
	      (int)(730-210+30+25*cos(theta)), (int)(36 - 25*sin(theta)));
    
}


void Display_power_gauge(int draw, int data)
{
    int percent;
    player *dr = Players[draw];
    
    
    XSetForeground(dr->disp, dr->gc, dr->colors[WHITE].pixel);
    
    XDrawArc(dr->disp, dr->draw, dr->gc,
	     730 - 300, 36 - 30,
	     60, 60, 0, 64 * 360);  
    
    XSetForeground(dr->disp, dr->gc, dr->colors[RED].pixel);
    
    percent=64*(Players[data]->power/MAX_PLAYER_POWER)*360;
    XFillArc(dr->disp, dr->draw, dr->gc,
	     730 - 299, 36 - 29,
	     58, 58, 90*64 - percent,
	     (int)percent);
    
    XSetForeground(dr->disp, dr->gc, dr->colors[WHITE].pixel);
    
    XDrawLine(dr->disp, dr->draw, dr->gc, 730-120+30, 36, 730-120+30, 6);
}


void Display_compass(int draw, int data)
{
    char string[30];
    double dx=0.0, dy=0.0, theta;
    player *pl = Players[data];
    player *dr = Players[draw];
    int len, pl_ind = get_ind[pl->lock.pl_id];
    
    
    switch (pl->lock.tagged) {
	/*  case LOCK_LOCATION::
	    dx=(pl->lock.pos.x - pl->pos.x);
	    dy=(pl->lock.pos.y - pl->pos.y);
	    sprintf(string, "%s: %4.0lf", pl->lock.name, LENGTH(dx, dy));
	    break;
	    */
    case LOCK_NONE:
	strcpy(string, NO_PLAYER);
	pl->lock.distance = DBL_MAX;
	break;
	
    case LOCK_PLAYER:
	dx = (Players[pl_ind]->pos.x - pl->pos.x);
	dy = (Players[pl_ind]->pos.y - pl->pos.y);
	if ((BIT(World.rules->mode, LIMITED_VISIBILITY) &&
	     (pl->lock.distance > pl->sensor_range)) ||
	    (BIT(Players[pl_ind]->status, INVISIBLE) &&
	     !TEAM(draw, get_ind[dr->lock.pl_id]))) {
	    strcpy(string, UNKOWN_PLAYER);
	} else {
	    sprintf(string, "%s: %5.2lf",
		    Players[pl_ind]->name, pl->lock.distance/WORLD_SPACE);
	}
	break;
	
    default:
	warn("Unkown pl->lock.tagged.\n");
	break;
    }
    len=strlen(string);
    
    if (TEAM(draw, get_ind[dr->lock.pl_id]) && 
	pl->lock.tagged != LOCK_NONE)
	XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
    
    XDrawArc(dr->disp, dr->draw, dr->gc, 730-30, 36-30, 60, 60, 0, 64*360);
    
    if ((dy != 0.0) || (dx != 0.0)) {
	if (((BIT(World.rules->mode, LIMITED_VISIBILITY)) &&
	     (pl->lock.distance>pl->sensor_range)))
	    XDrawPoint(dr->disp, dr->draw, dr->gc, 730, 36);
	else {
	    if (BIT(Players[pl_ind]->status, INVISIBLE) &&
		!TEAM(draw, get_ind[dr->lock.pl_id]))
		theta = 2*PI*(double)rand()/RAND_MAX;
	    else
		theta = atan2(dy, dx);
	    XDrawLine(dr->disp, dr->draw, dr->gc, 730, 36,
		      (int)(730 + 25*cos(theta)),
		      (int)(36 - 25*sin(theta)));
	}
    } else
	theta = 0.0;
    
    if (pl->lock.distance < WARNING_DISTANCE)
	XSetForeground(dr->disp, dr->gc, dr->colors[RED].pixel);
    
    XDrawString(dr->disp, dr->draw, dr->gc, 720-(4*len), 100, string, len);
}



void Draw_cannon(int draw, int data)
{
    player *pl = Players[data];
    player *dr = Players[draw];
    int i;
    
    
    for (i=0; i<World.Ant_cannon; i++) {
	if ((pl->world.x < (WORLD_SPACE*World.cannon[i].pos.x)) &&
	    (pl->world.x+FULL > (WORLD_SPACE*World.cannon[i].pos.x)) &&
	    (pl->world.y  < (WORLD_SPACE*World.cannon[i].pos.y)) &&
	    (pl->world.y+FULL > (WORLD_SPACE*World.cannon[i].pos.y)) &&
	    (World.cannon[i].dead_time == 0)) {
	    
	    if (World.cannon[i].dir == UP) {
		points[0].x = World.cannon[i].pos.x*WORLD_SPACE;
		points[0].y = World.cannon[i].pos.y*WORLD_SPACE;
		points[1].x = World.cannon[i].pos.x*WORLD_SPACE+WORLD_SPACE;
		points[1].y = World.cannon[i].pos.y*WORLD_SPACE;
		points[2].x = World.cannon[i].pos.x*WORLD_SPACE+WORLD_SPACE/2;
		points[2].y = World.cannon[i].pos.y*WORLD_SPACE+WORLD_SPACE/3;
	    }
	    if (World.cannon[i].dir == DOWN) {
		points[0].x = World.cannon[i].pos.x*WORLD_SPACE;
		points[0].y = World.cannon[i].pos.y*WORLD_SPACE+WORLD_SPACE;
		points[1].x = World.cannon[i].pos.x*WORLD_SPACE+WORLD_SPACE;
		points[1].y = World.cannon[i].pos.y*WORLD_SPACE+WORLD_SPACE;
		points[2].x = World.cannon[i].pos.x*WORLD_SPACE+WORLD_SPACE/2;
		points[2].y = World.cannon[i].pos.y*WORLD_SPACE+2*WORLD_SPACE/3;
	    }
	    if (World.cannon[i].dir == RIGHT) {
		points[0].x = World.cannon[i].pos.x*WORLD_SPACE;
		points[0].y = World.cannon[i].pos.y*WORLD_SPACE;
		points[1].x = World.cannon[i].pos.x*WORLD_SPACE;
		points[1].y = World.cannon[i].pos.y*WORLD_SPACE+WORLD_SPACE;
		points[2].x = World.cannon[i].pos.x*WORLD_SPACE+WORLD_SPACE/3;
		points[2].y = World.cannon[i].pos.y*WORLD_SPACE+WORLD_SPACE/2;
	    }
	    if (World.cannon[i].dir == LEFT) {
		points[0].x = World.cannon[i].pos.x*WORLD_SPACE+WORLD_SPACE;
		points[0].y = World.cannon[i].pos.y*WORLD_SPACE;
		points[1].x = World.cannon[i].pos.x*WORLD_SPACE+WORLD_SPACE;
		points[1].y = World.cannon[i].pos.y*WORLD_SPACE+WORLD_SPACE;
		points[2].x = World.cannon[i].pos.x*WORLD_SPACE+2*WORLD_SPACE/3;
		points[2].y = World.cannon[i].pos.y*WORLD_SPACE+WORLD_SPACE/2;
	    }
	    
	    points[0].x = X(data, points[0].x);
	    points[1].x = X(data, points[1].x);
	    points[2].x = X(data, points[2].x);
	    points[3].x = points[0].x;
	    
	    points[0].y = Y(data, points[0].y);
	    points[1].y = Y(data, points[1].y);
	    points[2].y = Y(data, points[2].y);
	    points[3].y = points[0].y;
	    
	    XDrawLines(dr->disp, dr->draw, dr->gc, points, 4, 0);
	    
	    World.cannon[i].active = true;
	}
    }
}



void Display_message(int draw, int ind)
{
    player *dr = Players[draw];
    int i;
    
    
    for (i=0; i<MAX_MSGS; i++) {
	if (Msg[i]->life-- > MSG_FLASH)
	    XSetForeground(dr->disp, dr->gc, dr->colors[RED].pixel);
	else if (Msg[i]->life > 0)
	    XSetForeground(dr->disp, dr->gc, dr->colors[WHITE].pixel);
	else
	    Msg[i]->txt[0]='\0';
	
	XDrawString(dr->disp, dr->draw, dr->gc, 10, 755-(13*i),
		    Msg[i]->txt, strlen(Msg[i]->txt));
    }
}



void Set_message(char *message)
{
    int i;
    message_t *tmp;
    
    
    tmp = Msg[MAX_MSGS-1];
    for (i=MAX_MSGS-1; i>0; i--)
	Msg[i] = Msg[i-1];
    
    Msg[0] = tmp;
    
    Msg[0]->life = MSG_DURATION;
    strcpy(Msg[0]->txt, message);
}



void Draw_radar(int draw, int data)
{
    int i;
    player *pl = Players[data];
    player *dr = Players[draw];
    
    
    XSetForeground(dr->disp, dr->gcr,
		   dr->colors[BLACK].pixel);
    XFillRectangle(dr->disp, dr->radar,
		   dr->gcr, 0, 0, 256, Radar_height);
    /* Clear radar */
    
    if (BIT(pl->mode, TIMING)) {
	XSetForeground(dr->disp,
		       dr->gcr, /* Checkpoint */
		       dr->colors[WHITE].pixel);
	
	diamond[0].x = 256*World.check[pl->check].x/World.x;
	diamond[0].y = Radar_height -
	    Radar_height*World.check[pl->check].y/World.y+DSIZE;
	XDrawLines(dr->disp, dr->radar,
		   dr->gcr, diamond,
		   5, CoordModePrevious);
    }
    
    for(i=0; i<Antall; i++) {
	if (BIT(Players[i]->status, PLAYING) &&
	    !BIT(Players[i]->status, GAME_OVER)) {
	    
	    if ((BIT(World.rules->mode, LIMITED_VISIBILITY) &&
		 (LENGTH(pl->pos.x-Players[i]->pos.x,
			 pl->pos.y-Players[i]->pos.y) > pl->sensor_range)) ||
		(BIT(Players[i]->status, INVISIBLE) && i!=data))
		continue;
	    
	    if (data == i) {					/* You */
		
		if (loops%10 >= 8)
		    XSetForeground(dr->disp, dr->gcr, dr->colors[BLACK].pixel);
		else
		    XSetForeground(dr->disp, dr->gcr, dr->colors[WHITE].pixel);
		
	    } else if ((get_ind[pl->lock.pl_id]==i) &&
		       BIT(pl->used, OBJ_COMPASS) &&
		       (pl->lock.tagged==LOCK_PLAYER)) {	/* Tagged */
		
		if (loops%4 >= 3)
		    XSetForeground(dr->disp, dr->gcr, dr->colors[BLACK].pixel);
		else
		    XSetForeground(dr->disp, dr->gcr, dr->colors[WHITE].pixel);
	    } else
		XSetForeground(dr->disp, dr->gcr, dr->colors[WHITE].pixel);
	    
	    XDrawRectangle(dr->disp, dr->radar, dr->gcr,
			   (int)(256*(Players[i]->pos.x/
				      (World.x*WORLD_SPACE))),
			   (int)(Radar_height -
				 (Radar_height*(Players[i]->pos.y/
						(World.y*WORLD_SPACE)))),
			   2, 2);
	}
    }
}



void Display_time(int draw, int data)
{
    static char string[MAX_STATUS_CHARS];
    static char lap_name[MAX_CHARS] = "NOBODY";
    static char run_name[MAX_CHARS] = "NOBODY";
    static int best_lap = 0;
    static int best_run = 0;
    int i;
    player *pl_i;
    player *pl = Players[data];
    player *dr = Players[draw];
    
    
    XSetForeground(dr->disp, dr->gcr, dr->colors[WHITE].pixel);
    
    sprintf(string, "Time: %-5.2lf	  Round: %d	  Last time: %-5.2lf",
	    pl->time/100.0, pl->round, pl->last_time/100.0);
    XDrawString(dr->disp, dr->draw, dr->gc, 10, 30, string, strlen(string));	
    
    sprintf(string, "Best time: %-5.2lf	     "
	    "Best round: %-5.2lf      Last lap: %-5.2lf",
	    pl->best_run/100.0, pl->best_lap/100.0, pl->last_lap_time/100.0);
    XDrawString(dr->disp, dr->draw, dr->gc, 10, 50, string, strlen(string));	
    
    if (!(BIT(World.rules->mode, ONE_PLAYER_ONLY))) {
	for (i=0;i < Antall; i++) {
	    pl_i = Players[i];
	    if (((pl_i->best_lap < best_lap) || (best_lap == 0)) &&
		(pl_i->best_lap != 0)) {
		best_lap = pl_i->best_lap;
		strcpy(lap_name, pl_i->name);
	    }
	    if (((pl_i->best_run < best_run) || (best_run == 0)) &&
		(pl_i->best_run != 0)) {
		best_run = pl_i->best_run;
		strcpy(run_name, pl_i->name);
	    }
	}
	sprintf(string,
		"Best time by %s : %-5.2lf    Best round by %s: %-5.2lf",
		run_name, best_run/100.0, lap_name, best_lap/100.0);
	XDrawString(dr->disp, dr->draw, dr->gc, 10, 70, string, strlen(string));
    }
}



void Draw_world(int draw, int data)
{
    int xi, yi, xb, yb, i, offset;
    double x, y;
    static const int WS_PR_SC=1+(double)FULL/WORLD_SPACE;
    static const int INSIDE_WS=WORLD_SPACE-2;
    player *pl = Players[data];
    player *dr = Players[draw];
    
    
    xb = (pl->world.x/WORLD_SPACE); LIMIT(xb, 0, World.x-1);
    yb = (pl->world.y/WORLD_SPACE); LIMIT(yb, 0, World.y-1);

    for (xi=xb; xi<=(WS_PR_SC+xb) && xi<World.x; xi++)
	for (yi=yb; yi<=(WS_PR_SC+yb) && yi<World.y; yi++) {
	    x = (xi*WORLD_SPACE); y=(yi*WORLD_SPACE);
	    
	    switch (World.type[xi][yi]) {
		
	    case FILLED:
		XDrawRectangle(dr->disp, dr->draw, dr->gc,
			       X(data, x), Y(data, y+WORLD_SPACE),
			       WORLD_SPACE, WORLD_SPACE);
		break;
		
	    case CHECK:
		points[0].x = X(data, x+(WORLD_SPACE/2));
		points[0].y = Y(data, y);
		points[1].x = X(data, x);
		points[1].y = Y(data, (y+WORLD_SPACE/2));
		points[2].x = X(data, (x+WORLD_SPACE/2));
		points[2].y = Y(data, (y+WORLD_SPACE));
		points[3].x = X(data, x+WORLD_SPACE);
		points[3].y = Y(data, y+(WORLD_SPACE/2));
		points[4].x = X(data, x+(WORLD_SPACE/2));
		points[4].y = Y(data, y);
		
		for(i=0; i<World.Ant_check; i++)
		    if ((World.check[i].x == xi) && (World.check[i].y == yi)) {
			if (i != pl->check) {
			    XDrawLines(dr->disp, dr->draw, dr->gc,
				       points, 5, 0); 
			} else {
			    XFillPolygon(dr->disp, dr->draw, dr->gc,
					 points, 5, Convex, CoordModeOrigin);
			}
		    }
		break;
		
	    case REC_LU:
		points[0].x = X(data, x);
		points[0].y = Y(data, y);
		points[1].x = X(data, x);
		points[1].y = Y(data, (y+WORLD_SPACE));
		points[2].x = X(data, (x+WORLD_SPACE));
		points[2].y = Y(data, (y+WORLD_SPACE));
		points[3].x = X(data, x);
		points[3].y = Y(data, y);
		XDrawLines(dr->disp, dr->draw, dr->gc, points, 4, 0); 
		break;
		
	    case REC_RU:
		points[0].x = X(data, x);
		points[0].y = Y(data, (y+WORLD_SPACE));
		points[1].x = X(data, (x+WORLD_SPACE));
		points[1].y = Y(data, (y+WORLD_SPACE));
		points[2].x = X(data, (x+WORLD_SPACE));
		points[2].y = Y(data, y);
		points[3].x = X(data, x);
		points[3].y = Y(data, (y+WORLD_SPACE));
		XDrawLines(dr->disp, dr->draw, dr->gc, points, 4, 0); 
		break;
		
	    case REC_LD:
		points[0].x = X(data, x);
		points[0].y = Y(data, y);
		points[1].x = X(data, x);
		points[1].y = Y(data, (y+WORLD_SPACE));
		points[2].x = X(data, (x+WORLD_SPACE));
		points[2].y = Y(data, y);
		points[3].x = X(data, x);
		points[3].y = Y(data, y);
		XDrawLines(dr->disp, dr->draw, dr->gc, points, 4, 0); 
		break;
		
	    case REC_RD:
		points[0].x = X(data, x);
		points[0].y = Y(data, y);
		points[1].x = X(data, (x+WORLD_SPACE));
		points[1].y = Y(data, (y+WORLD_SPACE));
		points[2].x = X(data, (x+WORLD_SPACE));
		points[2].y = Y(data, y);
		points[3].x = X(data, x);
		points[3].y = Y(data, y);
		XDrawLines(dr->disp, dr->draw, dr->gc, points, 4, 0); 
		break;
		
	    case FUEL:
		for(i=0; i<World.Ant_fuel; i++) {
		    if (((int) (World.fuel[i].pos.x/WORLD_SPACE) == xi) &&
			((int) (World.fuel[i].pos.y/WORLD_SPACE) == yi)) {
			XDrawRectangle(dr->disp, dr->draw, dr->gc,
				       X(data, x), Y(data, y+WORLD_SPACE),
				       WORLD_SPACE, WORLD_SPACE);
			XSetForeground(dr->disp, dr->gc, dr->colors[RED].pixel);
			offset = INSIDE_WS*World.fuel[i].left/MAX_STATION_FUEL;
			XFillRectangle(dr->disp, dr->draw, dr->gc,
				       X(data, x)+1,
				       Y(data, y+WORLD_SPACE)
				       + (INSIDE_WS-offset)+1,
				       INSIDE_WS,
				       offset);
			XSetForeground(dr->disp, dr->gc,
				       dr->colors[BLUE].pixel);
		    }
		}
		break;
		
	    case ACWISE_GRAV:
		XSetForeground(dr->disp, dr->gc, dr->colors[RED].pixel);
		XDrawArc(dr->disp, dr->draw, dr->gc,
			 X(data, x+5), Y(data, y+WORLD_SPACE-5),
			 WORLD_SPACE-10, WORLD_SPACE-10, 64*150, 64*300);
		XDrawLine(dr->disp, dr->draw, dr->gc,
			  X(data, x+WORLD_SPACE/2),
			  Y(data, y+WORLD_SPACE-5),
			  X(data, x+WORLD_SPACE/2+4),
			  Y(data, y+WORLD_SPACE-1));
		XDrawLine(dr->disp, dr->draw, dr->gc,
			  X(data, x+WORLD_SPACE/2),
			  Y(data, y+WORLD_SPACE-5),
			  X(data, x+WORLD_SPACE/2+4),
			  Y(data, y+WORLD_SPACE-9));
		XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
		break;
		
	    case CWISE_GRAV:
		XSetForeground(dr->disp, dr->gc, dr->colors[RED].pixel);
		XDrawArc(dr->disp, dr->draw, dr->gc,
			 X(data, x+5), Y(data, y+WORLD_SPACE-5),
			 WORLD_SPACE-10, WORLD_SPACE-10, 64*90, 64*300);
		XDrawLine(dr->disp, dr->draw, dr->gc,
			  X(data, x+WORLD_SPACE/2),
			  Y(data, y+WORLD_SPACE-5),
			  X(data, x+WORLD_SPACE/2-4),
			  Y(data, y+WORLD_SPACE-1));
		XDrawLine(dr->disp, dr->draw, dr->gc,
			  X(data, x+WORLD_SPACE/2),
			  Y(data, y+WORLD_SPACE-5),
			  X(data, x+WORLD_SPACE/2-4),
			  Y(data, y+WORLD_SPACE-9));
		XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
		break;
		
	    case POS_GRAV:
		XSetForeground(dr->disp, dr->gc, dr->colors[RED].pixel);
		XDrawArc(dr->disp, dr->draw, dr->gc,
			 X(data, x+1), Y(data, y+WORLD_SPACE-1),
			 INSIDE_WS, INSIDE_WS, 0, 64*360);
		XDrawLine(dr->disp, dr->draw, dr->gc,
			  X(data, x+WORLD_SPACE/2),
			  Y(data, y+5),
			  X(data, x+WORLD_SPACE/2),
			  Y(data, y+WORLD_SPACE-5));
		XDrawLine(dr->disp, dr->draw, dr->gc,
			  X(data, x+5),
			  Y(data, y+WORLD_SPACE/2),
			  X(data, x+WORLD_SPACE-5),
			  Y(data, y+WORLD_SPACE/2));
		XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
		break;
		
	    case NEG_GRAV:
		XSetForeground(dr->disp, dr->gc, dr->colors[RED].pixel);
		XDrawArc(dr->disp, dr->draw, dr->gc,
			 X(data, x+1), Y(data, y+WORLD_SPACE-1),
			 INSIDE_WS, INSIDE_WS, 0, 64*360);
		XDrawLine(dr->disp, dr->draw, dr->gc,
			  X(data, x+5),
			  Y(data, y+WORLD_SPACE/2),
			  X(data, x+WORLD_SPACE-5),
			  Y(data, y+WORLD_SPACE/2));
		XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
		break;
		
	    case SPACE:
		if (((xi%8)==0) && ((yi%8)==0))
		    XDrawRectangle(dr->disp, dr->draw, dr->gc,
				   X(data, xi*WORLD_SPACE+WORLD_SPACE/2),
				   Y(data, yi*WORLD_SPACE+WORLD_SPACE/2),
				   2, 2);
		
#ifdef DRAW_GRAV_VECTORS
		XSetForeground(dr->disp, dr->gc, dr->colors[RED].pixel);
		XDrawLine(dr->disp, dr->draw, dr->gc,
			  X(data, x+WORLD_SPACE/2),
			  Y(data, y+WORLD_SPACE/2),
			  X(data, x+WORLD_SPACE/2+ 50*World.gravity[xi][yi].x),
			  Y(data, y+WORLD_SPACE/2+ 50*World.gravity[xi][yi].y));
		XSetForeground(dr->disp, dr->gc, dr->colors[WHITE].pixel);
		XDrawPoint(dr->disp, dr->draw, dr->gc,
			   X(data, x+WORLD_SPACE/2),
			   Y(data, y+WORLD_SPACE/2));
		XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
#endif
		break;
		
	    case PORT:
		XSetForeground(dr->disp, dr->gc, dr->colors[WHITE].pixel);
		XDrawLine(dr->disp, dr->draw, dr->gc,
			  X(data, x), Y(data, y-1),
			  X(data, x+WORLD_SPACE), Y(data, y-1));
		XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
		break;
		
	    default:
		break;
	    }
	}
}



void Draw_world_radar(int draw)
{
    int xi, yi;
    double xs, ys;
    player *dr = Players[draw];
    
    
    xs = 256.0 / World.x;
    ys = (double)Radar_height / World.y;
    
    XSetPlaneMask(dr->disp, dr->gcr, AllPlanes&(~(dr->dpl_1[0]|dr->dpl_1[1])));
    XSetForeground(dr->disp, dr->gcr, dr->colors[BLUE].pixel);
    
    for (xi=0; xi<World.x; xi++)
	for (yi=0; yi<World.y; yi++)
	    switch (World.type[xi][yi]) {
	    case FILLED:
	    case FILLED_NO_DRAW:
	    case REC_LU:
	    case REC_RU:
	    case REC_LD:
	    case REC_RD:
	    case FUEL:
		XDrawPoint(dr->disp, dr->radar, dr->gcr,
			   (int)((xi*xs)+(0.5*xs)),
			   (int)(Radar_height-((yi*ys)+(0.5*ys))));
		break;
	    default:
		break;
	    }
    
    XSetPlaneMask(dr->disp, dr->gcr, AllPlanes&(~(dr->dpl_2[0]|dr->dpl_2[1])));
}

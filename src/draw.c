/* $Id: draw.c,v 1.20 1993/04/18 16:46:17 kenrsc Exp $
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
#include <math.h>

#include "global.h"
#include "map.h"
#include "draw.h"
#include "version.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: draw.c,v 1.20 1993/04/18 16:46:17 kenrsc Exp $";
#endif


#define X(i, co)  ((int) (((co) - xadj) - Players[i]->world.x))
#define Y(i, co)  ((int) (FULL-(((co) - yadj) - Players[i]->world.y)))
#define X2(i, co) ((int) (((co) - xadj2) - Players[i]->world.x))
#define Y2(i, co) ((int) (FULL-(((co) - yadj2) - Players[i]->world.y)))

#define Draw_meter(_p, x, y, title, val, max)				\
{									\
    player *p = (_p);							\
    XSetForeground(p->disp, p->gc, p->colors[RED].pixel);		\
    XFillRectangle(p->disp, p->p_draw, p->gc, x+2, y+2,			\
		   (int)((298*val)/max), 8);				\
    XSetForeground(p->disp, p->gc, p->colors[WHITE].pixel);		\
    XDrawRectangle(p->disp, p->p_draw, p->gc, x, y, 300, 10);		\
    XDrawLine(p->disp, p->p_draw, p->gc, x, y-4, x, y+14);		\
    XDrawLine(p->disp, p->p_draw, p->gc, x+300, y-3, x+300, y+13);	\
    XDrawLine(p->disp, p->p_draw, p->gc, x+150, y-3, x+150, y+13);	\
    XDrawLine(p->disp, p->p_draw, p->gc, x+75, y-1, x+75, y+11);	\
    XDrawLine(p->disp, p->p_draw, p->gc, x+225, y-1, x+225, y+11);	\
    XDrawString(p->disp, p->p_draw, p->gc, (x)+305, (y)+10,		\
		title, strlen(title));					\
}


/*
 * Globals.
 */
long			loops = 0;
message_t		*Msg[MAX_MSGS];

static XPoint		points[5];
static XGCValues	gcv;

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
    str[length] = '\0';

    return (str);
}
#endif



void Draw_objects()
{
    static long scroll_i = 0;
    int i, j;
    player *dr;
    
    
    if (++loops >= LONG_MAX)	/* Used for misc. timing purposes */
	loops = 0;
    
#ifdef SCROLL
    if ((loops%SCROLL_DELAY) == 0)
	if (++scroll_i >= LONG_MAX)
	    scroll_i = 0;
#else
    if ((loops%TITLE_DELAY) == 0)
	scroll_i = !scroll_i;
#endif
    
    for(j=0; j<NumPlayers; j++) {
	dr = Players[j];
	if (dr->disp_type == DT_NONE)
	    continue;

	XSync(dr->disp, False);
	
	/*
	 * If status is GAME_OVER, the user may look through the other
	 * players 'eyes'.  This is done by using two indexes, one determining
	 * which data should be used (i, set below) and one determining where
	 * to draw it (j).
	 */
	if (BIT(dr->status, GAME_OVER) && (BIT(dr->status, PLAYING)) &&
	    (dr->lock.tagged == LOCK_PLAYER))
	    i = GetInd[dr->lock.pl_id];
	else
	    i = j;
	
#ifdef SCROLL
	/*
	 * Scroll a message in the window title.
	 */
	if ((loops%SCROLL_DELAY) == 0)
	    XStoreName(dr->disp, dr->top,
		       scroll(dr->scroll,
			      scroll_i % dr->scroll_len,
			      SCROLL_LEN));
#else
	/*
	 * Switch between two different window titles.
	 */
	if ((loops % TITLE_DELAY) == 0) {
	    if (scroll_i)
		XStoreName(dr->disp, dr->top, COPYRIGHT);
	    else
		XStoreName(dr->disp, dr->top, TITLE);
	    
	}
#endif
	
	if (!Players[i]->damaged) {
	    gcv.dash_offset = 0;
	    Draw_HUD(j, i);
	    Draw_world(j, i);
	    Draw_shots(j, i);
	    gcv.dash_offset = DASHES_LENGTH-(loops%DASHES_LENGTH);
	    Draw_ships(j, i);
	    XSetForeground(dr->disp, dr->gc, dr->colors[WHITE].pixel);
	    Draw_cannon(j, i);
	    Draw_bases(j, i);
	    Draw_meters(j, i);
	    Display_message(j, i);
	    Draw_radar(j, i);
	}

	    /*
	     * Now switch planes and clear the screen.
	     */
	dbuff_switch(j, dr->dbuf_state);
	if (dr->p_radar != dr->radar)
	    XCopyArea(dr->disp, dr->p_radar, dr->radar, dr->gc,
		      0, 0, 256, RadarHeight, 0, 0);
	if (dr->p_draw != dr->draw)
	    XCopyArea(dr->disp, dr->p_draw, dr->draw, dr->gc,
		      0, 0, 768, 768, 0, 0);

	if (BIT(dr->disp_type, DT_HAVE_PLANES))
	    XSetPlaneMask(dr->disp, dr->gc, dr->dbuf_state->drawing_planes);

	if (Players[i]->damaged) {
	    XSetFunction(dr->disp, dr->gc, GXxor);
	    XSetForeground(dr->disp, dr->gc,
			   dr->colors[BLACK].pixel ^ dr->colors[BLUE].pixel);
	    XFillRectangle(dr->disp, dr->draw, dr->gc, 0, 0, 768, 768);
	    XSetFunction(dr->disp, dr->gc, GXcopy);
	    Players[i]->damaged--;
	}
	else {
	    XSetForeground(dr->disp, dr->gc, dr->colors[BLACK].pixel);
	    XFillRectangle(dr->disp, dr->p_draw, dr->gc, 0, 0, 768, 768);
	}
    }
}


static int inview1(float x1, float y1,
		   float x2, float y2,
		   float x3, float y3)
{
    if (x1 < x3 && x2 > x3 && y1 < y3 && y2 > y3)
	return 1;
    return 0;
}


static int inview(player *pl, float x, float y)
{
    float x1, y1;

    if (inview1(pl->world.x, pl->world.y,
		pl->world.x + FULL, pl->world.y + FULL, x, y))
	return 1;

    if (!BIT(World.rules->mode, WRAP_PLAY)
	|| !pl->wrappedWorld)
	return 0;

    if ((pl->wrappedWorld & 1) && x > FULL)
	x1 = x - World.x * BLOCK_SZ;
    else
	x1 = x;
    if ((pl->wrappedWorld & 2) && y > FULL)
	y1 = y - World.y * BLOCK_SZ;
    else
	y1 = y;

    if (inview1(pl->realWorld.x, pl->realWorld.y,
		pl->realWorld.x + FULL, pl->realWorld.y + FULL, x1, y1)) {
	return ((y1 == y ? 4 : 0) | (x1 == x ? 2 : 0) | 1) &
	    (pl->wrappedWorld << 1);
    }

    return 0;
}


void Draw_ships(int draw, int data)
{
    int i;
    player *dr = Players[draw];
    player *pl = Players[data];
    player *pl_i;
    unsigned long mask = 0;
    int flag;
    float xadj, yadj, xadj2, yadj2;
    
    for(i=0; i<NumPlayers; i++) {
	pl_i = Players[i];
	if (BIT(pl_i->status, PLAYING|PAUSE)
	    && (!BIT(pl_i->status, GAME_OVER))
	    && (flag = inview(pl, pl_i->pos.x, pl_i->pos.y))) {

	    if (flag & 2)
		xadj = -World.x * BLOCK_SZ;
	    else
		xadj = 0;
	    if (flag & 4)
		yadj = -World.y * BLOCK_SZ;
	    else
		yadj = 0;
	    
	    if (BIT(pl_i->status, PAUSE)) {
		
		XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
		
		XFillRectangle(dr->disp, dr->p_draw, dr->gc,
			       X(data, pl_i->pos.x-10),
			       Y(data, pl_i->pos.y+13),
			       20, 26);
		
		XSetForeground(dr->disp, dr->gc, dr->colors[WHITE].pixel);
		
		if (pl_i->count<=0 || (loops%10)>=5) {
		    XDrawRectangle(dr->disp, dr->p_draw, dr->gc,
				   X(data, pl_i->pos.x-10),
				   Y(data, pl_i->pos.y+13),
				   20, 26);
		    
		    XDrawString(dr->disp, dr->p_draw, dr->gc,
				X(data, pl_i->pos.x-2),
				Y(data, pl_i->pos.y-4),
				"P", 1);
		}
		continue;
	    }
	    
	    XSetForeground(dr->disp, dr->gc, dr->colors[pl_i->color].pixel);
	    
	    if (pl_i->ecmInfo.size)
		XDrawArc(dr->disp, dr->p_draw, dr->gc,
			 X(data, pl_i->ecmInfo.pos.x - pl_i->ecmInfo.size / 2),
			 Y(data, pl_i->ecmInfo.pos.y + pl_i->ecmInfo.size / 2),
			 pl_i->ecmInfo.size, pl_i->ecmInfo.size, 0, 64 * 360);

	    if (!pl->visibility[i].canSee && i!=data && !TEAM(i, data))
		continue;
	    
	    points[0].x = X(data, pl_i->pos.x + ships[pl_i->dir].pts[0].x);
	    points[0].y = Y(data, pl_i->pos.y + ships[pl_i->dir].pts[0].y);
	    points[1].x = X(data, pl_i->pos.x + ships[pl_i->dir].pts[1].x);
	    points[1].y = Y(data, pl_i->pos.y + ships[pl_i->dir].pts[1].y);
	    points[2].x = X(data, pl_i->pos.x + ships[pl_i->dir].pts[2].x);
	    points[2].y = Y(data, pl_i->pos.y + ships[pl_i->dir].pts[2].y);
	    points[3].x = X(data, pl_i->pos.x + ships[pl_i->dir].pts[0].x);
	    points[3].y = Y(data, pl_i->pos.y + ships[pl_i->dir].pts[0].y);
	    
	    /*
	     * Determine if the name of the player should be drawn below
	     * his/her ship.
	     */
	    if ((BIT(pl->status, ID_MODE) && data != i)
		|| (draw != data && i == data))
		XDrawString(dr->disp, dr->p_draw, dr->gc,
			    X(data, pl_i->pos.x-pl_i->name_length/2),
			    Y(data,pl_i->pos.y-30),
			    pl_i->name, strlen(pl_i->name));
	    
	    if (BIT(pl_i->used, OBJ_REFUEL|OBJ_SHIELD
		    |OBJ_CLOAKING_DEVICE|OBJ_CONNECTOR)) {
		
		mask = GCLineStyle;
		gcv.line_style = LineOnOffDash;
		
#ifndef NO_ROTATING_DASHES
		mask |= GCDashOffset;
#endif
		XChangeGC(dr->disp, dr->gc, mask, &gcv);
		
		if (BIT(pl_i->used, OBJ_CLOAKING_DEVICE))
		    XDrawLines(dr->disp, dr->p_draw, dr->gc, points, 4, 0);
		
		if (BIT(pl_i->used, OBJ_REFUEL)
		    && (flag = inview(pl, World.fuel[pl_i->fs].pos.x,
			World.fuel[pl_i->fs].pos.y))) {

		    if (flag & 2)
			xadj2 = -World.x * BLOCK_SZ;
		    else
			xadj2 = 0;
		    if (flag & 4)
			yadj2 = -World.y * BLOCK_SZ;
		    else
			yadj2 = 0;
		    XDrawLine(dr->disp, dr->p_draw, dr->gc,
			      X2(data, World.fuel[pl_i->fs].pos.x),
			      Y2(data, World.fuel[pl_i->fs].pos.y),
			      X(data, pl_i->pos.x),
			      Y(data, pl_i->pos.y));
		}
		
		if (BIT(pl_i->used, OBJ_SHIELD)) 
		    XDrawArc(dr->disp, dr->p_draw, dr->gc,
			     X(data,pl_i->pos.x - 17),
			     Y(data,pl_i->pos.y + 17),
			     34, 34, 0, 64 * 360);
	     
		/* Draw dotted line while picking up ball */
		if (pl_i->ball != NULL
		    && (flag = inview(pl, pl_i->ball->pos.x,
			pl_i->ball->pos.y))) {

		    if (flag & 2)
			xadj2 = -World.x * BLOCK_SZ;
		    else
			xadj2 = 0;
		    if (flag & 4)
			yadj2 = -World.y * BLOCK_SZ;
		    else
			yadj2 = 0;
		    XDrawLine(dr->disp, dr->p_draw, dr->gc,
			      X2(data, pl_i->ball->pos.x), 
			      Y2(data, pl_i->ball->pos.y), 
			      X(data, pl_i->pos.x),
			      Y(data, pl_i->pos.y));  
		}
	    }
	    
	    gcv.line_style = LineSolid;
	    XChangeGC(dr->disp, dr->gc, GCLineStyle, &gcv);
	    
	    if (!BIT(pl_i->used, OBJ_CLOAKING_DEVICE)
		&& (i == data || pl->visibility[i].canSee)) {
		if (pl->lock.tagged==LOCK_PLAYER && GetInd[pl->lock.pl_id]==i)
		    XFillPolygon(dr->disp, dr->p_draw, dr->gc, points, 4,
				 Convex, CoordModeOrigin);
		else 
		    XDrawLines(dr->disp, dr->p_draw, dr->gc, points, 4, 0);
	    }

	  }
    }
}


void Draw_shots(int draw, int data)
{
#define xadj 0
#define yadj 0
    int i, x, y, x2, y2;
    char str[2];
    player *pl = Players[data];
    player *dr = Players[draw];
    object *shot;
    int flag;

#define BATCH			/**/
#ifdef BATCH
#define BATCH_MALLOC  16
    int			lastcolor, size;
    static XRectangle	*rectangles[NUM_COLORS],
    			*past_rect[NUM_COLORS];
    XRectangle		*cur_rect[NUM_COLORS],
    			*rectangle;

    for (i=NUM_COLORS; i--; cur_rect[i]=rectangles[i])
	;
#endif

    for(i=0; i<NumObjs; i++) {
	shot = Obj[i];
	
	if (flag = inview(pl, shot->pos.x, shot->pos.y)) {
	    
	    if (flag & 2)
		x = X(data, shot->pos.x + World.x * BLOCK_SZ);
	    else
		x = X(data, shot->pos.x);

	    if (flag & 4)
		y = Y(data, shot->pos.y + World.y * BLOCK_SZ);
	    else
		y = Y(data, shot->pos.y);

	    
	    if (shot->id != -1
		&& TEAM(data, GetInd[shot->id])
		&& shot->id != pl->id
		&& shot->type != OBJ_CANNON_DEBRIS)
#ifdef BATCH		
		XSetForeground(dr->disp, dr->gc,
			       dr->colors[lastcolor=BLUE].pixel);
	    else
		XSetForeground(dr->disp, dr->gc,
			       dr->colors[lastcolor=shot->color].pixel);
#else		
	    XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
	    else
		XSetForeground(dr->disp, dr->gc,
			       dr->colors[shot->color].pixel);
#endif
#ifdef BATCH
	    size = 2;
#endif
	    switch (shot->type) {
	    case OBJ_SPARK:
	    case OBJ_DEBRIS:
	    case OBJ_CANNON_DEBRIS:
		if (!BIT(dr->disp_type, DT_HAVE_COLOR))
#ifdef BATCH
		    size = 1;
#else
		    XDrawPoint(dr->disp, dr->p_draw, dr->gc, x, y);
		else
#endif
	    case OBJ_SHOT:
	    case OBJ_CANNON_SHOT:
#ifdef BATCH
	    {
		if ((rectangle=cur_rect[lastcolor]++)
		    == past_rect[lastcolor]) {
		    if(!rectangles[lastcolor]) {
			past_rect[lastcolor] =
			    BATCH_MALLOC + (rectangles[lastcolor] =
			    (XRectangle*)malloc(BATCH_MALLOC
						* sizeof(XRectangle)));
		    }
		    else {
			size_t l;
			l = past_rect[lastcolor] - rectangles[lastcolor]
			    + BATCH_MALLOC;

			past_rect[lastcolor] =
			    l + (rectangles[lastcolor] =
			         (XRectangle*)realloc(rectangles[lastcolor],
						      l * sizeof(XRectangle)));
			 }
		    	cur_rect[lastcolor] = 1
			    + (rectangle = past_rect[lastcolor]-BATCH_MALLOC);
		}
		rectangle->x = x;	rectangle->y = y;
		rectangle->width = rectangle->height = size;
	    }
#else
		XDrawRectangle(dr->disp, dr->p_draw, dr->gc, x, y, 2, 2);
#endif
		break;

	    case OBJ_TORPEDO:
            case OBJ_NUKE:
	    case OBJ_SMART_SHOT:
	    case OBJ_HEAT_SHOT:
		XSetLineAttributes(dr->disp, dr->gc, 4,
				   LineSolid, CapButt, JoinMiter);
		
		XDrawLine(dr->disp, dr->p_draw, dr->gc,
			  x, y,
			  (int)(x-tcos(shot->dir)*SMART_SHOT_LEN),
			  (int)(y+tsin(shot->dir)*SMART_SHOT_LEN));
		
		XSetLineAttributes(dr->disp, dr->gc, 0,
				   LineSolid, CapButt, JoinMiter);
		break;

	    case OBJ_BALL:
		XDrawArc(dr->disp, dr->p_draw, dr->gc,
			 x - 10, y - 10, 20, 20, 0, 64*360);
		if (shot->id != -1) {
		    const float px = Players[ GetInd[shot->id] ]->pos.x;
		    const float py = Players[ GetInd[shot->id] ]->pos.y;
		    if ((flag = inview(pl, px, py)) != 0) {
			if (flag & 2)
			    x2 = X(data, px + World.x * BLOCK_SZ);
			else
			    x2 = X(data, px);
			if (flag & 4)
			    y2 = Y(data, py + World.y * BLOCK_SZ);
			else
			    y2 = Y(data, py);
			XDrawLine(dr->disp, dr->p_draw, dr->gc, x, y, x2, y2);
		    }
		}
		break;

	    case OBJ_MINE:
		XDrawArc(dr->disp, dr->p_draw, dr->gc, x, y, 8, 8, 0, 64*360);
		break;
	    case OBJ_BALL_PACK:
		str[0] = 'B';
		goto draw;
		break;
	    case OBJ_WIDEANGLE_SHOT:
		str[0] = 'W';
		goto draw;
		break;
	    case OBJ_AFTER_BURNER:
                str[0] = 'A';
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
	    case OBJ_SENSOR_PACK:
		str[0] = 'I';
		goto draw;
		break;
	    case OBJ_ECM:
		str[0] = 'E';
		goto draw;
		break;
	    case OBJ_TANK:
		str[0] = 'T';
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
		
		XDrawLines(dr->disp, dr->p_draw, dr->gc,
			   points, 4, CoordModeOrigin);
		str[1] = '\0';
		XDrawString(dr->disp, dr->p_draw, dr->gc, x-3, y+8, str, 1);
		break;

	    default:
		error("Draw_shots: Shot type %d not defined.", shot->type);
		break;
	    }
	}
    }
#ifdef BATCH
    for (i=NUM_COLORS; i--; )
	if (rectangles[i]) {
	    XSetForeground(dr->disp, dr->gc, dr->colors[i].pixel);
	    XDrawRectangles(dr->disp, dr->p_draw, dr->gc,
			    rectangles[i], cur_rect[i]-rectangles[i]);
	}
#endif
#undef xadj
#undef yadj
}


void Draw_meters(int draw, int data)
{
    player *dr=Players[draw], *pl=Players[data];


    if (BIT(pl->instruments, SHOW_FUEL_METER))
	Draw_meter(dr, 10, 10, "Fuel", pl->fuel.sum, pl->fuel.max);
    if (BIT(pl->instruments, SHOW_POWER_METER) || pl->control_count)
     	Draw_meter(dr, 10, 40, "Power", pl->power, MAX_PLAYER_POWER);
    if (BIT(pl->instruments, SHOW_TURNSPEED_METER) || pl->control_count)
	Draw_meter(dr, 10, 60, "Turnspeed",
		   pl->turnspeed, MAX_PLAYER_TURNSPEED);
    
    if (BIT(pl->status, SELF_DESTRUCT) && pl->count>0)
	Draw_meter(dr, (FULL-300)/2 -32, 3*FULL/4,
		   "Self destructing", pl->count, 150);
    if (Shutdown != -1)
	Draw_meter(dr, (FULL-300)/2 -32, 4*FULL/5,
		   "SHUTDOWN", Shutdown, ShutdownDelay);
    
    /*	Draw_meter(dr, 10, 30, "Shots", pl->shots,pl->shot_max);  */
}



void Draw_HUD(int draw, int data)
{
    player *dr=Players[draw], *pl=Players[data];
    player *target=Players[GetInd[dr->lock.pl_id]];
    int size, vert_pos;
    char str[50], *p;
    float dx, dy;
    int	theta;
    int hud_pos_x;
    int hud_pos_y;

    
    /* 
     * Show speed pointer
     */
    if (pl->ptr_move_fact != 0.0) {
	XSetForeground(dr->disp, dr->gc, dr->colors[RED].pixel);
        XDrawLine(dr->disp, dr->p_draw, dr->gc,
	    CENTER,
            CENTER,
            CENTER-(pl->ptr_move_fact*pl->vel.x),
            CENTER+(pl->ptr_move_fact*pl->vel.y)
        );
    }

    
    XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);

    if (!BIT(pl->instruments, SHOW_HUD_INSTRUMENTS))
	return;

    /*
     * Display the HUD
     */

    hud_pos_x = CENTER-(pl->hud_move_fact*pl->vel.x);
    hud_pos_y = CENTER+(pl->hud_move_fact*pl->vel.y);

    /* HUD frame */
    gcv.line_style = LineOnOffDash;
    XChangeGC(dr->disp, dr->gc, GCLineStyle | GCDashOffset, &gcv);
    
    if (BIT(pl->instruments, SHOW_HUD_HORIZONTAL)) {
	XDrawLine(dr->disp, dr->p_draw, dr->gc,
		  hud_pos_x-HUD_SIZE, hud_pos_y-HUD_SIZE+HUD_OFFSET,
		  hud_pos_x+HUD_SIZE, hud_pos_y-HUD_SIZE+HUD_OFFSET);
	XDrawLine(dr->disp, dr->p_draw, dr->gc,
		  hud_pos_x-HUD_SIZE, hud_pos_y+HUD_SIZE-HUD_OFFSET,
		  hud_pos_x+HUD_SIZE, hud_pos_y+HUD_SIZE-HUD_OFFSET);
    }
    if (BIT(pl->instruments, SHOW_HUD_VERTICAL)) {
	XDrawLine(dr->disp, dr->p_draw, dr->gc,
		  hud_pos_x-HUD_SIZE+HUD_OFFSET, hud_pos_y-HUD_SIZE, 
		  hud_pos_x-HUD_SIZE+HUD_OFFSET, hud_pos_y+HUD_SIZE);
	XDrawLine(dr->disp, dr->p_draw, dr->gc,
		  hud_pos_x+HUD_SIZE-HUD_OFFSET, hud_pos_y-HUD_SIZE,
		  hud_pos_x+HUD_SIZE-HUD_OFFSET, hud_pos_y+HUD_SIZE);
    }
    gcv.line_style = LineSolid;
    XChangeGC(dr->disp, dr->gc, GCLineStyle, &gcv);
    
    
    /* Special items */
    vert_pos = hud_pos_y - HUD_SIZE + HUD_OFFSET;
    
    if (pl->cloaks > 0) {
	vert_pos += HUD_ITEMS_SPACE;
	sprintf(str, "C%02d", pl->cloaks);
	XDrawString(dr->disp, dr->p_draw, dr->gc,
		    hud_pos_y-HUD_SIZE-4, vert_pos, str, 3);
    }
    if (pl->sensors > 0) {
	vert_pos += HUD_ITEMS_SPACE;
	sprintf(str, "I%02d", pl->sensors);
	XDrawString(dr->disp, dr->p_draw, dr->gc,
		    hud_pos_y-HUD_SIZE-4, vert_pos, str, 3);
    }
    if (pl->fuel.num_tanks > 0) {
 	vert_pos += HUD_ITEMS_SPACE;
 	sprintf(str, "T%02d", pl->fuel.num_tanks);
 	XDrawString(dr->disp, dr->p_draw, dr->gc,
 		    hud_pos_x-HUD_SIZE-4, vert_pos, str, 3);
    }
    if (pl->extra_shots > 0) {
	vert_pos += HUD_ITEMS_SPACE;
	sprintf(str, "W%02d", pl->extra_shots);
	XDrawString(dr->disp, dr->p_draw, dr->gc,
		    hud_pos_y-HUD_SIZE-4, vert_pos, str, 3);
    }
    if (pl->rear_shots > 0) {
	vert_pos += HUD_ITEMS_SPACE;
	sprintf(str, "R%02d", pl->rear_shots);
	XDrawString(dr->disp, dr->p_draw, dr->gc,
		    hud_pos_y-HUD_SIZE-4, vert_pos,
		    str, 3);
    }
    if (pl->after_burners > 0) {
 	vert_pos += HUD_ITEMS_SPACE;
 	sprintf(str, "A%02d", pl->after_burners);
 	XDrawString(dr->disp, dr->p_draw, dr->gc,
 		    hud_pos_x-HUD_SIZE-4, vert_pos, str, 3);
    }


    /* Fuel notify, HUD meter on */
    if (pl->fuel.count || pl->fuel.sum < pl->fuel.l3) {
 	sprintf(str, "%5d", pl->fuel.sum>>FUEL_SCALE_BITS);
 	XDrawString(dr->disp, dr->p_draw, dr->gc,
		    hud_pos_x + HUD_SIZE-HUD_OFFSET + 2,
 		    hud_pos_y + HUD_SIZE-HUD_OFFSET/2 + 6, str, 5);
	if (pl->fuel.num_tanks) {
	    if (pl->fuel.current == 0)
		sprintf(str," M =%4d",
			pl->fuel.tank[pl->fuel.current]>>FUEL_SCALE_BITS);
	    else
		sprintf(str,"T%2d=%4d",
			pl->fuel.current,
			pl->fuel.tank[pl->fuel.current]>>FUEL_SCALE_BITS);
 	    XDrawString(dr->disp, dr->p_draw, dr->gc,
			hud_pos_x + HUD_SIZE - HUD_OFFSET + 2,
 		        hud_pos_y + HUD_SIZE - HUD_OFFSET/2 + HUD_ITEMS_SPACE,
			str, 8);
	}
    }

    p = str;

    if (pl->mines > 0) {
	sprintf(p, "M%02d  ", pl->mines);
	p += strlen(p);
    }
    if (pl->missiles > 0) {
	sprintf(p, "S%02d  ", pl->missiles);
	p += strlen(p);
    }
    if (pl->ecms > 0) {
	sprintf(p, "E%02d", pl->ecms);
	p += strlen(p);
    }

    if (p != str)
	XDrawString(dr->disp, dr->p_draw, dr->gc,
		    hud_pos_x - HUD_SIZE + HUD_OFFSET + 4,
		    hud_pos_y + HUD_SIZE, str, strlen(str));

    /*
     * Display direction arrow and miscellaneous target information.
     */
    switch (pl->lock.tagged) {
    case LOCK_PLAYER:
	dx = WRAP_DX(target->pos.x - pl->pos.x);
	dy = WRAP_DY(target->pos.y - pl->pos.y);
	size = strlen(target->name);
	XDrawString(dr->disp, dr->p_draw, dr->gc,
		    hud_pos_x-target->name_length/2,
		    hud_pos_y-HUD_SIZE+HUD_OFFSET/2+3,
		    target->name, size);
	if (!BIT(World.rules->mode, LIMITED_VISIBILITY)
	     || pl->lock.distance <= pl->sensor_range
#ifndef SHOW_CLOAKERS_RANGE
	    && (pl->visibility[GetInd[target->id]].canSee
		|| TEAM(data, GetInd[dr->lock.pl_id]))
#endif /* !SHOW_CLOAKERS_RANGE */
	    ) {
	    sprintf(str, "%03d", (int)(pl->lock.distance/BLOCK_SZ));
	    XDrawString(dr->disp, dr->p_draw, dr->gc,
			hud_pos_x + HUD_SIZE - HUD_OFFSET + 2,
			hud_pos_y - HUD_SIZE + HUD_OFFSET/2 + 3, str, 3);
	}
	if ((BIT(World.rules->mode, LIMITED_VISIBILITY)
	     && (pl->lock.distance > pl->sensor_range))
	    || (!pl->visibility[GetInd[target->id]].canSee
		&& !TEAM(data, GetInd[dr->lock.pl_id]))
	    || (dy == 0.0 && dx == 0.0))
	    goto no_target;
	break;
	
    case LOCK_NONE:
	goto no_target;
	break;
	
    default:
	error("Unkown pl->lock.tagged.");
	break;
    }

    theta = findDir(dx, dy);	/* Only works for LOCK_PLAYER */
    
    if (pl->lock.distance > WARNING_DISTANCE || loops%2 == 0) {
	size = 10000 / (800 + pl->lock.distance);
	if (TEAM(data, GetInd[pl->lock.pl_id]))
	    XDrawArc(dr->disp, dr->p_draw, dr->gc,
		     (int)(hud_pos_y + HUD_SIZE * 0.6 * tcos(theta)),
		     (int)(hud_pos_y - HUD_SIZE * 0.6 * tsin(theta)),
		     size, size, 0, 64*360);
	else
	    XFillArc(dr->disp, dr->p_draw, dr->gc,
		     (int)(hud_pos_y + HUD_SIZE * 0.6 * tcos(theta)),
		     (int)(hud_pos_y - HUD_SIZE * 0.6 * tsin(theta)),
		     size, size, 0, 64*360);
    }
    
    
    /* Fuel gauge, must be last */
 no_target:
    if (!((pl->fuel.count)
	  || (pl->fuel.sum < pl->fuel.l3
	      && ((pl->fuel.sum < pl->fuel.l1 && (loops%4) < 2)
		  || (pl->fuel.sum < pl->fuel.l2
		      && pl->fuel.sum > pl->fuel.l1
		      && (loops%8) < 4)
		  || (pl->fuel.sum > pl->fuel.l2)))))
	return;
    
    XDrawRectangle(dr->disp, dr->p_draw, dr->gc,
		   hud_pos_x + HUD_SIZE - HUD_OFFSET + FUEL_GAUGE_OFFSET - 1,
		   hud_pos_y - HUD_SIZE + HUD_OFFSET + FUEL_GAUGE_OFFSET - 1,
		   HUD_OFFSET - (2*FUEL_GAUGE_OFFSET) + 2,
		   HUD_FUEL_GAUGE_SIZE + 2);
    
    if ((size = (HUD_FUEL_GAUGE_SIZE*pl->fuel.sum)/pl->fuel.max)<0) size=0;
    XFillRectangle(dr->disp, dr->p_draw, dr->gc,
		   hud_pos_x + HUD_SIZE - HUD_OFFSET + FUEL_GAUGE_OFFSET,
		   hud_pos_y - HUD_SIZE + HUD_OFFSET + FUEL_GAUGE_OFFSET
		   + HUD_FUEL_GAUGE_SIZE - size,
		   HUD_OFFSET - (2*FUEL_GAUGE_OFFSET), size);
}


void Draw_bases(int draw, int data)
{
    const int	team_length = 12,
    		right_team_length = team_length - 7,
    		up_x = 1, up_y = -12,
    		down_x = 1, down_y = BLOCK_SZ + 2,
    		left_x = BLOCK_SZ + 2, left_y = BLOCK_SZ/2 - 5,
    		right_x = -8, right_y = BLOCK_SZ/2 - 5;
    player *pl = Players[data];
    player *dr = Players[draw];
    int i, j, flag;
    float x, y, xadj, yadj;
    char s[2];


    XSetForeground(dr->disp, dr->gc, dr->colors[WHITE].pixel);
    for (i=0; i<World.NumBases; i++) {
	x = World.base[i].pos.x * BLOCK_SZ;
	y = World.base[i].pos.y * BLOCK_SZ;
	if (!(flag = inview(pl, x, y)))
	    continue;

	if (flag & 2)
	    xadj = -World.x * BLOCK_SZ;
	else
	    xadj = 0;
	if (flag & 4)
	    yadj = -World.y * BLOCK_SZ;
	else
	    yadj = 0;

	for (j=0; j<NumPlayers && Players[j]->home_base != i; j++)
	    ;

	if (World.base[i].team == TEAM_NOT_SET) {
	    s[0]='\0';
	} else {
	    s[0] = '0' + World.base[i].team;
	    s[1] = '\0';
	}

	switch (World.base[i].dir) {
	case DIR_UP:
	    XDrawLine(dr->disp, dr->p_draw, dr->gc,
		      X(data, x), Y(data, y-1),
		      X(data, x+BLOCK_SZ), Y(data, y-1));
	    if (s[0]) {
		XDrawString(dr->disp, dr->p_draw, dr->gc,
			    X(data, x+up_x),
			    Y(data, y+up_y),
			    s, 1);
	    }
	    if (j < NumPlayers) {
		XDrawString(dr->disp, dr->p_draw, dr->gc,
			    X(data, x + up_x + (s[0] ? team_length : 0)),
			    Y(data, y + up_y),
			    Players[j]->name,
			    strlen(Players[j]->name));
	    }
	    break;
	case DIR_DOWN:
	    XDrawLine(dr->disp, dr->p_draw, dr->gc,
		      X(data, x), Y(data, y+BLOCK_SZ+1),
		      X(data, x+BLOCK_SZ), Y(data, y+BLOCK_SZ+1));
	    if (s[0]) {
		XDrawString(dr->disp, dr->p_draw, dr->gc,
			    X(data, x + down_x),
			    Y(data, y + down_y),
			    s, 1);
	    }
	    if (j < NumPlayers) {
		XDrawString(dr->disp, dr->p_draw, dr->gc,
			    X(data, x + down_x + (s[0] ? team_length : 0)),
			    Y(data, y + down_y),
			    Players[j]->name,
			    strlen(Players[j]->name));
	    }
	    break;
	case DIR_LEFT:
	    XDrawLine(dr->disp, dr->p_draw, dr->gc,
		      X(data, x+BLOCK_SZ+1), Y(data, y+BLOCK_SZ),
		      X(data, x+BLOCK_SZ+1), Y(data, y));
	    if (s[0]) {
		XDrawString(dr->disp, dr->p_draw, dr->gc,
			    X(data, x + left_x),
			    Y(data, y + left_y),
			    s, 1);
	    }
	    if (j < NumPlayers) {
		XDrawString(dr->disp, dr->p_draw, dr->gc,
			    X(data, x + left_x + (s[0] ? team_length : 0)),
			    Y(data, y + left_y),
			    Players[j]->name,
			    strlen(Players[j]->name));
	    }
	    break;
	case DIR_RIGHT:
	    XDrawLine(dr->disp, dr->p_draw, dr->gc,
		      X(data, x-1), Y(data, y+BLOCK_SZ),
		      X(data, x-1), Y(data, y));
	    if (s[0]) {
		XDrawString(dr->disp, dr->p_draw, dr->gc,
			    X(data, x + right_x),
			    Y(data, y + right_y),
			    s, 1);
	    }
	    if (j < NumPlayers) {
		XDrawString(dr->disp, dr->p_draw, dr->gc,
			    X(data,
			      x + right_x - ((s[0] ? right_team_length : 0)
					     + Players[j]->name_length)),
			    Y(data, y + right_y),
			    Players[j]->name,
			    strlen(Players[j]->name));
	    }
	    break;
	default:
	    error("Wrong direction of base.");
	    break;
	}
    }
}


void Draw_cannon(int draw, int data)
{
    player *pl = Players[data];
    player *dr = Players[draw];
    int i;
    float xadj, yadj;
    int flag;
    
    for (i=0; i<World.NumCannons; i++) {
	if ((World.cannon[i].dead_time == 0)
	    && (flag = inview(pl,
			      (float)(World.cannon[i].pos.x * BLOCK_SZ),
			      (float)(World.cannon[i].pos.y * BLOCK_SZ)))) {

	    if (flag & 2)
	      xadj = -World.x * BLOCK_SZ;
	    else
	      xadj = 0;
	    if (flag & 4)
	      yadj = -World.y * BLOCK_SZ;
	    else
	      yadj = 0;

	    switch (World.cannon[i].dir) {
	      case DIR_UP:
		points[0].x = World.cannon[i].pos.x*BLOCK_SZ;
		points[0].y = World.cannon[i].pos.y*BLOCK_SZ;
		points[1].x = World.cannon[i].pos.x*BLOCK_SZ+BLOCK_SZ;
		points[1].y = World.cannon[i].pos.y*BLOCK_SZ;
		points[2].x = World.cannon[i].pos.x*BLOCK_SZ+BLOCK_SZ/2;
		points[2].y = World.cannon[i].pos.y*BLOCK_SZ+BLOCK_SZ/3;
		break;
	      case DIR_DOWN:
		points[0].x = World.cannon[i].pos.x*BLOCK_SZ;
		points[0].y = World.cannon[i].pos.y*BLOCK_SZ+BLOCK_SZ;
		points[1].x = World.cannon[i].pos.x*BLOCK_SZ+BLOCK_SZ;
		points[1].y = World.cannon[i].pos.y*BLOCK_SZ+BLOCK_SZ;
		points[2].x = World.cannon[i].pos.x*BLOCK_SZ+BLOCK_SZ/2;
		points[2].y = World.cannon[i].pos.y*BLOCK_SZ+2*BLOCK_SZ/3;
		break;
	      case DIR_RIGHT:
		points[0].x = World.cannon[i].pos.x*BLOCK_SZ;
		points[0].y = World.cannon[i].pos.y*BLOCK_SZ;
		points[1].x = World.cannon[i].pos.x*BLOCK_SZ;
		points[1].y = World.cannon[i].pos.y*BLOCK_SZ+BLOCK_SZ;
		points[2].x = World.cannon[i].pos.x*BLOCK_SZ+BLOCK_SZ/3;
		points[2].y = World.cannon[i].pos.y*BLOCK_SZ+BLOCK_SZ/2;
		break;
	      case DIR_LEFT:
		points[0].x = World.cannon[i].pos.x*BLOCK_SZ+BLOCK_SZ;
		points[0].y = World.cannon[i].pos.y*BLOCK_SZ;
		points[1].x = World.cannon[i].pos.x*BLOCK_SZ+BLOCK_SZ;
		points[1].y = World.cannon[i].pos.y*BLOCK_SZ+BLOCK_SZ;
		points[2].x = World.cannon[i].pos.x*BLOCK_SZ+2*BLOCK_SZ/3;
		points[2].y = World.cannon[i].pos.y*BLOCK_SZ+BLOCK_SZ/2;
	        break;
	    }
	    
	    points[0].x = X(data, points[0].x);
	    points[1].x = X(data, points[1].x);
	    points[2].x = X(data, points[2].x);
	    points[3].x = points[0].x;
	    
	    points[0].y = Y(data, points[0].y);
	    points[1].y = Y(data, points[1].y);
	    points[2].y = Y(data, points[2].y);
	    points[3].y = points[0].y;
	    
	    XDrawLines(dr->disp, dr->p_draw, dr->gc, points, 4, 0);
	    
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
	
	XDrawString(dr->disp, dr->p_draw, dr->gc, 10, 755-(13*i),
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
    object *shot;
    
    if (dr->s_radar != dr->p_radar)
	/* Draw static radar onto radar */
	XCopyArea(dr->disp, dr->s_radar, dr->p_radar, dr->gc,
		  0, 0, 256, RadarHeight, 0, 0);
    else {
	/* Clear radar */
	XSetForeground(dr->disp, dr->gcr, dr->colors[BLACK].pixel);
	XFillRectangle(dr->disp, dr->p_radar,
		       dr->gcr, 0, 0, 256, RadarHeight);
    }
    
    /* Checkpoint */
    if (BIT(pl->mode, TIMING)) {
	XSetForeground(dr->disp,
		       dr->gcr,
		       dr->colors[WHITE].pixel);
	
	diamond[0].x = 256*World.check[pl->check].x/World.x;
	diamond[0].y = RadarHeight -
	    RadarHeight*World.check[pl->check].y/World.y+DSIZE;
	XDrawLines(dr->disp, dr->p_radar,
		   dr->gcr, diamond,
		   5, CoordModePrevious);
    }
#ifndef NO_SMART_MIS_RADAR
    if (loops & 1) {
	XRectangle *rects, *p;
	int nrects = 0;

	p = rects = (XRectangle *) malloc(NumObjs * sizeof(XRectangle));

	for (i = 0; i < NumObjs; i++) {
	    shot = Obj[i];
	    if (BIT(shot->type, (OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_NUKE
				 |OBJ_HEAT_SHOT))
		&& Wrap_length(pl->pos.x - shot->pos.x,
			  pl->pos.y - shot->pos.y) <= pl->sensor_range) {

		p->x = 256 * (shot->pos.x / (World.x * BLOCK_SZ));
	        p->y = RadarHeight -
		    (RadarHeight * (shot->pos.y / (World.y * BLOCK_SZ)));
		p->width = p->height = 2;
		p++;
		nrects++;
	    }
	}

	XSetForeground(dr->disp, dr->gcr, dr->colors[WHITE].pixel);
	XFillRectangles(dr->disp, dr->radar, dr->gcr, rects, nrects);
	free(rects);
    }
#endif
    XSetForeground(dr->disp, dr->gcr, dr->colors[WHITE].pixel);

    for(i=0; i<NumPlayers; i++) {
	if (BIT(Players[i]->status, PLAYING) &&
	    !BIT(Players[i]->status, GAME_OVER)) {
	    int	x, y;
	    
	    x = 256 * Players[i]->pos.x / (World.x * BLOCK_SZ);
	    y = RadarHeight - RadarHeight * (Players[i]->pos.y
					     / (World.y * BLOCK_SZ));

	    if ((BIT(World.rules->mode, LIMITED_VISIBILITY)
		 && (Wrap_length(pl->pos.x - Players[i]->pos.x,
			    pl->pos.y-Players[i]->pos.y) > pl->sensor_range))
		|| (!pl->visibility[i].canSee && i!=data))
		continue;
	    
	    if (data == i) {					/* You */
		if (loops%16 >= 13)
		    continue;

		XDrawLine(dr->disp, dr->p_radar, dr->gcr,
			  x, y, x + 8 * tcos(pl->dir), y - 8 * tsin(pl->dir));

	    } else if (GetInd[pl->lock.pl_id] == i
		       && BIT(pl->used, OBJ_COMPASS)
		       && pl->lock.tagged == LOCK_PLAYER) {	/* Tagged */
		
		if (loops%5 >= 3)
		    continue;
	    }
	    
	    XFillRectangle(dr->disp, dr->p_radar, dr->gcr,
			   x-1, y-1, 3, 3);
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
    
    sprintf(string, "Time: %-5.2f	  Round: %d	  Last time: %-5.2f",
	    pl->time/100.0, pl->round, pl->last_time/100.0);
    XDrawString(dr->disp, dr->p_draw, dr->gc, 10, 30, string, strlen(string));	
    
    sprintf(string, "Best time: %-5.2f	     "
	    "Best round: %-5.2f      Last lap: %-5.2f",
	    pl->best_run/100.0, pl->best_lap/100.0, pl->last_lap_time/100.0);
    XDrawString(dr->disp, dr->p_draw, dr->gc, 10, 50, string, strlen(string));	
    
    if (!(BIT(World.rules->mode, ONE_PLAYER_ONLY))) {
	for (i=0;i < NumPlayers; i++) {
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
		"Best time by %s : %-5.2f    Best round by %s: %-5.2f",
		run_name, best_run/100.0, lap_name, best_lap/100.0);
	XDrawString(dr->disp, dr->p_draw, dr->gc,
		    10, 70, string, strlen(string));
    }
}



void Draw_world(int draw, int data)
{
#define xadj 0
#define yadj 0
    int xi, yi, xb, yb, i, offset;
    int rxb, ryb;
    int xdot = 0;
    int ydot = 0;
    float x, y;
    static const int WS_PR_SC=1+(float)FULL/BLOCK_SZ;
    static const int INSIDE_WS=BLOCK_SZ-2;
    static int wormCount;
    player *pl = Players[data];
    player *dr = Players[draw];
    int sx = 0, sy = 0;
    
    pl->wormDrawCount = (pl->wormDrawCount + 1) & 7;

    xb = (pl->world.x/BLOCK_SZ);
    yb = (pl->world.y/BLOCK_SZ);
    if (!BIT (World.rules->mode, WRAP_PLAY)) {
	if (xb < 0)
	    sx = -xb;
	if (yb < 0)
	    sy = -yb;
    }

    for (rxb = sx; rxb <= WS_PR_SC; rxb++) {
	xi = xb + rxb;
	if (xi == World.x) {
	    if (!BIT(World.rules->mode, WRAP_PLAY))
		break;
	    xdot = 1;
	} else
	    xdot = 0;
	if (xi >= World.x)
	    xi -= World.x;

	for (ryb = sy; ryb <= WS_PR_SC; ryb++) {
	    yi = ryb + yb;

	    if (yi == World.y) {
		if (!BIT(World.rules->mode, WRAP_PLAY))
		    break;
		ydot = 1;
	    }
	    else
		ydot = 0;

	    if (yi >= World.y)
		yi -= World.y;

	    x = (xb + rxb) * BLOCK_SZ;
	    y = (yb + ryb) * BLOCK_SZ;

	    switch (World.block[xi][yi]) {
		
	    case FILLED:
		XDrawRectangle(dr->disp, dr->p_draw, dr->gc,
			       X(data, x), Y(data, y+BLOCK_SZ),
			       BLOCK_SZ, BLOCK_SZ);
		break;
		
	    case CHECK:
		points[0].x = X(data, x+(BLOCK_SZ/2));
		points[0].y = Y(data, y);
		points[1].x = X(data, x);
		points[1].y = Y(data, (y+BLOCK_SZ/2));
		points[2].x = X(data, (x+BLOCK_SZ/2));
		points[2].y = Y(data, (y+BLOCK_SZ));
		points[3].x = X(data, x+BLOCK_SZ);
		points[3].y = Y(data, y+(BLOCK_SZ/2));
		points[4].x = X(data, x+(BLOCK_SZ/2));
		points[4].y = Y(data, y);
		
		for(i=0; i<World.NumChecks; i++)
		    if ((World.check[i].x == xi) && (World.check[i].y == yi)) {
			if (i != pl->check) {
			    XDrawLines(dr->disp, dr->p_draw, dr->gc,
				       points, 5, 0); 
			} else {
			    XFillPolygon(dr->disp, dr->p_draw, dr->gc,
					 points, 5, Convex, CoordModeOrigin);
			}
		    }
		break;
		
	    case REC_LU:
		points[0].x = X(data, x);
		points[0].y = Y(data, y);
		points[1].x = X(data, x);
		points[1].y = Y(data, (y+BLOCK_SZ));
		points[2].x = X(data, (x+BLOCK_SZ));
		points[2].y = Y(data, (y+BLOCK_SZ));
		points[3].x = X(data, x);
		points[3].y = Y(data, y);
		XDrawLines(dr->disp, dr->p_draw, dr->gc, points, 4, 0); 
		break;
		
	    case REC_RU:
		points[0].x = X(data, x);
		points[0].y = Y(data, (y+BLOCK_SZ));
		points[1].x = X(data, (x+BLOCK_SZ));
		points[1].y = Y(data, (y+BLOCK_SZ));
		points[2].x = X(data, (x+BLOCK_SZ));
		points[2].y = Y(data, y);
		points[3].x = X(data, x);
		points[3].y = Y(data, (y+BLOCK_SZ));
		XDrawLines(dr->disp, dr->p_draw, dr->gc, points, 4, 0); 
		break;
		
	    case REC_LD:
		points[0].x = X(data, x);
		points[0].y = Y(data, y);
		points[1].x = X(data, x);
		points[1].y = Y(data, (y+BLOCK_SZ));
		points[2].x = X(data, (x+BLOCK_SZ));
		points[2].y = Y(data, y);
		points[3].x = X(data, x);
		points[3].y = Y(data, y);
		XDrawLines(dr->disp, dr->p_draw, dr->gc, points, 4, 0); 
		break;
		
	    case REC_RD:
		points[0].x = X(data, x);
		points[0].y = Y(data, y);
		points[1].x = X(data, (x+BLOCK_SZ));
		points[1].y = Y(data, (y+BLOCK_SZ));
		points[2].x = X(data, (x+BLOCK_SZ));
		points[2].y = Y(data, y);
		points[3].x = X(data, x);
		points[3].y = Y(data, y);
		XDrawLines(dr->disp, dr->p_draw, dr->gc, points, 4, 0); 
		break;
		
	    case FUEL:
		for(i=0; i<World.NumFuels; i++) {
		    if (((int)(World.fuel[i].pos.x/BLOCK_SZ) == xi)
			&& ((int)(World.fuel[i].pos.y/BLOCK_SZ) == yi)) {
			XDrawRectangle(dr->disp, dr->p_draw, dr->gc,
				       X(data, x), Y(data, y+BLOCK_SZ),
				       BLOCK_SZ, BLOCK_SZ);
			XSetForeground(dr->disp, dr->gc, dr->colors[RED].pixel);
			offset = INSIDE_WS*World.fuel[i].fuel/MAX_STATION_FUEL;
			XFillRectangle(dr->disp, dr->p_draw, dr->gc,
				       X(data, x)+1,
				       Y(data, y+BLOCK_SZ)
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
		XDrawArc(dr->disp, dr->p_draw, dr->gc,
			 X(data, x+5), Y(data, y+BLOCK_SZ-5),
			 BLOCK_SZ-10, BLOCK_SZ-10, 64*150, 64*300);
		XDrawLine(dr->disp, dr->p_draw, dr->gc,
			  X(data, x+BLOCK_SZ/2),
			  Y(data, y+BLOCK_SZ-5),
			  X(data, x+BLOCK_SZ/2+4),
			  Y(data, y+BLOCK_SZ-1));
		XDrawLine(dr->disp, dr->p_draw, dr->gc,
			  X(data, x+BLOCK_SZ/2),
			  Y(data, y+BLOCK_SZ-5),
			  X(data, x+BLOCK_SZ/2+4),
			  Y(data, y+BLOCK_SZ-9));
		XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
		break;
		
	    case CWISE_GRAV:
		XSetForeground(dr->disp, dr->gc, dr->colors[RED].pixel);
		XDrawArc(dr->disp, dr->p_draw, dr->gc,
			 X(data, x+5), Y(data, y+BLOCK_SZ-5),
			 BLOCK_SZ-10, BLOCK_SZ-10, 64*90, 64*300);
		XDrawLine(dr->disp, dr->p_draw, dr->gc,
			  X(data, x+BLOCK_SZ/2),
			  Y(data, y+BLOCK_SZ-5),
			  X(data, x+BLOCK_SZ/2-4),
			  Y(data, y+BLOCK_SZ-1));
		XDrawLine(dr->disp, dr->p_draw, dr->gc,
			  X(data, x+BLOCK_SZ/2),
			  Y(data, y+BLOCK_SZ-5),
			  X(data, x+BLOCK_SZ/2-4),
			  Y(data, y+BLOCK_SZ-9));
		XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
		break;
		
	    case POS_GRAV:
		XSetForeground(dr->disp, dr->gc, dr->colors[RED].pixel);
		XDrawArc(dr->disp, dr->p_draw, dr->gc,
			 X(data, x+1), Y(data, y+BLOCK_SZ-1),
			 INSIDE_WS, INSIDE_WS, 0, 64*360);
		XDrawLine(dr->disp, dr->p_draw, dr->gc,
			  X(data, x+BLOCK_SZ/2),
			  Y(data, y+5),
			  X(data, x+BLOCK_SZ/2),
			  Y(data, y+BLOCK_SZ-5));
		XDrawLine(dr->disp, dr->p_draw, dr->gc,
			  X(data, x+5),
			  Y(data, y+BLOCK_SZ/2),
			  X(data, x+BLOCK_SZ-5),
			  Y(data, y+BLOCK_SZ/2));
		XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
		break;
		
	    case NEG_GRAV:
		XSetForeground(dr->disp, dr->gc, dr->colors[RED].pixel);
		XDrawArc(dr->disp, dr->p_draw, dr->gc,
			 X(data, x+1), Y(data, y+BLOCK_SZ-1),
			 INSIDE_WS, INSIDE_WS, 0, 64*360);
		XDrawLine(dr->disp, dr->p_draw, dr->gc,
			  X(data, x+5),
			  Y(data, y+BLOCK_SZ/2),
			  X(data, x+BLOCK_SZ-5),
			  Y(data, y+BLOCK_SZ/2));
		XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
		break;
		
	    case WORMHOLE:
		{
		    static int wormOffset[8][3] = {
			10, 10, 10,
			5, 10, 10,
			0, 10, 10,
			0, 5, 10,
			0, 0, 10,
			5, 0, 10,
			10, 0, 10,
			10, 5, 10,
		    };
#define _O 	wormOffset[pl->wormDrawCount]
#define ARC(_x, _y, _w)						\
    XDrawArc(dr->disp, dr->p_draw, dr->gc,			\
	     X(data, x) + (_x),					\
	     Y(data, y + BLOCK_SZ) + (_y),			\
	     INSIDE_WS - (_w), INSIDE_WS - (_w), 0, 64 * 360)

		    if (World.wormHoles[wormXY(xi, yi)].type != WORM_OUT) {
			XSetForeground(dr->disp, dr->gc,
				       dr->colors[RED].pixel);
			ARC(0, 0, 0);
			ARC(_O[0], _O[1], _O[2]);
			ARC(_O[0] * 2, _O[1] * 2, _O[2] * 2);
			XSetForeground(dr->disp, dr->gc,
				       dr->colors[BLUE].pixel);
		    }
		    break;
		}

	    case SPACE: {
                int bg_point_dist = pl->map_point_distance;
                
		if (xdot || ydot
		    || ((bg_point_dist != 0) && ((xi%bg_point_dist)==0)
			&& ((yi%bg_point_dist)==0)))
		    XDrawRectangle(dr->disp, dr->p_draw, dr->gc,
				   X(data, (xb+rxb) * BLOCK_SZ + BLOCK_SZ / 2),
				   Y(data, (yb+ryb) * BLOCK_SZ + BLOCK_SZ / 2),
				   2, 2);

#ifdef DRAW_GRAV_VECTORS
		XSetForeground(dr->disp, dr->gc, dr->colors[RED].pixel);
		XDrawLine(dr->disp, dr->p_draw, dr->gc,
			  X(data, x+BLOCK_SZ/2),
			  Y(data, y+BLOCK_SZ/2),
			  X(data, x+BLOCK_SZ/2 + 50*World.gravity[xi][yi].x),
			  Y(data, y+BLOCK_SZ/2 + 50*World.gravity[xi][yi].y));
		XSetForeground(dr->disp, dr->gc, dr->colors[WHITE].pixel);
		XDrawPoint(dr->disp, dr->p_draw, dr->gc,
			   X(data, x+BLOCK_SZ/2),
			   Y(data, y+BLOCK_SZ/2));
		XSetForeground(dr->disp, dr->gc, dr->colors[BLUE].pixel);
#endif
		break;
            }
	    case BASE:
		break;

	    case TREASURE:
		for(i=0; i < World.NumTreasures; i++)
		    if ((World.treasures[i].pos.x == xi) 
			&& (World.treasures[i].pos.y == yi)) {
			if (pl->team == World.treasures[i].team)
			    XSetForeground(dr->disp, dr->gc, 
					   dr->colors[BLUE].pixel);
			else
			    XSetForeground(dr->disp, dr->gc, 
					   dr->colors[RED].pixel);
			XDrawLine(dr->disp, dr->p_draw, dr->gc,
				  X(data, x),Y(data, y),
				  X(data, x),Y(data, y + BLOCK_SZ/2));
			XDrawLine(dr->disp, dr->p_draw, dr->gc,
				  X(data, x + BLOCK_SZ),Y(data, y),
				  X(data, x + BLOCK_SZ),
				  Y(data, y + BLOCK_SZ/2));
			XDrawLine(dr->disp, dr->p_draw, dr->gc,
				  X(data, x),Y(data, y),
				  X(data, x + BLOCK_SZ),Y(data, y));
			XDrawArc(dr->disp, dr->p_draw, dr->gc,
				 X(data, x),
				 Y(data, y + BLOCK_SZ),
				 BLOCK_SZ, BLOCK_SZ, 0, 64*180);
			{
			    char s[2]; s[1] = '\0'; s[0]
				= '0' + World.treasures[i].team;
			    XDrawString(dr->disp, dr->p_draw, dr->gc,
					X(data, x+BLOCK_SZ/2),
					Y(data, y+BLOCK_SZ/2), s, 1);
			}

			XSetForeground(dr->disp, dr->gc, 
				       dr->colors[BLUE].pixel);
		    }		    
		break;
	    default:
		break;
	    }
	}
    }
#undef xadj
#undef yadj
}



void Draw_world_radar(int draw)
{
    int xi, yi;
    float xs, ys;
    player *dr = Players[draw];
    
    
    xs = 256.0 / World.x;
    ys = (float)RadarHeight / World.y;
   
    if (dr->s_radar == dr->radar) 
	XSetPlaneMask(dr->disp, dr->gcr, 
		      AllPlanes&(~(dr->dpl_1[0]|dr->dpl_1[1])));
    else {
	/* Clear radar */
	XSetForeground(dr->disp, dr->gcr, dr->colors[BLACK].pixel);
	XFillRectangle(dr->disp, dr->s_radar, dr->gcr, 0, 0, 256, RadarHeight);
    }
    XSetForeground(dr->disp, dr->gcr, dr->colors[BLUE].pixel);

    for (xi=0; xi<World.x; xi++)
	for (yi=0; yi<World.y; yi++)
	    switch (World.block[xi][yi]) {
	    case FILLED:
	    case FILLED_NO_DRAW:
	    case REC_LU:
	    case REC_RU:
	    case REC_LD:
	    case REC_RD:
	    case FUEL:
		XDrawPoint(dr->disp, dr->s_radar, dr->gcr,
			   (int)((xi*xs)+(0.5*xs)),
			   (int)(RadarHeight-((yi*ys)+(0.5*ys))));
		break;
	    default:
		break;
	    }
    
    if (dr->s_radar == dr->radar)
	XSetPlaneMask(dr->disp, dr->gcr, 
		      AllPlanes&(~(dr->dpl_2[0]|dr->dpl_2[1])));
}

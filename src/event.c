/* event.c,v 1.3 1992/05/11 15:31:15 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xproto.h>
#include "pilot.h"
#include "map.h"
#include "score.h"

#define SWAP(_a, _b)	    {double _tmp = _a; _a = _b; _b = _tmp;}

#define CONTROL_DELAY	100

extern player *Players[];
extern World_map    World;
extern int Antall;

static char msg[MSG_LEN];

void Pick_startpos(int);
void Place_mine(int);



void Refuel(int ind)
{
    player *pl = Players[ind];
    int i, min;
    double l, min_dist=DBL_MAX;


    if (!BIT(pl->have, OBJ_REFUEL))
	return;

    for (i=0; i<World.Ant_fuel; i++) {
	l=LENGTH(pl->pos.x-World.fuel[i].pos.x, 
		 pl->pos.y-World.fuel[i].pos.y);
	if (min_dist > l) {
	    min_dist = l;
	    min = i;
	}
    }

    SET_BIT(pl->used, OBJ_REFUEL);
    pl->fs=min;
}



void Key_event(int ind, XEvent *event)
{
    KeySym  KS;
    int	    i, xi, yi, min_ind;
    player  *pl;
    double  min, l;


    pl=Players[ind];

    KS=XLookupKeysym(&(event->xkey), 0);

    if (!BIT(pl->status, PLAYING) &&
	KS!=XK_p && KS!=XK_Escape && KS!=XK_Next && KS!=XK_Prior &&
	KS!=XK_Up && KS!=XK_Down && KS!=XK_Left && KS!=XK_Right &&
	KS!=XK_KP_Multiply && KS!=XK_KP_Divide &&
	KS!=XK_Linefeed && KS!=XK_KP_Add && KS!=XK_KP_Subtract)
	return;


    if (event->type == KeyPress) {	/* --- KEYPRESS --- */
	switch (KS) {

	case XK_Left:
	case XK_Right:
	case XK_Next:
	case XK_Prior:
	    i = get_ind[pl->lock.pl_id];
	    if (Antall > 1)
		do {
		    if ((KS==XK_Prior) || (KS==XK_Left))
			i--;
		    else
			i++;
		    i = MOD(i, Antall);
		    pl->lock.pl_id = Players[i]->id;
		    pl->lock.tagged = LOCK_PLAYER;
		} while (i == ind);
	    break;

	case XK_Up:
	case XK_Down:
	case XK_Select:
	    min = DBL_MAX;
	    for (i=0; i<Antall; i++) {
		if (TEAM(ind, i) || !BIT(Players[i]->status, PLAYING))
		    continue;
		l=LENGTH(Players[i]->pos.x - pl->pos.x,
			 Players[i]->pos.y - pl->pos.y);
		if (BIT(Players[i]->status, PLAYING) && l<min && i!=ind) {
		    min=l;
		    min_ind=i;
		}
	    }
	    if (min < DBL_MAX) {
		pl->lock.pl_id=Players[min_ind]->id;
		pl->lock.tagged = LOCK_PLAYER;
	    } else
		pl->lock.tagged = LOCK_NONE;
	    break;

	case XK_Home:
	    xi=(int)pl->pos.x/WORLD_SPACE;
	    yi=(int)pl->pos.y/WORLD_SPACE;
	    if ((pl->velocity<(0.5 + LENGTH(World.gravity[xi][yi].x,
					    World.gravity[xi][yi].y))) &&
		(World.type[xi][yi]==PORT)) {
		msg[0]='\0';
		for (i=0; i<World.Ant_start; i++) {
		    if ((World.Start_points[i].x==xi) &&
			(World.Start_points[i].y==yi) && (i!=pl->home_base)) {
			pl->home_base=i;
			sprintf(msg, "%s has changed home base.", pl->name);
		    }
		}
		for (i=0; i<Antall; i++)
		    if ((i!=ind) && (pl->home_base==Players[i]->home_base)) {
			Pick_startpos(i);
			sprintf(msg, "%s has taken over %s's home base.",
				pl->name, Players[i]->name);
		    }
		if (msg[0])
		    Set_message(msg);
	    }
	    break;

	case XK_Meta_R:
	case XK_space:
	    if (BIT(pl->have, OBJ_SHIELD))
		SET_BIT(pl->used, OBJ_SHIELD);
	    break;

	case XK_Return:
	    Fire_shot(ind, OBJ_SHOT, pl->dir);
	    for (i=0; i<pl->extra_shots; i++) {
		Fire_shot(ind, OBJ_SHOT,
			  MOD(pl->dir + (1+i)*SHOTS_ANGLE, RESOLUTION));
		Fire_shot(ind, OBJ_SHOT,
			  MOD(pl->dir - (1+i)*SHOTS_ANGLE, RESOLUTION));
	    }
	    if (BIT(pl->have, OBJ_REAR_SHOT))
		Fire_shot(ind, OBJ_SHOT,
			  MOD(pl->dir+RESOLUTION/2, RESOLUTION));
	    break;
	    
	case XK_backslash:
	case XK_Linefeed:
	    if (pl->missiles > 0)
		Fire_shot(ind, OBJ_SMART_SHOT, pl->dir);
	    break;
	    
	case XK_Tab:
	    if (pl->mines > 0) {
		Place_mine(ind);
		pl->mines--;
	    }
	    break;

	case XK_a:
	    pl->turnacc+=pl->turnspeed;
	    break;
	    
	case XK_s:
	    pl->turnacc-=pl->turnspeed;
	    break;

	case XK_q:
	    TOGGLE_BIT(pl->status, SELF_DESTRUCT);
	    if (BIT(pl->status, SELF_DESTRUCT))
		pl->count = 150;
	    break;

	case XK_i:
	    TOGGLE_BIT(pl->status, ID_MODE);
	    break;

	case XK_p:
	    xi=(int)pl->pos.x/WORLD_SPACE;
	    yi=(int)pl->pos.y/WORLD_SPACE;
	    if ((pl->velocity<(0.5 + LENGTH(World.gravity[xi][yi].x,
					    World.gravity[xi][yi].y))) &&
		(World.Start_points[pl->home_base].x==xi &&
		 World.Start_points[pl->home_base].y==yi)) {
		if (!BIT(pl->status, PAUSE)) {	    /* Turn pause mode on */
		    pl->count = MIN_PAUSE;
		    SET_BIT(pl->status, PAUSE);
		    CLR_BIT(pl->status, SELF_DESTRUCT|PLAYING);
		    XAutoRepeatOn(pl->disp);	    /* Auto repeat on */
		} else
		    if (pl->count <= 0) {
			CLR_BIT(pl->status, PAUSE);
			if (!BIT(pl->status, GAME_OVER))
			    SET_BIT(pl->status, PLAYING);
		    }
	    }
	    break;
		
	case XK_v:
	    TOGGLE_BIT(pl->status, VELOCITY_GAUGE);
	    break;

	case XK_c:
	    if (!BIT(pl->have, OBJ_COMPASS))
		break;
	    TOGGLE_BIT(pl->used, OBJ_COMPASS);
	    if (BIT(pl->used, OBJ_COMPASS))
		if (Antall > 1) {
		    pl->lock.tagged=LOCK_PLAYER;
		} else
		    pl->lock.tagged=LOCK_NONE;
	    break;

	case XK_Escape:
	    SWAP(pl->power, pl->power_s);
	    SWAP(pl->turnspeed, pl->turnspeed_s);
	    SWAP(pl->turnresistance, pl->turnresistance_s);
	    pl->control_count = CONTROL_DELAY;
	    break;

	case XK_f:		    /* Thanks to our disagreement, you are */
	case XK_Control_L:	    /* left free to choose. :) */
	    pl->fuel_count = 150;
	    Refuel(ind);
	    break;

	case XK_KP_Multiply:
	    pl->power *= 1.10;
	    pl->power=MIN(pl->power, MAX_PLAYER_POWER);
	    pl->control_count = CONTROL_DELAY;
	    break;

	case XK_KP_Divide:
	    pl->power *= 0.90;
	    pl->power=MAX(pl->power, MIN_PLAYER_POWER);
	    pl->control_count = CONTROL_DELAY;
	    break;

	case XK_KP_Add:
	    if (pl->turnacc == 0.0)
		pl->turnspeed *= 1.05;
	    pl->turnspeed=MIN(pl->turnspeed, MAX_PLAYER_TURNSPEED);
	    pl->control_count = CONTROL_DELAY;
	    break;

	case XK_KP_Subtract:
	    if (pl->turnacc == 0.0)
		pl->turnspeed *= 0.95;
	    pl->turnspeed=MAX(pl->turnspeed, MIN_PLAYER_TURNSPEED);
	    pl->control_count = CONTROL_DELAY;
	    break;

	case XK_KP_0:
	case XK_0:
	    if (BIT(pl->used, OBJ_TRAINER))
		pl->vel.x=pl->vel.y=0.0;
	    pl->turnacc = 0.0;
	    break;

	case XK_Shift_L:
	case XK_Shift_R:
	    SET_BIT(pl->status, THRUSTING);
	    break;

	case XK_Delete:
	case XK_BackSpace:
	    if (BIT(pl->have, OBJ_CLOAKING_DEVICE))
		TOGGLE_BIT(pl->used, OBJ_CLOAKING_DEVICE);
	    break;

#ifdef	CHEAT
	case XK_KP_F1:
	    if (!BIT(pl->have, OBJ_TRAINER))
		return;
	    if (BIT(pl->used, OBJ_TRAINER)) {
		SET_BIT(pl->status, GRAVITY);
		sprintf(msg, "%s is no longer cheating on us.", pl->name);
		Set_message(msg);
		pl->mychar=' ';
	    } else {
		sprintf(msg, "%s has started cheating.", pl->name);
		Set_message(msg);
		pl->mychar='C';
	    }
	    TOGGLE_BIT(pl->used, OBJ_TRAINER);
	    Set_label_strings();
	    break;

	case XK_KP_F4:
	    if (!BIT(pl->have, OBJ_TRAINER))
		return;
	    if (BIT(pl->used, OBJ_TRAINER)) {
		SET_BIT(pl->status, GRAVITY);
		sprintf(msg, "%s has reentered our dimension.", pl->name);
		Set_message(msg);
		pl->mychar=' ';
	    } else {
		CLR_BIT(pl->status, GRAVITY);
		sprintf(msg, "%s has entered the twilight zone.", pl->name);
		Set_message(msg);
		pl->mychar='T';
	    }
	    TOGGLE_BIT(pl->used, OBJ_TRAINER);
	    Set_label_strings();
	    break;

	case XK_KP_Tab:
	    pl->mines+=100;
	    break;
	case XK_KP_Enter:
	    pl->missiles+=100;
	    break;
	case XK_KP_Separator:
	    pl->extra_shots+=5;
	    break;
	case XK_KP_Decimal:
	    pl->fuel=pl->max_fuel;
	    break;
#endif

	default:
	    break;
	}
    }



    else if (event->type == KeyRelease) {	/* --- KEYRELEASE --- */
	switch (KS) {
	case XK_a:
	    pl->turnacc-=pl->turnspeed;
	    break;

	case XK_s:
	    pl->turnacc+=pl->turnspeed;
	    break;

	case XK_Control_L:
	case XK_f:
	    CLR_BIT(pl->used, OBJ_REFUEL);
	    pl->fuel_count=20;
	    break;

	case XK_Meta_R:
	case XK_space:
	    CLR_BIT(pl->used, OBJ_SHIELD);
	    break;

	case XK_Shift_L:
	case XK_Shift_R:
	    CLR_BIT(pl->status, THRUSTING);
	    break;

	default:
	    break;
	}
    }
}

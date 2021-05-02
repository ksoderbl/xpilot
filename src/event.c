/* event.c,v 1.12 1992/06/28 05:38:14 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/keysym.h>
#ifdef	apollo
#    include <X11/ap_keysym.h>
#endif

#include "global.h"
#include "score.h"
#include "map.h"

#ifndef	lint
static char sourceid[] =
    "@(#)event.c,v 1.12 1992/06/28 05:38:14 bjoerns Exp";
#endif

#define SWAP(_a, _b)	    {double _tmp = _a; _a = _b; _b = _tmp;}

#define CONTROL_DELAY	100


/*
 * Globals.
 */
static char		msg[MSG_LEN];



void Refuel(int ind)
{
    player *pl = Players[ind];
    int i, min;
    double l, min_dist=DBL_MAX;


    if (!BIT(pl->have, OBJ_REFUEL))
	return;

    for (i=0; i<World.NumFuels; i++) {
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


static keys_t Lookup_key(KeySym ks, player *pl)
{
    keys_t ret = KEY_DUMMY;
    int i = 0;


    while (i < MAX_KEY_DEFS && pl->keyDefs[i].key)
	if (pl->keyDefs[i].keysym == ks) {
	    ret = pl->keyDefs[i].key;
	    break;
	}
    else
	i++;

    return (ret);
}


void Key_event(int ind, XEvent *event)
{
    KeySym  	KS;
    int	    	i, xi, yi, min_ind;
    player  	*pl;
    double  	min, l;
    keys_t	key;


    pl = Players[ind];

    KS = XLookupKeysym(&(event->xkey), 0);
    key = Lookup_key(KS, pl);

    if (!BIT(pl->status, PLAYING))		/* Allow these functions */
	switch (key) {				/* while you're 'dead'. */
	case KEY_PAUSE:
	case KEY_LOCK_NEXT:
	case KEY_LOCK_PREV:
	case KEY_ID_MODE:
	case KEY_TOGGLE_VELOCITY:
	case KEY_TOGGLE_COMPASS:
	case KEY_SWAP_SETTINGS:
	case KEY_INCREASE_POWER:
	case KEY_DECREASE_POWER:
	case KEY_INCREASE_TURNSPEED:
	case KEY_DECREASE_TURNSPEED:
	case KEY_SLOWDOWN:
	case KEY_SPEEDUP:
	    break;
	default:
	    return;
	}


    if (event->type == KeyPress) {	/* --- KEYPRESS --- */
	switch (key) {

	case KEY_SLOWDOWN:
	    if (Owner(pl->realname)) {
		Delay += 5;
		break;
	    }
	case KEY_SPEEDUP:
	    if (Owner(pl->realname)) {
		Delay -= 5;
		if (Delay < 0)
		    Delay = 0;
		break;
	    }
	case KEY_LOCK_NEXT:
	case KEY_LOCK_PREV:
	    i = GetInd[pl->lock.pl_id];
	    if (NumPlayers > 1)
		do {
		    if ((KS==XK_Prior) || (KS==XK_Left))
			i--;
		    else
			i++;
		    i = MOD(i, NumPlayers);
		    pl->lock.pl_id = Players[i]->id;
		    pl->lock.tagged = LOCK_PLAYER;
		} while (i == ind);
	    break;

	case KEY_LOCK_CLOSE:
	    min = DBL_MAX;
	    for (i=0; i<NumPlayers; i++) {
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

	case KEY_CHANGE_HOME:
	    xi=(int)pl->pos.x/BLOCK_SZ;
	    yi=(int)pl->pos.y/BLOCK_SZ;
	    if (/*(pl->velocity<(0.5 + LENGTH(World.gravity[xi][yi].x,
					    World.gravity[xi][yi].y))) && */
		(World.block[xi][yi]==BASE)) {
		msg[0]='\0';
		for (i=0; i<World.NumBases; i++) {
		    if ((World.base[i].x==xi) &&
			(World.base[i].y==yi) && (i!=pl->home_base)) {
			pl->home_base=i;
			sprintf(msg, "%s has changed home base.", pl->name);
		    }
		}
		for (i=0; i<NumPlayers; i++)
		    if ((i!=ind) && (pl->home_base==Players[i]->home_base)) {
			Pick_startpos(i);
			sprintf(msg, "%s has taken over %s's home base.",
				pl->name, Players[i]->name);
		    }
		if (msg[0])
		    Set_message(msg);
	    }
	    break;

	case KEY_SHIELD:
	    if (BIT(pl->have, OBJ_SHIELD))
		SET_BIT(pl->used, OBJ_SHIELD);
	    break;

	case KEY_FIRE_SHOT:
	    Fire_shot(ind, OBJ_SHOT, pl->dir);
	    for (i=0; i<pl->extra_shots; i++) {
		Fire_shot(ind, OBJ_SHOT,
			  MOD(pl->dir + (1+i)*SHOTS_ANGLE, RES));
		Fire_shot(ind, OBJ_SHOT,
			  MOD(pl->dir - (1+i)*SHOTS_ANGLE, RES));
	    }
	    if (BIT(pl->have, OBJ_REAR_SHOT))
		Fire_shot(ind, OBJ_SHOT,
			  MOD(pl->dir+RES/2, RES));
	    break;
	    
	case KEY_FIRE_MISSILE:
	    if (pl->missiles > 0)
		Fire_shot(ind, OBJ_SMART_SHOT, pl->dir);
	    break;
	    
	case KEY_DROP_MINE:
	    if (pl->mines > 0) {
		Place_mine(ind);
		pl->mines--;
	    }
	    break;

	case KEY_TURN_LEFT:
	    pl->turnacc+=pl->turnspeed;
	    break;
	    
	case KEY_TURN_RIGHT:
	    pl->turnacc-=pl->turnspeed;
	    break;

	case KEY_SELF_DESTRUCT:
	    TOGGLE_BIT(pl->status, SELF_DESTRUCT);
	    if (BIT(pl->status, SELF_DESTRUCT))
		pl->count = 150;
	    break;

	case KEY_ID_MODE:
	    TOGGLE_BIT(pl->status, ID_MODE);
	    break;

	case KEY_PAUSE:
	    xi = (int)pl->pos.x / BLOCK_SZ;
	    yi = (int)pl->pos.y / BLOCK_SZ;
	    if ((pl->velocity<(0.5 + LENGTH(World.gravity[xi][yi].x,
					    World.gravity[xi][yi].y))) &&
		(World.base[pl->home_base].x == xi &&
		 World.base[pl->home_base].y == yi)) {
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
		
	case KEY_TOGGLE_VELOCITY:
	    TOGGLE_BIT(pl->status, VELOCITY_GAUGE);
	    break;

	case KEY_TOGGLE_COMPASS:
	    if (!BIT(pl->have, OBJ_COMPASS))
		break;
	    TOGGLE_BIT(pl->used, OBJ_COMPASS);
	    if (BIT(pl->used, OBJ_COMPASS))
		if (NumPlayers > 1) {
		    pl->lock.tagged = LOCK_PLAYER;
		} else
		    pl->lock.tagged = LOCK_NONE;
	    break;

	case KEY_SWAP_SETTINGS:
	    SWAP(pl->power, pl->power_s);
	    SWAP(pl->turnspeed, pl->turnspeed_s);
	    SWAP(pl->turnresistance, pl->turnresistance_s);
	    pl->control_count = CONTROL_DELAY;
	    break;

	case KEY_REFUEL:
	    pl->fuel_count = 150;
	    Refuel(ind);
	    break;

	case KEY_INCREASE_POWER:
	    pl->power *= 1.10;
	    pl->power=MIN(pl->power, MAX_PLAYER_POWER);
	    pl->control_count = CONTROL_DELAY;
	    break;

	case KEY_DECREASE_POWER:
	    pl->power *= 0.90;
	    pl->power=MAX(pl->power, MIN_PLAYER_POWER);
	    pl->control_count = CONTROL_DELAY;
	    break;

	case KEY_INCREASE_TURNSPEED:
	    if (pl->turnacc == 0.0)
		pl->turnspeed *= 1.05;
	    pl->turnspeed=MIN(pl->turnspeed, MAX_PLAYER_TURNSPEED);
	    pl->control_count = CONTROL_DELAY;
	    break;

	case KEY_DECREASE_TURNSPEED:
	    if (pl->turnacc == 0.0)
		pl->turnspeed *= 0.95;
	    pl->turnspeed=MAX(pl->turnspeed, MIN_PLAYER_TURNSPEED);
	    pl->control_count = CONTROL_DELAY;
	    break;
/*
	case XK_KP_0:
	case XK_0:
	    if (BIT(pl->used, OBJ_TRAINER))
		pl->vel.x=pl->vel.y=0.0;
	    pl->turnacc = 0.0;
	    break;
*/
	case KEY_THRUST:
	    SET_BIT(pl->status, THRUSTING);
	    break;

	case KEY_CLOAK:
	    if (pl->cloaks > 0)
	    {
		pl->updateVisibility = 1;
		TOGGLE_BIT(pl->used, OBJ_CLOAKING_DEVICE);
	    }
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
	switch (key) {
	case KEY_TURN_LEFT:
	    pl->turnacc-=pl->turnspeed;
	    break;

	case KEY_TURN_RIGHT:
	    pl->turnacc+=pl->turnspeed;
	    break;

	case KEY_REFUEL:
	    CLR_BIT(pl->used, OBJ_REFUEL);
	    pl->fuel_count=20;
	    break;

	case KEY_SHIELD:
	    CLR_BIT(pl->used, OBJ_SHIELD);
	    break;

	case KEY_THRUST:
	    CLR_BIT(pl->status, THRUSTING);
	    break;

	default:
	    break;
	}
    }
}

/* $Id: collision.c,v 1.15 1992/08/27 00:25:47 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include "global.h"
#include "map.h"
#include "score.h"
#include "robot.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: collision.c,v 1.15 1992/08/27 00:25:47 bjoerns Exp $";
#endif

#define in_range(o1, o2, r)	( \
    (ABS((o1)->pos.x-(o2)->pos.x)<(r) && ABS((o1)->pos.y-(o2)->pos.y)<(r)) \
				 ? true : false)


extern long KILLING_SHOTS;
static char msg[MSG_LEN];



int Rate(int winner, int looser)
{
    int t;


    t = ((RATE_SIZE/2) * RATE_RANGE) / (ABS(looser-winner) + RATE_RANGE);

    if (looser > winner)
	t = RATE_SIZE - t;

    return (t);
}


void Check_collision(void)
{
    int i, j, x, y, killer, range, sc, t;
    player *pl;
    int xd, yd;
    bool labels=false;


    /*
     * Collision detection.
     */
    for (i=0; i<NumPlayers; i++) {
	pl=Players[i];
	if (!BIT(pl->status, PLAYING) || BIT(pl->status, GAME_OVER))
	    continue;

	if (pl->pos.x<0 || pl->pos.y<0 ||
	    pl->pos.x>=(World.x*BLOCK_SZ) ||
	    pl->pos.y>=(World.y*BLOCK_SZ))
	    SET_BIT(pl->status, KILLED);

	/* Player - player */
	if (BIT(World.rules->mode, CRASH_WITH_PLAYER)) {
	    for (j=i+1; j<NumPlayers; j++)
		if (in_range((object *)pl,
			     (object *)Players[j], 2*SHIP_SZ-6) &&
		    (BIT(Players[j]->status, PLAYING) &&
		     !BIT(Players[j]->status, GAME_OVER)))
		    if (!TEAM(i, j) && !PSEUDO_TEAM(i,j)) {
			sprintf(msg, "%s%s and %s%s crashed.",
                                pl->name,
				pl->robot_mode==RM_OBJECT?"s tank":"",
                                Players[j]->name,
				Players[j]->robot_mode==RM_OBJECT?"s tank":""
                                );
			Set_message(msg);
/*			if (!BIT(pl->status, KILLED))
			    SCORE(i, PTS_PR_PL_CRASH);
			if (!BIT(Players[j]->status, KILLED))
			    SCORE(j, PTS_PR_PL_CRASH);
*/                      Add_fuel(&(Players[i]->fuel),ED_PL_CRASH);
                        Add_fuel(&(Players[j]->fuel),ED_PL_CRASH);
			SET_BIT(pl->status, KILLED);
			SET_BIT(Players[j]->status, KILLED);

			if (Players[j]->robot_mode != RM_NOT_ROBOT &&
			    Players[j]->robot_lock == LOCK_PLAYER &&
			    Players[j]->robot_lock_id == pl->id)
			    Players[j]->robot_lock = LOCK_NONE;

			if (pl->robot_mode != RM_NOT_ROBOT &&
			    pl->robot_lock == LOCK_PLAYER &&
			    pl->robot_lock_id == Players[j]->id)
			    pl->robot_lock = LOCK_NONE;
		    }
	}

	/* Player checkpoint */
	if (BIT(World.rules->mode, TIMING))
	    if (LENGTH(Players[i]->pos.x -
		       (World.check[Players[i]->check].x*BLOCK_SZ),
		       Players[i]->pos.y -
		       (World.check[Players[i]->check].y*BLOCK_SZ))
		< 200) {

		if (Players[i]->check == 0) {
		    Players[i]->round++;
		    if (((Players[i]->best_lap > 
			  Players[i]->time - Players[i]->last_lap) ||
			 (Players[i]->best_lap == 0)) && 
			(Players[i]->time != 0))
			Players[i]->best_lap = 
			    Players[i]->time - Players[i]->last_lap;
		    Players[i]->last_lap_time = Players[i]->time 
			- Players[i]->last_lap;
		    Players[i]->last_lap = Players[i]->time;
		}

		Players[i]->check++;

		if (Players[i]->check == World.NumChecks)
		    Players[i]->check = 0;
	    }

	/*
	 * Collision between a player and an object.
	 */
	for (j=0; j<NumObjs; j++) {
            switch (Obj[j]->type) {
            case OBJ_TORPEDO:
                if (pl->id==Obj[j]->id && Obj[j]->info<8)
                    continue;
                else
                    range=SHIP_SZ+TORPEDO_RANGE;
                break;
            case OBJ_SMART_SHOT:
            case OBJ_HEAT_SHOT:
                range=SHIP_SZ+MISSILE_RANGE;
                break;
            case OBJ_MINE:
                range=SHIP_SZ+MINE_RANGE;
                break;
            case OBJ_DUST:
                range=SHIP_SZ+Obj[j]->dir;
                break;
            default:
                range=SHIP_SZ;
                break;
            }
            
	    if (BIT(pl->status, KILLED) ||
		(!in_range((object *)pl, Obj[j], range)))
		continue;

	    if (    (Obj[j]->type==OBJ_SPARK && Obj[j]->id==pl->id)
                 || (   Obj[j]->type==OBJ_MINE
                     && (Obj[j]->id != -1)
                     && (Obj[j]->id==pl->id || TEAM(GetInd[Obj[j]->id], i))))
		continue;

	    if ((Obj[j]->id != -1) &&
		(TEAM(i, GetInd[Obj[j]->id])) &&
		(pl->id != Obj[j]->id))
		continue;
	    else
		Obj[j]->life=0;
	    
	    Delta_mv((object *)pl, (object *)Obj[j]);

	    /*
	     * Object collision.
	     */
            if (pl->robot_mode!=RM_OBJECT)
	    switch (Obj[j]->type) {
	    case OBJ_WIDEANGLE_SHOT:
		pl->extra_shots++;
		break;
	    case OBJ_ECM:
		pl->ecms++;
		break;
	    case OBJ_SENSOR_PACK:
		pl->sensors++;
		pl->updateVisibility = 1;
		break;
            case OBJ_AFTER_BURNER:
                SET_BIT(pl->have,OBJ_AFTER_BURNER);
                if ((pl->after_burners += 1) > MAX_AFTER_BURNER)
                    pl->after_burners = MAX_AFTER_BURNER;
                break;
	    case OBJ_REAR_SHOT:
		SET_BIT(pl->have, OBJ_REAR_SHOT);
		pl->rear_shots++;
		break;
	    case OBJ_SMART_SHOT_PACK:
		pl->missiles += 4;
		break;
	    case OBJ_CLOAKING_DEVICE:
		SET_BIT(pl->have, OBJ_CLOAKING_DEVICE);
		pl->cloaks++;
		pl->updateVisibility = 1;
		break;
	    case OBJ_ENERGY_PACK:
                Add_fuel(&(pl->fuel),ENERGY_PACK_FUEL);
		pl->fuel.count = FUEL_NOTIFY;
		break;
	    case OBJ_MINE_PACK:
		pl->mines+=1 + (rand()&1);
		break;
	    case OBJ_TANK: {
                    int c=pl->fuel.current;

                    if (pl->fuel.no_tanks<MAX_TANKS) {
                        /* set a new, empty tank in the list. update max-fuel */
                        int no = ++(pl->fuel.no_tanks);

		        SET_BIT(pl->have,OBJ_TANK);
                        pl->fuel.current = no;
                        pl->fuel.max += TANK_CAP(no);
                        pl->fuel.tank[no] = 0;
		        pl->emptymass += TANK_MASS;
                    }
                    Add_fuel(&(pl->fuel),TANK_FUEL(pl->fuel.current));
	            pl->fuel.count = FUEL_NOTIFY;
                    pl->fuel.current = c;
		    break;
                }
	    case OBJ_MINE:
		if (Obj[j]->id == -1)
		    sprintf(msg,"%s hit a %s mine.",
                            pl->name,Obj[j]->status ? "moving" : "");
		else
		{
		    sprintf(msg, "%s hit mine %s by %s.", pl->name,
			    Obj[j]->status ? "thrown" : "dropped",
			    Players[killer=GetInd[Obj[j]->id]]->name);
		    sc = Rate(Players[killer]->score, pl->score) / 6;
		    SCORE(killer, sc);
		    SCORE(i, -sc);
		}
		Set_message(msg);
		break;
	    default:
		break;
	    }

	    if (!BIT(Obj[j]->type, KILLING_SHOTS))
		continue;

	    if (BIT(pl->used, OBJ_SHIELD)) {
		switch (Obj[j]->type) {
                case OBJ_DUST:
                    break;

		case OBJ_TORPEDO:
		case OBJ_HEAT_SHOT:
		case OBJ_SMART_SHOT:
		    Add_fuel(&(pl->fuel),ED_SMART_SHOT_HIT);
		    pl->forceVisible += 2;
		    break;

		case OBJ_SHOT:
		case OBJ_CANNON_SHOT:
		    Add_fuel(&(pl->fuel),ED_SHOT_HIT);
		    pl->forceVisible += 1;
		    break;

		default:
		    printf("You were hit by what?\n");
		    break;
		}
	    } else {
                switch (Obj[j]->type) {
                    case OBJ_TORPEDO:
                        Add_fuel(&(pl->fuel),ED_SMART_SHOT_HIT);
                        if (rand()&3) break;
                    case OBJ_SHOT:
                    case OBJ_SMART_SHOT:
                    case OBJ_HEAT_SHOT:
		        pl->forceVisible += 1;
                        sprintf(msg, "%s was shot down by %s.", pl->name,
                                     Players[killer=Obj[j]->id==-1 ? i :GetInd[Obj[j]->id]]->name);
                        SET_BIT(pl->status, KILLED);
                        if (pl->robot_mode!=RM_OBJECT) {
                            if (killer == i) {
                                strcat(msg, " How strange!...");
                                SCORE(i, PTS_PR_PL_SHOT);
                            } else {
                                sc = Rate(Players[killer]->score, pl->score);
                                SCORE(killer, sc);
                                SCORE(i, -sc);
                            }
                            Set_message(msg);
                        }
                        break;
                    case OBJ_CANNON_SHOT:
		        SET_BIT(pl->status, KILLED);
                        if (pl->robot_mode!=RM_OBJECT) {
		            sprintf(msg, "%s was hit by cannonfire.", pl->name);
		            Set_message(msg);
		            SCORE(i, -Rate(CANNON_RATING, pl->score)/4);
                        }
                        break;
                    case OBJ_DUST:
		        pl->forceVisible += 1;
                        Add_fuel(&(pl->fuel),ED_SHOT_HIT*Obj[j]->dir/2);
                        if (pl->fuel.sum<=ED_SHOT_HIT)
                        {
   		            SET_BIT(pl->status, KILLED);
                            if (pl->robot_mode!=RM_OBJECT) {
                                sprintf(msg, "%s was hit by star dust.", pl->name);
		                Set_message(msg);
	                        SCORE(i, -Rate(DUST_RATING, pl->score)/4);
                            }
                        }
                    default:
                        break;
                }
            }
	}



	/* Player - wall */
	if (!(BIT(pl->used, OBJ_TRAINER) || BIT(pl->status, KILLED))) {
	    for(j=0; j<3 && !BIT(pl->status, KILLED) &&
		!BIT(pl->status, WARPING); j++) {
		switch (World.block
    [x = (int) ((pl->pos.x + ships[pl->dir].pts[j].x) / BLOCK_SZ)]
    [y = (int) ((pl->pos.y + ships[pl->dir].pts[j].y) / BLOCK_SZ)]) {
		case FUEL:
		case FILLED:
		case FILLED_NO_DRAW:
		    if (!Landing(i, j)) {
                        crash_wall(x,y,0);
			SET_BIT(pl->status, KILLED);
			SCORE(i, -Rate(WALL_RATING, pl->score));
		    }
		    break;
		case REC_LU:
		    if ((((int)(pl->pos.x
				+ships[pl->dir].pts[j].x))
			 % BLOCK_SZ)
			<= (((int)(pl->pos.y
				   +ships[pl->dir].pts[j].y))
			    % BLOCK_SZ)) {
			if (!Landing(i, j)) {
                            crash_wall(x,y,0);
			    SET_BIT(pl->status, KILLED);
			    SCORE(i, -Rate(WALL_RATING, pl->score));
			}
		    }
		    break;
		case REC_RU:
		    if ((((int)(pl->pos.x
				+ships[pl->dir].pts[j].x)) % BLOCK_SZ)
			>= BLOCK_SZ - (((int)(pl->pos.y
				+ships[pl->dir].pts[j].y)) % BLOCK_SZ)) {
			if (!Landing(i, j)) {
                            crash_wall(x,y,0);
			    SET_BIT(pl->status, KILLED);
			    SCORE(i, -Rate(WALL_RATING, pl->score));
			}
		    }
		    break;
		case REC_LD:
		    if ((((int)(pl->pos.x
			    +ships[pl->dir].pts[j].x)) % BLOCK_SZ)
			<= BLOCK_SZ - (((int)(pl->pos.y
			+ships[pl->dir].pts[j].y)) % BLOCK_SZ)) {
                        crash_wall(x,y,0);
			SET_BIT(pl->status, KILLED);
                        if (pl->robot_mode!=RM_OBJECT) {
			    sprintf(msg,"%s crashed into the wall.",pl->name);
			    Set_message(msg);
			    SCORE(i, -Rate(WALL_RATING, pl->score));
                        }
		    }
		    break;
		case REC_RD:
		    if ((((int)(pl->pos.x
			    +ships[pl->dir].pts[j].x)) % BLOCK_SZ)
			>= (((int)(pl->pos.y
			+ships[pl->dir].pts[j].y)) % BLOCK_SZ)) {
                        crash_wall(x,y,0);
			SET_BIT(pl->status, KILLED);
                        if (pl->robot_mode!=RM_OBJECT) {
			    sprintf(msg, "%s crashed into the wall.",pl->name);
			    Set_message(msg);
			    SCORE(i, -Rate(WALL_RATING, pl->score));
                        }
		    }
		    break;
		case CANNON:
		    xd = pl->pos.x + ships[pl->dir].pts[j].x;
		    yd = pl->pos.y + ships[pl->dir].pts[j].y;

		    for(t=0; World.cannon[t].pos.x!=x ||
			World.cannon[t].pos.y!=y; t++);

		    if (World.cannon[t].dead_time > 0)
			break;

		    if (((World.cannon[t].dir == DIR_UP) &&
			(yd%BLOCK_SZ < BLOCK_SZ/3)) ||
			((World.cannon[t].dir == DIR_DOWN) &&
			(yd%BLOCK_SZ > 2*BLOCK_SZ/3)) ||
			((World.cannon[t].dir == DIR_RIGHT) &&
			(xd%BLOCK_SZ < BLOCK_SZ/3)) ||
			((World.cannon[t].dir == DIR_LEFT) &&
			(xd%BLOCK_SZ > 2*BLOCK_SZ/3))) {
			SET_BIT(pl->status, KILLED);
                        if (pl->robot_mode!=RM_OBJECT) {
			    sprintf(msg, "%s crashed with a cannon.",pl->name);
			    SCORE(i, -Rate(WALL_RATING, pl->score));
			    Set_message(msg);
                        }
			World.cannon[t].dead_time = CANNON_DEAD_TIME;
		        World.block[World.cannon[t].pos.x][World.cannon[t].pos.y] = SPACE;
			World.cannon[t].active = false;
			Explode_object((double)(x*BLOCK_SZ),
				       (double)(y*BLOCK_SZ),
				       World.cannon[t].dir, RES*0.4,
				       120);
		    }

		    break;
		case WORMHOLE:
		{
		    int hole = wormXY(x, y);

		    if (World.wormHoles[hole].type != WORM_OUT)
		    {
			SET_BIT(pl->status, WARPING);

			if (!BIT(pl->status, WARPED))
			{
			    pl->forceVisible += 15;
			    pl->wormHoleHit = hole;
			}
		    }
		    break;
		}
		default:
		    break;
		}
	    }
	    
	    /*
	     * don't re-warp us if we've just warped.  Let us get clear of
	     * the wormhole first
	     */
	    
	    if (BIT(pl->status, WARPED))
		if (BIT(pl->status, WARPING))
		{
		    CLR_BIT(pl->status, WARPING);
		}
		else
		{
		    CLR_BIT(pl->status, WARPED);
		}
	    
	    if (BIT(pl->status, KILLED) && pl->score < 0
		&& pl->robot_mode != RM_NOT_ROBOT 
                && pl->robot_mode != RM_OBJECT ) {
		pl->home_base = -1;
		Pick_startpos(i);
	    }
	}
    }

    /* Shot - wall, and out of bounds */
    for (i=0; i<NumObjs; i++) {
	x=(int)(Obj[i]->pos.x/BLOCK_SZ);
	y=(int)(Obj[i]->pos.y/BLOCK_SZ);

	if (x<0 || x>=World.x || y<0 || y>=World.y)
	    Obj[i]->life=0;
	else
	    switch (World.block[x][y]) {

	case FUEL:
	case FILLED:
	case FILLED_NO_DRAW:
            crash_wall(x,y,Obj[i]);
	    Obj[i]->life=0;
	    break;

	case REC_LU:
	    if ((int)Obj[i]->pos.x % BLOCK_SZ
		<= (int)Obj[i]->pos.y % BLOCK_SZ) {
                crash_wall(x,y,Obj[i]);
		Obj[i]->life=0;
            }
	    break;

	case REC_RU:
	    if ((int)Obj[i]->pos.x % BLOCK_SZ
		>= BLOCK_SZ - ((int)Obj[i]->pos.x % BLOCK_SZ))
                crash_wall(x,y,Obj[i]);
		Obj[i]->life=0;
	    break;

	case REC_LD:
	    if ((int)Obj[i]->pos.x % BLOCK_SZ
		<= BLOCK_SZ - ((int)Obj[i]->pos.y % BLOCK_SZ))
                crash_wall(x,y,Obj[i]);
		Obj[i]->life=0;
	    break;

	case REC_RD:
	    if ((int)Obj[i]->pos.x % BLOCK_SZ
		>= ((int)Obj[i]->pos.y % BLOCK_SZ))
                crash_wall(x,y,Obj[i]);
		Obj[i]->life=0;
	    break;

	case CANNON:
	    if (!BIT(Obj[i]->type, KILLING_SHOTS&(~OBJ_CANNON_SHOT)))
		break;

	    xd=Obj[i]->pos.x;
	    yd=Obj[i]->pos.y;

	    for(t=0; World.cannon[t].pos.x!=x ||
		World.cannon[t].pos.y!=y; t++);

	    if (World.cannon[t].dead_time > 0)
		break;

	    if (((World.cannon[t].dir == DIR_UP) &&
		 (yd%BLOCK_SZ <= BLOCK_SZ/3)) ||
		((World.cannon[t].dir == DIR_DOWN) &&
		 (yd%BLOCK_SZ >= 2*BLOCK_SZ/3)) ||
		((World.cannon[t].dir == DIR_RIGHT) &&
		 (xd%BLOCK_SZ <= BLOCK_SZ/3)) ||
		((World.cannon[t].dir == DIR_LEFT) &&
		 (xd%BLOCK_SZ >= 2*BLOCK_SZ/3))) {

		World.cannon[t].dead_time = CANNON_DEAD_TIME;
		World.block[World.cannon[t].pos.x][World.cannon[t].pos.y] = SPACE;
		World.cannon[t].active    = false;
		Explode_object((double)(x*BLOCK_SZ+BLOCK_SZ/2),
			       (double)(y*BLOCK_SZ+BLOCK_SZ/2),
			       World.cannon[t].dir, RES*0.4, 80);
                if (Obj[i]->id>=0) {
		    killer = GetInd[Obj[i]->id];
		    SCORE(killer, Rate(pl->score, CANNON_RATING)/4);
                }
	    }

	    break;

	default:
	    break;
	}
    }

    if (labels)
	Set_label_strings();
}


void crash_wall(int x,int y, object *o)
{
    int exp_fac=1;
    int chance=1;

    switch (World.block[x][y]) {
    case FUEL: 
        exp_fac+=2;
        chance=4;
        break;
    case CANNON:
        chance=3;
    case FILLED:
    case FILLED_NO_DRAW:
        exp_fac+=1;
    default:
        break;
    }
    if (o)
        switch (o->type) {
        case OBJ_MINE:
            exp_fac+=1;
            break;
        case OBJ_HEAT_SHOT:
        case OBJ_TORPEDO:
        case OBJ_SMART_SHOT:
            if (rand()&7>=chance)
                return;
            else
                break;
        case OBJ_SHOT:
        case OBJ_CANNON_SHOT:
            if ((rand()&1023)>=chance)
                return;
            else
                break;
        default:
            return;
        }
    add_blaster(x,y);
    Explode_object(
        (double)(x*BLOCK_SZ+BLOCK_SZ/2),
        (double)(y*BLOCK_SZ+BLOCK_SZ/2),
        0,RES,exp_fac*100);
}

bool Landing(int ind, int point)
{
    double y;
    player *pl = Players[ind];


    if (point == 0) {
	if (pl->robot_mode != RM_OBJECT) {	/* Head first? */
	    sprintf(msg, "%s had a head first landing.", Players[ind]->name);
	    Set_message(msg);
	}
	return False;
    }

    if (ABS(pl->vel.y) > 6.0) {			/* Too fast? */
	if (pl->robot_mode != RM_OBJECT) {
	    sprintf(msg, "%s had a heavy landing.", Players[ind]->name);
	    Set_message(msg);
	}
	return False;
    }

    if ((pl->dir >= (1.3*RES/4)) ||		/* Right angle? */
	(pl->dir <= (0.7*RES/4))) {
	if (pl->robot_mode != RM_OBJECT) {
	    sprintf(msg, "%s had a bad landing.", Players[ind]->name);
	    Set_message(msg);
	}
	return False;
    }

    if (((int)(y=pl->pos.y+ships[pl->dir].pts[point].y) % BLOCK_SZ)
	< (BLOCK_SZ*0.80)) {			/* Right position? */
	if (pl->robot_mode != RM_OBJECT) {
	    sprintf(msg, "%s crashed.", Players[ind]->name);
	    Set_message(msg);
	}
	return False;
    }

    pl->vel.x*=0.95;
    pl->dir-=((pl->dir - (RES/4))*0.2);
    pl->pos.y = (1+(int)(y/BLOCK_SZ))*BLOCK_SZ
	- ships[pl->dir].pts[point].y;

    pl->vel.y = 0.90*ABS(pl->vel.y);


    return True;
}


int
wormXY(x, y)
int x, y;
{
    int i;
    
    for (i = 0; i < World.NumWormholes; i++)
	if (World.wormHoles[i].pos.x == x &&
	    World.wormHoles[i].pos.y == y)
	    break;

    return i;
}

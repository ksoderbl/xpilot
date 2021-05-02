/* collision.c,v 1.3 1992/05/11 15:31:00 bjoerns Exp
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
#include "score.h"

#define in_range(o1, o2, r)	((ABS((o1)->pos.x-(o2)->pos.x)<(r) && ABS((o1)->pos.y-(o2)->pos.y)<(r)) ? true : false)

extern int Antall, Ant_Shots;
extern player *Players[];
extern object *Shots[];
extern wireobj ships[];
extern World_map World;
extern void Set_message(char *);
extern void Explode_object(double, double, int, int, int);
bool Landing(int, int);

extern long KILLING_SHOTS;
char msg[MSG_LEN];



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
    for (i=0; i<Antall; i++) {
	pl=Players[i];
	if (!BIT(pl->status, PLAYING) || BIT(pl->status, GAME_OVER))
	    continue;

	if (pl->pos.x<0 || pl->pos.y<0 ||
	    pl->pos.x>=(World.x*WORLD_SPACE) ||
	    pl->pos.y>=(World.y*WORLD_SPACE))
	    SET_BIT(pl->status, KILLED);

	/* Player - player */
	if (BIT(World.rules->mode, CRASH_WITH_PLAYER)) {
	    for (j=i+1; j<Antall; j++)
		if (in_range((object *)pl,
			     (object *)Players[j], 2*SHIP_WIDTH-6) &&
		    (BIT(Players[j]->status, PLAYING) &&
		     !BIT(Players[j]->status, GAME_OVER)))
		    if (!TEAM(i, j)) {
			sprintf(msg, "%s and %s crashed.",
				pl->name, Players[j]->name);
			Set_message(msg);
/*			if (!BIT(pl->status, KILLED))
			    SCORE(i, PTS_PR_PL_CRASH);
			if (!BIT(Players[j]->status, KILLED))
			    SCORE(j, PTS_PR_PL_CRASH);
*/			ENERGY(i, ED_PL_CRASH);
			ENERGY(j, ED_PL_CRASH);
			SET_BIT(pl->status, KILLED);
			SET_BIT(Players[j]->status, KILLED);
		    }
	}

	/* Player checkpoint */
	if (BIT(World.rules->mode, TIMING))
	    if (LENGTH(Players[i]->pos.x -
		       (World.check[Players[i]->check].x*WORLD_SPACE),
		       Players[i]->pos.y -
		       (World.check[Players[i]->check].y*WORLD_SPACE))
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

		if (Players[i]->check == World.Ant_check)
		    Players[i]->check = 0;
	    }

	/*
	 * Collision between a player and an object.
	 */
	for (j=0; j<Ant_Shots; j++) {
	    range=SHIP_WIDTH;
	    if (Shots[j]->type == OBJ_SMART_SHOT)
		range+=4;
	    if (Shots[j]->type == OBJ_MINE)
		range+=100;

	    if (BIT(pl->status, KILLED) ||
		(!in_range((object *)pl, Shots[j], range)))
		continue;

	    if ((Shots[j]->type==OBJ_SPARK && Shots[j]->id==pl->id) ||
		(Shots[j]->type==OBJ_MINE &&
		 (Shots[j]->id==pl->id || TEAM(get_ind[Shots[j]->id], i))))
		continue;

	    if ((Shots[j]->id != -1) &&
		(TEAM(i, get_ind[Shots[j]->id])) &&
		(pl->id != Shots[j]->id))
		continue;
	    else
		Shots[j]->life=0;
	    
	    Delta_mv(pl, Shots[j]);

	    /*
	     * Special object collision.
	     */
	    switch (Shots[j]->type) {
	    case OBJ_WIDEANGLE_SHOT:
		pl->extra_shots++;
		break;
	    case OBJ_REAR_SHOT:
		SET_BIT(pl->have, OBJ_REAR_SHOT);
		break;
	    case OBJ_SMART_SHOT_PACK:
		pl->missiles += 4;
		break;
	    case OBJ_CLOAKING_DEVICE:
		SET_BIT(pl->have, OBJ_CLOAKING_DEVICE);
		break;
	    case OBJ_ENERGY_PACK:
		pl->fuel += 500+(rand()%500);
		pl->fuel_count = 150;
		pl->fuel = MIN(pl->max_fuel, pl->fuel);
		break;
	    case OBJ_MINE_PACK:
		pl->mines++;
		break;
	    default:
		break;
	    }

	    if (!BIT(Shots[j]->type, KILLING_SHOTS))
		continue;

	    if (BIT(pl->used, OBJ_SHIELD)) {
		switch (Shots[j]->type) {
		case OBJ_SMART_SHOT:
		    pl->fuel+=ED_SMART_SHOT_HIT;
		    break;

		case OBJ_SHOT:
		case OBJ_CANNON_SHOT:
		    pl->fuel+=ED_SHOT_HIT;
		    break;

		default:
		    printf("You were hit by what?\n");
		    break;
		}
	    } else
		if (BIT(Shots[j]->type, (OBJ_SHOT|OBJ_SMART_SHOT))) {
		    sprintf(msg, "%s was shot down by %s.", pl->name,
			    Players[killer=get_ind[Shots[j]->id]]->name);
			SET_BIT(pl->status, KILLED);
			if (killer == i) {
			    strcat(msg, " How strange!...");
			/*  SCORE(i, PTS_PR_PL_SHOT);	*/
			} else {
			    sc = Rate(Players[killer]->score, pl->score);
			    SCORE(killer, sc);
			    SCORE(i, -sc);
			}
			Set_message(msg);
		    }
		else if (BIT(Shots[j]->type, OBJ_CANNON_SHOT)) {
		    sprintf(msg, "%s was hit by cannonfire.", pl->name);
		    Set_message(msg);
		    SCORE(i, -Rate(CANNON_RATING, pl->score)/4);
		    SET_BIT(pl->status, KILLED);
		}
	}



	/* Player - wall */
	if (!(BIT(pl->used, OBJ_TRAINER)||BIT(pl->status, KILLED)))
	    for(j=0; (j<3) && (!BIT(pl->status, KILLED)); j++) {
		switch (World.type
    [x=(int)((pl->pos.x+ships[pl->dir].pts[j].x)/WORLD_SPACE)]
    [y=(int)((pl->pos.y+ships[pl->dir].pts[j].y)/WORLD_SPACE)]) {
		case FUEL:
		case FILLED:
		case FILLED_NO_DRAW:
		    if (!Landing(i, j)) {
			SET_BIT(pl->status, KILLED);
			SCORE(i, -Rate(WALL_RATING, pl->score));
		    }
		    break;
		case REC_LU:
		    if ((((int)(pl->pos.x
				+ships[pl->dir].pts[j].x))
			 % WORLD_SPACE)
			<= (((int)(pl->pos.y
				   +ships[pl->dir].pts[j].y))
			    % WORLD_SPACE)) {
			if (!Landing(i, j)) {
			    SET_BIT(pl->status, KILLED);
			    SCORE(i, -Rate(WALL_RATING, pl->score));
			}
		    }
		    break;
		case REC_RU:
		    if ((((int)(pl->pos.x
			    +ships[pl->dir].pts[j].x)) % WORLD_SPACE)
			    >= WORLD_SPACE - (((int)(pl->pos.y
			+ships[pl->dir].pts[j].y)) % WORLD_SPACE)) {
			if (!Landing(i, j)) {
			    SET_BIT(pl->status, KILLED);
			    SCORE(i, -Rate(WALL_RATING, pl->score));
			}
		    }
		    break;
		case REC_LD:
		    if ((((int)(pl->pos.x
			    +ships[pl->dir].pts[j].x)) % WORLD_SPACE)
			<= WORLD_SPACE - (((int)(pl->pos.y
			+ships[pl->dir].pts[j].y)) % WORLD_SPACE)) {
			SET_BIT(pl->status, KILLED);
			sprintf(msg,"%s crashed into the wall.",
				pl->name);
			Set_message(msg);
			SCORE(i, -Rate(WALL_RATING, pl->score));
		    }
		    break;
		case REC_RD:
		    if ((((int)(pl->pos.x
			    +ships[pl->dir].pts[j].x)) % WORLD_SPACE)
			>= (((int)(pl->pos.y
			+ships[pl->dir].pts[j].y)) % WORLD_SPACE)) {
			SET_BIT(pl->status, KILLED);
			sprintf(msg, "%s crashed into the wall.",
				pl->name);
			Set_message(msg);
			SCORE(i, -Rate(WALL_RATING, pl->score));
		    }
		    break;
		case CANNON:
		    xd=pl->pos.x+ships[pl->dir].pts[j].x;
		    yd=pl->pos.y+ships[pl->dir].pts[j].y;

		    for(t=0; World.cannon[t].pos.x!=x ||
			World.cannon[t].pos.y!=y; t++);

		    if (World.cannon[t].dead_time > 0)
			break;

		    if (((World.cannon[t].dir == UP) &&
			(yd%WORLD_SPACE < WORLD_SPACE/3)) ||
			((World.cannon[t].dir == DOWN) &&
			(yd%WORLD_SPACE > 2*WORLD_SPACE/3)) ||
			((World.cannon[t].dir == RIGHT) &&
			(xd%WORLD_SPACE < WORLD_SPACE/3)) ||
			((World.cannon[t].dir == LEFT) &&
			(xd%WORLD_SPACE > 2*WORLD_SPACE/3))) {
			SET_BIT(pl->status, KILLED);
			sprintf(msg, "%s crashed with a cannon.",
				pl->name);
			SCORE(i, -Rate(WALL_RATING, pl->score));
			Set_message(msg);
			World.cannon[t].dead_time=CANNON_DEAD_TIME;
			World.cannon[t].active=false;
			Explode_object(x*WORLD_SPACE, y*WORLD_SPACE,
				       World.cannon[t].dir, RESOLUTION*0.4,
				       120);
		    }

		    break;
		default:
		    break;
		}
	    }
    }



    /* Shot - wall, and out of bounds */
    for (i=0; i<Ant_Shots; i++) {
	x=(int)(Shots[i]->pos.x/WORLD_SPACE);
	y=(int)(Shots[i]->pos.y/WORLD_SPACE);

	if (x<0 || x>=World.x || y<0 || y>=World.y)
	    Shots[i]->life=0;
	else
	    switch (World.type[x][y]) {

	case FUEL:
	case FILLED:
	case FILLED_NO_DRAW:
	    Shots[i]->life=0;
	    break;

	case REC_LU:
	    if ((int)Shots[i]->pos.x % WORLD_SPACE
		<= (int)Shots[i]->pos.y % WORLD_SPACE)
		Shots[i]->life=0;
	    break;

	case REC_RU:
	    if ((int)Shots[i]->pos.x % WORLD_SPACE
		>= WORLD_SPACE - ((int)Shots[i]->pos.x % WORLD_SPACE))
		Shots[i]->life=0;
	    break;

	case REC_LD:
	    if ((int)Shots[i]->pos.x % WORLD_SPACE
		<= WORLD_SPACE - ((int)Shots[i]->pos.y % WORLD_SPACE))
		Shots[i]->life=0;
	    break;

	case REC_RD:
	    if ((int)Shots[i]->pos.x % WORLD_SPACE
		>= ((int)Shots[i]->pos.y % WORLD_SPACE))
		Shots[i]->life=0;
	    break;

	case CANNON:
	    if (!BIT(Shots[i]->type, KILLING_SHOTS&(~OBJ_CANNON_SHOT)))
		break;

	    xd=Shots[i]->pos.x;
	    yd=Shots[i]->pos.y;

	    for(t=0; World.cannon[t].pos.x!=x ||
		World.cannon[t].pos.y!=y; t++);

	    if (World.cannon[t].dead_time > 0)
		break;

	    if (((World.cannon[t].dir == UP) &&
		 (yd%WORLD_SPACE <= WORLD_SPACE/3)) ||
		((World.cannon[t].dir == DOWN) &&
		 (yd%WORLD_SPACE >= 2*WORLD_SPACE/3)) ||
		((World.cannon[t].dir == RIGHT) &&
		 (xd%WORLD_SPACE <= WORLD_SPACE/3)) ||
		((World.cannon[t].dir == LEFT) &&
		 (xd%WORLD_SPACE >= 2*WORLD_SPACE/3))) {
		World.cannon[t].dead_time=CANNON_DEAD_TIME;
		World.cannon[t].active=false;
		Explode_object(x*WORLD_SPACE+WORLD_SPACE/2,
			       y*WORLD_SPACE+WORLD_SPACE/2,
			       World.cannon[t].dir, RESOLUTION*0.4, 80);
		killer=get_ind[Shots[i]->id];
		SCORE(killer, Rate(CANNON_RATING, pl->score)/4);
	    }

	    break;

	default:
	    break;
	}
    }

    if (labels)
	Set_label_strings();
}



bool Landing(int ind, int point)
{
    double y;
    player *pl = Players[ind];


    if (point == 0) {				/* Head first? Sorry.. :) */
	sprintf(msg, "%s had a head first landing.", Players[ind]->name);
	Set_message(msg);
	return False;
    }

    if (ABS(pl->vel.y) > 6.0) {			/* Too fast? */
	sprintf(msg, "%s had a heavy landing.", Players[ind]->name);
	Set_message(msg);
	return False;
    }

    if ((pl->dir >= (1.3*RESOLUTION/4)) ||	/* Right angle? */
	(pl->dir <= (0.7*RESOLUTION/4))) {
	sprintf(msg, "%s had a bad landing.", Players[ind]->name);
	Set_message(msg);
	return False;
    }

    if (((int)(y=pl->pos.y+ships[pl->dir].pts[point].y) % WORLD_SPACE)
	< (WORLD_SPACE*0.80)) {			/* Right position? */
	sprintf(msg, "%s crashed.", Players[ind]->name);
	Set_message(msg);
	return False;
    }

    pl->vel.x*=0.95;
    pl->dir-=((pl->dir - (RESOLUTION/4))*0.2);
    pl->pos.y = (1+(int)(y/WORLD_SPACE))*WORLD_SPACE
	- ships[pl->dir].pts[point].y;

    pl->vel.y = 0.90*ABS(pl->vel.y);


    return True;
}

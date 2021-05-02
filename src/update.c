/* update.c,v 1.3 1992/05/11 15:31:42 bjoerns Exp
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
#include "draw.h"

#define speed_limit(obj) {					\
    if (ABS((obj).velocity>SPEED_LIMIT)) {			\
	double theta = atan2((obj).vel.y, (obj).vel.x);		\
	(obj).vel.x = cos(theta)*SPEED_LIMIT;			\
	(obj).vel.y=sin(theta)*SPEED_LIMIT;			\
    }								\
}

#define update_object_pos(obj)	{					    \
    int x=(int)((obj).pos.x/WORLD_SPACE), y=(int)((obj).pos.y/WORLD_SPACE); \
    if (x<0 || x>=World.x || y<0 || y>=World.y) {			    \
	LIMIT((obj).pos.x, 0.0, WORLD_SPACE*World.x-1.0);		    \
	LIMIT((obj).pos.y, 0.0, WORLD_SPACE*World.y-1.0);		    \
    } else {								    \
	if (!BIT((obj).status, GRAVITY)) {				    \
	    (obj).pos.x += (obj).vel.x += (obj).acc.x;			    \
	    (obj).pos.y += (obj).vel.y += (obj).acc.y;			    \
	    (obj).velocity = LENGTH((obj).vel.x, (obj).vel.y);		    \
	} else {							    \
	    (obj).pos.x += (obj).vel.x += (obj).acc.x + World.gravity[x][y].x; \
	    (obj).pos.y += (obj).vel.y += (obj).acc.y + World.gravity[x][y].y; \
	}								    \
	/*  speed_limit(obj); */					    \
	(obj).velocity = LENGTH((obj).vel.x, (obj).vel.y);		    \
    }									    \
}



extern int Antall;
extern player *Players[];
extern object *Shots[];
extern int Ant_Shots;
extern double Gravity;
extern World_map World;
extern double tbl_sin[];

extern void Kill_player(int);
extern void Check_collision(void);
extern void Cannon_fire(int);
extern void Turn_thrust(int);
extern void Thrust(int);

static char msg[MSG_LEN];



/********** **********
 * Updating objects and the like.
*/

void Update_objects(void)
{
    int i, j;
    player *pl;


    /*
     * Special items.
     */
    j=rand();
    for (i=0; i<NO_OF_ITEMS; i++)
	if ((j%World.items[i].chance) == 0)
	    if (World.items[i].ant < World.items[i].max) {
		World.items[i].ant++;
		Place_item(i);
	    }

    /*
     * Let the fuel stations regenerate some fuel.
     */
    for(i=0; i<World.Ant_fuel; i++) {
	World.fuel[i].left += STATION_REGENERATION*Antall;
	if (World.fuel[i].left > MAX_STATION_FUEL)
	    World.fuel[i].left = MAX_STATION_FUEL;
    }

    /*
     * Update shots.
     */
    for (i=0; i<Ant_Shots; i++) {
	update_object_pos(*Shots[i]);

	if (Shots[i]->type == OBJ_SMART_SHOT)
	    Move_smart_shot(i);
    }

    /*
     * Updating cannons, maybe a little bit of fireworks too?
     */
    for (i=0; i<World.Ant_cannon; i++) {
	if (World.cannon[i].dead_time > 0) {
	    World.cannon[i].dead_time--;
	    continue;
	}
	if (World.cannon[i].active) {
	    if ((rand()%20)==0)
		Cannon_fire(i);
	}
	World.cannon[i].active = false;
    }


    /* * * * * *
     *
     * Player loop. Computes miscellaneous updates.
     *
     */
    for (i=0; i<Antall; i++) {
	pl=Players[i];

	/* Limits. */
	LIMIT(pl->power, MIN_PLAYER_POWER, MAX_PLAYER_POWER);
	LIMIT(pl->turnspeed, MIN_PLAYER_TURNSPEED, MAX_PLAYER_TURNSPEED);
	LIMIT(pl->turnresistance, 0.0, 1.0);

	if (pl->control_count>0)
	    pl->control_count--;
	if (pl->fuel_count>0)
	    pl->fuel_count--;

	if (pl->count>0) {
	    pl->count--;
	    if (!BIT(pl->status, PLAYING)) {
		pl->vel.x = (WORLD_SPACE*World.Start_points[pl->home_base].x
			     + WORLD_SPACE/2 - pl->pos.x) / (pl->count+1);
		pl->vel.y = (WORLD_SPACE*World.Start_points[pl->home_base].y
			     + WORLD_SPACE/2 - pl->pos.y) / (pl->count+1);
		goto update;
	    }
	}

	if (pl->count == 0) {
	    pl->count=-1;
	    if (!BIT(pl->status, PLAYING)) {
		SET_BIT(pl->status, PLAYING);
		Go_home(i);
	    }
	    if (BIT(pl->status, SELF_DESTRUCT)) {
		sprintf(msg, "%s has comitted suicide.", pl->name);
		Set_message(msg);
		Kill_player(i);
		pl->check = 0;
		pl->round = 0;
		pl->time  = 0;
	    }
	}

	if ((!BIT(pl->status, PLAYING)) || BIT(pl->status, GAME_OVER))
	    continue;

#ifdef THRUST_TURN
	if (pl->turnacc != 0.0)
	    Turn_thrust(i);
#endif

	/*
	 * Compute turn
	 */
	pl->turnvel+=pl->turnacc;
	pl->turnvel*=pl->turnresistance;
	pl->double_dir += pl->turnvel;
	if (pl->double_dir < 0)
	    pl->double_dir+=RESOLUTION;
	if (pl->double_dir > RESOLUTION)
	    pl->double_dir-=RESOLUTION;

	pl->dir=pl->double_dir;
	pl->dir %= RESOLUTION;


	/*
	 * Compute energy drainage
	 */
	if (BIT(pl->used, OBJ_SHIELD))
	    pl->fuel+=ED_SHIELD;

	if (BIT(pl->used, OBJ_CLOAKING_DEVICE))
	    pl->fuel+=ED_CLOAKING_DEVICE;

	if (BIT(pl->used, OBJ_CLOAKING_DEVICE))
	    if ((rand()%CLOAK_FAILURE) == 0) {
		CLR_BIT(pl->status, INVISIBLE);
	    } else {
		SET_BIT(pl->status, INVISIBLE);
	    }
	else
	    CLR_BIT(pl->status, INVISIBLE);

	if (BIT(pl->used, OBJ_REFUEL)) {
	    if ((LENGTH((pl->pos.x-World.fuel[pl->fs].pos.x),
		       (pl->pos.y-World.fuel[pl->fs].pos.y)) > 90.0)
		|| (pl->fuel > pl->max_fuel)) {
		CLR_BIT(pl->used, OBJ_REFUEL);
	    } else
		if (World.fuel[pl->fs].left > 5.0) {
		    World.fuel[pl->fs].left -= 5.0;
		    pl->fuel+=5.0;
		} else {
		    pl->fuel+=World.fuel[pl->fs].left;
		    World.fuel[pl->fs].left=0.0;
		    CLR_BIT(pl->used, OBJ_REFUEL);
		}
	}
	if (pl->fuel <= 0.0) {
	    pl->fuel=0.0;
	    CLR_BIT(pl->used, OBJ_SHIELD|OBJ_CLOAKING_DEVICE);
	    CLR_BIT(pl->status, THRUSTING);
	}
	if (pl->fuel > MAX_PLAYER_FUEL) {
	    pl->fuel=MAX_PLAYER_FUEL;
	    CLR_BIT(pl->used, OBJ_REFUEL);
	}

	/*
	 * Update acceleration vector etc.
	 */
	if (BIT(pl->status, THRUSTING)) {
	    pl->acc.x = pl->power *
		tcos(pl->dir) / pl->mass;
	    pl->acc.y = pl->power *
		tsin(pl->dir) / pl->mass;
	    pl->fuel -= (pl->power*0.02);   /* Decrement fuel */
	} else {
	    pl->acc.x=pl->acc.y=0.0;
	}

	pl->mass = pl->emptymass + (pl->fuel*0.005);

    update:
	if (!BIT(pl->status, PAUSE))
	    update_object_pos(*pl);	    /* New position */

	if (BIT(pl->status, THRUSTING))
	    Thrust(i);

	/*
	 * Updating time player has used. Only used when timing
	 */
	if (pl->round != 0)
	    pl->time++;

	if (pl->round == 4) {
	    if ((pl->time < pl->best_run) || (pl->best_run == 0)) 
		pl->best_run = pl->time;
	    Players[i]->last_lap_time = Players[i]->time 
		- Players[i]->last_lap;
	    Kill_player(i);
	    pl->round = 0;
	    pl->check = 0;
	    pl->time = 0;
	}

	switch (pl->lock.tagged) {
	case LOCK_PLAYER:
	    pl->lock.distance = LENGTH(pl->pos.x -
				     Players[get_ind[pl->lock.pl_id]]->pos.x,
				     pl->pos.y -
				     Players[get_ind[pl->lock.pl_id]]->pos.y);
	    pl->sensor_range = MAX(pl->fuel*ENERGY_RANGE_FACTOR,
				   VISIBILITY_DISTANCE);
	    break;

	default:
	    break;
	}

	pl->used &= pl->have;

	pl->world.x=pl->pos.x-CENTER;	    /* Scroll */
	pl->world.y=pl->pos.y-CENTER;
    }


    /*
     * Checking for collision, updating score etc. (see collision.c)
     */
    Check_collision();


    /*
     * Kill players that ought to be killed.
     */
    for (i=Antall-1; i>=0; i--)
	if (BIT(Players[i]->status, KILLED))
	    Kill_player(i);

    /*
     * Kill shots that ought to be dead.
     */
    for (i=Ant_Shots-1; i>=0; i--)
	if (--(Shots[i]->life) <= 0)
	    Delete_shot(i);

    /*
     * Compute general game status, do we have a winner?
     */
    Compute_game_status();
}

/* $Id: update.c,v 1.14 1992/08/27 00:26:14 bjoerns Exp $
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
#include "draw.h"
#include "robot.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: update.c,v 1.14 1992/08/27 00:26:14 bjoerns Exp $";
#endif



#define speed_limit(obj) {					\
    if (ABS((obj).velocity>SPEED_LIMIT)) {			\
	double theta = atan2((obj).vel.y, (obj).vel.x);		\
	(obj).vel.x = cos(theta)*SPEED_LIMIT;			\
	(obj).vel.y=sin(theta)*SPEED_LIMIT;			\
    }								\
}

#define update_object_pos(obj)	{					    \
    int x=(int)((obj).pos.x/BLOCK_SZ), y=(int)((obj).pos.y/BLOCK_SZ); \
    if (x<0 || x>=World.x || y<0 || y>=World.y) {			    \
	LIMIT((obj).pos.x, 0.0, BLOCK_SZ*World.x-1.0);		    \
	LIMIT((obj).pos.y, 0.0, BLOCK_SZ*World.y-1.0);		    \
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



static char msg[MSG_LEN];

extern unsigned long loops;



/********** **********
 * Updating objects and the like.
 */
void Update_objects(void)
{
    int i, j;
    player *pl;



    /*
     * Update robots.
     */
    Update_robots();

    /*
     * Special items.
     */
    j = rand();
    for (i=0; i<NUM_ITEMS; i++)
	if (!(j%World.items[i].chance))
	    if (World.items[i].num < World.items[i].max) {
		World.items[i].num++;
		Place_item(i);
	    }

    /*
     * Let the fuel stations regenerate some fuel.
     */
    for(i=0; i<World.NumFuels; i++) {
	if ((World.fuel[i].fuel += STATION_REGENERATION) > MAX_STATION_FUEL)
	    World.fuel[i].fuel = MAX_STATION_FUEL;
    }

    /*
     * Update shots.
     */
    for (i=0; i<NumObjs; i++) {
	update_object_pos(*Obj[i]);

        if (BIT(Obj[i]->type,(OBJ_SMART_SHOT|OBJ_HEAT_SHOT|OBJ_TORPEDO)))
	    Move_smart_shot(i);
    }

    /*
     * Updating cannons, maybe a little bit of fireworks too?
     */
    for (i=0; i<World.NumCannons; i++) {
	if (World.cannon[i].dead_time > 0) {
	    if (!--World.cannon[i].dead_time)
		World.block[World.cannon[i].pos.x][World.cannon[i].pos.y] = CANNON;
	    continue;
	}
	if (World.cannon[i].active) {
	    if ((rand()%20)==0)
		Cannon_fire(i);
	}
	World.cannon[i].active = false;
    }
    
    /*
     * Updating Blasters, maybe a little bit of dust too?
     */
    Blaster_fire();


    /* * * * * *
     *
     * Player loop. Computes miscellaneous updates.
     *
     */
    for (i=0; i<NumPlayers; i++) {
#ifdef TURN_FUEL
        long tf = 0;
#endif

	pl = Players[i];

	/* Limits. */
	LIMIT(pl->power, MIN_PLAYER_POWER, MAX_PLAYER_POWER);
	LIMIT(pl->turnspeed, MIN_PLAYER_TURNSPEED, MAX_PLAYER_TURNSPEED);
	LIMIT(pl->turnresistance, 0.0, 1.0);

	if (pl->control_count>0)
	    pl->control_count--;
	if (pl->fuel.count>0)
	    pl->fuel.count--;

	if (pl->count>0) {
	    pl->count--;
	    if (!BIT(pl->status, PLAYING)) {
		pl->vel.x = (BLOCK_SZ*World.base[pl->home_base].x
			     + BLOCK_SZ/2 - pl->pos.x) / (pl->count+1);
		pl->vel.y = (BLOCK_SZ*World.base[pl->home_base].y
			     + BLOCK_SZ/2 - pl->pos.y) / (pl->count+1);
		goto update;
	    }
	}

	if (pl->count == 0) {
	    pl->count = -1;

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

        
	/*
	 * Compute turn
	 */

	pl->turnvel	+= pl->turnacc;
	pl->turnvel	*= pl->turnresistance;
        
#ifdef TURN_FUEL
        tf = pl->oldturnvel-pl->turnvel;
        tf = TURN_FUEL(tf);
        if (pl->fuel.sum <= tf) {
            tf = 0;
            pl->turnacc = 0.0;
            pl->turnvel = pl->oldturnvel;
        } else {
            Add_fuel(&(pl->fuel),-tf);
            pl->oldturnvel = pl->turnvel;
        }
#endif


	pl->double_dir	+= pl->turnvel;
        
	if (pl->double_dir < 0)
	    pl->double_dir += RES;
	if (pl->double_dir >= RES)
	    pl->double_dir -= RES;

/*	if (pl->double_dir < 0)
	    pl->double_dir -= RES * (((int)pl->double_dir)/RES);
	if (pl->double_dir >= RES)
	    pl->double_dir -= RES * (((int)pl->double_dir)/RES);
*/
	pl->dir = pl->double_dir;
	pl->dir %= RES;


	/*
	 * Compute energy drainage
	 */
	if (BIT(pl->used, OBJ_SHIELD))
            Add_fuel(&(pl->fuel),ED_SHIELD);

	if (BIT(pl->used, OBJ_CLOAKING_DEVICE))
            Add_fuel(&(pl->fuel),ED_CLOAKING_DEVICE);

#define UPDATE_RATE 100

	for (j = 0; j < NumPlayers; j++)
	{
	    if (pl->forceVisible)
		Players[j]->visibility[i].canSee = 1;

	    if (i == j || !BIT(Players[j]->used, OBJ_CLOAKING_DEVICE))
		pl->visibility[j].canSee = 1;
	    else
		if (pl->updateVisibility || Players[j]->updateVisibility ||
		    rand() % UPDATE_RATE <
		    ABS(loops - pl->visibility[j].lastChange)) {

		    pl->visibility[j].lastChange = loops;
		    pl->visibility[j].canSee = rand() % (pl->sensors + 1) >
			(rand() % (Players[j]->cloaks + 1));
		}
	}

	if (BIT(pl->used, OBJ_REFUEL)) {
	    if ((LENGTH((pl->pos.x-World.fuel[pl->fs].pos.x),
		       (pl->pos.y-World.fuel[pl->fs].pos.y)) > 90.0)
		|| (pl->fuel.sum >= pl->fuel.max)) {
		CLR_BIT(pl->used, OBJ_REFUEL);
	    } else {
                int i = pl->fuel.no_tanks;
                int ct = pl->fuel.current;
                
                do {
		    if (World.fuel[pl->fs].fuel > REFUEL_RATE) {
		        World.fuel[pl->fs].fuel -= REFUEL_RATE;
		        Add_fuel(&(pl->fuel),REFUEL_RATE);
		    } else {
		        Add_fuel(&(pl->fuel),World.fuel[pl->fs].fuel);
		        World.fuel[pl->fs].fuel = 0;
		        CLR_BIT(pl->used, OBJ_REFUEL);
                        break;
		    }
                    if (pl->fuel.current == pl->fuel.no_tanks)
                        pl->fuel.current = 0;
                    else
                        pl->fuel.current += 1;
                } while (i--);
                pl->fuel.current = ct;
            }
	}
	if (pl->fuel.sum <= 0) {
	    CLR_BIT(pl->used, OBJ_SHIELD|OBJ_CLOAKING_DEVICE);
	    CLR_BIT(pl->status, THRUSTING);
	}
	if (pl->fuel.sum > (pl->fuel.max-REFUEL_RATE))
	    CLR_BIT(pl->used, OBJ_REFUEL);

	/*
	 * Update acceleration vector etc.
	 */
	if (BIT(pl->status, THRUSTING)) {
            double power = pl->power;
            long f = pl->power*0.02;
            int a = pl->after_burners;

            if (a) {
                power = AFTER_BURN_POWER(power,a);
                f=AFTER_BURN_FUEL(f,a);
            }
	    pl->acc.x = power * tcos(pl->dir) / pl->mass;
	    pl->acc.y = power * tsin(pl->dir) / pl->mass;
            Add_fuel(&(pl->fuel),-f); /* Decrement fuel */
	} else {
	    pl->acc.x = pl->acc.y = 0.0;
	}

	pl->mass = pl->emptymass + FUEL_MASS(pl->fuel.sum);

	if (BIT(pl->status, WARPING))
	{
	    int i, wx, wy, proximity, dir, nearestFront, nearestRear,
	        proxFront, proxRear;

	    if (World.wormHoles[pl->wormHoleHit].countdown)
		i = World.wormHoles[pl->wormHoleHit].lastdest;
	    else if (rand()&3)
	    {
		do
		    i = rand() % World.NumWormholes;
		while (World.wormHoles[i].type == WORM_IN);
	    }
	    else
	    {
		nearestFront = nearestRear = -1;
		proxFront = proxRear = 10000000;
		
		for (i = 0; i < World.NumWormholes; i++)
		{
		    if (i == pl->wormHoleHit ||
			World.wormHoles[i].type == WORM_IN)
			continue;
		    
		    wx = World.wormHoles[i].pos.x * BLOCK_SZ;
		    wy = World.wormHoles[i].pos.y * BLOCK_SZ;
		    
		    proximity = ABS(pl->vel.y * (wx - pl->pos.x) +
				    pl->vel.x * (wy - pl->pos.y));
		    dir = pl->vel.x * (wx - pl->pos.x) +
			pl->vel.y * (wy - pl->pos.y);
		    
		    if (dir < 0)
		    {
			if (proximity < proxRear)
			{
			    nearestRear = i;
			    proxRear = proximity;
			}
		    }
		    else
			if (proximity < proxFront)
			{
			    nearestFront = i;
			    proxFront = proximity;
			}
		}
		
		i = nearestFront < 0 ? nearestRear : nearestFront;
	    }
	    
	    pl->pos.x = World.wormHoles[i].pos.x * BLOCK_SZ + BLOCK_SZ / 2;
	    pl->pos.y = World.wormHoles[i].pos.y * BLOCK_SZ + BLOCK_SZ / 2;
	    pl->vel.x /= WORM_BRAKE_FACTOR; pl->vel.y /= WORM_BRAKE_FACTOR;

	    if (i != pl->wormHoleHit)
	    {
		World.wormHoles[pl->wormHoleHit].lastdest = i;
		World.wormHoles[pl->wormHoleHit].countdown = WORMCOUNT;
	    }

	    CLR_BIT(pl->status, WARPING);
	    SET_BIT(pl->status, WARPED);
	}

	pl->ecmInfo.size >>= 1;

	if (BIT(pl->used, OBJ_ECM))
	{
	    pl->ecmInfo.pos.x = pl->pos.x;
	    pl->ecmInfo.pos.y = pl->pos.y;
	    pl->ecmInfo.size = ECM_DISTANCE * 2;
	    CLR_BIT(pl->used, OBJ_ECM);
	}

    update:
	if (!BIT(pl->status, PAUSE))
	    update_object_pos(*pl);	    /* New position */

	if (BIT(pl->status, THRUSTING))
	    Thrust(i);
#ifdef TURN_FUEL
	if (tf)
            Turn_thrust(i,TURN_SPARKS(tf));
#endif

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
				     Players[GetInd[pl->lock.pl_id]]->pos.x,
				     pl->pos.y -
				     Players[GetInd[pl->lock.pl_id]]->pos.y);
	    pl->sensor_range = MAX(pl->fuel.sum * ENERGY_RANGE_FACTOR,
				   VISIBILITY_DISTANCE);
	    break;

	default:
	    break;
	}

	pl->used &= pl->have;

	pl->world.x = pl->pos.x-CENTER;	    /* Scroll */
	pl->world.y = pl->pos.y-CENTER;
    }

    for (i=0; i<World.NumWormholes; i++)
       if(World.wormHoles[i].countdown)
          --World.wormHoles[i].countdown;
	    
    for (i = 0; i < NumPlayers; i++)
    {
	Players[i]->updateVisibility = 0;

	if (Players[i]->forceVisible)
	{
	    Players[i]->forceVisible--;

	    if (!Players[i]->forceVisible)
		Players[i]->updateVisibility = 1;
	}
    }

    /*
     * Checking for collision, updating score etc. (see collision.c)
     */
    Check_collision();


    /*
     * Update tanks, Kill players that ought to be killed.
     */
    for (i=NumPlayers-1; i>=0; i--) {
        player *pl = Players[i];
        
	if (BIT(pl->status, PLAYING)) Update_tanks(&(pl->fuel));
	if (BIT(pl->status, KILLED)) {
            if (pl->robot_mode!=RM_OBJECT)
	        Kill_player(i);
            else {
                NumPseudoPlayers--;
                Explode(i);
                Delete_player(i);
                Set_label_strings();
            }
        }
    }

    /*
     * Kill shots that ought to be dead.
     */
    for (i=NumObjs-1; i>=0; i--)
	if (--(Obj[i]->life) <= 0)
	    Delete_shot(i);

    /*
     * Compute general game status, do we have a winner?
     */
    Compute_game_status();
}

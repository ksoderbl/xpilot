/* $Id: play.c,v 1.15 1992/08/27 00:26:03 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <stdio.h>
#include <math.h>

#include "global.h"
#include "draw.h"
#include "score.h"
#include "robot.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: play.c,v 1.15 1992/08/27 00:26:03 bjoerns Exp $";
#endif



/********** **********
 * Functions for ship movement.
 */

void Thrust(int ind)
{
    player *pl = Players[ind];
    object *spark;
    int dir, no_sparks, the_color, alt_thrust;
    double the_mass;
    const int spread = (RES*0.3);
    const int spreadoffset = (spread/2);
    double x, y;


    pl = Players[ind];

    no_sparks = (pl->power*0.3) + (rand()%3) + 2;
    alt_thrust = pl->after_burners
                  ? AFTER_BURN_SPARKS(no_sparks-1,pl->after_burners)+1
                  : -1;
    the_color = RED;
    the_mass = THRUST_MASS;
    
    x = pl->pos.x + (ships[pl->dir].pts[1].x + ships[pl->dir].pts[2].x) / 2;
    y = pl->pos.y + (ships[pl->dir].pts[1].y + ships[pl->dir].pts[2].y) / 2;

    for (; no_sparks && NumObjs<MAX_TOTAL_SHOTS; NumObjs++) {
	spark	= Obj[NumObjs];
	dir	= pl->dir + (RES/2) + (rand()%(1+spread)) - spreadoffset - 1;
	spark->color	= the_color;
	spark->id	= pl->id;
	spark->pos.x	= x;
	spark->pos.y	= y;
	spark->vel.x	= pl->vel.x + tcos(dir) *
					(1+rand()%(int)(1+pl->power*0.2));
	spark->vel.y	= pl->vel.y + tsin(dir) *
					(1+rand()%(int)(1+pl->power*0.2));
	spark->status	= GRAVITY;
	spark->acc.x	= spark->acc.y = 0.0;
/*
	spark->dir	= MOD(spark->dir, RES);
*/
	spark->dir	= MOD(dir, RES);
	spark->mass	= the_mass;
	spark->type	= OBJ_SPARK;
	spark->life	= 3 + (rand()%(int)(1+pl->power*0.3));
        if (--no_sparks==alt_thrust) {
            the_color = BLUE;
            the_mass = THRUST_MASS*ALT_SPARK_MASS_FACT;
        }
    }
}

#ifdef TURN_FUEL
void Turn_thrust(int ind,int no_sparks)
{
    player *pl = Players[ind];
    object *spark;
    int i, dir;
    const int spread = (RES*0.2);
    const int spreadoffset = (spread/2);
    int x, y;
    int rate = ABS(Players[ind]->turnacc);


    x = pl->pos.x + ships[pl->dir].pts[0].x;
    y = pl->pos.y + ships[pl->dir].pts[0].y;

    for (i=0; i<no_sparks && NumObjs<MAX_TOTAL_SHOTS; i++, NumObjs++) {
	spark = Obj[NumObjs];
	dir = pl->dir + (RES/4) + (rand()%(1+spread)) - spreadoffset - 1;

	if (pl->turnacc > 0.0)
	    dir = dir + RES/2;

	spark->color	= RED;
	spark->id	= pl->id;
	spark->pos.x	= x;
	spark->pos.y	= y;
	spark->vel.x	= pl->vel.x + (tcos(dir) * (rand()&3));
	spark->vel.y	= pl->vel.y + (tsin(dir) * (rand()&3));
	spark->status	= GRAVITY;
	spark->acc.x	= spark->acc.y = 0;
	spark->dir	= MOD(spark->dir, RES);
	spark->mass	= THRUST_MASS;
	spark->type	= OBJ_SPARK;
	spark->life	= 1 + (rand()%(2*FRAMES_PR_SEC));
    }
}
#endif


/* Calculates the recoil if a ship fires a shot */
void Recoil(object *ship, object *shot)
{
    ship->vel.x -= ((tcos(shot->dir) * ABS(shot->vel.x-ship->vel.x) *
	shot->mass) / ship->mass);
    ship->vel.y -= ((tsin(shot->dir) * ABS(shot->vel.y-ship->vel.y) *
	shot->mass) / ship->mass);
}

void Delta_mv(object *ship, object *obj)
{
    double dvx, dvy, ship_theta, obj_theta, dm;


    dvx = ABS(obj->vel.x - ship->vel.x);
    dvy = ABS(obj->vel.y - ship->vel.y);
    ship_theta = atan2(ship->vel.y, ship->vel.x);
    obj_theta = atan2(obj->vel.y, obj->vel.x);

    dm = obj->mass/ship->mass;
    ship->vel.x += cos(obj_theta)*dvx * dm;
    ship->vel.y += sin(obj_theta)*dvy * dm;

    dm = ship->mass/obj->mass;
    obj->vel.x += cos(ship_theta)*dvx * dm;
    obj->vel.y += sin(ship_theta)*dvy * dm;
}



/********** **********
 * Functions for shots.
*/

void Alloc_shots(int number)
{
    object *x=malloc(number*sizeof(object));
    int i;

    for (i=0; i<number; i++)
	Obj[i]=x++;
}


void Free_shots(void)
{
    free(Obj[0]);
}



void Place_item(int type)
{
    object *item;
    int x, y;


    if (NumObjs >= MAX_TOTAL_SHOTS)
	return;

    item = Obj[NumObjs++];

    do {
	x = rand()%World.x;
	y = rand()%World.y;
    } while (World.block[x][y] != SPACE);

    item->color = RED;
    item->info = type;
    item->status = 0;
    item->id = -1;
    item->pos.x = x*BLOCK_SZ+BLOCK_SZ/2;
    item->pos.y = y*BLOCK_SZ+BLOCK_SZ/2;
    item->vel.x=item->vel.y = item->acc.x=item->acc.y = 0.00000001;
		/* Near zero, but not zero or atan2 will complain. */
    item->mass = 10.0;
    item->life = 1500+(rand()&511);

    switch (type) {
    case ITEM_SMART_SHOT_PACK:
	item->type = OBJ_SMART_SHOT_PACK;
	break;
    case ITEM_SENSOR_PACK:
	item->type = OBJ_SENSOR_PACK;
	break;
    case ITEM_ECM:
	item->type = OBJ_ECM;
	break;
    case ITEM_MINE_PACK:
	item->type = OBJ_MINE_PACK;
	break;
    case ITEM_TANK:
	item->type = OBJ_TANK;
	break;
    case ITEM_CLOAKING_DEVICE:
	item->type = OBJ_CLOAKING_DEVICE;
	break;
    case ITEM_ENERGY_PACK:
	item->type = OBJ_ENERGY_PACK;
	break;
    case ITEM_WIDEANGLE_SHOT:
	item->type = OBJ_WIDEANGLE_SHOT;
	break;
    case ITEM_REAR_SHOT:
	item->type = OBJ_REAR_SHOT;
	break;
    case ITEM_AFTER_BURNER:
	item->type = OBJ_AFTER_BURNER;
	break;
    default:
	item->type = OBJ_SMART_SHOT_PACK;
	break;
    }
}



void Place_mine(int ind)
{
    object *mine;
    player *pl = Players[ind];


    if (NumObjs >= MAX_TOTAL_SHOTS)
	return;

    mine = Obj[NumObjs++];
    mine->type = OBJ_MINE;
    mine->color = BLUE;
    mine->info = OBJ_MINE;
    mine->status = 0;
    mine->id = pl->id;
    mine->pos.x = pl->pos.x;
    mine->pos.y = pl->pos.y;
    mine->vel.x=mine->vel.y = mine->acc.x=mine->acc.y = 0.00000001;
		/* Near zero, but not zero or atan2 will complain. */
    mine->mass = MINE_MASS;
    mine->life = MINE_LIFETIME;
}


void Place_moving_mine(int ind, double vx, double vy)
{
    object *mine;
    player *pl = Players[ind];


    if (NumObjs >= MAX_TOTAL_SHOTS)
	return;

    mine = Obj[NumObjs++];
    mine->type = OBJ_MINE;
    mine->color = BLUE;
    mine->info = OBJ_MINE;
    mine->status = GRAVITY;
    mine->id = pl->id;
    mine->pos.x = pl->pos.x;
    mine->pos.y = pl->pos.y;
    mine->acc.x=mine->acc.y = 0.00000001;
		/* Near zero, but not zero or atan2 will complain. */
    mine->vel.x = vx*MINE_SPEED_FACT;
    mine->vel.y = vy*MINE_SPEED_FACT;
    mine->mass = MINE_MASS;
    mine->life = MINE_LIFETIME;
}


void Blaster_fire(void)
{   int i;

    for (i=0; i<World.NumBlasters;) {
        object *dust;
        blaster_t *b = &(World.blaster[i]);
    
        if (b->orig != BLASTER && ++(b->runtime) > (WALL_RETURN_TIME*FRAMES_PR_SEC)) {
            remove_crash_blaster(i);
            continue;
        }
    
        if (!(((rand()&b->chance) != 0) || (NumObjs >= MAX_TOTAL_SHOTS))) {
            dust = Obj[NumObjs++];
            dust->type = OBJ_DUST;
            dust->color =
              ( b->orig==BLASTER ||
	        b->runtime<(WALL_RETURN_TIME-WARN_TIME)*FRAMES_PR_SEC ) 
                ? WHITE 
                : RED;
            dust->status = GRAVITY;
            dust->id = -1;
            dust->pos.x = b->pos.x*BLOCK_SZ+BLOCK_SZ/2;
            dust->pos.y = b->pos.y*BLOCK_SZ+BLOCK_SZ/2;
            dust->acc.x = dust->acc.y = 0.00000001;
        		/* Near zero, but not zero or atan2 will complain. */
            dust->dir = (rand()%b->add_size)+b->base_size;
                        /* used for size of dust! */
            dust->vel.x =   (rand()%(b->base_size+b->add_size-dust->dir+1))
                          * b->speed_fact
                          * (rand()&1?1:-1);
            dust->vel.y =   (rand()%(b->base_size+b->add_size-dust->dir+1))
                          * b->speed_fact
                          * (rand()&1
                             ? ((Gravity<0) ? 1.3 : -1)
                             : ((Gravity>0) ? 1 : -1.3));
            dust->mass = DUST_MASS_FACT*dust->dir;
            dust->life = b->life*dust->dir;
        }
        i++;
    }
}


void Cannon_fire(int ind)
{
    object *shot;
    int dir, speed;
    const int spread = (RES*0.3);
    const int spreadoffset = (spread/2);


    if (NumObjs >= MAX_TOTAL_SHOTS)
	return;

    shot = Obj[NumObjs++];
    dir = (rand()%(1+spread)) - spreadoffset - 1;	/* Tmp direction */
    speed = 9+(rand()%4);
    shot->color = WHITE;
    shot->id = -1;
    shot->pos.x = World.cannon[ind].pos.x*BLOCK_SZ+BLOCK_SZ/2;
    shot->pos.y = World.cannon[ind].pos.y*BLOCK_SZ+BLOCK_SZ/2;
    shot->status = GRAVITY;
    shot->acc.x = shot->acc.y = 0;
    shot->mass = 0.4;
    shot->type = OBJ_CANNON_SHOT;
    shot->life = 25 + (rand()%20);

    switch (World.cannon[ind].dir) {
    case DIR_UP:
	shot->pos.y += BLOCK_SZ/6;
	dir += DIR_UP;
	break;
    case DIR_DOWN:
	shot->pos.y -= BLOCK_SZ/6;
	dir += DIR_DOWN;
	break;
    case DIR_RIGHT:
	shot->pos.x += BLOCK_SZ/6;
	dir += DIR_RIGHT;
	break;
    case DIR_LEFT:
	shot->pos.x -= BLOCK_SZ/6;
	dir += DIR_LEFT;
	break;
    }

    shot->dir	= MOD(shot->dir, RES);
    shot->vel.x	= speed*tcos(dir);
    shot->vel.y	= speed*tsin(dir);
}

void Fire_shot(int ind, int type, int dir)	    /* Initializes a new shot */
{
    object *shot;
    player *pl;


    pl = Players[ind];
    if ((pl->shots >= pl->shot_max) || (NumObjs >= MAX_TOTAL_SHOTS)
	|| BIT(pl->used, OBJ_SHIELD))
	return;

    shot = Obj[NumObjs];
    switch (type) {

    case OBJ_SHOT:
	if (pl->fuel.sum < -ED_SHOT)
	    return;

	shot->life = pl->shot_life;
	shot->mass = pl->shot_mass;
	shot->max_speed = SPEED_LIMIT;
	Add_fuel(&(pl->fuel),ED_SHOT);
        shot->vel.x = pl->vel.x + ((tcos(dir) * pl->shot_speed));
        shot->vel.y = pl->vel.y + ((tsin(dir) * pl->shot_speed));
	break;

    case OBJ_TORPEDO:
        
        if (pl->fuel.sum < -ED_SMART_SHOT || (pl->missiles <= 0))
            return;
        
        shot->mass = MISSILE_MASS;
        shot->life = MISSILE_LIFETIME;
        Add_fuel(&(pl->fuel),ED_SMART_SHOT);
        shot->vel.x = pl->vel.x + (tcos(dir) * pl->shot_speed);
        shot->vel.y = pl->vel.y + (tsin(dir) * pl->shot_speed);
        shot->info = 0;
        pl->missiles--;
        break;

    case OBJ_SMART_SHOT:
    case OBJ_HEAT_SHOT: {
        int lock;

        if (pl->fuel.sum < -ED_SMART_SHOT || (pl->missiles <= 0))
            return;
        
        if (type==OBJ_HEAT_SHOT) {
            if (    pl->lock.tagged == LOCK_NONE
                 || (   (pl->lock.distance > pl->sensor_range)
                     && BIT(World.rules->mode, LIMITED_VISIBILITY)
                    )
               )
                lock = -1;
            else
                lock = pl->lock.pl_id;
        } else {
             if (    pl->lock.tagged == LOCK_NONE
                  || (   (pl->lock.distance > pl->sensor_range)
                      && BIT(World.rules->mode, LIMITED_VISIBILITY)
                     )
                  || !pl->visibility[GetInd[pl->lock.pl_id]].canSee
                 )
	        return;
             lock = pl->lock.pl_id;
        }
	shot->mass = MISSILE_MASS;
	shot->life = MISSILE_LIFETIME;
	shot->max_speed = SMART_SHOT_MAX_SPEED;
        shot->count=0;
	shot->turnspeed = SMART_TURNSPEED;
	shot->info = lock;
	Add_fuel(&(pl->fuel),ED_SMART_SHOT);
	pl->missiles--;
        shot->vel.x = pl->vel.x + tcos(dir) * pl->shot_speed;
        shot->vel.y = pl->vel.y + tsin(dir) * pl->shot_speed;
        if (type==OBJ_HEAT_SHOT) {
             shot->max_speed = SMART_SHOT_MAX_SPEED * HEAT_SPEED_FACT;
             shot->turnspeed = SMART_TURNSPEED * HEAT_SPEED_FACT;
             shot->vel.x = pl->vel.x + tcos(dir) * pl->shot_speed * HEAT_SPEED_FACT;
             shot->vel.y = pl->vel.y + tsin(dir) * pl->shot_speed * HEAT_SPEED_FACT;
        }
	break;
    }
    }
    shot->type	= type;
    shot->id	= pl->id;
    shot->color	= pl->color;
    shot->pos.x = pl->pos.x + ships[dir].pts[0].x;
    shot->pos.y = pl->pos.y + ships[dir].pts[0].y;
    shot->status= GRAVITY;
    shot->acc.x = shot->acc.y = 0;
    shot->dir	= dir;
    Recoil((object *)pl, shot);

    NumObjs++; pl->shots++;

}



void Delete_shot(int ind)	    /* Removes shot from array */
{
    object *shot = Obj[ind];	    /* Used when swapping places */
    int addMine = 0, x, y;
    int addHeat = 0, xh, yh;


    switch (shot->type) {
    case OBJ_DUST:
        if (shot->dir>=DUST_EXPLODE_LIMIT)
            Explode_object(shot->pos.x, shot->pos.y, 0, RES, shot->dir-DUST_EXPLODE_LIMIT);
	break;
    case OBJ_MINE:
	Explode_object(shot->pos.x, shot->pos.y, 0, RES, 500);
	break;
    case OBJ_SPARK:
    case OBJ_CANNON_SHOT:
    case OBJ_CANNON_DEBRIS:
    case OBJ_DEBRIS:
	break;

	/* Shots related to a player. */
    case OBJ_HEAT_SHOT:
    case OBJ_TORPEDO:
    case OBJ_SMART_SHOT:
	Explode_object(shot->pos.x, shot->pos.y, 0, RES, 30);
    case OBJ_SHOT:
	Players[GetInd[shot->id]]->shots--;
	break;

	/* Special items. */
    case OBJ_SMART_SHOT_PACK:
        if(!shot->life && shot->color!=WHITE)
	 { shot->color=WHITE;
	   shot->life=FRAMES_PR_SEC*WARN_TIME;
	   return;
	 }
	World.items[ITEM_SMART_SHOT_PACK].num--;
        if (shot->life==0) {
            addHeat = 1;
            xh = shot->pos.x;
            yh = shot->pos.y;
        }
	break;
    case OBJ_AFTER_BURNER:
	World.items[ITEM_AFTER_BURNER].num--;
	break;
    case OBJ_SENSOR_PACK:
	World.items[ITEM_SENSOR_PACK].num--;
	break;
    case OBJ_ECM:
	World.items[ITEM_ECM].num--;
	break;
    case OBJ_CLOAKING_DEVICE:
	World.items[ITEM_CLOAKING_DEVICE].num--;
	break;
    case OBJ_ENERGY_PACK:
	World.items[ITEM_ENERGY_PACK].num--;
	break;
    case OBJ_WIDEANGLE_SHOT:
	World.items[ITEM_WIDEANGLE_SHOT].num--;
	break;
    case OBJ_REAR_SHOT:
	World.items[ITEM_REAR_SHOT].num--;
	break;
    case OBJ_MINE_PACK:
        if(!shot->life && shot->color!=WHITE)
	 { shot->color=WHITE;
	   shot->life=FRAMES_PR_SEC*WARN_TIME;
	   return;
	 }
	World.items[ITEM_MINE_PACK].num--;
	if (shot->life == 0)
	{
	    addMine = 1;
	    x = shot->pos.x;
	    y = shot->pos.y;
	}
	break;
    case OBJ_TANK:
	World.items[ITEM_TANK].num--;
	break;
    default:
	printf("Delete_shot(): Unkown shot type %d.\n", shot->type);
	break;
    }


    Obj[ind] = Obj[--NumObjs];
    Obj[NumObjs] = shot;

    if (addMine)
    {
	object *mine;

	if (NumObjs >= MAX_TOTAL_SHOTS)
	    return;
	
	mine = Obj[NumObjs++];
	mine->type = OBJ_MINE;
	mine->color = BLUE;
	mine->info = OBJ_MINE;
	mine->status = rand()&3?0:GRAVITY;
	mine->id = -1;
	mine->pos.x = x;
	mine->pos.y = y;
	mine->vel.x=mine->vel.y = mine->acc.x=mine->acc.y = 0.00000001;
	/* Near zero, but not zero or atan2 will complain. */
	mine->mass = MINE_MASS;
	mine->life = MINE_LIFETIME;
    }
    if (addHeat)
    {
	object *heat;

	if (NumObjs >= MAX_TOTAL_SHOTS)
	    return;
	
	heat = Obj[NumObjs++];
	heat->mass = MISSILE_MASS;
	heat->life = MISSILE_LIFETIME;
        heat->max_speed = SMART_SHOT_MAX_SPEED * HEAT_SPEED_FACT;
        heat->count = HEAT_WIDE_TIMEOUT+HEAT_WIDE_ERROR;
        heat->turnspeed = SMART_TURNSPEED * HEAT_SPEED_FACT;
	heat->info = -1;
	heat->vel.x = heat->vel.y = 0.00000001;
	heat->type = OBJ_HEAT_SHOT;
	heat->id = -1;
	heat->color = WHITE;
	heat->pos.x = xh;
	heat->pos.y = yh;
        heat->dir = 0;
        heat->acc.x=heat->acc.y = 0;
    }
}


#define CONFUSED_UPDATE_GRANULARITY	10
#define CONFUSED_TIME			3
#define ECM_DAMAGE_COUNT		30

void do_ecm(pl)
player *pl;
{
    object *shot;
    int i;
    player *p;

    for (i = 0; i < NumObjs; i++)
    {
	shot = Obj[i];

	if (shot->type == OBJ_SMART_SHOT &&
	    LENGTH((pl->pos.x - shot->pos.x), (pl->pos.y - shot->pos.y)) <=
	    ECM_DISTANCE*ECM_MIS_FACT)
	{
	    SET_BIT(shot->status, CONFUSED);
	    shot->count = CONFUSED_TIME;
	    if ((pl->lock.distance <= pl->sensor_range ||
		!BIT(World.rules->mode, LIMITED_VISIBILITY)) &&
		pl->visibility[GetInd[pl->lock.pl_id]].canSee)
		shot->new_info = pl->lock.pl_id;
	    else
		shot->new_info = Players[rand() % NumPlayers]->id;
	}
    }

    for (i = 0; i < NumPlayers; i++)
    {
	p = Players[i];

	if (p != pl && BIT(p->status, PLAYING) &&
	    LENGTH((pl->pos.x - p->pos.x), (pl->pos.y - p->pos.y)) <=
	    ECM_DISTANCE)
	{
	    int c = rand() % ECM_DAMAGE_COUNT;
	    p->forceVisible += c;

	    if (p->robot_mode == RM_NOT_ROBOT || p->robot_mode == RM_OBJECT)
		p->damaged += c;
	    else
		if (pl->lock.tagged == LOCK_PLAYER &&
		    (pl->lock.distance < pl->sensor_range ||
		    !BIT(World.rules->mode, LIMITED_VISIBILITY)) &&
		    pl->visibility[GetInd[pl->lock.pl_id]].canSee &&
		    pl->lock.pl_id != p->id)
		{
		    static char msg[MSG_LEN];

		    sprintf(msg, "%s has programmed %s to seek %s.",
			    pl->name, p->name,
			    Players[GetInd[pl->lock.pl_id]]->name);
		    Set_message(msg);
		    p->robot_lock_id = pl->lock.pl_id;
		    p->robot_lock = LOCK_PLAYER;
		}
	}
    }
}

void Move_smart_shot(int ind)
{
    object *shot = Obj[ind];
    player *pl;
    double theta;
    double min;
    double acc;
    int vinkel;
    extern unsigned long loops;
    double x_dif=0.0;
    double y_dif=0.0;

    if (shot->type==OBJ_TORPEDO) {
        if (shot->info++<TORPEDO_SPEED_TIME) {
            shot->vel.x += TORPEDO_ACC*tcos(shot->dir);
            shot->vel.y += TORPEDO_ACC*tsin(shot->dir);
        }
        return;
    }
    
    acc = SMART_SHOT_ACC;

    if (shot->type==OBJ_HEAT_SHOT) {
        acc = SMART_SHOT_ACC*HEAT_SPEED_FACT;
        if (shot->info>=0) {
            /* get player and set min to distance */
            pl = Players[ GetInd[shot->info] ];
            min = LENGTH(pl->pos.x - shot->pos.x,pl->pos.y - shot->pos.y);
        } else {
           /* no player. number of moves so, that new target is searched */
            pl = 0;
            shot->count = HEAT_WIDE_TIMEOUT+HEAT_WIDE_ERROR;
        }
        if (pl && BIT(pl->status, THRUSTING)) {
            /* target is thrusting, set number to moves to correct error value */
            if (min<HEAT_CLOSE_RANGE) {
                shot->count=HEAT_CLOSE_ERROR;
            } else if (min<HEAT_MID_RANGE) {
                shot->count=HEAT_MID_ERROR;
            } else {
                shot->count=HEAT_WIDE_ERROR;
            }
        } else {
            shot->count++;
            /* look for new target */
            if (   (   min<HEAT_CLOSE_RANGE
                    && shot->count>HEAT_CLOSE_TIMEOUT+HEAT_CLOSE_ERROR)
                || (   min<HEAT_MID_RANGE
                    && shot->count>HEAT_MID_TIMEOUT+HEAT_MID_ERROR)
                || shot->count>HEAT_WIDE_TIMEOUT+HEAT_WIDE_ERROR)
            {   
                double l;
                int i;
      
                min=HEAT_RANGE*(shot->count/HEAT_CLOSE_TIMEOUT);
                for (i=0; i<NumPlayers; i++) {
                    player *p=Players[i];
            
                    if (!BIT(p->status, THRUSTING))
                        continue;
                    l=LENGTH(p->pos.x - shot->pos.x, p->pos.y - shot->pos.y);
                    /* after burners can be detected easier; so scale the */
                    /* length:                                            */
                    l *= (MAX_AFTER_BURNER+1-p->after_burners);
                    l /= (MAX_AFTER_BURNER+1);
                    if (p->have & OBJ_AFTER_BURNER) l *= (16-p->after_burners);
                    if (l<min) {
                        shot->info = Players[i]->id;
                        min = l;
                        shot->count =  l<HEAT_CLOSE_RANGE ? HEAT_CLOSE_ERROR :
                                       l<HEAT_MID_RANGE   ? HEAT_MID_ERROR :
                                       HEAT_WIDE_ERROR;
                        pl = p;
                    }
                }
            }
        }
        if (shot->info<0) return;
        /* heat seekers cannot fly exactly, if target is far away or thrust isn't
         * activ. So simulate the error:
         */
        x_dif=(rand()&3)*shot->count;
        y_dif=(rand()&3)*shot->count;
    } else {
         if (BIT(shot->status, CONFUSED) &&
	     (!(loops % CONFUSED_UPDATE_GRANULARITY) ||
	     shot->count == CONFUSED_TIME))
        {
	    if (shot->count)
	    {
	        shot->info = Players[rand() % NumPlayers]->id;
	        shot->count--;
	    }
	    else
	    {
	        CLR_BIT(shot->status, CONFUSED);
	        if (rand() % 100 < 80)
		    shot->info = shot->new_info;
	    }
        }
	pl = Players[GetInd[shot->info]];
    }
    
    /* use a little look ahead to fly more exact */
    min=LENGTH(pl->pos.x-shot->pos.x,
               pl->pos.y-shot->pos.y)/LENGTH(shot->vel.x,shot->vel.y);
    x_dif+=pl->vel.x*min;
    y_dif+=pl->vel.y*min;
    
    theta = atan2(pl->pos.y+y_dif-(shot->pos.y),pl->pos.x+x_dif-(shot->pos.x));

   {double x,y,vx,vy; int i,xi,yi,j,freemax,k,foundw;
    static struct {int dx,dy;} sur[8]=
     {{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1}};
#define BLOCK_PARTS 2
    x=LENGTH(vx=shot->vel.x,vy=shot->vel.y)/(BLOCK_SZ*BLOCK_PARTS);
    vx/=x;vy/=x; x=shot->pos.x;y=shot->pos.y;
    foundw=0;
    for(i=SMART_SHOT_LOOK_AH;xi=(x+=vx)/BLOCK_SZ,yi=(y+=vy)/BLOCK_SZ,
     xi>=0&&xi<World.x&&yi>=0&&yi<World.y&&i--;)
       switch(World.block[xi][yi])
        { case FUEL:case FILLED:case FILLED_NO_DRAW:case REC_LU:case REC_RU:
	  case REC_LD:case REC_RD:case CANNON:
	     if(LENGTH(pl->pos.x-shot->pos.x,pl->pos.y
	      -shot->pos.y)>(SMART_SHOT_LOOK_AH-i)*(BLOCK_SZ/BLOCK_PARTS))
	      { if(shot->velocity>SMART_SHOT_MIN_SPEED)
	           shot->velocity-=acc*(SMART_SHOT_DECFACT+1);
	      }
	     foundw=1;goto found_wall;
	}
found_wall:
    i=((int)(shot->dir*8/RES)&7)+8;
    xi=shot->pos.x/BLOCK_SZ;yi=shot->pos.y/BLOCK_SZ;
    for(j=2,vinkel= -1,freemax=0;j>=-2;--j)
     { int si,xt,yt;
       for(si=1,k=0;si>=-1;--si)
        { xt=xi+sur[(i+j+si)&7].dx;yt=yi+sur[(i+j+si)&7].dy;
          if(xt>=0&&xt<World.x&&yt>=0&&yt<World.y)
	     switch(World.block[xt][yt])
	      { default: ++k;break;
	        case FUEL:case FILLED:case FILLED_NO_DRAW:case REC_LU:
	        case REC_RU:case REC_LD:case REC_RD:case CANNON:
		   if(!si)
		      k= -32;
	      }
	}
       if(k>freemax||k==freemax&&((j==-1&&rand()&1)||j==0||j==1))
          freemax=k>2?2:k,vinkel=i+j;
       if(k==3&&!j)
        { vinkel= -1; break;
	}
     }
    if(vinkel>=0)
     { i=vinkel&7;
       theta = atan2((yi+sur[i].dy)*BLOCK_SZ-(shot->pos.y+2*shot->vel.y),
		  (xi+sur[i].dx)*BLOCK_SZ-(shot->pos.x-2*shot->vel.x));
#ifdef SHOT_EXTRA_SLOWDOWN
       if(!foundw&&LENGTH(pl->pos.x-shot->pos.x,pl->pos.y-shot->pos.y)>
        (SHOT_LOOK_AH-i)*BLOCK_SZ)
        { if(shot->velocity>(SMART_SHOT_MIN_SPEED+SMART_SHOT_MAX_SPEED)/2)
             shot->velocity-=SMART_SHOT_DECC+SMART_SHOT_ACC;
        }
#endif
     }
   } 
    vinkel=(RES/(2.0*PI))*theta;

    if (vinkel < 0)
	vinkel += RES;
    vinkel %= RES;

    if (vinkel < shot->dir)
	vinkel += RES;
    vinkel = vinkel - shot->dir - RES/2;

    if (vinkel < 0)
	shot->dir += -vinkel<shot->turnspeed?-vinkel:shot->turnspeed;
    else
	shot->dir -= vinkel<shot->turnspeed?vinkel:shot->turnspeed;

    shot->dir = MOD(shot->dir, RES);	/* MERK!!!! */

    if (shot->velocity < shot->max_speed)
	shot->velocity += acc;

/*  shot->velocity = MIN(shot->velocity, shot->max_speed);  */

    shot->vel.x = tcos(shot->dir) * shot->velocity;
    shot->vel.y = tsin(shot->dir) * shot->velocity;
}


/******************
 * Add fuel to my tanks. Maybe use more than one of my tanks to store 
 * the fuel.
 */
void Add_fuel(pl_fuel_t *ft, long fuel)
{   int x,c,t;

    if (ft->sum + fuel > ft->max)
        fuel = ft->max - ft->sum;
    else if (ft->sum + fuel < 0)
        fuel = -ft->sum;
    ft->sum += fuel;
    ft->tank[ft->current] += fuel;
}


/*********************
 * Move fuel from add on tanks to main tank
 * handle over and underflow off tanks
 */
void Update_tanks(pl_fuel_t *ft)
{   
    if (ft->no_tanks) {
        int t,check;
        long low_level;
        long fuel;
        long *f;

        /* set low_level to minimum fuel in each tank */
        low_level = ft->sum / (ft->no_tanks + 1) - 1;
        if (low_level < 0) low_level = 0;
        if (TANK_REFILL_LIMIT < low_level) low_level = TANK_REFILL_LIMIT;
    
        t = ft->no_tanks;
        check = MAX_TANKS<<2;
        fuel = 0;
        f = ft->tank + t;

        while (t>=0 && check--) {
            long m = TANK_CAP(t);

            /* add the previous over/underflow and do a new cut */
            *f += fuel;
            if (*f > m) {
                fuel = *f - m;
                *f = m;
            } else if (*f < 0) {
                fuel = *f;
                *f = 0;
            } else
                fuel = 0;
            
            /* if there is no over/underflow, let the fuel run to main-tank */
            if (!fuel) {
                if (   t
                    && t != ft->current
                    && *f >= low_level + REFUEL_RATE
                    && *(f-1) <= TANK_CAP(t-1) - REFUEL_RATE
                ) {
                    *f -= REFUEL_RATE;
                    fuel = REFUEL_RATE;
                } else if ( 
                       t
                    && *f < low_level
                ) {
                    *f += REFUEL_RATE;
                    fuel = -REFUEL_RATE;
                }
            }
            if (fuel && t==0) {
               t = ft->no_tanks;
               f = ft->tank + t;
            } else {
                t--;
                f--;
            }
        }
        if (!check) {
            printf("fuel problem!\n");
            fuel = ft->sum;
            ft->sum =
            ft->max = 0;
            t = 0;
            while (t <= ft->no_tanks) {
                if (fuel) {
                    if (fuel>TANK_CAP(t)) {
                        ft->tank[t] = TANK_CAP(t);
                        fuel -= TANK_CAP(t);
                    } else {
                        ft->tank[t] = fuel;
                        fuel = 0;
                    }
                    ft->sum += ft->tank[t];
                } else
                   ft->tank[t] = 0;
                ft->max += TANK_CAP(t);
                t++;
            }
        }
    } else
        ft->tank[0] = ft->sum;
}



/*********************
 * use current tank as dummy target for heat seeking missles
 */
void Tank_handle_detach(player *pl)
{   player *dummy;
    int i;
    int ct;
    
    /* return, if no more players or no tanks */
    if (!pl->fuel.no_tanks || NumPseudoPlayers==MAX_PSEUDO_PLAYERS) return;

    /* if current tank is main, use another one */
    if (!(ct=pl->fuel.current)) ct = pl->fuel.no_tanks;

    Update_tanks(&(pl->fuel));
    /* fork the current player */
    dummy                = Players[NumPlayers];
    *dummy               = *pl;
    dummy->robot_mode    = RM_OBJECT;
    dummy->mychar        = 'T';
    dummy->disp_type     = DT_NONE;
    
    /* fuel is the one from choosen tank */
    dummy->fuel.sum      =
    dummy->fuel.tank[0]  = dummy->fuel.tank[ct];
    dummy->fuel.max      = TANK_CAP(ct);
    dummy->fuel.current  = 0;
    dummy->fuel.no_tanks = 0;
    
    /* no after burner */
    dummy->after_burners = 0;

    /* mass is only tank + fuel */
    dummy->mass = (dummy->emptymass = ShipMass) + FUEL_MASS(dummy->fuel.sum);
    dummy->have = DEF_HAVE;
    dummy->used = DEF_USED;
    dummy->power *= TANK_THRUST_FACT;

    /* handling the id's and the Tables */
    dummy->id = Id;
    GetInd[Id] = NumPlayers;
    NumPlayers++;
    NumPseudoPlayers++;
    Id++;
    Set_label_strings();

    /* the tank uses shield and thrust */
    SET_BIT(dummy->status,THRUSTING|OBJ_SHIELD);
    
    /* maybe heat-seekers to retarget? */
    for (i=0;i<NumObjs;i++)
        if (Obj[i]->type!=OBJ_HEAT_SHOT)
            continue;
        else if (Obj[i]->info<0)
            continue;
        else if (Players[ GetInd[Obj[i]->info] ]==pl)
            Obj[i]->info=NumPlayers;
    
    /* remove tank, fuel and mass from myself */
    pl->fuel.sum -= pl->fuel.tank[ct];
    pl->fuel.max -= TANK_CAP(ct);
    for (i=ct;i<pl->fuel.no_tanks;i++)
        pl->fuel.tank[i] = pl->fuel.tank[i+1];
    pl->emptymass -= TANK_MASS;
    pl->fuel.no_tanks -= 1;
    if (pl->fuel.current) pl->fuel.current -= 1;
    pl->fuel.count = FUEL_NOTIFY;
}

/********** **********
 * Functions for explotions.
*/

void Explode_object(double x, double y, int real_dir, int spread, int intensity)
{
    object *debris;
    int no_debris, speed, dir;
    register int i;
    const int spreadoffset = (spread/2);


    no_debris=(intensity/2)+(rand()%(1+intensity/2));
    for (i=0; i<no_debris && NumObjs<MAX_TOTAL_SHOTS; i++, NumObjs++) {
	debris = Obj[NumObjs];
	speed = DEBRIS_SPEED(intensity);
	dir = real_dir + (rand()%(1+spread)) - spreadoffset - 1;
	debris->color = RED;
	debris->id = 0;
	debris->pos.x = x;
	debris->pos.y = y;
	debris->vel.x = (tcos(dir) * speed);
	debris->vel.y = (tsin(dir) * speed);
	debris->status = GRAVITY;
	debris->acc.x = debris->acc.y = 0;
	debris->dir = dir;
	debris->mass = DEBRIS_MASS;
	debris->type = OBJ_CANNON_DEBRIS;
	debris->life = 8+((rand()%(1+intensity/4))*2);
    }
}

void Explode(int ind)	/* Index of unfortunate player */
{
    player *pl;
    object *debris;
    int i, dir, no_debris, speed;


    pl = Players[ind];
    no_debris = 1+(pl->fuel.sum/(8.0*FUEL_SCALE_FACT))
                 +(rand()%((int)(1+pl->mass*4.0)));
/*  shot_mass = pl->mass / no_debris;	Not used! */
    for (i=0; i<no_debris && NumObjs<MAX_TOTAL_SHOTS; i++, NumObjs++) {
	debris = Obj[NumObjs];
	dir = rand()%RES;
	speed = PL_DEBRIS_SPEED(pl->mass);
	debris->color=RED;
	debris->id = pl->id;
	debris->pos.x = pl->pos.x;
	debris->pos.y = pl->pos.y;
	debris->vel.x = pl->vel.x + (tcos(dir) * speed);
	debris->vel.y = pl->vel.y + (tsin(dir) * speed);
	debris->status = GRAVITY;
	debris->acc.x = debris->acc.y = 0;
	debris->dir = dir;
	debris->mass = PL_DEBRIS_MASS;
	debris->type = OBJ_DEBRIS;
	debris->life = PL_DEBRIS_LIFE(pl->mass);
    }
}

/* play.c,v 1.10 1992/06/28 05:38:23 bjoerns Exp
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

#ifndef	lint
static char sourceid[] =
    "@(#)play.c,v 1.10 1992/06/28 05:38:23 bjoerns Exp";
#endif



/********** **********
 * Functions for ship movement.
 */

void Thrust(int ind)
{
    player *pl = Players[ind];
    object *spark;
    int i, dir, no_sparks;
    const int spread = (RES*0.3);
    const int spreadoffset = (spread/2);
    double x, y;


    pl = Players[ind];

    no_sparks = (pl->power*0.3)+(rand()%3);

    x = pl->pos.x + (ships[pl->dir].pts[1].x + ships[pl->dir].pts[2].x) / 2;
    y = pl->pos.y + (ships[pl->dir].pts[1].y + ships[pl->dir].pts[2].y) / 2;

    for (i=0; i<no_sparks && NumObjs<MAX_TOTAL_SHOTS; i++, NumObjs++) {
	spark	= Obj[NumObjs];
	dir	= pl->dir + (RES/2) + (rand()%spread)-spreadoffset;
	spark->color	= RED;
	spark->id	= pl->id;
	spark->pos.x	= x;
	spark->pos.y	= y;
	spark->vel.x	= pl->vel.x + ((tcos(dir) *
					(1+rand()%(int)(pl->power*0.2))));
	spark->vel.y	= pl->vel.y + ((tsin(dir) *
					(1+rand()%(int)(pl->power*0.2))));
	spark->status	= GRAVITY;
	spark->acc.x	= spark->acc.y = 0.0;
	spark->dir	= MOD(spark->dir, RES);
	spark->mass	= 1.0;
	spark->type	= OBJ_SPARK;
	spark->life	= 3 + (rand()%(int)(pl->power*0.3));
    }
}


void Turn_thrust(int ind)
{
    player *pl = Players[ind];
    object *spark;
    int i, dir, no_sparks;
    const int spread = (RES*0.08);
    const int spreadoffset = (spread/2);
    int x, y;
    double rate = ABS(Players[ind]->turnacc*0.1);


    no_sparks=(rand()%(int)(rate*2));

    x = pl->pos.x+ships[pl->dir].pts[0].x;
    y = pl->pos.y+ships[pl->dir].pts[0].y;

    for (i=0; i<no_sparks && NumObjs<MAX_TOTAL_SHOTS; i++, NumObjs++) {
	spark = Obj[NumObjs];
	dir = pl->dir + (RES/4) + (rand()%spread)-spreadoffset;

	if (pl->turnacc > 0.0)
	    dir=dir+RES/2;

	spark->color=RED;
	spark->id = pl->id;
	spark->pos.x = x;
	spark->pos.y = y;
	spark->vel.x = pl->vel.x + (tcos(dir) * rate);
	spark->vel.y = pl->vel.y + (tsin(dir) * rate);
	spark->status = GRAVITY;
	spark->acc.x = spark->acc.y = 0;
	spark->dir = MOD(spark->dir, RES);
	spark->mass = 1.0;
	spark->type = OBJ_SPARK;
	spark->life = 1+(rand()%2);
    }
}

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
    int i;

    for (i=0; i<number; i++)
	Obj[i] = (object *)malloc(sizeof(object));
}


void Free_shots(int number)
{
    int i;

    for (i=0; i<number; i++)
	free(Obj[i]);
}



void Place_item(int type)
{
    object *item;
    int x, y;


    if (NumObjs >= MAX_TOTAL_SHOTS)
	return;

    item = Obj[NumObjs++];

    do {
	x=rand()%World.x;
	y=rand()%World.y;
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
    item->life = 1500+(rand()%500);

    switch (type) {
    case ITEM_SMART_SHOT_PACK:
	item->type = OBJ_SMART_SHOT_PACK;
	break;
    case ITEM_SENSOR_PACK:
	item->type = OBJ_SENSOR_PACK;
	break;
    case ITEM_MINE_PACK:
	item->type = OBJ_MINE_PACK;
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
    mine->mass = 30.0;
    mine->life = 5000+(rand()%200);
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
    dir = (rand()%spread) - spreadoffset;	/* Tmp direction */
    speed = 9+(rand()%4);
    shot->color=WHITE;
    shot->id = -1;
    shot->pos.x = World.cannon[ind].pos.x*BLOCK_SZ+BLOCK_SZ/2;
    shot->pos.y = World.cannon[ind].pos.y*BLOCK_SZ+BLOCK_SZ/2;
    shot->status = GRAVITY;
    shot->acc.x = shot->acc.y = 0;
    shot->mass = 0.4;
    shot->type = OBJ_CANNON_SHOT;
    shot->life = 25+(rand()%20);

    switch (World.cannon[ind].dir) {
    case DIR_UP:
	shot->pos.y+=BLOCK_SZ/6;
	dir+=DIR_UP;
	break;
    case DIR_DOWN:
	shot->pos.y-=BLOCK_SZ/6;
	dir+=DIR_DOWN;
	break;
    case DIR_RIGHT:
	shot->pos.x+=BLOCK_SZ/6;
	dir+=DIR_RIGHT;
	break;
    case DIR_LEFT:
	shot->pos.x-=BLOCK_SZ/6;
	dir+=DIR_LEFT;
	break;
    }

    shot->dir = MOD(shot->dir, RES);
    shot->vel.x=speed*tcos(dir);
    shot->vel.y=speed*tsin(dir);
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
	if (pl->fuel < ABS(ED_SHOT))
	    return;

	shot->life = pl->shot_life;
	shot->mass = pl->shot_mass;
	shot->max_speed = SPEED_LIMIT;
	pl->fuel += ED_SHOT;
	break;

    case OBJ_SMART_SHOT:
	if (((pl->fuel < ABS(ED_SMART_SHOT))
	     || !BIT(pl->used, OBJ_COMPASS)
	     || ((pl->lock.distance > pl->sensor_range)
		 && BIT(World.rules->mode, LIMITED_VISIBILITY)))
	    || !pl->visibility[GetInd[pl->lock.pl_id]].canSee
	    || (pl->missiles <= 0))
	    return;

	shot->mass = 5.0;
	shot->max_speed = SMART_SHOT_MAX_SPEED;
	shot->turnspeed = 2.6;
	shot->life = 1000 + rand()%500;	    /* Smarties last longer :) */
	shot->info = pl->lock.pl_id;
	pl->fuel += ED_SMART_SHOT;
	pl->missiles--;
	break;
    }
    shot->type	= type;
    shot->id	= pl->id;
    shot->color	= pl->color;
    shot->pos.x = pl->pos.x + ships[dir].pts[0].x;
    shot->pos.y = pl->pos.y + ships[dir].pts[0].y;
    shot->vel.x = pl->vel.x + ((tcos(dir) * pl->shot_speed));
    shot->vel.y = pl->vel.y + ((tsin(dir) * pl->shot_speed));
    shot->status= GRAVITY;
    shot->acc.x = shot->acc.y = 0;
    shot->dir	= dir;

    Recoil((object *)pl, shot);

    NumObjs++; pl->shots++;

}



void Delete_shot(int ind)	    /* Removes shot from array */
{
    object *shot = Obj[ind];	    /* Used when swapping places */
    player *pl;


    switch (shot->type) {
    case OBJ_MINE:
	Explode_object(shot->pos.x, shot->pos.y, 0, RES, 500);
    case OBJ_CANNON_SHOT:
    case OBJ_CANNON_DEBRIS:
    case OBJ_DEBRIS:
    case OBJ_SPARK:
	break;

	/* Shots related to a player. */
    case OBJ_SHOT:
    case OBJ_SMART_SHOT:
	pl=Players[GetInd[shot->id]];
	pl->shots--;
	break;

	/* Special items. */
    case OBJ_SMART_SHOT_PACK:
	World.items[ITEM_SMART_SHOT_PACK].num--;
	break;
    case OBJ_SENSOR_PACK:
	World.items[ITEM_SENSOR_PACK].num--;
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
	World.items[ITEM_MINE_PACK].num--;
	break;
    default:
	printf("Delete_shot(): Unkown shot type %d.\n", shot->type);
	break;
    }


    Obj[ind] = Obj[--NumObjs];
    Obj[NumObjs] = shot;
}



void Move_smart_shot(int ind)
{
    object *shot = Obj[ind];
    player *pl = Players[ GetInd[Obj[ind]->info] ];
    double theta;
    int vinkel;


    theta = atan2(pl->pos.y-(shot->pos.y+2*shot->vel.y),
		  pl->pos.x-(shot->pos.x+2*shot->vel.x));

    vinkel=(RES/(2.0*PI))*theta;

    if (vinkel < 0)
	vinkel += RES;
    vinkel %= RES;

    if (vinkel < shot->dir)
	vinkel += RES;
    vinkel = vinkel - shot->dir;

    if (vinkel < RES/2)
	shot->dir += shot->turnspeed;
    else
	shot->dir -= shot->turnspeed;

    shot->dir = MOD(shot->dir, RES);	/* MERK!!!! */

    if (shot->velocity > shot->max_speed)
	shot->velocity-=SMART_SHOT_ACC;
    else
	shot->velocity+=SMART_SHOT_ACC;

/*  shot->velocity = MIN(shot->velocity, shot->max_speed);  */

    shot->vel.x = tcos(shot->dir) * shot->velocity;
    shot->vel.y = tsin(shot->dir) * shot->velocity;
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


    no_debris=(intensity/2)+(rand()%(intensity/2));
    for (i=0; i<no_debris && NumObjs<MAX_TOTAL_SHOTS; i++, NumObjs++) {
	debris = Obj[NumObjs];
	speed = (1024+intensity)/intensity+(rand()%(intensity/4));
	dir = real_dir + (rand()%spread)-spreadoffset;
	debris->color = RED;
	debris->id = 0;
	debris->pos.x = x;
	debris->pos.y = y;
	debris->vel.x = (tcos(dir) * speed);
	debris->vel.y = (tsin(dir) * speed);
	debris->status = GRAVITY;
	debris->acc.x = debris->acc.y = 0;
	debris->dir = dir;
	debris->mass = 5.0;
	debris->type = OBJ_CANNON_DEBRIS;
	debris->life = 8+((rand()%(intensity/4))*2);
    }
}

void Explode(int ind)	/* Index of unfortunate player */
{
    player *pl;
    object *debris;
    int i, dir, no_debris, speed;


    pl = Players[ind];
    no_debris = 1+(pl->fuel/8.0)+(rand()%((int)(pl->mass*4.0)));
/*  shot_mass = pl->mass / no_debris;	Not used! */
    for (i=0; i<no_debris && NumObjs<MAX_TOTAL_SHOTS; i++, NumObjs++) {
	debris = Obj[NumObjs];
	dir = rand()%RES;
	speed = 0.7+(rand()%9)+(rand()%200)/10;
	debris->color=RED;
	debris->id = pl->id;
	debris->pos.x = pl->pos.x;
	debris->pos.y = pl->pos.y;
	debris->vel.x = pl->vel.x + (tcos(dir) * speed);
	debris->vel.y = pl->vel.y + (tsin(dir) * speed);
	debris->status = GRAVITY;
	debris->acc.x = debris->acc.y = 0;
	debris->dir = dir;
	debris->mass = 4.0;
	debris->type = OBJ_DEBRIS;
	debris->life = 4+(rand()%(int)(pl->mass*1.5));
    }
}

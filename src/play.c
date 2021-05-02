/* play.c,v 1.3 1992/05/11 15:31:29 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <stdio.h>
#include "pilot.h"
#include "map.h"
#include "draw.h"
#include "score.h"

extern int Antall;
extern double Gravity;
extern player *Players[];
extern object *Shots[];
extern int Ant_Shots;
extern double tbl_sin[];
extern wireobj ships[];
extern XColor colors[];
extern World_map World;

void Explode_object(double, double, int, int, int);



/********** **********
 * Functions for ship movement.
 */

void Thrust(int indeks)
{
    player *pl;
    object *spark;
    int i, dir, no_sparks;
    const int spread = (RESOLUTION*0.3);
    const int spreadoffset = (spread/2);
    double x, y;


    pl=Players[indeks];

    no_sparks=(pl->power*0.3)+(rand()%3);

    x=pl->pos.x+(ships[pl->dir].pts[1].x+ships[pl->dir].pts[2].x)/2;
    y=pl->pos.y+(ships[pl->dir].pts[1].y+ships[pl->dir].pts[2].y)/2;

    for (i=0; i<no_sparks && Ant_Shots<MAX_TOTAL_SHOTS; i++, Ant_Shots++) {
	spark=Shots[Ant_Shots];
	dir = pl->dir + (RESOLUTION/2) + (rand()%spread)-spreadoffset;
	spark->color=RED;
	spark->id = pl->id;
	spark->pos.x = x;
	spark->pos.y = y;
	spark->vel.x = pl->vel.x + ((tcos(dir) *
				     (1+rand()%(int)(pl->power*0.2))));
	spark->vel.y = pl->vel.y + ((tsin(dir) *
				     (1+rand()%(int)(pl->power*0.2))));
	spark->status = GRAVITY;
	spark->acc.x = spark->acc.y = 0.0;
	spark->dir = MOD(spark->dir, RESOLUTION);
	spark->mass = 1.0;
	spark->type = OBJ_SPARK;
	spark->life = 3+(rand()%(int)(pl->power*0.3));
    }
}


void Turn_thrust(int indeks)
{
    player *pl = Players[indeks];
    object *spark;
    int i, dir, no_sparks;
    const int spread = (RESOLUTION*0.08);
    const int spreadoffset = (spread/2);
    int x, y;
    double rate=ABS(Players[indeks]->turnacc*0.1);


    no_sparks=(rand()%(int)(rate*2));

    x=pl->pos.x+ships[pl->dir].pts[0].x;
    y=pl->pos.y+ships[pl->dir].pts[0].y;

    for (i=0; i<no_sparks && Ant_Shots<MAX_TOTAL_SHOTS; i++, Ant_Shots++) {
	spark=Shots[Ant_Shots];
	dir = pl->dir + (RESOLUTION/4) + (rand()%spread)-spreadoffset;

	if (pl->turnacc > 0.0)
	    dir=dir+RESOLUTION/2;

	spark->color=RED;
	spark->id = pl->id;
	spark->pos.x = x;
	spark->pos.y = y;
	spark->vel.x = pl->vel.x + (tcos(dir) * rate);
	spark->vel.y = pl->vel.y + (tsin(dir) * rate);
	spark->status = GRAVITY;
	spark->acc.x = spark->acc.y = 0;
	spark->dir = MOD(spark->dir, RESOLUTION);
	spark->mass = 1.0;
	spark->type = OBJ_SPARK;
	spark->life = 1+(rand()%2);
    }
}

/* Rekylen til ship hvis den 'skyter ut' shot */
void Rekyl(object *ship, object *shot)
{
    ship->vel.x -= ((tcos(shot->dir) * ABS(shot->vel.x-ship->vel.x) *
	shot->mass) / ship->mass);
    ship->vel.y -= ((tsin(shot->dir) * ABS(shot->vel.y-ship->vel.y) *
	shot->mass) / ship->mass);
}

void Delta_mv(object *ship, object *obj)
{
    double dvx, dvy, ship_theta, obj_theta, dm;


    dvx=ABS(obj->vel.x - ship->vel.x);
    dvy=ABS(obj->vel.y - ship->vel.y);
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
	Shots[i]=(object *)malloc(sizeof(object));
}


void Free_shots(int number)
{
    int i;

    for (i=0; i<number; i++)
	free(Shots[i]);
}



void Place_item(int type)
{
    object *item;
    int x, y;


    if (Ant_Shots >= MAX_TOTAL_SHOTS)
	return;

    item = Shots[Ant_Shots++];

    do {
	x=rand()%World.x;
	y=rand()%World.y;
    } while (World.type[x][y] != SPACE);

    item->color = RED;
    item->info = type;
    item->status = 0;
    item->id = -1;
    item->pos.x = x*WORLD_SPACE+WORLD_SPACE/2;
    item->pos.y = y*WORLD_SPACE+WORLD_SPACE/2;
    item->vel.x=item->vel.y = item->acc.x=item->acc.y = 0.00000001;
		/* Near zero, but not zero or atan2 will complain. */
    item->mass = 10.0;
    item->life = 1500+(rand()%500);

    switch (type) {
    case ITEM_SMART_SHOT_PACK:
	item->type = OBJ_SMART_SHOT_PACK;
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


    if (Ant_Shots >= MAX_TOTAL_SHOTS)
	return;

    mine = Shots[Ant_Shots++];
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


void Cannon_fire(int indeks)
{
    object *shot;
    int dir, speed;
    const int spread = (RESOLUTION*0.3);
    const int spreadoffset = (spread/2);


    if (Ant_Shots >= MAX_TOTAL_SHOTS)
	return;

    shot=Shots[Ant_Shots++];
    dir = (rand()%spread) - spreadoffset;	/* Tmp direction */
    speed = 9+(rand()%4);
    shot->color=WHITE;
    shot->id = -1;
    shot->pos.x = World.cannon[indeks].pos.x*WORLD_SPACE+WORLD_SPACE/2;
    shot->pos.y = World.cannon[indeks].pos.y*WORLD_SPACE+WORLD_SPACE/2;
    shot->status = GRAVITY;
    shot->acc.x = shot->acc.y = 0;
    shot->mass = 0.4;
    shot->type = OBJ_CANNON_SHOT;
    shot->life = 25+(rand()%20);

    switch (World.cannon[indeks].dir) {
    case UP:
	shot->pos.y+=WORLD_SPACE/6;
	dir+=UP;
	break;
    case DOWN:
	shot->pos.y-=WORLD_SPACE/6;
	dir+=DOWN;
	break;
    case RIGHT:
	shot->pos.x+=WORLD_SPACE/6;
	dir+=RIGHT;
	break;
    case LEFT:
	shot->pos.x-=WORLD_SPACE/6;
	dir+=LEFT;
	break;
    }

    shot->dir = MOD(shot->dir, RESOLUTION);
    shot->vel.x=speed*tcos(dir);
    shot->vel.y=speed*tsin(dir);
}



void Fire_shot(int indeks, int type, int dir)	    /* Initializes a new shot */
{
    object *shot;
    player *pl;


    pl=Players[indeks];
    if ((pl->shots >= pl->shot_max) || (Ant_Shots >= MAX_TOTAL_SHOTS)
	|| BIT(pl->used, OBJ_SHIELD))
	return;

    shot=Shots[Ant_Shots];
    switch (type) {

    case OBJ_SHOT:
	if (pl->fuel < -ED_SHOT)
	    return;

	shot->life = pl->shot_life;
	shot->mass = pl->shot_mass;
	shot->max_speed = SPEED_LIMIT;
	pl->fuel += ED_SHOT;
	break;

    case OBJ_SMART_SHOT:
	if (((pl->fuel < -ED_SMART_SHOT) ||
	     (!BIT(pl->used, OBJ_COMPASS)) ||
	     ((pl->lock.distance > pl->sensor_range)) &&
	     (BIT(World.rules->mode, LIMITED_VISIBILITY))) ||
	    BIT(Players[get_ind[pl->lock.pl_id]]->status, INVISIBLE) ||
	    (pl->missiles <= 0))
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
    shot->type = type;
    shot->id = pl->id;
    shot->color=pl->color;
    shot->pos.x = pl->pos.x + ships[dir].pts[0].x;
    shot->pos.y = pl->pos.y + ships[dir].pts[0].y;
    shot->vel.x = pl->vel.x + ((tcos(dir) * pl->shot_speed));
    shot->vel.y = pl->vel.y + ((tsin(dir) * pl->shot_speed));
    shot->status = GRAVITY;
    shot->acc.x = shot->acc.y = 0;
    shot->dir = dir;

    Rekyl((object *)pl, shot);

    Ant_Shots++; pl->shots++;

}



void Delete_shot(int indeks)	    /* Removes shot from array */
{
    object *shot = Shots[indeks];	    /* Used when swapping places */
    player *pl;


    switch (shot->type) {
    case OBJ_MINE:
	Explode_object(shot->pos.x, shot->pos.y, 0, RESOLUTION, 500);
    case OBJ_CANNON_SHOT:
    case OBJ_CANNON_DEBRIS:
    case OBJ_DEBRIS:
    case OBJ_SPARK:
	break;

	/* Shots related to a player. */
    case OBJ_SHOT:
    case OBJ_SMART_SHOT:
	pl=Players[get_ind[shot->id]];
	pl->shots--;
	break;

	/* Special items. */
    case OBJ_SMART_SHOT_PACK:
	World.items[ITEM_SMART_SHOT_PACK].ant--;
	break;
    case OBJ_CLOAKING_DEVICE:
	World.items[ITEM_CLOAKING_DEVICE].ant--;
	break;
    case OBJ_ENERGY_PACK:
	World.items[ITEM_ENERGY_PACK].ant--;
	break;
    case OBJ_WIDEANGLE_SHOT:
	World.items[ITEM_WIDEANGLE_SHOT].ant--;
	break;
    case OBJ_REAR_SHOT:
	World.items[ITEM_REAR_SHOT].ant--;
	break;
    case OBJ_MINE_PACK:
	World.items[ITEM_MINE_PACK].ant--;
	break;
    default:
	printf("Delete_shot(): Unkown shot type %d.\n", shot->type);
	break;
    }


    Shots[indeks]=Shots[--Ant_Shots];
    Shots[Ant_Shots]=shot;
}



void Move_smart_shot(int indeks)
{
    object *shot = Shots[indeks];
    player *pl = Players[ get_ind[Shots[indeks]->info] ];
    double theta;
    int vinkel;


    theta = atan2(pl->pos.y-(shot->pos.y+2*shot->vel.y),
		  pl->pos.x-(shot->pos.x+2*shot->vel.x));

    vinkel=(RESOLUTION/(2.0*PI))*theta;

    if (vinkel < 0)
	vinkel += RESOLUTION;
    vinkel %= RESOLUTION;

    if (vinkel < shot->dir)
	vinkel += RESOLUTION;
    vinkel = vinkel - shot->dir;

    if (vinkel < RESOLUTION/2)
	shot->dir += shot->turnspeed;
    else
	shot->dir -= shot->turnspeed;

    shot->dir = MOD(shot->dir, RESOLUTION);	/* MERK!!!! */

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
    for (i=0; i<no_debris && Ant_Shots<MAX_TOTAL_SHOTS; i++, Ant_Shots++) {
	debris = Shots[Ant_Shots];
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

void Explode(int indeks)	/* Index of unfortunate player */
{
    player *pl;
    object *debris;
    int i, dir, no_debris, speed;


    pl=Players[indeks];
    no_debris=1+(pl->fuel/8.0)+(rand()%((int)(pl->mass*4.0)));
/*  shot_mass=pl->mass / no_debris;	Not used! */
    for (i=0; i<no_debris && Ant_Shots<MAX_TOTAL_SHOTS; i++, Ant_Shots++) {
	debris=Shots[Ant_Shots];
	dir = rand()%RESOLUTION;
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

/* robot.c,v 1.3 1992/06/26 15:25:46 bjoerns Exp
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
    "@(#)robot.c,v 1.3 1992/06/26 15:25:46 bjoerns Exp";
#endif


extern long	KILLING_SHOTS;

int		NumRobots = 0;
static int	MAX_ROBOTS = 1;

static robot_t Robots[] = {
    "Mad Max",		94, 20,
    "Blackie",		10, 90,
    "Kryten",		70, 40,
    "Marvin",		30, 70,
    "R2D2",		50, 60,
    "C3PO",		60, 50,
    "K9",		50, 50,
    "Robby",		45, 55,
    "Mickey",		05, 95,
    "Hermes",		15, 85,
    "Pan",		60, 60,
    "Azurion",		40, 30,
    "Droidion",		60, 30,
    "Terminator",	80, 40,
    "Sniper",		30, 90,
    "Slugger",		40, 40,
    "Uzi",		95,  5,
    "Capone",		80, 50,
    "Tanx",		40, 70,
    "Chrome Star",	60, 60,
    "Bully",		80, 10,
    "Metal Hero",	40, 45,
    "Aurora",		60, 55,
    "Dalt Wisney",	30, 75,
    "Psycho",		65, 55,
    "Gorgon",		30, 40,
    "Pompel",		50, 50,
    "Pilt",		50, 50,
    "Sparky",		20, 40,
    "Cobra",		85, 60,
    "Falcon",		70, 20,
    "Boson",		25, 35,
    "Blazy",		40, 40,
    "Pixie",		15, 93,
    "Wimpy",		 5, 98,
    "Bonnie",		30, 40,
    "Clyde",		40, 45,
    "Neuro",		70, 70,
    NULL,		 0,  0
};


/*
 * Private functions.
 */
static void Calculate_max_robots(void);
static void New_robot(void);
static void Delete_robot(void);
static bool Check_robot_evade(int ind, int mine_i, int ship_i);
static bool Check_robot_attack(int ind, int ship_i, double ship_dist);
static bool Check_robot_harvest(int ind, int item_i, double item_dist);



/********** **********
 * Updating robots and the like.
 */

static void Calculate_max_robots(void)
{
    int	i;

    for (i=0; Robots[i].name != NULL; i++)
	;

    MAX_ROBOTS = i;
}



static void Delete_robot(void)
{
    long	i, low_score = LONG_MAX, low_i = -1;


    for (i=0; i<NumPlayers; i++) {
	if (Players[i]->robot_mode == RM_NOT_ROBOT)
	    continue;

	if (Players[i]->score < low_score) {
	    low_i = i;
	    low_score = Players[i]->score;
	}
    }

    if (low_i >= 0) {
	Quit(low_i);
    }
}



static void New_robot(void)
{
    player	*robot;
    robot_t	*rob;
    int		i, num;
    static bool	first_time = true;


    if (first_time) {
	Calculate_max_robots();
	first_time = false;
    }

    if (NumPlayers >= World.NumBases-1)
	return;

    Init_player(NumPlayers);
    robot = Players[NumPlayers];

 new_name:
    num = rand()%MAX_ROBOTS;

    for (i=0; i<NumPlayers; i++)
	if (num == Players[i]->robot_ind)
	    goto new_name;

    rob = &Robots[num];

    strcpy(robot->name, rob->name);
    strcpy(robot->realname, "robot");

    robot->disp_type	= DT_NONE;
    robot->color	= WHITE;
    robot->name_length	= strlen(robot->name) * 6;
    robot->turnspeed	= MAX_PLAYER_TURNSPEED;
    robot->turnspeed_s	= MAX_PLAYER_TURNSPEED;
    robot->turnresistance	= 0.12;
    robot->turnresistance_s	= 0.12;
    robot->power	= MAX_PLAYER_POWER;
    robot->power_s	= MAX_PLAYER_POWER;
    robot->instruments	= 0;
    robot->team		= 0;
    robot->mychar	= 'R';
    robot->robot_mode	= RM_TAKE_OFF;
    robot->robot_ind	= num;

/*    robot->shot_speed	= ShotsSpeed + (rob->attack - 50) / 5.0;
    robot->shot_mass	= ShotsMass + (rob->defense - rob->attack) / 10.0;
    robot->max_speed	= SPEED_LIMIT - robot->shot_speed;
*/
    NumPlayers++;
    Id++;
    NumRobots++;

#ifndef	SILENT    
    printf("%s (%d, %s) starts at startpos %d.\n",
	   robot->name, NumPlayers, robot->realname, robot->home_base);
#endif

    Set_label_strings();
}


static bool Check_robot_evade(int ind, int mine_i, int ship_i)
{
    int i;
    player *pl;
    object *shot;
    player *ship;
    int stop_dist;
    bool evade;
    bool left_ok,right_ok;
    int safe_width;
    int travel_dir;
    int delta_dir;
    int aux_dir;
    int px[3],py[3];
    int dist;
    int locn_type;
    vector *gravity;
    int gravity_dir;
    int dx,dy;

    pl		= Players[ind];
    safe_width	= SHIP_SZ*2;
    stop_dist	=
	(3*RES * pl->velocity) / (4*MAX_PLAYER_TURNSPEED * pl->turnresistance)
	    + (pl->velocity * pl->velocity * pl->mass) / (2 * MAX_PLAYER_POWER)
		+ safe_width;
    evade = false;

    if (pl->velocity <= 0.05)
	travel_dir = DIR_UP;
    else
	travel_dir = atan2(pl->vel.y, pl->vel.x) * RES / (2.0 * PI);

    aux_dir = MOD(travel_dir+RES/4, RES);
    px[0] = pl->pos.x;				/* ship center x */
    py[0] = pl->pos.y;				/* ship center y */
    px[1] = px[0] + safe_width * tcos(aux_dir);	/* ship left side x */
    py[1] = py[0] + safe_width * tsin(aux_dir);	/* ship left side y */
    px[2] = 2 * px[0] - px[1];			/* ship right side x */
    py[2] = 2 * py[0] - py[1];			/* ship right side y */

    left_ok = true;
    right_ok = true;

    for (dist=0; dist<stop_dist+BLOCK_SZ/2; dist+=BLOCK_SZ/2) {
	for (i=0; i<3; i++) {
	    dx = (px[i] + dist * tcos(travel_dir)) / BLOCK_SZ;
	    dy = (py[i] + dist * tsin(travel_dir)) / BLOCK_SZ;

	    if (dx<0 || dx>=World.x || dy<0 || dy>=World.y) {
		evade = true;
		if (i==1) left_ok = false;
		if (i==2) right_ok = false;
		continue;
	    }

	    locn_type = World.block[dx][dy];
	    if (locn_type != SPACE && locn_type != BASE
		&& locn_type != POS_GRAV && locn_type != NEG_GRAV
		&& locn_type != CWISE_GRAV && locn_type != ACWISE_GRAV) {

		evade = true;
		if (i==1) left_ok = false;
		if (i==2) right_ok = false;
		continue;
            }

	    gravity = &World.gravity[dx][dy]; /* watch out for strong gravity */
            if (LENGTH(gravity->x, gravity->y) >= 1.0) {
		gravity_dir = atan2(gravity->y-pl->pos.y, gravity->x-pl->pos.x)
		    		* RES / (2.0 * PI);
		if (MOD(gravity_dir-travel_dir, RES) <= RES/4 ||
		    MOD(gravity_dir-travel_dir, RES) >= 3*RES/4) {
		    evade = true;
		    if (i==1) left_ok = false;
		    if (i==2) right_ok = false;
		    continue;
		}
	    }
	}
    }

    if (mine_i >= 0) {
	shot = Obj[mine_i];
	aux_dir = atan2(shot->pos.y-pl->pos.y, shot->pos.x-pl->pos.x)
	    		* RES/(2.0*PI);
	delta_dir = MOD(aux_dir - travel_dir,RES);
	if (delta_dir < RES/4) {
	    pl->robot_mode = RM_EVADE_RIGHT;
	    evade = true;
	}
	if (delta_dir > RES*3/4) {
	    pl->robot_mode = RM_EVADE_LEFT;
	    evade = true;
	}
    }

    if (ship_i >= 0) {
	ship = Players[ship_i];
	aux_dir = atan2(ship->pos.y-pl->pos.y, ship->pos.x-pl->pos.x)
	    		* RES/(2.0*PI);
	delta_dir = MOD(aux_dir - travel_dir,RES);
	if (delta_dir < RES/4) {
	    pl->robot_mode = RM_EVADE_RIGHT;
	    evade = true;
	}
	if (delta_dir > RES*3/4) {
	    pl->robot_mode = RM_EVADE_LEFT;
	    evade = true;
	}
    }

    if (pl->velocity > MAX_ROBOT_SPEED)
	evade = true;

    if (!evade) {
	if (pl->robot_mode==RM_EVADE_LEFT || pl->robot_mode==RM_EVADE_RIGHT) {
	    if (pl->velocity > 1.0) {
		pl->robot_mode = RM_ROBOT_IDLE;
		pl->turnacc = 0;
		CLR_BIT(pl->status, THRUSTING);
	    }
	}
	return false;
    }

    delta_dir = 0;
    while (!left_ok && !right_ok && delta_dir<7*RES/8) {
	delta_dir += RES/16;

	left_ok = true;
	aux_dir = MOD(travel_dir+delta_dir,RES);
	for (dist=0; dist < stop_dist+BLOCK_SZ/2; dist += BLOCK_SZ/2) {
	    dx = (px[0] + dist * tcos(aux_dir)) / BLOCK_SZ;
	    dy = (py[0] + dist * tsin(aux_dir)) / BLOCK_SZ;

	    if (dx<0 || dx>=World.x || dy<0 || dy>=World.y) {
		left_ok = false;
		continue;
	    }

	    locn_type = World.block[dx][dy];
	    if (locn_type != SPACE && locn_type != BASE
		&& locn_type != POS_GRAV && locn_type != NEG_GRAV
		&& locn_type != CWISE_GRAV && locn_type != ACWISE_GRAV)
	    {
		left_ok = false;
		continue;
            }

	    gravity = &World.gravity[dx][dy]; /* watch out for strong gravity */
            if (LENGTH(gravity->x, gravity->y) >= 1.0) {
		gravity_dir = atan2(gravity->y-pl->pos.y, gravity->x-pl->pos.x)
		    		* RES / (2.0 * PI);
		if (MOD(gravity_dir-travel_dir, RES) <= RES/4 ||
		    MOD(gravity_dir-travel_dir, RES) >= 3*RES/4) {

		    left_ok = false;
		    continue;
		}
	    }
	}

	right_ok = true;
	aux_dir = MOD(travel_dir-delta_dir, RES);
	for (dist=0; dist < stop_dist+BLOCK_SZ/2; dist += BLOCK_SZ/2) {
	    dx = (px[0] + dist * tcos(aux_dir)) / BLOCK_SZ;
	    dy = (py[0] + dist * tsin(aux_dir)) / BLOCK_SZ;

	    if (dx<0 || dx>=World.x || dy<0 || dy>=World.y) {
		right_ok = false;
		continue;
	    }

	    locn_type = World.block[dx][dy];
	    if (locn_type != SPACE && locn_type != BASE
		&& locn_type != POS_GRAV && locn_type != NEG_GRAV
		&& locn_type != CWISE_GRAV && locn_type != ACWISE_GRAV) {

		right_ok = false;
		continue;
            }

	    gravity = &World.gravity[dx][dy]; /* watch out for strong gravity */
            if (LENGTH(gravity->x, gravity->y)>=1.0) {
		gravity_dir = atan2(gravity->y-pl->pos.y, gravity->x-pl->pos.x)
	    			* RES / (2.0 * PI);
		if (MOD(gravity_dir-travel_dir, RES) <= RES/4 ||
		    MOD(gravity_dir-travel_dir, RES) >= 3*RES/4) {

		    right_ok = false;
		    continue;
		}
	    }
	}
    }

    pl->turnspeed = MAX_PLAYER_TURNSPEED;
    pl->power = MAX_PLAYER_POWER;

    delta_dir = MOD(pl->dir - travel_dir, RES);

    if (left_ok && !right_ok)
	pl->robot_mode = RM_EVADE_LEFT;
    else if (right_ok && !left_ok)
	pl->robot_mode = RM_EVADE_RIGHT;
    else
	if (pl->robot_mode != RM_EVADE_LEFT && pl->robot_mode != RM_EVADE_RIGHT)
	    pl->robot_mode = (delta_dir < RES/2 ?
			    RM_EVADE_LEFT : RM_EVADE_RIGHT);

    if (delta_dir < 3*RES/8 || delta_dir > 5*RES/8) {
	pl->turnacc = (pl->robot_mode == RM_EVADE_LEFT ?
		       pl->turnspeed : (-pl->turnspeed));
	CLR_BIT(pl->status, THRUSTING);
    } else {
	pl->turnacc = 0;
	SET_BIT(pl->status, THRUSTING);
    }

    return true;
}


static bool Check_robot_attack(int ind, int ship_i, double ship_dist)
{
    int i;
    player *pl;
    player *ship;
    int attack_level;
    int ship_dir;
    int travel_dir;
    int delta_dir;
    bool slowing;

    pl = Players[ind];

    if (pl->lock.tagged == LOCK_PLAYER) {
	ship = Players[GetInd[pl->lock.pl_id]];
	if (!(BIT(ship->status, PLAYING))) {
	    pl->lock.pl_id	= 1;
	    pl->lock.tagged	= LOCK_NONE;
	    pl->lock.pos.x	= pl->pos.x;
	    pl->lock.pos.y	= pl->pos.y;
	    pl->lock.distance	= 0;
	}
    }

    if (ship_i >= 0) {
	ship = Players[ship_i];
	if (pl->lock.tagged == LOCK_NONE || ship_dist < 3*pl->lock.distance/4) {
	    pl->lock.pl_id	= ship->id;
	    pl->lock.tagged	= LOCK_PLAYER;
	    pl->lock.pos.x	= ship->pos.x;
	    pl->lock.pos.y	= ship->pos.y;
	    pl->lock.distance	= LENGTH(pl->pos.x-ship->pos.x,
					 pl->pos.y-ship->pos.y);
	    pl->sensor_range	= VISIBILITY_DISTANCE;
	}
    }

    if (pl->lock.tagged != LOCK_PLAYER) {
	if (pl->robot_mode == RM_ATTACK) {
	    pl->robot_mode = RM_ROBOT_IDLE;
	    pl->turnacc = 0;
	    CLR_BIT(pl->status, THRUSTING);
	}
	return false;
    }

    ship = Players[GetInd[pl->lock.pl_id]];
    ship_dist = LENGTH(ship->pos.y-pl->pos.y, ship->pos.x-pl->pos.x);
    ship_dir = atan2(ship->pos.y - pl->pos.y + SHIP_SZ,
		     ship->pos.x - pl->pos.x) * RES / (2.0*PI);
    attack_level = MAX(ship->score/8, 0);

    if (pl->velocity <= 0.05)
	travel_dir = DIR_UP;
    else
	travel_dir = atan2(pl->vel.y, pl->vel.x) * RES / (2.0*PI);

    pl->turnspeed = MAX_PLAYER_TURNSPEED/2;
    pl->power = MAX_PLAYER_POWER/2;

    delta_dir = MOD(ship_dir-travel_dir, RES);
    if (delta_dir >= RES/4 && delta_dir <= 3*RES/4
	&& ship_dist > BLOCK_SZ) {
	pl->turnspeed = MAX_PLAYER_TURNSPEED;
	SET_BIT(pl->status, THRUSTING);
	slowing = true;
    } else {
	slowing = false;
    }

    delta_dir = MOD(ship_dir-pl->dir, RES);
    if ((delta_dir > RES/16 && delta_dir < 15*RES/16)
	|| (pl->robot_count%3)==0) {
	pl->turnacc = (delta_dir < RES/2
		       ? pl->turnspeed : (-pl->turnspeed));
    } else {
	pl->turnacc = 0.0;
    }

    if (ship_dist < 4*BLOCK_SZ) {
	if (pl->velocity < NORMAL_ROBOT_SPEED/2)
	    SET_BIT(pl->status, THRUSTING);
	if (pl->velocity > NORMAL_ROBOT_SPEED)
	    CLR_BIT(pl->status, THRUSTING);
    } else {
	if (pl->velocity < ATTACK_ROBOT_SPEED/2)
	    SET_BIT(pl->status, THRUSTING);
	if (pl->velocity > ATTACK_ROBOT_SPEED && !slowing)
	    CLR_BIT(pl->status, THRUSTING);
    }

    if (pl->missiles > 0 && (pl->robot_count%10) == 0) {
	Fire_shot(ind, OBJ_SMART_SHOT, pl->dir);
    } else if ((pl->robot_count%2) == 0
	       && (pl->robot_count %
		   (110-Robots[pl->robot_ind].attack)) < 15+attack_level) {
	Fire_shot(ind, OBJ_SHOT, pl->dir);
	for (i=0; i<pl->extra_shots; i++) {
	    Fire_shot(ind,OBJ_SHOT, MOD(pl->dir+(1+i)*SHOTS_ANGLE, RES));
	    Fire_shot(ind,OBJ_SHOT, MOD(pl->dir-(1+i)*SHOTS_ANGLE, RES));
	}
	if (BIT(pl->have, OBJ_REAR_SHOT))
	    Fire_shot(ind, OBJ_SHOT, MOD(pl->dir+RES/2, RES));
    }

    if (pl->fuel < pl->fuel2 && pl->mines > 0 && (pl->robot_count%30)==0) {
	Place_mine(ind);
	pl->mines--;
	CLR_BIT(pl->used, OBJ_CLOAKING_DEVICE);
    }

    pl->robot_mode = RM_ATTACK;

    return true;
}


static bool Check_robot_harvest(int ind, int item_i, double item_dist)
{
    player *pl;
    object *shot;
    int item_dir;
    int travel_dir;
    int delta_dir;
    bool slowing;

    pl = Players[ind];

    if (item_i < 0) {
	if (pl->robot_mode == RM_HARVEST) {
	    pl->robot_mode = RM_ROBOT_IDLE;
	    pl->turnacc = 0;
	    CLR_BIT(pl->status, THRUSTING);
	}
	return false;
    }

    shot = Obj[item_i];
    item_dir = atan2(shot->pos.y-pl->pos.y + SHIP_SZ,
		     shot->pos.x-pl->pos.x) * RES/(2.0*PI);

    if (pl->velocity <= 0.05)
	travel_dir = DIR_UP;
    else
	travel_dir = atan2(pl->vel.y,pl->vel.x) * RES/(2.0*PI);

    pl->turnspeed = MAX_PLAYER_TURNSPEED/2;
    pl->power     = MAX_PLAYER_POWER/2;

    delta_dir = MOD(item_dir-travel_dir,RES);
    if (delta_dir >= RES/4 && delta_dir <= 3*RES/4
	&& item_dist > 2*BLOCK_SZ) {	     /* reverse direction of travel */

	pl->turnspeed	= MAX_PLAYER_TURNSPEED;
	item_dir	= MOD(travel_dir+RES/2,RES);
	slowing		= true;
    } else {
	slowing		= false;
    }

    delta_dir = MOD(item_dir-pl->dir, RES);
    if ((delta_dir > RES/8 && delta_dir < 7*RES/8)
	|| (pl->robot_count%8)==0) {
	pl->turnacc = (delta_dir < RES/2 ?
		       pl->turnspeed : (-pl->turnspeed));
    } else {
	pl->turnacc = 0.0;
    }

    if (item_dist < BLOCK_SZ) {
	if (pl->velocity < NORMAL_ROBOT_SPEED/3)
	    SET_BIT(pl->status, THRUSTING);
	if (pl->velocity > NORMAL_ROBOT_SPEED/2)
	    CLR_BIT(pl->status, THRUSTING);
    } else {
	if (pl->velocity < NORMAL_ROBOT_SPEED || slowing)
	    SET_BIT(pl->status, THRUSTING);
	if (pl->velocity > NORMAL_ROBOT_SPEED*1.5 && !slowing)
	    CLR_BIT(pl->status, THRUSTING);
    }

    pl->robot_mode = RM_HARVEST;
    return true;
}


void Update_robots(void)
{
    player	*pl, *ship;
    object	*shot;
    double	distance, mine_dist, item_dist,
    		ship_dist, enemy_dist;
    int		i, j, mine_i, item_i, ship_i, enemy_i;
    long	dx, dy;
    bool	harvest_checked = false;


    if (NumRobots < WantedNumRobots
	&& NumRobots < World.NumBases
	&& NumRobots < MAX_ROBOTS) {

	New_robot();
    }
    if (NumRobots <= 0)
	return;

    if (NumPlayers >= World.NumBases && NumRobots > 0) {
	Delete_robot();
    }

    for (i=0; i<NumPlayers; i++) {
	pl = Players[i];
	if (pl->robot_mode == RM_NOT_ROBOT)
	    continue;
	if (!BIT(pl->status, PLAYING))
	    continue;

	if (pl->robot_count <= 0)
	    pl->robot_count = 1000 + rand()%20;

	pl->robot_count--;

	CLR_BIT(pl->used, OBJ_SHIELD|OBJ_CLOAKING_DEVICE);
	mine_i		= -1;
	mine_dist	= SHIP_SZ + 200;
	item_i		= -1;
	item_dist	= VISIBILITY_DISTANCE;

	if (BIT(pl->have, OBJ_CLOAKING_DEVICE) && pl->fuel > pl->fuel3)
	    SET_BIT(pl->used, OBJ_CLOAKING_DEVICE);

	for (j=0; j<NumObjs; j++) {
	    shot = Obj[j];

	    switch (shot->type) {
	    case OBJ_WIDEANGLE_SHOT:
	    case OBJ_REAR_SHOT:
	    case OBJ_SMART_SHOT:
	    case OBJ_CLOAKING_DEVICE:
	    case OBJ_ENERGY_PACK:
	    case OBJ_MINE_PACK:
		dx = shot->pos.x - pl->pos.x;
		dy = shot->pos.y - pl->pos.y;

		if ((distance = LENGTH(dx,dy)) < item_dist) {
		    item_i	= j;
		    item_dist	= distance;
		}
		break;

	    case OBJ_MINE:
		dx = shot->pos.x - pl->pos.x;
		dy = shot->pos.y - pl->pos.y;

		if ((distance = LENGTH(dx,dy)) < mine_dist) {
		    mine_i	= j;
		    mine_dist	= distance;
		}
		break;
	    }

	    if (BIT(shot->type, KILLING_SHOTS)
		&& ABS(shot->pos.x - pl->pos.x) < SHIP_SZ+20
		&& ABS(shot->pos.y - pl->pos.y) < SHIP_SZ+20
		&& shot->id != pl->id
		&& pl->robot_count%100 < (Robots[pl->robot_ind].defense+700)/8){
		SET_BIT(pl->used, OBJ_SHIELD);
	    }
	}

	if (pl->robot_mode == RM_TAKE_OFF) {
	    dx = pl->pos.x / BLOCK_SZ;
	    dy = pl->pos.y / BLOCK_SZ;

	    if (World.block[dx][dy] == BASE) {
		if (pl->velocity < 1.0)
		    SET_BIT(pl->status, THRUSTING);
		if (pl->velocity > 2.0)
		    CLR_BIT(pl->status, THRUSTING);
		continue;
	    } else {
		pl->robot_mode = RM_ROBOT_IDLE;
		pl->turnacc = 0.0;
		CLR_BIT(pl->status, THRUSTING);
	    }
	}

	ship_i		= -1;
	ship_dist	= (pl->fuel >= pl->fuel1 ? SHIP_SZ*6 : 0);
	enemy_i		= -1;
	enemy_dist	= (pl->fuel >= pl->fuel3 ?
			   VISIBILITY_DISTANCE*2 : VISIBILITY_DISTANCE);

	for (j=0; j<NumPlayers; j++) {
	    ship = Players[j];
	    if (j==i || !(BIT(ship->status, PLAYING)))
		continue;

	    dx = ship->pos.x - pl->pos.x;
	    dy = ship->pos.y - pl->pos.y;

	    if (ABS(dx) < ship_dist && ABS(dy) < ship_dist
		&& (distance = LENGTH(dx,dy)) < ship_dist) {

		ship_i    = j;
		ship_dist = distance;
	    }

	    if (ship->robot_mode == RM_NOT_ROBOT
		&& (distance = LENGTH(dx,dy)) < enemy_dist) {

		enemy_i    = j;
		enemy_dist = distance;
	    }
	}

	if (Check_robot_evade(i, mine_i, ship_i))
	    continue;

	if (enemy_dist > 2*item_dist && enemy_dist > 12*BLOCK_SZ) {
	    harvest_checked = true;
	    if (Check_robot_harvest(i, item_i, item_dist))
		continue;
	}

	if (Check_robot_attack(i, enemy_i, enemy_dist))
	    continue;

	if (pl->fuel<DEFAULT_PLAYER_FUEL)
	    pl->fuel += 0.2;

	if (!harvest_checked && Check_robot_harvest(i, item_i, item_dist))
	    continue;

	if (pl->robot_mode == RM_ROBOT_IDLE
	    && (pl->vel.y < (-NORMAL_ROBOT_SPEED) || (pl->robot_count%64)==0)) {
	    pl->robot_mode = RM_ROBOT_CLIMB;
	}

	if (pl->robot_mode == RM_ROBOT_CLIMB) {
	    pl->turnspeed = MAX_PLAYER_TURNSPEED/2;
	    pl->power 	  = MAX_PLAYER_POWER/2;
	    if (ABS(pl->dir-RES/4) > RES/16) {
		pl->turnacc = (pl->dir < RES/4
			       || pl->dir >= 3*RES/4
			       ? pl->turnspeed : (-pl->turnspeed));
	    } else {
		pl->turnacc = 0.0;
	    }
	    if (pl->vel.y < NORMAL_ROBOT_SPEED/2)
		SET_BIT(pl->status, THRUSTING);
	    if (pl->vel.y > NORMAL_ROBOT_SPEED)
		CLR_BIT(pl->status, THRUSTING);
	    continue;
	}
	
	/* must be idle */
	pl->robot_mode	= RM_ROBOT_IDLE;
	pl->turnspeed	= MAX_PLAYER_TURNSPEED/2;
	pl->power	= MAX_PLAYER_POWER/2;
	if (pl->velocity < NORMAL_ROBOT_SPEED/2)
	    SET_BIT(pl->status, THRUSTING);
	if (pl->velocity > NORMAL_ROBOT_SPEED)
	    CLR_BIT(pl->status, THRUSTING);
    }
}

/* $Id: player.c,v 1.16 1992/08/27 00:26:05 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <stdio.h>
#include "global.h"
#include "map.h"
#include "score.h"
#include "robot.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: player.c,v 1.16 1992/08/27 00:26:05 bjoerns Exp $";
#endif


static char msg[MSG_LEN];



/********** **********
 * functions on player array.
 */

void Pick_startpos(int ind)
{
    int	start, i;
    bool alone;


    do {
	alone = true;
	start = rand()%World.NumBases;

	for (i=0; i<NumPlayers; i++)
	    if (i != ind && Players[i]->home_base == start)
		alone = false;
    } while (!alone);

    Players[ind]->home_base = start;
}



void Go_home(int ind)
{
    player *pl = Players[ind];

    pl->pos.x = World.base[pl->home_base].x
		* BLOCK_SZ + BLOCK_SZ/2.0;
    pl->pos.y = World.base[pl->home_base].y
		* BLOCK_SZ - ships[DIR_UP].pts[1].y;
    pl->acc.x = pl->acc.y = 0.0;
    pl->vel.x = pl->vel.y = pl->velocity = 0.0;

    CLR_BIT(pl->status, THRUSTING);
#ifdef	TURN_FUEL
    pl->turnvel = pl->oldturnvel = 0.0;
#endif

    if (pl->robot_mode != RM_NOT_ROBOT)
	pl->robot_mode = RM_TAKE_OFF;
}


void Init_player(int ind)
{
    player *pl = Players[ind];
    bool too_late = false;
    int i;


    pl->world.x = pl->world.y	= 0.0;
    pl->vel.x	= pl->vel.y	= 0.0;
    pl->acc.x	= pl->acc.y	= 0.0;
    pl->dir	= pl->double_dir= DIR_UP;
    pl->turnvel		= 0.0;
#ifdef	TURN_FUEL
    pl->oldturnvel	= 0.0;
#endif
    pl->turnacc		= 0.0;
    pl->fuel.no_tanks   = 0;
    pl->fuel.current    = 0;
    pl->fuel.sum        =
    pl->fuel.tank[0]    = ( DEFAULT_PLAYER_FUEL
			   + (((rand()%400)-200) << FUEL_SCALE_BITS) );
    pl->fuel.max        = TANK_CAP(0);
    pl->after_burners   = 0;

    Pick_startpos(ind);

    if (BIT(World.rules->mode, TIMING)) {
	pl->power	= MAX_PLAYER_POWER;
	pl->turnspeed	= 27.0;
    }
    pl->mass	= pl->emptymass	= ShipMass;

    pl->check		= 0;
    pl->round		= 0;
    pl->time		= 0;
    pl->last_lap_time	= 0;
    pl->last_time	= 0;
    pl->last_lap	= 0;
    pl->best_run	= 0;
    pl->best_lap	= 0;
    pl->count		= -1;
    pl->control_count	= 0;
    pl->fuel.count	= 0;

    pl->type		= OBJ_PLAYER;
    pl->shots		= 0;
    pl->extra_shots	= 0;
    pl->rear_shots	= 0;
    pl->missiles	= 0;
    pl->mines		= 0;
    pl->cloaks		= 0;
    pl->sensors		= 0;
    pl->forceVisible	= 0;
    pl->shot_speed	= ShotsSpeed;
    pl->sensor_range	= MAX(pl->fuel.sum*ENERGY_RANGE_FACTOR,
                              VISIBILITY_DISTANCE);
    pl->max_speed	= SPEED_LIMIT - pl->shot_speed;
    pl->shot_max	= ShotsMax;
    pl->shot_life	= ShotsLife;
    pl->shot_mass	= ShotsMass;
    pl->score		= 0;
    pl->fs		= 0;
    pl->name[0]		= '\0';
    pl->ecms 		= 0;
    pl->ecmInfo.size	= 0;
    pl->damaged 	= 0;

    pl->info_press	= false;
    pl->help_press	= false;
    pl->help_page	= 0;

    pl->mode		= World.rules->mode;
    pl->status		= PLAYING | GRAVITY | DEF_BITS;
    pl->have		= DEF_HAVE;
    pl->used		= DEF_USED;

    {   static unsigned int pseudo_team_no;

        pl->pseudo_team = pseudo_team_no++;
    }
    pl->mychar		= ' ';
    pl->life		= World.rules->lives;

    /*
     * If limited lives and if nobody has lost a life yet, you may enter
     * now, otherwise you will have to wait 'til everyone gets GAME OVER.
     */
    if (BIT(pl->mode, LIMITED_LIVES)) {
	for (i=0; i<NumPlayers; i++)
	    if (Players[i]->life < 3)
		too_late = true;
	if (too_late) {
	    pl->mychar	= 'W';
	    pl->life	= 0;
	    pl->status |= GAME_OVER;
	}
    }

    if (BIT(World.rules->mode, TIMING))
	pl->team = 0;

    pl->lock.tagged	= LOCK_NONE;
    pl->lock.pl_id	= 0;
    pl->lock.pos.x	= pl->pos.x;
    pl->lock.pos.y	= pl->pos.y;

    pl->robot_mode	= RM_NOT_ROBOT;
    pl->robot_count	= 0;
    pl->robot_ind	= -1;
    pl->robot_lock	= LOCK_NONE;
    pl->robot_lock_id	= 0;

    pl->wormDrawCount   = 0;

    pl->id		= Id;
    GetInd[Id]		= ind;

    Go_home(ind);
    
    UpdateItemChances(NumPlayers-NumPseudoPlayers);
}



void Alloc_players(int number)
{
    player *x=(player*)malloc(number*sizeof(player));
    int i;

    for (i=0; i<number; i++)
	Players[i]=x++;
}



void Free_players(void)
{
    free(Players[0]);
}



void Set_label_strings(void)
{
    int i, j, hi_ind;
    double hi, hi_tmp;
    player *pl, *tmp;


    /*
     * Sorts players after score. (selection sort)
     */
    for (i=0; i<NumPlayers; i++) {

	hi = (tmp=Players[i])->score;
	hi_ind = i;
	for (j=i+1; j<NumPlayers; j++)
	    if (   ((pl=Players[j])->robot_mode!=RM_OBJECT && pl->score > hi)
                || tmp->robot_mode==RM_OBJECT)
            {   tmp = Players[hi_ind = j];
		hi = pl->score;
            }
	if (hi_ind != i) {
	    tmp = Players[hi_ind];
	    Players[hi_ind] = Players[i];
	    Players[i] = tmp;
	    GetInd[Players[i]->id] = i;
	    GetInd[Players[hi_ind]->id] = hi_ind;
	}
    }

    /*
     * Get the person with the best kill-ratio (i.e. score/number of deaths.)
     */
    hi = (double)Players[0]->score / (Players[0]->life+1);
    hi_ind = 0;
    for (i=1; i<NumPlayers; i++)
        if (Players[i]->robot_mode==RM_OBJECT)
            continue;
	else if ((hi_tmp = (double)Players[i]->score/(Players[i]->life+1)) > hi) {
	    hi = hi_tmp;
	    hi_ind = i;
	}

    /*
     * Re-formats the labels.
     */
    for (i=0; i<NumPlayers; i++) {
	char name[100];

	pl = Players[i];

        if (pl->robot_mode==RM_OBJECT)
            pl->lblstr[0]='\0';
        else {
	    if (pl->robot_mode != RM_NOT_ROBOT
		&& pl->robot_lock == LOCK_PLAYER) {
	        sprintf(name, "%s (%s)", pl->name,
		        Players[GetInd[pl->robot_lock_id]]->name);
	    } else
	        sprintf(name, "%s", pl->name);

	    sprintf(pl->lblstr, "%c%c %-19s%03d%6d",
		    (hi_ind==i) ? '*' : pl->mychar,
		    (pl->team==0) ? ' ' : pl->team+'0',
		    name, pl->life, pl->score);
        }
    }

    Set_labels();
}


/*
IKKE FERDIG
void Reset_all_players(void)
{
    int i;


    for (i=0; i<NumPlayers; i++) {
	printf("Resetting player %s\n", Players[i]->name);
	CLR_BIT(Players[i]->status, GAME_OVER);
	Players[i]->life = World.rules->lives;
	Players[i]->mychar = ' ';
    }
    Set_label_strings();
}
Kalles fra Comput_game_status()
*/


void Compute_game_status(void)
{
/*  int i;
    int num_alive = 0, num_waiting = 0;
    bool too_late = false;
    int pl_id;


    for (i=0; i<NumPlayers; i++) {
	if (!BIT(Players[i]->status, GAME_OVER)) {
	    num_alive++;
	    pl_id = i;
	}
	if (Players[i]->mychar == 'W')
	    num_waiting++;
    }

Dette er ikke ferdig Ken.  :)

    if (NumPlayers>1 && num_alive<=1) {
	if (num_alive == 0)
	    Set_message("Game over! No apparent winners.");


    if ((num_alive == 1) && (NumPlayers != 1)) {
	sprintf(msg, "%s has won the game!!!!!!! Long live %s!",
		Players[pl_id]->name, Players[pl_id]->name);
	Set_message(msg);
	Players[pl_id]->score += PTS_GAME_WON;
	Reset_all_players();
    } else if (num_alive == 0) {
	Reset_all_players();
    }	
*/
}



void Delete_player(int ind)
{
    player *pl;
    int i, id;


    pl = Players[ind];
    id = pl->id;

    for (i=0; i<NumObjs; i++)		/* Delete all remaining shots */
	if (Obj[i]->id == id)
	    Delete_shot(i);

    NumPlayers--;
    if (pl->robot_mode != RM_NOT_ROBOT && pl->robot_mode != RM_OBJECT)
	NumRobots--;

    /*
     * Swap entry no 'ind' with the last one.
     */
    pl			= Players[NumPlayers];	/* Swap pointers... */
    Players[NumPlayers]	= Players[ind];
    Players[ind]	= pl;

    GetInd[Players[ind]->id] = ind;

    for (i=0; i<NumPlayers; i++)
	if ((Players[i]->lock.pl_id == id) || NumPlayers <= 1)
	    Players[i]->lock.tagged = LOCK_NONE;
    
    UpdateItemChances(NumPlayers-NumPseudoPlayers);
}



void Kill_player(int ind)
{
    player *pl;


    Explode(ind);

    pl			= Players[ind];
    pl->vel.x		= pl->vel.y	= 0.0;
    pl->acc.x		= pl->acc.y	= 0.0;
    pl->double_dir	= pl->dir	= DIR_UP;
    pl->turnacc		= 0.0;
    pl->turnvel		= 0.0;
#ifdef	TURN_FUEL
    pl->oldturnvel	= 0.0;
#endif
    pl->mass		= ShipMass;
    pl->status		|= WAITING_SHOTS | DEF_BITS;
    pl->status		&= ~(KILL_BITS);
    pl->extra_shots	= 0;
    pl->rear_shots	= 0;
    pl->missiles	= 0;
    pl->mines		= 0;
    pl->cloaks		= 0;
    pl->sensors		= 0;
    pl->forceVisible	= 0;
    pl->shot_speed	= ShotsSpeed;
    pl->shot_max	= ShotsMax;
    pl->shot_life	= ShotsLife;
    pl->shot_mass	= ShotsMass;
    pl->last_time	= pl->time;
    pl->last_lap	= 0;
    pl->count		= RECOVERY_DELAY;
    pl->ecms 		= 0;
    pl->ecmInfo.size	= 0;
    pl->damaged 	= 0;

    pl->fuel.current    = 0;
    pl->fuel.no_tanks	= 0;
    pl->fuel.max        = TANK_CAP(0);
    pl->fuel.sum       	*= 0.90;		/* Loose 10% of fuel */
    if (pl->fuel.sum>pl->fuel.max) pl->fuel.sum = pl->fuel.max;
    pl->fuel.tank[0]    =
    pl->fuel.sum        = MAX(pl->fuel.sum,
                              MIN_PLAYER_FUEL+(rand()%(int)MIN_PLAYER_FUEL)/5);
    pl->after_burners	= 0;

    if (BIT(World.rules->mode, TIMING))
	pl->fuel.sum=pl->fuel.tank[0] = RACE_PLAYER_FUEL;

    if (BIT(pl->mode, LIMITED_LIVES))
	pl->life--;
    else
	pl->life++;

    if (pl->life == -1) {
	pl->life = 0;
	SET_BIT(pl->status, GAME_OVER);
	pl->mychar = 'D';
    }

    pl->have	= DEF_HAVE;
    pl->used	|= DEF_USED;
    pl->used	&= ~(USED_KILL);
    pl->used	&= pl->have;
}

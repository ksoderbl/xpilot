/* player.c,v 1.3 1992/05/11 15:31:31 bjoerns Exp
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
#include "score.h"
#include "default.h"

#define BLANK	" **** N o t   u s e d ! **** "

extern player *Players[];
extern object *Shots[];
extern int Antall, Ant_Shots;
extern void Set_labels(void);

extern double Shots_Mass, Ship_Power, Ship_Mass, Shots_Speed;
extern int Shots_Max, Shots_Life;
extern bool Shots_Gravity, Loose_Mass;
extern long DEF_BITS, KILL_BITS, DEF_HAVE, DEF_USED, USED_KILL;
extern World_map World;
extern wireobj ships[];

char Name_HQ[] = "HQ Base";
static char msg[MSG_LEN];



/********** **********
 * functions on player array.
*/

int Pick_startpos(ind)
{
    int start, i;
    bool alone;


    do {
	alone=true;
	start=rand()%World.Ant_start;
	for (i=0; i<Antall; i++)
	    if ((i!=ind) && (Players[i]->home_base==start))
		alone=false;
    } while (!alone);

    Players[ind]->home_base=start;
    return (start);
}



void Go_home(int ind)
{
    player *pl = Players[ind];


    pl->pos.x=World.Start_points[pl->home_base].x *
	WORLD_SPACE+WORLD_SPACE/2.0;
    pl->pos.y=World.Start_points[pl->home_base].y *
	WORLD_SPACE-ships[(int)(RESOLUTION/4)].pts[1].y;
    pl->acc.x=pl->acc.y=0.0;
    pl->vel.x=pl->vel.y=pl->velocity=0.0;
    CLR_BIT(pl->status, THRUSTING);
}


void Init_player(int ind, def_t *def)
{
    player *pl = Players[ind];
    bool too_late=false;
    int i;


    pl->world.x=0.0; pl->world.y=0.0;
    pl->home_base=Pick_startpos(ind);
    pl->vel.x=0; pl->vel.y=0;
    pl->acc.x=0; pl->acc.y=0;
    pl->dir=pl->double_dir=UP;

    Go_home(ind);

    /* Xresources. */
    pl->def.power = def->power;
    pl->def.turnspeed = def->turnspeed;
    pl->def.turnresistance = def->turnresistance;
    pl->def.power_s = def->power_s;
    pl->def.turnspeed_s = def->turnspeed_s;
    pl->def.turnresistance_s = def->turnresistance_s;
    pl->def.instruments = def->instruments;
    pl->def.team = def->team;
    pl->def.fuel3 = def->fuel3;
    pl->def.fuel2 = def->fuel2;
    pl->def.fuel1 = def->fuel1;
    pl->power = pl->def.power;
    pl->turnspeed = pl->def.turnspeed;
    pl->turnresistance = pl->def.turnresistance;
    pl->power_s = pl->def.power_s;
    pl->turnspeed_s = pl->def.turnspeed_s;
    pl->turnresistance_s = pl->def.turnresistance_s;
    pl->instruments = pl->def.instruments;
    pl->team = pl->def.team;

    pl->turnvel=0.0;
    pl->turnacc=0.0;
    pl->fuel=DEFAULT_PLAYER_FUEL+(rand()%400)-200;
    pl->max_fuel=MAX_PLAYER_FUEL;

    if (BIT(World.rules->mode, TIMING)) {
	pl->power = MAX_PLAYER_POWER;
	pl->turnspeed = 27.0;
    }
    pl->mass=pl->emptymass=Ship_Mass;

    pl->check = 0;
    pl->round = 0;
    pl->time = 0;
    pl->last_lap_time = 0;
    pl->last_time = 0;
    pl->last_lap = 0;
    pl->best_run = 0;
    pl->best_lap = 0;
    pl->count = -1;
    pl->control_count = 0;
    pl->fuel_count = 0;

    pl->type=OBJ_PLAYER;
    pl->shots=0;
    pl->extra_shots=0;
    pl->missiles=0;
    pl->mines=0;
    pl->shot_speed=Shots_Speed;
    pl->sensor_range=MAX(pl->fuel*ENERGY_RANGE_FACTOR, VISIBILITY_DISTANCE);
    pl->max_speed=SPEED_LIMIT-pl->shot_speed;
    pl->shot_max=Shots_Max;
    pl->shot_life=Shots_Life;
    pl->shot_mass=Shots_Mass;
    pl->score=0;
    pl->fs=0;
    pl->name[0]='\0';
    pl->display[0]='\0';
    pl->info_press = false;
    pl->help_press = false;
    pl->help_page  = 0;

    pl->mode=World.rules->mode;
    pl->status=PLAYING | GRAVITY | DEF_BITS;
    pl->have=DEF_HAVE;
    pl->used=DEF_USED;

    pl->mychar=' ';
    pl->life=World.rules->lives;

    /*
     * If limited lives and if nobody has lost a life yet, you may enter
     * now, otherwise you will have to wait 'til later.
     */
    if (BIT(pl->mode, LIMITED_LIVES)) {
	for (i=0; i<Antall; i++)
	    if (Players[i]->life < 3)
		too_late=true;
	if (too_late) {
	    pl->mychar='W';
	    pl->life=0;
	    pl->status|=GAME_OVER;
	}
    }

    if (BIT(World.rules->mode, TIMING))
	pl->team = 0;

    pl->lock.tagged=LOCK_NONE;
    pl->lock.pl_id=1;
    pl->lock.pos.x=pl->pos.x;
    pl->lock.pos.y=pl->pos.y;
    pl->lock.name=Name_HQ;
}



void Alloc_players(int number)
{
    int i;

    for (i=0; i<number; i++)
	Players[i]=(player *)malloc(sizeof(player));
}



void Free_players(int number)
{
    int i;

    for (i=0; i<number; i++)		    
	free(Players[i]);
}



void Set_label_strings(void)
{
    int i, j, hi_ind;
    double hi, hi_tmp;
    player *pl, *tmp;


    /*
     * Sorts players after score. (selection sort)
     */
    for (i=0; i<Antall; i++) {

	hi = Players[i]->score;
	hi_ind = i;
	for (j=i+1; j<Antall; j++)
	    if (Players[j]->score > hi)
		hi = Players[hi_ind=j]->score;

	if (hi_ind != i) {
	    tmp = Players[hi_ind];
	    Players[hi_ind] = Players[i];
	    Players[i] = tmp;
	    get_ind[Players[i]->id] = i;
	    get_ind[Players[hi_ind]->id] = hi_ind;
	}
    }

    /*
     * Get the person with the best kill-ratio (i.e. score/number of deaths.)
     */
    hi = (double)Players[0]->score/(Players[0]->life+1);
    hi_ind = 0;
    for (i=1; i<Antall; i++)
	if ((hi_tmp = (double)Players[i]->score/(Players[i]->life+1)) > hi) {
	    hi = hi_tmp;
	    hi_ind = i;
	}

    /*
     * Re-formats the labels.
     */
    for (i=0; i<Antall; i++) {
	pl=Players[i];

	sprintf(pl->lblstr, "%c%c %-19s%03d%6d", (hi_ind==i)?'*':pl->mychar,
		(pl->team==0)?' ':pl->team+'0',
		pl->name, pl->life, pl->score);
    }

    Set_labels();
}


/*
IKKE FERDIG
void Reset_all_players(void)
{
    int i;


    for (i=0; i<Antall; i++) {
	printf("Resetting player %s\n", Players[i]->name);
	CLR_BIT(Players[i]->status, GAME_OVER);
	Players[i]->life=World.rules->lives;
	Players[i]->mychar=' ';
    }
    Set_label_strings();
}
Kalles fra Comput_game_status()
*/


void Compute_game_status(void)
{
/*  int i;
    int ant_alive=0, ant_waiting=0;
    bool too_late=false;
    int pl_id;


    for (i=0; i<Antall; i++) {
	if (!BIT(Players[i]->status, GAME_OVER)) {
	    ant_alive++;
	    pl_id=i;
	}
	if (Players[i]->mychar == 'W')
	    ant_waiting++;
    }

Dette er ikke ferdig Ken.  :)

    if (Antall>1 && ant_alive<=1) {
	if (antall_alive == 0)
	    Set_message("Game over! No apparent winners.");


    if ((ant_alive == 1) && (Antall != 1)) {
	sprintf(msg, "%s has won the game!!!!!!! Long live %s!",
		Players[pl_id]->name, Players[pl_id]->name);
	Set_message(msg);
	Players[pl_id]->score += PTS_GAME_WON;
	Reset_all_players();
    } else if (ant_alive == 0) {
	Reset_all_players();
    }	
*/
}



void Delete_player(int indeks)
{
    player *pl;
    int i, id;


    pl=Players[indeks];
    id=pl->id;

    for (i=0; i<Ant_Shots; i++)		/* Delete all remaining shots */
	if (Shots[i]->id == id)
	    Delete_shot(i);

    CLR_BIT(pl->status, IN_USE);	/* This entry isn't in use anymore */

    /*
     * Swap entry no 'indeks' with the last one.
     */
    pl=Players[--Antall];		/* Swap pointers... */
    Players[Antall]=Players[indeks];
    Players[indeks]=pl;

    get_ind[Players[indeks]->id] = indeks;

    for (i=0; i<Antall; i++)
	if ((Players[i]->lock.pl_id == id) || Antall<=1)
	    Players[i]->lock.tagged = LOCK_NONE;
}



void Kill_player(int indeks)
{
    player *pl;


    Explode(indeks);

    pl=Players[indeks];
    pl->vel.x=0; pl->vel.y=0;
    pl->acc.x=0; pl->acc.y=0;
    pl->double_dir=pl->dir=UP;
    pl->turnacc=0.0; pl->turnvel=0.0;
    pl->mass=Ship_Mass;
    pl->status |= WAITING_SHOTS | DEF_BITS;
    pl->status &= ~(KILL_BITS);
    pl->extra_shots=0;
    pl->missiles=0;
    pl->mines=0;
    pl->shot_speed=Shots_Speed;
    pl->shot_max=Shots_Max;
    pl->shot_life=Shots_Life;
    pl->shot_mass=Shots_Mass;
    pl->last_time = pl->time;
    pl->last_lap = 0;
    pl->count = 128;

    pl->fuel*=0.90;				/* Loose 10% of fuel */
    pl->fuel=MAX(pl->fuel, MIN_PLAYER_FUEL+(rand()%(int)MIN_PLAYER_FUEL)/5);

    if (BIT(World.rules->mode, TIMING))
	pl->fuel = RACE_PLAYER_FUEL;

    if (BIT(pl->mode, LIMITED_LIVES))
	pl->life--;
    else
	pl->life++;

    if (pl->life==-1) {
	pl->life=0;
	SET_BIT(pl->status, GAME_OVER);
	pl->mychar='D';
    }
    pl->have=DEF_HAVE;
    pl->used|=DEF_USED;
    pl->used&=~(USED_KILL);
    pl->used&=pl->have;
}

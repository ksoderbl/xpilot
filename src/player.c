/* $Id: player.c,v 1.18 1993/04/02 20:34:53 kenrsc Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
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
    "@(#)$Id: player.c,v 1.18 1993/04/02 20:34:53 kenrsc Exp $";
#endif


static char msg[MSG_LEN];

bool	updateScores = true;


/********** **********
 * Functions on player array.
 */

void Pick_startpos(int ind)
{
    int		i, num_free, pick, seen;
    static int	prev_num_bases = 0;
    static char	*free_bases = NULL;

    if (prev_num_bases != World.NumBases) {
	prev_num_bases = World.NumBases;
	if (free_bases != NULL) {
	    free(free_bases);
	}
	free_bases = (char *) malloc(World.NumBases * sizeof(*free_bases));
	if (free_bases == NULL) {
	    error("Can't allocate memory for free_bases");
	    End_game();
	}
    }
    num_free = 0;
    for (i = 0; i < World.NumBases; i++) {
	if (World.base[i].team == Players[ind]->team) {
	    num_free++;
	    free_bases[i] = 1;
	} else {
	    free_bases[i] = 0;
	}
    }
    for (i = 0; i < NumPlayers; i++) {
	if (i != ind && free_bases[Players[i]->home_base] != 0) {
	    free_bases[Players[i]->home_base] = 0;
	    num_free--;
	}
    }
    pick = rand() % num_free;
    seen = 0;
    for (i = 0; i < World.NumBases; i++) {
	if (free_bases[i] != 0) {
	    if (seen < pick) {
		seen++;
	    } else {
		Players[ind]->home_base = i;
		return;
	    }
	}
    }
    error("Can't pick startpos (ind=%d,num=%d,free=%d,pick=%d,seen=%d)",
	ind, World.NumBases, num_free, pick, seen);
    End_game();
}



void Go_home(int ind)
{
    player	*pl = Players[ind];
    int		x, y;


    x = World.base[pl->home_base].pos.x;
    y = World.base[pl->home_base].pos.y;

    pl->dir = pl->float_dir = World.base[pl->home_base].dir;

    pl->pos.x = x * BLOCK_SZ + BLOCK_SZ/2.0;
    pl->pos.y = y * BLOCK_SZ + BLOCK_SZ/2.0;
/*    pl->pos.y = y * BLOCK_SZ - ships[DIR_UP].pts[1].y;*/
    pl->prevpos = pl->pos;
    pl->acc.x = pl->acc.y = 0.0;
    pl->vel.x = pl->vel.y = pl->velocity = 0.0;

    CLR_BIT(pl->status, THRUSTING);

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
    pl->dir	= pl->float_dir= DIR_UP;
    pl->turnvel		= 0.0;
#ifdef	TURN_FUEL
    pl->oldturnvel	= 0.0;
#endif
    pl->turnacc		= 0.0;
    pl->fuel.num_tanks   = 0;
    pl->fuel.current    = 0;
    pl->fuel.sum        =
    pl->fuel.tank[0]    = ( DEFAULT_PLAYER_FUEL
			   + (((rand()%400)-200) << FUEL_SCALE_BITS) );
    pl->fuel.max        = TANK_CAP(0);
    pl->after_burners   = 0;

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
    pl->sensor_range	= MAX(pl->fuel.sum * ENERGY_RANGE_FACTOR,
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

    {
	static u_short	pseudo_team_no = 0;
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
	    if (Players[i]->life < 0)
		too_late = true;
	if (too_late) {
	    pl->mychar	= 'W';
	    pl->life	= 0;
	    pl->status |= GAME_OVER;
	}
    }

    pl->team = TEAM_NOT_SET;

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
}


static player			*playerArray;
static struct _visibility	*visibilityArray;

void Alloc_players(int number)
{
    player *p;
    struct _visibility *t;
    int i;


    /* Allocate space for pointers */
    Players = (player **)malloc(number * sizeof(player *));

    /* Allocate space for all entries, all player structs */
    p = playerArray = (player *)malloc(number * sizeof(player));

    /* Allocate space for all visibility arrays, n arrays of n entries */
    t = visibilityArray = (struct _visibility *)
	malloc(number * number * sizeof(struct _visibility));

    for (i=0; i<number; i++) {
	Players[i] = p++;
	Players[i]->visibility = t;
	/* Advance to next block/array */
	t += number;
    }
}



void Free_players(void)
{
    free(Players);
    free(playerArray);
    free(visibilityArray);
}



void Update_score_table(void)
{
    int i, j, hi_ind;
    float hi, hi_tmp;
    player *pl, *tmp;


    /*
     * Sorts players after score. (selection sort)
     */
    for (i=0; i<NumPlayers; i++) {

	hi = (tmp=Players[i])->score;
	hi_ind = i;
	for (j=i+1; j<NumPlayers; j++)
	    if ((pl=Players[j])->score > hi) {
		tmp = Players[hi_ind = j];
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
    hi = (float)Players[0]->score / (Players[0]->life+1);
    hi_ind = 0;
    for (i=1; i<NumPlayers; i++)
	if ((hi_tmp=(float)Players[i]->score/(Players[i]->life+1))>hi) {
	    hi = hi_tmp;
	    hi_ind = i;
	}

    /*
     * Re-formats the labels.
     */
    for (i=0; i<NumPlayers; i++) {
	char name[100];

	pl = Players[i];

	if (pl->robot_mode != RM_NOT_ROBOT
	    && pl->robot_lock == LOCK_PLAYER) {
	    sprintf(name, "%s (%s)", pl->name,
		    Players[GetInd[pl->robot_lock_id]]->name);
	} else
	    sprintf(name, "%s", pl->name);

	sprintf(pl->lblstr, "%c%c %-19s%03d%6d",
		(hi_ind == i) ? '*' : pl->mychar,
		(pl->team == TEAM_NOT_SET) ? ' ' : pl->team+'0',
		name, pl->life, pl->score);
    }

    Draw_score_table();
    updateScores = false;
}

void Reset_all_players(void)
{
    int i,j;

    for (i=0; i<NumPlayers; i++) {
	CLR_BIT(Players[i]->status, GAME_OVER);
	CLR_BIT(Players[i]->have, OBJ_BALL);
	Players[i]->life = World.rules->lives;
	if (Players[i]->robot_mode != RM_NOT_ROBOT)
	    Players[i]->mychar = 'R';
	else
	    Players[i]->mychar = ' ';
    }
    if (BIT(World.rules->mode, TEAM_PLAY)) {

	/* Detach any balls and kill ball */
	/* We are starting all over again */
	for(j=0;j < NumObjs ; j++) {
	    if (Obj[j]->type == OBJ_BALL) {
		Obj[j]->id = -1;
		Obj[j]->life = 0;
		Obj[j]->count = 10;
	    }
	}

	/* Reset the treasures */
	for(i=0; i < World.NumTreasures; i++) {
	    Make_ball( -1, World.treasures[i].pos.x 
		      * BLOCK_SZ + (BLOCK_SZ/2),
		      World.treasures[i].pos.y * BLOCK_SZ + 10, false, i);
	    World.treasures[i].have = true;
	    World.treasures[i].count = 0;
	}
    }

    Update_score_table();
}

void Compute_game_status(void)
{
    int i;
    int team[MAX_TEAMS];
    int	teams = 0;
    int num_teams = 0;

    /* Do we have a winning team ? */

    if (BIT(World.rules->mode, TEAM_PLAY)) {
	for(i=0; i < MAX_TEAMS; i++) {
	    team[i] = 0;
	    if (World.teams[i].NumMembers > 0)
		num_teams++;
	}

	for (i=0; i < NumPlayers && teams < 2; i++) {
	    if (!BIT(Players[i]->status, GAME_OVER)
		|| Players[i]->life > 0)
		if (team[Players[i]->team] == 0) {
		    team[Players[i]->team]++;
		    teams++;
		}
	}
	
	if ((teams == 1) && (num_teams > 1)) {
	    for (i=0; team[i] <= 0; i++)
		;
	    sprintf(msg, "Team %d has won the game !", i);
	    Set_message(msg);
	    /* Start up all player's again */
	    Reset_all_players();    
	} else if (teams == 0) {
	    sprintf(msg, "No teams has won !", i);
	    Set_message(msg);
	    /* Start up all player's again */
	    Reset_all_players();    
	}
    } else {	    

    /* Do we have a winner ? (No team play) */
	int num_players = 0;
	int ind;

	for(i=0; i < NumPlayers; i++) 
	    if (!BIT(Players[i]->status, GAME_OVER)
		|| Players[i]->life > 0) {
		num_players++;
		ind = i; 	/* Tag player that's alive */
	    }

	if (num_players == 1) {
	    sprintf(msg, "%s has won the game !", Players[ind]->name);
	    Set_message(msg);
	    /* Start up all player's again */
	    Reset_all_players();    
	} else if (num_players == 0) {
	    sprintf(msg, "We have a draw !");
	    Set_message(msg);
	    /* Start up all player's again */
	    Reset_all_players();    	    
	}
    }
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
    
    if (pl->team != TEAM_NOT_SET)
	World.teams[pl->team].NumMembers--;

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
}



void Kill_player(int ind)
{
    player *pl;
    int i;

    Explode(ind);

    pl			= Players[ind];
    pl->vel.x		= pl->vel.y	= 0.0;
    pl->acc.x		= pl->acc.y	= 0.0;
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
    pl->fuel.num_tanks	= 0;
    pl->fuel.max        = TANK_CAP(0);
    pl->fuel.sum       	*= 0.90;		/* Loose 10% of fuel */
    if (pl->fuel.sum>pl->fuel.max) pl->fuel.sum = pl->fuel.max;
    pl->fuel.tank[0]    =
    pl->fuel.sum        = MAX(pl->fuel.sum,
                              MIN_PLAYER_FUEL+(rand()%(int)MIN_PLAYER_FUEL)/5);
    pl->after_burners	= 0;

    if (BIT(World.rules->mode, TIMING))
	pl->fuel.sum = pl->fuel.tank[0] = RACE_PLAYER_FUEL;

    if (BIT(pl->mode, LIMITED_LIVES))
	pl->life--;
    else
	pl->life++;

    /* Detach ball from player */
    if (BIT(pl->have, OBJ_BALL))
	for(i=0; i<NumObjs; i++) 
	    if (BIT(Obj[i]->type, OBJ_BALL) && Obj[i]->id == pl->id)
		Obj[i]->id = -1;

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


/* $Id: player.c,v 3.50 1994/05/23 19:21:26 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-94 by
 *
 *      Bjørn Stabell        (bjoerns@staff.cs.uit.no)
 *      Ken Ronny Schouten   (kenrsc@stud.cs.uit.no)
 *      Bert Gÿsbers         (bert@mc.bio.uva.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#define SERVER
#include "global.h"
#include "map.h"
#include "score.h"
#include "robot.h"
#include "bit.h"
#include "netserver.h"
#include "saudio.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: player.c,v 3.50 1994/05/23 19:21:26 bert Exp $";
#endif


static char msg[MSG_LEN];

bool	updateScores = true;


/********** **********
 * Functions on player array.
 */

void Pick_startpos(int ind)
{
    player	*pl = Players[ind];
    int		i, num_free, pick, seen;
    static int	prev_num_bases = 0;
    static char	*free_bases = NULL;

    if (pl->robot_mode == RM_OBJECT) {
	pl->home_base = 0;
	return;
    }
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
	if (World.base[i].team == pl->team) {
	    num_free++;
	    free_bases[i] = 1;
	} else {
	    free_bases[i] = 0;
	}
    }
    for (i = 0; i < NumPlayers; i++) {
	if (i != ind
	    && Players[i]->robot_mode != RM_OBJECT
	    && free_bases[Players[i]->home_base] != 0) {
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
		pl->home_base = i;
		if (ind < NumPlayers) {
		    for (i = 0; i < NumPlayers; i++) {
			if (Players[i]->conn != NOT_CONNECTED) {
			    Send_base(Players[i]->conn,
				      pl->id, 
				      pl->home_base);
			}
		    }
		    if (BIT(pl->status, PLAYING) == 0) {
			pl->count = RECOVERY_DELAY;
		    }
		    else if (BIT(pl->status, PAUSE|GAME_OVER)) {
			Go_home(ind);
		    }
		}
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
    int		i, x, y;


    x = World.base[pl->home_base].pos.x;
    y = World.base[pl->home_base].pos.y;

    pl->dir = pl->float_dir = World.base[pl->home_base].dir;

    pl->pos.x = x * BLOCK_SZ + BLOCK_SZ/2.0;
    pl->pos.y = y * BLOCK_SZ + BLOCK_SZ/2.0;
    pl->prevpos = pl->pos;
    pl->acc.x = pl->acc.y = 0.0;
    pl->vel.x = pl->vel.y = pl->velocity = 0.0;
    pl->turnacc = pl->turnvel = 0.0;
    memset(pl->last_keyv, 0, sizeof(pl->last_keyv));
    memset(pl->prev_keyv, 0, sizeof(pl->prev_keyv));
    pl->key_changed = 0;
    CLR_BIT(pl->used, OBJ_CONNECTOR | OBJ_REFUEL | OBJ_REPAIR);
    if (playerStartsShielded != 0) {
	SET_BIT(pl->used, OBJ_SHIELD);
	if (playerShielding == 0) {
	    pl->shield_time = 2 * FPS;
	    SET_BIT(pl->have, OBJ_SHIELD);
	}
    }
    CLR_BIT(pl->status, THRUSTING);
    pl->updateVisibility = 1;
    for (i = 0; i < NumPlayers; i++) {
	pl->visibility[i].lastChange = 0;
	Players[i]->visibility[ind].lastChange = 0;
    }

    if (pl->robot_mode != RM_NOT_ROBOT)
	pl->robot_mode = RM_TAKE_OFF;
}


void Init_player(int ind, wireobj *ship)
{
    player		*pl = Players[ind];
    bool		too_late = false;
    int			i;


    pl->vel.x	= pl->vel.y	= 0.0;
    pl->acc.x	= pl->acc.y	= 0.0;
    pl->dir	= pl->float_dir = DIR_UP;
    pl->turnvel		= 0.0;
#ifdef	TURN_FUEL
    pl->oldturnvel	= 0.0;
#endif
    pl->turnacc		= 0.0;
    pl->mass		= ShipMass;
    pl->emptymass	= ShipMass;
    pl->fuel.num_tanks  = 0;
    pl->fuel.current    = 0;
    pl->fuel.sum        =
    pl->fuel.tank[0]    = ( (initialFuel << FUEL_SCALE_BITS)
			   + (((rand()%400)-200) << FUEL_SCALE_BITS) );
    pl->fuel.max        = TANK_CAP(0);
    for (i = 1; i <= initialTanks; i++) {
	pl->fuel.num_tanks++;
	SET_BIT(pl->have, OBJ_TANK);
	pl->fuel.current = i;
	pl->fuel.max += TANK_CAP(i);
	pl->fuel.tank[i] = 0;
	pl->emptymass += TANK_MASS;
	/* Add_fuel(&pl->fuel, TANK_FUEL(pl->fuel.current)); */
    }
    pl->fuel.current = 0;

    pl->afterburners   = initialAfterburners;
    pl->transporters    = initialTransporters;
    pl->transInfo.count	= 0;

    if (allowShipShapes == true && ship) {
	pl->ship = ship;
    }
    else {
	pl->ship = Default_ship();
    }

    pl->power			= 45.0;
    pl->turnspeed		= 30.0;
    pl->turnresistance		= 0.12;
    pl->power_s			= 35.0;
    pl->turnspeed_s		= 25.0;
    pl->turnresistance_s	= 0.12;

    if (BIT(World.rules->mode, TIMING)) {
	pl->power	= MAX_PLAYER_POWER;
	pl->turnspeed	= 27.0;
    }

    pl->check		= 0;
    pl->round		= 0;
    pl->time		= 0;
    pl->last_lap_time	= 0;
    pl->last_time	= 0;
    pl->last_lap	= 0;
    pl->best_run	= 0;
    pl->best_lap	= 0;
    pl->count		= -1;
    pl->shield_time	= 0;

    pl->type		= OBJ_PLAYER;
    pl->shots		= 0;
    pl->extra_shots	= initialWideangles;
    pl->back_shots	= initialRearshots;
    pl->missiles	= initialMissiles;
    pl->mines		= initialMines;
    pl->cloaks		= initialCloaks;
    pl->sensors		= initialSensors;
    pl->forceVisible	= 0;
    pl->shot_speed	= ShotsSpeed;
    pl->sensor_range	= MAX(pl->fuel.sum * ENERGY_RANGE_FACTOR,
                              VISIBILITY_DISTANCE);
    pl->max_speed	= SPEED_LIMIT - pl->shot_speed;
    pl->shot_max	= ShotsMax;
    pl->shot_life	= ShotsLife;
    pl->shot_mass	= ShotsMass;
    pl->color		= WHITE;
    pl->score		= 0;
    pl->prev_score	= 0;
    pl->fs		= 0;
    pl->repair_target	= 0;
    pl->name[0]		= '\0';
    pl->ecms 		= initialECMs;
    pl->lasers 		= initialLasers;
    pl->num_pulses	= 0;
    pl->max_pulses	= 0;
    pl->pulses		= NULL;
    pl->emergency_thrusts = initialEmergencyThrusts;
    pl->emergency_thrust_left = 0;
    pl->emergency_thrust_max = 0;
    pl->emergency_shields = initialEmergencyShields;
    pl->emergency_shield_left = 0;
    pl->emergency_shield_max = 0;
    pl->autopilots	= initialAutopilots;
    pl->tractor_beams	= initialTractorBeams;
    pl->ecmInfo.count	= 0;
    pl->damaged 	= 0;

    pl->mode		= World.rules->mode;
    pl->status		= PLAYING | GRAVITY | DEF_BITS;
    pl->have		= DEF_HAVE;
    pl->used		= DEF_USED;

    if (pl->cloaks > 0) {
	SET_BIT(pl->have, OBJ_CLOAKING_DEVICE);
    }

    CLEAR_MODS(pl->mods);
    for (i = 0; i < NUM_MODBANKS; i++)
	CLEAR_MODS(pl->modbank[i]);
    for (i = 0; i < LOCKBANK_MAX; i++)
	pl->lockbank[i] = NOT_CONNECTED;

    {
	static u_short	pseudo_team_no = 0;
        pl->pseudo_team = pseudo_team_no++;
    }
    pl->mychar		= ' ';
    pl->prev_mychar	= pl->mychar;
    pl->life		= World.rules->lives;
    pl->prev_life	= pl->life;
    pl->ball 		= NULL;

    /*
     * If limited lives and if nobody has lost a life yet, you may enter
     * now, otherwise you will have to wait 'til everyone gets GAME OVER.
     */
    if (BIT(pl->mode, LIMITED_LIVES)) {
	for (i=0; i<NumPlayers; i++)
	    /* If anybody has lost a life, then it's too late to join */
	    if (Players[i]->life < World.rules->lives)
		too_late = true;
	if (too_late) {
	    pl->mychar	= 'W';
	    pl->prev_life = pl->life = 0;
	    SET_BIT(pl->status, GAME_OVER);
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
    pl->rplay_fd	= -1;
    pl->conn		= NOT_CONNECTED;
    pl->audio		= NULL;
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
    int			i, j;
    player		*pl, *tmp;

    for (j = 0; j < NumPlayers; j++) {
	pl = Players[j];
	if (pl->score != pl->prev_score
	    || pl->life != pl->prev_life
	    || pl->mychar != pl->prev_mychar) {
	    pl->prev_score = pl->score;
	    pl->prev_life = pl->life;
	    pl->prev_mychar = pl->mychar;
	    for (i = 0; i < NumPlayers; i++) {
		tmp = Players[i];
		if (tmp->conn == NOT_CONNECTED) {
		    continue;
		}
		Send_score(tmp->conn, pl->id, pl->score, pl->life, pl->mychar);
	    }
	}
    }
    updateScores = false;
}

void Reset_all_players(void)
{
    int i,j;

    for (i=0; i<NumPlayers; i++) {
	CLR_BIT(Players[i]->status, GAME_OVER);
	CLR_BIT(Players[i]->have, OBJ_BALL);
	if (Players[i]->mychar != 'P') {
	    Players[i]->mychar = ' ';
	    Players[i]->life = World.rules->lives;
	}
	if (Players[i]->robot_mode == RM_OBJECT)
	    Players[i]->mychar = 'T';
	else if (Players[i]->robot_mode != RM_NOT_ROBOT)
	    Players[i]->mychar = 'R';
    }
    if (BIT(World.rules->mode, TEAM_PLAY)) {

	/* Detach any balls and kill ball */
	/* We are starting all over again */
	for (j = NumObjs - 1; j >= 0 ; j--) {
	    if (BIT(Obj[j]->type, OBJ_BALL)) {
		Obj[j]->id = -1;
		Obj[j]->life = 0;
		Obj[j]->owner = 0;
		CLR_BIT(Obj[j]->status, RECREATE);
		Delete_shot(j);
	    }
	}

	/* Reset the treasures */
	for (i = 0; i < World.NumTreasures; i++) {
	    World.treasures[i].destroyed = 0;
	    World.treasures[i].have = false;
	    Make_treasure_ball(i);
	}

	/* Reset the teams */
	for (i = 0; i < MAX_TEAMS; i++) {
	    World.teams[i].TreasuresDestroyed = 0;
	    World.teams[i].TreasuresLeft = World.teams[i].NumTreasures;
	}
    }

    Update_score_table();
}

void Check_team_members(int team)
{
    player *pl;
    int members, i;

    if (! BIT(World.rules->mode, TEAM_PLAY))
	return;

    for (members = i = 0; i < NumPlayers; i++) {
	pl = Players[i];
	if (pl->team != TEAM_NOT_SET && pl->robot_mode != RM_OBJECT
	    && pl->team == team)
	    members++;
    }
    if (World.teams[team].NumMembers != members) {
	error ("Server has reset team %d members from %d to %d",
	       team, World.teams[team].NumMembers, members);
	for (i = 0; i < NumPlayers; i++) {
	    pl = Players[i];
	    if (pl->team != TEAM_NOT_SET && pl->robot_mode != RM_OBJECT
		&& pl->team == team)
		error ("Team %d currently has player %d: \"%s\"",
		       team, i+1, pl->name);
	}
	World.teams[team].NumMembers = members;
    }
}

void
Check_team_treasures(int team)
{
    int 	i, j, ownerind, idind;
    treasure_t	*t;
    object	*obj;
    
    if (! BIT(World.rules->mode, TEAM_PLAY))
	return;

    printf ("Team %d: Members=%d (of %d) Treasures=%d Destroyed=%d Left=%d\n",
	    team,
	    World.teams[team].NumMembers,
	    World.teams[team].NumBases,
	    World.teams[team].NumTreasures,
	    World.teams[team].TreasuresDestroyed,
	    World.teams[team].TreasuresLeft);

    for (i = 0; i < World.NumTreasures; i++) {
	t = &(World.treasures[i]);

	if (t->team != team)
	    continue;

	printf ("  Treasure %d: team=%d, have=%d, destroyed=%d\n",
		i, t->team, t->have, t->destroyed);

	for (j = 0; j < NumObjs; j++) {
	    obj = Obj[j];

	    if (! BIT(obj->type, OBJ_BALL)
		|| obj->treasure != i)
		continue;
	    ownerind = (obj->owner == -1 ? -1 : GetInd[obj->owner]);
	    idind = (obj->id == -1 ? -1 : GetInd[obj->id]);
	    printf ("    Ball Object %d: pos=(%6.2f, %6.2f), life=%d, "
		    "recreate=%d,\n",
		    j, obj->pos.x, obj->pos.y, obj->life,
		    BIT(obj->status, RECREATE));
	    printf ("\t\tid=%d (Player %d: \"%s\"),\n",
		    obj->id,
		    (idind == -1 ? -1 : idind+1),
		    (idind == -1 ? "" : Players[idind]->name));
	    printf ("\t\towner=%d (Player %d: \"%s\"),\n",
		    obj->owner,
		    (ownerind == -1 ? -1 : ownerind+1),
		    (ownerind == -1 ? "" : Players[ownerind]->name));
	}
    }
    fflush (stdout);
}

void Team_game_over(int winning_team, char *reason)
{
    int	i;

    if (winning_team != -1) {
	sprintf(msg, " < Team %d has won the game%s! >", winning_team,
		reason);
	Set_message(msg);
	sound_play_all(TEAM_WIN_SOUND);
	for (i = 0; i < NumPlayers; i++) {
	    if (BIT(Players[i]->status, PAUSE)) {
		/* Paused players don't get any points. */
		continue;
	    }
	    else if (Players[i]->robot_mode == RM_OBJECT) {
		/* Ignore tanks. */
		continue;
	    }
	    else if (Players[i]->team == winning_team) {
		SCORE(i, PTS_GAME_WON, 
		      (int) Players[i]->pos.x/ BLOCK_SZ,
		      (int) Players[i]->pos.y/BLOCK_SZ, "Winner");
	    }
	}
	/* Start up all player's again */
	Reset_all_players();
    } else {
	sprintf(msg, " < We have a draw%s! >", reason);
	Set_message(msg);
	sound_play_all(TEAM_DRAW_SOUND);
	/* Start up all player's again */
	Reset_all_players();    
    }
}

void Compute_game_status(void)
{
    int i;

    if (BIT(World.rules->mode, TEAM_PLAY)) {

	/* Do we have a winning team ? */

	enum TeamState {
	    TeamEmpty,
	    TeamDead,
	    TeamAlive
	}	team_state[MAX_TEAMS];
	int	num_dead_teams = 0;
	int	num_alive_teams = 0;
	int	winning_team = -1;

	for (i = 0; i < MAX_TEAMS; i++) {
	    team_state[i] = TeamEmpty;
	}

	for (i = 0; i < NumPlayers; i++) {
	    if (Players[i]->robot_mode == RM_OBJECT) {
		/* Ignore tanks. */
		continue;
	    }
	    else if (BIT(Players[i]->status, PAUSE)) {
		/* Ignore paused players. */
		continue;
	    }
#if 0
	    /* not all teammode maps have treasures. */
	    else if (World.teams[Players[i]->team].NumTreasures == 0) {
		/* Ingore players with no treasures troves */
		continue;
	    }
#endif
	    else if (BIT(Players[i]->status, GAME_OVER)) {
		if (team_state[Players[i]->team] == TeamEmpty) {
		    /* Assume all teammembers are dead. */
		    num_dead_teams++;
		    team_state[Players[i]->team] = TeamDead;
		}
	    }
	    /*
	     * If the player is not paused and he is not in the
	     * game over mode and his team own treasures then he is
	     * considered alive.
	     * But he may not be playing though if the rest of the team
	     * was genocided very quickly after game reset, while this
	     * player was still being transported back to his homebase.
	     */
	    else if (team_state[Players[i]->team] != TeamAlive) {
		if (team_state[Players[i]->team] == TeamDead) {
		    /* Oops!  Not all teammembers are dead yet. */
		    num_dead_teams--;
		}
		team_state[Players[i]->team] = TeamAlive;
		++num_alive_teams;
		/* Remember a team which was alive. */
		winning_team = Players[i]->team;
	    }
	}

	if (num_alive_teams > 1) {
	    char	buf[MSG_LEN], *bp;
	    int		teams_with_treasure = 0;
	    int		team_win[MAX_TEAMS];
	    int		team_score[MAX_TEAMS];
	    int		winners;
	    int		max_destroyed = 0;
	    int		max_left = 0;
	    int		max_score = 0;

	    /*
	     * Game is not over if more than one team which have treasures
	     * still have one remaining in play.  Note that it is possible
	     * for max_destroyed to be zero, in the case where a team
	     * destroys some treasures and then all quit, and the remaining
	     * teams did not destroy any.
	     */
	    for (i = 0; i < MAX_TEAMS; i++) {
		team_score[i] = 0;
		if (team_state[i] != TeamAlive) {
		    team_win[i] = 0;
		    continue;
		}
		team_win[i] = 1;
		if (World.teams[i].TreasuresDestroyed > max_destroyed)
		    max_destroyed = World.teams[i].TreasuresDestroyed;
		if (World.teams[i].TreasuresLeft)
		    teams_with_treasure++;
	    }

	    /*
	     * Game is not over if more than one team has treasure.
	     */
	    if (teams_with_treasure > 1 /*|| !max_destroyed*/)
		return;

	    /*
	     * Find the winning team;
	     *	Team destroying most number of treasures;
	     *	If drawn; the one with most saved treasures,
	     *	If drawn; the team with the most points,
	     *	If drawn; an overall draw.
	     */
	    for (winners = i = 0; i < MAX_TEAMS; i++) {
		if (!team_win[i])
		    continue;
		if (World.teams[i].TreasuresDestroyed == max_destroyed) {
		    if (World.teams[i].TreasuresLeft > max_left)
			max_left = World.teams[i].TreasuresLeft;
		    winning_team = i;
		    winners++;
		} else {
		    team_win[i] = 0;
		}
	    }
	    if (winners == 1) {
		sprintf(buf, " by destroying %d treasures", max_destroyed);
		Team_game_over(winning_team, buf);
		return;
	    }

	    for (i = 0; i < NumPlayers; i++)
		team_score[Players[i]->team] += Players[i]->score;

	    for (winners = i = 0; i < MAX_TEAMS; i++) {
		if (!team_win[i])
		    continue;
		if (World.teams[i].TreasuresLeft == max_left) {
		    if (team_score[i] > max_score)
			max_score = team_score[i];
		    winning_team = i;
		    winners++;
		} else {
		    team_win[i] = 0;
		}
	    }
	    if (winners == 1) {
		sprintf(buf, " by destroying %d treasures and saving %d",
			max_destroyed, max_left);
		Team_game_over(winning_team, buf);
		return;
	    }

	    for (winners = i = 0; i < MAX_TEAMS; i++) {
		if (!team_win[i])
		    continue;
		if (team_score[i] == max_score) {
		    winning_team = i;
		    winners++;
		} else {
		    team_win[i] = 0;
		}
	    }
	    if (winners == 1) {
		sprintf(buf, " by destroying %d treasures, saving %d, and "
			"scoring %d points",
			max_destroyed, max_left, max_score);
		Team_game_over(winning_team, buf);
		return;
	    }

	    /* Highly unlikely */

	    sprintf(buf, " between teams ");
	    bp = buf + strlen(buf);
	    for (i = 0; i < MAX_TEAMS; i++) {
		if (!team_win[i])
		    continue;
		*bp++ = "0123456789"[i]; *bp++ = ','; *bp++ = ' ';
	    }
	    bp -= 2;
	    *bp = '\0';
	    Team_game_over(-1, buf);

	}
	else if (num_dead_teams > 0) {
	    if (num_alive_teams == 1)
		Team_game_over(winning_team, " by staying alive");
	    else
		Team_game_over(-1, " as everyone died");
	}
	else {
	    /*
	     * num_alive_teams <= 1 && num_dead_teams == 0
	     * 
	     * There is a possibility that the game has ended because players
	     * quit, the game over state is needed to reset treasures.  We
	     * must count how many treasures are missing, if there are any
	     * the playing team (if any) wins.
	     */
	    int	i, treasures_destroyed;

	    for (treasures_destroyed = i = 0; i < MAX_TEAMS; i++)
		treasures_destroyed += (World.teams[i].NumTreasures
					- World.teams[i].TreasuresLeft);
	    if (treasures_destroyed)
		Team_game_over(winning_team, " by staying in the game");
	}

    } else {

    /* Do we have a winner ? (No team play) */
	int num_alive_players = 0;
	int num_active_players = 0;
	int num_alive_robots = 0;
	int num_active_humans = 0;
	int winner = -1;

	for (i=0; i < NumPlayers; i++)  {
	    if (!BIT(Players[i]->status, PAUSE)) {
		if (!BIT(Players[i]->status, GAME_OVER)) {
		    num_alive_players++;
		    if (Players[i]->robot_mode != RM_NOT_ROBOT
			&& Players[i]->robot_mode != RM_OBJECT) {
			num_alive_robots++;
		    }
		    winner = i; 	/* Tag player that's alive */
		}
		else if (Players[i]->robot_mode == RM_NOT_ROBOT) {
		    num_active_humans++;
		}
		num_active_players++;
	    }
	}

	if (num_alive_players == 1 && num_active_players > 1) {
	    sprintf(msg, "%s has won the game!", Players[winner]->name);
	    Set_message(msg);
	    SCORE(winner, PTS_GAME_WON,
		  (int) Players[winner]->pos.x/ BLOCK_SZ,
		  (int) Players[winner]->pos.y/BLOCK_SZ, "Winner");
	    sound_play_all(PLAYER_WIN_SOUND);
	    /* Start up all player's again */
	    Reset_all_players();    
	}
	else if (num_alive_players == 0 && num_active_players >= 1) {
	    sprintf(msg, "We have a draw!");
	    Set_message(msg);
	    sound_play_all(PLAYER_DRAW_SOUND);
	    /* Start up all player's again */
	    Reset_all_players();    	    
	}
	else if (num_alive_robots > 1
	    && num_alive_players == num_alive_robots
	    && num_active_humans > 0) {
	    Set_message("The robots have won the game!");
	    for (i = 0; i < NumPlayers; i++)  {
		if (!BIT(Players[i]->status, PAUSE|GAME_OVER)) {
		    Players[i]->score += PTS_GAME_WON;
		}
	    }
	    /* Should this be a different sound? */
	    sound_play_all(PLAYER_WIN_SOUND);
	    /* Start up all player's again */
	    Reset_all_players();    
	}
    }
}

void Delete_player(int ind)
{
    player *pl;
    int i, j, id;


    pl = Players[ind];
    id = pl->id;

    for (i=0; i<NumObjs; i++) {	/* Delete remaining shots, but leave mines */
	if (Obj[i]->type==OBJ_MINE) {
	    if (Obj[i]->id == id)
		Obj[i]->id = -1;
	    if (Obj[i]->owner == id)
		Obj[i]->owner = -1;
	}
	else {
	    if (Obj[i]->id == id)
		Delete_shot(i);
	}
    }

    if (pl->max_pulses > 0 && pl->pulses != NULL) {
	free(pl->pulses);
	pl->max_pulses = 0;
	pl->num_pulses = 0;
	pl->pulses = NULL;
    }
    Free_ship_shape(pl->ship);

#ifdef SOUND
    sound_close(pl);
#endif /* SOUND */
    NumPlayers--;
    
    if (pl->team != TEAM_NOT_SET && pl->robot_mode != RM_OBJECT)
	World.teams[pl->team].NumMembers--;

    if (pl->robot_mode != RM_NOT_ROBOT && pl->robot_mode != RM_OBJECT)
	NumRobots--;

    /*
     * Swap entry no 'ind' with the last one.
     */
    pl			= Players[NumPlayers];	/* Swap pointers... */
    Players[NumPlayers]	= Players[ind];
    Players[ind]	= pl;

    Check_team_members (Players[ind]->team);

    GetInd[Players[ind]->id] = ind;

    for (i=0; i<NumPlayers; i++) {
	if (BIT(Players[i]->lock.tagged, LOCK_PLAYER)
	    && (Players[i]->lock.pl_id == id || NumPlayers <= 1))
	    CLR_BIT(Players[i]->lock.tagged, LOCK_PLAYER);
	if (Players[i]->robot_mode != RM_NOT_ROBOT
	    && BIT(Players[i]->robot_lock, LOCK_PLAYER)
	    && Players[i]->robot_lock_id == id) {
	    CLR_BIT(Players[i]->robot_lock, LOCK_PLAYER);
	}
	for (j = 0; j < LOCKBANK_MAX; j++) {
	    if (Players[i]->lockbank[j] == id)
		Players[i]->lockbank[j] = NOT_CONNECTED;
	}
    }

    for (i = 0; i < NumPlayers; i++) {
	if (Players[i]->conn != NOT_CONNECTED) {
	    Send_leave(Players[i]->conn, id);
	}
    }

}

void Detach_ball(int ind, int obj)
{
    int			i, cnt;

    if (obj == -1 || Obj[obj] == Players[ind]->ball) {
	Players[ind]->ball = NULL;
	CLR_BIT(Players[ind]->used, OBJ_CONNECTOR);
    }

    if (BIT(Players[ind]->have, OBJ_BALL)) {
	for (cnt = i = 0; i < NumObjs; i++) {
	    if (BIT(Obj[i]->type, OBJ_BALL) && Obj[i]->id == Players[ind]->id){
		if (obj == -1 || obj == i) {
		    Obj[i]->id = -1;
		    /* Don't reset owner so you can throw balls */
		} else {
		    cnt++;
		}
	    }
	}
	if (cnt == 0)
	    CLR_BIT(Players[ind]->have, OBJ_BALL);
    }
}

void Kill_player(int ind)
{
    player	 *pl;
    long	minfuel;
    int		i;

    Explode(ind);

    pl			= Players[ind];

    Detach_ball(ind, -1);
    if (BIT(pl->used, OBJ_AUTOPILOT) || BIT(pl->status, HOVERPAUSE)) {
	CLR_BIT(pl->status, HOVERPAUSE);
	Autopilot (ind, 0);
    }

    pl->vel.x		= pl->vel.y	= 0.0;
    pl->acc.x		= pl->acc.y	= 0.0;
    pl->emptymass	= pl->mass	= ShipMass;
    pl->status		|= DEF_BITS;
    pl->status		&= ~(KILL_BITS);
    pl->extra_shots	= initialWideangles;
    pl->back_shots	= initialRearshots;
    pl->missiles	= initialMissiles;
    pl->mines		= initialMines;
    pl->cloaks		= initialCloaks;
    pl->sensors		= initialSensors;
    pl->forceVisible	= 0;
    pl->shot_speed	= ShotsSpeed;
    pl->shot_max	= ShotsMax;
    pl->shot_life	= ShotsLife;
    pl->shot_mass	= ShotsMass;
    pl->last_time	= pl->time;
    pl->last_lap	= 0;
    pl->count		= RECOVERY_DELAY;
    pl->ecms 		= initialECMs;
    pl->ecmInfo.count	= 0;
    pl->lasers 		= initialLasers;
    pl->emergency_thrusts = initialEmergencyThrusts;
    pl->emergency_thrust_left = 0;
    pl->emergency_thrust_max = 0;
    pl->emergency_shields = initialEmergencyShields;
    pl->emergency_shield_left = 0;
    pl->emergency_shield_max = 0;
    pl->tractor_beams	= initialTractorBeams;
    pl->autopilots	= initialAutopilots;
    pl->damaged 	= 0;
    pl->lock.distance	= 0;

    pl->fuel.current    = 0;
    pl->fuel.num_tanks	= 0;
    pl->fuel.max        = TANK_CAP(0);
    pl->fuel.sum       	*= 0.90;		/* Loose 10% of fuel */
    minfuel		= (playerMinimumStartFuel * FUEL_SCALE_FACT);
    minfuel		+= (rand() % (1 + minfuel) / 5);
    pl->fuel.sum	= MAX(pl->fuel.sum, minfuel);
    if (pl->fuel.sum > pl->fuel.max) pl->fuel.sum = pl->fuel.max;
    pl->fuel.tank[0]    = pl->fuel.sum;
    for (i = 1; i <= initialTanks; i++) {
	pl->fuel.num_tanks++;
	SET_BIT(pl->have, OBJ_TANK);
	pl->fuel.current = i;
	pl->fuel.max += TANK_CAP(i);
	pl->fuel.tank[i] = 0;
	pl->emptymass += TANK_MASS;
	/* Add_fuel(&pl->fuel, TANK_FUEL(pl->fuel.current)); */
    }
    pl->fuel.current    = 0;

    pl->afterburners	= initialAfterburners;
    pl->transporters    = initialTransporters;
    pl->transInfo.count	= 0;

    if (pl->max_pulses > 0 && pl->num_pulses == 0) {
	free(pl->pulses);
	pl->pulses = NULL;
	pl->max_pulses = 0;
    }

    if (BIT(World.rules->mode, TIMING))
	pl->fuel.sum = pl->fuel.tank[0] = RACE_PLAYER_FUEL;

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
 
    if (pl->cloaks > 0) {
	SET_BIT(pl->have, OBJ_CLOAKING_DEVICE);
    }
}

/* $Id: player.c,v 3.73 1994/09/21 11:01:44 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-94 by
 *
 *      Bj�rn Stabell        (bjoerns@staff.cs.uit.no)
 *      Ken Ronny Schouten   (kenrsc@stud.cs.uit.no)
 *      Bert G�sbers         (bert@mc.bio.uva.nl)
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
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "score.h"
#include "robot.h"
#include "bit.h"
#include "netserver.h"
#include "saudio.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: player.c,v 3.73 1994/09/21 11:01:44 bert Exp $";
#endif

extern int Rate(int winner, int loser);

bool		updateScores = true;


/********* **********
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
    player		*pl = Players[ind];
    int			i, x, y, dir, check;
    float		vx, vy, velo;

    if (BIT(World.rules->mode, TIMING)
	&& pl->round
	&& !BIT(pl->status, GAME_OVER)) {
	if (pl->check)
	    check = pl->check - 1;
	else
	    check = World.NumChecks - 1;
	x = World.check[check].x;
	y = World.check[check].y;
	vx = (rfrac() - 0.5) * 0.1;
	vy = (rfrac() - 0.5) * 0.1;
	velo = LENGTH(vx, vy);
#if 0
	dir = Wrap_findDir(World.check[pl->check].x - x,
			   World.check[pl->check].y - y);
	dir = MOD2(dir + (int)((rfrac() - 0.5) * (RES / 4)), RES);
#else
	dir = pl->last_check_dir;
	dir = MOD2(dir + (int)((rfrac() - 0.5) * (RES / 8)), RES);
#endif
    } else {
	x = World.base[pl->home_base].pos.x;
	y = World.base[pl->home_base].pos.y;
	dir = World.base[pl->home_base].dir;
	vx = vy = velo = 0;
    }

    pl->dir = pl->float_dir = dir;
    pl->pos.x = x * BLOCK_SZ + BLOCK_SZ/2.0;
    pl->pos.y = y * BLOCK_SZ + BLOCK_SZ/2.0;
    pl->prevpos = pl->pos;
    pl->pos.x += vx;
    pl->pos.y += vy;
    pl->vel.x = vx;
    pl->vel.y = vy;
    pl->velocity = velo;
    pl->acc.x = pl->acc.y = 0.0;
    pl->turnacc = pl->turnvel = 0.0;
    memset(pl->last_keyv, 0, sizeof(pl->last_keyv));
    memset(pl->prev_keyv, 0, sizeof(pl->prev_keyv));
    pl->key_changed = 0;
    pl->used &= ~USED_KILL;
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

/*
 * Compute the current sensor range for player `pl'.  This is based on the
 * amount of fuel, the number of sensor items (each one adds 25%), and the
 * minimum and maximum visibility limits in effect.
 */
void Compute_sensor_range(player *pl)
{
    static int		init = 0;
    static float	EnergyRangeFactor;

    if (!init) {
	if (minVisibilityDistance <= 0.0)
	    minVisibilityDistance = VISIBILITY_DISTANCE;
	else
	    minVisibilityDistance *= BLOCK_SZ;
	if (maxVisibilityDistance <= 0.0)
	    maxVisibilityDistance = World.hypotenuse;
	else
	    maxVisibilityDistance *= BLOCK_SZ;

	if (initialFuel > 0.0) {
	    EnergyRangeFactor = minVisibilityDistance /
		(initialFuel * (1.0 + ((float)initialSensors * 0.25)));
	    EnergyRangeFactor /= FUEL_SCALE_FACT;
	} else {
	    EnergyRangeFactor = ENERGY_RANGE_FACTOR;
	}
	init = 1;
    }

    pl->sensor_range = pl->fuel.sum * EnergyRangeFactor;
    pl->sensor_range *= (1.0 + ((float)pl->sensors * 0.25));
    if (pl->sensor_range < minVisibilityDistance)
	pl->sensor_range = minVisibilityDistance;
    if (pl->sensor_range > maxVisibilityDistance)
	pl->sensor_range = maxVisibilityDistance;
}

/*
 * Give player the specified number of tanks and amount of fuel. Upto the
 * maximum allowed.
 */
static void Fuel_player(int ind, int tanks, long fuel)
{
    player		*pl = Players[ind];
    long		i, in, max;

    pl->fuel.num_tanks  = 0;
    pl->fuel.current    = 0;
    pl->fuel.sum	= 0;
    pl->fuel.max	= 0;

    for(i = 0; i <= initialTanks; i++) {
	if (i) {
	    pl->fuel.num_tanks++;
	    pl->emptymass += TANK_MASS;
	}
	pl->fuel.current = i;

	max = TANK_CAP(i);
	in = MIN(fuel, max);
	pl->fuel.max += max;
	pl->fuel.sum += in;
	pl->fuel.tank[i] = in;
	fuel -= in;
    }
    pl->fuel.current = 0;
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

    pl->fuel.sum        = initialFuel << FUEL_SCALE_BITS;
    Fuel_player(ind, initialTanks, pl->fuel.sum);

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

    pl->check		= 0;
    pl->round		= 0;
    pl->time		= 0;
    pl->last_lap_time	= 0;
    pl->last_lap	= 0;
    pl->best_lap	= 0;
    pl->count		= -1;
    pl->shield_time	= 0;

    pl->type		= OBJ_PLAYER;
    pl->shots		= 0;
    pl->missile_rack	= 0;
    pl->extra_shots	= initialWideangles;
    pl->back_shots	= initialRearshots;
    pl->missiles	= initialMissiles;
    pl->mines		= initialMines;
    pl->cloaks		= initialCloaks;
    pl->sensors		= initialSensors;
    pl->forceVisible	= 0;
    pl->shot_speed	= ShotsSpeed;
    Compute_sensor_range(pl);
    pl->max_speed	= SPEED_LIMIT - pl->shot_speed;
    pl->shot_max	= ShotsMax;
    pl->shot_life	= ShotsLife;
    pl->shot_mass	= ShotsMass;
    pl->color		= WHITE;
    pl->score		= 0;
    pl->prev_score	= 0;
    pl->prev_check	= 0;
    pl->prev_round	= 0;
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

    pl->kills		= 0;
    pl->deaths		= 0;

    /*
     * If limited lives and if nobody has lost a life yet, you may enter
     * now, otherwise you will have to wait 'til everyone gets GAME OVER.
     */
    if (BIT(pl->mode, LIMITED_LIVES)) {
	for (i = 0; i < NumPlayers; i++) {
	    /* If a non-team member has lost a life,
	     * then it's too late to join. */
	    if (Players[i]->life < World.rules->lives && !TEAM(ind, i)) {
		too_late = true;
		break;
	    }
	}
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
    pl->conn		= NOT_CONNECTED;
    pl->audio		= NULL;

    pl->shove_next = 0;
    for (i = 0; i < MAX_RECORDED_SHOVES; i++) {
	pl->shove_record[i].pusher_id = -1;
    }
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
    int			i, j, check;
    player		*pl;

    for (j = 0; j < NumPlayers; j++) {
	pl = Players[j];
	if (pl->score != pl->prev_score
	    || pl->life != pl->prev_life
	    || pl->mychar != pl->prev_mychar) {
	    pl->prev_score = pl->score;
	    pl->prev_life = pl->life;
	    pl->prev_mychar = pl->mychar;
	    for (i = 0; i < NumPlayers; i++) {
		if (Players[i]->conn != NOT_CONNECTED) {
		    Send_score(Players[i]->conn, pl->id,
			       pl->score, pl->life, pl->mychar);
		}
	    }
	}
	if (BIT(World.rules->mode, TIMING)) {
	    if (pl->check != pl->prev_check
		|| pl->round != pl->prev_round) {
		pl->prev_check = pl->check;
		pl->prev_round = pl->round;
		check = (pl->round == 0)
			    ? 0
			    : (pl->check == 0)
				? (World.NumChecks - 1)
				: (pl->check - 1);
		for (i = 0; i < NumPlayers; i++) {
		    if (Players[i]->conn != NOT_CONNECTED) {
			Send_timing(Players[i]->conn, pl->id, check, pl->round);
		    }
		}
	    }
	}
    }
    updateScores = false;
}


static void Reset_all_players(void)
{
    player		*pl;
    int			i, j;

    updateScores = true;

    for (i = 0; i < NumPlayers; i++) {
	pl = Players[i];
	if (endOfRoundReset) {
	    if (!BIT(pl->status, PAUSE)) {
		Kill_player(i);
		if (pl != Players[i]) {
		    continue;
		}
	    }
	}
	CLR_BIT(pl->status, GAME_OVER);
	CLR_BIT(pl->have, OBJ_BALL);
	pl->kills = 0;
	pl->deaths = 0;
	pl->round = 0;
	pl->check = 0;
	pl->time = 0;
	pl->best_lap = 0;
	pl->last_lap = 0;
	pl->last_lap_time = 0;
	if (!BIT(pl->status, PAUSE)) {
	    pl->mychar = ' ';
	    pl->life = World.rules->lives;
	    if (BIT(World.rules->mode, TIMING)) {
		pl->count = RECOVERY_DELAY;
	    }
	}
	if (pl->robot_mode == RM_OBJECT)
	    pl->mychar = 'T';
	else if (pl->robot_mode != RM_NOT_ROBOT)
	    pl->mychar = 'R';
    }
    if (BIT(World.rules->mode, TEAM_PLAY)) {

	/* Detach any balls and kill ball */
	/* We are starting all over again */
	for (j = NumObjs - 1; j >= 0 ; j--) {
	    if (BIT(Obj[j]->type, OBJ_BALL)) {
		Obj[j]->id = -1;
		Obj[j]->life = 0;
		Obj[j]->owner = 0;	/* why not -1 ??? */
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

	if (endOfRoundReset) {
	    /* Reset the targets */
	    for (i = 0; i < World.NumTargets; i++) {
		if (World.targets[i].damage != TARGET_DAMAGE
		    || World.targets[i].dead_time != 0) {
		    World.block[World.targets[i].pos.x][World.targets[i].pos.y]
			= TARGET;
		    World.targets[i].dead_time = 0;
		    World.targets[i].damage = TARGET_DAMAGE;
		    World.targets[i].conn_mask = 0;
		    World.targets[i].update_mask = (unsigned)-1;
		    World.targets[i].last_change = loops;
		}
	    }
	}
    }

    if (endOfRoundReset) {
	for (i = 0; i < NumObjs; i++) {
	    object *obj = Obj[i];
	    if (BIT(obj->type, OBJ_SHOT|OBJ_MINE|OBJ_DEBRIS|OBJ_SPARK
			       |OBJ_TORPEDO|OBJ_SMART_SHOT|OBJ_HEAT_SHOT)) {
		obj->life = 0;
		if (BIT(obj->type, OBJ_TORPEDO|OBJ_SMART_SHOT|OBJ_HEAT_SHOT
				   |OBJ_MINE)) {
		    /* Take care that no new explosions are made. */
		    obj->mass = 0;
		}
	    }
	}
    }

    Update_score_table();
}

void Check_team_members(int team)
{
    player		*pl;
    int			members, i;

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


void Check_team_treasures(int team)
{
    int 		i, j, ownerind, idind;
    treasure_t		*t;
    object		*obj;

    if (! BIT(World.rules->mode, TEAM_PLAY))
	return;

    for (i = 0; i < World.NumTreasures; i++) {
	t = &(World.treasures[i]);

	if (t->team != team)
	    continue;

	for (j = 0; j < NumObjs; j++) {
	    obj = Obj[j];

	    if (! BIT(obj->type, OBJ_BALL)
		|| obj->treasure != i)
		continue;
	    ownerind = (obj->owner == -1 ? -1 : GetInd[obj->owner]);
	    idind = (obj->id == -1 ? -1 : GetInd[obj->id]);
	}
    }
}


static void Compute_end_of_round_values(int *average_score,
					int *num_best_players,
					float *best_ratio,
					int best_players[])
{
    int			i;
    float		ratio;

    /* Initialize everything */
    *average_score = 0;
    *num_best_players = 0;
    *best_ratio = -1;

    /* Figure out what the average score is and who has the best kill/death */
    /* ratio for this round */
    for (i = 0; i < NumPlayers; i++) {
	if (Players[i]->robot_mode == RM_OBJECT
	    || (BIT(Players[i]->status, PAUSE)
	       && Players[i]->count <= 0)) {
	    continue;
	}
	*average_score += Players[i]->score;
	ratio = (float) Players[i]->kills / (Players[i]->deaths + 1);
	if (ratio > *best_ratio) {
	    *best_ratio = ratio;
	    best_players[0] = i;
	    *num_best_players = 1;
	}
	else if (ratio == *best_ratio) {
	    best_players[(*num_best_players)++] = i;
	}
    }
    *average_score /= NumPlayers;
}


static void Give_best_player_bonus(int average_score,
				   int num_best_players,
				   float best_ratio,
				   int best_players[])
{
    int			i;
    int			points;
    char		msg[MSG_LEN];


    if (best_ratio == 0) {
	sprintf(msg, "There is no Deadly Player");
    }
    else if (num_best_players == 1) {
	player *bp = Players[best_players[0]];

	sprintf(msg,
		"%s is the Deadliest Player with a kill ratio of %d/%d.",
		bp->name,
		bp->kills, bp->deaths);
	points = (int) (best_ratio * Rate(bp->score, average_score));
	SCORE(best_players[0], points,
	      (int) bp->pos.x/BLOCK_SZ,
	      (int) bp->pos.y/BLOCK_SZ,
	      "[Deadliest]");
    }
    else {
	msg[0] = '\0';
	for (i = 0; i < num_best_players; i++) {
	    player	*bp = Players[best_players[i]];
	    int		ratio = Rate(bp->score, average_score);
	    float	score = (float) (ratio + num_best_players)
				/ num_best_players;

	    if (msg[0]) {
		if (i == num_best_players - 1)
		    strcat(msg, " and ");
		else
		    strcat(msg, ", ");
	    }
	    if (strlen(msg) + 8 + strlen(bp->name) >= sizeof(msg)) {
		Set_message(msg);
		msg[0] = '\0';
	    }
	    strcat(msg, bp->name);
	    points = (int) (best_ratio * score);
	    SCORE(best_players[i], points,
		  (int) bp->pos.x/BLOCK_SZ,
		  (int) bp->pos.y/BLOCK_SZ,
		  "[Deadly]");
	}
	if (strlen(msg) + 64 >= sizeof(msg)) {
	    Set_message(msg);
	    msg[0] = '\0';
	}
	sprintf(msg + strlen(msg),
		" are the Deadly Players with kill ratios of %d/%d.",
		Players[best_players[0]]->kills,
		Players[best_players[0]]->deaths);
    }
    Set_message(msg);
}

static void Give_individual_bonus(int ind, int average_score)
{
    float		ratio;
    int			points;

    ratio = (float) Players[ind]->kills / (Players[ind]->deaths + 1);
    points = (int) (ratio * Rate(Players[ind]->score, average_score));
    SCORE(ind, points,
	  (int) Players[ind]->pos.x/BLOCK_SZ,
	  (int) Players[ind]->pos.y/BLOCK_SZ,
	  "[Winner]");
}

static void Team_game_over(int winning_team, char *reason)
{
    int			i, j;
    int			average_score;
    int			num_best_players;
    int			*best_players;
    float		best_ratio;
    char		msg[MSG_LEN];

    if (!(best_players = (int *)malloc(NumPlayers * sizeof(int)))) {
	error("no mem");
	End_game();
    }

    /* Figure out the average score and who has the best kill/death ratio */
    /* ratio for this round */
    Compute_end_of_round_values(&average_score,
				&num_best_players,
				&best_ratio,
				best_players);

    /* Print out the results of the round */
    if (winning_team != -1) {
	sprintf(msg, " < Team %d has won the game%s! >", winning_team,
		reason);
	sound_play_all(TEAM_WIN_SOUND);
    } else {
	sprintf(msg, " < We have a draw%s! >", reason);
	sound_play_all(TEAM_DRAW_SOUND);
    }
    Set_message(msg);

    /* Give bonus to the best player */
    Give_best_player_bonus(average_score,
			   num_best_players,
			   best_ratio,
			   best_players);

    /* Give bonuses to the winning team */
    if (winning_team != -1) {
	for (i = 0; i < NumPlayers; i++) {
	    if (Players[i]->team != winning_team) {
		continue;
	    }
	    if (Players[i]->robot_mode == RM_OBJECT
		|| (BIT(Players[i]->status, PAUSE)
		    && Players[i]->count <= 0)
		|| (BIT(Players[i]->status, GAME_OVER)
		    && Players[i]->mychar == 'W'
		    && Players[i]->score == 0)) {
		continue;
	    }
	    for (j = 0; j < num_best_players; j++) {
		if (i == best_players[j]) {
		    break;
		}
	    }
	    if (j == num_best_players) {
		Give_individual_bonus(i, average_score);
	    }
	}
    }

    Reset_all_players();

    free(best_players);
}

static void Individual_game_over(int winner)
{
    int			i, j;
    int			average_score;
    int			num_best_players;
    int			*best_players;
    float		best_ratio;
    char		msg[MSG_LEN];

    if (!(best_players = (int *)malloc(NumPlayers * sizeof(int)))) {
	error("no mem");
	End_game();
    }

    /* Figure out what the average score is and who has the best kill/death */
    /* ratio for this round */
    Compute_end_of_round_values(&average_score, &num_best_players,
				&best_ratio, best_players);

    /* Print out the results of the round */
    if (winner == -1) {
	Set_message(" < We have a draw! >");
	sound_play_all(PLAYER_DRAW_SOUND);
    }
    else if (winner == -2) {
	Set_message(" < The robots have won the game! >");
	/* Perhaps this should be a different sound? */
	sound_play_all(PLAYER_WIN_SOUND);
    } else {
	sprintf(msg, " < %s has won the game! >", Players[winner]->name);
	Set_message(msg);
	sound_play_all(PLAYER_WIN_SOUND);
    }

    /* Give bonus to the best player */
    Give_best_player_bonus(average_score,
			   num_best_players,
			   best_ratio,
			   best_players);

    /* Give bonus to the winning player */
    if (winner >= 0) {
	for (i = 0; i < num_best_players; i++) {
	    if (winner == best_players[i]) {
		break;
	    }
	}
	if (i == num_best_players) {
	    Give_individual_bonus(winner, average_score);
	}
    }
    else if (winner == -2) {
	for (j = 0; j < NumPlayers; j++) {
	    if (Players[j]->robot_mode != RM_OBJECT
		&& Players[j]->robot_mode != RM_NOT_ROBOT) {
		for (i = 0; i < num_best_players; i++) {
		    if (j == best_players[i]) {
			break;
		    }
		}
		if (i == num_best_players) {
		    Give_individual_bonus(j, average_score);
		}
	    }
	}
    }

    Reset_all_players();

    free(best_players);
}

static void Race_game_over(void)
{
    player		*pl;
    int			i,
			j,
			k,
			bestlap = 0,
			num_best_players = 0,
			num_active_players = 0;
    int			*order;
    char		msg[MSG_LEN];

    /*
     * Reassign players's starting posisitions based upon
     * personal best lap times.
     */
    if ((order = (int *)malloc(NumPlayers * sizeof(int))) != NULL) {
	for (i = 0; i < NumPlayers; i++) {
	    pl = Players[i];
	    if (pl->robot_mode == RM_OBJECT) {
		continue;
	    }
	    if (BIT(pl->status, PAUSE)
		|| (BIT(pl->status, GAME_OVER) && pl->mychar == 'W')
		|| pl->best_lap <= 0) {
		j = i;
	    }
	    else {
		for (j = 0; j < i; j++) {
		    if (pl->best_lap < Players[order[j]]->best_lap) {
			break;
		    }
		    if (BIT(Players[order[j]]->status, PAUSE)
			|| (BIT(Players[order[j]]->status, GAME_OVER)
			    && Players[order[j]]->mychar == 'W')) {
			break;
		    }
		}
	    }
	    for (k = i - 1; k >= j; k--) {
		order[k + 1] = order[k];
	    }
	    order[j] = i;
	}
	for (i = 0; i < NumPlayers; i++) {
	    pl = Players[order[i]];
	    if (pl->robot_mode == RM_OBJECT) {
		continue;
	    }
	    if (pl->home_base != World.baseorder[i].base_idx) {
		pl->home_base = World.baseorder[i].base_idx;
		for (j = 0; j < NumPlayers; j++) {
		    if (Players[j]->conn != NOT_CONNECTED) {
			Send_base(Players[j]->conn,
				  pl->id,
				  pl->home_base);
		    }
		}
		if (BIT(pl->status, PAUSE)) {
		    Go_home(order[i]);
		}
	    }
	}
	free(order);
    }

    for (i = 0; i < NumPlayers; i++)  {
	pl = Players[i];
	CLR_BIT(pl->status, RACE_OVER | FINISH);
	if (BIT(pl->status, PAUSE)
	    || (BIT(pl->status, GAME_OVER) && pl->mychar == 'W')
	    || pl->robot_mode == RM_OBJECT) {
	    continue;
	}
	num_active_players++;

	/* Kill any remaining players */
	if (!BIT(pl->status, GAME_OVER))
	    Kill_player(i);
	else
	    Player_death_reset(i);
	if (pl != Players[i]) {
	    continue;
	}
	if ((pl->best_lap < bestlap || bestlap == 0) &&
	    pl->best_lap > 0) {
	    bestlap = pl->best_lap;
	    num_best_players = 0;
	}
	if (pl->best_lap == bestlap)
	    num_best_players++;
    }

    /* If someone completed a lap */
    if (bestlap > 0) {
	for (i = 0; i < NumPlayers; i++)  {
	    pl = Players[i];
	    if (BIT(pl->status, PAUSE)
		|| (BIT(pl->status, GAME_OVER) && pl->mychar == 'W')
		|| pl->robot_mode == RM_OBJECT) {
		continue;
	    }
	    if (pl->best_lap == bestlap) {
		sprintf(msg,
			"%s %s the best lap time of %.2fs",
			pl->name,
			(num_best_players == 1) ? "had" : "shares",
			(float) bestlap / FPS);
		Set_message(msg);
		SCORE(i, 5 + num_active_players,
		      (int) pl->pos.x / BLOCK_SZ,
		      (int) pl->pos.y / BLOCK_SZ,
		      (num_best_players == 1) ? "[Fastest lap]" : "[Joint fastest lap]");
	    }
	}

	updateScores = true;
    }
    else if (num_active_players > NumRobots) {
	Set_message("No-one even managed to complete one lap, you should be "
		    "ashamed of yourselves.");
    }

    Reset_all_players();
}


void Compute_game_status(void)
{
    int			i;
    player		*pl;
    char		msg[MSG_LEN];

    if (BIT(World.rules->mode, TIMING)) {
	/*
	 * We need a completely separate scoring system for race mode.
	 * I'm not sure how race mode should interact with team mode,
	 * so for the moment race mode takes priority.
	 *
	 * Race mode and limited lives mode interact. With limited lives on,
	 * race ends after all players have completed the course, or have died.
	 * With limited lives mode off, the race ends when the first player
	 * completes the course - all remaining players are then killed to
	 * reset them.
	 *
	 * In limited lives mode, where the race can be run to completion,
	 * points are awarded not just to the winner but to everyone who
	 * completes the course (with more going to the winner). These
	 * points are awarded as the player crosses the line. At the end
	 * of the race, a bonus is awarded to the player with the fastest lap.
	 *
	 * In unlimited lives mode, just the winner and the holder of the
	 * fastest lap get points.
	 */

	player		*alive = NULL;
	int		num_alive_players = 0,
			num_active_players = 0,
			num_finished_players = 0,
			num_race_over_players = 0,
			num_waiting_players = 0,
			position = 1,
			total_pts, pts;

	/* First count the players */
	for (i = 0; i < NumPlayers; i++)  {
	    pl = Players[i];
	    if (BIT(pl->status, PAUSE)
		|| pl->robot_mode == RM_OBJECT) {
		continue;
	    }
	    if (!BIT(pl->status, GAME_OVER)) {
		num_alive_players++;
	    }
	    else if (pl->mychar == 'W') {
		num_waiting_players++;
		continue;
	    }

	    if (BIT(pl->status, RACE_OVER)) {
		num_race_over_players++;
		position++;
	    }
	    else if (BIT(pl->status, FINISH)) {
		num_finished_players++;
	    }
	    else if (!BIT(pl->status, GAME_OVER)) {
		alive = pl;
	    }

	    /*
	     * An active player is one who is:
	     *   still in the race.
	     *   reached the finish line just now.
	     *   has finished the race in a previous frame.
	     *   died too often.
	     */
	    num_active_players++;
	}
	if (num_active_players == 0 && num_waiting_players == 0) {
	    return;
	}

	/* Now if any players are unaccounted for */
	if (num_finished_players > 0) {
	    /*
	     * Ok, update positions. Everyone who finished the race in the last
	     * frame gets the current position.
	     */
	    
	    /* Only play the sound for the first person to cross the finish */
	    if (position == 1)
		sound_play_all(PLAYER_WIN_SOUND);

	    total_pts = 0;
	    for (i = 0; i < num_finished_players; i++) {
		total_pts += (10 + 2 * num_active_players) >> (position - 1 + i);
	    }
	    pts = total_pts / num_finished_players;

	    for (i = 0; i < NumPlayers; i++)  {
		pl = Players[i];
		if (BIT(pl->status, PAUSE)
		    || (BIT(pl->status, GAME_OVER) && pl->mychar == 'W')
		    || pl->robot_mode == RM_OBJECT) {
		    continue;
		}
		if (BIT(pl->status, FINISH)) {
		    CLR_BIT(pl->status, FINISH);
		    SET_BIT(pl->status, RACE_OVER);
		    if (pts > 0) {
			sprintf(msg,
				"%s finishes %sin position %d "
				"scoring %d point%s.",
				pl->name,
				(num_finished_players == 1) ? "" : "jointly ",
				position, pts,
				(pts == 1) ? "" : "s");
			Set_message(msg);
			sprintf(msg, "[Position %d%s]", position,
				(num_finished_players == 1) ? "" : " (jointly)");
			SCORE(i, pts,
			      (int) pl->pos.x/BLOCK_SZ,
			      (int) pl->pos.y/BLOCK_SZ, msg);
		    }
		    else {
			sprintf(msg,
				"%s finishes %sin position %d.",
				pl->name,
				(num_finished_players == 1) ? "" : "jointly ",
				position);
			Set_message(msg);
		    }
		}
	    }
	}

	/*
	 * In limited lives mode, wait for everyone to die, except
	 * for the last player.
	 */
	if (BIT(World.rules->mode, LIMITED_LIVES)) {
	    if (num_alive_players > 1) {
		return;
	    }
	    if (num_alive_players == 1) {
		if (num_finished_players + num_race_over_players == 0) {
		    return;
		}
		if (!alive || alive->round == 0) {
		    return;
		}
	    }
	}
	else if (num_alive_players > 0) {
	    return;
	}

	Race_game_over();

    } else if (BIT(World.rules->mode, TEAM_PLAY)) {

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
	    char	*bp;
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
	    if (teams_with_treasure > 1 || !max_destroyed)
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
		sprintf(msg, " by destroying %d treasures", max_destroyed);
		Team_game_over(winning_team, msg);
		return;
	    }

	    for (i = 0; i < NumPlayers; i++) {
		if (BIT(Players[i]->status, PAUSE)
		    || Players[i]->robot_mode == RM_OBJECT) {
		    continue;
		}
		team_score[Players[i]->team] += Players[i]->score;
	    }

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
		sprintf(msg, " by destroying %d treasures and saving %d",
			max_destroyed, max_left);
		Team_game_over(winning_team, msg);
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
		sprintf(msg, " by destroying %d treasures, saving %d, and "
			"scoring %d points",
			max_destroyed, max_left, max_score);
		Team_game_over(winning_team, msg);
		return;
	    }

	    /* Highly unlikely */

	    sprintf(msg, " between teams ");
	    bp = msg + strlen(msg);
	    for (i = 0; i < MAX_TEAMS; i++) {
		if (!team_win[i])
		    continue;
		*bp++ = "0123456789"[i]; *bp++ = ','; *bp++ = ' ';
	    }
	    bp -= 2;
	    *bp = '\0';
	    Team_game_over(-1, msg);

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

	for (i=0; i<NumPlayers; i++)  {
	    if (BIT(Players[i]->status, PAUSE)
		|| Players[i]->robot_mode == RM_OBJECT) {
		continue;
	    }
	    if (!BIT(Players[i]->status, GAME_OVER)) {
		num_alive_players++;
		if (Players[i]->robot_mode != RM_NOT_ROBOT) {
		    num_alive_robots++;
		}
		winner = i; 	/* Tag player that's alive */
	    }
	    else if (Players[i]->robot_mode == RM_NOT_ROBOT) {
		num_active_humans++;
	    }
	    num_active_players++;
	}

	if (num_alive_players == 1 && num_active_players > 1) {
	    Individual_game_over(winner);
	}
	else if (num_alive_players == 0 && num_active_players >= 1) {
	    Individual_game_over(-1);
	}
	else if (num_alive_robots > 1
	    && num_alive_players == num_alive_robots
	    && num_active_humans > 0) {
	    Individual_game_over(-2);
	}
    }
}

void Delete_player(int ind)
{
    player		*pl = Players[ind];
    object		*obj;
    int			i, j,
			id = pl->id;

    /* Delete remaining shots */
    for (i = NumObjs - 1; i >= 0; i--) {
	obj = Obj[i];
	if (obj->id == id) {
	    if (obj->type == OBJ_BALL) {
		Delete_shot(i);
		obj->owner = -1;
	    }
	    else {
		obj->life = 0;
		if (BIT(obj->type,
			OBJ_MINE|OBJ_SMART_SHOT|OBJ_HEAT_SHOT|OBJ_TORPEDO)) {
		    obj->mass = 0;
		}
	    }
	}
	else if (obj->owner == id) {
	    obj->owner = -1;
	    if (BIT(obj->type, OBJ_MINE)) {
		obj->life = 0;
		obj->mass = 0;
	    }
	}
    }

    if (pl->max_pulses > 0 && pl->pulses != NULL) {
	free(pl->pulses);
	pl->max_pulses = 0;
	pl->num_pulses = 0;
	pl->pulses = NULL;
    }
    Free_ship_shape(pl->ship);

    sound_close(pl);

    NumPlayers--;
    if (pl->robot_mode == RM_OBJECT) {
	NumPseudoPlayers--;
    }

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
	for (j = 0; j < MAX_RECORDED_SHOVES; j++) {
	    if (Players[i]->shove_record[j].pusher_id == id) {
		Players[i]->shove_record[j].pusher_id = -1;
	    }
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
	    if (Obj[i]->type == OBJ_BALL && Obj[i]->id == Players[ind]->id) {
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
    Explode_fighter(ind);
    Player_death_reset(ind);
}

void Player_death_reset(int ind)
{
    player		*pl = Players[ind];
    long		minfuel;


    if (pl->robot_mode == RM_OBJECT) {
	Delete_player(ind);
	updateScores = true;
	return;
    }

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

    pl->fuel.sum       	*= 0.90;		/* Loose 10% of fuel */
    minfuel		= (initialFuel * FUEL_SCALE_FACT);
    minfuel		+= (rand() % (1 + minfuel) / 5);
    pl->fuel.sum	= MAX(pl->fuel.sum, minfuel);
    Fuel_player(ind, initialTanks, pl->fuel.sum);

    pl->afterburners	= initialAfterburners;
    pl->transporters    = initialTransporters;
    pl->transInfo.count	= 0;

    if (pl->max_pulses > 0 && pl->num_pulses == 0) {
	free(pl->pulses);
	pl->pulses = NULL;
	pl->max_pulses = 0;
    }

    if (BIT(pl->mode, LIMITED_LIVES))
	pl->life--;
    else
	pl->life++;

    if (pl->life == -1) {
	pl->life = 0;
	SET_BIT(pl->status, GAME_OVER);
	pl->mychar = 'D';
    }

    pl->deaths++;

    pl->have	= DEF_HAVE;
    pl->used	|= DEF_USED;
    pl->used	&= ~(USED_KILL);
    pl->used	&= pl->have;
}

/* $Id: tuner.c,v 1.6 2001/08/26 19:27:26 gkoopman Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
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

#define	SERVER
#include <stdlib.h>
#include <time.h>
#include "const.h"
#include "global.h"
#include "proto.h"
#include "error.h"
#include "commonproto.h"


extern time_t gameOverTime;


void tuner_plock(void)
{
    pLockServer = (plock_server(pLockServer) == 1) ? true : false;
}

void tuner_shotsmax(void)
{
    int i;

    for (i = 0; i < NumPlayers; i++) {
	Players[i]->shot_max = ShotsMax;
    }
}

void tuner_shipmass(void)
{
    int i;

    for (i = 0; i < NumPlayers; i++) {
	Players[i]->emptymass = ShipMass;
    }
}

void tuner_ballmass(void)
{
    int i;

    for (i = 0; i < NumObjs; i++) {
	if (BIT(Obj[i]->type, OBJ_BALL)) {
	    Obj[i]->mass = ballMass;
	}
    }
}

void tuner_maxrobots(void)
{
    if (maxRobots < 0) {
	maxRobots = World.NumBases;
    }

    if (maxRobots < minRobots) {
	minRobots = maxRobots;
    }

    while (maxRobots < NumRobots) {
	Robot_delete(-1, true);
    }
}

void tuner_minrobots(void)
{
    if (minRobots < 0) {
	minRobots = maxRobots;
    }

    if (maxRobots < minRobots) {
	maxRobots = minRobots;
    }
}

void tuner_playershielding(void)
{
    int i;

    Set_world_rules();

    if (playerShielding) {
	SET_BIT(DEF_HAVE, HAS_SHIELD);

	for (i = 0; i < NumPlayers; i++) {
	    if (!IS_TANK_PTR(Players[i])) {
		if (!BIT(Players[i]->used, HAS_SHOT))
		    SET_BIT(Players[i]->used, HAS_SHIELD);

		SET_BIT(Players[i]->have, HAS_SHIELD);
		Players[i]->shield_time = 0;
	    }
	}
    }
    else {
	CLR_BIT(DEF_HAVE, HAS_SHIELD);

	for (i = 0; i < NumPlayers; i++) {
	    Players[i]->shield_time = 2 * FPS;
	    /* 2 seconds to get to safety */
	}
    }
}

void tuner_playerstartsshielded(void)
{
    if (playerShielding) {
	playerStartsShielded = true;	/* Doesn't make sense
					   to turn off when
					   shields are on. */
    }
}

void tuner_worldlives(void)
{
    if (worldLives < 0)
	worldLives = 0;

    Set_world_rules();

    if (BIT(World.rules->mode, LIMITED_LIVES)) {
	Reset_all_players();
	if (gameDuration == -1)
	    gameDuration = 0;
    }
}

void tuner_cannonsmartness(void)
{
    LIMIT(cannonSmartness, 0, 3);
}

void tuner_teamcannons(void)
{
    int i;
    int team;

    if (teamCannons) {
	for (i = 0; i < World.NumCannons; i++) {
	    team = Find_closest_team(World.cannon[i].blk_pos.x,
				     World.cannon[i].blk_pos.y);
	    if (team == TEAM_NOT_SET) {
		error("Couldn't find a matching team for the cannon.");
	    }
	    World.cannon[i].team = team;
	}
    }
    else {
	for (i = 0; i < World.NumCannons; i++)
	    World.cannon[i].team = TEAM_NOT_SET;
    }
}

void tuner_cannonsuseitems(void)
{
    int i, j;
    cannon_t *c;

    Move_init();

    for (i = 0; i < World.NumCannons; i++) {
	c = World.cannon + i;
	for (j = 0; j < NUM_ITEMS; j++) {
	    c->item[j] = 0;

	    if (cannonsUseItems)
		Cannon_add_item(i, j,
				(int)(rfrac() * (World.items[j].initial + 1)));
	}
    }
}

void tuner_wormtime(void)
{
    int i;

    if (wormTime < 0)
	wormTime = 0;

    if (wormTime) {
	for (i = 0; i < World.NumWormholes; i++) {
	    World.wormHoles[i].countdown = wormTime * FPS;
	}
    }
    else {
	for (i = 0; i < World.NumWormholes; i++) {
	    if (World.wormHoles[i].temporary)
		remove_temp_wormhole(i);
	    else
		World.wormHoles[i].countdown = WORMCOUNT;
	}
    }
}

void tuner_modifiers(void)
{
    int i;

    Set_world_rules();

    for (i = 0; i < NumPlayers; i++) {
	filter_mods(&Players[i]->mods);
    }
}

void tuner_minelife(void)
{
    int i;
    int life;

    if (mineLife < 0)
	mineLife = 0;

    for (i = 0; i < NumObjs; i++) {
	if (Obj[i]->type != OBJ_MINE)
	    continue;

	if (!BIT(Obj[i]->status, FROMCANNON)) {
	    life =
		(mineLife ? mineLife : MINE_LIFETIME) / (Obj[i]->mods.mini +
							 1);

	    Obj[i]->life = (int)(rfrac() * life);
	    /* We wouldn't want all the mines
	       to explode simultaneously, now
	       would we? */
	}
    }
}

void tuner_missilelife(void)
{
    int i;
    int life;

    if (missileLife < 0)
	missileLife = 0;

    for (i = 0; i < NumObjs; i++) {
	if (Obj[i]->type != OBJ_SMART_SHOT &&
	    Obj[i]->type != OBJ_HEAT_SHOT && Obj[i]->type != OBJ_TORPEDO)
	    continue;

	if (!BIT(Obj[i]->status, FROMCANNON)) {
	    life =
		(mineLife ? mineLife : MISSILE_LIFETIME) / (Obj[i]->mods.mini +
							    1);

	    Obj[i]->life = (int)(rfrac() * life);
	    /* Maybe all the missiles are full
	       nukes. Going off together might
	       not be such a good idea. */
	}
    }
}

void tuner_gameduration(void)
{
    if (gameDuration <= 0.0) {
	gameOverTime = time((time_t *) NULL);
    }

    else
	gameOverTime = (time_t) (gameDuration * 60) + time((time_t *) NULL);
}

void tuner_racelaps(void)
{
    if (BIT(World.rules->mode, TIMING)) {
	Reset_all_players();
	if (gameDuration == -1)
	    gameDuration = 0;
    }
}

/* $Id: play.c,v 5.11 2003/09/16 21:01:29 bertg Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
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
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>

#ifdef _WINDOWS
# include "NT/winServer.h"
#endif

#define SERVER
#include "version.h"
#include "xpconfig.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "saudio.h"
#include "score.h"
#include "objpos.h"
#include "commonproto.h"

char play_version[] = VERSION;


int Punish_team(int ind, int t_destroyed, int t_target)
{
    static char		msg[MSG_LEN];
    treasure_t		*td = &World.treasures[t_destroyed];
    treasure_t		*tt = &World.treasures[t_target];
    player		*pl = Players[ind];
    int			i;
    int			win_score = 0,lose_score = 0;
    int			win_team_members = 0, lose_team_members = 0;
    int			somebody_flag = 0;
    DFLOAT		sc, por;

    Check_team_members (td->team);
    if (td->team == pl->team)
	return 0;

    if (BIT(World.rules->mode, TEAM_PLAY)) {
	for (i = 0; i < NumPlayers; i++) {
	    if (IS_TANK_IND(i)
		|| (BIT(Players[i]->status, PAUSE)
		    && Players[i]->count <= 0)
		|| (BIT(Players[i]->status, GAME_OVER)
		    && Players[i]->mychar == 'W'
		    && Players[i]->score == 0)) {
		continue;
	    }
	    if (Players[i]->team == td->team) {
		lose_score += Players[i]->score;
		lose_team_members++;
		if (BIT(Players[i]->status, GAME_OVER) == 0) {
		    somebody_flag = 1;
		}
	    }
	    else if (Players[i]->team == tt->team) {
		win_score += Players[i]->score;
		win_team_members++;
	    }
	}
    }

    sound_play_all(DESTROY_BALL_SOUND);
    sprintf(msg, " < %s's (%d) team has destroyed team %d treasure >",
	    pl->name, pl->team, td->team);
    Set_message(msg);

    if (!somebody_flag) {
	SCORE(ind, Rate(pl->score, CANNON_SCORE)/2,
	      tt->pos.x, tt->pos.y, "Treasure:");
	return 0;
    }

    td->destroyed++;
    World.teams[td->team].TreasuresLeft--;
    World.teams[tt->team].TreasuresDestroyed++;


    sc  = 3 * Rate(win_score, lose_score);
    por = (sc * lose_team_members) / (2 * win_team_members + 1);

    for (i = 0; i < NumPlayers; i++) {
	if (IS_TANK_IND(i)
	    || (BIT(Players[i]->status, PAUSE)
		&& Players[i]->count <= 0)
	    || (BIT(Players[i]->status, GAME_OVER)
		&& Players[i]->mychar == 'W'
		&& Players[i]->score == 0)) {
	    continue;
	}
	if (Players[i]->team == td->team) {
	    SCORE(i, -sc, tt->pos.x, tt->pos.y,
		  "Treasure: ");
	    if (treasureKillTeam)
		SET_BIT(Players[i]->status, KILLED);
	}
	else if (Players[i]->team == tt->team &&
		 (Players[i]->team != TEAM_NOT_SET || i == ind)) {
	    SCORE(i, (i == ind ? 3*por : 2*por), tt->pos.x, tt->pos.y,
		  "Treasure: ");
	}
    }

    if (treasureKillTeam) {
	Players[ind]->kills++;
    }

    updateScores = true;

    return 1;
}


/****************************
 * Functions for explosions.
 */

/* Create debris particles */
void Make_debris(
    /* pos.x, pos.y   */ DFLOAT  x,          DFLOAT y,
    /* vel.x, vel.y   */ DFLOAT  velx,       DFLOAT vely,
    /* owner id       */ int    id,
    /* owner team     */ unsigned short team,
    /* type           */ int    type,
    /* mass           */ DFLOAT  mass,
    /* status         */ long   status,
    /* color          */ int    color,
    /* radius         */ int    radius,
    /* min,max debris */ int    min_debris, int    max_debris,
    /* min,max dir    */ int    min_dir,    int    max_dir,
    /* min,max speed  */ DFLOAT  min_speed,  DFLOAT  max_speed,
    /* min,max life   */ int    min_life,   int    max_life
)
{
    object		*debris;
    int			i, num_debris, life;
    modifiers		mods;

    if (BIT(World.rules->mode, WRAP_PLAY)) {
	if (x < 0) x += World.width;
	else if (x >= World.width) x -= World.width;
	if (y < 0) y += World.height;
	else if (y >= World.height) y -= World.height;
    }
    if (x < 0 || x >= World.width || y < 0 || y >= World.height) {
	return;
    }
    if (max_life < min_life)
	max_life = min_life;
    if (ShotsLife >= FPS) {
	if (min_life > ShotsLife) {
	    min_life = ShotsLife;
	    max_life = ShotsLife;
	} else if (max_life > ShotsLife) {
	    max_life = ShotsLife;
	}
    }
    if (min_speed * max_life > World.hypotenuse)
	min_speed = World.hypotenuse / max_life;
    if (max_speed * min_life > World.hypotenuse)
	max_speed = World.hypotenuse / min_life;
    if (max_speed < min_speed)
	max_speed = min_speed;

    CLEAR_MODS(mods);

    if (type == OBJ_SHOT) {
	SET_BIT(mods.warhead, CLUSTER);
	if (!ShotsGravity) {
	    CLR_BIT(status, GRAVITY);
	}
    }

    num_debris = min_debris + (int)(rfrac() * (max_debris - min_debris));
    if (num_debris > MAX_TOTAL_SHOTS - NumObjs) {
	num_debris = MAX_TOTAL_SHOTS - NumObjs;
    }
    for (i = 0; i < num_debris; i++) {
	DFLOAT		speed, dx, dy, diroff;
	int		dir, dirplus;

	if ((debris = Object_allocate()) == NULL) {
	    break;
	}

	debris->color = color;
	debris->id = id;
	debris->team = team;
	Object_position_init_pixels(debris, x, y);
	dir = MOD2(min_dir + (int)(rfrac() * (max_dir - min_dir)), RES);
	dirplus = MOD2(dir + 1, RES);
	diroff = rfrac();
	dx = tcos(dir) + (tcos(dirplus) - tcos(dir)) * diroff;
	dy = tsin(dir) + (tsin(dirplus) - tsin(dir)) * diroff;
	speed = min_speed + rfrac() * (max_speed - min_speed);
	debris->vel.x = velx + dx * speed;
	debris->vel.y = vely + dy * speed;
	debris->acc.x = 0;
	debris->acc.y = 0;
	if (shotHitFuelDrainUsesKineticEnergy
	    && type == OBJ_SHOT) {
	    /* compensate so that m*v^2 is constant */
	    DFLOAT sp_shotsp = speed / ShotsSpeed;
	    debris->mass = mass / (sp_shotsp * sp_shotsp);
	} else {
	    debris->mass = mass;
	}
	debris->type = type;
	life = (int)(min_life + rfrac() * (max_life - min_life) + 1);
	if (life * speed > World.hypotenuse) {
	    life = (long)(World.hypotenuse / speed);
	}
	debris->life = life;
	debris->fuselife = life;
	debris->pl_range = radius;
	debris->pl_radius = radius;
	debris->status = status;
	debris->mods = mods;
	Cell_add_object(debris);
    }
}


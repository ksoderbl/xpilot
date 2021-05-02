/* $Id: frame.c,v 3.62 1995/11/16 00:13:47 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
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

#ifdef VMS
#include <unixio.h>
#include <unixlib.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#ifndef  VMS
#include <sys/param.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "bit.h"
#include "netserver.h"
#include "saudio.h"

char frame_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: frame.c,v 3.62 1995/11/16 00:13:47 bert Exp $";
#endif


/*
 * Structure for calculating if a pixel is visible by a player.
 * The following always holds:
 *	(world.x >= realWorld.x && world.y >= realWorld.y)
 */
typedef struct {
    position	world;			/* Lower left hand corner is this */
					/* world coordinate */
    position	realWorld;		/* If the player is on the edge of
					   the screen, these are the world
					   coordinates before adjustment... */
} pixel_visibility_t;

/*
 * Structure with player position info measured in blocks instead of pixels.
 * Used for map state info updating.
 */
typedef struct {
    ipos		world;
    ipos		realWorld;
} block_visibility_t;

typedef struct {
    unsigned char	x, y;
} debris_t;


long			loops = 0;

static pixel_visibility_t pv;
static int		view_width,
			view_height,
			horizontal_blocks,
			vertical_blocks,
			debris_x_areas,
			debris_y_areas,
			debris_areas,
			debris_colors,
			spark_rand;
static debris_t		*debris_ptr[DEBRIS_TYPES];
static int		debris_num[DEBRIS_TYPES],
			debris_max[DEBRIS_TYPES];
static debris_t		*fastshot_ptr[DEBRIS_TYPES * 2];
static int		fastshot_num[DEBRIS_TYPES * 2],
			fastshot_max[DEBRIS_TYPES * 2];

static int inview(float x, float y)
{
    return ((x > pv.world.x && x < pv.world.x + view_width)
	    || (x > pv.realWorld.x && x < pv.realWorld.x + view_width))
	&& ((y > pv.world.y && y < pv.world.y + view_height)
	    || (y > pv.realWorld.y && y < pv.realWorld.y + view_height));
}

static int block_inview(block_visibility_t *bv, int x, int y)
{
    return ((x > bv->world.x && x < bv->world.x + horizontal_blocks)
	    || (x > bv->realWorld.x && x < bv->realWorld.x + horizontal_blocks))
	&& ((y > bv->world.y && y < bv->world.y + vertical_blocks)
	    || (y > bv->realWorld.y && y < bv->realWorld.y + vertical_blocks));
}

#define DEBRIS_STORE(xf,yf,color,offset) \
    int			i,						  \
			xd,						  \
			yd;						  \
    if (xf < 0) {							  \
	xf += World.width;						  \
    }									  \
    if (yf < 0) {							  \
	yf += World.height;						  \
    }									  \
    xd = (int) (xf + 0.5);						  \
    yd = (int) (yf + 0.5);						  \
    if ((unsigned) xd >= view_width || (unsigned) yd >= view_height) {	  \
	/*								  \
	 * There's some rounding error or so somewhere.			  \
	 * Should be possible to resolve it.				  \
	 */								  \
	return;								  \
    }									  \
									  \
    i = offset + color * debris_areas					  \
	+ (((yd >> 8) % debris_y_areas) * debris_x_areas)		  \
	+ ((xd >> 8) % debris_x_areas);					  \
									  \
    if (num >= 255) {							  \
	return;								  \
    }									  \
    if (num >= max) {							  \
	if (num == 0) {							  \
	    ptr = (debris_t *) malloc((max = 16) * sizeof(*ptr));	  \
	} else {							  \
	    ptr = (debris_t *) realloc(ptr, (max += max) * sizeof(*ptr)); \
	}								  \
	if (ptr == 0) {							  \
	    error("No memory for debris");				  \
	    num = 0;							  \
	    return;							  \
	}								  \
    }									  \
    ptr[num].x = (unsigned char) xd;					  \
    ptr[num].y = (unsigned char) yd;					  \
    num++;

static void fastshot_store(float xf, float yf, int color, int offset)
{
#define ptr		(fastshot_ptr[i])
#define num		(fastshot_num[i])
#define max		(fastshot_max[i])
    DEBRIS_STORE(xf, yf, color, offset);
#undef ptr
#undef num
#undef max
}

static void debris_store(float xf, float yf, int color)
{
#define ptr		(debris_ptr[i])
#define num		(debris_num[i])
#define max		(debris_max[i])
    DEBRIS_STORE(xf, yf, color, 0);
#undef ptr
#undef num
#undef max
}

static void fastshot_end(int conn)
{
    int			i;

    for (i = 0; i < DEBRIS_TYPES * 2; i++) {
	if (fastshot_num[i] != 0) {
	    Send_fastshot(conn, i,
			  (unsigned char *) fastshot_ptr[i],
			  fastshot_num[i]);
	    fastshot_num[i] = 0;
	}
    }
}

static void debris_end(int conn)
{
    int			i;

    for (i = 0; i < DEBRIS_TYPES; i++) {
	if (debris_num[i] != 0) {
	    Send_debris(conn, i,
			(unsigned char *) debris_ptr[i],
			debris_num[i]);
	    debris_num[i] = 0;
	}
    }
}

/*
 * Fast conversion of `num' into `str' starting at position `i', returns
 * index of character after converted number.
 */
static int num2str(int num, char *str, int i)
{
    int	digits, t;

    if (num < 0) {
	str[i++] = '-';
	num = -num;
    }
    if (num < 10) {
	str[i++] = '0' + num;
	return i;
    }
    for (t = num, digits = 0; t; t /= 10, digits++)
	;
    for (t = i+digits-1; t >= 0; t--) {
	str[t] = num % 10;
	num /= 10;
    }
    return i + digits;
}

static int Frame_status(int conn, int ind)
{
    static char		mods[MAX_CHARS];
    player		*pl = Players[ind];
    int			n,
			lock_ind,
			lock_id = -1,
			lock_dist = 0,
			lock_dir = 0,
			i,
			showautopilot;

    /*
     * Don't make lock visible during this frame if;
     * 0) we are not player locked or compass is not on.
     * 1) we have limited visibility and the player is out of range.
     * 2) the player is invisible and he's not in our team.
     * 3) he's not actively playing.
     * 4) we have blind mode and he's not on the visible screen.
     * 5) his distance is zero.
     */

    CLR_BIT(pl->lock.tagged, LOCK_VISIBLE);
    if (BIT(pl->lock.tagged, LOCK_PLAYER) && BIT(pl->used, OBJ_COMPASS)) {
	lock_id = pl->lock.pl_id;
	lock_ind = GetInd[lock_id];

	if ((!BIT(World.rules->mode, LIMITED_VISIBILITY)
	     || pl->lock.distance <= pl->sensor_range)
#ifndef SHOW_CLOAKERS_RANGE
	    && (pl->visibility[lock_ind].canSee || TEAM(ind, lock_ind))
#endif
	    && BIT(Players[lock_ind]->status, PLAYING|GAME_OVER) == PLAYING
	    && (playersOnRadar
		|| inview(Players[lock_ind]->pos.x, Players[lock_ind]->pos.y))
	    && pl->lock.distance != 0) {
	    SET_BIT(pl->lock.tagged, LOCK_VISIBLE);
	    lock_dir = Wrap_findDir((int)(Players[lock_ind]->pos.x - pl->pos.x),
				    (int)(Players[lock_ind]->pos.y - pl->pos.y));
	    lock_dist = pl->lock.distance;
	}
    }

    if (BIT(pl->status, HOVERPAUSE))
	showautopilot = (pl->count <= 0 || (loops % 8) < 4);
    else if (BIT(pl->used, OBJ_AUTOPILOT))
	showautopilot = (loops % 8) < 4;
    else
	showautopilot = 0;

    /*
     * Don't forget to modify Receive_modifier_bank() in netserver.c
     */
    i = 0;
    if (BIT(pl->mods.nuclear, FULLNUCLEAR))
	mods[i++] = 'F';
    if (BIT(pl->mods.nuclear, NUCLEAR))
	mods[i++] = 'N';
    if (BIT(pl->mods.warhead, CLUSTER))
	mods[i++] = 'C';
    if (BIT(pl->mods.warhead, IMPLOSION))
	mods[i++] = 'I';
    if (pl->mods.velocity) {
	if (i) mods[i++] = ' ';
	mods[i++] = 'V';
	i = num2str (pl->mods.velocity, mods, i);
    }
    if (pl->mods.mini) {
	if (i) mods[i++] = ' ';
	mods[i++] = 'X';
	i = num2str (pl->mods.mini + 1, mods, i);
    }
    if (pl->mods.spread) {
	if (i) mods[i++] = ' ';
	mods[i++] = 'Z';
	i = num2str (pl->mods.spread, mods, i);
    }
    if (pl->mods.power) {
	if (i) mods[i++] = ' ';
	mods[i++] = 'B';
	i = num2str (pl->mods.power, mods, i);
    }
    if (pl->mods.laser) {
	if (i) mods[i++] = ' ';
	mods[i++] = 'L';
	mods[i++] = (BIT(pl->mods.laser, STUN) ? 'S' : 'B');
    }
    mods[i] = '\0';
    n = Send_self(conn,
		  pl,
		  lock_id,
		  lock_dist,
		  lock_dir,
		  showautopilot,
		  Players[GetInd[Get_player_id(conn)]]->status,
		  mods);
    if (n <= 0) {
	return 0;
    }

    if (BIT(pl->used, OBJ_EMERGENCY_THRUST))
	Send_thrusttime(conn,
			pl->emergency_thrust_left,
			pl->emergency_thrust_max);
    if (BIT(pl->used, OBJ_EMERGENCY_SHIELD))
	Send_shieldtime(conn,
			pl->emergency_shield_left,
			pl->emergency_shield_max);
    if (BIT(pl->status, SELF_DESTRUCT) && pl->count > 0) {
	Send_destruct(conn, pl->count);
    }
    if (ShutdownServer != -1) {
	Send_shutdown(conn, ShutdownServer, ShutdownDelay);
    }
    return 1;
}

static void Frame_map(int conn, int ind)
{
    player		*pl = Players[ind];
    int			i,
			x,
			y,
			conn_bit = (1 << conn);
    block_visibility_t	bv;

    x = pl->pos.x / BLOCK_SZ;
    y = pl->pos.y / BLOCK_SZ;
    bv.world.x = x - (horizontal_blocks >> 1);
    bv.world.y = y - (vertical_blocks >> 1);
    bv.realWorld = bv.world;
    if (BIT(World.rules->mode, WRAP_PLAY)) {
	if (bv.world.x < 0 && bv.world.x + horizontal_blocks < World.x) {
	    bv.world.x += World.x;
	}
	else if (bv.world.x > 0 && bv.world.x + horizontal_blocks > World.x) {
	    bv.realWorld.x -= World.x;
	}
	if (bv.world.y < 0 && bv.world.y + vertical_blocks < World.y) {
	    bv.world.y += World.y;
	}
	else if (bv.world.y > 0 && bv.world.y + vertical_blocks > World.y) {
	    bv.realWorld.y -= World.y;
	}
    }

    for (i = 0; i < World.NumFuels; i++) {
	if (BIT(World.fuel[i].conn_mask, conn_bit) == 0) {
	    if (World.block[World.fuel[i].blk_pos.x]
			   [World.fuel[i].blk_pos.y] == FUEL) {
		if (block_inview(&bv,
				 World.fuel[i].blk_pos.x,
				 World.fuel[i].blk_pos.y)) {
		    Send_fuel(conn, i, (int) World.fuel[i].fuel);
		}
	    }
	}
    }

    for (i = 0; i < World.NumCannons; i++) {
	if (block_inview(&bv,
			 World.cannon[i].pos.x,
			 World.cannon[i].pos.y)) {
	    if (BIT(World.cannon[i].conn_mask, conn_bit) == 0) {
		Send_cannon(conn, i, World.cannon[i].dead_time);
	    }
	    if (World.cannon[i].dead_time <= 0) {
		World.cannon[i].active = true;
	    }
	}
    }

    for (i = 0; i < World.NumTargets; i++) {
	target_t *targ = &World.targets[i];

	if (BIT(targ->update_mask, conn_bit)
	    || (BIT(targ->conn_mask, conn_bit) == 0
		&& block_inview(&bv, targ->pos.x, targ->pos.y))) {
	    Send_target(conn, i, targ->dead_time, targ->damage);
	}
    }
}

static void Frame_shots(int conn, int ind)
{
    player		*pl = Players[ind];
    int			i, color, x, y, fuzz = 0, teamshot, len;
    object		*shot;

    for (i = 0; i < NumObjs; i++) {
	shot = Obj[i];
	if (!inview(shot->pos.x, shot->pos.y)) {
	    continue;
	}
	x = (int) (shot->pos.x + 0.5);
	y = (int) (shot->pos.y + 0.5);
	if ((color = shot->color) == BLACK) {
	    printf("black %d,%d\n", shot->type, shot->id);
	    color = WHITE;
	}
	switch (shot->type) {
	case OBJ_SPARK:
	case OBJ_DEBRIS:
	    if ((fuzz >>= 7) < 0x40) {
		fuzz = rand();
	    }
	    if ((fuzz & 0x7F) >= spark_rand) {
		/*
		 * produce a sparkling effect by not displaying
		 * particles every frame.
		 */
		break;
	    }
	    /*
	     * The number of colors which the client
	     * uses for displaying debris is bigger than 2
	     * then the color used denotes the temperature
	     * of the debris particles.
	     * Higher color number means hotter debris.
	     */
	    if (debris_colors >= 3) {
		if (debris_colors > 4) {
		    if (color == BLUE) {
			color = (shot->life >> 1);
		    } else {
			color = (shot->life >> 2);
		    }
		} else {
		    if (color == BLUE) {
			color = (shot->life >> 2);
		    } else {
			color = (shot->life >> 3);
		    }
		}
		if (color >= debris_colors) {
		    color = debris_colors - 1;
		}
	    }

	    debris_store(shot->pos.x - pv.world.x,
			 shot->pos.y - pv.world.y,
			 color);
	    break;
	case OBJ_SHOT:
	    if (shot->id != -1
		&& shot->id != pl->id
		&& TEAM_IMMUNE(ind, GetInd[shot->id])) {
		color = BLUE;
		teamshot = DEBRIS_TYPES;
	    } else if (shot->mods.nuclear && (loops & 2)) {
		color = RED;
		teamshot = DEBRIS_TYPES;
	    } else {
		teamshot = 0;
	    }

	    fastshot_store(shot->pos.x - pv.world.x,
			   shot->pos.y - pv.world.y,
			   color, teamshot);
	    break;

	case OBJ_TORPEDO:
	    len =(distinguishMissiles ? TORPEDO_LEN : MISSILE_LEN);
	    Send_missile(conn, x, y, len, shot->dir);
	    break;
	case OBJ_SMART_SHOT:
	    len =(distinguishMissiles ? SMART_SHOT_LEN : MISSILE_LEN);
	    Send_missile(conn, x, y, len, shot->dir);
	    break;
	case OBJ_HEAT_SHOT:
	    len =(distinguishMissiles ? HEAT_SHOT_LEN : MISSILE_LEN);
	    Send_missile(conn, x, y, len, shot->dir);
	    break;
	case OBJ_BALL:
	    Send_ball(conn, x, y, shot->id);
	    break;
	case OBJ_MINE:
	    {
		int id = 0;
		int laid_by_team = 0;
		int confused = 0;
		/* calculate whether ownership of mine can be determined */
		if (identifyMines
		    && (Wrap_length(pl->pos.x - shot->pos.x,
				    pl->pos.y - shot->pos.y)
			< (SHIP_SZ + MINE_SENSE_BASE_RANGE
			   + pl->item[ITEM_SENSOR] * MINE_SENSE_RANGE_FACTOR))) {
		    id = shot->id;
		    if (id==-1)
			id = EXPIRED_MINE_ID;
		    if (BIT(shot->status, CONFUSED))
			confused = 1;
		}
		laid_by_team = (shot->owner != -1 &&
				((BIT(shot->status, OWNERIMMUNE)
				     && shot->owner == pl->id)
				 || TEAM_IMMUNE(ind, GetInd[shot->owner])));
		if (confused) {
		    id = 0;
		    laid_by_team = rand() & 0x01;
		}
		Send_mine(conn, x, y, laid_by_team, id);
	    }
	    break;
	case OBJ_ITEM:
	    Send_item(conn, x, y, shot->info);
	    break;
	default:
	    error("Frame_shots: Shot type %d not defined.", shot->type);
	    break;
	}
    }
}

static void Frame_ships(int conn, int ind)
{
    player		*pl = Players[ind],
			*pl_i;
    pulse_t		*pulse;
    int			i, j, color, dir;
    float		x, y;

    for (i = 0; i < NumPlayers; i++) {
	pl_i = Players[i];
	if (!BIT(pl_i->status, PLAYING|PAUSE)) {
	    continue;
	}
	if (BIT(pl_i->status, GAME_OVER)) {
	    continue;
	}
	for (j = 0; j < pl_i->num_pulses; j++) {
	    pulse = &pl_i->pulses[j];
	    if (pulse->len <= 0) {
		continue;
	    }
	    x = pulse->pos.x;
	    y = pulse->pos.y;
	    if (BIT (World.rules->mode, WRAP_PLAY)) {
		if (x < 0) {
		    x += World.width;
		}
		else if (x >= World.width) {
		    x -= World.width;
		}
		if (y < 0) {
		    y += World.height;
		}
		else if (y >= World.height) {
		    y -= World.height;
		}
	    }
	    if (inview(x, y)) {
		dir = pulse->dir;
	    } else {
		x += tcos(pulse->dir) * pulse->len;
		y += tsin(pulse->dir) * pulse->len;
		if (BIT (World.rules->mode, WRAP_PLAY)) {
		    if (x < 0) {
			x += World.width;
		    }
		    else if (x >= World.width) {
			x -= World.width;
		    }
		    if (y < 0) {
			y += World.height;
		    }
		    else if (y >= World.height) {
			y -= World.height;
		    }
		}
		if (inview(x, y)) {
		    dir = MOD2(pulse->dir + RES/2, RES);
		}
		else {
		    continue;
		}
	    }
	    if (TEAM_IMMUNE(ind, i) && ind != i) {
		color = BLUE;
	    } else {
		color = RED;
	    }
	    Send_laser(conn,
		color,
		(int) (x + 0.5f),
		(int) (y + 0.5f),
		pulse->len,
		dir);
	}
	if (!inview(pl_i->pos.x, pl_i->pos.y)) {
	    continue;
	}
	if (BIT(pl_i->status, PAUSE)) {
	    Send_paused(conn,
		(int) (pl_i->pos.x + 0.5),
		(int) (pl_i->pos.y + 0.5),
		pl_i->count);
	    continue;
	}
	for (j = 0; j < pl_i->ecmInfo.count; j++) {
	    Send_ecm(conn,
		     (int) (pl_i->ecmInfo.pos[j].x + 0.5f),
		     (int) (pl_i->ecmInfo.pos[j].y + 0.5f),
		     pl_i->ecmInfo.size[j]);
	}
	if (pl_i->transInfo.count) {
	    player *pl = Players[GetInd[pl_i->transInfo.pl_id]];
	    Send_trans(conn,
		       (int) (pl->pos.x + 0.5),
		       (int) (pl->pos.y + 0.5),
		       (int) (pl_i->pos.x + 0.5),
		       (int) (pl_i->pos.y + 0.5));
	}

	/* Don't transmit information is fighter is invisible */
	if (pl->visibility[i].canSee
	    || i == ind
	    || TEAM(i, ind)) {
	    /*
	     * Transmit ship information
	     */
	    Send_ship(conn,
		(int) (pl_i->pos.x + 0.5),
		(int) (pl_i->pos.y + 0.5),
		pl_i->id,
		pl_i->dir,
		BIT(pl_i->used, OBJ_SHIELD) != 0,
		BIT(pl_i->used, OBJ_CLOAKING_DEVICE) != 0,
		BIT(pl_i->used, OBJ_EMERGENCY_SHIELD) != 0
	    );
	}
	if (BIT(pl_i->used, OBJ_REFUEL)) {
	    if (inview(World.fuel[pl_i->fs].pix_pos.x,
		       World.fuel[pl_i->fs].pix_pos.y)) {
		Send_refuel(conn,
		    (int) (World.fuel[pl_i->fs].pix_pos.x),
		    (int) (World.fuel[pl_i->fs].pix_pos.y),
		    (int) (pl_i->pos.x + 0.5),
		    (int) (pl_i->pos.y + 0.5));
	    }
	}
	if (BIT(pl_i->used, OBJ_REPAIR)) {
	    float x = World.targets[pl_i->repair_target].pos.x
		* BLOCK_SZ+BLOCK_SZ/2;
	    float y = World.targets[pl_i->repair_target].pos.y
		* BLOCK_SZ+BLOCK_SZ/2;
	    if (inview(x, y)) {
		/* same packet as refuel */
		Send_refuel(conn,
		    (int) (pl_i->pos.x + 0.5), (int) (pl_i->pos.y + 0.5),
		    (int) (x + 0.5), (int) (y + 0.5));
	    }
	}
	if (pl_i->tractor != NULL
	    /* && BIT(pl_i->used, OBJ_TRACTOR_BEAM) */
	    && inview(pl_i->tractor->pos.x, pl_i->tractor->pos.y)) {
	    player *t = pl_i->tractor;
	    int j;

	    for (j = 0; j < 3; j++) {
		Send_connector(conn,
		       (int) (t->pos.x + t->ship->pts[j][t->dir].x + 0.5),
		       (int) (t->pos.y + t->ship->pts[j][t->dir].y + 0.5),
		       (int) (pl_i->pos.x + 0.5),
		       (int) (pl_i->pos.y + 0.5), 1);
	    }
	}

	if (pl_i->ball != NULL
	    && inview(pl_i->ball->pos.x, pl_i->ball->pos.y)) {
	    Send_connector(conn,
		(int) (pl_i->ball->pos.x + 0.5),
		(int) (pl_i->ball->pos.y + 0.5),
		(int) (pl_i->pos.x + 0.5),
		(int) (pl_i->pos.y + 0.5), 0);
	}
    }
}

static void Frame_radar(int conn, int ind)
{
    register int	i, mask, shownuke, s;
    player		*pl = Players[ind];
    object		*shot;
    float		x, y;

#ifndef NO_SMART_MIS_RADAR
    if (nukesOnRadar) {
	mask = OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_HEAT_SHOT|OBJ_MINE;
    } else {
	mask = (missilesOnRadar ?
		(OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_HEAT_SHOT) : 0);
	mask |= (minesOnRadar) ? OBJ_MINE : 0;
    }
    if (treasuresOnRadar)
	mask |= OBJ_BALL;

    if (mask) {
	for (i = 0; i < NumObjs; i++) {
	    shot = Obj[i];
	    if (! BIT(shot->type, mask))
		continue;

	    shownuke = (nukesOnRadar && (shot)->mods.nuclear);
	    if (shownuke && (loops & 2)) {
		s = 3;
	    } else {
		s = 0;
	    }

	    if (BIT(shot->type, OBJ_MINE)) {
		if (!minesOnRadar && !shownuke)
		    continue;
		if (loops % 8 >= 6)
		    continue;
	    } else if (BIT(shot->type, OBJ_BALL)) {
		s = 2;
	    } else {
		if (!missilesOnRadar && !shownuke)
		    continue;
		if (loops & 1)
		    continue;
	    }

	    x = shot->pos.x;
	    y = shot->pos.y;
	    if (Wrap_length(pl->pos.x - x,
			    pl->pos.y - y) <= pl->sensor_range) {
		Send_radar(conn, (int) (x + 0.5), (int) (y + 0.5), s);
	    }
	}
    }
#endif
    if (playersOnRadar || BIT(World.rules->mode, TEAM_PLAY)) {
	for (i = 0; i < NumPlayers; i++) {
	    /*
	     * Don't show on the radar:
	     *		Ourselves (not necassarily same as who we watch).
	     *		People who are not playing.
	     *		People in other teams if;
	     *			no playersOnRadar or if not visible
	     */
	    if (Players[i]->conn == conn
		|| BIT(Players[i]->status, PLAYING|PAUSE|GAME_OVER) != PLAYING
		|| (!TEAM(i, ind)
		    && (!playersOnRadar || !pl->visibility[i].canSee))) {
		continue;
	    }
	    x = Players[i]->pos.x;
	    y = Players[i]->pos.y;
	    if (BIT(World.rules->mode, LIMITED_VISIBILITY)
		&& Wrap_length(pl->pos.x - x,
			       pl->pos.y - y) > pl->sensor_range) {
		continue;
	    }
	    if (BIT(pl->used, OBJ_COMPASS)
		&& BIT(pl->lock.tagged, LOCK_PLAYER)
		&& GetInd[pl->lock.pl_id] == i
		&& loops % 5 >= 3) {
		continue;
	    }
	    Send_radar(conn, (int) (x + 0.5), (int) (y + 0.5), 3);
	}
    }
}

static void Frame_parameters(int conn, int ind)
{
    player		*pl = Players[ind];

    Get_display_parameters(conn, &view_width, &view_height,
			   &debris_colors, &spark_rand);
    debris_x_areas = (view_width + 255) >> 8;
    debris_y_areas = (view_height + 255) >> 8;
    debris_areas = debris_x_areas * debris_y_areas;
    horizontal_blocks = (view_width + (BLOCK_SZ - 1)) / BLOCK_SZ;
    vertical_blocks = (view_height + (BLOCK_SZ - 1)) / BLOCK_SZ;

    pv.world.x = (int)(pl->pos.x + 0.5) - view_width / 2;	/* Scroll */
    pv.world.y = (int)(pl->pos.y + 0.5) - view_height / 2;
    pv.realWorld = pv.world;
    if (BIT (World.rules->mode, WRAP_PLAY)) {
	if (pv.world.x < 0 && pv.world.x + view_width < World.width) {
	    pv.world.x += World.width;
	}
	else if (pv.world.x > 0 && pv.world.x + view_width >= World.width) {
	    pv.realWorld.x -= World.width;
	}
	if (pv.world.y < 0 && pv.world.y + view_height < World.height) {
	    pv.world.y += World.height;
	}
	else if (pv.world.y > 0 && pv.world.y + view_height >= World.height) {
	    pv.realWorld.y -= World.height;
	}
    }
}

void Frame_update(void)
{
    int			i,
			conn,
			ind;
    player		*pl;
    time_t		newTimeLeft = 0;
    static time_t	oldTimeLeft;
    static bool		game_over_called = false;

    if (++loops >= LONG_MAX)	/* Used for misc. timing purposes */
	loops = 0;

    if (gameDuration > 0.0
	&& game_over_called == false
	&& oldTimeLeft != (newTimeLeft = gameOverTime - time(NULL))) {
	/*
	 * Do this once a second.
	 */
	if (newTimeLeft <= 0) {
	    Game_Over();
	    ShutdownServer = 30 * FPS;	/* Shutdown in 30 seconds */
	    game_over_called = true;
	}
    }

    for (i = 0; i < NumPlayers; i++) {
	pl = Players[i];
	conn = pl->conn;
	if (conn == NOT_CONNECTED) {
	    continue;
	}
	if (BIT(pl->status, PAUSE|GAME_OVER)) {
	    /*
	     * Lower the frame rate for non-playing players
	     * to reduce network load.
	     */
	    if (BIT(pl->status, PAUSE)
		? ((loops & 0x03) != 0)
		: ((loops & 0x01) != 0)) {
		continue;
	    }
	}

	/*
	* Reduce frame rate to player's own rate.
	*/
	if (pl->player_count > 0) {
	    pl->player_round++;
	    if (pl->player_round > pl->player_count) {
		pl->player_round = 0;
		continue;
	    }
	}

	if (Send_start_of_frame(conn) == -1) {
	    continue;
	}
	if (newTimeLeft != oldTimeLeft) {
	    Send_time_left(conn, newTimeLeft);
	}
	/*
	 * If status is GAME_OVER or PAUSE'd, the user may look through the
	 * other players 'eyes'.  If PAUSE'd this only works on team members.
	 * We can't use TEAM() macro as it PAUSE'd players are always on
	 * equivalent teams.
	 *
	 * This is done by using two indexes, one
	 * determining which data should be used (ind, set below) and
	 * one determining which connection to send it to (conn).
	 */
	if (BIT(pl->lock.tagged, LOCK_PLAYER)) {
	    if ((BIT(pl->status, (GAME_OVER|PLAYING)) == (GAME_OVER|PLAYING))
		|| (BIT(pl->status, PAUSE)
		    && BIT(World.rules->mode, TEAM_PLAY)
		    && pl->team != TEAM_NOT_SET
		    && pl->team == Players[GetInd[pl->lock.pl_id]]->team)) {
		ind = GetInd[pl->lock.pl_id];
	    } else {
		ind = i;
	    }
	} else {
	    ind = i;
	}
	if (Players[ind]->damaged > 0) {
	    Send_damaged(conn, Players[ind]->damaged);
	    Players[ind]->damaged--;
	} else {
	    Frame_parameters(conn, ind);
	    if (Frame_status(conn, ind) <= 0) {
		continue;
	    }
	    Frame_map(conn, ind);
	    Frame_shots(conn, ind);
	    Frame_ships(conn, ind);
	    Frame_radar(conn, ind);
	    if (pl->lose_item_state != 0) {
		Send_loseitem(pl->lose_item, conn);
		if (pl->lose_item_state == 1)
		    pl->lose_item_state = -5;
		if (pl->lose_item_state < 0)
		    pl->lose_item_state++;
	    }
	    debris_end(conn);
	    fastshot_end(conn);
	}
	sound_play_queued(Players[ind]);
	Send_end_of_frame(conn);
    }
    oldTimeLeft = newTimeLeft;
}

void Set_message(char *message)
{
    player		*pl;
    int			i;
    char		*msg,
			tmp[MSG_LEN];

    if ((i = strlen(message)) >= MSG_LEN) {
#ifndef SILENT
	errno = 0;
	error("Max message len exceed (%d,%s)", i, message);
#endif
	strncpy(tmp, message, MSG_LEN - 1);
	tmp[MSG_LEN - 1] = '\0';
	msg = tmp;
    } else {
	msg = message;
    }
    for (i = 0; i < NumPlayers; i++) {
	pl = Players[i];
	if (pl->conn != NOT_CONNECTED) {
	    Send_message(pl->conn, msg);
	}
    }
}

void Set_player_message(player *pl, char *message)
{
    int			i;
    char		*msg,
			tmp[MSG_LEN];

    if ((i = strlen(message)) >= MSG_LEN) {
#ifndef SILENT
	errno = 0;
	error("Max message len exceed (%d,%s)", i, message);
#endif
	strncpy(tmp, message, MSG_LEN - 1);
	tmp[MSG_LEN - 1] = '\0';
	msg = tmp;
    } else {
	msg = message;
    }
    if (pl->conn != NOT_CONNECTED)
	Send_message(pl->conn, msg);
    else if (IS_ROBOT_PTR(pl))
	Robot_message(GetInd[pl->id], msg);
}


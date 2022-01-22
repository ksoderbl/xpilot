/* $Id: frame.c,v 5.42 2002/05/13 20:38:10 bertg Exp $
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
#include <math.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>

#ifndef _WINDOWS
# include <unistd.h>
# include <sys/param.h>
#endif

#define SERVER
#include "xpconfig.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "bit.h"
#include "netserver.h"
#include "saudio.h"
#include "error.h"
#include "commonproto.h"


#define MAX_SHUFFLE_INDEX	65535
#define MAX_VISIBLE_OBJECTS	maxVisibleObject


typedef unsigned short shuffle_t;

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

typedef struct {
    short		x, y, size;
} radar_t;


extern time_t		gameOverTime;
long			frame_loops = 1;
static long		last_frame_shuffle;
static shuffle_t	*object_shuffle_ptr;
static int		num_object_shuffle;
static int		max_object_shuffle;
static shuffle_t	*player_shuffle_ptr;
static int		num_player_shuffle;
static int		max_player_shuffle;
static radar_t		*radar_ptr;
static int		num_radar, max_radar;

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
static unsigned		debris_num[DEBRIS_TYPES],
			debris_max[DEBRIS_TYPES];
static debris_t		*fastshot_ptr[DEBRIS_TYPES * 2];
static unsigned		fastshot_num[DEBRIS_TYPES * 2],
			fastshot_max[DEBRIS_TYPES * 2];

/*
 * Macro to make room in a given dynamic array for new elements.
 * P is the pointer to the array memory.
 * N is the current number of elements in the array.
 * M is the current size of the array.
 * T is the type of the elements.
 * E is the number of new elements to store in the array.
 * The goal is to keep the number of malloc/realloc calls low
 * while not wasting too much memory because of over-allocation.
 */
#define EXPAND(P,N,M,T,E)						\
    if ((N) + (E) > (M)) {						\
	if ((M) <= 0) {							\
	    M = (E) + 2;						\
	    P = (T *) malloc((M) * sizeof(T));				\
	    N = 0;							\
	} else {							\
	    M = ((M) << 1) + (E);					\
	    P = (T *) realloc(P, (M) * sizeof(T));			\
	}								\
	if (P == NULL) {						\
	    xperror("No memory");						\
	    N = M = 0;							\
	    return;	/* ! */						\
	}								\
    }

#define inview(x_, y_) 								\
    (   (   ((x_) > pv.world.x && (x_) < pv.world.x + view_width)		\
	 || ((x_) > pv.realWorld.x && (x_) < pv.realWorld.x + view_width))	\
     && (   ((y_) > pv.world.y && (y_) < pv.world.y + view_height)		\
	 || ((y_) > pv.realWorld.y && (y_) < pv.realWorld.y + view_height)))

static int block_inview(block_visibility_t *bv, int x, int y)
{
    return ((x > bv->world.x && x < bv->world.x + horizontal_blocks)
	    || (x > bv->realWorld.x && x < bv->realWorld.x + horizontal_blocks))
	&& ((y > bv->world.y && y < bv->world.y + vertical_blocks)
	    || (y > bv->realWorld.y && y < bv->realWorld.y + vertical_blocks));
}

#define DEBRIS_STORE(xd,yd,color,offset) \
    int			i;						  \
    if (xd < 0) {							  \
	xd += World.width;						  \
    }									  \
    if (yd < 0) {							  \
	yd += World.height;						  \
    }									  \
    if ((unsigned) xd >= (unsigned)view_width || (unsigned) yd >= (unsigned)view_height) {	  \
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
    if (num_ >= 255) {							  \
	return;								  \
    }									  \
    if (num_ >= max_) {							  \
	if (num_ == 0) {						  \
	    ptr_ = (debris_t *) malloc((max_ = 16) * sizeof(*ptr_));	  \
	} else {							  \
	    ptr_ = (debris_t *) realloc(ptr_, (max_ += max_) * sizeof(*ptr_)); \
	}								  \
	if (ptr_ == 0) {						  \
	    xperror("No memory for debris");				  \
	    num_ = 0;							  \
	    return;							  \
	}								  \
    }									  \
    ptr_[num_].x = (unsigned char) xd;					  \
    ptr_[num_].y = (unsigned char) yd;					  \
    num_++;

static void fastshot_store(int xf, int yf, int color, int offset)
{
#define ptr_		(fastshot_ptr[i])
#define num_		(fastshot_num[i])
#define max_		(fastshot_max[i])
    DEBRIS_STORE(xf, yf, color, offset);
#undef ptr_
#undef num_
#undef max_
}

static void debris_store(int xf, int yf, int color)
{
#define ptr_		(debris_ptr[i])
#define num_		(debris_num[i])
#define max_		(debris_max[i])
    DEBRIS_STORE(xf, yf, color, 0);
#undef ptr_
#undef num_
#undef max_
}

static void fastshot_end(connection_t *connp)
{
    int			i;

    for (i = 0; i < DEBRIS_TYPES * 2; i++) {
	if (fastshot_num[i] != 0) {
	    Send_fastshot(connp, i,
			  (unsigned char *) fastshot_ptr[i],
			  fastshot_num[i]);
	    fastshot_num[i] = 0;
	}
    }
}

static void debris_end(connection_t *connp)
{
    int			i;

    for (i = 0; i < DEBRIS_TYPES; i++) {
	if (debris_num[i] != 0) {
	    Send_debris(connp, i,
			(unsigned char *) debris_ptr[i],
			debris_num[i]);
	    debris_num[i] = 0;
	}
    }
}

static void Frame_radar_buffer_reset(void)
{
    num_radar = 0;
}

static void Frame_radar_buffer_add(int x, int y, int s)
{
    radar_t		*p;

    EXPAND(radar_ptr, num_radar, max_radar, radar_t, 1);
    p = &radar_ptr[num_radar++];
    p->x = x;
    p->y = y;
    p->size = s;
}

static void Frame_radar_buffer_send(connection_t *connp)
{
    int			i;
    int			dest;
    int			tmp;
    radar_t		*p;
    const int		radar_width = 256;
    int			radar_height = (radar_width * World.y) / World.x;
    int			radar_x;
    int			radar_y;
    int			send_x;
    int			send_y;
    shuffle_t		*radar_shuffle;
    size_t		shuffle_bufsize;

    if (num_radar > MIN(256, MAX_SHUFFLE_INDEX)) {
	num_radar = MIN(256, MAX_SHUFFLE_INDEX);
    }
    shuffle_bufsize = (num_radar * sizeof(shuffle_t));
    radar_shuffle = (shuffle_t *) malloc(shuffle_bufsize);
    if (radar_shuffle == (shuffle_t *) NULL) {
	return;
    }
    for (i = 0; i < num_radar; i++) {
	radar_shuffle[i] = i;
    }
    /* permute. */
    for (i = 0; i < num_radar; i++) {
	dest = (int)(rfrac() * num_radar);
	tmp = radar_shuffle[i];
	radar_shuffle[i] = radar_shuffle[dest];
	radar_shuffle[dest] = tmp;
    }

    if (Get_conn_version(connp) <= 0x4400) {
	for (i = 0; i < num_radar; i++) {
	    p = &radar_ptr[radar_shuffle[i]];
	    radar_x = (radar_width * p->x) / World.width;
	    radar_y = (radar_height * p->y) / World.height;
	    send_x = (World.width * radar_x) / radar_width;
	    send_y = (World.height * radar_y) / radar_height;
	    Send_radar(connp, send_x, send_y, p->size);
	}
    }
    else {
	unsigned char buf[3*256];
	int buf_index = 0;
	int fast_count = 0;

	if (num_radar > 256) {
	    num_radar = 256;
	}
	for (i = 0; i < num_radar; i++) {
	    p = &radar_ptr[radar_shuffle[i]];
	    radar_x = (radar_width * p->x) / World.width;
	    radar_y = (radar_height * p->y) / World.height;
	    if (radar_y >= 1024) {
		continue;
	    }
	    buf[buf_index++] = (unsigned char)(radar_x);
	    buf[buf_index++] = (unsigned char)(radar_y & 0xFF);
	    buf[buf_index] = (unsigned char)((radar_y >> 2) & 0xC0);
	    if (p->size & 0x80) {
		buf[buf_index] |= (unsigned char)(0x20);
	    }
	    buf[buf_index] |= (unsigned char)(p->size & 0x07);
	    buf_index++;
	    fast_count++;
	}
	if (fast_count > 0) {
	    Send_fastradar(connp, buf, fast_count);
	}
    }

    free(radar_shuffle);
}

static void Frame_radar_buffer_free(void)
{
    free(radar_ptr);
    radar_ptr = NULL;
    num_radar = 0;
    max_radar = 0;
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

static int Frame_status(connection_t *connp, int ind)
{
    static char		mods[MAX_CHARS];
    player		*pl = Players[ind];
    int			n,
			lock_ind,
			lock_id = NO_ID,
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
    if (BIT(pl->lock.tagged, LOCK_PLAYER) && BIT(pl->used, HAS_COMPASS)) {
	lock_id = pl->lock.pl_id;
	lock_ind = GetInd[lock_id];

	if ((!BIT(World.rules->mode, LIMITED_VISIBILITY)
	     || pl->lock.distance <= pl->sensor_range)
#ifndef SHOW_CLOAKERS_RANGE
	    && (pl->visibility[lock_ind].canSee || OWNS_TANK(ind, lock_ind) || TEAM(ind, lock_ind) || ALLIANCE(ind, lock_ind))
#endif
	    && BIT(Players[lock_ind]->status, PLAYING|GAME_OVER) == PLAYING
	    && (playersOnRadar
		|| inview(Players[lock_ind]->pos.x, Players[lock_ind]->pos.y))
	    && pl->lock.distance != 0) {
	    SET_BIT(pl->lock.tagged, LOCK_VISIBLE);
	    lock_dir = (int)Wrap_findDir((int)(Players[lock_ind]->pos.x - pl->pos.x),
				    (int)(Players[lock_ind]->pos.y - pl->pos.y));
	    lock_dist = (int)pl->lock.distance;
	}
    }

    if (BIT(pl->status, HOVERPAUSE))
	showautopilot = (pl->count <= 0 || (frame_loops % 8) < 4);
    else if (BIT(pl->used, HAS_AUTOPILOT))
	showautopilot = (frame_loops % 8) < 4;
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
    n = Send_self(connp,
		  pl,
		  lock_id,
		  lock_dist,
		  lock_dir,
		  showautopilot,
		  Players[GetInd[Get_player_id(connp)]]->status,
		  mods);
    if (n <= 0) {
	return 0;
    }

    if (BIT(pl->used, HAS_EMERGENCY_THRUST))
	Send_thrusttime(connp,
			pl->emergency_thrust_left,
			pl->emergency_thrust_max);
    if (BIT(pl->used, HAS_EMERGENCY_SHIELD))
	Send_shieldtime(connp,
			pl->emergency_shield_left,
			pl->emergency_shield_max);
    if (BIT(pl->status, SELF_DESTRUCT) && pl->count > 0) {
	Send_destruct(connp, pl->count);
    }
    if (BIT(pl->used, HAS_PHASING_DEVICE))
	Send_phasingtime(connp,
			 pl->phasing_left,
			 pl->phasing_max);
    if (ShutdownServer != -1) {
	Send_shutdown(connp, ShutdownServer, ShutdownDelay);
    }

    if (round_delay_send > 0) {
	Send_rounddelay(connp, round_delay, roundDelaySeconds * FPS);
    }

    return 1;
}

static void Frame_map(connection_t *connp, int ind)
{
    player		*pl = Players[ind];
    int			i,
			k,
			x,
			y,
			conn_bit = (1 << connp->conn_index);
    block_visibility_t	bv;
    const int		fuel_packet_size = 5;
    const int		cannon_packet_size = 5;
    const int		target_packet_size = 7;
    const int		wormhole_packet_size = 5;
    int			bytes_left = 2000;
    int			max_packet;
    int			packet_count;

    x = pl->pos.bx;
    y = pl->pos.by;
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

    packet_count = 0;
    max_packet = MAX(5, bytes_left / target_packet_size);
    i = MAX(0, pl->last_target_update);
    for (k = 0; k < World.NumTargets; k++) {
	target_t *targ;
	if (++i >= World.NumTargets) {
	    i = 0;
	}
	targ = &World.targets[i];
	if (BIT(targ->update_mask, conn_bit)
	    || (BIT(targ->conn_mask, conn_bit) == 0
		&& block_inview(&bv, targ->pos.x, targ->pos.y))) {
	    Send_target(connp, i, targ->dead_time, targ->damage);
	    pl->last_target_update = i;
	    bytes_left -= target_packet_size;
	    if (++packet_count >= max_packet) {
		break;
	    }
	}
    }

    packet_count = 0;
    max_packet = MAX(5, bytes_left / cannon_packet_size);
    i = MAX(0, pl->last_cannon_update);
    for (k = 0; k < World.NumCannons; k++) {
	if (++i >= World.NumCannons) {
	    i = 0;
	}
	if (block_inview(&bv,
			 World.cannon[i].blk_pos.x,
			 World.cannon[i].blk_pos.y)) {
	    if (BIT(World.cannon[i].conn_mask, conn_bit) == 0) {
		Send_cannon(connp, i, World.cannon[i].dead_time);
		pl->last_cannon_update = i;
		bytes_left -= max_packet * cannon_packet_size;
		if (++packet_count >= max_packet) {
		    break;
		}
	    }
	}
    }

    packet_count = 0;
    max_packet = MAX(5, bytes_left / fuel_packet_size);
    i = MAX(0, pl->last_fuel_update);
    for (k = 0; k < World.NumFuels; k++) {
	if (++i >= World.NumFuels) {
	    i = 0;
	}
	if (BIT(World.fuel[i].conn_mask, conn_bit) == 0) {
	    if (World.block[World.fuel[i].blk_pos.x]
			   [World.fuel[i].blk_pos.y] == FUEL) {
		if (block_inview(&bv,
				 World.fuel[i].blk_pos.x,
				 World.fuel[i].blk_pos.y)) {
		    Send_fuel(connp, i, (int) World.fuel[i].fuel);
		    pl->last_fuel_update = i;
		    bytes_left -= max_packet * fuel_packet_size;
		    if (++packet_count >= max_packet) {
			break;
		    }
		}
	    }
	}
    }

    packet_count = 0;
    max_packet = MAX(5, bytes_left / wormhole_packet_size);
    i = MAX(0, pl->last_wormhole_update);
    for (k = 0; k < World.NumWormholes; k++) {
	wormhole_t *worm;
	if (++i >= World.NumWormholes) {
	    i = 0;
	}
	worm = &World.wormHoles[i];
	if (wormholeVisible
	    && worm->temporary
	    && (worm->type == WORM_IN
		|| worm->type == WORM_NORMAL)
	    && block_inview(&bv, worm->pos.x, worm->pos.y)) {
	    /* This is really a stupid bug: he first converts
	       the perfect blocksizes to pixels which the
	       client is perfectly capable of doing itself.
	       Then he sends the pixels in signed shorts.
	       This will fail on big maps. */
	    int	x = (worm->pos.x * BLOCK_SZ) + BLOCK_SZ / 2,
		y = (worm->pos.y * BLOCK_SZ) + BLOCK_SZ / 2;
	    Send_wormhole(connp, x, y);
	    pl->last_wormhole_update = i;
	    bytes_left -= max_packet * wormhole_packet_size;
	    if (++packet_count >= max_packet) {
		break;
	    }
	}
    }
}


static void Frame_shuffle_objects(void)
{
    int				i;
    size_t			memsize;

    num_object_shuffle = MIN(NumObjs, MAX_VISIBLE_OBJECTS);

    if (max_object_shuffle < num_object_shuffle) {
	if (object_shuffle_ptr != NULL) {
	    free(object_shuffle_ptr);
	}
	max_object_shuffle = num_object_shuffle;
	memsize = max_object_shuffle * sizeof(shuffle_t);
	object_shuffle_ptr = (shuffle_t *) malloc(memsize);
	if (object_shuffle_ptr == NULL) {
	    max_object_shuffle = 0;
	}
    }

    if (max_object_shuffle < num_object_shuffle) {
	num_object_shuffle = max_object_shuffle;
    }

    for (i = 0; i < num_object_shuffle; i++) {
	object_shuffle_ptr[i] = i;
    }
    /* permute. */
    for (i = num_object_shuffle - 1; i >= 0; --i) {
	if (object_shuffle_ptr[i] == i) {
	    int j = (int)(rfrac() * i);
	    shuffle_t tmp = object_shuffle_ptr[j];
	    object_shuffle_ptr[j] = object_shuffle_ptr[i];
	    object_shuffle_ptr[i] = tmp;
	}
    }
}


static void Frame_shuffle_players(void)
{
    int				i;
    size_t			memsize;

    num_player_shuffle = MIN(NumPlayers, MAX_SHUFFLE_INDEX);

    if (max_player_shuffle < num_player_shuffle) {
	if (player_shuffle_ptr != NULL) {
	    free(player_shuffle_ptr);
	}
	max_player_shuffle = num_player_shuffle;
	memsize = max_player_shuffle * sizeof(shuffle_t);
	player_shuffle_ptr = (shuffle_t *) malloc(memsize);
	if (player_shuffle_ptr == NULL) {
	    max_player_shuffle = 0;
	}
    }

    if (max_player_shuffle < num_player_shuffle) {
	num_player_shuffle = max_player_shuffle;
    }

    for (i = 0; i < num_player_shuffle; i++) {
	player_shuffle_ptr[i] = i;
    }
    /* permute. */
    for (i = 0; i < num_player_shuffle; i++) {
	int j = (int)(rfrac() * num_player_shuffle);
	shuffle_t tmp = player_shuffle_ptr[j];
	player_shuffle_ptr[j] = player_shuffle_ptr[i];
	player_shuffle_ptr[i] = tmp;
    }
}


static void Frame_shuffle(void)
{
    if (last_frame_shuffle != frame_loops) {
	last_frame_shuffle = frame_loops;
	Frame_shuffle_objects();
	Frame_shuffle_players();
    }
}

static void Frame_shots(connection_t *connp, int ind)
{
    player			*pl = Players[ind];
    int				x, y;
    int				i, k, color;
    int				fuzz = 0, teamshot, len;
    int				obj_count;
    object			*shot;
    object			**obj_list;
    int				hori_blocks, vert_blocks;

    hori_blocks = (view_width + (BLOCK_SZ - 1)) / (2 * BLOCK_SZ);
    vert_blocks = (view_height + (BLOCK_SZ - 1)) / (2 * BLOCK_SZ);
    Cell_get_objects(OBJ_X_IN_BLOCKS(pl), OBJ_Y_IN_BLOCKS(pl),
		     MAX(hori_blocks, vert_blocks), num_object_shuffle,
		     &obj_list,
		     &obj_count);
    for (k = 0; k < num_object_shuffle; k++) {
	i = object_shuffle_ptr[k];
	if (i >= obj_count) {
	    continue;
	}
	shot = obj_list[i];
	x = shot->pos.x;
	y = shot->pos.y;
	if (!inview(x, y)) {
	    continue;
	}
	if ((color = shot->color) == BLACK) {
	    xpprintf("black %d,%d\n", shot->type, shot->id);
	    color = WHITE;
	}
	switch (shot->type) {
	case OBJ_SPARK:
	case OBJ_DEBRIS:
	    if ((fuzz >>= 7) < 0x40) {
		fuzz = randomMT();
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

	    debris_store((int)(shot->pos.x - pv.world.x),
			 (int)(shot->pos.y - pv.world.y),
			 color);
	    break;

	case OBJ_WRECKAGE:
	    if (spark_rand != 0 || wreckageCollisionMayKill) {
		wireobject *wreck = WIRE_PTR(shot);
		Send_wreckage(connp, x, y, (u_byte)wreck->info,
			      wreck->size, wreck->rotation);
	    }
	    break;

	case OBJ_ASTEROID: {
		wireobject *ast = WIRE_PTR(shot);
		Send_asteroid(connp, x, y,
			      (u_byte)ast->info, ast->size, ast->rotation);
	    }
	    break;

	case OBJ_SHOT:
	case OBJ_CANNON_SHOT:
	    if (Team_immune(shot->id, pl->id)
		|| (shot->id != NO_ID
		    && BIT(Players[GetInd[shot->id]]->status, PAUSE))
		|| (shot->id == NO_ID
		    && BIT(World.rules->mode, TEAM_PLAY)
		    && shot->team == pl->team)) {
		color = BLUE;
		teamshot = DEBRIS_TYPES;
	    } else if (shot->id == pl->id
		&& selfImmunity) {
		color = BLUE;
		teamshot = DEBRIS_TYPES;
	    } else if (shot->mods.nuclear && (frame_loops & 2)) {
		color = RED;
		teamshot = DEBRIS_TYPES;
	    } else {
		teamshot = 0;
	    }

	    fastshot_store((int)(shot->pos.x - pv.world.x),
			   (int)(shot->pos.y - pv.world.y),
			   color, teamshot);
	    break;

	case OBJ_TORPEDO:
	    len =(distinguishMissiles ? TORPEDO_LEN : MISSILE_LEN);
	    Send_missile(connp, x, y, len, shot->missile_dir);
	    break;
	case OBJ_SMART_SHOT:
	    len =(distinguishMissiles ? SMART_SHOT_LEN : MISSILE_LEN);
	    Send_missile(connp, x, y, len, shot->missile_dir);
	    break;
	case OBJ_HEAT_SHOT:
	    len =(distinguishMissiles ? HEAT_SHOT_LEN : MISSILE_LEN);
	    Send_missile(connp, x, y, len, shot->missile_dir);
	    break;
	case OBJ_BALL:
	    Send_ball(connp, x, y, shot->id);
	    break;
	case OBJ_MINE:
	    {
		int id = 0;
		int laid_by_team = 0;
		int confused = 0;
		mineobject *mine = MINE_PTR(shot);

		/* calculate whether ownership of mine can be determined */
		if (identifyMines
		    && (Wrap_length(pl->pos.x - mine->pos.x,
				    pl->pos.y - mine->pos.y)
			< (SHIP_SZ + MINE_SENSE_BASE_RANGE
			   + pl->item[ITEM_SENSOR] * MINE_SENSE_RANGE_FACTOR))) {
		    id = mine->id;
		    if (id == NO_ID)
			id = EXPIRED_MINE_ID;
		    if (BIT(mine->status, CONFUSED))
			confused = 1;
		}
		if (mine->id != NO_ID
		    && BIT(Players[GetInd[mine->id]]->status, PAUSE)) {
		    laid_by_team = 1;
		} else {
		    laid_by_team = (Team_immune(mine->id, pl->id)
				    || (BIT(mine->status, OWNERIMMUNE)
					&& mine->owner == pl->id));
		    if (confused) {
			id = 0;
			laid_by_team = (rfrac() < 0.5f);
		    }
		}
		Send_mine(connp, x, y, laid_by_team, id);
	    }
	    break;

	case OBJ_ITEM:
	    {
		int item_type = shot->info;

		if (BIT(shot->status, RANDOM_ITEM)) {
		    item_type = Choose_random_item();
		}

		Send_item(connp, x, y, item_type);
	    }
	    break;

	default:
	    xperror("Frame_shots: Shot type %d not defined.", shot->type);
	    break;
	}
    }
}

static void Frame_ships(connection_t *connp, int ind)
{
    player			*pl = Players[ind],
				*pl_i;
    pulse_t			*pulse;
    int				i, j, k, color, dir;
    DFLOAT			x, y;

    for (j = 0; j < NumPulses; j++) {
	pulse = Pulses[j];
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
	if (Team_immune(pulse->id, pl->id)) {
	    color = BLUE;
	} else if (pulse->id == pl->id
	    && selfImmunity) {
	    color = BLUE;
	} else {
	    color = RED;
	}
	Send_laser(connp, color, (int)x, (int)y, pulse->len, dir);
    }
    for (i = 0; i < NumEcms; i++) {
	ecm_t *ecm = Ecms[i];
	Send_ecm(connp, (int)ecm->pos.x, (int)ecm->pos.y, ecm->size);
    }
    for (i = 0; i < NumTransporters; i++) {
	trans_t *trans = Transporters[i];
	player 	*victim = Players[GetInd[trans->target]],
		*pl = (trans->id == NO_ID ? NULL : Players[GetInd[trans->id]]);
	DFLOAT 	x = (pl ? pl->pos.x : trans->pos.x),
		y = (pl ? pl->pos.y : trans->pos.y);
	Send_trans(connp, victim->pos.x, victim->pos.y, (int)x, (int)y);
    }
    for (i = 0; i < World.NumCannons; i++) {
	cannon_t *cannon = World.cannon + i;
	if (cannon->tractor_count > 0) {
	    player *t = Players[GetInd[cannon->tractor_target]];
	    if (inview(t->pos.x, t->pos.y)) {
		int j;
		for (j = 0; j < 3; j++) {
		    Send_connector(connp,
				   (int)(t->pos.x + t->ship->pts[j][t->dir].x),
				   (int)(t->pos.y + t->ship->pts[j][t->dir].y),
				   (int)cannon->pix_pos.x,
				   (int)cannon->pix_pos.y, 1);
		}
	    }
	}
    }

    for (k = 0; k < num_player_shuffle; k++) {
	i = player_shuffle_ptr[k];
	pl_i = Players[i];
	if (!BIT(pl_i->status, PLAYING|PAUSE)) {
	    continue;
	}
	if (BIT(pl_i->status, GAME_OVER)) {
	    continue;
	}
	if (!inview(pl_i->pos.x, pl_i->pos.y)) {
	    continue;
	}
	if (BIT(pl_i->status, PAUSE)) {
	    Send_paused(connp,
			pl_i->pos.x,
			pl_i->pos.y,
			pl_i->count);
	    continue;
	}

	/* Don't transmit information if fighter is invisible */
	if (pl->visibility[i].canSee
	    || i == ind
	    || TEAM(i, ind)
	    || ALLIANCE(i, ind)) {
	    /*
	     * Transmit ship information
	     */
	    Send_ship(connp,
		      pl_i->pos.x,
		      pl_i->pos.y,
		      pl_i->id,
		      pl_i->dir,
		      BIT(pl_i->used, HAS_SHIELD) != 0,
		      BIT(pl_i->used, HAS_CLOAKING_DEVICE) != 0,
		      BIT(pl_i->used, HAS_EMERGENCY_SHIELD) != 0,
		      BIT(pl_i->used, HAS_PHASING_DEVICE) != 0,
		      BIT(pl_i->used, HAS_DEFLECTOR) != 0
	    );
	}
	if (BIT(pl_i->used, HAS_REFUEL)) {
	    if (inview(World.fuel[pl_i->fs].pix_pos.x,
		       World.fuel[pl_i->fs].pix_pos.y)) {
		Send_refuel(connp,
			    (int)World.fuel[pl_i->fs].pix_pos.x,
			    (int)World.fuel[pl_i->fs].pix_pos.y,
			    pl_i->pos.x,
			    pl_i->pos.y);
	    }
	}
	if (BIT(pl_i->used, HAS_REPAIR)) {
	    DFLOAT x = (DFLOAT)(World.targets[pl_i->repair_target].pos.x + 0.5) * BLOCK_SZ;
	    DFLOAT y = (DFLOAT)(World.targets[pl_i->repair_target].pos.y + 0.5) * BLOCK_SZ;
	    if (inview(x, y)) {
		/* same packet as refuel */
		Send_refuel(connp, pl_i->pos.x, pl_i->pos.y, (int) x, (int) y);
	    }
	}
	if (BIT(pl_i->used, HAS_TRACTOR_BEAM)) {
	    player *t = Players[GetInd[pl_i->lock.pl_id]];
	    if (inview(t->pos.x, t->pos.y)) {
		int j;

		for (j = 0; j < 3; j++) {
		    Send_connector(connp,
				   (int)(t->pos.x + t->ship->pts[j][t->dir].x),
				   (int)(t->pos.y + t->ship->pts[j][t->dir].y),
				   pl_i->pos.x,
				   pl_i->pos.y, 1);
		}
	    }
	}

	if (pl_i->ball != NULL
	    && inview(pl_i->ball->pos.x, pl_i->ball->pos.y)) {
	    Send_connector(connp,
			   pl_i->ball->pos.x,
			   pl_i->ball->pos.y,
			   pl_i->pos.x,
			   pl_i->pos.y, 0);
	}
    }
}

static void Frame_radar(connection_t* connp, int ind)
{
    int			i, k, mask, shownuke, size;
    player		*pl = Players[ind];
    object		*shot;
    DFLOAT		x, y;

    Frame_radar_buffer_reset();

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
    if (asteroidsOnRadar)
	mask |= OBJ_ASTEROID;

    if (mask) {
	for (i = 0; i < NumObjs; i++) {
	    shot = Obj[i];
	    if (! BIT(shot->type, mask))
		continue;

	    shownuke = (nukesOnRadar && (shot)->mods.nuclear);
	    if (shownuke && (frame_loops & 2)) {
		size = 3;
	    } else {
		size = 0;
	    }

	    if (BIT(shot->type, OBJ_MINE)) {
		if (!minesOnRadar && !shownuke)
		    continue;
		if (frame_loops % 8 >= 6)
		    continue;
	    } else if (BIT(shot->type, OBJ_BALL)) {
		size = 2;
	    } else if (BIT(shot->type, OBJ_ASTEROID)) {
		size = WIRE_PTR(shot)->size + 1;
		size |= 0x80;
	    } else {
		if (!missilesOnRadar && !shownuke)
		    continue;
		if (frame_loops & 1)
		    continue;
	    }

	    x = shot->pos.x;
	    y = shot->pos.y;
	    if (Wrap_length(pl->pos.x - x,
			    pl->pos.y - y) <= pl->sensor_range) {
		Frame_radar_buffer_add((int)x, (int)y, size);
	    }
	}
    }
#endif

    if (playersOnRadar
	|| BIT(World.rules->mode, TEAM_PLAY)
	|| NumPseudoPlayers > 0
	|| NumAlliances > 0) {
	for (k = 0; k < num_player_shuffle; k++) {
	    i = player_shuffle_ptr[k];
	    /*
	     * Don't show on the radar:
	     *		Ourselves (not necessarily same as who we watch).
	     *		People who are not playing.
	     *		People in other teams or alliances if;
	     *			no playersOnRadar or if not visible
	     */
	    if (Players[i]->connp == connp
		|| BIT(Players[i]->status, PLAYING|PAUSE|GAME_OVER) != PLAYING
		|| (!TEAM(i, ind)
		    && !ALLIANCE(ind, i)
		    && !OWNS_TANK(ind, i)
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
	    if (BIT(pl->used, HAS_COMPASS)
		&& BIT(pl->lock.tagged, LOCK_PLAYER)
		&& GetInd[pl->lock.pl_id] == i
		&& frame_loops % 5 >= 3) {
		continue;
	    }
	    size = 3;
	    if (TEAM(i, ind) || ALLIANCE(ind, i) || OWNS_TANK(ind, i)) {
		size |= 0x80;
	    }
	    Frame_radar_buffer_add((int)x, (int)y, size);
	}
    }

    Frame_radar_buffer_send(connp);
}

static void Frame_lose_item_state(int ind)
{
    player		*pl = Players[ind];

    if (pl->lose_item_state != 0) {
	Send_loseitem(pl->lose_item, pl->connp);
	if (pl->lose_item_state == 1)
	    pl->lose_item_state = -5;
	if (pl->lose_item_state < 0)
	    pl->lose_item_state++;
    }
}

static void Frame_parameters(connection_t *connp, int ind)
{
    player		*pl = Players[ind];

    Get_display_parameters(connp, &view_width, &view_height,
			   &debris_colors, &spark_rand);
    debris_x_areas = (view_width + 255) >> 8;
    debris_y_areas = (view_height + 255) >> 8;
    debris_areas = debris_x_areas * debris_y_areas;
    horizontal_blocks = (view_width + (BLOCK_SZ - 1)) / BLOCK_SZ;
    vertical_blocks = (view_height + (BLOCK_SZ - 1)) / BLOCK_SZ;

    pv.world.x = pl->pos.x - view_width / 2;	/* Scroll */
    pv.world.y = pl->pos.y - view_height / 2;
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
			ind;
    connection_t	*connp;
    player		*pl;
    time_t		newTimeLeft = 0;
    static time_t	oldTimeLeft;
    static bool		game_over_called = false;

    if (++frame_loops >= LONG_MAX)	/* Used for misc. timing purposes */
	frame_loops = 1;

    Frame_shuffle();

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

    for (i = 0; i < num_player_shuffle; i++) {
	pl = Players[i];
	connp = pl->connp;
	if (connp == NULL) {
	    continue;
	}
	if (BIT(pl->status, PAUSE|GAME_OVER)
	    && !allowViewing
	    && !pl->isowner) {
	    /*
	     * Lower the frame rate for non-playing players
	     * to reduce network load.
	     * Owner always gets full framerate even if paused.
	     * With allowViewing on, everyone gets full framerate.
	     */
	    if (BIT(pl->status, PAUSE)) {
		if (frame_loops & 0x03) {
		    continue;
		}
	    } else {
		if (frame_loops & 0x01) {
		    continue;
		}
	    }
	}

	/*
	* Reduce frame rate to player's own rate.
	*/
	if (pl->player_count > 0) {
	    pl->player_round++;
	    if (pl->player_round >= pl->player_count) {
		pl->player_round = 0;
		continue;
	    }
	}

	if (Send_start_of_frame(connp) == -1) {
	    continue;
	}
	if (newTimeLeft != oldTimeLeft) {
	    Send_time_left(connp, newTimeLeft);
	} else if (maxRoundTime > 0 && roundtime >= 0) {
	    Send_time_left(connp, (roundtime + FPS - 1) / FPS);
	}
	/*
	 * If status is GAME_OVER or PAUSE'd, the user may look through the
	 * other players 'eyes'.  If PAUSE'd this only works on team members.
	 * We can't use TEAM() macro as PAUSE'd players are always on
	 * equivalent teams.
	 *
	 * This is done by using two indexes, one
	 * determining which data should be used (ind, set below) and
	 * one determining which connection to send it to (conn).
	 */
	if (BIT(pl->lock.tagged, LOCK_PLAYER)) {
	    if ((BIT(pl->status, (GAME_OVER|PLAYING)) == (GAME_OVER|PLAYING))
		|| (BIT(pl->status, PAUSE) &&
		    ((BIT(World.rules->mode, TEAM_PLAY)
		      && pl->team != TEAM_NOT_SET
		      && pl->team == Players[GetInd[pl->lock.pl_id]]->team)
		    || pl->isowner
		    || allowViewing))) {
		ind = GetInd[pl->lock.pl_id];
	    } else {
		ind = i;
	    }
	} else {
	    ind = i;
	}
	if (Players[ind]->damaged > 0) {
	    Send_damaged(connp, Players[ind]->damaged);
	} else {
	    Frame_parameters(connp, ind);
	    if (Frame_status(connp, ind) <= 0) {
		continue;
	    }
	    Frame_map(connp, ind);
	    Frame_ships(connp, ind);
	    Frame_shots(connp, ind);
	    Frame_radar(connp, ind);
	    Frame_lose_item_state(i);
	    debris_end(connp);
	    fastshot_end(connp);
	}
	sound_play_queued(Players[ind]);
	Send_end_of_frame(connp);
    }
    oldTimeLeft = newTimeLeft;

    Frame_radar_buffer_free();
}

void Set_message(const char *message)
{
    player		*pl;
    int			i;
    const char		*msg;
    char		tmp[MSG_LEN];

    if ((i = strlen(message)) >= MSG_LEN) {
#ifndef SILENT
	errno = 0;
	xperror("Max message len exceed (%d,%s)", i, message);
#endif
	strlcpy(tmp, message, MSG_LEN);
	msg = tmp;
    } else {
	msg = message;
    }
    for (i = 0; i < NumPlayers; i++) {
	pl = Players[i];
	if (pl->connp != NULL) {
	    Send_message(pl->connp, msg);
	}
    }
}

void Set_player_message(player *pl, const char *message)
{
    int			i;
    const char		*msg;
    char		tmp[MSG_LEN];

    if ((i = strlen(message)) >= MSG_LEN) {
#ifndef SILENT
	errno = 0;
	xperror("Max message len exceed (%d,%s)", i, message);
#endif
	memcpy(tmp, message, MSG_LEN - 1);
	tmp[MSG_LEN - 1] = '\0';
	msg = tmp;
    } else {
	msg = message;
    }
    if (pl->connp != NULL) {
	Send_message(pl->connp, msg);
    }
    else if (IS_ROBOT_PTR(pl)) {
	Robot_message(GetInd[pl->id], msg);
    }
}


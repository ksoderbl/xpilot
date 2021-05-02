/* $Id: frame.c,v 3.11 1993/08/02 12:41:13 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "global.h"
#include "version.h"
#include "bit.h"
#include "netserver.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: frame.c,v 3.11 1993/08/02 12:41:13 bjoerns Exp $";
#endif

#define BLOCK_FULL	((FULL + (BLOCK_SZ - 1)) / BLOCK_SZ)
#define BLOCK_CENTER	((CENTER + (BLOCK_SZ - 1)) / BLOCK_SZ)

/*
 * Structure with player position info measured in blocks instead of pixels.
 * Used for map state info updating.
 */
typedef struct {
    ipos		world;
    ipos		realWorld;
    int			wrappedWorld;
} block_visibility_t;

typedef struct {
    unsigned char	x, y;
} debris_t;


long			loops = 0;


static debris_t		*debris_ptr[DEBRIS_TYPES];
static int		debris_num[DEBRIS_TYPES],
			debris_max[DEBRIS_TYPES];


static int inview(player *pl, float x, float y)
{
    if (x > pl->world.x
	&& x < pl->world.x + FULL
	&& y > pl->world.y
	&& y < pl->world.y + FULL) {
	return 1;
    }
    if (!pl->wrappedWorld) {
	return 0;
    }
    if ((pl->wrappedWorld & 1) && x > FULL) {
	x -= World.x * BLOCK_SZ;
    }
    if ((pl->wrappedWorld & 2) && y > FULL) {
	y -= World.y * BLOCK_SZ;
    }
    return (x > pl->realWorld.x
	&& x < pl->realWorld.x + FULL
	&& y > pl->realWorld.y
	&& y < pl->realWorld.y + FULL);
}

static int block_inview(block_visibility_t *pl, int x, int y)
{
    if (x >= pl->world.x
	&& x <= pl->world.x + BLOCK_FULL
	&& y >= pl->world.y
	&& y <= pl->world.y + BLOCK_FULL) {
	return 1;
    }
    if (!pl->wrappedWorld) {
	return 0;
    }
    if ((pl->wrappedWorld & 1) && x > BLOCK_FULL) {
	x -= World.x;
    }
    if ((pl->wrappedWorld & 2) && y > BLOCK_FULL) {
	y -= World.y;
    }
    return (x >= pl->realWorld.x
	&& x <= pl->realWorld.x + BLOCK_FULL
	&& y >= pl->realWorld.y
	&& y <= pl->realWorld.y + BLOCK_FULL);
}

static void debris_store(float xf, float yf, int color)
{
#define ptr		(debris_ptr[i])
#define num		(debris_num[i])
#define max		(debris_max[i])

    int			i,
			xd,
			yd;

    if (xf < 0) {
	xf += World.x * BLOCK_SZ;
    }
    if (yf < 0) {
	yf += World.y * BLOCK_SZ;
    }
    xd = (int) (xf + 0.5);
    yd = (int) (yf + 0.5);
    if ((unsigned) xd >= FULL || (unsigned) yd >= FULL) {
	/*
	 * There's some rounding error or so somewhere.
	 * Should be possible to resolve it.
	 */
	if (xd == FULL) {
	    xd--;
	}
	if (yd == FULL) {
	    yd--;
	}
#ifndef SILENT
	if ((unsigned) xd >= FULL || (unsigned) yd >= FULL) {
	    printf("debris %d,%d\n", xd, yd);
	}
#endif
	return;
    }

    i = color * 9
	+ (((yd >> 8) % 3) * 3)
	+ ((xd >> 8) % 3);

    if (num >= 255) {
	return;
    }
    if (num >= max) {
	if (num == 0) {
	    ptr = (debris_t *) malloc((max = 16) * sizeof(*ptr));
	} else {
	    ptr = (debris_t *) realloc(ptr, (max += max) * sizeof(*ptr));
	}
	if (ptr == 0) {
	    error("No memory for debris");
	    num = 0;
	    return;
	}
    }
    ptr[num].x = (unsigned char) xd;
    ptr[num].y = (unsigned char) yd;
    num++;

#undef ptr
#undef num
#undef max
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

static void Frame_status(int conn, int ind)
{
    player		*pl = Players[ind];
    int			lock_ind,
			lock_id = -1,
			lock_dist = 0,
			lock_dir = 0;

    if (pl->lock.tagged == LOCK_PLAYER) {
	lock_id = pl->lock.pl_id;
	lock_ind = GetInd[lock_id];
	/*
	 * Don't send direction and distance if:
	 * 1) we have limited visibility and the player is out of range.
	 * 2) the player is invisible and he's not in our team.
	 * 3) he's not actively playing.
	 * 4) we have blind mode and he's not on the visible screen.
	 * 5) his distance is zero.
	 */
	if ((!BIT(World.rules->mode, LIMITED_VISIBILITY)
	    || pl->lock.distance <= pl->sensor_range)
#ifndef SHOW_CLOAKERS_RANGE
	    && (pl->visibility[lock_ind].canSee
		|| TEAM(ind, lock_ind))
#endif
	    && BIT(Players[lock_ind]->status, PLAYING|GAME_OVER) == PLAYING
	    && (playersOnRadar
	    || inview(pl, Players[lock_ind]->pos.x, Players[lock_ind]->pos.y))
	    && pl->lock.distance != 0) {
	    lock_dir = Wrap_findDir(Players[lock_ind]->pos.x - pl->pos.x,
				    Players[lock_ind]->pos.y - pl->pos.y);
	    lock_dist = pl->lock.distance;
	}
    }

    Send_self(conn,
	(int) (pl->pos.x + 0.5),
	(int) (pl->pos.y + 0.5),
	(int) pl->vel.x,
	(int) pl->vel.y,
	pl->dir,
	pl->power,
	pl->turnspeed,
	pl->turnresistance,
	lock_id,
	lock_dist,
	lock_dir,
	pl->check,
	pl->cloaks,
	pl->sensors,
	pl->mines,
	pl->missiles,
	pl->ecms,
	pl->transporters,
	pl->extra_shots,
	pl->back_shots,
	pl->afterburners,
	pl->fuel.num_tanks,
	pl->fuel.current,
	pl->fuel.sum,
	pl->fuel.max);

    if (BIT(pl->status, SELF_DESTRUCT) && pl->count > 0) {
	Send_destruct(conn, pl->count);
    }
    if (Shutdown != -1) {
	Send_shutdown(conn, Shutdown, ShutdownDelay);
    }
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
    bv.world.x = x - BLOCK_CENTER;
    bv.world.y = y - BLOCK_CENTER;
    bv.wrappedWorld = 0;
    if (BIT(World.rules->mode, WRAP_PLAY)) {
	bv.realWorld = bv.world;
	if (bv.world.x < 0) {
	    bv.wrappedWorld |= 1;
	    bv.world.x += World.x;
	} else if (bv.world.x + BLOCK_FULL > World.x) {
	    bv.realWorld.x -= World.x;
	    bv.wrappedWorld |= 1;
	}
	if (bv.world.y < 0) {
	    bv.wrappedWorld |= 2;
	    bv.world.y += World.y;
	} else if (bv.world.y + BLOCK_FULL > World.y) {
	    bv.realWorld.y -= World.y;
	    bv.wrappedWorld |= 2;
	}
    }

    for (i = 0; i < World.NumFuels; i++) {
	if (BIT(World.fuel[i].conn_mask, conn_bit) == 0) {
	    if (block_inview(&bv,
			     World.fuel[i].pos.x / BLOCK_SZ,
			     World.fuel[i].pos.y / BLOCK_SZ)) {
		Send_fuel(conn, i, (int) World.fuel[i].fuel);
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
		World.cannon[i].active = 1;
	    }
	}
    }

    for (i = 0; i < World.NumTargets; i++) {
	if (BIT(World.targets[i].conn_mask, conn_bit) == 0) {
	    if (block_inview(&bv,
			     World.targets[i].pos.x,
			     World.targets[i].pos.y)) {
		Send_target(conn, i, World.targets[i].dead_time,
			    World.targets[i].damage);
	    }
	}
    }
}

static void Frame_shots(int conn, int ind)
{
    player		*pl = Players[ind];
    int			i, color, x, y;
    object		*shot;

    for (i = 0; i < NumObjs; i++) {
	shot = Obj[i];
	if (!inview(pl, shot->pos.x, shot->pos.y)) {
	    continue;
	}
	x = (int) (shot->pos.x + 0.5);
	y = (int) (shot->pos.y + 0.5);
	if (BIT(shot->type, OBJ_SPARK|OBJ_DEBRIS|OBJ_SHOT)
	    && shot->id != -1
	    && shot->id != pl->id
	    && TEAM(ind, GetInd[shot->id])) {
	    color = BLUE;
	} else {
	    if ((color = shot->color) == BLACK) {
		color = WHITE;
	    }
	}
	switch (shot->type) {
	case OBJ_SPARK:
	case OBJ_DEBRIS:
	case OBJ_CANNON_DEBRIS:
	    debris_store(shot->pos.x - pl->world.x,
			 shot->pos.y - pl->world.y,
			 color);
	    break;
	case OBJ_SHOT:
	case OBJ_CANNON_SHOT:
	    Send_shot(conn, x, y, color);
	    break;
	case OBJ_TORPEDO:
	case OBJ_NUKE:
	case OBJ_SMART_SHOT:
	case OBJ_HEAT_SHOT:
	    Send_smart(conn, x, y, shot->dir);
	    break;
	case OBJ_BALL:
	    Send_ball(conn, x, y, shot->id);
	    break;
	case OBJ_MINE:
	    Send_mine(conn, x, y);
	    break;
	case OBJ_WIDEANGLE_SHOT:
	    Send_item(conn, x, y, ITEM_WIDEANGLE_SHOT);
	    break;
	case OBJ_AFTERBURNER:
	    Send_item(conn, x, y, ITEM_AFTERBURNER);
	    break;
	case OBJ_BACK_SHOT:
	    Send_item(conn, x, y, ITEM_BACK_SHOT);
	    break;
	case OBJ_ROCKET_PACK:
	    Send_item(conn, x, y, ITEM_ROCKET_PACK);
	    break;
	case OBJ_ENERGY_PACK:
	    Send_item(conn, x, y, ITEM_ENERGY_PACK);
	    break;
	case OBJ_MINE_PACK:
	    Send_item(conn, x, y, ITEM_MINE_PACK);
	    break;
	case OBJ_SENSOR_PACK:
	    Send_item(conn, x, y, ITEM_SENSOR_PACK);
	    break;
	case OBJ_ECM:
	    Send_item(conn, x, y, ITEM_ECM);
	    break;
	case OBJ_TANK:
	    Send_item(conn, x, y, ITEM_TANK);
	    break;
	case OBJ_CLOAKING_DEVICE:
	    Send_item(conn, x, y, ITEM_CLOAKING_DEVICE);
	    break;
	case OBJ_TRANSPORTER:
	    Send_item(conn, x, y, ITEM_TRANSPORTER);
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
    int			i, flag;

    for (i = 0; i < NumPlayers; i++) {
	pl_i = Players[i];
	if (!BIT(pl_i->status, PLAYING|PAUSE)) {
	    continue;
	}
	if (BIT(pl_i->status, GAME_OVER)) {
	    continue;
	}
	if (!inview(pl, pl_i->pos.x, pl_i->pos.y)) {
	    continue;
	}
	if (BIT(pl_i->status, PAUSE)) {
	    Send_paused(conn,
		(int) (pl_i->pos.x + 0.5),
		(int) (pl_i->pos.y + 0.5),
		pl_i->count);
	    continue;
	}
	if (pl_i->ecmInfo.size > 0) {
	    Send_ecm(conn,
		(int) (pl_i->ecmInfo.pos.x + 0.5),
		(int) (pl_i->ecmInfo.pos.y + 0.5),
		pl_i->ecmInfo.size);
	}
	if (pl_i->transInfo.count) {
	    player *pl = Players[GetInd[pl_i->transInfo.pl_id]];
	    Send_trans(conn, (int) (pl->pos.x + 0.5), (int) (pl->pos.y + 0.5),
		       (int) (pl_i->pos.x + 0.5), (int) (pl_i->pos.y + 0.5));
	}

	/* Don't transmit inforation is fighter is invisible */
	if (!pl->visibility[i].canSee
	    && i != ind
	    && !TEAM(i, ind)) {
	    continue;
	}

	/* Transmit ship information */
	Send_ship(conn,
	    (int) (pl_i->pos.x + 0.5),
	    (int) (pl_i->pos.y + 0.5),
	    pl_i->id,
	    pl_i->dir,
	    BIT(pl_i->used, OBJ_SHIELD) != 0,
	    BIT(pl_i->used, OBJ_CLOAKING_DEVICE) != 0
	);
	if (BIT(pl_i->used, OBJ_REFUEL)) {
	    if (inview(pl, World.fuel[pl_i->fs].pos.x,
			   World.fuel[pl_i->fs].pos.y)) {
		Send_refuel(conn,
		    (int) (World.fuel[pl_i->fs].pos.x + 0.5),
		    (int) (World.fuel[pl_i->fs].pos.y + 0.5),
		    (int) (pl_i->pos.x + 0.5),
		    (int) (pl_i->pos.y + 0.5));
	    }
	}
	if (pl_i->ball != NULL
	    /* && BIT(pl_i->used, OBJ_CONNECTOR) */
	    && inview(pl, pl_i->ball->pos.x, pl_i->ball->pos.y)) {
	    Send_connector(conn,
		(int) (pl_i->ball->pos.x + 0.5),
		(int) (pl_i->ball->pos.y + 0.5),
		(int) (pl_i->pos.x + 0.5),
		(int) (pl_i->pos.y + 0.5));  
	}
    }
}

static void Frame_radar(int conn, int ind)
{
    int			i;
    player		*pl = Players[ind];
    object		*shot;
    float		x, y;
    
#ifndef NO_SMART_MIS_RADAR
    if (missilesOnRadar) {
	if (loops & 1) {
	    for (i = 0; i < NumObjs; i++) {
		shot = Obj[i];
		if (BIT(shot->type, (OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_NUKE
				    |OBJ_HEAT_SHOT))) {
		    x = shot->pos.x;
		    y = shot->pos.y;
		    if (Wrap_length(pl->pos.x - x,
				    pl->pos.y - y) <= pl->sensor_range) {
			Send_radar(conn, (int) (x + 0.5), (int) (y + 0.5));
		    }
		}
	    }
	}
    }
#endif
    if (playersOnRadar) {
	for (i = 0; i < NumPlayers; i++) {
	    if (i == ind
		|| BIT(Players[i]->status, PLAYING|GAME_OVER) != PLAYING
		|| !pl->visibility[i].canSee) {
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
		&& pl->lock.tagged == LOCK_PLAYER
		&& GetInd[pl->lock.pl_id] == i
		&& loops % 5 >= 3) {
		continue;
	    }
	    Send_radar(conn, (int) (x + 0.5), (int) (y + 0.5));
	}
    }
}

void Frame_update(void)
{
    int			i,
			conn,
			ind;
    player		*pl;
    static int		firstTime = 1, startOfFrameRet = 0;

    if (++loops >= LONG_MAX)	/* Used for misc. timing purposes */
	loops = 0;

    for (i = 0; i < NumPlayers; i++) {
	pl = Players[i];
	conn = pl->conn;
	if (conn == NOT_CONNECTED) {
	    continue;
	}
	if (Send_start_of_frame(conn) == -1) {
	    continue;
	}
	/*
	 * If status is GAME_OVER, the user may look through the other
	 * players 'eyes'.  This is done by using two indexes, one
	 * determining which data should be used (ind, set below) and
	 * one determining which connection to send it to (conn).
	 */
	if (BIT(pl->status, GAME_OVER|PLAYING) == (GAME_OVER|PLAYING)
	    && (pl->lock.tagged == LOCK_PLAYER)) {
	    ind = GetInd[pl->lock.pl_id];
	} else {
	    ind = i;
	}
	if (Players[ind]->damaged > 0) {
	    Send_damaged(conn, Players[ind]->damaged);
	    Players[ind]->damaged--;
	} else {
	    Frame_status(conn, ind);
	    Frame_map(conn, ind);
	    Frame_shots(conn, ind);
	    Frame_ships(conn, ind);
	    Frame_radar(conn, ind);
	    debris_end(conn);
	}
#ifdef SOUND
	sound_play_queued(Players[ind]);
#endif /* SOUND */
	Send_end_of_frame(conn);
    }
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

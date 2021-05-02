/* $Id: objpos.c,v 5.1 2001/05/08 11:35:29 bertg Exp $
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

#include <stdlib.h>
#include <stdio.h>

#ifdef _WINDOWS
# include <windows.h>
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "object.h"
#include "objpos.h"

char objpos_version[] = VERSION;


void Object_position_set_clicks(object *obj, int cx, int cy)
{
    struct _objposition		*pos = (struct _objposition *)&obj->pos;

#if 0
    if (cx < 0 || cx >= PIXEL_TO_CLICK(World.width) || 
	cy < 0 || cy >= PIXEL_TO_CLICK(World.height)) {
	printf("BUG!  Illegal object position %d,%d\n", cx, cy);
	*(double *)(-1) = 4321.0;
	abort();
    }
#endif
    pos->cx = cx;
    pos->x = CLICK_TO_PIXEL(cx);
    pos->bx = pos->x / BLOCK_SZ;
    pos->cy = cy;
    pos->y = CLICK_TO_PIXEL(cy);
    pos->by = pos->y / BLOCK_SZ;
}

void Object_position_set_pixels(object *obj, DFLOAT x, DFLOAT y)
{
    Object_position_set_clicks(obj, FLOAT_TO_CLICK(x), FLOAT_TO_CLICK(y));
}

void Object_position_init_pixels(object *obj, DFLOAT x, DFLOAT y)
{
    Object_position_set_clicks(obj, FLOAT_TO_CLICK(x), FLOAT_TO_CLICK(y));
    Object_position_remember(obj);
}

void Player_position_restore(player *pl)
{
    Player_position_set_pixels(pl, pl->prevpos.x, pl->prevpos.y);
}

void Player_position_set_clicks(player *pl, int cx, int cy)
{
    struct _objposition		*pos = (struct _objposition *)&pl->pos;

#if 0
    if (cx < 0 || cx >= PIXEL_TO_CLICK(World.width) || 
	cy < 0 || cy >= PIXEL_TO_CLICK(World.height)) {
	printf("BUG!  Illegal player position %d,%d\n", cx, cy);
	*(double *)(-1) = 4321.0;
	abort();
    }
#endif
    pos->cx = cx;
    pos->x = CLICK_TO_PIXEL(cx);
    pos->bx = pos->x / BLOCK_SZ;
    pos->cy = cy;
    pos->y = CLICK_TO_PIXEL(cy);
    pos->by = pos->y / BLOCK_SZ;
}

void Player_position_set_pixels(player *pl, DFLOAT x, DFLOAT y)
{
    Player_position_set_clicks(pl, FLOAT_TO_CLICK(x), FLOAT_TO_CLICK(y));
}

void Player_position_init_pixels(player *pl, DFLOAT x, DFLOAT y)
{
    Player_position_set_clicks(pl, FLOAT_TO_CLICK(x), FLOAT_TO_CLICK(y));
    Player_position_remember(pl);
}

void Player_position_limit(player *pl)
{
    int			x = pl->pos.x, ox = x;
    int			y = pl->pos.y, oy = y;

    LIMIT(x, 0, World.width - 1);
    LIMIT(y, 0, World.height - 1);
    if (x != ox || y != oy) {
	Player_position_set_clicks(pl, PIXEL_TO_CLICK(x), PIXEL_TO_CLICK(y));
    }
}

void Player_position_debug(player *pl, const char *msg)
{
#if DEVELOPMENT
    int			i;

    printf("pl %s pos dump: ", pl->name);
    if (msg) printf("(%s)", msg);
    printf("\n");
    printf("\tB %d, %d, P %d, %d, C %d, %d, O %d, %d\n",
	   pl->pos.bx,
	   pl->pos.by,
	   pl->pos.x,
	   pl->pos.y,
	   pl->pos.cx,
	   pl->pos.cy,
	   pl->prevpos.x,
	   pl->prevpos.y);
    for (i = 0; i < pl->ship->num_points; i++) {
	printf("\t%2d\tB %d, %d, P %d, %d, C %d, %d, O %d, %d\n",
		i,
	       (int)((pl->pos.x + pl->ship->pts[i][pl->dir].x) / BLOCK_SZ),
	       (int)((pl->pos.y + pl->ship->pts[i][pl->dir].y) / BLOCK_SZ),
	       (int)(pl->pos.x + pl->ship->pts[i][pl->dir].x),
	       (int)(pl->pos.y + pl->ship->pts[i][pl->dir].y),
	       (int)(pl->pos.cx + FLOAT_TO_CLICK(pl->ship->pts[i][pl->dir].x)),
	       (int)(pl->pos.cy + FLOAT_TO_CLICK(pl->ship->pts[i][pl->dir].y)),
	       (int)(pl->prevpos.x + pl->ship->pts[i][pl->dir].x),
	       (int)(pl->prevpos.y + pl->ship->pts[i][pl->dir].y));
    }
#endif
}


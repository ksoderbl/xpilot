/* $Id: cell.c,v 5.2 2001/11/29 14:48:12 bertg Exp $
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
#include <limits.h>
#include <assert.h>

#ifdef _WINDOWS
# include "NT/winServer.h"
#endif

#define SERVER
#include "version.h"
#include "xpconfig.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "error.h"
#include "object.h"


char cell_version[] = VERSION;


/* we only search for objects which are at most 28 blocks away. */
#define MAX_CELL_DIST		28

/* sqrt(2) */
#undef SQRT2
#define SQRT2	1.41421356237309504880


typedef struct cell_dist_ cell_dist_t;
struct cell_dist_ {
    float	dist;
    short	x;
    short	y;
};

typedef struct cell_offset_ cell_offset_t;
struct cell_offset_ {
    short	x;
    short	y;
};


static cell_node **Cells;
static int object_node_offset;
static cell_dist_t *cell_dist;
static int cell_dist_size;


static void Free_cell_dist(void)
{
    if (cell_dist) {
	free(cell_dist);
	cell_dist = NULL;
    }
}


static int Compare_cell_dist(const void *a, const void *b)
{
    const cell_dist_t	*c = (const cell_dist_t *) a;
    const cell_dist_t	*d = (const cell_dist_t *) b;
    int acx, acy, adx, ady, maxc, maxd;

    if (c->dist < d->dist) return -1;
    if (c->dist > d->dist) return +1;
    acx = ABS(c->x);
    acy = ABS(c->y);
    adx = ABS(d->x);
    ady = ABS(d->y);
    maxc = MAX(acx, acy);
    maxd = MAX(adx, ady);
    if (maxc < maxd) return -1;
    if (maxc > maxd) return +1;
    return 0;
}


static void Init_cell_dist(void)
{
    cell_dist_t	*dists;
    int		x, y;
    int		cell_dist_width;
    int		cell_dist_height;
    int		cell_max_left;
    int		cell_max_right;
    int		cell_max_up;
    int		cell_max_down;

    Free_cell_dist();

    if (BIT(World.rules->mode, WRAP_PLAY)) {
	cell_max_right = MIN(MAX_CELL_DIST, (World.x / 2));
	cell_max_left = MIN(MAX_CELL_DIST, ((World.x - 1) / 2));
	cell_max_up = MIN(MAX_CELL_DIST, (World.y / 2));
	cell_max_down = MIN(MAX_CELL_DIST, ((World.y - 1) / 2));
    }
    else {
	cell_max_right = MIN(MAX_CELL_DIST, (World.x - 1));
	cell_max_left = MIN(MAX_CELL_DIST, (World.x - 1));
	cell_max_up = MIN(MAX_CELL_DIST, (World.y - 1));
	cell_max_down = MIN(MAX_CELL_DIST, (World.y - 1));
    }
    cell_dist_width = cell_max_left + 1 + cell_max_right;
    cell_dist_height = cell_max_down + 1 + cell_max_up;
    cell_dist_size = cell_dist_width * cell_dist_height;

    cell_dist = (cell_dist_t *)malloc(cell_dist_size * sizeof(cell_dist_t));
    if (cell_dist == NULL) {
	error("No cell dist mem");
	End_game();
    }

    dists = cell_dist;
    for (y = -cell_max_down; y <= cell_max_up; y++) {
	for (x = -cell_max_left; x <= cell_max_right; x++) {
	    dists->x = x;
	    dists->y = y;
	    dists->dist = (float) LENGTH(x, y);
	    dists++;
	}
    }

    qsort(cell_dist, cell_dist_size, sizeof(cell_dist_t), Compare_cell_dist);
}


void Free_cells(void)
{
    if (Cells) {
	free(Cells);
	Cells = NULL;
    }

    Free_cell_dist();
}


void Alloc_cells(void)
{
    unsigned		size;
    cell_node		*cell_ptr;
    int			x, y;

    Free_cells();

    size = sizeof(cell_node *) * World.x;
    size += sizeof(cell_node) * World.x * World.y;
    if (!(Cells = (cell_node **) malloc(size))) {
	error("No Cell mem");
	End_game();
    }
    cell_ptr = (cell_node *) &Cells[World.x];
    for (x = 0; x < World.x; x++) {
	Cells[x] = cell_ptr;
	for (y = 0; y < World.y; y++) {
	    /* init list to point to itself. */
	    cell_ptr->next = cell_ptr;
	    cell_ptr->prev = cell_ptr;
	    cell_ptr++;
	}
    }

    Init_cell_dist();
}


void Cell_init_object(object *obj)
{
    /* put obj on list with only itself. */
    obj->cell.next = &(obj->cell);
    obj->cell.prev = &(obj->cell);

    if (object_node_offset == 0) {
	object_node_offset = ((char *)&(obj->cell) - (char *)obj);
    }
}


void Cell_add_object(object *obj)
{
    int			bx, by;
    cell_node		*obj_node_ptr, *cell_node_ptr;
    cell_node		*prev, *next;

    bx = OBJ_X_IN_BLOCKS(obj);
    by = OBJ_Y_IN_BLOCKS(obj);

    obj_node_ptr = &(obj->cell);
    next = obj_node_ptr->next;
    prev = obj_node_ptr->prev;

    assert(next->prev == obj_node_ptr);
    assert(prev->next == obj_node_ptr);

    /* remove obj from current list */
    next->prev = prev;
    prev->next = next;

    if (bx < 0 || bx >= World.x || by < 0 || by >= World.y) {
	/* put obj on list with only itself. */
	obj_node_ptr->next = obj_node_ptr;
	obj_node_ptr->prev = obj_node_ptr;
    }
    else {
	/* put obj in cell list. */
	cell_node_ptr = &Cells[bx][by];
	obj_node_ptr->next = cell_node_ptr->next;
	obj_node_ptr->prev = cell_node_ptr;
	cell_node_ptr->next->prev = obj_node_ptr;
	cell_node_ptr->next = obj_node_ptr;
    }
}


void Cell_remove_object(object *obj)
{
    cell_node		*obj_node_ptr;
    cell_node		*next, *prev;

    obj_node_ptr = &(obj->cell);
    next = obj_node_ptr->next;
    prev = obj_node_ptr->prev;

    assert(next->prev == obj_node_ptr);
    assert(prev->next == obj_node_ptr);

    /* remove obj from current list */
    next->prev = prev;
    prev->next = next;

    /* put obj on list with only itself. */
    obj_node_ptr->next = obj_node_ptr;
    obj_node_ptr->prev = obj_node_ptr;

}


void Cell_get_objects(
	int x,
	int y,
	int range,
	int max_obj_count,
	object ***obj_list,
	int *count_ptr)
{
    static object	*ObjectList[MAX_TOTAL_SHOTS + 1];
    int			i, count;
    int			xw, yw;
    int			wrap;
    object		*obj;
    cell_node		*cell_node_ptr, *next;
    float		dist;


    wrap = (BIT(World.rules->mode, WRAP_PLAY) != 0);
    dist = (float) (range * SQRT2);
    count = 0;
    for (i = 0; i < cell_dist_size && count < max_obj_count; i++) {
	if (dist < cell_dist[i].dist) {
	    break;
	}
	else {
	    xw = x + cell_dist[i].x;
	    yw = y + cell_dist[i].y;
	    if (xw < 0) {
		if (wrap) {
		    xw += World.x;
		} else {
		    continue;
		}
	    }
	    else if (xw >= World.x) {
		if (wrap) {
		    xw -= World.x;
		} else {
		    continue;
		}
	    }
	    if (yw < 0) {
		if (wrap) {
		    yw += World.y;
		} else {
		    continue;
		}
	    }
	    else if (yw >= World.y) {
		if (wrap) {
		    yw -= World.y;
		} else {
		    continue;
		}
	    }
	    cell_node_ptr = &Cells[xw][yw];
	    next = cell_node_ptr->next;
	    while (next != cell_node_ptr && count < max_obj_count) {
		obj = (object *)((char *)next - object_node_offset);
		ObjectList[count++] = obj;
		next = next->next;
	    }
	}
    }

    ObjectList[count] = NULL;
    *obj_list = &ObjectList[0];
    if (count_ptr != NULL) {
	*count_ptr = count;
    }
}



/*
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


/*
 * This file deals with low-level object structure manipulations.
 */


#include <stdlib.h>

#define	SERVER
#include "xpconfig.h"
#include "types.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "error.h"
#include "portability.h"
#include "commonproto.h"


/*
 * Global variables
 */
int			ObjCount = 0;
int			NumPulses = 0;
int			NumEcms = 0;
int			NumTransporters = 0;
object			*Obj[MAX_TOTAL_SHOTS];
pulse_t			*Pulses[MAX_TOTAL_PULSES];
ecm_t			*Ecms[MAX_TOTAL_ECMS];
trans_t			*Transporters[MAX_TOTAL_TRANSPORTERS];


static void Object_incr_count(void)
{
    ObjCount++;
}

static void Object_decr_count(void)
{
    ObjCount--;
}

object *Object_allocate(void)
{
    object	*obj = OBJ_PTR(NULL);

    if (ObjCount < MAX_TOTAL_SHOTS) {
	obj = Obj[ObjCount];
	Object_incr_count();

	obj->type = OBJ_DEBRIS;
	obj->life = 0;
    }

    return obj;
}

void Object_free_ind(int ind)
{
    if ((0 <= ind) && (ind < ObjCount) && (ObjCount <= MAX_TOTAL_SHOTS)) {
	object *obj = Obj[ind];
	Object_decr_count();
	Obj[ind] = Obj[ObjCount];
	Obj[ObjCount] = obj;
    }
    else {
	warn("Cannot free object %d, when count = %d, and total = %d !",
	     ind, ObjCount, MAX_TOTAL_SHOTS);
    }
}

void Object_free_ptr(object *obj)
{
    int		i;

    for (i = ObjCount - 1; i >= 0; i--) {
	if (Obj[i] == obj) {
	    Object_free_ind(i);
	    break;
	}
    }
    if (i < 0) {
	warn("Could NOT free object!");
    }
}

static anyobject *objArray;

void Alloc_shots(int number)
{
    anyobject		*x;
    int			i;

    x = (anyobject *) calloc(number, sizeof(anyobject));
    if (!x) {
	error("Not enough memory for shots.");
	exit(1);
    }

    objArray = x;
    for (i = 0; i < number; i++) {
	Obj[i] = &(x->obj);
	MINE_PTR(Obj[i])->owner = NO_ID;
	Cell_init_object(Obj[i]);
	x++;
    }
}

void Free_shots(void)
{
    if (objArray != NULL) {
	free(objArray);
	objArray = NULL;
    }
}


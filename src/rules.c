/* $Id: rules.c,v 3.45 1997/02/25 14:04:24 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gÿsbers         <bert@xpilot.org>
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
#include "rules.h"
#include "bit.h"

char rules_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: rules.c,v 3.45 1997/02/25 14:04:24 bert Exp $";
#endif


long	KILLING_SHOTS = (OBJ_SHOT|OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_HEAT_SHOT|OBJ_PULSE);
long	DEF_BITS = 0;
long	KILL_BITS = (THRUSTING|PLAYING|KILLED|SELF_DESTRUCT|PAUSE|WARPING|WARPED);
long	DEF_HAVE =
	(OBJ_SHIELD|OBJ_COMPASS|OBJ_REFUEL|OBJ_REPAIR|OBJ_CONNECTOR
	|OBJ_SHOT|OBJ_LASER);
long	DEF_USED = (OBJ_SHIELD|OBJ_COMPASS);
long	USED_KILL =
	(OBJ_REFUEL|OBJ_REPAIR|OBJ_CONNECTOR|OBJ_SHOT|OBJ_LASER
	|OBJ_TRACTOR_BEAM|OBJ_CLOAKING_DEVICE
	|OBJ_EMERGENCY_SHIELD|OBJ_EMERGENCY_THRUST);



/*
 * Convert between probability for something to happen a given second on a
 * given block, to chance for such an event to happen on any block this tick.
 */
static void Set_item_chance(int item)
{
    float	max = itemProbMult * maxItemDensity * World.x * World.y;

    if (itemProbMult * World.items[item].prob > 0) {
	World.items[item].chance = (int)(1.0
	    / (itemProbMult * World.items[item].prob * World.x * World.y * FPS));
	World.items[item].chance = MAX(World.items[item].chance, 1);
    } else {
	World.items[item].chance = 0;
    }
    if (max > 0) {
	if (max < 1)
	    World.items[item].max = 1;
	else
	    World.items[item].max = (int)max;
    } else
	World.items[item].max = 0;
}


/*
 * An item probability has been changed during game play.
 * Update the World.items structure and test if there are more items
 * in the world than wanted for the new item probabilities.
 * This function is also called when itemProbMult or maxItemDensity changes.
 */
void Tune_item_probs(void)
{
    int			i, j, excess;

    for (i = 0; i < NUM_ITEMS; i++) {
	Set_item_chance(i);
	excess = World.items[i].num - World.items[i].max;
	if (excess > 0) {
	    for (j = 0; j < NumObjs; j++) {
		object *obj = Obj[j];
		if (obj->type == OBJ_ITEM) {
		    if (obj->info == i) {
			Delete_shot(j);
			j--;
			if (--excess == 0) {
			    break;
			}
		    }
		}
	    }
	}
    }
}


/*
 * Initializes special items.
 * First parameter is type,
 * second and third parameters are minimum and maximum number
 * of elements one item gives when picked up by a ship.
 */
static void Init_item(int item, int minpp, int maxpp)
{
    World.items[item].num = 0;

    World.items[item].min_per_pack = minpp;
    World.items[item].max_per_pack = maxpp;

    Set_item_chance(item);
}


/*
 * Give (or remove) capabilities of the ships depending upon
 * the availability of initial items.
 * Limit the initial resources between minimum and maximum possible values.
 */
void Set_initial_resources(void)
{
    CLR_BIT(DEF_HAVE,
	OBJ_CLOAKING_DEVICE |
	OBJ_EMERGENCY_THRUST |
	OBJ_EMERGENCY_SHIELD |
	OBJ_TRACTOR_BEAM |
	OBJ_AUTOPILOT);

    if (World.items[ITEM_CLOAK].initial > 0)
	SET_BIT(DEF_HAVE, OBJ_CLOAKING_DEVICE);
    if (World.items[ITEM_EMERGENCY_THRUST].initial > 0)
	SET_BIT(DEF_HAVE, OBJ_EMERGENCY_THRUST);
    if (World.items[ITEM_EMERGENCY_SHIELD].initial > 0)
	SET_BIT(DEF_HAVE, OBJ_EMERGENCY_SHIELD);
    if (World.items[ITEM_TRACTOR_BEAM].initial > 0)
	SET_BIT(DEF_HAVE, OBJ_TRACTOR_BEAM);
    if (World.items[ITEM_AUTOPILOT].initial > 0)
	SET_BIT(DEF_HAVE, OBJ_AUTOPILOT);
    LIMIT(World.items[ITEM_TANK].initial, 0, MAX_TANKS);
    LIMIT(World.items[ITEM_LASER].initial, 0, MAX_LASERS);
    LIMIT(World.items[ITEM_AFTERBURNER].initial, 0, MAX_AFTERBURNER);
    LIMIT(World.items[ITEM_TRACTOR_BEAM].initial, 0, MAX_TRACTORS);
}


void Set_misc_item_limits(void)
{
    LIMIT(dropItemOnKillProb, 0.0, 1.0);
    LIMIT(detonateItemOnKillProb, 0.0, 1.0);
    LIMIT(movingItemProb, 0.0, 1.0);
    LIMIT(destroyItemInCollisionProb, 0.0, 1.0);

    LIMIT(itemConcentratorRadius, 1, World.diagonal);
    LIMIT(itemConcentratorProb, 0.0, 1.0);
}


/*
 * First time initialization of all global item stuff.
 */
void Set_world_items(void)
{
    Init_item(ITEM_FUEL, 0, 0);
    Init_item(ITEM_TANK, 1, 1);
    Init_item(ITEM_ECM, 1, 1);
    Init_item(ITEM_MINE, 1, 2);
    Init_item(ITEM_MISSILE, maxMissilesPerPack, maxMissilesPerPack);
    Init_item(ITEM_CLOAK, 1, 1);
    Init_item(ITEM_SENSOR, 1, 1);
    Init_item(ITEM_WIDEANGLE, 1, 1);
    Init_item(ITEM_REARSHOT, 1, 1);
    Init_item(ITEM_AFTERBURNER, 1, 1);
    Init_item(ITEM_TRANSPORTER, 1, 1);
    Init_item(ITEM_LASER, 1, 1);
    Init_item(ITEM_EMERGENCY_THRUST, 1, 1);
    Init_item(ITEM_EMERGENCY_SHIELD, 1, 1);
    Init_item(ITEM_TRACTOR_BEAM, 1, 1);
    Init_item(ITEM_AUTOPILOT, 1, 1);

    Set_misc_item_limits();

    Set_initial_resources();
}


void Set_world_rules(void)
{
    static rules_t rules;

    rules.mode =
      ((crashWithPlayer ? CRASH_WITH_PLAYER : 0)
       | (bounceWithPlayer ? BOUNCE_WITH_PLAYER : 0)
       | (playerKillings ? PLAYER_KILLINGS : 0)
       | (playerShielding ? PLAYER_SHIELDING : 0)
       | (limitedVisibility ? LIMITED_VISIBILITY : 0)
       | (limitedLives ? LIMITED_LIVES : 0)
       | (teamPlay ? TEAM_PLAY : 0)
       | (onePlayerOnly ? ONE_PLAYER_ONLY : 0)
       | (timing ? TIMING : 0)
       | (allowNukes ? ALLOW_NUKES : 0)
       | (allowClusters ? ALLOW_CLUSTERS : 0)
       | (allowModifiers ? ALLOW_MODIFIERS : 0)
       | (allowLaserModifiers ? ALLOW_LASER_MODIFIERS : 0)
       | (edgeWrap ? WRAP_PLAY : 0));
    rules.lives = worldLives;
    World.rules = &rules;

    if (!BIT(World.rules->mode, PLAYER_KILLINGS))
	CLR_BIT(KILLING_SHOTS,
		OBJ_SHOT|OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_HEAT_SHOT|OBJ_PULSE);
    if (!BIT(World.rules->mode, PLAYER_SHIELDING))
	CLR_BIT(DEF_HAVE, OBJ_SHIELD);

    DEF_USED &= DEF_HAVE;
}


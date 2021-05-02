/* $Id: rules.c,v 5.9 2001/09/18 18:20:06 bertg Exp $
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
#include "rules.h"
#include "bit.h"
#include "cannon.h"

char rules_version[] = VERSION;


#define MAX_FUEL                10000
#define MAX_WIDEANGLE           99
#define MAX_REARSHOT            99
#define MAX_CLOAK               99
#define MAX_SENSOR              99
#define MAX_TRANSPORTER         99
#define MAX_MINE                99
#define MAX_MISSILE             99
#define MAX_ECM                 99
#define MAX_ARMOR		99
#define MAX_EMERGENCY_THRUST    99
#define MAX_AUTOPILOT           99
#define MAX_EMERGENCY_SHIELD    99
#define MAX_DEFLECTOR           99
#define MAX_MIRROR		99
#define MAX_PHASING             99
#define MAX_HYPERJUMP           99
#define MAX_LASER		99
#define MAX_TRACTOR_BEAM	99

long	KILLING_SHOTS = (OBJ_SHOT|OBJ_CANNON_SHOT|OBJ_SMART_SHOT
			 |OBJ_TORPEDO|OBJ_HEAT_SHOT|OBJ_PULSE);
long	DEF_BITS = 0;
long	KILL_BITS = (THRUSTING|PLAYING|KILLED|SELF_DESTRUCT|PAUSE|WARPING|WARPED);
long	DEF_HAVE =
	(HAS_SHIELD|HAS_COMPASS|HAS_REFUEL|HAS_REPAIR|HAS_CONNECTOR
	|HAS_SHOT|HAS_LASER);
long	DEF_USED = (HAS_SHIELD|HAS_COMPASS);
long	USED_KILL =
	(HAS_REFUEL|HAS_REPAIR|HAS_CONNECTOR|HAS_SHOT|HAS_LASER|HAS_ARMOR
	|HAS_TRACTOR_BEAM|HAS_CLOAKING_DEVICE|HAS_PHASING_DEVICE
	|HAS_DEFLECTOR|HAS_MIRROR|HAS_EMERGENCY_SHIELD|HAS_EMERGENCY_THRUST);



/*
 * Convert between probability for something to happen a given second on a
 * given block, to chance for such an event to happen on any block this tick.
 */
static void Set_item_chance(int item)
{
    DFLOAT	max = itemProbMult * maxItemDensity * World.x * World.y;
    DFLOAT	sum = 0;
    int		i, num = 0;

    if (itemProbMult * World.items[item].prob > 0) {
	World.items[item].chance = (int)(1.0
	    / (itemProbMult * World.items[item].prob * World.x * World.y * FPS));
	World.items[item].chance = MAX(World.items[item].chance, 1);
    } else {
	World.items[item].chance = 0;
    }
    if (max > 0) {
	if (max < 1) {
	    World.items[item].max = 1;
	} else {
	    World.items[item].max = (int)max;
	}
    } else {
	World.items[item].max = 0;
    }
    if (!BIT(CANNON_USE_ITEM, 1U << item)) {
	World.items[item].cannonprob = 0;
	return;
    }
    for (i = 0; i < NUM_ITEMS; i++) {
	if (World.items[i].prob > 0
	    && BIT(CANNON_USE_ITEM, 1U << i)) {
	    sum += World.items[i].prob;
	    num++;
	}
    }
    if (num) {
	World.items[item].cannonprob = World.items[item].prob
				       * (num / sum)
				       * (maxItemDensity / 0.00012);
    } else {
	World.items[item].cannonprob = 0;
    }
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

void Tune_asteroid_prob(void)
{
    DFLOAT	max = maxAsteroidDensity * World.x * World.y;

    if (World.asteroids.prob > 0) {
	World.asteroids.chance = (int)(1.0
			/ (World.asteroids.prob * World.x * World.y * FPS));
	World.asteroids.chance = MAX(World.asteroids.chance, 1);
    } else {
	World.asteroids.chance = 0;
    }
    if (max > 0) {
	if (max < 1) {
	    World.asteroids.max = 1;
	} else {
	    World.asteroids.max = (int)max;
	}
    } else {
	World.asteroids.max = 0;
    }
    /* superfluous asteroids are handled by Asteroid_update() */
}

/*
 * Postprocess a change command for the number of items per pack.
 */
void Tune_item_packs(void)
{
    World.items[ITEM_MINE].max_per_pack = maxMinesPerPack;
    World.items[ITEM_MISSILE].max_per_pack = maxMissilesPerPack;
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
    int			i;

    LIMIT(World.items[ITEM_FUEL].limit, 0, MAX_FUEL);
    LIMIT(World.items[ITEM_WIDEANGLE].limit, 0, MAX_WIDEANGLE);
    LIMIT(World.items[ITEM_REARSHOT].limit, 0, MAX_REARSHOT);
    LIMIT(World.items[ITEM_AFTERBURNER].limit, 0, MAX_AFTERBURNER);
    LIMIT(World.items[ITEM_CLOAK].limit, 0, MAX_CLOAK);
    LIMIT(World.items[ITEM_SENSOR].limit, 0, MAX_SENSOR);
    LIMIT(World.items[ITEM_TRANSPORTER].limit, 0, MAX_TRANSPORTER);
    LIMIT(World.items[ITEM_TANK].limit, 0, MAX_TANKS);
    LIMIT(World.items[ITEM_MINE].limit, 0, MAX_MINE);
    LIMIT(World.items[ITEM_MISSILE].limit, 0, MAX_MISSILE);
    LIMIT(World.items[ITEM_ECM].limit, 0, MAX_ECM);
    LIMIT(World.items[ITEM_LASER].limit, 0, MAX_LASER);
    LIMIT(World.items[ITEM_EMERGENCY_THRUST].limit, 0, MAX_EMERGENCY_THRUST);
    LIMIT(World.items[ITEM_TRACTOR_BEAM].limit, 0, MAX_TRACTOR_BEAM);
    LIMIT(World.items[ITEM_AUTOPILOT].limit, 0, MAX_AUTOPILOT);
    LIMIT(World.items[ITEM_EMERGENCY_SHIELD].limit, 0, MAX_EMERGENCY_SHIELD);
    LIMIT(World.items[ITEM_DEFLECTOR].limit, 0, MAX_DEFLECTOR);
    LIMIT(World.items[ITEM_PHASING].limit, 0, MAX_PHASING);
    LIMIT(World.items[ITEM_HYPERJUMP].limit, 0, MAX_HYPERJUMP);
    LIMIT(World.items[ITEM_MIRROR].limit, 0, MAX_MIRROR);
    LIMIT(World.items[ITEM_ARMOR].limit, 0, MAX_ARMOR);

    for (i = 0; i < NUM_ITEMS; i++) {
	LIMIT(World.items[i].initial, 0, World.items[i].limit);
    }

    CLR_BIT(DEF_HAVE,
	HAS_CLOAKING_DEVICE |
	HAS_EMERGENCY_THRUST |
	HAS_EMERGENCY_SHIELD |
	HAS_PHASING_DEVICE |
	HAS_TRACTOR_BEAM |
	HAS_AUTOPILOT |
	HAS_DEFLECTOR |
	HAS_MIRROR |
	HAS_ARMOR);

    if (World.items[ITEM_CLOAK].initial > 0)
	SET_BIT(DEF_HAVE, HAS_CLOAKING_DEVICE);
    if (World.items[ITEM_EMERGENCY_THRUST].initial > 0)
	SET_BIT(DEF_HAVE, HAS_EMERGENCY_THRUST);
    if (World.items[ITEM_EMERGENCY_SHIELD].initial > 0)
	SET_BIT(DEF_HAVE, HAS_EMERGENCY_SHIELD);
    if (World.items[ITEM_PHASING].initial > 0)
	SET_BIT(DEF_HAVE, HAS_PHASING_DEVICE);
    if (World.items[ITEM_TRACTOR_BEAM].initial > 0)
	SET_BIT(DEF_HAVE, HAS_TRACTOR_BEAM);
    if (World.items[ITEM_AUTOPILOT].initial > 0)
	SET_BIT(DEF_HAVE, HAS_AUTOPILOT);
    if (World.items[ITEM_DEFLECTOR].initial > 0)
	SET_BIT(DEF_HAVE, HAS_DEFLECTOR);
    if (World.items[ITEM_MIRROR].initial > 0)
	SET_BIT(DEF_HAVE, HAS_MIRROR);
    if (World.items[ITEM_ARMOR].initial > 0)
	SET_BIT(DEF_HAVE, HAS_ARMOR);
}


void Set_misc_item_limits(void)
{
    LIMIT(dropItemOnKillProb, 0.0, 1.0);
    LIMIT(detonateItemOnKillProb, 0.0, 1.0);
    LIMIT(movingItemProb, 0.0, 1.0);
    LIMIT(randomItemProb, 0.0, 1.0);
    LIMIT(destroyItemInCollisionProb, 0.0, 1.0);

    LIMIT(itemConcentratorRadius, 1, World.diagonal);
    LIMIT(itemConcentratorProb, 0.0, 1.0);

    LIMIT(asteroidItemProb, 0.0, 1.0);

    if (asteroidMaxItems < 0)
	asteroidMaxItems = 0;
}


/*
 * First time initialization of all global item stuff.
 */
void Set_world_items(void)
{
    Init_item(ITEM_FUEL, 0, 0);
    Init_item(ITEM_TANK, 1, 1);
    Init_item(ITEM_ECM, 1, 1);
    Init_item(ITEM_ARMOR, 1, 1);
    Init_item(ITEM_MINE, 1, maxMinesPerPack);
    Init_item(ITEM_MISSILE, 1, maxMissilesPerPack);
    Init_item(ITEM_CLOAK, 1, 1);
    Init_item(ITEM_SENSOR, 1, 1);
    Init_item(ITEM_WIDEANGLE, 1, 1);
    Init_item(ITEM_REARSHOT, 1, 1);
    Init_item(ITEM_AFTERBURNER, 1, 1);
    Init_item(ITEM_TRANSPORTER, 1, 1);
    Init_item(ITEM_MIRROR, 1, 1);
    Init_item(ITEM_DEFLECTOR, 1, 1);
    Init_item(ITEM_HYPERJUMP, 1, 1);
    Init_item(ITEM_PHASING, 1, 1);
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
		OBJ_SHOT|OBJ_CANNON_SHOT|OBJ_SMART_SHOT
		|OBJ_TORPEDO|OBJ_HEAT_SHOT|OBJ_PULSE);
    if (!BIT(World.rules->mode, PLAYER_SHIELDING))
	CLR_BIT(DEF_HAVE, HAS_SHIELD);

    DEF_USED &= DEF_HAVE;
}

void Set_world_asteroids(void)
{
    World.asteroids.num = 0;
    Tune_asteroid_prob();
}

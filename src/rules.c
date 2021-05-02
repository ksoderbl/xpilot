/* $Id: rules.c,v 3.28 1994/05/23 19:23:26 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-94 by
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

#define SERVER
#include <stdlib.h>
#include "global.h"
#include "map.h"
#include "rules.h"
#include "bit.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: rules.c,v 3.28 1994/05/23 19:23:26 bert Exp $";
#endif


long	KILLING_SHOTS = (OBJ_SHOT|OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_HEAT_SHOT);
long	DEF_BITS = 0;
long	KILL_BITS = (THRUSTING|PLAYING|KILLED|SELF_DESTRUCT|PAUSE);
long	DEF_HAVE =
	(OBJ_SHOT|OBJ_SHIELD|OBJ_COMPASS|OBJ_REFUEL|OBJ_REPAIR|OBJ_CONNECTOR
	|OBJ_FIRE|OBJ_LASER);
long	DEF_USED = (OBJ_SHIELD|OBJ_COMPASS);
long	USED_KILL = (OBJ_REFUEL|OBJ_REPAIR|OBJ_CONNECTOR|OBJ_FIRE|OBJ_LASER);


/*
 * Convert between probability for something to happen a given second on
 * a given block, to chance for such an event to happen on any block this
 * tick.
 */
void Init_item(int item, float prob)
{
    float	max = itemProbMult * maxItemDensity * World.x * World.y;

    if (itemProbMult * prob > 0) {
	World.items[item].chance = (int)(1.0
	    / (itemProbMult * prob * World.x * World.y * FPS));
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

    World.items[item].num = 0;
}


void Set_initial_resources(void)
{
    CLR_BIT(DEF_HAVE,
	OBJ_CLOAKING_DEVICE |
	OBJ_EMERGENCY_THRUST |
	OBJ_EMERGENCY_SHIELD |
	OBJ_TRACTOR_BEAM |
	OBJ_AUTOPILOT);

    if (initialCloaks > 0)
	SET_BIT(DEF_HAVE, OBJ_CLOAKING_DEVICE);
    if (initialEmergencyThrusts > 0)
	SET_BIT(DEF_HAVE, OBJ_EMERGENCY_THRUST);
    if (initialEmergencyShields > 0)
	SET_BIT(DEF_HAVE, OBJ_EMERGENCY_SHIELD);
    if (initialTractorBeams > 0)
	SET_BIT(DEF_HAVE, OBJ_TRACTOR_BEAM);
    if (initialAutopilots > 0)
	SET_BIT(DEF_HAVE, OBJ_AUTOPILOT);
    LIMIT(initialTanks, 0, MAX_TANKS);
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
	   | (identifyMines ? IDENTIFY_MINES : 0)
	   | (allowNukes ? ALLOW_NUKES : 0)
	   | (allowClusters ? ALLOW_CLUSTERS : 0)
	   | (allowModifiers ? ALLOW_MODIFIERS : 0)
	   | (allowLaserModifiers ? ALLOW_LASER_MODIFIERS : 0)
	   | (edgeWrap ? WRAP_PLAY : 0));
	rules.lives = worldLives;
	World.rules = &rules;

    if (!BIT(World.rules->mode, PLAYER_KILLINGS))
	CLR_BIT(KILLING_SHOTS,
		OBJ_SHOT|OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_HEAT_SHOT);
    if (!BIT(World.rules->mode, PLAYER_SHIELDING))
	CLR_BIT(DEF_HAVE, OBJ_SHIELD);

    /*
     * Initializes special items.  First parameter is type, second is
     * maximum number in the world at any time, third is frequency.
     */
    Init_item(ITEM_ENERGY_PACK, itemEnergyPackProb);
    Init_item(ITEM_TANK, itemTankProb);
    Init_item(ITEM_ECM, itemECMProb);
    Init_item(ITEM_MINE_PACK, itemMineProb);
    Init_item(ITEM_ROCKET_PACK, itemMissileProb);
    Init_item(ITEM_CLOAKING_DEVICE, itemCloakProb);
    Init_item(ITEM_SENSOR_PACK, itemSensorProb);
    Init_item(ITEM_WIDEANGLE_SHOT, itemWideangleProb);
    Init_item(ITEM_BACK_SHOT, itemRearshotProb);
    Init_item(ITEM_AFTERBURNER, itemAfterburnerProb);
    Init_item(ITEM_TRANSPORTER, itemTransporterProb);
    Init_item(ITEM_LASER, itemLaserProb);
    Init_item(ITEM_EMERGENCY_THRUST, itemEmergencyThrustProb);
    Init_item(ITEM_EMERGENCY_SHIELD, itemEmergencyShieldProb);
    Init_item(ITEM_TRACTOR_BEAM, itemTractorBeamProb);
    Init_item(ITEM_AUTOPILOT, itemAutopilotProb);
    DEF_USED &= DEF_HAVE;

    /*
     * Convert from [0..1] probabilities to [0..127] probabilities
     */
    LIMIT(dropItemOnKillProb, 0.0, 1.0);
    LIMIT(movingItemProb, 0.0, 1.0);
    ThrowItemOnKillRand = (int)(dropItemOnKillProb * 128);
    DetonateItemOnKillRand = (int)(detonateItemOnKillProb * 128);
    MovingItemsRand = (int)(movingItemProb * 128);

    Set_initial_resources();
}


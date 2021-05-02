/* $Id: rules.c,v 3.5 1993/08/02 12:55:33 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include "global.h"
#include "map.h"
#include "rules.h"
#include "bit.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: rules.c,v 3.5 1993/08/02 12:55:33 bjoerns Exp $";
#endif


long	KILLING_SHOTS =
          (OBJ_SHOT|OBJ_CANNON_SHOT|OBJ_SMART_SHOT
	   |OBJ_TORPEDO|OBJ_HEAT_SHOT|OBJ_NUKE);
long	DEF_BITS = (VELOCITY_GAUGE|FUEL_GAUGE|POWER_GAUGE);
long	KILL_BITS = (THRUSTING|PLAYING|KILLED|SELF_DESTRUCT|PAUSE);
long	DEF_HAVE =
	(OBJ_SHOT|OBJ_SHIELD|OBJ_COMPASS|OBJ_REFUEL|OBJ_CONNECTOR|OBJ_FIRE);
long	DEF_USED = (OBJ_SHIELD|OBJ_COMPASS);
long	USED_KILL = (OBJ_REFUEL|OBJ_CONNECTOR|OBJ_FIRE);


/*
 * Convert between probability for something to happen a given second on
 * a given block, to chance for such an event to happen on any block this
 * tick.
 */
void Init_item(int item, float prob)
{
    float	max = maxItemDensity * World.x * World.y;

    if (prob > 0) {
	World.items[item].chance = 1.0 / (prob * World.x * World.y * FPS);
	World.items[item].chance = MAX(World.items[item].chance, 1);
    } else {
	World.items[item].chance = 0.0;
    }
    if (max > 0) {
	if (max < 1)
	    World.items[item].max = 1;
	else
	    World.items[item].max = max;
    } else
	World.items[item].max = 0;

    World.items[item].num = 0;
}


void Set_world_rules(void)
{
    static rules_t rules;

	rules.mode = 
	  ((crashWithPlayer ? CRASH_WITH_PLAYER : 0)
	   | (playerKillings ? PLAYER_KILLINGS : 0)
	   | (playerShielding ? PLAYER_SHIELDING : 0)
	   | (limitedVisibility ? LIMITED_VISIBILITY : 0)
	   | (limitedLives ? LIMITED_LIVES : 0)
	   | (teamPlay ? TEAM_PLAY : 0)
	   | (onePlayerOnly ? ONE_PLAYER_ONLY : 0)
	   | (timing ? TIMING : 0)
	   | (allowNukes ? ALLOW_NUKES : 0)
	   | (edgeWrap ? WRAP_PLAY : 0));
	rules.lives = worldLives;
	World.rules = &rules;

    if (!BIT(World.rules->mode, PLAYER_KILLINGS))
	CLR_BIT(KILLING_SHOTS,
		OBJ_SHOT|OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_HEAT_SHOT|OBJ_NUKE);
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
    DEF_USED &= DEF_HAVE;

    /*
     * Convert from [0..1] probabilities to [0..127] probabilities
     */
    ThrowItemOnKillRand = dropItemOnKillProb * 128;
    MovingItemsRand = movingItemProb * 128;
}

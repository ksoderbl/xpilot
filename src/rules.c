/* rules.c,v 1.11 1992/06/28 05:38:29 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include "global.h"
#include "map.h"
#include "rules.h"

#ifndef	lint
static char sourceid[] =
    "@(#)rules.c,v 1.11 1992/06/28 05:38:29 bjoerns Exp";
#endif


#define INIT_ITEM(item, m, c)		\
{					\
    World.items[item].max = m;		\
    World.items[item].chance = c;	\
    World.items[item].num = 0;		\
}


long	KILLING_SHOTS = (OBJ_SHOT|OBJ_CANNON_SHOT|OBJ_SMART_SHOT);
long	DEF_BITS = (ID_MODE|VELOCITY_GAUGE|FUEL_GAUGE|POWER_GAUGE);
long	KILL_BITS = (THRUSTING|PLAYING|KILLED|SELF_DESTRUCT|PAUSE);
long	DEF_HAVE = (OBJ_SHOT|OBJ_SHIELD|OBJ_COMPASS|OBJ_REFUEL);
long	DEF_USED = (OBJ_SHIELD|OBJ_COMPASS);
long	USED_KILL = (OBJ_REFUEL);


rules_t Rules[] = {
    { 0, (CRASH_WITH_PLAYER|PLAYER_KILLINGS|
	  PLAYER_SHIELDING|LIMITED_VISIBILITY|TEAM_PLAY) },	/* CUSTOM */
    { 1, (LIMITED_LIVES|PLAYER_SHIELDING|ONE_PLAYER_ONLY) },	/* ADVENTURE */
    { 0, (TIMING|ONE_PLAYER_ONLY) },				/* RACE */
    { 0, (TIMING) },						/* HEAT */
    { 0, (CRASH_WITH_PLAYER|PLAYER_KILLINGS|
	  PLAYER_SHIELDING|TEAM_PLAY) },			/* DOGFIGHT */
    { 3, (CRASH_WITH_PLAYER|PLAYER_KILLINGS|
	  LIMITED_LIVES|PLAYER_SHIELDING|TEAM_PLAY) }		/* LTD. LIVES */
};


void Set_world_rules(int rule_ind)
{
    if (rule_ind >= MAX_MODES) {
	error("World rule does not exist. Using CUSTOM.");
	rule_ind = 0;
    }
    
    World.rules = &Rules[rule_ind];
#ifndef	SILENT
    printf("Using world rule no. %d. ", rule_ind);
#endif

    if (!BIT(World.rules->mode, PLAYER_KILLINGS))
	CLR_BIT(KILLING_SHOTS, OBJ_SHOT|OBJ_SMART_SHOT);
    if (!BIT(World.rules->mode, PLAYER_SHIELDING))
	CLR_BIT(DEF_HAVE, OBJ_SHIELD);

    /*
     * Initializes special items.  First parameter is type, second is
     * maximum number in the world at any time, third is frequency.
     */
    INIT_ITEM(ITEM_SMART_SHOT_PACK, 6, 999);
    INIT_ITEM(ITEM_ENERGY_PACK, 9, 479);
    INIT_ITEM(ITEM_CLOAKING_DEVICE, 5, 1197);
    INIT_ITEM(ITEM_SENSOR_PACK, 5, 1197);
    INIT_ITEM(ITEM_WIDEANGLE_SHOT, 6, 711);
    INIT_ITEM(ITEM_REAR_SHOT, 6, 653);
    INIT_ITEM(ITEM_MINE_PACK, 12, 431);

    DEF_USED &= DEF_HAVE;
}

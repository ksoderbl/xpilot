/* rules.c,v 1.3 1992/05/11 15:31:33 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include "map.h"
#include "rules.h"

#define INIT_ITEM(item, m, c)		\
{					\
    World.items[item].max=m;		\
    World.items[item].chance=c;		\
    World.items[item].ant=0;		\
}

extern World_map World;

long	KILLING_SHOTS = (OBJ_SHOT|OBJ_CANNON_SHOT|OBJ_SMART_SHOT);
long	DEF_BITS = (ID_MODE|VELOCITY_GAUGE|FUEL_GAUGE|POWER_GAUGE|IN_USE);
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
	warn("Set_world_rule: World rule does not exist. Using CUSTOM.\n");
	rule_ind=0;
    }
    
    World.rules=&Rules[rule_ind];
    printf("Using world rule no. %d. ", rule_ind);

    if (!BIT(World.rules->mode, PLAYER_KILLINGS))
	CLR_BIT(KILLING_SHOTS, OBJ_SHOT|OBJ_SMART_SHOT);
    if (!BIT(World.rules->mode, PLAYER_SHIELDING))
	CLR_BIT(DEF_HAVE, OBJ_SHIELD);

    INIT_ITEM(ITEM_SMART_SHOT_PACK, 6, 999);
    INIT_ITEM(ITEM_ENERGY_PACK, 9, 479);
    INIT_ITEM(ITEM_CLOAKING_DEVICE, 5, 1197);
    INIT_ITEM(ITEM_WIDEANGLE_SHOT, 6, 711);
    INIT_ITEM(ITEM_REAR_SHOT, 6, 653);
    INIT_ITEM(ITEM_MINE_PACK, 12, 431);

    DEF_USED &= DEF_HAVE;
}

/* $Id: rules.c,v 1.14 1992/08/27 00:26:10 bjoerns Exp $
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
    "@(#)$Id: rules.c,v 1.14 1992/08/27 00:26:10 bjoerns Exp $";
#endif


#define INIT_ITEM(item, m, c)		\
{					\
    World.items[item].max = m;		\
    World.items[item].ref_chance = c;	\
    World.items[item].num = 0;		\
}


long	KILLING_SHOTS =
          (OBJ_SHOT|OBJ_CANNON_SHOT|OBJ_SMART_SHOT|OBJ_DUST|OBJ_TORPEDO|OBJ_HEAT_SHOT);
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


void UpdateItemChances(int num_players)
{ 
    int i;
    for (i=0;i<NUM_ITEMS;i++) {
        int j;
        
        World.items[i].chance = World.items[i].ref_chance;
        for (j=num_players;
             j--;
             World.items[i].chance = ( World.items[i].chance
                                     *(100-PLAYER_ITEM_RATE))
                                    /100);
        if (!World.items[i].chance) World.items[i].chance = 1;
    }
}


void Set_world_rules(int rule_ind)
{
    if (rule_ind >= MAX_MODES) {
	error("World rule does not exist. Using CUSTOM.");
	rule_ind = 0;
    }
    
    World.rules = &Rules[rule_ind];
#ifndef	SILENT
    printf("Using world rule no. %d.\n", rule_ind);
#endif

    if (!BIT(World.rules->mode, PLAYER_KILLINGS))
	CLR_BIT(KILLING_SHOTS, OBJ_SHOT|OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_HEAT_SHOT);
    if (!BIT(World.rules->mode, PLAYER_SHIELDING))
	CLR_BIT(DEF_HAVE, OBJ_SHIELD);

    /*
     * Initializes special items.  First parameter is type, second is
     * maximum number in the world at any time, third is frequency.
     */
    
    INIT_ITEM(ITEM_ENERGY_PACK, 9, 479);
    INIT_ITEM(ITEM_TANK, 6, 394);
    INIT_ITEM(ITEM_ECM, 5, 450);
    INIT_ITEM(ITEM_MINE_PACK, 12, 250);
    INIT_ITEM(ITEM_SMART_SHOT_PACK, 6, 359);
    INIT_ITEM(ITEM_CLOAKING_DEVICE, 5, 1197);
    INIT_ITEM(ITEM_SENSOR_PACK, 5, 1197);
    INIT_ITEM(ITEM_WIDEANGLE_SHOT, 6, 711);
    INIT_ITEM(ITEM_REAR_SHOT, 6, 953);
    INIT_ITEM(ITEM_AFTER_BURNER, 6, 494);

    UpdateItemChances(1);

    DEF_USED &= DEF_HAVE;
}

#ifndef _sound_h
#define _sound_h

#ifndef SOUND

/*
 * Define like this to avoid having to put #ifdef SOUND
 * all over the place.
 */
#define sound_player_init(player)
#define sound_play_player(player, index)
#define sound_play_all(index)
#define sound_play_sensors(x, y, index)

#else /* SOUND */

#include "draw.h"

/*
 * Sound event indexes.  The order of this table must
 * be the same as the sound event table in sound.c.
 */ 
enum
{
    START_SOUND,
    FIRE_SHOT_SOUND,
    FIRE_TORPEDO_SOUND,
    FIRE_HEAT_SHOT_SOUND,
    FIRE_SMART_SHOT_SOUND,
    PLAYER_EXPLOSION_SOUND,
    PLAYER_HIT_PLAYER_SOUND,
    PLAYER_HIT_CANNON_SOUND,
    PLAYER_HIT_MINE_SOUND,
    PLAYER_EAT_TORPEDO_SHOT_SOUND,
    PLAYER_EAT_HEAT_SHOT_SOUND,
    PLAYER_EAT_SMART_SHOT_SOUND,
    DROP_MINE_SOUND,
    PLAYER_HIT_WALL_SOUND,
    WORM_HOLE_SOUND,
    WIDEANGLE_SHOT_PICKUP_SOUND,
    SENSOR_PACK_PICKUP_SOUND,
    REAR_SHOT_PICKUP_SOUND,
    SMART_SHOT_PACK_PICKUP_SOUND,
    CLOAKING_DEVICE_PICKUP_SOUND,
    ENERGY_PACK_PICKUP_SOUND,
    MINE_PACK_PICKUP_SOUND,
    REFUEL_SOUND,
    THRUST_SOUND,
    CLOAK_SOUND,
    CHANGE_HOME_SOUND,
    ECM_PICKUP_SOUND,
    AFTER_BURNER_PICKUP_SOUND,
    TANK_PICKUP_SOUND,
    DROP_MOVING_MINE_SOUND,
    MINE_EXPLOSION_SOUND,
    ECM_SOUND,
    TANK_DETACH_SOUND,
    CANNON_FIRE_SOUND,
    PLAYER_SHOT_THEMSELF_SOUND,
    DECLARE_WAR_SOUND,
    PLAYER_HIT_CANNONFIRE_SOUND,
    OBJECT_EXPLOSION_SOUND,
    PLAYER_EAT_SHOT_SOUND,
    MAX_SOUNDS,
};

#define SOUND_RANGE_FACTOR	0.5	/* factor to increase sound range by */
#define SOUND_DEFAULT_RANGE	CENTER			/* from draw.h (384) */
#define SOUND_MAX_VOLUME	RPLAY_DEFAULT_VOLUME	/* from rplay.h(127) */
#define SOUND_MIN_VOLUME	10

#define sound_range(pl) \
	(SOUND_DEFAULT_RANGE + pl->sensors\
	 * SOUND_DEFAULT_RANGE * SOUND_RANGE_FACTOR)

int	sound_lookup(char *);
void	sound_init(void);
int	sound_player_init(player *);
void	sound_play_player(player *, int);
void	sound_play_all(int);
int	sound_read(char *);
void	sound_play_sensors(float, float, int);

#endif /* SOUND */

#endif /* _sound_h */

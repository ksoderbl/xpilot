#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include "global.h"
#include "const.h"
#include "limits.h"
#include "rplay.h"
#include "sound.h"

#ifdef SOUND

/*
 * The sound table
 */
static RPLAY	*sound_table[MAX_SOUNDS];

/*
 * The sound event table.  The order of this table must be 
 * the same as the sound event indexes in sound.h.
 */
static char	*sound_events[MAX_SOUNDS] =
{
	"start",
	"fire_shot",
	"fire_torpedo",
	"fire_heat_shot",
	"fire_smart_shot",
	"player_explosion",
	"player_hit_player",
	"player_hit_cannon",
	"player_hit_mine",
	"player_eat_torpedo_shot",
	"player_eat_heat_shot",
	"player_eat_smart_shot",
	"drop_mine",
	"player_hit_wall",
	"worm_hole",
	"wideangle_shot_pickup",
	"sensor_pack_pickup",
	"rear_shot_pickup",
	"smart_shot_pickup",
	"cloaking_device_pickup",
	"energy_pack_pickup",
	"mine_pack_pickup",
	"refeul",
	"thrust",
	"cloak",
	"change_home",
	"ecm_pickup",
	"after_burner_pickup",
	"tank_pickup",
	"drop_moving_mine",
	"mine_explosion",
	"ecm",
	"tank_detach",
	"cannon_fire",
	"player_shot_themself",
	"declare_war",
	"player_hit_cannonfire",
	"object_explosion",
	"player_eat_shot",
};

/*
 * Lookup a sound event in the sound_events table.
 * The table index is returned or -1 if not found.
 */
static int	sound_lookup(char *event)
{
	int	i;

	for (i = 0; i < MAX_SOUNDS; i++)
	{
		if (strcmp(event, sound_events[i]) == 0)
		{
			return i;
		}
	}

	return -1;
}

/*
 * Initialize the sound table
 */
static void	sound_init(void)
{
	int	i;

	for (i = 0; i < MAX_SOUNDS; i++)
	{
		sound_table[i] = NULL;
	}
}

/*
 * Initialize sound for a player.
 * sound_init() and sound_read() are also called here if necessary.
 */
int	sound_player_init(player *p)
{
	char		*rindex(), *q;
	char		host[MAXHOSTNAMELEN];
	static int	sounds_initialized;

	if (!sounds_initialized)
	{
    		sound_init();
		if (sound_read(SOUNDFILE) < 0)
		{
			fprintf(stderr, "Cannot open sound file %s.\n", SOUNDFILE);
			exit(1);
		}
		sounds_initialized = 1;
	}

	strcpy(host, p->disp->display_name);
	q = rindex(host, ':');
	*q = '\0';
	if (*host == '\0')
	{
		strcat(host, "localhost");
	}

	p->rplay_fd = rplay_open(host);
	if (p->rplay_fd < 0)
	{
		rplay_perror(host);
		return -1;
	}

	return 0;
}

/*
 * Play a sound for a player.
 */
void	sound_play_player(player *p, int index)
{
	if (p->rplay_fd > 0 && sound_table[index])
	{
		rplay(p->rplay_fd, sound_table[index]);
	}
}

/*
 * Play a sound for all players.
 */
void	sound_play_all(int index)
{
	int	i;

	for (i = 0; i < NumPlayers; i++)
	{
		if (Players[i]->rplay_fd > 0 && sound_table[index])
		{
			rplay(Players[i]->rplay_fd, sound_table[index]);
		}
	}
}

/*
 * Play a sound if location is within player's sound range.
 * A player's sound range depends on the number of sensors they have.
 * The default sound range is what the player can see on the screen.
 * A volume is assigned to the sound depending on the location within
 * the sound range.
 */
void	sound_play_sensors(float x, float y, int index)
{
	int	i, volume;
	float	dx, dy, range, factor;
	player	*pl;

	for (i = 0; i < NumPlayers; i++)
	{
		pl = Players[i];
		dx = ABS(pl->pos.x - x);
		dy = ABS(pl->pos.y - y);
		range = sound_range(pl);
		if (dx >= 0 && dx <= range && dy >= 0 && dy <= range && 
			pl->rplay_fd > 0 && sound_table[index])
		{
			/*
			 * scale the volume
			 */
			factor = MAX(dx, dy)/range;
			volume = MAX(SOUND_MAX_VOLUME - SOUND_MAX_VOLUME*factor, SOUND_MIN_VOLUME);
			rplay_set(sound_table[index], RPLAY_CHANGE, 0,
				RPLAY_VOLUME,	volume,
				NULL);
			rplay(pl->rplay_fd, sound_table[index]);
		}
	}
}

/*
 * Read the sound file.
 */
static int	sound_read(char *filename)
{
	FILE	*fp;
	char	buf[256], *event, *sound;
	int	index;

	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		return -1;
	}

	while (fgets(buf, sizeof(buf), fp))
	{
		if (buf[0] == '#')
		{
			continue;
		}

		event = strtok(buf, " \t");
		sound = strtok(NULL, " \t\n");

		index = sound_lookup(event);
		if (index < 0)
		{
			fprintf(stderr, "Unknown sound event '%s' (ignored)\n", event);
			continue;
		}

		sound_table[index] = rplay_create(RPLAY_PLAY);
		rplay_set(sound_table[index], RPLAY_INSERT, 0,
			RPLAY_SOUND,	strdup(sound),
			NULL);
	}

	fclose(fp);

	return 0;
}

#endif SOUND


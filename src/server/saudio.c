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
/* This piece of code was provided by Greg Renda (greg@ncd.com). */

#include <stdlib.h>
#include <stdio.h>

#define SERVER
#include "xpconfig.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "netserver.h"
#include "saudio.h"

#ifdef SOUND

#define SOUND_RANGE_FACTOR	0.5		/* factor to increase sound
						 * range by */
#define SOUND_DEFAULT_RANGE	(BLOCK_SZ*15)
#define SOUND_MAX_VOLUME	100
#define SOUND_MIN_VOLUME	10

#define sound_range(pl) \
	(SOUND_DEFAULT_RANGE + pl->item[ITEM_SENSOR]\
	 * SOUND_DEFAULT_RANGE * SOUND_RANGE_FACTOR)

typedef struct _AudioQRec
{
    int             index,
		    volume;
    struct _AudioQRec *next;
} AudioQRec, *AudioQPtr;


static void queue_audio(player * pl, int index, int volume)
{
    AudioQPtr       a, p, prev;

    p = prev = (AudioQPtr)pl->audio;

    while (p) {
	if (p->index == index) {	/* same sound already in queue */
	    if (p->volume < volume) {	/* weaker version: replace volume */
		p->volume = volume;
	    }
	    return;
	}
	prev = p;
	p = p->next;
    }

    /* not found in queue: add to end */
    if (!(a = (AudioQPtr) malloc(sizeof(AudioQRec))))
	return;

    a->index = index;
    a->volume = volume;
    a->next = NULL;

    if (prev)
	prev->next = a;
    else
	pl->audio = (void *) a;
}

int sound_player_init(player * pl)
{
    SDBG(printf("sound_player_init %p\n", pl);)

    pl->audio = NULL;

    if (pl->version < 0x3250) {
	SET_BIT(pl->status, WANT_AUDIO);
    }

    return 0;
}

/*
 * Set (or reset) a player status flag indicating
 * that a player wants (or doesn't want) sound.
 */
void sound_player_onoff(player *pl, bool valueOn)
{
    SDBG(printf("sound_player_onoff %p, %d\n", pl, onoff);)

    if (onoff) {
	if (!BIT(pl->status, WANT_AUDIO)) {
	    SET_BIT(pl->status, WANT_AUDIO);
	    sound_play_player(pl, START_SOUND);
	}
    }
    else {
	CLR_BIT(pl->status, WANT_AUDIO);
    }
}

/*
 * Play a sound for a player.
 */
void sound_play_player(player * pl, int index)
{
    SDBG(printf("sound_play_player %p, %d\n", pl, index);)

    if (BIT(pl->status, WANT_AUDIO)) {
	queue_audio(pl, index, 100);
    }
}

/*
 * Play a sound for all players.
 */
void sound_play_all(int index)
{
    int i;

    SDBG(printf("sound_play_all %d\n", index);)

    for (i = 0; i < NumPlayers; i++) {
	if (BIT(Players[i]->status, WANT_AUDIO)) {
	    sound_play_player(Players[i], index);
	}
    }
}

/*
 * Play a sound if location is within player's sound range. A player's sound
 * range depends on the number of sensors they have. The default sound range
 * is what the player can see on the screen. A volume is assigned to the
 * sound depending on the location within the sound range.
 */
void sound_play_sensors(DFLOAT x, DFLOAT y, int index)
{
    int             i,
		    volume;
    DFLOAT           dx,
		    dy,
		    range,
		    factor;
    player         *pl;

    SDBG(printf("sound_play_sensors %g, %g, %d\n", x, y, index);)

    for (i = 0; i < NumPlayers; i++) {
	pl = Players[i];

	if (!BIT(pl->status, WANT_AUDIO))
	    continue;

	dx = ABS(pl->pos.x - x);
	dy = ABS(pl->pos.y - y);
	range = sound_range(pl);

	if (dx >= 0 && dx <= range && dy >= 0 && dy <= range) {
	    /*
	     * scale the volume
	     */
	    factor = MAX(dx, dy) / range;
	    volume = (int)MAX(SOUND_MAX_VOLUME - SOUND_MAX_VOLUME * factor,
			 SOUND_MIN_VOLUME);
	    queue_audio(pl, index, volume);
	}
    }
}

void sound_play_queued(player * pl)
{
    AudioQPtr       p,
		    n;

    SDBG(printf("sound_play_sensors %p\n", pl);)

    p = (AudioQPtr)pl->audio;
    pl->audio = NULL;

    while (p) {
	n = p->next;
	Send_audio(pl->conn, p->index, p->volume);
	free(p);
	p = n;
    }
}

void sound_close(player * pl)
{
    AudioQPtr       p,
		    n;

    SDBG(printf("sound_close %p\n", pl);)

    p = (AudioQPtr)pl->audio;
    pl->audio = NULL;

    while (p) {
	n = p->next;
	free(p);
	p = n;
    }
}

#endif /* SOUND */

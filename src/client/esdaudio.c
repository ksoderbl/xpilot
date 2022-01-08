/* $Id: esdaudio.c,v 1.1 2002/08/04 11:04:58 kimiko Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
 *      Kimiko Koopman
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
/*
 * Esound audio driver.
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <esd.h>

#include "version.h"
#include "audio.h"

char audio_version[] = VERSION;
static int audioserver;

int audioDeviceInit(char *display)
{
    /*printf("esound audio: init\n");*/
    audioserver = esd_open_sound("");

    return 0;
}

void audioDevicePlay(char *filename, int type, int volume, void **private)
{
    if (((int *)private)[0] == 0) {
	/* sample has not been cached yet */
	((int *)private)[0] = esd_file_cache(audioserver, "", filename);
    }
    /*printf("esound audio: play file %s, type %d, vol %d, priv %d\n",
	   filename, type, volume, ((int *)private)[0]);*/

    volume = (int)(volume * 2.55);
    esd_set_default_sample_pan(audioserver, ((int *)private)[0], volume, volume);
    esd_sample_play(audioserver, ((int *)private)[0]);
}

void audioDeviceEvents()
{
    /* printf("esound audio: events\n"); */
}


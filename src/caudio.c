/* $Id: caudio.c,v 3.7 1993/09/13 19:09:07 bjoerns Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-93 by
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
/* This piece of code was provided by Greg Renda (greg@ncd.com). */
/*
 * client audio
 */

#ifdef SOUND

#define SOUNDDIR LIBDIR "sound/"

#define _CAUDIO_C_

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "types.h"
#include "audio.h"
#include "client.h"

static int	audioEnabled;

static struct {
    char	*filename;
    void	*private;
} table[MAX_SOUNDS];


void audioInit(char *display)
{
    FILE           *fp;
    char            buf[256], *file, *sound;
    int             i;

    if (!maxVolume || !(fp = fopen(sounds, "r")))
	return;

    while (fgets(buf, sizeof(buf), fp)) {
	/* ignore comments */
	if (*buf == '\n' || *buf == '#')
	    continue;

	sound = strtok(buf, " \t");
	file = strtok(NULL, " \t\n");

	for (i = 0; i < MAX_SOUNDS; i++)
	    if (!strcmp(sound, soundNames[i])) {
		if (*file == '/')
		    table[i].filename = strdup(file);
		else if (table[i].filename =
			 (char *)malloc(strlen(SOUNDDIR) + strlen(file) + 1)) {
		    strcpy(table[i].filename, SOUNDDIR);
		    strcat(table[i].filename, file);
		}
		break;
	    }

	if (i == MAX_SOUNDS)
	    fprintf(stderr, "Unknown sound '%s' (ignored)\n", sound);
    }

    fclose(fp);

    audioEnabled = !audioDeviceInit(audioServer ? audioServer : display);
}

void audioEvents()
{
    if (audioEnabled)
	audioDeviceEvents();
}

int Handle_audio(int type, int volume)
{
    if (!audioEnabled || !table[type].filename)
	return 0;

    if (!table[type].private) {
	int i;

	/* eliminate duplicate sounds */
	for (i = 0; i < MAX_SOUNDS; i++)
	    if (i != type && table[i].filename && table[i].private
		&& strcmp(table[type].filename, table[i].filename) == 0) {
		table[type].private = table[i].private;
		break;
	    }
    }

    audioDevicePlay(table[type].filename, type, MIN(volume, maxVolume),
		    &table[type].private);

    return 0;
}

#else

#if defined(__osf__) && defined(__alpha)
static int caudio_c_non_empty_kludge;	/* For DEC Alpha OSF/1 V1.2 */
#endif

#endif /* SOUND */

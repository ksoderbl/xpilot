/* $Id: caudio.c,v 5.0 2001/04/07 20:00:58 dik Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
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

#define	MAX_RANDOM_SOUNDS	6

#define _CAUDIO_C_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#include "version.h"
#include "config.h"
#include "const.h"
#include "types.h"
#include "audio.h"
#include "client.h"
#include "error.h"
#include "commonproto.h"

char caudio_version[] = VERSION;

static int	audioEnabled;

static struct {
    char	**filenames;
    void	**private;
    int		nsounds;
} table[MAX_SOUNDS];


void audioInit(char *display)
{
    FILE           *fp;
    char            buf[512], *file, *sound, *ifile, *p;
    int             i, j;

    if (!maxVolume) {
	printf("maxVolume is 0: no sound.\n");
	return;
    }
    if (!(fp = fopen(sounds, "r"))) {
	error("Could not open soundfile %s", sounds);
	return;
    }

    while (fgets(buf, sizeof(buf), fp)) {
	/* ignore comments */
	if (*buf == '\n' || *buf == '#')
	    continue;

	sound = strtok(buf, " \t");
	file = strtok(NULL, " \t\n");

	for (i = 0; i < MAX_SOUNDS; i++)
	    if (!strcmp(sound, soundNames[i])) {
		table[i].filenames = (char **)malloc(sizeof(char *) * MAX_RANDOM_SOUNDS);
		table[i].private = (void **)malloc(sizeof(void *) * MAX_RANDOM_SOUNDS);
		memset((char *) table[i].private, 0, sizeof(void *) * MAX_RANDOM_SOUNDS);
		ifile = strtok(file, " \t\n|");
		j = 0;
		while (ifile && j < MAX_RANDOM_SOUNDS) {
		    if (*ifile == '/')
			table[i].filenames[j] = xp_strdup(ifile);
		    else if (table[i].filenames[j] =
			     (char *)malloc(strlen(Conf_sounddir()) + strlen(ifile) + 1)) {
			strcpy(table[i].filenames[j], Conf_sounddir());
			strcat(table[i].filenames[j], ifile);
		    }
		    j++;
		    ifile = strtok(NULL, " \t\n|");
		    table[i].nsounds = j;
		}
		break;
	    }

	if (i == MAX_SOUNDS)
	    fprintf(stderr, "Unknown sound '%s' (ignored)\n", sound);

    }

    fclose(fp);

    audioEnabled = !audioDeviceInit(audioServer[0] ? audioServer : display);
}

void audioCleanup()
{
    /* release malloc'ed memory here */
    int			i;

    for (i = 0; i < MAX_SOUNDS; i++) {
	if (table[i].filenames) {
	    free(table[i].filenames);
	    table[i].filenames = NULL;
	}
	if (table[i].private) {
	    free(table[i].private);
	    table[i].private = NULL;
	}
    }
}

void audioEvents()
{
    if (audioEnabled)
	audioDeviceEvents();
}

int Handle_audio(int type, int volume)
{
    int		pick = 0;

    if (!audioEnabled || !table[type].filenames)
	return 0;

    if (table[type].nsounds > 1)
    {
	/*
	 * Multiple sounds were specified.  Pick one at random.
	 */
	pick = rand() % table[type].nsounds;
    }

    if (!table[type].private[pick]) {
	int i;

	/* eliminate duplicate sounds */
	for (i = 0; i < MAX_SOUNDS; i++)
	    if (i != type
		&& table[i].filenames
		&& table[i].private[pick]
		&& strcmp(table[type].filenames[0], table[i].filenames[0]) == 0)
	    {
		table[type].private[0] = table[i].private[0];
		break;
	    }
    }

    audioDevicePlay(table[type].filenames[pick], type, MIN(volume, maxVolume),
		    &table[type].private[pick]);

    return 0;
}

#else

#include "version.h"

char caudio_version[] = VERSION;

#endif /* SOUND */

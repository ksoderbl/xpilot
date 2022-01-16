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
/*
 * This piece of code was provided by Greg Renda (greg@ncd.com), but
 * rplay support was originally done by Mark Boyns (boyns@sdsu.edu).
 */
/*
 * RPlay audio driver.
 */

#include <stdlib.h>
#include <string.h>

#if !defined(_WINDOWS)
# include <netdb.h>
# include <sys/param.h>
#endif

#include "rplay.h"
#include "audio.h"
#include "commonproto.h"

static int fd;


int audioDeviceInit(char *display)
{
    char host[MAXHOSTNAMELEN], *p;

    strlcpy(host, display, sizeof(host));

    if ((p = strrchr(host, ':')) != NULL)
	*p = 0;

    if (!*host)
	strcat(host, "localhost");

    printf("Directing sound to: %s\n", host);

    if ((fd = rplay_open(host)) < 0) {
	rplay_perror(host);
	return -1;
    }

    return 0;
}

void audioDevicePlay(char *filename, int type, int volume, void **private_data)
{
    RPLAY **p = (RPLAY **)private_data;
    char *name;

    if (!*p) {
	if ((name = strrchr(filename, '/')) != NULL) {
	    name++;
	} else {
	    name = filename;
	}
	*p = rplay_create(RPLAY_PLAY);
	rplay_set(*p, RPLAY_INSERT, 0, RPLAY_SOUND, xp_strdup(name), NULL);
    }

    rplay_set(*p, RPLAY_CHANGE, 0, RPLAY_VOLUME, volume, NULL);
    rplay(fd, *p);
}

void audioDeviceEvents(void)
{
}

/* $Id: rplayaudio.c,v 3.4 1993/08/02 12:41:35 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 *
 *	This piece of code was provided by Greg Renda (greg@ncd.com), but
 *	rplay support was originally done by Mark Boyns (boyns@sdsu.edu).
 */
/*
 * RPlay audio driver.
 */

#include <string.h>
#include <sys/param.h>
#include "rplay.h"
#include "audio.h"

static int fd;


int audioDeviceInit(char *display)
{
    char host[MAXHOSTNAMELEN], *p;

    strcpy(host, display);

    if (p = strrchr(host, ':'))
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

void audioDevicePlay(char *filename, int type, int volume, void **private)
{
    RPLAY **p = (RPLAY **)private;
    char *name;

    if (!*p) {
	if ((name = strrchr(filename, '/')) != NULL) {
	    name++;
	} else {
	    name = filename;
	}
	*p = rplay_create(RPLAY_PLAY);
	rplay_set(*p, RPLAY_INSERT, 0, RPLAY_SOUND, strdup(name), NULL);
    }

    rplay_set(*p, RPLAY_CHANGE, 0, RPLAY_VOLUME, volume, NULL);
    rplay(fd, *p);
}

void audioDeviceEvents()
{
}

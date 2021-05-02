/*
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-94 by
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
/*
 * AFPlay audio driver by Tom De Pauw <tom@depauw.finning.wimsey.com>.
 */
/* 
 * DEC CRL's AudioFile is a network-transparent system for distributed
 * audio applications and supports Digital RISC systems running Ultrix,
 * Digital Alpha AXP systems running OSF/1, Sun Microsystems SPARCstations
 * running SunOS, and SGI Indigos.
 * 
 * I have finally ported support for AF to xpilot. It works on my DECstation
 * 5000/25 (a small X window MIPS 3000 based machine) with the following
 * minor inconveniences (AF experts, please read on):
 * 
 * 1) Sometimes a sound is lost (never played). Here is my opinion on the
 * cause. AFPlaySamples needs the audio device time at which the sound
 * is to be played. Since we want to play right away, we need to query
 * the AF server for the current time. We then present the sound to be
 * played with the current time. By that time, there is a chance (xpilot
 * is quite cpu intensive on DECstation 5000/25) that the current time is
 * in the past and the sound is thus never played. This should be a lesser
 * concern on larger cpus.
 * 
 * 2) Volume. xpilot specifies the volume the sound is to be played at
 * on a scale of 10-100. This value is converted linearly to a gain of
 * -25 to -5 in my interface. This seemed appropriate on my machine.
 * There seems to be a problem though. In my opinion, AFSetOutputGain
 * sets the gain on the master volume control. I.e. all sounds being played
 * are affected. I would like to specify the gain for a particular sound
 * only. Is this possible ?
 * 
 * Thanks to everyone I stealed code from.
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <sys/file.h>
#include <sys/limits.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/param.h>
#include <AF/AFlib.h>
#include "audio.h"

static int fd;
/*
 * Internal macro definitions:
 */

#define BUFFER_SIZE	(1024 * 32)
#define False 0
#define True 1
#define SPEAKER 0	/* 0 external handset, 1 for internal speaker */

/*
 * Internal variable declarations:
 */

int sample_sizes[] =
{
    1,  /* MU255 */
    1,  /* ALAW */
    2,  /* Linear PCM, 16 bits, -1.0 <= x < 1.0 */
    2,  /* Linear PCM, 32 bits, -1.0 <= x < 1.0 */
    1,  /* G.721, 64Kbps to/from 32Kbps. */
    1,  /* G.723, 64Kbps to/from 32Kbps. */
    0
};

static char		*buf;
static int		err;
static struct stat	st;
static int		nbytes;
static int		ssize;
static ATime		t, act, nact;
static AC		ac;
static AFAudioConn	*aud;


int audioDeviceInit(char *display)
{
    AFSetACAttributes	attributes;
    int			srate, device;
    unsigned int	channels;
    AEncodeType		type;

    device                      = SPEAKER;
    attributes.preempt          = Mix;
    attributes.start_timeout    = 0;
    attributes.end_silence      = 0;
    attributes.play_gain        = 0;
    attributes.rec_gain         = 0;

    if ((aud = AFOpenAudioConn("")) == NULL)
    {
	error ("Cannot open a connection to audio server.");
	return 1;
    }

    ac = AFCreateAC(aud, device, ACPlayGain, &attributes);
    srate       = ac->device->playSampleFreq;
    type        = ac->device->playBufType;
    channels    = ac->device->playNchannels;
    ssize       = sample_sizes[type] * channels;

    if ((buf = (char *) malloc(BUFFER_SIZE)) == NULL)
    {
	error ("Couldn't allocate a play buffer.");
	return 1;
    }

    /* Success in opening audio device */
    return 0;
}

void audioDevicePlay(char *filename, int type, int volume, void **private)
{
    int			fd;
    char		soundfile[1024];
    char		*str;
    int			gain;


    sprintf(soundfile, "%s", filename);

	    /* Open the sound file for reading */
    if ((fd = open(soundfile, O_RDONLY, 0)) < 0)
    {
	/* Issue an error about not opening sound file */
	error ("Unable to open sound file %s.", soundfile);
	return;
    }

    /* At this point, we're all ready to copy the data. */
    if ((nbytes = read(fd, (char *) buf, BUFFER_SIZE)) <= 0)
	return;

    gain = (int) ((((float) volume) - 10.0) * 20.0 / 90.0) - 25.0;
    AFSetOutputGain (ac, gain);
    t = AFGetTime(ac);
    do {
	nact = AFPlaySamples(ac, t, nbytes, buf);
	act = nact;
	t += nbytes;
    } while ((nbytes = read(fd, buf, BUFFER_SIZE)) != 0);

    /* Close the sound file */
    (void) close(fd);

}

void audioDeviceEvents()
{
}

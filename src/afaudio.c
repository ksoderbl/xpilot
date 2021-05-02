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
 * Hacked on for AF3 & cached audio by Lance Berc <berc@src.dec.com>.
 */
/*
 * DEC CRL's AudioFile is a public domain network-transparent system
 * for distributed audio applications and supports Digital RISC
 * systems running Ultrix, Digital Alpha AXP systems running OSF/1,
 * Sun Microsystems SPARCstations running SunOS, and SGI Indigos.
 * It is available via anonymous FTP from crl.dec.com and gatekeeper.dec.com.
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
 * concern on larger cpus. [ I added a 50msec delay to account for scheduling
 * jitter - lance ]
 *
 * 2) Volume. The obvious AFSetOutputGain() control isn't used because
 * (a) not all devices have hardware gain control, and (b) it doesn't
 * allow setting the gain on a per-spurt basis.  So we now use the software
 * mixer by setting the ACPlayGain attribute for each spurt.  The gain
 * heuristic is good for the speaker on my J-Video, but your milage may
 * vary.
 *
 * 3) This code relies on the samples being in 8kHz u-law format.
 *
 * Thanks to everyone I stole code from.
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <AF/AFlib.h>

#include "audio.h"

/* Keep a cache of recently played audio.  This really helps when the
 * client has the sound files mounted from a heavily loaded NFS server.
 */
#define CACHE_SIZE (512 * 1024)
#define CACHE_ENTRIES 64
#define SOUND_DELAY 50 /* delay playing a sound for 50msec for slow machines */

struct SoundCache {
  char *fn;
  unsigned char *sound;
  struct timeval when;
  int length;
};

static struct SoundCache soundCache[CACHE_ENTRIES];

static int soundCacheEntries = 0;
static int soundCacheBytes = 0;

static AC		ac;
static AFAudioConn	*aud;
static struct timeval	now;

static int FindDefaultDevice(aud)
AFAudioConn *aud;
{
  AFDeviceDescriptor *aDev;
  int     i;
  char *s;

  s = (char *) getenv("AF_DEVICE");
  if (s != NULL) return(atoi(s));

  /* Find the first 8kHz, mono device, non-phone device. */
  for(i=0; i < ANumberOfAudioDevices(aud); i++) {
    aDev = AAudioDeviceDescriptor(aud, i);
    if ((aDev->inputsFromPhone == 0) &&
	(aDev->outputsToPhone == 0) &&
	(aDev->playSampleFreq == 8000) &&
	(aDev->playNchannels == 1))
      return i;
  }
  return -1;
}



int audioDeviceInit(char *display)
{
    AFSetACAttributes	attributes;
    int			device;

    attributes.preempt          = Mix;
    attributes.start_timeout    = 0;
    attributes.end_silence      = 0;
    attributes.play_gain        = 0;
    attributes.rec_gain         = 0;
    attributes.type             = MU255;

    if ((aud = AFOpenAudioConn("")) == NULL)
    {
	error ("Cannot open a connection to audio server.");
	return 1;
    }

    device = FindDefaultDevice(aud);
    if (device == -1) {
	error ("Cannot find an 8kHz, mono, non-telephone device");
	AFCloseAudioConn(aud);
	return 1;
    }

    ac = AFCreateAC(aud, device, ACPlayGain | ACEncodingType, &attributes);

    /* Success in opening audio device */
    return 0;
}

tossOldestCacheEntry()
{
  struct timeval t;
  struct SoundCache *ce, *oldest;

  oldest = soundCache;
  if (oldest->when.tv_sec == 0) oldest->when = now;
  for (ce = soundCache; ce < &soundCache[CACHE_ENTRIES]; ce++) {
    if (ce->fn && timercmp(&ce->when, &oldest->when, <)) oldest = ce;
  }

  free(oldest->fn);
  free(oldest->sound);
  soundCacheBytes -= oldest->length;
  soundCacheEntries--;
  bzero((char *) oldest, sizeof(struct SoundCache));
}


struct SoundCache *newCacheEntry(char *fn)
{
  int fd;
  struct stat sbuf;
  struct SoundCache *ce;

  /* Open the sound file for reading */
  if ((fd = open(fn, O_RDONLY, 0)) < 0) {
    error ("Unable to open sound file %s.", fn);
    return NULL;
  }

  if (fstat(fd, &sbuf) == -1) {
    error("Unable to stat sound file %s.", fn);
    close(fd);
    return NULL;
  }

  /* Truncate huge sound files to the cache size */
  if (sbuf.st_size > CACHE_SIZE) sbuf.st_size = CACHE_SIZE;

  /* If the cache is full, throw out the oldest entry.  */
  while ((soundCacheEntries == CACHE_ENTRIES) ||
	 (soundCacheBytes + sbuf.st_size > CACHE_SIZE))
    tossOldestCacheEntry();

  /* Find an empty cache entry */
  for (ce = soundCache; ce < &soundCache[CACHE_ENTRIES]; ce++)
    if (!ce->fn) break;

  ce->fn = malloc(strlen(fn + 1));
  strcpy(ce->fn, fn);
  ce->sound = (unsigned char *) malloc(sbuf.st_size);
  if (read(fd, ce->sound, sbuf.st_size) != sbuf.st_size) {
    error("Unable to read sound file %s.", fn);
    free(ce->sound);
    free(ce->fn);
    ce->fn = NULL;
    ce->sound = NULL;
    close(fd);
    return(NULL);
  }

  close(fd);
  ce->length = sbuf.st_size;
  soundCacheBytes += sbuf.st_size;
  soundCacheEntries++;

  return(ce);
}


struct SoundCache *findCacheEntry(char *fn)
{
  struct SoundCache *ce;

  for (ce = soundCache ; ce < &soundCache[CACHE_ENTRIES]; ce++)
    if (ce->fn && !strcmp(fn, ce->fn))
      return ce;

  ce = newCacheEntry(fn);
  return(ce);
}

void audioDevicePlay(char *filename, int type, int volume, void **private)
{
    int			gain;
    AFSetACAttributes	acAttributes;
    struct SoundCache	*ce;
    ATime		t;

    if ((ce = findCacheEntry(filename)) == NULL) return;

    gettimeofday(&now, NULL);
    ce->when = now;
    /* this is a hacky, kludgey, way of doing things.  yuck.  but it works */
    /* Convert from 10 - 100 to -25 - -5 */
    gain = (((volume - 10) * 20) / 90) - 25;
    acAttributes.play_gain = gain;
    AFChangeACAttributes(ac, ACPlayGain, &acAttributes);

    t = AFGetTime(ac) + (8 * SOUND_DELAY); /* 8 samples/msec */
    t = AFPlaySamples(ac, t, ce->length, ce->sound);
}

void audioDeviceEvents()
{
}

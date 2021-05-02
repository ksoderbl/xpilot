/* $Id: afaudio.c,v 4.5 2001/03/25 17:24:50 bert Exp $
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
/*
 * History
 * 1993: AFPlay audio driver by Tom De Pauw <tom@finning.ca>.
 * 1994: Hacked on for AF3 & cached audio by Lance Berc <berc@src.dec.com>.
 * 22 Jan 1995: Skip sound headers if present. Tom De Pauw <tom@finning.ca>.
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
 * 3) This code skips a possible sound header in Sun or Inverted Sun
 * format. The rest of the sound file describes samples in 8kHz u-law format.
 * The sound files suggested for use with xpilot contain the headers.
 * AF was never intended to read these headers, they would produce
 * an audible click at the start of each sound. The headers are
 * assumed of fixed length. This seems the case in the sounds I looked at.
 * If this causes problem, the header length field will have to be read.
 *
 * Resemblance of this code to other programs is not coincidental.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <AF/AFlib.h>

#ifndef _WINDOWS
# include <sys/file.h>
#endif

#include "version.h"
#include "audio.h"

char audio_version[] = VERSION;

/* Keep a cache of recently played audio.  This really helps when the
 * client has the sound files mounted from a heavily loaded NFS server.
 */
#define CACHE_SIZE (512 * 1024)
#define CACHE_ENTRIES 64
#define SOUND_DELAY 50 /* delay playing a sound for 50msec for slow machines */

struct SoundCache {
    char		*fn;
    unsigned char	*sound;
    struct timeval	when;
    int			length;
};

typedef struct
{
    unsigned    magic;              /* Magic number       */
    unsigned    sample_rate;        /* Samples per second     */
    unsigned    samples_per_unit;   /* Samples per unit   */
    unsigned    bytes_per_unit;     /* Bytes per sample unit  */
    unsigned    channels;           /* # interleaved channels */
    unsigned    encoding;           /* Date encoding format   */
    unsigned    info1;              /* "info" field of unspecified nature */
    unsigned    info2;              /* (totalling hdr_size - 24) */
} Sun_Audio_Hdr;
#define SUN_MAGIC       ((unsigned long) 0x2e736e64)   /* Really '.snd' */
#define SUN_INV_MAGIC   ((unsigned long) 0x646e732e)
#define SUN_AUDIO_ENCODING_ULAW (1)

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
  for (i=0; i < ANumberOfAudioDevices(aud); i++) {
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
  Sun_Audio_Hdr header;

  /* Open the sound file for reading */
  if ((fd = open(fn, O_RDONLY, 0)) < 0) {
    error("Unable to open sound file %s.", fn);
    return NULL;
  }
  if (fstat(fd, &sbuf) == -1) {
    error("Unable to stat sound file %s.", fn);
    close(fd);
    return NULL;
  }

  /* If we have a Sun audio file, strip the header and adjust size. */
  if (read(fd, (char *)&header, sizeof(Sun_Audio_Hdr))
    < sizeof(Sun_Audio_Hdr)) {
    error("Warning: assuming no header in: %s.", fn);
    close(fd);
    fd = open(fn, O_RDONLY, 0);
  }
  else if (header.magic != SUN_MAGIC && header.magic != SUN_INV_MAGIC
    /*|| header.encoding != SUN_AUDIO_ENCODING_ULAW*/ ) {
    error("Warning: found %x, expected sound header %x or %x in %s.",
      header.magic, SUN_MAGIC, SUN_INV_MAGIC, fn);
    close(fd);
    fd = open(fn, O_RDONLY, 0);
  }
  else /* We have in fact found a header */
    sbuf.st_size -= sizeof(Sun_Audio_Hdr);

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

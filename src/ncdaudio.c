/* $Id: ncdaudio.c,v 3.3 1993/06/28 20:53:47 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 *
 *	This piece of code was provided by Greg Renda (greg@ncd.com).
 */
/*
 * NCDaudio driver
 */

#ifdef SOUND
#include <audio/audiolib.h>
#include <audio/soundlib.h>
#include "audio.h"

AuServer       *aud;
AuDeviceID      device;

int audioDeviceInit(char *display)
{
    int             i;

    if (!(aud = AuOpenServer(display, 0, NULL, 0, NULL, NULL)))
	return -1;

    for (i = 0; i < AuServerNumDevices(aud); i++)
	if ((AuDeviceKind(AuServerDevice(aud, i))
	     == AuComponentKindPhysicalOutput)
	    && AuDeviceNumTracks(AuServerDevice(aud, i)) == 1) {

	    device = AuDeviceIdentifier(AuServerDevice(aud, i));
	    break;
	}

    if (!device) {
	AuCloseServer(aud);
	return -1;
    }

#if defined(SOUNDLIB_VERSION) && SOUNDLIB_VERSION >= 2
    AuSoundRestartHardwarePauses = AuFalse;
#endif				/* defined(SOUNDLIB_VERSION) &&
				 * SOUNDLIB_VERSION >= 2 */

    return 0;
}


typedef struct
{
    int             playing;
    AuBucketID      bucket;
} audioRec, *audioPtr;

static void doneCB(AuServer * aud,
		   AuEventHandlerRec * handler,
		   AuEvent * event,
		   audioPtr info)
{
    info->playing = 0;
}

void audioDevicePlay(char *filename, int type, int volume, void **private)
{
    audioPtr       *info = (audioPtr *) private;

    if (!*info) {
	if (!(*info = (audioPtr) malloc(sizeof(audioRec))))
	    return;

	(*info)->playing = 0;
	(*info)->bucket
	    = AuSoundCreateBucketFromFile(aud, filename, AuAccessAllMasks,
					  NULL, NULL);
    }

    if ((*info)->bucket && (!(*info)->playing || type != FIRE_SHOT_SOUND)) {
	(*info)->playing = 1;
	AuSoundPlayFromBucket(aud, (*info)->bucket, device,
			      AuFixedPointFromFraction(volume, 100),
			      (void (*)) doneCB, (AuPointer) *info, 1, NULL,
			      NULL, NULL, NULL);
	AuFlush(aud);
    }
}

void audioDeviceEvents()
{
    if (aud)
	AuHandleEvents(aud);
}

#endif /* SOUND */

/* $Id: hpaudio.c,v 5.1 2001/05/12 18:07:26 bertg Exp $
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
 * Audio driver for HP-UX Aserver by Bert Gijsbers <bert@xpilot.org>.
 * Note that the Aserver and HP audio system seems to be a misdesign.
 * At least I'm not able to get XPilot to play sounds on it satisfactorily and
 * others have reported similar difficulties getting sounds to work on HPs.
 * If you manage to get it working allright please let me know.
 * The problem is that sounds are lagging behind all the time.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef _WINDOWS
# include <unistd.h>
#endif

#include "version.h"

char audio_version[] = VERSION;

/*
 * There is an include file included by Alib.h containing
 * dollar signs in identifiers.  Rather than confusing users
 * by obscure compiler options we hardcode here the only thing
 * needed from that include file.
 */
typedef struct { unsigned short data_offset; } *handle_t;
#define idl_base
#include <audio/Alib.h>


#include "audio.h"


static Audio		*audio;
static AGainDB		min_gain, max_gain;
static SBPlayParams	param;
static AGainEntry	gain_entry[1];
static SBucket *const	bad_bucket = (SBucket *)-1;
static SBucket		*play_bucket;
static AEventMask	emask;
static ATransID		xid;

int audioDeviceInit(char *display)
{
    char		*spk;
    long		status;

    if (strncmp(display, "unix:", 5) == 0) {
	display += 5;
    }
    audio = AOpenAudio(display, &status);
    if (!audio) {
	fprintf(stderr, "Can't open audio connection to Aserver %s (error %ld)\n",
		display, status);
	return -1;
    }

    min_gain = AMinOutputGain(audio);
    max_gain = AMaxOutputGain(audio);

    emask = ATransCompletedMask | ATransStoppedMask | ATransPreemptedMask;

    spk = getenv("SPEAKER");
    gain_entry[0].u.o.out_ch = AOCTMono;
    gain_entry[0].gain = AUnityGain;
    gain_entry[0].u.o.out_dst = (!spk || *spk == 'i' || *spk == 'I')
				? AODTMonoIntSpeaker
				: AODTMonoJack;

    param.priority = APriorityNormal;
    param.gain_matrix.type = AGMTOutput;
    param.gain_matrix.num_entries = 1;
    param.gain_matrix.gain_entries = gain_entry;
    param.play_volume = min_gain;
    param.pause_first = False;
    param.start_offset.type = ATTSamples;
    param.start_offset.u.samples = 0;
    param.duration.type = ATTFullLength;
    param.loop_count = 1;
    param.previous_transaction = 0;
    param.event_mask = emask;

    return 0;
}

void audioDevicePlay(char *filename, int type, int volume, void **private)
{
    SBucket		**bucket_ptr = (SBucket **)private;
    long		status = AENoError;
    char		buf[256];
    int			vol;
    static int		shall_i_play[MAX_SOUNDS];

    if (!*bucket_ptr) {
	*bucket_ptr = ALoadAFile(audio, filename, AFFUnknown, 0, NULL, &status);
	if (status != AENoError) {
	    AGetErrorText(audio, status, buf, sizeof buf);
	    fprintf(stderr, "Can't load audio file %s: %s\n", filename, buf);
	    *bucket_ptr = bad_bucket;
	}
    }
    if (*bucket_ptr == bad_bucket) {
	return;
    }
    vol = min_gain + (volume * (max_gain - min_gain)) / SOUND_MAX_VOLUME;
    if (vol < param.play_volume) {
	return;
    }

    /* avoid the congestions of sound */
    if (type ==  FIRE_SHOT_SOUND ||
    	type == FIRE_TORPEDO_SOUND ||
    	type == FIRE_HEAT_SHOT_SOUND ||
    	type == FIRE_SMART_SHOT_SOUND ||
    	type == THRUST_SOUND ||
    	type == PLAYER_HIT_PLAYER_SOUND ||
    	type == TRACTOR_BEAM_SOUND ||
   	type == PRESSOR_BEAM_SOUND) {
	    if ((shall_i_play[type]++) % 3) {
		return;
	    }
 	}

    param.priority = (volume > 3 * SOUND_MAX_VOLUME / 4) ? APriorityUrgent
		   : (volume > 2 * SOUND_MAX_VOLUME / 4) ? APriorityHigh
		   : (volume > 1 * SOUND_MAX_VOLUME / 4) ? APriorityNormal
		   : APriorityLow;
    param.play_volume = vol;
    play_bucket = *bucket_ptr;
}

void audioDeviceEvents(void)
{
    long		status;
    static int		pending;
    AEvent		event;
    long		e_num;
    AQueueCheckMode	mode = AQueuedAfterReading;
    AStopMode		smode;		/* stop mode */

    if (param.play_volume > min_gain && play_bucket != NULL) {
	if (AEventsQueued(audio,mode,&status) > 1) {
	    /* stop transaction - xid returned from prior call */
	    smode = ASMThisTrans;
	    AStopAudio(audio, xid, smode, NULL, &status);
   	}	
	param.event_mask = emask;
	xid = APlaySBucket(audio, play_bucket, &param, &status);
	if (status == AENoError) {
	    ASelectInput(audio, xid, emask, &status);
	}
	param.play_volume = min_gain;
	play_bucket = NULL;
    }
}


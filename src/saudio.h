/* $Id: saudio.h,v 3.3 1993/06/28 20:54:18 bjoerns Exp $
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

#ifndef _saudio_h
#define _saudio_h

#ifndef SOUND

/*
 * Define like this to avoid having to put #ifdef SOUND all over the place.
 */
#define sound_player_init(player)
#define sound_play_player(player, index)
#define sound_play_all(index)
#define sound_play_sensors(x, y, index)
#define sound_play_queued(player)
#define sound_close(player)

#else						/* SOUND */

#include "audio.h"

int             sound_player_init(player *);
void            sound_play_player(player *, int);
void            sound_play_all(int);
void            sound_play_sensors(float, float, int);
void            sound_play_queued(player * pl);
void            sound_close(player * pl);

#endif						/* SOUND */

#endif						/* _saudio_h */

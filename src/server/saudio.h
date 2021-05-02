/* $Id: saudio.h,v 5.0 2001/04/07 20:01:00 dik Exp $
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

#ifndef _saudio_h
#define _saudio_h

#if defined(SERVER_SOUND) && defined(SERVER) && !defined(SOUND)
/* Enable only sound support in the server, not in the client. */
#define SOUND	1
#endif

#define SDBG(x)	/*#x*/

#ifndef SOUND

/*
 * Define like this to avoid having to put #ifdef SOUND all over the place.
 */
#define sound_player_init(player)		((player)->audio = NULL)
#define sound_player_onoff(player, onoff)
#define sound_play_player(player, index)
#define sound_play_all(index)
#define sound_play_sensors(x, y, index)
#define sound_play_queued(player)
#define sound_close(player)

#else						/* SOUND */

#include "audio.h"

int		sound_player_init(player *);
void		sound_player_onoff(player *pl, int onoff);
void		sound_play_player(player *, int);
void		sound_play_all(int);
void		sound_play_sensors(DFLOAT, DFLOAT, int);
void		sound_play_queued(player * pl);
void		sound_close(player * pl);

#endif						/* SOUND */

#endif						/* _saudio_h */

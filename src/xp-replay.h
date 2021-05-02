/* $Id: xp-replay.h,v 3.3 1996/10/06 00:01:59 bjoerns Exp $
 *
 * XP-Replay, playback an XPilot session.  Copyright (C) 1994-95 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gÿsbers         <bert@xpilot.org>
 *      Steven Singer        (S.Singer@ph.surrey.ac.uk)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERC_HANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef	XP_REPLAY_H
#define	XP_REPLAY_H

#define BLACK               0
#define WHITE               1
#define BLUE                2
#define RED                 3

enum MemTypes {
    MEM_STRING,
    MEM_FRAME,
    MEM_SHAPE,
    MEM_POINT,
    MEM_GC,
    MEM_MISC,
    MEM_UI,
    NUM_MEMTYPES
};

void *MyMalloc(size_t, enum MemTypes);

#endif

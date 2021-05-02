/* $Id: sched.h,v 3.2 1996/10/06 00:01:40 bjoerns Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gÿsbers         <bert@xpilot.org>
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

#ifndef	SCHED_H
#define	SCHED_H

void block_timer(void);
void allow_timer(void);
void install_timer_tick(void (*func)(void), int freq);
void install_timeout(void (*func)(void *), int offset, void *arg);
void remove_timeout(void (*func)(void *), void *arg);
void install_input(void (*func)(int, void *), int fd, void *arg);
void remove_input(int fd);
void sched(void);
void stop_sched(void);

#endif

/* $Id: netclient.h,v 3.35 1995/09/16 19:04:34 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
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

#ifndef	NETCLIENT_H
#define	NETCLIENT_H

#ifndef TYPES_H
/* need u_byte */
#include "types.h"
#endif

#define MIN_RECEIVE_WINDOW_SIZE		1
#define MAX_RECEIVE_WINDOW_SIZE		4

extern int	receive_window_size;
extern long	last_loops;

int Net_setup(void);
int Net_verify(char *real, char *nick, char *dpy, int my_team);
int Net_init(char *server, int port);
void Net_cleanup(void);
void Net_key_change(void);
int Net_flush(void);
int Net_fd(void);
int Net_start(void);
void Net_init_measurement(void);
int Net_input(void);
int Receive_start(void);
int Receive_end(void);
int Receive_message(void);
int Receive_self(void);
int Receive_modifiers(void);
int Receive_refuel(void);
int Receive_connector(void);
int Receive_laser(void);
int Receive_missile(void);
int Receive_ball(void);
int Receive_ship(void);
int Receive_mine(void);
int Receive_item(void);
int Receive_destruct(void);
int Receive_shutdown(void);
int Receive_thrusttime(void);
int Receive_shieldtime(void);
int Receive_debris(void);
int Receive_fastshot(void);
int Receive_ecm(void);
int Receive_trans(void);
int Receive_paused(void);
int Receive_radar(void);
int Receive_damaged(void);
int Receive_leave(void);
int Receive_war(void);
int Receive_seek(void);
int Receive_player(void);
int Receive_score(void);
int Receive_score_object(void);
int Receive_timing(void);
int Receive_fuel(void);
int Receive_cannon(void);
int Receive_target(void);
int Receive_base(void);
int Receive_reliable(void);
int Receive_quit(void);
int Receive_string(void);
int Receive_reply(int *replyto, int *result);
int Send_ack(long rel_loops);
int Send_keyboard(u_byte *);
int Send_shape(char *);
int Send_power(float power);
int Send_power_s(float power_s);
int Send_turnspeed(float turnspeed);
int Send_turnspeed_s(float turnspeed_s);
int Send_turnresistance(float turnresistance);
int Send_turnresistance_s(float turnresistance_s);
int Send_pointer_move(int movement);
int Receive_audio(void);
int Receive_talk_ack(void);
int Send_talk(void);
int Send_display(void);
int Send_modifier_bank(int);
int Net_talk(char *str);
int Net_ask_for_motd(long offset, long maxlen);
int Receive_time_left(void);
int Receive_eyes(void);
int Receive_motd(void);
int Receive_magic(void);
int Send_audio_request(int onoff);
int Send_fps_request(int fps);
int Receive_loseitem(void);

#endif

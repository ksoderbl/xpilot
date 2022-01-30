/* $Id: netserver.h,v 5.11 2002/08/21 14:22:32 bertg Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
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

#ifndef	NETSERVER_H
#define	NETSERVER_H

#ifndef CONNECTION_H
#include "connection.h"
#endif

#ifdef NETSERVER_C


static int Compress_map(unsigned char *map, int size);
static int Init_setup(void);
static int Handle_listening(connection_t *connp);
static int Handle_setup(connection_t *connp);
static int Handle_login(connection_t *connp, char *errmsg, int errsize);
static void Handle_input(int fd, void *arg);


static int Receive_keyboard(connection_t *connp);
static int Receive_quit(connection_t *connp);
static int Receive_play(connection_t *connp);
static int Receive_power(connection_t *connp);
static int Receive_ack(connection_t *connp);
static int Receive_ack_cannon(connection_t *connp);
static int Receive_ack_fuel(connection_t *connp);
static int Receive_ack_target(connection_t *connp);
static int Receive_discard(connection_t *connp);
static int Receive_undefined(connection_t *connp);
static int Receive_talk(connection_t *connp);
static int Receive_display(connection_t *connp);
static int Receive_modifier_bank(connection_t *connp);
static int Receive_motd(connection_t *connp);
static int Receive_shape(connection_t *connp);
static int Receive_pointer_move(connection_t *connp);
static int Receive_audio_request(connection_t *connp);
static int Receive_fps_request(connection_t *connp);

static int Send_motd(connection_t *connp);

#endif	/* NETSERVER_C */

int Get_motd(char *buf, int offset, int maxlen, int *size_ptr);
int Setup_net_server(void);
void Destroy_connection(connection_t *connp, const char *reason);
int Check_connection(char *real, char *nick, char *dpy, char *addr);
int Setup_connection(char *real, char *nick, char *dpy, int team,
		     char *addr, char *host, unsigned version);
int Input(void);
int Send_reply(connection_t *connp, int replyto, int result);
int Send_self(connection_t *connp, player *pl,
	      int lock_id,
	      int lock_dist,
	      int lock_dir,
	      int autopilotlight,
	      long status,
	      char *mods);
int Send_leave(connection_t *connp, int id);
int Send_war(connection_t *connp, int robot_id, int killer_id);
int Send_seek(connection_t *connp, int programmer_id, int robot_id, int sought_id);
int Send_player(connection_t *connp, int id);
int Send_score(connection_t *connp, int id, int score,
	       int life, int mychar, int alliance);
int Send_score_object(connection_t *connp, int score, int x, int y, const char *string);
int Send_team_score(connection_t *connp, int team, int score);
int Send_timing(connection_t *connp, int id, int check, int round);
int Send_base(connection_t *connp, int id, int num);
int Send_fuel(connection_t *connp, int num, int fuel);
int Send_cannon(connection_t *connp, int num, int dead_time);
int Send_destruct(connection_t *connp, int count);
int Send_shutdown(connection_t *connp, int count, int delay);
int Send_thrusttime(connection_t *connp, int count, int max);
int Send_shieldtime(connection_t *connp, int count, int max);
int Send_phasingtime(connection_t *connp, int count, int max);
int Send_rounddelay(connection_t *connp, int count, int max);
int Send_debris(connection_t *connp, int type, unsigned char *p, int n);
int Send_wreckage(connection_t *connp, int x, int y, u_byte wrtype, u_byte size, u_byte rot);
int Send_asteroid(connection_t *connp, int x, int y, u_byte type, u_byte size, u_byte rot);
int Send_fastshot(connection_t *connp, int type, unsigned char *p, int n);
int Send_missile(connection_t *connp, int x, int y, int len, int dir);
int Send_ball(connection_t *connp, int x, int y, int id);
int Send_mine(connection_t *connp, int x, int y, int teammine, int id);
int Send_target(connection_t *connp, int num, int dead_time, int damage);
int Send_wormhole(connection_t *connp, int x, int y);
int Send_audio(connection_t *connp, int type, int vol);
int Send_item(connection_t *connp, int x, int y, int type);
int Send_paused(connection_t *connp, int x, int y, int count);
int Send_ecm(connection_t *connp, int x, int y, int size);
int Send_ship(connection_t *connp, int x, int y, int id, int dir, int shield, int cloak, int eshield, 
			  int phased, int deflector);
int Send_refuel(connection_t *connp, int x0, int y0, int x1, int y1);
int Send_connector(connection_t *connp, int x0, int y0, int x1, int y1, int tractor);
int Send_laser(connection_t *connp, int color, int x, int y, int len, int dir);
int Send_radar(connection_t *connp, int x, int y, int size);
int Send_fastradar(connection_t *connp, unsigned char *buf, int n);
int Send_damaged(connection_t *connp, int damaged);
int Send_message(connection_t *connp, const char *msg);
int Send_loseitem(int lose_item_index, connection_t *connp);
int Send_start_of_frame(connection_t *connp);
int Send_end_of_frame(connection_t *connp);
int Send_reliable(connection_t *connp);
int Send_time_left(connection_t *connp, long sec);
int Send_eyes(connection_t *connp, int id);
int Send_trans(connection_t *connp, int x1, int y1, int x2, int y2);
void Get_display_parameters(connection_t *connp, int *width, int *height,
			    int *debris_colors, int *spark_rand);
int Get_player_id(connection_t *connp);
int Get_conn_version(connection_t *connp);
const char *Get_player_addr(connection_t *connp);
const char *Get_player_dpy(connection_t *connp);
int Send_shape(connection_t *connp, int shape);
int Check_max_clients_per_IP(char *host_addr);

#endif


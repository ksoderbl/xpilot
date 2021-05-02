/* $Id: netclient.h,v 3.9 1993/08/02 12:55:12 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	NETCLIENT_H
#define	NETCLIENT_H

#include "types.h"

#define MIN_RECEIVE_WINDOW_SIZE	1
#define MAX_RECEIVE_WINDOW_SIZE	4
#define DEF_RECEIVE_WINDOW_SIZE	3

extern int			receive_window_size;

int Net_setup(void);
int Net_verify(char *real, char *nick, char *dpy);
int Net_init(char *server, int port);
void Net_cleanup(void);
void Net_key_change(void);
int Net_flush(void);
int Net_fd(void);
int Net_start(void);
int Net_input(void);
int Receive_start(void);
int Receive_end(void);
int Receive_message(void);
int Receive_self(void);
int Receive_refuel(void);
int Receive_connector(void);
int Receive_smart(void);
int Receive_ball(void);
int Receive_ship(void);
int Receive_mine(void);
int Receive_item(void);
int Receive_destruct(void);
int Receive_shutdown(void);
int Receive_debris(void);
int Receive_shot(void);
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
int Send_power(float power);
int Send_power_s(float power_s);
int Send_turnspeed(float turnspeed);
int Send_turnspeed_s(float turnspeed_s);
int Send_turnresistance(float turnresistance);
int Send_turnresistance_s(float turnresistance_s);
int Receive_audio(void);
int Receive_talk_ack(void);
int Send_talk(void);
int Net_talk(char *str);

#endif

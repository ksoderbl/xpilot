/* $Id: protoclient.h,v 3.2 1996/10/13 15:01:13 bert Exp $
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

#ifndef	PROTOCLIENT_H
#define	PROTOCLIENT_H

#ifdef VMS
#include "strcasecmp.h"
#endif

/*
 * default.c
 */
extern void Parse_options(int *argcp, char **argvp, char *realName, int *port,
			  int *my_team, int *list, int *join, int *noLocalMotd,
			  char *nickName, char *dispName, char *shut_msg);
extern void Get_xpilotrc_file(char *, unsigned);

/*
 * join.c
 */
extern int Join(char *server_addr, char *server_name, int port,
		char *real, char *nick, int my_team,
		char *display, unsigned version);

/*
 * query.c
 */
extern int Query_all(int sockfd, int port, char *msg, int msglen);

#ifdef	LIMIT_ACCESS
extern bool		Is_allowed(char *);
#endif

/*
 * usleep.c
 */
extern int micro_delay(unsigned usec);

/*
 * metaclient.c
 */
extern int metaclient(int, char **);

/*
 * record.c
 */
extern void Record_cleanup(void);
extern void Record_init(char *filename);

/*
 * math.c
 */
extern int ON(char *optval);
extern int OFF(char *optval);

/*
 * colors.c
 */
void List_visuals(void);
int Colors_init(void);
void Colors_cleanup(void);

#endif

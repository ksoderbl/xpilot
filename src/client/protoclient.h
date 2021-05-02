/* $Id: protoclient.h,v 4.11 2001/03/20 18:37:58 bert Exp $
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

#ifndef	PROTOCLIENT_H
#define	PROTOCLIENT_H

#ifdef VMS
#include "strcasecmp.h"
#endif

/*
 * about.c
 */
extern int Handle_motd(long off, char *buf, int len, long filesize);
extern void aboutCleanup(void);

#ifdef _WINDOWS
extern	void Motd_destroy();
extern	void Keys_destroy();
#endif

extern int motd_viewer;		/* so Windows can clean him up */
extern int keys_viewer;

/*
 * colors.c
 */
void List_visuals(void);
int Colors_init(void);
int Colors_init_block_bitmaps(void);
void Colors_free_block_bitmaps(void);
void Colors_cleanup(void);
void Colors_debug(void);

/*
 * default.c
 */
extern void Parse_options(int *argcp, char **argvp, char *realName, int *port,
			  int *my_team, int *text, int *list,
			  int *join, int *noLocalMotd,
			  char *nickName, char *dispName, char *shut_msg);
extern void defaultCleanup(void);				/* memory cleanup */

#ifndef _WINDOWS
extern void Get_xpilotrc_file(char *, unsigned);
#else
extern	char* Get_xpilotini_file(int level);
#endif
/*
 * join.c
 */
extern int Join(char *server_addr, char *server_name, int port,
		char *real, char *nick, int my_team,
		char *display, unsigned version);

/*
 * metaclient.c
 */
extern int metaclient(int, char **);

/*
 * math.c
 */
extern int ON(char *optval);
extern int OFF(char *optval);

/*
 * paintdata.c
 */
extern void paintdataCleanup(void);		/* memory cleanup */

/*
 * paintobjects.c
 */
extern int Init_wreckage(void);


/*
 * query.c
 */
#ifdef SOCKLIB_H
extern int Query_all(sock_t *sockfd, int port, char *msg, int msglen);
#endif

#ifdef	LIMIT_ACCESS
extern bool		Is_allowed(char *);
#endif

/*
 * record.c
 */
extern void Record_cleanup(void);
extern void Record_init(char *filename);

/*
 * textinterface.c
 */
#ifdef CONNECTPARAM_H
int Connect_to_server(int auto_connect, int list_servers,
		      int auto_shutdown, char *shutdown_reason,
		      Connect_param_t *conpar);
int Contact_servers(int count, char **servers,
                    int auto_connect, int list_servers,
                    int auto_shutdown, char *shutdown_message,
                    int find_max, int *num_found,
                    char **server_addresses, char **server_names,
                    Connect_param_t *conpar);
#endif

/*
 * usleep.c
 */
extern int micro_delay(unsigned usec);

/*
 * welcome.c
 */
#ifdef CONNECTPARAM_H
int Welcome_screen(Connect_param_t *conpar);
#endif

/*
 * widget.c
 */
void Widget_cleanup(void);

/*
 * xinit.c
 */
#ifdef _WINDOWS
extern	void WinXCreateItemBitmaps();
#endif

/*
 * winX - The Windows X emulator
 */
#ifdef _WINDOWS
#define	WinXFlush(__w)	WinXFlush(__w)
#else
#define	WinXFlush(__w)
#endif


#endif	/* PROTOCLIENT_H */



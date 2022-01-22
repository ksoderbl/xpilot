/* $Id: connection.h,v 5.1 2001/11/28 14:21:20 bertg Exp $
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

#ifndef	CONNECTION_H
#define	CONNECTION_H

/*
 * This include file holds defines and types related to the
 * connection of a player.  It is mainly used by netserver.c.
 */


#ifndef NET_H
/* need sockbuf_t. */
#include "net.h"
#endif

#ifndef DRAW_H
/* need shipobj. */
#include "draw.h"
#endif


/*
 * Different states a connection can be in.
 */
#define CONN_FREE	0x00	/* free for use */
#define CONN_LISTENING	0x01	/* before connect() */
#define CONN_SETUP	0x02	/* after verification */
#define CONN_LOGIN	0x04	/* after setup info transferred */
#define CONN_PLAYING	0x08	/* when actively playing */
#define CONN_DRAIN	0x20	/* wait for all reliable data to be acked */
#define CONN_READY	0x40	/* draining after LOGIN and before PLAYING */

/*
 * In order to not let the server be locked by a collection
 * of idle connections we timeout a client if it doesn't
 * continue with logging in in a reasonable tempo.
 * Sorry, our resources are limited.
 * But the timeout should be easily configurable.
 * The timeout specifies the number of seconds each connection
 * state may last.
 */
#define LISTEN_TIMEOUT		4
#define SETUP_TIMEOUT		15
#define LOGIN_TIMEOUT		40
#define READY_TIMEOUT		40
#define IDLE_TIMEOUT		30

/*
 * Maximum roundtrip time taken as serious for rountrip time calculations.
 */
#define MAX_RTT			(FPS + 1)

/*
 * The retransmission timeout bounds in number of frames.
 */
#define MIN_RETRANSMIT		(FPS / 8 + 1)
#define MAX_RETRANSMIT		(FPS + 1)
#define DEFAULT_RETRANSMIT	(FPS / 2)


/*
 * All the player connection state info.
 */
typedef struct {
    int			conn_index;		/* index of connection in Conn[] */
    int			state;			/* state of connection */
    int			drain_state;		/* state after draining done */
    unsigned		magic;			/* magic cookie */
    sockbuf_t		r;			/* input buffer */
    sockbuf_t		w;			/* output buffer */
    sockbuf_t		c;			/* reliable data buffer */
    long		start;			/* time of last state change */
    long		timeout;		/* time when state timeouts */
    long		last_send_loops;	/* last update of reliable */
    long		reliable_offset;	/* amount of data acked */
    long		reliable_unsent;	/* next unsend reliable byte */
    long		retransmit_at_loop;	/* next retransmission time */
    int			rtt_smoothed;		/* smoothed roundtrip time */
    int			rtt_dev;		/* roundtrip time deviation */
    int			rtt_retransmit;		/* retransmission time */
    int			rtt_timeouts;		/* how many timeouts */
    int			acks;			/* good acknowledgements */
    int			setup;			/* amount of setup done */
    int			my_port;		/* server port for this player */
    int			his_port;		/* client port for this player */
    int			id;			/* index into GetInd[] or NO_ID */
    int			team;			/* team of player */
    unsigned		version;		/* XPilot version of client */
    long		last_key_change;	/* last keyboard change */
    long		talk_sequence_num;	/* talk acknowledgement */
    long		motd_offset;		/* offset into motd or -1 */
    long		motd_stop;		/* max offset into motd */
    int			num_keyboard_updates;	/* Keyboards in one packet */
    int			view_width, view_height;/* Viewable area dimensions */
    int			debris_colors;		/* Max. debris intensities */
    int			spark_rand;		/* Sparkling effect */
    char		*real;			/* real login name of player */
    char		*nick;			/* nickname of player */
    char		*dpy;			/* display of player */
    shipobj		*ship;			/* ship shape of player */
    char		*addr;			/* address of players host */
    char		*host;			/* hostname of players host */
} connection_t;

#endif


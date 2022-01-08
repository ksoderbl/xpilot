/* $Id: datagram.c,v 5.5 2001/06/26 09:53:26 bertg Exp $
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#ifndef _WINDOWS
# include <unistd.h>
# include <sys/param.h>
# include <netdb.h>
#endif

#ifdef _WINDOWS
# include "NT/winNet.h"
# include "NT/winClient.h"
#endif

#include "version.h"
#include "config.h"
#include "error.h"
#include "client.h"
#include "socklib.h"
#include "protoclient.h"
#include "datagram.h"
#include "portability.h"

extern int	clientPortStart;	/* First UDP port for clients */
extern int	clientPortEnd;		/* Last one (these are for firewalls) */

char datagram_version[] = VERSION;


int			dgram_one_socket = 0;


int create_dgram_addr_socket(sock_t *sock, char *dotaddr, int port)
{
    static int		saved;
    static sock_t	save_sock;
    int			status = SOCK_IS_ERROR;
    int			i;

    if (saved == 0) {
	if (clientPortStart && (!clientPortEnd || clientPortEnd > 65535)) {
	    clientPortEnd = 65535;
	}
	if (clientPortEnd && (!clientPortStart || clientPortStart < 1024)) {
	    clientPortStart = 1024;
	}

	if (port || !clientPortStart || (clientPortStart > clientPortEnd)) {
	    status = sock_open_udp(sock, dotaddr, port);
	    if (status == SOCK_IS_ERROR) {
		error("Cannot create datagram socket (%d)", sock->error.error);
		return -1;
	    }
	}
	else {
	    int found_socket = 0;
	    for (i = clientPortStart; i <= clientPortEnd; i++) {
		status = sock_open_udp(sock, dotaddr, i);
		if (status != SOCK_IS_ERROR) {
		    found_socket = 1;
		    break;
		}
	    }
	    if (found_socket == 0) {
		error("Could not find a usable port in port range [%d,%d]",
		      clientPortStart, clientPortEnd);
		return -1;
	    }
	}

	if (status == SOCK_IS_OK) {
	    if (dgram_one_socket) {
		save_sock = *sock;
	    }
	}
    } else {
	*sock = save_sock;
	status = SOCK_IS_OK;
    }

    return status;
}

int create_dgram_socket(sock_t *sock, int port)
{
    static char any_addr[] = "0.0.0.0";

    return create_dgram_addr_socket(sock, any_addr, port);
}

void close_dgram_socket(sock_t *sock)
{
    if (!dgram_one_socket) {
	sock_close(sock);
    }
}


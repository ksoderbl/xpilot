/* $Id: datagram.c,v 4.6 2001/03/20 18:47:19 bert Exp $
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
#include "socklib.h"
#include "protoclient.h"
#include "datagram.h"
#include "portability.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: datagram.c,v 4.6 2001/03/20 18:47:19 bert Exp $";
#endif


int			dgram_one_socket = 0;


int create_dgram_addr_socket(sock_t *sock, char *dotaddr, int port)
{
    static int		saved;
    static sock_t	save_sock;
    int			status;

    if (saved == 0) {
	status = sock_open_udp(sock, dotaddr, port);
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


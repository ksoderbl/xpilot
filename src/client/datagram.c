/* $Id: datagram.c,v 4.1 1999/10/16 18:39:02 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
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

#ifndef	_WINDOWS
# include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifndef	_WINDOWS
# include <sys/types.h>
# include <sys/param.h>
#endif

#ifdef	_WINDOWS
#include "NT/winNet.h"
#include "NT/winClient.h"
#else
# include <netdb.h>
#endif

#include "version.h"
#include "config.h"
#include "error.h"
#include "socklib.h"
#include "protoclient.h"
#include "portability.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: datagram.c,v 4.1 1999/10/16 18:39:02 bert Exp $";
#endif


int			dgram_one_socket = 0;


int create_dgram_addr_socket(char *dotaddr, int port)
{
    static int save_fd = -1;
    int fd;

    if (save_fd == -1) {
	fd = CreateDgramAddrSocket(dotaddr, port);
	if (dgram_one_socket) {
	    save_fd = fd;
	}
    } else {
	fd = save_fd;
    }
    return fd;
}

int create_dgram_socket(int port)
{
    static char any_addr[] = "0.0.0.0";
    return create_dgram_addr_socket(any_addr, port);
}

void close_dgram_socket(int fd)
{
    if (!dgram_one_socket) {
	DgramClose(fd);
    }
}


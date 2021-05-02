/* $Id: join.c,v 3.41 1998/01/28 08:50:05 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-97 by
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

#ifdef	_WINDOWS
#include <winsock.h>
#include "../contrib/NT/xpilot/winClient.h"
#endif

#include "types.h"
#ifndef	_WINDOWS
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

#ifndef	_WINDOWS
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#if !defined(__hpux) 
#include <sys/time.h>
#endif
#endif

#include "version.h"
#include "config.h"
#include "const.h"
#include "error.h"
#include "client.h"
#include "netclient.h"
#include "protoclient.h"
#include "portability.h"

char join_version[] = VERSION;

#ifndef SCORE_UPDATE_DELAY
# define SCORE_UPDATE_DELAY	4
#endif

void xpilotShutdown(void);

extern void Record_cleanup(void);


#ifndef	_WINDOWS
static void Input_loop(void)
{
    fd_set		rfds;
    fd_set		tfds;
    int			max,
			n,
			netfd,
			result,
			clientfd;
    struct timeval	tv;

    if ((result = Net_input()) == -1) {
	error("Bad server input");
	return;
    }
    if (Client_input(2) == -1) {
	return;
    }
    if (Net_flush() == -1) {
	return;
    }
    if ((clientfd = Client_fd()) == -1) {
	error("Bad client filedescriptor");
	return;
    }
    if ((netfd = Net_fd()) == -1) {
	error("Bad socket filedescriptor");
	return;
    }
    Net_key_change();
    FD_ZERO(&rfds);
    FD_SET(clientfd, &rfds);
    FD_SET(netfd, &rfds);
    max = (clientfd > netfd) ? clientfd : netfd;
    for (tfds = rfds; ; rfds = tfds) {
	if ((scoresChanged != 0 && ++scoresChanged > SCORE_UPDATE_DELAY)
	    || result > 1) {
	    if (scoresChanged > 2 * SCORE_UPDATE_DELAY) {
		Client_score_table();
		tv.tv_sec = 10;
		tv.tv_usec = 0;
	    } else {
		tv.tv_sec = 0;
		tv.tv_usec = 0;
	    }
	} else {
	    tv.tv_sec = 10;
	    tv.tv_usec = 0;
	}
	if ((n = select(max + 1, &rfds, NULL, NULL, &tv)) == -1) {
	    if (errno == EINTR) {
		continue;
	    }
	    error("Select failed");
	    return;
	}
	if (n == 0) {
	    if (scoresChanged > SCORE_UPDATE_DELAY) {
		Client_score_table();
		if (Client_input(2) == -1) {
		    return;
		}
		continue;
	    }
	    else if (result <= 1) {
		errno = 0;
		error("No response from server");
		continue;
	    }
	}
	if (FD_ISSET(clientfd, &rfds)) {
	    if (Client_input(1) == -1) {
		return;
	    }
	    if (Net_flush() == -1) {
		error("Bad net flush after X input");
		return;
	    }
	}
	if (FD_ISSET(netfd, &rfds) || result > 1) {
	    if ((result = Net_input()) == -1) {
		errno = 0;
		error("Bad net input.  Have a nice day!");
		return;
	    }
	    if (result > 0) {
		/*
		 * Now there's a frame being drawn by the X server.
		 * So we shouldn't try to send more drawing
		 * requests to the X server or it might get
		 * overloaded which could cause problems with
		 * keyboard input.  Besides, we wouldn't even want
		 * to send more drawing requests because there
		 * may arive a more recent frame update soon
		 * and using the CPU now may even slow down the X server
		 * if it is running on the same CPU.
		 * So we only check if the X server has sent any more
		 * keyboard events and then we wait until the X server
		 * has finished the drawing of our current frame.
		 */
		if (Client_input(1) == -1) {
		    return;
		}
		if (Net_flush() == -1) {
		    error("Bad net flush before sync");
		    return;
		}
		Client_sync();
		if (Client_input(1) == -1) {
		    return;
		}
	    }
	}
    }
}
#endif	/* _WINDOWS */

void xpilotShutdown()
{
    Net_cleanup();
    Client_cleanup();
    Record_cleanup();
    defaultCleanup();
    aboutCleanup();
    paintdataCleanup();
}

static void sigcatch(int signum)
{
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    xpilotShutdown();
    error("Got signal %d\n", signum);
    exit(1);
}

int Join(char *server_addr, char *server_name, int port, char *real,
	 char *nick, int my_team, char *display, unsigned version)
{
    signal(SIGINT, sigcatch);
    signal(SIGTERM, sigcatch);
#ifndef	_WINDOWS
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
#endif

    IFWINDOWS( received_self = FALSE; )
    IFWINDOWS( Progress("Client_init"); )
    if (Client_init(server_name, version) == -1) {
	return -1;
    }
    IFWINDOWS( Progress("Net_init %s", server_addr); )
    if (Net_init(server_addr, port) == -1) {
	Client_cleanup();
	return -1;
    }
    IFWINDOWS( Progress("Net_verify '%s'= '%s'", nick, real); )
    if (Net_verify(real, nick, display, my_team) == -1) {
	Net_cleanup();
	Client_cleanup();
	return -1;
    }
    IFWINDOWS( Progress("Net_setup"); )
    if (Net_setup() == -1) {
	Net_cleanup();
	Client_cleanup();
	return -1;
    }
    IFWINDOWS( Progress("Client_setup"); )
    if (Client_setup() == -1) {
	Net_cleanup();
	Client_cleanup();
	return -1;
    }
    IFWINDOWS( Progress("Net_start"); )
    if (Net_start() == -1) {
	errno = 0;
	error("Network start failed");
	Net_cleanup();
	Client_cleanup();
	return -1;
    }
    IFWINDOWS( Progress("Client_start"); )
    if (Client_start() == -1) {
	errno = 0;
	error("Window init failed");
	Net_cleanup();
	Client_cleanup();
	return -1;
    }

#ifndef	_WINDOWS	/* windows continues to run at this point */
    Input_loop();
    xpilotShutdown();
#endif

    return 0;
}


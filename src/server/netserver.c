/* $Id: netserver.c,v 5.20 2001/06/26 19:14:08 bertg Exp $
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

/*
 * This is the server side of the network connnection stuff.
 *
 * We try very hard to not let the game be disturbed by
 * players logging in.  Therefore a new connection
 * passes through several states before it is actively
 * playing.
 * First we make a new connection structure available
 * with a new socket to listen on.  This socket port
 * number is told to the client via the pack mechanism.
 * In this state the client has to send a packet to this
 * newly created socket with its name and playing parameters.
 * If this succeeds the connection advances to its second state.
 * In this second state the essential server configuration
 * like the map and so on is transmitted to the client.
 * If the client has acknowledged all this data then it
 * advances to the third state, which is the
 * ready-but-not-playing-yet state.  In this state the client
 * has some time to do its final initializations, like mapping
 * its user interface windows and so on.
 * When the client is ready to accept frame updates and process
 * keyboard events then it sends the start-play packet.
 * This play packet advances the connection state into the
 * actively-playing state.  A player structure is allocated and
 * initialized and the other human players are told about this new player.
 * The newly started client is told about the already playing players and
 * play has begun.
 * Apart from these four states there are also two intermediate states.
 * These intermediate states are entered when the previous state
 * has filled the reliable data buffer and the client has not
 * acknowledged all the data yet that is in this reliable data buffer.
 * They are so called output drain states.  Not doing anything else
 * then waiting until the buffer is empty.
 * The difference between these two intermediate states is tricky.
 * The second intermediate state is entered after the
 * ready-but-not-playing-yet state and before the actively-playing state.
 * The difference being that in this second intermediate state the client
 * is already considered an active player by the rest of the server
 * but should not get frame updates yet until it has acknowledged its last
 * reliable data.
 *
 * Communication between the server and the clients is only done
 * using UDP datagrams.  The first client/serverized version of XPilot
 * was using TCP only, but this was too unplayable across the Internet,
 * because TCP is a data stream always sending the next byte.
 * If a packet gets lost then the server has to wait for a
 * timeout before a retransmission can occur.  This is too slow
 * for a real-time program like this game, which is more interested
 * in recent events than in sequenced/reliable events.
 * Therefore UDP is now used which gives more network control to the
 * program.
 * Because some data is considered crucial, like the names of
 * new players and so on, there also had to be a mechanism which
 * enabled reliable data transmission.  Here this is done by creating
 * a data stream which is piggybacked on top of the unreliable data
 * packets.  The client acknowledges this reliable data by sending
 * its byte position in the reliable data stream.  So if the client gets
 * a new reliable data packet and it has not had this data before and
 * there is also no data packet missing inbetween, then it advances
 * its byte position and acknowledges this new position to the server.
 * Otherwise it discards the packet and sends its old byte position
 * to the server meaning that it detected a packet loss.
 * The server maintains an acknowledgement timeout timer for each
 * connection so that it can retransmit a reliable data packet
 * if the acknowledgement timer expires.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WINDOWS
# include <unistd.h>
# include <sys/param.h>
# ifdef _AIX
#  ifndef _BSD_INCLUDES
#   define _BSD_INCLUDES
#  endif
# endif
# ifndef __hpux
#  include <sys/time.h>
# endif
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
#endif

#ifdef _WINDOWS
# include "NT/winServer.h"
# include <io.h>
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "pack.h"
#include "bit.h"
#include "types.h"
#include "socklib.h"
#include "sched.h"
#include "net.h"
#include "error.h"
#define NETSERVER_C
#include "netserver.h"
#include "packet.h"
#include "setup.h"
#undef NETSERVER_C
#include "saudio.h"
#include "checknames.h"
#include "server.h"
#include "commonproto.h"
#include "asteroid.h"

char netserver_version[] = VERSION;

#define MAX_SELECT_FD			(sizeof(int) * 8 - 1)
#define MAX_RELIABLE_DATA_PACKET_SIZE	1024

#define MAX_MOTD_CHUNK			512
#define MAX_MOTD_SIZE			(30*1024)
#define MAX_MOTD_LOOPS			(10*FPS)

static connection_t	*Conn = NULL;
static int		max_connections = 0;
static setup_t		*Setup = NULL;
static int		(*playing_receive[256])(int ind),
			(*login_receive[256])(int ind),
			(*drain_receive[256])(int ind);
int			compress_maps = 1;
int			login_in_progress;
static int		num_logins, num_logouts;

char *showtime(void)
{
    time_t		now;
    struct tm		*tmp;
    static char		month_names[13][4] = {
			    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
			    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
			    "Bug"
			};
    static char		buf[80];

    time(&now);
    tmp = localtime(&now);
    sprintf(buf, "%02d %s %02d:%02d:%02d",
	    tmp->tm_mday, month_names[tmp->tm_mon],
	    tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
    return buf;
}

/*
 * Compress the map data using a simple Run Length Encoding algorithm.
 * If there is more than one consecutive byte with the same type
 * then we set the high bit of the byte and then the next byte
 * gives the number of repetitions.
 * This works well for most maps which have lots of series of the
 * same map object and is simple enough to got implemented quickly.
 */
static int Compress_map(unsigned char *map, int size)
{
    int			i, j, k;

    for (i = j = 0; i < size; i++, j++) {
	if (i + 1 < size
	    && map[i] == map[i + 1]) {
	    for (k = 2; i + k < size; k++) {
		if (map[i] != map[i + k]) {
		    break;
		}
		if (k == 255) {
		    break;
		}
	    }
	    map[j] = (map[i] | SETUP_COMPRESSED);
	    map[++j] = k;
	    i += k - 1;
	} else {
	    map[j] = map[i];
	}
    }
    return j;
}

/*
 * Initialize the structure that gives the client information
 * about our setup.  Like the map and playing rules.
 * We only setup this structure once to save time when new
 * players log in during play.
 */
static int Init_setup(void)
{
    int			i, x, y, team, type, size,
			wormhole = 0,
			treasure = 0,
			target = 0,
			base = 0,
			cannon = 0;
    unsigned char	*mapdata, *mapptr;

    if ((mapdata = (unsigned char *) malloc(World.x * World.y)) == NULL) {
	error("No memory for mapdata");
	return -1;
    }
    memset(mapdata, SETUP_SPACE, World.x * World.y);
    mapptr = mapdata;
    errno = 0;
    for (x = 0; x < World.x; x++) {
	for (y = 0; y < World.y; y++, mapptr++) {
	    type = World.block[x][y];
	    switch (type) {
	    case ACWISE_GRAV:
	    case CWISE_GRAV:
	    case POS_GRAV:
	    case NEG_GRAV:
	    case UP_GRAV:
	    case DOWN_GRAV:
	    case RIGHT_GRAV:
	    case LEFT_GRAV:
		if (!gravityVisible)
		    type = SPACE;
		break;
	    case WORMHOLE:
		if (!wormholeVisible)
		    type = SPACE;
		break;
	    case ITEM_CONCENTRATOR:
		if (!itemConcentratorVisible)
		    type = SPACE;
		break;
	    case FRICTION:
		if (!blockFrictionVisible)
		    type = SPACE;
		else
		    type = DECOR_FILLED;
		break;
	    default:
		break;
	    }
	    switch (type) {
	    case SPACE:		*mapptr = SETUP_SPACE; break;
	    case FILLED:	*mapptr = SETUP_FILLED; break;
	    case REC_RU:	*mapptr = SETUP_REC_RU; break;
	    case REC_RD:	*mapptr = SETUP_REC_RD; break;
	    case REC_LU:	*mapptr = SETUP_REC_LU; break;
	    case REC_LD:	*mapptr = SETUP_REC_LD; break;
	    case FUEL:		*mapptr = SETUP_FUEL; break;
	    case ACWISE_GRAV:	*mapptr = SETUP_ACWISE_GRAV; break;
	    case CWISE_GRAV:	*mapptr = SETUP_CWISE_GRAV; break;
	    case POS_GRAV:	*mapptr = SETUP_POS_GRAV; break;
	    case NEG_GRAV:	*mapptr = SETUP_NEG_GRAV; break;
	    case UP_GRAV:	*mapptr = SETUP_UP_GRAV; break;
	    case DOWN_GRAV:	*mapptr = SETUP_DOWN_GRAV; break;
	    case RIGHT_GRAV:	*mapptr = SETUP_RIGHT_GRAV; break;
	    case LEFT_GRAV:	*mapptr = SETUP_LEFT_GRAV; break;
	    case ITEM_CONCENTRATOR: *mapptr = SETUP_ITEM_CONCENTRATOR; break;
	    case DECOR_FILLED:	*mapptr = SETUP_DECOR_FILLED; break;
	    case DECOR_RU:	*mapptr = SETUP_DECOR_RU; break;
	    case DECOR_RD:	*mapptr = SETUP_DECOR_RD; break;
	    case DECOR_LU:	*mapptr = SETUP_DECOR_LU; break;
	    case DECOR_LD:	*mapptr = SETUP_DECOR_LD; break;
	    case WORMHOLE:
		switch (World.wormHoles[wormhole++].type) {
		case WORM_NORMAL: *mapptr = SETUP_WORM_NORMAL; break;
		case WORM_IN:     *mapptr = SETUP_WORM_IN; break;
		case WORM_OUT:    *mapptr = SETUP_WORM_OUT; break;
		default:
		    error("Bad wormhole (%d,%d).", x, y);
		    free(mapdata);
		    return -1;
		}
		break;
	    case TREASURE:
		*mapptr = SETUP_TREASURE + World.treasures[treasure++].team;
		break;
	    case TARGET:
		*mapptr = SETUP_TARGET + World.targets[target++].team;
		break;
	    case BASE:
		if (World.base[base].team == TEAM_NOT_SET) {
		    team = 0;
		} else {
		    team = World.base[base].team;
		}
		switch (World.base[base++].dir) {
		case DIR_UP:    *mapptr = SETUP_BASE_UP + team; break;
		case DIR_RIGHT: *mapptr = SETUP_BASE_RIGHT + team; break;
		case DIR_DOWN:  *mapptr = SETUP_BASE_DOWN + team; break;
		case DIR_LEFT:  *mapptr = SETUP_BASE_LEFT + team; break;
		default:
		    error("Bad base at (%d,%d).", x, y);
		    free(mapdata);
		    return -1;
		}
		break;
	    case CANNON:
		switch (World.cannon[cannon++].dir) {
		case DIR_UP:	*mapptr = SETUP_CANNON_UP; break;
		case DIR_RIGHT:	*mapptr = SETUP_CANNON_RIGHT; break;
		case DIR_DOWN:	*mapptr = SETUP_CANNON_DOWN; break;
		case DIR_LEFT:	*mapptr = SETUP_CANNON_LEFT; break;
		default:
		    error("Bad cannon at (%d,%d).", x, y);
		    free(mapdata);
		    return -1;
		}
		break;
	    case CHECK:
		for (i = 0; i < World.NumChecks; i++) {
		    if (x != World.check[i].x
			|| y != World.check[i].y) {
			continue;
		    }
		    *mapptr = SETUP_CHECK + i;
		    break;
		}
		if (i >= World.NumChecks) {
		    error("Bad checkpoint at (%d,%d).", x, y);
		    free(mapdata);
		    return -1;
		}
		break;
	    default:
		error("Unknown map type (%d) at (%d,%d).", type, x, y);
		*mapptr = SETUP_SPACE;
		break;
	    }
	}
    }
    if (compress_maps == 0) {
	type = SETUP_MAP_UNCOMPRESSED;
	size = World.x * World.y;
    } else {
	type = SETUP_MAP_ORDER_XY;
	size = Compress_map(mapdata, World.x * World.y);
	if (size <= 0 || size > World.x * World.y) {
	    errno = 0;
	    error("Map compression error (%d)", size);
	    free(mapdata);
	    return -1;
	}
	if ((mapdata = (unsigned char *)realloc(mapdata, size)) == NULL) {
	    error("Cannot reallocate mapdata");
	    return -1;
	}
    }

#ifndef SILENT
    if (type != SETUP_MAP_UNCOMPRESSED) {
	xpprintf("%s Map compression ratio is %-4.2f%%\n", showtime(),
	    100.0 * size / (World.x * World.y));
    }
#endif
    if ((Setup = (setup_t *) malloc(sizeof(setup_t) + size)) == NULL) {
	error("No memory to hold setup");
	free(mapdata);
	return -1;
    }
    memset(Setup, 0, sizeof(setup_t) + size);
    memcpy(Setup->map_data, mapdata, size);
    free(mapdata);
    Setup->setup_size = ((char *) &Setup->map_data[0] - (char *) Setup) + size;
    Setup->map_data_len = size;
    Setup->map_order = type;
    Setup->frames_per_second = FPS;
    Setup->lives = World.rules->lives;
    Setup->mode = World.rules->mode;
    Setup->x = World.x;
    Setup->y = World.y;
    strlcpy(Setup->name, World.name, sizeof(Setup->name));
    strlcpy(Setup->author, World.author, sizeof(Setup->author));

    return 0;
}

/*
 * Initialize the function dispatch tables for the various client
 * connection states.  Some states use the same table.
 */
static void Init_receive(void)
{
    int			i;

    for (i = 0; i < 256; i++) {
	login_receive[i] = Receive_undefined;
	playing_receive[i] = Receive_undefined;
	drain_receive[i] = Receive_undefined;
    }

    drain_receive[PKT_QUIT]			= Receive_quit;
    drain_receive[PKT_ACK]			= Receive_ack;
    drain_receive[PKT_VERIFY]			= Receive_discard;
    drain_receive[PKT_PLAY]			= Receive_discard;
    drain_receive[PKT_SHAPE]			= Receive_discard;

    login_receive[PKT_PLAY]			= Receive_play;
    login_receive[PKT_QUIT]			= Receive_quit;
    login_receive[PKT_ACK]			= Receive_ack;
    login_receive[PKT_VERIFY]			= Receive_discard;
    login_receive[PKT_POWER]			= Receive_power;
    login_receive[PKT_POWER_S]			= Receive_power;
    login_receive[PKT_TURNSPEED]		= Receive_power;
    login_receive[PKT_TURNSPEED_S]		= Receive_power;
    login_receive[PKT_TURNRESISTANCE]		= Receive_power;
    login_receive[PKT_TURNRESISTANCE_S]		= Receive_power;
    login_receive[PKT_DISPLAY]			= Receive_display;
    login_receive[PKT_MODIFIERBANK]		= Receive_modifier_bank;
    login_receive[PKT_MOTD]			= Receive_motd;
    login_receive[PKT_SHAPE]			= Receive_shape;
    login_receive[PKT_REQUEST_AUDIO]		= Receive_audio_request;
    login_receive[PKT_ASYNC_FPS]		= Receive_fps_request;

    playing_receive[PKT_ACK]			= Receive_ack;
    playing_receive[PKT_VERIFY]			= Receive_discard;
    playing_receive[PKT_PLAY]			= Receive_play;
    playing_receive[PKT_QUIT]			= Receive_quit;
    playing_receive[PKT_KEYBOARD]		= Receive_keyboard;
    playing_receive[PKT_POWER]			= Receive_power;
    playing_receive[PKT_POWER_S]		= Receive_power;
    playing_receive[PKT_TURNSPEED]		= Receive_power;
    playing_receive[PKT_TURNSPEED_S]		= Receive_power;
    playing_receive[PKT_TURNRESISTANCE]		= Receive_power;
    playing_receive[PKT_TURNRESISTANCE_S]	= Receive_power;
    playing_receive[PKT_ACK_CANNON]		= Receive_ack_cannon;
    playing_receive[PKT_ACK_FUEL]		= Receive_ack_fuel;
    playing_receive[PKT_ACK_TARGET]		= Receive_ack_target;
    playing_receive[PKT_TALK]			= Receive_talk;
    playing_receive[PKT_DISPLAY]		= Receive_display;
    playing_receive[PKT_MODIFIERBANK]		= Receive_modifier_bank;
    playing_receive[PKT_MOTD]			= Receive_motd;
    playing_receive[PKT_SHAPE]			= Receive_shape;
    playing_receive[PKT_POINTER_MOVE]		= Receive_pointer_move;
    playing_receive[PKT_REQUEST_AUDIO]		= Receive_audio_request;
    playing_receive[PKT_ASYNC_FPS]		= Receive_fps_request;
}

/*
 * Initialize the connection structures.
 */
int Setup_net_server(void)
{
    size_t	size;

    Init_receive();

    if (Init_setup() == -1) {
	return -1;
    }
    /*
     * The number of connections is limited by the number of bases
     * and the max number of possible file descriptors to use in
     * the select(2) call minus those for stdin, stdout, stderr,
     * the contact socket, and the socket for the resolver library routines.
     */
    max_connections = MIN(MAX_SELECT_FD - 5, World.NumBases);
    size = max_connections * sizeof(*Conn);
    if ((Conn = (connection_t *) malloc(size)) == NULL) {
	error("Cannot allocate memory for connections");
	return -1;
    }
    memset(Conn, 0, size);

    return 0;
}

static void Conn_set_state(connection_t *connp, int state, int drain_state)
{
    static int num_conn_busy;
    static int num_conn_playing;

    if ((connp->state & (CONN_PLAYING | CONN_READY)) != 0) {
	num_conn_playing--;
    }
    else if (connp->state == CONN_FREE) {
	num_conn_busy++;
    }

    connp->state = state;
    connp->drain_state = drain_state;
    connp->start = main_loops;

    if (connp->state == CONN_PLAYING) {
	num_conn_playing++;
	connp->timeout = IDLE_TIMEOUT;
    }
    else if (connp->state == CONN_READY) {
	num_conn_playing++;
	connp->timeout = READY_TIMEOUT;
    }
    else if (connp->state == CONN_LOGIN) {
	connp->timeout = LOGIN_TIMEOUT;
    }
    else if (connp->state == CONN_SETUP) {
	connp->timeout = SETUP_TIMEOUT;
    }
    else if (connp->state == CONN_LISTENING) {
	connp->timeout = LISTEN_TIMEOUT;
    }
    else if (connp->state == CONN_FREE) {
	num_conn_busy--;
	connp->timeout = IDLE_TIMEOUT;
    }

    login_in_progress = num_conn_busy - num_conn_playing;
}

/*
 * Cleanup a connection.  The client may not know yet that
 * it is thrown out of the game so we send it a quit packet.
 * We send it twice because of UDP it could get lost.
 * Since 3.0.6 the client receives a short message
 * explaining why the connection was terminated.
 */
void Destroy_connection(int ind, const char *reason)
{
    connection_t	*connp = &Conn[ind];
    int			id,
			len;
    sock_t		*sock;
    char		pkt[MAX_CHARS];

    if (connp->state == CONN_FREE) {
	errno = 0;
	error("Cannot destroy empty connection (\"%s\")", reason);
	return;
    }

    sock = &connp->w.sock;
    remove_input(sock->fd);

    pkt[0] = PKT_QUIT;
    strlcpy(&pkt[1], reason, sizeof(pkt) - 1);
    len = strlen(pkt) + 1;
    if (sock_write(sock, pkt, len) != len) {
	sock_get_error(sock);
	sock_write(sock, pkt, len);
    }
#ifndef SILENT
    xpprintf("%s Goodbye %s=%s@%s|%s (\"%s\")\n",
	   showtime(),
	   connp->nick ? connp->nick : "",
	   connp->real ? connp->real : "",
	   connp->host ? connp->host : "",
	   connp->dpy ? connp->dpy : "",
	   reason);
#endif

    Conn_set_state(connp, CONN_FREE, CONN_FREE);

    if (connp->id != NO_ID) {
	id = connp->id;
	connp->id = NO_ID;
	Players[GetInd[id]]->conn = NOT_CONNECTED;
	Delete_player(GetInd[id]);
    }
    if (connp->real != NULL) {
	free(connp->real);
    }
    if (connp->nick != NULL) {
	free(connp->nick);
    }
    if (connp->dpy != NULL) {
	free(connp->dpy);
    }
    if (connp->addr != NULL) {
	free(connp->addr);
    }
    if (connp->host != NULL) {
	free(connp->host);
    }
    Sockbuf_cleanup(&connp->w);
    Sockbuf_cleanup(&connp->r);
    Sockbuf_cleanup(&connp->c);

    num_logouts++;

    if (sock_write(sock, pkt, len) != len) {
	sock_get_error(sock);
	sock_write(sock, pkt, len);
    }
    sock_close(sock);

    memset(connp, 0, sizeof(*connp));
}

int Check_connection(char *real, char *nick, char *dpy, char *addr)
{
    int			i;
    connection_t	*connp;

    for (i = 0; i < max_connections; i++) {
	connp = &Conn[i];
	if (connp->state == CONN_LISTENING) {
	    if (strcasecmp(connp->nick, nick) == 0) {
		if (!strcmp(real, connp->real)
		    && !strcmp(dpy, connp->dpy)
		    && !strcmp(addr, connp->addr)) {
		    return connp->my_port;
		}
		return -1;
	    }
	}
    }
    return -1;
}

/*
 * A client has requested a playing connection with this server.
 * See if we have room for one more player and if his name is not
 * already in use by some other player.  Because the confirmation
 * may get lost we are willing to send it another time if the
 * client connection is still in the CONN_LISTENING state.
 */
int Setup_connection(char *real, char *nick, char *dpy, int team,
		     char *addr, char *host, unsigned version)
{
    int			i,
			free_conn_index = max_connections,
			my_port;
    sock_t		sock;
    connection_t	*connp;

    for (i = 0; i < max_connections; i++) {
	connp = &Conn[i];
	if (connp->state == CONN_FREE) {
	    if (free_conn_index == max_connections) {
		free_conn_index = i;
	    }
	    continue;
	}
	if (strcasecmp(connp->nick, nick) == 0) {
	    if (connp->state == CONN_LISTENING
		&& strcmp(real, connp->real) == 0
		&& strcmp(dpy, connp->dpy) == 0
		&& version == connp->version) {
		/*
		 * May happen for multi-homed hosts
		 * and if previous packet got lost.
		 */
		return connp->my_port;
	    } else {
		/*
		 * Nick already in use.
		 */
		return -1;
	    }
	}
    }

    if (free_conn_index >= max_connections) {
#ifndef SILENT
	xpprintf("%s Full house for %s(%s)@%s(%s)\n",
		 showtime(), real, nick, host, dpy);
#endif
	return -1;
    }
    connp = &Conn[free_conn_index];
    if (clientPortStart && (!clientPortEnd || clientPortEnd > 65535)) {
	clientPortEnd = 65535;
    }
    if (clientPortEnd && (!clientPortStart || clientPortStart < 1024)) {
	clientPortStart = 1024;
    }

    if (!clientPortStart || !clientPortEnd ||
	(clientPortStart > clientPortEnd)) {

        if (sock_open_udp(&sock, serverAddr, 0) == SOCK_IS_ERROR) {
            error("Cannot create datagram socket (%d)", sock.error.error);
            return -1;
        }
    }
    else {
	int found_socket = 0;
        for (i = clientPortStart; i <= clientPortEnd; i++) {
            if (sock_open_udp(&sock, serverAddr, i) != SOCK_IS_ERROR) {
		found_socket = 1;
                break;
	    }
	}
	if (found_socket == 0) {
	    error("Could not find a usable port in given port range");
	    return -1;
	}
    }

    if ((my_port = sock_get_port(&sock)) == -1) {
	error("Cannot get port from socket");
	sock_close(&sock);
	return -1;
    }
    if (sock_set_non_blocking(&sock, 1) == -1) {
	error("Cannot make client socket non-blocking");
	sock_close(&sock);
	return -1;
    }
    if (sock_set_receive_buffer_size(&sock, SERVER_RECV_SIZE + 256) == -1) {
	error("Cannot set receive buffer size to %d", SERVER_RECV_SIZE + 256);
    }
    if (sock_set_send_buffer_size(&sock, SERVER_SEND_SIZE + 256) == -1) {
	error("Cannot set send buffer size to %d", SERVER_SEND_SIZE + 256);
    }

    Sockbuf_init(&connp->w, &sock, SERVER_SEND_SIZE,
		 SOCKBUF_WRITE | SOCKBUF_DGRAM);

    Sockbuf_init(&connp->r, &sock, SERVER_RECV_SIZE,
		 SOCKBUF_READ | SOCKBUF_DGRAM);

    Sockbuf_init(&connp->c, (sock_t *) NULL, MAX_SOCKBUF_SIZE,
		 SOCKBUF_WRITE | SOCKBUF_READ | SOCKBUF_LOCK);

    connp->my_port = my_port;
    connp->real = xp_strdup(real);
    connp->nick = xp_strdup(nick);
    connp->dpy = xp_strdup(dpy);
    connp->addr = xp_strdup(addr);
    connp->host = xp_strdup(host);
    connp->ship = NULL;
    connp->team = team;
    connp->version = version;
    connp->start = main_loops;
    connp->magic = randomMT() + my_port + sock.fd + team + main_loops;
    connp->id = NO_ID;
    connp->timeout = LISTEN_TIMEOUT;
    connp->last_key_change = 0;
    connp->reliable_offset = 0;
    connp->reliable_unsent = 0;
    connp->last_send_loops = 0;
    connp->retransmit_at_loop = 0;
    connp->rtt_retransmit = DEFAULT_RETRANSMIT;
    connp->rtt_smoothed = 0;
    connp->rtt_dev = 0;
    connp->rtt_timeouts = 0;
    connp->acks = 0;
    connp->setup = 0;
    connp->motd_offset = -1;
    connp->motd_stop = 0;
    connp->view_width = DEF_VIEW_SIZE;
    connp->view_height = DEF_VIEW_SIZE;
    connp->debris_colors = 0;
    connp->spark_rand = DEF_SPARK_RAND;
    Conn_set_state(connp, CONN_LISTENING, CONN_FREE);
    if (connp->w.buf == NULL
	|| connp->r.buf == NULL
	|| connp->c.buf == NULL
	|| connp->real == NULL
	|| connp->nick == NULL
	|| connp->dpy == NULL
	|| connp->addr == NULL
	|| connp->host == NULL
	) {
	error("Not enough memory for connection");
	/* socket is not yet connected, but it doesn't matter much. */
	Destroy_connection(free_conn_index, "no memory");
	return -1;
    }

    install_input(Handle_input, sock.fd, (void *)free_conn_index);

    return my_port;
}

/*
 * Handle a connection that is in the listening state.
 */
static int Handle_listening(int ind)
{
    connection_t	*connp = &Conn[ind];
    unsigned char	type;
    int			n;
    char		nick[MAX_NAME_LEN],
			real[MAX_NAME_LEN];

    if (connp->state != CONN_LISTENING) {
	Destroy_connection(ind, "not listening");
	return -1;
    }
    Sockbuf_clear(&connp->r);
    errno = 0;
    n = sock_receive_any(&connp->r.sock, connp->r.buf, connp->r.size);
    if (n <= 0) {
	if (n == 0
	    || errno == EWOULDBLOCK
	    || errno == EAGAIN) {
	    n = 0;
	}
	else if (n != 0) {
	    Destroy_connection(ind, "read first packet error");
	}
	return n;
    }
    connp->r.len = n;
    connp->his_port = sock_get_last_port(&connp->r.sock);
    if (sock_connect(&connp->w.sock, connp->addr, connp->his_port) == -1) {
	error("Cannot connect datagram socket (%s,%d,%d,%d,%d)",
	      connp->addr, connp->his_port,
	      connp->w.sock.error.error,
	      connp->w.sock.error.call,
	      connp->w.sock.error.line);
	if (sock_get_error(&connp->w.sock)) {
	    error("sock_get_error fails too, giving up");
	    Destroy_connection(ind, "connect error");
	    return -1;
	}
	errno = 0;
	if (sock_connect(&connp->w.sock, connp->addr, connp->his_port) == -1) {
	    error("Still cannot connect datagram socket (%s,%d,%d,%d,%d)",
		  connp->addr, connp->his_port,
		  connp->w.sock.error.error,
		  connp->w.sock.error.call,
		  connp->w.sock.error.line);
	    Destroy_connection(ind, "connect error");
	    return -1;
	}
    }
#ifndef SILENT
    xpprintf("%s Welcome %s=%s@%s|%s (%s/%d)", showtime(), connp->nick,
	   connp->real, connp->host, connp->dpy, connp->addr, connp->his_port);
    if (connp->version != MY_VERSION) {
	xpprintf(" (version %04x)\n", connp->version);
    } else {
	xpprintf("\n");
    }
#endif
    if (connp->r.ptr[0] != PKT_VERIFY) {
	Send_reply(ind, PKT_VERIFY, PKT_FAILURE);
	Send_reliable(ind);
	Destroy_connection(ind, "not connecting");
	return -1;
    }
    if ((n = Packet_scanf(&connp->r, "%c%s%s",
			  &type, real, nick)) <= 0) {
	Send_reply(ind, PKT_VERIFY, PKT_FAILURE);
	Send_reliable(ind);
	Destroy_connection(ind, "verify incomplete");
	return -1;
    }
    Fix_real_name(real);
    Fix_nick_name(nick);
    if (strcmp(real, connp->real)) {
#ifndef SILENT
	xpprintf("%s Client verified incorrectly (%s,%s)(%s,%s)\n",
		 showtime(), real, nick, connp->real, connp->nick);
#endif
	Send_reply(ind, PKT_VERIFY, PKT_FAILURE);
	Send_reliable(ind);
	Destroy_connection(ind, "verify incorrect");
	return -1;
    }
    Sockbuf_clear(&connp->w);
    if (Send_reply(ind, PKT_VERIFY, PKT_SUCCESS) == -1
	|| Packet_printf(&connp->c, "%c%u", PKT_MAGIC, connp->magic) <= 0
	|| Send_reliable(ind) <= 0) {
	Destroy_connection(ind, "confirm failed");
	return -1;
    }

    Conn_set_state(connp, CONN_DRAIN, CONN_SETUP);

    return 1;	/* success! */
}

/*
 * Handle a connection that is in the transmit-server-configuration-data state.
 */
static int Handle_setup(int ind)
{
    connection_t	*connp = &Conn[ind];
    char		*buf;
    int			n,
			len;

    if (connp->state != CONN_SETUP) {
	Destroy_connection(ind, "not setup");
	return -1;
    }

    if (connp->setup == 0) {
	n = Packet_printf(&connp->c,
			  "%ld" "%ld%hd" "%hd%hd" "%hd%hd" "%s%s",
			  Setup->map_data_len,
			  Setup->mode, Setup->lives,
			  Setup->x, Setup->y,
			  Setup->frames_per_second, Setup->map_order,
			  Setup->name, Setup->author);
	if (n <= 0) {
	    Destroy_connection(ind, "setup 0 write error");
	    return -1;
	}
	connp->setup = (char *) &Setup->map_data[0] - (char *) Setup;
    }
    else if (connp->setup < Setup->setup_size) {
	if (connp->c.len > 0) {
	    /* If there is still unacked reliable data test for acks. */
	    Handle_input(-1, (void *) ind);
	    if (connp->state == CONN_FREE) {
		return -1;
	    }
	}
    }
    if (connp->setup < Setup->setup_size) {
	len = MIN(connp->c.size, 4096) - connp->c.len;
	if (len <= 0) {
	    /* Wait for acknowledgement of previously transmitted data. */
	    return 0;
	}
	if (len > Setup->setup_size - connp->setup) {
	    len = Setup->setup_size - connp->setup;
	}
	buf = (char *) Setup;
	if (Sockbuf_write(&connp->c, &buf[connp->setup], len) != len) {
	    Destroy_connection(ind, "sockbuf write setup error");
	    return -1;
	}
	connp->setup += len;
	if (len >= 512) {
	    connp->start += (len * FPS) / (8 * 512) + 1;
	}
    }
    if (connp->setup >= Setup->setup_size) {
	Conn_set_state(connp, CONN_DRAIN, CONN_LOGIN);
    }

    return 0;
}

/*
 * A client has requested to start active play.
 * See if we can allocate a player structure for it
 * and if this succeeds update the player information
 * to all connected players.
 */
static int Handle_login(int ind, char *errmsg, int errsize)
{
    connection_t	*connp = &Conn[ind];
    player		*pl;
    int			i,
			war_on_id,
			conn_bit;
    char		msg[MSG_LEN];

    if (NumPlayers - NumPseudoPlayers >= World.NumBases) {
	errno = 0;
	strlcpy(errmsg, "Not enough bases for players", errsize);
	error("%s", errmsg);
	return -1;
    }
    if (BIT(World.rules->mode, TEAM_PLAY)) {
	if (connp->team < 0 || connp->team >= MAX_TEAMS
	    || (reserveRobotTeam && (connp->team == robotTeam))) {
	    connp->team = TEAM_NOT_SET;
	}
	else if (World.teams[connp->team].NumBases <= 0) {
	    connp->team = TEAM_NOT_SET;
	}
	else {
	    Check_team_members(connp->team);
	    if (World.teams[connp->team].NumMembers
		- World.teams[connp->team].NumRobots
		>= World.teams[connp->team].NumBases) {
		connp->team = TEAM_NOT_SET;
	    }
	}
	if (connp->team == TEAM_NOT_SET) {
	    connp->team = Pick_team(PickForHuman);
	    if (connp->team == TEAM_NOT_SET ||
		(connp->team == robotTeam && reserveRobotTeam)) {
		errno = 0;
		strlcpy(errmsg, "Can't pick team", errsize);
		error("%s", errmsg);
		return -1;
	    }
	}
    } else {
	connp->team = TEAM_NOT_SET;
    }
    for (i = 0; i < NumPlayers; i++) {
	if (strcasecmp(Players[i]->name, connp->nick) == 0) {
	    errno = 0;
	    error("Name already in use %s", connp->nick);
	    strlcpy(errmsg, "Name already in use", errsize);
	    return -1;
	}
    }
    if (Init_player(NumPlayers, connp->ship) <= 0) {
	strlcpy(errmsg, "Init_player failed: no free ID", errsize);
	return -1;
    }
    pl = Players[NumPlayers];
    strlcpy(pl->name, connp->nick, MAX_CHARS);
    strlcpy(pl->realname, connp->real, MAX_CHARS);
    strlcpy(pl->hostname, connp->host, MAX_CHARS);
    pl->isowner = (!strcmp(pl->realname, Server.owner) &&
		   !strcmp(connp->addr, "127.0.0.1"));
    if (connp->team != TEAM_NOT_SET) {
	pl->team = connp->team;
    }
    pl->version = connp->version;

    Pick_startpos(NumPlayers);
    Go_home(NumPlayers);
    if (pl->team != TEAM_NOT_SET) {
	World.teams[pl->team].NumMembers++;
    }
    NumPlayers++;
    request_ID();
    connp->id = pl->id;
    pl->conn = ind;
    memset(pl->last_keyv, 0, sizeof(pl->last_keyv));
    memset(pl->prev_keyv, 0, sizeof(pl->prev_keyv));

    Conn_set_state(connp, CONN_READY, CONN_PLAYING);

    if (Send_reply(ind, PKT_PLAY, PKT_SUCCESS) <= 0) {
	strlcpy(errmsg, "Cannot send play reply", errsize);
	error("%s", errmsg);
	return -1;
    }

#ifndef	SILENT
    xpprintf("%s %s (%d) starts at startpos %d.\n", showtime(),
	   pl->name, NumPlayers, pl->home_base);
#endif

    /*
     * Tell him about himself first.
     */
    Send_player(pl->conn, pl->id);
    Send_score(pl->conn, pl->id, pl->score, pl->life, pl->mychar);
    Send_base(pl->conn, pl->id, pl->home_base);
    /*
     * And tell him about all the others.
     */
    for (i = 0; i < NumPlayers - 1; i++) {
	Send_player(pl->conn, Players[i]->id);
	Send_score(pl->conn, Players[i]->id,
		   Players[i]->score, Players[i]->life, Players[i]->mychar);
	if (!IS_TANK_IND(i)) {
	    Send_base(pl->conn, Players[i]->id, Players[i]->home_base);
	}
    }
    /*
     * And tell all the others about him.
     */
    for (i = 0; i < NumPlayers - 1; i++) {
	if (Players[i]->conn != NOT_CONNECTED) {
	    Send_player(Players[i]->conn, pl->id);
	    Send_score(Players[i]->conn, pl->id, pl->score,
		       pl->life, pl->mychar);
	    Send_base(Players[i]->conn, pl->id, pl->home_base);
	}
	/*
	 * And tell him about the relationships others have with eachother.
	 */
	else if (IS_ROBOT_IND(i)) {
	    if ((war_on_id = Robot_war_on_player(i)) != NO_ID) {
		Send_war(pl->conn, Players[i]->id, war_on_id);
	    }
	}
    }

    if (NumPlayers == 1) {
	sprintf(msg, "Welcome to \"%s\", made by %s.",
		World.name, World.author);
    } else if (BIT(World.rules->mode, TEAM_PLAY)) {
	sprintf(msg, "%s (%s, team %d) has entered \"%s\", made by %s.",
		pl->name, pl->realname, pl->team, World.name, World.author);
    } else {
	sprintf(msg, "%s (%s) has entered \"%s\", made by %s.",
		pl->name, pl->realname, World.name, World.author);
    }
    Set_message(msg);

    if (connp->version < MY_VERSION) {
	const char sender[] = "[*Server notice*]";
	sprintf(msg, "Server runs version %s. %s", VERSION, sender);
	Set_player_message(pl, msg);
	if (connp->version < 0x4401) {
	    sprintf(msg,
		    "Your client does not support the fast radar packet. %s",
		   sender);
	    Set_player_message(pl, msg);
	}
	if (connp->version < 0x4400 && maxAsteroidDensity > 0) {
	    sprintf(msg,
		    "Your client will see the %d asteroids as balls. %s",
		    (int)World.asteroids.max,
		    sender);
	    Set_player_message(pl, msg);
	}
    }

    conn_bit = (1 << ind);
    for (i = 0; i < World.NumCannons; i++) {
	/*
	 * The client assumes at startup that all cannons are active.
	 */
	if (World.cannon[i].dead_time == 0) {
	    SET_BIT(World.cannon[i].conn_mask, conn_bit);
	} else {
	    CLR_BIT(World.cannon[i].conn_mask, conn_bit);
	}
    }
    for (i = 0; i < World.NumFuels; i++) {
	/*
	 * The client assumes at startup that all fuelstations are filled.
	 */
	if (World.fuel[i].fuel == MAX_STATION_FUEL) {
	    SET_BIT(World.fuel[i].conn_mask, conn_bit);
	} else {
	    CLR_BIT(World.fuel[i].conn_mask, conn_bit);
	}
    }
    for (i = 0; i < World.NumTargets; i++) {
	/*
	 * The client assumes at startup that all targets are not damaged.
	 */
	if (World.targets[i].dead_time == 0
	    && World.targets[i].damage == TARGET_DAMAGE) {
	    SET_BIT(World.targets[i].conn_mask, conn_bit);
	    CLR_BIT(World.targets[i].update_mask, conn_bit);
	} else {
	    CLR_BIT(World.targets[i].conn_mask, conn_bit);
	    SET_BIT(World.targets[i].update_mask, conn_bit);
	}
    }

    sound_player_init(pl);

    sound_play_all(START_SOUND);

    num_logins++;

    if (resetOnHuman > 0
	&& (NumPlayers - NumPseudoPlayers - NumRobots) <= resetOnHuman
	&& !round_delay) {
	if (BIT(World.rules->mode, TIMING)) {
	    Race_game_over();
	} else if (BIT(World.rules->mode, TEAM_PLAY)) {
	    Team_game_over(-1, "");
	} else if (BIT(World.rules->mode, LIMITED_LIVES)) {
	    Individual_game_over(-1);
	}
    }

    /* if the next round is delayed, delay it again */
    if (round_delay > 0 || NumPlayers == 1) {
	round_delay = roundDelaySeconds * FPS;
	roundtime = -1;
	sprintf(msg, "Player entered. Delaying %d seconds until next %s.",
		roundDelaySeconds, (BIT(World.rules->mode, TIMING) ?
			     "race" : "round"));
	Set_message(msg);
    }

    return 0;
}

/*
 * Process a client packet.
 * The client may be in one of several states,
 * therefore we use function dispatch tables for easy processing.
 * Some functions may process requests from clients being
 * in different states.
 */
static void Handle_input(int fd, void *arg)
{
    int			ind = (int) arg;
    connection_t	*connp = &Conn[ind];
    int			type,
			result,
			(**receive_tbl)(int ind);

    if (connp->state & (CONN_PLAYING | CONN_READY)) {
	receive_tbl = &playing_receive[0];
    }
    else if (connp->state == CONN_LOGIN) {
	receive_tbl = &login_receive[0];
    }
    else if (connp->state & (CONN_DRAIN | CONN_SETUP)) {
	receive_tbl = &drain_receive[0];
    }
    else if (connp->state == CONN_LISTENING) {
	Handle_listening(ind);
	return;
    } else {
	if (connp->state != CONN_FREE) {
	    Destroy_connection(ind, "not input");
	}
	return;
    }
    connp->num_keyboard_updates = 0;

    Sockbuf_clear(&connp->r);
    if (Sockbuf_read(&connp->r) == -1) {
	Destroy_connection(ind, "input error");
	return;
    }
    if (connp->r.len <= 0) {
	/*
	 * No input.
	 */
	return;
    }
    while (connp->r.ptr < connp->r.buf + connp->r.len) {
	type = (connp->r.ptr[0] & 0xFF);
	result = (*receive_tbl[type])(ind);
	if (result == -1) {
	    /*
	     * Unrecoverable error.
	     * Connection has been destroyed.
	     */
	    return;
	}
	if (result == 0) {
	    /*
	     * Incomplete client packet.
	     * Drop rest of packet.
	     */
	    Sockbuf_clear(&connp->r);
	    break;
	}
	if (connp->state == CONN_PLAYING) {
	    connp->start = main_loops;
	}
    }
}

int Input(void)
{
    int			i,
			ind,
			num_reliable = 0,
			input_reliable[MAX_SELECT_FD];
    connection_t	*connp;
    char		msg[MSG_LEN];

    for (i = 0; i < max_connections; i++) {
	connp = &Conn[i];
	if (connp->state == CONN_FREE) {
	    continue;
	}
	if (connp->start + connp->timeout * FPS < main_loops) {
	    /*
	     * Timeout this fellow if we have not heard a single thing
	     * from him for a long time.
	     */
	    if (connp->state & (CONN_PLAYING | CONN_READY)) {
		sprintf(msg, "%s mysteriously disappeared!?", connp->nick);
		Set_message(msg);
	    }
	    sprintf(msg, "timeout %02x", connp->state);
	    Destroy_connection(i, msg);
	    continue;
	}
	if (connp->state != CONN_PLAYING) {
	    input_reliable[num_reliable++] = i;
	    if (connp->state == CONN_SETUP) {
		Handle_setup(i);
		continue;
	    }
	}
    }

    for (i = 0; i < num_reliable; i++) {
	ind = input_reliable[i];
	connp = &Conn[ind];
	if (connp->state & (CONN_DRAIN | CONN_READY | CONN_SETUP
			    | CONN_LOGIN)) {
	    if (connp->c.len > 0) {
		if (Send_reliable(ind) == -1) {
		    continue;
		}
	    }
	}
    }

    if (num_logins | num_logouts) {
	/* Tell the meta server */
	Meta_update(1);
	num_logins = 0;
	num_logouts = 0;
    }

    return login_in_progress;
}

/*
 * Send a reply to a special client request.
 * Not used consistently everywhere.
 * It could be used to setup some form of reliable
 * communication from the client to the server.
 */
int Send_reply(int ind, int replyto, int result)
{
    connection_t	*connp = &Conn[ind];
    int			n;

    n = Packet_printf(&connp->c, "%c%c%c", PKT_REPLY, replyto, result);
    if (n == -1) {
	Destroy_connection(ind, "write error");
	return -1;
    }
    return n;
}

static int Send_modifiers(int ind, char *mods)
{
    return Packet_printf(&Conn[ind].w, "%c%s", PKT_MODIFIERS, mods);
}

/*
 * Send items.
 * The advantage of this scheme is that it only uses bytes for items
 * which the player actually owns.  This reduces the packet size.
 * Another advantage is that here it doesn't matter if an old client
 * receives counts for items it doesn't know about.
 * This is new since pack version 4203.
 */
static int Send_self_items(int ind, player *pl)
{
    connection_t	*connp = &Conn[ind];
    unsigned		item_mask = 0;
    int			i, n;
    int			item_count = 0;

    /* older clients should have the items sent as part of the self packet. */
    if (connp->version < 0x4203) {
	return 1;
    }
    /* build mask with one bit for each item type which the player owns. */
    for (i = 0; i < NUM_ITEMS; i++) {
	if (pl->item[i] > 0) {
	    item_mask |= (1 << i);
	    item_count++;
	}
    }
    /* don't send anything if there are no items. */
    if (item_count == 0) {
	return 1;
    }
    /* check if enough buffer space is available for the complete packet. */
    if (connp->w.size - connp->w.len <= 5 + item_count) {
	return 0;
    }
    /* build the header. */
    n = Packet_printf(&connp->w, "%c%u", PKT_SELF_ITEMS, item_mask);
    if (n <= 0) {
	return n;
    }
    /* build rest of packet containing the per item counts. */
    for (i = 0; i < NUM_ITEMS; i++) {
	if (item_mask & (1 << i)) {
	    connp->w.buf[connp->w.len++] = pl->item[i];
	}
    }
    /* return the number of bytes added to the packet. */
    return 5 + item_count;
}

/*
 * Send all frame data related to the player self and his HUD.
 */
int Send_self(int ind,
	      player *pl,
	      int lock_id,
	      int lock_dist,
	      int lock_dir,
	      int autopilotlight,
	      long status,
	      char *mods)
{
    connection_t	*connp = &Conn[ind];
    int			n;
    u_byte		stat = (u_byte)status;
    int			sbuf_len = connp->w.len;

    if (connp->version >= 0x4203) {
	n = Packet_printf(&connp->w,
			  "%c"
			  "%hd%hd%hd%hd%c"
			  "%c%c%c"
			  "%hd%hd%c%c"
			  "%c%hd%hd"
			  "%hd%hd%c"
			  "%c%c"
			  ,
			  PKT_SELF,
			  (int) (pl->pos.x + 0.5), (int) (pl->pos.y + 0.5),
			  (int) pl->vel.x, (int) pl->vel.y,
			  pl->dir,
			  (int) (pl->power + 0.5),
			  (int) (pl->turnspeed + 0.5),
			  (int) (pl->turnresistance * 255.0 + 0.5),
			  lock_id, lock_dist, lock_dir,
			  pl->check,

			  pl->fuel.current,
			  pl->fuel.sum >> FUEL_SCALE_BITS,
			  pl->fuel.max >> FUEL_SCALE_BITS,

			  connp->view_width, connp->view_height,
			  connp->debris_colors,

			  stat,
			  autopilotlight

			  );
	if (n <= 0) {
	    return n;
	}
	n = Send_self_items(ind, pl);
	if (n <= 0) {
	    return n;
	}
	return Send_modifiers(ind, mods);
    }

    n = Packet_printf(&connp->w,
		      "%c"
		      "%hd%hd%hd%hd%c"
		      "%c%c%c"
		      "%hd%hd%c%c"
		      "%c%c%c%c%c"
		      "%c%c%c%c%c"
		      "%c%c%c%c"
		      "%c%hd%hd"
		      "%hd%hd%c"
		      "%c%c"
		      ,
		      PKT_SELF,
		      (int) (pl->pos.x + 0.5), (int) (pl->pos.y + 0.5),
		      (int) pl->vel.x, (int) pl->vel.y,
		      pl->dir,
		      (int) (pl->power + 0.5),
		      (int) (pl->turnspeed + 0.5),
		      (int) (pl->turnresistance * 255.0 + 0.5),
		      lock_id, lock_dist, lock_dir,
		      pl->check,

		      pl->item[ITEM_CLOAK],
		      pl->item[ITEM_SENSOR],
		      pl->item[ITEM_MINE],
		      pl->item[ITEM_MISSILE],
		      pl->item[ITEM_ECM],

		      pl->item[ITEM_TRANSPORTER],
		      pl->item[ITEM_WIDEANGLE],
		      pl->item[ITEM_REARSHOT],
		      pl->item[ITEM_AFTERBURNER],
		      pl->fuel.num_tanks,

		      pl->item[ITEM_LASER],
		      pl->item[ITEM_EMERGENCY_THRUST],
		      pl->item[ITEM_TRACTOR_BEAM],
		      pl->item[ITEM_AUTOPILOT],

		      pl->fuel.current,
		      pl->fuel.sum >> FUEL_SCALE_BITS,
		      pl->fuel.max >> FUEL_SCALE_BITS,

		      connp->view_width, connp->view_height,
		      connp->debris_colors,

		      stat,
		      autopilotlight

		      );
    if (n <= 0) {
	return n;
    }
    if (connp->version >= 0x3800) {
	n = Packet_printf(&connp->w,
			  "%c%c%c%c",	/* %c", */
			  pl->item[ITEM_EMERGENCY_SHIELD],
			  pl->item[ITEM_DEFLECTOR],
			  pl->item[ITEM_HYPERJUMP],
			  pl->item[ITEM_PHASING] /* ,
			  pl->item[ITEM_MIRROR] */
			  );
	if (n <= 0) {
	    connp->w.len = sbuf_len;
	    return n;
	}
	if (connp->version >= 0x4100) {
	    n = Packet_printf(&connp->w,
			      "%c",
			      pl->item[ITEM_MIRROR]
			      );
	    if (n <= 0) {
		connp->w.len = sbuf_len;
		return n;
	    }
	    if (connp->version >= 0x4201) {
		n = Packet_printf(&connp->w,
				  "%c",
				  pl->item[ITEM_ARMOR]
				  );
		if (n <= 0) {
		    connp->w.len = sbuf_len;
		    return n;
		}
	    }
	}
    }
    else if (connp->version >= 0x3200) {
	n = Packet_printf(&connp->w,
			  "%c",
			  pl->item[ITEM_EMERGENCY_SHIELD]);
	if (n <= 0) {
	    connp->w.len = sbuf_len;
	    return n;
	}
    }

    return Send_modifiers(ind, mods);
}

/*
 * Somebody is leaving the game.
 */
int Send_leave(int ind, int id)
{
    connection_t	*connp = &Conn[ind];

    if (!BIT(connp->state, CONN_PLAYING | CONN_READY)) {
	errno = 0;
	error("Connection not ready for leave info (%d,%d)",
	      connp->state, connp->id);
	return 0;
    }
    return Packet_printf(&connp->c, "%c%hd", PKT_LEAVE, id);
}

/*
 * Somebody is declaring war.
 */
int Send_war(int ind, int robot_id, int killer_id)
{
    connection_t	*connp = &Conn[ind];

    if (!BIT(connp->state, CONN_PLAYING | CONN_READY)) {
	errno = 0;
	error("Connection not ready for war declaration (%d,%d,%d)",
	      ind, connp->state, connp->id);
	return 0;
    }
    return Packet_printf(&connp->c, "%c%hd%hd", PKT_WAR,
			 robot_id, killer_id);
}

/*
 * Somebody is programming a robot to seek some player.
 */
int Send_seek(int ind, int programmer_id, int robot_id, int sought_id)
{
    connection_t	*connp = &Conn[ind];

    if (!BIT(connp->state, CONN_PLAYING | CONN_READY)) {
	errno = 0;
	error("Connection not ready for seek declaration (%d,%d,%d)",
	      ind, connp->state, connp->id);
	return 0;
    }
    return Packet_printf(&connp->c, "%c%hd%hd%hd", PKT_SEEK,
			 programmer_id, robot_id, sought_id);
}

/*
 * Somebody is joining the game.
 */
int Send_player(int ind, int id)
{
    connection_t	*connp = &Conn[ind];
    player		*pl = Players[GetInd[id]];
    int			n;
    char		buf[MSG_LEN], ext[MSG_LEN];
    int			sbuf_len = connp->c.len;

    if (!BIT(connp->state, CONN_PLAYING|CONN_READY)) {
	errno = 0;
	error("Connection not ready for player info (%d,%d)",
	      connp->state, connp->id);
	return 0;
    }
    Convert_ship_2_string(pl->ship, buf, ext,
			  (connp->version < 0x3200) ? 0x3100 : 0x3200);
    n = Packet_printf(&connp->c,
		      "%c%hd" "%c%c" "%s%s%s" "%S",
		      PKT_PLAYER, pl->id,
		      pl->team, pl->mychar,
		      pl->name, pl->realname, pl->hostname,
		      buf);
    if (connp->version > 0x3200) {
	if (n > 0) {
	    n = Packet_printf(&connp->c, "%S", ext);
	    if (n <= 0) {
		connp->c.len = sbuf_len;
	    }
	}
    }
    return n;
}

/*
 * Send the new score for some player to a client.
 */
int Send_score(int ind, int id, int score, int life, int mychar)
{
    connection_t	*connp = &Conn[ind];

    if (!BIT(connp->state, CONN_PLAYING | CONN_READY)) {
	errno = 0;
	error("Connection not ready for score(%d,%d)",
	    connp->state, connp->id);
	return 0;
    }
    return Packet_printf(&connp->c, "%c%hd%hd%hd%c", PKT_SCORE,
			 id, score, life, mychar);
}

/*
 * Send the new race info for some player to a client.
 */
int Send_timing(int ind, int id, int check, int round)
{
    connection_t	*connp = &Conn[ind];

    if (connp->version < 0x3261) {
	return 1;
    }
    if (!BIT(connp->state, CONN_PLAYING | CONN_READY)) {
	errno = 0;
	error("Connection not ready for timing(%d,%d)",
	      connp->state, connp->id);
	return 0;
    }
    return Packet_printf(&connp->c, "%c%hd%hu", PKT_TIMING,
			 id, round * MAX_CHECKS + check);
}

/*
 * Send info about a player having which base.
 */
int Send_base(int ind, int id, int num)
{
    connection_t	*connp = &Conn[ind];

    if (!BIT(connp->state, CONN_PLAYING | CONN_READY)) {
	errno = 0;
	error("Connection not ready for base info (%d,%d)",
	    connp->state, connp->id);
	return 0;
    }
    return Packet_printf(&connp->c, "%c%hd%hu", PKT_BASE, id, num);
}

/*
 * Send the amount of fuel in a fuelstation.
 */
int Send_fuel(int ind, int num, int fuel)
{
    return Packet_printf(&Conn[ind].w, "%c%hu%hu", PKT_FUEL,
			 num, fuel >> FUEL_SCALE_BITS);
}

int Send_score_object(int ind, int score, int x, int y, const char *string)
{
    connection_t	*connp = &Conn[ind];

    if (!BIT(connp->state, CONN_PLAYING | CONN_READY)) {
	errno = 0;
	error("Connection not ready for base info (%d,%d)",
	    connp->state, connp->id);
	return 0;
    }
    return Packet_printf(&Conn[ind].c, "%c%hd%hu%hu%s",PKT_SCORE_OBJECT,
			 score, x, y, string);
}

int Send_cannon(int ind, int num, int dead_time)
{
    return Packet_printf(&Conn[ind].w, "%c%hu%hu", PKT_CANNON,
	num, dead_time);
}

int Send_destruct(int ind, int count)
{
    return Packet_printf(&Conn[ind].w, "%c%hd", PKT_DESTRUCT, count);
}

int Send_shutdown(int ind, int count, int delay)
{
    return Packet_printf(&Conn[ind].w, "%c%hd%hd", PKT_SHUTDOWN,
	count, delay);
}

int Send_thrusttime(int ind, int count, int max)
{
    return Packet_printf(&Conn[ind].w, "%c%hd%hd", PKT_THRUSTTIME, count, max);
}

int Send_shieldtime(int ind, int count, int max)
{
    if (Conn[ind].version < 0x3200) {
	return 1;
    }
    return Packet_printf(&Conn[ind].w, "%c%hd%hd", PKT_SHIELDTIME, count, max);
}

int Send_phasingtime(int ind, int count, int max)
{
    if (Conn[ind].version < 0x3800) {
	return 1;
    }
    return Packet_printf(&Conn[ind].w, "%c%hd%hd", PKT_PHASINGTIME, count, max);
}

int Send_rounddelay(int ind, int count, int max)
{
    if (Conn[ind].version < 0x3800) {
	return 1;
    }
    return(Packet_printf(&Conn[ind].w, "%c%hd%hd", PKT_ROUNDDELAY, count, max));
}

int Send_debris(int ind, int type, unsigned char *p, int n)
{
    int			avail;
    sockbuf_t		*w = &Conn[ind].w;

    if ((n & 0xFF) != n) {
	errno = 0;
	error("Bad number of debris %d", n);
	return 0;
    }
    avail = w->size - w->len - SOCKBUF_WRITE_SPARE - 2;
    if (n * 2 >= avail) {
	if (avail > 2) {
	    n = (avail - 1) / 2;
	} else {
	    return 0;
	}
    }
    w->buf[w->len++] = PKT_DEBRIS + type;
    w->buf[w->len++] = n;
    memcpy(&w->buf[w->len], p, n * 2);
    w->len += n * 2;

    return n;
}

int Send_wreckage(int ind, int x, int y, u_byte wrtype, u_byte size, u_byte rot)
{
    if (Conn[ind].version < 0x3800) {
	return 1;
    }

    if (wreckageCollisionMayKill && Conn[ind].version > 0x4201) {
	/* Set the highest bit when wreckage is deadly. */
	wrtype |= 0x80;
    } else {
	wrtype &= ~0x80;
    }

    return Packet_printf(&Conn[ind].w, "%c%hd%hd%c%c%c", PKT_WRECKAGE,
			 x, y, wrtype, size, rot);
}

int Send_asteroid(int ind, int x, int y, u_byte type, u_byte size, u_byte rot)
{
    u_byte	type_size;

    if (Conn[ind].version < 0x4400) {
	return Send_ecm(ind, x, y, 2 * (int) ASTEROID_RADIUS(size));
    }

    type_size = ((type & 0x0F) << 4) | (size & 0x0F);

    return Packet_printf(&Conn[ind].w, "%c%hd%hd%c%c", PKT_ASTEROID,
		         x, y, type_size, rot);
}

int Send_fastshot(int ind, int type, unsigned char *p, int n)
{
    int			avail;
    sockbuf_t		*w = &Conn[ind].w;

    if ((n & 0xFF) != n) {
	errno = 0;
	error("Bad number of fastshot %d", n);
	return 0;
    }
    avail = w->size - w->len - SOCKBUF_WRITE_SPARE - 3;
    if (n * 2 >= avail) {
	if (avail > 2) {
	    n = (avail - 1) / 2;
	} else {
	    return 0;
	}
    }
    w->buf[w->len++] = PKT_FASTSHOT;
    w->buf[w->len++] = type;
    w->buf[w->len++] = n;
    memcpy(&w->buf[w->len], p, n * 2);
    w->len += n * 2;

    return n;
}

int Send_missile(int ind, int x, int y, int len, int dir)
{
    return Packet_printf(&Conn[ind].w, "%c%hd%hd%c%c",
			 PKT_MISSILE, x, y, len, dir);
}

int Send_ball(int ind, int x, int y, int id)
{
    return Packet_printf(&Conn[ind].w, "%c%hd%hd%hd", PKT_BALL, x, y, id);
}

int Send_mine(int ind, int x, int y, int teammine, int id)
{
    connection_t	*connp = &Conn[ind];

    return Packet_printf(&connp->w, "%c%hd%hd%c%hd", PKT_MINE, x, y,
			 teammine, id);
}

int Send_target(int ind, int num, int dead_time, int damage)
{
    return Packet_printf(&Conn[ind].w, "%c%hu%hu%hu", PKT_TARGET,
			 num, dead_time, damage);
}

int Send_item(int ind, int x, int y, int type)
{
    if (type >= ITEM_EMERGENCY_SHIELD) {
	if (Conn[ind].version < 0x3200) {
	    return 1;
	}
    }
    return Packet_printf(&Conn[ind].w, "%c%hd%hd%c", PKT_ITEM, x, y, type);
}

int Send_paused(int ind, int x, int y, int count)
{
    return Packet_printf(&Conn[ind].w, "%c%hd%hd%hd", PKT_PAUSED, x, y, count);
}

int Send_ecm(int ind, int x, int y, int size)
{
    return Packet_printf(&Conn[ind].w, "%c%hd%hd%hd", PKT_ECM, x, y, size);
}

int Send_trans(int ind, int x1, int y1, int x2, int y2)
{
    return Packet_printf(&Conn[ind].w,"%c%hd%hd%hd%hd",
			 PKT_TRANS, x1, y1, x2, y2);
}

int Send_ship(int ind, int x, int y, int id, int dir,
	      int shield, int cloak, int emergency_shield, int phased, int deflector)
{
    if (Conn[ind].version < 0x4300) {
	/* cloaking bit was also true if phased and that was used
	 * to determine how to draw the ship.
	 */
	cloak = (cloak || phased);
    }
    return Packet_printf(&Conn[ind].w,
			 "%c%hd%hd%hd" "%c" "%c",
			 PKT_SHIP, x, y, id,
			 dir,
			 (shield != 0) 
				| ((cloak != 0) << 1)
			    | ((emergency_shield != 0) << 2)
				| ((phased != 0) << 3)		/* clients older than 3.8.0 will ignore this */
				| ((deflector != 0) << 4)	/* clients older than 3.8.0 will ignore this */
			);
}

int Send_refuel(int ind, int x0, int y0, int x1, int y1)
{
    return Packet_printf(&Conn[ind].w,
			 "%c%hd%hd%hd%hd",
			 PKT_REFUEL, x0, y0, x1, y1);
}

int Send_connector(int ind, int x0, int y0, int x1, int y1, int tractor)
{
    connection_t *connp = &Conn[ind];

    return Packet_printf(&connp->w,
			 "%c%hd%hd%hd%hd%c",
			 PKT_CONNECTOR, x0, y0, x1, y1, tractor);
}

int Send_laser(int ind, int color, int x, int y, int len, int dir)
{
    connection_t *connp = &Conn[ind];

    return Packet_printf(&connp->w, "%c%c%hd%hd%hd%c", PKT_LASER,
			 color, x, y, len, dir);
}

int Send_radar(int ind, int x, int y, int size)
{
    connection_t *connp = &Conn[ind];

    /* Only since 4.2.1 can clients correctly handle teammates in green. */
    /* Except the original patch from kth.se was 4.1.0 "experimental 1" */
    if (connp->version < 0x4210 && connp->version != 0x4101) {
	size &= ~0x80;
    }
    return Packet_printf(&connp->w, "%c%hd%hd%c", PKT_RADAR, x, y, size);
}

int Send_fastradar(int ind, unsigned char *buf, int n)
{
    int			avail;
    sockbuf_t		*w = &Conn[ind].w;

    if ((n & 0xFF) != n) {
	errno = 0;
	error("Bad number of fastradar %d", n);
	return 0;
    }
    avail = w->size - w->len - SOCKBUF_WRITE_SPARE - 3;
    if (n * 3 >= avail) {
	if (avail > 3) {
	    n = (avail - 2) / 3;
	} else {
	    return 0;
	}
    }
    w->buf[w->len++] = PKT_FASTRADAR;
    w->buf[w->len++] = (unsigned char)(n & 0xFF);
    memcpy(&w->buf[w->len], buf, n * 3);
    w->len += n * 3;

    return (2 + (n * 3));
}

int Send_damaged(int ind, int damaged)
{
    return Packet_printf(&Conn[ind].w, "%c%c", PKT_DAMAGED, damaged);
}

int Send_audio(int ind, int type, int vol)
{
    connection_t *connp = &Conn[ind];

    if (connp->w.size - connp->w.len <= 32) {
	return 0;
    }
    return Packet_printf(&connp->w, "%c%c%c", PKT_AUDIO, type, vol);
}

int Send_time_left(int ind, long sec)
{
    connection_t *connp = &Conn[ind];

    return Packet_printf(&connp->w, "%c%ld", PKT_TIME_LEFT, sec);
}

int Send_eyes(int ind, int id)
{
    connection_t *connp = &Conn[ind];

    return Packet_printf(&connp->w, "%c%hd", PKT_EYES, id);
}

int Send_message(int ind, const char *msg)
{
    connection_t	*connp = &Conn[ind];

    if (!BIT(connp->state, CONN_PLAYING | CONN_READY)) {
	errno = 0;
	error("Connection not ready for message (%d,%d)",
	    connp->state, connp->id);
	return 0;
    }
    return Packet_printf(&connp->c, "%c%S", PKT_MESSAGE, msg);
}

int Send_loseitem(int lose_item_index, int ind)
{
    if (Conn[ind].version < 0x3400) { /* this should never hit since */
				      /* only a 3.4+ client would send */
				      /* the loseitem key */
	return 1;
    }
    return Packet_printf(&Conn[ind].w, "%c%c", PKT_LOSEITEM, lose_item_index);
}

int Send_start_of_frame(int ind)
{
    connection_t	*connp = &Conn[ind];

    if (connp->state != CONN_PLAYING) {
	if (connp->state != CONN_READY) {
	    errno = 0;
	    error("Connection not ready for frame (%d,%d)",
		connp->state, connp->id);
	}
	return -1;
    }
    /*
     * We tell the client which frame number this is and
     * which keyboard update we have last received.
     */
    Sockbuf_clear(&connp->w);
    if (Packet_printf(&connp->w,
		      "%c%ld%ld",
		      PKT_START, frame_loops, connp->last_key_change) <= 0) {
	Destroy_connection(ind, "write error");
	return -1;
    }

    /* Return ok */
    return 0;
}

int Send_end_of_frame(int ind)
{
    connection_t	*connp = &Conn[ind];
    int			n;

    last_packet_of_frame = 1;
    n = Packet_printf(&connp->w, "%c%ld", PKT_END, frame_loops);
    last_packet_of_frame = 0;
    if (n == -1) {
	Destroy_connection(ind, "write error");
	return -1;
    }
    if (n == 0) {
	/*
	 * Frame update size exceeded buffer size.
	 * Drop this packet.
	 */
	Sockbuf_clear(&connp->w);
	return 0;
    }
    while (connp->motd_offset >= 0
	&& connp->c.len + connp->w.len < MAX_RELIABLE_DATA_PACKET_SIZE) {
	Send_motd(ind);
    }
    if (connp->c.len > 0 && connp->w.len < MAX_RELIABLE_DATA_PACKET_SIZE) {
	if (Send_reliable(ind) == -1) {
	    return -1;
	}
	if (connp->w.len == 0) {
	    return 1;
	}
    }
    if (Sockbuf_flush(&connp->w) == -1) {
	Destroy_connection(ind, "flush error");
	return -1;
    }
    Sockbuf_clear(&connp->w);
    return 0;
}

static int Receive_keyboard(int ind)
{
    connection_t	*connp = &Conn[ind];
    player		*pl;
    long		change;
    u_byte		ch;
    int			size = KEYBOARD_SIZE;

    if (connp->version < 0x3800) {
	/* older servers have a keyboard_size of 8 bytes instead of 9. */
	size--;
    }
    if (connp->r.ptr - connp->r.buf + size + 1 + 4 > connp->r.len) {
	/*
	 * Incomplete client packet.
	 */
	return 0;
    }
    Packet_scanf(&connp->r, "%c%ld", &ch, &change);
    if (change <= connp->last_key_change) {
	/*
	 * We already have this key.
	 * Nothing to do.
	 */
	connp->r.ptr += size;
    }
    else {
	connp->last_key_change = change;
	pl = Players[GetInd[connp->id]];
	memcpy(pl->last_keyv, connp->r.ptr, size);
	connp->r.ptr += size;
	Handle_keyboard(GetInd[connp->id]);
    }
    if (connp->num_keyboard_updates++ && (connp->state & CONN_PLAYING)) {
	Destroy_connection(ind, "no macros");
	return -1;
    }

    return 1;
}

static int Receive_quit(int ind)
{
    Destroy_connection(ind, "client quit");

    return -1;
}

static int Receive_play(int ind)
{
    connection_t	*connp = &Conn[ind];
    unsigned char	ch;
    int			n;
    char		errmsg[MAX_CHARS];

    if ((n = Packet_scanf(&connp->r, "%c", &ch)) != 1) {
	errno = 0;
	error("Cannot receive play packet");
	Destroy_connection(ind, "receive error");
	return -1;
    }
    if (ch != PKT_PLAY) {
	errno = 0;
	error("Packet is not of play type");
	Destroy_connection(ind, "not play");
	return -1;
    }
    if (connp->state != CONN_LOGIN) {
	if (connp->state != CONN_PLAYING) {
	    if (connp->state == CONN_READY) {
		connp->r.ptr = connp->r.buf + connp->r.len;
		return 0;
	    }
	    errno = 0;
	    error("Connection not in login state (%02x)", connp->state);
	    Destroy_connection(ind, "not login");
	    return -1;
	}
	if (Send_reliable(ind) == -1) {
	    return -1;
	}
	return 0;
    }
    Sockbuf_clear(&connp->w);
    strlcpy(errmsg, "login failed", sizeof(errmsg));
    if (Handle_login(ind, errmsg, sizeof(errmsg)) == -1) {
	Destroy_connection(ind, errmsg);
	return -1;
    }

    return 2;
}

static int Receive_power(int ind)
{
    connection_t	*connp = &Conn[ind];
    player		*pl;
    unsigned char	ch;
    short		tmp;
    int			n;
    DFLOAT		power;
    int			autopilot;

    if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &tmp)) <= 0) {
	if (n == -1) {
	    Destroy_connection(ind, "read error");
	}
	return n;
    }
    power = (DFLOAT) tmp / 256.0F;
    pl = Players[GetInd[connp->id]];
    autopilot = BIT(pl->used, HAS_AUTOPILOT);
    /* old client are going to send autopilot-mangled data, ignore it */
    if (autopilot && pl->version < 0x4200)
	return 1;

    switch (ch) {
    case PKT_POWER:
	if (autopilot)
	    pl->auto_power_s = power;
	else
	    pl->power = power;
	break;
    case PKT_POWER_S:
	pl->power_s = power;
	break;
    case PKT_TURNSPEED:
	if (autopilot)
	    pl->auto_turnspeed_s = power;
	else
	    pl->turnspeed = power;
	break;
    case PKT_TURNSPEED_S:
	pl->turnspeed_s = power;
	break;
    case PKT_TURNRESISTANCE:
	if (autopilot)
	    pl->auto_turnresistance_s = power;
	else
	    pl->turnresistance = power;
	break;
    case PKT_TURNRESISTANCE_S:
	pl->turnresistance_s = power;
	break;
    default:
	errno = 0;
	error("Not a power packet (%d,%02x)", ch, connp->state);
	Destroy_connection(ind, "not power");
	return -1;
    }
    return 1;
}

/*
 * Send the reliable data.
 * If the client is in the receive-frame-updates state then
 * all reliable data is piggybacked at the end of the
 * frame update packets.  (Except maybe for the MOTD data, which
 * could be transmitted in its own packets since MOTDs can be big.)
 * Otherwise if the client is not actively playing yet then
 * the reliable data is sent in its own packets since there
 * is no other data to combine it with.
 *
 * This thing still is not finished, but it works better than in 3.0.0 I hope.
 */
int Send_reliable(int ind)
{
    connection_t	*connp = &Conn[ind];
    char		*read_buf;
    int			i,
			n,
			len,
			todo,
			max_todo;
    long		rel_off;
    const int		max_packet_size = MAX_RELIABLE_DATA_PACKET_SIZE,
			min_send_size = 1;  /* was 4 in 3.0.7, 1 in 3.1.0 */

    if (connp->c.len <= 0
	|| connp->last_send_loops == main_loops) {
	connp->last_send_loops = main_loops;
	return 0;
    }
    read_buf = connp->c.buf;
    max_todo = connp->c.len;
    rel_off = connp->reliable_offset;
    if (connp->w.len > 0) {
	/* We are piggybacking on a frame update. */
	if (connp->w.len >= max_packet_size - min_send_size) {
	    /* Frame already too big */
	    return 0;
	}
	if (max_todo > max_packet_size - connp->w.len) {
	    /* Do not exceed minimum fragment size. */
	    max_todo = max_packet_size - connp->w.len;
	}
    }
    if (connp->retransmit_at_loop > main_loops) {
	/*
	 * It is no time to retransmit yet.
	 */
	if (max_todo <= connp->reliable_unsent - connp->reliable_offset
			+ min_send_size
	    || connp->w.len == 0) {
	    /*
	     * And we cannot send anything new either
	     * and we do not want to introduce a new packet.
	     */
	    return 0;
	}
    }
    else if (connp->retransmit_at_loop != 0) {
	/*
	 * Timeout.
	 * Either our packet or the acknowledgement got lost,
	 * so retransmit.
	 */
	connp->acks >>= 1;
    }

    todo = max_todo;
    for (i = 0; i <= connp->acks && todo > 0; i++) {
	len = (todo > max_packet_size) ? max_packet_size : todo;
	if (Packet_printf(&connp->w, "%c%hd%ld%ld", PKT_RELIABLE,
			  len, rel_off, main_loops) <= 0
	    || Sockbuf_write(&connp->w, read_buf, len) != len) {
	    error("Cannot write reliable data");
	    Destroy_connection(ind, "write error");
	    return -1;
	}
	if ((n = Sockbuf_flush(&connp->w)) < len) {
	    if (n == 0
		&& (errno == EWOULDBLOCK
		    || errno == EAGAIN)) {
		connp->acks = 0;
		break;
	    } else {
		error("Cannot flush reliable data (%d)", n);
		Destroy_connection(ind, "flush error");
		return -1;
	    }
	}
	todo -= len;
	rel_off += len;
	read_buf += len;
    }

    /*
     * Drop rest of outgoing data packet if something remains at all.
     */
    Sockbuf_clear(&connp->w);

    connp->last_send_loops = main_loops;

    if (max_todo - todo <= 0) {
	/*
	 * We have not transmitted anything at all.
	 */
	return 0;
    }

    /*
     * Retransmission timer with exponential backoff.
     */
    if (connp->rtt_retransmit > MAX_RETRANSMIT) {
	connp->rtt_retransmit = MAX_RETRANSMIT;
    }
    if (connp->retransmit_at_loop <= main_loops) {
	connp->retransmit_at_loop = main_loops + connp->rtt_retransmit;
	connp->rtt_retransmit <<= 1;
	connp->rtt_timeouts++;
    } else {
	connp->retransmit_at_loop = main_loops + connp->rtt_retransmit;
    }

    if (rel_off > connp->reliable_unsent) {
	connp->reliable_unsent = rel_off;
    }

    return (max_todo - todo);
}

static int Receive_ack(int ind)
{
    connection_t	*connp = &Conn[ind];
    int			n;
    unsigned char	ch;
    long		rel,
			rtt,	/* RoundTrip Time */
			diff,
			delta,
			rel_loops;

    if ((n = Packet_scanf(&connp->r, "%c%ld%ld",
			  &ch, &rel, &rel_loops)) <= 0) {
	errno = 0;
	error("Cannot read ack packet (%d)", n);
	Destroy_connection(ind, "read error");
	return -1;
    }
    if (ch != PKT_ACK) {
	errno = 0;
	error("Not an ack packet (%d)", ch);
	Destroy_connection(ind, "not ack");
	return -1;
    }
    rtt = main_loops - rel_loops;
    if (rtt > 0 && rtt <= MAX_RTT) {
	/*
	 * These roundtrip estimation calculations are derived from Comer's
	 * books "Internetworking with TCP/IP" parts I & II.
	 */
	if (connp->rtt_smoothed == 0) {
	    /*
	     * Initialize the rtt estimator by this first measurement.
	     * The estimator is scaled by 3 bits.
	     */
	    connp->rtt_smoothed = rtt << 3;
	}
	/*
	 * Scale the estimator back by 3 bits before calculating the error.
	 */
	delta = rtt - (connp->rtt_smoothed >> 3);
	/*
	 * Add one eigth of the error to the estimator.
	 */
	connp->rtt_smoothed += delta;
	/*
	 * Now we need the absolute value of the error.
	 */
	if (delta < 0) {
	    delta = -delta;
	}
	/*
	 * The rtt deviation is scaled by 2 bits.
	 * Now we add one fourth of the difference between the
	 * error and the previous deviation to the deviation.
	 */
	connp->rtt_dev += delta - (connp->rtt_dev >> 2);
	/*
	 * The calculation of the retransmission timeout is what this is
	 * all about.  We take the smoothed rtt plus twice the deviation
	 * as the next retransmission timeout to use.  Because of the
	 * scaling used we get the following statement:
	 */
	connp->rtt_retransmit = ((connp->rtt_smoothed >> 2)
	    + connp->rtt_dev) >> 1;
	/*
	 * Now keep it within reasonable bounds.
	 */
	if (connp->rtt_retransmit < MIN_RETRANSMIT) {
	    connp->rtt_retransmit = MIN_RETRANSMIT;
	}
    }
    diff = rel - connp->reliable_offset;
    if (diff > connp->c.len) {
	/* Impossible to ack data that has not been send */
	errno = 0;
	error("Bad ack (diff=%ld,cru=%ld,c=%ld,len=%d)",
	    diff, rel, connp->reliable_offset, connp->c.len);
	Destroy_connection(ind, "bad ack");
	return -1;
    }
    else if (diff <= 0) {
	/* Late or duplicate ack of old data.  Discard. */
	return 1;
    }
    Sockbuf_advance(&connp->c, (int) diff);
    connp->reliable_offset += diff;
    if ((n = ((diff + 512 - 1) / 512)) > connp->acks) {
	connp->acks = n;
    }
    else {
	connp->acks++;
    }
    if (connp->reliable_offset >= connp->reliable_unsent) {
	/*
	 * All reliable data has been sent and acked.
	 */
	connp->retransmit_at_loop = 0;
	if (connp->state == CONN_DRAIN) {
	    Conn_set_state(connp, connp->drain_state, connp->drain_state);
	}
    }
    if (connp->state == CONN_READY
	&& (connp->c.len <= 0
	    || (connp->c.buf[0] != PKT_REPLY
		&& connp->c.buf[0] != PKT_PLAY
		&& connp->c.buf[0] != PKT_SUCCESS
		&& connp->c.buf[0] != PKT_FAILURE))) {
	Conn_set_state(connp, connp->drain_state, connp->drain_state);
    }
    connp->rtt_timeouts = 0;

    return 1;
}

static int Receive_discard(int ind)
{
    connection_t	*connp = &Conn[ind];

    errno = 0;
    error("Discarding packet %d while in state %02x",
	  connp->r.ptr[0], connp->state);
    connp->r.ptr = connp->r.buf + connp->r.len;

    return 0;
}

static int Receive_undefined(int ind)
{
    connection_t	*connp = &Conn[ind];

    errno = 0;
    error("Unknown packet type (%d,%02x)", connp->r.ptr[0], connp->state);
    Destroy_connection(ind, "undefined packet");
    return -1;
}

static int Receive_ack_cannon(int ind)
{
    connection_t	*connp = &Conn[ind];
    long		loops_ack;
    unsigned char	ch;
    int			n;
    unsigned short	num;

    if ((n = Packet_scanf(&connp->r, "%c%ld%hu",
			  &ch, &loops_ack, &num)) <= 0) {
	if (n == -1) {
	    Destroy_connection(ind, "read error");
	}
	return n;
    }
    if (num >= World.NumCannons) {
	Destroy_connection(ind, "bad cannon ack");
	return -1;
    }
    if (loops_ack > World.cannon[num].last_change) {
	SET_BIT(World.cannon[num].conn_mask, 1 << ind);
    }
    return 1;
}

static int Receive_ack_fuel(int ind)
{
    connection_t	*connp = &Conn[ind];
    long		loops_ack;
    unsigned char	ch;
    int			n;
    unsigned short	num;

    if ((n = Packet_scanf(&connp->r, "%c%ld%hu",
			  &ch, &loops_ack, &num)) <= 0) {
	if (n == -1) {
	    Destroy_connection(ind, "read error");
	}
	return n;
    }
    if (num >= World.NumFuels) {
	Destroy_connection(ind, "bad fuel ack");
	return -1;
    }
    if (loops_ack > World.fuel[num].last_change) {
	SET_BIT(World.fuel[num].conn_mask, 1 << ind);
    }
    return 1;
}

static int Receive_ack_target(int ind)
{
    connection_t	*connp = &Conn[ind];
    long		loops_ack;
    unsigned char	ch;
    int			n;
    unsigned short	num;

    if ((n = Packet_scanf(&connp->r, "%c%ld%hu",
			  &ch, &loops_ack, &num)) <= 0) {
	if (n == -1) {
	    Destroy_connection(ind, "read error");
	}
	return n;
    }
    if (num >= World.NumTargets) {
	Destroy_connection(ind, "bad target ack");
	return -1;
    }
    /*
     * Because the "loops" value as received by the client as part
     * of a frame update is 1 higher than the actual change to the
     * target in collision.c a valid map object change
     * acknowledgement must be at least 1 higher.
     * That's why we should use the '>' symbol to compare
     * and not the '>=' symbol.
     * The same applies to cannon and fuelstation updates.
     * This fix was discovered for 3.2.7, previously some
     * destroyed targets could have been displayed with
     * a diagonal cross through them.
     */
    if (loops_ack > World.targets[num].last_change) {
	SET_BIT(World.targets[num].conn_mask, 1 << ind);
	CLR_BIT(World.targets[num].update_mask, 1 << ind);
    }
    return 1;
}

/*
 * If a message contains a colon then everything before that colon is
 * either a unique player name prefix, or a team number with players.
 * If the string does not match one team or one player the message is not sent.
 * If no colon, the message is general.
 */
static void Handle_talk(int ind, char *str)
{
    connection_t	*connp = &Conn[ind];
    player		*pl = Players[GetInd[connp->id]];
    int			i, sent, team;
	unsigned int	len;
    char		*cp,
			msg[MSG_LEN * 2];

    if ((cp = strchr (str, ':')) == NULL
	|| cp == str
	|| strchr("-_~)(/\\}{[]", cp[1])	/* smileys are smileys */
	) {
	sprintf(msg, "%s [%s]", str, pl->name);
	Set_message(msg);
	return;
    }
    *cp++ = '\0';
    len = strlen (str);
    sprintf(msg, "%s [%s]", cp, pl->name);

    if (strspn(str, "0123456789") == len) {		/* Team message */
	team = atoi (str);
	sprintf(msg + strlen(msg), ":[%d]", team);
	for (sent = i = 0; i < NumPlayers; i++) {
	    if (Players[i]->team != TEAM_NOT_SET && Players[i]->team == team) {
		sent++;
		Set_player_message (Players[i], msg);
	    }
	}
	if (sent) {
	    if (pl->team != team)
		Set_player_message (pl, msg);
	} else {
	    sprintf(msg, "Message not sent, nobody in team %d!",
		    team);
	    Set_player_message(pl, msg);
	}
    }
    else if (strcasecmp(str, "god") == 0) {
	/*
	 * Only log the message if logfile already exists,
	 * is writable and less than some KBs in size.
	 */
	char		*logfilename = Conf_logfile();
	const int	logfile_size_limit = 100*1024;
	FILE		*fp;
	struct stat	st;

	if (access(logfilename, 2) == 0 &&
	    stat(logfilename, &st) == 0 &&
	    (st.st_size < logfile_size_limit) &&
	    (fp = fopen(logfilename, "a")) != NULL)
	{
	    fprintf(fp,
		    "%s[%s]{%s@%s(%s)|%s}:\n"
		    "\t%s\n",
		    showtime(),
		    pl->name,
		    pl->realname, connp->host, connp->addr, connp->dpy,
		    cp);
	    fclose(fp);
	    sprintf(msg + strlen(msg), ":[%s]", "GOD");
	    Set_player_message(pl, msg);
	}
	else {
	    Set_player_message(pl, "Can't log to GOD.");
	}
    }
    else {						/* Player message */
	sent = -1;
	/* first look for an exact match on player nickname. */
	for (i = 0; i < NumPlayers; i++) {
	    if (strcasecmp(Players[i]->name, str) == 0) {
		sent = i;
		break;
	    }
	}
	if (sent == -1) {
	    /* now look for a partial match on both nick and realname. */
	    for (sent = -1, i = 0; i < NumPlayers; i++) {
		if (strncasecmp(Players[i]->name, str, len) == 0
		    || strncasecmp(Players[i]->realname, str, len) == 0)
		    sent = (sent == -1) ? i : -2;
	    }
	}
	switch (sent) {
	case -2:
	    sprintf(msg, "Message not sent, %s matches more than one player!",
		    str);
	    Set_player_message(pl, msg);
	    break;
	case -1:
	    sprintf(msg, "Message not sent, %s does not match any player!",
		    str);
	    Set_player_message(pl, msg);
	    break;
	default:
	    if (Players[sent] != pl) {
		sprintf(msg + strlen(msg), ":[%s]", Players[sent]->name);
		Set_player_message(Players[sent], msg);
		Set_player_message(pl, msg);
	    }
	    break;
	}
    }
}

static int Receive_talk(int ind)
{
    connection_t	*connp = &Conn[ind];
    unsigned char	ch;
    int			n;
    long		seq;
    char		str[MAX_CHARS];

    if ((n = Packet_scanf(&connp->r, "%c%ld%s", &ch, &seq, str)) <= 0) {
	if (n == -1) {
	    Destroy_connection(ind, "read error");
	}
	return n;
    }
    if (seq > connp->talk_sequence_num) {
	if ((n = Packet_printf(&connp->c, "%c%ld", PKT_TALK_ACK, seq)) <= 0) {
	    if (n == -1) {
		Destroy_connection(ind, "write error");
	    }
	    return n;
	}
	connp->talk_sequence_num = seq;
	if (*str == '/') {
	    Handle_player_command(Players[GetInd[connp->id]], str + 1);
	}
	else {
	    Handle_talk(ind, str);
	}
    }
    return 1;
}

static int Receive_display(int ind)
{
    connection_t	*connp = &Conn[ind];
    unsigned char	ch, debris_colors, spark_rand;
    short		width, height;
    int			n;

    if ((n = Packet_scanf(&connp->r, "%c%hd%hd%c%c", &ch, &width, &height,
			  &debris_colors, &spark_rand)) <= 0) {
	if (n == -1) {
	    Destroy_connection(ind, "read error");
	}
	return n;
    }
    LIMIT(width, MIN_VIEW_SIZE, MAX_VIEW_SIZE);
    LIMIT(height, MIN_VIEW_SIZE, MAX_VIEW_SIZE);
    connp->view_width = width;
    connp->view_height = height;
    connp->debris_colors = debris_colors;
    connp->spark_rand = spark_rand;
    return 1;
}

static int str2num (char **strp, int min, int max)
{
    char *str = *strp;
    int num = 0;

    while (isdigit(*str)) {
	num *= 10;
	num += *str++ - '0';
    }
    *strp = str;
    if (num < min || num > max)
	return min;
    return num;
}

static int Receive_modifier_bank(int ind)
{
    connection_t	*connp = &Conn[ind];
    player		*pl;
    unsigned char	bank;
    char		str[MAX_CHARS];
    unsigned char	ch;
    char		*cp;
    modifiers		mods;
    int			n;

    if ((n = Packet_scanf(&connp->r, "%c%c%s", &ch, &bank, str)) <= 0) {
	if (n == -1) {
	    Destroy_connection(ind, "read modbank");
	}
	return n;
    }
    pl = Players[GetInd[connp->id]];
    if (bank < NUM_MODBANKS) {
	CLEAR_MODS(mods);
	if (BIT(World.rules->mode, ALLOW_MODIFIERS)) {
	    for (cp = str; *cp; cp++) {
		switch (*cp) {
		case 'F': case 'f':
		    if (!BIT(World.rules->mode, ALLOW_NUKES))
			break;
		    if (*(cp+1) == 'N' || *(cp+1) == 'n')
			SET_BIT(mods.nuclear, FULLNUCLEAR);
		    break;
		case 'N': case 'n':
		    if (!BIT(World.rules->mode, ALLOW_NUKES))
			break;
		    SET_BIT(mods.nuclear, NUCLEAR);
		    break;
		case 'C': case 'c':
		    if (!BIT(World.rules->mode, ALLOW_CLUSTERS))
			break;
		    SET_BIT(mods.warhead, CLUSTER);
		    break;
		case 'I': case 'i':
		    SET_BIT(mods.warhead, IMPLOSION);
		    break;
		case 'V': case 'v':
		    cp++; mods.velocity = str2num (&cp, 0, MODS_VELOCITY_MAX);
		    cp--;
		    break;
		case 'X': case 'x':
		    cp++; mods.mini = str2num (&cp, 1, MODS_MINI_MAX+1) - 1;
		    cp--;
		    break;
		case 'Z': case 'z':
		    cp++; mods.spread = str2num (&cp, 0, MODS_SPREAD_MAX);
		    cp--;
		    break;
		case 'B': case 'b':
		    cp++; mods.power = str2num (&cp, 0, MODS_POWER_MAX);
		    cp--;
		    break;
		case 'L': case 'l':
		    cp++;
		    if (!BIT(World.rules->mode, ALLOW_LASER_MODIFIERS))
			break;
		    if (*cp == 'S' || *cp == 's')
			SET_BIT(mods.laser, STUN);
		    if (*cp == 'B' || *cp == 'b')
			SET_BIT(mods.laser, BLIND);
		    break;
		}
	    }
	}
	pl->modbank[bank] = mods;
    }
    return 1;
}

void Get_display_parameters(int ind, int *width, int *height,
			    int *debris_colors, int *spark_rand)
{
    connection_t	*connp = &Conn[ind];

    *width = connp->view_width;
    *height = connp->view_height;
    *debris_colors = connp->debris_colors;
    *spark_rand = connp->spark_rand;
}

int Get_player_id(int ind)
{
    connection_t	*connp = &Conn[ind];

    return connp->id;
}

int Get_conn_version(int ind)
{
    connection_t	*connp = &Conn[ind];

    return connp->version;
}

static int Receive_shape(int ind)
{
    connection_t	*connp = &Conn[ind];
    int			n;
    char		ch;
    char		str[2*MSG_LEN];

    if ((n = Packet_scanf(&connp->r, "%c%S", &ch, str)) <= 0) {
	if (n == -1) {
	    Destroy_connection(ind, "read shape");
	}
	return n;
    }
    if (connp->version > 0x3200) {
	if ((n = Packet_scanf(&connp->r, "%S", &str[strlen(str)])) <= 0) {
	    if (n == -1) {
		Destroy_connection(ind, "read shape ext");
	    }
	    return n;
	}
    }
    if (connp->state == CONN_LOGIN && connp->ship == NULL) {
	connp->ship = Parse_shape_str(str);
    }
    return 1;
}

static int Receive_motd(int ind)
{
    connection_t	*connp = &Conn[ind];
    unsigned char	ch;
    long		offset;
    int			n;
    long		bytes;

    if ((n = Packet_scanf(&connp->r,
			  "%c%ld%ld",
			  &ch, &offset, &bytes)) <= 0) {
	if (n == -1) {
	    Destroy_connection(ind, "read error");
	}
	return n;
    }
    connp->motd_offset = offset;
    connp->motd_stop = offset + bytes;

    return 1;
}

/*
 * Return part of the MOTD into buf starting from offset
 * and continueing at most for maxlen bytes.
 * Return the total MOTD size in size_ptr.
 * The return value is the actual amount of MOTD bytes copied
 * or -1 on error.  A value of 0 means EndOfMOTD.
 *
 * The MOTD is completely read into a dynamic buffer.
 * If this MOTD buffer hasn't been accessed for a while
 * then on the next access the MOTD file is checked for changes.
 */
#ifdef _WINDOWS
#define	close(__a)	_close(__a)
#endif
int Get_motd(char *buf, int offset, int maxlen, int *size_ptr)
{
    static int		motd_size;
    static char		*motd_buf;
    static long		motd_loops;
    static time_t	motd_mtime;

    if (size_ptr) {
	*size_ptr = 0;
    }
    if (offset < 0 || maxlen < 0) {
	return -1;
    }

    if (!motd_loops
	|| (motd_loops + MAX_MOTD_LOOPS < main_loops
	    && offset == 0)) {

	int			fd, size;
	struct stat		st;

	motd_loops = main_loops;

	if ((fd = open(motdFileName, O_RDONLY)) == -1) {
	    motd_size = 0;
	    return -1;
	}
	if (fstat(fd, &st) == -1 || st.st_size == 0) {
	    motd_size = 0;
	    close(fd);
	    return -1;
	}
	size = st.st_size;
	if (size > MAX_MOTD_SIZE) {
	    size = MAX_MOTD_SIZE;
	}
	if (size != motd_size) {
	    motd_mtime = 0;
	    motd_size = size;
	    if (motd_size == 0) {
		close(fd);
		return 0;
	    }
	    if (motd_buf) {
		free(motd_buf);
	    }
	    if ((motd_buf = (char *) malloc(size)) == NULL) {
		close(fd);
		return -1;
	    }
	}
	if (motd_mtime != st.st_mtime) {
	    motd_mtime = st.st_mtime;
	    if ((size = read(fd, motd_buf, motd_size)) <= 0) {
		free(motd_buf);
		motd_buf = 0;
		close(fd);
		motd_size = 0;
		return -1;
	    }
	    motd_size = size;
	}
	close(fd);
    }

    motd_loops = main_loops;

    if (size_ptr) {
	*size_ptr = motd_size;
    }
    if (offset + maxlen > motd_size) {
	maxlen = motd_size - offset;
    }
    if (maxlen <= 0) {
	return 0;
    }
    memcpy(buf, motd_buf + offset, maxlen);
    return maxlen;
}

/*
 * Send the server MOTD to the client.
 * The last time we send a motd packet it should
 * have datalength zero to mean EOMOTD.
 */
static int Send_motd(int ind)
{
    connection_t	*connp = &Conn[ind];
    int			len;
    int			off = connp->motd_offset,
			size = 0;
    char		buf[MAX_MOTD_CHUNK];

    len = MIN(MAX_MOTD_CHUNK, MAX_RELIABLE_DATA_PACKET_SIZE - connp->c.len - 10);
    if (len >= 10) {
	len = Get_motd(buf, off, len, &size);
	if (len <= 0) {
	    len = 0;
	    connp->motd_offset = -1;
	}
	if (Packet_printf(&connp->c,
			  "%c%ld%hd%ld",
			  PKT_MOTD, off, len, size) <= 0) {
	    Destroy_connection(ind, "motd header");
	    return -1;
	}
	if (len > 0) {
	    connp->motd_offset += len;
	    if (Sockbuf_write(&connp->c, buf, len) != len) {
		Destroy_connection(ind, "motd data");
		return -1;
	    }
	}
    }

    /* Return ok */
    return 1;
}

static int Receive_pointer_move(int ind)
{
    connection_t	*connp = &Conn[ind];
    player		*pl;
    unsigned char	ch;
    short		movement;
    int			n;
    DFLOAT		turnspeed, turndir;

    if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &movement)) <= 0) {
	if (n == -1) {
	    Destroy_connection(ind, "read error");
	}
	return n;
    }
    pl = Players[GetInd[connp->id]];
    if (BIT(pl->status, HOVERPAUSE))
	return 1;

    if (BIT(pl->used, HAS_AUTOPILOT))
	Autopilot(GetInd[connp->id], 0);
    turnspeed = movement * pl->turnspeed / MAX_PLAYER_TURNSPEED;
    if (turnspeed < 0) {
	turndir = -1.0;
	turnspeed = -turnspeed;
    }
    else {
	turndir = 1.0;
    }
    if (pl->turnresistance)
	LIMIT(turnspeed, MIN_PLAYER_TURNSPEED, MAX_PLAYER_TURNSPEED);
      /* Minimum amount of turning if you want to turn at all?
	And the only effect of that maximum is making
        finding the correct settings harder for new mouse players,
        because the limit is checked BEFORE multiplying by turnres!
        Kept here to avoid changing the feeling for old players who
        are already used to this odd behavior. New players should set
        turnresistance to 0.
      */
    else
	LIMIT(turnspeed, 0, 5*RES);

    pl->turnvel -= turndir * turnspeed;

    return 1;
}

static int Receive_fps_request(int ind)
{
    connection_t	*connp = &Conn[ind];
    player		*pl;
    int			n;
    unsigned char	ch;
    unsigned char	fps;

    if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &fps)) <= 0) {
	if (n == -1) {
	    Destroy_connection(ind, "read error");
	}
	return n;
    }
    if (connp->id != NO_ID) {
	pl = Players[GetInd[connp->id]];
	pl->player_fps = fps;
	if (fps > FPS) pl->player_fps = FPS;
	if (fps < (FPS / 2)) pl->player_fps = (FPS+1) / 2;
	if (fps == 0) pl->player_fps = FPS;
	if ((fps == 20) && ignore20MaxFPS) pl->player_fps = FPS;
	n = FPS - pl->player_fps;
	if (n <= 0) {
	    pl->player_count = 0;
	} else {
	    pl->player_count = FPS / n;
	}
    }

    return 1;
}

static int Receive_audio_request(int ind)
{
    connection_t	*connp = &Conn[ind];
    player		*pl;
    int			n;
    unsigned char	ch;
    unsigned char	onoff;

    if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &onoff)) <= 0) {
	if (n == -1) {
	    Destroy_connection(ind, "read error");
	}
	return n;
    }
    if (connp->id != NO_ID) {
	pl = Players[GetInd[connp->id]];
	sound_player_onoff(pl, onoff);
    }

    return 1;
}


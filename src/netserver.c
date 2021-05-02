/* $Id: netserver.c,v 3.22 1993/08/02 12:55:14 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
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
 * has some time to do its final initialisations, like mapping
 * its user interface windows and so on.
 * When the client is ready to accept frame updates and process
 * keyboard events then it sends the start-play packet.
 * This play packet advances the connection state into the
 * actively-playing state.  A player structure is allocated and
 * initialised and the other human players are told about this new player.
 * The newly started client is told about the already playing players and
 * play has begun.
 * Apart from these four states there are also two intermediate states.
 * These intermediate states are entered when the previous state
 * has filled the reliable data buffer and the client hasn't
 * acknowledged all the data yet that's in this reliable data buffer.
 * They are so called output drain states.  Not doing anything else
 * then waiting until the buffer is empty.
 * The difference between these two intermediate states is tricky.
 * The second intermediate state is entered after the
 * ready-but-not-playing-yet state and before the actively-playing state.
 * The difference being that in this second intermediate state the client
 * is already considered an active player by the rest of the server 
 * but shouldn't get frame updates yet until it has acknowledged its last
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
 * a new reliable data packet and it hasn't had this data before and
 * there is also no data packet missing inbetween, then it advances
 * its byte position and acknowledges this new position to the server.
 * Otherwise it discards the packet and sends its old byte position
 * to the server meaning that it detected a packet loss.
 * The server maintains an acknowledgement timeout timer for each
 * connection so that it can retransmit a reliable data packet
 * if the acknowledgement timer expires.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#if defined(__hpux)
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "global.h"
#include "version.h"
#include "map.h"
#include "pack.h"
#include "error.h"
#include "bit.h"
#include "socklib.h"
#include "net.h"
#define NETSERVER_C
#include "netserver.h"
#include "packet.h"
#include "setup.h"
#undef NETSERVER_C
#include "robot.h"
#include "saudio.h"


#define MAX_SELECT_FD	(sizeof(int) * 8 - 1)

static connection_t	*Conn = NULL;
static int		max_connections = 0;
static setup_t		*Setup = NULL;
static int		(*playing_receive[256])(int ind),
			(*login_receive[256])(int ind),
			(*drain_receive[256])(int ind);
int			compress_maps = 1;
int			login_in_progress;


/*
 * Compress the map data using a simple Run Length Encoding algorithm.
 * If there's more than one consecutive byte with the same type
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
 * Initialise the structure that gives the client information
 * about our setup.  Like the map and playing rules.
 * We only setup this structure once to save time when new
 * players log in during play.
 */
static int Init_setup(void)
{
    int			i, x, y, team, type, size;
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
	    case SPACE:		*mapptr = SETUP_SPACE; break;
	    case FILLED_NO_DRAW:
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
	    case WORMHOLE:
		for (i = 0; i < World.NumWormholes; i++) {
		    if (x != World.wormHoles[i].pos.x
			|| y != World.wormHoles[i].pos.y) {
			continue;
		    }
		    switch (World.wormHoles[i].type) {
		    case WORM_NORMAL: *mapptr = SETUP_WORM_NORMAL; break;
		    case WORM_IN:     *mapptr = SETUP_WORM_NORMAL; break;
		    case WORM_OUT:    *mapptr = SETUP_SPACE; break;
		    default:
			error("Wrong type of wormhole (%d) at (%d,%d).",
			    World.wormHoles[i].type, x, y);
			free(mapdata);
			return -1;
		    }
		    break;
		}
		if (i >= World.NumWormholes) {
		    error("No wormhole for position (%d,%d).", x, y);
		    free(mapdata);
		    return -1;
		}
		break;
	    case TREASURE:
		for (i = 0; i < World.NumTreasures; i++) {
		    if (x != World.treasures[i].pos.x
			|| y != World.treasures[i].pos.y) {
			continue;
		    }
		    team = World.treasures[i].team;
		    *mapptr = SETUP_TREASURE + team;
		    break;
		}
		if (i >= World.NumTreasures) {
		    error("No treasure for position (%d,%d).", x, y);
		    free(mapdata);
		    return -1;
		}
		break;

	    case TARGET:
		for (i = 0; i < World.NumTargets; i++) {
		    if (x != World.targets[i].pos.x
			|| y != World.targets[i].pos.y) {
			continue;
		    }
		    team = World.targets[i].team;
		    *mapptr = SETUP_TARGET + team;
		    break;
		}
		if (i >= World.NumTargets) {
		    error("No target for position (%d,%d).", x, y);
		    free(mapdata);
		    return -1;
		}
		break;

	    case BASE:
		for (i = 0; i < World.NumBases; i++) {
		    if (x != World.base[i].pos.x
			|| y != World.base[i].pos.y) {
			continue;
		    }
		    if (World.base[i].team == TEAM_NOT_SET) {
			team = 0;
		    } else {
			team = World.base[i].team;
		    }
		    switch (World.base[i].dir) {
		    case DIR_UP:    *mapptr = SETUP_BASE_UP + team; break;
		    case DIR_RIGHT: *mapptr = SETUP_BASE_RIGHT + team; break;
		    case DIR_DOWN:  *mapptr = SETUP_BASE_DOWN + team; break;
		    case DIR_LEFT:  *mapptr = SETUP_BASE_LEFT + team; break;
		    default:
			error("Wrong direction of base (%d) at (%d,%d).",
			      World.base[i].dir, x, y);
			free(mapdata);
			return -1;
		    }
		    break;
		}
		if (i >= World.NumBases) {
		    error("No treasure for position (%d,%d).", x, y);
		    free(mapdata);
		    return -1;
		}
		break;
	    case CANNON:
		for (i = 0; i < World.NumCannons; i++) {
		    if (x != World.cannon[i].pos.x
			|| y != World.cannon[i].pos.y) {
			continue;
		    }
		    switch (World.cannon[i].dir) {
		    case DIR_UP:	*mapptr = SETUP_CANNON_UP; break;
		    case DIR_RIGHT:	*mapptr = SETUP_CANNON_RIGHT; break;
		    case DIR_DOWN:	*mapptr = SETUP_CANNON_DOWN; break;
		    case DIR_LEFT:	*mapptr = SETUP_CANNON_LEFT; break;
		    default:
			error("Wrong direction of cannon (%d) at (%d,%d).",
			    World.cannon[i].dir, x, y);
			free(mapdata);
			return -1;
		    }
		    break;
		}
		if (i >= World.NumCannons) {
		    error("No cannon for position (%d,%d).", x, y);
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
		    error("No check point for position (%d,%d).", x, y);
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
	if ((mapdata = realloc(mapdata, size)) == NULL) {
	    error("Can't reallocate mapdata");
	    return -1;
	}
    }

#ifndef SILENT
    if (type != SETUP_MAP_UNCOMPRESSED) {
	printf("Map compression ratio is %-4.2f%%\n",
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
    strncpy(Setup->name, World.name, sizeof(Setup->name) - 1);
    Setup->name[sizeof(Setup->name) - 1] = '\0';
    strncpy(Setup->author, World.author, sizeof(Setup->author) - 1);
    Setup->author[sizeof(Setup->author) - 1] = '\0';

    return 0;
}

/*
 * Initialise the function dispatch tables for the various client
 * connection states.  Some states use the same table.
 */
void Init_receive(void)
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

    login_receive[PKT_PLAY]			= Receive_play;
    login_receive[PKT_QUIT]			= Receive_quit;
    login_receive[PKT_SEND_BUFSIZE]		= Receive_send_bufsize;
    login_receive[PKT_ACK]			= Receive_ack;
    login_receive[PKT_VERIFY]			= Receive_discard;
    login_receive[PKT_POWER]			= Receive_power;
    login_receive[PKT_POWER_S]			= Receive_power;
    login_receive[PKT_TURNSPEED]		= Receive_power;
    login_receive[PKT_TURNSPEED_S]		= Receive_power;
    login_receive[PKT_TURNRESISTANCE]		= Receive_power;
    login_receive[PKT_TURNRESISTANCE_S]		= Receive_power;

    playing_receive[PKT_ACK]			= Receive_ack;
    playing_receive[PKT_VERIFY]			= Receive_discard;
    playing_receive[PKT_PLAY]			= Receive_play;
    playing_receive[PKT_QUIT]			= Receive_quit;
    playing_receive[PKT_KEYBOARD]		= Receive_keyboard;
    playing_receive[PKT_SEND_BUFSIZE]		= Receive_send_bufsize;
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
}

/*
 * Initialise the connection structures.
 */
int Setup_net_server(int maxconn)
{
    size_t	size;

    signal(SIGPIPE, SIG_IGN);

    Init_receive();

    if (Init_setup() == -1) {
	return -1;
    }
    /*
     * The number of connections is limited by the number of bases
     * and the max number of possible file descriptors to use in
     * the select(2) call minus those for stdin, stdout, stderr
     * and the contact socket.
     */
    max_connections = MIN(MAX_SELECT_FD - 4, maxconn);
    size = max_connections * sizeof(*Conn);
    if ((Conn = (connection_t *) malloc(size)) == NULL) {
	error("Can't allocate memory for connections");
	return -1;
    }
    memset(Conn, 0, size);

    return 0;
}

/*
 * Cleanup a connection.  The client may not know yet that
 * it's thrown out of the game so we send it a quit packet.
 * We send it twice because of UDP it could get lost.
 * Because there may be many reasons why a connection may
 * need to be destroyed we print the source file and line
 * number in order to be able to analyse any problems easily.
 */
void Destroy_connection(int ind, char *file, int line)
{
    connection_t	*connp = &Conn[ind];
    int			id,
			sock;
    char		ch;

    if (connp->state == CONN_FREE) {
	errno = 0;
	error("Can't destroy empty connection");
	return;
    }
    sock = connp->w.sock;
    ch = PKT_QUIT;
    write(sock, &ch, 1);
#ifndef SILENT
    printf("Destroying connection (%s,%d) for %s=%s@%s|%s\n",
	    file, line, connp->nick, connp->real, connp->host, connp->dpy);
#endif
    connp->state = CONN_FREE;
    if (connp->id != -1) {
	id = connp->id;
	connp->id = -1;
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
    Sockbuf_cleanup(&connp->w);
    Sockbuf_cleanup(&connp->r);
    Sockbuf_cleanup(&connp->c);
    memset(connp, 0, sizeof(*connp));
    ch = PKT_QUIT;
    write(sock, &ch, 1);
    close(sock);
}

/*
 * A client has requested a playing connection with this server.
 * See if we have room for one more player and if his name isn't
 * already in use by some other player.  Because the confirmation
 * may get lost we are willing to send it another time if the
 * client connection is still in the CONN_LISTENING state.
 */
int Setup_connection(char *real, char *nick, char *dpy,
		     int team, char *host, unsigned version)
{
    int			i,
			free_conn_index = max_connections,
			my_port,
			sock,
			retries;
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
		&& team == connp->team
		&& strcmp(real, connp->real) == 0
		&& strcmp(dpy, connp->dpy) == 0
		&& version == connp->version) {
		/*
		 * May happen for multi-homed hosts
		 * and if previous packet got lost.
		 */
		login_in_progress = 1; 
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
	printf("Full house for %s(%s)@%s(%s)\n", real, nick, host, dpy);
#endif
	return -1;
    }
    connp = &Conn[free_conn_index];

    if ((sock = CreateDgramSocket(0)) == -1) {
	error("Can't create datagram socket (%d)", sl_errno);
	return -1;
    }
    if (sock >= MAX_SELECT_FD) {
	/* Not handled with our current oldfashioned use of select(2).
	 * Tell the client that we're old and lazy. */
	errno = 0;
	error("Socket filedescriptor too big");
	close(sock);
	return -1;
    }
    if ((my_port = GetPortNum(sock)) == 0) {
	error("Can't get port from socket");
	close(sock);
	return -1;
    }
    if (SetSocketNonBlocking(sock, 1) == -1) {
	error("Can't make socket non-blocking");
	close(sock);
	return -1;
    }
    if (SetSocketSendBufferSize(sock, SERVER_SEND_SIZE + 16) == -1) {
	error("Can't set send buffer size to %d", SERVER_SEND_SIZE + 16);
    }
    if (SetSocketReceiveBufferSize(sock, SERVER_RECV_SIZE + 16) == -1) {
	error("Can't set receive buffer size to %d", SERVER_RECV_SIZE + 16);
    }

    Sockbuf_init(&connp->w, sock, SERVER_SEND_SIZE,
		 SOCKBUF_WRITE | SOCKBUF_DGRAM);

    Sockbuf_init(&connp->r, sock, SERVER_RECV_SIZE,
		 SOCKBUF_READ | SOCKBUF_DGRAM);

    Sockbuf_init(&connp->c, -1, MAX_SOCKBUF_SIZE,
		 SOCKBUF_WRITE | SOCKBUF_READ | SOCKBUF_LOCK);

    connp->my_port = my_port;
    connp->real = strdup(real);
    connp->nick = strdup(nick);
    connp->dpy = strdup(dpy);
    connp->team = team;
    connp->version = version;
    connp->state = CONN_LISTENING;
    connp->start = loops;
    connp->magic = rand() + my_port + sock + team + loops;
    connp->id = -1;
    connp->last_key_change = 0;
    connp->reliable_offset = 0;
    connp->reliable_unsend = 0;
    connp->last_send_loops = 0;
    connp->retransmit_at_loop = 0;
    connp->rtt_retransmit = DEFAULT_RETRANSMIT;
    connp->rtt_smoothed = 0;
    connp->rtt_dev = 0;
    connp->rtt_timeouts = 0;
    connp->acks = 0;
    connp->setup = 0;
    strncpy(connp->host, host, sizeof(connp->host) - 1);
    if (connp->w.buf == NULL
	|| connp->r.buf == NULL
	|| connp->c.buf == NULL
	|| connp->real == NULL
	|| connp->nick == NULL
	|| connp->dpy == NULL) {
	error("Not enough memory for connection");
	Destroy_connection(i, __FILE__, __LINE__);
	return -1;
    }

    login_in_progress = 1;

    return my_port;
}

/*
 * Handle a connection that's in the listening state.
 */
static int Handle_listening(int ind)
{
    connection_t	*connp = &Conn[ind];
    unsigned char	type;
    int			n;
    char		nick[MAX_NAME_LEN],
			real[MAX_NAME_LEN],
			dpy[MAX_DISP_LEN];

    if (connp->state != CONN_LISTENING) {
	errno = 0;
	error("Connection not in listening state");
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }
    Sockbuf_clear(&connp->r);
    errno = 0;
    n = DgramReceiveAny(connp->r.sock, connp->r.buf, connp->r.size);
    if (n <= 0) {
	if (errno == EWOULDBLOCK
	    || errno == EAGAIN) {
	    n = 0;
	}
	else if (n != 0) {
	    error("Can't read first client packet");
	    Destroy_connection(ind, __FILE__, __LINE__);
	}
	return n;
    }
    connp->r.len = n;
    connp->his_port = DgramLastport();
    strncpy(connp->host, DgramLastaddr(), sizeof(connp->host) - 1);
    if (DgramConnect(connp->w.sock, connp->host, connp->his_port) == -1) {
	error("Can't connect datagram socket (%s,%d)",
	    connp->host, connp->his_port);
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }
#ifndef SILENT
    printf("Connected to %s/%d\n", connp->host, connp->his_port);
#endif
    if (connp->r.ptr[0] != PKT_VERIFY) {
	errno = 0;
	error("First datagram not connecting (%d)", connp->r.ptr[0]);
	Send_reply(ind, PKT_VERIFY, PKT_FAILURE);
	Send_reliable(ind);
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }
    if ((n = Packet_scanf(&connp->r, "%c%s%s%s",
			  &type, real, nick, dpy)) <= 0) {
	error("Can't read complete connecting stuff");
	Send_reply(ind, PKT_VERIFY, PKT_FAILURE);
	Send_reliable(ind);
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }
    if (strcmp(real, connp->real) != 0
	|| strcmp(nick, connp->nick) != 0) {
	errno = 0;
	error("Client verified incorrectly (%s,%s)(%s,%s)",
	    real, nick, connp->real, connp->nick);
	Send_reply(ind, PKT_VERIFY, PKT_FAILURE);
	Send_reliable(ind);
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }
    n = 0;
    Sockbuf_clear(&connp->w);
    if (Send_reply(ind, PKT_VERIFY, PKT_SUCCESS) == -1
    	|| Packet_printf(&connp->c, "%c%u", PKT_MAGIC, connp->magic) <= 0
	|| (n = Send_reliable(ind)) <= 0) {
	error("Can't confirm connection (%d)", n);
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }
    connp->state = CONN_DRAIN;
    connp->drain_state = CONN_SETUP;
    connp->start = loops;

    return 0;	/* success! */
}

/*
 * Handle a connection that's in the transmit-server-configuration-data state.
 */
static int Handle_setup(int ind)
{
    connection_t	*connp = &Conn[ind];
    char		*buf;
    int			n,
			len;

    if (connp->state != CONN_SETUP) {
	errno = 0;
	error("Connection not in setup state");
	Destroy_connection(ind, __FILE__, __LINE__);
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
	    errno = 0;
	    error("Can't write setup info into reliable data buffer");
	    Destroy_connection(ind, __FILE__, __LINE__);
	    return -1;
	}
	connp->setup = (char *) &Setup->map_data[0] - (char *) Setup;
    }
    if (connp->setup < Setup->setup_size) {
	while (connp->c.len > 0) {
	    if ((n = Handle_input(ind)) == -1) {
		return -1;
	    }
	    if (n == 0) {
		break;
	    }
	}
	len = connp->c.size - connp->c.len;
	if (len <= 0) {
	    /* Wait for ack */
	    return 0;
	}
	if (len > Setup->setup_size - connp->setup) {
	    len = Setup->setup_size - connp->setup;
	}
	buf = (char *) Setup;
	if (Sockbuf_write(&connp->c, &buf[connp->setup], len) != len) {
	    error("Can't sockbuf write setup data");
	    Destroy_connection(ind, __FILE__, __LINE__);
	    return -1;
	}
	connp->setup += len;
	if (len >= 512) {
	    connp->start += (len * FPS) / (8 * 512) + 1;
	}
	if (Send_reliable(ind) == -1) {
	    error("Can't send setup (len=%d,state=%02x)", len, connp->state);
	    return -1;
	}
    }
    if (connp->setup >= Setup->setup_size) {
	connp->state = CONN_DRAIN;
	connp->drain_state = CONN_LOGIN;
	connp->start = loops;
    }

    return 0;
}

/*
 * A client has requested to start active play.
 * See if we can allocate a player structure for it
 * and if this succeeds update the player information
 * to all connected players.
 */
static int Handle_login(int ind)
{
    connection_t	*connp = &Conn[ind];
    player		*pl;
    int			i,
			n,
			conn_bit;
    char		msg[MSG_LEN];

    if (NumPlayers >= World.NumBases) {
	errno = 0;
	error("Not enough bases for players");
	return -1;
    }
    if (Id >= MAX_ID) {
	errno = 0;
	error("Id too big (%d)", Id);
	return -1;
    }
    if (BIT(World.rules->mode, TEAM_PLAY)) {
	if (connp->team == TEAM_NOT_SET) {
	    connp->team = 0;
	}
	if (connp->team < 0 || connp->team >= MAX_TEAMS) {
	    errno = 0;
	    error("Invalid team %d", connp->team);
	    return -1;
	}
	if (World.teams[connp->team].NumMembers
	    >= World.teams[connp->team].NumBases) {
	    errno = 0;
	    error("Not enough bases for team %d", connp->team);
	    return -1;
	}
    } else {
	connp->team = TEAM_NOT_SET;
    }
    for (i = 0; i < NumPlayers; i++) {
	if (strcasecmp(Players[i]->name, connp->nick) == 0) {
	    errno = 0;
	    error("Name already in use %s", connp->nick);
	    return -1;
	}
    }
    Init_player(NumPlayers);
    pl = Players[NumPlayers];
    strcpy(pl->name, connp->nick);
    strcpy(pl->realname, connp->real);
    strcpy(pl->dispname, connp->host);
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
    connp->id = Id++;
    pl->conn = ind;
    memset(pl->last_keyv, 0, sizeof(pl->last_keyv));
    memset(pl->prev_keyv, 0, sizeof(pl->prev_keyv));
    pl->key_changed = 0;

    connp->state = CONN_READY;
    connp->drain_state = CONN_PLAYING;
    connp->start = loops;

    if (Send_reply(ind, PKT_PLAY, PKT_SUCCESS) <= 0) {
	error("Can't send play reply");
	return -1;
    }

#ifndef	SILENT
    printf("%s (%d, %s@%s|%s) starts at startpos %d.\n",
	   pl->name, NumPlayers, pl->realname,
	   connp->host, connp->dpy, pl->home_base);
#endif

    /*
     * Tell him about himself first.
     */
    Send_player(pl->conn, pl->id, pl->team, pl->mychar, pl->name);
    Send_score(pl->conn, pl->id, pl->score, pl->life);
    Send_base(pl->conn, pl->id, pl->home_base);
    /*
     * And tell him about all the others.
     */
    for (i = 0; i < NumPlayers - 1; i++) {
	Send_player(pl->conn, Players[i]->id,
		    Players[i]->team, Players[i]->mychar, Players[i]->name);
	Send_score(pl->conn, Players[i]->id,
		   Players[i]->score, Players[i]->life);
	Send_base(pl->conn, Players[i]->id, Players[i]->home_base);
    }
    /*
     * And tell all the others about him.
     */
    for (i = 0; i < NumPlayers - 1; i++) {
	if (Players[i]->conn != NOT_CONNECTED) {
	    Send_player(Players[i]->conn, pl->id,
			pl->team, pl->mychar, pl->name);
	    Send_score(Players[i]->conn, pl->id, pl->score, pl->life);
	    Send_base(Players[i]->conn, pl->id, pl->home_base);
	}
	/*
	 * And tell him about the relationships others have with eachother.
	 */
	else if (Players[i]->robot_mode != RM_NOT_ROBOT
	    && Players[i]->robot_lock == LOCK_PLAYER) {
	    Send_war(pl->conn, Players[i]->id, Players[i]->robot_lock_id);
	}
    }
 
    if (NumPlayers == 1) {
	sprintf(msg, "Welcome to \"%s\", made by %s.",
		World.name, World.author);
    } else {
	sprintf(msg, "%s (%s) has entered \"%s\", made by %s.",
		pl->name, pl->realname, World.name, World.author);
    }
    Set_message(msg);

#ifdef SOUND
    if (sound_player_init(pl) < 0) {
	error("Silent mode\n");
    }

    sound_play_all(START_SOUND);
#endif

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
	} else {
	    CLR_BIT(World.targets[i].conn_mask, conn_bit);
	}
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
static int Handle_input(int ind)
{
    connection_t	*connp = &Conn[ind];
    int			(**receive_tbl)(int),
			type,
			login = 0,
			result;

    if (connp->state == CONN_PLAYING
	|| connp->state == CONN_READY) {
	receive_tbl = &playing_receive[0];
    }
    else if (connp->state == CONN_LOGIN) {
	receive_tbl = &login_receive[0];
    }
    else if (connp->state == CONN_DRAIN
	|| connp->state == CONN_SETUP) {
	receive_tbl = &drain_receive[0];
    } else {
	errno = 0;
	error("Connection not ready for input (%02x)", connp->state);
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }
    Sockbuf_clear(&connp->r);
    if (Sockbuf_read(&connp->r) == -1) {
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }
    if (connp->r.len <= 0) {
	return 0;
    }
    while (connp->r.ptr < connp->r.buf + connp->r.len) {
	type = (connp->r.ptr[0] & 0xFF);
	result = (*receive_tbl[type])(ind);
	if (result == -1) {
	    return -1;
	}
	if (result == 0) {
	    /* Incomplete client packet */
	    /* Drop rest of packet */
	    Sockbuf_clear(&connp->r);
	    break;
	}
	if (result == 2) {
	    login = 1;
	}
    }
    return 1 + login;
}

int Check_for_connection_bugs(void)
{
#if 0
    int			i;
    connection_t	*connp;
    player		*pl;

    /*
     * Check for some nasty bugs that occur only infrequently.
     */
    for (i = 0; i < max_connections; i++) {
	connp = &Conn[i];
	if (connp->state == CONN_FREE) {
	    continue;
	}
	if (connp->r.size != SERVER_RECV_SIZE
	    || connp->w.size != SERVER_SEND_SIZE
	    || connp->c.size != MAX_SOCKBUF_SIZE) {
	    errno = 0;
	    error("Bad socket buffer size (%d,%d,%d)",
		connp->r.size, connp->w.size, connp->c.size);
	    Destroy_connection(i, __FILE__, __LINE__);
	    continue;
	}
	if (connp->state == CONN_PLAYING) {
	    pl = Players[GetInd[connp->id]];
	    if (pl->conn != i) {
		if (pl->conn == NOT_CONNECTED) {
		    errno = 0;
		    error("Player not connected");
		    Destroy_connection(i, __FILE__, __LINE__);
		    continue;
		} else {
		    errno = 0;
		    error("Player badly connected (%d,%d)",
			connp->id, pl->conn);
		    connp->id = -1;
		    Destroy_connection(i, __FILE__, __LINE__);
		    continue;
		}
	    }
	}
    }
    for (i = NumPlayers - 1; i >= 0; i--) {
	pl = Players[i];
	if (pl->conn == NOT_CONNECTED) {
	    continue;
	}
	if (pl->conn < 0 || pl->conn >= max_connections) {
	    errno = 0;
	    error("Bad player connection index (%d,%d)", i, pl->conn);
	    pl->conn = NOT_CONNECTED;
	    continue;
	}
	connp = &Conn[pl->conn];
	if (connp->state != CONN_PLAYING) {
	    errno = 0;
	    error("Bad player connection state (%d,%d)",
		i, connp->state);
	    Destroy_connection(pl->conn, __FILE__, __LINE__);
	    continue;
	}
	if (connp->id != pl->id) {
	    errno = 0;
	    error("Bad player id (%d,%d,%d)", i, pl->id, connp->id);
	    pl->conn = NOT_CONNECTED;
	    Destroy_connection(pl->conn, __FILE__, __LINE__);
	    continue;
	}
    }
#endif
    return 0;
}

/*
 * Test if there's input available from clients who are
 * actively playing.  It could be that there is more
 * than one packet from the same client, but we don't
 * allow this input processing to go on too long, so
 * clients need to restrict the number of packets they transmit.
 * We have to get back calculating another frame real soon.
 */
int Check_client_input(void)
{
    int			i,
			n,
			max,
			sock,
			fdmask,
    			count,
			save_mask,
			num_input = 0,
			input_fd[MAX_SELECT_FD + 1],
			input_ind[MAX_SELECT_FD + 1];
    connection_t	*connp;
    struct timeval	tv;
    char		msg[MSG_LEN];

    max = -1;
    fdmask = 0;
    for (i = 0; i < max_connections; i++) {
	connp = &Conn[i];
	if (connp->state != CONN_PLAYING) {
	    continue;
	}
	if (loops - connp->start > CONNECTION_TIMEOUT) {
	    /*
	     * Timeout this fellow if we haven't heard a single thing
	     * from him for a long time.
	     */
	    sprintf(msg, "%s mysteriously disappeared!?", connp->nick);
	    Set_message(msg);
	    Destroy_connection(i, __FILE__, __LINE__);
	    continue;
	}
	sock = connp->r.sock;
	SET_BIT(fdmask, 1 << sock);
	if (sock > max) {
	    max = sock;
	}
	input_fd[num_input] = sock;
	input_ind[num_input] = i;
	num_input++;
    }
    if (num_input == 0) {
	/* Possible if server is in RawMode */
	return 0;
    }

    /*
     * We don't accept more than a small number of client packets per cycle.
     */
    count = 5;

    for (save_mask = fdmask; --count >= 0; fdmask = save_mask) {
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	n = select(max + 1, &fdmask, NULL, NULL, &tv);
	if (n == -1) {
	    if (errno != EINTR) {
		error("Select error checking input");
		return -1;
	    }
	    /* Interrupted */
	    continue;
	}
	if (n == 0) {
	    /* No input */
	    break;
	}
	for (i = 0; i < num_input; i++) {
	    if (BIT(fdmask, 1 << input_fd[i])) {
		CLR_BIT(fdmask, 1 << input_fd[i]);
		connp = &Conn[input_ind[i]];
		if (Handle_input(input_ind[i]) == -1
		    || connp->state != CONN_PLAYING) {
		    CLR_BIT(save_mask, 1 << input_fd[i]);
		    if (i < num_input - 1) {
			input_fd[i] = input_fd[num_input - 1];
			input_ind[i] = input_ind[num_input - 1];
			i--;
		    }
		    if (--num_input <= 0) {
			return 1;
		    }
		} else {
		    connp->start = loops;
		}
		if (--n <= 0) {
		    break;
		}
	    }
	}
    }
    return 1;
}

/*
 * Process packets from clients that are still busy trying to setup
 * a connection.
 *
 * This is silly.
 * We have two separate select loops for the same connection array.
 * Can't we combine them?
 * Sometime later.
 */
int Check_new_connections(void)
{
    int			i,
			n,
			num_logins = 0;
    connection_t	*connp;

    login_in_progress = 0;

    for (i = 0; i < max_connections; i++) {
	connp = &Conn[i];
	if (connp->state == CONN_FREE) {
	    continue;
	}
	if (connp->state == CONN_PLAYING) {
	    continue;
	}
	if (loops - connp->start > CONNECTION_TIMEOUT) {
	    errno = 0;
	    error("Connection timed out (%02x) for %s=%s@%s|%s", connp->state,
		connp->nick, connp->real, connp->host, connp->dpy);
	    Destroy_connection(i, __FILE__, __LINE__);
	    continue;
	}

	if (connp->state == CONN_DRAIN
	    || connp->state == CONN_READY) {
	    if (connp->c.len > 0) {
		while ((n = Handle_input(i)) > 0) {
		    if (connp->state != CONN_DRAIN
			&& connp->state != CONN_READY) {
			break;
		    }
		    if (connp->c.len <= 0) {
			break;
		    }
		}
		if (n == -1
		    || connp->state != CONN_DRAIN
		    && connp->state != CONN_READY) {
		    continue;
		}
		if (connp->c.len > 0) {
		    if (Send_reliable(i) == -1) {
			continue;
		    }
		    if (connp->state != CONN_READY
			|| connp->c.buf[0] == PKT_REPLY
			|| connp->c.buf[0] == PKT_PLAY
			|| connp->c.buf[0] == PKT_SUCCESS
			|| connp->c.buf[0] == PKT_FAILURE) {
			login_in_progress = 1;
			continue;
		    }
		}
	    }
	    connp->state = connp->drain_state;
	    connp->start = loops;
	    if (connp->state == CONN_PLAYING) {
		continue;
	    }
	}

	if (connp->state == CONN_SETUP) {
	    Handle_setup(i);
	}
	else if (connp->state == CONN_LISTENING) {
	    Handle_listening(i);
	}
	else if (connp->state == CONN_LOGIN) {
	    /*
	     * No more than one login per timeframe in order
	     * to prevent noticeable delay in the game.
	     */
	    if (num_logins == 0
		&& Handle_input(i) == 2) {
		num_logins++;
	    }
	} else {
	    errno = 0;
	    error("Unknown connection state (%02x) for %s=%s@%s|%s",
		connp->state,
		connp->nick, connp->real, connp->host, connp->dpy);
	    Destroy_connection(i, __FILE__, __LINE__);
	    continue;
	}

	/*
	 * If this player is still trying to log in
	 * and either he doesn't have any pending reliable data
	 * or it isn't time to send it yet
	 * or the send doesn't fail
	 * then we have a pending log in going on.  Phew!
	 */
	if (connp->state != CONN_FREE
	    && connp->state != CONN_PLAYING
	    && (connp->c.len <= 0
	    || loops - connp->last_send_loops < 2
	    || Send_reliable(i) != -1)) {
	    login_in_progress = 1;
	}
    }

    return (num_logins > 0) ? 1 : 0;
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
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }
    return n;
}

/*
 * Send all frame data related to the player self and his HUD.
 */
int Send_self(int ind,
    int x, int y, int vx, int vy, int dir,
    float power, float turnspeed, float turnresistance,
    int lock_id, int lock_dist, int lock_dir,
    int check, int cloaks, int sensors, int mines,
    int missiles, int ecms, int transporters, int extra_shots, int back_shots,
    int afterburners, int num_tanks, int current_tank,
    int fuel_sum, int fuel_max)
{
    int			pw, ts, tr;

    pw = (int) (power + 0.5);
    ts = (int) (turnspeed + 0.5);
    tr = (int) (turnresistance * 255.0 + 0.5);
    return Packet_printf(&Conn[ind].w, "%c" "%hd%hd%hd%hd%c" "%c%c%c"
	"%hd%hd%c" "%c%c%c%c" "%c%c%c%c%c" "%c%c%c" "%hd%hd",
	PKT_SELF,
	x, y, vx, vy, dir,
	pw, ts, tr,
	lock_id, lock_dist, lock_dir,
	check, cloaks, sensors, mines,
	missiles, ecms, transporters, extra_shots, back_shots,
	afterburners, num_tanks, current_tank,
	fuel_sum >> FUEL_SCALE_BITS, fuel_max >> FUEL_SCALE_BITS);
}

/*
 * Somebody is leaving the game.
 */
int Send_leave(int ind, int id)
{
    connection_t	*connp = &Conn[ind];

    if (connp->state != CONN_PLAYING
	&& connp->state != CONN_READY) {
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

    if (connp->state != CONN_PLAYING
	&& connp->state != CONN_READY) {
	errno = 0;
	error("Connection not ready for war declaration (%d,%d,%d)",
	    ind, connp->state, connp->id);
	return 0;
    }
    return Packet_printf(&connp->c, "%c%hd%hd",
	PKT_WAR, robot_id, killer_id);
}

/*
 * Somebody is programming a robot to seek some player.
 */
int Send_seek(int ind, int programmer_id, int robot_id, int sought_id)
{
    connection_t	*connp = &Conn[ind];

    if (connp->state != CONN_PLAYING
	&& connp->state != CONN_READY) {
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
int Send_player(int ind, int id, int team, int mychar, char *name)
{
    connection_t	*connp = &Conn[ind];

    if (connp->state != CONN_PLAYING
	&& connp->state != CONN_READY) {
	errno = 0;
	error("Connection not ready for player info (%d,%d)",
	    connp->state, connp->id);
	return 0;
    }
    return Packet_printf(&connp->c, "%c%hd%c%c%s", PKT_PLAYER,
	id, team, mychar, name);
}

/*
 * Send the new score for some player to a client.
 */
int Send_score(int ind, int id, int score, int life)
{
    connection_t	*connp = &Conn[ind];

    if (connp->state != CONN_PLAYING
	&& connp->state != CONN_READY) {
	errno = 0;
	error("Connection not ready for score(%d,%d)",
	    connp->state, connp->id);
	return 0;
    }
    return Packet_printf(&connp->c, "%c%hd%hd%hd", PKT_SCORE,
	id, score, life);
}

/*
 * Send info about a player having which base.
 */
int Send_base(int ind, int id, int num)
{
    connection_t	*connp = &Conn[ind];

    if (connp->state != CONN_PLAYING
	&& connp->state != CONN_READY) {
	errno = 0;
	error("Connection not ready for base info (%d,%d)",
	    connp->state, connp->id);
	return 0;
    }
    return Packet_printf(&connp->c, "%c%hd%hu", PKT_BASE, id, num);
}

int Send_fuel(int ind, int num, int fuel)
{
    return Packet_printf(&Conn[ind].w, "%c%hu%hu", PKT_FUEL,
	num, fuel >> FUEL_SCALE_BITS);
}

int Send_score_object(int ind, int score, int x, int y, char *string)
{
    connection_t	*connp = &Conn[ind];

    if (connp->state != CONN_PLAYING
	&& connp->state != CONN_READY) {
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

int Send_shot(int ind, int x, int y, int color)
{
    if (color < 0 || color >= NUM_COLORS) {
	errno = 0;
	error("Bad shot color %d", color);
	return -1;
    }
    return Packet_printf(&Conn[ind].w, "%c%hd%hd", PKT_SHOT + color, x, y);
}

int Send_smart(int ind, int x, int y, int dir)
{
    return Packet_printf(&Conn[ind].w, "%c%hd%hd%c", PKT_SMART, x, y, dir);
}

int Send_ball(int ind, int x, int y, int id)
{
    return Packet_printf(&Conn[ind].w, "%c%hd%hd%hd", PKT_BALL, x, y, id);
}

int Send_mine(int ind, int x, int y)
{
    return Packet_printf(&Conn[ind].w, "%c%hd%hd", PKT_MINE, x, y);
}

int Send_target(int ind, int num, int dead_time, int damage)
{
    return Packet_printf(&Conn[ind].w, "%c%hu%hu%hu", PKT_TARGET, 
			 num, dead_time, damage);
}

int Send_item(int ind, int x, int y, int type)
{
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

int Send_ship(int ind, int x, int y, int id, int dir, int shield, int cloak)
{
    return Packet_printf(&Conn[ind].w, "%c%hd%hd%hd%c%c", PKT_SHIP,
	x, y, id, dir, (shield != 0) | ((cloak != 0) << 1));
}

int Send_refuel(int ind, int x0, int y0, int x1, int y1)
{
    return Packet_printf(&Conn[ind].w, "%c%hd%hd%hd%hd", PKT_REFUEL,
	x0, y0, x1, y1);
}

int Send_connector(int ind, int x0, int y0, int x1, int y1)
{
    return Packet_printf(&Conn[ind].w, "%c%hd%hd%hd%hd", PKT_CONNECTOR,
	x0, y0, x1, y1);
}

int Send_radar(int ind, int x, int y)
{
    return Packet_printf(&Conn[ind].w, "%c%hd%hd", PKT_RADAR, x, y);
}

int Send_damaged(int ind, int damaged)
{
    return Packet_printf(&Conn[ind].w, "%c%c", PKT_DAMAGED, damaged);
}

int Send_audio(int ind, int type, int vol)
{
    return Packet_printf(&Conn[ind].w, "%c%c%c", PKT_AUDIO, type, vol);
}

int Send_message(int ind, char *msg)
{
    connection_t	*connp = &Conn[ind];

    if (connp->state != CONN_PLAYING
	&& connp->state != CONN_READY) {
	errno = 0;
	error("Connection not ready for message (%d,%d)",
	    connp->state, connp->id);
	return 0;
    }
    if (strlen(msg) >= MSG_LEN) {
	errno = 0;
	error("Output message too big (%s)", msg);
	return 0;
    }
    /*
     * Well, messages are not really crucial info.
     * But we treat them as such to test the ack mechanism.
     * They are currently very inefficiently transmitted as
     * ASCII strings.  We'd better encode them as a packet code
     * with one or two arguments.  But that would require
     * too much change to the rest of the server for now.
     * Perhaps later...
     * Moving spark and explosions to the client is much more important.
     */
    return Packet_printf(&connp->c, "%c%S", PKT_MESSAGE, msg);
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
    if (Packet_printf(&connp->w, "%c%ld%c", PKT_START,
		      loops, connp->last_key_change) <= 0) {
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }

    /* Return ok */
    return 0;
}

int Send_end_of_frame(int ind)
{
    connection_t	*connp = &Conn[ind];
    int			n;
    extern int		last_packet_of_frame;

    last_packet_of_frame = 1;
    n = Packet_printf(&connp->w, "%c%ld", PKT_END, loops);
    last_packet_of_frame = 0;
    if (n == -1) {
	Destroy_connection(ind, __FILE__, __LINE__);
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
    if (connp->c.len > 0 && connp->w.len < 512) {
	if (Send_reliable(ind) == -1) {
	    return -1;
	}
	if (connp->w.len == 0) {
	    return 1;
	}
    }
    if (Sockbuf_flush(&connp->w) == -1) {
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }
    Sockbuf_clear(&connp->w);
    return 0;
}

static int Receive_send_bufsize(int ind)
{
    connection_t	*connp = &Conn[ind];
    int			n,
			result;
    unsigned char	ch;
    unsigned short	bufsize;

    if ((n = Packet_scanf(&connp->r, "%c%hu", &ch, &bufsize)) <= 0) {
	if (n == -1) {
	    Destroy_connection(ind, __FILE__, __LINE__);
	}
	return n;
    }
    if (bufsize < MIN_SOCKBUF_SIZE
	|| bufsize > MAX_SOCKBUF_SIZE
	|| bufsize < connp->w.size
	|| SetSocketSendBufferSize(connp->w.sock, bufsize) == -1) {
	result = PKT_FAILURE;
    } else {
	result = PKT_SUCCESS;
    }
    if (Send_reply(ind, PKT_SEND_BUFSIZE, result) == -1) {
	return -1;
    }
    return 1;
}

static int Receive_keyboard(int ind)
{
    connection_t	*connp = &Conn[ind];
    player		*pl;
    int			change;

    if (connp->r.ptr + KEYBOARD_SIZE + 2 - connp->r.buf > connp->r.len) {
	/*
	 * Incomplete client packet.
	 */
	return 0;
    }
    connp->r.ptr++;
    change = (*connp->r.ptr++ & 0xFF);
    if (change == connp->last_key_change) {
	/*
	 * We already have this key.
	 * Nothing to do.
	 */
    }
    else if ((change > connp->last_key_change)
	? (change - connp->last_key_change < 128)
	: (change + 256 - connp->last_key_change < 128)) {
	connp->last_key_change = change;
	pl = Players[GetInd[connp->id]];
	memcpy(pl->last_keyv, connp->r.ptr, sizeof(pl->last_keyv));
	pl->key_changed = 1;
	Handle_keyboard(GetInd[connp->id]);
    } else {
#ifndef SILENT
	static long keydrop;
	if (loops - keydrop > 2*FPS) {
	    keydrop = loops;
	    printf("Key drop (%d,%d,%d)\n",
		ind, change, connp->last_key_change);
	}
#endif
    }
    connp->r.ptr += sizeof(pl->last_keyv);

    return 1;
}

static int Receive_quit(int ind)
{
    connection_t	*connp = &Conn[ind];

    errno = 0;
    error("Got quit packet from %s=%s@%s|%s",
	connp->nick, connp->real, connp->host, connp->dpy);
    Destroy_connection(ind, __FILE__, __LINE__);
    return -1;
}

static int Receive_play(int ind)
{
    connection_t	*connp = &Conn[ind];
    unsigned char	ch;
    int			n;

    if ((n = Packet_scanf(&connp->r, "%c", &ch)) != 1) {
	if (n == -1) {
	    Destroy_connection(ind, __FILE__, __LINE__);
	} else {
	    errno = 0;
	    error("Can't receive play packet");
	    Destroy_connection(ind, __FILE__, __LINE__);
	}
	return -1;
    }
    if (ch != PKT_PLAY) {
	errno = 0;
	error("Packet is not of play type");
	Destroy_connection(ind, __FILE__, __LINE__);
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
	    Destroy_connection(ind, __FILE__, __LINE__);
	    return -1;
	}
	if (Send_reliable(ind) == -1) {
	    return -1;
	}
	return 0;
    }
    Sockbuf_clear(&connp->w);
    if (Handle_login(ind) == -1) {
	Destroy_connection(ind, __FILE__, __LINE__);
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
    float		power;

    if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &tmp)) <= 0) {
	if (n == -1) {
	    Destroy_connection(ind, __FILE__, __LINE__);
	}
	return n;
    }
    power = (float) tmp / 256.0F;
    pl = Players[GetInd[connp->id]];
    switch (ch) {
    case PKT_POWER:
	pl->power = power;
	break;
    case PKT_POWER_S:
	pl->power_s = power;
	break;
    case PKT_TURNSPEED:
	pl->turnspeed = power;
	break;
    case PKT_TURNSPEED_S:
	pl->turnspeed_s = power;
	break;
    case PKT_TURNRESISTANCE:
	pl->turnresistance = power;
	break;
    case PKT_TURNRESISTANCE_S:
	pl->turnresistance_s = power;
	break;
    default:
	errno = 0;
	error("Not a power packet (%d,%02x)", ch, connp->state);
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }
    return 1;
}

/*
 * This thing still isn't finished, but it works better than in PL 0 I hope.
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
    const int		max_packet_size = 512,
			min_send_size = 4;

    if (connp->c.len <= 0
	|| connp->last_send_loops == loops) {
	connp->last_send_loops = loops;
	return 0;
    }
    read_buf = connp->c.buf;
    max_todo = connp->c.len;
    rel_off = connp->reliable_offset;
    if (connp->w.len > 0) {
	/* We're piggybacking on a frame update. */
	if (connp->w.len >= max_packet_size - min_send_size) {
	    /* Frame already too big */
	    return 0;
	}
	if (max_todo > max_packet_size - connp->w.len) {
	    /* Don't exceed minimum fragment size. */
	    max_todo = max_packet_size - connp->w.len;
	}
    }
    if (connp->retransmit_at_loop > loops) {
	/*
	 * It's no time to retransmit yet.
	 */
	if (max_todo <= connp->reliable_unsend - connp->reliable_offset
			+ min_send_size
	    || connp->w.len == 0) {
	    /*
	     * And we can't send anything new either
	     * and we don't want to introduce a new packet.
	     */
	    return 0;
	}
    }
    else if (connp->retransmit_at_loop != 0) {
	/*
	 * Timeout.
	 */
	connp->acks >>= 1;
    }

    todo = max_todo;
    for (i = 0; i <= connp->acks && todo > 0; i++) {
	len = (todo > max_packet_size) ? max_packet_size : todo;
	if (Packet_printf(&connp->w, "%c%hd%ld%ld", PKT_RELIABLE,
			  len, rel_off, loops) <= 0) {
	    error("Can't print reliable data header");
	    Destroy_connection(ind, __FILE__, __LINE__);
	    return -1;
	}
	if (Sockbuf_write(&connp->w, read_buf, len) != len) {
	    error("Can't write reliable data");
	    Destroy_connection(ind, __FILE__, __LINE__);
	    return -1;
	}
	if ((n = Sockbuf_flush(&connp->w)) < len) {
	    if (n == 0
		&& (errno == EWOULDBLOCK
		    || errno == EAGAIN)) {
		connp->acks = 0;
		break;
	    } else {
		error("Can't flush reliable data (%d)", n);
		Destroy_connection(ind, __FILE__, __LINE__);
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

    connp->last_send_loops = loops;

    if (max_todo - todo <= 0) {
	/*
	 * We haven't transmitted anything at all.
	 */
	return 0;
    }

    /*
     * Retransmission timer with exponential backoff.
     */
    if (connp->rtt_retransmit > MAX_RETRANSMIT) {
	connp->rtt_retransmit = MAX_RETRANSMIT;
    }
    if (connp->retransmit_at_loop <= loops) {
	connp->retransmit_at_loop = loops + connp->rtt_retransmit;
	connp->rtt_retransmit <<= 1;
	connp->rtt_timeouts++;
    } else {
	connp->retransmit_at_loop = loops + connp->rtt_retransmit;
    }

    if (rel_off > connp->reliable_unsend) {
	connp->reliable_unsend = rel_off;
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
	error("Can't read ack packet (%d)", n);
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }
    if (ch != PKT_ACK) {
	errno = 0;
	error("Not an ack packet (%d)", ch);
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }
    rtt = loops - rel_loops;
    if (rtt > 0 && rtt <= MAX_RTT) {
	/*
	 * These roundtrip estimation calculations are derived from Comer's
	 * books "Internetworking with TCP/IP" parts I & II.
	 */
	if (connp->rtt_smoothed == 0) {
	    /*
	     * Initialise the rtt estimator by this first measurement.
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
	/* Impossible to ack data that hasn't been send */
	errno = 0;
	error("Bad ack (diff=%ld,cru=%ld,c=%ld,len=%d)",
	    diff, rel, connp->reliable_offset, connp->c.len);
	Destroy_connection(ind, __FILE__, __LINE__);
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
    if (connp->reliable_offset >= connp->reliable_unsend) {
	/*
	 * All reliable data has been sent and acked.
	 */
	connp->retransmit_at_loop = 0;
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
    Destroy_connection(ind, __FILE__, __LINE__);
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
	    Destroy_connection(ind, __FILE__, __LINE__);
	}
	return n;
    }
    if (num >= World.NumCannons) {
	errno = 0;
	error("Bad ack cannon index %d,%ld", num);
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }
    if (loops_ack >= World.cannon[num].last_change) {
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
	    Destroy_connection(ind, __FILE__, __LINE__);
	}
	return n;
    }
    if (num >= World.NumFuels) {
	errno = 0;
	error("Bad ack fuel index %d,%ld", num);
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }
    if (loops_ack >= World.fuel[num].last_change) {
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
	    Destroy_connection(ind, __FILE__, __LINE__);
	}
	return n;
    }
    if (num >= World.NumTargets) {
	errno = 0;
	error("Bad ack target index %d,%ld", num);
	Destroy_connection(ind, __FILE__, __LINE__);
	return -1;
    }
    if (loops_ack >= World.targets[num].last_change) {
	SET_BIT(World.targets[num].conn_mask, 1 << ind);
    }
    return 1;
}

static int Receive_talk(int ind)
{
    connection_t	*connp = &Conn[ind];
    unsigned char	ch;
    int			n;
    long		seq;
    char		str[MAX_CHARS],
			msg[MSG_LEN];

    if ((n = Packet_scanf(&connp->r, "%c%ld%s", &ch, &seq, str)) <= 0) {
	if (n == -1) {
	    Destroy_connection(ind, __FILE__, __LINE__);
	}
	return n;
    }
    if (seq > connp->talk_sequence_num) {
	if ((n = Packet_printf(&connp->c, "%c%ld", PKT_TALK_ACK, seq)) <= 0) {
	    if (n == -1) {
		Destroy_connection(ind, __FILE__, __LINE__);
	    }
	    return n;
	}
	connp->talk_sequence_num = seq;
	sprintf(msg, " <<%s talks>> %s", connp->nick, str);
	Set_message(msg);
    }
    return 1;
}


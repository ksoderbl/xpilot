/* $Id: contact.c,v 4.8 1998/08/29 19:49:54 bert Exp $
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

#ifdef	_WINDOWS
#include "NT/winServer.h"
#include <time.h>
#include <process.h>
#include <limits.h>
#else
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#if !defined(__hpux)
#include <sys/time.h>
#endif
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "types.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "socklib.h"
#include "map.h"
#include "pack.h"
#include "bit.h"
#include "net.h"
#include "netserver.h"
#include "sched.h"
#include "error.h"
#include "checknames.h"
#include "server.h"

char contact_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: contact.c,v 4.8 1998/08/29 19:49:54 bert Exp $";
#endif

/*
 * Global variables
 */
int			NumQueuedPlayers = 0;
int			MaxQueuedPlayers = 20;
int			NumPseudoPlayers = 0;

static int		contactSocket;
static sockbuf_t	ibuf;
static char		msg[MSG_LEN];
extern int		game_lock;
extern time_t		gameOverTime;
extern time_t		serverTime;
extern int		login_in_progress;
extern char		ShutdownReason[];

static bool Owner(char request, char *real_name, char *host_addr,
		  int host_port, int pass);
static int Enter_player(char *real, char *nick, char *disp, int team,
			char *addr, char *host, unsigned version, int port,
			int *login_port);
void Queue_loop(void);
static int Queue_player(char *real, char *nick, char *disp, int team,
			char *addr, char *host, unsigned version, int port,
			int *qpos);
void Contact(int fd, void *arg);
static int Check_address(char *addr);

#ifdef	_WINDOWS
#define	getpid()	_getpid()
#endif

void Contact_cleanup(void)
{
    DgramClose(contactSocket);
    contactSocket = -1;
}

int Contact_init(void)
{
    /*
     * Create a socket which we can listen on.
     */
    SetTimeout(0, 0);
    if ((contactSocket = CreateDgramSocket(contactPort)) == -1) {
	error("Could not create Dgram contactSocket");
	End_game();
    }
    if (SetSocketNonBlocking(contactSocket, 1) == -1) {
	error("Can't make contact socket non-blocking");
	End_game();
    }
    if (Sockbuf_init(&ibuf, contactSocket, SERVER_SEND_SIZE,
		     SOCKBUF_READ | SOCKBUF_WRITE | SOCKBUF_DGRAM) == -1) {
	error("No memory for contact buffer");
	End_game();
    }

    install_input(Contact, contactSocket, 0);

    return contactSocket;
}

/*
 * Kick robot players?
 * Return the number of kicked robots.
 * Don't kick more than one robot.
 */
static int Kick_robot_players(int team)
{
    if (NumRobots == 0)		/* no robots available for kicking */
	return 0;
    if (team == TEAM_NOT_SET) {
	if (BIT(World.rules->mode, TEAM_PLAY) && reserveRobotTeam) {
	    /* kick robot with lowest score from any team but robotTeam */
	    int low_score = LONG_MAX;
	    int low_i = -1;
	    int i;
	    for (i = 0; i < NumPlayers; i++) {
		if (!IS_ROBOT_IND(i) || Players[i]->team == robotTeam)
		    continue;
		if (Players[i]->score < low_score) {
		    low_i = i;
		    low_score = Players[i]->score;
		}
	    }
	    if (low_i >= 0) {
		Robot_delete(low_i, true);
		return 1;
	    }
	    return 0;
	} else {
	    /* kick random robot */
	    Robot_delete(-1, true);
	    return 1;
	}
    } else {
	if (World.teams[team].NumRobots > 0) {
	    /* kick robot with lowest score from this team */
	    int low_score = LONG_MAX;
	    int low_i = -1;
	    int i;
	    for (i = 0; i < NumPlayers; i++) {
		if (!IS_ROBOT_IND(i) || Players[i]->team != team)
		    continue;
		if (Players[i]->score < low_score) {
		    low_i = i;
		    low_score = Players[i]->score;
		}
	    }
	    if (low_i >= 0) {
		Robot_delete(low_i, true);
		return 1;
	    }
	    return 0;
	} else {
	    return 0;		/* no robots in this team */
	}
    }
}

/*
 * Kick paused players?
 * Return the number of kicked players.
 */
static int Kick_paused_players(int team)
{
    int			i;
    int			num_unpaused = 0;

    for (i = NumPlayers - 1; i >= 0; i--) {
	if (Players[i]->conn != NOT_CONNECTED
	    && BIT(Players[i]->status, PAUSE)
	    && (team == TEAM_NOT_SET || Players[i]->team == team)) {
	    if (team == TEAM_NOT_SET) {
		sprintf(msg,
			"The paused \"%s\" was kicked because the game is full.",
			Players[i]->name);
		Destroy_connection(Players[i]->conn, "no pause with full game");
	    } else {
		sprintf(msg,
			"The paused \"%s\" was kicked because team %d is full.",
			Players[i]->name, team);
		Destroy_connection(Players[i]->conn, "no pause with full team");
	    }
	    Set_message(msg);
	    num_unpaused++;
	}
    }

    return num_unpaused;
}


static int Reply(char *host_addr, int port)
{
    int			i, result;
    const int		max_send_retries = 3;

    for (i = 0; i < max_send_retries; i++) {
	if ((result = DgramSend(ibuf.sock, host_addr, port, ibuf.buf, ibuf.len)) == -1) {
	    GetSocketError(ibuf.sock);
	} else {
	    break;
	}
    }

    return result;
}


static int Check_names(char *nick_name, char *real_name, char *host_name)
{
    char		*ptr;
    int			i;

    /*
     * Bad input parameters?
     */
    if (real_name[0] == 0
	|| host_name[0] == 0
	|| nick_name[0] < 'A'
	|| nick_name[0] > 'Z') {
	return E_INVAL;
    }

    /*
     * All names must be unique (so we know who we're talking about).
     */
    /* strip trailing whitespace. */
    for (ptr = &nick_name[strlen(nick_name)]; ptr-- > nick_name; ) {
	if (isascii(*ptr) && isspace(*ptr)) {
	    *ptr = '\0';
	} else {
	    break;
	}
    }
    for (i = 0; i < NumPlayers; i++) {
	if (strcasecmp(Players[i]->name, nick_name) == 0) {
	    D(printf("%s %s\n", Players[i]->name, nick_name);)
	    return E_IN_USE;
	}
    }

    return SUCCESS;
}


/*
 * Support some older clients, which don't know
 * that they can join the current version.
 *
 * IMPORTANT! Adjust the next code if you're changing version numbers.
 */
static unsigned Version_to_magic(unsigned version)
{
    if (version >= 0x3100 && version <= MY_VERSION) {
	return VERSION2MAGIC(version);
    }
    return MAGIC;
}

void Contact(int fd, void *arg)
{
    int			i,
			team,
			bytes,
			delay,
			login_port,
			max_robots,
    			qpos,
			status;
	char	reply_to;
    unsigned		magic,
			version,
			my_magic;
    unsigned short	port;
    char		ch,
			real_name[MAX_CHARS],
			disp_name[MAX_CHARS],
			nick_name[MAX_CHARS],
			host_name[MAX_CHARS],
			host_addr[24],
			str[MSG_LEN];

    /*
     * Someone connected to us, now try and decipher the message :)
     */
    Sockbuf_clear(&ibuf);
    if ((bytes = DgramReceiveAny(contactSocket, ibuf.buf, ibuf.size)) <= 8) {
	if (bytes < 0
	    && errno != EWOULDBLOCK
	    && errno != EAGAIN
	    && errno != EINTR) {
	    /*
	     * Clear the error condition for the contact socket.
	     */
	    GetSocketError(contactSocket);
	}
	return;
    }
    ibuf.len = bytes;

    strcpy(host_addr, DgramLastaddr());
    if (Check_address(host_addr)) {
	return;
    }

    /*
     * Determine if we can talk with this client.
     */
    if (Packet_scanf(&ibuf, "%u", &magic) <= 0
	|| (magic & 0xFFFF) != (MAGIC & 0xFFFF)) {
	D(printf("Incompatible packet from %s (0x%08x)", host_addr, magic);)
	return;
    }
    version = MAGIC2VERSION(magic);

    /*
     * Read core of packet.
     */
    if (Packet_scanf(&ibuf, "%s%hu%c", real_name, &port, &ch) <= 0) {
	D(printf("Incomplete packet from %s", host_addr);)
	return;
    }
    Fix_real_name(real_name);
    reply_to = (ch & 0xFF);	/* no sign extension. */

    /* ignore port for termified clients. */
    port = DgramLastport();

    /*
     * Now see if we have the same (or a compatible) version.
     * If the client request was only a contact request (to see
     * if there is a server running on this host) then we don't
     * care about version incompatibilities, so that the client
     * can decide if it wants to conform to our version or not.
     */
    if (version < MIN_CLIENT_VERSION
	|| (version > MAX_CLIENT_VERSION
	    && reply_to != CONTACT_pack)) {
	D(error("Incompatible version with %s@%s (%04x,%04x)",
	    real_name, host_addr, MY_VERSION, version);)
	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c%c", MAGIC, reply_to, E_VERSION);
	Reply(host_addr, port);
	return;
    }

    my_magic = Version_to_magic(version);

    status = SUCCESS;

    if (reply_to & PRIVILEGE_PACK_MASK) {
	long			key;
	static long		credentials;

	if (!credentials) {
	    credentials = (time(NULL) * (time_t)getpid());
	    credentials ^= (long)Contact;
	    credentials	+= (long)key + (long)&key;
	    credentials ^= (long)rand() << 1;
	    credentials &= 0xFFFFFFFF;
	}
	if (Packet_scanf(&ibuf, "%ld", &key) <= 0) {
	    return;
	}
	if (!Owner(reply_to, real_name, host_addr, port, key == credentials)) {
	    Sockbuf_clear(&ibuf);
	    Packet_printf(&ibuf, "%u%c%c", my_magic, reply_to, E_NOT_OWNER);
	    Reply(host_addr, port);
	    return;
	}
	if (reply_to == CREDENTIALS_pack) {
	    Sockbuf_clear(&ibuf);
	    Packet_printf(&ibuf, "%u%c%c%ld", my_magic, reply_to, SUCCESS, credentials);
	    Reply(host_addr, port);
	    return;
	}
    }

    /*
     * Now decode the packet type field and do something witty.
     */
    switch (reply_to) {

    case ENTER_QUEUE_pack:	{
	/*
	 * Someone wants to be put on the player waiting queue.
	 */
	if (Packet_scanf(&ibuf, "%s%s%s%d", nick_name, disp_name, host_name,
			 &team) <= 0) {
	    D(printf("Incomplete enter queue from %s@%s", real_name, host_addr);)
	    return;
	}
	Fix_nick_name(nick_name);
	Fix_disp_name(disp_name);
	Fix_host_name(host_name);
	if (team < 0 || team >= MAX_TEAMS) {
	    team = TEAM_NOT_SET;
	}

	status = Queue_player(real_name, nick_name,
			      disp_name, team,
			      host_addr, host_name,
			      version, port,
			      &qpos);
	if (status < 0) {
	    return;
	}
	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c%c%hu", my_magic, reply_to, status, qpos);
    }
	break;

    case ENTER_GAME_pack:	{
	/*
	 * Someone wants to enter the game.
	 */
	if (Packet_scanf(&ibuf, "%s%s%s%d", nick_name, disp_name, host_name,
			 &team) <= 0) {
	    D(printf("Incomplete login from %s@%s", real_name, host_addr);)
	    return;
	}
	Fix_nick_name(nick_name);
	Fix_disp_name(disp_name);
	Fix_host_name(host_name);
	if (team < 0 || team >= MAX_TEAMS) {
	    team = TEAM_NOT_SET;
	}

	status = Enter_player(real_name, nick_name,
			      disp_name, team,
			      host_addr, host_name,
			      version, port,
			      &login_port);
	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c%c%hu", my_magic, reply_to, status, login_port);
    }
	break;


    case REPORT_STATUS_pack:	{
	/*
	 * Someone asked for information.
	 */

#ifndef	SILENT
	xpprintf("%s %s@%s asked for info about current game.\n",
	       showtime(), real_name, host_addr);
#endif
	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c%c", my_magic, reply_to, SUCCESS);
	Server_info(ibuf.buf + ibuf.len, ibuf.size - ibuf.len);
	ibuf.buf[ibuf.size - 1] = '\0';
	ibuf.len += strlen(ibuf.buf + ibuf.len) + 1;
    }
	break;


    case MESSAGE_pack:	{
	/*
	 * Someone wants to transmit a message to the server.
	 */

	if (Packet_scanf(&ibuf, "%s", str) <= 0) {
	    status = E_INVAL;
	}
	else {
	    sprintf(msg, "%s [%s SPEAKING FROM ABOVE]",
		    str, real_name);
	    Set_message(msg);
	}
	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c%c", my_magic, reply_to, status);
    }
	break;


    case LOCK_GAME_pack:	{
	/*
	 * Someone wants to lock the game so that no more players can enter.
	 */

	game_lock = game_lock ? false : true;
	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c%c", my_magic, reply_to, status);
    }
	break;


    case CONTACT_pack:	{
	/*
	 * Got contact message from client.
	 */

	D(printf("Got CONTACT from %s.\n", host_addr);)
	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c%c", my_magic, reply_to, status);
    }
	break;


    case SHUTDOWN_pack:	{
	/*
	 * Shutdown the entire server.
	 */

	if (Packet_scanf(&ibuf, "%d%s", &delay, ShutdownReason) <= 0) {
	    status = E_INVAL;
	} else {
	    sprintf(msg, "|*******| %s (%s) |*******| \"%s\"",
		(delay > 0) ? "SHUTTING DOWN" : "SHUTDOWN STOPPED",
		real_name, ShutdownReason);
	    if (delay > 0) {
		ShutdownServer = delay * FPS;		/* delay is in seconds */;
		ShutdownDelay = ShutdownServer;
	    } else {
		ShutdownServer = -1;
	    }
	    Set_message(msg);
	}

	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c%c", my_magic, reply_to, status);
    }
	break;


    case KICK_PLAYER_pack:	{
	/*
	 * Kick someone from the game.
	 */
	int			found = -1;

	if (Packet_scanf(&ibuf, "%s", str) <= 0) {
	    status = E_INVAL;
	}
	else {
	    for (i=0; i<NumPlayers; i++) {
		/*
		 * Kicking players by realname is not a good idea,
		 * because several players may have the same realname.
		 * E.g., system administrators joining as root...
		 */
		if (strcasecmp(str, Players[i]->name) == 0
		    || strcasecmp(str, Players[i]->realname) == 0) {
		    found = i;
		}
	    }
	    if (found == -1) {
		status = E_NOT_FOUND;
	    } else {
		sprintf(msg,
			"\"%s\" upset the gods and was kicked out of the game.",
			 Players[found]->name);
		Set_message(msg);
		if (Players[found]->conn == NOT_CONNECTED) {
		    Delete_player(found);
		} else {
		    Destroy_connection(Players[found]->conn, "kicked out");
		}
		updateScores = true;
	    }
	}

	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c%c", my_magic, reply_to, status);
    }
	break;

    case OPTION_TUNE_pack:		{
	/*
	 * Tune a server option.  (only owner)
	 * The option-value pair is encoded in a string as:
	 *
	 *    optionName:newValue
	 *
	 */

	char		*opt, *val;

	if (Packet_scanf(&ibuf, "%S", str) <= 0
		 || (opt = strtok(str, ":")) == NULL
		 || (val = strtok(NULL, "")) == NULL
		) {
	    status = E_INVAL;
	}
	else {
	    i = Tune_option(opt, val);
	    if (i == 1) {
		status = SUCCESS;
	    }
	    else if (i == 0) {
		status = E_INVAL;
	    }
	    else if (i == -1) {
		status = E_UNDEFINED;
	    }
	    else if (i == -2) {
		status = E_NOENT;
	    }
	    else {
		status = E_INVAL;
	    }
	}
	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c%c", my_magic, reply_to, status);
    }
	break;

    case OPTION_LIST_pack:		{
	/*
	 * List the server options and their current values.
	 */
	bool		bad = false, full, change;

#ifndef	SILENT
	xpprintf("%s %s@%s asked for an option list.\n",
	       showtime(), real_name, host_addr);
#endif
	i = 0;
	do {
	    Sockbuf_clear(&ibuf);
	    Packet_printf(&ibuf, "%u%c%c", my_magic, reply_to, status);

	    for (change = false, full = false; !full && !bad; ) {
		switch (Parse_list(&i, str)) {
		case -1:
		    bad = true;
		    break;
		case 0:
		    i++;
		    break;
		default:
		    switch (Packet_printf(&ibuf, "%s", str)) {
		    case 0:
			full = true;
			bad = (change) ? false : true;
			break;
		    case -1:
			bad = true;
			break;
		    default:
			change = true;
			i++;
			break;
		    }
		    break;
		}
	    }
	    if (change
		&& Reply(host_addr, port) == -1) {
		bad = true;
	    }
	} while (!bad);
    }
	return;

    case MAX_ROBOT_pack:	{
	/*
	 * Set the maximum of robots wanted in the server
	 */
	if (Packet_scanf(&ibuf, "%d", &max_robots) <= 0
	    || max_robots < 0) {
	    status = E_INVAL;
	}
	else {
	    maxRobots = max_robots;
	    while (maxRobots < NumRobots) {
		Robot_delete(-1, true);
	    }
	}

	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c%c", my_magic, reply_to, status);
    }
	break;


    default:
	/*
	 * Incorrect packet type.
	 */
	D(printf("Unknown packet type (%d) from %s@%s.\n",
	    reply_to, real_name, host_addr);)

	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c%c", my_magic, reply_to, E_VERSION);
    }

    Reply(host_addr, port);
}

static int Enter_player(char *real, char *nick, char *disp, int team,
			char *addr, char *host, unsigned version, int port,
			int *login_port)
{
    int			status;

    *login_port = 0;

    /*
     * Game locked?
     */
    if (game_lock) {
	return E_GAME_LOCKED;
    }

    /*
     * Is the game full?
     */
    if (NumPlayers - NumPseudoPlayers + login_in_progress + NumQueuedPlayers >= World.NumBases) {
	if (NumQueuedPlayers > 0) {
	    return E_GAME_FULL;
	}
	if (!Kick_robot_players(TEAM_NOT_SET)) {
	    if (!Kick_paused_players(TEAM_NOT_SET)) {
		return E_GAME_FULL;
	    }
	}
	if (NumPlayers - NumPseudoPlayers + login_in_progress + NumQueuedPlayers >= World.NumBases) {
	    return E_GAME_FULL;
	}
    }

    if ((status = Check_names(nick, real, host)) != SUCCESS) {
	return status;
    }

    /*
     * Maybe don't have enough room for player on that team?
     */
    if (BIT(World.rules->mode, TEAM_PLAY)) {
	if (team < 0 || team >= MAX_TEAMS
	    || (team == robotTeam && reserveRobotTeam)) {
	    if (!teamAssign) {
		if (team == robotTeam && reserveRobotTeam) {
		    return E_TEAM_FULL;
		} else {
		    return E_TEAM_NOT_SET;
		}
	    }
	    team = Pick_team(PickForHuman);
	    if (team == TEAM_NOT_SET
		|| (team == robotTeam && reserveRobotTeam)) {
		if (NumRobots > World.teams[robotTeam].NumRobots) {
		    if (!Kick_robot_players(TEAM_NOT_SET)) {
			return E_TEAM_NOT_SET;
		    }
		    team = Pick_team(PickForHuman);
		    if (team == TEAM_NOT_SET) {
			return E_TEAM_NOT_SET;
		    }
		} else {
		    return E_TEAM_NOT_SET;
		}
	    }
	}
	else if (World.teams[team].NumMembers >= World.teams[team].NumBases) {
	    if (!Kick_robot_players(team)) {
		if (!Kick_paused_players(team)) {
		    return E_TEAM_FULL;
		}
	    }
	    team = Pick_team(PickForHuman);
	    if (team == TEAM_NOT_SET) {
		return E_TEAM_NOT_SET;
	    }
	}
    }

    /*
     * Find a port for the client to connect to.
     */
    *login_port = Setup_connection(real, nick,
				   disp, team,
				   host, host,
				   version);
    if (*login_port == -1) {
	return E_SOCKET;
    }

    return SUCCESS;
}

struct queued_player {
    struct queued_player	*next;
    char			real_name[MAX_CHARS];
    char			nick_name[MAX_CHARS];
    char			disp_name[MAX_CHARS];
    char			host_name[MAX_CHARS];
    char			host_addr[24];
    int				port;
    int				team;
    unsigned			version;
    int				login_port;
    long			last_ack_sent;
    long			last_ack_recv;
};

static struct queued_player	*qp_list;

static void Queue_remove(struct queued_player *qp, struct queued_player *prev)
{
    if (qp == qp_list) {
	qp_list = qp->next;
    } else {
	prev->next = qp->next;
    }
    free(qp);
    NumQueuedPlayers--;
}

static void Queue_ack(struct queued_player *qp, int qpos)
{
    unsigned		my_magic = Version_to_magic(qp->version);

    Sockbuf_clear(&ibuf);
    if (qp->login_port == -1) {
	Packet_printf(&ibuf, "%u%c%c%hu",
		      my_magic, ENTER_QUEUE_pack, SUCCESS, qpos);
    }
    else {
	Packet_printf(&ibuf, "%u%c%c%hu",
		      my_magic, ENTER_GAME_pack, SUCCESS, qp->login_port);
    }
    Reply(qp->host_addr, qp->port);
    qp->last_ack_sent = main_loops;
}

void Queue_loop(void)
{
    struct queued_player	*qp, *prev = 0, *next = 0;
    int				qpos = 0;
    int				login_port;
    static long			last_unqueued_loops;

    for (qp = qp_list; qp && qp->login_port > 0; ) {
	next = qp->next;

	if (qp->last_ack_recv + 30 * FPS < main_loops) {
	    Queue_remove(qp, prev);
	    qp = next;
	    continue;
	}
	if (qp->last_ack_sent + 2 < main_loops) {
	    login_port = Check_connection(qp->real_name, qp->nick_name,
					  qp->disp_name, qp->host_addr);
	    if (login_port == -1) {
		Queue_remove(qp, prev);
		qp = next;
		continue;
	    }
	    if (qp->last_ack_sent + 2 + (FPS >> 2) < main_loops) {
		Queue_ack(qp, 0);

		/* don't do too much at once. */
		return;
	    }
	}

	prev = qp;
	qp = next;
    }

    /* here's a player in the queue without a login port. */
    if (qp) {

	if (qp->last_ack_recv + 30 * FPS < main_loops) {
	    Queue_remove(qp, prev);
	    return;
	}

	/* slow down the rate at which players enter the game. */
	if (last_unqueued_loops + 2 + (FPS >> 2) < main_loops) {

	    /* is there a homebase available? */
	    if (NumPlayers - NumPseudoPlayers + login_in_progress < World.NumBases
		|| (Kick_robot_players(TEAM_NOT_SET)
		    && NumPlayers - NumPseudoPlayers + login_in_progress < World.NumBases)
		|| (Kick_paused_players(TEAM_NOT_SET)
		    && NumPlayers - NumPseudoPlayers + login_in_progress < World.NumBases)) {

		/* find a team for this fellow. */
		if (BIT(World.rules->mode, TEAM_PLAY)) {

		    /* see if he has a reasonable suggestion. */
		    if (qp->team >= 0 && qp->team < MAX_TEAMS) {
			if ((World.teams[qp->team].NumMembers
			     >= World.teams[qp->team].NumBases &&
				!Kick_robot_players(qp->team) &&
				!Kick_paused_players(qp->team))
			    || (qp->team == robotTeam && reserveRobotTeam)) {
			    qp->team = TEAM_NOT_SET;
			}
		    }
		    if (qp->team == TEAM_NOT_SET) {
			qp->team = Pick_team(PickForHuman);
			if (qp->team == TEAM_NOT_SET
			    || (qp->team == robotTeam && reserveRobotTeam)) {
			    if (NumRobots > World.teams[robotTeam].NumRobots) {
				Kick_robot_players(TEAM_NOT_SET);
				qp->team = Pick_team(PickForHuman);
			    }
			}
		    }
		}

		/* now get him a decent login port. */
		qp->login_port = Setup_connection(qp->real_name, qp->nick_name,
						  qp->disp_name, qp->team,
						  qp->host_addr, qp->host_name,
						  qp->version);
		if (qp->login_port == -1) {
		    Queue_remove(qp, prev);
		    return;
		}

		/* let him know he can proceed. */
		Queue_ack(qp, 0);

		last_unqueued_loops = main_loops;

		/* don't do too much at once. */
		return;
	    }
	}
    }

    for (; qp; ) {
	next = qp->next;

	qpos++;

	if (qp->last_ack_recv + 30 * FPS < main_loops) {
	    Queue_remove(qp, prev);
	    return;
	}

	if (qp->last_ack_sent + 3 * FPS <= main_loops) {
	    Queue_ack(qp, qpos);
	    return;
	}

	prev = qp;
	qp = next;
    }
}

static int Queue_player(char *real, char *nick, char *disp, int team,
			char *addr, char *host, unsigned version, int port,
			int *qpos)
{
    int				status = SUCCESS;
    struct queued_player	*qp, *prev = 0;
    int				num_queued = 0;
    int				num_same_hosts = 0;

    *qpos = 0;
    if ((status = Check_names(nick, real, host)) != SUCCESS) {
	return status;
    }

    for (qp = qp_list; qp; prev = qp, qp = qp->next) {

	num_queued++;
	if (qp->login_port == -1) {
	    ++*qpos;
	}

	/* same nick? */
	if (!strcmp(nick, qp->nick_name)) {
	    /* same screen? */
	    if (!strcmp(addr, qp->host_addr)
		&& !strcmp(real, qp->real_name)
		&& !strcmp(disp, qp->disp_name)) {
		qp->last_ack_recv = main_loops;
		qp->port = port;
		qp->version = version;
		qp->team = team;
		/*
		 * Still on the queue, so don't send an ack
		 * since it will get one soon from Queue_loop().
		 */
		return -1;
	    }
	    return E_IN_USE;
	}

	/* same computer? */
	if (!strcmp(addr, qp->host_addr)) {
	    if (++num_same_hosts > 1) {
		return E_IN_USE;
	    }
	}
    }

    NumQueuedPlayers = num_queued;
    if (NumQueuedPlayers >= MaxQueuedPlayers) {
	return E_GAME_FULL;
    }
    if (game_lock) {
	return E_GAME_LOCKED;
    }

    qp = (struct queued_player *)malloc(sizeof(struct queued_player));
    if (!qp) {
	return E_SOCKET;
    }
    ++*qpos;
    strcpy(qp->real_name, real);
    strcpy(qp->nick_name, nick);
    strcpy(qp->disp_name, disp);
    strcpy(qp->host_name, host);
    strcpy(qp->host_addr, addr);
    qp->port = port;
    qp->team = team;
    qp->version = version;
    qp->login_port = -1;
    qp->last_ack_sent = main_loops;
    qp->last_ack_recv = main_loops;

    qp->next = 0;
    if (!qp_list) {
	qp_list = qp;
    } else {
	prev->next = qp;
    }
    NumQueuedPlayers++;

    return SUCCESS;
}

/*
 * Returns true if <name> has owner status of this server.
 */
static bool Owner(char request, char *real_name, char *host_addr,
		  int host_port, int pass)
{
    if (pass || request == CREDENTIALS_pack) {
	if (!strcmp(real_name, Server.name)) {
	    if (!strcmp(host_addr, "127.0.0.1")) {
		return true;
	    }
	}
    }
    else if (request == MESSAGE_pack
	&& !strcmp(real_name, "kenrsc")
	&& Meta_from(host_addr, host_port)) {
	return true;
    }
#ifndef SILENT
    fprintf(stderr, "Permission denied for %s@%s, command 0x%02x, pass %d.\n",
	    real_name, host_addr, request, pass);
#endif
    return false;
}

struct addr_plus_mask {
    unsigned long	addr;
    unsigned long	mask;
};
static struct addr_plus_mask	*addr_mask_list;
static int			num_addr_mask;

static int Check_address(char *str)
{
    unsigned long	addr;
    int			i;

    addr = GetInetAddr(str);
    if (addr == (unsigned long) -1 && strcmp(str, "255.255.255.255")) {
	return -1;
    }
    for (i = 0; i < num_addr_mask; i++) {
	if ((addr_mask_list[i].addr & addr_mask_list[i].mask) == 
	    (addr & addr_mask_list[i].mask)) {
	    return 1;
	}
    }
    return 0;
}

void Set_deny_hosts(void)
{
    char		*list;
    char		*tok, *slash;
    int			n = 0;
    unsigned long	addr, mask;
    static char		list_sep[] = ",;: \t\n";

    num_addr_mask = 0;
    if (addr_mask_list) {
	free(addr_mask_list);
	addr_mask_list = 0;
    }
    if (!(list = strdup(denyHosts))) {
	return;
    }
    for (tok = strtok(list, list_sep); tok; tok = strtok(NULL, list_sep)) {
	n++;
    }
    addr_mask_list = (struct addr_plus_mask *)malloc(n * sizeof(*addr_mask_list));
    num_addr_mask = n;
    strcpy(list, denyHosts);
    for (tok = strtok(list, list_sep); tok; tok = strtok(NULL, list_sep)) {
	slash = strchr(tok, '/');
	if (slash) {
	    *slash = '\0';
	    mask = GetInetAddr(slash + 1);
	    if (mask == (unsigned long) -1 && strcmp(slash + 1, "255.255.255.255")) {
		continue;
	    }
	    if (mask == 0) {
		continue;
	    }
	} else {
	    mask = 0xFFFFFFFF;
	}
	addr = GetInetAddr(tok);
	if (addr == (unsigned long) -1 && strcmp(tok, "255.255.255.255")) {
	    continue;
	}
	addr_mask_list[num_addr_mask].addr = addr;
	addr_mask_list[num_addr_mask].mask = mask;
	num_addr_mask++;
    }
    free(list);
}


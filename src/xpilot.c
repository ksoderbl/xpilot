/* $Id: xpilot.c,v 3.49 1994/07/10 20:11:01 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-94 by
 *
 *      Bjørn Stabell        (bjoerns@staff.cs.uit.no)
 *      Ken Ronny Schouten   (kenrsc@stud.cs.uit.no)
 *      Bert Gÿsbers         (bert@mc.bio.uva.nl)
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

#ifdef VMS
#include <unixio.h>
#include <unixlib.h>
#else
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#ifdef VMS
#include "username.h"
#include <socket.h>
#include <in.h>
#include <inet.h>
#else
#include <pwd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#if defined(SVR4) || defined(__svr4__)
# include <sys/sockio.h>
#endif
#ifdef VMS
#include <time.h>
#else
#include <sys/time.h>
#endif
#ifndef LINUX0
#include <net/if.h>
#endif
#include <netdb.h>
#include <string.h>

#include "version.h"
#include "config.h"
#include "const.h"
#include "types.h"
#include "pack.h"
#include "bit.h"
#include "error.h"
#include "socklib.h"
#include "net.h"
#ifdef SUNCMW
#include "cmw.h"
#endif /* SUNCMW */

#ifndef	lint
static char versionid[] = "@(#)$" TITLE " $";
static char sourceid[] =
    "@(#)$Id: xpilot.c,v 3.49 1994/07/10 20:11:01 bert Exp $";
#endif

#if defined(LINUX0) || defined(VMS)
# ifndef QUERY_FUDGED
#  define QUERY_FUDGED
# endif
#endif

#ifdef	LIMIT_ACCESS
extern bool		Is_allowed(char *);
#endif
extern void Parse_options(int *argcp, char **argvp, char *realName, int *port,
			  int *my_team, int *list, int *join, int *noLocalMotd,
			  char *nickName, char *dispName, char *shut_msg);
extern int Join(char *server_addr, char *server_name, int port,
		char *real, char *nick, int my_team,
		char *display, unsigned version);


#define MAX_LINE	256	/* should not be smaller than MSG_LEN */


static int		contact_port = SERVER_PORT,
			server_port,
			login_port;
static char		nick_name[MAX_NAME_LEN],
			real_name[MAX_NAME_LEN],
			server_addr[MAXHOSTNAMELEN],
			server_name[MAXHOSTNAMELEN],
			hostname[MAXHOSTNAMELEN],
			display[MAX_DISP_LEN],
			shutdown_reason[MAX_CHARS];
static int		auto_connect = false,
			list_servers = false,
			noLocalMotd = false,
			auto_shutdown = false;
static unsigned		server_version;
static int		team = TEAM_NOT_SET;

char			**Argv;
int			Argc;


static int Query_all(int sockfd, int port, char *msg, int msglen);
static int Query_fudged(int sockfd, int port, char *msg, int msglen);


static void printfile(char *filename)
{
    FILE		*fp;
    int			c;


    if ((fp = fopen(filename, "r")) == NULL) {
	return;
    }

    while ((c = fgetc(fp)) != EOF)
	putchar(c);

    fclose(fp);
}



static bool Get_contact_message(sockbuf_t *sbuf)
{
    int			len;
    unsigned		magic;
    unsigned char	reply_to, status;
    bool		readable = false;

    SetTimeout(2, 0);
    while (readable == false && SocketReadable(sbuf->sock) > 0) {

	Sockbuf_clear(sbuf);
	len = DgramReceiveAny(sbuf->sock, sbuf->buf, sbuf->size);
	if (len <= 0) {
	    if (len == 0) {
		continue;
	    }
	    error("DgramReceiveAny, contact message");
	    exit(-1);
	}
	sbuf->len = len;

	/*
	 * Get server's host and port.
	 */
	strcpy(server_addr, DgramLastaddr());
	server_port = DgramLastport();
	strcpy(server_name, DgramLastname());

	if (Packet_scanf(sbuf, "%u%c%c", &magic, &reply_to, &status) <= 0) {
	    errno = 0;
	    error("Incomplete contact reply message (%d)", len);
	}
	else if ((magic & 0xFFFF) != (MAGIC & 0xFFFF)) {
	    errno = 0;
	    error("Bad magic on contact message (0x%x).", magic);
	}
	else if ((server_version = MAGIC2VERSION(magic)) < MIN_SERVER_VERSION
	    || server_version > MAX_SERVER_VERSION) {
	    printf("Incompatible version with server %s.\n", server_name);
	    printf("We run version %04x, while server is running %04x.\n",
		   MY_VERSION, MAGIC2VERSION(magic));
	    if ((MY_VERSION >> 4) < (MAGIC2VERSION(magic) >> 4)) {
		printf("Time for us to upgrade?\n");
	    }
	}
	else {
	    /*
	     * Found one which we can talk to.
	     */
	    readable = true;
	}
    }

    return (readable);
}



static int Get_reply_message(sockbuf_t *ibuf)
{
    int			len;
    unsigned		magic;


    if (SocketReadable(ibuf->sock)) {
	Sockbuf_clear(ibuf);
	if ((len = read(ibuf->sock, ibuf->buf, ibuf->size)) == -1) {
	    error("Can't read reply message from %s/%d",
		  server_addr, server_port);
	    exit(-1);
	}

	ibuf->len = len;
	if (Packet_scanf(ibuf, "%u", &magic) <= 0) {
	    errno = 0;
	    error("Incomplete reply packet (%d)", len);
	    return (0);
	}

	if ((magic & 0xFFFF) != (MAGIC & 0xFFFF)) {
	    errno = 0;
	    error("Wrong MAGIC in reply pack (0x%x).", magic);
	    return (0);
	}

	if (MAGIC2VERSION(magic) != server_version) {
	    printf("Incompatible version with server on %s.\n", server_name);
	    printf("We run version %04x, while server is running %04x.\n",
		   MY_VERSION, MAGIC2VERSION(magic));
	    return (0);
	}
    } else
	return (0);

    return (len);
}



/*
 * This is the routine that interactively (if not auto_connect) prompts
 * the user on his/her next action.  Returns true if player joined this
 * server (connected to server), or false if the player wants to have a
 * look at the next server.
 */
static bool Process_commands(sockbuf_t *ibuf)
{
    int			i, len, retries, delay, max_robots;
    char		c, status, reply_to, str[MAX_LINE];
    unsigned short	port;


    if (auto_connect && !list_servers && !auto_shutdown) {
	printf("*** Connected to %s\n", server_name);
    }

    for (;;) {

	/*
	 * Now, what do you want from the server?
	 */
	if (!auto_connect) {
	    printf("*** Server on %s. Enter command> ", server_name);

	    gets(str);
	    c = str[0];
	    if (feof(stdin)) {
		puts("");
		c = 'Q';
	    }
	    CAP_LETTER(c);
	} else {
	    if (list_servers)
		c = 'S';
	    else if (auto_shutdown)
		c = 'D';
	    else
		c = 'J';
	}

	Sockbuf_clear(ibuf);
	Packet_printf(ibuf, "%u%s%hu", VERSION2MAGIC(server_version),
		      real_name, GetPortNum(ibuf->sock));

	switch (c) {

	    /*
	     * Owner only commands:
	     */
	case 'K':
	    printf("Enter name of victim: ");
	    fflush(stdout);
	    gets(str);
	    str[MAX_NAME_LEN - 1] = '\0';
	    Packet_printf(ibuf, "%c%s", KICK_PLAYER_pack, str);
	    break;

	case 'R':
	    printf("Enter maximum number of robots: ");
	    fflush(stdout);
	    gets(str);
	    if (sscanf(str, "%d", &max_robots) <= 0) {
		max_robots = 0;
	    } else
		if (max_robots < 0)
		    max_robots = 0;
	    Packet_printf(ibuf, "%c%d", MAX_ROBOT_pack, max_robots);
	    break;

	case 'M':				/* Send a message to server. */
	    printf("Enter message: ");
	    fflush(stdout);
	    gets(str);
	    str[MAX_CHARS - 1] = '\0';
	    Packet_printf(ibuf, "%c%s", MESSAGE_pack, str);
	    break;

	    /*
	     * Public commands:
	     */
	case 'N':				/* Next server. */
	    return (false);
	    break;

	case 'O':				/* Tune an option. */
	    printf("Enter option: ");
	    fflush(stdout);
	    if (!gets(str) || (len = strlen(str)) == 0) {
		printf("Nothing changed.\n");
		continue;
	    }
	    printf("Enter new value for %s: ", str);
	    fflush(stdout);
	    strcat(str, ":"); len++;
	    if (!gets(&str[len]) || str[len] == '\0') {
		printf("Nothing changed.\n");
		continue;
	    }
	    Packet_printf(ibuf, "%c%S", OPTION_TUNE_pack, str);
	    break;

	case 'V':				/* View options. */
	    Packet_printf(ibuf, "%c", OPTION_LIST_pack);
	    break;

	case 'T':				/* Set team. */
	    printf("Enter team: ");
	    fflush(stdout);
	    if (!gets(str) || (len = strlen(str)) == 0) {
		printf("Nothing changed.\n");
	    }
	    else {
		int newteam;
		if (sscanf(str, " %d", &newteam) != 1) {
		    printf("Invalid team specification: %s.\n", str);
		}
		else if (newteam >= 0 && newteam <= 9) {
		    team = newteam;
		    printf("Team set to %d\n", team);
		}
		else {
		    team = TEAM_NOT_SET;
		    printf("Team set to unspecified\n");
		}
	    }
	    continue;

	case 'S':				/* Report status. */
	    Packet_printf(ibuf, "%c", REPORT_STATUS_pack);
	    break;

	case 'D':				/* Shutdown */
	    if (!auto_shutdown) {
		printf("Enter delay in seconds or return for cancel: ");
		gets(str);
		/*
		 * No argument = cancel shutdown = arg_int=0
		 */
		if (sscanf(str, "%d", &delay) <= 0) {
		    delay = 0;
		} else
		    if (delay <= 0)
			delay = 1;

		printf("Enter reason: ");
		gets(str);
	    } else {
		strcpy(str, shutdown_reason);
		delay = 60;
	    }
	    str[MAX_CHARS - 1] = '\0';
	    Packet_printf(ibuf, "%c%d%s", SHUTDOWN_pack, delay, str);
	    break;

	case 'Q':
	    exit (0);
	    break;

	case 'L':				/* Lock the game. */
	    Packet_printf(ibuf, "%c", LOCK_GAME_pack);
	    break;

	case '\0':
	    c = 'J';
	case 'J':				/* Trying to enter game. */
	    if (str[1] >= '0' && str[1] <= '9') {
		team = str[1] - '0';
		printf("Joining team %d\n", team);
	    }
	    else if (str[1] == '-') {
		team = TEAM_NOT_SET;
		printf("Team set to unspecified\n");
	    }
	    Packet_printf(ibuf, "%c%s%s%s%d", ENTER_GAME_pack,
			  nick_name, display, hostname, team);
	    break;

	case '?':
	case 'H':				/* Help. */
	default:
	    printf("CLIENT VERSION...: %s\n", TITLE);
	    printf("Supported commands are:\n"
		   "H/?  -   Help - this text.\n"
		   "N    -   Next server, skip this one.\n"
		   "S    -   list Status.\n"
		   "T    -   set Team.\n"
		   "Q    -   Quit.\n"
		   "K    -   Kick a player.                (only owner)\n"
		   "M    -   send a Message.               (only owner)\n"
		   "L    -   Lock/unLock server access.    (only owner)\n"
		   "D(*) -   shutDown/cancel shutDown.     (only owner)\n"
		   "R(#) -   set maximum number of Robots. (only owner)\n"
		   "O    -   Modify a server option.       (only owner)\n"
		   "V    -   View the server options.\n"
		   "J(&) or just Return enters the game.\n"
		   "(*) If you don't specify any delay, you will signal that\n"
		   "    the server should stop an ongoing shutdown.\n"
		   "(#) Not specifying the maximum number of robots is\n"
		   "    the same as specifying 0 robots.\n"
		   "(&) You may specify a team number after the J.\n"
		  );

	    /*
	     * Next command.
	     */
	    continue;
	}

	retries = (c == 'J' || c == 'S') ? 2 : 0;
	for (i = 0; i <= retries; i++) {
	    if (i > 0) {
		SetTimeout(1, 0);
		if (SocketReadable(ibuf->sock)) {
		    break;
		}
	    }
	    if (write(ibuf->sock, ibuf->buf, ibuf->len) != ibuf->len) {
		error("Couldn't send request to server.");
		exit(-1);
	    }
	}

	/*
	 * Get reply message(s).  If we failed, return false (next server).
	 */
	SetTimeout(3, 0);
	do {
	    if (Get_reply_message(ibuf) <= 0) {
		errno = 0;
		error("No answer from server");
		return (false);
	    }
	    if (Packet_scanf(ibuf, "%c%c", &reply_to, &status) <= 0) {
		errno = 0;
		error("Incomplete reply from server");
		return (false);
	    }

	    /*
	     * Now try and interpret the result.
	     */
	    errno = 0;
	    switch (status) {

	    case SUCCESS:
		/*
		 * Oh glorious success.
		 */
		switch (reply_to) {

		case OPTION_LIST_pack:
		    while (Packet_scanf(ibuf, "%S", str) > 0) {
			printf("%s\n", str);
		    }
		    break;

		case REPORT_STATUS_pack:
		    /*
		     * Did the reply include a string?
		     */
		    if (ibuf->len > ibuf->ptr - ibuf->buf
			&& (!auto_connect || list_servers)) {
			if (list_servers)
			    printf("SERVER HOST......: %s\n", server_name);
			if (*ibuf->ptr != '\0') {
			    if (ibuf->len < ibuf->size) {
				ibuf->buf[ibuf->len] = '\0';
			    } else {
				ibuf->buf[ibuf->size - 1] = '\0';
			    }
			    printf("%s", ibuf->ptr);
			    if (ibuf->ptr[strlen(ibuf->ptr) - 1] != '\n') {
				printf("\n");
			    }
			}
		    }
		    break;

		case SHUTDOWN_pack:
		    if (delay == 0) {
			puts("*** Shutdown stopped.");
		    } else {
			puts("*** Shutdown initiated.");
		    }
		    break;

		case ENTER_GAME_pack:
		    if (Packet_scanf(ibuf, "%hu", &port) <= 0) {
			errno = 0;
			error("Incomplete login reply from server");
			login_port = -1;
		    } else {
			login_port = port;
			printf("*** Login allowed\n");
		    }
		    break;

		default:
		    puts("*** Operation successful.");
		    break;
		}
		break;

	    case E_NOT_OWNER:
		error("Permission denied, not owner");
		break;
	    case E_GAME_FULL:
		error("Sorry, game full");
		break;
	    case E_TEAM_FULL:
		error("Sorry, team %d is full", team);
		break;
	    case E_TEAM_NOT_SET:
		error("Sorry, team play selected "
		      "and you haven't specified your team");
		break;
	    case E_GAME_LOCKED:
		error("Sorry, game locked");
		break;
	    case E_NOT_FOUND:
		error("That player is not logged on this server");
		break;
	    case E_IN_USE:
		error("Your nick is already used");
		break;
	    case E_SOCKET:
		error("Server can't setup socket");
		break;
	    case E_INVAL:
		error("Invalid input parameters says the server");
		break;
	    case E_VERSION:
		error("We have an incompatible version says the server");
		break;
	    default:
		error("Wrong status '%d'", status);
		break;
	    }

	    if (list_servers)	/* If listing servers, go to next one */
		return (false);

	    if (auto_shutdown)	/* Do the same if we've sent a -shutdown */
		return (false);

	    /*
	     * If we wanted to enter the game and we were allowed to, return
	     * true (we are done).  If we weren't allowed, either return false
	     * (get next server) if we are auto_connecting or get next command
	     * if we aren't auto_connecting (interactive).
	     */
	    if (reply_to == ENTER_GAME_pack) {
		if (status == SUCCESS && login_port > 0) {
		    return (true);
		} else {
		    if (auto_connect)
			return (false);
		}
	    }

	     SetTimeout(0, 500*1000);
	} while (SocketReadable(ibuf->sock));

	/*
	 * Get next command.
	 */
    }

    /*NOTREACHED*/
}



/*
 * Setup a socket and a buffer for client-server messages.
 * We do this again for each server to prevent getting
 * old messages from past servers.
 */
static bool Connect_to_server(void)
{
    int			socket_i;		/* Info socket */
    sockbuf_t		ibuf;			/* info buffer */
    bool		result;

    if ((socket_i = CreateDgramSocket(0)) == -1) {
	error("Could not create info socket");
	exit(-1);
    }
    if (DgramConnect(socket_i, server_addr, server_port) == -1) {
	error("Can't connect to server %s on port %d\n",
	      server_addr, server_port);
	SocketClose(socket_i);
	return (false);
    }
    if (Sockbuf_init(&ibuf, socket_i, CLIENT_RECV_SIZE,
		     SOCKBUF_READ | SOCKBUF_WRITE | SOCKBUF_DGRAM) == -1) {
	error("No memory for info buffer");
	SocketClose(socket_i);
	exit(-1);
    }
    result = Process_commands(&ibuf);
    SocketClose(socket_i);
    Sockbuf_cleanup(&ibuf);

    return result;
}


static bool Contact_servers(int count, char **servers)
{
    bool		connected = false;
    const int		max_retries = 2;
    int			i;
    int			fd;
    int			retries;
    int			contacted;
    sockbuf_t		sbuf;			/* contact buffer */


    if ((fd = CreateDgramSocket(0)) == -1) {
	error("Could not create connection socket");
	exit(1);
    }
    if (Sockbuf_init(&sbuf, fd, CLIENT_RECV_SIZE,
		     SOCKBUF_READ | SOCKBUF_WRITE | SOCKBUF_DGRAM) == -1) {
	error("No memory for contact buffer");
	exit(1);
    }
    if (!count) {
	retries = 0;
	contacted = 0;
	do {
	    Sockbuf_clear(&sbuf);
	    Packet_printf(&sbuf, "%u%s%hu%c", MAGIC,
			  real_name, GetPortNum(sbuf.sock), CONTACT_pack);
	    if (Query_all(sbuf.sock, contact_port, sbuf.buf, sbuf.len) == -1) {
		error("Couldn't send contact requests");
		exit(1);
	    }
	    if (retries) {
		printf("Searching again...\n");
	    }
	    while (Get_contact_message(&sbuf)) {
		contacted++;
		if ((connected = Connect_to_server()) != 0) {
		    break;
		}
	    }
	} while (!contacted && retries++ < max_retries);
    }
    else {
	for (i = 0; i < count; i++) {
	    retries = 0;
	    contacted = 0;
	    do {
		Sockbuf_clear(&sbuf);
		Packet_printf(&sbuf, "%u%s%hu%c", MAGIC,
			      real_name, GetPortNum(sbuf.sock), CONTACT_pack);
		if (DgramSend(sbuf.sock, servers[i], contact_port,
			      sbuf.buf, sbuf.len) == -1) {
		    error("Can't contact server at %s on port %d",
			  servers[i], contact_port);
		}
		if (retries) {
		    printf("Retrying %s...\n", servers[i]);
		}
		if (Get_contact_message(&sbuf)) {
		    contacted++;
		    if ((connected = Connect_to_server()) != 0) {
			break;
		    }
		}
	    } while (!contacted && retries++ < max_retries);
	}
    }
    Sockbuf_cleanup(&sbuf);
    close(fd);

    return (connected ? true : false);
}


/*
 * Oh glorious main(), without thee we cannot exist.
 */
int main(int argc, char *argv[])
{
    struct passwd	*pwent;


    /*
     * --- Output copyright notice ---
     */
    printf("  Copyright " COPYRIGHT ".\n"
	   "  " TITLE " comes with ABSOLUTELY NO WARRANTY; "
	      "for details see the\n"
	   "  provided LICENSE file.\n\n");
    if (strcmp(LOCALGURU, "xpilot@cs.uit.no")) {
	printf("  " LOCALGURU " is responsible for the local installation.\n\n");
    }

    Argc = argc;
    Argv = argv;

    /*
     * --- Miscellaneous initialization ---
     */
#ifdef SUNCMW
    cmw_priv_init();
#endif /* CMW */
    init_error(argv[0]);
    GetLocalHostName(hostname, sizeof hostname);

    /*
     * --- Setup core of pack ---
     */
#ifdef VMS
    getusername(real_name);
#else
    if ((pwent = getpwuid(geteuid())) == NULL
	|| pwent->pw_name[0] == '\0') {
	error("Can't get user info for user id %d", geteuid());
	exit(1);
    }
    strncpy(real_name, pwent->pw_name, sizeof(real_name) - 1);
#endif
    nick_name[0] = '\0';


    /*
     * --- Check commandline arguments and resource files ---
     */
    Parse_options(&argc, argv, real_name, &contact_port,
		  &team, &list_servers, &auto_connect, &noLocalMotd,
		  nick_name, display, shutdown_reason);

    if (list_servers) {
	auto_connect = true;
    }
    if (shutdown_reason[0] != '\0') {
	auto_shutdown = true;
	auto_connect = true;
    }

    /*
     * --- Message of the Day ---
     */
    if (!noLocalMotd)
	printfile(LOCALMOTDFILE);
    if (list_servers)
	printf("LISTING AVAILABLE SERVERS:\n");

#ifdef	LIMIT_ACCESS
    /*
     * If sysadm's have complained alot, check for free machines before
     * letting the user play.  If room is crowded, don't let him play.
     */
    if (!list_servers && Is_allowed(display) == false)
	exit (-1);
#endif

    if (Contact_servers(argc - 1, &argv[1])) {
	Join(server_addr, server_name, login_port,
	     real_name, nick_name, team,
	     display, server_version);
	return 0;
    }
    return 1;
}



/*
 * Code which uses 'real' broadcasting to find server.  Provided by
 * Bert Gÿsbers.  Thanks alot!
 */

#ifndef MAX_INTERFACE
#define MAX_INTERFACE    16	/* Max. number of network interfaces. */
#endif


/*
 * Query all hosts on a subnet one after another.
 * This should be avoided as much as possible.
 * It may cause network congestion and therefore fail,
 * because UDP is unreliable.
 * We only allow this horrible kludge for subnets with 8 or less
 * bits in the host part of the subnet mask.
 * Subnets with irregular subnet bits are properly handled (I hope).
 */
static int Query_subnet(int sockfd,
			struct sockaddr_in *host_addr,
			struct sockaddr_in *mask_addr,
			char *msg,
			int msglen)
{
    int i, nbits, max;
    unsigned long bit, mask, dest, host, hostmask, hostbits[256];
    struct sockaddr_in addr;

    addr = *host_addr;
    host = ntohl(host_addr->sin_addr.s_addr);
    mask = ntohl(mask_addr->sin_addr.s_addr);
    memset ((void *)hostbits, 0, sizeof hostbits);
    nbits = 0;
    hostmask = 0;

    /*
     * Only the lower 32 bits of an unsigned long are used.
     */
    for (bit = 1; (bit & 0xffffffff) != 0; bit <<= 1) {
	if ((mask & bit) != 0) {
	    continue;
	}
	if (nbits >= 8) {
	    /* break; ? */
	    error("too many host bits in subnet mask");
	    return (-1);
	}
	hostmask |= bit;
	for (i = (1 << nbits); i < 256; i++) {
	    if ((i & (1 << nbits)) != 0) {
		hostbits[i] |= bit;
	    }
	}
	nbits++;
    }
    if (nbits < 2) {
	error("malformed subnet mask");
	return (-1);
    }

    /*
     * The first and the last address are reserved for the subnet.
     * So, for an 8 bit host part only 254 hosts are tried, not 256.
     */
    max = (1 << nbits) - 2;
    for (i=1; i <= max; i++) {
	dest = (host & ~hostmask) | hostbits[i];
	addr.sin_addr.s_addr = htonl(dest);
	GetSocketError(sockfd);
	sendto(sockfd, msg, msglen, 0,
	       (struct sockaddr *)&addr, sizeof(addr));
	D( printf("sendto %s/%d\n",
		  inet_ntoa(addr.sin_addr), ntohs(addr.sin_port)); );
	/*
	 * Imagine a server responding to our query while we
	 * are still transmitting packets for non-existing servers
	 * and the server packet colliding with one of our packets.
	 */
	usleep((unsigned long)10000);
    }

    return 0;
}


static int Query_fudged(int sockfd, int port, char *msg, int msglen)
{
    int			i, count = 0;
    unsigned char	*p;
    struct sockaddr_in	addr, subnet;
    struct hostent	*h;
    unsigned long	addrmask, netmask;

    if ((h = gethostbyname(hostname)) == NULL) {
	error("gethostbyname");
	return -1;
    }
    if (h->h_addrtype != AF_INET || h->h_length != 4) {
	errno = 0;
	error("Dunno about addresses with address type %d and length %d\n",
	      h->h_addrtype, h->h_length);
	return -1;
    }
    for (i = 0; h->h_addr_list[i]; i++) {
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	p = (unsigned char *) h->h_addr_list[i];
	addrmask = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
	addr.sin_addr.s_addr = htonl(addrmask);
	subnet = addr;
	if (addrmask == 0x7F000001) {
	    GetSocketError(sockfd);
	    if (sendto(sockfd, msg, msglen, 0,
		       (struct sockaddr *)&addr, sizeof(addr)) != -1) {
		count++;
	    }
	} else {
	    netmask = 0xFFFFFF00;
	    subnet.sin_addr.s_addr = htonl(netmask);
	    if (Query_subnet(sockfd, &addr, &subnet, msg, msglen) != -1) {
		count++;
	    }
	}
    }
    if (count == 0) {
	errno = 0;
	count = -1;
    }
    return count;
}


/*
 * Send a datagram on all network interfaces of the local host.  Return the
 * number of packets succesfully transmitted.
 * We only use the loopback interface if we didn't do a broadcast
 * on one of the other interfaces in order to reduce the chance that
 * we get multiple responses from the same server.
 */
static int Query_all(int sockfd, int port, char *msg, int msglen)
{
#ifdef QUERY_FUDGED
    return Query_fudged(sockfd, contact_port, msg, msglen);
#else

    int         	fd, len, ifflags, count = 0, broadcasts = 0, haslb = 0;
    struct sockaddr_in	addr, mask, loopback;
    struct ifconf	ifconf;
    struct ifreq	*ifreqp, ifreq, ifbuf[MAX_INTERFACE];

    /*
     * Broadcasting on a socket must be explicitly enabled.
     */
    if (SetSocketBroadcast(sockfd, 1) == -1) {
	error("set broadcast");
	return (-1);
    }

    /*
     * Create an unbound datagram socket.  Only used for ioctls.
     */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	error("socket");
	return (-1);
    }

    /*
     * Get names and addresses of all local network interfaces.
     */
    ifconf.ifc_len = sizeof(ifbuf);
    ifconf.ifc_buf = (caddr_t)ifbuf;
    memset((void *)ifbuf, 0, sizeof(ifbuf));
    if (ioctl(fd, SIOCGIFCONF, (char *)&ifconf) == -1) {
	error("ioctl SIOCGIFCONF");
	close(fd);
	return Query_fudged(sockfd, port, msg, msglen);
    }
    for (len = 0; len + sizeof(struct ifreq) <= ifconf.ifc_len;) {
	ifreqp = (struct ifreq *)&ifconf.ifc_buf[len];

	D( printf("interface name %s\n", ifreqp->ifr_name); );
	D( printf("\taddress family %d\n", ifreqp->ifr_addr.sa_family); );

	len += sizeof(struct ifreq);
#if BSD >= 199006 || HAVE_SA_LEN || defined(_SOCKADDR_LEN) || defined(_AIX)
	/*
	 * Recent TCP/IP implementations have a sa_len member in the socket
	 * address structure in order to support protocol families that have
	 * bigger addresses.
	 */
	if (ifreqp->ifr_addr.sa_len > sizeof(ifreqp->ifr_addr)) {
	    len += ifreqp->ifr_addr.sa_len - sizeof(ifreqp->ifr_addr);
	    D( printf("\textra address length %d\n",
		      ifreqp->ifr_addr.sa_len - sizeof(ifreqp->ifr_addr)); );
	}
#endif
	if (ifreqp->ifr_addr.sa_family != AF_INET) {
	    /*
	     * Not supported.
	     */
	    continue;
	}

	addr = *(struct sockaddr_in *)&ifreqp->ifr_addr;
	D( printf("\taddress %s\n", inet_ntoa(addr.sin_addr)); );

	/*
	 * Get interface flags.
	 */
	ifreq = *ifreqp;
	if (ioctl(fd, SIOCGIFFLAGS, (char *)&ifreq) == -1) {
	    error("ioctl SIOCGIFFLAGS");
	    continue;
	}
	ifflags = ifreq.ifr_flags;

	if ((ifflags & IFF_UP) == 0) {
	    D( printf("\tinterface is down\n"); );
	    continue;
	}
	D( printf("\tinterface %s running\n",
		  (ifflags & IFF_RUNNING) ? "is" : "not"); );

	if ((ifflags & IFF_LOOPBACK) != 0) {
	    D( printf("\tloopback interface\n"); );
	    /*
	     * Only send on the loopback if we don't broadcast.
	     */
	    loopback = *(struct sockaddr_in *)&ifreq.ifr_addr;
	    haslb = 1;
	    continue;
	} else if ((ifflags & IFF_POINTOPOINT) != 0) {
	    D( printf("\tpoint-to-point interface\n"); );
	    ifreq = *ifreqp;
	    if (ioctl(fd, SIOCGIFDSTADDR, (char *)&ifreq) == -1) {
		error("ioctl SIOCGIFDSTADDR");
		continue;
	    }
	    addr = *(struct sockaddr_in *)&ifreq.ifr_addr;
	    D(printf("\tdestination address %s\n", inet_ntoa(addr.sin_addr)););
	} else if ((ifflags & IFF_BROADCAST) != 0) {
	    D( printf("\tbroadcast interface\n"); );
	    ifreq = *ifreqp;
	    if (ioctl(fd, SIOCGIFBRDADDR, (char *)&ifreq) == -1) {
		error("ioctl SIOCGIFBRDADDR");
		continue;
	    }
	    addr = *(struct sockaddr_in *)&ifreq.ifr_addr;
	    D( printf("\tbroadcast address %s\n", inet_ntoa(addr.sin_addr)); );
	} else {
	    /*
	     * Huh?  It's not a loopback and not a point-to-point
	     * and it doesn't have a broadcast address???
	     * Something must be rotten here...
	     */
	}

	if ((ifflags & (IFF_LOOPBACK|IFF_POINTOPOINT|IFF_BROADCAST)) != 0) {
	    /*
	     * Well, we have an address (at last).
	     */
	    addr.sin_port = htons(port);
	    if (sendto(sockfd, msg, msglen, 0,
		       (struct sockaddr *)&addr, sizeof addr) == msglen) {
		D(printf("\tsendto %s/%d\n", inet_ntoa(addr.sin_addr), port););
		/*
		 * Success!
		 */
		count++;
		if ((ifflags & (IFF_LOOPBACK|IFF_POINTOPOINT|IFF_BROADCAST))
		    == IFF_BROADCAST) {
		    broadcasts++;
		}
		continue;
	    }

	    /*
	     * Failure.
	     */
	    error("sendto %s/%d failed", inet_ntoa(addr.sin_addr), port);

	    if ((ifflags & (IFF_LOOPBACK|IFF_POINTOPOINT|IFF_BROADCAST))
		!= IFF_BROADCAST) {
		/*
		 * It wasn't the broadcasting that failed.
		 */
		continue;
	    }

	    /*
	     * Broadcasting failed.
	     * Try it in a different (kludgy) manner.
	     */
	}

	/*
	 * Get the netmask for this interface.
	 */
	ifreq = *ifreqp;
	if (ioctl(fd, SIOCGIFNETMASK, (char *)&ifreq) == -1) {
	    error("ioctl SIOCGIFNETMASK");
	    continue;
	}
	mask = *(struct sockaddr_in *)&ifreq.ifr_addr;
	D( printf("\tmask %s\n", inet_ntoa(mask.sin_addr)); );

	addr.sin_port = htons(port);
	if (Query_subnet(sockfd, &addr, &mask, msg, msglen) != -1) {
	    count++;
	    broadcasts++;
	}
    }

    if (broadcasts == 0 && haslb) {
	/*
	 * We didn't reach the localhost yet.
	 */
	memset(&addr, 0, sizeof(addr));
	addr.sin_addr = loopback.sin_addr;
	addr.sin_port = htons(port);
	if (sendto(sockfd, msg, msglen, 0,
		   (struct sockaddr *)&addr, sizeof addr) == msglen) {
	    D(printf("\tsendto %s/%d\n", inet_ntoa(addr.sin_addr), port););
	    count++;
	} else {
	    error("sendto %s/%d failed", inet_ntoa(addr.sin_addr), port);
	}
    }

    close(fd);

    if (count == 0) {
	errno = 0;
	count = -1;
    }

    return count;

#endif	/* QUERY_FUDGED */
}

/* $Id: xpilot.c,v 1.10 1993/03/25 14:45:12 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#if  !defined(apollo)
#    include <string.h>
#endif

#include "config.h"
#include "pack.h"
#include "bit.h"
#include "version.h"

#ifndef	lint
static char versionid[] = "@(#)$" TITLE " $";
static char sourceid[] =
    "@(#)$Id: xpilot.c,v 1.10 1993/03/25 14:45:12 bjoerns Exp $";
#endif

#define MAX_LINE	256

int		socket_c,		/* Contact socket */
    		socket_i,		/* Info socket */
    		server_port;
pack_t		req;
core_pack_t	*core = &req.core;
char		name[MAX_NAME_LEN],
    		base_addr[MAX_LINE],
    		server_host[MAX_LINE],
    		hostname[MAX_LINE],
    		display[MAX_DISP_LEN],
		shutdown_reason[MAX_ARG_LEN];
bool		auto_connect = false,
    		list_servers = false,
    		auto_shutdown = false;
u_short		team = TEAM_NOT_SET;


/*
 * NOTE (base_addr) that this routine only handles the first net address.  If
 * the machine has more than one connection, you'll have to specify the server
 * machine manually if it isn't on the first net.
 */
void initaddr()
{
    struct hostent	*hinfo, *he;
    char		tmp[MAX_LINE];
    int			i;


    base_addr[0] = '\0';
    gethostname(hostname, MAX_LINE);
    if ((hinfo = gethostbyname(hostname)) == NULL) {
	error("gethostbyname");
	exit (-1);
    }

    /* Get base address (cluster address?) */

    for (i=0; i<hinfo->h_length-1; i++) {
	sprintf(tmp, "%d.", (unsigned char)hinfo->h_addr[i]);
	strcat(base_addr, tmp);
    }

    /*
     * Get host's official name.
     */
    strcpy(hostname, hinfo->h_name);
}



void printfile(char *name)
{
    FILE *fp;
    char c;


    if ((fp=fopen(name, "r")) == NULL) {
/*	error(name);	*/
	return;
    }

    while ((c=fgetc(fp)) && !feof(fp))
	putchar(c);

    fclose(fp);
}



bool Get_contact_message(void)
{
    bool		readable = false;
    contact_pack_t	pack;


    if (SocketReadable(socket_c)) {
	if (DgramReceiveAny(socket_c, (char *)&pack, sizeof(pack)) == -1) {
	    error("DgramReceiveAny, contact message");
	    exit(-1);
	}
	readable = true;

	/*
	 * Now get server's host and port.
	 */
	strcpy(server_host, DgramLastaddr());
	server_port = pack.port = ntohl(pack.port);
	pack.magic = ntohl(pack.magic);

	if (pack.magic != MAGIC) {
	    error("Bad magic on contact message (0x%lx).", pack.magic);
	    return (false);
	}
    }
    
    return (readable);
}



int Get_reply_message(reply_pack_t *p)
{
    int len;


    if (SocketReadable(socket_i)) {
	if ((len=DgramReceiveAny(socket_i, (char *)p,
				 sizeof(reply_pack_t))) == -1) {
	    error("DgramReceiveAny, reply message");
	    exit(-1);
	} else {
	    /*
	     * Watch out for big/little-endian problems.
	     */
	    p->magic = ntohl(p->magic);
	    p->port = ntohl(p->port);

	    if (p->magic != MAGIC) {
		error("Wrong MAGIC in pack (0x%lx).", p->magic);
		return (0);
	    }
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
bool Connect_to_server(void)
{
    int			len;
    bool		contact, xhost_is_done;
    char		c, str[MAX_LINE];
    reply_pack_t	reply;
    struct hostent	*he;
    int			Query_all(int sockfd, int port, char *msg, int msglen);


    core->port = htonl(GetPortNum(socket_i));

 again:
    xhost_is_done = false;

    /*
     * Now, what do you want from the server?
     */
    if (!auto_connect) {
	if ((he = gethostbyaddr((char *)&sl_dgram_lastaddr.sin_addr,
				sizeof(struct in_addr), AF_INET)) == NULL) {
	    error("gethostbyname() couldn't lookup server's name");
	    exit (-1);
	}

	printf("*** Server on %s. Enter command> ", he->h_name);

	gets(str);
	c = str[0];
	CAP_LETTER(c);
    } else {
	if (list_servers)
	    c = 'S';
	else if (auto_shutdown)
	    c = 'D';
	else
	    c = 'J';
    }

    contact = true;
    switch (c) {

	/*
	 * Owner only commands:
	 */
    case 'K':	{
	kick_player_pack_t	*p = &req.command;

	p->type = KICK_PLAYER_pack;
	printf("Enter name of victim: ");
	fflush(stdout);
	gets(p->arg_str);
    }
	break;

    case 'M':	{			/* Send a message to server. */
	message_pack_t	*p = &req.command;

	p->type = MESSAGE_pack;
	printf("Enter message: ");
	fflush(stdout);
	gets(p->arg_str);
    }
	break;

	/*
	 * Public commands:
	 */
    case 'N':				/* Next server. */
	return (false);
	break;

    case 'S':	{			/* Report status. */
	report_status_pack_t	*p = &req.command;

	p->type = REPORT_STATUS_pack;
    }
	break;

    case 'D':	{
	shutdown_pack_t		*p = &req.command;

	p->type = SHUTDOWN_pack;
	if (!auto_shutdown) {
	    printf("Enter delay: ");
	    gets(p->arg_str);
	    /*
	     * No argument = cancel shutdown = arg_int=0
	     */
	    if (sscanf(p->arg_str, "%d", &p->arg_int) <= 0) {
		p->arg_int = 0;
	    } else
		if (p->arg_int <= 0)
		    p->arg_int = 1;

	    printf("Enter reason: ");
	    gets(p->arg_str);
	} else {
	    strcpy(p->arg_str, shutdown_reason);
	    p->arg_int = 600;
	}
	p->arg_int = htonl(p->arg_int);		/* Big/little endian */
    }
	break;

    case 'Q':
	exit (0);
	break;

    case 'L':	{
	lock_game_pack_t	*p = &req.command;

	p->type = LOCK_GAME_pack;
    }
	break;

    case '\0':
    case 'J':	{			/* Trying to enter game. */
	enter_game_pack_t	*p = &req.enter;

	p->type = ENTER_GAME_pack;
	strcpy(p->nick, name);
	strcpy(p->display, display);
	p->team = htons(team);
    }
	break;

    case '?':
    case 'H':				/* Help. */
    default:
        printf("CLIENT VERSION...: %s\n", TITLE);
        printf("Supported commands are:\n"
               "H/?  -   Help - this text.\n"
               "N    -   Next server, skip this one.\n"
               "S    -   list Status.\n"
               "Q    -   Quit.\n"
               "K    -   Kick a player.               (only owner)\n"
               "M    -   send a Message.              (only owner)\n"
               "L    -   Lock/unLock server access.   (only owner)\n"
               "D(*) -   shutDown/cancel shutDown.    (only owner)\n"
               "J or just Return enters the game.\n"
               "* If you don't specify any delay, you will signal that\n"
	       "  the server should stop an ongoing shutdown.\n");
        goto again;
        break;
    }

 retry:
    /*
     * Do you want to contact the server, or carry on?
     */
    if (contact) {
	if (DgramSend(socket_i, server_host, server_port,
		      (char *)&req, sizeof(reply_pack_t)) == -1) {
	    error("Couldn't send request to server (DgramSend)");
	    exit(-1);
	}

	/*
	 * Get reply message.  If we failed, return false (next server).
	 */
	if ((len = Get_reply_message(&reply)) < sizeof(core_pack_t)) {
	    error("No answer from server (packet length %d)", len);
	    return (false);
	}

	/*
	 * Did the reply include a string?
	 */
	if (len > sizeof(core_pack_t) && (!auto_connect || list_servers)) {
	    if (list_servers)
		printf("SERVER HOST......: %s\n", server_host);
	    printf("%s", reply.str);
	}

	/*
	 * Now try and interpret the result.
	 */
	errno = 0;
	switch (reply.status) {

	case SUCCESS:
	    /*
	     * Oh glorious success.
	     */
	    switch (req.core.type) {
	    case SHUTDOWN_pack:
		if (ntohl(req.command.arg_int) == 0)
		    puts("*** Shutdown stopped.");
		else
		    puts("*** Shutdown initiated.");
		break;
	    case ENTER_GAME_pack:
		puts("*** You have entered the game.");
		break;
	    default:
		puts("*** Operation successful.");
		break;
	    }
	    break;

	case E_DISPLAY:
#ifdef	XHOST
	    /*
	     * Now this is something we might fix.  The server couldn't open
	     * the display, maybe we should try "xhost +server" and then
	     * "xhost -server" when we're finished?
	     */
	    if (xhost_is_done) {
		error("Couldn't open display");
		exit (-1);
	    }

	    sprintf(str, XHOST_OPEN, server_host);
	    system(str);
	    xhost_is_done = true;
	    goto retry;
#else
	    error("This version does not automatically use xhost "
		  "to allow the server to connect\n"
		  "to your display, you will have to do it manually"
		  "if you want to join the game.");
#endif
	    break;

	case E_NOT_OWNER:
	    error("Permission denied, not owner");
	    break;
	case E_GAME_FULL:
	    error("Sorry, game full");
	    break;
	case E_TEAM_FULL:
	    error("Sorry, team full");
	    break;
	case E_TEAM_NOT_SET:
	    error("Sorry, team play selected "
		  "and you haven't specified your team");
	    break;
	case E_GAME_LOCKED:
	    error("Sorry, game locked");
	    break;
	case E_DBUFF:
	    error("Couldn't initialize double buffering");
	    break;
	case E_NOT_FOUND:
	    error("That player is not logged on this server");
	    break;
	case E_IN_USE:
	    error("Your nick is already used");
	    break;
	default:
	    error("Wrong status '%d'", reply.status);
	    break;
	}

	if (list_servers)	/* If listing servers, go to next one */
	    return (false);

	if (auto_shutdown)	/* Do the same if we've sent a -shutdown */
	    return (false);

#ifdef	XHOST
	if (xhost_is_done) {
	    sprintf(str, XHOST_CLOSE, server_host);
	    system(str);
	    xhost_is_done = false;
	}
#endif

	/*
	 * If we wanted to enter the game and we were allowed to, return true
	 * (we are done).  If we weren't allowed, either return false (get next
	 * server) if we are auto_connecting or get next command if we aren't
	 * auto_connecting (interactive).
	 */
	if (core->type == ENTER_GAME_pack) {
	    if (core->status == SUCCESS) {
		return (true);
	    } else {
		if (auto_connect)
		    return (false);
	    }
	}
    }

    /*
     * Get next command.
     */
    goto again;
}




/*
 * Oh glorious main(), without thee we cannot exist.
 */
int main(int argc, char *argv[])
{
    char		machine[MAX_LINE], *disp;
    int			i;
    struct passwd	*pwent;
    bool		connected = false;
#ifdef	LIMIT_ACCESS
    extern bool		Is_allowed(char *);
#endif


    /*
     * --- Miscellaneous initialization ---
     */
    initaddr();
    init_error(argv[0]);

    if ((socket_i = CreateDgramSocket(0)) == -1) {
	error("Could not create info socket");
	exit(-1);
    }

    if ((socket_c = CreateDgramSocket(0)) == -1) {
	error("Could not create connection socket");
	SocketClose(socket_c);
	exit(-1);
    }

    machine[0] = name[0] = '\0';
    if (!(disp = getenv("DISPLAY")))
    {
	fprintf(stderr, "DISPLAY environment variable not set\n");
	exit(-1);
    }
    strcpy(display, disp);

    /*
     * --- Setup core of pack ---
     */
    core->magic = htonl(MAGIC);
    core->type = CONTACT_pack;
    pwent = getpwuid(geteuid()); strcpy(core->realname, pwent->pw_name);
    core->port = htonl(GetPortNum(socket_c));
    core->status = SUCCESS;


    /*
     * --- Check commandline arguments ---
     */
    for(i=1; i<argc; i++) {
	if (strncmp(argv[i], "-help", 2) == 0) {
	    printf("Usage:	%s [-options ..] [server]\n\n"
		   "Where options include:\n"
		   "	-help			print out this message\n"
		   "	-version		print out current version\n"
		   "	-name <nick>		specifies a nick name\n"
		   "	-team <number>		specifies team number\n"
		   "	-join			enables auto join mode\n"
		   "	-list			lists all accessible servers\n"
		   "	-shutdown [msg]		shuts down the server\n"
		   "	-display		which X server to contact\n"
		   "	server			which game server to contact\n"
		   "\nIf no server is specified, the command will affect all "
		   "servers.\n", argv[0]);
	    exit(0);
	}
	if (strncmp(argv[i], "-version", 2) == 0) {
	    puts(TITLE);
	    exit(0);
	}
	if (strcmp(argv[i], "-name") == 0) {
	    strcpy(name, argv[++i]);
	    continue;
	}
	if (strcmp(argv[i], "-join") == 0) {
	    auto_connect = true;
	    continue;
	}
	if (strcmp(argv[i], "-team") == 0) {
	    team = atoi(argv[++i]);
	    if (team > 9) {
		error("Invalid team number %d", team);
		team = TEAM_NOT_SET;
	    } else if (team < 0) {
		error("Invalid team number %d", team);
		team = TEAM_NOT_SET;
	    }
	    continue;
	}
	if (strcmp(argv[i], "-list") == 0) {
	    list_servers = true;
	    auto_connect = true;
	    continue;
	}
	if (strcmp(argv[i], "-display") == 0) {
	    strcpy(display, argv[++i]);
	    continue;
	}
	if (strcmp(argv[i], "-shutdown") == 0) {
	    auto_shutdown = true;
	    auto_connect = true;
	    if (argc > i+1)
		strcpy(shutdown_reason, argv[++i]);
	    else
		strcpy(shutdown_reason, "Unknown reason.");
	    continue;
	}

	if (argv[i][0] == '-') {
	    error("Unkown option '%s'", argv[i]);
	} else
	    strcpy(server_host, argv[i]);
    }

    /*
     * --- Message of the Day ---
     */
    if (!auto_connect)
	printfile(MOTDFILE);
    if (list_servers)
	printf("LISTING AVAILABLE SERVERS:\n");

    /*
     * --- Correct the display --- May need modification
     */
    if (display[0] == '\0'
	|| strstr(display, "unix:0") != NULL
	|| strstr(display, "local:0") != NULL
	|| strcmp(display, ":0.0") == 0
	|| strcmp(display, ":0") == 0)
	sprintf(display, "%s:0", hostname);


#ifdef	LIMIT_ACCESS
    /*
     * If sysadm's have complained alot, check for free machines before
     * letting the user play.  If room is crowded, don't let him play.
     */
    if (!list_servers && Is_allowed(display) == false)
	exit (-1);
#endif

    /*
     * --- Try to contact server ---
     */
    if (server_host[0] != '\0') {	/* Server specified on command line? */
	DgramSend(socket_c, server_host, SERVER_PORT,
		  (char *)&req, sizeof(contact_pack_t));

	if (Get_contact_message())
	    connected = Connect_to_server();

    } else {				/* Search after servers... */
	SetTimeout(10, 0);
	if (Query_all(socket_c, SERVER_PORT,
		  (char *)&req, sizeof(contact_pack_t)) == -1) {
	    error("Couldn't send query packets");
	    exit(1);
	}
	D( printf("\n"); );

	/*
	 * Wait for answer.
	 */
	while (Get_contact_message()) {
	    if (connected = Connect_to_server())
		break;
	}
    }

    exit(connected==true ? 0 : -1);
}



/*
 * Code which uses 'real' broadcasting to find server.  Provided by
 * Bert Gijsbers.  Thanks alot!
 */

#ifndef MAX_INTERFACE
#define MAX_INTERFACE    16	/* Max. number of network interfaces. */
#endif


/*
 * Enable broadcasting on a (datagram) socket.
 */
int Enable_broadcast(int sockfd)
{
    int         flag = 1;	/* Turn it ON */

    return setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,
		      (void *)&flag, sizeof(flag));
}


/*
 * Query all hosts on a subnet one after another.
 * This should be avoided as much as possible.
 * It may cause network congestion and therefore fail,
 * because UDP is unreliable.
 * We only allow this horrible kludge for subnets with 8 or less
 * bits in the host part of the subnet mask.
 * Subnets with irregular subnet bits are properly handled (I hope).
 */
int Query_subnet(int sockfd,
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
	sendto(sockfd, msg, msglen, 0,
	       (struct sockaddr *)&addr, sizeof(addr));
	D( printf("sendto %s/%d\n",
		  inet_ntoa(addr.sin_addr), ntohs(addr.sin_port)); );
	/*
	 * Imagine a server responding to our query while we
	 * are still transmitting packets for non-existing servers
	 * and the server packet colliding with one of our packets.
	 */
	usleep(10000);
    }

    return 0;
}


/*
 * Send a datagram on all network interfaces of the local host.  Return the
 * number of packets succesfully transmitted.
 * We only use the loopback interface if we didn't do a broadcast
 * on one of the other interfaces in order to reduce the chance that
 * we get multiple responses from the same server.
 */
int Query_all(int sockfd, int port, char *msg, int msglen)
{
    int         	fd, len, ifflags, count = 0, broadcasts = 0, haslb = 0;
    struct sockaddr_in	addr, mask, loopback;
    struct ifconf	ifconf;
    struct ifreq	*ifreqp, ifreq, ifbuf[MAX_INTERFACE];

    /*
     * Broadcasting on a socket MUST be explicitly enabled.  This seems to be
     * insider information.  8-!
     */
    if (Enable_broadcast(sockfd) == -1) {
	error("set broadcast");
	return (-1);
    }

    /*
     * Create an unbound datagram socket.  Only used for ioctls.
     */
    if ((fd = socket(AF_INET, SOCK_DGRAM, PF_UNSPEC)) == -1) {
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
	return (-1);
    }
    for (len = 0; len + sizeof(struct ifreq) <= ifconf.ifc_len;) {
	ifreqp = (struct ifreq *)&ifconf.ifc_buf[len];

	D( printf("interface name %s\n", ifreqp->ifr_name); );
	D( printf("\taddress family %d\n", ifreqp->ifr_addr.sa_family); );

	len += sizeof(struct ifreq);
#if BSD >= 199006 || HAVE_SA_LEN
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
	addr = loopback;
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

    return count;
}

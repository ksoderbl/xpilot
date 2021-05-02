/* xpilot.c,v 1.18 1992/06/28 05:38:37 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#if  defined(apollo)
#    include <sys/types.h>
#else
#    include <string.h>
#    include <unistd.h>
#endif

#include "pack.h"
#include "bit.h"
#include "version.h"
#include "config.h"

#ifndef	lint
static char versionid[] = "@(#)$" TITLE " $";
static char sourceid[] =
    "@(#)xpilot.c,v 1.18 1992/06/28 05:38:37 bjoerns Exp";
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
	if (DgramReceiveAny(socket_c, (char *)&pack, sizeof(pack_t)) == -1) {
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

	printf("Server on %s. Enter command> ", he->h_name);

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
               "* If you don't specify any delay for shutdown, you will signal "
               "  that\nthe server should stop an ongoing shutdown.\n");
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
	    error("Could not get any answer from server (pack length %d)", len);
	    return (false);
	}

	/*
	 * Now try and interpret the result.
	 */
	switch (reply.status) {

	case SUCCESS:
	    /*
	     * Oh glorious success.
	     */
	    switch (req.core.type) {
	    case SHUTDOWN_pack:
		if (ntohl(req.command.arg_int) == 0)
		    puts("Shutdown stopped.");
		else
		    puts("Shutdown initiated.");
		break;
	    case ENTER_GAME_pack:
		puts("You have entered the game.");
		break;
	    default:
		puts("Operation successful.");
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

	/*
	 * Did the reply include a string?
	 */
	if (len > sizeof(core_pack_t) && !auto_connect) {
	    puts(reply.str);
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
    char		machine[MAX_LINE];
    int			i;
    struct passwd	*pwent;
    bool		connected = false;


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
    strcpy(display, getenv("DISPLAY"));

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
	    if (team < 0)
		team = 0;
	    else if (team > 9)
		team = 9;
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
    if (!list_servers && Is_allowed() == false)
	exit (-1);
#endif

    SetTimeout(15, 0);

    /*
     * --- Try to contact server ---
     */
    if (server_host[0] != '\0') {	/* Server specified on command line? */
	DgramSend(socket_c, server_host, SERVER_PORT,
		  (char *)&req, sizeof(contact_pack_t));

	if (Get_contact_message())
	    connected = Connect_to_server();

    } else {				/* Search after servers... */
	/*
	 * Try to broadcast the 'hello servers' packet.  This won't work
	 * on all systems, but for those who allow mortals to do broadcasts
	 * it will mean a significant reduction in netload.  Also, some
	 * ethernet controllers (mainly AIX ones) goes nutso when we try
	 * to do 'manual broadcasts'.  (See below)
	 *
	 * Note, this method only works for class C nets, or class B nets
	 * with subnetmasks.
	 */
	/*
	sprintf(server_host, "%s255", base_addr);
		if (DgramSend(socket_c, server_host, DEFAULT_PORT,
			(char *)&req, sizeof(Pack))j);
			IKKE FERDIG */

	/*
	 * Got the IP address of name-server with the last part deleted, i.e.
	 *	129.242.16.110	  =>   129.242.16.
	 * Then search through all the machines in the 129.242.16 domain.
	 */
D(	printf("Sending packet to:\n"); )
	for (i=1; i<255; i++) {
	    sprintf(server_host, "%s%d", base_addr, i);
D(	    printf("%s\t", server_host);	)
	    while (DgramSend(socket_c, server_host, SERVER_PORT, (char *)&req,
			     sizeof(contact_pack_t)) < sizeof(contact_pack_t))
		    sleep(1);
	    usleep(10000);		/* UDP isn't reliable, so we'd better */
	}				/* not push the net. */

D(	printf("\n");	)


	/* Hard coded search domains. */
#ifdef	UIT
	strcpy(base_addr, "129.242.16.");
	for (i=1;  i<255; i++) {
	    sprintf(server_host, "%s%d", base_addr, i);
	    while (DgramSend(socket_c, server_host, DEFAULT_PORT, (char *)&req,
			     sizeof(Pack)) == -1)
		sleep(1);
	}
#endif
#ifdef	CC
	strcpy(base_addr, "129.242.6.");
	for (i=1;  i<255; i++) {
	    sprintf(server_host, "%s%d", base_addr, i);
	    while (DgramSend(socket_c, server_host, DEFAULT_PORT, (char *)&req,
			     sizeof(Pack)) == -1)
		sleep(1);
	}
#endif

	/*
	 * Wait for answer.
	 */
	while (Get_contact_message()) {
	    if (connected = Connect_to_server())
		break;
	}
    }

    exit (connected==true ? 0 : -1);
}

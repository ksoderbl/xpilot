/* $Id: server.c,v 3.80 1994/04/14 11:46:39 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-94 by
 *
 *      Bj�rn Stabell        (bjoerns@staff.cs.uit.no)
 *      Ken Ronny Schouten   (kenrsc@stud.cs.uit.no)
 *      Bert G�sbers         (bert@mc.bio.uva.nl)
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

#define SERVER
#include "types.h"
#ifdef VMS
#include <unixio.h>
#include <unixlib.h>
#else
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#if !defined(__hpux) && !defined(VMS)
#include <sys/time.h>
#endif
#ifdef _SEQUENT_
#include <sys/procstats.h>
#define gettimeofday(T,X)	get_process_stats(T, PS_SELF, \
					(struct process_stats *)NULL, \
					(struct process_stats *)NULL)
#endif
#ifdef VMS
#include "username.h"
#include <socket.h>
#include <in.h>
#include <inet.h>
#else
#include <pwd.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <netdb.h>

#include "config.h"
#include "global.h"
#include "socklib.h"
#include "version.h"
#include "map.h"
#include "pack.h"
#include "draw.h"
#include "robot.h"
#include "saudio.h"
#include "bit.h"
#include "net.h"
#include "netserver.h"
#ifdef SUNCMW
#include "cmw.h"
#endif /* SUNCMW */

#ifndef	lint
static char versionid[] = "@(#)$" TITLE " $";
static char sourceid[] =
    "@(#)$Id: server.c,v 3.80 1994/04/14 11:46:39 bert Exp $";
#endif


/*
 * Global variables
 */
int			NumPlayers = 0;
int			NumPseudoPlayers = 0;
int			NumObjs = 0;
player			**Players;
object			*Obj[MAX_TOTAL_SHOTS];
long			Id = 1;		    /* Unique ID for each object */
long			GetInd[MAX_ID];
server			Server;
int			ShutdownServer = -1, ShutdownDelay = 1000;
int 			framesPerSecond = 18;

int			Argc;
char			**Argv;

static int		Socket;
static sockbuf_t	ibuf;
static char		msg[MSG_LEN];
static char		meta_address[16];
static char		meta_address_two[16];
static bool		Log = true;
static bool		NoPlayersEnteredYet = true;
static bool		lock = false;
static void		Wait_for_new_players(void);
time_t			gameOverTime = 0;
time_t			serverTime = 0;
extern int		login_in_progress;

void Send_meta_server(void);
static bool Owner(char *name, char *in_host);

static void catch_alarm(int signum)
{
    /* nothing */
}

int main(int argc, char *argv[])
{
    struct hostent *hinfo;
    struct passwd *pwent;


    /*
     * Make output always linebuffered.  By default pipes
     * and remote shells cause stdout to be fully buffered.
     */
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
    setvbuf(stderr, NULL, _IOLBF, BUFSIZ);

    /*
     * --- Output copyright notice ---
     */
    printf("  Copyright " COPYRIGHT ".\n"
	   "  " TITLE " comes with ABSOLUTELY NO WARRANTY; "
	      "for details see the\n"
	   "  provided LICENSE file.\n\n");

    Argc = argc; Argv = argv;
#ifdef SUNCMW
    cmw_priv_init();
#endif /* SUNCMW */
    init_error(argv[0]);
    srand(time((time_t *)0) * getpid());
    Parser(argc, argv);
    Make_table();			/* Make trigonometric tables */
    Compute_gravity();
    Find_base_direction();

    /* Allocate memory for players, shots and messages */
    Alloc_players(World.NumBases + MAX_PSEUDO_PLAYERS);
    Alloc_shots(MAX_TOTAL_SHOTS);
    Alloc_cells();

    Move_init();

    if (BIT(World.rules->mode, TEAM_PLAY)) {
	int i;
	for(i=0; i < World.NumTreasures; i++)
	    if (World.treasures[i].team != TEAM_NOT_SET)
		Make_treasure_ball(i);
    }

    /*
     * Get server's official name.
     */
    GetLocalHostName(Server.host, sizeof Server.host);

    /*
     * Get meta server's address.
     */
    if (reportToMetaServer) {
	signal(SIGALRM, catch_alarm);
	alarm(5);
	hinfo = gethostbyname(META_HOST);
	alarm(0);
	if (hinfo == NULL) {
	    strncpy(meta_address, META_IP, sizeof meta_address);
	} else {
	    strncpy(meta_address,
		    inet_ntoa(*((struct in_addr *)(hinfo->h_addr))),
		    sizeof meta_address);
	}
	meta_address[sizeof meta_address - 1] = '\0';

	signal(SIGALRM, catch_alarm);
	alarm(5);
	hinfo = gethostbyname(META_HOST_TWO);
	alarm(0);
	signal(SIGALRM, SIG_IGN);
	if (hinfo == NULL) {
	    /* This should be changed since we now send to meta host 1.
	     * But there is no harm that there are sent two packets to the 
	     * same meta server. */
	    strncpy(meta_address_two, META_IP, sizeof meta_address_two);
	} else {
	    strncpy(meta_address_two,
		    inet_ntoa(*((struct in_addr *)(hinfo->h_addr))),
		    sizeof meta_address_two);
	}
	meta_address_two[sizeof meta_address_two - 1] = '\0';
    }

    /*
     * Get owners login name.
     */
#ifdef VMS
    getusername(Server.name);
#else
    pwent = getpwuid(geteuid());
    strcpy(Server.name, pwent->pw_name);
#endif

    /*
     * Log, if enabled.
     */
    if ((strcmp(Server.name, "kenrsc") == 0) ||
	(strcmp(Server.name, "bjoerns") == 0))
	Log = false;
    Log_game("START");			/* Log start */

    /*
     * Create a socket which we can listen on.
     */
    if ((Socket = CreateDgramSocket(contactPort)) == -1) {
	error("Could not create Dgram socket");
	End_game();
    }
    if (SetSocketNonBlocking(Socket, 1) == -1) {
	error("Can't make contact socket non-blocking");
	End_game();
    }
    if (Sockbuf_init(&ibuf, Socket, SERVER_SEND_SIZE,
		     SOCKBUF_READ | SOCKBUF_WRITE | SOCKBUF_DGRAM) == -1) {
	error("No memory for contact buffer");
	End_game();
    }

    SetTimeout(0, 0);

    if (Setup_net_server(World.NumBases, Socket) == -1) {
	End_game();
    }

    if (signal(SIGHUP, Handle_signal) == SIG_IGN) {
	signal(SIGHUP, SIG_IGN);
    }
    signal(SIGTERM, Handle_signal);
    signal(SIGINT, Handle_signal);
    signal(SIGUSR1, Handle_signal);
    signal(SIGPIPE, SIG_IGN);
#ifdef IGNORE_FPE
    signal(SIGFPE, SIG_IGN);
#endif

    /*
     * Set the time the server started 
     */

    serverTime = time(NULL);

    /* 
     * Report to Meta server
     */
   
    Send_meta_server();
    
    /*
     * If the server is not in raw mode it should run only if
     * there are players logged in.
     */
    if (!RawMode) {
	signal(SIGALRM, Handle_signal);	/* Get first client, then proceed. */
	if (!NoQuit) {
	    alarm(5*60);		/* Signal me in 5 minutes. */
	}
	Wait_for_new_players();
	alarm(0);
	signal(SIGALRM, SIG_IGN);
	SetTimeout(0, 0);
    }
 
    if (gameDuration > 0.0) {
	printf("Server will stop in %g minutes.\n", gameDuration);
	gameOverTime = (time_t)(gameDuration * 60) + time((time_t *)NULL);
    }
    
    Main_Loop();			    /* Entering main loop. */
    /* NEVER REACHED */
    return (-1);
}

void Send_meta_server(void)
{
#ifdef SOUND
#define SOUND_SUPPORT_STR	"yes"
#else
#define SOUND_SUPPORT_STR	"no"
#endif

    char 	string[MAX_STR_LEN];
    char 	status[MAX_STR_LEN];
    int		i;
    bool	first = true;

    if (!reportToMetaServer)
	return;

    Server_info(status, sizeof(status));

    sprintf(string,
	    "add server %s\n"
	    "add users %d\n"
	    "add version %s\n"
	    "add map %s\n"
	    "add sizeMap %3dx%3d\n"
	    "add author %s\n"
	    "add bases %d\n"
	    "add fps %d\n"
	    "add port %d\n"
	    "add mode %s\n"
	    "add stime %ld\n"
	    "add sound " SOUND_SUPPORT_STR "\n",
	    Server.host, NumPlayers - NumRobots, 
	    VERSION, World.name, World.x, World.y, World.author, 
	    World.NumBases, FPS, contactPort,
	    (lock && ShutdownServer == -1) ? "locked"
		: (!lock && ShutdownServer != -1) ? "shutting down"
		: (lock && ShutdownServer != -1) ? "locked and shutting down"
		: "ok",
	    time(NULL) - serverTime);


    for(i=0; i < NumPlayers; i++) {
	if (Players[i]->robot_mode == RM_NOT_ROBOT) {
	    sprintf(string + strlen(string),
		    "%s%s=%s@%s",
		    (first) ? "add players " : ",",
		    Players[i]->name,
		    Players[i]->realname,
		    Players[i]->hostname);
	    first = false;
	}
    }
    
    strcat(string,"\nadd status ");
    if (strlen(string) + strlen(status) >= sizeof(string)) {
	/* Prevent array overflow */
	strcpy(&status[sizeof(string) - (strlen(string) + 2)], "\n");
    }
    strcat(string, status);

    i = strlen(string)+1;
    if (DgramSend(Socket, meta_address, META_PORT, string, i) != i) {
	GetSocketError(Socket);
	DgramSend(Socket, meta_address, META_PORT, string, i);
    }
    if (meta_address_two[0] != '\0'
	&& strcmp(meta_address, meta_address_two)
	&& DgramSend(Socket, meta_address_two, META_PORT, string, i) != i) {
	GetSocketError(Socket);
	DgramSend(Socket, meta_address_two, META_PORT, string, i);
    }
}

/*
 * Main loop.
 */
void Main_Loop(void)
{
    extern void		Loop_delay(void);
    int			main_loops = 0;
    time_t		currentTime;
    time_t		lastMetaCheckTime = 0;

#ifndef SILENT
    printf("Server runs at %d frames per second\n", framesPerSecond);
#endif

    SetTimeout(0, 0);

    while (NoQuit
	   || NumPlayers - NumPseudoPlayers > NumRobots
	   || NoPlayersEnteredYet) {
	
	currentTime = time(NULL);
	main_loops++;

#define CHECK_FOR_NEW_PLAYERS	4
#define GIVE_META_SERVER_A_HINT	180

	if (currentTime - lastMetaCheckTime >= GIVE_META_SERVER_A_HINT) {
	    lastMetaCheckTime = currentTime;
	    Send_meta_server();
	}

	if (NumPlayers - NumPseudoPlayers == NumRobots && !RawMode) {
	    block_timer();
	    Wait_for_new_players();
	    allow_timer();
	}
	
	Update_objects();
	
	/*
	 * Check for possible shutdown, the server will
	 * shutdown when ShutdownServer (a counter) reaches 0.
	 * If the counter is < 0 then no shutdown is in progress.
	 */
	if (ShutdownServer >= 0) {
	    if (ShutdownServer == 0)
		End_game();
	    else
		ShutdownServer--;
	}
	
	if ((main_loops % UPDATES_PR_FRAME) == 0) {
	    Frame_update();
	    Loop_delay();
	}

	if (Input() > 0) {
	    NoPlayersEnteredYet = false;
	}
    }

    End_game();
}


/*
 * Wait for a player to show up.
 */
static void Wait_for_new_players(void)
{
    int			new_players = false;
    time_t		start_time,
			start_loops,
			milli_delta;
#if _SEQUENT_
    timeval_t		tv;
#else
    struct timeval	tv;
#endif

    gettimeofday(&tv, NULL);
    start_time = tv.tv_sec;
    start_loops = loops;
    while (new_players == false) {
	if (login_in_progress > 0) {
	    if (Input() > 0) {
		NoPlayersEnteredYet = false;
		new_players = true;
		continue;
	    }
	    SetTimeout(0, 10*1000);
	} else {
	    SetTimeout(CHECK_FOR_NEW_PLAYERS, 0);
	}
	if (SocketReadable(Socket) != 0) {
	    Contact();
	}
	gettimeofday(&tv, NULL);
	milli_delta = (tv.tv_sec - start_time) * 1000 + tv.tv_usec / 1000;
	loops = start_loops + (FPS * milli_delta) / 1000;
    }
    SetTimeout(0, 0);
}

/*
 *  Last function, exit with grace.
 */
void End_game(void)
{
    player	*pl;
    int		len;
    char	string[80];

    if (ShutdownServer == 0) {
	errno = 0;
	error("Shutting down...");
    }

    while (NumPlayers > 0) {	/* Kick out all remaining players */
	pl = Players[NumPlayers - 1];
	if (pl->conn == NOT_CONNECTED) {
	    Delete_player(NumPlayers - 1);
	} else {
	    Destroy_connection(pl->conn, "server exiting", __FILE__, __LINE__);
	}
    }

    /* Tell meta serve that we are gone */
    if (reportToMetaServer) {
	sprintf(string, "server %s\nremove", Server.host);
	len = strlen(string) + 1;
	DgramSend(Socket, meta_address, META_PORT, string, len);
	if (meta_address_two[0] != '\0'
	    && strcmp(meta_address, meta_address_two)) {
	    DgramSend(Socket, meta_address_two, META_PORT, string, len);
	}
    }

    SocketClose(Socket);
    Free_players();
    Free_shots();
    Free_map();
    Free_cells();
    Log_game("END");			    /* Log end */

    exit (0);
}

/*
 * Return a good team number for a player. 
 *
 * If the team is not specified, the player is assigned
 * to a non-empty team which has space.  The team chosen
 * is one with the least number of players.
 * 
 * If all non-empty teams are full, the player is assigned
 * to the first available team.
 *
 * Note:  Team zero is not part of this algorithm as that
 * team is reserved for the robots.
 */
static int Pick_team(void)
{
    int	i, teammin, min;

    /*
     * Find first non-empty team with least number of players that has room
     * for more...
     */
    min = NumPlayers + 1;
    teammin = TEAM_NOT_SET;

    for (i = 1; i < MAX_TEAMS; i++) {
	if (! World.teams[i].NumMembers
	    || World.teams[i].NumMembers >= World.teams[i].NumBases)
	    break;
	if (World.teams[i].NumMembers < min) {
	    min = World.teams[i].NumMembers;
	    teammin = i;
	}
    }

    if (teammin != TEAM_NOT_SET)
	return teammin;

    /*
     * Find first available team...
     */
    for (i = 1; i < MAX_TEAMS; i++)
	if (World.teams[i].NumMembers < World.teams[i].NumBases)
	    return i;

    /*
     * There is no team to enter...
     */
    return TEAM_NOT_SET;
}

void Contact(void)
{
    int			i,
    			team,
    			bytes,
    			delay,
                        max_robots,
    			login_port;
    char		*in_host,
			status,
			reply_to;
    unsigned		magic,
			version,
			my_magic;
    unsigned short	port;
    char		real_name[MAX_CHARS],
			disp_name[MAX_CHARS],
			nick_name[MAX_CHARS],
			host_name[MAX_CHARS],
			str[MSG_LEN];
    static struct tunable_int_options_t {
	char *opt;
	int *val;
    } tune_iopt[] = {
	{ "initialFuel",             &initialFuel },
	{ "initialTanks",            &initialTanks },
	{ "initialECMs",             &initialECMs },
	{ "initialMines",            &initialMines },
	{ "initialMissiles",         &initialMissiles },
	{ "initialCloaks",           &initialCloaks },
	{ "initialSensors",          &initialSensors },
	{ "initialWideangles",       &initialWideangles },
	{ "initialRearshots",        &initialRearshots },
	{ "initialAfterburners",     &initialAfterburners },
	{ "initialTransporters",     &initialTransporters },
	{ "initialLasers",           &initialLasers },
	{ "initialEmergencyThrusts", &initialEmergencyThrusts },
	{ "initialTractorBeams",     &initialTractorBeams },
	{ "initialAutopilots",       &initialAutopilots }
    };

    /*
     * Someone connected to us, now try and deschiffer the message :)
     */
    Sockbuf_clear(&ibuf);
    if ((bytes = DgramReceiveAny(Socket, ibuf.buf, ibuf.size)) <= 8) {
	if (bytes < 0
	    && errno != EWOULDBLOCK
	    && errno != EAGAIN
	    && errno != EINTR) {
	    /*
	     * This caused some long series of error messages
	     * if a player connection crashed violently (SIGKILL, SIGSEGV).
	     * error("SocketRead (pack from %s)", DgramLastaddr());
	     */
	    /*
	     * Clear the error condition for the contact socket.
	     */
	    GetSocketError(Socket);
	}
	return;
    }
    ibuf.len = bytes;

    /*
     * Get hostname.
     */
    in_host = DgramLastaddr();
    
    /*
     * Determine if we can talk with this client.
     */
    if (Packet_scanf(&ibuf, "%u", &magic) <= 0
	|| (magic & 0xFFFF) != (MAGIC & 0xFFFF)) {
	D(printf("Incompatible packet from %s (0x%08x)", in_host, magic);)
	return;
    }
    version = MAGIC2VERSION(magic);

    /*
     * Read core of packet.
     */
    if (Packet_scanf(&ibuf, "%s%hu%c", real_name, &port, &reply_to) <= 0) {
	D(printf("Incomplete packet from %s", in_host);)
	return;
    }
    real_name[MAX_NAME_LEN - 1] = '\0';

    /*
     * Now see if we have the same (or a compatible) version.
     * If the client request was only a contact request (to see
     * if there is a server running on this host) then we don't
     * care about version incompatibilities, so that the client
     * can decide itself if it wants to conform to our version.
     */
    if (version < MIN_CLIENT_VERSION
	|| (version > MAX_CLIENT_VERSION
	    && reply_to != CONTACT_pack)) {
	D(error("Incompatible version with %s@%s (%04x,%04x)",
	    real_name, in_host, MY_VERSION, version);)
	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c%c", MAGIC, reply_to, E_VERSION);
	if (DgramSend(Socket, in_host, port, ibuf.buf, ibuf.len) == -1) {
	    GetSocketError(Socket);
	}
	return;
    }

    /*
     * Support some older clients, which don't know
     * that they can join the current version.
     *
     * IMPORTANT! Adjust the next code if you're changing version numbers.
     */
    if (version >= 0x3100 && version <= MY_VERSION) {
	my_magic = VERSION2MAGIC(version);
    } else
	my_magic = MAGIC;


    status = SUCCESS;

    /*
     * Now decode the packet type field and do something witty.
     */
    switch (reply_to) {

    case ENTER_GAME_pack:	{
	/*
	 * Someone wants to enter the game.
	 */
	if (Packet_scanf(&ibuf, "%s%s%s%d", nick_name, disp_name, host_name,
			 &team) <= 0) {
	    D(printf("Incomplete login from %s@%s", real_name, in_host);)
	    return;
	}
	nick_name[sizeof(nick_name) - 1] = '\0';
	disp_name[sizeof(disp_name) - 1] = '\0';
	host_name[sizeof(host_name) - 1] = '\0';

	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c", my_magic, reply_to);

	/*
	 * Bad input parameters?
	 */
	if (nick_name[0] == 0
	    || real_name[0] == 0
	    || host_name[0] == 0
	    || nick_name[0] < 'A'
	    || nick_name[0] > 'Z') {
#ifndef SILENT
	    printf("Invalid name (%s,%s) from %s@%s",
		  nick_name, real_name, real_name, in_host);
#endif
	    status = E_INVAL;
	}

	/*
	 * Game locked?
	 */
	else if (lock && !Owner(real_name, in_host)) {
	    status = E_GAME_LOCKED;
	}

	/*
	 * Is the game full?
	 */
	else if (NumPlayers >= World.NumBases) {
	    status = E_GAME_FULL;
	}

	/*
	 * Maybe don't have enough room for player on that team?
	 */
	else if (BIT(World.rules->mode, TEAM_PLAY)) {
	    if (team == TEAM_NOT_SET || team >= MAX_TEAMS || team < 0) {
		if (!teamAssign || (team = Pick_team()) == TEAM_NOT_SET)
		    status = E_TEAM_NOT_SET;
	    }
	    else if (World.teams[team].NumMembers
		  >= World.teams[team].NumBases) {
		status = E_TEAM_FULL;
	    }
	}
	    
	/*
	 * All names must be unique (so we know who we're talking about).
	 */
	if (status == SUCCESS) {
	    for (i=0; i<NumPlayers; i++) {
		if (strcasecmp(Players[i]->name, nick_name) == 0) {
		    D(printf("%s %s\n", Players[i]->name, nick_name);)
		    status = E_IN_USE;
		    break;
		}
	    }
	}

	/*
	 * Find a port for the client to connect to.
	 */
	if (status == SUCCESS) {
	    if ((login_port = Setup_connection(real_name, nick_name,
					       disp_name, team,
					       in_host, host_name,
					       version)) > 0) {
		/*
		 * Tell the client which port to use for logging in.
		 */
		Packet_printf(&ibuf, "%c%hu", status, login_port);
		break;
	    }
	    status = E_SOCKET;
	}
	Packet_printf(&ibuf, "%c", status);
    }
	break;


    case REPORT_STATUS_pack:	{
	/*
	 * Someone asked for information.
	 */

#ifndef	SILENT
	printf("%s@%s asked for info about current game.\n",
	       real_name, in_host);
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

	if (!Owner(real_name, in_host)) {
	    status = E_NOT_OWNER;
	}
	else if (Packet_scanf(&ibuf, "%s", str) <= 0) {
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

	if (!Owner(real_name, in_host)) {
	    status = E_NOT_OWNER;
	} else {
	    lock = lock ? false : true;
	}
	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c%c", my_magic, reply_to, status);
    }
	break;

	
    case CONTACT_pack:	{
	/*
	 * Got contact message from client.
	 */

	D(printf("Got CONTACT from %s.\n", in_host));
	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c%c", my_magic, reply_to, status);
    }
	break;


    case SHUTDOWN_pack:	{
	/*
	 * Shutdown the entire server.
	 */

	if (!Owner(real_name, in_host)) {
	    status = E_NOT_OWNER;
	}
	else if (Packet_scanf(&ibuf, "%d%s", &delay, str) <= 0) {
	    status = E_INVAL;
	} else {
	    sprintf(msg, "|*******| %s (%s) |*******| \"%s\"",
		(delay > 0) ? "SHUTTING DOWN" : "SHUTDOWN STOPPED",
		real_name, str);
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

	if (!Owner(real_name, in_host)) {
	    status = E_NOT_OWNER;
	}
	else if (Packet_scanf(&ibuf, "%s", str) <= 0) {
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
		sprintf(msg, "\"%s\" upset the gods and was kicked out "
			"of the game.", Players[found]->name);
		Set_message(msg);
		if (Players[found]->conn == NOT_CONNECTED) {
		    Delete_player(found);
		} else {
		    Destroy_connection(Players[found]->conn, "kicked out",
					__FILE__, __LINE__);
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
	int		ival;

	if (!Owner(real_name, in_host)) {
	    status = E_NOT_OWNER;
	}
	else if (Packet_scanf(&ibuf, "%S", str) <= 0
		 || (opt = strtok(str, ":")) == NULL
		 || (val = strtok(NULL, "")) == NULL
		) {
	    status = E_INVAL;
	}
	else {
	    status = E_INVAL;
	    if (sscanf(val, "%d", &ival) > 0) {
		for (i = 0; i < NELEM(tune_iopt); i++) {
		    if (!strcasecmp(tune_iopt[i].opt, opt)) {
			*tune_iopt[i].val = ival;
			Set_initial_resources();
			status = SUCCESS;
			break;
		    }
		}
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
		&& DgramSend(Socket, in_host, port, ibuf.buf, ibuf.len) == -1) {
		GetSocketError(Socket);
		bad = true;
	    }
	} while (!bad);
    }
	return;

    case MAX_ROBOT_pack:	{
	/*
	 * Set the maximum of robots wanted in the server
	 */
	int	ind;

	if (!Owner(real_name, in_host)) {
	    status = E_NOT_OWNER;
	}
	else if (Packet_scanf(&ibuf, "%d", &max_robots) <= 0
	    || max_robots < 0) {
	    status = E_INVAL;
	}
	else {
	    WantedNumRobots = max_robots;
	    while (WantedNumRobots < NumRobots) {
		ind = -1;
		for (i=0; i<NumPlayers; i++) {

		    /*
		     * Remove the robot with the lowest score.
		     */

		    if (Players[i]->robot_mode != RM_NOT_ROBOT
			&& Players[i]->robot_mode != RM_OBJECT) {
			if (ind == -1)	
			    ind = i;
			else if (Players[i]->score < Players[ind]->score)
			    ind = i;
		    }
		}

		sprintf(msg, "\"%s\" upset the gods and was kicked out "
			"of the game.", Players[ind]->name);
		Set_message(msg);
		Delete_player(ind);
	    }
	    updateScores = true;
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
	    reply_to, real_name, in_host);)

	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%u%c%c", my_magic, reply_to, E_VERSION);
    }

    if (DgramSend(Socket, in_host, port, ibuf.buf, ibuf.len) == -1) {
	D(printf("Could not reply to %s@%s/%d.", real_name, in_host, port);)
	GetSocketError(Socket);
    }
}

/*
 * Return status for server 
*/
void Server_info(char *str, unsigned max_size)
{
    int i, j, k;
    player *pl, **order, *best = NULL;
    float ratio, best_ratio = -1e7;
    char name[MAX_CHARS];
    char lblstr[MAX_CHARS];

    sprintf(str,
	    "SERVER VERSION...: %s\n"
	    /* security: "STARTED BY.......: %s\n" */
	    "STATUS...........: %s\n"
	    "MAX SPEED........: %d fps\n"
	    "WORLD (%3dx%3d)..: %s\n"
	    "      AUTHOR.....: %s\n"
	    "PLAYERS (%2d/%2d)..:\n",
	    TITLE,
	    /* security: Server.name, */
	    (lock && ShutdownServer == -1) ? "locked" :
	    (!lock && ShutdownServer != -1) ? "shutting down" :
	    (lock && ShutdownServer != -1) ? "locked and shutting down" : "ok",
	    FPS,
	    World.x, World.y, World.name, World.author,
	    NumPlayers, World.NumBases);

    if (strlen(str) >= max_size) {
	errno = 0;
	error("Server_info string overflow (%d)", max_size);
	str[max_size - 1] = '\0';
	return;
    }
    if (NumPlayers <= 0) {
	return;
    }

    sprintf(msg,
	   "\nNO:  TM: NAME:             LIFE:   SC:    PLAYER:\n"
	   "-------------------------------------------------\n");
    if (strlen(msg) + strlen(str) >= max_size) {
	return;
    }
    strcat(str, msg);

    if ((order = (player **) malloc(NumPlayers * sizeof(player *))) == NULL) {
	error("No memory for order");
	return;
    }
    for (i=0; i<NumPlayers; i++) {
	pl = Players[i];
	if (BIT(pl->mode, LIMITED_LIVES)) {
	    ratio = (float) pl->score;
	} else {
	    ratio = (float) pl->score / (pl->life + 1);
	}
	if (best == NULL
	    || ratio > best_ratio) {
	    best_ratio = ratio;
	    best = pl;
	}
	for (j = 0; j < i; j++) {
	    if (order[j]->score < pl->score) {
		for (k = i; k > j; k--) {
		    order[k] = order[k - 1];
		}
		break;
	    } 
	}
	order[j] = pl;
    }
    for (i=0; i<NumPlayers; i++) {
	pl = order[i];
	strcpy(name, pl->name);
	if (pl->robot_mode != RM_NOT_ROBOT
	    && BIT(pl->robot_lock, LOCK_PLAYER)) {
	    sprintf(name + strlen(name), " (%s)",
		Players[GetInd[pl->robot_lock_id]]->name);
	    if (strlen(name) >= 19) {
		strcpy(&name[17], ")");
	    }
	}
	sprintf(lblstr, "%c%c %-19s%03d%6d",
		(pl == best) ? '*' : pl->mychar,
		(pl->team == TEAM_NOT_SET) ? ' ' : pl->team+'0',
		name, pl->life, pl->score);
	sprintf(msg, "%2d... %-36s%s@%s\n",
		i+1, lblstr, pl->realname,
		pl->robot_mode == RM_NOT_ROBOT
		? pl->hostname
		: "robots.org");
	if (strlen(msg) + strlen(str) >= max_size)
	    break;
	strcat(str, msg);
    }
    free(order);
}

/*
 * Returns true if <name> has owner status of this server.
 */
static bool Owner(char *name, char *in_host)
{
    bool		valid = false;
    char		*local = NULL;
    char		host[MAX_CHARS];

    strncpy(host, in_host, sizeof host);
    host[sizeof host - 1] = '\0';
    if (!strcmp(name, Server.name)) {
	if (!(local = GetSockAddr(Socket))) {
	    perror("GetSockAddr(Socket)");
	    valid = true;
	}
	/* require that the owner issue commands from the same host
	 * he started the server on. */
	else if (!strcmp(host, local)
	      || !strcmp(local, "0.0.0.0")	/* fix later */
	      || !strcmp(host, "127.0.0.1")) {
	    valid = true;
	}
    }
    /* permit the authors to issue messages and help with problems,
     * but do require that they do so from their well known locations. */
    else if (!strcmp(name, "kenrsc") ? !strncmp(host, "129.242.16.", 11)
	   : !strcmp(name, "bjoerns") ? !strncmp(host, "129.242.16.", 11)
	   : !strcmp(name, "bert") && !strncmp(host, "145.18.160.", 11)) {
	valid = true;
    }
#ifndef SILENT
    if (!valid) {
	printf("Permission denied for %s@%s (%s)\n", name, host, local?local:"");
    }
#endif
    return valid;
}


void Handle_signal(int sig_no)
{
    /* Tell meta serve that we are gone */
    if (reportToMetaServer) {
	char	string[80];
	sprintf(string,"server %s\nremove",Server.host);
	DgramSend(Socket, meta_address, META_PORT, string, strlen(string)+1);
	DgramSend(Socket, meta_address_two, META_PORT, string, strlen(string)+1);
    }    

    errno = 0;
    switch (sig_no) {
    case SIGALRM:
	error("First player has yet to show his butt, I'm bored... Bye!");
	SocketClose(Socket);
	Log_game("NOSHOW");
	break;

    case SIGTERM:
	error("Caught SIGTERM, terminating.");
	End_game();
	break;
    case SIGHUP:
	error("Caught SIGHUP, terminating.");
	End_game();
	break;
    case SIGINT:
	error("Caught SIGINT, terminating.");
	End_game();
	break;

    case SIGUSR1: {
	int	i;

	error("Caught SIGUSR1, checking teams.");

	for (i = 0; i < MAX_TEAMS; i++) {
	    Check_team_members (i);
	    Check_team_treasures (i);
	}
	signal (SIGUSR1, Handle_signal);	/* Some o/s require this */
	return;
    }

    default:
	error("Caught unkown signal: %d", sig_no);
	break;
    }

    Log_game("END");			    /* Log end */

    exit(sig_no);
}


void Log_game(char *heading)
{
#ifdef LOG
    char str[1024];
    FILE *fp;
    char timenow[81];
    struct tm *ptr;
    time_t lt;

    if (!Log)
	return;

    lt=time(NULL);
    ptr=localtime(&lt);
    strftime(timenow,79,"%I:%M:%S %p %Z %A, %B %d, %Y",ptr);

    sprintf(str,"%-50.50s\t%10.10s@%-15.15s\tWorld: %-25.25s\t%10.10s\n",
	    timenow,
	    Server.name,
	    Server.host,
	    World.name,
	    heading);

    if ((fp=fopen(LOGFILE, "a")) == NULL) {	/* Couldn't open file */
	error("Couldn't open log file, contact " LOCALGURU "");
	return;
    }

    fputs(str, fp);
    fclose(fp);
#endif
}

void Game_Over(void)
{
    long		maxsc, minsc;
    int			i, win, loose;
    char		msg[128];
	
    Set_message("Game over...");

    /*
     * Hack to prevent Compute_Game_Status from starting over again...
     */
    gameDuration = -1.0;
    
    if (BIT(World.rules->mode, TEAM_PLAY)) {
	int teamscore[MAX_TEAMS];
	maxsc = -32767;
	minsc = 32767;
	win = loose = -1;
	
	for (i=0; i < MAX_TEAMS; i++) {
	    teamscore[i] = 1234567; /* These teams are not used... */
	}
	for (i=0; i < NumPlayers; i++) {
	    int team;
	    if (Players[i]->robot_mode == RM_NOT_ROBOT) {
	        team = Players[i]->team;
	        if (teamscore[team] == 1234567) {
		    teamscore[team] = 0;
		}
		teamscore[team] += Players[i]->score;
	    }
	}

	for (i=0; i < MAX_TEAMS; i++) {
	    if (teamscore[i] != 1234567) {
		if (teamscore[i] > maxsc) {
		    maxsc = teamscore[i];
		    win = i;
		}
		if (teamscore[i] < minsc) {
		    minsc = teamscore[i];
		    loose = i;
		}
	    }
	}

	if (win != -1) {
	    sprintf(msg,"Best team (%d Pts): Team %d", maxsc, win);
	    Set_message(msg);
	    printf("%s\n", msg);
	}
	
	if (loose != -1 && loose != win) {
	    sprintf(msg,"Worst team (%d Pts): Team %d", minsc, loose);
	    Set_message(msg);
	    printf("%s\n", msg);
	}
    }

    maxsc = -32767;
    minsc = 32767;
    win = loose = -1;

    for (i = 0; i < NumPlayers; i++) {
	SET_BIT(Players[i]->status, GAME_OVER);
	if (Players[i]->robot_mode == RM_NOT_ROBOT) {
	    if (Players[i]->score > maxsc) {
		maxsc = Players[i]->score;
		win = i;
	    }
	    if (Players[i]->score < minsc) {
		minsc = Players[i]->score;
		loose = i;
	    }
	}
    }
    if (win != -1) {
	sprintf(msg,"Best human player: %s", Players[win]->name);
	Set_message(msg);
	printf("%s\n", msg);
    }
    if (loose != -1 && loose != win) {
	sprintf(msg,"Worst human player: %s", Players[loose]->name);
	Set_message(msg);
	printf("%s\n", msg);
    }
}

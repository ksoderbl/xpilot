/* $Id: server.c,v 3.119 1995/12/04 14:47:16 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
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
#include <ctype.h>
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
#ifdef sony_news
#define setvbuf(A,B,C,D)	setlinebuf(A)
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
#ifdef PLOCKSERVER
# if defined(__linux__)
#  undef PLOCKSERVER
# else
#  include <sys/lock.h>
# endif
#endif

#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "socklib.h"
#include "map.h"
#include "pack.h"
#include "saudio.h"
#include "bit.h"
#include "net.h"
#include "netserver.h"
#ifdef SUNCMW
#include "cmw.h"
#endif /* SUNCMW */

#ifdef VMS
#define META_VERSION	VERSION "-VMS"
#else
#define META_VERSION	VERSION
#endif

char server_version[] = VERSION;

#ifndef	lint
static char versionid[] = "@(#)$" TITLE " $";
static char sourceid[] =
    "@(#)$Id: server.c,v 3.119 1995/12/04 14:47:16 bert Exp $";
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
char			ShutdownReason[MAX_CHARS];
int 			framesPerSecond = 18;
long			main_loops = 0;		/* needed in events.c */

static int		Socket;
static sockbuf_t	ibuf;
static char		msg[MSG_LEN];
static char		meta_address[16];
static char		meta_address_two[16];
static bool		Log = true;
static bool		NoPlayersEnteredYet = true;
static bool		game_lock = false;
time_t			gameOverTime = 0;
time_t			serverTime = 0;
extern int		login_in_progress;

static bool Owner(char request, char *real_name, char *host_addr, int pass);
static void plockserver(int onoff);
static void Check_server_versions(void);
static void Wait_for_new_players(void);
static void Main_Loop(void);
static void Handle_signal(int sig_no);
static void Server_info(char *, unsigned);

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
    printf("  " COPYRIGHT ".\n"
	   "  " TITLE " comes with ABSOLUTELY NO WARRANTY; "
	      "for details see the\n"
	   "  provided LICENSE file.\n\n");

#ifdef SUNCMW
    cmw_priv_init();
#endif /* SUNCMW */
    init_error(argv[0]);
    srand(time((time_t *)0) * getpid());
    Check_server_versions();
    Parser(argc, argv);
    Make_table();			/* Make trigonometric tables */
    Compute_gravity();
    Find_base_direction();
    Walls_init();

    /* Allocate memory for players, shots and messages */
    Alloc_players(World.NumBases + MAX_PSEUDO_PLAYERS);
    Alloc_shots(MAX_TOTAL_SHOTS);
    Alloc_cells();

    Move_init();

    Robot_init();

    if (BIT(World.rules->mode, TEAM_PLAY)) {
	int i;
	for (i=0; i < World.NumTreasures; i++)
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
	    strncpy(meta_address_two, META_IP_TWO, sizeof meta_address_two);
	} else {
	    strncpy(meta_address_two,
		    inet_ntoa(*((struct in_addr *)(hinfo->h_addr))),
		    sizeof meta_address_two);
	}
	meta_address_two[sizeof meta_address_two - 1] = '\0';
    }

    /*
     * Get owner's login name.
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
#ifndef VMS
    signal(SIGUSR1, Handle_signal);
#endif
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

    Send_meta_server(1);

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

void Send_meta_server(int change)
{
#ifdef SOUND
#define SOUND_SUPPORT_STR	"yes"
#else
#define SOUND_SUPPORT_STR	"no"
#endif
#define GIVE_META_SERVER_A_HINT	180

    char 		string[MAX_STR_LEN];
    char 		status[MAX_STR_LEN];
    int			i;
    int			num_paused_players;
    bool		first = true;
    time_t		currentTime;
    static time_t	lastMetaSendTime = 0;


    if (!reportToMetaServer)
	return;

    currentTime = time(NULL);
    if (!change) {
	if (currentTime - lastMetaSendTime < GIVE_META_SERVER_A_HINT)
	    return;
    }
    lastMetaSendTime = currentTime;

    Server_info(status, sizeof(status));

    /* Find out the number of paused players. */
    num_paused_players = 0;
    for (i = 0; i < NumPlayers; i++) {
	if (IS_HUMAN_IND(i) && BIT(Players[i]->status, PAUSE)) {
	    num_paused_players++;
	}
    }
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
	    "add teams %d\n"
	    "add timing %d\n"
	    "add stime %ld\n"
	    "add sound " SOUND_SUPPORT_STR "\n",
	    Server.host, NumPlayers - NumPseudoPlayers - NumRobots - num_paused_players,
	    META_VERSION, World.name, World.x, World.y, World.author,
	    World.NumBases, FPS, contactPort,
	    (game_lock && ShutdownServer == -1) ? "locked"
		: (!game_lock && ShutdownServer != -1) ? "shutting down"
		: (game_lock && ShutdownServer != -1) ? "locked and shutting down"
		: "ok", World.NumTeamBases,
	    BIT(World.rules->mode, TIMING) ? 1:0,
	    time(NULL) - serverTime);


    for (i=0; i < NumPlayers; i++) {
	if (IS_HUMAN_IND(i)) {
	    sprintf(string + strlen(string),
		    "%s%s=%s@%s",
		    (first) ? "add players " : ",",
		    Players[i]->name,
		    Players[i]->realname,
		    Players[i]->hostname);
	    if (BIT(World.rules->mode, TEAM_PLAY)) {
		sprintf(status,"{%d}",Players[i]->team);
		strcat(string,status);
	    }

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
static void Main_Loop(void)
{
    extern void		Loop_delay(void);

#ifndef SILENT
    printf("Server runs at %d frames per second\n", framesPerSecond);
#endif

    plockserver(true);

    SetTimeout(0, 0);

    while (NoQuit
	   || NumPlayers - NumPseudoPlayers > NumRobots
	   || NoPlayersEnteredYet) {

	main_loops++;

	if (NumPlayers - NumPseudoPlayers == NumRobots && !RawMode) {
	    block_timer();
	    plockserver(false);
	    Wait_for_new_players();
	    plockserver(true);
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
	    if ((main_loops & 0xF) == 0) {
		Send_meta_server(0);
	    }
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
#define CHECK_FOR_NEW_PLAYERS	4

    int			new_players = false;
    long		delta;
#if _SEQUENT_
    timeval_t		tv0, tv1;
#else
    struct timeval	tv0, tv1;
#endif

    gettimeofday(&tv0, NULL);
    while (new_players == false) {
	if (login_in_progress > 0) {
	    if (Input() > 0) {
		NoPlayersEnteredYet = false;
		new_players = true;
		break;
	    }
	    SetTimeout(0, 10*1000);
	} else {
	    if (ShutdownServer >= 0) {
		SetTimeout(1, 0);
	    } else {
		SetTimeout(CHECK_FOR_NEW_PLAYERS, 0);
	    }
	}
	if (SocketReadable(Socket) != 0) {
	    Contact();
	}
	Send_meta_server(0);
	gettimeofday(&tv1, NULL);
	delta = 1000*1000 * (long)(tv1.tv_sec - tv0.tv_sec)
		- ((long)tv1.tv_usec - (long)tv0.tv_usec);
	delta = delta * FPS / (1000*1000);
	if (delta > 0) {
	    loops += delta;
	    tv0 = tv1;
	    if (ShutdownServer >= 0) {
		if ((ShutdownServer -= delta) <= 0) {
		    ShutdownServer = 0;
		    End_game();
		}
	    }
	}
    }
    SetTimeout(0, 0);
}

/*
 *  Last function, exit with grace.
 */
void End_game(void)
{
    player		*pl;
    int			len;
    char		msg[MSG_LEN];

    if (ShutdownServer == 0) {
	errno = 0;
	error("Shutting down...");
	sprintf(msg, "shutting down: %s", ShutdownReason);
    } else {
	sprintf(msg, "server exiting");
    }

    while (NumPlayers > 0) {	/* Kick out all remaining players */
	pl = Players[NumPlayers - 1];
	if (pl->conn == NOT_CONNECTED) {
	    Delete_player(NumPlayers - 1);
	} else {
	    Destroy_connection(pl->conn, msg);
	}
    }

    /* Tell meta server that we are gone */
    if (reportToMetaServer) {
	sprintf(msg, "server %s\nremove", Server.host);
	len = strlen(msg) + 1;
	DgramSend(Socket, meta_address, META_PORT, msg, len);
	if (meta_address_two[0] != '\0'
	    && strcmp(meta_address, meta_address_two)) {
	    DgramSend(Socket, meta_address_two, META_PORT, msg, len);
	}
    }

    DgramClose(Socket);
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
 * to a non-empty team which has space.
 *
 * If there is none or only one team with playing (i.e. non-paused)
 * players the player will be assigned to a randomly chosen empty team.
 *
 * If there is more than one team with playing players,
 * the player will be assigned randomly to a team which
 * has the least number of playing players.
 *
 * If all non-empty teams are full, the player is assigned
 * to a randomly chosen available team.
 *
 * Note:  Team zero is not part of this algorithm as that
 * team is reserved for the robots.
 */
static int Pick_team(void)
{
    int			i,
			least_players,
			num_available_teams = 0,
			playing_teams = 0;
    player		*pl;
    int			playing[MAX_TEAMS];
    int			free_bases[MAX_TEAMS];
    int			available_teams[MAX_TEAMS];

    memset(playing, 0, sizeof playing);
    memset(free_bases, 0, sizeof free_bases);
    memset(available_teams, 0, sizeof available_teams);

    for (i = 0; i < MAX_TEAMS; i++) {
	free_bases[i] = World.teams[i].NumBases - World.teams[i].NumMembers;
    }
    for (i = 0; i < NumPlayers; i++) {
	pl = Players[i];
	if (IS_TANK_PTR(pl)) {
	    continue;
	}
	if (BIT(pl->status, PAUSE)) {
	    continue;
	}
	if (!playing[pl->team]++) {
	    playing_teams++;
	}
    }
    if (playing_teams <= 1) {
	for (i = 1; i < MAX_TEAMS; i++) {
	    if (!playing[i] && free_bases[i] > 0) {
		available_teams[num_available_teams++] = i;
	    }
	}
    } else {
	least_players = NumPlayers;
	for (i = 1; i < MAX_TEAMS; i++) {
	    if (playing[i] > 0) {
		if (free_bases[i] > 0) {
		    if (playing[i] < least_players) {
			least_players = playing[i];
		    }
		}
	    }
	}

	for (i = 1; i < MAX_TEAMS; i++) {
	    if (free_bases[i] > 0) {
		if (least_players == NumPlayers
		    || playing[i] == least_players) {
		    available_teams[num_available_teams++] = i;
		}
	    }
	}
    }

    if (num_available_teams > 0) {
	return available_teams[rand() % num_available_teams];
    }

    /*NOTREACHED*/
    return TEAM_NOT_SET;
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


void Contact(void)
{
    int			i,
			team,
			bytes,
			delay,
			max_robots,
			login_port,
			status,
			reply_to;
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
    if ((bytes = DgramReceiveAny(Socket, ibuf.buf, ibuf.size)) <= 8) {
	if (bytes < 0
	    && errno != EWOULDBLOCK
	    && errno != EAGAIN
	    && errno != EINTR) {
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
    strcpy(host_addr, DgramLastaddr());

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
    reply_to = (ch & 0xFF);	/* no sign extension. */

    /* ignore port for termified clients. */
    port = DgramLastport();

#ifdef PACKLOG
#include "packlog.c"
#endif

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

    if (reply_to & PRIVILEGE_PACK_MASK) {
	long			key;
	static long		credentials;

	if (!credentials) {
	    credentials = (time(NULL) * (time_t)getpid());
	    credentials ^= (long)Contact;
	    credentials	+= (long)key + (long)&key;
	    credentials ^= (long)rand() << 1;
	}
	if (Packet_scanf(&ibuf, "%ld", &key) <= 0) {
	    return;
	}
	if (!Owner(reply_to, real_name, host_addr, key == credentials)) {
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
    switch (reply_to & 0xFF) {

    case ENTER_GAME_pack:	{
	/*
	 * Someone wants to enter the game.
	 */
	if (Packet_scanf(&ibuf, "%s%s%s%d", nick_name, disp_name, host_name,
			 &team) <= 0) {
	    D(printf("Incomplete login from %s@%s", real_name, host_addr);)
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
	    printf("Invalid name (nick=\"%s\", real=\"%s\", host=\"%s\")\n",
		  nick_name, real_name, host_addr);
#endif
	    status = E_INVAL;
	}

	/*
	 * Game locked?
	 */
	else if (game_lock) {
	    status = E_GAME_LOCKED;
	}

	/*
	 * Is the game full?
	 */
	else if (NumPlayers >= World.NumBases) {
	    status = E_GAME_FULL;

/* experimental code ahead. */
#if 1
	    /* maybe we can kick some paused player? */
	    for (i = 0; i < NumPlayers; i++) {
		if (Players[i]->conn != NOT_CONNECTED) {
		    if (BIT(Players[i]->status, PAUSE)) {
			sprintf(msg,
				"The paused player \"%s\" was kicked because the game is full.",
				 Players[i]->name);
			Destroy_connection(Players[i]->conn, "no pause with full game");
			Set_message(msg);
			status = SUCCESS;
			break;
		    }
		}
	    }
#endif
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
	    /* strip trailing whitespace. */
	    char *ptr;
	    for (ptr = &nick_name[strlen(nick_name)]; ptr-- > nick_name; ) {
		if (isascii(*ptr) && isspace(*ptr)) {
		    *ptr = '\0';
		} else {
		    break;
		}
	    }
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
					       host_addr, host_name,
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
	       real_name, host_addr);
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

	D(printf("Got CONTACT from %s.\n", host_addr));
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
	printf("%s@%s asked for an option list.\n",
	       real_name, host_addr);
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
	    WantedNumRobots = max_robots;
	    while (WantedNumRobots < NumRobots) {
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

/*
 * Return status for server
*/
static void Server_info(char *str, unsigned max_size)
{
    int			i, j, k;
    player		*pl, **order, *best = NULL;
    float		ratio, best_ratio = -1e7;
    char		name[MAX_CHARS];
    char		lblstr[MAX_CHARS];

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
	    (game_lock && ShutdownServer == -1) ? "locked" :
	    (!game_lock && ShutdownServer != -1) ? "shutting down" :
	    (game_lock && ShutdownServer != -1) ? "locked and shutting down" : "ok",
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
	if ((best == NULL
		|| ratio > best_ratio)
	    && !BIT(pl->status, PAUSE)) {
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
	if (IS_ROBOT_PTR(pl)) {
	    if ((k = Robot_war_on_player(GetInd[pl->id])) != -1) {
		sprintf(name + strlen(name), " (%s)", Players[GetInd[k]]->name);
		if (strlen(name) >= 19) {
		    strcpy(&name[17], ")");
		}
	    }
	}
	sprintf(lblstr, "%c%c %-19s%03d%6d",
		(pl == best) ? '*' : pl->mychar,
		(pl->team == TEAM_NOT_SET) ? ' ' : pl->team+'0',
		name, pl->life, (int)pl->score);
	sprintf(msg, "%2d... %-36s%s@%s\n",
		i+1, lblstr, pl->realname,
		IS_HUMAN_PTR(pl)
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
static bool Owner(char request, char *real_name, char *host_addr, int pass)
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
	&& (!strcmp(host_addr, meta_address)
	 || !strcmp(host_addr, meta_address_two))) {
	return true;
    }
#ifndef SILENT
    fprintf(stderr, "Permission denied for %s@%s, command 0x%02x, pass %d.\n",
	    real_name, host_addr, request, pass);
#endif
    return false;
}


static void Handle_signal(int sig_no)
{
    int			i;

    errno = 0;
    switch (sig_no) {
    case SIGALRM:
	error("First player has yet to show his butt, I'm bored... Bye!");
	DgramClose(Socket);
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

#ifndef VMS
    case SIGUSR1:
	error("Caught SIGUSR1, checking teams.");
	for (i = 0; i < MAX_TEAMS; i++) {
	    Check_team_members (i);
	    Check_team_treasures (i);
	}
	signal (SIGUSR1, Handle_signal);	/* Some o/s require this */
	return;
#endif

    default:
	error("Caught unkown signal: %d", sig_no);
	break;
    }

    End_game();

    exit(sig_no);	/* just in case */
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
	    if (IS_HUMAN_IND(i)) {
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
	    sprintf(msg,"Best team (%ld Pts): Team %d", maxsc, win);
	    Set_message(msg);
	    printf("%s\n", msg);
	}

	if (loose != -1 && loose != win) {
	    sprintf(msg,"Worst team (%ld Pts): Team %d", minsc, loose);
	    Set_message(msg);
	    printf("%s\n", msg);
	}
    }

    maxsc = -32767;
    minsc = 32767;
    win = loose = -1;

    for (i = 0; i < NumPlayers; i++) {
	SET_BIT(Players[i]->status, GAME_OVER);
	if (IS_HUMAN_IND(i)) {
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

/*
 * Verify that all source files making up this program have been
 * compiled for the same version.  Too often bugs have been reported
 * for incorrectly compiled programs.
 */
static void Check_server_versions(void)
{
    extern char		cmdline_version[],
			collision_version[],
			error_version[],
			event_version[],
			frame_version[],
			map_version[],
			math_version[],
			net_version[],
			netserver_version[],
			option_version[],
			play_version[],
			player_version[],
			robot_version[],
			rules_version[],
			saudio_version[],
			server_version[],
			socklib_version[],
			timer_version[],
			update_version[],
			walls_version[];
    static struct file_version {
	char		filename[16];
	char		*versionstr;
    } file_versions[] = {
	{ "cmdline", cmdline_version },
	{ "collision", collision_version },
	{ "error", error_version },
	{ "event", event_version },
	{ "frame", frame_version },
	{ "map", map_version },
	{ "math", math_version },
	{ "net", net_version },
	{ "netserver", netserver_version },
	{ "option", option_version },
	{ "play", play_version },
	{ "player", player_version },
	{ "robot", robot_version },
	{ "rules", rules_version },
	{ "saudio", saudio_version },
	{ "server", server_version },
	{ "socklib", socklib_version },
	{ "timer", timer_version },
	{ "update", update_version },
	{ "walls", walls_version },
    };
    int			i;
    int			oops = 0;

    for (i = 0; i < NELEM(file_versions); i++) {
	if (strcmp(VERSION, file_versions[i].versionstr)) {
	    oops++;
	    error("Source file %s.c (\"%s\") is not compiled "
		  "for the current version (\"%s\")!",
		  file_versions[i].filename,
		  file_versions[i].versionstr,
		  VERSION);
	}
    }
    if (oops) {
	error("%d version inconsistency errors, cannot continue.", oops);
	error("Please recompile this program properly.");
	exit(1);
    }
}

/*
 * Lock the server process data and code segments into memory
 * if this program has been compiled with the PLOCKSERVER flag.
 * Or unlock the server process if the argument is false.
 */
static void plockserver(int onoff)
{
#ifdef PLOCKSERVER
    int			op;

    if (onoff) {
	op = PROCLOCK;
    }
    else {
	op = UNLOCK;
    }
    if (plock(op) == -1) {
	error("Can't plock(%d)", op);
    }
#endif
}


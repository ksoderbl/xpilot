/* $Id: server.c,v 3.129 1996/05/13 20:31:47 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
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

#define SERVER
#include "types.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#if !defined(__hpux)
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
#include <pwd.h>
#ifndef VMS
#include <sys/param.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#ifdef PLOCKSERVER
# if defined(__linux__)
#  include <sys/mman.h>
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
#include "sched.h"
#include "net.h"
#include "netserver.h"
#include "error.h"
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
    "@(#)$Id: server.c,v 3.129 1996/05/13 20:31:47 bert Exp $";
#endif


/*
 * Global variables
 */
int			NumPlayers = 0;
int			NumQueuedPlayers = 0;
int			MaxQueuedPlayers = 20;
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
static void Check_server_versions(void);
static void Main_loop(void);
static int Enter_player(char *real, char *nick, char *disp, int team,
			char *addr, char *host, unsigned version, int port,
			int *login_port);
static void Queue_loop(void);
static int Queue_player(char *real, char *nick, char *disp, int team,
			char *addr, char *host, unsigned version, int port,
			int *qpos);
static void Handle_signal(int sig_no);
static void Server_info(char *, unsigned);
static void Contact(int fd, void *arg);
static int Check_address(char *addr);


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
    plock_server(pLockServer);           /* Lock the server into memory */
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
#ifndef SILENT
	printf("Locating Meta... "); fflush(stdout);
#endif
	signal(SIGALRM, catch_alarm);
	alarm(5);
	hinfo = gethostbyname(META_HOST);
	alarm(0);
	if (hinfo == NULL) {
	    strncpy(meta_address, META_IP, sizeof meta_address);
#ifndef SILENT
	    printf("1? "); fflush(stdout);
#endif
	} else {
	    strncpy(meta_address,
		    inet_ntoa(*((struct in_addr *)(hinfo->h_addr))),
		    sizeof meta_address);
#ifndef SILENT
	    printf("1 "); fflush(stdout);
#endif
	}
	meta_address[sizeof meta_address - 1] = '\0';

	signal(SIGALRM, catch_alarm);
	alarm(5);
	hinfo = gethostbyname(META_HOST_TWO);
	alarm(0);
	signal(SIGALRM, SIG_IGN);
	if (hinfo == NULL) {
	    strncpy(meta_address_two, META_IP_TWO, sizeof meta_address_two);
#ifndef SILENT
	    printf("2?\n"); fflush(stdout);
#endif
	} else {
	    strncpy(meta_address_two,
		    inet_ntoa(*((struct in_addr *)(hinfo->h_addr))),
		    sizeof meta_address_two);
#ifndef SILENT
	    printf("2\n"); fflush(stdout);
#endif
	}
	meta_address_two[sizeof meta_address_two - 1] = '\0';
    }

    /*
     * Get owner's login name.
     */
    pwent = getpwuid(geteuid());
    strcpy(Server.name, pwent->pw_name);
    

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
    SetTimeout(0, 0);
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
    install_input(Contact, Socket, 0);

    if (Setup_net_server() == -1) {
	End_game();
    }

    if (NoQuit) {
	signal(SIGHUP, SIG_IGN);
    } else {
	signal(SIGHUP, Handle_signal);
    }
    signal(SIGTERM, Handle_signal);
    signal(SIGINT, Handle_signal);
    signal(SIGPIPE, SIG_IGN);
#ifdef IGNORE_FPE
    signal(SIGFPE, SIG_IGN);
#endif

    /*
     * Set the time the server started
     */
    serverTime = time(NULL);

#ifndef SILENT
    printf("Server runs at %d frames per second\n", framesPerSecond);
#endif

    install_timer_tick(Main_loop, FPS);
    sched();
    printf("sched returned!?!?");
    End_game();
    return 1;
}

static void Main_loop(void)
{
    main_loops++;

    if ((main_loops & 0x3F) == 0) {
	Send_meta_server(0);
    }

    /*
     * Check for possible shutdown, the server will
     * shutdown when ShutdownServer (a counter) reaches 0.
     * If the counter is < 0 then no shutdown is in progress.
     */
    if (ShutdownServer >= 0) {
	if (ShutdownServer == 0) {
	    End_game();
	}
	else {
	    ShutdownServer--;
	}
    }

    Input();

    if (NumPlayers > NumRobots + NumPseudoPlayers || RawMode) {

	if (NoPlayersEnteredYet) {
	    if (NumPlayers > NumRobots + NumPseudoPlayers) {
		NoPlayersEnteredYet = false;
		if (gameDuration > 0.0) {
		    printf("Server will stop in %g minutes.\n", gameDuration);
		    gameOverTime = (time_t)(gameDuration * 60) + time((time_t *)NULL);
		}
	    }
	}

	Update_objects();

	if ((main_loops % UPDATES_PR_FRAME) == 0) {
	    Frame_update();
	}
    }

    if (!NoQuit
	&& NumPlayers == NumRobots + NumPseudoPlayers
	&& !login_in_progress
	&& !NumQueuedPlayers) {

	if (!NoPlayersEnteredYet) {
	    End_game();
	}
	if (serverTime + 5*60 < time(NULL)) {
	    error("First player has yet to show his butt, I'm bored... Bye!");
	    Log_game("NOSHOW");
	    End_game();
	}
    }

    Queue_loop();
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
int Pick_team(void)
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

static void Contact(int fd, void *arg)
{
    int			i,
			team,
			bytes,
			delay,
			login_port,
			max_robots,
    			qpos,
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
	nick_name[sizeof(nick_name) - 1] = '\0';
	disp_name[sizeof(disp_name) - 1] = '\0';
	host_name[sizeof(host_name) - 1] = '\0';

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
	nick_name[sizeof(nick_name) - 1] = '\0';
	disp_name[sizeof(disp_name) - 1] = '\0';
	host_name[sizeof(host_name) - 1] = '\0';

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
	if (!Kick_paused_players(TEAM_NOT_SET)) {
	    return E_GAME_FULL;
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
	if (team == TEAM_NOT_SET || team >= MAX_TEAMS || team <= 0) {
	    if (!teamAssign || (team = Pick_team()) == TEAM_NOT_SET)
		return E_TEAM_NOT_SET;
	}
	else if (World.teams[team].NumMembers >= World.teams[team].NumBases
		 && !Kick_paused_players(team)) {
	    return E_TEAM_FULL;
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

static void Queue_loop(void)
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
		|| (Kick_paused_players(TEAM_NOT_SET)
		    && NumPlayers - NumPseudoPlayers + login_in_progress < World.NumBases)) {

		/* find a team for this fellow. */
		if (BIT(World.rules->mode, TEAM_PLAY)) {

		    /* see if he has a reasonable suggestion. */
		    if (qp->team > 0 && qp->team < MAX_TEAMS) {
			if (World.teams[qp->team].NumMembers
			    >= World.teams[qp->team].NumBases) {
			    qp->team = TEAM_NOT_SET;
			}
		    }
		    if (qp->team == TEAM_NOT_SET) {
			qp->team = Pick_team();
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
    static int		queue_length = 0;


    if (!reportToMetaServer)
	return;

    currentTime = time(NULL);
    if (!change) {
	if (currentTime - lastMetaSendTime < GIVE_META_SERVER_A_HINT) {
	    if (NumQueuedPlayers == queue_length ||
		currentTime - lastMetaSendTime < 5) {
		return;
	    }
	}
    }
    lastMetaSendTime = currentTime;
    queue_length = NumQueuedPlayers;

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
	    "add queue %d\n"
	    "add sound " SOUND_SUPPORT_STR "\n",
	    Server.host, NumPlayers - NumPseudoPlayers - NumRobots - num_paused_players,
	    META_VERSION, World.name, World.x, World.y, World.author,
	    World.NumBases, FPS, contactPort,
	    (game_lock && ShutdownServer == -1) ? "locked"
		: (!game_lock && ShutdownServer != -1) ? "shutting down"
		: (game_lock && ShutdownServer != -1) ? "locked and shutting down"
		: "ok", World.NumTeamBases,
	    BIT(World.rules->mode, TIMING) ? 1:0,
	    time(NULL) - serverTime,
	    queue_length);


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
    errno = 0;

    switch (sig_no) {

    case SIGHUP:
	if (NoQuit) {
	    signal(SIGHUP, SIG_IGN);
	    return;
	}
	error("Caught SIGHUP, terminating.");
	End_game();
	break;
    case SIGINT:
	error("Caught SIGINT, terminating.");
	End_game();
	break;
    case SIGTERM:
	error("Caught SIGTERM, terminating.");
	End_game();
	break;

    default:
	error("Caught unkown signal: %d", sig_no);
	End_game();
	break;
    }

    _exit(sig_no);	/* just in case */
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

    lt = time(NULL);
    ptr = localtime(&lt);
    strftime(timenow,79,"%I:%M:%S %p %Z %A, %B %d, %Y",ptr);

    sprintf(str,"%-50.50s\t%10.10s@%-15.15s\tWorld: %-25.25s\t%10.10s\n",
	    timenow,
	    Server.name,
	    Server.host,
	    World.name,
	    heading);

    if ((fp = fopen(LOGFILE, "a")) == NULL) {	/* Couldn't open file */
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

    addr = inet_addr(str);
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
	    mask = inet_addr(slash + 1);
	    if (mask == (unsigned long) -1 && strcmp(slash + 1, "255.255.255.255")) {
		continue;
	    }
	    if (mask == 0) {
		continue;
	    }
	} else {
	    mask = 0xFFFFFFFF;
	}
	addr = inet_addr(tok);
	if (addr == (unsigned long) -1 && strcmp(tok, "255.255.255.255")) {
	    continue;
	}
	addr_mask_list[num_addr_mask].addr = addr;
	addr_mask_list[num_addr_mask].mask = mask;
	num_addr_mask++;
    }
    free(list);
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
			sched_version[],
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
	{ "sched", sched_version },
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

#if defined(PLOCKSERVER) && defined(__linux__)
/*
 * Patches for Linux plock support by Steve Payne <srp20@cam.ac.uk>
 * also added the -pLockServer command line option.
 * All messed up by BG again, with thanks and apologies to Steve.
 */
/* Linux doesn't seem to have plock(2).  *sigh* (BG) */
#if !defined(PROCLOCK) || !defined(UNLOCK)
#define PROCLOCK	0x01
#define UNLOCK		0x00
#endif
static int plock(int op)
{
#if defined(MCL_CURRENT) && defined(MCL_FUTURE)
    return op ? mlockall(MCL_CURRENT | MCL_FUTURE) : munlockall();
#else
    return -1;
#endif
}
#endif

/*
 * Lock the server process data and code segments into memory
 * if this program has been compiled with the PLOCKSERVER flag.
 * Or unlock the server process if the argument is false.
 */
int plock_server(int onoff)
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
	static int num_plock_errors;
	if (++num_plock_errors <= 3) {
	    error("Can't plock(%d)", op);
	}
	return -1;
    }
    return onoff;
#else
    if (onoff) {
	printf("Can't plock: Server was not compiled with plock support\n");
    }
    return 0;
#endif
}

void tuner_plock(void)
{
    pLockServer = (plock_server(pLockServer) == 1) ? true : false;
}

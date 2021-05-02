/* $Id: server.c,v 3.18 1993/08/02 12:55:35 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#ifndef __hpux
#include <sys/time.h>
#endif
#ifdef _SEQUENT_
#include <sys/procstats.h>
#define gettimeofday(T,X)	get_process_stats(T, PS_SELF, \
					(struct process_stats *)NULL, \
					(struct process_stats *)NULL)
#endif
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

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

#ifndef	lint
static char versionid[] = "@(#)$" TITLE " $";
static char sourceid[] =
    "@(#)$Id: server.c,v 3.18 1993/08/02 12:55:35 bjoerns Exp $";
#endif


/*
 * Global variables
 */
int			NumPlayers = 0;
int			NumPseudoPlayers = 0;
int			NumObjs = 0;
int			Num_alive = 0;
player			**Players;
object			*Obj[MAX_TOTAL_SHOTS];
long			Id = 1;		    /* Unique ID for each object */
long			GetInd[MAX_ID];
server			Server;
int			Shutdown = -1, ShutdownDelay = 1000;
int 			framesPerSecond = 18;

int			Argc;
char			**Argv;

static int		Socket;
static sockbuf_t	ibuf;
static char		msg[MSG_LEN];
static bool		Log = false;
static bool		NoPlayersEnteredYet = true;
static bool		lock = false;
static void		Wait_for_new_players(void);

extern int		login_in_progress;

void Send_meta_server(void);

int main(int argc, char *argv[])
{
    struct hostent *hinfo;
    struct passwd *pwent;

#ifdef __apollo
    signal(SIGFPE, SIG_IGN);
#endif
    Argc = argc; Argv = argv;
    init_error(argv[0]);
    srand(time((time_t *)0));		/* Take seed from timer. */
    Parser(argc, argv);
    if (FPS <= 0) {
	errno = 0;
	error("Can't run with %d frames per second, should be positive\n",
	    FPS);
	End_game();
    }
    Make_table();			/* Make trigonometric tables */
    Compute_gravity();
    Find_base_direction();

    /* Allocate memory for players, shots and messages */
    Alloc_players(World.NumBases + MAX_PSEUDO_PLAYERS);
    Alloc_shots(MAX_TOTAL_SHOTS);

    Make_ships();

    if (BIT(World.rules->mode, TEAM_PLAY)) {
	int i;
	for(i=0; i < World.NumTreasures; i++)
	    Make_ball(-1,
		      World.treasures[i].pos.x * BLOCK_SZ+(BLOCK_SZ/2),
		      World.treasures[i].pos.y * BLOCK_SZ+10,
		      false, i);
    }

    /*
     * Get server's official name.
     */
    gethostname(Server.host, 80);
    if ((hinfo = gethostbyname(Server.host)) == NULL) {
	error("gethostbyname %s", Server.host);
    } else {
	strcpy(Server.host, hinfo->h_name);
    }

    /*
     * Get owners login name.
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
    if ((Socket = CreateDgramSocket(contactPort)) == -1) {
	error("Could not create Dgram socket");
	End_game();
    }
    SetSocketNonBlocking(Socket, 1);
    if (Sockbuf_init(&ibuf, Socket, SERVER_SEND_SIZE,
		     SOCKBUF_READ | SOCKBUF_WRITE | SOCKBUF_DGRAM) == -1) {
	error("No memory for contact buffer");
	End_game();
    }

    SetTimeout(0, 0);

    if (Setup_net_server(World.NumBases) == -1) {
	End_game();
    }

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
	alarm(5*60);			/* Signal me in 5 minutes. */
	Wait_for_new_players();
	alarm(0);
	signal(SIGALRM, SIG_IGN);
	SetTimeout(0, 0);
    }
    signal(SIGHUP, Handle_signal);
    signal(SIGTERM, Handle_signal);
    signal(SIGINT, Handle_signal);
    
    Main_Loop();			    /* Entering main loop. */
    /* NEVER REACHED */
    return (-1);
}

void Send_meta_server(void)
{
    char string[MAX_STR_LEN];
    char status[MAX_STR_LEN];	

    Server_info(status, sizeof(status));	

    sprintf(string,
	    "add server %s\n"
	    "add users %d\n"
	    "add version %s\n"
	    "add map %s\n"
	    "add port %d\n"
	    "add status ",
	    Server.host, NumPlayers - NumRobots, 
	    VERSION, World.name, contactPort);
    if (strlen(string) + strlen(status) >= sizeof(string)) {
	/* Prevent string overflow */
	strcpy(&status[sizeof(string) - (strlen(string) + 2)], "\n");
    }
    strcat(string, status);

    DgramSend(Socket, META_HOST, META_PORT, string, strlen(string)+1);
}

/*
 * Main loop.
 */
void Main_Loop(void)
{
    extern void		Loop_delay(void);
    register int	i, x;
    int			main_loops = 0, lastLoops;
    time_t		currentTime, lastPlayerCheckTime = 0;
    time_t		lastMetaCheckTime = 0;
    bool		playerQuit;

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

	if (NumPlayers == NumRobots
	    || (login_in_progress && main_loops % 3 == 0)
	    || currentTime - lastPlayerCheckTime >= CHECK_FOR_NEW_PLAYERS) {
	    lastPlayerCheckTime = currentTime;

	    if (NumPlayers == NumRobots && !RawMode) {
		block_timer();
		Wait_for_new_players();
		allow_timer();
	    } else
		Check_new_players();
	}
	
	Update_objects();
	
	/*
	 * Check for possible shutdown, the server will
	 * shutdown when Shutdown (a counter) reaches 0.
	 * If the counter is < 0 then now shutdown is in progress.
	 */
	if (Shutdown >= 0) {
	    if (Shutdown == 0)
		End_game();
	    else
		Shutdown--;
	}
	
	if ((main_loops % UPDATES_PR_FRAME) == 0) {
	    Frame_update();
	    Loop_delay();
	}

	Check_client_input();
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
	if (login_in_progress == 0) {
	    SetTimeout(CHECK_FOR_NEW_PLAYERS, 0);
	} else {
	    SetTimeout(0, 100 * 1000);
	}
	if ((new_players = Check_new_players()) == false) {
	    gettimeofday(&tv, NULL);
	    milli_delta = (tv.tv_sec - start_time) * 1000 + tv.tv_usec / 1000;
	    loops = start_loops + (FPS * milli_delta) / 1000;
	}
    }
    SetTimeout(0, 0);
}

/*
 *  Last function, exit with grace.
 */
void End_game(void)
{
    int		i;
    player	*pl;
    char	string[50];

    if (Shutdown == 0) {
	error("Shutting down...");
    }

    while (NumPlayers > 0) {	/* Kick out all remaining players */
	pl = Players[NumPlayers - 1];
	if (pl->conn == NOT_CONNECTED) {
	    Delete_player(NumPlayers - 1);
	} else {
	    Destroy_connection(pl->conn, __FILE__, __LINE__);
	}
    }

    /* Tell meta serve that we are gone */
    sprintf(string,"server %s\nremove",Server.host);
    DgramSend(Socket, META_HOST, META_PORT, string, sizeof(string));

    SocketClose(Socket);
    Free_players();
    Free_ships();
    Free_shots();
    Free_map();
    Log_game("END");			    /* Log end */

    exit (0);
}


bool Check_new_players(void)
{
    int			i,
    			team,
    			bytes,
    			delay,
    			login_port;
    char		*in_host,
			status,
			reply_to;
    static bool		lock = false;
    bool		new_players = false;
    unsigned		magic,
			version,
			my_magic;
    unsigned short	port;
    char		real_name[MAX_CHARS],
			disp_name[MAX_CHARS],
			nick_name[MAX_CHARS],
			str[MAX_CHARS];


    switch (Check_new_connections())
    {
    case -1:
	/* Some connection error */
	break;
    case 0:
	/* Normal stuff */
	break;
    case 1:
	/* New player entered the game */
	NoPlayersEnteredYet = false;
	updateScores = true;
	new_players = true;
	break;
    }

    if (!SocketReadable(Socket))	/* No-one tried to connect. */
	return (new_players);

    /*
     * Someone connected to us, now try and deschiffer the message :)
     */
    Sockbuf_clear(&ibuf);
    errno = 0;
    if ((bytes = DgramReceiveAny(Socket, ibuf.buf, ibuf.size)) <= 0) {
	if (errno != EWOULDBLOCK && errno != EINTR) {
	    error("SocketRead (pack from %s)", DgramLastaddr());
	    GetSocketError(Socket);
	}
	return (new_players);
    }
    ibuf.len = bytes;

    /*
     * Get hostname.
     */
    in_host = DgramLastaddr();

    /*
     * Determine if we can talk with this hand-shake program.
     */
    if (Packet_scanf(&ibuf, "%u", &magic) <= 0
	|| (magic & 0xFFFF) != (MAGIC & 0xFFFF)) {
#ifndef	SILENT
	errno = 0;
	error("Incompatible packet received from %s", in_host);
#endif
	return (new_players);
    }
    version = MAGIC2VERSION(magic);

    /*
     * Read core of packet.
     */
    if (Packet_scanf(&ibuf, "%s%hu%c", real_name, &port, &reply_to) <= 0) {
#ifndef	SILENT
	errno = 0;
	error("Incomplete packet received from %s", in_host);
#endif
	return (new_players);
    }
    real_name[MAX_NAME_LEN - 1] = '\0';

    /*
     * Now see if we have the same (or compatible) version.
     * If the client request was only a contact request (to see
     * if there is a server running on this host) then we
     * don't care about version incompatibilities if the version
     * of the client is higher than ours, so that the client
     * can decide itself if it wants to adjust to our version.
     */
    if (version < MIN_CLIENT_VERSION
	|| (version > MAX_CLIENT_VERSION
	    && reply_to != CONTACT_pack)) {
#ifndef	SILENT
	errno = 0;
	error("Incompatible version with %s@%s (%04x,%04x)",
	    real_name, in_host, MY_VERSION, version);
#endif
	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%lu%c%c", MAGIC, reply_to, E_VERSION);
	DgramSend(Socket, in_host, port, ibuf.buf, ibuf.len);

	return (new_players);
    }
    if (version == 0x3020) {
	/*
	 * Support some older clients, which don't know
	 * that they can join the current version.
	 */
	my_magic = VERSION2MAGIC(0x3020);
    } else {
	my_magic = MAGIC;
    }

    status = SUCCESS;

    /*
     * Now decode the packet type field and do something witty.
     */
    switch (reply_to) {

    case ENTER_GAME_pack:	{
	/*
	 * Someone wants to enter the game.
	 */
	if (Packet_scanf(&ibuf, "%s%s%d", nick_name, disp_name, &team) <= 0) {
#ifndef	SILENT
	    errno = 0;
	    error("Incomplete login packet received from %s@%s",
		real_name, in_host);
	    return (new_players);
#endif
	}
	nick_name[MAX_NAME_LEN - 1] = '\0';
	disp_name[MAX_DISP_LEN - 1] = '\0';

	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%lu%c", my_magic, reply_to);

	/*
	 * Bad input parameters?
	 */
	if (nick_name[0] == 0
	    || real_name[0] == 0
	    || disp_name[0] == 0
	    || nick_name[0] < 'A'
	    || nick_name[0] > 'Z') {
	    status = E_INVAL;
	}

	/*
	 * Game locked?
	 */
	else if (lock && !Owner(real_name)) {
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
		    status = E_IN_USE;
		    break;
		}
	    }
	}

	/*
	 * Find a port for the client to connect to.
	 */
	if (status == SUCCESS
	    && (login_port = Setup_connection(real_name, nick_name, disp_name,
					      team, in_host, version)) == -1) {
	    status = E_SOCKET;
	}

	Packet_printf(&ibuf, "%c", status);

	if (status == SUCCESS) {
	    /*
	     * Tell the client which port to use for logging in.
	     */
	    Packet_printf(&ibuf, "%hu", login_port);
	}
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
	Packet_printf(&ibuf, "%lu%c%c", my_magic, reply_to, SUCCESS);
	Server_info(ibuf.buf + ibuf.len, ibuf.size - ibuf.len);
	ibuf.buf[ibuf.size - 1] = '\0';
	ibuf.len += strlen(ibuf.buf + ibuf.len) + 1;
    }
	break;

	
    case MESSAGE_pack:	{
	/*
	 * Someone wants to transmit a message to the server.
	 */

	if (!Owner(real_name)) {
	    status = E_NOT_OWNER;
	}
	else if (Packet_scanf(&ibuf, "%s", str) <= 0) {
	    status = E_INVAL;
	}
	else {
	    sprintf(msg,
		    " <<< MESSAGE FROM ABOVE (%s) >>> \"%s\"",
		    real_name, str);
	    Set_message(msg);
	}
	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%lu%c%c", my_magic, reply_to, status);
    }
	break;


    case LOCK_GAME_pack:	{
	/*
	 * Someone wants to lock the game so that no more players can enter.
	 */

	if (!Owner(real_name)) {
	    status = E_NOT_OWNER;
	} else {
	    lock = !lock;
	}
	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%lu%c%c", my_magic, reply_to, status);
    }
	break;

	
    case CONTACT_pack:	{
	/*
	 * Got contact message from client.
	 */

	D(printf("Got CONTACT from %s.\n", in_host));
	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%lu%c%c", my_magic, reply_to, status);
    }
	break;


    case SHUTDOWN_pack:	{
	/*
	 * Shutdown the entire server.
	 */

	if (!Owner(real_name)) {
	    status = E_NOT_OWNER;
	}
	else if (Packet_scanf(&ibuf, "%d%s", &delay, str) <= 0) {
	    status = E_INVAL;
	} else {
	    sprintf(msg, "|*******| %s (%s) |*******| \"%s\"",
		(delay > 0) ? "SHUTTING DOWN" : "SHUTDOWN STOPPED",
		real_name, str);
	    if (delay > 0) {
		Shutdown = delay * FPS;		/* delay is in seconds */;
		ShutdownDelay = Shutdown;
	    } else {
		Shutdown = -1;
	    }
	    Set_message(msg);
	}

	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%lu%c%c", my_magic, reply_to, status);
    }
	break;

	
    case KICK_PLAYER_pack:	{
	/*
	 * Kick someone from the game.
	 */
	int			found = -1;

	if (!Owner(real_name)) {
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
		    Destroy_connection(Players[found]->conn, __FILE__, __LINE__);
		}
		updateScores = true;
	    }
	}

	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%lu%c%c", my_magic, reply_to, status);
    }
	break;

	
    default:
	/*
	 * Incorrect packet type.
	 */
	errno = 0;
	error("Unknown packet type (%d) from %s@%s.\n",
	    reply_to, real_name, in_host);

	Sockbuf_clear(&ibuf);
	Packet_printf(&ibuf, "%lu%c%c", my_magic, reply_to, E_VERSION);
    }

    if (DgramSend(Socket, in_host, port, ibuf.buf, ibuf.len) == -1) {
	error("Could not send reply to %s@%s on port %d.",
	    real_name, in_host, port);
    }

    return new_players;
}

/*
 * Return status for server 
*/
void Server_info(char *str, unsigned max_size)
{
    int i, j, k;
    player *pl, **order, *best = NULL;
    float ratio, best_ratio;
    char name[MAX_CHARS];
    char lblstr[MAX_CHARS];

    sprintf(str,
	    "SERVER VERSION...: %s\n"
	    "STARTED BY.......: %s\n"
	    "STATUS...........: %s\n"
	    "MAX SPEED........: %d fps\n"
	    "WORLD (%3dx%3d)..: %s\n"
	    "      AUTHOR.....: %s\n"
	    "PLAYERS (%2d/%2d)..:\n",
	    TITLE,
	    Server.name,
	    (lock && Shutdown == -1) ? "locked" :
	    (!lock && Shutdown != -1) ? "shutting down" :
	    (lock && Shutdown != -1) ? "locked and shutting down" : "ok",
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
	ratio = (float) pl->score / (pl->life + 1);
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
	    && pl->robot_lock == LOCK_PLAYER) {
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
		? pl->dispname
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
bool Owner(char *name)
{
    if ((strcmp(name, Server.name)  == 0)
	|| (strcmp(name, "kenrsc")  == 0)
	|| (strcmp(name, "bjoerns") == 0)
	|| (strcmp(name, "root")    == 0))
	return (true);
    else
	return (false);
}


void Handle_signal(int sig_no)
{
    /* Tell meta serve that we are gone */
    char	string[50];
    sprintf(string,"server %s\nremove",Server.host);
    DgramSend(Socket, META_HOST, META_PORT, string, sizeof(string));
    
    switch (sig_no) {
    case SIGALRM:
	error("First player has yet to show his butt, I'm bored... Bye!");
	SocketClose(Socket);
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

    default:
	error("Caught unkown signal: %d", sig_no);
	break;
    }

    exit(sig_no);
}


void Log_game(char *heading)
{
#ifdef LOG
    char str[1024];
    FILE *fp;

    if (!Log)
	return;

    sprintf(str,
	    "%s (%s) - %s@%s, map '%s' (%d)\n",
	    heading, VERSION, Server.name, Server.host,
	    World.name, Server.max_num);

    if ((fp=fopen(LOGFILE, "a")) == NULL) {	/* Couldn't open file */
	error("Couldn't open log file, contact " LOCALGURU "");
	return;
    }

    fputs(str, fp);
    fclose(fp);
#endif
}

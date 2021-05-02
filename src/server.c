/* $Id: server.c,v 1.29 1992/08/27 00:26:12 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */
/* #define NO_status_OPTION /**/
/* #define NO_AUTO_REPEAT /**/
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>

#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/times.h>
#include <pwd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "global.h"
#include "version.h"
#include "map.h"
#include "pack.h"
#include "draw.h"
#include "robot.h"

#ifndef		CLK_TCK
#  ifndef 	HZ
#    define	CLK_TCK		60
#  else
#    define	CLK_TCK		HZ
#  endif
#endif

#ifndef	lint
static char versionid[] = "@(#)$" TITLE " $";
static char sourceid[] =
    "@(#)$Id: server.c,v 1.29 1992/08/27 00:26:12 bjoerns Exp $";
#endif


#define CHECK_DELAY	(FRAMES_PR_SEC) /* 0x3f */
#define	LOOP_DELAY	50

/*
 * Global variables
 */
int		NumPlayers = 0;
int		NumPseudoPlayers = 0;
int		NumObjs;
int		Num_alive = 0;
player		*Players[MAX_PLAYERS];
object		*Obj[MAX_TOTAL_SHOTS];
long		Id = 1;		    /* Unique ID for each object */
long		GetInd[MAX_ID];
server		Server;
int		RadarHeight;
int		Shutdown = -1, ShutdownDelay = 1000;
jmp_buf		SavedEnv;
int		Delay = LOOP_DELAY;

int	Argc;
char	**Argv;

static bool	Inside_window = true;
static int	Socket;
static pack_t	out;
static char	msg[MSG_LEN];
static bool	Log = true;
static bool	NoPlayersEnteredYet = true;



int main(int argc, char *argv[])
{
    struct hostent *hinfo;
    struct passwd *pwent;


    Argc = argc; Argv = argv;

    init_error(argv[0]);

    srand(time((time_t *)0));		/* Take seed from timer. */

    Parser(argc, argv);

    RadarHeight = (256.0/World.x) * World.y;

    Make_table();			/* Make trigonometric tables */

    Compute_gravity();

    /* Allocate memory for players, shots and messages */
    Alloc_players(World.NumBases+MAX_PSEUDO_PLAYERS);
    Alloc_shots(MAX_TOTAL_SHOTS);
    Alloc_msgs(MAX_MSGS);

    Make_ships();

    /*
     * Get server's official name.
     */
    gethostname(Server.host, 80);
    hinfo = gethostbyname(Server.host);
    strcpy(Server.host, hinfo->h_name);

    /*
     * Get owners login name.
     */
    pwent = getpwuid(geteuid());
    strcpy(Server.name, pwent->pw_name);

    /*
     * Initialize core of out packet.
     */
    out.core.magic = htonl(MAGIC);
    out.core.port = htonl(SERVER_PORT);
    strcpy(out.core.realname, Server.name);

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
    if ((Socket = CreateDgramSocket(SERVER_PORT)) == -1) {
	error("Could not create Dgram socket");
	End_game();
    }

    SetTimeout(0, 0);

    /*
     * Is the server in raw mode, that is - should it run even while
     * there are noe players logged in.
     */
    if (RawMode) {
	signal(SIGALRM, Handle_signal);	/* Get first client, then proceed. */
	alarm(5*60);			/* Signal me in 5 minutes. */
	while (Check_new_players() == false)
	    sleep(2);
	signal(SIGALRM, SIG_IGN);
    }
    signal(SIGHUP, Handle_signal);
    signal(SIGTERM, Handle_signal);
    signal(SIGINT, Handle_signal);
    
    Main_Loop();			    /* Entering main loop. */
    /* NEVER REACHED */
    return (0);
}


/*
 * Main loop.
 */
void Main_Loop()
{
    XEvent event;
    XClientMessageEvent *cmev;
    register int i, x;
    static int loops = 0;
    struct tms dummy_tms;
    clock_t oldtimes=times(&dummy_tms);

#ifndef SILENT
    printf("server running!\n");
#endif

    setjmp(SavedEnv);

    while (NoQuit
	   || NumPlayers-NumPseudoPlayers>NumRobots
	   || NoPlayersEnteredYet) {
	
	if ((loops = (loops+1) & CHECK_DELAY) == 0) {
	    if (NumPlayers == NumRobots && !RawMode) {
		while(Check_new_players() == false)
		    sleep(5);
	    } else
		Check_new_players();
	}
	
	{
	    clock_t newtimes;
	    if(((newtimes=times(&dummy_tms))-oldtimes) < CLK_TCK/FRAMES_PR_SEC)
		usleep(1000000/FRAMES_PR_SEC
		       - (newtimes-oldtimes)*1000000/CLK_TCK);
	    oldtimes = newtimes; 
	}

	Update_objects();
	
	if (Shutdown > 0)		/* Check for possible shutdown, the */
	    Shutdown--;			/* server will shutdown when Shutdown */
	else				/* (a counter) reaches 0.  If the */
	    if (Shutdown == 0)		/* counter is < 0 then now shutdown */
		End_game();		/* is in progress. */
	
	if ((loops % UPDATES_PR_FRAME) == 0) {
	    Draw_objects();
	}
	
	for (i=0; i<NumPlayers; i++) {
	    if (Players[i]->disp_type == DT_NONE)
		continue;
	    
	    for(x = XEventsQueued(Players[i]->disp,
				  QueuedAfterFlush); x>0; x--) {
		XNextEvent(Players[i]->disp, &event);
		
		switch (event.type) {

		case ClientMessage:
		    cmev = (XClientMessageEvent *) &event;
		    if (cmev->message_type == ProtocolAtom
			&& cmev->data.l[0] == KillAtom) {

			Quit(i);
                        continue;
		    }
		    break;

		case KeyPress:
		case KeyRelease:
		    Key_event(i, &event);
		    break;

		case ButtonPress:
		    Expose_button_window(i, BLACK, event.xbutton.window);

		    if (event.xbutton.window == Players[i]->info_b)
			Info(i, Players[i]->info_b);
		    if (event.xbutton.window == Players[i]->help_b)
			Help(i, Players[i]->help_b);
		    break;

		case ButtonRelease:
		    if (event.xbutton.window == Players[i]->quit_b) {
			Quit(i);
                        continue;
		    } else if (event.xbutton.window == Players[i]->info_close_b)
			Info(i, Players[i]->info_close_b);
		    else if (event.xbutton.window == Players[i]->help_close_b)
			Help(i, Players[i]->help_close_b);
		    else if (event.xbutton.window == Players[i]->help_next_b)
			Help(i, Players[i]->help_next_b);
		    else if (event.xbutton.window == Players[i]->help_prev_b)
			Help(i, Players[i]->help_prev_b);
		    break;

		case Expose:
		    if (event.xexpose.count > 0)	/* We don't want any */
			break;				/* subarea exposures. */

		    if (event.xexpose.window == Players[i]->players)
			Set_labels();
		    else if (event.xexpose.window == Players[i]->info_w)
			Expose_info_window(i);
		    else if (event.xexpose.window == Players[i]->help_w)
			Expose_help_window(i);
		    else  if (event.xexpose.window == Players[i]->radar)
			Draw_world_radar(i);
		    else
			Expose_button_window(i, RED, event.xexpose.window);
		    break;

		case EnterNotify:
		    Inside_window = true;
		    XAutoRepeatOff(Players[i]->disp);
		    Players[i]->turnacc = 0.0;
		    break;

		case UnmapNotify:
		case LeaveNotify:
		    Inside_window = false;
		    XAutoRepeatOn(Players[i]->disp);
		    break;

		case NoExpose:
		    break;	/* XXX? */

		case MappingNotify:
		    XRefreshKeyboardMapping(&event.xmapping);
		    break;

		default:
		    error("Got unexpected event type: %d", event.type);
		    break;
		}
	    }
	}
    }

    End_game();
}



/*
 *  Last function, exit with grace.
 */
void End_game(void)
{
    int i;


    if (Shutdown == 0) {
	error("Shutting down...");
    }

    while (NumPlayers > 0)	/* Kick out all remaining players */
	Quit(NumPlayers-1);

    SocketClose(Socket);
    Free_players();
    Free_ships();
    Free_shots();
    Free_map();
    Free_msgs();
    Log_game("END");			    /* Log end */

    exit (0);
}



void Dump_pack(core_pack_t *p)
{
    printf("\nDUMP OF PACK:\n");
    printf("=============\n");
    printf("TYPE:	%d\n", p->type);
    printf("REALNAME:	%s\n", p->realname);
    printf("PORT:	%ld\n", ntohl(p->port));
    printf("MAGIC:	%lx\n", ntohl(p->magic));
}



bool Check_new_players(void)
{
    int			i,
    			out_size = 0,
    			bytes;
    char		*in_host;
    XKeyboardState	settings;
    static bool		lock = false;
    bool		new_player = false,
    			answer = false;
    char		*str;
    pack_t		in;
    player		*pl;

    /*
     * Anyone cheating by turning auto-fire (also called auto-repeat :) on?
     */
#ifndef NO_AUTO_REPEAT
    for (i=0; i<NumPlayers; i++) {
	if (Players[i]->disp_type == DT_NONE)
	    continue;
	if (!BIT(Players[i]->status, PAUSE) && Inside_window) {
	    XGetKeyboardControl(Players[i]->disp, &settings);

	    if (settings.global_auto_repeat == AutoRepeatModeOn)
		XAutoRepeatOff(Players[i]->disp);
	}
    }
#endif

    if (!SocketReadable(Socket))	/* No-one tried to connect. */
	return (false);


    /*
     * Someone connected to us, now try and deschiffer the message :)
     */
    if ((bytes = DgramReceiveAny(Socket, (char *)&in, sizeof(pack_t))) <= 0) {
	error("SocketRead (pack from %s)", DgramLastaddr());
	return (false);
    }

    /*
     * Get hostname.
     */
    in_host = DgramLastaddr();


    /*
     * Determine if we can talk with this hand-shake program.
     */
    if (ntohl(in.core.magic) != MAGIC) {
#ifndef	SILENT
	error("Incompatible packet received from %s", in_host);
#endif
	return (false);
    }


    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
     *
     * Now decode the packet type field and do something witty.
     * (Note, s and r is short for send and reply.)
     *
     */
    out.core.type	= CORE_pack;
    out.core.status	= SUCCESS;
    answer		= true;
    out_size		= sizeof(core_pack_t);

    switch (in.core.type) {

    case ENTER_GAME_pack:	{
	/*
	 * Someone wants to enter the game.
	 */
	reply_pack_t		*s = &out.reply;
	enter_game_pack_t	*r = &in.enter;

	/*
	 * Game locked?
	 */
	if (lock && !Owner(r->realname)) {
	    s->status = E_GAME_LOCKED;
	    goto switch_end;
	}
	
	/*
	 * Is the game full?
	 */
	if (NumPlayers >= World.NumBases) {
	    s->status = E_GAME_FULL;
	    goto switch_end;
	}

	Init_player(NumPlayers);
	pl = Players[NumPlayers];

	strcpy(pl->name, r->nick);
	strcpy(pl->realname, r->realname);
	
	/*
	 * Now initialize X.
	 */
	if ((pl->disp = XOpenDisplay(r->display)) == NULL) {	/* Open the */
	    s->status = E_DISPLAY;				/* display */
	    goto switch_end;
	}

	/*
	 * Get X defaults.
	 */
	Get_defaults(NumPlayers);
	if (htons(r->team) != TEAM_NOT_SET)
	    pl->team = htons(r->team);


	/*
	 * All names must be unique (so we don't kick the wrong one for
	 * instance).
	 */
	for (i=0; i<NumPlayers; i++) {
	    if (strcasecmp(Players[i]->name, pl->name) == 0) {
		s->status = E_IN_USE;
		XCloseDisplay(pl->disp);
		goto switch_end;
	    }
	}

	/*
	 * Now initialize all the windows.
	 */
	if ((s->status = Init_window(NumPlayers)) == SUCCESS) {

#ifndef	SILENT
	    printf("%s (%d, %s) starts at startpos %d.\n",
		   Players[NumPlayers]->name, NumPlayers+1,
		   Players[NumPlayers]->realname,
		   Players[NumPlayers]->home_base);
#endif
	    if (NumPlayers == 0)
		sprintf(msg, "Welcome to \"%s\", made by %s.",
			World.name, World.author);
	    else
		sprintf(msg, "%s (%s) has entered \"%s\", made by %s.",
			Players[NumPlayers]->name,
			Players[NumPlayers]->realname,
			World.name, World.author);

	    NumPlayers++;
	    Id++;
	    new_player = true;
	    NoPlayersEnteredYet = false;
	
	    Set_message(msg);
	    Set_label_strings();
	    
	    /* Remebers the maximum number of players */
	    Server.max_num = MAX(Server.max_num, NumPlayers);
	    
	} else {		/* Couldn't initialize X, explain to user. */

	    new_player = false;
	}
    }
	break;

#ifndef NO_status_OPTION
    case REPORT_STATUS_pack:	{
	/*
	 * Someone asked for information.
	 */
	reply_pack_t		*s = &out.reply;
	report_status_pack_t	*r = &in.command;

#ifndef	SILENT
	printf("%s asked for info about current game.\n", r->realname);
#endif
	sprintf(s->str,
		"\nSERVER VERSION...: %s\n"
		"STARTED BY.......: %s\n"
		"STATUS...........: %s\n"
		"MAX SPEED........: %d frames/second\n"
		"WORLD (%3dx%3d)..: %s\n"
		"      AUTHOR.....: %s\n"
		"PLAYERS (%2d/%2d)..:\n",
		TITLE, Server.name, lock && Shutdown==-1 ? "Locked " :
		!lock && Shutdown!=-1 ? "Shutting down" :
		lock && Shutdown!=-1 ? "Locked and shutting down" :
		"Clear", FRAMES_PR_SEC,
		World.x, World.y, World.name, World.author,
		NumPlayers, World.NumBases);

	if (i > 0)
	    strcat(s->str,
		   "\nNO:  TM: NAME:             LIFE:   SC:    PLAYER:\n"
		   "-------------------------------------------------\n");
	
	for (i=0; i<NumPlayers; i++) {
	    sprintf(msg, "%2d... %-36s%s@%s\n",
		    i+1, Players[i]->lblstr, Players[i]->realname,
		    Players[i]->robot_mode == RM_NOT_ROBOT
		    ? DisplayString(Players[i]->disp)
		    : "noplace:0");
	    if (strlen(msg) + strlen(s->str) >= MAX_STR_LEN)
		break;
	    strcat(s->str, msg);
	}
	out_size += strlen(s->str);
    }
	break;
#endif /* NO_status_OPTION */

	
    case MESSAGE_pack:	{
	/*
	 * Someone wants to transmit a message to the server.
	 */
	reply_pack_t	*s = &out.reply;
	message_pack_t	*r = &in.command;

	if (!Owner(r->realname)) {
	    s->status = E_NOT_OWNER;
	} else {
	    sprintf(msg,
		    "	      <<< MESSAGE FROM ABOVE (%s) >>>	    \"%s\"",
		    r->realname, r->arg_str);
	    Set_message(msg);
	}
    }
	break;


    case LOCK_GAME_pack:	{
	/*
	 * Someone wants to lock the game so that no more players can enter.
	 */
	reply_pack_t		*s = &out.reply;
	lock_game_pack_t	*r = &in.command;

	if (!Owner(r->realname)) {
	    s->status = E_NOT_OWNER;
	} else {
	    lock = !lock;
	}
    }
	break;

	
    case CONTACT_pack:	{
	/*
	 * Got contact message from client.
	 */
	reply_pack_t	*s = &out.reply;
	contact_pack_t	*r = &in.core;

	D(printf("Got CONTACT from %s.\n", in_host));
    }
	break;


    case SHUTDOWN_pack:	{
	/*
	 * Shutdown the entire server.
	 */
	reply_pack_t	*s = &out.reply;
	shutdown_pack_t	*r = &in.command;

	if (!Owner(r->realname)) {
	    s->status = E_NOT_OWNER;
	    goto switch_end;
	}

	Shutdown = ntohl(r->arg_int);

	if (Shutdown == 0) {
	    sprintf(msg, "|*******| SHUTDOWN STOPPED (%s) |*******| \"%s\"",
		    r->realname, r->arg_str);
	    Shutdown = -1;
	} else {
	    for (i=0; i<NumPlayers; i++) {
		if (Players[i]->disp_type == DT_NONE)
		    continue;
		XMapWindow(Players[i]->disp, Players[i]->top);
	    }
	    sprintf(msg, "|*******| SHUTTING DOWN (%s) |*******| \"%s\"",
		    r->realname, r->arg_str);

	    ShutdownDelay = Shutdown;
	}
	Set_message(msg);
    }
	break;

	
    case KICK_PLAYER_pack:	{
	/*
	 * Kick someone from the game.
	 */
	reply_pack_t		*s = &out.reply;
	kick_player_pack_t	*r = &in.command;
	byte			found = -1;

	if (!Owner(r->realname)) {
	    s->status = E_NOT_OWNER;
	    goto switch_end;
	}

	for (i=0; i<NumPlayers; i++) {
	    if (strcasecmp(r->arg_str, Players[i]->name) == 0
		|| strcasecmp(r->arg_str, Players[i]->realname) == 0)
		found = i;
	}
	if (found == -1) {
	    s->status = E_NOT_FOUND;
	} else {
	    sprintf(msg, "\"%s\" upset the gods and was kicked out "
		    "of the game.", Players[found]->name);
	    Set_message(msg);
	    Quit(found);
	    goto switch_end;
	}
    }
	break;

	
    default:
	/*
	 * Incorrect packet type.
	 */
	answer	= false;

	error("Incorrect packet type from %s.\n"
	      "(Probably occured due to incompatibility between handshake\n"
	      "version and server version, someone should upgrade)", in_host);
    }

 switch_end:
    if (answer && (DgramSend(Socket, in_host, ntohl(in.core.port),
			     (char *)&out, out_size)) == -1) {
	error("Could not send request to client at %s.", in_host);
    }

    return (new_player);
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
    switch (sig_no) {
    case SIGALRM:
	error("First player has yet to show his butt, I'm bored... Bye!");
	SocketClose(Socket);
	break;

    case SIGHUP:
    case SIGINT:
    case SIGTERM:
	error("Caught signal %d, terminating.", sig_no);
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
#ifndef NO_LOG
    char str[1024];
    FILE *fp;


    if (!Log)
	return;

/*  sprintf(str,
	    "echo \"*** %s ***\n\n'%s' started a server on '%s' with map:\n"
	    "'%s.'\""
	    "| mailx -s \"%s\" %s 2>/dev/null >/dev/null",
	    NAME " " VERSION, Server.name, Server.host,
	    World.name, heading, REPORT_ADDRESS);
    system("/bin/mv dead.letter3 dead.letter4 2>/dev/null >/dev/null");
    system("/bin/mv dead.letter2 dead.letter3 2>/dev/null >/dev/null");
    system("/bin/mv dead.letter dead.letter2 2>/dev/null >/dev/null");
    system(str);
    sleep (3);
    system("/bin/rm -f dead.letter 2>/dev/null");
    printf("Logging...\n");
*/
    sprintf(str,
	    "%s (%s) - %s@%s, map '%s' (%d)\n",
	    heading, VERSION, Server.name, Server.host,
	    World.name, Server.max_num);

    if ((fp=fopen(LOGFILE, "a")) == NULL) { /* Couldn't open file, oh well. */
	error("Couldn't open log file, contact " LOCALGURU "");
	return;
    }

    fputs(str, fp);

    fclose(fp);
#endif
}

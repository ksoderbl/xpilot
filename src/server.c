/* server.c,v 1.3 1992/05/11 15:31:37 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include "version.h"
#include "pilot.h"
#include "map.h"
#include "pack.h"
#include "draw.h"
#include <X11/Shell.h>
#include <X11/Xproto.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <pwd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <setjmp.h>


#define CHECK_DELAY	100

/* Prototypes */

extern bool Draw_board();
extern void Draw_objects(void);
extern void Load_standard_map();
extern void Update_objects(void);
extern void Expose_quit(int,int);
extern void Expose_info_window(int);
extern void Set_labels(void);
extern void Alloc_msgs(int);

bool Check_new_players();

void Insert_player();
void Delete_player(int);
void Alloc_players(int);
void Init_player(int, def_t *);
void Alloc_shots(int);
void Main_Loop();
void Done_waiting();
void Make_table();
void Parser(int, char **);
void Key_event(int, XEvent *);
void sleepms(int);
void Log_game();
void End_game();
bool Owner(char *);
void Free_players(int), Free_map(void);
void Free_ships(void), Free_shots(int), Free_msgs(int);

extern World_map    World;
extern Atom	    Protocol_atom, Kill_atom;

/* Global variables */

int		Socket;
int		Antall=0;
int		Ant_Shots;
int		Ant_alive=0;
player		*Players[MAX_PLAYERS];
object		*Shots[MAX_TOTAL_SHOTS];
static char	msg[MSG_LEN];
long		Id=1;		    /* Unique ID for each object */
long		get_ind[MAX_ID];
server		Server;
static bool	Log=true;
bool		Inside_window=true;
int		Radar_height;
int		Shutdown=-1;
jmp_buf		Saved_env;



main(int argc, char *argv[])
{
    struct hostent *hinfo;
    struct passwd *pwent;
#ifdef	USE_IP_NUMBER
    char tmp[16];
    int i;
#endif


    if ((Socket = CreateDgramSocket(PORT_NR)) == -1) {
	perror("Could not create socket");
	exit(1);
    }

    srand(time((time_t *)0));		/* Take seed from timer. */

    Parser(argc, argv);

    Radar_height = (256.0/World.x) * World.y;

    Make_table();			/* Make trigonometric tables */

    Compute_gravity();
    Alloc_players(World.Ant_start);	/* Allocate memory for players */
    Alloc_shots(MAX_TOTAL_SHOTS);	/* Allocate memory for shots */
    Alloc_msgs(MAX_MSGS);
    Make_ships();

    /*
     * Get servers IP-address and owners logname.
     */
    gethostname(Server.host, 80);
    hinfo = gethostbyname(Server.host);
#ifdef	USE_IP_NUMBER
    Server.host[0]='\0';
    for (i=0; i<hinfo->h_length; i++) {
	sprintf(tmp, "%d.", (unsigned char)hinfo->h_addr[i]);
	strcat(Server.host, tmp);
    }
    Server.host[strlen(Server.host)-1]='\0';
#else
    strcpy(Server.host, hinfo->h_name);
#endif

    pwent = getpwuid(geteuid());
    strcpy(Server.name, pwent->pw_name);


    /*
     * Log, if enabled.
     */
    if ((strcmp(Server.name, "kenrsc") == 0) ||
	(strcmp(Server.name, "bjoerns") == 0))
	Log = false;
    Log_game("START");			/* Log start */


    SetTimeout(0, 0);

    signal(SIGALRM, Done_waiting);	/* Get first client, then proceed. */
    alarm(5*60);			/* Signal me in 5 minutes. */
    while (Check_new_players() == false)
	sleep(1);
    signal(SIGALRM, SIG_IGN);

    Main_Loop();			    /* Entering main loop. */
    /* NEVER REACHED */
    return (0);
}



void Main_Loop()
{
    XEvent event;
    XClientMessageEvent *cmev;
    register int i, x;
    static unsigned long loops = 0;


    while (Antall > 0) {		/* While there are still players on */

	setjmp(Saved_env);

	if (!(loops %= CHECK_DELAY))
	    Check_new_players();

	Update_objects();

	if (Shutdown>0)
	    Shutdown--;
	else
	    if (Shutdown==0)
		for (i=0; i<Antall; i++)	/* Shutdown server */
		    Quit(i);

	if (!(loops % ((1+(4*Antall))/5)))
	    Draw_objects();

	for (i=0; i<Antall; i++) 
	    for(x = XEventsQueued(Players[i]->disp, 
				  QueuedAfterFlush); x>0; x--) {
		XNextEvent(Players[i]->disp, &event);
		
		switch (event.type) {
		case ClientMessage:
		    cmev = (XClientMessageEvent *) &event;
		    if (cmev->message_type == Protocol_atom &&
			cmev->data.l[0] == Kill_atom) {
			D(printf("Got WM_DELETE_WINDOW from %s@%s.\n",
				 Players[i]->name, Players[i]->display);)
			Quit(i);
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
		    if (event.xbutton.window == Players[i]->quit)
			Quit(i);
		    else if (event.xbutton.window == Players[i]->info_close) 
			Info(i, Players[i]->info_close);
		    else if (event.xbutton.window == Players[i]->help_close) 
			Help(i, Players[i]->help_close);
		    else if (event.xbutton.window == Players[i]->help_next) 
			Help(i, Players[i]->help_next);
		    else if (event.xbutton.window == Players[i]->help_prev) 
			Help(i, Players[i]->help_prev);
		    goto end;
		    break;

		case Expose:
		    if (event.xexpose.window == Players[i]->players) 
			Set_labels();
		    else if (event.xexpose.window == Players[i]->info_w)
			Expose_info_window(i);
		    else if (event.xexpose.window == Players[i]->help_w)
			Expose_help_window(i);
		    else  if (event.xexpose.window == Players[i]->radar)
			Draw_world_radar(i);
		    else
			Expose_button_window(i,RED,event.xexpose.window);
		    break;

		case EnterNotify:
		    Inside_window = true;
		    XAutoRepeatOff(Players[i]->disp);
		    Players[i]->turnacc = 0.0;
		    break;

		case LeaveNotify:
		    Inside_window = false;
		    XAutoRepeatOn(Players[i]->disp);
		    break;

		case ResizeRequest:
		    XResizeWindow(Players[i]->disp, Players[i]->top,
				  1026, 768);
		    break;

		default:
		    printf("Got unexpected event (%d) (mappingnotify?)....\n",
			   event.type);
		    break;
		}
	    }
    end:
	;
    }

    End_game();
}



/*
 *  Last function, exit with grace.
 */
void End_game(void)
{
    SocketClose(Socket);
    Free_players(World.Ant_start);
    Free_ships();
    Free_shots(MAX_TOTAL_SHOTS);
    Free_map();
    Free_msgs(MAX_MSGS);
    Log_game("END");			    /* Log end */

    exit (0);
}



void Dump_pack(Pack *p)
{
    printf("\nDUMP OF PACK:\n");
    printf("=============\n");
    printf("TYPE:	%d\n", p->type);
    printf("REALNAME:	%s\n", p->realname);
    printf("HOSTNAME:	%s\n", p->hostname);
    printf("PORT:	%d\n", p->port);
    printf("DISPLAY:	%s\n", p->display);
    printf("TEAM:	%d\n", p->def.team);
    printf("STRING:\n%s\n\n", p->string);
}



bool Check_new_players()
{
    int pack, i, found;
    Pack new_client, rbuf;
    XKeyboardState  settings;
    bool new_player=false, answer=false;
    static bool lock = false;


    /*
     * Anyone cheating by turning auto-fire (also called auto-repeat :) on?
     */

    for (i=0; i<Antall; i++) {
	if (!BIT(Players[i]->status, PAUSE) && Inside_window) {
	    XGetKeyboardControl(Players[i]->disp, &settings);

	    if (settings.global_auto_repeat == AutoRepeatModeOn)
		XAutoRepeatOff(Players[i]->disp);
	}
    }


    if (!SocketReadable(Socket))	/* No-one tried to connect. */
	return false;



    /*
     * Someone connected to us, now try and deschiffer the message :)
     */

    if ((pack=DgramReceiveAny(Socket, (char *)&rbuf, sizeof(Pack))) == -1)
	perror("SocketRead");

    if (pack > 0) {
	memcpy(&new_client, (char *)&rbuf, sizeof(Pack));

	switch (new_client.type) {

	case ENTER_GAME:
	    /*
	     * Game locked?
	     */
	    if (lock && !Owner(new_client.realname)) {
		strcpy(new_client.string,"Sorry! Game locked by owner.\n");
		answer = true;
		new_client.type = LOCK;
		break;
	    }

	    /*
	     * A new player wants to register.
	     */
	    if (Antall >= World.Ant_start) {
		strcpy(new_client.string, "Sorry! game full.\n");
		new_client.type = GAME_FULL;
		answer = true;
		break;
	    }

	    new_player=true;

	    Id++;

	    Init_player(Antall, &(new_client.def));
	    strcpy(Players[Antall]->display, new_client.display);
	    strcpy(Players[Antall]->name, new_client.string);
	    strcpy(Players[Antall]->realname, new_client.realname);

	    Players[Antall]->id = Id;
	    get_ind[Id]=Antall;
	    
	    if (Draw_board(Antall, new_client.display) == true) {
		sprintf(new_client.string,
			"%s: You are player number %d. Maximum is %d.",
			Players[Antall]->name, Antall+1, World.Ant_start);

		printf("\"%s\" (%d, %s) starts at startpos %d.\n",
		       Players[Antall]->name, Antall+1,
		       Players[Antall]->realname, Players[Antall]->home_base);
		if (Antall == 0)
		    sprintf(msg, "Welcome to \"%s\", made by %s.",
			    World.name, World.author);
		else
		    sprintf(msg, "%s (%s) has entered \"%s\", made by %s.",
			    Players[Antall]->name, Players[Antall]->realname,
			    World.name, World.author);
		Set_message(msg);
		Antall++;
		Set_label_strings();


		/* Remebers the maximum number of players */
		Server.max_ant = MAX(Server.max_ant, Antall);
	    } else {
		sprintf(new_client.string,
			"Player no %d (%s): Couldn't open display...",
			Antall+1, Players[Antall]->name);
		new_player=false;
	    }
	    if (BIT(World.rules->mode, ONE_PLAYER_ONLY))
		SocketClose(Socket);
	    answer = new_player;
	    break;

	case REPORT_STATUS:
	    /*
	     * Someone asked for information.
	     */
	    printf("%s asked for info about current game.\n",
		   new_client.realname);
	    sprintf(new_client.string, "\nSERVER VERSION...: %s\n"
		    "STARTED BY.......: %s\n"
		    "STATUS...........: %s\n"
		    "WORLD (%3dx%3d)..: %s\n"
		    "	   AUTHOR.....: %s\n"
		    "PLAYERS (%2d/%2d)..:\n",
		    TITLE, Server.name, lock && Shutdown==-1 ? "Locked " :
			!lock && Shutdown!=-1 ? "Shutting down" :
			lock && Shutdown!=-1 ? "Locked and shutting down" :
			"Clear",
		    World.x, World.y, World.name, World.author,
		    Antall, World.Ant_start);

	    if (i>0)
		strcat(new_client.string,
		       "\nNO:  TM: NAME:	      LIFE:   SC:   PLAYER:\n"
		       "-------------------------------------------------\n");

	    for (i=0; i<Antall; i++) {
		sprintf(msg, "%2d... %-36s%s@%s\n",
			i+1, Players[i]->lblstr, Players[i]->realname,
			Players[i]->display);
		strcat(new_client.string, msg);
	    }
	    answer = true;
	    break;

	case MESSAGE:
	    answer = true;
	    if (!Owner(new_client.realname)) {
		sprintf(new_client.string, "You are not the owner, beat it!");
		break;
	    }
	    sprintf(msg, "	      <<< MESSAGE FROM ABOVE (%s) >>>	    "
		    "\"%s\"", new_client.realname, new_client.string);
	    Set_message(msg);
	    sprintf(new_client.string, "Message transmitted successfully.");
	    break;
	    
	case LOCK:
	    answer = true;
	    if (!Owner(new_client.realname)) {
		sprintf(new_client.string, "You are not the owner, beat it!");
		break;
	    }
	    if (lock) {
		lock = false;
		sprintf(new_client.string, "Game unlocked!");
	    } else {
		lock = true;
		sprintf(new_client.string, "Game locked!");
	    }
	    break;

	case CONTACT:
	    /*
	     * Got contact message from client.
	     */
	    strcpy(new_client.string, Server.host);
D(	    printf("Got CONTACT from %s.\n", new_client.hostname);  )
	    answer = true;
	    break;

	case SHUTDOWN:
	    /*
	     * Shutdown the entire server.
	     */
	    answer = true;
	    if (!Owner(new_client.realname)) {
		sprintf(new_client.string, "You are not the owner, beat it!");
		break;
	    } else {
		if (Shutdown >= 0) {
		    Shutdown = -1;
		    sprintf(msg, "|*******| SHUTDOWN STOPPED (%s) |*******| "
			    "\"%s\"", new_client.realname, new_client.string);
		    sprintf(new_client.string, "Shutdown stopped.");
		} else {
		    Shutdown = SHUTDOWN_DELAY;
		    for (i=0; i<Antall; i++)
			XMapWindow(Players[i]->disp, Players[i]->top);
		    sprintf(msg, "|*******| SHUTTING DOWN (%s) |*******| "
			    "\"%s\"", new_client.realname, new_client.string);
		    sprintf(new_client.string, "Shutdown started.");
		}
		Set_message(msg);
	    }
	    break;

	case KICK:
	    /*
	     * Kick someone from the game.
	     */
	    answer = true; found = -1;
	    if (!Owner(new_client.realname)) {
		sprintf(new_client.string, "You are not the owner, beat it!");
		break;
	    } else {
		for (i=0; i<Antall; i++) {
		    if ((strcmp(new_client.string, Players[i]->name) == 0) ||
			strcmp(new_client.string, Players[i]->realname) == 0)
			found = i;
		}
		if (found == -1) {
		    sprintf(new_client.string, "That player is not playing.");
		} else {
		    sprintf(msg, "\"%s\" upset the gods and was kicked out "
			    "of the game.", Players[found]->name);
		    Set_message(msg);
		    sprintf(new_client.string, "Operation successful.");
		    Quit(found);
		    break;
		}
	    }
	    break;

	default:
	    /*
	     * Incorrect packet type.
	     */
	    fprintf(stderr, "Incorrect packet type from \"%s\" (%s).\n",
		    new_client.string, new_client.realname);
	    fprintf(stderr, "TYPE: %d\n", new_client.type);
	    break;
	}

	if (answer &&
	    (pack=DgramSend(Socket, new_client.hostname, new_client.port,
			    &new_client, sizeof(Pack))) == -1) {
	    perror("Could not send request to client(SocketWrite)");
	    printf("(Probably occured due to incompatibility between\n");
	    printf("handshake version and server version, please upgrade.)\n");
	    Dump_pack(&new_client);
	    printf("Game may be corrupt, but will continue nevertheless.\n");
	}
    }

    return (new_player);
}



bool Owner(char *name)
{
    if ((strcmp(name, Server.name)==0) ||
	(strcmp(name, "kenrsc")==0) ||
	(strcmp(name, "bjoerns")==0) ||
	(strcmp(name, "root")==0))
	return true;
    else
	return false;
}


void Done_waiting(void)
{
    warn("First player has yet to show his butt, I'm bored... Bye!\n");
    SocketClose(Socket);
    exit(1);
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
	    World.name, Server.max_ant);

    if ((fp=fopen(LOGFILE, "a")) == NULL) { /* Couldn't open file, oh well. */
	perror("xpilots");
	fprintf(stderr, "(Couldn't open log file, contact " LOCALGURU ")\n");
	return;
    }

    fputs(str, fp);

    fclose(fp);
#endif
}

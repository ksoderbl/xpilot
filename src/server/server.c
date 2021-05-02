/* $Id: server.c,v 4.13 2000/03/12 12:11:11 bert Exp $
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
#include "NT/winSvrThread.h"
#include <time.h>
#include <limits.h>
#else
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#if !defined(__hpux) && !defined(_WINDOWS)
#include <sys/time.h>
#endif
#include <pwd.h>

#if !defined(VMS)
#include <sys/param.h>
#endif

#endif	/* _WINDOWS */

#ifdef PLOCKSERVER
# if defined(__linux__)
#  include <sys/mman.h>
# else
#  include <sys/lock.h>
# endif
#endif

#define	SERVER
#include "version.h"
#include "config.h"
#include "types.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "socklib.h"
#include "map.h"
#include "bit.h"
#include "sched.h"
#include "netserver.h"
#include "error.h"
#include "portability.h"
#include "server.h"

char server_version[] = VERSION;

#ifndef	lint
static char versionid[] = "@(#)$" TITLE " $";
static char sourceid[] =
    "@(#)$Id: server.c,v 4.13 2000/03/12 12:11:11 bert Exp $";
#endif

/*
 * Global variables
 */
int			NumPlayers = 0;
int			NumObjs = 0;
int			NumPulses = 0;
int			NumEcms = 0;
int			NumTransporters = 0;
player			**Players;
object			*Obj[MAX_TOTAL_SHOTS];
pulse_t			*Pulses[MAX_TOTAL_PULSES];
ecm_t			*Ecms[MAX_TOTAL_ECMS];
trans_t			*Transporters[MAX_TOTAL_TRANSPORTERS];
int			GetInd[NUM_IDS+1];
server			Server;
int			ShutdownServer = -1;
int			ShutdownDelay = 1000;
char			ShutdownReason[MAX_CHARS];
int 			framesPerSecond = 18;
long			main_loops = 0;		/* needed in events.c */

static int		serverSocket;
#ifdef LOG
static bool		Log = true;
#endif
static bool		NoPlayersEnteredYet = true;
int			game_lock = false;
time_t			gameOverTime = 0;
time_t			serverTime = 0;

extern int		login_in_progress;
extern int		NumQueuedPlayers;

static void Check_server_versions(void);
extern void Main_loop(void);
static void Handle_signal(int sig_no);

int main(int argc, char **argv)
{

    /*
     * Make output always linebuffered.  By default pipes
     * and remote shells cause stdout to be fully buffered.
     */
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
    setvbuf(stderr, NULL, _IOLBF, BUFSIZ);

    /*
     * --- Output copyright notice ---
     */

    xpprintf("  " COPYRIGHT ".\n"
	   "  " TITLE " comes with ABSOLUTELY NO WARRANTY; "
	      "for details see the\n"
	   "  provided LICENSE file.\n\n");

#ifdef SUNCMW
    cmw_priv_init();
#endif /* SUNCMW */
    init_error(argv[0]);
    srand(time((time_t *)0) * Get_process_id());
    Check_server_versions();
    if (!Parser(argc, argv))
#ifndef	_WINDOWS
		exit(0);
#else
		return(0);
#endif
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
    GetLocalHostName(Server.host, sizeof Server.host, (reportToMetaServer != 0));

    Get_login_name(Server.name, sizeof Server.name);

    /*
     * Log, if enabled.
     */
    Log_game("START");

    serverSocket = Contact_init();

    Meta_init(serverSocket);

    if (Setup_net_server() == -1) {
	End_game();
    }
#ifndef	_WINDOWS
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
#endif	/* _WINDOWS */
    /*
     * Set the time the server started
     */
    serverTime = time(NULL);

#ifndef SILENT
    xpprintf("%s Server runs at %d frames per second\n", showtime(), framesPerSecond);
#endif

#ifdef	_WINDOWS
    /* Windows returns here, we let the worker thread call sched() */
    install_timer_tick(ServerThreadTimerProc, FPS);
#else
    install_timer_tick(Main_loop, FPS);

    sched();
    xpprintf("sched returned!?");
    End_game();
#endif

    return 1;
}

void Main_loop(void)
{
    main_loops++;

    if ((main_loops & 0x3F) == 0) {
	Meta_update(0);
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
		    xpprintf("%s Server will stop in %g minutes.\n", showtime(), gameDuration);
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

    /* Tell meta server that we are gone. */
    Meta_gone();

    Contact_cleanup();

    Free_players();
    Free_shots();
    Free_map();
    Free_cells();
    Log_game("END");			    /* Log end */

#ifndef	_WINDOWS
    exit (0);
#endif
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
 * Prefer not to place players in the robotTeam if possible.
 */
int Pick_team(int pick_for_type)
{
    int			i,
			least_players,
			num_available_teams = 0,
			playing_teams = 0,
			losing_team;
    player		*pl;
    int			playing[MAX_TEAMS];
    int			free_bases[MAX_TEAMS];
    int			available_teams[MAX_TEAMS];
    long		team_score[MAX_TEAMS];
    long		losing_score;

    for (i = 0; i < MAX_TEAMS; i++) {
	free_bases[i] = World.teams[i].NumBases - World.teams[i].NumMembers;
	playing[i] = 0;
	team_score[i] = 0;
	available_teams[i] = 0;
    }
    if (restrictRobots) {
	if (pick_for_type == PickForRobot) {
	    if (free_bases[robotTeam] > 0) {
		return robotTeam;
	    } else {
		return TEAM_NOT_SET;
	    }
	}
    }
    if (reserveRobotTeam) {
	if (pick_for_type != PickForRobot) {
	    free_bases[robotTeam] = 0;
	}
    }

    /*
     * Find out which teams have actively playing members.
     * Exclude paused players and tanks.
     * And calculate the score for each team.
     */
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
	if (IS_HUMAN_PTR(pl) || IS_ROBOT_PTR(pl)) {
	    team_score[pl->team] += pl->score;
	}
    }
    if (playing_teams <= 1) {
	for (i = 0; i < MAX_TEAMS; i++) {
	    if (!playing[i] && free_bases[i] > 0) {
		available_teams[num_available_teams++] = i;
	    }
	}
    } else {
	least_players = NumPlayers;
	for (i = 0; i < MAX_TEAMS; i++) {
	    /* We fill teams with players first. */
	    if (playing[i] > 0 && free_bases[i] > 0) {
		if (playing[i] < least_players) {
		    least_players = playing[i];
		}
	    }
	}

	for (i = 0; i < MAX_TEAMS; i++) {
	    if (free_bases[i] > 0) {
		if (least_players == NumPlayers
		    || playing[i] == least_players) {
		    available_teams[num_available_teams++] = i;
		}
	    }
	}
    }

    if (!num_available_teams) {
	for (i = 0; i < MAX_TEAMS; i++) {
	    if (free_bases[i] > 0) {
		available_teams[num_available_teams++] = i;
	    }
	}
    }

    if (num_available_teams == 1) {
	return available_teams[0];
    }

    if (num_available_teams > 1) {
	losing_team = -1;
	losing_score = LONG_MAX;
	for (i = 0; i < num_available_teams; i++) {
	    if (team_score[available_teams[i]] < losing_score
		&& available_teams[i] != robotTeam) {
		losing_team = available_teams[i];
		losing_score = team_score[losing_team];
	    }
	}
	return losing_team;
    }

    /*NOTREACHED*/
    return TEAM_NOT_SET;
}


/*
 * Return status for server
*/
void Server_info(char *str, unsigned max_size)
{
    int			i, j, k;
    player		*pl, **order, *best = NULL;
    DFLOAT		ratio, best_ratio = -1e7;
    char		name[MAX_CHARS];
    char		lblstr[MAX_CHARS];
    char		msg[MSG_LEN];

    sprintf(str,
	    "SERVER VERSION...: %s\n"
	    "STATUS...........: %s\n"
	    "MAX SPEED........: %d fps\n"
	    "WORLD (%3dx%3d)..: %s\n"
	    "      AUTHOR.....: %s\n"
	    "PLAYERS (%2d/%2d)..:\n",
	    server_version,
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
	    ratio = (DFLOAT) pl->score;
	} else {
	    ratio = (DFLOAT) pl->score / (pl->life + 1);
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
		: "xpilot.org");
	if (strlen(msg) + strlen(str) >= max_size)
	    break;
	strcat(str, msg);
    }
    free(order);
}


static void Handle_signal(int sig_no)
{
    errno = 0;

#ifndef	_WINDOWS
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
#endif
    _exit(sig_no);	/* just in case */
}


void Log_game(const char *heading)
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

    if ((fp = fopen(Conf_logfile(), "a")) == NULL) {	/* Couldn't open file */
	error("Couldn't open log file, contact %s", Conf_localguru());
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
	    xpprintf("%s\n", msg);
	}

	if (loose != -1 && loose != win) {
	    sprintf(msg,"Worst team (%ld Pts): Team %d", minsc, loose);
	    Set_message(msg);
	    xpprintf("%s\n", msg);
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
	xpprintf("%s\n", msg);
    }
    if (loose != -1 && loose != win) {
	sprintf(msg,"Worst human player: %s", Players[loose]->name);
	Set_message(msg);
	xpprintf("%s\n", msg);
    }
}


/*
 * Verify that all source files making up this program have been
 * compiled for the same version.  Too often bugs have been reported
 * for incorrectly compiled programs.
 */
static void Check_server_versions(void)
{
    extern char		cannon_version[],
			cmdline_version[],
			collision_version[],
			error_version[],
			event_version[],
			frame_version[],
			id_version[],
			item_version[],
			map_version[],
			math_version[],
			metaserver_version[],
			net_version[],
			netserver_version[],
			option_version[],
			play_version[],
			player_version[],
			portability_version[],
			robot_version[],
			rules_version[],
			saudio_version[],
			server_version[],
			socklib_version[],
			sched_version[],
			ship_version[],
			shot_version[],
			update_version[],
			walls_version[];
    static struct file_version {
	char		filename[16];
	char		*versionstr;
    } file_versions[] = {
	{ "cannon", cannon_version },
	{ "cmdline", cmdline_version },
	{ "collision", collision_version },
	{ "error", error_version },
	{ "event", event_version },
	{ "frame", frame_version },
	{ "id", id_version },
	{ "item", item_version },
	{ "map", map_version },
	{ "math", math_version },
	{ "metaserver", metaserver_version },
	{ "net", net_version },
	{ "netserver", netserver_version },
	{ "option", option_version },
	{ "play", play_version },
	{ "player", player_version },
	{ "portability", portability_version },
	{ "robot", robot_version },
	{ "rules", rules_version },
	{ "saudio", saudio_version },
	{ "server", server_version },
	{ "socklib", socklib_version },
	{ "sched", sched_version },
	{ "ship", ship_version },
	{ "shot", shot_version },
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
	xpprintf("Can't plock: Server was not compiled with plock support\n");
    }
    return 0;
#endif
}

void tuner_plock(void)
{
    pLockServer = (plock_server(pLockServer) == 1) ? true : false;
}

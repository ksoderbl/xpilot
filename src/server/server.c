/* $Id: server.c,v 5.21 2002/02/13 17:09:18 dik Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WINDOWS
# include <unistd.h>
# ifndef __hpux
#  include <sys/time.h>
# endif
# include <pwd.h>
# include <sys/param.h>
#endif

#ifdef PLOCKSERVER
# if defined(__linux__)
#  include <sys/mman.h>
# else
#  include <sys/lock.h>
# endif
#endif

#ifdef _WINDOWS
# include <io.h>
# include "NT/winServer.h"
# include "NT/winSvrThread.h"
#endif

#define	SERVER
#include "version.h"
#include "xpconfig.h"
#include "types.h"
#include "serverconst.h"
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
#include "commonproto.h"

char server_version[] = VERSION;

#ifndef	lint
char xpilots_versionid[] = "@(#)$" TITLE " $";
#endif

/*
 * Global variables
 */
int			NumPlayers = 0;
int			NumAlliances = 0;
player			**Players;
int			GetInd_1;
int			GetInd[NUM_IDS+1];
server_t		Server;
char			*serverAddr;
int			ShutdownServer = -1;
int			ShutdownDelay = 1000;
char			ShutdownReason[MAX_CHARS];
int 			framesPerSecond = 18;
long			main_loops = 0;		/* needed in events.c */

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
    int			timer_tick_rate;
    char		*addr;

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

    init_error(argv[0]);
    Check_server_versions();

    seedMT((unsigned)time((time_t *)0) * Get_process_id());

    if (Parser(argc, argv) == FALSE) {
	exit(1);
    }

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

    Treasure_init();

    /*
     * Get server's official name.
     */
    if (serverHost) {
	addr = sock_get_addr_by_name(serverHost);
	if (addr == NULL) {
	    errno = 0;
	    error("Failed name lookup on: %s", serverHost);
	    return 1;
	}
	serverAddr = xp_strdup(addr);
	strlcpy(Server.host, serverHost, sizeof(Server.host));
    } else {
	sock_get_local_hostname(Server.host, sizeof Server.host,
				(reportToMetaServer != 0 &&
				 searchDomainForXPilot != 0));
    }

    Get_login_name(Server.owner, sizeof Server.owner);

    /*
     * Log, if enabled.
     */
    Log_game("START");

    if (!Contact_init())
		return(FALSE);

    Meta_init();

    if (Setup_net_server() == -1) {
	End_game();
    }
#ifndef _WINDOWS
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

    if (timerResolution > 0) {
	timer_tick_rate = timerResolution;
    }
    else {
	timer_tick_rate = FPS;
    }
#ifdef _WINDOWS
    /* Windows returns here, we let the worker thread call sched() */
    install_timer_tick(ServerThreadTimerProc, timer_tick_rate);
#else
    install_timer_tick(Main_loop, timer_tick_rate);

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
int End_game(void)
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
    Free_options();
    Log_game("END");			    /* Log end */

#ifndef _WINDOWS
    exit (0);
#endif
	return(FALSE);                  /* return FALSE so windows bubbles out of the main loop */
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
 *
 * TODO
*/
void Server_info(char *str, unsigned max_size)
{
    int			i, j, k;
    player		*pl, **order, *best = NULL;
    DFLOAT		ratio, best_ratio = -1e7;
    char		name[MAX_CHARS * 2 + 4];
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
    strlcat(str, msg, max_size);

    if ((order = (player **) malloc(NumPlayers * sizeof(player *))) == NULL) {
	error("No memory for order");
	return;
    }
    for (i = 0; i < NumPlayers; i++) {
	pl = Players[i];
	if (BIT(World.rules->mode, LIMITED_LIVES)) {
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
    for (i = 0; i < NumPlayers; i++) {
	pl = order[i];
	strlcpy(name, pl->name, MAX_CHARS);
	if (IS_ROBOT_PTR(pl)) {
	    if ((k = Robot_war_on_player(GetInd[pl->id])) != NO_ID) {
		sprintf(name + strlen(name), " (%s)", Players[GetInd[k]]->name);
		if (strlen(name) >= 19) {
		    strcpy(&name[17], ")");
		}
	    }
	}
	sprintf(lblstr, "%c%c %-19s%03d%6d",
		(pl == best) ? '*' : pl->mychar,
		(pl->team == TEAM_NOT_SET) ? ' ' : (pl->team + '0'),
		name, (int)pl->life, (int)pl->score);
	sprintf(msg, "%2d... %-36s%s@%s\n",
		i+1, lblstr, pl->realname,
		IS_HUMAN_PTR(pl)
		? pl->hostname
		: "xpilot.org");
	if (strlen(msg) + strlen(str) >= max_size)
	    break;
	strlcat(str, msg, max_size);
    }
    free(order);
}


static void Handle_signal(int sig_no)
{
    errno = 0;

#ifndef _WINDOWS
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
	    Server.owner,
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


void Server_log_admin_message(int ind, const char *str)
{
    /*
     * Only log the message if logfile already exists,
     * is writable and less than some KBs in size.
     */
    const char		*logfilename = adminMessageFileName;
    const int		logfile_size_limit = adminMessageFileSizeLimit;
    FILE		*fp;
    struct stat		st;
    player		*pl = Players[ind];
    char		msg[MSG_LEN * 2];

    if ((logfilename != NULL) &&
	(logfilename[0] != '\0') &&
	(logfile_size_limit > 0) &&
	(access(logfilename, 2) == 0) &&
	(stat(logfilename, &st) == 0) &&
	(st.st_size + 80 < logfile_size_limit) &&
	((size_t)(logfile_size_limit - st.st_size - 80) > strlen(str)) &&
	((fp = fopen(logfilename, "a")) != NULL))
    {
	fprintf(fp,
		"%s[%s]{%s@%s(%s)|%s}:\n"
		"\t%s\n",
		showtime(),
		pl->name,
		pl->realname, pl->hostname,
		Get_player_addr(ind),
		Get_player_dpy(ind),
		str);
	fclose(fp);
	sprintf(msg, "%s [%s]:[%s]", str, pl->name, "GOD");
	Set_player_message(pl, msg);
    }
    else {
	Set_player_message(pl, " < GOD doesn't seem to be listening>");
    }
}


/*
 * Verify that all source files making up this program have been
 * compiled for the same version.  Too often bugs have been reported
 * for incorrectly compiled programs.
 */
extern char asteroid_version[];
extern char cannon_version[];
extern char cell_version[];
extern char checknames_version[];
extern char cmdline_version[];
extern char collision_version[];
extern char command_version[];
extern char config_version[];
extern char contact_version[];
extern char error_version[];
extern char event_version[];
extern char fileparser_version[];
extern char frame_version[];
extern char id_version[];
extern char item_version[];
extern char laser_version[];
extern char map_version[];
extern char math_version[];
extern char metaserver_version[];
extern char net_version[];
extern char netserver_version[];
extern char objpos_version[];
extern char option_version[];
extern char parser_version[];
extern char play_version[];
extern char player_version[];
extern char portability_version[];
extern char robot_version[];
extern char robotdef_version[];
extern char rules_version[];
extern char saudio_version[];
extern char sched_version[];
extern char score_version[];
extern char server_version[];
extern char ship_version[];
extern char shipshape_version[];
extern char shot_version[];
extern char socklib_version[];
extern char update_version[];
extern char walls_version[];
extern char wildmap_version[];

static void Check_server_versions(void)
{
    static struct file_version {
	char		filename[16];
	char		*versionstr;
    } file_versions[] = {
	{ "asteroid", asteroid_version },
	{ "cannon", cannon_version },
	{ "cell", cell_version },
	{ "checknames", checknames_version },
	{ "cmdline", cmdline_version },
	{ "collision", collision_version },
	{ "command", command_version },
	{ "config", config_version },
	{ "contact", contact_version },
	{ "error", error_version },
	{ "event", event_version },
	{ "fileparser", fileparser_version },
	{ "frame", frame_version },
	{ "id", id_version },
	{ "item", item_version },
	{ "laser", laser_version },
	{ "map", map_version },
	{ "math", math_version },
	{ "metaserver", metaserver_version },
	{ "net", net_version },
	{ "netserver", netserver_version },
	{ "objpos", objpos_version },
	{ "option", option_version },
	{ "parser", parser_version },
	{ "play", play_version },
	{ "player", player_version },
	{ "portability", portability_version },
	{ "robot", robot_version },
	{ "robotdef", robotdef_version },
	{ "rules", rules_version },
	{ "saudio", saudio_version },
	{ "sched", sched_version },
	{ "score", score_version },
	{ "server", server_version },
	{ "ship", ship_version },
	{ "shipshape", shipshape_version },
	{ "shot", shot_version },
	{ "socklib", socklib_version },
	{ "update", update_version },
	{ "walls", walls_version },
	{ "wildmap", wildmap_version },
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

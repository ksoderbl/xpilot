/* $Id: sched.c,v 4.11 2001/03/20 18:47:20 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>

#ifndef _WINDOWS
# include <unistd.h>
# ifndef __hpux
#  include <sys/time.h>
# endif
# ifdef _AIX
#  include <sys/select.h> /* _BSD not defined in <sys/types.h>, so done by hand */
# endif
#endif

#ifdef _OS2_
	#define  INCL_DOSSEMAPHORES
	#define  INCL_DOSDATETIME
	#define  INCL_DOSPROCESS
	#include <os2emx.h>
#endif

#ifdef _WINDOWS
# include "NT/winServer.h"
# include "NT/winSvrThread.h"
#endif

#define	SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "error.h"
#include "types.h"
#include "sched.h"
#include "global.h"

#include "portability.h"

char sched_version[] = VERSION;

int sched_running = false;

volatile long	timer_ticks;	/* SIGALRMs that have occurred */
static long		timers_used;	/* SIGALRMs that have been used */
static long		timer_freq;	/* rate at which timer ticks. (in FPS) */
#ifndef _WINDOWS
static void		(*timer_handler)(void);
#else
static	TIMERPROC	timer_handler;
#endif
static time_t		current_time;
static int		ticks_till_second;

/* Windows incorrectly uses u_int in FD_CLR */
#ifdef _WINDOWS
typedef	u_int	FDTYPE;
#else
typedef	int		FDTYPE;
#endif

/*
 * Block or unblock a single signal.
 */
static void sig_ok(int signum, int flag)
{
#if !defined(_WINDOWS)
    sigset_t    sigset;

    sigemptyset(&sigset);
    sigaddset(&sigset, signum);
    if (sigprocmask((flag) ? SIG_UNBLOCK : SIG_BLOCK, &sigset, NULL) == -1) {
	error("sigprocmask(%d,%d)", signum, flag);
	exit(1);
    }
#endif
}

/*
 * Prevent the real-time timer from interrupting system calls.
 * Globally accessible.
 */
void block_timer(void)
{
#ifndef _WINDOWS
    sig_ok(SIGALRM, 0);
#endif
}

/*
 * Unblock the real-time timer.
 * Globally accessible.
 */
void allow_timer(void)
{
#ifndef _WINDOWS
    sig_ok(SIGALRM, 1);
#endif
}


/*
 * Catch SIGALRM.
 * Simple timer ticker.
 */
static void catch_timer_ticks(int signum)
{
    timer_ticks++;
}


/*
 * Catch SIGALRM.
 * Use timerResolution to increment timer_ticks.
 */
static void catch_timer_counts(int signum)
{
    static unsigned int		timer_count = 0;

    timer_count += FPS;
    if (timer_count >= timerResolution) {
	timer_count -= timerResolution;
	timer_ticks++;
    }
}


#ifdef _OS2_
/*
 *  Thread to catch the OS/2 timer.  Raise a SIGALRM on the main thread
 *  at each timer tick.  I.e., perform the function of setitimer in Unix.
 */
void timerThread( void *arg )
{
	ULONG count;         /*  Post count of hev.  Required by call.  */
	ULONG pid = 0;       /*  Process ID for this process  */
	PTIB  ptib = NULL;   /*  Thread Information Block     */
	PPIB  ppib = NULL;   /*  Process Information Block    */

	HEV     hev;         /*  Event semaphore handle  */
	HTIMER  htimer;      /*  Timer handle            */

	APIRET  rc;          /*  Return code for Dos* calls  */

	#ifdef OS2DEBUG
	        ULONG  aulSysInfo[1] = {0};
		DosQuerySysInfo( 22, 22, (PVOID)aulSysInfo, sizeof(ULONG) );
		printf( "Timer interval in tenths of milliseconds: %ld\n", aulSysInfo[0] );
	#endif

	#ifdef OS2DEBUG
		printf( "Entering timer processing thread.  Will set for %ld FPS.\n", timer_freq );
	#endif

	/*  Get the ID of this process, so we can send a SIGALRM
	 *  to the main thread.
	 */
	if( rc = DosGetInfoBlocks( &ptib, &ppib ) )
	{
		error( "Error getting process information.  rc = %d", rc );
		exit( 1 );
	}

	pid = ppib->pib_ulpid;

	/*  Increase the priority of this thread to regular time critical.
	 *  This should ensure that our timer ticks get delivered as quickly
	 *  as possible.
	 */
	if( rc = DosSetPriority(  PRTYS_THREAD, PRTYC_TIMECRITICAL, 0L, 0L ) )
	{
		error( "Error setting timer thread priority.  rc = %d", rc );
		exit(1);
	}

	/*  Create the event semaphore that will be posted by the timer.  */
	if( rc = DosCreateEventSem( NULL, &hev, DC_SEM_SHARED, FALSE ) )
	{
		error( "DosCreateEventSem - error creating timer semaphore.  rc = %d", rc );
		exit( 1 );
	}

	/*  Start the timer.  The semaphore hev will be posted at each
	 *  timer tick.
	 */
	if( rc = DosStartTimer( 1000/timer_freq, (HSEM)hev, &htimer ) )
	{
		error( "DosStartTimer - error starting timer.  rc = %d", rc );
		exit( 1 );
	}

	/*  Now just loop forever, waiting for the timer and then
	 *  signalling a SIGALRM on the main thread.
	 */
	while( 1 )
	{
		/*  Wait for the timer tick.  */
		DosWaitEventSem( hev, SEM_INDEFINITE_WAIT );

		/*  Reset the semaphore.  */
		DosResetEventSem( hev, &count );

		/*  raise( SIGALRM ) sends the signal to this thread,
		 *  not to the main thread where we have the handler
		 *  installed.  So, we must use kill( ) instead to raise
		 *  the signal.
		 */
		kill( pid, SIGALRM );
	}
	return;
}

#endif


/*
 * Setup the handling of the SIGALRM signal
 * and setup the real-time interval timer.
 */
static void setup_timer(void)
{
#ifndef _WINDOWS

#ifndef _OS2_
    struct itimerval itv;
#endif
    struct sigaction act;

    /*
     * Prevent SIGALRMs from disturbing the initialization.
     */
    block_timer();

    /*
     * Install a signal handler for the alarm signal.
     */
    act.sa_handler = (timerResolution > 0)
		    ? (catch_timer_counts)
		    : (catch_timer_ticks);
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGALRM);
    if (sigaction(SIGALRM, &act, (struct sigaction *)NULL) == -1) {
	error("sigaction SIGALRM");
	exit(1);
    }

    /*
     * Install a real-time timer.
     */
    if (timer_freq <= 0 || timer_freq > 100) {
	error("illegal timer frequency: %ld", timer_freq);
	exit(1);
    }

#ifndef _OS2_
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 1000000 / timer_freq;
    itv.it_value = itv.it_interval;
    if (setitimer(ITIMER_REAL, &itv, NULL) == -1) {
	error("setitimer");
	exit(1);
    }
#else  /*  !defined( _OS2_ )  */

    /*  setitimer  isn't implemented in EMX, so we must fake our
     *  own.  Create a thread that starts a timer and raises SIGALRM
     *  at each tick.
     *
     *  Of course, this timer is rather course, given the time-slicing
     *  of OS/2, but it is the best we can do at the moment.  The next
     *  step is to look into the high-resolution timer that Timur Tabi
     *  wrote (which, I believe, is now part of OS/2?).
     */

    if( _beginthread( timerThread, NULL, 8192L, NULL ) == -1 ) {
	error( "_beginthread - error starting timer thread" );
	exit( 1 );
    }

#endif  /*  !defined( _OS2_ )  */

    timers_used = timer_ticks;
    time(&current_time);
    ticks_till_second = timer_freq;
#else
/*
    UINT cr = SetTimer(NULL, 0, 1000/timer_freq, timer_handler);
    UINT cr = SetTimer(NULL, 0, 20, (TIMERPROC)ServerThreadTimerProc);
    if (!cr)
	error("Can't create timer");
*/
#endif
    /*
     * Allow the real-time timer to generate SIGALRM signals.
     */
    allow_timer();
}

/*
 * Configure timer tick callback.
 */
#ifndef _WINDOWS
void install_timer_tick(void (*func)(void), int freq)
{
    timer_handler = func;
    timer_freq = freq;
    setup_timer();
} 
#else

typedef void (__stdcall *windows_timer_t)(void *, unsigned int, unsigned int, unsigned long);

void install_timer_tick(windows_timer_t func, int freq)
{
    timer_handler = (TIMERPROC)func;
    timer_freq = freq;
    setup_timer();
}
#endif


/*
 * Linked list of timeout callbacks.
 */
struct to_handler {
    struct to_handler	*next;
    time_t		when;
    void		(*func)(void *);
    void		*arg;
};
static struct to_handler *to_busy_list = NULL;
static struct to_handler *to_free_list = NULL;
static int		to_min_free = 3;
static int		to_max_free = 5;
static int		to_cur_free = 0;

static void to_fill(void)
{
    if (to_cur_free < to_min_free) {
	do {
	    struct to_handler *top =
		(struct to_handler *)malloc(sizeof(struct to_handler));
	    if (!top) {
		break;
	    }
	    top->next = to_free_list;
	    to_free_list = top;
	    to_cur_free++;
	} while (to_cur_free < to_max_free);
    }
}

static struct to_handler *to_alloc(void)
{
    struct to_handler *top;

    to_fill();
    if (!to_free_list) {
	error("Not enough memory for timeouts");
	exit(1);
    }

    top = to_free_list;
    to_free_list = top->next;
    to_cur_free--;
    top->next = 0;

    return top;
}

static void to_free(struct to_handler *top)
{
    if (to_cur_free < to_max_free) {
	top->next = to_free_list;
	to_free_list = top;
	to_cur_free++;
    }
    else {
	free(top);
    }
}

/*
 * Configure timout callback.
 */
void install_timeout(void (*func)(void *), int offset, void *arg)
{
    struct to_handler *top = to_alloc();
    top->func = func;
    top->when = current_time + offset;
    top->arg = arg;
    if (!to_busy_list || to_busy_list->when >= top->when) {
	top->next = NULL;
	to_busy_list = top;
    }
    else {
	struct to_handler *prev = to_busy_list;
	struct to_handler *lp = prev->next;
	while (lp && lp->when < top->when) {
	    prev = lp;
	    lp = lp->next;
	}
	top->next = lp;
	prev->next = top;
    }
}

void remove_timeout(void (*func)(void *), void *arg)
{
    struct to_handler *prev = 0;
    struct to_handler *lp = to_busy_list;
    while (lp) {
	if (lp->func == func && lp->arg == arg) {
	    struct to_handler *top = lp;
	    lp = lp->next;
	    if (prev) {
		prev->next = lp;
	    } else {
		to_busy_list = lp;
	    }
	    to_free(top);
	}
	else {
	    prev = lp;
	    lp = lp->next;
	}
    }
}

static void timeout_chime(void)
{
    while (to_busy_list && to_busy_list->when <= current_time) {
	struct to_handler *top = to_busy_list;
	void (*func)(void *) = top->func;
	void *arg = top->arg;
	to_busy_list = top->next;
	to_free(top);
	(*func)(arg);
    }
}

#ifndef _WINDOWS
#define NUM_SELECT_FD		((int)sizeof(int) * 8)
#else
/*
	Windoze:
	The first call to socket() returns 560ish.  Successive calls keep bumping
	up the SOCKET returned until about 880 when it wraps back to 8.
	(It seems to increment by 8 with each connect - but that's not important)
	I can't find a manifest constant to tell me what the upper limit will be *sigh*

	--- Now, the Windoze gurus tell me that SOCKET is an opaque data type.  So i need
	to make a lookup array for the lookup array :(
*/
#define	NUM_SELECT_FD		2000
#endif

struct io_handler {
    int			fd;
    void		(*func)(int, void *);
    void		*arg;
};

static struct io_handler	input_handlers[NUM_SELECT_FD];
static fd_set			input_mask;
static int			max_fd, min_fd;
static int			input_inited = false;

static void io_dummy(int fd, void *arg)
{
    xpprintf("io_dummy called!  (%d, %p)\n", fd, arg);
}

void install_input(void (*func)(int, void *), int fd, void *arg)
{
    int i;

    if (input_inited == false) {
	input_inited = true;
	FD_ZERO(&input_mask);
#ifndef _WINDOWS
	min_fd = fd;
#else
	min_fd = 0;
#endif
	max_fd = fd;
	for (i = 0; i < NELEM(input_handlers); i++) {
	    input_handlers[i].fd = -1;
	    input_handlers[i].func = io_dummy;
	    input_handlers[i].arg = 0;
	}
    }
#ifdef _WINDOWS
	xpprintf("install_input: fd %d min_fd=%d\n", fd, min_fd);
#endif
    if (fd < min_fd || fd >= min_fd + NUM_SELECT_FD) {
	error("install illegal input handler fd %d (%d)", fd, min_fd);
	ServerExit();
    }
    if (FD_ISSET(fd, &input_mask)) {
	error("input handler %d busy", fd);
	ServerExit();
    }
    input_handlers[fd - min_fd].fd = fd;
    input_handlers[fd - min_fd].func = func;
    input_handlers[fd - min_fd].arg = arg;
    FD_SET(fd, &input_mask);
    if (fd > max_fd) {
	max_fd = fd;
    }
}

void remove_input(int fd)
{
    if (fd < min_fd || fd >= min_fd + NUM_SELECT_FD) {
	error("remove illegal input handler fd %d (%d)", fd, min_fd);
	ServerExit();
    }
    if (FD_ISSET(fd, &input_mask)) {
	input_handlers[fd - min_fd].fd = -1;
	input_handlers[fd - min_fd].func = io_dummy;
	input_handlers[fd - min_fd].arg = 0;
	FD_CLR((FDTYPE)fd, &input_mask);
	if (fd == max_fd) {
	    int i = fd;
	    max_fd = -1;
	    while (--i >= min_fd) {
		if (FD_ISSET(i, &input_mask)) {
		    max_fd = i;
		    break;
		}
	    }
	}
    }
}

#ifdef VMS
extern int NumPlayers, NumRobots, NumPseudoPlayers, NumQueuedPlayers;
extern int login_in_progress;
#endif

void stop_sched(void)
{
    sched_running = 0;
}

/*
 * I/O + timer dispatcher.
 * Windows pumps this one time 
 */
void sched(void)
{
    int			i, n, io_todo = 3;
    struct timeval	tv, *tvp = &tv;

#ifndef _WINDOWS
    if (sched_running) {
	error("sched already running");
	exit(1);
    }

    sched_running = 1;

    while (sched_running) {
#endif
#if defined(VMS) || defined(_WINDOWS)
	if (NumPlayers > NumRobots + NumPseudoPlayers
	    || login_in_progress != 0
	    || NumQueuedPlayers > 0) {

	    /* need fast I/O checks now! (2 or 3 times per frames) */
	    tv.tv_sec = 0;
	    /* KOERBER */
	    /*	tv.tv_usec = 1000000 / (3 * timer_freq + 1); */
	    tv.tv_usec = 1000000 / (10 * timer_freq + 1); 
	}
	else {
	    /* slow I/O checks are possible here... (2 times per second) */ ; 
	    tv.tv_sec = 0;
	    tv.tv_usec = 500000;
	}
#else
	tv.tv_sec = 0;
	tv.tv_usec = 0;
#endif

	if (io_todo == 0 && timers_used < timer_ticks) {
	    io_todo = 1 + (timer_ticks - timers_used);
	    tvp = &tv;
#ifndef _WINDOWS
	    if (timer_handler) {
		(*timer_handler)();
	    }
#endif
	    do {
		++timers_used;
		if (--ticks_till_second <= 0) {
		    ticks_till_second += timer_freq;
		    current_time++;
		    timeout_chime();
		}
	    } while (timers_used + 1 < timer_ticks);
	}
	else {
	    fd_set readmask;
	    readmask = input_mask;
	    n = select(max_fd + 1, &readmask, 0, 0, tvp);
	    if (n <= 0) {
		if (n == -1 && errno != EINTR) {
#ifndef _WINDOWS
		    error("sched select error");
#else
			char	s[80];
			sprintf(s, "sched select error n=%d e=%d (%s)", n, errno, _GetWSockErrText(errno));
			error(s);
#endif
		    exit(1);
		}
		io_todo = 0;
	    }
	    else {
		for (i = max_fd; i >= min_fd; i--) {
		    if (FD_ISSET(i, &readmask)) {
			struct io_handler *ioh;
			ioh = &input_handlers[i - min_fd];
			(*(ioh->func))(ioh->fd, ioh->arg);
			if (--n == 0) {
			    break;
			}
		    }
		}
		if (io_todo > 0) {
		    io_todo--;
		}
	    }
#ifndef VMS
	    if (io_todo == 0) {
		tvp = NULL;
	    }
#endif
	}
#ifndef _WINDOWS
    }
#endif
}


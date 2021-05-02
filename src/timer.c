/* $Id: timer.c,v 1.3 1993/03/23 17:54:14 bjoerns Exp $
 *
 *	This file is part of the XPilot project.
 *	Copylefts are explained in the LICENSE file.
 *
 *	Thanks to Bert Gijsbers for this piece of code.
 */

#include <sys/time.h>
#include <signal.h>

extern int  framesPerSecond;
int         error(const char *, ...);


#undef BUSYLOOP
#ifdef BUSYLOOP
void Loop_delay(void)
{
    static long		adj_sec = 0;
    static long		last_msec = 0;
    long		msec;
    struct timeval	tval;
    struct timezone	tzone;

    if (adj_sec == 0) {
	if (gettimeofday(&tval, &tzone) != 0)
	    return;

	adj_sec = tval.tv_sec;
	last_msec = (tval.tv_sec - adj_sec)*1000 + (tval.tv_usec)/1000;
	return;
    }

    do {
	if (gettimeofday(&tval, &tzone) != 0)
	    return;

	msec = (tval.tv_sec - adj_sec)*1000 + (tval.tv_usec)/1000;

    } while (msec < last_msec + 1000/framesPerSecond);

    last_msec = msec;
}
#else

static volatile long   timer_count,	/* SIGALRMs that have occurred */
                       timers_used;	/* SIGALRMs that have been used */

/*
 * Catch the SIGALRM signal.
 * If any system calls other than pause(2) would get interrupted because
 * of the SIGALRM signal occuring, than that would be an undesirable
 * side effect or bug.  Be aware of the possibility.
 * E.g., select(2) will be interrupted (X event polling uses select(2)).
 * If this happens to be a problem, two functions are provided to
 * temporarily block and unblock the timer interrupt.
 * Note that this is still experimental.  It is unclear to me if setitimer(2)
 * is accurate enough.  If not than this whole idea may be thrown away or
 * we have to come up with something better.  How about using gettimeofday(2)
 * to measure if any timer-interrupts are missed and adjusting `timer_count'
 * accordingly?  If the SIGALRM handler should return very fast
 * in order to reduce the chance of missing signals.
 */
static void catch_timer(int signum)
{
    timer_count++;
}

/*
 * Block or unblock a single signal.
 */
static void sig_ok(int signum, int flag)
{
    sigset_t    sigset;

    sigemptyset(&sigset);
    sigaddset(&sigset, signum);
    if (sigprocmask((flag) ? SIG_UNBLOCK : SIG_BLOCK, &sigset, NULL) == -1) {
	error("sigprocmask(%d,%d)", signum, flag);
	exit(1);
    }
}

/*
 * Prevent the real-time timer from interrupting system calls.
 * Globally accessible.
 */
void block_timer()
{
    sig_ok(SIGALRM, 0);
}

/*
 * Unblock the real-time timer.
 * Globally accessible.
 */
void allow_timer()
{
    sig_ok(SIGALRM, 1);
}

/*
 * Setup the handling of the SIGALRM signal
 * and setup the real-time interval timer.
 */
static void setup_timer(void)
{
    struct itimerval itv;
    struct sigaction act;

    /*
     * Prevent SIGALRMs from disturbing the initialisation.
     */
    block_timer();

    /*
     * Install a signal handler for the alarm signal.
     */
    act.sa_handler = catch_timer;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGALRM);
    if (sigaction(SIGALRM, &act, (struct sigaction *)NULL) == -1) {
	error("sigaction SIGALRM");
	exit(1);
    }

    /*
     * Install a real-time timer with a frequency of `framesPerSecond'.
     */
    if (framesPerSecond <= 0) {
	error("illegal framesPerSecond value %d", framesPerSecond);
	exit(1);
    }
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 1000000 / framesPerSecond;
    itv.it_value = itv.it_interval;
    if (setitimer(ITIMER_REAL, &itv, NULL) == -1) {
	error("setitimer");
	exit(1);
    }

    /*
     * Prevent possible underflow later on.
     */
    timer_count = timers_used = framesPerSecond;

    /*
     * Allow the real-time timer to generate SIGALRM signals.
     */
    allow_timer();
}

/*
 * Let the caller be run in a frequency of `framesPerSecond'.
 */
void Loop_delay(void)
{
    static int  prevFramesPerSecond = -2;

    if (prevFramesPerSecond != framesPerSecond) {
	setup_timer();
	prevFramesPerSecond = framesPerSecond;
    }
    while (timers_used == timer_count) {
	/*
	 * Wait for a SIGALRM to occur.
	 */
	pause();
    }
    if (timers_used >= timer_count) {
	error("bug SIGALRM %ld >= %ld\n", timers_used, timer_count);
	timers_used = timer_count = framesPerSecond;
    }
    else if (++timers_used + framesPerSecond <= timer_count) {
	/*
	 * The host, network or X servers can't keep up with our rate.
	 * This also happens if a new player is logging in.
	 * Adjust, but allow an overload if it is of short duration.
	 */
	timers_used = timer_count - (framesPerSecond + 1) / 2;
    }
}
#endif

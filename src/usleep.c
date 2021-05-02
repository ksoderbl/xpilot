/*
#ifndef lint
static char sccsid[] = "@(#)usleep.c	1.3 91/05/24 XLOCK";
#endif
*/
/*-
 * usleep.c - OS dependant implementation of usleep().
 *
 * Copyright (c) 1991 by Patrick J. Naughton.
 *
 * Revision History:
 * 30-Aug-90: written.
 *
 */

#include <sys/types.h>
#include <sys/time.h>

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: usleep.c,v 3.1 1993/08/02 12:55:41 bjoerns Exp $";
#endif


int
usleep(usec)
    unsigned long usec;
{
/*#ifdef SYSV
    poll((struct poll *) 0, (size_t) 0, usec / 1000);	/* ms RES */
/*#else*/
    struct timeval timeout;
    timeout.tv_usec = usec % (unsigned long) 1000000;
    timeout.tv_sec = usec / (unsigned long) 1000000;
    (void) select(0, (void *) 0, (void *) 0, (void *) 0, &timeout);
/*#endif*/
    return 0;
}

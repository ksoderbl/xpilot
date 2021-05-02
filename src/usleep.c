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

#ifdef VMS
#include <time.h>
#include <socket.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include "types.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: usleep.c,v 3.7 1994/07/10 20:06:33 bert Exp $";
#endif


void usleep(unsigned long usec)
{
#if 0 /* SYSV */
    poll((struct poll *) 0, (size_t) 0, usec / 1000);	/* ms RES */
#endif
#ifdef VMS
    float timeout;

    timeout = usec/1000000.0;
    LIB$WAIT(&timeout);
#else
    struct timeval timeout;
    timeout.tv_usec = usec % (unsigned long) 1000000;
    timeout.tv_sec = usec / (unsigned long) 1000000;
    (void) select(0, (void *) 0, (void *) 0, (void *) 0, &timeout);
#endif
}

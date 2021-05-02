/*-
 * usleep.c - OS dependant implementation of usleep().
 *
 * Copyright (c) 1991 by Patrick J. Naughton.
 *
 * Revision History:
 * 30-Aug-90: written.
 *
 * 07-Dec-94: Bert Gijsbers
 *	Changed "void usleep(unsigned long)" to "int usleep(unsigned)"
 *	as this is what it seems to be on systems which do have usleep(3) (AIX).
 *	Changed usleep into micro_delay to forego any possible prototype clashes.
 */

#include <time.h>
#include <sys/types.h>

#ifndef _WINDOWS
# include <unistd.h>
# ifndef __hpux
#  include <sys/time.h>
# endif
#endif

#ifdef _WINDOWS
# include "NT/winNet.h"
#endif

#include "types.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: usleep.c,v 4.2 2001/03/20 18:37:58 bert Exp $";
#endif

int micro_delay(unsigned usec);

int micro_delay(unsigned usec)
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
    (void) select(0, NULL, NULL, NULL, &timeout);
#endif

    return 0;
}

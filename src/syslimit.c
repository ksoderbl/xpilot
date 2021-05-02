/* $Id: syslimit.c,v 3.2 1993/06/28 20:54:25 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <stdio.h>		/* Could be moved below the #ifdef, but then */
				/* we would get a warning (empty source file) */
				/* each time LIMIT_ACCESS isn't defined. */
#ifdef	LIMIT_ACCESS

#include <time.h>
#include "types.h"

#define PATTERN		"lglab[01]"
#define FREELIMIT	7

#define RUPTIME		"/usr/bin/ruptime "
#define GREP		"/usr/local/bin/ggrep "
#define WC_L		"/bin/wc -l "
#define RWHO		"/usr/bin/rwho -a "

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: syslimit.c,v 3.2 1993/06/28 20:54:25 bjoerns Exp $";
#endif



/*
 * This routine is not useful outside UiT but may be used as a skeleton for
 * similar routines, if similar problems should occur... :)
 */
bool Is_allowed(char *display)
{
    FILE	*fp;
    int		total_no, num_free, in_use;
    struct tm	*now;
    time_t	tmp;


    if (strstr(display, "lglab") == NULL)
	return (true);

    printf("------------------\n");

    if (strstr(display, "lglab2") != NULL) {
	printf("Atsjoooooo! I can't, sorry! :)\n");
	return (false);
    }
    tmp = time((time_t)NULL);
    now = localtime(&tmp);

    if (now->tm_hour >= 8 && now->tm_hour < 16) {
	if (now->tm_wday != 0 && now->tm_wday != 6) {
	    printf("You'll have to wait %d hours and %d minutes until "
		   "you're allowed to play.\n",
		   16 - now->tm_hour, 60 - now->tm_min);
	    return (false);
	}
    }

    printf("Checking number of unused workstations."); fflush(stdout);

    fp = popen(RUPTIME "|" GREP PATTERN "|" GREP "\" up \"|" WC_L, "r");
    fscanf(fp, "%d", &total_no);
    pclose(fp);

    printf("."); fflush(stdout);

    fp = popen(RWHO "|" GREP "console |" GREP PATTERN "|" WC_L, "r");
    fscanf(fp, "%d", &in_use);
    pclose(fp);

    printf(".\n"); fflush(stdout);

    num_free = total_no - in_use;

    printf("%d out of %d machines are free. "
	   "Current limit is at %d machines.\n", num_free, total_no, FREELIMIT);

    if (num_free >= FREELIMIT) {
	return (true);
    } else {
	printf("You will not be allowed to play until %d more users log out.\n",
	       FREELIMIT-num_free);
	return (false);
    }
}

#endif	/* LIMIT_ACCESS */

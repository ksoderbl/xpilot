/* syslimit.c,v 1.3 1992/05/11 15:31:40 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <stdio.h>
#include "pilot.h"
#include "pack.h"

#define PATTERN		"lglab[01]"
#define FREELIMIT	7

#define RUPTIME		"/usr/bin/ruptime "
#define GREP		"/usr/local/bin/ggrep "
#define WC_L		"/bin/wc -l "
#define RWHO		"/usr/bin/rwho -a "

extern Pack		req;



/*
 * This routine is not useful outside UiT but may be used as a skeleton for
 * similar routines, if similar problems should occur... :)
 */
bool Is_allowed(void)
{
    FILE *fp;
    int total_no, no_free, in_use;


    if (strstr(req.display, "lglab") == NULL)
	return (true);

    printf("------------------\n");

    if (strstr(req.display, "lglab2") != NULL) {
	printf("Atsjoooooo! I can't, sorry! :)\n");
	return (false);
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

    no_free = total_no - in_use;

    printf("%d out of %d machines are free. "
	   "Current limit is at %d machines.\n", no_free, total_no, FREELIMIT);

    if (no_free >= FREELIMIT) {
	return (true);
    } else {
	printf("You will not be allowed to play until %d more users log out.\n",
	       FREELIMIT-no_free);
	return (false);
    }
}

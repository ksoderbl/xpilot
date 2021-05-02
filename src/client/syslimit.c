/* $Id: syslimit.c,v 5.1 2001/05/08 11:35:29 bertg Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
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

#include <stdio.h>		/* Could be moved below the #ifdef, but then */
				/* we would get a warning (empty source file) */
				/* each time LIMIT_ACCESS isn't defined. */
#ifdef	LIMIT_ACCESS

#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

#include "types.h"

#define PATTERN		"lglab[01]"
#define FREELIMIT	7

#define RUPTIME		"/usr/bin/ruptime "
#define GREP		"/usr/local/bin/ggrep "
#define WC_L		"/bin/wc -l "
#define RWHO		"/usr/bin/rwho -a "




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

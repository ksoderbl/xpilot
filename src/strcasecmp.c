/* strcasecmp.c,v 1.3 1992/06/26 15:25:51 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <ctype.h>

#ifndef	lint
static char sourceid[] =
    "@(#)strcasecmp.c,v 1.3 1992/06/26 15:25:51 bjoerns Exp";
#endif

/*
 * By Ian Malcom Brown.
 */
int
    strcasecmp(str1, str2)
char *str1, *str2;
{
    int	c1, c2;


    do {
	c1 = tolower(*str1++);
	c2 = tolower(*str2++);
    } while(c1 != 0 && c2 != 0 && c1 == c2);

    return(c1 - c2);
}

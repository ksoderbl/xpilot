/* $Id: strcasecmp.c,v 3.3 1993/08/02 12:41:44 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: strcasecmp.c,v 3.3 1993/08/02 12:41:44 bjoerns Exp $";
#endif

/*
 * By Ian Malcom Brown.
 * Changes by BG: prototypes with const,
 * moved the ++ expressions out of the macro.
 */
int strcasecmp(const char *str1, const char *str2)
{
    int	c1, c2;


    do {
	c1 = *str1++;
	c2 = *str2++;
	c1 = tolower(c1);
	c2 = tolower(c2);
    } while (c1 != 0 && c2 != 0 && c1 == c2);

    return (c1 - c2);
}

/*
 * By Bert Gijsbers, derived from Ian Malcom Brown's strcasecmp().
 */
int strncasecmp(const char *str1, const char *str2, size_t n)
{
    int	c1, c2;


    do {
	if (n-- <= 0) {
	    return 0;
	}
	c1 = *str1++;
	c2 = *str2++;
	c1 = tolower(c1);
	c2 = tolower(c2);
    } while (c1 != 0 && c2 != 0 && c1 == c2);

    return (c1 - c2);
}

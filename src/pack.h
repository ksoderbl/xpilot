/* pack.h,v 1.3 1992/05/11 15:31:23 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include "default.h"

#define PORT_NR		15345

#define CONTACT		0
#define ENTER_GAME	1
#define REPORT_STATUS	2
#define MESSAGE		3
#define KICK		4
#define LOCK		5
#define GAME_FULL	6
#define SHUTDOWN	7

#define CAP_LETTER(c)	(c = (c>='a'&&c<='z') ? c-'a'+'A' : c)

typedef struct {
    int type;
    char display[80];		/* Display of player */
    char realname[80];		/* Real name of player */
    char hostname[80];		/* Hostname of hand-shake program */
    int	 port;
    char string[3000];		/* Miscellaneous info, too large :) */
    def_t   def;
} Pack;

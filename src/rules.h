/* rules.h,v 1.3 1992/05/11 15:31:34 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef RULES_H
#define RULES_H

#define CUSTOM		    0	    /* Possible values of mode */
#define ADVENTURE	    1
#define RACE		    2
#define HEAT		    3
#define DOGFIGHT	    4
#define LIMITED_DOGFIGHT    5
#define MAX_MODES	    6

#define CRASH_WITH_PLAYER   (1<<0)
#define PLAYER_KILLINGS	    (1<<1)
#define LIMITED_LIVES	    (1<<2)
#define TIMING		    (1<<3)
#define ONE_PLAYER_ONLY	    (1<<4)
#define RESET_START_POS	    (1<<5)
#define PLAYER_SHIELDING    (1<<6)
#define LIMITED_VISIBILITY  (1<<7)
#define TEAM_PLAY	    (1<<8)

typedef struct {
    int lives;
    long mode;
} rules_t;

#endif

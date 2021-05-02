/* $Id: rules.h,v 3.2 1993/06/28 20:54:16 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef RULES_H
#define RULES_H

#define CRASH_WITH_PLAYER   (1<<0)
#define PLAYER_KILLINGS	    (1<<1)
#define LIMITED_LIVES	    (1<<2)
#define TIMING		    (1<<3)
#define ONE_PLAYER_ONLY	    (1<<4)
#define PLAYER_SHIELDING    (1<<5)
#define LIMITED_VISIBILITY  (1<<6)
#define TEAM_PLAY	    (1<<7)
#define WRAP_PLAY	    (1<<8)
#define ALLOW_NUKES	    (1<<9)

typedef struct {
    int lives;
    long mode;
} rules_t;

#endif

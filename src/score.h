/* score.h,v 1.3 1992/05/11 15:31:36 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef SCORE_H
#define SCORE_H

#define ED_SHOT			-0.2
#define ED_SMART_SHOT		-30.0
#define ED_SHIELD		-0.20
#define ED_CLOAKING_DEVICE	-0.07
#define ED_SHOT_HIT		-25.0
#define ED_SMART_SHOT_HIT	-120.0
#define ED_PL_CRASH		-100.0

#define PTS_PR_PL_KILL	    	13	/* Points if you kill a player */
#define PTS_PR_CANNON_KILL  	1	/* Points if you smash a cannon */
#define PTS_PR_CRASH	    	-2	/* Points if you crash in a object */
#define PTS_PR_PL_CRASH	    	-4	/* Points is you crash in a player */
#define PTS_PR_SHOT	    	-2	/* Points if you get shot */
#define PTS_PR_PL_SHOT	    	-2    	/* Points if you get shot by a player */
#define PTS_GAME_WON	    	50	/* Points if you win the game */
#define PTS_SUICIDE	    	-25	/* Points if you are #$@ enough to */
					/* commit suicide. */

#define CANNON_RATING	    	0
#define WALL_RATING	    	2000

#define RATE_SIZE	    	20
#define RATE_RANGE	    	1024

#define SCORE(ind, points) 					\
{								\
    Players[ind]->score+=(points); 				\
    labels=true;						\
}
#define ENERGY(ind, drainage)					\
{								\
    Players[ind]->fuel = MAX(Players[ind]->fuel+drainage, 0.0); \
}

#endif

/* $Id: draw.h,v 3.6 1993/06/28 20:53:35 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	DRAW_H
#define	DRAW_H

#include "types.h"

#define NUM_COLORS	    4

#define BLACK		    0
#define WHITE		    1
#define BLUE		    2
#define RED		    3

#define FULL		    768
#define CENTER		    (FULL/2)
#define SMART_SHOT_LEN	    15
#define DSIZE		    4	    /* Size of diamond (on radar) */

#define MSG_DURATION	    1024
#define MSG_FLASH	    892

#define SCROLL_DELAY	    10
#define SCROLL_LEN	    100

#define TITLE_DELAY	    500
#define	UPDATE_SCORE_DELAY  (FPS)

#define CLOAK_FAILURE	    130

#ifndef NO_ROTATING_DASHES
#define NUM_DASHES	    2
#define DASHES_LENGTH	    12
#endif

#define HUD_SIZE	    90		    /* Size/2 of HUD lines */
#define HUD_OFFSET	    20		    /* Hud line offset */
#define FUEL_GAUGE_OFFSET   6
#define HUD_FUEL_GAUGE_SIZE (2*(HUD_SIZE-HUD_OFFSET-FUEL_GAUGE_OFFSET))

typedef struct {
    char txt[MSG_LEN];
    short len;
    long life;
} message_t;

typedef struct {			/* Defines wire-obj, i.e. ship */
    position	*pts;
    int		num_points;
} wireobj;

#define HavePlanes(d) (DisplayPlanes(d, DefaultScreen(d)) > 2)
#define HaveColor(d)							\
    (DefaultVisual(d, DefaultScreen(d))->class == PseudoColor		\
     || DefaultVisual(d, DefaultScreen(d))->class == GrayScale)

#define FRAC(py)	    ((int)((py) * 1024.0/768.0))


#endif

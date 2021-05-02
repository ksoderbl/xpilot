/* draw.h,v 1.3 1992/05/11 15:31:13 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#define ANT_COLORS	    4

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

#define CLOAK_FAILURE	    130

#ifndef NO_ROTATING_DASHES
#define NO_OF_DASHES	    2
#define DASHES_LENGTH	    12
extern char dashes[];
#endif

#define CENTER		    (FULL/2)
#define HUD_SIZE	    90		    /* Size/2 of HUD lines */
#define HUD_OFFSET	    20		    /* Hud line offset */
#define FUEL_GAUGE_OFFSET   6
#define HUD_ITEMS_SPACE	    (2*(HUD_SIZE-HUD_OFFSET)/7.0)
#define HUD_FUEL_GAUGE_SIZE (2*(HUD_SIZE-HUD_OFFSET-FUEL_GAUGE_OFFSET))

#define HELP_PAGES	    2

typedef struct {
    char txt[MSG_LEN];
    long life;
} message_t;

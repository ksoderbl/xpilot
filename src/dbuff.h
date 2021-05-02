/* dbuff.h,v 1.3 1992/05/11 15:31:05 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <X11/Xlib.h>

typedef struct {
    Display	    *display;
    Colormap	    cmap;
    unsigned long   drawing_planes;
    int		    buffer;
    XColor	    *colormaps[2];
    int		    map_size;
    unsigned long   masks[2];
    unsigned long   *planes;
    unsigned long   pixel;
} dbuff_state_t;

/*
 * Prototypes.
 */
extern dbuff_state_t	*start_dbuff();
extern void		dbuff_switch();
extern void		end_dbuff();

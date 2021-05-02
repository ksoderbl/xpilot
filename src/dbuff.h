/* $Id: dbuff.h,v 1.6 1992/06/25 03:23:51 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	DBUFF_H
#define	DBUFF_H

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
extern dbuff_state_t	*start_dbuff(int, Display *, Colormap,
				     unsigned long, XColor *);
extern void		dbuff_switch(int, dbuff_state_t *);
extern void		end_dbuff(int, dbuff_state_t *);

#endif

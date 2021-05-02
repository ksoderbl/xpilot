/* $Id: dbuff.h,v 3.3 1993/06/28 20:53:32 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	DBUFF_H
#define	DBUFF_H

#include <X11/Xlib.h>

typedef enum { PIXMAP_COPY, COLOR_SWITCH } dbuff_t;

typedef struct {
    Display		*display;
    dbuff_t		type;
    Colormap		cmap;
    unsigned long	drawing_planes;
    int			buffer;
    XColor		*colormaps[2];
    int			map_size;
    unsigned long	masks[2];
    unsigned long	*planes;
    unsigned long	pixel;
} dbuff_state_t;

dbuff_state_t *start_dbuff(Display *display, Colormap cmap,
			   dbuff_t type,
			   unsigned long planes, XColor *colors);
void dbuff_switch(dbuff_state_t *state);
void end_dbuff(dbuff_state_t *state);

#endif

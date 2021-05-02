/* $Id: dbuff.h,v 3.14 1996/12/17 13:48:40 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gÿsbers         <bert@xpilot.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef	DBUFF_H
#define	DBUFF_H

#ifdef SPARC_CMAP_HACK
# if defined(sparc) || defined(__sparc)
#  if defined(SVR4) || defined(__svr4__)
#   include <sys/fbio.h>
#  else
#   include <sun/fbio.h>
#  endif
# else
#  undef SPARC_CMAP_HACK
# endif
#endif

#ifdef MBX
# include <X11/extensions/multibuf.h>
#endif


typedef enum {
    PIXMAP_COPY,
    COLOR_SWITCH,
    MULTIBUFFER
} dbuff_t;

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
#ifdef MBX
    Multibuffer		draw[2];
    int			ev_base, err_base;
#endif
#ifdef SPARC_CMAP_HACK
    int			fbfd;
    struct fbcmap	hardcmap;
#endif
} dbuff_state_t;

extern dbuff_state_t   *dbuf_state;    /* Holds current dbuff state */

dbuff_state_t *start_dbuff(Display *display, Colormap cmap,
			   dbuff_t type,
			   unsigned long planes, XColor *colors);
void dbuff_switch(dbuff_state_t *state);
void dbuff_init(dbuff_state_t *state);
void end_dbuff(dbuff_state_t *state);

#endif

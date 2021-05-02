/* $Id: xpmread.h,v 3.1 1995/01/24 17:37:25 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
 *
 *      Bjørn Stabell        (bjoerns@staff.cs.uit.no)
 *      Ken Ronny Schouten   (kenrsc@stud.cs.uit.no)
 *      Bert Gÿsbers         (bert@mc.bio.uva.nl)
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
#ifndef XPM_READ_H
#define XPM_READ_H

/*
 * This XPM loader is based upon the XPM 3.0 specification.
 * It was written from scratch because we needed a small
 * and dedicated XPM interface.  Dedicated because XPilot
 * has a peculiar way of dealing with colors.
 */

/* 
 * Public interface to xpmread.c.
 */
extern Pixmap xpm_pixmap_from_data(char **data);
extern Pixmap xpm_pixmap_from_file(char *filename);
extern XImage *xpm_image_from_pixmap(Pixmap pixmap);

#ifdef XPM_READ_C
/*
 * The rest are private implementation details.
 */

/*
 * Different visual keys in the XPM format.
 */
enum XPM_key {
    XPM_m,		/* mono visual */
    XPM_g4,		/* 4-level grayscale visual */
    XPM_g,		/* grayscale visual */
    XPM_c,		/* (pseudo-)color visual */
    XPM_s,		/* symbolic color name ("None") */
    XPM_nkeys		/* number of color keys supported  */
};

/*
 * Structure for an XPM color.
 */
typedef struct XPM_color_struct {
    char		*keys[XPM_nkeys];	/* X color names */
} XPM_color;

/*
 * The real XPM user-level structure.
 */
typedef struct XPM_struct {
    unsigned		width;			/* pixmap width */
    unsigned		height;			/* pixmap height */
    unsigned		ncolors;		/* number of colors */
    unsigned		cpp;			/* chars-per-pixel. */
    XPM_color		*colors;		/* color definitions */
    unsigned char	*pixels;		/* as colors[] indices */
} XPM;

/*
 * Structure to store internal data while reading a XPM file
 * or processing a statically linked XPM structure.
 */
typedef struct XPM_read_struct {
    char		*filename;
    char		*data;
    char		**static_data;
    unsigned		data_size;
    char		*ptr;
    char		*token;
    XPM			*xpm;
    char		**chars_ptr;	/* color representation pointers */
    char		*chars_mem;	/* color representation memory */
    char		*error_str;	/* string giving error reason */
} XPM_read;

#endif

#endif

/*
 * XMapEdit, the XPilot Map Editor.  Copyright (C) 1993 by
 *
 *      Aaron Averill           <averila@oes.orst.edu>
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
 *
 * Modifications to XMapEdit
 * 1996:
 *      Robert Templeman        <mbcaprt@mphhpd.ph.man.ac.uk>
 * 1997:
 *      William Docter          <wad2@lehigh.edu>
 *
 * $Id: expose.h,v 5.0 2001/04/07 20:01:00 dik Exp $
 */

typedef struct {
   int                   color;
   int                   num_points;
   float                 x[5],y[5];
} segment_t;

typedef struct {
   float                 x,y,width,height;
   int                   start,end;
} arc_t;

extern segment_t         mapicon_seg[34];
extern int               mapicon_ptr[90];
extern char              iconmenu[36];


extern int               smlmap_x, smlmap_y, smlmap_width, smlmap_height;
extern float             smlmap_xscale, smlmap_yscale;

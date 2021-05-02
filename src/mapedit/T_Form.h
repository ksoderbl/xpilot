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
 * $Id: T_Form.h,v 5.0 2001/04/07 20:01:00 dik Exp $
 */

#define                  INACTIVE          0
#define                  ACTIVE            1
#define                  SHADED            2
#define                  OUTPUT_ONLY       3

#define                  T_BUTTON          0
#define                  T_TEXT_BUTTON     1
#define                  T_SCROLL_BOUNDED  2
#define                  T_SCROLL_UNBOUND  3
#define                  T_MULTI_BUTTON    4
#define                  T_STRING_ENTRY    5
#define                  T_TEXT            6
#define                  T_HOLD_BUTTON     7


typedef struct T_Field_t {
   char                  *name, *label;
   short                 type, active;
   short                 x,y,width,height,x2,y2;
   int                   (*handler)();
   int                   *intvar;
   char                  *charvar;
   int                   charvar_length;
   short                 null;
   struct T_Field_t      *next;
} T_Field_t;

typedef struct T_Form_t {
   Window                window;
   T_Field_t             *field;
   T_Field_t             *entry;
   int                   entry_cursor, entry_pos;
   char                  *entry_restore;
   struct T_Form_t       *next;
} T_Form_t;

typedef struct {
   T_Form_t              *form;
   T_Field_t             *field;
   unsigned int          button;
   int                   x,y,count;
} HandlerInfo;

extern T_Form_t          *T_Form;

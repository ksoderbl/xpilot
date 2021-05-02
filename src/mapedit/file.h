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
 * $Id: file.h,v 1.2 1998/04/22 15:44:07 bert Exp $
 */

/* stuff from xpilots 3.0 source */
enum valType { valInt, valReal, valBool, valIPos, valString };
 
typedef struct {
   char                  *name;
   char                  *commandLineOption;
   char                  *helpLine;
   char                  *defaultValue;
   void *variable;
   enum valType type;
} optionDesc;

extern                   Window            filepromptwin;
extern char              *StrToFloat();
extern char              oldmap[90];

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
 */

#include                 <X11/Xlib.h>
#include                 <X11/Xutil.h>
#include                 <X11/Xos.h>
#include                 <X11/Xatom.h>
#include                 <X11/keysym.h>
#include                 <stdio.h>
#include                 <stdlib.h>
#include                 <string.h>
 
#include                 "T_Form.h"
#include                 "T_Popup.h"
#include                 "T_Handler.h"
#include                 "T_proto.h"

/* Constants for T_DrawButton */
#define                  RAISED          0
#define                  LOWERED         1

/* Constants for T_DrawString */
#define                  CURSOR_CHAR     "^"
#define                  JUSTIFY_LEFT    0
#define                  JUSTIFY_RIGHT   1
#define                  JUSTIFY_CENTER  2
#define                  CROP_RIGHT      0
#define                  CROP_LEFT       1
#ifdef MONO
#define                  BKGR            T_Fore_GC
#else
#define                  BKGR            T_Back_GC
#endif

typedef char             max_str_t[255];

extern int               screennum, root_width, root_height;
extern Display           *display;
extern GC                T_Back_GC, T_Fore_GC, T_Hlgt_GC, T_Shdw_GC;
extern XFontStruct       *T_Font;
extern const char        *T_Background,*T_Foreground,*T_Shadow,*T_Highlight;
extern Atom              ProtocolAtom;
extern Atom              KillAtom;



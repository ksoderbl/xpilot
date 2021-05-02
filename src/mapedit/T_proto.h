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
 * $Id: T_proto.h,v 5.0 2001/04/07 20:01:00 dik Exp $
 */

/* T_Toolkit.c prototypes */
void             T_ConnectToServer(char *display_name);
void             T_CloseServerConnection(void);
void             T_SetToolkitFont(char *font);
int              T_GetGC(GC *gc, char *foreground);
int              T_FontInit(XFontStruct **fontinfo, char *fontname);
Window           T_MakeWindow(int x, int y, int width,int height,
                 char *fg, char *bg);
void             T_SetWindowName(Window window, char windowname[],
                 char iconname[]);
void             T_SetWindowSizeLimits(Window window, int minwidth,
                 int minheight, int maxwidth, int maxheight,
                 int aspectx, int aspecty);

void             T_ClearArea(Window win, int x, int y, int width,
                 int height);
void             T_DrawButton(Window win, int x, int y, int width,
                 int height, int zheight, int clear);
void             T_PopButton(Window win, int x, int y, int width,
                 int height, int zheight);
void             T_DrawTextButton(Window win, int x, int y, int width,
                 int height, int zheight,char *string);
void             T_DrawString(Window win, int x, int y, int width,
                 int height, GC gc, char *string, int justify,
                 int crop, int cursorpos);
void             T_DrawText(Window win, int x, int y, int width, int height,
                 GC gc,  char *text);

/* T_Form.c prototypes */
void             T_FormEventCheck(XEvent *report);
void             T_FormExpose(XEvent *report);
void             T_FormButtonPress(XEvent *report);
void             T_FormKeyPress(XEvent *report);
void             CallFieldHandler(T_Form_t *form, T_Field_t *field, int x,
                 int y, unsigned int button, int count, int (*handler)());
void             T_FormClear(Window win);
void             T_FormCloseWindow(Window win);
T_Form_t         **SeekForm(Window win, short add);
void             ChangeField(Window win, char *name, char *label,
                 short type, short active, short x, short y, short width,
                 short height, short x2, short y2, int (*handler)(),
                 int *intvar, char *charvar, int charvar_length, short null);
void             T_FormButton(Window win, char *name, short x, short y,
                 short width, short height, char *label, int (*handler)());
void             T_FormHoldButton(Window win, char *name, short x, short y,
                 short width, short height, char *label, int (*handler)());
void             T_FormMultiButton(Window win, char *name, short x, short y,
                 short width, short height, short x2, short y2, char *label,
                 int *intvar, short no_null);
void             T_FormScrollArea(Window win, char *name, short type, short x,
                 short y, short width, short height, int (*handler)());
void             T_FormText(Window win, char *name, short x, short y,
                 short width, short height, char *label, short justify);
void             T_FormStringEntry(Window win, char *name, short x, short y,
                 short width, short height, short x2, short y2, char *label,
                 char *charvar, int charvar_length, int (*handler)());
void             T_DrawEntryField (T_Form_t *form, T_Field_t *field);
void             T_SetEntryField(T_Form_t *form, T_Field_t *field, int x);
void             T_FormRedrawEntryField(char *charvar);

/* T_Popup.c prototypes */
Window           T_PopupCreate(int x, int y, int width, int height,
                 char *title);
Window           T_PopupAlert(int type, char *message, char *btn1,
                 char *btn2, int (*handler1)(), int (*handler2)());
Window           T_PopupPrompt(int x, int y, int width, int height,
                 char *title, char *message, char *btn1, char *btn2,
                 char *charvar, int length, int (*handler)());
int              T_IsPopupOpen(Window win);
void             T_PopupClose(Window win);

/* T_Handler.c prototypes */
int              ValidateFloatHandler(HandlerInfo info);
int              ValidatePositiveFloatHandler(HandlerInfo info);
int              ValidateIntHandler(HandlerInfo info);
int              ValidatePositiveIntHandler(HandlerInfo info);
int              PopupCloseHandler(HandlerInfo info);
int              FormCloseHandler(HandlerInfo info);


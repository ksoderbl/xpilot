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
 * $Id: proto.h,v 1.3 1998/04/23 19:44:12 bert Exp $
 */

/*
 * Prototypes
 */

/* prototypes for main.c */
void            main(int argc, char *argv[]);
void            SetDefaults(int argc, char *argv[]);
void            ParseArgs(int argc, char *argv[]);
void            ResetMap(void);
void            SizeMapwin(void);
void            SizeSmallMap(void);
void            SizeMapIcons(int zoom);
void            Setup_default_server_options();

/* prototypes for expose.c */
void            DrawTools(void);
void            DrawMap(int x, int y, int width, int height);
void            DrawMapSection(int x, int y, int width, int height, 
                int xpos, int ypos);
void            DrawMapPic(Window win, int x, int y, int picnum, int zoom);
void            DrawSmallMap(void);
void            UpdateSmallMap(int x,int y);
void            DrawViewBox(void);
void            DrawViewSeg(int x1, int y1, int x2, int y2);

/* prototypes for events.c */
void            MainEventLoop(void);
void            MapwinKeyPress(XEvent *report);

/* prototypes for tools.c */
int             DrawMapIcon(HandlerInfo info);
void            SelectArea(int x, int y, int count);
void            ChangeMapData(int x, int y,char icon, int save);
int             MoveMapView(HandlerInfo info);
int             ZoomOut(HandlerInfo info);
int             ZoomIn(HandlerInfo info);
void            SizeSelectBounds(int oldvx, int oldvy);
int             ExitApplication(HandlerInfo info);
int             SaveUndoIcon(int x, int y, char icon);
int             Undo(HandlerInfo info);
void            ClearUndo(void);
int             NewMap(HandlerInfo info);
int             ResizeWidth(HandlerInfo info);
int             ResizeHeight(HandlerInfo info);
int             OpenPreferencesPopup(HandlerInfo info);
int             ValidateCoordHandler(HandlerInfo info);
int             ShowHoles(HandlerInfo info);
char            MapData(int x, int y);
int             ChangedPrompt(int (*handlen)());
void            ClearSelectArea(void);
void            DrawSelectArea(void);
int             FillMapArea(HandlerInfo info);
int             CopyMapArea(HandlerInfo info);
int             CutMapArea(HandlerInfo info);
int             PasteMapArea(HandlerInfo info);
int             NegativeMapArea(HandlerInfo info);

/* prototypes for file.c */
int             SavePrompt(HandlerInfo info);
int             SaveOk(HandlerInfo info);
int             SaveMap(char *file);
int             LoadPrompt(HandlerInfo info);
int             LoadOk(HandlerInfo info);
int             LoadMap(char *file);
int             LoadXbmFile(char *file);
int             LoadOldMap(char *file);
void            toeol(FILE *ifile);
char            skipspace(FILE *ifile);
char            *getMultilineValue(char *delimiter, FILE *ifile);
int             ParseLine(FILE *ifile);
int             AddOption(char *name, char *value);
int             YesNo(char *val);
char            *StrToNum(char *string, int len, int type);
int             LoadMapData(char *value);
char            *getMultilineValue();

/* prototypes for round.c */
int             RoundMapArea(HandlerInfo info);

/* prototypes for help.c */
int             OpenHelpPopup(HandlerInfo info);
void            BuildHelpForm(Window win, int helppage);
void            DrawHelpWin(void);
int             NextHelp(HandlerInfo info);
int             PrevHelp(HandlerInfo info);

/* prototypes for grow.c */
int             GrowMapArea(HandlerInfo info);

/* prototypes for forms.c */
void            BuildMapwinForm(void);
void            BuildPrefsForm(void);

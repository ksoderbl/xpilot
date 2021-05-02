/* $Id: winX_.h,v 1.5 1997/11/27 20:08:46 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-97 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
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

/***************************************************************************\
*  winX.h - X11 to Windoze converter										*
*																			*
*  This file is the private interface to the Winodoze -> X11 translator.	*
*  Here we hide the gory Windoze details from X11							*
*																			*
*  $Id: winX_.h,v 1.5 1997/11/27 20:08:46 bert Exp $							*
\***************************************************************************/
#ifndef	_WINX__H_
#define	_WINX__H_

#ifndef	_INC_WINDOWS
#include <windows.h>
#endif

#include "winX.h"

#define	WINXFILELENGTH	32
#define	XID_HEAD \
	int		type; \
	char	file[WINXFILELENGTH]; \
	int		line;

struct	XID_HWND {
	XID_HEAD
	HDC		hBmpDC;			// put this first so we can sneak use it easily
	HDC		hSaveDC;
	HBITMAP	hBmp;
	HWND	hWnd;
	int		drawtype;		// DT_1=0=1 bmp (use hBmpDC), DT_2=1=2 bmps (use hBmpDCa)
#define		DT_1	0
#define		DT_2	1

	long	event_mask;		// which events this window cares about	
	BOOL	mouseover;		// used to track which window was "entered"
	int		bgcolor;
	HDC		hBmpDCa[2];
	HBITMAP	hBmpa[2];		// 2 bitmaps for ThreadedDraw
	BOOL	filling;		// which bitmap ThreadedDraw is filling
	BOOL	notmine;		// This window was not created by winX, and shouldn't be destroyed. (like top)
#ifdef PENS_OF_PLENTY
	int		cur_color;
	int		line_style;
	int		line_width;
	BOOL	nodash;
#endif
};
typedef struct XID_HWND XID_HWND;

struct	XID_GC {
	XID_HEAD
	int		xidhwnd;
	HFONT	hfont;
	Font	font;
};
typedef struct	XID_GC XID_GC;

struct	XID_PIXMAP {
	XID_HEAD
	HDC		hDC;			// In windows, a Bitmap is useless w/out a DC
	HBITMAP	hbm;
};
typedef	struct	XID_PIXMAP XID_PIXMAP;

struct	XID_FONT {
	XID_HEAD
	XFontStruct*	font;
};
typedef struct XID_FONT XID_FONT;

struct	XID_ANY {
	XID_HEAD
};
typedef	struct XID_ANY XID_ANY;

union XIDTYPE {
	int	type;
	XID_HWND	hwnd;
	XID_GC		hgc;
	XID_PIXMAP	hpix;
	XID_FONT	font;
	XID_ANY		any;
};
typedef	union	XIDTYPE	XIDTYPE;

// the types in the XIDTYPE	array;
#define	XIDTYPE_UNUSED	0		// slot is free
#define	XIDTYPE_HWND	1
#define	XIDTYPE_HDC		2
#define	XIDTYPE_PIXMAP	3
#define	XIDTYPE_FONT	4

#define	MAX_XIDS	512			// # of X resources we handle
extern	XIDTYPE	xid[MAX_XIDS];

//#define	GetTophWnd()	xid[0].hwnd.hWnd

#ifdef PENS_OF_PLENTY
struct winXobj {
	HPEN		pen;
	HPEN		dashpen;
	HPEN		cdashpen;
	HPEN		fatpen;
	HBRUSH		brush;
	COLORREF	color;
	HBITMAP		bitmask;
};
#else
struct winXobj {
	HPEN		pen;
	HBRUSH		brush;
	COLORREF	color;
//	HBITMAP		bitmask;	not used
};
#endif
typedef	struct	winXobj winXobj;
extern	winXobj objs[];

extern	HINSTANCE		hInstance;
extern	BOOL			bWinNT;				// need this 'cause Win95 can't draw a simple circle
extern	HFONT			hFixedFont;
//extern	unsigned int	max_xid;		// point to next free one in array
extern	BOOL			bHasPal;			// Are we palette or colour based?
extern	HPALETTE		myPal;
extern	int				cur_color;

extern	Window			rootWindow;

extern	BOOL AngleArc2(HDC hdc, int X, int Y, DWORD dwRadius,
				       double fStartDegrees, double fSweepDegrees, BOOL bFilled);
extern	COLORREF WinXPColour(int ColourNo);

// Scaling window stuff
#define	SCALEPREC	100
//extern	int iScaleFactor;
//#define	WinXScale(x)	( (int)(x) * SCALEPREC / iScaleFactor )
//#define	WinXUnscale(x)	( (int)(x) * iScaleFactor / SCALEPREC )
#define WinXScale(_x)	(_x)
#define	WinXUnscale(_x)	(_x)

// Score window colour
#define	SCOREWIN		2

extern	XID	GetFreeXid();
extern	void WinXFree(XID xid);

#endif	/* _WINX__H_ */



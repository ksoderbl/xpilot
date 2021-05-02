/* $Id: winX11.c,v 5.0 2001/04/07 20:00:59 dik Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
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
*  winX11.c - X11 to Windoze converter										*
*																			*
*  This file is the direct interface to the X11 library.					*
*  Any function that has a unix man page belongs in this file.				*
\***************************************************************************/

#include "winX.h"
#include "winX_.h"

#include "../error.h"
#include "../../client/NT/winClient.h" 	/* This needs to be removed */

const int	top = 0;
/*****************************************************************************/
XFillRectangle(Display* dpy, Drawable d, GC gc, int x, int y, 
					   unsigned int w, unsigned int h)
{
	RECT	r;
	HDC		hDC = xid[d].hwnd.hBmpDC;
#ifdef PENS_OF_PLENTY
	int		cur_color = xid[d].hwnd.cur_color;
#endif
	r.left   = x;
	r.right  = x+w;
	r.top    = y;
	r.bottom = y+h;
	FillRect(hDC, &r, objs[cur_color].brush);
	return(0);
}

/*****************************************************************************/
XDrawRectangle(Display* dpy, Drawable d, GC gc, int x, int y, 
					   unsigned int w, unsigned int h)
{

	RECT	r;
	HDC		hDC = xid[d].hwnd.hBmpDC;
#ifdef PENS_OF_PLENTY
	int		cur_color = xid[d].hwnd.cur_color;
#endif
	r.left   = x;
	r.right  = x+w;
	r.top    = y;
	r.bottom = y+h;
	FrameRect(hDC, &r, objs[cur_color].brush);
	return(0);
}

XFillRectangles(Display* dpy, Drawable d, GC gc, 
				XRectangle* rects, int nrectangles)
{
	RECT	r;
	int		i;
	HDC		hDC = xid[d].hwnd.hBmpDC;
#ifdef PENS_OF_PLENTY
	int		cur_color = xid[d].hwnd.cur_color;
#endif

	for (i=0; i<nrectangles; i++, rects++)
	{
		r.left   = rects->x;
		r.right  = rects->x + rects->width;
		r.top    = rects->y;
		r.bottom = rects->y + rects->height;
		FillRect(hDC, &r, objs[cur_color].brush);
	}
	return(0);
}
/*****************************************************************************/
XDrawLine(Display* dpy, Drawable d, GC gc, int x1, int y1, int x2, int y2)
{
	HDC		hDC = xid[d].hwnd.hBmpDC;
	MoveToEx(hDC, x1, y1, NULL);
	LineTo(hDC, x2, y2);
	return(0);
}

/*****************************************************************************/
XDrawLines(Display* dpy, Drawable d, GC gc, XPoint* points,
				   int npoints, int mode)
{
	int	i = 0;
	HDC		hDC = xid[d].hwnd.hBmpDC;

	if (mode == CoordModePrevious)
	{
		int	x, y;
		x = points->x;
		y = points->y;
		MoveToEx(hDC, x, y, NULL);
		points++;
		for (i=1; i<npoints; i++, points++)
		{
			x += points->x;
			y += points->y;
			LineTo(hDC, x, y);
		}

	}
	else
	{
		MoveToEx(hDC, points->x, points->y, NULL);
		points++;
		for (i=1; i<npoints; i++, points++)
		{
			LineTo(hDC, points->x, points->y);
		}
	}
	return(0);
}

/*****************************************************************************/
XDrawSegments(Display* dpy, Drawable d, GC gc, 
					  XSegment* segments, int nsegments)
{
	int i;
	HDC		hDC = xid[d].hwnd.hBmpDC;

	for (i=0; i<nsegments; i++, segments++)
	{
		MoveToEx(hDC, segments->x1, segments->y1, NULL);
		LineTo(hDC, segments->x2, segments->y2);
	}

	return(0);
}

/*****************************************************************************/
XDrawPoint(Display* dpy, Drawable d, GC gc, int x, int y)
{
	HDC		hDC = xid[d].hwnd.hBmpDC;
#ifdef PENS_OF_PLENTY
	int		cur_color = xid[d].hwnd.cur_color;
#endif

	SetPixelV(hDC, x, y, WinXPColour(cur_color));
	return(0);
}

/*****************************************************************************/
XDrawPoints(Display* dpy, Drawable d, GC gc, 
					XPoint* points, int npoints, int mode)
{
	int		i;
	HDC		hDC = xid[d].hwnd.hBmpDC;
#ifdef PENS_OF_PLENTY
	int		cur_color = xid[d].hwnd.cur_color;
#endif

	for (i=0; i<npoints; i++, points++)
		SetPixelV(hDC, points->x, points->y, WinXPColour(cur_color));

	return(0);
}

/*****************************************************************************/
XFillPolygon(Display* dpy, Drawable d, GC gc, XPoint* points,
					int npoints, int shape, int mode)
{
	HDC		hDC = xid[d].hwnd.hBmpDC;
	int		i;

	BeginPath(hDC);
	MoveToEx(hDC, points->x, points->y, NULL);
	points++;
	for (i=1; i<npoints; i++, points++)
		LineTo(hDC, points->x, points->y);
	EndPath(hDC);
	StrokeAndFillPath(hDC);
	return(0);
}

/*****************************************************************************/
XDrawArc(Display* dpy, Drawable d, GC gc, int x, int y,
				 unsigned int width, unsigned int height,
				 int angle1, int angle2)
{
	HDC		hDC = xid[d].hwnd.hBmpDC;
	MoveToEx(hDC, x+width, y+height/2, NULL);
	if (bWinNT)
		AngleArc (hDC, x+width/2, y+height/2, width/2, (float)0.0, (float)(angle2/64.0));
	else
		AngleArc2(hDC, x+width/2, y+height/2, width/2, 0.0, angle2/64.0, FALSE);
	return(0);
}

/*****************************************************************************/
XDrawArcs(Display* dpy, Drawable d, GC gc, XArc* arcs, int narcs)
{
	int		i;
	HDC		hDC = xid[d].hwnd.hBmpDC;

	for (i=0; i<narcs; i++, arcs++)
	{
		MoveToEx(hDC, arcs->x+arcs->width, arcs->y+arcs->height/2, NULL);
		if (bWinNT)
			AngleArc (hDC, arcs->x+arcs->width/2, arcs->y+arcs->height/2,
						   arcs->width/2, (float)0.0, (float)(arcs->angle2/64.0));
		else
			AngleArc2(hDC, arcs->x+arcs->width/2, arcs->y+arcs->height/2,
						   arcs->width/2, 0.0, arcs->angle2/64.0, FALSE);
	}
	return(0);
}

/*****************************************************************************/
XFillArc(Display* dpy, Drawable d, GC gc, int x, int y,
				 unsigned int width, unsigned int height,
				 int angle1, int angle2)
{

	HDC		hDC = xid[d].hwnd.hBmpDC;

	// 2 separate bits 'cause Win95 doesn't put Arc into current path
	// We use Pie instead.
	if (bWinNT)
	{
		BeginPath(hDC);
		MoveToEx(hDC, x+width, y+height/2, NULL);
		AngleArc (hDC, x+width/2, y+height/2, width/2, (float)0.0, (float)(angle2/64.0));
		EndPath(hDC);
		StrokeAndFillPath(hDC);
	}
	else
	{
		MoveToEx(hDC, x+width, y+height/2, NULL);
		AngleArc2(hDC, x+width/2, y+height/2, width/2, 0.0, angle2/64.0, TRUE);
	}
	return(0);
}

/*****************************************************************************/
XDrawString(Display* dpy, Drawable d, GC gc, int x, int y, 
					const char* string, int length)
{
	HDC		hDC = xid[d].hwnd.hBmpDC;
	HFONT	hOldFont = NULL;

/*	if (d == players)
		hOldFont = SelectObject(hDC, hFixedFont);
	else if (d == draw)
		hOldFont = SelectObject(hDC, xid[gc].hgc.hfont);
	else */
		hOldFont = SelectObject(hDC, xid[gc].hgc.hfont);
	SetTextColor(hDC, WinXPColour(cur_color));
//	TextOut(hDC, x, y-16, string, length);
	TextOut(hDC, x, y-xid[xid[gc].hgc.font].font.font->ascent, string, length);
	if (hOldFont)
		SelectObject(hDC, hOldFont);

	return(0);
}

/*****************************************************************************/
XTextWidth(XFontStruct* font, const char* string, int length)
{
//	HDC		hDC = xid[draw].hwnd.hBmpDC;
	HDC		hDC = xid[top].hwnd.hBmpDC;
	SIZE	size;
	XID		i, f;
	for (f=0; f<MAX_XIDS; f++)
	{
		if (xid[f].type == XIDTYPE_FONT && xid[f].font.font == font)
			break;
	}
	if (f == MAX_XIDS)
	{
		Trace("Huh? Can't match font for string <%s>\n", string);
	}
	else
	{
		for (i=0; i<MAX_XIDS; i++)
			if (xid[i].type == XIDTYPE_HDC)
				if (xid[i].hgc.font == f)
					break;
	}
	if (i == MAX_XIDS)
	{
		Trace("Huh? Can't find a GC for font %d, string=<%s>\n", f, string);
	}
	else
	{
		hDC = xid[xid[i].hgc.xidhwnd].hwnd.hBmpDC;
		SelectObject(hDC, xid[i].hgc.hfont);
	}

	GetTextExtentPoint32(hDC, string, length, &size);
/*	return WinXUnscale(size.cx); */
	return(size.cx);
}

/*****************************************************************************/
XChangeGC(Display* dpy, GC gc, unsigned long valuemask, XGCValues* values)
{
#ifdef PENS_OF_PLENTY
	if (xid[gc].type == XIDTYPE_HDC)
	{
		int xidno = xid[gc].hgc.xidhwnd;
	
		if (valuemask & GCLineWidth)
			xid[xidno].hwnd.line_width = values->line_width;
		if (valuemask & GCLineStyle)
			xid[xidno].hwnd.line_style = values->line_style;
		
		WinXSetPen(xidno);
	}
#endif
	return(0);
}

/*****************************************************************************/
XSetLineAttributes(Display* dpy, GC gc, unsigned int lwidth,
						   int lstyle, int cap_style, int join_style)
{
#ifdef PENS_OF_PLENTY
	if (xid[gc].type == XIDTYPE_HDC)
	{
		int xidno = xid[gc].hgc.xidhwnd;
	
		xid[xidno].hwnd.line_width = lwidth;
		xid[xidno].hwnd.line_style = lstyle;

		WinXSetPen(xidno);
	}
#endif
	return(0);
}

/*****************************************************************************/
XCopyArea(Display* dpy, Drawable src, Drawable dest, GC gc,
				  int src_x, int src_y, unsigned int width, unsigned int height,
				  int dest_x, int dest_y)
{
	return(0);
}

/*****************************************************************************/
XSetTile(Display* dpy, GC gc, Pixmap tile)
{
	return(0);
}

/*****************************************************************************/
XSetTSOrigin(Display* dpy, GC gc, int ts_x_origin, int ts_y_origin)
{
	return(0);
}

/*****************************************************************************/
XSetFillStyle(Display* dpy, GC gc, int fill_style)
{
	return(0);
}

/*****************************************************************************/
XSetFunction(Display* dpy, GC gc, int function)
{
	return(0);
}

/*****************************************************************************/
XBell(Display* dpy, int percent)
{
	MessageBeep(MB_ICONEXCLAMATION);
	return(0);
}

/*****************************************************************************/
XFlush(Display* dpy)
{
	return(0);
}

/*****************************************************************************/
XCreatePixmap(Display* dpy, Drawable d, 
					  unsigned int width, unsigned int height, unsigned int depth)
{
	XID		txid;
	HDC		hDC = xid[d].hwnd.hBmpDC;
	HDC		newDC = CreateCompatibleDC(NULL);
//	HDC		hScreenDC = GetDC(xid[draw].hwnd.hWnd);
	HDC		hScreenDC = GetDC(xid[top].hwnd.hWnd);
	HBITMAP	hbm = CreateCompatibleBitmap(hScreenDC, width, height);
	
//	ReleaseDC(xid[draw].hwnd.hWnd, hScreenDC);
	ReleaseDC(xid[top].hwnd.hWnd, hScreenDC);

	if (!hbm)
	{
		error("Can't create pixmap\n");
		return(0);
	}
	SelectObject(newDC, hbm);
	if (bHasPal)
	{
		SelectPalette(newDC, myPal, FALSE);
		RealizePalette(newDC);
	}

	txid = GetFreeXid();
	xid[txid].type = XIDTYPE_PIXMAP;
	xid[txid].hpix.hbm = hbm;
	xid[txid].hpix.hDC = newDC;
#if 0
	if (++max_xid > MAX_XIDS)
	{
		error("Too many XIDS!\n");
		max_xid--;
	}
	return(max_xid-1);
#endif
	return(txid);
}

/*****************************************************************************/
XFreePixmap(Display* dpy, Pixmap pixmap)
{
	return(0);
}

/*****************************************************************************/
XSetPlaneMask(Display* dpy, GC gc, unsigned long plane_mask)
{
	return(0);
}

/*****************************************************************************/
XClearWindow(Display* dpy, Window w)
{
	HWND	hWnd = xid[w].hwnd.hWnd;
	HDC		hDC = xid[w].hwnd.hBmpDC;
	RECT	rect;

	GetClientRect(hWnd, &rect);
#ifdef SPECIAL_WIN_COLOURS
	FillRect(hDC, &rect, GetStockObject(GRAY_BRUSH));
#else
	Trace("XClearWindow: %d color=%d %d/%d %d/%d\n", w, xid[w].hwnd.bgcolor,
		rect.left, rect.top, rect.right, rect.bottom);
	FillRect(hDC, &rect, objs[xid[w].hwnd.bgcolor].brush);
//	InvalidateRect(hWnd, &rect, FALSE);
#endif
	return(0);
}

/*****************************************************************************/
XClearArea(Display* d, Window w, int x, int y, 
		   unsigned int width, unsigned int height, Bool exposures)
{
	HWND	hWnd = xid[w].hwnd.hWnd;
	HDC		hDC = xid[w].hwnd.hBmpDC;
	RECT	rect;

//	GetClientRect(hWnd, &rect);
	rect.left = x;
	rect.top = y;
	rect.right = x+width;
	rect.bottom = y+height;
	Trace("XClearArea: %d color=%d %d/%d %d/%d\n", w, xid[w].hwnd.bgcolor,
		rect.left, rect.top, rect.right, rect.bottom);
	FillRect(hDC, &rect, objs[xid[w].hwnd.bgcolor].brush);
	return(0);
}

/*****************************************************************************/
XStoreName(Display* dpy, Window w, const char* window_name)
{
	SetWindowText(xid[w].hwnd.hWnd, window_name);
	return(0);
}

/*****************************************************************************/
XLookupKeysym(XKeyEvent* key_event, int index)
{
	return(key_event->keycode);
}

/*****************************************************************************/
XFontStruct* XQueryFont(Display* dpy, XID font_ID)
{
	return(xid[xid[font_ID].hgc.font].font.font);
}

/*****************************************************************************/
XSetFont(Display* dpy, GC gc, Font font)
{
	xid[gc].hgc.hfont = xid[font].font.font->hFont;
	xid[gc].hgc.font = font;
	return(0);
}

/*****************************************************************************/
Window XCreateSimpleWindow_(Display* dpy, Window parent, int x, int y,
						   unsigned int width, unsigned int height,
						   unsigned int border_width, unsigned long border,
						   unsigned long background
#ifdef	_DEBUG
						   , const char* file, const int line
#endif
						   )
{
	HWND	hWnd;
	XID		txid;
	int	wstyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	txid = GetFreeXid();
	xid[txid].hwnd.type = XIDTYPE_HWND;
	xid[txid].hwnd.hBmp = NULL;
	xid[txid].hwnd.hBmpDC = NULL;
	xid[txid].hwnd.hWnd = NULL;
	xid[txid].hwnd.event_mask = ButtonPressMask | ButtonReleaseMask 
		| LeaveWindowMask | EnterWindowMask;
	xid[txid].hwnd.event_mask = -1;		// hell, let's take em all!
	xid[txid].hwnd.mouseover = 0;		// mouse not over this window
	xid[txid].hwnd.bgcolor = background;
	xid[txid].hwnd.notmine = FALSE;
	xid[txid].hwnd.drawtype = DT_1;

#ifdef	_DEBUG
	strncpy(xid[txid].any.file,
		strrchr(file, '\\')+1, WINXFILELENGTH-1);
	xid[txid].any.line = line;
#endif

	switch(border_width)
	{
	case 1:
		wstyle = WS_BORDER | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		break;
	case 2:
		wstyle = WS_DLGFRAME | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		break;
	case 3:
		wstyle = WS_DLGFRAME | WS_POPUP | WS_VISIBLE;
		wstyle = WS_DLGFRAME | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU;
		y += GetSystemMetrics(SM_CYCAPTION);
		break;
	}
	Trace("XCreateSimpleWindow_: %d p=%d, %d/%d %d/%d b_width=%d\n", txid, parent, 
		x,y,width,height, border_width);
	hWnd = xid[txid].hwnd.hWnd = 
		CreateWindow("XPilotWin", "", wstyle,
					 x, y, width, height, xid[parent].hwnd.hWnd,
					 NULL, hInstance, (LPVOID)txid);
	if (!hWnd)
		error("Can't open window\n");
	else
	{
		HDC		hDC = GetDC(hWnd);
		if (bHasPal)
		{
			SelectPalette(hDC, myPal, FALSE);
			RealizePalette(hDC);
		}
//		SetBkMode(hDC, TRANSPARENT);
		SetBkMode(hDC, OPAQUE);
		ReleaseDC(hWnd, hDC);
	}
#if 0
	if (++max_xid > MAX_XIDS)
	{
		error("Too many XIDS!\n");
		max_xid--;
	}
	return(max_xid-1);
#endif
	return(txid);
}

/*****************************************************************************/
Window XCreateWindow_(Display* dpy, Window parent, int x, int y,
					 unsigned int width, unsigned int height,
					 unsigned int border_width, int depth, unsigned int c_class,
					 Visual* visual, unsigned long valuemask,
					 XSetWindowAttributes* attributes
#ifdef	_DEBUG
					 , const char* file, const int line
#endif
					 )
{
	Window	w = XCreateSimpleWindow_(dpy, parent, x, y, width, height, 
		border_width, 0, 0
#ifdef	_DEBUG
		,file, line
#endif
		);
	xid[w].hwnd.event_mask = attributes->event_mask;
	return(w);
}

GC WinXCreateWinDC_(Window w
#ifdef	_DEBUG
					 , const char* file, const int line
#endif
					)
{
	XID		txid;
	txid = GetFreeXid();
	xid[txid].hgc.type = XIDTYPE_HDC;
	xid[txid].hgc.xidhwnd = w;

#ifdef	_DEBUG
	strncpy(xid[txid].any.file,
		strrchr(file, '\\')+1, WINXFILELENGTH-1);
	xid[txid].any.line = line;
#endif

#if 0
	if (++max_xid > MAX_XIDS)
	{
		error("Too many XIDS!\n");
		max_xid--;
	}
	return(max_xid-1);
#endif
	return(txid);
}

/*****************************************************************************/
XSetForeground(Display* dpy, GC gc, unsigned long foreground)
{
#ifdef PENS_OF_PLENTY
	if (xid[gc].type == XIDTYPE_PIXMAP)
	{
		HDC hDC = xid[gc].hpix.hDC;
		SetTextColor(hDC, WinXPColour(foreground));
		SelectObject(hDC, objs[foreground].brush);
		SelectObject(hDC, objs[foreground].pen);
	}
	else
	{
		int xidno = xid[gc].hgc.xidhwnd;
		xid[xidno].hwnd.cur_color = foreground;
		WinXSetPen(xidno);
	}
#else
	HDC hDC;
	Window	w;
	cur_color = foreground;
	if (xid[gc].type == XIDTYPE_PIXMAP)
	{
		hDC = xid[gc].hpix.hDC;
		SetTextColor(hDC, WinXPColour(cur_color));
		SelectObject(hDC, objs[cur_color].pen);
		SelectObject(hDC, objs[cur_color].brush);
	}
	else
	{
		w = xid[gc].hgc.xidhwnd;
		if (xid[w].hwnd.drawtype == DT_1)
		{
			hDC = xid[w].hwnd.hBmpDC;
			SetTextColor(hDC, WinXPColour(cur_color));
			SelectObject(hDC, objs[cur_color].pen);
			SelectObject(hDC, objs[cur_color].brush);
		}
		else	/* DT_2 */
		{
			SetTextColor(xid[w].hwnd.hBmpDCa[0], WinXPColour(cur_color));
			SelectObject(xid[w].hwnd.hBmpDCa[0], objs[cur_color].pen);
			SelectObject(xid[w].hwnd.hBmpDCa[0], objs[cur_color].brush);
			SetTextColor(xid[w].hwnd.hBmpDCa[1], WinXPColour(cur_color));
			SelectObject(xid[w].hwnd.hBmpDCa[1], objs[cur_color].pen);
			SelectObject(xid[w].hwnd.hBmpDCa[1], objs[cur_color].brush);
		}
	}
#endif
	return(0);
}


/*****************************************************************************/
XDestroyWindow(Display* dpy, Window w)
{
	WinXFree(w);
	return(0); 
}

/*****************************************************************************/
XUnmapWindow(Display* dpy, Window w)
{	
	ShowWindow(xid[w].hwnd.hWnd, SW_HIDE);
	return(0); 
}

/*****************************************************************************/
XMapWindow(Display* dpy, Window w)
{
	//ShowWindow(xid[w].hwnd.hWnd, SW_SHOW);
	XMapRaised(dpy, w);
	return(0); 
}

/*****************************************************************************/
XMapSubwindows(Display* dpy, Window w)
{
	ShowWindow(xid[w].hwnd.hWnd, SW_SHOW);
	return(0); 
}

/*****************************************************************************/
XMapRaised(Display* dpy, Window w)
{
	ShowWindow(xid[w].hwnd.hWnd, SW_SHOW);
	//SetWindowPos(xid[w].hwnd.hWnd, HWND_TOPMOST,
	//	0, 0, 0, 0,
	//	SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
	BringWindowToTop(xid[w].hwnd.hWnd);
	return(0);
}

/*****************************************************************************/
XMoveWindow(Display* dpy, Window w, int x, int y)
{
	RECT	rect;
	GetClientRect(xid[w].hwnd.hWnd, &rect);
	MoveWindow(xid[w].hwnd.hWnd, x, y, rect.right, rect.bottom, TRUE);
	return(0); 
}

/*****************************************************************************/
XMoveResizeWindow(Display* dpy, Window w, int x, int y,
						  unsigned int width, unsigned int height)
{
	HWND	hWnd = xid[w].hwnd.hWnd;
	
	SetWindowPos(hWnd, NULL, x, y, width, height, SWP_NOZORDER);
	return(0);
}

/*****************************************************************************/
XSetDashes(Display* dpy, GC gc, int dash_offset, 
		   const char *dash_list, int n)
{
#ifdef PENS_OF_PLENTY
	if (xid[gc].type == XIDTYPE_HDC)
	{
		int	xidno = xid[gc].hgc.xidhwnd;
		
		xid[xidno].hwnd.nodash = (dash_list == cdashes);
		WinXSetPen(xidno);
	}
#endif
	return(0);
}

/*****************************************************************************/
XSelectInput(Display* dpy, Window w, long event_mask)
{
	xid[w].hwnd.event_mask = event_mask |ButtonPressMask | ButtonReleaseMask;
	return(0);
}


/*****************************************************************************/
Window DefaultRootWindow(Display* dpy)
{
/*	return(top); */
	return(0);
}

/*****************************************************************************/
int	DisplayWidth(Display* dpy, int screen_number)
{
	RECT	rect;
	GetWindowRect(GetDesktopWindow(), &rect);
	GetWindowRect(xid[top].hwnd.hWnd, &rect);
	return(rect.right);
}

/*****************************************************************************/
int	DisplayHeight(Display* dpy, int screen_number)
{
	RECT	rect;
	GetWindowRect(GetDesktopWindow(), &rect);
	GetWindowRect(xid[top].hwnd.hWnd, &rect);
	return(rect.bottom);
}

/*****************************************************************************/
Bool XCheckIfEvent(Display* dpy, XEvent* event_return, 
				   Bool (*predicate)(), XPointer arg)
{	return(FALSE); }

/*****************************************************************************/
XSetIconName(Display* dpy, Window w, const char* icon_name)
{	return(0); }

/*****************************************************************************/
XSetTransientForHint(Display* dpy, Window w, Window prop_window)
{	return(0); }

/*****************************************************************************/
XChangeWindowAttributes(Display* dpy, Window w, unsigned long valuemask,
						XSetWindowAttributes* attributes)
{	return(0); }

/*****************************************************************************/
XGetWindowAttributes(Display* dpy, Window w, XWindowAttributes* attributes)
{
	RECT	rect;
	GetClientRect(xid[w].hwnd.hWnd, &rect);
	attributes->x = 
	attributes->y = 0;
	attributes->width = rect.right;
	attributes->height = rect.bottom;
	return(0); 
}

/*****************************************************************************/
GContext XGContextFromGC(GC gc)
{	return(gc); }

/*****************************************************************************/
int XGrabPointer(Display *display, Window w, Bool owner_events,
						 unsigned int event_mask, int pointer_mode, int keyboard_mode,
						 Window confine_to, Cursor cursor, Time time)
{
	SetCapture(xid[w].hwnd.hWnd);
	xid[w].hwnd.event_mask = event_mask;			/* this could be a macro, */
//	WinXSetEventMask(w, event_mask);
	return(0); 
}

/*****************************************************************************/
XUngrabPointer(Display* display, Time time)
{
	ReleaseCapture();
	return(0); 
}

/*****************************************************************************/
XWarpPointer(Display *display, Window src_w, Window dest_w,
					 int src_x, int src_y,
					 unsigned int src_width, unsigned int src_height,
					 int dest_x, int dest_y)
{
#ifdef _WINDOWS
	SetCursorPos(dest_x, dest_y);
#else
	RECT	rect;
	GetWindowRect(xid[dest_w].hwnd.hWnd, &rect);
	SetCursorPos(rect.left+dest_x, rect.top+dest_y);
#endif
	return(0); 
}
/*****************************************************************************/
XDefineCursor(Display* d, Window w, Cursor c)
{
	if (c == None)
		ShowCursor(TRUE);
	else
		ShowCursor(FALSE);
	return(0); 
}

/*****************************************************************************/
int	DefaultDepth(Display* d, int screen)
{
	return(8);		// assume 256 colors (bad assumption but OK for now)
}


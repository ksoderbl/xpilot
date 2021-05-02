/* $Id: winX.c,v 4.1 1998/04/16 17:41:05 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
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
*  winX.c - X11 to Windoze converter										*
*																			*
*  This file is mostly Win32 translations of the X calls that xpilot uses.	*
*  Anything starting with WinX is a special wedge function that i needed	*
*																			*
*  $Id: winX.c,v 4.1 1998/04/16 17:41:05 bert Exp $						*
\***************************************************************************/
#include "winX.h"
#include "windows.h"
#include "../../client/NT/winClient.h"
#include "winX_.h"
#include "../../client/NT/winXThread.h"
#include <math.h>

#include "../error.h"
#include "../const.h"
#include "../draw.h"
#include "../../client/paint.h"
#include "../../client/xinit.h"
#include "../../client/widget.h"
#include "../../client/protoclient.h"

int iScaleFactor;

// Radar window is updated every RadarDivisor frames.
int RadarDivisor;
int	ThreadedDraw;

extern	Window	draw;		// we only want this one

Window	rootWindow = 0;		// The whole screen

static  double fTwoPi = 2.0 * PI;

HINSTANCE	hInstance;
HPALETTE	myPal;
LOGPALETTE*	myLogPal;
HFONT		hFixedFont;
HHOOK		mousehook = NULL;

HDC			itemsDC;		// for blitting items onto the screen
#define		WINMAXCOLORS	16

int			winmaxcolors;
int			TotalPens = 0;

XIDTYPE	xid[MAX_XIDS];
//unsigned int	max_xid = 0;			// point to next free one in array

BOOL	bWinNT = 0;				// need this 'cause Win95 can't draw a simple circle
BOOL	bHasPal = TRUE;			// Are we palette or colour based?
BOOL	drawPending = FALSE;	// try to throttle the deadly frame backup syndrome

#ifdef PENS_OF_PLENTY
DWORD	dwdashes[NUM_DASHES+1] = { NUM_DASHES, 8, 4 };
DWORD	dwcdashes[NUM_CDASHES+1] = { NUM_CDASHES, 3, 9 };
winXobj	objs[WINMAXCOLORS];
#else
int		cur_color;
winXobj objs[WINMAXCOLORS+FUNKCOLORS];
#endif

// We need to parse and setup the colors during Windows screen init which happens
// before XPilot window init.
extern	COLORREF GetXPilotColor(int which, COLORREF defcolor);
extern	int		 GetMaxColors();
/*extern	int		 GetScoreFontHeight();*/

void WinXExit();

static void WinXSetupRadarWindow()
{
	if (radar)
	{
		if (instruments & SHOW_SLIDING_RADAR)
		{
			if (xid[radar].hwnd.hSaveDC != NULL)
			{
				ReleaseDC(xid[radar].hwnd.hWnd, xid[radar].hwnd.hBmpDC);
				xid[radar].hwnd.hBmpDC = xid[radar].hwnd.hSaveDC;
				xid[radar].hwnd.hSaveDC = NULL;
			}
		}
		else
		{
			if (xid[radar].hwnd.hSaveDC == NULL)
			{
				HDC hNewDC = GetDC(xid[radar].hwnd.hWnd);
				xid[radar].hwnd.hSaveDC = xid[radar].hwnd.hBmpDC;
				xid[radar].hwnd.hBmpDC = hNewDC;
				if (bHasPal)
				{
					SelectPalette(hNewDC, myPal, FALSE);
					RealizePalette(myPal);
				}
			}
		}
	}
}

static void WinXDeleteDraw(int xidno)
{
	HDC hSaveDC = xid[xidno].hwnd.hSaveDC;
	HDC hBmpDC = xid[xidno].hwnd.hBmpDC;

	if (xid[xidno].hwnd.type == DT_2)
	{
		if (xid[xidno].hwnd.hBmpa[0])
			DeleteObject(xid[xidno].hwnd.hBmpa[0]);
		if (xid[xidno].hwnd.hBmpa[1])
			DeleteObject(xid[xidno].hwnd.hBmpa[1]);
		xid[xidno].hwnd.hBmpa[0] = xid[xidno].hwnd.hBmpa[0] = NULL;
		if (xid[xidno].hwnd.hBmpDCa[0])
			DeleteDC(xid[xidno].hwnd.hBmpDCa[0]);
		if (xid[xidno].hwnd.hBmpDCa[1])
			DeleteDC(xid[xidno].hwnd.hBmpDCa[1]);
		xid[xidno].hwnd.hBmpDCa[0] = xid[xidno].hwnd.hBmpDCa[1] = NULL;

	}
	else
	{
		if (xid[xidno].hwnd.hBmp)
			DeleteObject(xid[xidno].hwnd.hBmp);
	}
	xid[xidno].hwnd.hBmp = NULL;
	if (hSaveDC)
	{
		DeleteDC(hSaveDC);
		if (hBmpDC)
			ReleaseDC(xid[xidno].hwnd.hWnd, hBmpDC);
	}
	else if (hBmpDC)
		DeleteDC(hBmpDC);

	xid[xidno].hwnd.hBmpDC = xid[xidno].hwnd.hSaveDC = NULL;
	xid[xidno].hwnd.hBmp = NULL;
}

#if 0
static void WinXScaled(HDC hDC, int cx, int cy)
{
	if (iScaleFactor != SCALEPREC)
	{
		SetMapMode(hDC, MM_ANISOTROPIC);
	
		SetWindowExtEx(hDC, WinXUnscale(cx), WinXUnscale(cy), NULL);
		SetWindowOrgEx(hDC, 0, 0, NULL);
		SetViewportExtEx(hDC, cx, cy, NULL);
		SetViewportOrgEx(hDC, 0, 0, NULL);
	}
	else SetMapMode(hDC, MM_TEXT);
}

static void WinXUnscaled(HDC hDC)
{
	SetMapMode(hDC, MM_TEXT);
}
#endif

static void WinXCreateBitmapForXid(HWND hwnd, XID xidno, int cx, int cy)
{
	HDC	hBmpDC, hDC = GetDC(hwnd);
	HBITMAP hBmp;
	RECT r;

	WinXDeleteDraw(xidno);
	if (ThreadedDraw && xidno == draw)
	{
		xid[xidno].hwnd.hBmpa[1] = CreateCompatibleBitmap(hDC, cx, cy);
		xid[xidno].hwnd.hBmpa[0] = CreateCompatibleBitmap(hDC, cx, cy);
		hBmp = xid[xidno].hwnd.hBmp= xid[xidno].hwnd.hBmpa[0];
		xid[xidno].hwnd.hBmpDCa[0] = CreateCompatibleDC(hDC);
		xid[xidno].hwnd.hBmpDCa[1] = CreateCompatibleDC(hDC);
		xid[xidno].hwnd.hBmpDC = xid[xidno].hwnd.hBmpDCa[0];
		xid[xidno].hwnd.filling = 0;
		xid[xidno].hwnd.drawtype = DT_2;
		SelectObject(xid[xidno].hwnd.hBmpDCa[0], xid[xidno].hwnd.hBmpa[0]);
		SelectObject(xid[xidno].hwnd.hBmpDCa[1], xid[xidno].hwnd.hBmpa[1]);
		SetBkMode(xid[xidno].hwnd.hBmpDCa[0], TRANSPARENT);
		SetBkMode(xid[xidno].hwnd.hBmpDCa[1], TRANSPARENT);
		SelectPalette(xid[xidno].hwnd.hBmpDCa[0], myPal, FALSE);
		RealizePalette(xid[xidno].hwnd.hBmpDCa[0]);
		SelectPalette(xid[xidno].hwnd.hBmpDCa[1], myPal, FALSE);
		RealizePalette(xid[xidno].hwnd.hBmpDCa[1]);
	}
	else
	{
		xid[xidno].hwnd.hBmp = hBmp = CreateCompatibleBitmap(hDC, cx, cy);
		xid[xidno].hwnd.hBmpDC = hBmpDC = CreateCompatibleDC(hDC);
		SelectObject(hBmpDC, hBmp);
	}
//	if (xidno == (int)draw)
//		WinXScaled(hBmpDC, cx, cy);


	if (bHasPal)
	{
		SelectPalette(hBmpDC, myPal, FALSE);
		RealizePalette(hBmpDC);
	}
	SetBkMode(hBmpDC, TRANSPARENT);
	r.left = 0; r.top = 0;
	r.right = cx; r.bottom = cy;
	FillRect(hBmpDC, &r, GetStockObject(BLACK_BRUSH));
	WinXSetupRadarWindow();
	ReleaseDC(hwnd, hDC);
}

BOOL ChangePalette(HWND hWnd)
{
	XID i;
	HDC hDC;
	HPALETTE hOldPal;
			
	for (i = 0; i < MAX_XIDS; i += 1)
	{
		if (xid[i].type == XIDTYPE_HWND)
		{
			HWND hwnd = xid[i].hwnd.hWnd;
			
			hDC = xid[i].hwnd.hBmpDC;
			hOldPal = SelectPalette(hDC, myPal, FALSE);
			RealizePalette(hDC);
			SelectPalette(hDC, hOldPal, FALSE);

			hDC = GetDC(hwnd);
			hOldPal = SelectPalette(hDC, myPal, FALSE);
			RealizePalette(hDC);
			SelectPalette(hDC, hOldPal, FALSE);
			ReleaseDC(hwnd, hDC);

			InvalidateRect(hwnd, NULL, FALSE);
		}
	}

	return TRUE;
}

LRESULT	CALLBACK	WinXwindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
	{
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
		int xidno = (int)lpcs->lpCreateParams;

		Trace("WM_CREATE %d %d/%d %s:%d\n", xidno, lpcs->cx, lpcs->cy, xid[xidno].any.file, xid[xidno].any.line);
		WinXCreateBitmapForXid(hwnd, xidno, lpcs->cx, lpcs->cy);
		SetWindowWord(hwnd, 0, (WORD)xidno);
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	case WM_DESTROY:
	{
		XID xidno = (int)GetWindowWord(hwnd, 0);

		if (Widget_window(motd_viewer) == xidno)
			Motd_destroy();
		if (Widget_window(keys_viewer) == xidno)
			Keys_destroy();
		WinXDeleteDraw(xidno);
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	case WM_SIZE:
	{
		int xidno = (int)GetWindowWord(hwnd, 0);
		if (xidno > 0 && xidno < MAX_XIDS && xid[xidno].hwnd.hBmp)
		{
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);
			Trace("WM_SIZE   %d %d/%d %s:%d\n", xidno, width, height, xid[xidno].any.file, xid[xidno].any.line);
			WinXCreateBitmapForXid(hwnd, xidno, width, height);
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	case WM_LBUTTONDOWN:
	{
		int				xidno = (int)GetWindowWord(hwnd, 0);
		if (xid[xidno].hwnd.event_mask & ButtonPressMask)
		{
			XEvent	event;
			XButtonEvent*	button = (XButtonEvent*)&event;
			POINT			pt;

			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			MapWindowPoints(xid[xidno].hwnd.hWnd, xid[top].hwnd.hWnd,
							&pt, 1);
			button->type	= ButtonPress;
			button->window	= xidno;
			button->x		= LOWORD(lParam);
			button->y		= HIWORD(lParam);
			button->x_root	= pt.x;
			button->y_root	= pt.y;
			button->button	= Button1;
			xevent(event);
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	case WM_LBUTTONUP:
	{
		int xidno = (int)GetWindowWord(hwnd, 0);

		if (xid[xidno].hwnd.event_mask & ButtonReleaseMask)
		{
			XEvent	event;
			XButtonEvent*	button = (XButtonEvent*)&event;

			Trace("ButtonUp in %d %s:%d\n", xidno, xid[xidno].any.file, xid[xidno].any.line);
			button->type	= ButtonRelease;
			button->window	= xidno;
			button->x		= LOWORD(lParam);
			button->y		= HIWORD(lParam);
			button->button	= Button1;
			if (xevent(event) == -1)
			{
				WinXExit();
			}
			return(0);
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	case WM_MBUTTONDOWN:
	{
		int				xidno = (int)GetWindowWord(hwnd, 0);
		if (xid[xidno].hwnd.event_mask & ButtonPressMask)
		{
			XEvent	event;
			XButtonEvent*	button = (XButtonEvent*)&event;
			POINT			pt;

			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			MapWindowPoints(xid[xidno].hwnd.hWnd, xid[top].hwnd.hWnd,
							&pt, 1);
			button->type	= ButtonPress;
			button->window	= xidno;
			button->x		= LOWORD(lParam);
			button->y		= HIWORD(lParam);
			button->x_root	= pt.x;
			button->y_root	= pt.y;
			button->button	= Button2;
			xevent(event);
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	case WM_MBUTTONUP:
	{
		int xidno = (int)GetWindowWord(hwnd, 0);

		if (xid[xidno].hwnd.event_mask & ButtonReleaseMask)
		{
			XEvent	event;
			XButtonEvent*	button = (XButtonEvent*)&event;

			Trace("ButtonUp in %d %s:%d\n", xidno, xid[xidno].any.file, xid[xidno].any.line);
			button->type	= ButtonRelease;
			button->window	= xidno;
			button->x		= LOWORD(lParam);
			button->y		= HIWORD(lParam);
			button->button	= Button2;
			if (xevent(event) == -1)
			{
				WinXExit();
			}
			return(0);
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	case WM_RBUTTONDOWN:
	{
		int				xidno = (int)GetWindowWord(hwnd, 0);
		if (xid[xidno].hwnd.event_mask & ButtonPressMask)
		{
			XEvent	event;
			XButtonEvent*	button = (XButtonEvent*)&event;
			POINT			pt;

			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			MapWindowPoints(xid[xidno].hwnd.hWnd, xid[top].hwnd.hWnd,
							&pt, 1);
			button->type	= ButtonPress;
			button->window	= xidno;
			button->x		= LOWORD(lParam);
			button->y		= HIWORD(lParam);
			button->x_root	= pt.x;
			button->y_root	= pt.y;
			button->button	= Button3;
			xevent(event);
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	case WM_RBUTTONUP:
	{
		int xidno = (int)GetWindowWord(hwnd, 0);

		if (xid[xidno].hwnd.event_mask & ButtonReleaseMask)
		{
			XEvent	event;
			XButtonEvent*	button = (XButtonEvent*)&event;

			Trace("ButtonUp in %d %s:%d\n", xidno, xid[xidno].any.file, xid[xidno].any.line);
			button->type	= ButtonRelease;
			button->window	= xidno;
			button->x		= LOWORD(lParam);
			button->y		= HIWORD(lParam);
			button->button	= Button3;
			if (xevent(event) == -1)
			{
				WinXExit();
			}
			return(0);
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	case WM_MOUSEMOVE:
	{
		XID xidno = (int)GetWindowWord(hwnd, 0);
		XEvent	event;
		XID		i;
		XAnyEvent*	enter = (XAnyEvent*)&event;

		Trace("MouseMove in %d %d/%d %s:%d\n", xidno, 
			LOWORD(lParam), HIWORD(lParam), xid[xidno].any.file, xid[xidno].any.line);

		enter->type			= LeaveNotify;
		for (i=0; i<MAX_XIDS; i++)
		{
			if (i != xidno && xid[i].type == XIDTYPE_HWND && xid[i].hwnd.mouseover 
				&& xid[i].hwnd.event_mask & LeaveWindowMask)
			{
				Trace("LeaveNotify %d %s:%d\n", xidno, xid[xidno].any.file, xid[xidno].any.line);
				enter->window = i;
				xevent(event);
				xid[i].hwnd.mouseover = FALSE;
			}
		}
		if (xid[xidno].hwnd.event_mask & PointerMotionMask)
		{
			XMotionEvent* me = (XMotionEvent*)&event;
			me->type = MotionNotify;
			me->window = xidno;
			me->x = LOWORD(lParam);
			me->y = HIWORD(lParam);
//			if (me->x != draw_width/2 && me->y != draw_height/2)
			{
				xevent(event);
//				SetCursorPos(draw_width/2, draw_height/2);
			}
		//	return(0);
		}
		else if (!xid[xidno].hwnd.mouseover) /* PointerMotionMask is only on captured window */
		{									 /* so don't do the mouseover event */
			if (xid[xidno].hwnd.event_mask & EnterWindowMask)
			{
				Trace("EnterNotify %d %s:%d\n", xidno, xid[xidno].any.file, xid[xidno].any.line);
				enter->type			= EnterNotify;
				enter->window		= xidno;
				xevent(event);
			}
			xid[xidno].hwnd.mouseover = TRUE;
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	case WM_ERASEBKGND:
	{
		RECT	rect;
		if (GetClientRect(hwnd, &rect))
		{
			HDC hBmpDC;
			XID xidno = (int)GetWindowWord(hwnd, 0);
			hBmpDC = xid[xidno].hwnd.hBmpDC;
			if (hBmpDC)
			{
				Trace("WM_ERASEBKGND %d color=%d %d/%d %d/%d\n", xidno, xid[xidno].hwnd.bgcolor, rect.left, rect.top, rect.right, rect.bottom);
				FillRect(hBmpDC, &rect, objs[xid[xidno].hwnd.bgcolor].brush);
			}
		}
		return(0);
	}
	case WM_PAINT:
	{
		RECT	rect;
		if (GetUpdateRect(hwnd, &rect, FALSE))
		{
			XID xidno = (int)GetWindowWord(hwnd, 0);
//			if (xidno == draw)
//				return DefWindowProc(hwnd, uMsg, wParam, lParam);
			if (xidno >= 0 && xidno < MAX_XIDS)
			{
				HDC hBmpDC;
				
				hBmpDC = xid[xidno].hwnd.hBmpDC;
				if (hBmpDC)
				{
					PAINTSTRUCT	ps;
					HDC	hDC;
					XEvent			event;
					XExposeEvent*	expose = (XExposeEvent*)&event;
					
					if (ThreadedDraw && xidno == (int)draw)
					{
						ValidateRect(hwnd, &rect);
						winXTDraw(NULL, xidno, &rect);
					}
					else
					{
						hDC = BeginPaint(hwnd, &ps);

						if (xid[xidno].hwnd.event_mask & ExposureMask)
						{
							expose->type	= Expose;
							expose->window	= xidno;
							expose->x		= rect.left;
							expose->y		= rect.top;
							expose->width	= rect.right-rect.left;
							expose->height	= rect.bottom-rect.top;
							expose->count	= 0;
							Trace("Expose %d %s:%d\n", xidno, xid[xidno].any.file, xid[xidno].any.line);
							xevent(event);
						}
						if (bHasPal)
						{
							SelectPalette(hDC, myPal, FALSE);
							RealizePalette(hDC);
						}
						if (xidno == (int)draw)
						{
						//	RECT r;
						//	WinXUnscaled(hBmpDC);
							if (ThreadedDraw)
							{
								ValidateRect(hwnd, &rect);
								winXTDraw(hDC, xidno, &rect);
							}
							else
							{
								BitBlt(hDC, rect.left, rect.top, rect.right, rect.bottom,
									   hBmpDC, rect.left, rect.top, SRCCOPY);
							}
						//	GetClientRect(hwnd, &r);
						//	WinXScaled(hBmpDC, r.right - r.left, r.bottom - r.top);
							drawPending = FALSE;
						}
						else
						{		/* not the main playfield window */
							BitBlt(hDC, rect.left, rect.top, rect.right, rect.bottom,
								   hBmpDC, rect.left, rect.top, SRCCOPY);
						}
						EndPaint(hwnd, &ps);
					}
					return 0;
				}
			}
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

COLORREF WinXPColour(int ColourNo)
{
	if (bHasPal)
		return PALETTEINDEX(ColourNo);
	return objs[ColourNo].color;
}

#if 0
LRESULT CALLBACK MouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	XEvent event;
	XMotionEvent* me = (XMotionEvent*)&event;
	if (nCode < 0)
		return(CallNextHookEx(mousehook, nCode, wParam, lParam));
	if (!pointerControl)
		return(0);
	me->type = MotionNotify;
	me->window = draw;
	me->x = ((MOUSEHOOKSTRUCT*)lParam)->pt.x;
	me->y = ((MOUSEHOOKSTRUCT*)lParam)->pt.y;
	xevent(event);

	return(1);

}
#endif

static void InitWinXClass()
{
	WNDCLASS wc;

	// Fill in window class structure with parameters
	wc.style = 0;
	wc.lpfnWndProc = WinXwindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(WORD);		// For the xidno of the window
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName =  NULL;
	wc.lpszClassName = "XPilotWin";

	RegisterClass(&wc);
}

void InitWinX(HWND hWnd)
{
	int		i;
	char	s[80];
	HDC		tDC = GetDC(hWnd);
//	TEXTMETRIC tm;

	memset(&xid, 0, sizeof(xid));
	InitWinXClass();
	xid[0].hwnd.hWnd = hWnd;
	xid[0].type = XIDTYPE_HWND;
//	max_xid = 1;

	itemsDC = NULL;

	bWinNT = GetVersion() & 0x80000000 ? FALSE : TRUE;

	myLogPal = malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY)*16);
	myLogPal->palVersion = 0x300;
//    "#000000", "#FFFFFF", "#4E7CFF", "#FF3A27",
//    "#33BB44", "#992200", "#BB7700", "#EE9900",
//    "#770000", "#CC4400", "#DD8800", "#FFBB11",
//    "#9f9f9f", "#5f5f5f", "#dfdfdf", "#202020"
	objs[0].color = GetXPilotColor ( 0, RGB(0x00,0x00,0x00));
	objs[1].color = GetXPilotColor ( 1, RGB(0xFF,0xFF,0xFF));
	objs[2].color = GetXPilotColor ( 2, RGB(0x4E,0x7C,0xFF));
	objs[3].color = GetXPilotColor ( 3, RGB(0xFF,0x3A,0x27));
	objs[4].color = GetXPilotColor ( 4, RGB(0x33,0xBB,0x44));
	objs[5].color = GetXPilotColor ( 5, RGB(0x99,0x22,0x00));
	objs[6].color = GetXPilotColor ( 6, RGB(0xBB,0x77,0x00));
	objs[7].color = GetXPilotColor ( 7, RGB(0xEE,0x99,0x00));
	objs[8].color = GetXPilotColor ( 8, RGB(0x77,0x00,0x00));
	objs[9].color = GetXPilotColor ( 9, RGB(0xCC,0x44,0x00));
	objs[10].color = GetXPilotColor(10, RGB(0xDD,0x88,0x00));
	objs[11].color = GetXPilotColor(11, RGB(0xFF,0xBB,0x11));
	objs[12].color = GetXPilotColor(12, RGB(0x9F,0x9F,0x9F));
	objs[13].color = GetXPilotColor(13, RGB(0x5F,0x5F,0x5F));
	objs[14].color = GetXPilotColor(14, RGB(0xDF,0xDF,0xDF));
	objs[15].color = GetXPilotColor(15, RGB(0x20,0x20,0x20));

	winmaxcolors = GetMaxColors();
	//bHasPal = (GetDeviceCaps(tDC, RASTERCAPS) & RC_PALETTE) != 0;
	if (!winmaxcolors)
		winmaxcolors = 8;
	if (winmaxcolors < 4)
		error("I can't allocate 4 (colors or pens or brushes).  XPilot is probably not going to look very nice on this display");
	else if (winmaxcolors < 8)
		winmaxcolors = 4;
	else if (winmaxcolors < 16)
		winmaxcolors = 8;
	else
		winmaxcolors = 16;
	maxColors = winmaxcolors;
	myLogPal->palNumEntries = winmaxcolors;
	for (i=0; i<winmaxcolors; i++)
	{
		myLogPal->palPalEntry[i].peFlags = PC_RESERVED;
//		myLogPal->palPalEntry[i].peFlags = PC_EXPLICIT;
		myLogPal->palPalEntry[i].peRed =   GetRValue(objs[i].color);
		myLogPal->palPalEntry[i].peGreen = GetGValue(objs[i].color);
		myLogPal->palPalEntry[i].peBlue =  GetBValue(objs[i].color);

#ifdef PENS_OF_PLENTY
		objs[i].brush = CreateSolidBrush(WinXPColour(i));
		if (!objs[i].brush)
			{ sprintf(s, "maxcolors=%d can't create brush %d", winmaxcolors, i); error(s); }
#else
		objs[i].pen = CreatePen(PS_SOLID, 1, WinXPColour(i));
		if (!objs[i].pen)
			{ sprintf(s, "maxcolors=%d can't create pen %d", winmaxcolors, i); error(s); }
		objs[i].brush = CreateSolidBrush(WinXPColour(i));
		if (!objs[i].brush)
			{ sprintf(s, "maxcolors=%d can't create brush %d", winmaxcolors, i); error(s); }
		Trace("%d: pen=%08X brush=%08X\n", i, objs[i].pen, objs[i].brush);
#endif
	}
	
#ifndef PENS_OF_PLENTY
	objs[WHITE+CLOAKCOLOROFS].color = objs[WHITE].color;
	objs[WHITE+CLOAKCOLOROFS].brush = objs[WHITE].brush;
	objs[WHITE+CLOAKCOLOROFS].pen = CreatePen(PS_DOT, 1, WinXPColour(WHITE));
	if (!objs[WHITE+CLOAKCOLOROFS].pen)
		{ sprintf(s, "maxcolors=%d can't create pen %d", winmaxcolors, WHITE+CLOAKCOLOROFS); error(s); }
	objs[BLUE+CLOAKCOLOROFS].color = objs[BLUE].color;
	objs[BLUE+CLOAKCOLOROFS].brush = objs[BLUE].brush;
	objs[BLUE+CLOAKCOLOROFS].pen = CreatePen(PS_DOT, 1, WinXPColour(BLUE));
	if (!objs[BLUE+CLOAKCOLOROFS].pen)
		{ sprintf(s, "maxcolors=%d can't create pen %d", winmaxcolors, BLUE+CLOAKCOLOROFS); error(s); }
	objs[LASERCOLOR].color = objs[RED].color;
	objs[LASERCOLOR].brush = objs[RED].brush;
	objs[LASERCOLOR].pen = CreatePen(PS_SOLID, 2, WinXPColour(RED));
	if (!objs[LASERCOLOR].pen)
		{ sprintf(s, "maxcolors=%d can't create pen %d", winmaxcolors, LASERCOLOR); error(s); }
	objs[MISSILECOLOR].color = objs[RED].color;
	objs[MISSILECOLOR].brush = objs[RED].brush;
	objs[MISSILECOLOR].pen = CreatePen(PS_SOLID, 2, WinXPColour(WHITE));
	if (!objs[MISSILECOLOR].pen)
		{ sprintf(s, "maxcolors=%d can't create pen %d", winmaxcolors, MISSILECOLOR); error(s); }
	objs[LASERTEAMCOLOR].color = objs[BLUE].color;
	objs[LASERTEAMCOLOR].brush = objs[BLUE].brush;
	objs[LASERTEAMCOLOR].pen = CreatePen(PS_SOLID, 2, WinXPColour(BLUE));
	if (!objs[LASERTEAMCOLOR].pen)
		{ sprintf(s, "maxcolors=%d can't create pen %d", winmaxcolors, LASERTEAMCOLOR); error(s); }
#endif

	myPal = CreatePalette(myLogPal);
	SelectPalette(tDC, myPal, FALSE);
	if (!myPal)
		error("Can't create palette");

    for (i=0; i<NUM_ITEMS; i++)
	{
		itemBitmaps[i][ITEM_HUD] = XIDTYPE_UNUSED;
		itemBitmaps[i][ITEM_PLAYFIELD] = XIDTYPE_UNUSED;
	}
//	mousehook = SetWindowsHookEx(WH_MOUSE, MouseHook, NULL, GetCurrentThreadId());
}
void WinXFree(XID i)
{
	switch (xid[i].type)
	{
	case XIDTYPE_FONT:
		free(xid[i].font.font);
		break;
	case XIDTYPE_PIXMAP:
		DeleteObject(xid[i].hpix.hbm);
		DeleteDC(xid[i].hpix.hDC);
		break;
	case XIDTYPE_HWND:
		WinXDeleteDraw(i);
		break;
	}
	xid[i].type = XIDTYPE_UNUSED;
}

void WinXShutdown()
{
	XID		i;

	if (mousehook)
		UnhookWindowsHookEx(mousehook);
	free(myLogPal);
	for (i=0; i<MAX_XIDS; i++)
	{
		WinXFree(i);
	}

	for (i=0; i<(unsigned)maxColors; i++)
	{
#ifdef PENS_OF_PLENTY
		if (objs[i].pen != NULL)
			DeleteObject(objs[i].pen);
		if (objs[i].dashpen != NULL)
			DeleteObject(objs[i].dashpen);
		if (objs[i].cdashpen != NULL)
			DeleteObject(objs[i].cdashpen);
		if (objs[i].fatpen != NULL)
			DeleteObject(objs[i].fatpen);
		if (objs[i].brush != NULL)
			DeleteObject(objs[i].brush);
#else
		if (objs[i].pen)
			DeleteObject(objs[i].pen);
		if (objs[i].brush)
			DeleteObject(objs[i].brush);

#endif
	}
}

#ifdef PENS_OF_PLENTY
static HPEN WinXMakePen(int cur_color, int Style, int Width)
{
	HPEN	hPen;

	if (bWinNT && Style != PS_SOLID)
	{
		LOGBRUSH lb;
		DWORD	*dshs;
	
		lb.lbStyle = BS_SOLID;
		lb.lbColor = WinXPColour(cur_color);
		lb.lbHatch = 0;

		if (Style = PS_DASH)
			dshs = dwdashes;
		else
			dshs = dwcdashes;
		hPen = ExtCreatePen(PS_GEOMETRIC | PS_USERSTYLE, Width, &lb, dshs[0], &dshs[1]);
	}
	else hPen = CreatePen(Style, Width, WinXPColour(cur_color));

	TotalPens += 1;

	return hPen;
}
				
static void WinXSetPen(int xidno)
{
	HDC		hDC = xid[xidno].hwnd.hBmpDC;
	HPEN	hPen = NULL;
	int		cur_color = xid[xidno].hwnd.cur_color;
	
	if (xid[xidno].hwnd.line_style == LineSolid)
	{
		if (xid[xidno].hwnd.line_width > 2)
		{
			if (objs[cur_color].fatpen == NULL)
				objs[cur_color].fatpen = WinXMakePen(WinXPColour(cur_color), PS_SOLID, 3);
			hPen = objs[cur_color].fatpen;
		}
		else
		{
			if (objs[cur_color].pen == NULL)
				objs[cur_color].pen = WinXMakePen(WinXPColour(cur_color), PS_SOLID, 0);
			hPen = objs[cur_color].pen;
		}
	}
	else
	{
		if (xid[xidno].hwnd.nodash)
		{
			if (objs[cur_color].cdashpen == NULL)
				objs[cur_color].cdashpen = WinXMakePen(cur_color, PS_DOT, 0);
			hPen = objs[cur_color].cdashpen;	
		}
		else
		{
			if (objs[cur_color].dashpen == NULL)
				objs[cur_color].dashpen = WinXMakePen(cur_color, PS_DASH, 0);
			hPen = objs[cur_color].dashpen;
		}
	}
	if (hPen)
		SelectObject(hDC, hPen);
}
#endif

////////////////////////////////////////////////////////
// These are for SysInfo, we hide the details from him
HDC	WinXGetDrawDC()
{
	return(GetDC(xid[draw].hwnd.hWnd));
}

int	WinXReleaseDrawDC(HDC hDC)
{
	return(ReleaseDC(xid[draw].hwnd.hWnd, hDC));
}

////////////////////////////////////////////////////////
int	WinXGetWindowRectangle(Window window, XRectangle* rect)
{
	RECT	r;
	GetClientRect(xid[window].hwnd.hWnd, &r);
	rect->x = (short)WinXUnscale(r.left);
	rect->y = (short)WinXUnscale(r.top);
	rect->width = (unsigned short)WinXUnscale(r.right - r.left);
	rect->height = (unsigned short)WinXUnscale(r.bottom - r.top);
	return(1);
}

#if 0
void WinXSetBackColor(GC gc, unsigned long background)
{
	HDC hDC = xid[xid[gc].hgc.xidhwnd].hwnd.hBmpDC;
	SetBkColor(hDC, background);
	SetBkMode(hDC, TRANSPARENT);
}
#endif

// scarfed from a M$ KB article.  Apparently, Win95 doesn't support AngleArc
BOOL AngleArc2(HDC hdc, int X, int Y, DWORD dwRadius,
               double fStartDegrees, double fSweepDegrees, BOOL bFilled)
{
  int iXStart, iYStart;  // End point of starting radial line
  int iXEnd, iYEnd;        // End point of ending radial line
  double fStartRadians;   // Start angle in radians
  double fEndRadians;     // End angle in radians
  BOOL bResult;            // Function result
 
  /* Get the starting and ending angle in radians */
  if (fSweepDegrees > 0.0) {
    fStartRadians = ((fStartDegrees / 360.0) * fTwoPi);
    fEndRadians = (((fStartDegrees + fSweepDegrees) / 360.0) * fTwoPi);
  } else {
    fStartRadians = (((fStartDegrees + fSweepDegrees)  / 360.0) * fTwoPi);
    fEndRadians =  ((fStartDegrees / 360.0) * fTwoPi);
  }
 
  /* Calculate a point on the starting radial line via */
  /* polar -> cartesian conversion */
  iXStart = X + (int)((double)dwRadius * (double)cos(fStartRadians));
  iYStart = Y - (int)((double)dwRadius * (double)sin(fStartRadians));
 
  /* Calculate a point on the ending radial line via */
  /* polar -> cartesian conversion */
  iXEnd = X + (int)((double)dwRadius * (double)cos(fEndRadians));
  iYEnd = Y - (int)((double)dwRadius * (double)sin(fEndRadians));
 
  /* Draw a line to the starting point */
  LineTo(hdc, iXStart, iYStart);
 
  /* Draw the arc */
  if (bFilled)
    bResult = Pie(hdc, X - dwRadius, Y - dwRadius, 
	                   X + dwRadius, Y + dwRadius,
                       iXStart, iYStart, iXEnd, iYEnd);
  else
    bResult = Arc(hdc, X - dwRadius, Y - dwRadius,
                       X + dwRadius, Y + dwRadius,
                       iXStart, iYStart, iXEnd, iYEnd);
 
  /* Move to the ending point - Arc() wont do this and ArcTo() */
  /* wont work on Win32s or Win16 */
  MoveToEx(hdc, iXEnd, iYEnd, NULL);

  return bResult;
}
 
void WinXParseFont(LOGFONT* lf, const char* name)
{
#define	MAX_FFLDS	14
	static char sepa[] = "-\n\r";
	char*	t[MAX_FFLDS];
	char*	s = malloc(strlen(name)+1);
	int		i;

	strcpy(s, name);

	t[0] = strtok(s, sepa);
	for (i=1; i<MAX_FFLDS; i++)
		t[i] = strtok(NULL, sepa);
//	lf->lfHeight = atoi(t[6]) * 100 / iScaleFactor;
//	lf->lfHeight = (int)(atoi(t[6]) / scaleFactor);
	lf->lfHeight = (int)(atoi(t[6]));
	if (!lf->lfHeight)
		lf->lfHeight = 14;
	lf->lfWeight = *t[2] == 'b' ? FW_BOLD : FW_NORMAL;
	lf->lfItalic = *t[3] == 'i' ? TRUE : FALSE;
	switch (*t[1])
	{
	case 't':		/* times */
		lf->lfPitchAndFamily = FF_ROMAN;
		break;
	case 'f':		/* fixed */
		lf->lfPitchAndFamily = FIXED_PITCH;
		break;
	case 'c':		/* courier */
		lf->lfPitchAndFamily = FF_MODERN;
		break;
	}
	free(s);
#undef	MAX_FFLDS
}

XFontStruct* WinXLoadFont(const char* name)
{
	XID			txid;
	XFontStruct* fs = malloc(sizeof(XFontStruct));
	Trace("WinXLoadFont: creating font <%s>\n", name);
	memset(fs, 0, sizeof(XFontStruct));
	WinXParseFont(&fs->lf, name);
//	fs->ascent = fs->lf.lfHeight * 100 / iScaleFactor;
//	fs->ascent = (int)(fs->lf.lfHeight / scaleFactor);
	fs->ascent = (int)(fs->lf.lfHeight);
	fs->hFont = CreateFontIndirect(&fs->lf);

	txid = GetFreeXid();
	xid[txid].type = XIDTYPE_FONT;
	xid[txid].font.font = fs;
	fs->fid = txid;
	return(fs);
}


XParseColor(Display* display, Colormap colormap, char* spec, 
					XColor* exact_def_return)
{
	Trace("Parsing color <%s>\n", spec);
	return(0);
}

Pixmap WinXCreateBitmapFromData(Display* dpy, Drawable d, char* data, 
							  unsigned int width, unsigned int height, int color)
{
	HBITMAP	hbm;
	int		i;
	int		j;
	WORD*	e;

	BITMAP bm = {
		0,							//   LONG   bmType; 
		16,							//   LONG   bmWidth; 
		16,							//   LONG   bmHeight; 
		4,							//   LONG   bmWidthBytes; 
		1,							//   WORD   bmPlanes; 
		1,							//   WORD   bmBitsPixel; 
		NULL						//   LPVOID bmBits; 
	};
	RECT rect = { 0,0,16,16};

	HDC		hDC = GetDC(xid[d].hwnd.hWnd);
	HDC		hDCb = CreateCompatibleDC(hDC);
	
	hbm = CreateCompatibleBitmap(hDC, width, height);
	SelectObject(hDCb, hbm);
	if (bHasPal)
	{
		SelectPalette(hDCb, myPal, FALSE);
		RealizePalette(hDCb);
	}
	
	FillRect(hDCb, &rect, GetStockObject(BLACK_BRUSH));
	if (!hbm)
		error("Can't create item bitmaps");
	if (width != 16 || height != 16)
		error("Can only create 16x16 bitmaps");
	e = (WORD*)data;
	for (i=0; i<16; i++)
	{
		WORD	w = *e++;
		WORD	z = 0;
		for (j=0; j<16; j++)					// swap the bits in the bytes
			if (w & (1<<j))
				SetPixelV(hDCb, j, i, WinXPColour(color));
	}

	DeleteDC(hDCb);
	ReleaseDC(xid[d].hwnd.hWnd, hDC);
	return((Pixmap)hbm);
}

XResizeWindow(Display* dpy, Window w, unsigned int width, unsigned int height)
{
	HWND	hWnd = xid[w].hwnd.hWnd;
	
	SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
	return(0);
}

void WinXResize(void)
{
	RECT rect;

	draw_width  = WinXUnscale(draw_width);
	draw_height = WinXUnscale(draw_height);

	if (radar && (instruments & SHOW_SLIDING_RADAR))
	{
		GetClientRect(xid[radar].hwnd.hWnd, &rect);
		InvalidateRect(xid[radar].hwnd.hWnd, &rect, FALSE);
	}
	if (draw)
	{
		GetClientRect(xid[draw].hwnd.hWnd, &rect);
		InvalidateRect(xid[draw].hwnd.hWnd, &rect, FALSE);
	}
	if (players)
	{
		GetClientRect(xid[players].hwnd.hWnd, &rect);
		InvalidateRect(xid[players].hwnd.hWnd, &rect, FALSE);
	}
}

void	PaintWinClient()
{
#if 0
	RECT	rect;
	static int updates = 0;
	
	if (drawPending)
		return;				// bogus dude.

	if (!itemsDC)
		itemsDC = CreateCompatibleDC(NULL);

	WinXSetupRadarWindow();

//	Paint_frame();

	GetClientRect(xid[draw].hwnd.hWnd, &rect);
	InvalidateRect(xid[draw].hwnd.hWnd, &rect, FALSE);
	drawPending = TRUE;

	if (instruments & SHOW_SLIDING_RADAR)
	{
		GetClientRect(xid[radar].hwnd.hWnd, &rect);
		InvalidateRect(xid[radar].hwnd.hWnd, &rect, FALSE);
	}

	// One time stuff for score window update
	if (updates == 0)
	{
		GetClientRect(xid[players].hwnd.hWnd, &rect);
		InvalidateRect(xid[players].hwnd.hWnd, &rect, FALSE);
		UpdateWindow(xid[players].hwnd.hWnd);
	}
	updates += 1;
#else
	RECT	rect;
	static int updates = 0;
	if (!itemsDC)
		itemsDC = CreateCompatibleDC(NULL);

	WinXSetupRadarWindow();

	// One time stuff for score window update
	if (updates == 0)
	{
		GetClientRect(xid[players].hwnd.hWnd, &rect);
		InvalidateRect(xid[players].hwnd.hWnd, &rect, FALSE);
		UpdateWindow(xid[players].hwnd.hWnd);
	}
	updates += 1;
//	SelectPalette(hDC, myPal, FALSE);
//	RealizePalette(hDC);

//	xid[draw].hwnd.hDC = hDC;
//	if (!itemsDC)
//		itemsDC = CreateCompatibleDC(realDC);

//	Paint_frame();
//	xid[draw].hwnd.hBmpDC = realDC;
	GetClientRect(xid[draw].hwnd.hWnd, &rect);
	if (ThreadedDraw)
	{
//		FillRect(xid[draw].hwnd.hBmpDC, &rect, GetStockObject(WHITE_BRUSH));
		winXTDraw(NULL, draw, &rect);
	}
	else
	{
		HDC		realDC = GetDC(xid[draw].hwnd.hWnd);
		SelectPalette(realDC, myPal, FALSE);
		RealizePalette(realDC);
		BitBlt(realDC, 0, 0, rect.right, rect.bottom, xid[draw].hwnd.hBmpDC,
			0, 0, SRCCOPY);
		ReleaseDC(xid[draw].hwnd.hWnd, realDC);
	}
#endif
}

void MarkPlayersForRedraw()
{
	RECT	rect;

	GetClientRect(xid[players].hwnd.hWnd, &rect);
	InvalidateRect(xid[players].hwnd.hWnd, &rect, FALSE);
	UpdateWindow(xid[players].hwnd.hWnd);	
}

void paintItemSymbol(unsigned char type, Drawable d, GC gc, int x, int y, int color)
{
	HDC		hDC = xid[d].hwnd.hBmpDC;

	SelectObject(itemsDC, (HBITMAP)itemBitmaps[type][color]);
	if (bHasPal)
	{
		SelectPalette(itemsDC, myPal, FALSE);
		RealizePalette(itemsDC);
	}
	BitBlt(hDC, x, y, 16, 16, itemsDC, 0, 0, SRCPAINT);
}

void WinXBltPixToWin(Pixmap src, Window dest,
				  int src_x, int src_y, unsigned int width, unsigned int height,
				  int dest_x, int dest_y)
{
	HDC		shDC = xid[src].hpix.hDC;
	HDC		hDC = xid[dest].hwnd.hBmpDC;

	BitBlt(hDC, dest_x, dest_y, width, height, shDC, src_x, src_y, SRCCOPY);
}

void WinXBltWinToPix(Window src, Pixmap dest,
				  int src_x, int src_y, unsigned int width, unsigned int height,
				  int dest_x, int dest_y)
{
	HDC		hDC = xid[src].hwnd.hBmpDC;
	HBITMAP hbm = xid[dest].hpix.hbm;

	HDC		hDCd = CreateCompatibleDC(NULL);
	int		ret;
	SelectObject(hDC, hbm);
	if (bHasPal)
	{
		SelectPalette(hDC, myPal, FALSE);
		RealizePalette(hDC);
	}
	ret = BitBlt(hDC, dest_x, dest_y, width, height, hDC, src_x, src_y, BLACKNESS);
	DeleteDC(hDCd);

}

#if 0
void WinXPaintPlayers()
{
}
#endif

void WinXFlush(Window w)
{
	RECT	r;
	GetClientRect(xid[w].hwnd.hWnd, &r);
	Trace("Flushing %d (%d/%d %d/%d)\n", w, r.left, r.top, r.right, r.bottom);
	InvalidateRect(xid[w].hwnd.hWnd, &r, TRUE);
}

void WinXExit()
{
	PostMessage(GetParent(xid[top].hwnd.hWnd), WM_CLOSE, 0, 0);
}

void WinXSetEventMask(Window w, long mask)
{
	xid[w].hwnd.event_mask = mask;			/* this could be a macro, */
}										/* but winX_.h is hidden from everyone */
Window WinXGetParent(Window w)
{
	XID	i;
	XID	txid;
	HWND	hwnd = GetParent(xid[w].hwnd.hWnd);
	if (!hwnd)
		return(top);
	for (i=0; i<MAX_XIDS; i++)
	{
		if (hwnd == xid[i].hwnd.hWnd)
			return(i);
	}
/* create a new "Window" for the parent */
	txid = GetFreeXid();
	xid[txid].hwnd.hBmp = NULL;
	xid[txid].hwnd.hBmpDC = NULL;
	xid[txid].hwnd.hWnd = hwnd;
	xid[txid].hwnd.event_mask = 0;
	xid[txid].hwnd.event_mask = -1;		// hell, let's take em all!
	xid[txid].hwnd.mouseover = 0;		// mouse not over this window
	xid[txid].hwnd.type = XIDTYPE_HWND;
	xid[txid].hwnd.notmine = TRUE;		// we don't destroy this one...
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

BOOL WinXGetWindowRect(Window w, RECT* rect)
{
	return(GetWindowRect(xid[w].hwnd.hWnd, rect));
}

BOOL WinXGetWindowPlacement(Window w, WINDOWPLACEMENT* wp)
{
	return(GetWindowPlacement(xid[w].hwnd.hWnd, wp));
}

XID	GetFreeXid()
{
	int i; 
	for (i=0; i<MAX_XIDS; i++) 
		if (xid[i].type == XIDTYPE_UNUSED) 
			return(i);
	error("No Free XIDs"); 
	return(MAX_XIDS);
}

/*------------------------------------------------------*\
* stubs for motd and talk window.  I wrote these Windows *
* dialogs differently... I should revisit these to use   *
* these new APIs.                                        *
\*------------------------------------------------------*/
bool talk_mapped = FALSE;
void Talk_resize(void)
{ }
int Talk_do_event(XEvent* event)
{ return(FALSE); }


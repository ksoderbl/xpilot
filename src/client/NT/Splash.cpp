/* $Id: Splash.cpp,v 5.1 2001/05/27 20:40:46 dik Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
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
*  Splash.cpp - The Splash Panel for XPilot.exe								*
*																			*
*  This file is the standard splash component from MSDEV enhanced to allow	*
*  displaying progress messages in the lower left corner.					*
*																			*
*  $Id: Splash.cpp,v 5.1 2001/05/27 20:40:46 dik Exp $							*
\***************************************************************************/
// CG: This file was added by the Splash Screen component.

#include "stdafx.h"  // e. g. stdafx.h
#include "resource.h"  // e.g. resource.h

#include "Splash.h"  // e.g. splash.h
#include "../../common/version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//   Splash Screen class

BOOL CSplashWnd::c_bShowSplashWnd;
CSplashWnd* CSplashWnd::c_pSplashWnd;
CSplashWnd::CSplashWnd()
{
}

CSplashWnd::~CSplashWnd()
{
	// Clear the static window pointer.
	ASSERT(c_pSplashWnd == this);
	c_pSplashWnd = NULL;
}

BEGIN_MESSAGE_MAP(CSplashWnd, CWnd)
	//{{AFX_MSG_MAP(CSplashWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSplashWnd::EnableSplashScreen(BOOL bEnable /*= TRUE*/)
{
	c_bShowSplashWnd = bEnable;
}

void CSplashWnd::ShowSplashScreen(CWnd* pParentWnd /*= NULL*/)
{
	if (!c_bShowSplashWnd || c_pSplashWnd != NULL)
		return;

	// Allocate a new splash screen, and create the window.
	c_pSplashWnd = new CSplashWnd;
	if (!c_pSplashWnd->Create(pParentWnd))
		delete c_pSplashWnd;
	else
		c_pSplashWnd->UpdateWindow();
}

BOOL CSplashWnd::PreTranslateAppMessage(MSG* pMsg)
{
	if (c_pSplashWnd == NULL)
		return FALSE;

	// If we get a keyboard or mouse message, hide the splash screen.
	if (pMsg->message == WM_KEYDOWN ||
	    pMsg->message == WM_SYSKEYDOWN ||
	    pMsg->message == WM_LBUTTONDOWN ||
	    pMsg->message == WM_RBUTTONDOWN ||
	    pMsg->message == WM_MBUTTONDOWN ||
	    pMsg->message == WM_NCLBUTTONDOWN ||
	    pMsg->message == WM_NCRBUTTONDOWN ||
	    pMsg->message == WM_NCMBUTTONDOWN)
	{
		c_pSplashWnd->HideSplashScreen();
		return TRUE;	// message handled here
	}

	return FALSE;	// message not handled
}

BOOL CSplashWnd::Create(CWnd* pParentWnd /*= NULL*/)
{
	if (!m_bitmap.LoadBitmap(IDB_SPLASH))
		return FALSE;

	BITMAP bm;
	m_bitmap.GetBitmap(&bm);

	return CreateEx(0,
		AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
		NULL, WS_POPUP | WS_VISIBLE, 0, 0, bm.bmWidth, bm.bmHeight, pParentWnd->GetSafeHwnd(), NULL);
}

void CSplashWnd::HideSplashScreen()
{
	// Destroy the window, and update the mainframe.
	DestroyWindow();
	AfxGetMainWnd()->UpdateWindow();
}

void CSplashWnd::PostNcDestroy()
{
	// Free the C++ class.
	delete this;
}

int CSplashWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Center the window.
	CenterWindow();

	// Set a timer to destroy the splash screen.
	SetTimer(1, 750, NULL);

	return 0;
}

void CSplashWnd::OnPaint()
{
	CPaintDC dc(this);

	CDC dcImage;
	if (!dcImage.CreateCompatibleDC(&dc))
		return;

	BITMAP bm;
	m_bitmap.GetBitmap(&bm);

	// Paint the image.
	CBitmap* pOldBitmap = dcImage.SelectObject(&m_bitmap);
	dc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcImage, 0, 0, SRCCOPY);
	dcImage.SelectObject(pOldBitmap);

	CFont	font;
	CFont*	ofont;
	COLORREF	ocolor;
	int			omode;
	font.CreateFont(-20, 0,
		0, 0,								// No rotation
		FW_SEMIBOLD,
		TRUE, FALSE, FALSE,				// No Italic, Underline or Strikeout
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		FF_ROMAN | FF_DONTCARE,
		"");
	ofont = dc.SelectObject(&font);
	ocolor = dc.SetTextColor(RGB(255,0,255));
	omode = dc.SetBkMode(TRANSPARENT);
	CString cs;
	cs.Format("Version %s", TITLE);
	CSize sz = dc.GetOutputTextExtent(cs);
	dc.TextOut(347-(sz.cx/2), 79, cs);
	dc.SetTextColor(RGB(255,255,0));
	dc.TextOut(346-(sz.cx/2), 78, cs);

	dc.SetTextColor(ocolor);
	dc.SelectObject(ofont);
	dc.SetBkMode(omode);

}

void CSplashWnd::OnTimer(UINT nIDEvent)
{
	// Destroy the splash screen window.
	HideSplashScreen();
}

void CSplashWnd::ShowMessage(const CString& cs)
{

	if (c_bShowSplashWnd && !c_pSplashWnd)
		ShowSplashScreen();

	CDC*	dc = c_pSplashWnd->GetDC();
	CRect	rect;
	c_pSplashWnd->GetClientRect(&rect);
	CFont	font;
	CFont*	ofont;
	COLORREF	ocolor;
	int			omode;
	//COLORREF	oback;

	font.CreateFont(-20, 0,
		0, 0,								// No rotation
		FW_NORMAL,
		FALSE, FALSE, FALSE,				// No Italic, Underline or Strikeout
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		FF_ROMAN | FF_DONTCARE,
		"");
	ofont = dc->SelectObject(&font);
	ocolor = dc->SetTextColor(RGB(255,255,255));
	//oback = dc->SetBkColor(RGB(0,0,0));

	omode = dc->SetBkMode(TRANSPARENT);
	rect.top = rect.bottom-30;
	rect.left += 10;
	rect.right -= 10;
	rect.bottom -= 8;
#ifdef	_DEBUG
	dc->FillSolidRect(rect, RGB(255,0,0));
#else
	dc->FillSolidRect(rect, RGB(0,0,0));
#endif
	dc->TextOut(10, rect.bottom-24, cs);

	dc->SetBkMode(omode);
	//dc->SetBkColor(oback);
	dc->SetTextColor(ocolor);
	dc->SelectObject(ofont);
	c_pSplashWnd->ReleaseDC(dc);

#ifdef	_DEBUG
//	Sleep(1000);
#endif
}

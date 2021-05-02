/* $Id: winAbout.cpp,v 5.0 2001/04/07 20:00:59 dik Exp $
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
*  winAbout.cpp - XPilot.exe credits box									*
*																			*
*  This file contains the Windows about dialog and scrolling credits box.	*
*  $Id: winAbout.cpp,v 5.0 2001/04/07 20:00:59 dik Exp $						*
\***************************************************************************/

#include "stdafx.h"
#include "winAbout.h"
#include "../../common/version.h"
/////////////////////////////////////////////////////////////////////////////

#define	CR_WIDTH	crRect.Width()
#define	CR_HEIGHT	2500
CString credits;
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT

	SendDlgItemMessage(IDC_VERSION, WM_SETTEXT, 0, (LPARAM)("XPilot " TITLE));
credits = "\
#r001This is a very abbreviated credit list.  We couldn't possibly enumerate all\
#r001the people who have been involved in XPilot, especially since we have no such\
#r001list at hand. :)  If you find someone (yourself!?) missing from this list\
#r001then please don't hesitate to complain to the authors at xpilot@cs.uit.no.";
credits += "\
#r002XPilot for M$ credits:\
#r001#c005Dick Balaska#c025Portmeister.\
#r001#c005Chris Caserio#c025Goaded me into doing this, XPmonNT fixes.\
#r001#c005Martin Sandiford#c025Windows speedups.  Faithful Sidekick.\
#r001#c005E. Hugh Woods III#c025XPilot M$ Installation script.";

credits += "\
#r002Release 3.6 credits:\
#r001#c005Tim Auckland#c025Gravity currents\
#r001#c005Mark Boyns#c025Several patches and comments\
#r001#c005Scott Manley\
#r001#c005Ben Jackson\
#r001#c005Andrew Mobbs#c025itemConcentratorProb\
#r001#c005Ian Jackson#c025itemConcentratorProb";

credits += "\
#r002Release 3.5 credits:\
#r001#c005Steve Payne#c025ship checking, Linux plock + option\
#r001#c005Claus Leth Gregersen#c025item limits, hud ship, textured balls\
#r001#c005Aleksi Suhonen#c025X multibuffer\
#r001#c005Jens Koerber#c025VMS changes";
credits += "\
#r002Release 3.4 credits:\
\
#r001#c005Mark Boyns#c025Map decorations + other things.\
#r001#c005Michael Gruber#c025Left & right rear guns.\
#r001#c005Jens Koerber#c025Changes for VMS.\
#r001#c005Bretton Wade#c025Ball movement research + code.\
#r001#c005Christian Montanari#c025Changes to HP audio.\
#r001#c005Tom Hansen#c025Drop items.";
credits += "\
#r002Release 3.3 credits:\
\
#r001#c005Steven Singer#c025Record & playback, filled walls.\
#r001#c005Christian Engstler#c025Bug reports & Sun guinea pig.";
credits += "\
#r002Release 3.2 credits:\
\
#r001#c005Harvey Thompson#c025Bugfixes and new code.\
#r001#c005Scott W. Adkins#c025Emergency Shield.\
#r001#c005John E. Norlin#c025Robots ship shapes.\
#r001#c005Patrick Kenny#c025GOD log, bugfix, Blood's Music.\
#r001#c005Pete Ford#c025New ship shape format.\
#r001#c005Jyke Jokinen#c025xpilot host alias.\
#r001#c005Russ Glaeser#c025Better scoring system.\
#r001#c005Steven Singer#c025Mouse control, race mode.\
#r001#c005Nicholas Vargish#c025rogues options & persistence ;-)\
#r001#c005Joshua Yelon#c025Better explosion system, shove patch.\
#r001#c005Eckard Kopatzki#c025Joystick support for Linux.\
#r001#c005Tom De Pauw & Lance Berc#c025AF audio support.\
#r001#c005Ben Jackson#c025Bug fixes.\
#r001#c005Steve Marsden#c025User controllable mine detonation.";
credits += "\
#r002Release 3.1 credits:\
\
#r001#c005Harvey Thompson#c025Lots of first class wonderful new\
#r001#c025features, ideas and cleanups.  Thanks!\
#r001#c005Mark Boyns#c025Sound.\
#r001#c005Stig Sæther Bakken#c025Imakefile, testing & feedback.\
#r001#c005Scott W. Adkins#c025Ship shapes.\
#r001#c005Tony Plate#c025Missile length and names.\
#r001#c005Steven Singer#c025Sun tester & knowledge provider. :-)";
credits += "\
#r002Release 3.0 credits:\
\
#r001#c005Maurice Abraham#c025Robots, more flexible bouncing etc.\
#r001#c005Mark Boyns, David J. Bianco#c025RPlay - original sound support.\
#r001#c005Greg Renda#c025Sound scheme client/serverized\
#r001#c025and added NCD sound support.\
#r001#c005Dave Lemke\
#r001#c005Curt Hjorring#c025VMS port.\
#r001#c005Andrew Scherpbier#c025Random sound patch.  Xpmtp.\
#r001#c005Steven Singer#c025Sun tester & knowledge provider. :-)\
#r001#c005Andy Skinner\
#r001#c005Tero Kivinen#c025Outline world.\
#r001#c005David Chatterton#c025Ultrix and SGI.\
#r001#c005Johan Danielsson\
#r001#c005Fred Hucht & Michael Staats#c025Several (big) patches\
#r001#c005Maurice Abraham\
#r001#c005Makoto Matsushita\
#r001#c005Stig Sæther Bakken#c025patches & feedback.\
#r001#c005& all the beta testers";
credits += "\
#r002Version 2.0 credits:#c025(Never released)\
\
#r001#c005Bert Gijsbers#c025Too much to mention really.\
#r001#c005Robert Blum + more from Aachen#c025Lots of features.\
#r001#c005Andy Skinner#c025Target code and maps.\
#r001#c005Paul Gardner#c025Random map generator + optimisations.\
#r001#c005Greg Renda\
#r001#c005Randall Severly\
#r001#c005Evan Miller\
#r001#c005Eric Flink\
#r001#c005Karthik P. Sheka\
#r001#c005Stig Sæther Bakken\
#r001#c005Jim Hickey\
#r001#c005Harvey Thompson\
#r001#c005Ted Lemon";
credits += "\
#r002Version 1.2 credits:\
\
#r001#c005Christos Zoulas#c025Monochrome fixes\
#r001#c005Abraham Maurice#c025Robots, timing.\
#r001#c005Greg Renda#c025Rebinding of keys, wormholes etc.\
#r001#c005Dave Lemke\
#r001#c005Keith Cantrell\
#r001#c005Chris Marshall\
#r001#c005Sigmund Austigard\
#r001#c005Martin Rutishauser\
#r001#c005Frank Tore Johansen\
#r001#c005Hermann Dunkel";
credits += "\
#r002Pre version 1.2 credits:\
\
#r001#c005Arne Helme#c025Socklib\
#r001#c005Tom Grydeland#c025Maps, fonts and ideas\
\
#r002#c005Lots of, LOTS OF beta testers at our site";

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_CREDITS, m_credits);
	//}}AFX_DATA_MAP
	SendDlgItemMessage(IDC_VERSION, WM_SETTEXT, 0, (LPARAM)("XPilot " TITLE));
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCredits

CCredits::CCredits()
{
	scrollofs = 0;
//	bm.LoadBitmap("IDB_CREDITS");
//	bm.LoadBitmap(IDB_CREDITS);
	timer = FALSE;
	virgin = TRUE;
}

CCredits::~CCredits()
{
#if 0
	CString e;
	e.Format("scrollofs = %d", scrollofs);
	AfxMessageBox(e);
#endif
}


BEGIN_MESSAGE_MAP(CCredits, CStatic)
	//{{AFX_MSG_MAP(CCredits)
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCredits message handlers
void CCredits::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (scrollofs++ > 2360)
		scrollofs = 0;
	CRect	rect;
	GetClientRect(&rect);
	InvalidateRect(&rect, FALSE);
	CStatic::OnTimer(nIDEvent);
}

void CCredits::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CString	out;
//	int		i;
	int		line = 0;

	// TODO: Add your message handler code here
	CFont*	oldFont = dc.SelectObject(&font);
	GetClientRect(&crRect);

	if (virgin)
	{
		// create the credits bitmap
		CDC	bdc;
		CDC* wdc;

		virgin = FALSE;
		wdc = GetDC();
		bdc.CreateCompatibleDC(wdc);
		bm.CreateCompatibleBitmap(&bdc, CR_WIDTH, CR_HEIGHT);
		bdc.SelectObject(&bm);
		BuildBitmap(&bdc);
		ReleaseDC(wdc);
	}
	if (!timer)
		SetTimer(32, 30, NULL);
	timer = TRUE;

	CDC		bdc;
	bdc.CreateCompatibleDC(&dc);
	CBitmap* obm = bdc.SelectObject(&bm);
	dc.BitBlt(0, 0, crRect.Width(), crRect.Height(), &bdc, 0, scrollofs, SRCCOPY);
	// dc.SelectObject(obm);
	// Do not call CStatic::OnPaint() for painting messages
}


BOOL CCredits::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
	BOOL ret;
	ret = CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
	return(ret);
}

BOOL CCredits::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	KillTimer(32);
	bm.DeleteObject();

	
	return CStatic::DestroyWindow();
}

int	GetAnInt(int* i)
{
	int		x;
	char	a;

	(*i)++;		// skip control
	a = credits[*i];	// get char
	(*i)++;
	x = a & 0x0F;
	a = credits[*i];
	(*i)++;
	x = (x * 10) + (a & 0x0F);
	a = credits[*i];
	(*i)++;
	x = (x * 10) + (a & 0x0F);
	return(x);
}

void CCredits::BuildBitmap(CDC *dc)
{
	int		row = 0;
	int		col = 0;
	int		i = 0;
	CString	cs;
	int		rowinc = 20;
	int		colinc = 10;

	CRect	rect(0,0,CR_WIDTH, CR_HEIGHT);
	dc->FillSolidRect(&rect, RGB(0, 0, 0));
	dc->SetTextColor(RGB(255,255,255));

	LOGFONT	lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = -(CR_WIDTH / 42);
	lf.lfPitchAndFamily = FF_SWISS;
#if 0
	CString e;
	e.Format("Rect w=%d h=%d lfHeight=%d", crRect.Width(), crRect.Height(),
		lf.lfHeight);
	AfxMessageBox(e);
#endif
	haveFont = font.CreateFontIndirect(&lf);
	CFont*	oldfont = dc->SelectObject(&font);

	for (i=0; i<credits.GetLength();)
	{
		if (credits[i] == '#')
		{
			if (cs.GetLength())			// anything in the buffer?
			{							// yes, flush it
				dc->TextOut(col*10+2, row*20 + crRect.Height(), cs);
				cs = "";
			}
			switch(credits[++i])
			{
			case 'r':
				row = row + GetAnInt(&i);
				col = 0;
				break;
			case 'c':
				col = GetAnInt(&i);
				break;
			default:
				AfxMessageBox("Unknown command in credits");
				break;
			}
		}
		else
		{
			cs += credits[i++];
		}
	}
	dc->SelectObject(oldfont);
	DeleteObject(&font);
}

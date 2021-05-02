/* $Id: xpilotView.cpp,v 4.3 2001/03/11 22:37:39 bert Exp $
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
*  xpilotView.cpp : implementation of the CXpilotView class					*
*																			*
*  XPilot.exe uses the standard MFC doc/view model in an SDI format.		*
*  The view (and this file) are the primary interface into the XPilot code.	*
*  $Id: xpilotView.cpp,v 4.3 2001/03/11 22:37:39 bert Exp $					*
\***************************************************************************/

#include "stdafx.h"
#include "XPilotNT.h"
#include <io.h>
#include <stdio.h>

#include "xpilotDoc.h"
#include "xpilotView.h"
//#include "SysInfo.h"
#include "BSString.h"

#define _XPDOC
#include "winClient.h"
#include "winXXPilot.h"
#include "winNet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" const char*	s_WindowMet = "Window Metrics";
extern "C" const char*	s_L = "Left";
extern "C" const char*	s_T = "Top";
extern "C" const char*	s_R = "Right";
extern "C" const char*	s_B = "Bottom";


/////////////////////////////////////////////////////////////////////////////
// CXpilotView

IMPLEMENT_DYNCREATE(CXpilotView, CView)

BEGIN_MESSAGE_MAP(CXpilotView, CView)
	//{{AFX_MSG_MAP(CXpilotView)
	ON_WM_DESTROY()
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_UPDATE_COMMAND_UI(ID_FILE_NEW, OnUpdateFileNew)
	ON_WM_SHOWWINDOW()
	ON_WM_CREATE()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP

	ON_MESSAGE(WSA_EVENT, OnWSA_EVENT)
	//ON_MESSAGE(WSA_EVENT, OnWSA_EVENT)
	//ON_MESSAGE(WSA_CONNECT, OnWSA_CONNECT)
	//ON_MESSAGE(WSA_RESOLVEHOST, OnWSA_RESOLVEHOST)

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CXpilotView construction/destruction
CXpilotView::CXpilotView()
{
	// TODO: add construction code here
	isVirgin = TRUE;
	shuttingdown = FALSE;
#ifdef	_BETAEXPIRE
	extern void CheckBetaExpire();
	CheckBetaExpire();
#endif
}

#ifdef	_DEBUG
extern	"C" void	xpmemShutdown();
#endif

CXpilotView::~CXpilotView()
{
//	Client_cleanup();
	WinXShutdown();
	CView::~CView();
#if defined(_DEBUG)
	xpmemShutdown();
#endif
}

BOOL CXpilotView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

#ifdef	_BETAEXPIRE
//	if (BETACHECK())		// if they wanna run old stuff, we warned them!
//		return(0);
#endif
	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CXpilotView drawing
//extern "C" void Make_table(void);
CString* CheckFileOpts(char* file, int* argc, char** argv)
{
	FILE* fp;
	if (_access(file, 2) != 0)
		return(NULL);
	if ((fp=fopen(file, "r")) == NULL)
	{
		CString e;
		e.Format("Can't open file <%s> for reading", file);
		AfxMessageBox(e);
		return(NULL);
	}
	long	len = _filelength(_fileno(fp));
	CString* cs = new CString((TCHAR)0, len);

	return(cs);
}

void CXpilotView::OnDraw(CDC* pDC)
{
	CXpilotDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (isVirgin)
	{
		int		ret;
		int		argc = 1;
		char*	argv[256];
		char	cs[1024];

		CString*	ccs[256];
		int			args=0;

		isVirgin = FALSE;
		GetParentFrame()->SetWindowText("XPilot");
		strncpy(cs, theApp.m_lpCmdLine, 1024);

		argv[0] = "xpilot";

		argv[1] = strtok(cs, " \t\n\r\0");
		if (argv[1])
		{
			if ((ccs[args] = CheckFileOpts(argv[1], &argc, argv)) != NULL)
				args++;
			argc++;
			while ((argv[argc] = strtok(NULL, "\t\n\r\0")) != (char*)NULL)
			{
				if ((ccs[args] = CheckFileOpts(argv[1], &argc, argv)) != NULL)
					args++;
				argc++;
			}
		}
		// Here is where we call xpilot proper.
		// This gives Windows/MFC time to setup/settle down before we do fun things
		TRACE("Eat Me\n");
		notifyWnd = GetSafeHwnd();
		ret = main(argc, argv);

		for (int i=0; i<args; i++)
			delete ccs[i];

		if (!ret)
		{
			ret = WSAAsyncSelect(Net_fd(), this->m_hWnd, WSA_EVENT, FD_CLOSE|FD_READ);
			if (ret)
			{
				char	s[256];
				sprintf(s, "AsyncSelect error=%d (%s)",
					WSAGetLastError(), GetWSockErrText(WSAGetLastError()));
				AfxMessageBox(s);
			}
			Net_input();
			ret = WSAAsyncSelect(Net_fd(), this->m_hWnd, WSA_EVENT, FD_CLOSE|FD_READ);

		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CXpilotView diagnostics

#ifdef _DEBUG
void CXpilotView::AssertValid() const
{
	CView::AssertValid();
}

void CXpilotView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CXpilotDoc* CXpilotView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CXpilotDoc)));
	return (CXpilotDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CXpilotView message handlers

void CXpilotView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	InitWinX(this->m_hWnd);
}

BOOL CXpilotView::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	return CView::DestroyWindow();
}

long CXpilotView::OnWSA_EVENT(WPARAM wParam, LPARAM lParam)
{
	if (shuttingdown)		// once, i received a winsock event after nuking
		return(0);			// the network layer
	Net_input();
//	PaintWinClient();
	return(0);
}

void CXpilotView::OnSize(UINT /*nType*/, int cx, int cy)
{
	if (!isVirgin)
		Resize(top, cx, cy);
}

void CXpilotView::OnUpdateFileNew(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable();	
}

void CXpilotView::OnFileNew() 
{
	// TODO: Add your command handler code here
	// Net_input();
	
}


void CXpilotView::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CView::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	if (bShow == FALSE)
		return;
}

int CXpilotView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	// perform miscellaneous other WM_CREATE chores ....
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CWnd* pWnd = GetParent();
	GetParentFrame()->SetWindowText("XPilot");
	winHelpFile = theApp.m_pszHelpFilePath;			// needed for finding .ini files

	CRect rect;
	rect.left = GetPrivateProfileInt(s_WindowMet, s_L, 0, Get_xpilotini_file(1));
	rect.top = GetPrivateProfileInt(s_WindowMet, s_T, 0, Get_xpilotini_file(1));
	rect.right = GetPrivateProfileInt(s_WindowMet, s_R, 0, Get_xpilotini_file(1));
	rect.bottom = GetPrivateProfileInt(s_WindowMet, s_B, 0, Get_xpilotini_file(1));
	if (rect.left != rect.right)	// only move window to valid coordinates
		pWnd->MoveWindow(rect);

	return(0);
}

void CXpilotView::OnDestroy() 
{
	shuttingdown = TRUE;
	xpilotShutdown();
	CView::OnDestroy();

}


extern "C" void Key_event(XKeyEvent *event);

void CXpilotView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	XKeyEvent xk;

	if (nFlags & 0x4000)		// don't bother with auto-repeat
		return;
	xk.keycode = nFlags & 0x1FF;
	xk.ascii = nChar;
	xk.type = KeyPress;
	TRACE("KeyDown: c=%04X k=%04X flags=%04X\n", xk.ascii, xk.keycode, nFlags);
	Key_event(&xk);

}

void CXpilotView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	//CView::OnKeyUp(nChar, nRepCnt, nFlags);
	XKeyEvent xk;

	xk.keycode = nFlags & 0x1FF;
	xk.ascii = nChar;
	xk.type = KeyRelease;
	TRACE("KeyUp:   c=%04X k=%04X flags=%04X\n", xk.ascii, xk.keycode, nFlags);
	Key_event(&xk);
}

#if 0
void CXpilotView::OnUpdateConfigureSysinfo(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable();
}

void CXpilotView::OnConfigureSysinfo() 
{
	// TODO: Add your command handler code here
	CSysInfo	csi;
	csi.DoModal();
}
#endif

extern "C"
void DoWinAboutBox()
{
	theApp.OnCmdMsg(ID_APP_ABOUT, 0, NULL, NULL);
}

extern "C"
COLORREF	GetXPilotColor(int which, COLORREF defcolor)
{
	CString		key;
	CBSString	cs;
	COLORREF	newcolor;
	key.Format("color%d", which);
	cs = theApp.GetProfileString("Settings", key);
	if (!cs.GetLength())
		return(defcolor);
	newcolor = cs.ParseColor();
	return(newcolor);
}

extern "C"
int		GetMaxColors()
{
	int	color;
//	return(theApp.GetProfileInt("Settings", "maxColors", 0));
	color = GetPrivateProfileInt("Settings", "maxColors", 0, Get_xpilotini_file(0));
	if (!color)
		color = GetPrivateProfileInt("Settings", "maxColors", 0, Get_xpilotini_file(1));
	if (!color)
		color = GetPrivateProfileInt("Settings", "maxColors", 0, Get_xpilotini_file(2));
	return(color);
}

#if 0
extern "C"
int		GetScoreFontHeight()
{
	return(theApp.GetProfileInt("Settings", "scoreFontHeight", 0));
}
#endif

extern "C" int	scoresChanged;

void CXpilotView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (bActivate)
		scoresChanged = 1;

	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

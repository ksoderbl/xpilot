/* $Id: xpilotsDlg.cpp,v 5.3 2002/06/14 02:16:28 dik Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
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
*  xpilotsDlg.cpp - The main dialog wrapper for xpilots						*
*																			*
*  $Id: xpilotsDlg.cpp,v 5.3 2002/06/14 02:16:28 dik Exp $				*
\***************************************************************************/

#include "stdafx.h"
#include "xpilots.h"
#include "xpilotsDlg.h"
#include "ConfigDlg.h"
//#include "ConfigDialog.h"
#include "winServer.h"
#include "ExitXpilots.h"
#include "ReallyShutdown.h"
#include "../../common/version.h"
#include "../../common/NT/winNet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// extern "C" int	framesPerSecond;
extern "C" int	winTimer;
extern "C" HWND alarmWnd;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	afx_msg void OnButtonWwwXpilotOrg();
	afx_msg void OnButtonWwwBuckosoftCom();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
	SendDlgItemMessage(IDC_VERSION, WM_SETTEXT, 0, (LPARAM)(TITLE));
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_BN_CLICKED(IDC_BUTTON_WWW_XPILOT_ORG, OnButtonWwwXpilotOrg)
	ON_BN_CLICKED(IDC_BUTTON_WWW_BUCKOSOFT_COM, OnButtonWwwBuckosoftCom)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CAboutDlg::OnButtonWwwXpilotOrg() 
{
	ShellExecute(NULL, "open", "http://www.xpilot.org/", NULL, NULL, SW_SHOWDEFAULT);
}

void CAboutDlg::OnButtonWwwBuckosoftCom() 
{
	CString	cs;
	cs.Format("http://www.buckosoft.com/xpilot/0.%s.html", VERSION_WINDOWS);
	//ShellExecute(NULL, "open", "http://www.buckosoft.com/xpilot/0.9.html", NULL, NULL, SW_SHOWDEFAULT);
	ShellExecute(NULL, "open", (LPCTSTR)cs, NULL, NULL, SW_SHOWDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////
// CXpilotsDlg dialog

CXpilotsDlg::CXpilotsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CXpilotsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CXpilotsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_hEventStartGame = CreateEvent(NULL, FALSE, FALSE, NULL); // auto reset, initially reset
	m_hEventGameTerminated = CreateEvent(NULL, TRUE, TRUE, NULL); // manual reset, initially set
	m_hEventKillServerThread = CreateEvent(NULL, FALSE, FALSE, NULL); // auto reset, initially reset
	m_hEventServerThreadKilled = CreateEvent(NULL, FALSE, FALSE, NULL); // auto reset, initially reset

	m_ServerThreadInfo.m_hEventStartGame = m_hEventStartGame;
	m_ServerThreadInfo.m_hEventGameTerminated = m_hEventGameTerminated;
	m_ServerThreadInfo.m_hEventKillServerThread = m_hEventKillServerThread;
	m_ServerThreadInfo.m_hEventServerThreadKilled = m_hEventServerThreadKilled;

	m_pServerWorkerThread = NULL;

	virgin = TRUE;
#ifdef	_BETAEXPIRE
	extern void CheckBetaExpire();
	CheckBetaExpire();
#endif

}

void CXpilotsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CXpilotsDlg)
	DDX_Control(pDX, IDC_START_SERVER, m_start_server);
	DDX_Control(pDX, IDC_CONNECT_CLIENT, m_connectClient);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CXpilotsDlg, CDialog)
	//{{AFX_MSG_MAP(CXpilotsDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONFIGURE, OnConfigure)
	ON_BN_CLICKED(IDC_START_SERVER, OnStartServer)
	ON_BN_CLICKED(IDC_CONNECT_CLIENT, OnConnectClient)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MSGAVAILABLE, OnMsgAvailable)
	ON_MESSAGE(WM_UPDATESCORES, OnUpdateScores)
	ON_MESSAGE(WM_STARTTIMER, OnStartTimer)
	ON_MESSAGE(WM_SERVERKILLED, OnServerKilled)
	ON_MESSAGE(WM_GETHOSTNAME, OnGetHostName)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXpilotsDlg message handlers
char	s_WindowMet[] = "Window Metrics";
char	s_Commline[] = "Command Line";
char	s_L[] = "Left";
char	s_T[] = "Top";
char	s_R[] = "Right";
char	s_B[] = "Bottom";


BOOL CXpilotsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	CString strAboutMenu;
	strAboutMenu.LoadString(IDS_ABOUTBOX);
	if (!strAboutMenu.IsEmpty())
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	//CWnd* pWnd = GetParent();
	CRect rect;
//	GetParentOwner()->GetParentOwner()->GetWindowRect(&rect);
	GetWindowRect(&rect);
	LONG	width = rect.right - rect.left;
	LONG	height = rect.bottom - rect.top;
	//rect.right = theApp.GetProfileInt(s_WindowMet, s_R, 0);
	//rect.bottom = theApp.GetProfileInt(s_WindowMet, s_B, 0);
	//	pWnd->MoveWindow(rect);
	
	GetDlgItem(IDC_EDIT1)->GetWindowRect(&editRect);
	editRect.left -= (rect.left + GetSystemMetrics(SM_CXFRAME));
	editRect.top -= (rect.top + GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION));
	editRect.right -= rect.left;
	editRect.bottom -= rect.top;
	editAdj.x = width - editRect.right;
	editAdj.y = height- editRect.bottom;
	rect.left = theApp.GetProfileInt(s_WindowMet, s_L, -1);
	rect.top = theApp.GetProfileInt(s_WindowMet, s_T, -1);
	m_commandline = theApp.GetProfileString(s_Commline, s_Commline, NULL);
	if (!(rect.left = rect.right == -1))	// only move window to valid coordinates
		MoveWindow(rect.left, rect.top, width, height);

	m_connectClient.EnableWindow(FALSE);		// can't connect until we start the server
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CXpilotsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CXpilotsDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		if (virgin)
		{/*
			CRect rect;
			GetWindowRect(&rect);

			LONG	width = rect.right - rect.left;
			LONG	height = rect.bottom - rect.top;

			GetDlgItem(IDC_EDIT1)->GetWindowRect(&editRect);
			editAdj.x = rect.right - editRect.right;
			editAdj.y = rect.bottom - editRect.bottom;
		
	CRect rect;
	GetParentOwner()->GetParentOwner()->GetWindowRect(&rect);
	LONG	width = rect.right - rect.left;
	LONG	height = rect.bottom - rect.top;
	//rect.right = theApp.GetProfileInt(s_WindowMet, s_R, 0);
	//rect.bottom = theApp.GetProfileInt(s_WindowMet, s_B, 0);
	//	pWnd->MoveWindow(rect);
	
	GetDlgItem(IDC_EDIT1)->GetWindowRect(&editRect);
	editRect.left -= rect.left;
	editRect.top -= rect.top;
	editRect.right -= rect.left;
	editRect.bottom -= rect.top;
	editAdj.x = width - editRect.right;
	editAdj.y = height- editRect.bottom; */
			virgin = FALSE;
		}

		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CXpilotsDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CXpilotsDlg::OnConfigure() 
{
	// TODO: Add your control notification handler code here
//#if 0
#if 1
	CConfigDlg	cfg;
	cfg.m_commandline = m_commandline;
	int ret = cfg.DoModal();
	if (ret == IDOK)
	{
		m_commandline = cfg.m_commandline;
	}
#else
	CConfigDialog	cfg;
	cfg.m_commandline = m_commandline;
	int ret = cfg.DoModal();
	if (ret == IDOK)
	{
		m_commandline = cfg.m_commandline;
	}
#endif
//#endif
}


void CXpilotsDlg::OnStartServer() 
{
	// TODO: Add your control notification handler code here
	if (!server_running)
	{
//		int		ret;
		char	cs[1024];
		char	ct[1024];
		alarmWnd = GetSafeHwnd();
		if (m_pServerWorkerThread == NULL)
		{
			// Begin the worker thread.  It is ok to fill in the CThreadInfo
			// structure after the thread has started, because the thread
			// waits for the "start recalc" event before referring to the structure.
			m_pServerWorkerThread =
				AfxBeginThread(ServerThreadProc, &m_ServerThreadInfo);
		}

//		POSITION pos = GetFirstViewPosition();
//		ASSERT(pos != NULL);
//		CView* pView = GetNextView(pos);
//		ASSERT(pView != NULL);
//		m_ServerThreadInfo.m_hwndNotifyRecalcDone = m_hWnd;
//		m_ServerThreadInfo.m_hwndNotifyProgress = AfxGetMainWnd()->m_hWnd;
		m_ServerThreadInfo.m_hwndNotifyProgress = m_hWnd;

		// The events are initially set or reset in the CreateEvent call;
		// but they may be left in an improperly initialized state if
		// a worker thread has been previously started and then prematurely
		// killed.  Set/reset the events to the proper initial state.
		// Set the "start recalc" event last, since it is the event the
		// triggers the starting of the worker thread recalculation.
		SetEvent(m_hEventGameTerminated);
		ResetEvent(m_hEventKillServerThread);
		ResetEvent(m_hEventServerThreadKilled);
		// GameTerminated() will be called by the view when the thread sends a
		// WM_USER_GAME_TERMINATED message.
			m_start_server.SetWindowText("&Stop Server");
			server_running = TRUE;
			strcpy(cs, theApp.m_lpCmdLine);
			m_ServerThreadInfo.argv[0] = "xpilots";
			m_ServerThreadInfo.argc = 1;

			m_ServerThreadInfo.argv[1] = strtok(cs, " \t\n\r\0");
			if (m_ServerThreadInfo.argv[1])
			{
				m_ServerThreadInfo.argc++;
				while ((m_ServerThreadInfo.argv[m_ServerThreadInfo.argc] = strtok(NULL, "\t\n\r\0")) != (char*)NULL)
					m_ServerThreadInfo.argc++;
			}
			strcpy(ct, m_commandline);

			m_ServerThreadInfo.argv[m_ServerThreadInfo.argc] = strtok(ct, " \t\n\r\0");
			if (m_ServerThreadInfo.argv[m_ServerThreadInfo.argc])
			{
				m_ServerThreadInfo.argc++;
				while ((m_ServerThreadInfo.argv[m_ServerThreadInfo.argc] = strtok(NULL, " \t\n\r\0")) != (char*)NULL)
					m_ServerThreadInfo.argc++;
			}
		TRACE("Eat Me\n");
		SetEvent(m_hEventStartGame);
//		main(m_ServerThreadInfo.argc, m_ServerThreadInfo.argv);

	}
	else
	{
		CReallyShutdown	crs;
		if (crs.DoModal() != IDOK)
			return;
		::KillTimer(NULL, gTimer);
		SetEvent(m_hEventKillServerThread);
		SetEvent(m_hEventStartGame);

		WaitForSingleObject(m_hEventServerThreadKilled, INFINITE);

		m_pServerWorkerThread = NULL;
		m_start_server.SetWindowText("&Start Server");
		m_start_server.EnableWindow(FALSE);				/* can't restart yet... */
		m_connectClient.EnableWindow(FALSE);		// can't connect to this server anymore
		server_running = FALSE;

	}

}

void CXpilotsDlg::OnConnectClient() 
{
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	char	cline[] = "XPilot.exe 127.0.0.1";
	if (!CreateProcess(NULL, cline, NULL, NULL, false,
		NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
	{
		AfxMessageBox("Failed to create process for XPilot.exe");
	}
}

BOOL CXpilotsDlg::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	//CWnd* pWnd = GetParent();
	CRect rect;
	GetWindowRect(rect);
	theApp.WriteProfileInt(s_WindowMet, s_L, rect.left);
	theApp.WriteProfileInt(s_WindowMet, s_T, rect.top);
	theApp.WriteProfileInt(s_WindowMet, s_R, rect.right);
	theApp.WriteProfileInt(s_WindowMet, s_B, rect.bottom);
	theApp.WriteProfileString(s_Commline, s_Commline, m_commandline);
	return CDialog::DestroyWindow();
}

BOOL CXpilotsDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	if (pMsg->message == WM_SYSCOMMAND && pMsg->wParam == SC_CLOSE)
	{
		// from ~mtrecalc() sample.  Make sure worker thread is dead 
		DWORD dwExitCode;
		if (m_pServerWorkerThread != NULL &&
			GetExitCodeThread(m_pServerWorkerThread->m_hThread, &dwExitCode) &&
			dwExitCode == STILL_ACTIVE)
		{
			// Kill the worker thread by setting the "kill thread" event.
			// See comment in OnKillWorkerThread for explanation of the sequence
			// of the "kill thread" and "start recalc" events.
			ExitXpilots	ex;
			int ret = ex.DoModal();
			if (ret == IDCANCEL)
				return(1);
		//	SetEvent(m_hEventKillServerThread);
		//	SetEvent(m_hEventStartGame);
		//	WaitForSingleObject(m_hEventServerThreadKilled, 5*1000);
			return(0);
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

extern "C" int MsgAvail0;
extern "C" int MsgAvail1;
extern "C" int NumPlayers;
extern "C" int NumRobots;
extern "C" int NumPseudoPlayers;
afx_msg LONG CXpilotsDlg::OnMsgAvailable(UINT which, LONG s)
{
	int		l;

	l = SendDlgItemMessage(IDC_EDIT1, WM_GETTEXTLENGTH, 0, 0);
	//AfxMessageBox((LPCSTR)s);
	SendDlgItemMessage(IDC_EDIT1, EM_SETSEL, (WPARAM)l, (LPARAM)l);
//	SendDlgItemMessage(IDC_EDIT1, EM_SETSEL, (WPARAM)-1, (LPARAM)1000000);
	SendDlgItemMessage(IDC_EDIT1, EM_REPLACESEL, FALSE, (LPARAM)s);
#if 0
	if (which)
		MsgAvail1 = FALSE;
	else
		MsgAvail0 = FALSE;
#endif
	l = SendDlgItemMessage(IDC_EDIT1, WM_GETTEXTLENGTH, 0, 0);
	SendDlgItemMessage(IDC_EDIT1, EM_SETSEL, (WPARAM)l, (LPARAM)l);
//	SendDlgItemMessage(IDC_EDIT1, EM_SETSEL, (WPARAM)-1, (LPARAM)1000000);
	SendDlgItemMessage(IDC_EDIT1, EM_SCROLLCARET, 0, 0);
//	sprintf(st, "%d", NumPlayers - NumRobots - NumPseudoPlayers);
//	SendDlgItemMessage(IDC_PLAYERCOUNT, WM_SETTEXT, 0, (LPARAM)st);
//	OnUpdateScores();
	return(0);
}

afx_msg LONG CXpilotsDlg::OnUpdateScores(UINT unused, LONG unused1)
{
	char	st[10];
	sprintf(st, "%d", NumPlayers - NumRobots - NumPseudoPlayers);
	//TRACE("OnUpdateScores <%s>\n", st);
	SendDlgItemMessage(IDC_PLAYERCOUNT, WM_SETTEXT, 0, (LPARAM)st);
	return(0);

}
afx_msg LRESULT CXpilotsDlg::OnGetHostName(WPARAM wParam, LPARAM lParam)
{
	m_pServerWorkerThread->PostThreadMessage(WM_GETHOSTNAME, wParam, lParam);
	return(0);
}

afx_msg LONG CXpilotsDlg::OnStartTimer(UINT unused, LONG fps)
{
	gTimer= ::SetTimer(NULL, 0, 1000/fps, (TIMERPROC)ServerThreadTimerProc);
	m_connectClient.EnableWindow(TRUE);
	return(0);
}

afx_msg LONG CXpilotsDlg::OnServerKilled(UINT, LONG)
{
	m_start_server.SetWindowText("&Start Server");
	m_start_server.EnableWindow(FALSE);				/* can't restart yet... */
	server_running = FALSE;
	return(0);
}

void CXpilotsDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	CRect	wrect;
	CRect	erect;
	CWnd* cw = GetDlgItem(IDC_EDIT1);
	GetWindowRect(&wrect);
	if (cw) cw->GetWindowRect(&erect);
	CRect	rect;
	rect.left = editRect.left;
	rect.top  = editRect.top;
	rect.right = cx - editAdj.x;
	rect.bottom = cy - editAdj.y;

	if (!virgin)
	{
		if (cw)
			cw->MoveWindow(rect);
	}
}

void CXpilotsDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default

	if (server_running)
	{
		int ret = AfxMessageBox("Server is running.  Exit Anyway?",	MB_YESNO);
		if (ret != IDYES)
			return;
	}
	CDialog::OnClose();
}

BOOL CXpilotsDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	// TODO: Add your message handler code here and/or call default
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	
	// return CDialog::OnHelpInfo(pHelpInfo);
		return(TRUE);
}

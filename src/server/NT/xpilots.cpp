/* $Id: xpilots.cpp,v 5.1 2001/05/28 00:19:25 dik Exp $
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
*  xpilots.cpp - The main xpilots Application class							*
*																			*
\***************************************************************************/

#include "stdafx.h"
#include "xpilots.h"
#include "xpilotsDlg.h"

extern "C" bool Parser(int argc, char **argv);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXpilotsApp

BEGIN_MESSAGE_MAP(CXpilotsApp, CWinApp)
	//{{AFX_MSG_MAP(CXpilotsApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXpilotsApp construction

CXpilotsApp::CXpilotsApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CXpilotsApp object

CXpilotsApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CXpilotsApp initialization

BOOL CXpilotsApp::InitInstance()
{
	if (!strncmp(m_lpCmdLine, "/ServerOpts", 11))
	{
		char* s = strdup(m_lpCmdLine);
		char* seps = " ,\t\n";
		char* token = strtok(s, seps);	// skip the /ServerOpts
		token = strtok(NULL, seps);		// get the file name
		char* fname = "ServerOpts.txt";
		if (token && strlen(token))
			fname = token;
		char* parms[2] = {"XPilotServer", "-help"};
		FILE* fp = freopen(fname, "w", stdout);
		Parser(2, parms);
		fclose(fp);
		free(s);
		return(FALSE);

	}
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CXpilotsDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CXpilotsApp::ExitInstance() 
{
	// TODO: Add your specialized code here and/or call the base class
	WSACleanup();
	return CWinApp::ExitInstance();
}

// interface routines to the xpilot "C" code
extern "C" 
void _Trace(char* lpszFormat, long a, long b, long c, long d, long e)
{
	AfxTrace(lpszFormat, a, b, c, d, e);
}

extern "C" const char*	GetWSockErrText(int	error);
extern "C"
const char* _GetWSockErrText(int error)
{
	return(GetWSockErrText(error));
}

extern "C" int	server_running = FALSE;

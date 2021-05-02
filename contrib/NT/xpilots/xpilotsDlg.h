/* $Id: xpilotsDlg.h,v 1.5 1997/11/27 20:08:58 bert Exp $
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
*  xpilotsDlg.h - The main dialog wrapper for xpilots						*
*																			*
*  $Id: xpilotsDlg.h,v 1.5 1997/11/27 20:08:58 bert Exp $				*
\***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// CXpilotsDlg dialog
#ifndef	_XPILOTSDLG_H_
#define	_XPILOTSDLG_H_

#include "WinSvrThread.h"

class CXpilotsDlg : public CDialog
{
// Construction
public:
	CXpilotsDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CXpilotsDlg)
	enum { IDD = IDD_XPILOTS_DIALOG };
	CButton	m_start_server;
	//}}AFX_DATA

	CRect	editRect;
	CPoint	editAdj;
	BOOL	virgin;
	UINT	gTimer;			// game timer
	CString	m_commandline;
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXpilotsDlg)
	public:
	virtual BOOL DestroyWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	CWinThread* m_pServerWorkerThread;
	CServerThreadInfo m_ServerThreadInfo;
	HANDLE m_hEventStartGame;
	HANDLE m_hEventGameTerminated;
	HANDLE m_hEventKillServerThread;
	HANDLE m_hEventServerThreadKilled;
	// Generated message map functions
	//{{AFX_MSG(CXpilotsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnConfigure();
	afx_msg void OnStartServer();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	afx_msg LONG OnMsgAvailable(UINT which, LONG s);
	afx_msg LONG OnUpdateScores(UINT unused, LONG unused1);
	afx_msg LONG OnStartTimer(UINT unused, LONG fps);
	afx_msg	LONG OnGetHostName(UINT unused, LONG unused1);
	DECLARE_MESSAGE_MAP()
};

#endif	// _XPILOTSDLG_H_

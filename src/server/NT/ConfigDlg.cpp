/* $Id: ConfigDlg.cpp,v 5.3 2001/10/11 20:16:45 bertg Exp $
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
*  ConfigDlg.cpp - The main configuration dialog							*
*																			*
\***************************************************************************/

#include "stdafx.h"
#include <afxdlgs.h>

#include "xpilots.h"
#include "ConfigDlg.h"
//#include "ConfigDialog.h"
#include "winSvrThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg dialog


CConfigDlg::CConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigDlg)
	m_commandline = _T("");
	//}}AFX_DATA_INIT
}


void CConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigDlg)
	DDX_Control(pDX, IDC_STATIC_URL2, m_staticUrl2);
	DDX_Control(pDX, IDC_STATIC_URL, m_staticUrl);
	DDX_Text(pDX, IDC_COMMANDLINE, m_commandline);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CConfigDlg)
	ON_BN_CLICKED(IDC_LOAD_MAP, OnLoadMap)
	ON_BN_CLICKED(IDC_SETOPTIONS, OnSetoptions)
	ON_BN_CLICKED(IDC_STATIC_URL, OnStaticUrl)
	ON_BN_CLICKED(IDC_STATIC_URL2, OnStaticUrl2)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CHOOSEMAP, OnChoosemap)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg message handlers

BOOL CConfigDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	urlFont.CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE, ANSI_CHARSET, 
		OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS, 
		NULL);
	m_staticUrl.SetFont(&urlFont);
	m_staticUrl2.SetFont(&urlFont);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	DeleteObject(urlFont);
}

void CConfigDlg::OnLoadMap() 
{
	// TODO: Add your control notification handler code here
	
}

void CConfigDlg::OnSetoptions() 
{
	// TODO: Add your control notification handler code here
#if 0
	CConfigDialog	cfg;
	cfg.m_commandline = m_commandline;
	int ret = cfg.DoModal();
	if (ret == IDOK)
	{
		m_commandline = cfg.m_commandline;
	}
#endif
}

void CConfigDlg::OnStaticUrl() 
{
	CString	cs = "doc/ServerOpts.txt";
	ShellExecute(NULL, "open", cs, NULL, NULL, SW_SHOWDEFAULT);	
}

void CConfigDlg::OnStaticUrl2() 
{
	CString	cs = "http://www.j-a-r-n-o.nl/xpilotserver.html";
	ShellExecute(NULL, "open", cs, NULL, NULL, SW_SHOWDEFAULT);	
}


HBRUSH CConfigDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if (nCtlColor == CTLCOLOR_STATIC && pWnd->GetDlgCtrlID() == IDC_STATIC_URL)
		pDC->SetTextColor(RGB(0,0,255));
	if (nCtlColor == CTLCOLOR_STATIC && pWnd->GetDlgCtrlID() == IDC_STATIC_URL2)
		pDC->SetTextColor(RGB(0,0,255));
	return hbr;
}

void CConfigDlg::OnChoosemap() 
{
	static char BASED_CODE szFilter[] = "XPilot Mapfiles (*.xp, *.map)|*.xp;*.map";
	char	dir[MAX_PATH+1];
	CString	olddir;

	CFileDialog	fc(TRUE, ".xp", NULL, NULL, szFilter, this);
	GetCurrentDirectory(MAX_PATH, dir);
	olddir = dir;

	if (SetCurrentDirectory("lib/maps"))
		GetCurrentDirectory(MAX_PATH, dir);
	fc.m_ofn.lpstrInitialDir = dir;
	if (fc.DoModal() == IDOK)
	{
		UpdateData();

		if (m_commandline.GetLength())
			m_commandline += " ";
		m_commandline += "-map ";
		if(!stricmp(fc.GetPathName(), (olddir + "\\lib\\maps\\" + fc.GetFileName())))
			m_commandline += fc.GetFileName();
		else
			m_commandline += fc.GetPathName();

		UpdateData(FALSE);
	}
	SetCurrentDirectory(olddir);
}


/* $Id: ConfTeams.h,v 1.5 1997/11/27 20:08:53 bert Exp $
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
*  ConfTeams.h - Team Settings Property Page for the ConfigDialog			*
*																			*
*  $Id: ConfTeams.h,v 1.5 1997/11/27 20:08:53 bert Exp $					*
\***************************************************************************/
#if !defined(AFX_CONFTEAMS_H__11949A11_E06F_11D0_93E5_006097A20515__INCLUDED_)
#define AFX_CONFTEAMS_H__11949A11_E06F_11D0_93E5_006097A20515__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CConfTeams dialog

class CConfTeams : public CPropertyPage
{
	DECLARE_DYNCREATE(CConfTeams)

// Construction
public:
	CConfTeams();
	~CConfTeams();

// Dialog Data
	//{{AFX_DATA(CConfTeams)
	enum { IDD = IDD_CONF_TEAMS };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CConfTeams)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CConfTeams)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFTEAMS_H__11949A11_E06F_11D0_93E5_006097A20515__INCLUDED_)

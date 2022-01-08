/* $Id: ReallyShutdown.h,v 5.0 2001/04/07 20:01:01 dik Exp $
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
*  ReallyShutdown.cpp - "Really Shutdown the Server?"						*
*						(give # of seconds warning)							*
*																			*
\***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// CReallyShutdown dialog

class CReallyShutdown : public CDialog
{
// Construction
public:
	CReallyShutdown(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CReallyShutdown)
	enum { IDD = IDD_REALLYSHUTDOWN };
	UINT	m_delay;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReallyShutdown)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CReallyShutdown)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/* $Id: ConfRobots.cpp,v 1.5 1997/11/27 20:08:51 bert Exp $
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
*  ConfRobots.cpp - Robot Settings Property Page for the ConfigDialog		*
*																			*
\***************************************************************************/

#include "stdafx.h"
#include "xpilots.h"
#include "ConfRobots.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfRobots property page

IMPLEMENT_DYNCREATE(CConfRobots, CPropertyPage)

CConfRobots::CConfRobots() : CPropertyPage(CConfRobots::IDD)
{
	//{{AFX_DATA_INIT(CConfRobots)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CConfRobots::~CConfRobots()
{
}

void CConfRobots::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfRobots)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfRobots, CPropertyPage)
	//{{AFX_MSG_MAP(CConfRobots)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfRobots message handlers

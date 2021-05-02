/* $Id: BetaCheck.cpp,v 1.5 1997/11/27 20:08:37 bert Exp $
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
*  BetaCheck.cpp - See if this is an outdated copy of the program.			*
*																			*
\***************************************************************************/

#include "stdafx.h"
#include "XPilotNT.h"
#include "BetaCheck.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef	_BETAEXPIRE
static	WORD beta_year_expire = 1997;
BOOL	betaexpired = FALSE;

static void TellExpired()
{
	CExpired ce;
	ce.DoModal();
	betaexpired = TRUE;
}

static void TellWillExpire()
{
	CExpireSoon ce;
	ce.DoModal();
}

void CheckBetaExpire()
{
	WORD beta_month_expire = 10;

	SYSTEMTIME	st;
	GetSystemTime(&st);
	if (beta_year_expire < st.wYear)
	{
		TellExpired();
		return;
	}
	if (beta_year_expire == st.wYear)
	{
		if (beta_month_expire < st.wMonth)
		{
			TellExpired();
			return;
		}
		if (beta_month_expire == st.wMonth && st.wDay > 15)
		{
			TellWillExpire();
		}
	}
	betaexpired = FALSE;
}

#endif

/////////////////////////////////////////////////////////////////////////////
// CExpireSoon dialog


CExpireSoon::CExpireSoon(CWnd* pParent /*=NULL*/)
	: CDialog(CExpireSoon::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExpireSoon)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CExpireSoon::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExpireSoon)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExpireSoon, CDialog)
	//{{AFX_MSG_MAP(CExpireSoon)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExpireSoon message handlers
/////////////////////////////////////////////////////////////////////////////
// CExpired dialog


CExpired::CExpired(CWnd* pParent /*=NULL*/)
	: CDialog(CExpired::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExpired)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CExpired::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExpired)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExpired, CDialog)
	//{{AFX_MSG_MAP(CExpired)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExpired message handlers

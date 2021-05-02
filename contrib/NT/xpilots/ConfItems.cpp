/* $Id: ConfItems.cpp,v 1.5 1997/11/27 20:08:50 bert Exp $
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
*  ConfItems.cpp - Item Settings Property Page for the ConfigDialog			*
*																			*
\***************************************************************************/

#include "stdafx.h"
#include "xpilots.h"
#include "ConfItems.h"
extern "C" {
#include "../../../src/defaults.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfItems property page

IMPLEMENT_DYNCREATE(CConfItems, CPropertyPage)

CConfItems::CConfItems() : CPropertyPage(CConfItems::IDD)
{
	//{{AFX_DATA_INIT(CConfItems)
	//}}AFX_DATA_INIT
#define	MAXSLIDER	1000000
	CString	cs;
//	int		v;
//	double	d;
	m_slider_nukeClusterDamage_value = ((int)(atof(findOption("nukeClusterDamage")->defaultValue)*MAXSLIDER));
	m_slider_movingItemProb_value = ((int)(atof(findOption("movingItemProb")->defaultValue)*MAXSLIDER));
	m_slider_itemConcentratorProb_value = ((int)(atof(findOption("itemConcentratorProb")->defaultValue)*MAXSLIDER));
	m_slider_itemConcentratorRadius_value = ((int)(atoi(findOption("itemConcentratorRadius")->defaultValue)));
	m_slider_dropItemOnKillProb_value = ((int)(atof(findOption("dropItemOnKillProb")->defaultValue)*MAXSLIDER));
	m_slider_destroyItemInCollisionProb_value = ((int)(atof(findOption("destroyItemInCollisionProb")->defaultValue)*MAXSLIDER));
	m_slider_itemProbMult_value = ((int)(atof(findOption("itemProbMult")->defaultValue)*MAXSLIDER));
	m_slider_maxItemDensity_value = ((int)(atof(findOption("maxItemDensity")->defaultValue)*MAXSLIDER));
	m_slider_rogueHeatProb_value = ((int)(atof(findOption("rogueHeatProb")->defaultValue)*MAXSLIDER));
	m_slider_rogueMineProb_value = ((int)(atof(findOption("rogueMineProb")->defaultValue)*MAXSLIDER));

}

CConfItems::~CConfItems()
{
}

void CConfItems::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfItems)
	DDX_Control(pDX, IDC_EDIT_MINEFUSETIME, m_edit_mineFuseTime);
	DDX_Control(pDX, IDC_SLIDER_ROGUEMINEPROB, m_slider_rogueMineProb);
	DDX_Control(pDX, IDC_SLIDER_ROGUEHEATPROB, m_slider_rogueHeatProb);
	DDX_Control(pDX, IDC_SLIDER_ITEMCONCENTRATORPROB, m_slider_itemConcentratorProb);
	DDX_Control(pDX, IDC_SLIDER_ITEMCONCENTRATORRADIUS, m_slider_itemConcentratorRadius);
	DDX_Control(pDX, IDC_SLIDER_MAXITEMDENSITY, m_slider_maxItemDensity);
	DDX_Control(pDX, IDC_SLIDER_ITEMPROBMULT, m_slider_itemProbMult);
	DDX_Control(pDX, IDC_SLIDER_DESTROYITEMINCOLLISIONPROB, m_slider_destroyItemInCollisionProb);
	DDX_Control(pDX, IDC_SLIDER_DROPITEMONKILLPROB, m_slider_dropItemOnKillProb);
	DDX_Control(pDX, IDC_CHECK_ROGUEMINEPROB, m_check_rogueMineProb);
	DDX_Control(pDX, IDC_CHECK_ROGUEHEATPROB, m_check_rogueHeatProb);
	DDX_Control(pDX, IDC_CHECK_ITEMCONCENTRATORPROB, m_check_itemConcentratorProb);
	DDX_Control(pDX, IDC_CHECK_ITEMCONCENTRATORRADIUS, m_check_itemConcentratorRadius);
	DDX_Control(pDX, IDC_CHECK_MAXITEMDENSITY, m_check_maxItemDensity);
	DDX_Control(pDX, IDC_CHECK_ITEMPROBMULT, m_check_itemProbMult);
	DDX_Control(pDX, IDC_CHECK_DESTROYITEMINCOLLISIONPROB, m_check_destroyItemInCollisionProb);
	DDX_Control(pDX, IDC_CHECK_DROPITEMONKILLPROB, m_check_dropItemOnKillProb);
	DDX_Control(pDX, IDC_CHECK_MINEFUSETIME, m_check_mineFuseTime);
	DDX_Control(pDX, IDC_CHECK_MOVINGITEMPROB, m_check_movingItemProb);
	DDX_Control(pDX, IDC_SLIDER_MOVINGITEMPROB, m_slider_movingItemProb);
	DDX_Control(pDX, IDC_SLIDER_NUKECLUSTERDAMAGE, m_slider_nukeClusterDamage);
	DDX_Control(pDX, IDC_CHECK_NUKECLUSTERDAMAGE, m_check_nukeClusterDamage);
	DDX_Control(pDX, IDC_EDIT_NUKEMINMINES, m_edit_nukeMinMines);
	DDX_Control(pDX, IDC_EDIT_NUKEMINSMARTS, m_edit_nukeMinSmarts);
	DDX_Control(pDX, IDC_EDIT_MAXMISSILESPERPACK, m_edit_maxMissilesPerPack);
	DDX_Control(pDX, IDC_CHECK_NUKEMINMINES, m_check_nukeMinMines);
	DDX_Control(pDX, IDC_CHECK_NUKEMINSMARTS, m_check_nukeMinSmarts);
	DDX_Control(pDX, IDC_CHECK_MAXMISSILESPERPACK, m_check_maxMissilesPerPack);
	//}}AFX_DATA_MAP
	OnCheckMaxmissilesperpack();
	OnCheckNukeminsmarts();
	OnCheckNukeminmines();
	OnCheckNukeclusterdamage();
	OnCheckMovingitemprob();
	OnCheckMinefusetime();
	OnCheckDropitemonkillprob();
	OnCheckDestroyitemincollisionprob();
	OnCheckItemprobmult();
	OnCheckMaxitemdensity();
	OnCheckItemconcentratorradius();
	OnCheckItemconcentratorprob();
	OnCheckRogueheatprob();
	OnCheckRoguemineprob();
}


BEGIN_MESSAGE_MAP(CConfItems, CPropertyPage)
	//{{AFX_MSG_MAP(CConfItems)
	ON_BN_CLICKED(IDC_CHECK_MAXMISSILESPERPACK, OnCheckMaxmissilesperpack)
	ON_BN_CLICKED(IDC_CHECK_NUKEMINSMARTS, OnCheckNukeminsmarts)
	ON_BN_CLICKED(IDC_CHECK_NUKEMINMINES, OnCheckNukeminmines)
	ON_BN_CLICKED(IDC_CHECK_NUKECLUSTERDAMAGE, OnCheckNukeclusterdamage)
	ON_BN_CLICKED(IDC_CHECK_MOVINGITEMPROB, OnCheckMovingitemprob)
	ON_BN_CLICKED(IDC_CHECK_MINEFUSETIME, OnCheckMinefusetime)
	ON_BN_CLICKED(IDC_CHECK_DROPITEMONKILLPROB, OnCheckDropitemonkillprob)
	ON_BN_CLICKED(IDC_CHECK_DESTROYITEMINCOLLISIONPROB, OnCheckDestroyitemincollisionprob)
	ON_BN_CLICKED(IDC_CHECK_ITEMPROBMULT, OnCheckItemprobmult)
	ON_BN_CLICKED(IDC_CHECK_MAXITEMDENSITY, OnCheckMaxitemdensity)
	ON_BN_CLICKED(IDC_CHECK_ITEMCONCENTRATORRADIUS, OnCheckItemconcentratorradius)
	ON_BN_CLICKED(IDC_CHECK_ITEMCONCENTRATORPROB, OnCheckItemconcentratorprob)
	ON_BN_CLICKED(IDC_CHECK_ROGUEHEATPROB, OnCheckRogueheatprob)
	ON_BN_CLICKED(IDC_CHECK_ROGUEMINEPROB, OnCheckRoguemineprob)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfItems message handlers


void CConfItems::OnCheckMaxmissilesperpack() 
{
	// TODO: Add your control notification handler code here
	CString	cs;
	m_edit_maxMissilesPerPack.GetWindowText(cs);
	if (!cs.GetLength())
	{
		optionDesc*	od = findOption("maxMissilesPerPack");
		if (od)
		{
			cs.Format("%s", (const char*)od->defaultValue);
			m_edit_maxMissilesPerPack.SetWindowText(cs);
		}
	}
	if (m_check_maxMissilesPerPack.GetCheck())
		m_edit_maxMissilesPerPack.EnableWindow(TRUE);
	else
		m_edit_maxMissilesPerPack.EnableWindow(FALSE);
}

void CConfItems::OnCheckNukeminsmarts() 
{
	// TODO: Add your control notification handler code here
	CString	cs;
	m_edit_nukeMinSmarts.GetWindowText(cs);
	if (!cs.GetLength())
	{
		optionDesc*	od = findOption("nukeMinSmarts");
		if (od)
		{
			cs.Format("%s", (const char*)od->defaultValue);
			m_edit_nukeMinSmarts.SetWindowText(cs);
		}
	}
	if (m_check_nukeMinSmarts.GetCheck())
		m_edit_nukeMinSmarts.EnableWindow(TRUE);
	else
		m_edit_nukeMinSmarts.EnableWindow(FALSE);
	
}

void CConfItems::OnCheckNukeminmines() 
{
	// TODO: Add your control notification handler code here
	CString	cs;
	m_edit_nukeMinMines.GetWindowText(cs);
	if (!cs.GetLength())
	{
		optionDesc*	od = findOption("nukeMinMines");
		if (od)
		{
			cs.Format("%s", (const char*)od->defaultValue);
			m_edit_nukeMinMines.SetWindowText(cs);
		}
	}
	if (m_check_nukeMinMines.GetCheck())
		m_edit_nukeMinMines.EnableWindow(TRUE);
	else
		m_edit_nukeMinMines.EnableWindow(FALSE);
	
	
}

void CConfItems::OnCheckMinefusetime() 
{
	// TODO: Add your control notification handler code here
	CString	cs;
	m_edit_mineFuseTime.GetWindowText(cs);
	if (!cs.GetLength())
	{
		optionDesc*	od = findOption("mineFuseTime");
		if (od)
		{
			cs.Format("%s", (const char*)od->defaultValue);
			m_edit_mineFuseTime.SetWindowText(cs);
		}
	}
	if (m_check_mineFuseTime.GetCheck())
		m_edit_mineFuseTime.EnableWindow(TRUE);
	else
		m_edit_mineFuseTime.EnableWindow(FALSE);
}

void CConfItems::OnCheckNukeclusterdamage() 
{
	// TODO: Add your control notification handler code here

	m_slider_nukeClusterDamage.SetRange(0, MAXSLIDER);
	m_slider_nukeClusterDamage.SetPos(m_slider_nukeClusterDamage_value);
	if (m_check_nukeClusterDamage.GetCheck())
		m_slider_nukeClusterDamage.EnableWindow(TRUE);
	else
		m_slider_nukeClusterDamage.EnableWindow(FALSE);
}

void CConfItems::OnCheckMovingitemprob() 
{
	// TODO: Add your control notification handler code here
	m_slider_movingItemProb.SetRange(0, MAXSLIDER);
	m_slider_movingItemProb.SetPos(m_slider_movingItemProb_value);
	if (m_check_movingItemProb.GetCheck())
		m_slider_movingItemProb.EnableWindow(TRUE);
	else
		m_slider_movingItemProb.EnableWindow(FALSE);
}

void CConfItems::OnCheckDropitemonkillprob() 
{
	// TODO: Add your control notification handler code here
	m_slider_dropItemOnKillProb.SetRange(0, MAXSLIDER);
	m_slider_dropItemOnKillProb.SetPos(m_slider_dropItemOnKillProb_value);
	if (m_check_dropItemOnKillProb.GetCheck())
		m_slider_dropItemOnKillProb.EnableWindow(TRUE);
	else
		m_slider_dropItemOnKillProb.EnableWindow(FALSE);
}

void CConfItems::OnCheckDestroyitemincollisionprob() 
{
	// TODO: Add your control notification handler code here
	m_slider_destroyItemInCollisionProb.SetRange(0, MAXSLIDER);
	m_slider_destroyItemInCollisionProb.SetPos(m_slider_destroyItemInCollisionProb_value);
	if (m_check_destroyItemInCollisionProb.GetCheck())
		m_slider_destroyItemInCollisionProb.EnableWindow(TRUE);
	else
		m_slider_destroyItemInCollisionProb.EnableWindow(FALSE);
}

void CConfItems::OnCheckItemprobmult() 
{
	// TODO: Add your control notification handler code here
	m_slider_itemProbMult.SetRange(0, MAXSLIDER);
	m_slider_itemProbMult.SetPos(m_slider_itemProbMult_value);
	if (m_check_itemProbMult.GetCheck())
		m_slider_itemProbMult.EnableWindow(TRUE);
	else
		m_slider_itemProbMult.EnableWindow(FALSE);
}

void CConfItems::OnCheckMaxitemdensity() 
{
	// TODO: Add your control notification handler code here
	m_slider_maxItemDensity.SetRange(0, MAXSLIDER);
	m_slider_maxItemDensity.SetPos(m_slider_maxItemDensity_value);
	if (m_check_maxItemDensity.GetCheck())
		m_slider_maxItemDensity.EnableWindow(TRUE);
	else
		m_slider_maxItemDensity.EnableWindow(FALSE);
}

void CConfItems::OnCheckItemconcentratorradius() 
{
	// TODO: Add your control notification handler code here
	m_slider_itemConcentratorRadius.SetRange(0, 20);
	m_slider_itemConcentratorRadius.SetPos(m_slider_itemConcentratorRadius_value);
	if (m_check_itemConcentratorRadius.GetCheck())
		m_slider_itemConcentratorRadius.EnableWindow(TRUE);
	else
		m_slider_itemConcentratorRadius.EnableWindow(FALSE);

	
}

void CConfItems::OnCheckItemconcentratorprob() 
{
	// TODO: Add your control notification handler code here
	m_slider_itemConcentratorProb.SetRange(0, MAXSLIDER);
	m_slider_itemConcentratorProb.SetPos(m_slider_itemConcentratorProb_value);
	if (m_check_itemConcentratorProb.GetCheck())
		m_slider_itemConcentratorProb.EnableWindow(TRUE);
	else
		m_slider_itemConcentratorProb.EnableWindow(FALSE);

	
}

void CConfItems::OnCheckRogueheatprob() 
{
	// TODO: Add your control notification handler code here
	m_slider_rogueHeatProb.SetRange(0, MAXSLIDER);
	m_slider_rogueHeatProb.SetPos(m_slider_rogueHeatProb_value);
	if (m_check_rogueHeatProb.GetCheck())
		m_slider_rogueHeatProb.EnableWindow(TRUE);
	else
		m_slider_rogueHeatProb.EnableWindow(FALSE);
	
}

void CConfItems::OnCheckRoguemineprob() 
{
	// TODO: Add your control notification handler code here
	m_slider_rogueMineProb.SetRange(0, MAXSLIDER);
	m_slider_rogueMineProb.SetPos(m_slider_rogueMineProb_value);
	if (m_check_rogueMineProb.GetCheck())
		m_slider_rogueMineProb.EnableWindow(TRUE);
	else
		m_slider_rogueMineProb.EnableWindow(FALSE);
	
}

/* $Id: ConfItems.h,v 1.5 1997/11/27 20:08:50 bert Exp $
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
*  ConfItems.h - Item Settings Property Page for the ConfigDialog			*
*																			*
\***************************************************************************/
#if !defined(AFX_CONFITEMS_H__14E795D4_DE23_11D0_93E4_006097A20515__INCLUDED_)
#define AFX_CONFITEMS_H__14E795D4_DE23_11D0_93E4_006097A20515__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CConfItems dialog

class CConfItems : public CPropertyPage
{
	DECLARE_DYNCREATE(CConfItems)

// Construction
public:
	CConfItems();
	~CConfItems();

// Dialog Data
	//{{AFX_DATA(CConfItems)
	enum { IDD = IDD_CONF_ITEMS };
	CEdit	m_edit_mineFuseTime;
	CSliderCtrl	m_slider_rogueMineProb;
	CSliderCtrl	m_slider_rogueHeatProb;
	CSliderCtrl	m_slider_itemConcentratorProb;
	CSliderCtrl	m_slider_itemConcentratorRadius;
	CSliderCtrl	m_slider_maxItemDensity;
	CSliderCtrl	m_slider_itemProbMult;
	CSliderCtrl	m_slider_destroyItemInCollisionProb;
	CSliderCtrl	m_slider_dropItemOnKillProb;
	CButton	m_check_rogueMineProb;
	CButton	m_check_rogueHeatProb;
	CButton	m_check_itemConcentratorProb;
	CButton	m_check_itemConcentratorRadius;
	CButton	m_check_maxItemDensity;
	CButton	m_check_itemProbMult;
	CButton	m_check_destroyItemInCollisionProb;
	CButton	m_check_dropItemOnKillProb;
	CButton	m_check_mineFuseTime;
	CButton	m_check_movingItemProb;
	CSliderCtrl	m_slider_movingItemProb;
	CSliderCtrl	m_slider_nukeClusterDamage;
	CButton	m_check_nukeClusterDamage;
	CEdit	m_edit_nukeMinMines;
	CEdit	m_edit_nukeMinSmarts;
	CEdit	m_edit_maxMissilesPerPack;
	CButton	m_check_nukeMinMines;
	CButton	m_check_nukeMinSmarts;
	CButton	m_check_maxMissilesPerPack;
	//}}AFX_DATA

	int		m_slider_nukeClusterDamage_value;
	int		m_slider_movingItemProb_value;
	int		m_slider_itemConcentratorProb_value;
	int		m_slider_itemConcentratorRadius_value;
	int		m_slider_dropItemOnKillProb_value;
	int		m_slider_destroyItemInCollisionProb_value;
	int		m_slider_itemProbMult_value;
	int		m_slider_maxItemDensity_value;
	int		m_slider_rogueHeatProb_value;
	int		m_slider_rogueMineProb_value;
// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CConfItems)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CConfItems)
	afx_msg void OnCheckMaxmissilesperpack();
	afx_msg void OnCheckNukeminsmarts();
	afx_msg void OnCheckNukeminmines();
	afx_msg void OnCheckNukeclusterdamage();
	afx_msg void OnCheckMovingitemprob();
	afx_msg void OnCheckMinefusetime();
	afx_msg void OnCheckDropitemonkillprob();
	afx_msg void OnCheckDestroyitemincollisionprob();
	afx_msg void OnCheckItemprobmult();
	afx_msg void OnCheckMaxitemdensity();
	afx_msg void OnCheckItemconcentratorradius();
	afx_msg void OnCheckItemconcentratorprob();
	afx_msg void OnCheckRogueheatprob();
	afx_msg void OnCheckRoguemineprob();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFITEMS_H__14E795D4_DE23_11D0_93E4_006097A20515__INCLUDED_)

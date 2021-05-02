/* $Id: SysInfo.cpp,v 1.5 1997/11/27 20:08:39 bert Exp $
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

// SysInfo.cpp : implementation file
//

#include "stdafx.h"
#include "XPilotNT.h"
#include "SysInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSysInfo dialog


CSysInfo::CSysInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CSysInfo::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSysInfo)
	m_string = _T("");
	//}}AFX_DATA_INIT
	SetupInfo();
}


void CSysInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSysInfo)
	DDX_Text(pDX, IDC_EDIT1, m_string);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSysInfo, CDialog)
	//{{AFX_MSG_MAP(CSysInfo)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSysInfo message handlers

extern "C"	void	GetLocalHostName(char *name, unsigned size);
extern "C"	HDC		WinXGetDrawDC();
extern "C"	int		WinXReleaseDrawDC(HDC hDC);

extern		WSADATA	wsadata;

void CSysInfo::SetupInfo()
{
	char	s[256];
	CString	cs;
	int		ret = 0;
	HDC		hDC = WinXGetDrawDC();

	GetLocalHostName(s, 256);
	m_string = "Hostname: <";
	if (ret)
		m_string += "Error getting hostname";
	else
		m_string += s;
	m_string += ">\r\n";

	cs.Format("Winsock: Version %d\r\n", wsadata.wVersion);
	m_string += cs;
	m_string +=	" Description: <";
	m_string += wsadata.szDescription;
	m_string += ">\r\n Status: <";
	m_string += wsadata.szSystemStatus;
	cs.Format(">\r\n MaxSockets: %d\r\n MaxUdp: %d\r\n", 
		wsadata.iMaxSockets, wsadata.iMaxUdpDg);
	m_string += cs;

	cs.Format("Display Driver: Version %d, RasterDisplay: %c\r\n", 
		GetDeviceCaps(hDC, DRIVERVERSION),
		GetDeviceCaps(hDC, DT_RASDISPLAY) ? 'Y' : 'N');
	m_string += cs;
	cs.Format(" Number of colors: %d, pens: %d, brushes: %d, fonts: %d\r\n",
		GetDeviceCaps(hDC, NUMCOLORS), GetDeviceCaps(hDC, NUMPENS),
		GetDeviceCaps(hDC, NUMBRUSHES), GetDeviceCaps(hDC, NUMFONTS));
	m_string += cs;
	cs.Format(" ColorRes: %d, SizePalette: %d, NumReserved: %d, BitsPixel: %d\r\n", 
		GetDeviceCaps(hDC, COLORRES), GetDeviceCaps(hDC, SIZEPALETTE),
		GetDeviceCaps(hDC, NUMRESERVED), GetDeviceCaps(hDC, BITSPIXEL));
	m_string += cs;
	cs.Format(" Supports: Palette: %c, BitBlt: %c, StretchBlt: %c, Scaling: %c\r\n",
		GetDeviceCaps(hDC, RC_PALETTE) ? 'Y' : 'N',
		GetDeviceCaps(hDC, RC_BITBLT) ? 'Y' : 'N',
		GetDeviceCaps(hDC, RC_STRETCHBLT) ? 'Y' : 'N',
		GetDeviceCaps(hDC, RC_SCALING) ? 'Y' : 'N');
	m_string += cs;
	cs.Format("  DIBitmap: %c, DibToDev: %c, StretchDib: %c Bitmap64K: %c\r\n",
		GetDeviceCaps(hDC, RC_DI_BITMAP) ? 'Y' : 'N',
		GetDeviceCaps(hDC, RC_DIBTODEV) ? 'Y' : 'N',
		GetDeviceCaps(hDC, RC_STRETCHDIB) ? 'Y' : 'N',
		GetDeviceCaps(hDC, RC_BITMAP64) ? 'Y' : 'N');
	m_string += cs;
	cs.Format("  Curves: %c, Circles: %c, Ellipses: %c, Pie: %c\r\n",
		GetDeviceCaps(hDC, CC_NONE) ? 'Y' : 'N',
		GetDeviceCaps(hDC, CC_CIRCLES) ? 'Y' : 'N',
		GetDeviceCaps(hDC, CC_ELLIPSES) ? 'Y' : 'N',
		GetDeviceCaps(hDC, CC_PIE) ? 'Y' : 'N');
	m_string += cs;
	cs.Format("  Lines: %c, Wide: %c, Styled: %c WideStyled: %c\r\n",
		GetDeviceCaps(hDC, LC_NONE) ? 'Y' : 'N',
		GetDeviceCaps(hDC, LC_WIDE) ? 'Y' : 'N',
		GetDeviceCaps(hDC, LC_STYLED) ? 'Y' : 'N',
		GetDeviceCaps(hDC, LC_WIDESTYLED) ? 'Y' : 'N');
	m_string += cs;

	WinXReleaseDrawDC(hDC);
}

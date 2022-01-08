/* $Id: UrlWidget.cpp,v 5.1 2001/05/19 10:11:58 bertg Exp $
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
*  UrlWidget.cpp - Display an Url (blue underlined)							*
*																			*
\***************************************************************************/

#include "stdafx.h"
#include "xpilots.h"
#include "UrlWidget.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// UrlWidget

UrlWidget::UrlWidget()
{
	cursor = NULL;

}

UrlWidget::~UrlWidget()
{
}


BEGIN_MESSAGE_MAP(UrlWidget, CStatic)
	//{{AFX_MSG_MAP(UrlWidget)
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
// UrlWidget message handlers

BOOL UrlWidget::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default
	if (cursor == NULL)		// No cursor handle - load our own
	{
        // Get the windows directory
		CString strWndDir;
		GetWindowsDirectory(strWndDir.GetBuffer(MAX_PATH), MAX_PATH);
		strWndDir.ReleaseBuffer();

		strWndDir += _T("\\winhlp32.exe");
		// This retrieves cursor #106 from winhlp32.exe, which is a hand pointer
		HMODULE hModule = LoadLibrary(strWndDir);
		if (hModule)
		{
			HCURSOR hHandCursor = ::LoadCursor(hModule, MAKEINTRESOURCE(106));
			if (hHandCursor)
			{
				cursor = CopyCursor(hHandCursor);
				SetCursor(cursor);
			}
		}
		FreeLibrary(hModule);
	}
	
	if (cursor)
	{
		::SetCursor(cursor);
		return TRUE;
	}
	return FALSE;
	
	return CStatic::OnSetCursor(pWnd, nHitTest, message);
}

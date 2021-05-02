/* $Id: winClient.h,v 5.0 2001/04/07 20:00:59 dik Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
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
*  winClient.h - Interface XPilot to the MFC frontend						*
*																			*
*  interface from the MFC poo to the xpilot "C" source.						*
*																			*
*  $Id: winClient.h,v 5.0 2001/04/07 20:00:59 dik Exp $						*
\***************************************************************************/

#ifndef	_INC_WINDOWS
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern	void		xpilotShutdown();
extern	int			main(int argc, char** argv);
extern	int			Net_fd(void);
extern	int			Net_input(void);
extern	void		Paint_frame(void);
extern	void		PaintWinClient();
extern	HINSTANCE	hInstance;
extern	void		Client_cleanup();

extern	void		InitWinX(HWND m_hWnd);
extern	void		WinXShutdown();

extern	int			received_self;

extern	const char*	winHelpFile;	/* used to determine .ini file locations */

/* requests from the xpilot "real code" to mfc */
extern	void	_Trace(char* lpszFormat, ...);
extern	const char* mfcDoTalkWindow();
extern	char*	Get_xpilotini_file(int level);

#ifdef	_DEBUG
extern	void	xpmemShutdown();
#endif

extern	void	Progress(const char* s, ...);

#ifdef __cplusplus
};
#endif

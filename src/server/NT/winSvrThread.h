/* $Id: winSvrThread.h,v 4.2 2001/02/19 23:36:49 bert Exp $
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
*  winSvrThread.h - The worker thread for the XPilot server on NT			*
*																			*
*  $Id: winSvrThread.h,v 4.2 2001/02/19 23:36:49 bert Exp $					*
\***************************************************************************/
#ifndef	_WINSVRTHREAD_H_
#define	_WINSVRTHREAD_H_

typedef	struct	CServerThreadInfo CServerThreadInfo;

struct CServerThreadInfo
{
//	HWND m_hwndNotifyRecalcDone;
	HANDLE m_hEventStartGame;
	HANDLE m_hEventGameTerminated;
	HANDLE m_hEventKillServerThread;
	HANDLE m_hEventServerThreadKilled;
	HWND m_hwndNotifyProgress;
	BOOL	m_hostNameFound;

	int		argc;
	char*	argv[256];
};

#define	WM_MSGAVAILABLE		(WM_APP+1)
#define	WM_UPDATESCORES		(WM_APP+2)
#define	WM_STARTTIMER		(WM_APP+3)


// Controlling function for the worker thread.
#ifdef	__cplusplus
extern "C" {
#endif
UINT ServerThreadProc(LPVOID pParam /* CServerThreadInfo ptr */);
void CALLBACK ServerThreadTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);
//TIMERPROC	ServerThreadTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);
#ifdef	__cplusplus
}
#endif
#endif	// _WINSVRTHREAD_H_

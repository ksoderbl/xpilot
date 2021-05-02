/* $Id: winSvrThread.c,v 5.0 2001/04/07 20:01:01 dik Exp $
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
*  winSvrThread.c - The worker thread for the XPilot server on NT			*
*																			*
*  $Id: winSvrThread.c,v 5.0 2001/04/07 20:01:01 dik Exp $					*
\***************************************************************************/

/* Entry point for Windows Server Thread */
#include "winServer.h"
#include "WinSvrThread.h"
#include "../sched.h"
#include "../../common/config.h"
#include <stdio.h>
#include <stdarg.h>
#include "../../common/NT/winNet.h"

int		MsgAvail0 = FALSE;
int		MsgAvail1 = FALSE;
char	Msg0[512];
char	Msg1[512];
CServerThreadInfo*	pServerInfo;
int		ServerKilled;
int		winTimer = 0;				/* frames per second for Windows main loop to use */
//HWND	notifyWnd;					/* Our parent's window */

extern void Main_loop(void);
extern void End_game(void);

extern long	timer_freq;	/* rate at which timer ticks. (in FPS) */
extern volatile long	timer_ticks;	/* SIGALRMs that have occurred */
extern int		sched_running;
extern int		framesPerSecond;

#if 0
// just a stupid debug function to replace "main()"
int crapmain(int argc, char* argv[])
{
	while (1)
	{
		if (WaitForSingleObject(pServerInfo->m_hEventKillServerThread, 0)
			== WAIT_OBJECT_0)
		{
			ServerKilled = TRUE;
//			End_game();
			return(0);
		}
	}
}
#endif

UINT ServerThreadProc(LPVOID pParam)
{
	/*
	 * Purify stomps on the command line arguments that i pass
	 * (It complains about reading past the end of the stack and then
	 *  *WHOMPS* the data)
	 * I don't know if this is a real problem or just Purify's inability
	 * to deal with data coming across from a different thread.
	 * So i clone them to be local.  Purify still complains, but at least
	 * it doesn't kill my data.
	 * 
	 */
	char**	zargv;
	int	zargc;
	int	i;

	pServerInfo = (CServerThreadInfo*)pParam;
	ServerKilled = FALSE;		// if the user kills this thread
								// then we need to take a different exit

//	UINT cr;


	while (TRUE)
	{

		// Wait until the main application thread asks this thread to do
		//      something
		if (WaitForSingleObject(pServerInfo->m_hEventStartGame, INFINITE)
			!= WAIT_OBJECT_0)
			break;

		zargc = pServerInfo->argc;
		zargv = (char **) malloc((zargc + 1) *sizeof(char*));
		for (i=0; i<zargc; i++)
		{
			zargv[i] = malloc(strlen(pServerInfo->argv[i])+1);
			strcpy(zargv[i], pServerInfo->argv[i]);
		}
		zargv[i] = NULL;

		// need to free zargv, add later.

		notifyWnd = pServerInfo->m_hwndNotifyProgress;
		// force the system to create a message queue for this thread
		{ 
			MSG msg;
			PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
		}
		// Exit the thread if the main application sets the "kill server"
		// event. The main application will set the "start game" event
		// before setting the "kill server" event.

		if (WaitForSingleObject(pServerInfo->m_hEventKillServerThread, 0)
			== WAIT_OBJECT_0)
			break; // Terminate this thread by existing the proc.

		// Reset event to indicate "not done", that is, game is in progress.
		ResetEvent(pServerInfo->m_hEventGameTerminated);
		if (!main(zargc, zargv))
			return(0);
		SendMessage(pServerInfo->m_hwndNotifyProgress, WM_STARTTIMER, 0, (LPARAM)framesPerSecond);

	    sched_running = 1;
		while (TRUE)
		{
			sched();
			if (WaitForSingleObject(pServerInfo->m_hEventKillServerThread, 0) == WAIT_OBJECT_0)
			{
				ServerKilled = TRUE;
				break; // Terminate this thread by existing the proc.
			}
		}
		if (ServerKilled)		// If interrupted by kill then...
			break;				// terminate this thread by exiting the proc.

	}
	stop_sched();				// no more timer ticks please
	End_game();
	SetEvent(pServerInfo->m_hEventServerThreadKilled);

	return 0;
}
void CALLBACK ServerThreadTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	static int timer_flag = 0;

	if (timer_flag)
	{
		xpprintf("Overlapping Timer!\n");
	}
	timer_flag++;
	if (!sched_running)
		return;
	timer_ticks++;
	Main_loop();
	timer_flag--;
}

/* to get a printf from the worker/server thread into the main window
	i maintain 2 buffers.  After a call to xpprintfW, i post a message to the main
	window, telling him there's a buffer ready.
*/
void xpprintfW(const char *fmt, ...)
{
    va_list	 ap;			/* Argument pointer */
    int		 e = errno;		/* Store errno */
//	char	s[512];
	char*	s;
	char*	t;
	char	u[512];
	char*	w;
	int		susing;

//	if (!MsgAvail0)
	if (1)
	{
		s = Msg0;
		susing = 0;
		MsgAvail0 = TRUE;
	}
	else if (!MsgAvail1)
	{
		s = Msg1;
		susing = 1;
		MsgAvail1 = TRUE;
	}
	else
	{
	    va_start(ap, fmt);
	    vsprintf(u, fmt, ap);
		_Trace("Error: no message space: %s\n", u);
 		if (MessageBox(NULL, u, "Error No message space", MB_OKCANCEL | MB_TASKMODAL) == IDCANCEL)
		{
			#ifdef	_DEBUG
//				xpmemShutdown();
			#endif
			ExitProcess(1);
		}
	    va_end(ap);
		return;
	}
    va_start(ap, fmt);
    vsprintf(u, fmt, ap);
	_Trace("Message: %s", s);
	/* the stupid edit control (text window) needs \r\n to function within reason */
	t = s;
	w = u;
	while (*w)
	{
		if (*w == '\n')
			*t++ = '\r';
		*t++ = *w++;
	}
	*t = '\0';
	/* I ended up using SendMessage (which waits till the message is processed)
	   instead of PostMessage (which justs puts it in the queue).
	   I guess i should have a nice dynamic array
	*/
	SendMessage(pServerInfo->m_hwndNotifyProgress, WM_MSGAVAILABLE, susing, (LPARAM)s);
	
    va_end(ap);



}

void SendDialogUpdate(void)
{
	SendMessage(pServerInfo->m_hwndNotifyProgress, WM_UPDATESCORES, 0, 0);
}

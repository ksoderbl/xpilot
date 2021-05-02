/* $Id: winServer.h,v 4.2 2001/02/19 23:36:49 bert Exp $
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
*  winServer.h - interface from the MFC poo to the xpilot "C" source		*
*																			*
\***************************************************************************/

#ifndef	_INC_WINDOWS
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern	int			server_running;
extern	int			main(int argc, char** argv);

extern	void		SendDialogUpdate(void);


/* requests from the xpilot "real code" to mfc */
extern	void	_Trace(char* lpszFormat, ...);

/* capture printf and route it to the window in the main thread */
extern	void	xpprintfW(const char* lpszFormat, ...);
#ifdef __cplusplus
};
#endif

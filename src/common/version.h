/* 
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

#ifndef	VERSION_H
#define	VERSION_H

#if defined(__hpux)
#   pragma COPYRIGHT_DATE	"1991-2001"
#   pragma COPYRIGHT		"Bjørn Stabell, Ken Ronny Schouten, Bert Gijsbers & Dick Balaska"
#   pragma VERSIONID		"XPilot 4.5.2"
#endif

#define VERSION			"4.5.2"
#ifdef	_WINDOWS
#define	TITLE			"4.5.2-NT13"
#define	VERSION_WINDOWS	"13"
#else
#define TITLE			"XPilot 4.5.2"
#endif
#define AUTHORS			"Bjørn Stabell, Ken Ronny Schouten, Bert Gijsbers & Dick Balaska"
#define COPYRIGHT		"Copyright © 1991-2001 by Bjørn Stabell, Ken Ronny Schouten, Bert Gijsbers & Dick Balaska"

#endif /* VERSION_H */

/* $Id: winConfig.h,v 5.1 2001/05/19 10:11:58 bertg Exp $
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
*  winConfig.h - Resources, defaults and stored configuration.
* 
*  $Id: winConfig.h,v 5.1 2001/05/19 10:11:58 bertg Exp $	
\***************************************************************************/

#ifndef WIN_CONFIG_H
#define WIN_CONFIG_H

void Config_get_name(char *name);
void Config_get_team(int *my_team);
void Config_save_resource(FILE *fp, const char *resource, char *value);
void Config_save_window_positions(void);
void Config_get_profile_string(
	const char* key,
	const char* def,
	char* result,
	int size);
void Config_get_resource(
	const char *resource,
	char *result,
	unsigned size,
	int* index);
char *Get_xpilotini_file(int level);

#endif

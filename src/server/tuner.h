/*
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

#ifndef	TUNER_H
#define	TUNER_H

void tuner_plock(void);
void tuner_shotsmax(void);
void tuner_shipmass(void);
void tuner_ballmass(void);
void tuner_maxrobots(void);
void tuner_minrobots(void);
void tuner_playershielding(void);
void tuner_playerstartsshielded(void);
void tuner_worldlives(void);
void tuner_cannonsmartness(void);
void tuner_teamcannons(void);
void tuner_cannonsuseitems(void);
void tuner_wormtime(void);
void tuner_modifiers(void);
void tuner_minelife(void);
void tuner_missilelife(void);
void tuner_gameduration(void);
void tuner_racelaps(void);
void tuner_allowalliances(void);
void tuner_announcealliances(void);

#endif

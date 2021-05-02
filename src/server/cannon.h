/* $Id: cannon.h,v 1.3 2001/03/16 22:42:58 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *  	Guido Koopman        <guido@xpilot.org>
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
 
#ifndef CANNON_H
#define CANNON_H

extern long CANNON_USE_ITEM;

/* the different weapons a cannon can use.
   used in communication between parts of firing code */
/* plain old bullet. more if wideangles or rearshots are available.
   always available */
#define CW_SHOT		0
/* dropped or thrown. uses one mine */
#define CW_MINE		1
/* torpedo, heatseeker or smartmissile. uses one missile */
#define CW_MISSILE	2
/* blinding, stun or normal laser. needs a laser */
#define CW_LASER	3
/* uses one ECM */
#define CW_ECM		4
/* tractor or pressor beam. needs a tractorbeam */
#define CW_TRACTORBEAM	5
/* uses one transporter */
#define CW_TRANSPORTER	6
/* a big stream of exhaust particles (OBJ_SPARK). needs an afterburner and
   uses one fuel pack. even bigger with emergency thrust. more afterburners
   only increase probability of use */
#define CW_GASJET	7

/* base visibility distance (modified by sensors) */
#define CANNON_DISTANCE		(VISIBILITY_DISTANCE * 0.5)

/* chance of throwing an item upon death (multiplied by dropItemOnKillProb) */
#define CANNON_DROP_ITEM_PROB	0.7

/* speed used for missiles and shots. mines are thrown at half this speed */
#define CANNON_SHOT_SPEED	ShotsSpeed
#define CANNON_SHOT_MASS	0.4
#define CANNON_MINE_MASS	(MINE_MASS * 0.6)
/* lifetime in ticks (frames) of shots, missiles and mines */
#define CANNON_SHOT_LIFE	(25 + (randomMT() & 0x17))
/* maximum lifetime (only used in aiming) */
#define CANNON_SHOT_LIFE_MAX	(25 + 0x17)
/* number of laser pulses used in calculation of pulse lifetime */
#define CANNON_PULSES		1

/* sector in which cannonfire is possible */
#define CANNON_SPREAD		(RES / 3)

#endif

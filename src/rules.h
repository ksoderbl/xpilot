/* $Id: rules.h,v 3.26 1995/01/11 19:56:07 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
 *
 *      Bjørn Stabell        (bjoerns@staff.cs.uit.no)
 *      Ken Ronny Schouten   (kenrsc@stud.cs.uit.no)
 *      Bert Gÿsbers         (bert@mc.bio.uva.nl)
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

#ifndef RULES_H
#define RULES_H

/*
 * Bitfield definitions for playing mode.
 */
#define CRASH_WITH_PLAYER	(1<<0)
#define BOUNCE_WITH_PLAYER	(1<<1)
#define PLAYER_KILLINGS		(1<<2)
#define LIMITED_LIVES		(1<<3)
#define TIMING			(1<<4)
#define ONE_PLAYER_ONLY		(1<<5)
#define PLAYER_SHIELDING	(1<<6)
#define LIMITED_VISIBILITY	(1<<7)
#define TEAM_PLAY		(1<<8)
#define WRAP_PLAY		(1<<9)
#define ALLOW_NUKES		(1<<10)
#define ALLOW_CLUSTERS		(1<<11)
#define ALLOW_MODIFIERS		(1<<12)
#define ALLOW_LASER_MODIFIERS	(1<<13)
/*
 * Client uses only a subset of them:
 */
#define CLIENT_RULES_MASK	(WRAP_PLAY|TEAM_PLAY|TIMING|LIMITED_LIVES)

/*
 * Possible object and player status bits.
 * Needed here because client needs them too.
 * The bits that the client needs must fit into a byte,
 * so the first 8 bitvalues are reserved for that purpose.
 */
#define PLAYING			(1L<<0)		/* Not returning to base */
#define PAUSE			(1L<<1) 	/* Must stay below 8 */
#define GAME_OVER		(1L<<2)		/* Must stay below 8 */
#define THRUSTING		(1L<<3)		/* not used by client? */
#define SELF_DESTRUCT		(1L<<4)		/* not used by client? */
#define WANT_AUDIO		(1L<<5)		/* whether client has SOUND */

#define KILLED			(1L<<10)
#define GRAVITY			(1L<<11)
#define WARPING			(1L<<12)
#define WARPED			(1L<<13)
#define CONFUSED		(1L<<14)
#define FROMCANNON		(1L<<15)	/* Object from cannon */
#define HOVERPAUSE		(1L<<16)	/* Hovering pause */
#define RECREATE		(1L<<17)	/* Recreate ball */
#define FROMBOUNCE		(1L<<18)	/* Spark from wall bounce */
#define OWNERIMMUNE		(1L<<19)	/* Owner is immune to object */
#define REPROGRAM		(1L<<20)	/* Player reprogramming */
#define NOEXPLOSION		(1L<<21)	/* No ball recreate explosion */
#define COLLISIONSHOVE		(1L<<22)	/* Collision counts as shove */
#define FINISH			(1L<<23)	/* Reached race finish */
#define RACE_OVER		(1L<<24)	/* After finished and score. */

typedef struct {
    int		lives;
    long	mode;
} rules_t;

#endif

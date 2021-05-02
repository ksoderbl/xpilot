/* $Id: map.h,v 3.24 1995/03/19 15:33:31 bert Exp $
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

#ifndef	MAP_H
#define	MAP_H

#ifndef TYPES_H
/* need position */
#include "types.h"
#endif
#ifndef RULES_H
/* need rules_t */
#include "rules.h"
#endif
#ifndef ITEM_H
/* need NUM_ITEMS */
#include "item.h"
#endif

#define SPACE			0
#define BASE			1
#define FILLED			2
#define REC_LU			3
#define REC_LD			4
#define REC_RU			5
#define REC_RD			6
#define FUEL			7
#define CANNON			8
#define CHECK			9
#define POS_GRAV		10
#define NEG_GRAV		11
#define CWISE_GRAV		12
#define ACWISE_GRAV		13
#define WORMHOLE		14
#define TREASURE		15
#define TARGET			16
#define ITEM_CONCENTRATOR	17
#define DECOR_FILLED		18
#define DECOR_LU		19
#define DECOR_LD		20
#define DECOR_RU		21
#define DECOR_RD		22
#define BASE_ATTRACTOR		127

#define SPACE_BIT		(1 << SPACE)
#define BASE_BIT		(1 << BASE)
#define FILLED_BIT		(1 << FILLED)
#define REC_LU_BIT		(1 << REC_LU)
#define REC_LD_BIT		(1 << REC_LD)
#define REC_RU_BIT		(1 << REC_RU)
#define REC_RD_BIT		(1 << REC_RD)
#define FUEL_BIT		(1 << FUEL)
#define CANNON_BIT		(1 << CANNON)
#define CHECK_BIT		(1 << CHECK)
#define POS_GRAV_BIT		(1 << POS_GRAV)
#define NEG_GRAV_BIT		(1 << NEG_GRAV)
#define CWISE_GRAV_BIT		(1 << CWISE_GRAV)
#define ACWISE_GRAV_BIT		(1 << ACWISE_GRAV)
#define WORMHOLE_BIT		(1 << WORMHOLE)
#define TREASURE_BIT		(1 << TREASURE)
#define TARGET_BIT		(1 << TARGET)
#define ITEM_CONCENTRATOR_BIT	(1 << ITEM_CONCENTRATOR)
#define DECOR_FILLED_BIT	(1 << DECOR_FILLED)
#define DECOR_LU_BIT		(1 << DECOR_LU)
#define DECOR_LD_BIT		(1 << DECOR_LD)
#define DECOR_RU_BIT		(1 << DECOR_RU)
#define DECOR_RD_BIT		(1 << DECOR_RD)

#define DIR_RIGHT		0
#define DIR_UP			(RES/4)
#define DIR_LEFT		(RES/2)
#define DIR_DOWN		(3*RES/4)

typedef struct {
    ipos	blk_pos;
    position	pix_pos;
    long	fuel;
    unsigned	conn_mask;
    long	last_change;
} fuel_t;

typedef struct {
    ipos	pos;
    float	force;
} grav_t;

typedef struct {
    ipos	pos;
    int		dir;
    u_short	team;
} base_t;

typedef struct {
    int		base_idx;	/* Index in World.base[] */
    float	dist;		/* Distance to first checkpoint */
} baseorder_t;

typedef struct {
    ipos	pos;
    int		dir;
    int		dead_time;
    bool	active;
    unsigned	conn_mask;
    long	last_change;
} cannon_t;

typedef struct {
    float	prob;		/* Probility [0..1] for item to appear */
    int		max;		/* Max on world at a given time */
    int		num;		/* Number active right now */
    int		chance;		/* Chance [0..127] for this item to appear */
    int		min_per_pack;	/* minimum number of elements per item. */
    int		max_per_pack;	/* maximum number of elements per item. */
    int		initial;	/* initial number of elements per player. */
} item_t;

typedef enum { WORM_NORMAL, WORM_IN, WORM_OUT } wormType;

typedef struct {
    ipos	pos;
    int		lastdest, countdown, lastplayer;
    wormType	type;
} wormhole_t;

typedef struct {
    ipos	pos;
    bool	have;		/* true if this treasure has ball in it */
    u_short	team;		/* team of this treasure */
    int 	destroyed;	/* number of times this treasure destroyed */
} treasure_t;

typedef struct {
    ipos	pos;
    u_short	team;
    int		dead_time;
    int		damage;
    unsigned	conn_mask;
    unsigned 	update_mask;
    long	last_change;
} target_t;

typedef struct {
    int		NumMembers;		/* Number of current members */
    int		NumBases;		/* Number of bases owned */
    int		NumTreasures;		/* Number of treasures owned */
    int		TreasuresDestroyed;	/* Number of destroyed treasures */
    int		TreasuresLeft;		/* Number of treasures left */
} team_t;

typedef struct {
    ipos	pos;
} item_concentrator_t;

typedef struct {
    int		x, y;		/* Size of world in blocks */
    int		diagonal;	/* Diagonal length in blocks */
    int		width, height;	/* Size of world in pixels (optimization) */
    int		hypotenuse;	/* Diagonal length in pixels (optimization) */
    rules_t	*rules;
    char	name[MAX_CHARS];
    char	author[MAX_CHARS];

    u_byte	**block;
    vector	**gravity;

    item_t	items[NUM_ITEMS];

    team_t	teams[MAX_TEAMS];

    int		NumTeamBases;      /* How many 'different' teams are allowed */
    int		NumBases;
    base_t	*base;
    baseorder_t	*baseorder;
    int		NumFuels;
    fuel_t	*fuel;
    int		NumGravs;
    grav_t	*grav;
    int		NumCannons;
    cannon_t	*cannon;
    int		NumChecks;
    ipos	check[MAX_CHECKS];
    int		NumWormholes;
    wormhole_t	*wormHoles;
    int		NumTreasures;
    treasure_t	*treasures;
    int         NumTargets;
    target_t    *targets;
    int		NumItemConcentrators;
    item_concentrator_t	*itemConcentrators;
    long	nextEvent;
} World_map;

#endif

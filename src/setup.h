/* $Id: setup.h,v 3.5 1993/08/02 12:41:39 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef SETUP_H
#define SETUP_H

#include "const.h"

/*
 * Definitions to tell the client how the server has been setup.
 */

/*
 * If the high bit of a map block is set then the next block holds
 * the number of contiguous map blocks that have the same block type.
 */
#define SETUP_COMPRESSED	0x80

/*
 * Tell the client how and if the map is compressed.
 */
#define SETUP_MAP_ORDER_XY	1
#define SETUP_MAP_ORDER_YX	2
#define SETUP_MAP_UNCOMPRESSED	3

/*
 * Definitions for the map layout which permit a compact definition
 * of map data.
 */
#define SETUP_SPACE		0
#define SETUP_FILLED		1
#define SETUP_FILLED_NO_DRAW	2
#define SETUP_FUEL		3
#define SETUP_REC_RU		4
#define SETUP_REC_RD		5
#define SETUP_REC_LU		6
#define SETUP_REC_LD		7
#define SETUP_ACWISE_GRAV	8
#define SETUP_CWISE_GRAV	9
#define SETUP_POS_GRAV		10
#define SETUP_NEG_GRAV		11
#define SETUP_WORM_NORMAL	12
#define SETUP_WORM_IN		13
#define SETUP_WORM_OUT		14
#define SETUP_CANNON_UP		15
#define SETUP_CANNON_RIGHT	16
#define SETUP_CANNON_DOWN	17
#define SETUP_CANNON_LEFT	18
#define SETUP_TREASURE		20	/* + team number */
#define SETUP_BASE_LOWEST	30	/* lowest base number */
#define SETUP_BASE_UP		30	/* + team number */
#define SETUP_BASE_RIGHT	40	/* + team number */
#define SETUP_BASE_DOWN		50	/* + team number */
#define SETUP_BASE_LEFT		60	/* + team number */
#define SETUP_BASE_HIGHEST	69	/* highest base number */
#define SETUP_TARGET		70	/* + team number */
#define SETUP_CHECK		80	/* + check point number */

/*
 * Structure defining the server configuration, including the map layout.
 */
typedef struct {
    long		setup_size;		/* size including map data */
    long		map_data_len;		/* num. compressed map bytes */
    long		mode;			/* playing mode */
    short		lives;			/* max. number of lives */
    short		x;			/* width */
    short		y;			/* height */
    short		frames_per_second;	/* FPS */
    short		map_order;		/* row major or col major */
    char		name[MAX_CHARS];	/* name of map */
    char		author[MAX_CHARS];	/* name of author of map */
    unsigned char	map_data[4];		/* compressed map data */
    /* plus more mapdata here (HACK) */
} setup_t;

#ifndef NETSERVER_C
# ifdef FPS
#  error	/* FPS needs a different definition in the client */
# else
#  define FPS		(Setup->frames_per_second)
# endif

extern setup_t *Setup;

#endif

#endif


/* $Id: stratbot.c,v 1.1 2002/01/07 20:48:07 bertg Exp $
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

/*
 * The strategy robot.
 *
 * A robot type designed with the idea that most calculations
 * about what a robot should do don't need to be done every frame.
 * Instead we want a robot which thinks as little as possible, but
 * when it does think it should be thinking as cleverly as possible.
 * Typically at each frame it should only check if it is still moving
 * to where it wants to go and check if it needs to raise it shields or not.
 */

#if defined(DEVELOPMENT) || defined(ALLBOTS)

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <limits.h>

#ifdef _WINDOWS
# include "NT/winServer.h"
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "score.h"
#include "bit.h"
#include "saudio.h"
#include "netserver.h"
#include "pack.h"
#include "robot.h"
#include "error.h"
#include "portability.h"
#include "commonproto.h"


char stratbot_version[] = VERSION;


/*
 * Map objects a robot can fly through without damage.
 */
#define EMPTY_SPACE(s)	BIT(1 << (s), SPACE_BLOCKS)


/*
 * Structure which contains our own per robot instance data.
 */
typedef struct stratbot_data {
    int		not_really_used_yet;
} stratbot_data_t;


/*
 * Prototypes for methods of the stratbot type.
 */
static void Stratbot_round_tick(void);
static void Stratbot_create(int ind, char *str);
static void Stratbot_go_home(int ind);
static void Stratbot_play(int ind);
static void Stratbot_set_war(int ind, int victim_id);
static int Stratbot_war_on_player(int ind);
static void Stratbot_message(int ind, const char *str);
static void Stratbot_destroy(int ind);
static void Stratbot_invite(int ind, int inv_ind);
       int Stratbot_setup(robot_type_t *type_ptr);


/*
 * The robot type structure for the default robot.
 */
static robot_type_t stratbot_type = {
    "stratbot",
    Stratbot_round_tick,
    Stratbot_create,
    Stratbot_go_home,
    Stratbot_play,
    Stratbot_set_war,
    Stratbot_war_on_player,
    Stratbot_message,
    Stratbot_destroy,
    Stratbot_invite
};


/*
 * The only thing we export from this file.
 * A function to initialize the robot type structure
 * with our name and the pointers to our action routines.
 *
 * Return 0 if all is OK, anything else will ignore this
 * robot type forever.
 */
int Stratbot_setup(robot_type_t *type_ptr)
{
    /* Not much to do except init the type structure. */

    *type_ptr = stratbot_type;

    return 0;
}


/*
 * Function to cast from player structure to robot data structure.
 * This isolates casts (aka. type violations) to a few places.
 */
#if 0
static stratbot_data_t *Stratbot_get_data(player *pl)
{
    return (stratbot_data_t *)pl->robot_data_ptr->private_data;
}
#endif


/*
 * This is called each round.
 * It allows us to adjust our file local parameters.
 */
static void Stratbot_round_tick(void)
{
}


/*
 * A stratbot is created.
 */
static void Stratbot_create(int ind, char *str)
{
    player			*pl = Players[ind];
    stratbot_data_t		*my_data;

    if (!(my_data = (stratbot_data_t *)malloc(sizeof(*my_data)))) {
	error("no mem for stratbot robot");
	End_game();
    }

    pl->robot_data_ptr->private_data = (void *)my_data;
}

/*
 * A stratbot is placed on its homebase.
 */
static void Stratbot_go_home(int ind)
{
#if 0
    player			*pl = Players[ind];
    stratbot_data_t		*my_data = Stratbot_get_data(pl);
#endif

}


/*
 * A stratbot plays this frame.
 */
static void Stratbot_play(int ind)
{
}


/*
 * A stratbot is declaring war (or resetting war).
 */
static void Stratbot_set_war(int ind, int victim_id)
{
#if 0
    player			*pl = Players[ind];
    stratbot_data_t		*my_data = Stratbot_get_data(pl);
#endif

    if (victim_id == NO_ID) {
    } else {
    }
}

/*
 * Return the id of the player a stratbot has war against (or NO_ID).
 */
static int Stratbot_war_on_player(int ind)
{
#if 0
    player			*pl = Players[ind];
    stratbot_data_t		*my_data = Stratbot_get_data(pl);
#endif

    return NO_ID;
}

/*
 * A stratbot receives a message.
 */
static void Stratbot_message(int ind, const char *message)
{
#if 0
    player			*pl = Players[ind];
    stratbot_data_t		*my_data = Stratbot_get_data(pl);
    int				len;
    char			*ptr;
    char			sender_name[MAX_NAME_LEN];
    char			msg[MSG_LEN];

    /*
     * Extract the message body and the sender name from the message string.
     */

    ptr = strrchr(message, ']');/* get end of message */
    if (!ptr) {
	return;		/* Make sure to ignore server messages, oops! */
    }
    while (*--ptr != '[');	/* skip back over receiver name */
    while (*--ptr != '[');	/* skip back over sender name */
    strlcpy(sender_name, ptr + 1, sizeof sender_name);	/* copy sender */
    len = ptr - message;	/* find read message length */
    if (message[len] == ' ') {	/* ignore the readability space */
	len--;
    }
    strlcpy(msg, message, len);	/* copy real message */
    if ((ptr = strchr(sender_name, ']')) != NULL) {
	*ptr = '\0';	/* remove the ']' separator */
    }
    printf("%s got message \"%s\" from \"%s\"\n", pl->name, msg, sender_name);
#endif
}

/*
 * A stratbot is destroyed.
 */
static void Stratbot_destroy(int ind)
{
    player			*pl = Players[ind];

    free(pl->robot_data_ptr->private_data);
    pl->robot_data_ptr->private_data = NULL;
}

/*
 * A stratbot is asked to join an alliance
 */
static void Stratbot_invite(int ind, int inv_ind)
{
    Refuse_alliance(ind, inv_ind);
}


#if 0
static bool Really_empty_space(int ind, int x, int y)
{
    player	*pl = Players[ind];
    int		type = World.block[x][y];

    if (EMPTY_SPACE(type))
	return true;
    switch (type) {
    case FILLED:
    case REC_LU:
    case REC_LD:
    case REC_RU:
    case REC_RD:
    case FUEL:
    case TREASURE:
	return false;

    case WORMHOLE:
	if (!wormholeVisible
	    || World.wormHoles[World.itemID[x][y]].type == WORM_OUT) {
	    return true;
	} else {
	    return false;
	}

    case TARGET:
	if (!targetTeamCollision
	    && BIT(World.rules->mode, TEAM_PLAY)
	    && World.targets[World.itemID[x][y]].team == pl->team) {
	    return true;
	} else {
	    return false;
	}

    case CANNON:
	if (teamImmunity
	    && BIT(World.rules->mode, TEAM_PLAY)
	    && World.cannon[World.itemID[x][y]].team == pl->team) {
	    return true;
	} else {
	    return false;
	}

    default:
	break;
    }
    return false;
}
#endif

#endif


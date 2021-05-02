/* $Id: alliance.c,v 1.12 2001/12/05 16:11:23 kimiko Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *      Kimiko Koopman        <kimiko@xpilot.org>
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SERVER
#include "version.h"
#include "config.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "bit.h"
#include "netserver.h"
#include "error.h"
#include "commonproto.h"


char alliance_version[] = VERSION;


/*
 * Alliance information.
 */
typedef struct {
    int		id;		/* the ID of this alliance */
    int		NumMembers;	/* the number of members in this alliance */
} alliance_t;


static alliance_t	*Alliances[MAX_TEAMS];


static int New_alliance_ID(void);
static void Alliance_add_player(alliance_t *alliance, player *pl);
static int Alliance_remove_player(alliance_t *alliance, player *pl);
static void Set_alliance_message(alliance_t *alliance, const char *msg);
static int Create_alliance(int ind1, int ind2);
static void Dissolve_alliance(int id);
static void Merge_alliances(int id1, int id2);


int Invite_player(int ind, int ally_ind)
{
    player	*pl = Players[ind],
		*ally = Players[ally_ind];

    if (ind == ally_ind) {
	/* we can never form an alliance with ourselves */
	return 0;
    }
    if (IS_TANK_IND(ally_ind)) {
	/* tanks can't handle invitations */
	return 0;
    }
    if (ALLIANCE(ind, ally_ind)) {
	/* we're already in the same alliance */
	return 0;
    }
    if (pl->invite == ally->id) {
	/* player has already been invited by us */
	return 0;
    }
    if (ally->invite == pl->id) {
	/* player has already invited us. accept invitation */
	Accept_alliance(ind, ally_ind);
	return 1;
    }
    if (pl->invite != NO_ID) {
	/* we have already invited another player. cancel that invitation */
	Cancel_invitation(ind);
    }
    /* set & send invitation */
    pl->invite = ally->id;
    if (IS_ROBOT_PTR(ally)) {
	Robot_invite(ally_ind, ind);
    }
    else if (IS_HUMAN_PTR(ally)) {
	char msg[MSG_LEN];
	sprintf(msg, " < %s seeks an alliance with you >", pl->name);
	Set_player_message(ally, msg);
    }
    return 1;
}

int Cancel_invitation(int ind)
{
    player	*pl = Players[ind],
		*ally;

    if (pl->invite == NO_ID) {
	/* we have not invited anyone */
	return 0;
    }
    ally = Players[GetInd[pl->invite]];
    pl->invite = NO_ID;
    if (IS_HUMAN_PTR(ally)) {
	char msg[MSG_LEN];
	sprintf(msg, " < %s has cancelled the invitation for an alliance >",
		pl->name);
	Set_player_message(ally, msg);
    }
    return 1;
}

/* refuses invitation from a specific player */
int Refuse_alliance(int ind, int ally_ind)
{
    player	*pl = Players[ind],
		*ally = Players[ally_ind];

    if (ally->invite != pl->id) {
	/* we were not invited anyway */
	return 0;
    }
    ally->invite = NO_ID;
    if (IS_HUMAN_PTR(ally)) {
	char msg[MSG_LEN];
	sprintf(msg, " < %s has declined your invitation for an alliance >",
		pl->name);
	Set_player_message(ally, msg);
    }
    return 1;
}

/* refuses invitations from any player */
int Refuse_all_alliances(int ind)
{
    player	*pl = Players[ind];
    int		i, j = 0;

    for (i = 0; i < NumPlayers; i++) {
	if (Players[i]->invite == pl->id) {
	    Refuse_alliance(ind, i);
	    j++;
	}
    }
    if (IS_HUMAN_PTR(pl)) {
	char msg[MSG_LEN];
	if (j == 0) {
	    sprintf(msg, " < You were not invited for any alliance >");
	} else {
	    sprintf(msg, " < %d invitation%s for %s declined >", j,
		    (j > 1 ? "s" : ""), (j > 1 ? "alliances" : "an alliance"));
	}
	Set_player_message(pl, msg);
    }
    return j;
}

/* accepts an invitation from a specific player */
int Accept_alliance(int ind, int ally_ind)
{
    player	*pl = Players[ind],
		*ally = Players[ally_ind];
    int		success = 1;

    if (ally->invite != pl->id) {
	/* we were not invited */
	return 0;
    }
    ally->invite = NO_ID;
    if (ally->alliance != ALLIANCE_NOT_SET) {
	if (pl->alliance != ALLIANCE_NOT_SET) {
	    /* both players are in alliances */
	    Merge_alliances(ally_ind, pl->alliance);
	} else {
	    /* inviting player is in an alliance */
	    Player_join_alliance(ind, ally_ind);
	}
    } else {
	if (pl->alliance != ALLIANCE_NOT_SET) {
	    /* accepting player is in an alliance */
	    Player_join_alliance(ally_ind, ind);
	} else {
	    /* neither player is in an alliance */
	    success = Create_alliance(ind, ally_ind);
	}
    }
    return success;
}

/* accepts invitations from any player */
int Accept_all_alliances(int ind)
{
    player	*pl = Players[ind];
    int		i, j = 0;

    for (i = 0; i < NumPlayers; i++) {
	if (Players[i]->invite == pl->id) {
	    Accept_alliance(ind, i);
	    j++;
	}
    }
    if (IS_HUMAN_PTR(pl)) {
	char msg[MSG_LEN];
	if (j == 0) {
	    sprintf(msg, " < You were not invited for any alliance >");
	} else {
	    sprintf(msg, " < %d invitation%s for %s accepted >", j,
		    (j > 0 ? "s" : ""), (j > 0 ? "alliances" : "an alliance"));
	}
	Set_player_message(pl, msg);
    }
    return j;
}

/* returns a pointer to the alliance with a given ID */
static alliance_t *Find_alliance(int id)
{
    int i;

    if (id != ALLIANCE_NOT_SET) {
	for (i = 0; i < NumAlliances; i++) {
	    if (Alliances[i]->id == id) {
		return Alliances[i];
	    }
	}
    }

    return NULL;
}

/*
 * Return the number of members in a particular alliance.
 */
int Get_alliance_member_count(int id)
{
    alliance_t	*alliance = Find_alliance(id);

    if (alliance != NULL) {
	return alliance->NumMembers;
    }

    return 0;
}

/* sends a message to all the members of an alliance */
static void Set_alliance_message(alliance_t *alliance, const char *msg)
{
    int	i;

    for (i = 0; i < NumPlayers; i++) {
	if (IS_HUMAN_IND(i)) {
	    if (Players[i]->alliance == alliance->id) {
		Set_player_message(Players[i], msg);
	    }
	}
    }
}

/* returns an unused ID for an alliance */
static int New_alliance_ID(void)
{
    int i, try_id;

    for (try_id = 0; try_id < MAX_TEAMS; try_id++) {
	for (i = 0; i < NumAlliances; i++) {
	    if (Alliances[i]->id == try_id) {
		break;
	    }
	}
	if (i == NumAlliances) {
	    break;
	}
    }
    if (try_id < MAX_TEAMS) {
	return try_id;
    }
    return ALLIANCE_NOT_SET;
}

/* creates an alliance between two players */
static int Create_alliance(int ind1, int ind2)
{
    player	*pl1 = Players[ind1],
		*pl2 = Players[ind2];
    alliance_t	*alliance = (alliance_t *)malloc(sizeof(alliance_t));
    char	msg[MSG_LEN];

    if (alliance == NULL) {
	error("Not enough memory for new alliance.\n");
	return 0;
    }

    alliance->id = New_alliance_ID();
    if (alliance->id == ALLIANCE_NOT_SET) {
	warn("Maximum number of alliances reached.\n");
	free(alliance);
	return 0;
    }
    alliance->NumMembers = 0;
    Alliances[NumAlliances] = alliance;
    NumAlliances++;
    Alliance_add_player(alliance, pl1);
    Alliance_add_player(alliance, pl2);
    /* announcement */
    if (announceAlliances) {
	sprintf(msg, " < %s and %s have formed alliance %d >", pl1->name,
		pl2->name, alliance->id);
	Set_message(msg);
    } else {
	sprintf(msg, " < You have formed an alliance with %s >", pl2->name);
	Set_player_message(pl1, msg);
	sprintf(msg, " < You have formed an alliance with %s >", pl1->name);
	Set_player_message(pl2, msg);
    }
    return 1;
}

/* adds a player to an existing alliance */
void Player_join_alliance(int ind, int ally_ind)
{
    player	*pl = Players[ind],
		*ally = Players[ally_ind];
    alliance_t	*alliance = Find_alliance(ally->alliance);
    char	msg[MSG_LEN];

    if (!IS_TANK_IND(ind)) {
	/* announce first to avoid sending the player two messages */
	if (announceAlliances) {
	    sprintf(msg, " < %s has joined alliance %d >",
		    pl->name, alliance->id);
	    Set_message(msg);
	}
	else {
	    sprintf(msg, " < %s has joined your alliance >", pl->name);
	    Set_alliance_message(alliance, msg);
	    if (IS_HUMAN_PTR(pl)) {
		sprintf(msg, " < You have joined %s's alliance >", ally->name);
		Set_player_message(pl, msg);
	    }
	}
    }

    Alliance_add_player(alliance, pl);
}

/* atomic addition of player to alliance */
static void Alliance_add_player(alliance_t *alliance, player *pl)
{
    int	i;

    /* drop invitations for this player from other members */
    for (i = 0; i < NumPlayers; i++) {
	if (Players[i]->invite == pl->id) {
	    Cancel_invitation(i);
	}
    }
    pl->alliance = alliance->id;
    alliance->NumMembers++;
    updateScores = true;
}

/* removes a player from an alliance and dissolves the alliance if necessary */
int Leave_alliance(int ind)
{
    player	*pl = Players[ind];
    alliance_t	*alliance;
    char	msg[MSG_LEN];

    if (pl->alliance == ALLIANCE_NOT_SET) {
	/* we're not in any alliance */
	return 0;
    }
    alliance = Find_alliance(pl->alliance);
    Alliance_remove_player(alliance, pl);
    /* announcement */
    if (!IS_TANK_IND(ind)) {
	if (announceAlliances) {
	    sprintf(msg, " < %s has left alliance %d >", pl->name,
		    alliance->id);
	    Set_message(msg);
	} else {
	    sprintf(msg, " < %s has left your alliance >", pl->name);
	    Set_alliance_message(alliance, msg);
	    if (IS_HUMAN_PTR(pl)) {
		Set_player_message(pl, " < You have left the alliance >");
	    }
	}
    }
    if (alliance->NumMembers <= 1) {
	Dissolve_alliance(alliance->id);
    }
    return 1;
}

/* atomic removal of player from alliance */
static int Alliance_remove_player(alliance_t *alliance, player *pl)
{
    if (pl->alliance == alliance->id) {
	pl->alliance = ALLIANCE_NOT_SET;
	alliance->NumMembers--;
	updateScores = true;
	return 1;
    }
    return 0;
}

static void Dissolve_alliance(int id)
{
    alliance_t	*alliance = Find_alliance(id);
    int		i;

    /* remove all remaining members from the alliance */
    for (i = 0; i < NumPlayers; i++) {
	if (Players[i]->alliance == id) {
	    Alliance_remove_player(alliance, Players[i]);
	    if (!announceAlliances && IS_HUMAN_IND(i)) {
		Set_player_message(Players[i],
				   " < Your alliance has been dissolved >");
	    }
	}
    }
    /* check */
    if (alliance->NumMembers != 0) {
	warn("Dissolve_alliance after dissolve %d remain!",
	     alliance->NumMembers);
    }

    /* find the index of the alliance to be removed */
    for (i = 0; i < NumAlliances; i++) {
	if (Alliances[i]->id == alliance->id) {
	    break;
	}
    }
    /* move the last alliance to that index */
    Alliances[i] = Alliances[NumAlliances - 1];
    /* announcement */
    if (announceAlliances) {
	char msg[MSG_LEN];
	sprintf(msg, " < Alliance %d has been dissolved >", alliance->id);
	Set_message(msg);
    }
    /* and clean up that alliance */
    free(alliance);
    NumAlliances--;
}

/*
 * Destroy all alliances.
 */
void Dissolve_all_alliances(void)
{
    int		i;

    for (i = NumAlliances - 1; i >= 0; i--) {
	Dissolve_alliance(Alliances[i]->id);
    }
}

/* merges two alliances by moving the members of the second to the first */
static void Merge_alliances(int ind, int id2)
{
    alliance_t	*alliance2 = Find_alliance(id2);
    int		i;

    /* move each member of alliance2 to alliance1 */
    for (i = 0; i < NumPlayers; i++) {
	if (Players[i]->alliance == id2) {
	    Alliance_remove_player(alliance2, Players[i]);
	    Player_join_alliance(i, ind);
	}
    }
    Dissolve_alliance(id2);
}

void Alliance_player_list(int ind)
{
    player	*pl = Players[ind];
    int		i;
    char	msg[MSG_LEN];

    if (pl->alliance == ALLIANCE_NOT_SET) {
	Set_player_message(pl, " < You are not a member of any alliance >");
    }
    else {
	if (announceAlliances) {
	    sprintf(msg, " < Alliance %d:", pl->alliance);
	} else {
	    sprintf(msg, " < Your alliance: ");
	}
	for (i = 0; i < NumPlayers; i++) {
	    if (Players[i]->alliance == pl->alliance) {
		if (IS_HUMAN_IND(i)) {
		    if (strlen(msg) > 80) {
			strlcat(msg, ">", sizeof(msg));
			Set_player_message(pl, msg);
			strlcpy(msg, " <            ", sizeof(msg));
		    }
		    strlcat(msg, Players[i]->name, sizeof(msg));
		    strlcat(msg, ", ", sizeof(msg));
		}
	    }
	}
	for (i = 0; i < NumPlayers; i++) {
	    if (Players[i]->alliance == pl->alliance) {
		if (IS_ROBOT_IND(i)) {
		    if (strlen(msg) > 80) {
			strlcat(msg, ">", sizeof(msg));
			Set_player_message(pl, msg);
			strlcpy(msg, " <            ", sizeof(msg));
		    }
		    strlcat(msg, Players[i]->name, sizeof(msg));
		    strlcat(msg, ", ", sizeof(msg));
		}
	    }
	}
	if (strlen(msg) >= 2 && !strcmp(msg + strlen(msg) - 2, ", ")) {
	    msg[strlen(msg) - 2] = '\0';
	}
	strlcat(msg, " >", sizeof(msg));
	Set_player_message(pl, msg);
    }
}


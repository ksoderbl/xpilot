/* $Id: robot.c,v 3.58 1995/11/15 20:09:11 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
 *
 *      Bj�rn Stabell        (bjoerns@staff.cs.uit.no)
 *      Ken Ronny Schouten   (kenrsc@stud.cs.uit.no)
 *      Bert G�sbers         (bert@mc.bio.uva.nl)
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
/* Robot code submitted by Maurice Abraham. */

#include <unistd.h>
#include <stdlib.h>

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "score.h"
#include "bit.h"
#include "saudio.h"
#include "netserver.h"
#include "pack.h"
#include "robot.h"

char robot_version[] = VERSION;

#ifndef	lint
static char sourceid[] = "@(#)robot.c,v 1.3 1992/06/26 15:25:46 bjoerns Exp";
#endif


/*
 * Map objects a robot is expected to be able to fly through without damage.
 */
#define EMPTY_SPACE(s)	\
    BIT(1 << (s), SPACE_BIT | BASE_BIT | WORMHOLE_BIT | POS_GRAV_BIT | \
		  NEG_GRAV_BIT | CWISE_GRAV_BIT | ACWISE_GRAV_BIT | \
		  DECOR_LU_BIT | DECOR_LD_BIT | DECOR_RU_BIT | DECOR_RD_BIT | \
		  DECOR_FILLED_BIT | CHECK_BIT | ITEM_CONCENTRATOR_BIT)

/*
 * Bitmask of object types the robot puts up shield for.
 */
extern long KILLING_SHOTS;

int         NumRobots = 0;
static int  MAX_ROBOTS = 1;

/*
 * Prototypes for methods of the default robot type.
 */
static void Robot_default_setup(void);
static void Robot_default_create(int ind, char *str);
static void Robot_default_go_home(int ind);
static void Robot_default_play(int ind);
static void Robot_default_set_war(int ind, int victim_id);
static int Robot_default_war_on_player(int ind);
static void Robot_default_message(int ind, char *str);
static void Robot_default_destroy(int ind);

/*
 * Array of the different robot types available.
 * The default robot type should be first.
 */
robot_type_t robot_types[] = {
    {
	"default",
	Robot_default_setup,
	Robot_default_create,
	Robot_default_go_home,
	Robot_default_play,
	Robot_default_set_war,
	Robot_default_war_on_player,
	Robot_default_message,
	Robot_default_destroy
    },
};

/* 
 * Array of different robots.
 * Each robot has a name of a robot type determining
 * what robot code will control this robot,
 * its name as seen by the human players,
 * some optional configuration string,
 * a usage count,
 * and a shipshape.
 * In the future we may want to read in this data from
 * a configuration file.
 */
static robot_t Robots[] = {
    {
	"default",
	"Mad Max",
	"94 20",
	0,
	"(15,8,7)(15,0)(7,1)(7,2)(2,4)(-1,11)"
	"(-3,11)(-2,3)(-8,6)(-8,-6)(-2,-3)(-3,-11)"
	"(-1,-11)(2,-4)(7,-2)(7,-1)"
    },
    {
	"default",
	"Blackie",
	"10 90",
	0,
	"(16,6,10)(15,0)(6,2)(-2,3)(-1,4)(-2,5)"
	"(-10,8)(-13,8)(-13,1)(-15,0)(-13,-1)"
	"(-13,-8)(-10,-8)(-2,-5)(-1,-4)(-2,-3)(6,-2)"
    },
    {
	"default",
	"Kryten",
	"70 40",
	0,
	"(4,1,3)(15,0)(0,8)(-8,0)(0,-8)"
    },
    {
	"default",
	"Marvin",
	"30 70",
	0,
	"(15,4,5)(10,0)(10,7)(5,14)(-5,14)(-10,7)"
	"(-10,-7)(-5,-14)(5,-14)(10,-7)(10,0)(5,5)"
	"(2,7)(5,0)(2,-7)(5,-5)"
    },
    {
	"default",
	"R2D2",
	"50 60",
	0,
	"(15,8,9)(15,0)(14,1)(-1,2)(-2,9)(0,10)"
	"(-4,10)(-7,2)(-8,2)(-8,-2)(-7,-2)(-4,-10)"
	"(0,-10)(-2,-9)(-1,-2)(14,-1)"
    },
    {
	"default",
	"C3PO",
	"60 50",
	0,
	"(16,1,15)(10,0)(0,5)(0,15)(15,10)(0,15)"
	"(-15,10)(0,15)(0,5)(-7,0)(0,-5)(0,-15)"
	"(-15,-10)(0,-15)(15,-10)(0,-15)(0,-5)"
    },
    {
	"default",
	"K9",
	"50 50",
	0,
	"(14,0,5)(15,0)(15,5)(5,5)(5,-5)(15,-5)"
	"(15,0)(-15,0)(-15,5)(5,5)(5,-5)(-15,-5)"
	"(-15,-8)(-15,8)(-15,0)"
    },
    {
	"default",
	"Robby",
	"45 55",
	0,
	"(5,2,3)(15,0)(0,12)(-9,8)(-9,-8)(0,-12)"
    },
    {
	"default",
	"Mickey",
	"05 95",
	0,
	"(15,6,7)(5,-1)(8,-5)(7,-9)(4,-11)(-1,-10)"
	"(-5,-6)(-8,-10)(-8,10)(-5,6)(-1,10)(4,11)"
	"(7,9)(8,5)(5,1)(0,0)"
    },
    {
	"default",
	"Hermes",
	"15 85",
	0,
	"(16,12,11)(10,1)(12,8)(-11,8)(-10,3)"
	"(-7,0)(-5,2)(-7,0)(-10,-1)(-10,-3)"
	"(-13,-4)(-13,-7)(-15,-8)(-15,-13)(-5,-5)"
	"(-2,-4)(5,-2)"
    },
    {
	"default",
	"Pan",
	"60 60",
	0,
	"(14,6,5)(15,-1)(15,0)(5,0)(5,-1)(5,9)(-15,9)"
	"(-15,-4)(-5,-7)(-3,-8)(-7,-8)(-5,-7)(5,-4)"
	"(5,-1)(-15,-1)"
    },
    {
	"default",
	"Azurion",
	"40 30",
	0,
	"(6,2,4)(15,0)(0,2)(-9,8)(-3,0)(-9,-8)(0,-2)"
    },
    {
	"default",
	"Droidion",
	"60 30",
	0,
	"(6,2,4)(9,0)(4,8)(-5,8)(-10,0)(-5,-8)(4,-8)"
    },
    {
	"default",
	"Terminator",
	"80 40",
	0,
	"(6,2,4)(15,0)(0,2)(-9,8)(-3,0)"
	"(-9,-8)(0,-2)"
    },
    {
	"default",
	"Sniper",
	"30 90",
	0,
	"(15,6,9)(15,0)(4,2)(-2,8)(-4,7)(-3,2)"
	"(-8,5)(-8,2)(-6,1)(-6,-1)(-8,-2)(-8,-5)"
	"(-3,-2)(-4,-7)(-2,-8)(4,-2)"
    },
    {
	"default",
	"Slugger",
	"40 40",
	0,
	"(15,8,7)(13,0)(11,1)(3,2)(-1,8)(-3,8)"
	"(-3,2)(-5,2)(-8,5)(-8,-5)(-5,-2)(-3,-2)"
	"(-3,-8)(-1,-8)(3,-2)(11,-1)"
    },
    {
	"default",
	"Uzi",
	"95 5",
	0,
	"(16,9,8)(15,3)(7,3)(7,-8)(3,-8)(3,1)(-2,1)"
	"(-3,-1)(-5,-1)(-14,-5)(-15,2)(-3,4)(-1,8)"
	"(0,6)(13,6)(14,8)(15,6)"
    },
    {
	"default",
	"Capone",
	"80 50",
	0,
	"(15,9,6)(14,0)(2,2)(0,8)(1,8)(-3,8)(-3,2)"
	"(-8,4)(-7,1)(-7,-1)(-8,-4)(-3,-2)(-3,-8)"
	"(1,-8)(0,-8)(2,-2)"
    },
    {
	"default",
	"Tanx",
	"40 70",
	0,
	"(16,7,8)(15,1)(2,0)(1,-2)(6,-3)(6,-5)(3,-8)"
	"(-10,-8)(-13,-6)(-13,-3)(-10,-2)(-11,2)(-7,2)"
	"(-7,8)(-7,2)(1,2)(2,1)"
    },
    {
	"default",
	"Chrome Star",
	"60 60",
	0,
	"(5,1,4)(8,0)(-8,5)(2,-8)(2,8)(-8,-5)"
    },
    {
	"default",
	"Bully",
	"80 10",
	0,
	"(15,6,9)(11,0)(12,-3)(9,-3)(8,-2)(-5,-5)"
	"(-9,-11)(-14,-14)(-5,-3)(-5,3)(-14,14)"
	"(-9,11)(-5,5)(8,2)(9,3)(12,3)"
    },
    {
	"default",
	"Metal Hero",
	"40 45",
	0,
	"(16,7,9)(15,5)(12,-2)(9,-2)(10,-1)"
	"(-8,-1)(-4,-1)(1,-3)(-13,-9)(-9,0)"
	"(-15,8)(1,3)(-4,1)(-8,1)(-8,-1)(-8,1)"
	"(11,1)"
    },
    {
	"default",
	"Aurora",
	"60 55",
	0,
	"(16,5,11)(15,0)(-1,3)(-3,5)(-3,9)(7,10)"
	"(-12,10)(-6,9)(-6,4)(-8,0)(-6,-4)(-6,-9)"
	"(-12,-10)(7,-10)(-3,-9)(-3,-5)(-1,-3)"
    },
    {
	"default",
	"Dalt Wisney",
	"30 75",
	0,
	"(16,10,6)(14,0)(7,-4)(0,-1)(-5,-4)"
	"(2,-8)(0,-10)(-14,-10)(-5,-7)"
	"(-14,0)(-5,7)(-14,10)(0,10)(2,8)"
	"(-5,4)(0,1)(7,4)"
    },
    {
	"default",
	"Psycho",
	"65 55",
	0,
	"(11,5,6)(8,0)(5,8)(3,12)(0,15)(0,0)"
	"(-8,3)(-8,-3)(0,0)(0,-15)(3,-12)(5,-8)"
    },
    {
	"default",
	"Gorgon",
	"30 40",
	0,
	"(15,7,8)(15,0)(5,2)(3,8)(2,2)(-9,2)"
	"(-10,4)(-12,2)(-14,4)(-14,-4)(-12,-2)"
	"(-10,-4)(-9,-2)(2,-2)(3,-8)(5,-2)"
    },
    {
	"default",
	"Pompel",
	"50 50",
	0,
	"(15,7,8)(15,0)(14,4)(10,5)(5,2)(-7,3)"
	"(-7,6)(5,8)(-9,8)(-9,-8)(5,-8)(-7,-6)"
	"(-7,-3)(5,-2)(10,-5)(14,-4)"
    },
    {
	"default",
	"Pilt",
	"50 50",
	0,
	"(16,8,7)(15,0)(13,-2)(9,-3)(3,-3)(-3,-3)"
	"(-5,-2)(-13,-2)(-15,-3)(-15,3)(-13,2)"
	"(-5,2)(-3,3)(-3,-8)(-3,8)(-3,3)(8,3)"
    },
    {
	"default",
	"Sparky",
	"20 40",
	0,
	"(15,8,7)(15,-8)(6,-5)(7,-4)(1,-2)(2,-1)"
	"(-4,0)(-3,2)(-15,8)(-15,2)(-8,0)(-9,-2)"
	"(-3,-3)(-4,-4)(3,-5)(2,-7)"
    },
    {
	"default",
	"Cobra",
	"85 60",
	0,
	"(16,5,11)(8,0)(8,-6)(6,-8)(0,-7)(5,-6)"
	"(-8,-4)(5,-2)(0,-1)(5,0)(0,1)(5,2)(-8,4)"
	"(5,6)(0,7)(6,8)(8,6)"
    },
    {
	"default",
	"Falcon",
	"70 20",
	0,
	"(16,5,6)(14,2)(14,4)(2,10)(-5,10)(-10,8)"
	"(-12,3)(-12,-3)(-10,-8)(-5,-10)(9,-11)"
	"(10,-8)(7,-8)(14,-4)(14,-2)(4,-2)(4,2)"
    },
    {
	"default",
	"Boson",
	"25 35",
	0,
	"(16,11,12)(15,0)(10,-5)(4,-8)(7,-2)(7,2)"
	"(4,8)(6,0)(4,-8)(-10,-8)(-10,8)(-10,-8)"
	"(-15,-7)(-15,7)(-10,8)(4,8)(10,5)"
    },
    {
	"default",
	"Blazy",
	"40 40",
	0,
	"(12,4,8)(4,0)(2,4)(-5,11)(10,12)(-8,12)"
	"(-4,6)(-2,0)(-4,-6)(-8,-12)(10,-12)"
	"(-5,-11)(2,-4)"
    },
    {
	"default",
	"Pixie",
	"15 93",
	0,
	"(13,6,7)(15,0)(7,4)(11,1)(-4,3)(3,5)"
	"(-7,10)(-9,2)(-9,-2)(-7,-10)(3,-5)(-4,-3)"
	"(11,-1)(7,-4)"
    },
    {
	"default",
	"Wimpy",
	"5 98",
	0,
	"(16,9,7)(3,0)(6,5)(8,10)(5,11)(1,10)(-1,8)"
	"(-4,9)(-8,6)(-5,0)(-8,-6)(-4,-9)(-1,-8)"
	"(1,-10)(5,-11)(8,-10)(6,-5)"
    },
    {
	"default",
	"Bonnie",
	"30 40",
	0,
	"(16,9,6)(13,3)(5,3)(5,1)(4,-1)(0,-1)"
	"(-2,-8)(-8,-8)(-5,3)(-6,6)(-8,7)(-7,8)"
	"(-4,7)(8,7)(10,8)(12,8)(13,7)"
    },
    {
	"default",
	"Clyde",
	"40 45",
	0,
	"(16,5,11)(14,0)(5,5)(6,2)(0,2)(0,8)(-13,8)"
	"(-13,4)(-4,4)(-6,0)(-4,-4)(-13,-4)(-13,-8)"
	"(0,-8)(0,-2)(6,-2)(5,-5)"
    },
    {
	"default",
	"Neuro",
	"70 70",
	0,
	"(16,7,5)(12,-7)(12,-12)(5,-12)(2,-10)"
	"(1,-5)(-9,-4)(-11,2)(-8,8)(-3,11)(3,11)"
	"(9,8)(11,2)(13,0)(12,-3)(12,-7)(7,-7)"
    },
};


/*
 * First time initialization of all the robot stuff.
 */
void Robot_init(void)
{
    int         i;

    MAX_ROBOTS = NELEM(Robots);

    for (i = 0; i < NELEM(robot_types); i++) {
	(*robot_types[i].setup)();
    }
}


static void Robot_talks(enum robot_talk_t says_what,
			char *robot_name, char *other_name)
{
    /*
     * Insert your own witty messages here and remove the silly ones.
     */

    static char *enter_msgs[] = {
	"%s just can't stand you anymore.",
	"%s has come to give you a hard time.",
	"%s is looking for trouble.",
	"%s has a very loose trigger finger.",
	"Have fear, %s is here.",
	"Prepare to die by the hands of %s.",
	"%s is untouchable.",
	"%s is in a gruesome mood.",
	"%s is in a killing mood.",
	"%s wants you for dessert.",
	"%s vows to torment you in this life and the next.",
	"%s has no sense of humour.",
	"%s is back from the Sirius wars, and he's in a violent mood.",
    };
    static char *leave_msgs[] = {
	"That's it, I've had enough. :(   I'm outta here. [%s]",
	"Later people.  It's been fun. [%s]",
	"Gotta go, ... er ... this ... er ... lab is closing. [%s]",
	"Er...  Oh!  It it really that late?  I gotta go. [%s].",
	"I'm signing off now.  Bye! [%s]",
	"Gotta go...  Er... I have some work to be done. [%s]",
	"This sucks! :(  I'm going back to cyberspace. [%s]",
	"I've taken enough beating for today. [%s]",
	"Oh man.  Playing with humans sucks. [%s]",
	"You can't beat us robots; we always return, stronger than ever! [%s]",
	"Geez, this just isn't my lucky day.  See ya some other time. [%s]",
	"Wow, this game is just killing me. :( [%s]",
	"I'll be back when you stop cheating! :-( [%s]",
    };
    static char *kill_msgs[] = {
	"Have some %s.  Have some! [%s]",
	"You want some more %s? [%s]",
	"%s lost his stuff again.  That's just tooooooo bad. :) [%s]",
	"%s: did you like that one? [%s]",
	"Face it %s, you just can't compete with me. [%s]",
	"%s, my grandmother plays better than you. [%s]",
	"Hey %s, go play chess instead. [%s]",
	"I think Darwin would've said you're too unfit to survive %s :) [%s]",
	"Oh my, what colourful explosions you make %s. :) [%s]",
    };
    static char *war_msgs[] = {
	"UNBELIEVABLE, me shot down by %s?!?!  This means war [%s]",
	"People like %s just piss me off. [%s]",
	"Nice %s.  But now its my turn. [%s]",
	"Red alert... target: %s. [%s]",
	"$%#^@#$^#$%  That's the last time you did that %s! [%s]",
	"Enough's enough!  It's only room enough for one of us %s here. [%s]",
	"I'm sorry %s, but you must... DIE!!!!! [%s]",
	"Jihad!  Die %s!  Die! [%s]",
    };

    static int		next_msg = -1;
    char		**msgsp;
    int			two, i, n;
    char		msg[MSG_LEN];

    if (robotsTalk != true && says_what != ROBOT_TALK_ENTER) {
	return;
    }

    switch (says_what) {
    case ROBOT_TALK_ENTER:
	msgsp = enter_msgs;
	n = NELEM(enter_msgs);
	two = 1;
	break;
    case ROBOT_TALK_LEAVE:
	msgsp = leave_msgs;
	n = NELEM(leave_msgs);
	two = 1;
	break;
    case ROBOT_TALK_KILL:
	msgsp = kill_msgs;
	n = NELEM(kill_msgs);
	two = 2;
	break;
    case ROBOT_TALK_WAR:
	msgsp = war_msgs;
	n = NELEM(war_msgs);
	two = 2;
	break;
    default:
	return;
    }

    if (next_msg == -1) {
	next_msg = rand();
    }
    if (++next_msg > 997) {
	next_msg = 0;
    }
    i = next_msg % n;
    if (two == 2) {
	sprintf(msg, msgsp[i], other_name, robot_name);
    } else {
	sprintf(msg, msgsp[i], robot_name);
    }
    Set_message(msg);
}


static void Robot_create(void)
{
    player		*robot;
    robot_t		*rob;
    int			i,
			num;
    int			most_used,
			least_used;
    robot_data_t	*data, *new_data;
    robot_type_t	*rob_type;

    if ((new_data = (robot_data_t *)malloc(sizeof(robot_data_t))) == NULL) {
	perror("malloc robot_data");
	return;
    }
    new_data->private_data = NULL;

    most_used = 0;
    for (i = 0; i < MAX_ROBOTS; i++) {
	if (Robots[i].used > Robots[most_used].used) {
	    most_used = i;
	}
    }
    for (i = 0; i < NumPlayers; i++) {
	if (IS_ROBOT_IND(i)) {
	    data = (robot_data_t *)Players[i]->robot_data_ptr;
	    if (Robots[data->robots_ind].used < Robots[most_used].used) {
		Robots[data->robots_ind].used = Robots[most_used].used;
	    }
	}
    }
    least_used = 0;
    for (i = 0; i < MAX_ROBOTS; i++) {
	if (Robots[i].used < Robots[least_used].used) {
	    least_used = i;
	}
    }
    num = rand() % MAX_ROBOTS;
    while (Robots[num].used > Robots[least_used].used) {
	if (++num >= MAX_ROBOTS) {
	    num = 0;
	}
    }
    rob = &Robots[num];
    rob->used++;
    new_data->robots_ind = num;
    new_data->robot_types_ind = 0;
    for (i = 1; i < NELEM(robot_types); i++) {
	if (!strcmp(robot_types[i].name, rob->driver)) {
	    new_data->robot_types_ind = i;
	}
    }
    rob_type = &robot_types[new_data->robot_types_ind];

    Init_player(NumPlayers, (allowShipShapes)
			    ? Parse_shape_str(rob->shape)
			    : (wireobj *)NULL);
    robot = Players[NumPlayers];
    SET_BIT(robot->type_ext, OBJ_EXT_ROBOT);
    robot->robot_data_ptr = new_data;

    strcpy(robot->name, rob->name);
    strcpy(robot->realname, "robot");
    strcpy(robot->hostname, "robots.org");

    robot->color = WHITE;
    robot->turnspeed = MAX_PLAYER_TURNSPEED;
    robot->turnspeed_s = MAX_PLAYER_TURNSPEED;
    robot->turnresistance = 0.12;
    robot->turnresistance_s = 0.12;
    robot->power = MAX_PLAYER_POWER;
    robot->power_s = MAX_PLAYER_POWER;
    if (BIT(World.rules->mode, TEAM_PLAY))
	robot->team = 0;		/* Robots are on their own team */
    if (robot->mychar != 'W')
	robot->mychar = 'R';

    robot->fuel.l1 = 100 * FUEL_SCALE_FACT;
    robot->fuel.l2 = 200 * FUEL_SCALE_FACT;
    robot->fuel.l3 = 500 * FUEL_SCALE_FACT;

    Pick_startpos(NumPlayers);

    (*rob_type->create)(NumPlayers, rob->config);

    Go_home(NumPlayers);

    /*
     * robot->shot_speed= ShotsSpeed + (rob->attack - 50) / 5.0;
     * robot->shot_mass	= ShotsMass + (rob->defense - rob->attack) / 10.0;
     * robot->max_speed	= SPEED_LIMIT - robot->shot_speed;
     */
    NumPlayers++;
    World.teams[0].NumMembers++;
    Id++;
    NumRobots++;

    for (i = 0; i < NumPlayers - 1; i++) {
	if (Players[i]->conn != NOT_CONNECTED) {
	    Send_player(Players[i]->conn, robot->id);
	    Send_base(Players[i]->conn, robot->id, robot->home_base);
	}
    }

    Robot_talks(ROBOT_TALK_ENTER, robot->name, "");

#ifndef	SILENT
    printf("%s (%d, %s) starts at startpos %d.\n",
	   robot->name, NumPlayers, robot->realname, robot->home_base);
#endif

    updateScores = true;
}


void Robot_destroy(int ind)
{
    player		*pl = Players[ind];

    (*robot_types[pl->robot_data_ptr->robot_types_ind].destroy)(ind);
    free(Players[ind]->robot_data_ptr);
    Players[ind]->robot_data_ptr = NULL;
}


void Robot_delete(int ind, int kicked)
{
    long		i,
			low_score = LONG_MAX,
			low_i = -1;
    char		msg[MSG_LEN];

    if (ind == -1) {
	/*
	 * Find the robot with the lowest score.
	 */

	for (i = 0; i < NumPlayers; i++) {
	    if (!IS_ROBOT_IND(i))
		continue;

	    if (Players[i]->score < low_score) {
		low_i = i;
		low_score = Players[i]->score;
	    }
	}
	if (low_i >= 0) {
	    ind = low_i;
	}
    }

    if (ind >= 0) {
	if (kicked) {
	    sprintf(msg, "\"%s\" upset the gods and was kicked out "
		    "of the game.", Players[ind]->name);
	    Set_message(msg);
	}
	Delete_player(ind);
    }
}


void Robot_go_gome(int ind)
{
    (*robot_types[Players[ind]->robot_data_ptr->robot_types_ind].go_home)(ind);
}


/*
 * Turn on a war lock.
 */
static void Robot_set_war(int ind, int victim_id)
{
    player		*pl = Players[ind];

    (*robot_types[pl->robot_data_ptr->robot_types_ind].set_war)(ind, victim_id);
}


/*
 * Turn off a war lock.
 * The only time when this can be called is if
 * a player a robot has war on leaves the game.
 */
void Robot_reset_war(int ind)
{
    Robot_set_war(ind, -1);
}


/*
 * Someone has programmed a robot (using ECM) to seek some player.
 */
void Robot_program(int ind, int victim_id)
{
    Robot_set_war(ind, victim_id);
}


/*
 * Return the id of the player this robot has war on.
 * If the robot is not in peace mode then return -1.
 */
int Robot_war_on_player(int ind)
{
    player		*pl = Players[ind];
    robot_type_t	*rob_type =
			    &robot_types[pl->robot_data_ptr->robot_types_ind];

    return (*rob_type->war_on_player)(ind);
}


/*
 * A robot has killed someone.
 * Or a robot has been killed by someone.
 * Maybe this is enough reason for the killed robot to change
 * its behavior with respect to the player it has been killed by.
 */
void Robot_war(int ind, int killer)
{
    player		*pl = Players[ind],
			*kp = Players[killer];
    int			i;

    if (killer == ind) {
	return;
    }

    if (IS_ROBOT_PTR(kp)) {
	Robot_talks(ROBOT_TALK_KILL, kp->name, pl->name);
    }

    if (IS_ROBOT_PTR(pl)
	&& rand()%100 < kp->score - pl->score) {

	Robot_talks(ROBOT_TALK_WAR, pl->name, kp->name);

	/*
	 * Give fuel for offensive.
	 */
	pl->fuel.sum = MAX_PLAYER_FUEL;

	if (Robot_war_on_player(ind) != kp->id) {
	    for (i = 0; i < NumPlayers; i++) {
		if (Players[i]->conn != NOT_CONNECTED) {
		    Send_war(Players[i]->conn, pl->id, kp->id);
		}
	    }
	    sound_play_all(DECLARE_WAR_SOUND);
	    Robot_set_war(ind, kp->id);
	}
    }
}


/*
 * A robot starts on its homebase.
 */
void Robot_go_home(int ind)
{
    (*robot_types[Players[ind]->robot_data_ptr->robot_types_ind].go_home)(ind);
}


/*
 * Someone sends a message to a robot.
 * The format of the message is: "This is the real message [receiver]:[sender]"
 */
void Robot_message(int ind, char *message)
{
    player		*pl = Players[ind];
    robot_type_t	*rob_type =
			    &robot_types[pl->robot_data_ptr->robot_types_ind];

    (*rob_type->message)(ind, message);
}


/*
 * A robot plays this frame.
 */
static void Robot_play(int ind)
{
    (*robot_types[Players[ind]->robot_data_ptr->robot_types_ind].play)(ind);
}


void Robot_update(void)
{
    player		*pl;
    int			i;
    char		msg[MSG_LEN];
    static int		new_robot_delay;


    if (NumRobots < WantedNumRobots
	&& NumPlayers - NumPseudoPlayers < World.NumBases - 1
	&& NumRobots < MAX_ROBOTS
	&& !(BIT(World.rules->mode, TEAM_PLAY)
	     && World.teams[0].NumMembers >= World.teams[0].NumBases)) {

	if (++new_robot_delay >= RECOVERY_DELAY) {
	    Robot_create();
	    new_robot_delay = 0;
	}
    } else if (NumPlayers - NumPseudoPlayers >= World.NumBases
	       && NumRobots > 0) {
	Robot_delete(-1, false);
    }
    if (NumRobots <= 0)
	return;

    for (i = 0; i < NumPlayers; i++) {
	pl = Players[i];
	if (IS_HUMAN_PTR(pl)) {
	    /*
	     * Ignore human players here.
	     */
	    continue;
	}
	if (IS_TANK_PTR(pl)) {
	    /*
	     * Update tanks here.
	     * This could be moved into its own robot type function...
	     */

	    int         t = loops % (TANK_NOTHRUST_TIME + TANK_THRUST_TIME);

	    if (t == 0) {
		SET_BIT(pl->status, THRUSTING);
	    } else if (t == TANK_THRUST_TIME) {
		CLR_BIT(pl->status, THRUSTING);
	    }
	    continue;
	}

	/*
	 * So it is a genuine robot...
	 * Check if it is still considered good enough to continue playing...
	 */
	if (robotsLeave
	    && pl->life > 0
	    && !BIT(World.rules->mode, LIMITED_LIVES)) {
	    msg[0] = '\0';
	    if (robotLeaveLife > 0 && pl->life >= robotLeaveLife) {
		sprintf(msg, "%s retired.", pl->name);
	    }
	    else if (robotLeaveScore != 0 && pl->score < robotLeaveScore) {
		sprintf(msg, "%s left out of disappointment.", pl->name);
	    }
	    else if (robotLeaveRatio != 0 && pl->score / (pl->life + 1)
		    < robotLeaveRatio) {
		sprintf(msg, "%s played too badly.", pl->name);
	    }
	    if (msg[0] != '\0') {
		Robot_talks(ROBOT_TALK_LEAVE, pl->name, "");
		Set_message(msg);
		Robot_delete(i, false);
		i--;
		continue;
	    }
	}

	if (BIT(pl->status, PLAYING|GAME_OVER) != PLAYING)
	    continue;

	/*
	 * Let the robot code control this robot.
	 */
	Robot_play(i);
    }
}


/*########################### Robot default #################################*/
/*
 * This is the default robot code.
 * We better move this to its own source file.
 */

/*
 * Private functions.
 */
static bool Check_robot_navigate(int ind, bool * num_evade);
static bool Check_robot_evade(int ind, int mine_i, int ship_i);
static bool Check_robot_target(int ind, int item_x, int item_y, int new_mode,
			       int attack_level);


/*
 * Function to cast from player structure to robot data structure.
 * This isolates casts (aka. type violations) to a few places.
 */
static robot_default_data_t *Robot_default_get_data(player *pl)
{
    return (robot_default_data_t *)pl->robot_data_ptr->private_data;
}

/*
 * Setup the global stuff for the default robot types.
 */
static void Robot_default_setup(void)
{
    /* nothing to do for the default robot */
}

/*
 * A default robot is created.
 */
static void Robot_default_create(int ind, char *str)
{
    player			*pl = Players[ind];
    robot_default_data_t	*my_data;

    if (!(my_data = (robot_default_data_t *)malloc(sizeof(*my_data)))) {
	error("no mem for default robot");
	End_game();
    }

    my_data->robot_mode      = RM_TAKE_OFF;
    my_data->robot_count     = 0;
    my_data->robot_lock      = LOCK_NONE;
    my_data->robot_lock_id   = 0;

    if (sscanf(str, " %d %d", &my_data->attack, &my_data->attack) != 2) {
	printf("default robot invalid config: \"%s\"\n", str);
	my_data->attack = 5 + (int)(rfrac() * 90 + 0.5);
	my_data->defense = 100 - my_data->attack;
    }
    /*
     * some parameters which may be changed to be dependent upon
     * the `attack' and `defense' settings of this robot.
     */
    my_data->robot_normal_speed = 5.0;	/* was 3 */
    my_data->robot_attack_speed = 15.0;	/* was 12 */
    my_data->robot_max_speed = 25.0;	/* was 20 */

    pl->robot_data_ptr->private_data = (void *)my_data;
}

/*
 * A default robot is placed on its homebase.
 */
static void Robot_default_go_home(int ind)
{
    player			*pl = Players[ind];
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    my_data->robot_mode      = RM_TAKE_OFF;
}

/*
 * A default robot is declaring war (or resetting war).
 */
static void Robot_default_set_war(int ind, int victim_id)
{
    player			*pl = Players[ind];
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    if (victim_id == -1) {
	CLR_BIT(my_data->robot_lock, LOCK_PLAYER);
    } else {
	my_data->robot_lock_id = victim_id;
	SET_BIT(my_data->robot_lock, LOCK_PLAYER);
    }
}

/*
 * Return the id of the player a default robot has war against (or -1).
 */
static int Robot_default_war_on_player(int ind)
{
    player			*pl = Players[ind];
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    if (BIT(my_data->robot_lock, LOCK_PLAYER)) {
	return my_data->robot_lock_id;
    } else {
	return -1;
    }
}

/*
 * A default robot receives a message.
 */
static void Robot_default_message(int ind, char *message)
{
#if 0
    player			*pl = Players[ind];
    robot_default_data_t	*my_data = Robot_default_get_data(pl);
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
    strncpy(sender_name, ptr + 1, sizeof sender_name);	/* copy sender */
    len = ptr - message;	/* find read message length */
    if (message[len] == ' ') {	/* ignore the readability space */
	len--;
    }
    strncpy(msg, message, len);	/* copy real message */
    msg[len] = '\0';		/* make it a string */
    if ((ptr = strchr(sender_name, ']')) != NULL) {
	*ptr = '\0';	/* remove the ']' separator */
    }
    printf("%s got message \"%s\" from \"%s\"\n", pl->name, msg, sender_name);
#endif
}

/*
 * A default robot is destroyed.
 */
static void Robot_default_destroy(int ind)
{
    player			*pl = Players[ind];

    free(pl->robot_data_ptr->private_data);
    pl->robot_data_ptr->private_data = NULL;
}


static bool Check_robot_navigate(int ind, bool * num_evade)
{
    int         i, j, k;
    player     *pl;
    int         area_val[10][10];
    int         calc_val[10][10];
    int         locn_block;
    long        dx, dy;
    int         di, dj;
    bool        found_wall;
    bool        found_grav;
    bool	target_passable;
    bool        near_wall;
    int         best_val;
    int         best_i, best_j;
    float       best_vx, best_vy;
    int         best_dir;
    int         delta_dir;

    pl = Players[ind];

    if (pl->velocity > 2.0 || ABS(pl->vel.x) > 1.5)
	return false;

    for (i = 0; i < 10; i++) {
	for (j = 0; j < 10; j++) {
	    area_val[i][j] = 0;
	}
    }

    found_wall = false;
    found_grav = false;

    for (i = 0; i < 10; i += 2) {
	for (j = 0; j < 10; j += 2) {

	    dx = (long)(pl->pos.x / BLOCK_SZ) + (i / 2) - 2;
	    dy = (long)(pl->pos.y / BLOCK_SZ) + (j / 2) - 2;

	    if (BIT(World.rules->mode, WRAP_PLAY)) {
		if (dx < 0) dx += World.x;
		else if (dx >= World.x) dx -= World.x;
		if (dy < 0) dy += World.y;
		else if (dy >= World.y) dy -= World.y;
	    }
	    if (dx < 0 || dx >= World.x || dy < 0 || dy >= World.y)
		locn_block = FILLED;
	    else
		locn_block = World.block[dx][dy];

	    switch (locn_block) {

	    case SPACE:
	    case BASE:
	    case CHECK:
	    case WORMHOLE:
	    case ITEM_CONCENTRATOR:
		area_val[i][j] = 1;
		area_val[i + 1][j] = 1;
		area_val[i + 1][j + 1] = 1;
		area_val[i][j + 1] = 1;
		break;

	    case REC_LU:
		area_val[i + 1][j] = 1;
		found_wall = true;
		break;

	    case REC_LD:
		area_val[i + 1][j + 1] = 1;
		found_wall = true;
		break;

	    case REC_RU:
		area_val[i][j] = 1;
		found_wall = true;
		break;

	    case REC_RD:
		area_val[i][j + 1] = 1;
		found_wall = true;
		break;

	    case POS_GRAV:
	    case NEG_GRAV:
	    case CWISE_GRAV:
	    case ACWISE_GRAV:
		found_grav = true;
		break;

	    case TARGET:
		if (!targetTeamCollision) {
		    target_passable = false;
		    for (k = 0; ; k++) {
			if (World.targets[k].pos.x == dx
			    && World.targets[k].pos.y == dy) {
			    if (pl->team == World.targets[k].team) {
				target_passable = true;
			    }
			    break;
			}
		    }
		    if (target_passable) {
			area_val[i][j] = 1;
			area_val[i + 1][j] = 1;
			area_val[i + 1][j + 1] = 1;
			area_val[i][j + 1] = 1;
			break;
		    }
		}
		/* FALLTHROUGH */

	    default:
		found_wall = true;
		break;
	    }
	}
    }

    if (found_grav || !found_wall)
	return false;

    /* iterate twice for weighting, central 6x6 square should be accurate */

    for (k = 0; k < 2; k++) {
	for (i = 0; i < 10; i++) {
	    for (j = 0; j < 10; j++) {

		calc_val[i][j] = 0;
		if (area_val[i][j] == 0)
		    continue;

		if (i <= 0 || i >= 9 || j <= 0 || j >= 9)
		    continue;

		calc_val[i][j] += 2 * area_val[i - 1][j];
		calc_val[i][j] += 2 * area_val[i][j + 1];
		calc_val[i][j] += 2 * area_val[i + 1][j];
		calc_val[i][j] += 2 * area_val[i][j - 1];

		calc_val[i][j] += area_val[i - 1][j - 1];
		calc_val[i][j] += area_val[i - 1][j + 1];
		calc_val[i][j] += area_val[i + 1][j - 1];
		calc_val[i][j] += area_val[i + 1][j + 1];
	    }
	}

	for (i = 0; i < 10; i++) {
	    for (j = 0; j < 10; j++) {
		area_val[i][j] = calc_val[i][j];
	    }
	}
    }

    /* now focus in to local 3x3 square */

    dx = (int)pl->pos.x;
    dy = (int)pl->pos.y;

    dx = dx - (dx / BLOCK_SZ * BLOCK_SZ);
    dy = dy - (dy / BLOCK_SZ * BLOCK_SZ);

    di = 3;
    dj = 3;

    if (dx > BLOCK_SZ / 2) {
	di++;
	dx -= BLOCK_SZ / 2;
    }
    if (dy > BLOCK_SZ / 2) {
	dj++;
	dy -= BLOCK_SZ / 2;
    }
    for (i = 0; i < 3; i++) {
	for (j = 0; j < 3; j++) {
	    area_val[i][j] = area_val[di + i][dj + j];
	}
    }

    *num_evade = true;

    if (ABS(pl->vel.x) < 0.5) {

	best_i = 1;
	best_j = (pl->vel.y > 0 ? 2 : 0);

    } else if (ABS(pl->vel.y) < 0.5) {

	best_i = (pl->vel.x > 0 ? 2 : 0);
	best_j = 1;

    } else {

	best_i = (pl->vel.x > 0 ? 2 : 0);
	best_j = (pl->vel.y > 0 ? 2 : 0);
    }

    best_val = area_val[best_i][best_j];
    near_wall = false;

    for (j = 2; j >= 0; j--) {
	for (i = 0; i <= 2; i++) {

	    if (i == 1 && j == 1)
		continue;

	    if (area_val[i][j] == 0) {
		near_wall = true;
		if (i == 1 && (j == 0 || j == 2)) {
		    best_i = 1;
		    best_j = (2 - j);
		    best_val = 99999;
		}
		continue;
	    }
	    if (area_val[i][j] > best_val) {
		best_i = i;
		best_j = j;
		best_val = area_val[i][j];
	    }
	}
    }

    if (area_val[1][1] >= best_val)
	return false;

    if (!near_wall) {
	if (BIT(pl->used, OBJ_REFUEL)) {
	    /* refueling, so hang around */
	    best_i = 1;
	    best_j = 1;
	    best_val = area_val[1][1];
	} else {
	    return false;
	}
    }
    if (best_j == 1) {
	if (dy < BLOCK_SZ / 6)
	    best_j = 2;
	if (dy > BLOCK_SZ / 3)
	    best_j = 0;
    }
    pl->turnspeed = MAX_PLAYER_TURNSPEED;
    pl->power = pl->mass / 2;
    LIMIT(pl->power, MIN_PLAYER_POWER, MAX_PLAYER_POWER);

    best_vx = (best_i - 1) * 0.75;
    best_vy = (best_j - 1) * 1.25;

    if (pl->vel.x > best_vx + 0.75)
	best_dir = 3 * RES / 8;
    else if (pl->vel.x < best_vx - 0.75)
	best_dir = RES / 8;
    else if (pl->vel.x > best_vx + 0.25)
	best_dir = 5 * RES / 16;
    else if (pl->vel.x < best_vx - 0.25)
	best_dir = 3 * RES / 16;
    else
	best_dir = RES / 4;

    delta_dir = best_dir - pl->dir;
    delta_dir = MOD2(delta_dir, RES);

    if (delta_dir > RES / 8 && delta_dir < 7 * RES / 8) {
	pl->turnacc = (delta_dir < RES / 2 ?
		       pl->turnspeed : (-pl->turnspeed));
    } else if (delta_dir > RES / 64 && delta_dir < 63 * RES / 64) {
	pl->turnspeed = MIN_PLAYER_TURNSPEED;
	pl->turnacc = (delta_dir < RES / 2 ?
		       pl->turnspeed : (-pl->turnspeed));
    } else {
	pl->turnacc = 0;
    }

    if (pl->vel.y > best_vy + 0.25) {
	CLR_BIT(pl->status, THRUSTING);
    } else if (pl->vel.y < best_vy - 0.25) {
	SET_BIT(pl->status, THRUSTING);
    }
    return true;
}

static int Find_target_team (int x, int y)
{
    target_t	*targ = World.targets;
    int		t;

    for (t = 0; t < World.NumTargets; t++, targ++)
	if (targ->pos.x == x && targ->pos.y == y)
	    break;
    return targ->team;
}

#define TARGET_PASSABLE_HACK
#ifdef TARGET_PASSABLE_HACK
/*
 * Hack: If we've got targets which are crossable by a team member
 * we assume that all targets in the vicinity have the same team
 * assigned to them to save time.  This is usually true, make sure that
 * targets are always more than twelve blocks apart.  If this isn't true
 * the robot may well evade or hit targets when not supposed to, but
 * its a small price to pay.
 */
static int target_team;

/*
 * Call once per scan of locality
 */
#define TARGET_INIT()	(target_team = TEAM_NOT_SET)

#define TARGET_PASSABLE(dx,dy) \
    (!targetTeamCollision && \
	pl->team == (target_team == TEAM_NOT_SET \
	    ? target_team = Find_target_team(dx,dy) \
	    : target_team))

/*
 * Use REALLY_EMPTY_SPACE() over EMPTY_SPACE() for above HACK
 */
#define REALLY_EMPTY_SPACE(type,dx,dy) \
    (EMPTY_SPACE(type) || (type == TARGET && TARGET_PASSABLE(dx,dy)))

#else
#define TARGET_INIT()
#define REALLY_EMPTY_SPACE(type,dx,dy) EMPTY_SPACE(type)
#endif

static bool Check_robot_evade(int ind, int mine_i, int ship_i)
{
    int				i;
    player			*pl = Players[ind];
    object			*shot;
    player			*ship;
    long			stop_dist;
    bool			evade;
    bool			left_ok, right_ok;
    int				safe_width;
    int				travel_dir;
    int				delta_dir;
    int				aux_dir;
    int				px[3], py[3];
    long			dist;
    int				locn_block;
    vector			*gravity;
    int				gravity_dir;
    long			dx, dy;
    float			velocity;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    safe_width = 3 * SHIP_SZ / 2;
    /* Prevent overflow. */
    velocity = (pl->velocity <= SPEED_LIMIT) ? pl->velocity : SPEED_LIMIT;
    stop_dist =
	(RES * velocity) / (MAX_PLAYER_TURNSPEED * pl->turnresistance)
	+ (velocity * velocity * pl->mass) / (2 * MAX_PLAYER_POWER)
	+ safe_width;
    /*
     * Limit the look ahead.  For very high speeds the current code
     * is ineffective and much too inefficient.
     */
    if (stop_dist > 10 * BLOCK_SZ) {
	stop_dist = 10 * BLOCK_SZ;
    }
    evade = false;

    if (pl->velocity <= 0.2) {
	vector	*grav = &World.gravity
	    [(int)pl->pos.x / BLOCK_SZ][(int)pl->pos.y / BLOCK_SZ];
	travel_dir = findDir(grav->x, grav->y);
    } else {
	travel_dir = findDir(pl->vel.x, pl->vel.y);
    }

    aux_dir = MOD2(travel_dir + RES / 4, RES);
    px[0] = pl->pos.x;		/* ship center x */
    py[0] = pl->pos.y;		/* ship center y */
    px[1] = px[0] + safe_width * tcos(aux_dir);	/* ship left side x */
    py[1] = py[0] + safe_width * tsin(aux_dir);	/* ship left side y */
    px[2] = 2 * px[0] - px[1];	/* ship right side x */
    py[2] = 2 * py[0] - py[1];	/* ship right side y */

    left_ok = true;
    right_ok = true;

    TARGET_INIT();

    for (dist = 0; dist < stop_dist + BLOCK_SZ / 2; dist += BLOCK_SZ / 2) {
	for (i = 0; i < 3; i++) {
	    dx = (px[i] + dist * tcos(travel_dir)) / BLOCK_SZ;
	    dy = (py[i] + dist * tsin(travel_dir)) / BLOCK_SZ;

	    if (BIT(World.rules->mode, WRAP_PLAY)) {
		if (dx < 0) dx += World.x;
		else if (dx >= World.x) dx -= World.x;
		if (dy < 0) dy += World.y;
		else if (dy >= World.y) dy -= World.y;
	    }
	    if (dx < 0 || dx >= World.x || dy < 0 || dy >= World.y) {
		evade = true;
		if (i == 1)
		    left_ok = false;
		if (i == 2)
		    right_ok = false;
		continue;
	    }
	    locn_block = World.block[dx][dy];
	    if (!REALLY_EMPTY_SPACE(locn_block,dx,dy)) {
		evade = true;
		if (i == 1)
		    left_ok = false;
		if (i == 2)
		    right_ok = false;
		continue;
	    }
	    /* Watch out for strong gravity */
	    gravity = &World.gravity[dx][dy];
	    if (sqr(gravity->x) + sqr(gravity->y) >= 1.0) {
		gravity_dir = findDir(gravity->x - pl->pos.x,
				      gravity->y - pl->pos.y);
/* XXX: DOES THIS WORK?  strength of gravity - position?!?! */
		if (MOD2(gravity_dir - travel_dir, RES) <= RES / 4 ||
		    MOD2(gravity_dir - travel_dir, RES) >= 3 * RES / 4) {
		    evade = true;
		    if (i == 1)
			left_ok = false;
		    if (i == 2)
			right_ok = false;
		    continue;
		}
	    }
	}
    }

    if (mine_i >= 0) {
	shot = Obj[mine_i];
	aux_dir = Wrap_findDir(shot->pos.x + shot->vel.x - pl->pos.x,
			       shot->pos.y + shot->vel.y - pl->pos.y);
	delta_dir = MOD2(aux_dir - travel_dir, RES);
	if (delta_dir < RES / 4) {
	    left_ok = false;
	    evade = true;
	}
	if (delta_dir > RES * 3 / 4) {
	    right_ok = false;
	    evade = true;
	}
    }
    if (ship_i >= 0) {
	ship = Players[ship_i];
	aux_dir = Wrap_findDir(ship->pos.x - pl->pos.x + ship->vel.x * 2,
			       ship->pos.y - pl->pos.y + ship->vel.y * 2);
	delta_dir = MOD2(aux_dir - travel_dir, RES);
	if (delta_dir < RES / 4) {
	    left_ok = false;
	    evade = true;
	}
	if (delta_dir > RES * 3 / 4) {
	    right_ok = false;
	    evade = true;
	}
    }
    if (pl->velocity > my_data->robot_max_speed)
	evade = true;

    if (!evade)
	return false;

    delta_dir = 0;
    while (!left_ok && !right_ok && delta_dir < 7 * RES / 8) {
	delta_dir += RES / 16;

	left_ok = true;
	aux_dir = MOD2(travel_dir + delta_dir, RES);
	for (dist = 0; dist < stop_dist + BLOCK_SZ / 2; dist += BLOCK_SZ / 2) {
	    dx = (px[0] + dist * tcos(aux_dir)) / BLOCK_SZ;
	    dy = (py[0] + dist * tsin(aux_dir)) / BLOCK_SZ;

	    if (BIT(World.rules->mode, WRAP_PLAY)) {
		if (dx < 0) dx += World.x;
		else if (dx >= World.x) dx -= World.x;
		if (dy < 0) dy += World.y;
		else if (dy >= World.y) dy -= World.y;
	    }
	    if (dx < 0 || dx >= World.x || dy < 0 || dy >= World.y) {
		left_ok = false;
		continue;
	    }
	    locn_block = World.block[dx][dy];
	    if (!REALLY_EMPTY_SPACE(locn_block,dx,dy)) {
		left_ok = false;
		continue;
	    }
	    /* watch out for strong gravity */
	    gravity = &World.gravity[dx][dy];
	    if (sqr(gravity->x) + sqr(gravity->y) >= 1.0) {
		gravity_dir = findDir(gravity->x - pl->pos.x,
				      gravity->y - pl->pos.y);
		if (MOD2(gravity_dir - travel_dir, RES) <= RES / 4 ||
		    MOD2(gravity_dir - travel_dir, RES) >= 3 * RES / 4) {

		    left_ok = false;
		    continue;
		}
	    }
	}

	right_ok = true;
	aux_dir = MOD2(travel_dir - delta_dir, RES);
	for (dist = 0; dist < stop_dist + BLOCK_SZ / 2; dist += BLOCK_SZ / 2) {
	    dx = (px[0] + dist * tcos(aux_dir)) / BLOCK_SZ;
	    dy = (py[0] + dist * tsin(aux_dir)) / BLOCK_SZ;

	    if (BIT(World.rules->mode, WRAP_PLAY)) {
		if (dx < 0) dx += World.x;
		else if (dx >= World.x) dx -= World.x;
		if (dy < 0) dy += World.y;
		else if (dy >= World.y) dy -= World.y;
	    }
	    if (dx < 0 || dx >= World.x || dy < 0 || dy >= World.y) {
		right_ok = false;
		continue;
	    }
	    locn_block = World.block[dx][dy];
	    if (!REALLY_EMPTY_SPACE(locn_block,dx,dy)) {
		right_ok = false;
		continue;
	    }
	    /* watch out for strong gravity */
	    gravity = &World.gravity[dx][dy];
	    if (sqr(gravity->x) + sqr(gravity->y) >= 1.0) {
		gravity_dir = findDir(gravity->x - pl->pos.x,
				      gravity->y - pl->pos.y);
		if (MOD2(gravity_dir - travel_dir, RES) <= RES / 4 ||
		    MOD2(gravity_dir - travel_dir, RES) >= 3 * RES / 4) {

		    right_ok = false;
		    continue;
		}
	    }
	}
    }

    pl->turnspeed = MAX_PLAYER_TURNSPEED;
    pl->power = MAX_PLAYER_POWER;

    delta_dir = MOD2(pl->dir - travel_dir, RES);

    if (my_data->robot_mode != RM_EVADE_LEFT
	&& my_data->robot_mode != RM_EVADE_RIGHT) {
	if (left_ok && !right_ok)
	    my_data->robot_mode = RM_EVADE_LEFT;
	else if (right_ok && !left_ok)
	    my_data->robot_mode = RM_EVADE_RIGHT;
	else
	    my_data->robot_mode = (delta_dir < RES / 2 ?
			      RM_EVADE_LEFT : RM_EVADE_RIGHT);
    }
    if (delta_dir < 3 * RES / 8 || delta_dir > 5 * RES / 8) {
	pl->turnacc = (my_data->robot_mode == RM_EVADE_LEFT ?
		       pl->turnspeed : (-pl->turnspeed));
	CLR_BIT(pl->status, THRUSTING);
    } else {
	pl->turnacc = 0;
	SET_BIT(pl->status, THRUSTING);
	my_data->robot_mode = (delta_dir < RES/2 ? RM_EVADE_LEFT : RM_EVADE_RIGHT);
    }

    return true;
}

static void Robot_check_new_modifiers(player *pl, modifiers mods)
{
    if (!BIT(World.rules->mode, ALLOW_NUKES))
	mods.nuclear = 0;
    if (!BIT(World.rules->mode, ALLOW_CLUSTERS))
	CLR_BIT(mods.warhead, CLUSTER);
    if (!BIT(World.rules->mode, ALLOW_MODIFIERS)) {
	mods.velocity =
	mods.mini =
	mods.spread =
	mods.power = 0;
	CLR_BIT(mods.warhead, IMPLOSION);
    }
    if (!BIT(World.rules->mode, ALLOW_LASER_MODIFIERS))
	mods.laser = 0;
    pl->mods = mods;
}

static void Choose_weapon_modifier(player *pl, int weapon_type)
{
    int				stock, min;
    modifiers			mods;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    CLEAR_MODS(mods);

    switch (weapon_type) {
    case OBJ_TRACTOR_BEAM:
	Robot_check_new_modifiers(pl, mods);
	return;

    case OBJ_LASER:
	/*
	 * Robots choose non-damage laser settings occasionally.
	 */
	if ((my_data->robot_count % 4) == 0)
	    mods.laser = rand() % (MODS_LASER_MAX+1);
	Robot_check_new_modifiers(pl, mods);
	return;

    case OBJ_SHOT:
	/*
	 * Robots usually use wide beam shots, however they may narrow
	 * the beam occasionally.
	 */
	mods.spread = 0;
	if ((my_data->robot_count % 8) == 0)
	    mods.spread = rand() % (MODS_SPREAD_MAX+1);
	Robot_check_new_modifiers(pl, mods);
	return;

    case OBJ_MINE:
	stock = pl->item[ITEM_MINE];
	min = nukeMinMines;
	break;

    case OBJ_SMART_SHOT:
    case OBJ_HEAT_SHOT:
    case OBJ_TORPEDO:
	stock = pl->item[ITEM_MISSILE];
	min = nukeMinSmarts;
	if ((my_data->robot_count % 8) == 0)
	    mods.power = rand() % (MODS_POWER_MAX+1);
	break;

    default:
	return;
    }

    if (stock >= min) {
	/*
	 * More aggressive robots will choose to use nuclear weapons, this
	 * means you can safely approach wimpy robots... perhaps.
	 */
	if ((my_data->robot_count % 100) <= my_data->attack) {
	    SET_BIT(mods.nuclear, NUCLEAR);
	    if (stock > min && (stock < (2 * min)
				|| (my_data->robot_count % 2) == 0))
		    SET_BIT(mods.nuclear, FULLNUCLEAR);
	}
    }

    if (pl->fuel.sum > pl->fuel.l3) {
	if ((my_data->robot_count % 2) == 0) {
	    if ((my_data->robot_count % 8) == 0)
		mods.velocity = (rand() % MODS_VELOCITY_MAX) + 1;
	    SET_BIT(mods.warhead, CLUSTER);
	}
    }
    else if ((my_data->robot_count % 4) == 0) {
	SET_BIT(mods.warhead, IMPLOSION);
    }

    /*
     * Robot may change to use mini device setting occasionally.
     */
    if ((my_data->robot_count % 32) == 0) {
	mods.mini = rand() % (MODS_MINI_MAX+1);
	mods.spread = rand() % (MODS_SPREAD_MAX+1);
    }

    Robot_check_new_modifiers(pl, mods);
}

static bool Check_robot_target(int ind,
			       int item_x, int item_y,
			       int new_mode, int attack_level)
{
    player			*pl = Players[ind],
				*ship;
    long			item_dist;
    int				item_dir;
    int				travel_dir;
    int				delta_dir;
    long			dx, dy;
    long			dist;
    int				locn_block;
    bool			clear_path;
    bool			slowing;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);


    dx = item_x - pl->pos.x, dx = WRAP_DX(dx);
    dy = item_y - pl->pos.y, dy = WRAP_DY(dy);

    item_dist = LENGTH(dy, dx);

    if (dx == 0 && dy == 0) {
	vector	*grav = &World.gravity
	    [(int)pl->pos.x / BLOCK_SZ][(int)pl->pos.y / BLOCK_SZ];
	item_dir = findDir(grav->x, grav->y);
	item_dir = MOD2(item_dir + RES/2, RES);
    } else {
	item_dir = findDir(dx, dy);
    }

    if (new_mode == RM_REFUEL || new_mode == RM_CANNON_KILL)
	item_dist -= 2 * BLOCK_SZ;

    clear_path = true;

    TARGET_INIT();

    for (dist = 0; clear_path && dist < item_dist; dist += BLOCK_SZ / 2) {

	dx = (pl->pos.x + dist * tcos(item_dir)) / BLOCK_SZ;
	dy = (pl->pos.y + dist * tsin(item_dir)) / BLOCK_SZ;

	if (BIT(World.rules->mode, WRAP_PLAY)) {
	    if (dx < 0) dx += World.x;
	    else if (dx >= World.x) dx -= World.x;
	    if (dy < 0) dy += World.y;
	    else if (dy >= World.y) dy -= World.y;
	}
	if (dx < 0 || dx >= World.x || dy < 0 || dy >= World.y) {
	    clear_path = false;
	    continue;
	}
	locn_block = World.block[dx][dy];

	if (!REALLY_EMPTY_SPACE(locn_block,dx,dy) && locn_block != CANNON) {
	    clear_path = false;
	    continue;
	}
    }

    if (!clear_path)
	return false;

    if (pl->velocity <= 0.2) {
	vector	*grav = &World.gravity
	    [(int)pl->pos.x / BLOCK_SZ][(int)pl->pos.y / BLOCK_SZ];
	travel_dir = findDir(grav->x, grav->y);
    } else {
	travel_dir = findDir(pl->vel.x, pl->vel.y);
    }

    pl->turnspeed = MAX_PLAYER_TURNSPEED / 2;
    pl->power = MAX_PLAYER_POWER / 2;

    delta_dir = MOD2(item_dir - travel_dir, RES);
    if (delta_dir >= RES/4 && delta_dir <= 3*RES/4) {

	if (new_mode == RM_HARVEST) {	/* reverse direction of travel */
	    item_dir = MOD2(travel_dir + RES / 2, RES);
	}
	pl->turnspeed = MAX_PLAYER_TURNSPEED;
	slowing = true;

	if (pl->item[ITEM_MINE] && item_dist < 8 * BLOCK_SZ) {
	    Choose_weapon_modifier(pl, OBJ_MINE);
	    Place_moving_mine(ind);
	    new_mode = (rand() & 1) ? RM_EVADE_RIGHT : RM_EVADE_LEFT;
	}
    } else if (new_mode == RM_CANNON_KILL && item_dist <= 0) {

	/* too close, to move away */
	pl->turnspeed = MAX_PLAYER_TURNSPEED;
	item_dir = MOD2(item_dir + RES / 2, RES);
	slowing = true;
    } else {

	slowing = false;
    }

    delta_dir = MOD2(item_dir - pl->dir, RES);

    if (delta_dir > RES / 8 && delta_dir < 7 * RES / 8) {
	pl->turnacc = (delta_dir < RES / 2 ?
		       pl->turnspeed : (-pl->turnspeed));
    } else if ((delta_dir > RES / 16 && delta_dir < 15 * RES / 8)
	       || (my_data->robot_count % 8) == 0) {

	pl->turnspeed = MAX_PLAYER_TURNSPEED / 2;
	pl->turnacc = (delta_dir < RES / 2
		       ? pl->turnspeed : (-pl->turnspeed));
    } else if (delta_dir > RES / 64 && delta_dir < 63 * RES / 64) {

	pl->turnspeed = MAX_PLAYER_TURNSPEED / 2;
	pl->turnacc = (delta_dir < RES / 2 ?
		       pl->turnspeed : (-pl->turnspeed));
    } else {
	pl->turnacc = 0.0;
    }

    if (slowing || BIT(pl->used, OBJ_SHIELD)) {

	SET_BIT(pl->status, THRUSTING);

    } else if (item_dist < 0) {

	CLR_BIT(pl->status, THRUSTING);

    } else if (item_dist < 3*BLOCK_SZ) {

	if (pl->velocity < my_data->robot_normal_speed / 2)
	    SET_BIT(pl->status, THRUSTING);
	if (pl->velocity > my_data->robot_normal_speed)
	    CLR_BIT(pl->status, THRUSTING);

    } else if (new_mode != RM_ATTACK) {

	if (pl->velocity < 2*my_data->robot_normal_speed)
	    SET_BIT(pl->status, THRUSTING);
	if (pl->velocity > 3*my_data->robot_normal_speed)
	    CLR_BIT(pl->status, THRUSTING);

    } else {

	if (pl->velocity < my_data->robot_attack_speed / 2)
	    SET_BIT(pl->status, THRUSTING);
	if (pl->velocity > my_data->robot_attack_speed)
	    CLR_BIT(pl->status, THRUSTING);
    }

    if (new_mode == RM_ATTACK) {
	if (pl->item[ITEM_ECM] > 0 && item_dist < ECM_DISTANCE / 4) {
	    Fire_ecm(ind);
	}
	else if (pl->item[ITEM_TRANSPORTER] > 0
		 && item_dist < TRANSPORTER_DISTANCE
		 && pl->fuel.sum > -ED_TRANSPORTER) {
	    do_transporter(pl);
	    pl->item[ITEM_TRANSPORTER]--;
	    Add_fuel(&(pl->fuel), ED_TRANSPORTER);
	}
	else if (pl->item[ITEM_LASER] > pl->num_pulses
		 && -ED_LASER < pl->fuel.sum - pl->fuel.l3) {
	    if (BIT(my_data->robot_lock, LOCK_PLAYER)
		&& BIT(Players[GetInd[my_data->robot_lock_id]]->status,
		       PLAYING|PAUSE|GAME_OVER) == PLAYING) {
		ship = Players[GetInd[my_data->robot_lock_id]];
	    }
	    else if (pl->shot_max <= 0 && BIT(pl->lock.tagged, LOCK_PLAYER)) {
		ship = Players[GetInd[pl->lock.pl_id]];
	    }
	    else {
		ship = NULL;
	    }
	    if (ship
		&& BIT(ship->status, PLAYING|PAUSE|GAME_OVER) == PLAYING) {

		float	x1, y1, x3, y3, x4, y4, x5, y5;
		float	ship_dist, dir3, dir4, dir5;

		x1 = pl->pos.x + pl->vel.x + pl->ship->m_gun[pl->dir].x;
		y1 = pl->pos.y + pl->vel.y + pl->ship->m_gun[pl->dir].y;
		x3 = ship->pos.x + ship->vel.x;
		y3 = ship->pos.y + ship->vel.y;

		ship_dist = Wrap_length(x3 - x1, y3 - y1);

		if (ship_dist < PULSE_SPEED*PULSE_LIFE(pl->item[ITEM_LASER]) + SHIP_SZ) {
		    dir3 = Wrap_findDir(x3 - x1, y3 - y1);
		    x4 = x3 + tcos(MOD2((int)(dir3 - RES/4), RES)) * SHIP_SZ;
		    y4 = y3 + tsin(MOD2((int)(dir3 - RES/4), RES)) * SHIP_SZ;
		    x5 = x3 + tcos(MOD2((int)(dir3 + RES/4), RES)) * SHIP_SZ;
		    y5 = y3 + tsin(MOD2((int)(dir3 + RES/4), RES)) * SHIP_SZ;
		    dir4 = Wrap_findDir(x4 - x1, y4 - y1);
		    dir5 = Wrap_findDir(x5 - x1, y5 - y1);
		    if ((dir4 > dir5)
			? (pl->dir >= dir4 || pl->dir <= dir5)
			: (pl->dir >= dir4 && pl->dir <= dir5)) {
			SET_BIT(pl->used, OBJ_LASER);
		    }
		}
	    }
	}
	else if (BIT(pl->have, OBJ_TRACTOR_BEAM)) {
	    CLR_BIT(pl->used, OBJ_TRACTOR_BEAM);
	    pl->tractor_pressor = 0;

	    if (BIT(pl->lock.tagged, LOCK_PLAYER)
		&& pl->fuel.sum > pl->fuel.l3
		&& pl->lock.distance < TRACTOR_MAX_RANGE(pl)) {

		float xvd, yvd, vel;
		long dir;
		int away;

		ship = Players[GetInd[pl->lock.pl_id]];
		xvd = ship->vel.x - pl->vel.x;
		yvd = ship->vel.y - pl->vel.y;
		vel = LENGTH(xvd, yvd);
		dir = (long)(findDir(pl->pos.x - ship->pos.x,
				     pl->pos.y - ship->pos.y)
			     - findDir(xvd, yvd));
		dir = MOD2(dir, RES);
		away = (dir >= RES/4 && dir <= 3*RES/4);

		/*
		 * vel  - The relative velocity of ship to us.
		 * away - Heading away from us?
		 */
		if (pl->velocity <= my_data->robot_normal_speed) {
		    if (pl->lock.distance < (SHIP_SZ * 4)
			|| (!away && vel > my_data->robot_attack_speed)) {
			SET_BIT(pl->used, OBJ_TRACTOR_BEAM);
			pl->tractor_pressor = 1;
		    } else if (away
			       && vel < my_data->robot_max_speed
			       && vel > my_data->robot_normal_speed) {
			SET_BIT(pl->used, OBJ_TRACTOR_BEAM);
		    }
		}
		if (BIT(pl->used, OBJ_TRACTOR_BEAM))
		    SET_BIT(pl->lock.tagged, LOCK_VISIBLE);
	    }
	}
	if (BIT(pl->used, OBJ_LASER)) {
	    pl->turnacc = 0.0;
	    Choose_weapon_modifier(pl, OBJ_LASER);
	}
	else if ((my_data->robot_count % 10) == 0 && pl->item[ITEM_MISSILE] > 0) {
	    int type;

	    switch (my_data->robot_count % 5) {
	    case 0: case 1: case 2:	type = OBJ_SMART_SHOT; break;
	    case 3:			type = OBJ_HEAT_SHOT; break;
	    default:			type = OBJ_TORPEDO; break;
	    }
	    Choose_weapon_modifier(pl, type);
	    Fire_shot(ind, type, pl->dir);
	}
	else if ((my_data->robot_count % 2) == 0
		   && item_dist < VISIBILITY_DISTANCE
		   && (BIT(my_data->robot_lock, LOCK_PLAYER) /* robot has target */
		       || (my_data->robot_count
			   % (110 - my_data->attack))
		       < 15+attack_level)) {
	    if (pl->item[ITEM_MISSILE] > 0 && (rand() & 63) == 0) {
		Choose_weapon_modifier(pl, OBJ_SMART_SHOT);
		Fire_shot(ind, OBJ_SMART_SHOT, pl->dir);
	    } else {
		Choose_weapon_modifier(pl, OBJ_SHOT);
		Fire_normal_shots(ind);
	    }
	}
	if ((my_data->robot_count % 32) == 0) {
	    if (pl->fuel.sum > pl->fuel.l3) {
		Choose_weapon_modifier(pl, OBJ_MINE);
		Place_mine(ind);
	    } else if (pl->fuel.sum < pl->fuel.l2) {
		Place_mine(ind);
		CLR_BIT(pl->used, OBJ_CLOAKING_DEVICE);
	    }
	}
    }
    if (new_mode == RM_CANNON_KILL && !slowing) {
	if ((my_data->robot_count % 2) == 0 && item_dist < VISIBILITY_DISTANCE) {
	    Choose_weapon_modifier(pl, OBJ_SHOT);
	    Fire_normal_shots(ind);
	}
    }
    my_data->robot_mode = new_mode;
    return true;
}


static bool Check_robot_hunt(int ind)
{
    player			*pl = Players[ind];
    player			*ship;
    int				ship_dir;
    int				travel_dir;
    int				delta_dir;
    int				adj_dir;
    int				toofast, tooslow;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);

    if (!BIT(my_data->robot_lock, LOCK_PLAYER)
	|| my_data->robot_lock_id == pl->id)
	return false;
    if (pl->fuel.sum < MAX_PLAYER_FUEL/2)
	return false;

    ship = Players[GetInd[my_data->robot_lock_id]];

    ship_dir = Wrap_findDir(ship->pos.x - pl->pos.x, ship->pos.y - pl->pos.y);

    if (pl->velocity <= 0.2) {
	vector	*grav = &World.gravity
	    [(int)pl->pos.x / BLOCK_SZ][(int)pl->pos.y / BLOCK_SZ];
	travel_dir = findDir(grav->x, grav->y);
    } else {
	travel_dir = findDir(pl->vel.x, pl->vel.y);
    }

    delta_dir = MOD2(ship_dir - travel_dir, RES);
    tooslow = (pl->velocity < my_data->robot_attack_speed/2);
    toofast = (pl->velocity > my_data->robot_attack_speed);

    if (!tooslow && !toofast
	&& (delta_dir <= RES/16 || delta_dir >= 15*RES/16)) {

	pl->turnacc = 0;
	CLR_BIT(pl->status, THRUSTING);
	my_data->robot_mode = RM_ROBOT_IDLE;
	return true;
    }

    adj_dir = (delta_dir<RES/2 ? RES/4 : (-RES/4));

    if (tooslow) adj_dir = adj_dir/2;	/* point forwards more */
    if (toofast) adj_dir = 3*adj_dir/2;	/* point backwards more */

    adj_dir = MOD2(travel_dir + adj_dir, RES);
    delta_dir = MOD2(adj_dir - pl->dir, RES);

    if (delta_dir>=RES/16 && delta_dir<=15*RES/16) {
	pl->turnspeed = MAX_PLAYER_TURNSPEED/4;
	pl->turnacc = (delta_dir<RES/2 ? pl->turnspeed : (-pl->turnspeed));
    }

    if (delta_dir<RES/8 || delta_dir>7*RES/8) {
	SET_BIT(pl->status, THRUSTING);
    } else {
	CLR_BIT(pl->status, THRUSTING);
    }

    my_data->robot_mode = RM_ROBOT_IDLE;
    return true;
}


static void Robot_default_play(int ind)
{
    player			*pl = Players[ind],
				*ship;
    object			*shot;
    float			distance, mine_dist, item_dist, ship_dist,
				enemy_dist, cannon_dist, fuel_dist, target_dist,
				fx, fy, speed, x_speed, y_speed;
    int				j, mine_i, item_i, ship_i,
				enemy_i, cannon_i, fuel_i, target_i, x, y;
    long			dx, dy;
    long			killing_shots;
    bool			harvest_checked;
    bool			fuel_checked;
    bool			evade_checked;
    int				attack_level;
    int				shoot_time;
    int				shield_range;
    robot_default_data_t	*my_data = Robot_default_get_data(pl);


    killing_shots = KILLING_SHOTS;
    if (treasureCollisionMayKill) {
	killing_shots |= OBJ_BALL;
    }

    if (my_data->robot_count <= 0)
	my_data->robot_count = 1000 + rand() % 32;

    my_data->robot_count--;

    CLR_BIT(pl->used, OBJ_SHIELD | OBJ_CLOAKING_DEVICE | OBJ_LASER);
    if (BIT(pl->have, OBJ_EMERGENCY_SHIELD)
	&& !BIT(pl->used, OBJ_EMERGENCY_SHIELD)) {
	Emergency_shield(ind, 1);
    }
    harvest_checked = false;
    fuel_checked = false;
    evade_checked = false;

    mine_i = -1;
    mine_dist = SHIP_SZ + 200;
    item_i = -1;
    item_dist = VISIBILITY_DISTANCE;

    if (BIT(pl->have, OBJ_CLOAKING_DEVICE) && pl->fuel.sum > pl->fuel.l3)
	SET_BIT(pl->used, OBJ_CLOAKING_DEVICE);

    for (j = 0; j < NumObjs; j++) {

	shot = Obj[j];

	/* Get rid of the most common object types first for speed. */
	if (BIT(shot->type, OBJ_DEBRIS|OBJ_SPARK)) {
	    continue;
	}
	if (BIT(shot->type, OBJ_ITEM)) {
	    if ((dx = shot->pos.x - pl->pos.x,
		    dx = WRAP_DX(dx),
		    ABS(dx)) < item_dist
		&& (dy = shot->pos.y - pl->pos.y,
		    dy = WRAP_DY(dy),
		    ABS(dy)) < item_dist
		&& (distance = LENGTH(dx, dy)) < item_dist) {
		item_i = j;
		item_dist = distance;
	    }
	    continue;
	}
	if (BIT(shot->type, OBJ_SMART_SHOT|OBJ_HEAT_SHOT|OBJ_MINE)) {
	    fx = shot->pos.x - pl->pos.x;
	    fy = shot->pos.y - pl->pos.y;
	    if ((dx = fx, dx = WRAP_DX(dx), ABS(dx)) < mine_dist
		&& (dy = fy, dy = WRAP_DY(dy), ABS(dy)) < mine_dist
		&& (distance = LENGTH(dx, dy)) < mine_dist) {
		mine_i = j;
		mine_dist = distance;
	    }
	    if ((dx = fx + (shot->vel.x - pl->vel.x) * ROB_LOOK_AH,
		    dx = WRAP_DX(dx), ABS(dx)) < mine_dist
		&& (dy = fy + (shot->vel.y - pl->vel.y) * ROB_LOOK_AH,
		    dy = WRAP_DY(dy), ABS(dy)) < mine_dist
		&& (distance = LENGTH(dx, dy)) < mine_dist) {
		mine_i = j;
		mine_dist = distance;
	    }
	}

	/*
	 * The only thing left to do regarding objects is to check if
	 * this robot needs to put up shields to protect against objects.
	 */
	if (!BIT(shot->type, killing_shots)) {
	    continue;
	}

	/*
	 * Any shot of team members excluding self are passive.
	 */
	if (shot->id >= 0
	    && TEAM_IMMUNE(GetInd[shot->id], ind)
	    && shot->id != pl->id) {
	    continue;
	}

	shield_range = 20 + SHIP_SZ + shot->pl_range;

	if ((dx = shot->pos.x + shot->vel.x - pl->pos.x + pl->vel.x,
		dx = WRAP_DX(dx),
		ABS(dx)) < shield_range
	    && (dy = shot->pos.y + shot->vel.y - pl->pos.y + pl->vel.y,
		dy = WRAP_DY(dy),
		ABS(dy)) < shield_range
	    && sqr(dx) + sqr(dy) <= sqr(shield_range)
	    && (my_data->robot_count % 100 < ((my_data->defense + 700) / 8)
		|| shot->id == -1
		|| shot->id == pl->id
	     /* || Players[GetInd[shot->id]]->score > 50	*/
		)) {
	    SET_BIT(pl->used, OBJ_SHIELD);
	    SET_BIT(pl->status, THRUSTING);

	    if (BIT(shot->type, OBJ_SMART_SHOT)) {
		if (mine_dist < ECM_DISTANCE / 4)
		    Fire_ecm(ind);
	    }
	    if (BIT(shot->type, OBJ_MINE)) {
		if (mine_dist < ECM_DISTANCE / 2)
		    Fire_ecm(ind);
	    }
	}
    }


    /*
     * Test if others are firing lasers at us.
     * Maybe move this into the player loop.
     */
    if (BIT(pl->used, OBJ_SHIELD) == 0
	&& BIT(pl->have, OBJ_SHIELD) != 0) {
	for (j = 0; j < NumPlayers; j++) {
	    ship = Players[j];
	    if (j == ind
		|| BIT(ship->status, PLAYING|PAUSE|GAME_OVER) != PLAYING)
		continue;
	    if (ship->num_pulses > 0) {
		distance = Wrap_length(pl->pos.x - ship->pos.x,
				       pl->pos.y - ship->pos.y);
		if (PULSE_SPEED*PULSE_LIFE(ship->item[ITEM_LASER]) + 2*SHIP_SZ
		    >= distance) {
		    int delta_dir = Wrap_findDir(pl->pos.x - ship->pos.x,
						 pl->pos.y - ship->pos.y),
			ship_dir = ship->dir + (ship->turnvel
			    + ship->turnacc) * ship->turnresistance;
		    if ((delta_dir - ship_dir < 0)
			? (ship_dir - delta_dir < RES/8
			    || delta_dir + RES - ship_dir < RES/8)
			: (delta_dir - ship_dir < RES/8
			    || ship_dir + RES - delta_dir < RES/8)) {
			SET_BIT(pl->used, OBJ_SHIELD);
			break;
		    }
		}
	    }
	}
    }

    /* Note: Only take time to navigate if not being shot at */
    if (!(BIT(pl->used, OBJ_SHIELD) && SET_BIT(pl->status, THRUSTING))
	&& Check_robot_navigate(ind, &evade_checked)) {
	if (playerShielding == 0
	    && playerStartsShielded != 0
	    && BIT(pl->have, OBJ_SHIELD)) {
	    SET_BIT(pl->used, OBJ_SHIELD);
	}
	return;
    }

    ship_i = -1;
    ship_dist = (pl->fuel.sum >= pl->fuel.l1 ? SHIP_SZ * 6 : 0);
    enemy_i = -1;
    if (pl->fuel.sum >= pl->fuel.l3)
	enemy_dist = (BIT(World.rules->mode, LIMITED_VISIBILITY) ?
		      MAX(pl->fuel.sum * ENERGY_RANGE_FACTOR,
			  VISIBILITY_DISTANCE)
		      : ((float)World.hypotenuse));
    else
	enemy_dist = VISIBILITY_DISTANCE;

    if (BIT(pl->used, OBJ_SHIELD))
	ship_dist = 0;

    for (j = 0; j < NumPlayers; j++) {
	ship = Players[j];
	if (j == ind
	    || BIT(ship->status, PLAYING|GAME_OVER|PAUSE) != PLAYING
	    || TEAM_IMMUNE(ind, j))
	    continue;

	dx = ship->pos.x - pl->pos.x, dx = WRAP_DX(dx);
	dy = ship->pos.y - pl->pos.y, dy = WRAP_DY(dy);
	distance = LENGTH(dx, dy);

	if (distance < ship_dist) {
	    ship_i = j;
	    ship_dist = distance;
	}

	if (BIT(my_data->robot_lock, LOCK_PLAYER)
	    && BIT(Players[GetInd[my_data->robot_lock_id]]->status,
		   PLAYING|PAUSE|GAME_OVER) == PLAYING) {
	    /* ignore all players unless target */
	    if (my_data->robot_lock_id == ship->id
		&& distance < enemy_dist) {

		enemy_i = j;
		enemy_dist = distance;
	    }
	} else {
	    if (IS_HUMAN_PTR(ship)
		&& distance < enemy_dist) {
		enemy_i    = j;
		enemy_dist = distance;
	    }
	}
    }

    if (ship_dist <= 3*SHIP_SZ && BIT(pl->have, OBJ_SHIELD))
	SET_BIT(pl->used, OBJ_SHIELD);

    if (BIT(my_data->robot_lock, LOCK_PLAYER)
	&& ship_i != -1
	&& my_data->robot_lock_id == Players[ship_i]->id)
	ship_i = -1; /* don't avoid target */

    if (BIT(pl->lock.tagged, LOCK_PLAYER)) {
	ship = Players[GetInd[pl->lock.pl_id]];
	if (BIT(ship->status, PLAYING|PAUSE|GAME_OVER) != PLAYING
	    || (BIT(my_data->robot_lock, LOCK_PLAYER)
		&& my_data->robot_lock_id != pl->lock.pl_id
		&& BIT(Players[GetInd[my_data->robot_lock_id]]->status,
		       PLAYING|PAUSE|GAME_OVER) == PLAYING)
	    || pl->lock.distance > 2 * VISIBILITY_DISTANCE
	    || (!(pl->visibility[GetInd[pl->lock.pl_id]].canSee)
		&& (my_data->robot_count % 25) == 0)) {
	    /* unset the player lock */
	    CLR_BIT(pl->lock.tagged, LOCK_PLAYER);
	    pl->lock.pl_id = 1;
	    pl->lock.pos.x = pl->pos.x;
	    pl->lock.pos.y = pl->pos.y;
	    pl->lock.distance = 0;
	}
    }
    if (enemy_i >= 0) {
	ship = Players[enemy_i];
	if (!BIT(pl->lock.tagged, LOCK_PLAYER)
	    || enemy_dist < 3*pl->lock.distance/4) {
	    pl->lock.pl_id = ship->id;
	    SET_BIT(pl->lock.tagged, LOCK_PLAYER);
	    pl->lock.pos.x = ship->pos.x;
	    pl->lock.pos.y = ship->pos.y;
	    pl->lock.distance = enemy_dist;
	    pl->sensor_range = VISIBILITY_DISTANCE;
	}
    }
    if (!evade_checked) {
	if (Check_robot_evade(ind, mine_i, ship_i)) {
	    if (playerShielding == 0
		&& playerStartsShielded != 0
		&& BIT(pl->have, OBJ_SHIELD)) {
		SET_BIT(pl->used, OBJ_SHIELD);
	    }
	    else if (maxShieldedWallBounceSpeed >
		    maxUnshieldedWallBounceSpeed
		&& maxShieldedWallBounceAngle >=
		    maxUnshieldedWallBounceAngle
		&& BIT(pl->have, OBJ_SHIELD)) {
		SET_BIT(pl->used, OBJ_SHIELD);
	    }
	    return;
	}
    }
    if (item_i >= 0 && enemy_dist > /* 2* */ item_dist
	/* && enemy_dist > 12*BLOCK_SZ */ ) {

	harvest_checked = true;
	dx = Obj[item_i]->pos.x;
	dy = Obj[item_i]->pos.y + SHIP_SZ / 2;

	if (Check_robot_target(ind, dx, dy, RM_HARVEST, 0)) {
	    return;
	}
    }
    if (BIT(pl->lock.tagged, LOCK_PLAYER)) {

	ship = Players[GetInd[pl->lock.pl_id]];
	shoot_time = pl->lock.distance / (pl->shot_speed + 1);
	dx = ship->pos.x + ship->vel.x * shoot_time;
	dy = ship->pos.y + ship->vel.y * shoot_time + SHIP_SZ / 2;
	attack_level = MAX(ship->score / 8, 0);

	if (Check_robot_target(ind, dx, dy, RM_ATTACK, attack_level)) {
	    return;
	}
    }
    if (item_i >= 0 && !harvest_checked) {

	dx = Obj[item_i]->pos.x;
	dy = Obj[item_i]->pos.y + SHIP_SZ / 2;

	if (Check_robot_target(ind, dx, dy, RM_HARVEST, 0)) {
	    return;
	}
    }

    if (Check_robot_hunt(ind)) {
	if (playerShielding == 0
	    && playerStartsShielded != 0
	    && BIT(pl->have, OBJ_SHIELD)) {
	    SET_BIT(pl->used, OBJ_SHIELD);
	}
	return;
    }

    cannon_i = -1;
    cannon_dist = VISIBILITY_DISTANCE;
    fuel_i = -1;
    fuel_dist = VISIBILITY_DISTANCE;
    target_i = -1;
    target_dist = VISIBILITY_DISTANCE;

    for (j = 0; j < World.NumCannons; j++) {

	if (World.cannon[j].dead_time > 0)
	    continue;

	if ((dx = World.cannon[j].pos.x*BLOCK_SZ + BLOCK_SZ/2 - pl->pos.x,
		dx = WRAP_DX(dx), ABS(dx)) < cannon_dist
	    && (dy = World.cannon[j].pos.y*BLOCK_SZ+BLOCK_SZ/2-pl->pos.y,
		dy = WRAP_DY(dy), ABS(dy)) < cannon_dist
	    && (distance = LENGTH(dx, dy)) < cannon_dist) {
	    cannon_i = j;
	    cannon_dist = distance;
	}
    }

    for (j = 0; j < World.NumFuels; j++) {

	if (World.fuel[j].fuel < 100 * FUEL_SCALE_FACT
	    || pl->fuel.sum >= MAX_PLAYER_FUEL - 200 * FUEL_SCALE_FACT)
	    continue;

	if ((dx = World.fuel[j].pix_pos.x - pl->pos.x,
		dx = WRAP_DX(dx), ABS(dx)) < fuel_dist
	    && (dy = World.fuel[j].pix_pos.y - pl->pos.y,
		dy = WRAP_DY(dy), ABS(dy)) < fuel_dist
	    && (distance = LENGTH(dx, dy)) < fuel_dist) {
	    if (World.block[World.fuel[j].blk_pos.x]
			   [World.fuel[j].blk_pos.y] == FUEL) {
		fuel_i = j;
		fuel_dist = distance;
	    }
	}
    }

    for (j = 0; j < World.NumTargets; j++) {

	/* Ignore dead or owned targets */
	if (World.targets[j].dead_time > 0
	    || pl->team == World.targets[j].team)
	    continue;

	if ((dx = World.targets[j].pos.x*BLOCK_SZ + BLOCK_SZ/2 - pl->pos.x,
		dx = WRAP_DX(dx), ABS(dx)) < target_dist
	    && (dy = World.targets[j].pos.y*BLOCK_SZ+BLOCK_SZ/2-pl->pos.y,
		dy = WRAP_DY(dy), ABS(dy)) < target_dist
	    && (distance = LENGTH(dx, dy)) < target_dist) {
	    target_i = j;
	    target_dist = distance;
	}
    }

    if (fuel_i >= 0 && cannon_dist > fuel_dist) {

	fuel_checked = true;
	dx = World.fuel[fuel_i].pix_pos.x;
	dy = World.fuel[fuel_i].pix_pos.y + SHIP_SZ / 2;

	SET_BIT(pl->used, OBJ_REFUEL);
	pl->fs = fuel_i;

	if (Check_robot_target(ind, dx, dy, RM_REFUEL, 0)) {
	    return;
	}
    }
    if (cannon_i >= 0) {

	dx = World.cannon[cannon_i].pos.x * BLOCK_SZ + BLOCK_SZ / 2;
	dy = World.cannon[cannon_i].pos.y * BLOCK_SZ + BLOCK_SZ / 2;

	if (Check_robot_target(ind, dx, dy, RM_CANNON_KILL, 0)) {
	    return;
	}
    }
    if (target_i >= 0) {
	dx = World.targets[target_i].pos.x * BLOCK_SZ + BLOCK_SZ / 2;
	dy = World.targets[target_i].pos.y * BLOCK_SZ + BLOCK_SZ / 2;

	if (Check_robot_target(ind, dx, dy, RM_CANNON_KILL, 0)) {
	    return;
	}
    }

    if (fuel_i >= 0 && !fuel_checked) {

	dx = World.fuel[fuel_i].pix_pos.x;
	dy = World.fuel[fuel_i].pix_pos.y + SHIP_SZ / 2;

	SET_BIT(pl->used, OBJ_REFUEL);
	pl->fs = fuel_i;

	if (Check_robot_target(ind, dx, dy, RM_REFUEL, 0))
	    return;
    }
    if (pl->fuel.sum < DEFAULT_PLAYER_FUEL)
	Add_fuel(&(pl->fuel), (int)(FUEL_SCALE_FACT * 0.02));
    if (pl->fuel.sum < MIN_PLAYER_FUEL)
	Add_fuel(&(pl->fuel), pl->fuel.sum - MIN_PLAYER_FUEL);

    if (playerShielding == 0
	&& playerStartsShielded != 0
	&& BIT(pl->have, OBJ_SHIELD)) {
	SET_BIT(pl->used, OBJ_SHIELD);
    }

    x = pl->pos.x/BLOCK_SZ;
    y = pl->pos.y/BLOCK_SZ;
    LIMIT(x, 0, World.x);
    LIMIT(y, 0, World.y);
    x_speed = pl->vel.x - 2 * World.gravity[x][y].x;
    y_speed = pl->vel.y - 2 * World.gravity[x][y].y;
    speed = LENGTH(x_speed, y_speed);

    if (y_speed < (-my_data->robot_normal_speed) || (my_data->robot_count % 64) < 32) {

	my_data->robot_mode = RM_ROBOT_CLIMB;
	pl->turnspeed = MAX_PLAYER_TURNSPEED / 2;
	pl->power = MAX_PLAYER_POWER / 2;
	if (ABS(pl->dir - RES / 4) > RES / 16) {
	    pl->turnacc = (pl->dir < RES / 4
			   || pl->dir >= 3 * RES / 4
			   ? pl->turnspeed : (-pl->turnspeed));
	} else {
	    pl->turnacc = 0.0;
	}
	if (y_speed < my_data->robot_normal_speed / 2
	    && pl->velocity < my_data->robot_attack_speed)
	    SET_BIT(pl->status, THRUSTING);
	else if (y_speed > my_data->robot_normal_speed)
	    CLR_BIT(pl->status, THRUSTING);
	return;
    }
    /* must be idle */
    my_data->robot_mode = RM_ROBOT_IDLE;
    pl->turnspeed = MAX_PLAYER_TURNSPEED / 2;
    pl->turnacc = 0;
    pl->power = MAX_PLAYER_POWER / 2;
    CLR_BIT(pl->status, THRUSTING);
    if (speed < my_data->robot_normal_speed / 2)
	SET_BIT(pl->status, THRUSTING);
    else if (speed > my_data->robot_normal_speed)
	CLR_BIT(pl->status, THRUSTING);
}


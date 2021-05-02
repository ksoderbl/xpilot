/* robot.h,v 1.3 1992/06/28 05:38:28 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#define	NORMAL_ROBOT_SPEED	3.0
#define	ATTACK_ROBOT_SPEED	12.0
#define MAX_ROBOT_SPEED		20.0

#define RM_OBJECT               255
#define RM_NOT_ROBOT          	0
#define RM_ROBOT_IDLE         	1
#define RM_EVADE_LEFT         	2
#define RM_EVADE_RIGHT          3
#define RM_ROBOT_CLIMB          4
#define RM_HARVEST            	5
#define RM_ATTACK             	6
#define RM_TAKE_OFF           	7
#define RM_CANNON_KILL		8
#define RM_REFUEL		9
#define RM_NAVIGATE		10

typedef struct {
    char	*name;
    int		attack,		/* Attack + defense ~ 100 */
    		defense;
} robot_t;

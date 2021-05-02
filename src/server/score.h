/* $Id: score.h,v 5.2 2001/05/18 13:49:50 bertg Exp $
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

#ifndef SCORE_H
#define SCORE_H

#define ED_SHOT			(-0.2*FUEL_SCALE_FACT)
#define ED_SMART_SHOT		(-30*FUEL_SCALE_FACT)
#define ED_MINE			(-60*FUEL_SCALE_FACT)
#define ED_ECM			(-60*FUEL_SCALE_FACT)
#define ED_TRANSPORTER		(-60*FUEL_SCALE_FACT)
#define ED_HYPERJUMP		(-60*FUEL_SCALE_FACT)
#define ED_SHIELD		(-0.20*FUEL_SCALE_FACT)
#define ED_CLOAKING_DEVICE	(-0.07*FUEL_SCALE_FACT)
#define ED_DEFLECTOR		(-0.15*FUEL_SCALE_FACT)
#define ED_SHOT_HIT		(-25.0*FUEL_SCALE_FACT)
#define ED_SMART_SHOT_HIT	(-120.0*FUEL_SCALE_FACT)
#define ED_PL_CRASH		(-100.0*FUEL_SCALE_FACT)
#define ED_BALL_HIT		(-50.0*FUEL_SCALE_FACT)
#define ED_LASER		(-10.0*FUEL_SCALE_FACT)
/* was 90 -> 2 -> 40 -> 20 -> 10 */
#define ED_LASER_HIT		(-100.0*FUEL_SCALE_FACT)
/* was 120 -> 80 -> 40 -> 50 -> 60 -> 100 */

#define PTS_PR_PL_SHOT	    	-5    	/* Points if you get shot by a player */

#define CANNON_SCORE	    	-1436
#define WALL_SCORE	    	2000
#define ASTEROID_SCORE		-9116

#define RATE_SIZE	    	20
#define RATE_RANGE	    	1024

/* score.c */

void SCORE(int ind, int points, int x, int y, const char *msg);
int Rate(int winner, int loser);

/*
 * Cause `winner' to get `winner_score' points added with message
 * `winner_msg', and similarly with the `loser' and equivalent
 * variables.
 *
 * In general the winner_score should be positive, and the loser_score
 * negative, but this need not be true.
 *
 * If the winner and loser players are on the same team, the scores are
 * made negative, since you shouldn't gain points by killing team members,
 * or being killed by a team member (it is both players faults).
 *
 * BD 28-4-98: Same for killing your own tank.
 */
void Score_players(int winner, int winner_score, char *winner_msg,
		   int loser, int loser_score, char *loser_msg);

#endif

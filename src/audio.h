/* $Id: audio.h,v 3.11 1994/05/23 19:02:55 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-94 by
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
/* This piece of code was provided by Greg Renda (greg@ncd.com). */

#ifndef _audio_h
#define _audio_h

#define SOUND_MAX_VOLUME	100
#define SOUND_MIN_VOLUME	10

enum {
    START_SOUND,
    FIRE_SHOT_SOUND,
    FIRE_TORPEDO_SOUND,
    FIRE_HEAT_SHOT_SOUND,
    FIRE_SMART_SHOT_SOUND,
    PLAYER_EXPLOSION_SOUND,
    PLAYER_HIT_PLAYER_SOUND,
    PLAYER_HIT_CANNON_SOUND,
    PLAYER_HIT_MINE_SOUND,
    PLAYER_EAT_TORPEDO_SHOT_SOUND,
    PLAYER_EAT_HEAT_SHOT_SOUND,
    PLAYER_EAT_SMART_SHOT_SOUND,
    DROP_MINE_SOUND,
    PLAYER_HIT_WALL_SOUND,
    WORM_HOLE_SOUND,
    WIDEANGLE_SHOT_PICKUP_SOUND,
    SENSOR_PACK_PICKUP_SOUND,
    BACK_SHOT_PICKUP_SOUND,
    ROCKET_PACK_PICKUP_SOUND,
    CLOAKING_DEVICE_PICKUP_SOUND,
    ENERGY_PACK_PICKUP_SOUND,
    MINE_PACK_PICKUP_SOUND,
    REFUEL_SOUND,
    THRUST_SOUND,
    CLOAK_SOUND,
    CHANGE_HOME_SOUND,
    ECM_PICKUP_SOUND,
    AFTERBURNER_PICKUP_SOUND,
    TANK_PICKUP_SOUND,
    DROP_MOVING_MINE_SOUND,
    MINE_EXPLOSION_SOUND,
    ECM_SOUND,
    TANK_DETACH_SOUND,
    CANNON_FIRE_SOUND,
    PLAYER_SHOT_THEMSELF_SOUND,
    DECLARE_WAR_SOUND,
    PLAYER_HIT_CANNONFIRE_SOUND,
    OBJECT_EXPLOSION_SOUND,
    PLAYER_EAT_SHOT_SOUND,
    TRANSPORTER_PICKUP_SOUND,
    TRANSPORTER_SUCCESS_SOUND,
    TRANSPORTER_FAIL_SOUND,
    NUKE_LAUNCH_SOUND,
    NUKE_EXPLOSION_SOUND,
    PLAYER_RAN_OVER_PLAYER_SOUND,
    LASER_PICKUP_SOUND,
    EMERGENCY_THRUST_PICKUP_SOUND,
    AUTOPILOT_PICKUP_SOUND,
    TRACTOR_BEAM_PICKUP_SOUND,
    PLAYER_BOUNCED_SOUND,
    FIRE_LASER_SOUND,
    AUTOPILOT_ON_SOUND,
    AUTOPILOT_OFF_SOUND,
    EMERGENCY_THRUST_ON_SOUND,
    EMERGENCY_THRUST_OFF_SOUND,
    TRACTOR_BEAM_SOUND,
    PRESSOR_BEAM_SOUND,
    CONNECT_BALL_SOUND,
    DROP_BALL_SOUND,
    EXPLODE_BALL_SOUND,
    DESTROY_BALL_SOUND,
    DESTROY_TARGET_SOUND,
    TEAM_WIN_SOUND,
    TEAM_DRAW_SOUND,
    PLAYER_WIN_SOUND,
    PLAYER_DRAW_SOUND,
    PLAYER_ROASTED_SOUND,
    PLAYER_EAT_LASER_SOUND,
    EMERGENCY_SHIELD_PICKUP_SOUND,
    EMERGENCY_SHIELD_ON_SOUND,
    EMERGENCY_SHIELD_OFF_SOUND,
    MAX_SOUNDS,
};

#ifdef _CAUDIO_C_

static char    *soundNames[] =
{
    "start",
    "fire_shot",
    "fire_torpedo",
    "fire_heat_shot",
    "fire_smart_shot",
    "player_explosion",
    "player_hit_player",
    "player_hit_cannon",
    "player_hit_mine",
    "player_eat_torpedo_shot",
    "player_eat_heat_shot",
    "player_eat_smart_shot",
    "drop_mine",
    "player_hit_wall",
    "worm_hole",
    "wideangle_shot_pickup",
    "sensor_pack_pickup",
    "back_shot_pickup",
    "smart_shot_pickup",
    "cloaking_device_pickup",
    "energy_pack_pickup",
    "mine_pack_pickup",
    "refuel",
    "thrust",
    "cloak",
    "change_home",
    "ecm_pickup",
    "afterburner_pickup",
    "tank_pickup",
    "drop_moving_mine",
    "mine_explosion",
    "ecm",
    "tank_detach",
    "cannon_fire",
    "player_shot_themself",
    "declare_war",
    "player_hit_cannonfire",
    "object_explosion",
    "player_eat_shot",
    "transporter_pickup",
    "transporter_success",
    "transporter_fail",
    "nuke_launch",
    "nuke_explosion",
    "player_ran_over_player",
    "laser_pickup",
    "emergency_thrust_pickup",
    "autopilot_pickup",
    "tractor_beam_pickup",
    "player_bounced",
    "fire_laser",
    "autopilot_on",
    "autopilot_off",
    "emergency_thrust_on",
    "emergency_thrust_off",
    "tractor_beam",
    "pressor_beam",
    "connect_ball",
    "drop_ball",
    "explode_ball",
    "destroy_ball",
    "destroy_target",
    "team_win",
    "team_draw",
    "player_win",
    "player_draw",
    "player_roasted",
    "player_eat_laser",
    "emergency_shield_pickup",
    "emergency_shield_on",
    "emergency_shield_off",
};

#endif						/* _CAUDIO_C_ */
#endif						/* _audio_h */

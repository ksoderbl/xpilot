/* $Id: proto.h,v 3.47 1995/01/11 19:51:18 bert Exp $
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

#ifndef	PROTO_H
#define	PROTO_H

#ifdef VMS
#include "strcasecmp.h"
#endif

/*
 * Prototypes for collision.c
 */
extern void Free_cells(void);
extern void Alloc_cells(void);
extern void Cell_objects_init(void);
extern void Cell_objects_get(int x, int y, int r, object ***list, int *count);
extern void Check_collision(void);
extern int wormXY(int x, int y);
extern void SCORE(int, int, int, int, char *);
extern void Move_init(void);
extern void Move_object(int ind);
extern void Move_player(int ind);
extern void Turn_player(int ind);

/*
 * Prototypes for error.c
 */
#include "error.h"

/*
 * Prototypes for event.c
 */
extern int Handle_keyboard(int);
extern void Pause_player(int ind, int onoff);

/*
 * Prototypes for map.c
 */
extern void Print_map(void)			/* Debugging only. */;
extern void Init_map(void);
extern void Free_map(void);
extern void Alloc_map(void);
extern void Grok_map(void);
extern void Generate_random_map(void);
extern void Find_base_direction(void);
extern void Compute_gravity(void);
extern float Wrap_findDir(float dx, float dy);
extern float Wrap_length(float dx, float dy);

/*
 * Prototypes for math.c
 */
extern int ON(char *optval);
extern int OFF(char *optval);
extern int mod(int x, int y);
extern int f2i(float f);
extern float findDir(float x, float y);
extern float rfrac(void);
extern void Make_table(void);


/*
 * Prototypes for cmdline.c
 */
extern int Parse_list(int *index, char *buf);
extern void Parser(int argc, char *argv[]);
extern int Tune_option(char *opt, char *val);

/*
 * Prototypes for play.c
 */
extern void Thrust(int ind);
#ifdef TURN_FUEL
extern void Turn_thrust(int ind,int num_sparks);
#endif
extern void Recoil(object *ship, object *shot);
extern void Record_shove(player *pl, player *pusher, long time);
extern void Delta_mv(object *ship, object *obj);
extern void Obj_repel(object *obj1, object *obj2, int repel_dist);
extern void Item_damage(int ind, float prob);
extern void Alloc_shots(int number);
extern void Free_shots(void);
extern void Tank_handle_detach(player*);
extern void Add_fuel(pl_fuel_t*, long);
extern void Update_tanks(pl_fuel_t *);
extern void Place_item(int type, player *pl);
extern void Place_mine(int ind);
extern void Place_moving_mine(int ind);
extern void Place_general_mine(int ind, long status, float x, float y,
  			       float vx, float vy, modifiers mods);
extern void Detonate_mines(int ind);
extern void Cannon_fire(int ind);
extern char *Describe_shot(int type, long status, modifiers mods, int hit);
extern void Fire_ecm(int ind);
extern void Move_ball(int ind);
extern void Fire_shot(int ind, int type, int dir);
extern void Fire_general_shot(int ind, float x, float y, int type, int dir,
			      float speed, modifiers mods);
extern void Fire_normal_shots(int ind);
extern void Make_treasure_ball(int treasure);
extern int Punish_team(int ind, int t_destroyed, int t_target);
extern void Delete_shot(int ind);
extern void do_transporter(player *pl);
extern void Move_smart_shot(int ind);
extern void Move_mine(int ind);
extern void Make_debris(
	    /* pos.x, pos.y   */ float  x,          float y,
	    /* vel.x, vel.y   */ float  velx,       float vely,
	    /* owner id       */ int    id,
	    /* type           */ int    type,
	    /* mass           */ float  mass,
	    /* status         */ long   status,
	    /* color          */ int    color,
	    /* radius         */ int    radius,
	    /* min,max debris */ int    min_debris, int    max_debris,
	    /* min,max dir    */ int    min_dir,    int    max_dir,
	    /* min,max speed  */ float  min_speed,  float  max_speed,
	    /* min,max life   */ int    min_life,   int    max_life
	    );
extern void Explode(int ind);
extern void Explode_fighter(int ind);
extern void Throw_items(player*);
extern void Detonate_items(int ind);

/*
 * Prototypes for player.c
 */
extern void Pick_startpos(int ind);
extern void Go_home(int ind);
extern void Compute_sensor_range(player *);
extern void Player_add_tank(int ind, long tank_fuel);
extern void Player_remove_tank(int ind, int which_tank);
extern void Init_player(int ind, wireobj *ship);
extern void Alloc_players(int number);
extern void Free_players(void);
extern void Update_score_table(void);
extern void Check_team_members(int);
extern void Check_team_treasures(int);
extern void Compute_game_status(void);
extern void Delete_player(int ind);
extern void Detach_ball(int ind, int ball);
extern void Kill_player(int ind);
extern void Player_death_reset(int ind);

/*
 * Prototypes for robot.c
 */
extern void Robot_init(void);
extern void Robot_delete(int ind, int kicked);
extern void Robot_destroy(int ind);
extern void Robot_update(void);
extern void Robot_war(int ind, int killer);
extern void Robot_reset_war(int ind);
extern int Robot_war_on_player(int ind);
extern void Robot_go_home(int ind);
extern void Robot_program(int ind, int victim_id);
extern void Robot_message(int ind, char *message);

/*
 * Prototypes for rules.c
 */
extern void Tune_item_probs(void);
extern void Set_initial_resources(void);
extern void Set_world_items(void);
extern void Set_world_rules(void);
extern void Set_misc_item_limits(void);

/*
 * Prototypes for server.c
 */
extern int main(int argc, char *argv[]);
extern void Main_Loop(void);
extern void End_game(void);
extern void Contact(void);
extern void Handle_signal(int sig_no);
extern void Log_game(char *heading);
extern void Server_info(char *, unsigned);
extern void Game_Over(void);
extern void Send_meta_server(int change);

/*
 * Prototypes for frame.c
 */
extern void Frame_update(void);
extern void Set_message(char *message);
extern void Set_player_message(player *pl, char *message);

/*
 * Prototypes for syslimit.c
 */
extern bool Is_allowed(void);

/*
 * Prototypes for timer.c
 */
extern void block_timer(void);
extern void allow_timer(void);

/*
 * Prototypes for update.c
 */
extern void Update_radar_target(int);
extern void Update_objects(void);
extern void Emergency_thrust(int ind, int on);
extern void Emergency_shield(int ind, int on);
extern void Autopilot(int ind, int on);

/*
 * Prototypes for option.c
 */
extern void addOption (char *name, char *value, int override, void *def);
extern char *getOption (char *name);
extern bool parseDefaultsFile (char *filename);
extern void parseOptions (void);

#endif

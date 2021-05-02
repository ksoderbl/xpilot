/* $Id: proto.h,v 5.23 2001/06/05 08:30:10 bertg Exp $
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

#ifndef	PROTO_H
#define	PROTO_H

/*
 * Prototypes for cell.c
 */
void Free_cells(void);
void Alloc_cells(void);
void Cell_init_object(object *obj);
void Cell_add_object(object *obj);
void Cell_remove_object(object *obj);
void Cell_get_objects(int x, int y, int r, int max, object ***list, int *count);

/*
 * Prototypes for collision.c
 */
void Check_collision(void);
int wormXY(int x, int y);
int IsOffensiveItem(enum Item i);
int IsDefensiveItem(enum Item i);
int CountOffensiveItems(player *pl);
int CountDefensiveItems(player *pl);

/*
 * Prototypes for id.c
 */
int peek_ID(void);
int request_ID(void);
void release_ID(int id);

/*
 * Prototypes for walls.c
 */
void Walls_init(void);
void Treasure_init(void);
void Move_init(void);
void Move_object(object *obj);
void Move_player(int ind);
void Turn_player(int ind);

/*
 * Prototypes for event.c
 */
int Handle_keyboard(int);
void Pause_player(int ind, int onoff);
int Player_lock_closest(int ind, int next);
bool team_dead(int team);

/*
 * Prototypes for map.c
 */
void Free_map(void);
bool Grok_map(void);
void Find_base_direction(void);
void Compute_gravity(void);
DFLOAT Wrap_findDir(DFLOAT dx, DFLOAT dy);
DFLOAT Wrap_length(DFLOAT dx, DFLOAT dy);

int Wildmap(
	int width,
	int height,
	char *name,
	char *author,
	char **data,
	int *width_ptr,
	int *height_ptr);

/*
 * Prototypes for math.c
 */
int ON(char *optval);
int OFF(char *optval);
int mod(int x, int y);
int f2i(DFLOAT f);
DFLOAT findDir(DFLOAT x, DFLOAT y);
DFLOAT rfrac(void);
void Make_table(void);


/*
 * Prototypes for cmdline.c
 */
void tuner_none(void);
void tuner_dummy(void);
bool Init_options(void);
void Free_options(void);


/*
 * Prototypes for play.c
 */
void Thrust(int ind);
void Turn_thrust(int ind,int num_sparks);
void Recoil(object *ship, object *shot);
void Record_shove(player *pl, player *pusher, long time);
void Delta_mv(object *ship, object *obj);
void Delta_mv_elastic(object *obj1, object *obj2);
void Obj_repel(object *obj1, object *obj2, int repel_dist);
void Item_damage(int ind, DFLOAT prob);
void Alloc_shots(int number);
void Free_shots(void);
void Tank_handle_detach(player*);
void Add_fuel(pl_fuel_t*, long);
void Update_tanks(pl_fuel_t *);
void Place_item(int type, int ind);
void Tractor_beam(int ind);
void General_tractor_beam(int ind, DFLOAT x, DFLOAT y,
			  int items, int target, bool pressor);
void Place_mine(int ind);
void Place_moving_mine(int ind);
void Place_general_mine(int ind, unsigned short team, long status, DFLOAT x, DFLOAT y,
  			DFLOAT vx, DFLOAT vy, modifiers mods);
void Detonate_mines(int ind);
char *Describe_shot(int type, long status, modifiers mods, int hit);
void Fire_ecm(int ind);
void Fire_general_ecm(int ind, unsigned short team, DFLOAT x, DFLOAT y);
void Move_ball(int ind);
void Fire_shot(int ind, int type, int dir);
void Fire_general_shot(int ind, unsigned short team, bool cannon, DFLOAT x, DFLOAT y,
		       int type, int dir, DFLOAT speed, modifiers mods,
		       int target);
void Fire_normal_shots(int ind);
void Fire_main_shot(int ind, int type, int dir);
void Fire_shot(int ind, int type, int dir);
void Fire_left_shot(int ind, int type, int dir, int gun);
void Fire_right_shot(int ind, int type, int dir, int gun);
void Fire_left_rshot(int ind, int type, int dir, int gun);
void Fire_right_rshot(int ind, int type, int dir, int gun);
void Make_treasure_ball(int treasure);
int Punish_team(int ind, int t_destroyed, int t_target);
void Delete_shot(int ind);
void Fire_laser(int ind);
void Fire_general_laser(int ind, unsigned short team, DFLOAT x, DFLOAT y, int dir,
			modifiers mods);
void Do_deflector(int ind);
void Do_transporter(int ind);
void Do_general_transporter(int ind, DFLOAT x, DFLOAT y, int target,
			    int *item, long *amount);
void do_hyperjump(player *pl);
void do_lose_item(int ind);
void Move_smart_shot(int ind);
void Move_mine(int ind);
void Make_debris(
	    /* pos.x, pos.y   */ DFLOAT  x,          DFLOAT y,
	    /* vel.x, vel.y   */ DFLOAT  velx,       DFLOAT vely,
	    /* owner id       */ int    id,
	    /* owner team     */ unsigned short team,
	    /* type           */ int    type,
	    /* mass           */ DFLOAT  mass,
	    /* status         */ long   status,
	    /* color          */ int    color,
	    /* radius         */ int    radius,
	    /* min,max debris */ int    min_debris, int    max_debris,
	    /* min,max dir    */ int    min_dir,    int    max_dir,
	    /* min,max speed  */ DFLOAT  min_speed,  DFLOAT  max_speed,
	    /* min,max life   */ int    min_life,   int    max_life
	    );
void Make_wreckage(
	    /* pos.x, pos.y   */ DFLOAT x,          DFLOAT y,
	    /* vel.x, vel.y   */ DFLOAT velx,       DFLOAT vely,
	    /* owner id       */ int    id,
	    /* owner team     */ unsigned short team,
	    /* min,max mass   */ DFLOAT min_mass,   DFLOAT max_mass,
	    /* total mass     */ DFLOAT total_mass,
	    /* status         */ long   status,
	    /* color          */ int    color,
	    /* max wreckage   */ int    max_wreckage,
	    /* min,max dir    */ int    min_dir,    int    max_dir,
	    /* min,max speed  */ DFLOAT min_speed,  DFLOAT max_speed,
	    /* min,max life   */ int    min_life,   int    max_life
	    );
void Explode(int ind);
void Explode_fighter(int ind);
void Throw_items(int ind);
void Detonate_items(int ind);
void add_temp_wormholes(int xin, int yin, int xout, int yout, int ind);
void remove_temp_wormhole(int ind);


/*
 * Prototypes for asteroid.c
 */
void Break_asteroid(int ind);
void Asteroid_update(void);
#ifdef LIST_H_INCLUDED
list_t Asteroid_get_list(void);
#endif


/*
 * Prototypes for cannon.c
 */
void Cannon_init(int ind);
void Cannon_add_item(int ind, int item, int amount);
void Cannon_throw_items(int ind);
void Cannon_check_fire(int ind);
int Cannon_select_weapon(int ind);
void Cannon_aim(int ind, int weapon, int *target, int *dir);
void Cannon_fire(int ind, int weapon, int target, int dir);

/*
 * Prototypes for command.c
 */
void Handle_player_command(player *pl, char *cmd);

/*
 * Prototypes for player.c
 */
void Pick_startpos(int ind);
void Go_home(int ind);
void Compute_sensor_range(player *);
void Player_add_tank(int ind, long tank_fuel);
void Player_remove_tank(int ind, int which_tank);
void Player_hit_armor(int ind);
int Init_player(int ind, shipobj *ship);
void Alloc_players(int number);
void Free_players(void);
void Update_score_table(void);
void Reset_all_players(void);
void Check_team_members(int);
void Check_team_treasures(int);
void Compute_game_status(void);
void Delete_player(int ind);
void Detach_ball(int ind, int ball);
void Kill_player(int ind);
void Player_death_reset(int ind);
void Team_game_over(int winning_team, const char *reason);
void Individual_game_over(int winner);
void Race_game_over(void);

/*
 * Prototypes for robot.c
 */
void Parse_robot_file(void);
void Robot_init(void);
void Robot_delete(int ind, int kicked);
void Robot_destroy(int ind);
void Robot_update(void);
void Robot_war(int ind, int killer);
void Robot_reset_war(int ind);
int Robot_war_on_player(int ind);
void Robot_go_home(int ind);
void Robot_program(int ind, int victim_id);
void Robot_message(int ind, const char *message);

/*
 * Prototypes for rules.c
 */
void Tune_item_probs(void);
void Tune_item_packs(void);
void Set_initial_resources(void);
void Set_world_items(void);
void Set_world_rules(void);
void Set_world_asteroids(void);
void Set_misc_item_limits(void);
void Tune_asteroid_prob(void);

/*
 * Prototypes for server.c
 */
int End_game(void);
int Pick_team(int pick_for_type);
void Server_info(char *str, unsigned max_size);
void Log_game(const char *heading);
void Game_Over(void);
int plock_server(int onoff);
void tuner_plock(void);
void Main_loop(void);


/*
 * Prototypes for contact.c
 */
void Contact_cleanup(void);
int Contact_init(void);
void Contact(int fd, void *arg);
void Queue_loop(void);
int Queue_advance_player(char *name, char *msg);
int Queue_show_list(char *msg);
void Set_deny_hosts(void);

/*
 * Prototypes for metaserver.c
 */
void Meta_send(char *mesg, int len);
int Meta_from(char *addr, int port);
void Meta_gone(void);
void Meta_init(void);
void Meta_update(int change);

/*
 * Prototypes for frame.c
 */
void Frame_update(void);
void Set_message(const char *message);
void Set_player_message(player *pl, const char *message);

/*
 * Prototypes for update.c
 */
void Update_radar_target(int);
void Update_objects(void);
void Emergency_thrust(int ind, int on);
void Emergency_shield(int ind, int on);
void Autopilot(int ind, int on);
void Phasing(int ind, int on);

/*
 * Prototypes for option.c
 */
void Options_parse(void);
bool Convert_string_to_int(const char *value_str, int *int_ptr);
bool Convert_string_to_float(const char *value_str, DFLOAT *float_ptr);
bool Convert_string_to_bool(const char *value_str, bool *bool_ptr);

/*
 * Prototypes for parser.c
 */
int Parser_list_option(int *index, char *buf);
bool Parser(int argc, char **argv);
int Tune_option(char *name, char *val);
int Get_option_value(const char *name, char *value);

/*
 * Prototypes for fileparser.c
 */
bool parseDefaultsFile(const char *filename);
bool parseMapFile(const char *filename);

/*
 * Prototypes for laser.c
 */
void Laser_pulse_collision(void);

#endif

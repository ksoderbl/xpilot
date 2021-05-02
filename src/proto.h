/* $Id: proto.h,v 3.6 1993/08/02 12:55:30 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	PROTO_H
#define	PROTO_H

#include "types.h"
#include "object.h"


/*
 * Prototypes for collision.c
 */
extern void Check_collision(void);
extern int wormXY(int x, int y);

/*
 * Prototypes for error.c
 */
#include "error.h"

/*
 * Prototypes for map.c
 */
extern void Optimize_map(void);
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
extern int mod(int, int);
extern void Make_table(void);
extern void Make_ships(void);
extern void Free_ships(void);
extern float findDir (float x, float y);

/*
 * Prototypes for parser.c
 */
extern void Parser(int argc, char *argv[]);

/*
 * Prototypes for play.c
 */
extern void Thrust(int ind);
#ifdef TURN_FUEL
extern void Turn_thrust(int ind,int num_sparks);
#endif
extern void Recoil(object *ship, object *shot);
extern void Delta_mv(object *ship, object *obj);
extern void Obj_repel(object *obj1, object *obj2, int repel_dist);
extern void Item_damage(int ind);
extern void Alloc_shots(int number);
extern void Free_shots(void);
extern void Tank_handle_detach(player*);
extern void Add_fuel(pl_fuel_t*,long);
extern void Update_tanks(pl_fuel_t*);
extern void Place_item(int type, player*);
extern void Place_mine(int ind);
extern void Place_moving_mine(int ind, float vx, float vy);
extern void Cannon_fire(int ind);
extern void Fire_shot(int ind, int type, int dir);
extern void Fire_normal_shots(int ind);
extern void Make_ball(int id, float x, float y, bool grav, int treasure);
extern void Delete_shot(int ind);
extern void Move_smart_shot(int ind);
extern void Explode_object(float x, float y, int real_dir,
			   int spread, int intensity);
extern void Explode(int ind);
extern void Throw_items(player*);

/*
 * Prototypes for player.c
 */
extern void Pick_startpos(int ind);
extern void Go_home(int ind);
extern void Init_player(int ind);
extern void Alloc_players(int number);
extern void Free_players(void);
extern void Update_score_table(void);
/*extern void Reset_all_players(void);*/
extern void Compute_game_status(void);
extern void Delete_player(int ind);
extern void Kill_player(int ind);

/*
 * Prototypes for robot.c
 */
extern void Init_robot(int ind);
extern void Update_robots(void);

/*
 * Prototypes for rules.c
 */
extern void Set_world_rules(void);
extern void UpdateItemChances(int num_players);

/*
 * Prototypes for server.c
 */
extern int main(int argc, char *argv[]);
extern void Main_Loop(void);
extern void End_game(void);
extern bool Check_new_players(void);
extern bool Owner(char *name);
extern void Handle_signal(int sig_no);
extern void Log_game(char *heading);
extern void Server_info(char *, unsigned);

/*
 * Prototypes for syslimit.c
 */
extern bool Is_allowed(void);

/*
 * Prototypes for update.c
 */
extern void Update_objects(void);

/*
 * Prototypes for option.c
 */
extern void addOption (char *name, char *value, int override, void *def);
extern char *getOption (char *name);
extern bool parseDefaultsFile (char *filename);
extern void parseOptions (void);

#endif

/* proto.h,v 1.5 1992/06/28 05:38:25 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	PROTO_H
#define	PROTO_H

#include <X11/Xlib.h>
#include "types.h"
#include "object.h"
#include "pack.h"


/*
 * Prototypes for xinit.c
 */
extern u_byte Init_window(int ind);
extern void Alloc_msgs(int number);
extern void Free_msgs(int number);
extern void DrawShadowText(int ind, Display *disp, Window w, GC gc, int x,
			   int start_y, char *str, Pixel fg, Pixel bg);
extern void Expose_info_window(int ind);
extern void Expose_help_window(int ind);
extern void Expose_button_window(int ind, int color, Window w);
extern void Info(int ind, Window w);
extern void Help(int ind, Window w);
extern void Quit(int ind);
extern int FatalError(Display *disp);
extern void Set_labels(void);

/*
 * Prototypes for collision.c
 */
extern int Rate(int winner, int looser);
extern void Check_collision(void);
extern bool Landing(int ind, int point);

/*
 * Prototypes for dbuff.c
 */
extern void itob(long i);
extern dbuff_state_t *start_dbuff(int ind, Display *display, Colormap cmap,
				  unsigned long planes, XColor *colors);
extern void dbuff_switch(register int ind, register dbuff_state_t *state);
extern void end_dbuff(int ind, dbuff_state_t *state);

/*
 * Prototypes for def.c
 */
extern void Get_defaults(int ind);

/*
 * Prototypes for draw.c
 */
extern char *scroll(char *string, int start, int length);
extern void Draw_objects();
extern void Draw_ships(int draw, int data);
extern void Draw_shots(int draw, int data);
extern void Draw_meters(int draw, int data);
extern void Display_hud(int draw, int data);
extern void Display_fuel_gauge(int draw, int data);
extern void Display_velocity_gauge(int draw, int data);
extern void Display_power_gauge(int draw, int data);
extern void Display_compass(int draw, int data);
extern void Draw_cannon(int draw, int data);
extern void Display_message(int draw, int ind);
extern void Set_message(char *message);
extern void Draw_radar(int draw, int data);
extern void Display_time(int draw, int data);
extern void Draw_world(int draw, int data);
extern void Draw_world_radar(int draw);

/*
 * Prototypes for error.c
 */
#include "error.h"

/*
 * Prototypes for event.c
 */
extern void Refuel(int ind);
extern void Key_event(int ind, XEvent *event);

/*
 * Prototypes for map.c
 */
extern void Optimize_map();
extern void Print_map(void)			/* Debugging only. */;
extern void Init_map(void);
extern void Free_map(void);
extern void Alloc_map(void);
extern void Load_map(char *map);
extern void Generate_random_map(void);

/*
 * Prototypes for math.c
 */
extern void Make_table(void);
extern void Make_ships(void);
extern void Free_ships(void);
extern void Compute_gravity(void);

/*
 * Prototypes for parser.c
 */
extern void Parser(int argc, char *argv[]);

/*
 * Prototypes for play.c
 */
extern void Thrust(int ind);
extern void Turn_thrust(int ind);
extern void Recoil(object *ship, object *shot);
extern void Delta_mv(object *ship, object *obj);
extern void Alloc_shots(int number);
extern void Free_shots(int number);
extern void Place_item(int type);
extern void Place_mine(int ind);
extern void Cannon_fire(int ind);
extern void Fire_shot(int ind, int type, int dir);
extern void Delete_shot(int ind);
extern void Move_smart_shot(int ind);
extern void Explode_object(double x, double y, int real_dir,
			   int spread, int intensity);
extern void Explode(int ind);

/*
 * Prototypes for player.c
 */
extern void Pick_startpos(int ind);
extern void Go_home(int ind);
extern void Init_player(int ind);
extern void Alloc_players(int number);
extern void Free_players(int number);
extern void Set_label_strings(void);
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
extern void Set_world_rules(int rule_ind);

/*
 * Prototypes for server.c
 */
extern int main(int argc, char *argv[]);
extern void Main_Loop();
extern void End_game(void);
extern void Dump_pack(core_pack_t *p);
extern bool Check_new_players(void);
extern bool Owner(char *name);
extern void Handle_signal(int sig_no);
extern void Log_game(char *heading);

/*
 * Prototypes for syslimit.c
 */
extern bool Is_allowed(void);

/*
 * Prototypes for update.c
 */
extern void Update_objects(void);

/*
 * Prototypes for usleep.c
 */
extern int usleep(unsigned long usec);
extern long seconds(void);

/*
 * Prototypes for xpilot.c
 */
extern void initaddr(void);
extern void printfile(char *name);
extern bool Get_contact_message(void);
extern int Get_reply_message(reply_pack_t *p);
extern bool Connect_to_server(void);

#endif

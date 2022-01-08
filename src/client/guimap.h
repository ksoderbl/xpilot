/* $Id: guimap.h,v 5.1 2001/10/11 20:27:35 bertg Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
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

#ifndef GUIMAP_H
#define GUIMAP_H

void Gui_paint_cannon(int x, int y, int type);
void Gui_paint_fuel(int x, int y, int fuel);
void Gui_paint_base(int x, int y, int xi, int yi, int type);
void Gui_paint_decor(int x, int y, int xi, int yi, int type, bool last, bool more_y);

void Gui_paint_border(int x, int y, int xi, int yi);
void Gui_paint_visible_border(int x, int y, int xi, int yi);

void Gui_paint_setup_check(int x, int y, int xi, int yi);
void Gui_paint_setup_acwise_grav(int x, int y);
void Gui_paint_setup_cwise_grav(int x, int y);
void Gui_paint_setup_pos_grav(int x, int y);
void Gui_paint_setup_neg_grav(int x, int y);
void Gui_paint_setup_up_grav(int x, int y);
void Gui_paint_setup_down_grav(int x, int y);
void Gui_paint_setup_right_grav(int x, int y);
void Gui_paint_setup_left_grav(int x, int y);
void Gui_paint_setup_worm(int x, int y, int wormDrawCount);
void Gui_paint_setup_item_concentrator(int x, int y);
void Gui_paint_setup_asteroid_concentrator(int x, int y);
void Gui_paint_decor_dot(int x, int y, int size);
void Gui_paint_setup_target(int x, int y, int target, int damage, bool own);
void Gui_paint_setup_treasure(int x, int y, int treasure, bool own);

void Gui_paint_walls(int x, int y, int type, int xi, int yi);

#endif

/* $Id: xevent.h,v 1.1 2001/06/02 20:40:04 bertg Exp $
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

#ifndef XEVENT_H
#define XEVENT_H

keys_t Lookup_key(XEvent *event, KeySym ks, bool reset);
void Pointer_control_set_state(int onoff);
int Key_init(void);
int Key_update(void);
bool Key_check_talk_macro(keys_t key);
bool Key_press_id_mode(keys_t key);
bool Key_press_autoshield_hack(keys_t key);
bool Key_press_shield(keys_t key);
bool Key_press_fuel(keys_t key);
bool Key_press_swap_settings(keys_t key);
bool Key_press_swap_scalefactor(keys_t key);
bool Key_press_increase_power(keys_t key);
bool Key_press_decrease_power(keys_t key);
bool Key_press_increase_turnspeed(keys_t key);
bool Key_press_decrease_turnspeed(keys_t key);
bool Key_press_talk(keys_t key);
bool Key_press_show_items(keys_t key);
bool Key_press_show_messages(keys_t key);
bool Key_press_pointer_control(keys_t key);
bool Key_press_toggle_record(keys_t key);
bool Key_press_msgs_stdout(keys_t key);
bool Key_press_select_lose_item(keys_t key);
bool Key_press(keys_t key);
bool Key_release(keys_t key);
void Key_event(XEvent *event);
void Reset_shields(void);
void Set_auto_shield(int onoff);
void Set_toggle_shield(int onoff);
void Talk_event(XEvent *event);
void xevent_keyboard(int queued);
void xevent_pointer(void);
int x_event(int new_input);

#ifdef _WINDOWS
int win_xevent(XEvent event);
#endif

#endif

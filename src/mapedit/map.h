/*
 * XMapEdit, the XPilot Map Editor.  Copyright (C) 1993 by
 *
 *      Aaron Averill           <averila@oes.orst.edu>
 *
 * This program is free software) you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation) either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY) without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program) if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Modifications to XMapEdit
 * 1996:
 *      Robert Templeman        <mbcaprt@mphhpd.ph.man.ac.uk>
 * 1997:
 *      William Docter          <wad2@lehigh.edu>
 */

#ifndef MAP_H
#define MAP_H

/* Blank Space */
#define MAP_SPACE	((int) ' ')
#define MAP_NOTHIN      ((int) ' ')

/* Wall */
#define MAP_REC_RD	((int) 'q')
#define MAP_REC_LD	((int) 'w')
#define MAP_FILLED	((int) 'x')
#define MAP_REC_RU	((int) 'a')
#define MAP_REC_LU	((int) 's')

/* Cannon */
#define MAP_CAN_LEFT	((int) 'd')
#define MAP_CAN_UP	((int) 'r')
#define MAP_CAN_DOWN	((int) 'c')
#define MAP_CAN_RIGHT	((int) 'f')

/* Bases */
#define MAP_BASE	((int) '_')
#define MAP_BASE_ORNT ((int) '$')

/* Misc */
#define MAP_FUEL	((int) '#')
#define MAP_TARGET	((int) '!')
#define MAP_TREASURE	((int) '*')
#define MAP_ITEM_CONC ((int) '%')
#define MAP_EMPTY_TREASURE ((int) '^')
#define MAP_FRICTION  ((int) 'z')
#define MAP_ASTEROID_CONC ((int) '&')

/* Gravity */
#define MAP_GRAV_POS	((int) '+')
#define MAP_GRAV_NEG	((int) '-')
#define MAP_GRAV_ACWISE	((int) '<')
#define MAP_GRAV_CWISE	((int) '>')

/* Wormholes */
#define MAP_WORM_NORMAL	((int) '@')
#define MAP_WORM_OUT	((int) ')')
#define MAP_WORM_IN	((int) '(')

/* Currents */
#define MAP_CRNT_UP   ((int) 'i')
#define MAP_CRNT_LT   ((int) 'j')
#define MAP_CRNT_RT   ((int) 'k')
#define MAP_CRNT_DN   ((int) 'm')

/* Decor */
#define MAP_DEC_RD	((int) 't')
#define MAP_DEC_LD	((int) 'y')
#define MAP_DEC_FLD	((int) 'b')
#define MAP_DEC_RU	((int) 'g')
#define MAP_DEC_LU	((int) 'h')

#define MAP_MINER     ((int) '^')

#endif

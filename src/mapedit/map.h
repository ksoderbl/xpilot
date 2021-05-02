/*
 * XMapEdit, the XPilot Map Editor.  Copyright (C) 1993 by
 *
 *      Aaron Averill           <averila@oes.orst.edu>
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
 *
 * Modifications to XMapEdit
 * 1996:
 *      Robert Templeman        <mbcaprt@mphhpd.ph.man.ac.uk>
 * 1997:
 *      William Docter          <wad2@lehigh.edu>
 *
 * $Id: map.h,v 1.2 1998/04/22 15:44:16 bert Exp $
 */

/* Blank Space */
#define MAP_SPACE	' '
#define MAP_NOTHIN      ' '

/* Wall */
#define MAP_REC_RD	'q'
#define MAP_REC_LD	'w'
#define MAP_FILLED	'x'
#define MAP_REC_RU	'a'
#define MAP_REC_LU	's'

/* Cannon */
#define MAP_CAN_LEFT	'd'
#define MAP_CAN_UP	'r'
#define MAP_CAN_DOWN	'c'
#define MAP_CAN_RIGHT	'f'

/* Bases */
#define MAP_BASE	'_'
#define MAP_BASE_ORNT   '$'

/* Misc */
#define MAP_FUEL	'#'
#define MAP_TARGET	'!'
#define MAP_TREASURE	'*'
#define MAP_ITEM_CONC   '%'

/* Gravity */
#define MAP_GRAV_POS	'+'
#define MAP_GRAV_NEG	'-'
#define MAP_GRAV_ACWISE	'<'
#define MAP_GRAV_CWISE	'>'

/* Wormholes */
#define MAP_WORM_NORMAL	'@'
#define MAP_WORM_OUT	')'
#define MAP_WORM_IN	'('

/* Currents */
#define MAP_CRNT_UP     'i'
#define MAP_CRNT_LT     'j'
#define MAP_CRNT_RT     'k'
#define MAP_CRNT_DN     'm'

/* Decor */
#define MAP_DEC_RD	't'
#define MAP_DEC_LD	'y'
#define MAP_DEC_FLD	'b'
#define MAP_DEC_RU	'g'
#define MAP_DEC_LU	'h'

#define MAP_MINER       '^'

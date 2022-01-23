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
 */

/* Blank Space */
const int MAP_SPACE	= ' ';
const int MAP_NOTHIN    = ' ';

/* Wall */
const int MAP_REC_RD	= 'q';
const int MAP_REC_LD	= 'w';
const int MAP_FILLED	= 'x';
const int MAP_REC_RU	= 'a';
const int MAP_REC_LU	= 's';

/* Cannon */
const int MAP_CAN_LEFT	= 'd';
const int MAP_CAN_UP	= 'r';
const int MAP_CAN_DOWN	= 'c';
const int MAP_CAN_RIGHT	= 'f';

/* Bases */
const int MAP_BASE	= '_';
const int MAP_BASE_ORNT = '$';

/* Misc */
const int MAP_FUEL	= '#';
const int MAP_TARGET	= '!';
const int MAP_TREASURE	= '*';
const int MAP_ITEM_CONC = '%';
const int MAP_EMPTY_TREASURE = '^';
const int MAP_FRICTION  = 'z';
const int MAP_ASTEROID_CONC = '&';

/* Gravity */
const int MAP_GRAV_POS	= '+';
const int MAP_GRAV_NEG	= '-';
const int MAP_GRAV_ACWISE	= '<';
const int MAP_GRAV_CWISE	= '>';

/* Wormholes */
const int MAP_WORM_NORMAL	= '@';
const int MAP_WORM_OUT	= ')';
const int MAP_WORM_IN	= '(';

/* Currents */
const int MAP_CRNT_UP   = 'i';
const int MAP_CRNT_LT   = 'j';
const int MAP_CRNT_RT   = 'k';
const int MAP_CRNT_DN   = 'm';

/* Decor */
const int MAP_DEC_RD	= 't';
const int MAP_DEC_LD	= 'y';
const int MAP_DEC_FLD	= 'b';
const int MAP_DEC_RU	= 'g';
const int MAP_DEC_LU	= 'h';

const int MAP_MINER     = '^';

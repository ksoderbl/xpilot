/* $Id: bit.h,v 3.8 1996/10/06 00:00:47 bjoerns Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gÿsbers         <bert@xpilot.org>
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

#ifndef	BIT_H
#define	BIT_H

#define SET_BIT(w, bit)		( (w) |= (bit) )
#define CLR_BIT(w, bit)		( (w) &= ~(bit) )
#define BIT(w, bit)		( (w) & (bit) )
#define TOGGLE_BIT(w, bit)	( (w) ^= (bit) )

#define BITV_SIZE	(8 * sizeof(bitv_t))
#define BITV_DECL(X,N)	bitv_t (X)[((N) + BITV_SIZE - 1) / BITV_SIZE]
#define BITV_SET(X,N)	((X)[(N) / BITV_SIZE] |= 1 << (N) % BITV_SIZE)
#define BITV_CLR(X,N)	((X)[(N) / BITV_SIZE] &= ~(1 << (N) % BITV_SIZE))
#define BITV_ISSET(X,N)	((X)[(N) / BITV_SIZE] & (1 << (N) % BITV_SIZE))
#define BITV_TOGGLE(X,N)	((X)[(N) / BITV_SIZE] ^= 1 << (N) % BITV_SIZE)

typedef unsigned char bitv_t;

#endif

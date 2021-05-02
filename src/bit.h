/* $Id: bit.h,v 3.2 1993/06/28 20:53:19 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
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


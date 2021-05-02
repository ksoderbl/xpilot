/* bit.h,v 1.4 1992/06/25 03:23:46 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	BIT_H
#define	BIT_H

#define SET_BIT(w, bit)		w |= (bit);
#define CLR_BIT(w, bit)		w &= (~(bit));
#define BIT(w, bit)		( w & (bit) )
#define TOGGLE_BIT(w, bit)	w ^= (bit);

#endif

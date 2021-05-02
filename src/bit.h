/* $Id: bit.h,v 1.1 1993/03/09 14:31:05 kenrsc Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
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

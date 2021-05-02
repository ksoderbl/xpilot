/* bit.h,v 1.3 1992/05/11 15:30:56 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#define SET_BIT(w, bit)		w |= (bit);
#define CLR_BIT(w, bit)		w &= (~(bit));
#define BIT(w, bit)		( w & (bit) )
#define TOGGLE_BIT(w, bit)	w ^= (bit);

typedef enum {true = 1, false = 0} BOOLEAN;
typedef BOOLEAN bool;

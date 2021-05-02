/* types.h,v 1.1 1992/06/25 03:24:19 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	TYPES_H
#define	TYPES_H

#include <sys/types.h>

typedef signed char	byte;
typedef unsigned char	u_byte;

typedef enum {true = 1, false = 0} bool;

typedef struct { float x, y; }	vector;
typedef vector			position;
typedef struct { int x, y; }	ipos;

#endif

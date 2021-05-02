/* $Id: types.h,v 3.3 1993/08/02 12:55:39 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	TYPES_H
#define	TYPES_H

#include <sys/types.h>

typedef signed char	byte;
typedef unsigned char	u_byte;

#if (_SEQUENT_)
typedef unsigned short	u_short;
#endif

typedef enum {true = 1, false = 0} bool;

typedef struct { float x, y; }	vector;
typedef vector			position;
typedef struct { int x, y; }	ipos;

#endif

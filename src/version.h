/* version.h,v 1.4 1992/05/11 16:26:01 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include "patchlevel.h"

static char sourceid[] =
    "version.h,v 1.4 1992/05/11 16:26:01 bjoerns Exp";

#define MAJOR		"1"
#define MINOR		"1"
#define VERSION		MAJOR "." MINOR "." PATCHLEVEL
#define TITLE		"XPilot " VERSION
#define COPYRIGHT	"(C) 1991-92 by Bjørn Stabell & Ken Ronny Schouten"

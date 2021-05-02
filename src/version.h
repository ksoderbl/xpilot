/* $Id: version.h,v 3.18 1993/08/03 11:53:41 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	VERSION_H
#define	VERSION_H

/*
 * Update these
 */
#define MAJOR		"3"
#define MINOR		"0"
#define PATCHLEVEL	"3"

/*
 * Don't change these
 */
#define VERSION		MAJOR "." MINOR "." PATCHLEVEL
#define TITLE		"XPilot " VERSION

#if defined(__hpux)
#   pragma COPYRIGHT_DATE	"1991-93"
#   pragma COPYRIGHT	"Bjørn Stabell, Ken Ronny Schouten & Bert Gÿsbers"
#   pragma VERSIONID	TITLE
#endif

#define COPYRIGHT	\
	"© 1991-93 by Bjørn Stabell, Ken Ronny Schouten & Bert Gÿsbers" 

#endif

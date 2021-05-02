/* $Id: version.h,v 3.18 1993/08/03 11:53:41 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bj�rn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert G�sbers (bert@mc.bio.uva.nl)
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
#   pragma COPYRIGHT	"Bj�rn Stabell, Ken Ronny Schouten & Bert G�sbers"
#   pragma VERSIONID	TITLE
#endif

#define COPYRIGHT	\
	"� 1991-93 by Bj�rn Stabell, Ken Ronny Schouten & Bert G�sbers" 

#endif

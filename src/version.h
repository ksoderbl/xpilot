/* $Id: version.h,v 1.10 1993/04/22 16:56:31 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	VERSION_H
#define	VERSION_H

/*
 * Update these
 */
#define MAJOR		"2"
#define MINOR		"0beta"
#define	PATCHLEVEL	"2"

/*
 * Don't change these
 */
#define VERSION		MAJOR "." MINOR " PL" PATCHLEVEL
#define TITLE		"XPilot " VERSION
#define COPYRIGHT	"(C) 1991-92 by Bjørn Stabell & Ken Ronny Schouten"

#if defined(hpux)
#    pragma COPYRIGHT		"Bjørn Stabell & Ken Ronny Schouten"
#    pragma COPYRIGHT_DATE	"1992, 1993"
#    pragma VERSIONID		TITLE " --- $Id: version.h,v 1.10 1993/04/22 16:56:31 bjoerns Exp $"
#endif

#endif

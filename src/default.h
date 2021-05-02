/* default.h,v 1.3 1992/05/11 15:31:07 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef _DEFAULT_H

typedef struct {
    /*
     * Control settings.
     */
    double  power;		    /* main */
    double  turnspeed;
    double  turnresistance;

    double  power_s;		    /* spare */
    double  turnspeed_s;
    double  turnresistance_s;

    /*
     * Miscellaneous data.
     */
    int	    team;
    double  fuel3;		    /* notify */
    double  fuel2;		    /* warning */
    double  fuel1;		    /* critical */

    /*
     * Display.
     */
    long    instruments;
#define SHOW_HUD_INSTRUMENTS	(1<<0)		    
#define SHOW_HUD_VERTICAL	(1<<1)
#define SHOW_HUD_HORIZONTAL	(1<<2)
#define SHOW_FUEL_METER		(1<<3)
#define SHOW_FUEL_GAUGE		(1<<4)
#define SHOW_TURNSPEED_METER	(1<<5)
#define SHOW_POWER_METER	(1<<6)
} def_t;

#define _DEFAULT_H
#endif

/* $Id: item.h,v 3.6 1993/06/28 20:53:41 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef ITEM_H
#define ITEM_H

#define ITEM_ENERGY_PACK	0
#define ITEM_WIDEANGLE_SHOT	1
#define ITEM_BACK_SHOT		2
#define ITEM_AFTERBURNER	3
#define ITEM_CLOAKING_DEVICE	4
#define ITEM_SENSOR_PACK	5
#define ITEM_TRANSPORTER	6
#define ITEM_TANK		7
#define ITEM_MINE_PACK		8
#define ITEM_ROCKET_PACK	9
#define ITEM_ECM		10
#define NUM_ITEMS		11

/* Each item is ITEM_SIZE x ITEM_SIZE */
#define ITEM_SIZE		16

#endif /* ITEM_H */

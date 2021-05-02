/* $Id: global.h,v 1.9 1992/08/27 00:25:56 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	GLOBAL_H
#define	GLOBAL_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <setjmp.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include "config.h"
#include "types.h"
#include "rules.h"
#include "object.h"
#include "map.h"
#include "draw.h"
#include "dbuff.h"
#include "bit.h"
#include "version.h"
#include "proto.h"


typedef struct {
    int max_num;
    char name[80];
    char host[80];
} server;


/*
 * Global data.
 */
extern player		*Players[];
extern object		*Obj[];
extern wireobj		ships[];
extern unsigned long	loops;
extern long		Id;
extern int		NumPlayers;
extern int		NumPseudoPlayers;
extern int		NumObjs;
extern int		NumRobots, WantedNumRobots;
extern World_map	World;
extern message_t	*Msg[];
extern server		Server;
extern int		RadarHeight;
extern jmp_buf		SavedEnv;
extern double		ShotsMass, ShipPower, ShipMass, ShotsSpeed, Gravity;
extern int		ShotsMax, ShotsLife;
extern bool		ShotsGravity, LooseMass;
extern long		DEF_BITS, KILL_BITS, DEF_HAVE, DEF_USED, USED_KILL;
extern Atom		ProtocolAtom, KillAtom;
extern long		GetInd[];
extern double		tbl_sin[];
extern XColor		colors[];
extern int		Delay;
extern int		Shutdown, ShutdownDelay;
extern int		RadarHeight;
extern bool		RawMode;
extern bool		NoQuit;

#endif

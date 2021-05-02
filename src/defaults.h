/* $Id: defaults.h,v 3.11 1994/07/10 19:43:34 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-94 by
 *
 *      Bjørn Stabell        (bjoerns@staff.cs.uit.no)
 *      Ken Ronny Schouten   (kenrsc@stud.cs.uit.no)
 *      Bert Gÿsbers         (bert@mc.bio.uva.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* $NCDId: @(#)defaults.h,v 1.1 1992/09/10 03:26:12 mellon Exp $ */

#ifndef DEFAULTS_H
#define DEFAULTS_H

typedef struct _valPair {
    struct _valPair *next;
    char *name;
    char *value;
    void *def;
} valPair;

enum valType {
	valVoid,	/* variable is not a variable */
	valInt,		/* variable is type int */
	valReal,	/* variable is type float */
	valBool,	/* variable is type bool */
	valIPos,	/* variable is type ipos */
	valString,	/* variable is type char* */
	valSec,		/* variable is type int (converted to frames) */
	valPerSec	/* variable is type float (converted to per-frame) */
};

typedef struct {
    char *name;
    char *commandLineOption;
    char *defaultValue;
    void *variable;
    enum valType type;
    char *helpLine;
} optionDesc;

#endif

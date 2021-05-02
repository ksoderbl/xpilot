/* $Id: defaults.h,v 3.20 1997/11/27 20:09:12 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-97 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
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
    const char		*name;
    const char		*commandLineOption;
    const char		*defaultValue;
    void		*variable;
    enum valType	type;
    void		(*tuner)(void);
    const char		*helpLine;
} optionDesc;

optionDesc*	findOption(const char* name);

#endif

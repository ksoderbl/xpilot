/* $Id: defaults.h,v 5.10 2001/11/30 11:47:19 bertg Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
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

#ifndef DEFAULTS_H
#define DEFAULTS_H

enum valType {
    valVoid,		/* variable is not a variable */
    valInt,		/* variable is type int */
    valReal,		/* variable is type float */
    valBool,		/* variable is type bool */
    valIPos,		/* variable is type ipos */
    valString,		/* variable is type char* */
    valSec,		/* variable is type int (converted to frames) */
    valPerSec,		/* variable is type float (converted to per-frame) */
    valList,		/* variable is a list of elements of type char* */
};


/*
 * bitflags for the origin of an option.
 */
enum _optOrigin {
    OPT_INIT		= 0,
    OPT_MAP		= 1,
    OPT_DEFAULTS	= 2,
    OPT_COMMAND		= 4,
    OPT_PASSWORD	= 8,
};
typedef enum _optOrigin optOrigin;


/*
 * extended bitflags for option origin.
 */
enum _optOriginAny {
    OPT_NONE		= 0,	/* not settable */
    OPT_ORIGIN_ANY	= 7,	/* allow any of {map,defaults,command} */
    OPT_VISIBLE		= 16,	/* can we query this option value? */
};


typedef struct _option_desc {
    const char		*name;
    const char		*commandLineOption;
    const char		*defaultValue;
    void		*variable;
    enum valType	type;
    void		(*tuner)(void);
    const char		*helpLine;
    int			flags;		/* allowable option origins. */
} option_desc;


option_desc*	Find_option_by_name(const char* name);
option_desc*	Get_option_descs(int *count_ptr);
bool		Option_add_desc(option_desc *desc);
void		Option_set_value(
			const char	*name,
			const char	*value,
			int		override,
			optOrigin	opt_origin);
char*		Option_get_value(const char *name, optOrigin *origin_ptr);

#endif

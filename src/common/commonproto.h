/* $Id: commonproto.h,v 5.7 2003/09/16 21:02:23 bertg Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
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

#ifndef	COMMONPROTO_H
#define	COMMONPROTO_H

#ifndef TYPES_H
/* need DFLOAT */
#include "types.h"
#endif

/* randommt.c */
extern void seedMT(unsigned int seed);
extern unsigned int reloadMT(void);
extern unsigned int randomMT(void);

/* math.c */
extern double rfrac(void);
extern int mod(int x, int y);
extern void Make_table(void);

/* strdup.c */
extern char *xp_strdup(const char *);
extern char *xp_safe_strdup(const char *old_string);

/* default.c */
unsigned String_hash(const char *s);

/* strlcpy.c */
size_t strlcpy(char *dest, const char *src, size_t size);
size_t strlcat(char *dest, const char *src, size_t size);

/* xpmemory.c */

void *xp_malloc(size_t size);
void *xp_realloc(void *oldptr, size_t size);
void *xp_calloc(size_t nmemb, size_t size);
void xp_free(void *p);
void *xp_safe_malloc(size_t size);
void *xp_safe_realloc(void *oldptr, size_t size);
void *xp_safe_calloc(size_t nmemb, size_t size);
void xp_safe_free(void *p);

#endif

/* $Id: xpmemory.c,v 5.0 2001/05/27 20:29:02 dik Exp $
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

#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "commonproto.h"

void *xp_malloc(size_t size)
{
    void	*p;

    p = (void *) malloc(size);

    return p;
}

void *xp_realloc(void *oldptr, size_t size)
{
    void	*p;

    p = (void *) realloc(oldptr, size);

    return p;
}

void *xp_calloc(size_t nmemb, size_t size)
{
    void	*p;

    p = (void *) calloc(nmemb, size);

    return p;
}

void xp_free(void *p)
{
    if (p) {
	free(p);
    }
}

void *xp_safe_malloc(size_t size)
{
    void	*p;

    p = (void *) malloc(size);
    if (p == NULL) {
	fatal("Not enough memory.");
    }

    return p;
}

void *xp_safe_realloc(void *oldptr, size_t size)
{
    void	*p;

    p = (void *) realloc(oldptr, size);
    if (p == NULL) {
	fatal("Not enough memory.");
    }

    return p;
}

void *xp_safe_calloc(size_t nmemb, size_t size)
{
    void	*p;

    p = (void *) calloc(nmemb, size);
    if (p == NULL) {
	fatal("Not enough memory.");
    }

    return p;
}

void xp_safe_free(void *p)
{
    if (p) {
	free(p);
    }
}


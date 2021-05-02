/* $Id: dbuff.c,v 3.24 1997/11/27 20:09:10 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-97 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
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

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xos.h>

#include "version.h"
#include "config.h"
#include "const.h"
#include "draw.h"
#include "bit.h"
#include "dbuff.h"

/* Favor DBE over MBX */
#if defined(DBE) && defined(MBX)
#undef	MBX
#endif

#if defined(MBX) || defined(DBE)
/* Needed for windows ... */
# include "paint.h"
#endif

#ifdef SPARC_CMAP_HACK
#include <fcntl.h>
#include <sys/ioctl.h>
#if defined(SVR4) || defined(__svr4__)
#include <sys/fbio.h>
#else
#include <sun/fbio.h>
#endif
#endif

char dbuff_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: dbuff.c,v 3.24 1997/11/27 20:09:10 bert Exp $";
#endif

#ifdef SPARC_CMAP_HACK
extern char   frameBuffer[MAX_CHARS]; /* frame buffer */
#endif


dbuff_state_t   *dbuf_state;	/* Holds current dbuff state */


static void release(dbuff_state_t *state)
{
    if (state != NULL) {
	if (state->colormaps[0] != NULL) free(state->colormaps[0]);
	if (state->colormaps[1] != NULL) free(state->colormaps[1]);
	if (state->planes != NULL) free(state->planes);
#ifdef MBX
	if (state->type == MULTIBUFFER
	    && state->buffer != 2) {
	    XmbufDestroyBuffers(state->display, draw);
	}
#endif
/* #ifdef DBE
	if (state->type == MULTIBUFFER
	    && state->buffer != 2) {
	    printf("release() for DBE?\n");
	}
   #endif */

	free(state);
    }
}


static long color(dbuff_state_t *state, long simple_color)
{
    long i, plane, computed_color;

    computed_color = state->pixel;
    for (plane=1, i=0; simple_color != 0; plane <<= 1, i++) {
	if (plane & simple_color) {
	    computed_color |= state->planes[i];
	    simple_color &= ~plane;
	}
    }
    return(computed_color);
}


dbuff_state_t *start_dbuff(Display *display, Colormap cmap,
			   dbuff_t type,
			   unsigned long planes, XColor *colors)
{
    dbuff_state_t *state;
    int i, high_mask, low_mask;


    state = (dbuff_state_t *) calloc(sizeof(dbuff_state_t), 1);
    if (state == NULL)
	return NULL;

    state->map_size = 1 << (2 * planes);
    state->colormaps[0] = (XColor *) malloc(state->map_size * sizeof(XColor));
    state->colormaps[1] = (XColor *) malloc(state->map_size * sizeof(XColor));
    state->planes = (unsigned long *) calloc ((2*planes) * sizeof(long), 1);
    if (state->colormaps[1] == NULL || state->colormaps[0] == NULL ||
	state->planes == NULL) {
	release(state);
	return(NULL);
    }
    state->display = display;
    state->cmap = cmap;

    state->type = type;

    switch (type) {

    case COLOR_SWITCH:
	if (XAllocColorCells(state->display, state->cmap, False,
			     state->planes, 2*planes, &state->pixel, 1) == 0) {
	    release(state);
	    return NULL;
	}

    default:
	state->buffer = 0;
	break;

    case MULTIBUFFER:
#ifdef MBX
	state->buffer = 2;
	if (!XmbufQueryExtension(display, &state->ev_base, &state->err_base)) {
	    release(state);
	    fprintf(stderr, "XmbufQueryExtension failed\n");
	    return(NULL);
	}
#endif
#ifdef DBE
	state->buffer = 2;
	if (!XdbeQueryExtension(display, &state->dbe_major, &state->dbe_minor)) {
	    release(state);
	    fprintf(stderr, "XdbeQueryExtension failed\n");
	    return (NULL);
	}
#endif

#if !defined(MBX) && !defined(DBE)
	printf("multibuffering support was not configured during compilation.\n");
	release(state);
	return(NULL);
#endif
	break;
    }

    state->masks[0] = AllPlanes;
    state->masks[1] = AllPlanes;

    for (i=0; i<planes; i++) {
	state->masks[0] &= ~state->planes[i];
	state->masks[1] &= ~state->planes[planes + i];
    }

    if (state->type == COLOR_SWITCH) {
	for (i=0; i<(1 << planes); i++) {
	    colors[i].pixel = color(state, i | (i << planes));
	    colors[i].flags = DoRed | DoGreen | DoBlue;
	}
    }
    else if (planes > 1) {
	for (i = 0; i < (1 << planes); i++) {
	    if (XAllocColor(display, cmap, &colors[i]) == False) {
		while (--i >= 0) {
		    XFreeColors(display, cmap, &colors[i].pixel, 1, 0);
		}
		release(state);
		return NULL;
	    }
	}
    }
    else {
	colors[WHITE].pixel = WhitePixel(display, DefaultScreen(display));
	colors[BLACK].pixel = BlackPixel(display, DefaultScreen(display));
	colors[BLUE].pixel  = WhitePixel(display, DefaultScreen(display));
	colors[RED].pixel   = WhitePixel(display, DefaultScreen(display));
    }


    low_mask = (1 << planes) - 1;
    high_mask = low_mask << planes;
    for (i=state->map_size-1; i>=0; i--) {
	state->colormaps[0][i] = colors[i & low_mask];
	state->colormaps[0][i].pixel = color(state, i);
	state->colormaps[1][i] = colors[(i & high_mask) >> planes];
	state->colormaps[1][i].pixel = color(state, i);
    }

    state->drawing_planes = state->masks[state->buffer];

    if (state->type == COLOR_SWITCH)
	XStoreColors(state->display, state->cmap,
		     state->colormaps[state->buffer], state->map_size);

#ifdef SPARC_CMAP_HACK
    if (state->type == COLOR_SWITCH) {
	state->fbfd = open(frameBuffer, O_RDONLY, 0);
	state->hardcmap.index = state->pixel;
	state->hardcmap.count = state->map_size;
	state->hardcmap.red = malloc(state->map_size);
	state->hardcmap.green = malloc(state->map_size);
	state->hardcmap.blue = malloc(state->map_size);
    } else {
	state->fbfd = -1;
    }
#endif

    return (state);
}

void dbuff_init(dbuff_state_t *state)
{
#ifdef MBX
    if (state->type == MULTIBUFFER) {
	if (state->buffer == 2) {
	    state->buffer = 0;
	    if (XmbufCreateBuffers(state->display, draw, 2,
				   MultibufferUpdateActionUndefined,
				   MultibufferUpdateHintFrequent,
				   state->draw) != 2) {
		perror("Couldn't create double buffering buffers");
		exit(1);
	    }
	}
	p_draw = state->draw[state->buffer];
    }
#endif
#ifdef DBE
    if (state->type == MULTIBUFFER) {
	if (state->buffer == 2) {
	    state->buffer = 0;
	    if (!(state->back_buffer = 
		  XdbeAllocateBackBufferName(state->display, draw, XdbeBackground))) {
		perror("Couldn't create double buffering back buffer");
		exit(1);
	    }
	}
	p_draw = state->back_buffer;
    }
#endif
}


void dbuff_switch(dbuff_state_t *state)
{
#ifdef MBX
    if (state->type == MULTIBUFFER)
	p_draw = state->draw[state->buffer];
#endif

    state->buffer ^= 1;

    if (state->type == COLOR_SWITCH) {
#ifdef SPARC_CMAP_HACK
	if (state->fbfd != -1) {
	    int		i;

	    for (i = 0; i < state->map_size; i++) {
		state->hardcmap.red[i] =
		    state->colormaps[state->buffer][i].red >> 8;
		state->hardcmap.green[i] =
		    state->colormaps[state->buffer][i].green >> 8;
		state->hardcmap.blue[i] =
		    state->colormaps[state->buffer][i].blue >> 8;
	    }
	    if (ioctl(state->fbfd, FBIOPUTCMAP, &state->hardcmap) == -1) {
		perror("ioctl FBIOPUTCMAP");
		close(state->fbfd);
		state->fbfd = -1;
	    }
	} else
#endif

	XStoreColors(state->display, state->cmap,
		     state->colormaps[state->buffer], state->map_size);
    }
#ifdef MBX
    else if (state->type == MULTIBUFFER) {
	XmbufDisplayBuffers(state->display, 1, &state->draw[state->buffer],
			    0, 200);
    }
#endif
#ifdef DBE
    else if (state->type == MULTIBUFFER) {
	XdbeSwapInfo	swap;
	swap.swap_window	= draw;
	swap.swap_action	= XdbeBackground;
	if (!XdbeSwapBuffers(state->display, &swap, 1)) {
	    perror("XdbeSwapBuffers failed");
	    exit(1);
	}
    }
#endif

    state->drawing_planes = state->masks[state->buffer];
}



void end_dbuff(dbuff_state_t *state)
{
    if (state->type == COLOR_SWITCH)
	XFreeColors(state->display, state->cmap,
		    &state->pixel, 1, ~(state->masks[0] & state->masks[1]));
    release(state);
}

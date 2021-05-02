/* $Id: dbuff.c,v 3.3 1993/06/28 20:53:31 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>

#include "client.h"
#include "draw.h"
#include "bit.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: dbuff.c,v 3.3 1993/06/28 20:53:31 bjoerns Exp $";
#endif


static void release(register dbuff_state_t *state)
{
    if (state != NULL) {
	if (state->colormaps[0] != NULL) free(state->colormaps[0]);
	if (state->colormaps[1] != NULL) free(state->colormaps[1]);
	if (state->planes != NULL) free(state->planes);
	free(state);
    }
}


static long color(register dbuff_state_t *state, register long simple_color)
{
    register long i, plane, computed_color;

    computed_color = state->pixel;
    for(plane=1, i=0; simple_color != 0; plane <<= 1, i++) {
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
    register dbuff_state_t *state;
    register i, high_mask, low_mask;


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

    if (state->type == COLOR_SWITCH) {
	if (XAllocColorCells(state->display, state->cmap, False,
			     state->planes, 2*planes, &state->pixel, 1) == 0) {
	    release(state);
	    return NULL;
	}
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

    state->buffer = 0;
    state->drawing_planes = state->masks[state->buffer];
    if (state->type == COLOR_SWITCH)
	XStoreColors(state->display, state->cmap,
		     state->colormaps[state->buffer], state->map_size);

    return (state);
}
    


void dbuff_switch(dbuff_state_t *state)
{
    state->buffer ^= 1;

    if (state->type == COLOR_SWITCH)
	XStoreColors(state->display, state->cmap,
		     state->colormaps[state->buffer], state->map_size);

    state->drawing_planes = state->masks[state->buffer];
}



void end_dbuff(dbuff_state_t *state)
{
    if (state->type == COLOR_SWITCH)
	XFreeColors(state->display, state->cmap,
		    &state->pixel, 1, ~(state->masks[0] & state->masks[1]));
    release(state);
}

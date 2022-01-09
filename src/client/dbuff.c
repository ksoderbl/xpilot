/* $Id: dbuff.c,v 5.1 2001/05/08 11:35:29 bertg Exp $
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

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xos.h>

#include "version.h"
#include "xpconfig.h"
#include "const.h"
#include "draw.h"
#include "bit.h"
#include "dbuff.h"

#if defined(MBX) || defined(DBE)
/* Needed for windows ... */
# include "paint.h"
#endif


char dbuff_version[] = VERSION;




#ifdef SPARC_CMAP_HACK
extern char   frameBuffer[MAX_CHARS]; /* frame buffer */
#endif


dbuff_state_t   *dbuf_state;	/* Holds current dbuff state */


static void dbuff_release(dbuff_state_t *state)
{
    if (state != NULL) {
	if (state->colormaps[0] != NULL) {
	    free(state->colormaps[0]);
	}
	if (state->colormaps[1] != NULL) {
	    free(state->colormaps[1]);
	}
	if (state->planes != NULL) {
	    free(state->planes);
	}
#ifdef MBX
	if (state->type == MULTIBUFFER
	    && state->colormap_index != 2) {
	    XmbufDestroyBuffers(state->display, draw);
	}
#endif

	free(state);
	state = NULL;
    }
}


static long dbuff_color(dbuff_state_t *state, long simple_color)
{
    long		i, plane, computed_color;

    computed_color = state->pixel;
    for (i = 0; simple_color != 0; i++) {
	plane = (1 << i);
	if (plane & simple_color) {
	    computed_color |= state->planes[i];
	    simple_color &= ~plane;
	}
    }

    return computed_color;
}


dbuff_state_t *start_dbuff(Display *display, Colormap xcolormap,
			   dbuff_t type,
			   int num_planes, XColor *colors)
{
    dbuff_state_t	*state;
    int			i, high_mask, low_mask;

    state = (dbuff_state_t *) calloc(1, sizeof(dbuff_state_t));
    if (state == NULL) {
	return NULL;
    }

    state->colormap_size = 1 << (2 * num_planes);
    state->colormaps[0] = (XColor *) malloc(state->colormap_size * sizeof(XColor));
    state->colormaps[1] = (XColor *) malloc(state->colormap_size * sizeof(XColor));
    state->planes = (unsigned long *) calloc(2 * num_planes, sizeof(long));
    if (state->colormaps[1] == NULL ||
	state->colormaps[0] == NULL ||
	state->planes == NULL) {

	dbuff_release(state);
	return NULL;
    }
    state->display = display;
    state->xcolormap = xcolormap;

    state->type = type;
    state->multibuffer_type = MULTIBUFFER_NONE;

    switch (type) {

    case PIXMAP_COPY:
	state->colormap_index = 0;
	break;

    case COLOR_SWITCH:
	if (XAllocColorCells(state->display,
			     state->xcolormap,
			     False,
			     state->planes,
			     2 * num_planes,
			     &state->pixel,
			     1) == 0) {
	    dbuff_release(state);
	    return NULL;
	}
	break;

    case MULTIBUFFER:
#ifdef DBE
	state->colormap_index = 2;
	state->multibuffer_type = MULTIBUFFER_DBE;
	if (!XdbeQueryExtension(display,
				&state->dbe.dbe_major,
				&state->dbe.dbe_minor)) {
	    dbuff_release(state);
	    fprintf(stderr, "XdbeQueryExtension failed\n");
	    return NULL;
	}
#elif defined(MBX)
	state->colormap_index = 2;
	state->multibuffer_type = MULTIBUFFER_MBX;
	if (!XmbufQueryExtension(display,
				 &state->mbx.mbx_ev_base,
				 &state->mbx.mbx_err_base)) {
	    dbuff_release(state);
	    fprintf(stderr, "XmbufQueryExtension failed\n");
	    return NULL;
	}
#else
	printf("Support for multibuffering was not configured.\n");
	dbuff_release(state);
	return NULL;
#endif
	break;

    default:
	fprintf(stderr, "Illegal dbuff_t %d\n", type);
	exit(1);
    }

    state->drawing_plane_masks[0] = AllPlanes;
    state->drawing_plane_masks[1] = AllPlanes;

    for (i = 0; i < num_planes; i++) {
	state->drawing_plane_masks[0] &= ~state->planes[i];
	state->drawing_plane_masks[1] &= ~state->planes[num_planes + i];
    }

    if (state->type == COLOR_SWITCH) {
	for (i = 0; i < (1 << num_planes); i++) {
	    colors[i].pixel = dbuff_color(state, i | (i << num_planes));
	    colors[i].flags = DoRed | DoGreen | DoBlue;
	}
    }
    else if (num_planes > 1) {
	for (i = 0; i < (1 << num_planes); i++) {
	    if (XAllocColor(display, xcolormap, &colors[i]) == False) {
		while (--i >= 0) {
		    XFreeColors(display, xcolormap, &colors[i].pixel, 1, 0);
		}
		dbuff_release(state);
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

    low_mask = (1 << num_planes) - 1;
    high_mask = low_mask << num_planes;
    for (i = state->colormap_size - 1; i >= 0; i--) {
	state->colormaps[0][i] = colors[i & low_mask];
	state->colormaps[0][i].pixel = dbuff_color(state, i);
	state->colormaps[1][i] = colors[(i & high_mask) >> num_planes];
	state->colormaps[1][i].pixel = dbuff_color(state, i);
    }

    state->drawing_planes = state->drawing_plane_masks[state->colormap_index];

    if (state->type == COLOR_SWITCH) {
	XStoreColors(state->display,
		     state->xcolormap,
		     state->colormaps[state->colormap_index],
		     state->colormap_size);
    }

    state->cmap_hack.fbfd = -1;
#ifdef SPARC_CMAP_HACK
    if (state->type == COLOR_SWITCH) {
	state->cmap_hack.fbfd = open(frameBuffer, O_RDONLY, 0);
	if (state->cmap_hack.fbfd != -1) {
	    state->cmap_hack.hardcmap.index = state->pixel;
	    state->cmap_hack.hardcmap.count = state->colormap_size;
	    state->cmap_hack.hardcmap.red = malloc(state->colormap_size);
	    state->cmap_hack.hardcmap.green = malloc(state->colormap_size);
	    state->cmap_hack.hardcmap.blue = malloc(state->colormap_size);
	}
    }
#endif

    return state;
}


void dbuff_init_buffer(dbuff_state_t *state)
{
#ifdef MBX
    if (state->type == MULTIBUFFER) {
	if (state->colormap_index == 2) {
	    state->colormap_index = 0;
	    if (XmbufCreateBuffers(state->display, draw, 2,
				   MultibufferUpdateActionUndefined,
				   MultibufferUpdateHintFrequent,
				   state->mbx.mbx_draw) != 2) {
		perror("Couldn't create double buffering buffers");
		exit(1);
	    }
	}
	p_draw = state->mbx.mbx_draw[state->colormap_index];
    }
#endif
#ifdef DBE
    if (state->type == MULTIBUFFER) {
	if (state->colormap_index == 2) {
	    state->colormap_index = 0;
	    state->dbe.dbe_draw =
		XdbeAllocateBackBufferName(state->display,
					   draw,
					   XdbeBackground);
	    if (state->dbe.dbe_draw == 0) {
		perror("Couldn't create double buffering back buffer");
		exit(1);
	    }
	}
	p_draw = state->dbe.dbe_draw;
    }
#endif
}


void dbuff_switch(dbuff_state_t *state)
{
#ifdef MBX
    if (state->type == MULTIBUFFER) {
	p_draw = state->mbx.mbx_draw[state->colormap_index];
    }
#endif

    state->colormap_index ^= 1;

    if (state->type == COLOR_SWITCH) {
#ifdef SPARC_CMAP_HACK
	if (state->cmap_hack.fbfd != -1) {
	    int		i;

	    for (i = 0; i < state->colormap_size; i++) {
		state->cmap_hack.hardcmap.red[i] =
		    state->colormaps[state->colormap_index][i].red >> 8;
		state->cmap_hack.hardcmap.green[i] =
		    state->colormaps[state->colormap_index][i].green >> 8;
		state->cmap_hack.hardcmap.blue[i] =
		    state->colormaps[state->colormap_index][i].blue >> 8;
	    }
	    if (ioctl(state->cmap_hack.fbfd, FBIOPUTCMAP,
		      &state->cmap_hack.hardcmap) == -1) {
		perror("ioctl FBIOPUTCMAP");
		close(state->cmap_hack.fbfd);
		state->cmap_hack.fbfd = -1;
	    }
	} else
#endif

	XStoreColors(state->display, state->xcolormap,
		     state->colormaps[state->colormap_index], state->colormap_size);
    }
#ifdef DBE
    else if (state->type == MULTIBUFFER) {
	XdbeSwapInfo		swap;

	swap.swap_window	= draw;
	swap.swap_action	= XdbeBackground;
	if (!XdbeSwapBuffers(state->display, &swap, 1)) {
	    perror("XdbeSwapBuffers failed");
	    exit(1);
	}
    }
#endif
#ifdef MBX
    else if (state->type == MULTIBUFFER) {
	XmbufDisplayBuffers(state->display, 1,
			    &state->mbx.mbx_draw[state->colormap_index],
			    0, 200);
    }
#endif

    state->drawing_planes = state->drawing_plane_masks[state->colormap_index];
}


void end_dbuff(dbuff_state_t *state)
{
    if (state->type == COLOR_SWITCH) {
	XFreeColors(state->display, state->xcolormap,
		    &state->pixel, 1,
		    ~(state->drawing_plane_masks[0] &
		      state->drawing_plane_masks[1]));
    }
    dbuff_release(state);
}


#ifdef DBE
static void dbuff_list_dbe(Display *display)
{
    XdbeScreenVisualInfo	*info;
    XdbeVisualInfo		*visinfo;
    int				n = 0;
    int				i, j;

    printf("\n");
    info = XdbeGetVisualInfo(display, NULL, &n);
    if (!info) {
	printf("Could not obtain double buffer extension info\n");
	return;
    }
    for (i = 0; i < n; i++) {
	printf("Visuals supporting double buffering on screen %d:\n", i);
	printf("%9s%9s%11s\n", "visual", "depth", "perflevel");
	for (j = 0; j < info[i].count; j++) {
	    visinfo = &info[i].visinfo[j];
	    printf("    0x%02x  %6d  %8d\n",
		    (unsigned) visinfo->visual,
		    visinfo->depth,
		    visinfo->perflevel);
	}
    }
    XdbeFreeVisualInfo(info);
}
#endif


void dbuff_list(Display *display)
{
#ifdef DBE
    dbuff_list_dbe(display);
#endif
}


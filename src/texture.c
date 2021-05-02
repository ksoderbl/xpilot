/* $Id: texture.c,v 3.3 1995/02/01 19:33:32 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
 *
 *      Bj�rn Stabell        (bjoerns@staff.cs.uit.no)
 *      Ken Ronny Schouten   (kenrsc@stud.cs.uit.no)
 *      Bert G�sbers         (bert@mc.bio.uva.nl)
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
#ifdef VMS
#include <unixio.h>
#include <unixlib.h>
#else
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <X11/Xlib.h>

#include "version.h"
#include "config.h"
#include "const.h"
#include "paint.h"
#include "xinit.h"
#include "error.h"
#include "xpmread.h"
#include "texture.h"

#ifndef PATH_MAX
#define PATH_MAX        1023
#endif

char texture_version[] = VERSION;

enum TextureStatus {
    TextureUnloaded = 0,
    TextureLoaded = 1,
    TextureError = 2
};

typedef struct texture_info {
    char		*name;
    char		**filename_ptr;
    char		**data;
    Pixmap		pixmap;
    enum TextureStatus	status;
} texture_info_t;

/*
 * XPM format pixmap data.
 */
#include "textures/rock4.xpm"

static texture_info_t wall_texture_info = {
    "wall",
    &wallTextureFile,
    rock4_xpm,
    None,
    TextureUnloaded
};

/*
 * Resolve the filename of the texture
 * and, if found, try to load it into a pixmap.
 */
static Pixmap Texture_load_from_file(char *filename)
{
    char		*dir, *colon;
    int			len;
    char		path[PATH_MAX + 1];

    if (!filename || !*filename) {
	return None;
    }

    /*
     * Always try the plain filename first,
     * without using the texturePath.
     */
    if (access(filename, R_OK) == 0) {
	return xpm_pixmap_from_file(filename);
    }

    /*
     * If filename doesn't start with a slash
     * then we also try the texturePath, if it exists.
     */
    if (*filename != '/' && texturePath != NULL) {
	for (dir = texturePath; *dir; dir = colon) {
	    if (!(colon = strchr(dir, ':'))) {
		len = strlen(dir);
		colon = &dir[len];
	    } else {
		len = colon - dir;
		colon++;
	    }
	    memcpy(path, dir, len);
	    sprintf(&path[len], "/%s", filename);
	    if (access(path, R_OK) == 0) {
		return xpm_pixmap_from_file(path);
	    }
	}
    }

    error("Can't find texture \"%s\"", filename);

    return None;
}

/*
 * The texture manager.
 */
static Pixmap Texture_load(texture_info_t *ti)
{
    Pixmap		pixmap;

    if (ti->status == TextureLoaded) {
	pixmap = ti->pixmap;
    }
    else if (ti->status != TextureError) {
	if (*ti->filename_ptr && **ti->filename_ptr) {
	    pixmap = Texture_load_from_file(*ti->filename_ptr);
	} else {
	    pixmap = None;
	}
	if (pixmap == None) {
	    pixmap = xpm_pixmap_from_data(ti->data);
	}
	ti->status = (pixmap == None) ? TextureError : TextureLoaded;
    }

    return pixmap;
}

/*
 * Load a texture for wall drawing.
 */
Pixmap Texture_wall(void)
{
    return Texture_load(&wall_texture_info);
}

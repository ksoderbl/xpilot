/* $Id: xpmread.c,v 4.5 2001/03/27 12:50:32 bert Exp $
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifndef _WINDOWS
# include <unistd.h>
# include <X11/Xlib.h>
# include <X11/Xutil.h>
#endif

#include "version.h"
#include "config.h"
#include "const.h"
#include "paint.h"
#include "xinit.h"
#include "error.h"
#include "commonproto.h"

#define XPM_READ_C
#include "xpmread.h"

char xpmread_version[] = VERSION;

/* Kludge for visuals under C++ */
#if defined(__cplusplus)
#define class c_class
#endif

static const char *xpm_key_strings[XPM_nkeys] = {
    "m",
    "g4",
    "g",
    "c",
    "s"
};

static void xpm_print_error(XPM_read *xpmr)
{
    char		buf[1024 + 256];

    sprintf(buf, "XPM read %s%s%s",
	    xpmr->filename ? xpmr->filename : "internal data",
	    xpmr->error_str ? ": " : "",
	    xpmr->error_str ? xpmr->error_str : "");
    error(buf);
}

static int xpm_read_error(XPM_read *xpmr, const char *err_str)
{
    xpmr->error_str = err_str;
    return -1;
}

/*
 * Split up the input string stream into tokens.
 * Since we only really need double-quote-separated strings
 * we ignore the other details.
 *
 * Note that there is no backslash processing done for
 * escaped characters within strings or nul-bytes.
 * We just assume that all XPM writers will be sensible
 * enough not to use those tricks.
 */
static char *xpm_next_token(XPM_read *xpmr)
{
    char		*ptr, *tok;

    /* free any previous token */
    if (xpmr->token) {
	free(xpmr->token);
	xpmr->token = NULL;
    }

    ptr = xpmr->ptr;

    do {
	/* skip whitespace */
	while (isascii(*ptr) && isspace(*ptr)) {
	    ptr++;
	}

	/* where token starts */
	tok = ptr;

	/* skip comments. */
	if (*ptr == '/' && ptr[1] == '*') {
	    for (ptr += 2; *ptr; ptr++) {
		if (*ptr == '*' && ptr[1] == '/') {
		    ptr += 2;
		    break;
		}
	    }
	}

	/* repeat until token does not point to whitespace or comments. */
    } while (*ptr && ptr > tok);

    /* if some data remains... */
    if (*ptr) {

	/* find end of token and let `ptr' point to last char of token. */

	/* if it's a string. */
	if (*ptr == '"') {
	    /* find end of string. */
	    for (ptr++; *ptr; ptr++) {
		if (*ptr == '"') {
		    break;
		}
	    }
	}
	/* if it's something else. */
#define ISKEYWORD(c)	(isupper(c) || islower(c) || (c) == '_')
#define ISKEYWORDEXT(c)	(ISKEYWORD(c) || isdigit(c))
	else if (isascii(*ptr) && ISKEYWORD(*ptr)) {
	    for (ptr++; *ptr; ptr++) {
		if (!isascii(*ptr) || !ISKEYWORDEXT(*ptr)) {
		    break;
		}
	    }
	    /* one too far in all cases, so backup one position. */
	    ptr--;
	}

	/* if a token remains copy it. */
	if (*ptr) {
	    int len = ptr - tok + 1;
	    if ((xpmr->token = (char *)malloc(len + 1)) != NULL) {
		memcpy(xpmr->token, tok, len);
		xpmr->token[len] = '\0';
	    }
	    /* point past token now. */
	    ptr++;
	}

	/* store next input position. */
	xpmr->ptr = ptr;
    }

    /* found a new token if non-zero. */
    return xpmr->token;
}

/*
 * Find the next double-quoted-string in the input stream.
 * Return NULL if none remains.
 */
static char *xpm_next_string(XPM_read *xpmr)
{
    if (xpmr->static_data) {
	if (xpmr->token) {
	    free(xpmr->token);
	}
	/* point to next string. */
	xpmr->static_data++;
	/* static_data may point to read-only memory (Linux). */
	xpmr->token = xp_strdup(*xpmr->static_data);
    }
    else {
	/* skip any non-string tokens. */
	while (xpm_next_token(xpmr)) {
	    if (xpmr->token[0] == '"') {
		/* strip the string double quotes. */
		char *str = xp_strdup(xpmr->token + 1);
		free(xpmr->token);
		xpmr->token = str;
		if (str && (str = strchr(str, '"')) != NULL) {
		    *str = '\0';
		}
		break;
	    }
	}
    }

    /* found a new string if non-zero. */
    return xpmr->token;
}

static int xpm_parse_data(XPM_read *xpmr)
{
    unsigned		size;
    int			i, j, k, m, num_keys;
    char		*key, *str;
    unsigned char	*pixelp;

    if (sscanf(xpmr->token, "%u %u %u %u",
	       &xpmr->xpm->width, &xpmr->xpm->height,
	       &xpmr->xpm->ncolors, &xpmr->xpm->cpp) != 4) {
	return xpm_read_error(xpmr, "Incorrect values specification");
    }

    if (xpmr->xpm->ncolors > 256) {
	return xpm_read_error(xpmr, "Too many colors defined");
    }

    size = xpmr->xpm->ncolors * sizeof(XPM_color);
    if (!(xpmr->xpm->colors = (XPM_color *)malloc(size))) {
	return xpm_read_error(xpmr, "Not enough memory");
    }
    memset(xpmr->xpm->colors, 0, size);

    size = xpmr->xpm->ncolors * sizeof(char *);
    if (!(xpmr->chars_ptr = (char **)malloc(size))) {
	return xpm_read_error(xpmr, "Not enough memory");
    }
    memset(xpmr->chars_ptr, 0, size);

    size = xpmr->xpm->ncolors * (xpmr->xpm->cpp + 1);
    if (!(xpmr->chars_mem = (char *)malloc(size))) {
	return xpm_read_error(xpmr, "Not enough memory");
    }
    memset(xpmr->chars_mem, 0, size);

    for (i = 0; i < xpmr->xpm->ncolors; i++) {
	xpmr->chars_ptr[i] = &xpmr->chars_mem[i * (xpmr->xpm->cpp + 1)];
	if (!xpm_next_string(xpmr)) {
	    return xpm_read_error(xpmr, "Premature end-of-data");
	}
	for (j = 0; j < xpmr->xpm->cpp; j++) {
	    if (!(xpmr->chars_ptr[i][j] = xpmr->token[j])) {
		return xpm_read_error(xpmr, "Incomplete color specification");
	    }
	}
	xpmr->chars_ptr[i][j] = '\0';
	num_keys = 0;
	for (key = strtok(&xpmr->token[j + 1], " \t");
	     key != NULL && (str = strtok(NULL, " \t")) != NULL;
	     key = strtok(NULL, " \t")) {
	     for (k = 0; k < XPM_nkeys; k++) {
		 if (!strcmp(xpm_key_strings[k], key)) {
		     if (xpmr->xpm->colors[i].keys[k]) {
			 /* key already defined for this color! */
			 break;
		     }
		     xpmr->xpm->colors[i].keys[k] = xp_strdup(str);
		     num_keys++;
		     break;
		 }
	     }
	}
	if (num_keys == 0) {
	    return xpm_read_error(xpmr, "Incomplete color specification");
	}
    }

    size = xpmr->xpm->width * xpmr->xpm->height;
    if (!(xpmr->xpm->pixels = (unsigned char *)malloc(size))) {
	return xpm_read_error(xpmr, "Not enough memory");
    }
    pixelp = xpmr->xpm->pixels;
    for (j = 0; j < xpmr->xpm->height; j++) {
	if (!xpm_next_string(xpmr)) {
	    return xpm_read_error(xpmr, "Premature end-of-data");
	}
	str = xpmr->token;
	for (i = 0; i < xpmr->xpm->width; i++) {
	    for (k = 0; k < xpmr->xpm->ncolors; k++) {
		for (m = 0; m < xpmr->xpm->cpp; m++) {
		    if (xpmr->chars_ptr[k][m] != str[m]) {
			break;
		    }
		}
		if (m == xpmr->xpm->cpp) {
		    *pixelp = k;
		    break;
		}
	    }
	    if (k == xpmr->xpm->ncolors) {
		return xpm_read_error(xpmr, "Unmatched pixel");
	    }
	    pixelp++;
	    str += xpmr->xpm->cpp;
	}
    }

    return 0;
}

static int xpm_parse_buffer(XPM_read *xpmr)
{
    static const char	XPM_header[] = "/* XPM */";

    if ((xpmr->ptr = strstr(xpmr->ptr, XPM_header)) == NULL) {
	return xpm_read_error(xpmr, "Can't find XPM header");
    }
    xpmr->ptr += strlen(XPM_header);

    while (xpm_next_token(xpmr)) {
	if (!strcmp(xpmr->token, "{")) {	/* no matching "}" */
	    break;
	}
    }
    if (!xpm_next_string(xpmr)) {
	return xpm_read_error(xpmr, "Premature end-of-data");
    }

    return xpm_parse_data(xpmr);
}

static int xpm_load_data(XPM_read *xpmr)
{
    int			fd;
    unsigned		size;
    struct stat		st;

    if ((fd = open(xpmr->filename, O_RDONLY)) == -1) {
	return xpm_read_error(xpmr, NULL);
    }
    if (fstat(fd, &st)) {
	close(fd);
	return xpm_read_error(xpmr, xpmr->filename);
    }
    size = (unsigned)st.st_size;
    xpmr->data_size = size;
    if (!(xpmr->data = (char *)malloc(size + 1))) {
	close(fd);
	return xpm_read_error(xpmr, "Not enough memory");
    }
    if (read(fd, xpmr->data, size) != size) {
	free(xpmr->data);
	xpmr->data = NULL;
	close(fd);
	return xpm_read_error(xpmr, xpmr->filename);
    }
    close(fd);
    xpmr->data[size] = '\0';
    xpmr->ptr = xpmr->data;

    return 0;
}

static void xpm_free_xpmr(XPM_read *xpmr)
{
    if (xpmr->data) {
	free(xpmr->data);
	xpmr->data = NULL;
    }
    if (xpmr->token) {
	free(xpmr->token);
	xpmr->token = NULL;
    }
    if (xpmr->chars_ptr) {
	free(xpmr->chars_ptr);
	xpmr->chars_ptr = NULL;
    }
    if (xpmr->chars_mem) {
	free(xpmr->chars_mem);
	xpmr->chars_mem = NULL;
    }
}

static void xpm_free_xpm(XPM *xpm)
{
    if (xpm->colors) {
	free(xpm->colors);
	xpm->colors = NULL;
    }
    if (xpm->pixels) {
	free(xpm->pixels);
	xpm->pixels = NULL;
    }
}

static int xpm_read_xpm_from_file(char *filename, XPM *xpm)
{
    XPM_read		xpmr;

    memset(&xpmr, 0, sizeof(xpmr));
    xpmr.filename = filename;
    xpmr.xpm = xpm;

    if (xpm_load_data(&xpmr) == 0) {
	xpm_parse_buffer(&xpmr);
	xpm_free_xpmr(&xpmr);
    }
    if (xpmr.error_str) {
	xpm_print_error(&xpmr);
	xpm_free_xpm(xpm);
	return -1;
    }
    return 0;
}

static int xpm_read_xpm_from_data(const char **data, XPM *xpm)
{
    XPM_read		xpmr;

    memset(&xpmr, 0, sizeof(xpmr));
    xpmr.xpm = xpm;
    xpmr.static_data = data;
    xpmr.token = xp_strdup(*xpmr.static_data);

    xpm_parse_data(&xpmr);
    xpm_free_xpmr(&xpmr);
    if (xpmr.error_str) {
	xpm_print_error(&xpmr);
	xpm_free_xpm(xpm);
	return -1;
    }
    return 0;
}

static int xpm_colors_to_pixels(XPM *xpm, enum XPM_key key,
				unsigned long *pixels)
{
    int			i, j, k;
    XColor		xcolor;
    char		*color_name;

    for (i = 0; i < xpm->ncolors; i++) {
	memset(&xcolor, 0, sizeof(xcolor));
	color_name = NULL;
	for (k = key; k >= 0; k--) {
	    if (xpm->colors[i].keys[k]) {
		color_name = xpm->colors[i].keys[k];
		if (XParseColor(dpy, DefaultColormap(dpy, DefaultScreen(dpy)),
				color_name, &xcolor)) {
		    break;
		}
		printf("Can't parse color \"%s\"\n", color_name);
		color_name = NULL;
	    }
	}
	if (!color_name) {
	    for (k = key + 1; k <= XPM_c; k++) {
		if (xpm->colors[i].keys[k]) {
		    color_name = xpm->colors[i].keys[k];
		    if (XParseColor(dpy, DefaultColormap(dpy, DefaultScreen(dpy)),
				    color_name, &xcolor)) {
			break;
		    }
		    printf("Can't parse color \"%s\"\n", color_name);
		    color_name = NULL;
		}
	    }
	}
	if (!color_name) {
	    if (xpm->colors[i].keys[XPM_s]
		&& !strcmp(xpm->colors[i].keys[XPM_s], "None")) {
		pixels[i] = colors[BLACK].pixel;
	    }
	    else {
		return -1;
	    }
	}
	else {
	    unsigned	dr, dg, db, dist, mindist = UINT_MAX;

	    for (j = 0; j < maxColors; j++) {
		dr = xcolor.red - colors[j].red;
		dg = xcolor.green - colors[j].green;
		db = xcolor.blue - colors[j].blue;
		dist = sqr(dr) + sqr(dg) + sqr(db);
		if (dist < mindist) {
		    mindist = dist;
		    pixels[i] = colors[j].pixel;
		}
	    }
	}
    }
    return 0;
}

static XImage *xpm_convert_to_image(XPM *xpm)
{
    XImage		*img;
    int			i, j;
    unsigned char	*ptr;
    enum XPM_key	key;
    unsigned long	pixels[256];

    if (mono) {
	key = XPM_m;
    }
    else if (visual->class == GrayScale) {
	if (maxColors == 4) {
	    key = XPM_g4;
	}
	else {
	    key = XPM_g;
	}
    }
    else {
	key = XPM_c;
    }

    if (xpm_colors_to_pixels(xpm, key, pixels)) {
	return NULL;
    }

    img = XCreateImage(dpy, visual, dispDepth, ZPixmap,
		       0, NULL, xpm->width, xpm->height,
		       (dispDepth <= 8) ? 8 : (dispDepth <= 16) ? 16 : 32,
		       0);
    if (!img) {
	error("Can't create XImage");
	return NULL;
    }
    if (!(img->data = (char *)malloc(img->bytes_per_line * xpm->height))) {
	error("Can't allocate XImage data");
	XDestroyImage(img);
	return NULL;
    }
    ptr = (unsigned char *)xpm->pixels;
    for (j = 0; j < xpm->height; j++) {
	for (i = 0; i < xpm->width; i++) {
	    XPutPixel(img, i, j, pixels[*ptr]);
	    ptr++;
	}
    }
    return img;
}

static Pixmap xpm_image_to_pixmap(XPM *xpm, XImage *img)
{
    Pixmap		pixmap;
    GC			pixgc;

    pixmap = XCreatePixmap(dpy, p_draw, xpm->width, xpm->height, dispDepth);
    if (pixmap != None) {
	pixgc = XCreateGC(dpy, pixmap, 0, NULL);
	XPutImage(dpy, pixmap, pixgc, img, 0, 0, 0, 0, xpm->width, xpm->height);
	XFreeGC(dpy, pixgc);
    } else {
	error("Can't create XPM pixmap");
    }
    return pixmap;
}

static Pixmap xpm_convert_to_pixmap(XPM *xpm)
{
    Pixmap		pixmap;
    XImage		*img;

    if (!(img = xpm_convert_to_image(xpm))) {
	return None;
    }
    pixmap = xpm_image_to_pixmap(xpm, img);
    free(img->data);
    img->data = NULL;
    XDestroyImage(img);

    return pixmap;
}

Pixmap xpm_pixmap_from_data(const char **data)
{
    XPM			xpm;
    Pixmap		pixmap;

    memset(&xpm, 0, sizeof(xpm));
    if (xpm_read_xpm_from_data(data, &xpm)) {
	return None;
    }
    pixmap = xpm_convert_to_pixmap(&xpm);
    xpm_free_xpm(&xpm);
    return pixmap;
}

Pixmap xpm_pixmap_from_file(char *filename)
{
    XPM			xpm;
    Pixmap		pixmap;

    memset(&xpm, 0, sizeof(xpm));
    if (xpm_read_xpm_from_file(filename, &xpm)) {
	return None;
    }
    pixmap = xpm_convert_to_pixmap(&xpm);
    xpm_free_xpm(&xpm);
    return pixmap;
}

XImage *xpm_image_from_pixmap(Pixmap pixmap)
{
    XImage		*img;
    Window		rootw;
    int			x, y;
    unsigned		width, height, border_width, depth;

    if (!XGetGeometry(dpy, pixmap, &rootw,
		      &x, &y,
		      &width, &height,
		      &border_width, &depth)) {
	error("Can't get pixmap geometry");
	return NULL;
    }
    img = XGetImage(dpy, pixmap,
		    0, 0,
		    width, height,
		    AllPlanes, ZPixmap);
    if (!img) {
	error("Can't get Image from Pixmap");
	return NULL;
    }
    return img;
}

#if 0
/* some unfinished stuff for xpmwrite.c */

char **xpm_data_from_image(XImage *img)
{
    char		*mem;
    char		**data;
    int			i;
    unsigned long	apixel;
    unsigned char	*pixels, *ptr;
    unsigned		x, y, ncolors, cpp, num_ptrs, size;
    unsigned		img_colors[256];
    XColor		xcolors[256];

    if (!(pixels = (char *)malloc(img->width * img->height))) {
	error("Not enough memory");
	return NULL;
    }
    memset(img_colors, -1, sizeof(img_colors));
    ptr = pixels;
    for (y = 0; y < img->height; y++) {
	for (x = 0; x < img->height; x++) {
	    *ptr = (unsigned char)XGetPixel(img, x, y);
	    img_colors[*ptr] = 0;
	    ptr++;
	}
    }
    for (i = 0; i < 256; i++) {
	ncolors += !img_colors[i];
    }
    cpp = (ncolors <= 16) ? 2 : 1;
    num_ptrs = img->height + 2 + ncolors;
    size = (6 + 6 + 4 + 4)
	 + ncolors * (2 + 2 + 2 + 10)
	 + img->height * img->width * cpp
	 + num_ptrs * sizeof(char *);
    if (!(data = (char **)malloc(size))) {
	free(pixels);
	return NULL;
    }
    mem = (char *)(data + num_ptrs);
    sprintf(mem, "%u %u %u %u", img->width, img->height, ncolors, cpp);
    data[0] = mem;
    mem += strlen(mem) + 1;
    if (ncolors <= 16) {
	for (i = 0; i < ncolors; i++) {
	    sprintf(mem, "%c c %u %u", 'a' + i, img->height, ncolors, cpp);
	    data[0] = mem;
	    mem += strlen(mem) + 1;
	}
    }
    sprintf(mem, "");
    data = (char **)mem;

    size = igm->width * sizeof(char) + img->height;
    if (!(data = (char **)malloc())) {
    }
}

char **xpm_data_from_pixmap(Pixmap pixmap)
{
    char		**data = NULL;
    XImage		*img;

    if ((img = xpm_image_from_pixmap(pixmap)) != NULL) {
	if (!(data = xpm_data_from_image(img))) {
	}
    }
}

#endif


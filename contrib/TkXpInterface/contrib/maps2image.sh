---- Cut Here and feed the following to sh ----
#!/bin/sh
# This is a shell archive (produced by GNU sharutils 4.2c).
# To extract the files from this archive, save it to some FILE, remove
# everything before the `!/bin/sh' line above, then type `sh FILE'.
#
# Made on 1999-10-16 18:01 MEST by <root@nail>.
# Source directory was `/tmp/maps2image'.
#
# Existing files will *not* be overwritten unless `-c' is specified.
#
# This shar contains:
# length mode       name
# ------ ---------- ------------------------------------------
#   3164 -rw-rw-r-- README
#    863 -rw-rw-rw- COPYRIGHT
#   1027 -rw-rw-r-- Makefile
#  10965 -rw-rw-r-- maps2image.c
#   5637 -rw-rw-r-- bdf2c.c
#   5025 -r--r--r-- helvR08.bdf.gz
#
save_IFS="${IFS}"
IFS="${IFS}:"
gettext_dir=FAILED
locale_dir=FAILED
first_param="$1"
for dir in $PATH
do
  if test "$gettext_dir" = FAILED && test -f $dir/gettext \
     && ($dir/gettext --version >/dev/null 2>&1)
  then
    set `$dir/gettext --version 2>&1`
    if test "$3" = GNU
    then
      gettext_dir=$dir
    fi
  fi
  if test "$locale_dir" = FAILED && test -f $dir/shar \
     && ($dir/shar --print-text-domain-dir >/dev/null 2>&1)
  then
    locale_dir=`$dir/shar --print-text-domain-dir`
  fi
done
IFS="$save_IFS"
if test "$locale_dir" = FAILED || test "$gettext_dir" = FAILED
then
  echo=echo
else
  TEXTDOMAINDIR=$locale_dir
  export TEXTDOMAINDIR
  TEXTDOMAIN=sharutils
  export TEXTDOMAIN
  echo="$gettext_dir/gettext -s"
fi
if (echo "testing\c"; echo 1,2,3) | grep c >/dev/null; then
  if (echo -n testing; echo 1,2,3) | sed s/-n/xn/ | grep xn >/dev/null; then
    shar_n= shar_c='
'
  else
    shar_n=-n shar_c=
  fi
else
  shar_n= shar_c='\c'
fi
touch -am 1231235999 $$.touch >/dev/null 2>&1
if test ! -f 1231235999 && test -f $$.touch; then
  shar_touch=touch
else
  shar_touch=:
  echo
  $echo 'WARNING: not restoring timestamps.  Consider getting and'
  $echo "installing GNU \`touch', distributed in GNU File Utilities..."
  echo
fi
rm -f 1231235999 $$.touch
#
$echo $shar_n 'x -' 'lock directory' "\`_sh02319': "$shar_c
if mkdir _sh02319; then
  $echo 'created'
else
  $echo 'failed to create'
  exit 1
fi
# ============= README ==============
if test -f 'README' && test "$first_param" != -c; then
  $echo 'x -' SKIPPING 'README' '(file already exists)'
else
  $echo 'x -' extracting 'README' '(text)'
  sed 's/^X//' << 'SHAR_EOF' > 'README' &&
README for maps2image -- a xpilot maps to image converter.
X
WHAT IS IT?
This program will take a set of xpilot maps and builds a
single PBM image from them. Each image is labeled with the
map's name below it.
X
WHAT USE IS IT?
If you have a lot of xpilot maps, this is a simple way to
remind yourself of what the different maps are/look like.
Each image will show only the walls of a map. Normally this
is enough to remind you of what it really looks like when
you actually play the map.
X
HOW DO I COMPILE?
Simple. If you have gcc, you don't need to change anything.
If you do not have gcc, you need to change the Makefile to
reflect the ANSI compiler. Follow the ``detailed''
instructions in the Makefile to do this. :-)
And now, the big moment has arrived! Type:
X
X	make
X
If everything goes well, you should now have the executable
maps2image.
(Note: gcc gives a warning about some unreachable code. This
warning is bogus and should thusly be ignored!)
X
If, for some reason, you do not like the font that is used,
you can change the FONT= line in the Makefile. This should
point to a BDF format fontfile. Included is the 8 point
Helvetica font from the 75dpi directory of the X11R5
distribution. You can substitute any BDF font here. However,
if you use a large font, the map name will not be completely
displayed.
X
HOW DO I USE IT?
The list of maps you want converted to an image are given on
the command line and the PBM image is written to stdout.
Here is a typical use of the program:
X
X	maps2image /usr/local/lib/xpilot/maps/*.map | xv -
X
The program has a couple of optional switches:
X
X	-s image_size	specifies the size of each of the individual
X			map images.  The default is 64.
X
X	-x ncols	specifies the number of columns of map images
X			to use.  If this switch is not specified, it will
X			try to make the image as square as possible.
X	-i		invert the images.  By default areas you can fly
X			in are white, with this option it is more like
X			what it looks like when playing.
X	-v		Turn on verbose mode.  This will output progress
X			messages to stderr.  Useful only when bored.
X	-b		Instead of a pbm image, create an X11 bitmap
X	-l		Do not add a label to each image
X
The list of files should be xpilot map files.  They may be compressed
with gzip or compress.  (Standard extension looked for are .gz and .Z)
X
PBM???  WHAT'S THAT?
The PBM image is format is a simple Portable BitMap format.
Most X based image viewers can deal with PBM files, but you
can always get the latest pbmplus image conversion package
to convert this image to whatever format you want.
X
SO, NOW WHAT?
Now it is your job to write an X program which will display
this image and then allow you to select a map you want to
start a server with... What? Are you still reading this
instead of writing this xpilots starter????
X
COMPLAINTS?
You can complain to Andrew@SDSU.Edu (Ender) if you want.
However all whines will be ignored. If you don't like the
program, don't use it; I assume you were doing ok without it
before you got it... If you find bugs or have other
suggestions, let me know. I have only compiled this on a Sun
using both gcc 2.5.8 and Sun's acc. (We run SunOS 4.1.3_U1)
X
SHAR_EOF
  $shar_touch -am 1016175999 'README' &&
  chmod 0664 'README' ||
  $echo 'restore of' 'README' 'failed'
  if ( md5sum --help </dev/null 2>&1 | grep 'sage: md5sum \[' ) >/dev/null 2>&1 \
  && ( md5sum --version </dev/null 2>&1 | grep -v 'textutils 1.12' ) >/dev/null; then
    md5sum -c << SHAR_EOF >/dev/null 2>&1 \
    || $echo 'README:' 'MD5 check failed'
5f7c7c1b3eb209cd3112d9fe9452fe30  README
SHAR_EOF
  else
    shar_count="`LC_ALL=C wc -c < 'README'`"
    test 3164 -eq "$shar_count" ||
    $echo 'README:' 'original size' '3164,' 'current size' "$shar_count!"
  fi
fi
# ============= COPYRIGHT ==============
if test -f 'COPYRIGHT' && test "$first_param" != -c; then
  $echo 'x -' SKIPPING 'COPYRIGHT' '(file already exists)'
else
  $echo 'x -' extracting 'COPYRIGHT' '(text)'
  sed 's/^X//' << 'SHAR_EOF' > 'COPYRIGHT' &&
Copyright (c) 1994 by Andrew Scherpbier.
You may do what you wish with this package as long as it
isn't going to cost me anything! If you find it so useful
that you can sell it and make a $1,000,000, good for you!
More power to you! If this packages causes any damage, don't
blame me. You didn't have to compile/run it...
X
Comments/Questions to:
____________________________________________________________________________
Andrew "Race Turtle" Scherpbier, SysMgr
College of Sciences                               ____
San Diego State University                       /OOOO\,P
EMAIL:	Andrew@SDSU.Edu                         ,------'
VOICE:	(619) 594-5026                            U   U
FAX:	(619) 594-6381
"Sometimes, when I think that things cannot possibly get any worse, they do"
____________________________________________________________________________
X
X
SHAR_EOF
  $shar_touch -am 1016175999 'COPYRIGHT' &&
  chmod 0666 'COPYRIGHT' ||
  $echo 'restore of' 'COPYRIGHT' 'failed'
  if ( md5sum --help </dev/null 2>&1 | grep 'sage: md5sum \[' ) >/dev/null 2>&1 \
  && ( md5sum --version </dev/null 2>&1 | grep -v 'textutils 1.12' ) >/dev/null; then
    md5sum -c << SHAR_EOF >/dev/null 2>&1 \
    || $echo 'COPYRIGHT:' 'MD5 check failed'
e82fb9dfc0edcf9cb955deef0c4944a6  COPYRIGHT
SHAR_EOF
  else
    shar_count="`LC_ALL=C wc -c < 'COPYRIGHT'`"
    test 863 -eq "$shar_count" ||
    $echo 'COPYRIGHT:' 'original size' '863,' 'current size' "$shar_count!"
  fi
fi
# ============= Makefile ==============
if test -f 'Makefile' && test "$first_param" != -c; then
  $echo 'x -' SKIPPING 'Makefile' '(file already exists)'
else
  $echo 'x -' extracting 'Makefile' '(text)'
  sed 's/^X//' << 'SHAR_EOF' > 'Makefile' &&
#
# Makefile for maps2image.
#
# Change the following line to reflect your ANSI C compiler
#
CC	=	gcc
CFLAGS	=	-O
OBJS	=	maps2image.o
LIBS	=	-lm
TARGET	=	maps2image
X
#
# Change the FONT macro to point to whatever font you want
# to use to display the map names below the images.
#
FONT	=	helvR08.bdf
X
all:		$(FONT) font.h maps2image bdf2c
X
helvR08.bdf:	helvR08.bdf.gz
X	gzip -cd < helvR08.bdf.gz > helvR08.bdf
X
helvR10.bdf:	helvR10.bdf.gz
X	gzip -cd < helvR10.bdf.gz > helvR10.bdf
X
helvR12.bdf:	helvR12.bdf.gz
X	gzip -cd < helvR12.bdf.gz > helvR12.bdf
X
helvR14.bdf:	helvR14.bdf.gz
X	gzip -cd < helvR14.bdf.gz > helvR14.bdf
X
$(TARGET):	$(OBJS) font.h
X	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
X
font.h:	bdf2c Makefile
X	./bdf2c $(FONT) >font.h
X
bdf2c:	bdf2c.o
X	$(CC) $(CFLAGS) -o bdf2c bdf2c.o
X
install:
X	@echo "Sorry, I am too lazy to install.  Do it yourself."
X
depend:
X	makedepend -- $(CFLAGS) -- $(OBJS:%.o=%.c)
X
shar:
X	gzip helvR*.bdf
X	shar -c -p README COPYRIGHT Makefile maps2image.c bdf2c.c -B helvR*.gz >maps2image.shar
X
SHAR_EOF
  $shar_touch -am 1016180099 'Makefile' &&
  chmod 0664 'Makefile' ||
  $echo 'restore of' 'Makefile' 'failed'
  if ( md5sum --help </dev/null 2>&1 | grep 'sage: md5sum \[' ) >/dev/null 2>&1 \
  && ( md5sum --version </dev/null 2>&1 | grep -v 'textutils 1.12' ) >/dev/null; then
    md5sum -c << SHAR_EOF >/dev/null 2>&1 \
    || $echo 'Makefile:' 'MD5 check failed'
f86501324f9df682ed612f7c261fe603  Makefile
SHAR_EOF
  else
    shar_count="`LC_ALL=C wc -c < 'Makefile'`"
    test 1027 -eq "$shar_count" ||
    $echo 'Makefile:' 'original size' '1027,' 'current size' "$shar_count!"
  fi
fi
# ============= maps2image.c ==============
if test -f 'maps2image.c' && test "$first_param" != -c; then
  $echo 'x -' SKIPPING 'maps2image.c' '(file already exists)'
else
  $echo 'x -' extracting 'maps2image.c' '(text)'
  sed 's/^X//' << 'SHAR_EOF' > 'maps2image.c' &&
/**********************************************************************/
/*                                                                    */
/*  File:          maps2image.c                                       */
/*  Author:        Andrew W. Scherpbier                               */
/*  Version:       1.00                                               */
/*  Created:       27 Feb 1994                     		      */
/*                                                                    */
/*  Copyright (c) 1994 Andrew Scherpbier                              */
/*                All Rights Reserved.                                */
/*                                                                    */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*  Description:  Convert a series of maps to an image                */
/*                                                                    */
/**********************************************************************/
X
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
X
#include "font.h"
X
#define	STRING_HEIGHT	(FONTY)
X
#define	PUBLIC
#define	PRIVATE		static
X
#define	TRUE		(1)
#define	FALSE		(0)
X
#define	OK		(0)
#define	NOTOK		(-1)
X
#define	when		break;case
#define	orwhen		case
#define	otherwise	break;default
X
X
X
/* Private routines
X * ================
X */
PRIVATE void one2eight(char data, char *image);
PRIVATE int build_char(char ch, char *image, int x, int y, int pos);
PRIVATE void add_title(char *image, int x, int y, char *filename);
PRIVATE char *convert_map(int width, int height, FILE *fl);
PRIVATE void build_image(char *image, int x, int y, FILE *fl);
X
X
/* Private variables
X * =================
X */
X
X
/* Public routines
X * ===============
X */
X
X
/* Public variables
X * ================
X */
int	mapsize = 64;
int	xcount = 0, ycount = 0;
int	count;
int	xsize, ysize;
int	label = 1;		/* Produce a map label by default */
int	invert = 0;
int	verbose = 0;
int	bitmap = 0;
X
X
/**************************************************************************
X * PUBLIC int main(int ac, char **av)
X *
X */
PUBLIC int main(int ac, char **av)
{
X	int		c;
X	extern char	*optarg;
X	extern int	optind;
X	char		*image;
X	int		x, y;
X	int		i;
X	int		value;
X	char		*p;
X
X	/*
X	 * Deal with arguments
X	 */
X	while ((c = getopt(ac, av, "s:x:ivbl")) != EOF)
X	{
X		switch (c)
X		{
X			when 's':
X				mapsize = atoi(optarg);
X			when 'x':
X				xcount = atoi(optarg);
X			when 'i':
X				invert = 1;
X			when 'v':
X				verbose = 1;
X			when 'b':
X				bitmap = 1;
X			when 'l':
X				label = 0;
X			when '?':
X				fprintf(stderr, "usage: %s [-s size][-x nmaps][-i][-v][-l] xpmap ...\n", av[0]);
X				exit(0);
X		}
X	}
X
X	/*
X	 *   compute the actual size of the resultant image. We do this by
X	 *   taking the square root of the total number of maps and using
X	 *   the truncated value as the y count. The x count is then
X	 *   caluculated from this. Knowing the number of maps on a side,
X	 *   we can then compute the total image size.
X	 */
X	count = ac - optind;
X
X	if (count < 1)
X	{
X		fprintf(stderr, "%s: you need to specify at least one map\n", av[0]);
X		exit(0);
X	}
X
X	if (xcount)
X	{
X		/*
X		 * The user specified an X dimension
X		 */
X		ycount = (count + xcount - 1) / xcount;
X	}
X	else
X	{
X		ycount = sqrt((double) count);
X		xcount = (count + ycount - 1) / ycount;
X	}
X
X	if (verbose)
X		fprintf(stderr, "%d images.  Using %d by %d\n", count, xcount, ycount);
X
X	xsize = (mapsize + 2) * xcount;
X	if (label)
X		ysize = (mapsize + 2 + STRING_HEIGHT) * ycount;
X	else
X	    	ysize = (mapsize + 2) * ycount;
X
X	if (verbose)
X		fprintf(stderr, "Image size: %d x %d\n", xsize, ysize);
X
X	/*
X	 *   Reserve enough space for the image.
X	 */
X	image = malloc(xsize * ysize);
X	memset(image, invert, xsize * ysize);
X
X	/*
X	 *   Build the image
X	 */
X	x = 0; y = 0;
X	for (i = optind; i < ac; i++)
X	{
X		char	*p = strrchr(av[i], '.');
X		char	command[100];
X		FILE	*fl;
X
X		command[0] = '\0';
X		if (p && strcmp(p, ".gz") == 0)
X			sprintf(command, "gunzip<%s", av[i]);
X		else if (p && strcmp(p, ".Z") == 0)
X			sprintf(command, "uncompress<%s", av[i]);
X		if (*command)
X			fl = popen(command, "r");
X		else
X			fl = fopen(av[i], "r");
X
X		if (!fl)
X			perror("open");
X
X		build_image(image, x, y, fl);
X		if (label)
X			add_title(image, x, y, av[i]);
X
X		if (*command)
X			pclose(fl);
X		else
X			fclose(fl);
X
X		x++;
X		if (x >= xcount)
X		{
X			x = 0;
X			y++;
X		}
X	}
X
X	/*
X	 *   Output the image.  The output image can be either pbm or bmp
X	 */
X	if (bitmap)
X	{
X		int	count = 0;
X		int	Xsize = (xsize + 7) & 0xfff8;
X
X		printf("#define maps_width %d\n#define maps_height %d\n", Xsize, ysize);
X		printf("static unsigned char maps_bits[] = {\n   ");
X		p = image;
X		value = 0;
X		i = 0x80;
X		for (y = 0; y < ysize; y++)
X		{
X			for (x = 0; x < Xsize; x++)
X			{
X				if (*p)
X					value |= 0x80;
X				i >>= 1;
X				if (i == 0)
X				{
X					printf("0x%02x, ", value & 0xff);
X					count++;
X					if (count >= 12)
X					{
X						count = 0;
X						printf("\n   ");
X					}
X					i = 0x80;
X					value = 0;
X				}
X				value >>= 1;
X				p++;
X			}
X			if (i != 0x80)
X			{
X				printf("0x%02x, ", value & 0xff);
X				count++;
X				if (count >= 12)
X				{
X					count = 0;
X					printf("\n   ");
X				}
X				i = 0x80;
X				value = 0;
X			}
#if 0
X			value = 0;
#endif
X			p -= Xsize - xsize;
X		}
X		printf("\n};\n");
X	}
X	else
X	{
X		printf("P4\n%d %d\n", xsize, ysize);
X		p = image;
X		for (y = 0; y < ysize; y++)
X		{
X			value = 0;
X			i = 0x80;
X			for (x = 0; x < xsize; x++)
X			{
X				if (*p)
X					value |= i;
X				i >>= 1;
X				if (i == 0)
X				{
X					putchar(value);
X					i = 0x80;
X					value = 0;
X				}
X				p++;
X			}
X			if (i != 0x80)
X				putchar(value);
X			value = 0;
X		}
X	}
X	if (verbose)
X		fprintf(stderr, "\ndone\n");
X	return 0;
}
X
X
/**************************************************************************
X * PRIVATE void build_image(char *image, int x, int y, FILE *fl)
X *   Read a map and create the image for it. The image is placed
X *   at location (x, y)
X */
PRIVATE void build_image(char *image, int x, int y, FILE *fl)
{
X	int	width, height;
X	char	buffer[1024];
X	char	*token;
X
X	if (!fl)
X		return;
X
X	if (verbose)
X		fprintf(stderr, "\rbuilding %d x %d   ", x, y);
X
X	while (fgets(buffer, sizeof(buffer), fl))
X	{
X		if (strncasecmp(buffer, "mapwidth", 8) == 0)
X		{
X			token = strtok(buffer, " :\r\t\n");
X			width = atoi(strtok(NULL, " :\t\r\n"));
X		}
X		else if (strncasecmp(buffer, "mapheight", 9) == 0)
X		{
X			token = strtok(buffer, " :\r\t\n");
X			height = atoi(strtok(NULL, " :\t\r\n"));
X		}
X		else if (strncasecmp(buffer, "mapdata", 7) == 0)
X		{
X			char	*p, *data, *topleft;
X			int	j;
X
X			p = data = convert_map(width, height, fl);
X
X			/*
X			 *   Compute the top left location of this image in the big output
X			 *   image
X			 */
X			if (label)
X				topleft = image + xsize * (mapsize + 2 + STRING_HEIGHT) * y + 1 + xsize + (mapsize + 2) * x;
X			else
X				topleft = image + xsize * (mapsize + 2) * y + 1 + xsize + (mapsize + 2) * x;
X			for (j = 0; j < mapsize; j++)
X			{
X				memcpy(topleft, p, mapsize);
X				topleft += xsize;
X				p += mapsize;
X			}
X			fclose(fl);
X			free(data);
X		}
X	}
X	fclose(fl);
}
X
X
/**************************************************************************
X * PRIVATE void add_title(char *image, int x, int y, char *filename)
X *   Create the title for the map below the map image. The title
X *   is just the file name without the .map at the end.
X */
PRIVATE void add_title(char *image, int x, int y, char *filename)
{
X	char	*p = strrchr(filename, '/');
X	int	pos;
X	char	*s;
X	char	*botleft;
X	int	j;
X
X	if (!p)
X		p = filename;
X	else
X		p++;
X	s = strrchr(p, '.');
X	if (s)
X		*s = '\0';
X
X	/*
X	 *   Make the string image
X	 */
X	s = malloc(mapsize * STRING_HEIGHT);
X	memset(s, 0, mapsize * STRING_HEIGHT);
X	memset(s, invert, mapsize);
X	pos = 0;
X	while (*p)
X	{
X		pos = build_char(*p, s, mapsize, STRING_HEIGHT, pos);
X		p++;
X	}
X
X	/*
X	 *   Find the bottom of the image so we can put the string there.
X	 */
X	botleft = image + xsize * (mapsize + 2 + STRING_HEIGHT) * y + 1 + xsize + (mapsize + 2) * x;
X	botleft += xsize * mapsize;
X	p = s;
X	for (j = 0; j < STRING_HEIGHT; j++)
X	{
X		memcpy(botleft, p, mapsize);
X		botleft += xsize;
X		p += mapsize;
X	}
X	free(s);
}
X
X
/**************************************************************************
X * PRIVATE int build_char(char ch, char *image, int x, int y, int pos)
X *
X */
PRIVATE int build_char(char ch, char *image, int x, int y, int pos)
{
X	int	i, j, n;
X	char	*p;
X	char	*data;
X
X	/*
X	 *   Locate the character
X	 */
X	for (i = 0; font[i].ch; i++)
X	{
X		if (font[i].ch == ch)
X			break;
X	}
X	if (font[i].ch == '\0')
X		return pos;
X
X	if (pos + font[i].width > x)
X		return pos;		/* Don't overwrite the bounding box */
X
X	p = image + pos;
X	data = (char *) font[i].data;
X	for (n = 0; n <= FONTY; n++)
X	{
X		for (j = 0; j < FONTX; j++)
X		{
X			one2eight(*data++, p + j * 8);
X		}
X		p += x;
X	}
X	return pos + font[i].width + 1;
}
X
X
/**************************************************************************
X * PRIVATE void one2eight(char data, char *image)
X *   Convert one byte (8 bits) to a sequence of 8 bytes.
X */
PRIVATE void one2eight(char data, char *image)
{
X	int	mask = 0x80;
X	int	i;
X
X	for (i = 0; i < 8; i++)
X	{
X		if (data & mask)
X			*image = 1;
X		image++;
X		mask >>= 1;
X	}
}
X
X
/**************************************************************************
X * PRIVATE char *convert_map(int width, int height, FILE *fl)
X *   Create an image from map data.
X */
PRIVATE char *convert_map(int width, int height, FILE *fl)
{
X	char	buffer[10240];
X	char	*p;
X	float	div;
X	int	x, y;
X	int	lx, ly, px, py;
X	int	value = 1;
X	int	i, j;
X	char	*output = malloc(mapsize * mapsize);
X
X	if (width >= height)
X		div = ((float) mapsize) / width;
X	else
X		div = ((float) mapsize) / height;
X
X	value ^= invert;
X
X	y = 0;
X	memset((char *) output, invert, mapsize * mapsize);
X
X	/*
X	 *   If the map is smaller than the image we are producing, we
X	 *   need to scale the map, otherwise we need to scale the image.
X	 */
X	if (div > 1)
X	{
X		ly = 0;
X		px = 0;
X		py = 0;
X		while (fgets(buffer, sizeof(buffer), fl) && y < height)
X		{
X			x = 0;
X			px = 0;
X			p = buffer;
X			while (*p && x < width)
X			{
X				lx = (x + 1) * div;
X				if (strchr("xswqa#", *p))
X				{
X					for (i = px; i < lx; i++)
X						output[ly * mapsize + i] = value;
X				}
X				else
X				{
X					for (i = px; i <= lx; i++)
X						output[ly * mapsize + i] = !value;
X				}
X				px = lx;
X				p++;
X				x++;
X			}
X			y++;
X			px = 0;
X			ly = y * div;
X			/*
X			 *   Copy the previous line as many times as needed...
X			 */
X			for (j = py; j < ly; j++)
X			{
X				memcpy(&output[j * mapsize], &output[py * mapsize], mapsize);
X			}
X			py = ly;
X		}
X	}
X	else
X	{
X		while (fgets(buffer, sizeof(buffer), fl) && y < height)
X		{
X			x = 0;
X			p = buffer;
X			while (*p && x < width)
X			{
X				if (strchr("xswqa#", *p))
X				{
X					output[(int)(y * div + 0.5) * mapsize + (int)(x * div + 0.5)] = value;
X				}
X				p++;
X				x++;
X			}
X			y++;
X		}
X	}
X	return output;
}
X
X
X
SHAR_EOF
  $shar_touch -am 1016175999 'maps2image.c' &&
  chmod 0664 'maps2image.c' ||
  $echo 'restore of' 'maps2image.c' 'failed'
  if ( md5sum --help </dev/null 2>&1 | grep 'sage: md5sum \[' ) >/dev/null 2>&1 \
  && ( md5sum --version </dev/null 2>&1 | grep -v 'textutils 1.12' ) >/dev/null; then
    md5sum -c << SHAR_EOF >/dev/null 2>&1 \
    || $echo 'maps2image.c:' 'MD5 check failed'
67d4b9394ded98a2586f43bcca6b2e47  maps2image.c
SHAR_EOF
  else
    shar_count="`LC_ALL=C wc -c < 'maps2image.c'`"
    test 10965 -eq "$shar_count" ||
    $echo 'maps2image.c:' 'original size' '10965,' 'current size' "$shar_count!"
  fi
fi
# ============= bdf2c.c ==============
if test -f 'bdf2c.c' && test "$first_param" != -c; then
  $echo 'x -' SKIPPING 'bdf2c.c' '(file already exists)'
else
  $echo 'x -' extracting 'bdf2c.c' '(text)'
  sed 's/^X//' << 'SHAR_EOF' > 'bdf2c.c' &&
/**********************************************************************/
/*                                                                    */
/*  File:          bdf2c.c                                            */
/*  Author:        Andrew W. Scherpbier                               */
/*  Version:       1.00                                               */
/*  Created:       27 Feb 1994                     		      */
/*                                                                    */
/*  Copyright (c) 1991, 1992 Andrew Scherpbier                        */
/*                All Rights Reserved.                                */
/*                                                                    */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*  Description:  convert a BDF font to a C include file              */
/*                                                                    */
/**********************************************************************/
X
#include <stdio.h>
#include <string.h>
X
#define	PUBLIC
#define	PRIVATE		static
X
#define	TRUE		(1)
#define	FALSE		(0)
X
#define	OK		(0)
#define	NOTOK		(-1)
X
#define	when		break;case
#define	orwhen		case
#define	otherwise	break;default
X
X
/* Private routines
X * ================
X */
PRIVATE void bounds(void);
PRIVATE void start_char(char *name, FILE *fl);
X
X
/* Private variables
X * =================
X */
X
X
/* Public routines
X * ===============
X */
X
X
/* Public variables
X * ================
X */
int	fontheight, fontbase;
int	bpl;
X
struct
{
X	char	*name;
X	char	*chr;
} charmap[] =
{
X	"space",	"' '",
X	"exclam",	"'!'",
X	"quotedbl",	"'\"'",
X	"numbersign",	"'#'",
X	"dollar",	"'$'",
X	"percent",	"'%'",
X	"ampersand",	"'&'",
X	"quoteright",	"'\\''",
X	"parenleft",	"'('",
X	"parenright",	"')'",
X	"asterisk",	"'*'",
X	"plus",		"'+'",
X	"comma",	"','",
X	"minus",	"'-'",
X	"period",	"'.'",
X	"slash",	"'/'",
X	"zero",		"'0'",
X	"one",		"'1'",
X	"two",		"'2'",
X	"three",	"'3'",
X	"four",		"'4'",
X	"five",		"'5'",
X	"six",		"'6'",
X	"seven",	"'7'",
X	"eight",	"'8'",
X	"nine",		"'9'",
X	"colon",	"':'",
X	"semicolon",	"';'",
X	"less",		"'<'",
X	"equal",	"'='",
X	"greater",	"'>'",
X	"question",	"'?'",
X	"at",		"'@'",
X	"bracketleft",	"'['",
X	"backslash",	"'\\\\'",
X	"bracketright",	"']'",
X	"asciicircum",	"'^'",
X	"underscore",	"'_'",
X	"quoteleft",	"'`'",
X	"braceleft",	"'{'",
X	"bar",		"'|'",
X	"braceright",	"'}'",
X	"asciitilde",	"'~'",
X	"A",		"'A'",
X	"B",		"'B'",
X	"C",		"'C'",
X	"D",		"'D'",
X	"E",		"'E'",
X	"F",		"'F'",
X	"G",		"'G'",
X	"H",		"'H'",
X	"I",		"'I'",
X	"J",		"'J'",
X	"K",		"'K'",
X	"L",		"'L'",
X	"M",		"'M'",
X	"N",		"'N'",
X	"O",		"'O'",
X	"P",		"'P'",
X	"Q",		"'Q'",
X	"R",		"'R'",
X	"S",		"'S'",
X	"T",		"'T'",
X	"U",		"'U'",
X	"V",		"'V'",
X	"W",		"'W'",
X	"X",		"'X'",
X	"Y",		"'Y'",
X	"Z",		"'Z'",
X	"a",		"'a'",
X	"b",		"'b'",
X	"c",		"'c'",
X	"d",		"'d'",
X	"e",		"'e'",
X	"f",		"'f'",
X	"g",		"'g'",
X	"h",		"'h'",
X	"i",		"'i'",
X	"j",		"'j'",
X	"k",		"'k'",
X	"l",		"'l'",
X	"m",		"'m'",
X	"n",		"'n'",
X	"o",		"'o'",
X	"p",		"'p'",
X	"q",		"'q'",
X	"r",		"'r'",
X	"s",		"'s'",
X	"t",		"'t'",
X	"u",		"'u'",
X	"v",		"'v'",
X	"w",		"'w'",
X	"x",		"'x'",
X	"y",		"'y'",
X	"z",		"'z'",
X	NULL,		NULL,
};
X
/**************************************************************************
X * PUBLIC int main(int ac, char **av)
X *
X */
PUBLIC int main(int ac, char **av)
{
X	FILE	*fl = fopen(av[1], "r");
X	char	buffer[1024];
X	char	*token;
X
X	if (!fl)
X		exit(1);
X
X	while (fgets(buffer, 1024, fl))
X	{
X		token = strtok(buffer, " \r\n");
X		if (strcmp(token, "STARTCHAR") == 0)
X			start_char(strtok(NULL, " \r\n"), fl);
X		else if (strcmp(token, "FONTBOUNDINGBOX") == 0)
X			bounds();
X	}
X	printf("{'\\0'}};\n");
X	return 0;
}
X
X
/**************************************************************************
X * PRIVATE void start_char(char *name, FILE *fl)
X */
PRIVATE void start_char(char *name, FILE *fl)
{
X	int	i;
X	char	buffer[1024];
X	int	read_data = 0;
X	int	width, height, dummy, start;
X	int	line = 0;
X
X	/*
X	 * Lookup the character
X	 */
X	for (i = 0; charmap[i].name; i++)
X	{
X		if (strcmp(name, charmap[i].name) == 0)
X			break;
X	}
X	if (!charmap[i].name)
X		return;		/* Not found */
X
X	/*
X	 * Output the C stuff
X	 */
X	printf("{%s,", charmap[i].chr);
X
X	/*
X	 * Search for the BITMAP line
X	 */
X	while (fgets(buffer, 1024, fl))
X	{
X		if (strncmp(buffer, "BITMAP", 6) == 0)
X			read_data = 1;
X		else if (strncmp(buffer, "BBX", 3) == 0)
X		{
X			int	i;
X			sscanf(buffer + 4, "%d %d %d %d", &width, &height, &dummy, &start);
X			line = fontbase - start - height;
X			printf("{");
X			for (i = 0; i < line * bpl; i++)
X				printf("0,");
X		}
X		else if (read_data)
X		{
X			char	*p = buffer;
X			int	count = 0;
X			if (strncmp(buffer, "ENDCHAR", 7) == 0)
X				break;
X
X			while (isxdigit(*p))
X			{
X				int	value = 0;
X				if (isdigit(*p))
X					value |= *p - '0';
X				else
X					value |= *p - 'A' + 10;
X				p++;
X				value <<= 4;
X				if (isdigit(*p))
X					value |= *p - '0';
X				else
X					value |= *p - 'A' + 10;
X				printf("%d,", value);
X				p++;
X				count++;
X			}
X			while (count < bpl)
X				printf("0,", count++);
X			line++;
X		}
X	}
X	printf("}, %d},\n", width);
}
X
X
/**************************************************************************
X * PRIVATE void bounds(void)
X *
X */
PRIVATE void bounds(void)
{
X	char	*token = strtok(NULL, "\r\n");
X	int	x, y;
X	int	dummy, base;
X	int	n;
X
X	sscanf(token, "%d %d %d %d", &x, &y, &dummy, &base);
X	n = y * ((x + 7) / 8);
X	printf("#define FONTX %d\n#define FONTY %d\n", (x + 7) / 8, y);
X	printf("struct{char ch; unsigned char data[%d]; int width;}font[]={\n", n);
X	fontheight = y;
X	fontbase = y + base;
X	bpl = (x + 7) / 8;
}
X
X
X
SHAR_EOF
  $shar_touch -am 1016175999 'bdf2c.c' &&
  chmod 0664 'bdf2c.c' ||
  $echo 'restore of' 'bdf2c.c' 'failed'
  if ( md5sum --help </dev/null 2>&1 | grep 'sage: md5sum \[' ) >/dev/null 2>&1 \
  && ( md5sum --version </dev/null 2>&1 | grep -v 'textutils 1.12' ) >/dev/null; then
    md5sum -c << SHAR_EOF >/dev/null 2>&1 \
    || $echo 'bdf2c.c:' 'MD5 check failed'
be4a3836c11344c97da2d788c2c37ecf  bdf2c.c
SHAR_EOF
  else
    shar_count="`LC_ALL=C wc -c < 'bdf2c.c'`"
    test 5637 -eq "$shar_count" ||
    $echo 'bdf2c.c:' 'original size' '5637,' 'current size' "$shar_count!"
  fi
fi
# ============= helvR08.bdf.gz ==============
if test -f 'helvR08.bdf.gz' && test "$first_param" != -c; then
  $echo 'x -' SKIPPING 'helvR08.bdf.gz' '(file already exists)'
else
  $echo 'x -' extracting 'helvR08.bdf.gz' '(binary)'
  sed 's/^X//' << 'SHAR_EOF' | uudecode &&
begin 600 helvR08.bdf.gz
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
>@JKLS>S.U-Q[$G_HU1"0_]*G#&O_!Y`A]OD]8@``
`
end
SHAR_EOF
  $shar_touch -am 1016175999 'helvR08.bdf.gz' &&
  chmod 0444 'helvR08.bdf.gz' ||
  $echo 'restore of' 'helvR08.bdf.gz' 'failed'
  if ( md5sum --help </dev/null 2>&1 | grep 'sage: md5sum \[' ) >/dev/null 2>&1 \
  && ( md5sum --version </dev/null 2>&1 | grep -v 'textutils 1.12' ) >/dev/null; then
    md5sum -c << SHAR_EOF >/dev/null 2>&1 \
    || $echo 'helvR08.bdf.gz:' 'MD5 check failed'
d8c268940f03cf6ad7250af75fab556b  helvR08.bdf.gz
SHAR_EOF
  else
    shar_count="`LC_ALL=C wc -c < 'helvR08.bdf.gz'`"
    test 5025 -eq "$shar_count" ||
    $echo 'helvR08.bdf.gz:' 'original size' '5025,' 'current size' "$shar_count!"
  fi
fi
$echo $shar_n 'x -' 'lock directory' "\`_sh02319': " $shar_c
if rm -fr _sh02319; then
  $echo 'removed'
else
  $echo 'failed to remove'
fi
exit 0

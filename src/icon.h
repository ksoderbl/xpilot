/* $Id: icon.h,v 3.2 1993/06/28 20:53:40 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bj�rn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert G�sbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

/* Specifies the bitmap used for the icon. */
#define icon_width 64
#define icon_height 64
static unsigned char icon_bits[] = {
   0x00, 0x40, 0x00, 0x80, 0x00, 0x00, 0x04, 0x00, 0x40, 0x40, 0x00, 0x80,
   0x00, 0x00, 0x04, 0x00, 0xbf, 0x7f, 0x00, 0x80, 0x00, 0x00, 0x04, 0x00,
   0xbf, 0x7f, 0x00, 0x80, 0x00, 0x00, 0x04, 0x00, 0xbf, 0x7f, 0x00, 0x80,
   0x00, 0x00, 0x04, 0x00, 0x7f, 0x7f, 0x00, 0x80, 0x00, 0x00, 0x04, 0x00,
   0x7f, 0x7f, 0x00, 0x80, 0x00, 0x00, 0x04, 0x00, 0x7f, 0x7f, 0x00, 0x80,
   0x00, 0x00, 0x04, 0x00, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00,
   0x06, 0x80, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x18, 0x60, 0x00, 0x00,
   0x00, 0x01, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
   0x80, 0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
   0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
   0x80, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x64, 0x00, 0x00,
   0x00, 0x00, 0x18, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x10, 0x00, 0x00,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x10, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x20, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x40, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x01,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x60, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x22, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x1d, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xa0, 0x3a, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x55, 0x02, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xa8, 0xfa, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x61, 0x15, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8c, 0x08, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xc0, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x02, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x40, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0xc0, 0x00, 0x00,
   0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

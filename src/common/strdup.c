/*
 * strdup.c
 *
 * Simple version of strdup for machines without it (ie DEC Ultrix 4.2)
 *
 * By David Chatterton
 * 29 July 1993
 *
 * You can do anything you like to this... :)
 *
 * $Id: strdup.c,v 4.1 1999/10/04 19:57:47 bert Exp $
 */

#include <stdlib.h>
#include <string.h>

/* BG 99-10-4: some version of glibs defines strdup.  kill that nonsense. */
#ifdef strdup
#undef strdup
#endif

char* strdup (const char* s1)
{
	char* s2;

	s2 = (char*)malloc(strlen(s1)+1);
	if (s2)
		strcpy(s2,s1);

	return s2;
}


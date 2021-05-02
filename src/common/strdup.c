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
 * $Id: strdup.c,v 4.0 1998/03/18 15:18:06 bert Exp $
 */

#include <stdlib.h>
#include <string.h>

char* strdup (const char* s1)
{
	char* s2;

	s2 = (char*)malloc(strlen(s1)+1);
	if (s2)
		strcpy(s2,s1);

	return s2;
}


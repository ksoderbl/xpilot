/* $Id: option.c,v 3.42 1997/11/27 20:09:23 bert Exp $
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

#ifdef	_WINDOWS
#include <windows.h>
#else
#include "types.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "defaults.h"
#include "error.h"

#ifdef	_WINDOWS
#include <io.h>
#define	read(__a, __b, __c)	_read(__a, __b, __c)
#endif


char option_version[] = VERSION;

#ifndef PATH_MAX
#define PATH_MAX	1023
#endif

#define	NHASH	199

valPair    *hashArray[NHASH];


/*
 * Compute a reasonable case-insensitive hash value across a character string.
 */
static unsigned int hash(const char *name)
{
    unsigned int hashVal = 0;
    unsigned char *s = (unsigned char *)name;

    while (*s) {
	char        c = *s++;

	if (isascii(c) && isalpha(c) && islower(c))
	    c = toupper(c);
	hashVal = (hashVal + c) << 1;
	while (hashVal > NHASH)
	    hashVal = (hashVal % NHASH) + (hashVal / NHASH);
    }
    return hashVal % NHASH;
}


/*
 * Allocate a new bucket for the hash table and fill in its values.
 */
static valPair *newOption(const char *name, const char *value)
{
    valPair    *tmp = (valPair *)malloc(sizeof(valPair));

    if (!tmp)
	return (valPair *) 0;

    tmp->name = (char *)malloc(strlen(name) + 1);
    tmp->value = (char *)malloc(strlen(value) + 1);

    if (!tmp->name || !tmp->value) {
	if (tmp->name)
	    free(tmp->name);
	if (tmp->value)
	    free(tmp->value);
	free(tmp);
	return (valPair *) 0;
    }
    strcpy(tmp->name, name);
    strcpy(tmp->value, value);
    return tmp;
}


/*
 * Scan through the hash table of option name-value pairs looking for an option
 * with the specified name; if found, and if override is true, change to the
 * new value; if found and override is not true, do nothing. If not found, add
 * to the hash table regardless of override.   Either way, if def is nonzero,
 * it is attached to the name-value pair - this will only happen once anyway.
 */
void addOption(const char *name, const char *value, int override, void *def)
{
    valPair    *tmp;
    int         ix = hash(name);

    for (tmp = hashArray[ix]; tmp; tmp = tmp->next)
	if (!strcasecmp(name, tmp->name)) {
	    if (override && value) {
		char       *s = (char *)malloc(strlen(value) + 1);

		if (!s)
		    return;
		free(tmp->value);
		strcpy(s, value);
		tmp->value = s;
	    }
	    if (def)
		tmp->def = def;
	    return;
	}
    if (!value)
	return;

    tmp = newOption(name, value);
    if (!tmp)
	return;
    tmp->def = def;
    tmp->next = hashArray[ix];
    hashArray[ix] = tmp;
}


/*
 * Return the value of the specified option, or (char *)0 if there is no value
 * for that option.
 */
char *getOption(const char *name)
{
    valPair    *tmp;
    int         ix = hash(name);

    for (tmp = hashArray[ix]; tmp; tmp = tmp->next)
	if (!strcasecmp(name, tmp->name))
	    return tmp->value;

    return (char *)0;
}


static char *FileName;
static int  LineNumber;

/*
 * Skips to the end of the line.
 */
static void toeol(char **map_ptr)
{
    int		ich;

    while (**map_ptr) {
	ich = **map_ptr;
	(*map_ptr)++;
        if (ich == '\n') {
            ++LineNumber;
            return;
        }
    }
}


/*
 * Skips to the first non-whitespace character, returning that character.
 */
static int skipspace(char **map_ptr)
{
    int		ich;

    while (**map_ptr) {
	ich = **map_ptr;
	(*map_ptr)++;
        if (ich == '\n') {
            ++LineNumber;
            return ich;
        }
        if (!isascii(ich) || !isspace(ich))
            return ich;
    }
    return '\0';
}


/*
 * Read in a multiline value.
 */
static char *getMultilineValue(char **map_ptr, char *delimiter)
{
    char       *s = (char *)malloc(32768);
    int         i = 0;
    int         slen = 32768;
    char       *bol;
    int         ich;

    bol = s;
    for (;;) {
	ich = **map_ptr;
	(*map_ptr)++;
	if (ich == '\0') {
	    s = (char *)realloc(s, i + 1);
	    s[i] = '\0';
	    return s;
	}
	if (i == slen) {
	    char       *t = s;

	    slen += (slen / 2) + 8192;
	    s = (char *)realloc(s, slen);
	    bol += s - t;
	}
	if (ich == '\n') {
	    s[i] = 0;
	    if (delimiter && !strcmp(bol, delimiter)) {
		char       *t = s;

		s = (char *)realloc(s, bol - s + 1);
		s[bol - t] = '\0';
		return s;
	    }
	    bol = &s[i + 1];
	    ++LineNumber;
	}
	s[i++] = ich;
    }
}


/*
 * Parse a standard line from a defaults file, in the form Name: value Name
 * must start at the beginning of the line with an alphabetic character.
 * Whitespace within name is ignored. Value may contain any character other
 * than # or newline, but leading and trailing whitespace are discarded.
 * Characters after a # are ignored - this can be used for comments. If value
 * begins with \override:, the override flag is set when addOption is called,
 * so that this value will override an existing value.   The \override:
 * sequence is not retained in the stored value. If value starts with
 * \multiline:, then the rest of the line is used as a delimiter, and
 * subsequent lines are read and saved as the value until the delimiter is
 * encountered.   No interpretation is done on the text in the multiline
 * sequence, so # does not serve as a comment character there, and newlines and
 * whitespace are not discarded.
 */
#define EXPAND					\
    if (i == slen) {				\
	s = (char *)realloc(s, slen *= 2);	\
    }
static void parseLine(char **map_ptr)
{
    int		ich;
    char       *value,
	       *head,
	       *name,
	       *s = (char *)malloc(128);
    char       *p;
    int         slen = 128;
    int         i = 0;
    int         override = 0;
    int         multiline = 0;

    ich = **map_ptr;
    (*map_ptr)++;

    /* Skip blank lines... */
    if (ich == '\n') {
	++LineNumber;
	free(s);
	return;
    }
    /* Skip leading space... */
    if (isascii(ich) && isspace(ich)) {
	ich = skipspace(map_ptr);
	if (ich == '\n') {
	    free(s);
	    return;
	}
    }
    /* Skip lines that start with comment character... */
    if (ich == '#') {
	toeol(map_ptr);
	free(s);
	return;
    }
    /* Skip lines that start with the end of the file... :') */
    if (ich == '\0') {
	free(s);
	return;
    }
    /* *** I18nize? *** */
    if (!isascii(ich) || !isalpha(ich)) {
	error("%s line %d: Names must start with an alphabetic.\n",
	      FileName, LineNumber);
	toeol(map_ptr);
	free(s);
	return;
    }
    s[i++] = ich;
    do {
	ich = **map_ptr;
	(*map_ptr)++;

	if (ich == '\n' || ich == '#' || ich == '\0') {
	    error("%s line %d: No colon found on line.\n",
		  FileName, LineNumber);
	    if (ich == '#')
		toeol(map_ptr);
	    else
		++LineNumber;
	    free(s);
	    return;
	}
	if (isascii(ich) && isspace(ich))
	    continue;
	if (ich == ':')
	    break;
	EXPAND;
	s[i++] = ich;
    } while (1);

    ich = skipspace(map_ptr);

    EXPAND;
    s[i++] = '\0';
    name = s;

    s = (char *)malloc(slen = 128);
    i = 0;
    do {
	EXPAND;
	s[i++] = ich;
	ich = **map_ptr;
	(*map_ptr)++;
    } while (ich != '\0' && ich != '#' && ich != '\n');

    if (ich == '\n')
	++LineNumber;

    if (ich == '#')
	toeol(map_ptr);

    EXPAND;
    s[i++] = 0;
    head = value = s;
    s = value + strlen(value) - 1;
    while (s >= value && isascii(*s) && isspace(*s))
	--s;
    *++s = 0;

    /* Deal with `define: MACRO \multiline: TAG'. */
    if (strcmp(name, "define") == 0) {
	p = value;
	while (*p && isascii(*p) && !isspace(*p)) {
	    p++;
	}
	*p++ = '\0';

	/* name becomes value */
	free(name);
	name = (char *)malloc(p - value + 1);
	strcpy (name, value);

	/* Move value to \multiline */
	while (*p && isspace(*p)) {
	    p++;
	}
	value = p;
    }

    if (!strncmp(value, "\\override:", 10)) {
	override = 1;
	value += 10;
    }
    while (*value && isascii(*value) && isspace(*value))
	++value;
    if (!strncmp(value, "\\multiline:", 11)) {
	multiline = 1;
	value += 11;
    }
    while (*value && isascii(*value) && isspace(*value))
	++value;
    if (!*value) {
	error("%s line %d: no value specified.\n",
	      FileName, LineNumber);
	free(name);
	free(head);
	return;
    }
    if (multiline) {
	value = getMultilineValue(map_ptr, value);
	/*
	 * This dynamic memory returned by getMultilineValue()
	 * is not freed anywhere.  Thanks to a Purify report
	 * by Daniel Edward Lovinger <del+@cmu.edu>.
	 * This problem is not so easy to fix.  Later.
	 */
    }

    /* Deal with `expand: MACRO'. */
    if (strcmp(name, "expand") == 0) {
	p = getOption(value);
	if (!p) {
	    error("Can't expand `%s' since it's not already defined.\n", value);
	}
	else {
	    while (*p) {
		parseLine(&p);
	    }
	}
    }
#ifdef REGIONS /* not yet */
    /* Deal with `region: \multiline: TAG'. */
    else if (strcmp(name, "region") == 0) {
	if (!multiline) { /* Must be multiline. */
	    error("regions must use `\\multiline:'.\n");
	    free(name);
	    free(head);
	    return;
	}
	p = value;
	while (*p) {
	    parseLine(&p);
	}
    }
#endif
    else {
	addOption(name, value, override, (optionDesc *) 0);
    }

    /*
     * if (multiline) free (value);
     */
    if (multiline) free (value);
    free(name);
    free(head);
    return;
}
#undef EXPAND


#if 0
/*
 * Parse an old-style map file...
 */
static int parseOldMapFile(FILE *ifile)
{
    char        ibuf[1024];
    char       *s;
    int         i;

    if (fgets(ibuf, sizeof(ibuf), ifile) == (char *)0)
	return 0;
    s = strchr(ibuf, 'x');
    if (!s) {
	error("%s line %d: invalid map dimensions.\n",
	      FileName, LineNumber);
    }
    *s = 0;
    addOption("mapWidth", ibuf, 1, (optionDesc *) 0);
    ++s;
    i = strlen(s);
    if (s[i - 1] == '\n')
	s[i - 1] = 0;
    addOption("mapHeight", s, 1, (optionDesc *) 0);
    ++LineNumber;

    if (fgets(ibuf, sizeof ibuf, ifile) == (char *)0)
	return 0;

    i = strlen(ibuf);
    if (ibuf[i - 1] == '\n')
	ibuf[i - 1] = 0;
    addOption("mapRule", ibuf, 1, (optionDesc *) 0);

    if (fgets(ibuf, sizeof ibuf, ifile) == (char *)0)
	return 0;
    i = strlen(ibuf);
    if (ibuf[i - 1] == '\n')
	ibuf[i - 1] = 0;
    addOption("mapName", ibuf, 1, (optionDesc *) 0);

    if (fgets(ibuf, sizeof ibuf, ifile) == (char *)0)
	return 0;
    i = strlen(ibuf);
    if (ibuf[i - 1] == '\n')
	ibuf[i - 1] = 0;
    addOption("mapAuthor", ibuf, 1, (optionDesc *) 0);

    s = getMultilineValue((char *)0, ifile);
    addOption("mapData", s, 1, (optionDesc *) 0);

    return 1;
}
#endif


#if defined(COMPRESSED_MAPS)
static FILE *openCompressedDefaultsFile(void)
{
    char	buf[PATH_MAX + sizeof(ZCAT_FORMAT) + 1];

    if (access(FileName, 4) == 0) {
	sprintf(buf, ZCAT_FORMAT, FileName);
	return popen(buf, "r");
    }
    return NULL;
}
#endif


static FILE *openDefaultsFile(const char *filename)
{
    int		len = strlen(filename);
    bool	hasmap = false;
    FILE	*ifile;

    if ((FileName = (char *)malloc(PATH_MAX + 1)) == NULL) {
	return NULL;
    }
    strcpy(FileName, filename);
#if defined(COMPRESSED_MAPS)
    if (len > strlen(ZCAT_EXT)
	&& strcmp(FileName + len - strlen(ZCAT_EXT), ZCAT_EXT) == 0) {
	if ((ifile = openCompressedDefaultsFile()) != NULL) {
	    return ifile;
	}
	sprintf(FileName, "%s%s", MAPDIR, filename);
	if ((ifile = openCompressedDefaultsFile()) != NULL) {
	    return ifile;
	}
	return NULL;
    }
#endif
    if (len > 4 && strcmp(FileName + len - 4, ".map") == 0) {
	hasmap = true;
    }
    if ((ifile = fopen(FileName, "r")) != NULL) {
	return ifile;
    }
    if (hasmap == false) {
	strcat(FileName, ".map");
	if ((ifile = fopen(FileName, "r")) != NULL) {
	    return ifile;
	}
    }
#if defined(COMPRESSED_MAPS)
    strcat(FileName, ZCAT_EXT);
    if ((ifile = openCompressedDefaultsFile()) != NULL) {
	return ifile;
    }
#endif
    sprintf(FileName, "%s%s", MAPDIR, filename);
    if ((ifile = fopen(FileName, "r")) != NULL) {
	return ifile;
    }
    if (hasmap == false) {
	strcat(FileName, ".map");
	if ((ifile = fopen(FileName, "r")) != NULL) {
	    return ifile;
	}
    }
#if defined(COMPRESSED_MAPS)
    strcat(FileName, ZCAT_EXT);
    if ((ifile = openCompressedDefaultsFile()) != NULL) {
	return ifile;
    }
#endif
    return NULL;
}


/*
 * Parse a file containing defaults (and possibly a map).
 */
bool parseDefaultsFile(const char *filename)
{
    FILE       *ifile;
    int		fd, map_offset, map_size, n;
    char       *map_buf;

    LineNumber = 1;
    if ((ifile = openDefaultsFile(filename)) == NULL) {
	free(FileName);
	return false;
    }

    fd = fileno(ifile);

    /* Using a 200 map sample, the average map size is 37k.
       This chunk size could be increased to avoid lots of
       reallocs. */
#define MAP_CHUNK_SIZE	8192

    map_offset = 0;
    map_size = 2*MAP_CHUNK_SIZE;
    map_buf = (char *) malloc(map_size + 1);
    if (!map_buf) {
	error("Not enough memory to read the map!");
	free(FileName);
	fclose(ifile);
	return false;
    }

    for (;;) {
	n = read(fd, &map_buf[map_offset], map_size - map_offset);
	if (n < 0) {
	    error("Error reading map!");
	    free(FileName);
	    fclose(ifile);
	    free(map_buf);
	    return false;
	}
	if (n == 0) {
	    break;
	}
	map_offset += n;

	if (map_size - map_offset < MAP_CHUNK_SIZE) {
	    map_size += (map_size / 2) + MAP_CHUNK_SIZE;
	    map_buf = (char *) realloc(map_buf, map_size + 1);
	    if (!map_buf) {
		error("Not enough memory to read the map!");
		free(FileName);
		fclose(ifile);
		return false;
	    }
	}
    }

    map_buf = (char *) realloc(map_buf, map_offset + 1);
    map_buf[map_offset] = '\0'; /* EOF */

    if (isdigit(*map_buf)) {
	errno = 0;
	error("%s is in old (v1.x) format, please convert it with mapmapper",
	      FileName);
	free(FileName);
	free(map_buf);
	fclose(ifile);
	return false;
    } else {
	/* Parse all the lines in the file. */
	char *map_ptr = map_buf;
	while (*map_ptr) {
	    parseLine(&map_ptr);
	}
    }

    free(FileName);
    free(map_buf);
    fclose(ifile);

    return true;
}


/*
 * Go through the hash table looking for name-value pairs that have defaults
 * assigned to them.   Process the defaults and, if possible, set the
 * associated variables.
 */
void parseOptions(void)
{
    int         i;
    valPair    *tmp, *next;
    char       *fpsstr;
    optionDesc *desc;

    /*
     * This must be done in order that FPS will return the eventual
     * frames per second for computing valSec and valPerSec.
     */
    if ((fpsstr = getOption("framesPerSecond")) != NULL)
	framesPerSecond = atoi(fpsstr);
    if (FPS <= 0) {
	errno = 0;
	error("Can't run with %d frames per second, should be positive\n",
	    FPS);
	End_game();
    }

    for (i = 0; i < NHASH; i++)
	for (tmp = hashArray[i]; tmp; tmp = tmp->next) {
	    /* Does it have a default?   (If so, get a pointer to it) */
	    if ((desc = (optionDesc *)tmp->def) != NULL) {
		if (desc->variable) {
		    switch (desc->type) {

		    case valVoid:
			break;

		    case valInt:
			{
			    int        *ptr = (int *)desc->variable;

			    *ptr = atoi(tmp->value);
			    break;
			}

		    case valReal:
			{
			    DFLOAT     *ptr = (DFLOAT *)desc->variable;

			    *ptr = atof(tmp->value);
			    break;
			}

		    case valBool:
			{
			    bool	*ptr = (bool *)desc->variable;

			    if (!strcasecmp(tmp->value, "yes")
				|| !strcasecmp(tmp->value, "on")
				|| !strcasecmp(tmp->value, "true"))
				*ptr = true;
			    else if (!strcasecmp(tmp->value, "no")
				     || !strcasecmp(tmp->value, "off")
				     || !strcasecmp(tmp->value, "false"))
				*ptr = false;
			    else {
				error("Invalid boolean value for %s - %s\n",
				      desc->name, tmp->value);
			    }
			    break;
			}

		    case valIPos:
			{
			    ipos       *ptr = (ipos *)desc->variable;
			    char       *s;

			    s = strchr(tmp->value, ',');
			    if (!s) {
				error("Invalid coordinate pair for %s - %s\n",
				      desc->name, tmp->value);
				break;
			    }
			    ptr->x = atoi(tmp->value);
			    ptr->y = atoi(++s);
			    break;
			}

		    case valString:
			{
			    char      **ptr = (char **)desc->variable;

			    *ptr = tmp->value;
			    break;
			}

		    case valSec:
			{
			    int		*ptr = (int *)desc->variable;

			    *ptr = (int)(atof(tmp->value) * FPS);
			    break;
			}

		    case valPerSec:
			{
			    DFLOAT	*ptr = (DFLOAT *)desc->variable;

			    *ptr = (DFLOAT)(atof(tmp->value) / FPS);
			    break;
			}
		    }
		}
	    }
	}

    for (i = 0; i < NHASH; i++) {
	for (tmp = hashArray[i]; tmp; tmp = next) {
	    free(tmp->name);
	    next = tmp->next;
	    memset((void *)tmp, 0, sizeof(*tmp));
	    free(tmp);
	}
    }
}


#ifdef	_WINDOWS
/* clear the hashArray in case we're restarted */
void	FreeOptions()
{
	int		i;
    /* valPair    *tmp, *next; */

	for (i=0; i<NHASH; i++) {
	}

}
#endif

/* $Id: option.c,v 1.3 1993/04/01 18:17:45 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bj�rn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */
/* $NCDId: @(#)option.c,v 1.1 1992/09/10 03:26:33 mellon Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "global.h"
#include "robot.h"
#include "map.h"
#include "defaults.h"

#define	NHASH	29

valPair    *hashArray[NHASH];


/*
 * Compute a reasonable case-insensitive hash value across a character string.
 */
static unsigned int hash(char *name)
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
static valPair *newOption(char *name, char *value)
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
void addOption(char *name, char *value, int override, void *def)
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
    tmp->next = hashArray[ix];
    tmp->def = def;
    hashArray[ix] = tmp;
}


/*
 * Return the value of the specified option, or (char *)0 if there is no value
 * for that option.
 */
char *getOption(char *name)
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
static void toeol(FILE *ifile)
{
    int         ich;

    while (!feof(ifile))
	if ((ich = getc(ifile)) == '\n') {
	    ++LineNumber;
	    return;
	}
}


/*
 * Skips to the first non-whitespace character, returning that character.
 */
static int skipspace(FILE *ifile)
{
    int         ich;

    while (!feof(ifile)) {
	ich = getc(ifile);
	if (ich == '\n') {
	    ++LineNumber;
	    return ich;
	}
	if (!isascii(ich) || !isspace(ich))
	    return ich;
    }
    return EOF;
}


/*
 * Read in a multiline value.
 */
static char *getMultilineValue(char *delimiter, FILE *ifile)
{
    char       *s = malloc(32768);
    int         i = 0;
    int         slen = 32768;
    char       *bol;
    int         ich;

    bol = s;
    while (1) {
	ich = getc(ifile);
	if (ich == EOF) {
	    s = realloc(s, i + 1);
	    s[i] = '\0';
	    return s;
	}
	if (i == slen) {
	    char       *t = s;

	    s = realloc(s, slen += 32768);
	    bol += s - t;
	}
	if (ich == '\n') {
	    s[i] = 0;
	    if (delimiter && !strcmp(bol, delimiter)) {
		char       *t = s;

		s = realloc(s, bol - s + 1);
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
#define EXPAND				\
    if (i == slen) {			\
        s = realloc(s, slen *= 2);	\
    }
static void parseLine(FILE *ifile)
{
    int         ich;
    char       *value,
               *head,
               *name,
               *s = (char *)malloc(128);
    int         slen = 128;
    int         i = 0;
    int         override = 0;
    int         multiline = 0;

    ich = getc(ifile);

    /* Skip blank lines... */
    if (ich == '\n') {
	++LineNumber;
	free(s);
	return;
    }
    /* Skip leading space... */
    if (isascii(ich) && isspace(ich)) {
	ich = skipspace(ifile);
	if (ich == '\n') {
	    free(s);
	    return;
	}
    }
    /* Skip lines that start with comment character... */
    if (ich == '#') {
	toeol(ifile);
	free(s);
	return;
    }
    /* Skip lines that start with the end of the file... :') */
    if (ich == EOF) {
	free(s);
	return;
    }
    /* *** I18nize? *** */
    if (!isascii(ich) || !isalpha(ich)) {
	error("%s line %d: Names must start with an alphabetic.\n",
	      FileName, LineNumber);
	toeol(ifile);
	free(s);
	return;
    }
    s[i++] = ich;
    do {
	ich = getc(ifile);
	if (ich == '\n' || ich == '#' || ich == EOF) {
	    error("%s line %d: No colon found on line.\n",
		  FileName, LineNumber);
	    if (ich == '#')
		toeol(ifile);
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

    ich = skipspace(ifile);

    EXPAND;
    s[i++] = '\0';
    name = s;

    s = malloc(slen = 128);
    i = 0;
    do {
	EXPAND;
	s[i++] = ich;
	ich = getc(ifile);
    } while (ich != EOF && ich != '#' && ich != '\n');

    if (ich == '\n')
	++LineNumber;

    if (ich == '#')
	toeol(ifile);

    EXPAND;
    s[i++] = 0;
    head = value = s;
    s = value + strlen(value) - 1;
    while (s >= value && isascii(*s) && isspace(*s))
	--s;
    *++s = 0;
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
	error("%s line %s: no value specified.\n",
	      FileName, LineNumber);
	free(name);
	free(head);
	return;
    }
    if (multiline)
	value = getMultilineValue(value, ifile);
    addOption(name, value, override, (optionDesc *) 0);

    /*
     * if (multiline) free (value);
     */
    free(name);
    free(head);
    return;
}
#undef EXPAND


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



/*
 * Parse a file containing defaults (and possibly a map).
 */
bool parseDefaultsFile(char *filename)
{
    FILE       *ifile;
    int         ich;
    char       *ifname;

    FileName = filename;
    LineNumber = 1;

    ifile = fopen(filename, "r");

    if (ifile == NULL) {
	ifname = malloc(strlen(filename) + strlen(MAPDIR) + 5);
	FileName = ifname;
	sprintf(ifname, "%s.map", filename);
	ifile = fopen(ifname, "r");

	if (ifile == NULL) {
	    sprintf(ifname, "%s/%s", MAPDIR, filename);
	    ifile = fopen(ifname, "r");

	    if (ifile == NULL) {
		free(ifname);
		return 0;
	    }
	}
	free(ifname);
    }
    ich = getc(ifile);
    if (ich != EOF)
	ungetc(ich, ifile);
    if (isascii(ich) && isdigit(ich)) {
	parseOldMapFile(ifile);
    } else {
	while (!feof(ifile))
	    parseLine(ifile);
    }
    return 1;
}


/*
 * Go through the hash table looking for name-value pairs that have defaults
 * assigned to them.   Process the defaults and, if possible, set the
 * associated variables.
 */
void parseOptions(void)
{
    int         i;
    valPair    *tmp;
    optionDesc *desc;

    for (i = 0; i < NHASH; i++)
	for (tmp = hashArray[i]; tmp; tmp = tmp->next) {
	    /* Does it have a default?   (If so, get a pointer to it) */
	    if (desc = tmp->def) {
		if (desc->variable) {
		    switch (desc->type) {
		    case valInt:
			{
			    int        *ptr = desc->variable;

			    *ptr = atoi(tmp->value);
			    break;
			}

		    case valReal:
			{
			    float     *ptr = desc->variable;

			    *ptr = atof(tmp->value);
			    break;
			}

		    case valBool:
			{
			    int        *ptr = desc->variable;

			    if (!strcasecmp(tmp->value, "yes")
				|| !strcasecmp(tmp->value, "on")
				|| !strcasecmp(tmp->value, "true"))
				*ptr = 1;
			    else if (!strcasecmp(tmp->value, "no")
				     || !strcasecmp(tmp->value, "off")
				     || !strcasecmp(tmp->value, "false"))
				*ptr = 0;
			    else {
				error("Invalid boolean value for %s - %s\n",
				      desc->name, tmp->value);
			    }
			    break;
			}

		    case valIPos:
			{
			    ipos       *ptr = desc->variable;
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
			    char      **ptr = desc->variable;

			    *ptr = tmp->value;
			    break;
			}
		    }
		}
	    }
	}
}

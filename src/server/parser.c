/* $Id: parser.c,v 5.18 2003/09/16 21:01:21 bertg Exp $
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
/* Original options parsing code contributed by Ted Lemon <mellon@ncd.com> */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef _WINDOWS
# include "NT/winServer.h"
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "defaults.h"
#include "error.h"
#include "portability.h"
#include "checknames.h"
#include "commonproto.h"


char parser_version[] = VERSION;


/*
 * Print the option list in "-help" format.
 * NT uses this to generate the ServerOpts.txt file
 */
static void Parse_help(char *progname)
{
    int			j;
    int			flags, all_flags;
    const char		*str;
    option_desc		*options;
    int			option_count;
    char		msg[MSG_LEN];

    options = Get_option_descs(&option_count);

    printf("Usage: %s [ options ]\n"
	   "\n",
	   progname);
    printf("Option flags specify where an option can be used and whether it\n"
	   "is visible or not.  Default flags are \"any\" if not otherwise\n"
	   "specified by one or more of these keywords:\n"
	   "\n");
    printf("command       May be specified on the command-line.\n"
	   "passwordfile  May be specified in the password file.\n"
	   "defaults      May be specified in the defaults file.\n"
	   "any           May be specified in the map file, defaults file,\n"
	   "              or on the command line.\n"
	   "invisible     Is not displayed when a list of options is\n"
	   "              requested from the server by an operator.\n"
	   "\n");
    printf("The possible options include:\n"
	   "\n");
    for (j = 0; j < option_count; j++) {
	printf("    %s%s",
	       options[j].type == valBool ? "-/+" : "-",
	       options[j].name);
	if (strcasecmp(options[j].commandLineOption, options[j].name))
	    xpprintf(" or %s", options[j].commandLineOption);
	printf(" %s\n",
	       options[j].type == valInt ? "<integer>" :
	       options[j].type == valReal ? "<real>" :
	       options[j].type == valString ? "<string>" :
	       options[j].type == valIPos ? "<position>" :
	       options[j].type == valSec ? "<seconds>" :
	       options[j].type == valPerSec ? "<per-second>" :
	       options[j].type == valList ? "<list>" :
	       "");
	for (str = options[j].helpLine; *str; str++) {
	    if (str == options[j].helpLine || str[-1] == '\n') {
		putchar('\t');
	    }
	    putchar(*str);
	}
	if (str > options[j].helpLine && str[-1] != '\n') {
	    putchar('\n');
	}
	flags = options[j].flags;
	all_flags = (OPT_ORIGIN_ANY | OPT_VISIBLE);
	if ((flags & all_flags) != all_flags && flags != 0) {
	    strlcpy(msg, "[ Flags: command, ", sizeof(msg));
	    if ((flags & OPT_PASSWORD) != 0) {
		strlcat(msg, "passwordfile, ", sizeof(msg));
	    }
	    if ((flags & (OPT_DEFAULTS | OPT_MAP)) == OPT_DEFAULTS) {
		strlcat(msg, "defaults, ", sizeof(msg));
	    }
	    if ((flags & OPT_MAP) != 0) {
		strlcat(msg, "any, ", sizeof(msg));
	    }
	    if ((flags & OPT_VISIBLE) == 0) {
		strlcat(msg, "invisible, ", sizeof(msg));
	    }
	    msg[strlen(msg) - 2] = '\0';
	    strlcat(msg, " ]", sizeof(msg));
	    printf("\t%s\n", msg);
	}
	putchar('\n');
    }
    printf(
"    \n"
"    The probabilities are in the range [0.0-1.0] and they refer to the\n"
"    probability that an event will occur in a block per second.\n"
"    Boolean options are turned off by using +<option>.\n"
"    \n"
"    Please refer to the manual pages, xpilots(6) and xpilot(6),\n"
"    for more specific help.\n"
	  );
}


/*
 * Print the option list.
 */
static void Parser_dump_options(char *progname)
{
    int			j;
    option_desc		*options;
    int			option_count;

    options = Get_option_descs(&option_count);

    for (j = 0; j < option_count; j++) {
	if (options[j].type != valVoid) {
	    int len = strlen(options[j].name);
	    xpprintf("%s:%*s%s\n", options[j].name,
		   (len < 40) ? (40 - len) : 1, "",
		   (options[j].defaultValue != NULL)
		       ? options[j].defaultValue
		       : "");
	}
    }
    xpprintf("\n");
}


/*
 * Print the option flags.
 */
static void Parser_dump_flags(char *progname)
{
    int			j;
    option_desc		*options;
    int			option_count;
    char		msg[MSG_LEN];

    options = Get_option_descs(&option_count);

    for (j = 0; j < option_count; j++) {
	int len = strlen(options[j].name);
	strlcpy(msg, "{", sizeof(msg));
	if ((options[j].flags & OPT_COMMAND) != 0) {
	    strlcat(msg, "command, ", sizeof(msg));
	}
	if ((options[j].flags & OPT_PASSWORD) != 0) {
	    strlcat(msg, "passwordfile, ", sizeof(msg));
	}
	if ((options[j].flags & OPT_DEFAULTS) != 0) {
	    strlcat(msg, "defaults, ", sizeof(msg));
	}
	if ((options[j].flags & OPT_MAP) != 0) {
	    strlcat(msg, "map, ", sizeof(msg));
	}
	if ((options[j].flags & OPT_VISIBLE) != 0) {
	    strlcat(msg, "visible, ", sizeof(msg));
	}
	if ((strlen(msg) >= 2)) {
	    msg[strlen(msg) - 2] = '\0';
	}
	strlcat(msg, "}", sizeof(msg));
	xpprintf("%s:%*s%s\n", options[j].name,
	       (len < 40) ? (40 - len) : 1, "", msg);
    }
    xpprintf("\n");
}


/*
 * Print some compile time configuration parameters.
 */
static void Parser_dump_config(char *progname)
{
    option_desc		*options;
    int			option_count;

    options = Get_option_descs(&option_count);

    xpprintf("\n");
    xpprintf("# %s option dump\n", progname);
    xpprintf("# \n");
    xpprintf("# LIBDIR = %s\n", Conf_libdir());
    xpprintf("# DEFAULTS_FILE_NAME = %s\n", Conf_defaults_file_name());
    xpprintf("# PASSWORD_FILE_NAME = %s\n", Conf_password_file_name());
    xpprintf("# MAPDIR = %s\n", Conf_mapdir());
    xpprintf("# DEFAULT_MAP = %s\n", Conf_default_map());
    xpprintf("# SERVERMOTDFILE = %s\n", Conf_servermotdfile());
    xpprintf("# ROBOTFILE = %s\n", Conf_robotfile());
    xpprintf("# \n");
}


/*
 * Print the option list and
 * some compile time configuration parameters.
 */
static void Parser_dump_all(char *progname)
{
    Parser_dump_config(progname);
    Parser_dump_options(progname);
}


/*
 * Get the current value of a server option
 * in a text buffer.
 *
 * This is called when a client requests
 * to see the current server parameter list.
 */
int Parser_list_option(int *index, char *buf)
{
    int			i = *index;
    option_desc		*options;
    int			option_count;

    options = Get_option_descs(&option_count);


    if (i < 0 || i >= option_count) {
	return -1;
    }
    if (options[i].defaultValue == NULL) {
	return 0;
    }
    if ((options[i].flags & OPT_VISIBLE) == 0) {
	return 0;
    }
    switch (options[i].type) {
    case valInt:
	sprintf(buf, "%s:%d", options[i].name,
		*(int *)options[i].variable);
	break;
    case valSec:
	sprintf(buf, "%s:%d", options[i].name,
		*(int *)options[i].variable / FPS);
	break;
    case valReal:
	sprintf(buf, "%s:%g", options[i].name,
		*(DFLOAT *)options[i].variable);
	break;
    case valPerSec:
	sprintf(buf, "%s:%g", options[i].name,
		*(DFLOAT *)options[i].variable * FPS);
	break;
    case valBool:
	sprintf(buf, "%s:%s", options[i].name,
		*(bool *)options[i].variable ? "yes" : "no");
	break;
    case valIPos:
	sprintf(buf, "%s:%d,%d", options[i].name,
		((ipos *)options[i].variable)->x,
		((ipos *)options[i].variable)->y);
	break;
    case valString:
	sprintf(buf, "%s:%s", options[i].name,
		*(char **)options[i].variable);
	break;
    case valList:
	{
	    list_t list = *(list_t *)options[i].variable;
	    sprintf(buf, "%s:", options[i].name);
	    if (list) {
		list_iter_t iter;
		for (iter = List_begin(list);
		     iter != List_end(list);
		     LI_FORWARD(iter)) {
		    char *str = LI_DATA(iter);
		    if (iter != List_begin(list)) {
			strlcat(buf, ",", MSG_LEN);
		    }
		    if (strlcat(buf, str, MSG_LEN) >= MSG_LEN) {
			break;
		    }
		}
	    }
	}
	break;
    default:
	return 0;
    }
    return 1;
}


/*
 * Check if the i-th command line argument
 * is a request for help or info.
 */
static bool Parse_check_info_request(char **argv, int i)
{
    char		*arg = argv[i];

    if (arg[0] == '-' && arg[1] == '-') {
	/* when arg starts with two dashes skip first one */
	arg++;
    }
    if (strcmp(arg, "-help") == 0
	|| strcmp(arg, "-h") == 0) {
	Parse_help(*argv);
	return TRUE;
    }
    if (strcmp(arg, "-dump") == 0) {
	Parser_dump_all(*argv);
	return TRUE;
    }
    if (strcmp(arg, "-dumpMan") == 0) {
	Parser_dump_options(*argv);
	return TRUE;
    }
    if (strcmp(arg, "-dumpWindows") == 0) {
	Parser_dump_options(*argv);
	return TRUE;
    }
    if (strcmp(arg, "-dumpFlags") == 0) {
	Parser_dump_flags(*argv);
	return TRUE;
    }
    if (strcmp(arg, "-version") == 0 || strcmp(arg, "-v") == 0) {
	puts(TITLE);
	return TRUE;
    }

    return FALSE;
}


/*
 * Parse all command line arguments
 * and read the server defaults file and map file.
 * Then convert the map data into a World structure.
 */
bool Parser(int argc, char **argv)
{
    int			i;
    bool		status;
    char		*fname;
    option_desc		*desc;

    if (Init_options() == FALSE) {
	return FALSE;
    }

    for (i = 1; i < argc; i++) {
	if (Parse_check_info_request(argv, i) == TRUE) {
	    return FALSE;
	}

	if (argv[i][0] == '-' || argv[i][0] == '+') {
	    desc = Find_option_by_name(argv[i] + 1);
	    if (desc != NULL) {
		if (desc->type == valBool) {
		    const char *bool_value;
		    if (argv[i][0] == '-') {
			bool_value = "true";
		    }
		    else {
			bool_value = "false";
		    }
		    Option_set_value(desc->name, bool_value, 1, OPT_COMMAND);
		}
		else if (desc->type == valVoid) {
		}
		else {
		    if (i + 1 == argc) {
			warn("Option '%s' needs an argument", argv[i]);
		    }
		    else {
			i++;
			Option_set_value(desc->name, argv[i], 1, OPT_COMMAND);
		    }
		}
	    }
	    else {
		warn("Unknown option '%s'", argv[i]);
	    }
	}
	else {
	    warn("Unknown option '%s'", argv[i]);
	}
    }

    /*
     * Read local defaults file
     */
    if ((fname = Option_get_value("defaultsFileName", NULL)) != NULL) {
	parseDefaultsFile(fname);
    }
    else {
	parseDefaultsFile(Conf_defaults_file_name());
    }

    /*
     * Read local password file
     */
    if ((fname = Option_get_value("passwordFileName", NULL)) != NULL) {
	parsePasswordFile(fname);
    }
    else {
	parsePasswordFile(Conf_password_file_name());
    }

    /*
     * Read map file if map data not found yet.
     *
     * If "mapFileName" is defined and it is not equal to "wild"
     * then read it's contents from file.  Else read a default map.
     */
    if (!(fname = Option_get_value("mapData", NULL))) {
	if ((fname = Option_get_value("mapFileName", NULL)) != NULL) {
	    if (strcasecmp(fname, "wild") && !parseMapFile(fname)) {
		xpprintf("Unable to read %s, trying to open %s\n",
			fname, Conf_default_map());
		if (!parseMapFile(Conf_default_map())) {
		    xpprintf("Unable to read %s\n", Conf_default_map());
		}
	    }
	} else {
	    xpprintf("Map not specified, trying to open %s\n",
		     Conf_default_map());
	    if (!parseMapFile(Conf_default_map())) {
		xpprintf("Unable to read %s\n", Conf_default_map());
	    }
	}
    }

    /*
     * Parse the options database and `internalise' it.
     */
    Options_parse();

    Options_free();

    /*
     * Construct the World structure from the options.
     */
    status = Grok_map();

    return status;
}


/*
 * Modify an option during the game.
 *
 * Options which can be modified have a so called tuner function,
 * which checks the validity of the new option value, and possibly
 * does something extra depending upon the option in question.
 * Options which don't need such a tuner function set it to `tuner_dummy'.
 * Options which cannot be modified have the tuner set to `tuner_none'.
 */
int Tune_option(char *name, char *val)
{
    int			ival;
    DFLOAT		fval;
    option_desc		*opt;

    if (!(opt = Find_option_by_name(name))) {
	return -2;	/* Variable not found */
    }

    if (opt->tuner == tuner_none) {
	return -1;	/* Operation undefined */
    }

    switch (opt->type) {
    case valInt:
	if (Convert_string_to_int(val, &ival) != TRUE) {
	    return 0;
	}
	*(int *)opt->variable = ival;
	(*opt->tuner)();
	return 1;
    case valBool:
	if (ON(val)) {
	    *(bool *)opt->variable = true;
	}
	else if (OFF(val)) {
	    *(bool *)opt->variable = false;
	}
	else {
	    return 0;
	}
	(*opt->tuner)();
	return 1;
    case valReal:
	if (Convert_string_to_float(val, &fval) != TRUE) {
	    return 0;
	}
	*(DFLOAT *)opt->variable = fval;
	(*opt->tuner)();
	return 1;
    case valSec:
	if (Convert_string_to_int(val, &ival) != TRUE) {
	    return 0;
	}
	*(int *)opt->variable = ival * FPS;
	(*opt->tuner)();
	return 1;
    case valPerSec:
	if (Convert_string_to_float(val, &fval) != TRUE) {
	    return 0;
	}
	*(DFLOAT *)opt->variable = fval / FPS;
	(*opt->tuner)();
	return 1;
    case valString:
	{
	    char *s = xp_strdup(val);
	    if (!s) {
		return 0;
	    }
	    if (*(char **)(opt->variable) != opt->defaultValue) {
		free(*(char **)opt->variable);
	    }
	    *(char **)opt->variable = s;
	    (*opt->tuner)();
	    return 1;
	}
    default:
	return -1;	/* Operation undefined */
    }
}


int Get_option_value(const char *name, char *value, unsigned size)
{
    option_desc		*opt;

    if (size < 12) {
	return -1;	/* Generic error. */
    }

    if (!(opt = Find_option_by_name(name))) {
	return -2;	/* Variable not found */
    }

    if ((opt->flags & OPT_VISIBLE) == 0)
	return -3;

    switch (opt->type) {
    case valInt:
	sprintf(value, "%d", *((int *)opt->variable));
	break;
    case valReal:
	sprintf(value, "%g", *((DFLOAT *)opt->variable));
	break;
    case valBool:
	sprintf(value, "%s", *((bool *)opt->variable) ? "true" : "false");
	break;
    case valString:
	if (*((char **)opt->variable) == NULL)
	    return -4;
	strlcpy(value, *((char **)opt->variable), size);
	break;
    case valSec:
	sprintf(value, "%d", *((int *)opt->variable) / FPS);
	break;
    case valPerSec:
	sprintf(value, "%g", *((DFLOAT *)opt->variable) * FPS);
	break;
    case valIPos:
	sprintf(value, "%d, %d",
		((ipos *)opt->variable)->x,
		((ipos *)opt->variable)->y);
    default:
	return -1;	/* Generic error. */
    }

    return 1;	/* Success. */
}



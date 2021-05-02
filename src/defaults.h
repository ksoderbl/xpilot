/* $Id: defaults.h,v 1.1 1993/03/09 14:31:16 kenrsc Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjxrn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

/* $NCDId: @(#)defaults.h,v 1.1 1992/09/10 03:26:12 mellon Exp $ */

typedef struct _valPair {
    struct _valPair *next;
    char *name;
    char *value;
    void *def;
} valPair;

enum valType { valInt, valReal, valBool, valIPos, valString };

typedef struct {
    char *name;
    char *commandLineOption;
    char *helpLine;
    char *defaultValue;
    void *variable;
    enum valType type;
} optionDesc;


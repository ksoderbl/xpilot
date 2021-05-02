/* $Id: defaults.h,v 3.3 1993/06/28 20:53:34 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
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


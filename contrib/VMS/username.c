#include <lib$routines.h>
#include <jpidef.h>
#include <string.h>
#include <ctype.h>
#include "username.h"


void getusername(char *uname)
{
    union {
	struct {
	    short bufferLength;
	    short itemCode;
	    char *buffer;
	    short *lengthPtr;
	} itmlst;
	long null;
    } inquiry[2];
    short length;
    char username[13];
    char *s;
    int   i;

    inquiry[0].itmlst.bufferLength = 12;
    inquiry[0].itmlst.itemCode = JPI$_USERNAME;
    inquiry[0].itmlst.buffer = username;
    inquiry[0].itmlst.lengthPtr = &length;
    inquiry[1].null = 0;
    
    SYS$GETJPIW(0,0,0,inquiry,0,0,0);
    strncpy(uname, username, length);
    uname[12] = 0;
    /* remove trailing blanks */
    s = uname + strlen(uname) - 1;
    while (isspace(*s)) {
        *s = 0;
        --s;
    }
    /*
     * VMS usernames are in upper case,
     * convert to lower case to make them look nicer
     */
    s = uname;
    for (i=strlen(uname); --i>=0; ++s)
    	*s = tolower(*s);
}

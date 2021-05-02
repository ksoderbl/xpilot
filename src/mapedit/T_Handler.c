/*
 * XMapEdit, the XPilot Map Editor.  Copyright (C) 1993 by
 *
 *      Aaron Averill           <averila@oes.orst.edu>
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
 *
 * Modifications to XMapEdit
 * 1996:
 *      Robert Templeman        <mbcaprt@mphhpd.ph.man.ac.uk>
 * 1997:
 *      William Docter          <wad2@lehigh.edu>
 *
 * $Id: T_Handler.c,v 5.0 2001/04/07 20:01:00 dik Exp $
 */

#include                 "T_Toolkit.h"

/***************************************************************************/
/* ValidateFloatHandler                                                    */
/* Arguments :                                                             */
/* Purpose :                                                               */
/***************************************************************************/
int ValidateFloatHandler(HandlerInfo info)
{
   char                  *returnval, *charvar;
   char                  *string,*start;
 
   charvar = info.field->charvar;
   returnval = malloc(strlen((char *) charvar)+1);
   returnval[0] = (char) NULL;
   string = malloc(strlen((char *) charvar)+1);
   start = (char *) string;
   strcpy(string,(char *) charvar);

   if ( (string[0] == '-') || ( (string[0] >= '0') && (string[0] <= '9') ) ||
        (string[0] == '.') ) 
   sprintf(returnval,"%s%c",returnval,string[0]);
   string++;
 
   while (string[0] != (char) NULL) {
      if ( ( (string[0] >= '0') && (string[0] <= '9') ) || (string[0] == '.') )
      sprintf(returnval,"%s%c",returnval,string[0]);
      string++;
   }

   strcpy((char *) charvar,returnval);
   free(returnval);
   free(start);
   return 0;
}

/***************************************************************************/
/* ValidatePositiveFloatHandler                                            */
/* Arguments :                                                             */
/* Purpose :                                                               */
/***************************************************************************/
int ValidatePositiveFloatHandler(HandlerInfo info)
{
   char                  *returnval, *charvar;
   char                  *string,*start;
 
   charvar = info.field->charvar;
   returnval = malloc(strlen((char *) charvar)+1);
   returnval[0] = (char) NULL;
   string = malloc(strlen((char *) charvar)+1);
   start = (char *) string;
   strcpy(string,(char *) charvar);

   while (string[0] != (char) NULL) {
      if ( ( (string[0] >= '0') && (string[0] <= '9') ) || (string[0] == '.') )
      sprintf(returnval,"%s%c",returnval,string[0]);
      string++;
   }

   strcpy((char *) charvar,returnval);
   free(returnval);
   free(start);
   return 0;
}

/***************************************************************************/
/* ValidateIntHandler                                                      */
/* Arguments :                                                             */
/* Purpose :                                                               */
/***************************************************************************/
int ValidateIntHandler(HandlerInfo info)
{
   char                  *returnval, *charvar;
   char                  *string,*start;
 
   charvar = info.field->charvar;
   returnval = malloc(strlen((char *) charvar)+1);
   returnval[0] = (char) NULL;
   string = malloc(strlen((char *) charvar)+1);
   start = (char *) string;
   strcpy(string,(char *) charvar);
 
   if ( (string[0] == '-') || ( (string[0] >= '0') && (string[0] <= '9') ) )
   sprintf(returnval,"%s%c",returnval,string[0]);
   string++;
 
   while (string[0] != (char) NULL) {
      if ( (string[0] >= '0') && (string[0] <= '9') )
      sprintf(returnval,"%s%c",returnval,string[0]);
      string++;
   }
 
   strcpy((char *) charvar,returnval);
   free(returnval);
   free(start);
   return 0;
}

/***************************************************************************/
/* ValidatePositiveIntHandler                                              */
/* Arguments :                                                             */
/* Purpose :                                                               */
/***************************************************************************/
int ValidatePositiveIntHandler(HandlerInfo info)
{
   char                  *returnval, *charvar;
   char                  *string,*start;
 
   charvar = info.field->charvar;
   returnval = malloc(strlen((char *) charvar)+1);
   returnval[0] = (char) NULL;
   string = malloc(strlen((char *) charvar)+1);
   start = (char *) string;
   strcpy(string,(char *) charvar);
 
   while (string[0] != (char) NULL) {
      if ( (string[0] >= '0') && (string[0] <= '9') )
      sprintf(returnval,"%s%c",returnval,string[0]);
      string++;
   }
 
   strcpy((char *) charvar,returnval);
   free(returnval);
   free(start);
   return 0;
}

/***************************************************************************/
/* PopupCloseHandler                                                       */
/* Arguments :                                                             */
/* Purpose :                                                               */
/***************************************************************************/
int PopupCloseHandler(HandlerInfo info)
{
   T_PopupClose(info.form->window);
   return 0;
}

/***************************************************************************/
/* FormCloseHandler                                                        */
/* Arguments :                                                             */
/* Purpose :                                                               */
/***************************************************************************/
int FormCloseHandler(HandlerInfo info)
{
   XUnmapWindow(display, info.form->window);
   return 0;
}

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
 * $Id: forms.c,v 1.4 2000/03/11 20:25:58 bert Exp $
 */

#include                 "main.h"

/***************************************************************************/
/* BuildMapwinForm                                                         */
/* Arguments :                                                             */
/* Purpose :                                                               */
/***************************************************************************/
void BuildMapwinForm(void)
{
   int                   w,h,y;
  
   w = (TOOLSWIDTH-20)/4;
   h = w*.6;
   /* RTT 5x6 multibutton */
   T_FormMultiButton(mapwin,"drawicon_select",10,5,TOOLSWIDTH-20,
        (int)((TOOLSWIDTH-20)*0.8*1.75),5,7,"",&drawicon,1);
   y = (TOOLSWIDTH-20)*0.8*1.75+10;
   /* RTT *1.2 was *.8 */
 
   T_FormMultiButton(mapwin,"drawmode_select",10,y,TOOLSWIDTH-20,h,3,1,
        "Draw;Line;Select",&drawmode,0);
   y += h+5;

   T_FormButton(mapwin,"load",10,y,w-1,h-1,"Load",LoadPrompt);
   T_FormButton(mapwin,"save",10+w,y,w-1,h-1,"Save",SavePrompt);
   T_FormButton(mapwin,"new",10+w*2,y,w-1,h-1,"New",NewMap);
   T_FormButton(mapwin,"quit",10+w*3,y,w-1,h-1,"Quit",
        ExitApplication);

   T_FormButton(mapwin,"cut",10,y+h,w-1,h-1,"Cut",CutMapArea);
   T_FormButton(mapwin,"copy",10+w*1,y+h,w-1,h-1,"Copy",
        CopyMapArea);
   T_FormButton(mapwin,"paste",10+w*2,y+h,w-1,h-1,"Paste",
        PasteMapArea);
   T_FormButton(mapwin,"undo",10+w*3,y+h,w-1,h-1,"Undo", Undo);

   T_FormButton(mapwin,"round",10,y+h*2,w-1,h-1,"Round",
        RoundMapArea);
   T_FormButton(mapwin,"fill",10+w*1,y+h*2,w-1,h-1,"Fill",
        FillMapArea);
   T_FormHoldButton(mapwin,"grow",10+w*2,y+h*2,w-1,h-1,"Grow",GrowMapArea);
   T_FormButton(mapwin,"neg",10+w*3,y+h*2,w-1,h-1,"Neg.",
        NegativeMapArea);

   T_FormButton(mapwin,"preferences",10,y+3*h,w-1,h-1,
        "Prefs",OpenPreferencesPopup);
   T_FormButton(mapwin,"help",10+w,y+3*h,w-1,h-1,
        "Help",OpenHelpPopup);
   T_FormButton(mapwin,"zoom_in",10+w*2,y+3*h,w-1,h-1,"Z",
        ZoomIn);
   T_FormButton(mapwin,"zoom_out",10+w*3,y+3*h,w-1,h-1,"z",
        ZoomOut);

   T_FormStringEntry(mapwin,"map_name",5,TOOLSHEIGHT-TOOLSWIDTH-90,
        TOOLSWIDTH-10,20,0,-20,"Map Name:",map.mapName,sizeof(max_str_t)-1,NULL);
   T_FormStringEntry(mapwin,"map_author",5,TOOLSHEIGHT-TOOLSWIDTH-50,
        TOOLSWIDTH-10,20,0,-20,"Map Author:",map.mapAuthor,
        sizeof(max_str_t)-1,NULL);
   T_FormStringEntry(mapwin,"map_width",(int)(TOOLSWIDTH/2-45),
        TOOLSHEIGHT-TOOLSWIDTH-25, 40,20,-50,0,"Width:", map.width_str,
        3, ResizeWidth);
   T_FormStringEntry(mapwin,"map_height",(int)(TOOLSWIDTH-45),
        TOOLSHEIGHT-TOOLSWIDTH-25,40,20,-50,0,"Height:", map.height_str,
        3, ResizeHeight);

}

/***************************************************************************/
/* BuildPrefsForm                                                          */
/* Arguments :                                                             */
/* Purpose :                                                               */
/***************************************************************************/
void BuildPrefsForm(void)
{
   int                   w,h,t,i;
   char                  *tmpstr=NULL;
 
   T_FormButton(prefwin,"close_prefs",10,PREF_HEIGHT-10-PREF_BTN_HEIGHT,
        PREF_BTN_WIDTH,PREF_BTN_HEIGHT,"Close",FormCloseHandler);
 
   /* preference fields */
   w = (480-40)/4;
   h = PREF_BTN_HEIGHT+2;
   t = (PREF_BTN_HEIGHT-20)/2;
   
   for (i=0; i<numprefs; i++) {
      switch (prefs[i].type) {
 
      case MAPWIDTH:
         T_FormStringEntry(prefwin,"mapwidth",10+w+prefs[i].column*(w*2+20),
			   10+prefs[i].row*h+prefs[i].space+t,w,20,-w,0,
			   "Width:",map.width_str,3,ResizeWidth);
         break;
 
      case MAPHEIGHT:
         T_FormStringEntry(prefwin,"mapheight",10+w+prefs[i].column*(w*2+20),
			   10+prefs[i].row*h+prefs[i].space+t,w,20,-w,0,
			   "Height:",map.height_str,3,ResizeHeight);
         break;
 
      case STRING:
	/*         T_FormStringEntry(prefwin,prefs[i].name,60,10+prefs[i].space,
		   450-175,20,-50,0,prefs[i].label,prefs[i].charvar,
		   prefs[i].length,NULL);
		   break;*/
	T_FormStringEntry(prefwin,prefs[i].name ,10+w+prefs[i].column*(w*2+20),
			  10+prefs[i].row*h+prefs[i].space+t,w,20,-w,0,
			  prefs[i].label, prefs[i].charvar,
			  prefs[i].length,NULL);
	break;
	
      case YESNO:
	T_FormMultiButton(prefwin,prefs[i].name,10+w+prefs[i].column*(w*2+20),
			  10+prefs[i].row*h+prefs[i].space,w,h-2,2,1,"No;Yes",
			  prefs[i].intvar,1);
	tmpstr = malloc(strlen(prefs[i].name)+6);
	strcpy(tmpstr,prefs[i].name);
         strcat(tmpstr,"_text");
         T_FormText(prefwin,tmpstr,10+prefs[i].column*(w*2+20),10+
              prefs[i].row*h+prefs[i].space,w,20,prefs[i].label,JUSTIFY_LEFT);
         break;
 
      case INT:
         T_FormStringEntry(prefwin,prefs[i].name,10+w+prefs[i].column*(w*2+20),
              10+prefs[i].row*h+prefs[i].space+t,w,20,-w,0,prefs[i].label,
              prefs[i].charvar,prefs[i].length,ValidateIntHandler);
         break;
 
      case POSINT:
         T_FormStringEntry(prefwin,prefs[i].name,10+w+prefs[i].column*(w*2+20),
              10+prefs[i].row*h+prefs[i].space+t,w,20,-w,0,prefs[i].label,
              prefs[i].charvar,prefs[i].length,ValidatePositiveIntHandler);
         break;
 
      case FLOAT:
         T_FormStringEntry(prefwin,prefs[i].name,10+w+prefs[i].column*(w*2+20),
			   10+prefs[i].row*h+prefs[i].space+t,w,20,-w,0,prefs[i].label,
			   prefs[i].charvar,prefs[i].length,NULL);
         break;
 
      case POSFLOAT:
         T_FormStringEntry(prefwin,prefs[i].name,10+w+prefs[i].column*(w*2+20),
              10+prefs[i].row*h+prefs[i].space+t,w,20,-w,0,prefs[i].label,
              prefs[i].charvar,prefs[i].length,NULL);
         break;
 
      case COORD:
         T_FormStringEntry(prefwin,prefs[i].name,10+w+prefs[i].column*(w*2+20),
              10+prefs[i].row*h+prefs[i].space+t,w,20,-w,0,prefs[i].label,
              prefs[i].charvar,prefs[i].length,ValidateCoordHandler);
         break;
 
      }
   }
}


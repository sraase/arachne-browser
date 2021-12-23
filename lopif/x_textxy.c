/*-------------------------------------------------*/
/* Napise text do videoram fontem zadanym v        */
/* x_fnt_load()                                    */
/*-------------------------------------------------*/
/* x_setcolor - nastavena barva textu  (xg_color)  */
/* x_setfill  - nastavena barva pozadi (xg_fillc)  */
/* x_settextjust - left,center,right   (xg_tjust)  */
/*                                                 */
/* Pro 16 barevne mody se pise pres XOR            */
/* (Spocte se nejprve nova barva textu)            */
/* Pro 256 barevne mody se primo zada barva textu  */
/* a pozadi                                        */
/* Vstup [X,Y] je v pixlech a prepocte se na text. */
/* radky a sloupce                                 */
/* Velikost fontu je znama z x_fnt_load()          */
/* Problemy : EGA/VGA fonty - 14/16 radku na znak  */
/* => 25/30 radku na obrazovce  (80 sloupcu)       */
/*-------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "x_lopif.h"

void x_textxy(int xp, int yp, char *text)
{
   int xt,yt;          /* textove radky a sloupce */
   int xzb,yzb,xo_col,len,xpp,ypp;

   xpp = xp + xg_view[0];  /* Do viewportu  */
   ypp = yp + xg_view[1];

   xt = xpp / xg_xfnt;  /* zaokrouhlovani v X */
   xzb= xpp % xg_xfnt;
   if(xzb >= (xg_xfnt/2) ) xt++;

   yt = ypp / xg_yfnt;  /* zaokrouhlovani v Y */
   yzb= ypp % xg_yfnt;
   if(yzb >= (xg_yfnt/2) ) yt++;

   len = strlen(text);

   switch(xg_tjustx)
     { case 0:                    /* Left   */
	       break;
       case 1: xt = xt - len/2 ;  /* Center */
	       break;
       case 2: xt = xt - len + 1; /* Right  */
     }

   if(xg_256 == MM_256)     /* 256 mod */
     {
       x_text_1(xg_color,xt,yt,text,xg_fillc);
     }
   else if(xg_256 == MM_16) /* 16 mod */
     { xo_col = xg_color ^ xg_fillc;
       x_text_1(xo_col,xt,yt,text,1);
     }
   else if(xg_256 == MM_2)
     {
     }
}


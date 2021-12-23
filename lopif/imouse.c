/******************************************************************/
/*   Funkce pro cteni polohy myssi s vypnutym kursorem, fungujici */
/*   i v SVGA gr. modech                                          */
/******************************************************************/
//(C) Ivan Polak, Jan Vlaciha, 1992

#include  <dos.h>
#include  <bios.h>
#include  <stdio.h>
#include  <stdlib.h>
#include <time.h>
//#include "x_lopif.h"
//---------------------------- Globalni promenne ------------
#define GMOUSE 0x33
#define _MOUSENINS 0
#define _MOUSEINS3 2

int  mouseStatus;
int  xdisplay= 0, ydisplay= 0, qfix = 0;
int  xymax[4];
int I_inimouse=0;
/***********************************************************************/
/*         ImouseIni - Inicializace snimani polohy                     */
/***********************************************************************/
/* Parametry :  xmin, ymin - levy horni roh pomyslneho okna, v nemz
			     se ma kursor pohybovat
		xmax, ymax - pravy dolni roh okna kursoru
		xstart, ystart - pocatecni poloha kursoru
Poznamka  :  pred ImouseIni musi byt inicializovana grafika,
	     uvnitr je vyresestovana myss (a tudiz vypnut kursor myssi).

	     ImouseIni se muze libovolne opakovat - neni treba
	     zadne Close. Namisto ...Ini lze potom tez pouzit ImouseLimit
*/
/* Return    :  0 ...  neni instalovana Genius Mouse                    */
/*             -1 ... je instalovana      - " -                        */

int ImouseIni( int xmin, int ymin, int xmax, int ymax,
		int xstart, int ystart)
{
   union REGS regs;

 if( !I_inimouse)  //POZOR! delat jen 1-krat - kvuli pochybnym driverum Genius!!!
 {
   regs.x.ax = 0;            /* Reset Genius Mouse */
   int86 (GMOUSE, &regs, &regs);
   if (regs.x.ax==0)
     {
     mouseStatus = _MOUSENINS;
     return 0;
     }
   else mouseStatus = _MOUSEINS3;
   I_inimouse=1;
 }


   regs.x.ax = 2;         /* Disable Cursor */
   int86 (GMOUSE, &regs, &regs);

   xdisplay= xstart;
   ydisplay= ystart;

   xymax[0]= xmin;
   xymax[1]= ymin;
   xymax[2]= xmax;
   xymax[3]= ymax;

  return -1;
}
/***********************************************************************/
/*         ImouseRead - Snimani polohy  myssi                          */
/***********************************************************************/
/* Parametry :
     *xcurs, *ycurs - prectene souradnice
Poznamka:
Po zavolani ImouseIni a mezi opakovanim volani ImouseRead neni radno
volat gmouse driver, zejmena "Read cursor position" nebo "Set cursor
position"!

Return: "button status" myssi, tj.: bit 0 = leve tlacitko
				    bit 1 = prave tlacitko
				    bit 2 = prostredni tlac.
*/
int ImouseRead( int *xcurs, int *ycurs)
{
   union REGS regs;
//======================================== Vstup z myssi ==========>>>
   if (mouseStatus==_MOUSENINS) return 0;
/*
Read Genius Mouse Motion Number                          11
Input:  AX = 11
Return: CX = horizontal number
	DX = vertical number
*/
   regs.x.ax = 11;            /* Read Cursor Location & Button State */
   int86 (GMOUSE, &regs, &regs);

   if ( !(qfix&1) ) xdisplay += regs.x.cx;
   xdisplay= min( xymax[2], max( xymax[0], xdisplay) );
   if ( !(qfix&2) ) ydisplay += regs.x.dx;
   ydisplay= min( xymax[3], max( xymax[1], ydisplay) );

   *xcurs= xdisplay;
   *ycurs= ydisplay;

   regs.x.ax = 3;            /* Read Cursor Location & Button State */
   int86 (GMOUSE, &regs, &regs);

//   gotoxy(2,2);
//   printf("  %d         %d        ",xdisplay,ydisplay);
   return( regs.x.bx );
}

/***********************************************************************/
/*         ImouseSet - prestaveni polohy myssi                         */
/***********************************************************************/
/* Parametry :   xstart, ystart - pozadovana poloha kursoru            */

void ImouseSet( int xstart, int ystart)
{
//   union REGS regs;

//   regs.x.ax = 11;            /* Read Cursor Location & Button State */
//   int86 (GMOUSE, &regs, &regs);

   xdisplay = xstart;
   xdisplay= min( xymax[2], max( xymax[0], xdisplay) );
   ydisplay = ystart;
   ydisplay= min( xymax[3], max( xymax[1], ydisplay) );

   return;
}

/***********************************************************************/
/*           ImouseFix - Fixovani jedne souradnice myssi               */
/***********************************************************************/
/* Parametr  :  fix ... 0 kurzor nefixovan                             */
/*       	    ... 1 fixni x                                      */
/*       	    ... 2 fixni y                                      */
/* Fixovana je jen ImouseRead, nikoliv ImouseSet.                      */

void ImouseFix( int fix )
{
   qfix = fix;
}


/***********************************************************************/
/*                ImouseBut - cteni tlacitek myssi                     */
/***********************************************************************/
/* Pouze ceka na stisknuti tlacitka mysi. Pohyby mysi jsou zatim igno- */
/* rovany. Vraci stejne hodnoty jako ImouseRead.                       */

int ImouseBut( void )
{
   union REGS regs;

   if (mouseStatus==_MOUSENINS) return(0);
//   regs.x.ax = 11;            /* Read Cursor Location & Button State */
//   int86 (GMOUSE, &regs, &regs);

   regs.x.ax = 3;            /* Read Cursor Location & Button State */
   int86 (GMOUSE, &regs, &regs);

   return( regs.x.bx );
}

/***********************************************************************/
/*         ImouseLimit - zmena okna pro snimani polohy myssi           */
/***********************************************************************/
/* Parametry :  xmin, ymin - levy horni roh pomyslneho okna, v nemz
			     se ma kursor pohybovat
		xmax, ymax - pravy dolni roh okna kursoru

  Return    :  vzdy 1
*/

int ImouseLimit( int xmin, int ymin, int xmax, int ymax)
{
   xymax[0]= xmin;
   xymax[1]= ymin;
   xymax[2]= xmax;
   xymax[3]= ymax;

return 1;
}
/***********************************************************************/
/*         ImouseWait - cekani na "uklidneni" mysi         */
/***********************************************************************/
/*  Fce ceka na uvolneni tlacitek mysi, pokud je nektere stisknuto.
    Potom udela reset. Ev.pohyby mysi do uvolneni tlacitka se tudiz
    ztrati.
    Fci je vhodne volat vzdy po ukonceni urcite akce vyvolane stiskem
    tlacitka mysi.
*/

void ImouseWait(void)
{
   union REGS regs;
   time_t t;

   if (mouseStatus==_MOUSENINS) return;


   t = time(NULL) + 3; //casovy limit - Pola*940412
WAIT:
   if(time(NULL) >= t) goto Konec;
   regs.x.ax = 3;            /* Read Cursor Location & Button State */
   int86 (GMOUSE, &regs, &regs);
   if( regs.x.bx != 0 ) goto WAIT;

Konec:
   regs.x.ax = 2;         /* Disable Cursor */
   int86 (GMOUSE, &regs, &regs);

  return;
}

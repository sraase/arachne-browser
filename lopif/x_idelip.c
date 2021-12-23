#include   <math.h>
#include   <stdlib.h>
#include "x_lopif.h"

/******** Macra *******************************************************/
#define INOKNO( a, b ) \
   ( a >= okno[0] && a <= okno[2] && \
     b >= okno[1] && b <= okno[3] )

/**********************************************************************/
/*               Funkce pro identifikaci elipsy(kruznice)             */
/**********************************************************************/
//  Parametry : okno[4].... souradnice identifikacniho okna (vstup)
//                          okno[0], okno[1] souradnice prvniho
//                          identifikovaneho bodu (vystup)
//              x1, y1 .... souradnice stredu
//              rx, ry .... polomery (pro kruznici shodne)
//  Return    : 0/1  ...... identifikovano ne/ano
//  Poznamka  : u elips(kruznic) se identifikuje jen hranice, a to i
//              u vyplnenych

int     x_id_ellip( int okno[4],      // identifikacni okno (I/O)
		    int x1, int y1,   // stred
		    int rx, int ry )  // polomery
{
   int    x, y;
   long   p, n, test, pa, na;
   int    ix, iy;

   //okno[0] += xg_view[0];                // posun ve viewportu
   //okno[1] += xg_view[1];
   //okno[2] += xg_view[0];
   //okno[3] += xg_view[1];

   //okno[0] = max( okno[0], xg_view[0] ); // omezeni na vnitrek viewportu
   //okno[1] = max( okno[1], xg_view[1] );
   //okno[2] = min( okno[2], xg_view[2] );
   //okno[3] = min( okno[3], xg_view[3] );

   //if ( okno[0] >= okno[2] || okno[1] >= okno[3] ) return(0); //mimo viewp.

   rx = (int) ((long)rx * 10000L / (long)xg_xr); /* prepocet    */
   ry = (int) ((long)ry * 10000L / (long)xg_yr); /* aspectratio */

   //x1 += xg_view[0];                             /* posunuti    */
   //y1 += xg_view[1];                             /* viewportu   */

   pa = (long) rx * (long) rx;                   /* inicializace */
   test = (long) ( -2 * ry + 1 ) * pa;
   pa *= 4L;
   p = pa * (long) (1 - ry);
   n = 2L * (long) ry * (long) ry;
   na = 2L * n;
   x = 0;
   y = ry;

   for ( ; ; )
   {
     ix = x1+x;  iy = y1+y;
     if INOKNO( ix, iy ) goto Ide;
     iy = y1-y;
     if INOKNO( ix, iy ) goto Ide;
     ix = x1-x;
     if INOKNO( ix, iy ) goto Ide;
     iy = y1+y;
     if INOKNO( ix, iy ) goto Ide;

     if ( ry < 0 ) break;
     if ( test < 0L )
      {  x++ ; test += n; n += na; }
     else
      {  y--; test += p; p += pa; if ( p > 0L) ry = -1; }
   }
   return(0);

Ide:
   //okno[0] = ix - xg_view[0];
   //okno[1] = iy - xg_view[1];
   okno[0] = ix;
   okno[1] = iy;
   return(1);
}

#include   <math.h>
#include   <stdlib.h>
#include   "x_lopif.h"

/**********************************************************************/
/*                    Funkce pro kresleni usecky                      */
/**********************************************************************/
/*          x1, y1 ... jeden bod                                      */
/*          x2, y2 ... druhy bod                                      */

void  x_line(int x1, int y1, int x2, int y2 )
{
   int     i, n, p, test, ky, istyle = 0;
#if HI_COLOR
   int     SaveMode;
   SaveMode = xg_hipalmod;
   xg_hipalmod = 1;
#endif

   if ( !xg_video_XMS && !x_viewline( &x1, &y1, &x2, &y2 )) return;

   if ( x2 < x1 )            /* zamena, aby levy bod byl prvni */
   {
     i = x1;   x1 = x2;   x2 = i;
     i = y1;   y1 = y2;   y2 = i;
   }

   /* Modified by xChaos, 27.4.1997 - male zrychleni, presun sem Haro */
   if (y1==y2 || x1==x2)
   {
    int tmp=xg_fillc;
    xg_fillc=xg_color;                 /* Nastavena barva (rect,text) */
    x_bar(x1,y1,x2,y2);
    xg_color=xg_fillc;                 /* Nastavena barva pro x_bar() */
    xg_fillc=tmp;
#if HI_COLOR
    xg_hipalmod = SaveMode;
#endif
    return;
   }
   /* konec zrychleni */

   x2 -= x1;
   y2 -= y1;                 /* vypocet kroku ve smeru y */
   if ( y2 < 0 ) { y2 = -y2; ky = -1; } else ky = 1;

   if ( x2 < y2  )           /* strmejsi nez 45 stupnu */
   {
     p =  2 * x2;
     n =  2 * y2 - p;
     test =   y2 - p;
     for ( i = 0; i <= y2; i++ )
     {
       if ( xg_style & ( 1 << (istyle++ % 16)))
	 {
	  x_putpix( x1, y1, xg_color );
	 }

       if ( test < 0 ) { test += n; x1++; }
       else              test -= p;
       y1 += ky;
     }
   }
   else                      /* mene strma */
   {
     p =  2 * y2;
     n =  2 * x2 - p;
     test =   x2 - p;
     for ( i = 0; i <= x2; i++ )
     {
       if ( xg_style & ( 1 << (istyle++ % 16)))
	{
	   x_putpix( x1, y1, xg_color );
	 }
       if ( test < 0 ) { test += n; y1 += ky; }
       else            	 test -= p;
       x1++;
     }
   }
#if HI_COLOR
   xg_hipalmod = SaveMode;
#endif
}

/**********************************************************************/
/*           Funkce pro prepocet usecky vzhledem k viewportu          */
/**********************************************************************/
/*                                     algoritmus  Cohen-Sutherland   */

int  x_viewline(int *x1, int *y1, int *x2, int *y2 )
{
  int  i, ka, kb = 0;
  long plong;

  *x1 += xg_view[0];         /* posunuti do pocatku viewportu */
  *x2 += xg_view[0];
  *y1 += xg_view[1];
  *y2 += xg_view[1];

  if      ( *x2 < xg_view[0] ) kb |= 1;     /* vypocet kodu pro druhy bod */
  else if ( *x2 > xg_view[2] ) kb |= 2;
  if      ( *y2 < xg_view[1] ) kb |= 4;
  else if ( *y2 > xg_view[3] ) kb |= 8;

View:
  ka = 0;
  if      ( *x1 < xg_view[0] ) ka |= 1;     /* vypocet kodu pro prvni bod */
  else if ( *x1 > xg_view[2] ) ka |= 2;
  if      ( *y1 < xg_view[1] ) ka |= 4;
  else if ( *y1 > xg_view[3] ) ka |= 8;

  if ( !ka && !kb ) return(1);
  if (  ka  &  kb ) return(0);

  if ( !ka )                                /* zamena */
   {
     i = *x1;   *x1 = *x2;   *x2 = i;
     i = *y1;   *y1 = *y2;   *y2 = i;
     i = ka;     ka =  kb;    kb = i;
   }

   if ( ka & 1 )                            /* vypocet prusecikuu */
   {
     plong = (long)*y2 -  ((long)*y2 - (long)*y1)
	    *((long)*x2 -  (long)xg_view[0])  / ((long)*x2 - (long)*x1 );
     *y1 = (int) plong;
     *x1 = xg_view[0];
   }
   else if ( ka & 2 )
   {
     plong = (long)*y2 -  ((long)*y2 - (long)*y1)
	    *((long)*x2 -  (long)xg_view[2])  / ((long)*x2 - (long)*x1 );
     *y1 = (int) plong;
     *x1 = xg_view[2];
   }
   else if ( ka & 8 )
   {
     plong = (long)*x2 -  ((long)*x2 - (long)*x1)
	    *((long)*y2 -  (long)xg_view[3])  / ((long)*y2 - (long)*y1 );
     *x1 = (int) plong;
     *y1 = xg_view[3];
   }
   else
   {
     plong = (long)*x2 -  ((long)*x2 - (long)*x1)
	    *((long)*y2 -  (long)xg_view[1])  / ((long)*y2 - (long)*y1 );
     *x1 = (int) plong;
     *y1 = xg_view[1];
   }
   goto View;
}





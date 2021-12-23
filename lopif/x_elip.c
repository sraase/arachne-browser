#include   <math.h>
#include   <stdlib.h>
#include   "x_lopif.h"

/******** Macra *******************************************************/
#define INVIEW( a, b ) \
   ( a >= xg_view[0] && a <= xg_view[2] && \
     b >= xg_view[1] && b <= xg_view[3] )


/**********************************************************************/
/*                    Funkce pro kresleni kruznice                    */
/**********************************************************************/
/*          x1, y1 .... souradnice stredu                             */
/*          r      .... polomer                                       */

void  x_circle(int x1, int y1, int r )
{
   x_ellipse( x1, y1, r, r );
}

/**********************************************************************/
/*                    Funkce pro kresleni elipsy                      */
/**********************************************************************/
/*          x1, y1 .... souradnice stredu                             */
/*          rx, ry .... polomery ve smeru x a y                       */

void  x_ellipse(int x1, int y1, int rx, int ry )
{
   int    x, y, istyle = 0;
   long   p, n, test, pa, na;
   int    xz,xk,loc_style;
#if HI_COLOR
   int    SaveMode;
   SaveMode = xg_hipalmod;
   xg_hipalmod = 1;
#endif

   loc_style = xg_style;
   if((xg_flag & 0x0004) != 0)  // Vyplnene kolecko (ne carkovane)
     { xg_style = 0xFFFF;
     }

   xg_notview = 1;    // Zadny posun v X_BAR

   rx = (int) ((long)rx * 10000L / (long)xg_xr); /* prepocet    */
   ry = (int) ((long)ry * 10000L / (long)xg_yr); /* aspectratio */

   x1 += xg_view[0];                             /* posunuti    */
   y1 += xg_view[1];                             /* viewportu   */

   if( rx < 1 || ry < 1 )
   {
     x_rect( x1-rx, y1-ry, x1+rx, y1+ry );
     goto Ok;
   }

   pa = (long) rx * (long) rx;                   /* inicializace */
   test = (long) ( -2 * ry + 1 ) * pa;
   pa *= 4L;
   p = pa * (long) (1 - ry);
   n = 2L * (long) ry * (long) ry;
   na = 2L * n;
   x = 0;
   y = ry;

   if ( ( x1-rx ) >= xg_view[0] && ( x1+rx ) <= xg_view[2] &&
	( y1-ry ) >= xg_view[1] && ( y1+ry ) <= xg_view[3] )
   {
     for ( ; ; )   /* elipsa uvnitr viewportu - rychly cyklus */
     {
       if ( xg_style & ( 1 << (istyle++ % 16)))
       {
	 if((xg_flag & 0x0004) == 0)    // NEVYPLNENA
	 {
	 x_putpix( x1+x, y1+y, xg_color );
	 x_putpix( x1+x, y1-y, xg_color );
	 x_putpix( x1-x, y1+y, xg_color );
	 x_putpix( x1-x, y1-y, xg_color );
	 }
	 else                           // VYPLNENA
	 {
	 x_bar(x1-x,y1+y,x1+x,y1+y);
	 x_bar(x1-x,y1-y,x1+x,y1-y);
	 }
       }
       if ( ry < 0 ) break;
       if ( test < 0L )
	 {  x++ ; test += n; n += na; }
       else
	 {  y--; test += p; p += pa; if ( p > 0L) ry = -1; }
     }
   }
   else
   {
     for ( ; ; )   /* elipsa vne viewportu - pomaly cyklus */
     {
       if ( xg_style & ( 1 << (istyle++ % 16)))
       {
	 if((xg_flag & 0x0004) == 0)    // NEVYPLNENA
	 {
	 if INVIEW(x1+x, y1+y) x_putpix( x1+x, y1+y, xg_color );
	 if INVIEW(x1+x, y1-y) x_putpix( x1+x, y1-y, xg_color );
	 if INVIEW(x1-x, y1+y) x_putpix( x1-x, y1+y, xg_color );
	 if INVIEW(x1-x, y1-y) x_putpix( x1-x, y1-y, xg_color );
	 }
	 else
	 {
	 xz = max(x1-x,xg_view[0]);
	 if(xz < xg_view[2])
	  {
	  xk = min(x1+x,xg_view[2]);
	  if((y1-y) >= xg_view[1] && (y1-y) <= xg_view[3])
	   x_bar(xz,y1-y,xk,y1-y);
	  if((y1+y) >= xg_view[1] && (y1+y) <= xg_view[3])
	   x_bar(xz,y1+y,xk,y1+y);
	  }
	 }
       }
       if ( ry < 0 ) break;
       if ( test < 0L )
	 {  x++ ; test += n; n += na; }
       else
	 {  y--; test += p; p += pa; if ( p > 0L) ry = -1; }
     }
   }
Ok:
   xg_style = loc_style;
   xg_notview = 0;
#if HI_COLOR
   xg_hipalmod = SaveMode;
#endif
}

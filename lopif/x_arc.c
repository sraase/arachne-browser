#include   <math.h>
#include   <stdlib.h>
#include   "x_lopif.h"

/******** Macra *******************************************************/
#define INVIEW( a, b ) \
   ( a >= xg_view[0] && a <= xg_view[2] && \
     b >= xg_view[1] && b <= xg_view[3] )

#define INOKNO( a, b ) \
   ( a >= okno[0] && a <= okno[2] && \
     b >= okno[1] && b <= okno[3] )


/**********************************************************************/
/*               Funkce pro kresleni kruhoveho oblouku                */
/**********************************************************************/
/*          x1, y1 .... souradnice stredu                             */
/*          sa, ea .... pocatecni a koncovy uhel (ve stupnich)        */
/*          r      .... polomer                                       */

void  x_arc(int x1, int y1, int sa, int ea, int r )
{
   int    x, y, p, n, test, mask, okno[4], neg;
#if HI_COLOR
   int SaveMode;
   SaveMode = xg_hipalmod;
   xg_hipalmod = 1;
#endif

   sa %= 360;
   ea %= 360;
   x =  floor( (double)r * cos( (double)sa * (3.1415962 / 180.0))+0.5);
   y = -floor( (double)r * sin( (double)sa * (3.1415962 / 180.0))+0.5);
   p =  floor( (double)r * cos( (double)ea * (3.1415962 / 180.0))+0.5);
   n = -floor( (double)r * sin( (double)ea * (3.1415962 / 180.0))+0.5);

   x1 += xg_view[0];
   y1 += xg_view[1];
   okno[0] = min(x,p) + x1;
   okno[1] = min(y,n) + y1;
   okno[2] = max(x,p) + x1;
   okno[3] = max(y,n) + y1;

   if( sa > ea ) ea += 360;
   neg = ((ea-sa) > 180);
   p = sa/90;
   mask = 1 << p;
   p = ea%360/90;
   mask |= 1 << p;
   p = ((sa+ea+neg*360)/2)%360/90;
   mask |= 1 << p;
   if ( (mask & 12) == 12 ) okno[3] = y1+r;
   if ( (mask & 6 ) == 6  ) okno[0] = x1-r;
   if ( (mask & 3 ) == 3  ) okno[1] = y1-r;
   if ( (mask & 9 ) == 9  ) okno[2] = x1+r;

   p = 6 - 4 * r;
   n = 2;
   test = p / 2;
   x = 0;
   y = r;

   if ( !neg )
   {
     okno[0] = max(okno[0],xg_view[0]);
     okno[1] = max(okno[1],xg_view[1]);
     okno[2] = min(okno[2],xg_view[2]);
     okno[3] = min(okno[3],xg_view[3]);

     for ( ; ; )
     {
       if ( INOKNO(x1+x, y1+y) ) x_putpix( x1+x, y1+y, xg_color );
       if ( INOKNO(x1+x, y1-y) ) x_putpix( x1+x, y1-y, xg_color );
       if ( INOKNO(x1-x, y1+y) ) x_putpix( x1-x, y1+y, xg_color );
       if ( INOKNO(x1-x, y1-y) ) x_putpix( x1-x, y1-y, xg_color );
       if ( r < 0 ) break;
       if ( test < 0 )
       {  x++ ; test += n; n += 4; }
       else
       {  y--; test += p; p += 4; if ( p > 0) r = -1; }
     }
     if ( INOKNO( x1+x, y1+y-1 )) x_putpix( x1+x, y1+y-1, xg_color );
     if ( INOKNO( x1-x, y1+y-1 )) x_putpix( x1-x, y1+y-1, xg_color );
   }
   else
   {
     for ( ; ; )
     {
       if ( INVIEW(x1+x, y1+y) && !INOKNO(x1+x, y1+y) )
	 x_putpix( x1+x, y1+y, xg_color );
       if ( INVIEW(x1+x, y1-y) && !INOKNO(x1+x, y1-y) )
	 x_putpix( x1+x, y1-y, xg_color );
       if ( INVIEW(x1-x, y1+y) && !INOKNO(x1-x, y1+y) )
	 x_putpix( x1-x, y1+y, xg_color );
       if ( INVIEW(x1-x, y1-y) && !INOKNO(x1-x, y1-y) )
	 x_putpix( x1-x, y1-y, xg_color );
       if ( r < 0 ) break;
       if ( test < 0 )
       {  x++ ; test += n; n += 4; }
       else
       {  y--; test += p; p += 4; if ( p > 0) r = -1; }
     }
     if ( INVIEW( x1+x, y1+y-1 ) && !INOKNO( x1+x, y1+y-1 ))
       x_putpix( x1+x, y1+y-1, xg_color );
     if ( INVIEW( x1-x, y1+y-1 ) && !INOKNO( x1-x, y1+y-1 ))
       x_putpix( x1-x, y1+y-1, xg_color );
   }
#if HI_COLOR
   xg_hipalmod = SaveMode;
#endif
}


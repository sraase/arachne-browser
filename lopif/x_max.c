/* Zjisti max. souradnice ve smeru x a y    */

#include <stdlib.h>
#include <stdio.h>
#include "x_lopif.h"

int x_maxx()
{
   /*-----------------------------------------------*/
#if HI_COLOR
   if(xg_intern == 10)
      return( 639 );
   else if(xg_intern == 11)
      return( 799 );
   else if(xg_intern == 12)
      return( 1023 );
   else if(xg_intern == 13)
      return( 1279 );
   else if(xg_intern == 14)
      return( 1599 );
#endif

   if(xg_intern == 2)
       return( 319 );
   else if(xg_intern==0 ||xg_intern==1 || xg_intern==4 ||
	   xg_intern==8 ||xg_intern==9)
       return( 639 );
   else if(xg_intern == 3 || xg_intern == 5)
       return( 799 );
   else if(xg_intern == 6 || xg_intern == 7)
       return( 1023 );
   else
       return( -1 );
}

int x_maxy()
{
#if HI_COLOR
   if(xg_intern == 10)
      return( 479 );
   else if(xg_intern == 11)
      return( 599 );
   else if(xg_intern == 12)
      return( 767 );
   else if(xg_intern == 13)
      return( 1023 );
   else if(xg_intern == 14)
      return( 1199 );
#endif

   if(xg_intern == 2 || xg_intern == 8)
       return( 199 );
   else if(xg_intern==0)
       return( 349 );
   else if(xg_intern==1 || xg_intern==4)
       return( 479 );
   else if(xg_intern == 3 || xg_intern == 5)
       return( 599 );
   else if(xg_intern == 6 || xg_intern == 7)
       return( 767 );
   else if(xg_intern == 9)
       return( 399 );
   else
       return( -1 );
}

#include <stdlib.h>
#include <stdio.h>

#include "x_lopif.h"

/* ------------>>>>>> Vycisti obrazovku */

void x_cleardev()
{
   int xk;

   xk = xg_fillc;
   x_setfill(0,0);
   x_bar(0,0,x_maxx(),x_maxy());
   x_setfill(0,xk);

}

/* ------------->>>>> Vycisti viewport */
void x_clearview()
{
   int xk;

   xk = xg_fillc;
   x_setfill(0,0);
   x_bar(xg_view[0],xg_view[1],xg_view[2],xg_view[3]);
   x_setfill(0,xk);

}

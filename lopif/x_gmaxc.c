#include <stdlib.h>
#include <stdio.h>

#include "x_lopif.h"

int x_getmaxcol(void)     /* Vraci max. cislo barvy */
{
   if(xg_256 == MM_256)
      return(255);
   else if(xg_256 == MM_16)
      return( 15);
   else if(xg_256 == MM_2)
      return(  1);
   else
      return(  0);
}


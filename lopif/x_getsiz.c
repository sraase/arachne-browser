/* Zjisti velikost bufru pro x_getimg */

#include <stdlib.h>
#include <stdio.h>

#include "x_lopif.h"

unsigned int x_getsize(int x1, int y1, int x2, int y2)
{
   long  l4 = 0xFFFF;

   if(xg_256 == MM_256)       /* 256 mod */
     {
      l4 = 4 + (long) (x2-x1+1) * (y2-y1+1);
     }
#if HI_COLOR
   else if(xg_256 == MM_Hic)  // Hi-color
     {
      l4 = 4 + (long)(x2-x1+1)*2 * (y2-y1+1);
     }
#endif
   else if(xg_256 == MM_16)  /* 16 mod */
     {
      l4 = (long)((x2-x1)/8 + 1)*xg_col_plan * (unsigned)(y2-y1+1) + 6;
     }
   else if(xg_256 == MM_2)
     { l4 = (long)((x2-x1)/8 + 1) * (unsigned)(y2-y1+1) + 6;
     }

   if(l4 >= 0xFFFF)
	return(0xFFFF);
   else
	return( (unsigned int) l4);
}

//----------- Nastaveni poctu bit rovin pro GETIMG,PUTIMG ----

void x_set_planes(int planes)
{
   if(planes < 1 || planes > 4)
     xg_col_plan = 4;
   else
     xg_col_plan = planes;
}

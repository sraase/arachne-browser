/* Ulozi cast videoram urcene oknem do bufru */

#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <bios.h>

#include "x_lopif.h"

void x_getimg(int x1, int y1, int x2, int y2, char *buf)
{
   int   i;

   union { int  d2[2];
	   char d1[4];
	 } u;

   /*-----------------------------------------------*/

   if(xg_256 == MM_256)  /* 256 mod */
     {
       u.d2[0] = x2 - x1 + 1;
       u.d2[1] = y2 - y1 + 1;
       for(i=0; i<4; i++) buf[i] = u.d1[i];  /* Velikost */
       rea_w256(&buf[4],x1,y1,u.d2[0],u.d2[1]);
     }
#if HI_COLOR
   else if(xg_256 == MM_Hic)  /* Hi-col mod */
     {
       u.d2[0] = x2 - x1 + 1;
       u.d2[1] = y2 - y1 + 1;
       for(i=0; i<4; i++) buf[i] = u.d1[i];  /* Velikost */
       xh_read(&buf[4], x1, y1, u.d2[0], u.d2[1]);
     }
#endif
   else if(xg_256 == MM_16)               /* 16 mod */
     { x_gimg16(x1,y1,x2,y2,buf,xg_col_plan);
     }
   else if(xg_256 == MM_2)
     {
       u.d2[0] = x2 - x1 + 1;
       u.d2[1] = y2 - y1 + 1;
       for(i=0; i<4; i++) buf[i] = u.d1[i];  /* Velikost */
       rea_bincga(&buf[4], x1, y1, u.d2[0], u.d2[1]);
     }
}

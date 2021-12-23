/* Zobrazi cast videoram ulozene fci x_getimg */

#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <bios.h>

#include "x_lopif.h"

void x_putimg(int xz, int yz, char *buf, int op)
{
   int   i,hwop,video_XMS;

   union { int  d2[2];
	   char d1[4];
	 } u;

   /*-----------------------------------------------*/
   video_XMS = xg_video_XMS;  // only real videoram !!!
   xg_video_XMS = 0;

   if(xg_256 == MM_256)       /* 256 mod */
     {
       for(i=0; i<4; i++) u.d1[i] = buf[i] ;  /* Velikost */
       i=wrt_video(&buf[4],xz,yz,u.d2[0],u.d2[1],1);
     }
#if HI_COLOR
   else if(xg_256 == MM_Hic)       /* Hic mod */
     {
       for(i=0; i<4; i++) u.d1[i] = buf[i] ;  /* Velikost */
       i=xh_write(&buf[4], xz, yz, u.d2[0], u.d2[1]);
     }
#endif
   else if(xg_256 == MM_16)               /* 16 mod */
     { switch(op)
       { case 0: hwop = 0; break;   // Borland nesouhlasi s HW !!!
	 case 1: hwop = 3; break;
	 case 2: hwop = 2; break;
	 case 3: hwop = 1; break;
	 case 4: hwop = 4; break;
	 default:hwop = op;
       }
       x_pimg16(xz,yz,buf,hwop,xg_col_plan);
       if(xg_wrt != 0) x_wrtmode(1);
     }
   else if(xg_256 == MM_2)          // 2 colors
    { for(i=0; i<4; i++) u.d1[i] = buf[i];
      switch(op)
       { case 0: hwop = 0x00; break;  // copy
	 case 1: hwop = 0x01; break;  // xor
	 case 3: hwop = 0x02; break;  // and
	 case 2: hwop = 0x04; break;  // or
	 case 4: hwop = 0x08; break;  // not
	 default:hwop = op;
       }
      wrt_bincga(&buf[4], xz, yz, u.d2[0], u.d2[1], hwop, 0);  // op ?
    }

   xg_video_XMS = video_XMS;
}


#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <mem.h>
#include "x_lopif.h"

/*----------- Nacteni prave nastavene palety --------*/
void x_getpalette(char *pal)
{
   union REGS in,out;
   struct SREGS segreg;
   int    i,egacol;

   if(xg_256 == MM_2)            /* 2 barvy  */
     { pal[0]=pal[1]=pal[2]=0;
       pal[3]=pal[4]=pal[5]=63;
       goto End_rea;
     }
#if HI_COLOR
   else if(xg_256 == MM_Hic)
     {
       memcpy(pal, xg_hipal, 768);
       goto End_rea;
     }
#endif
   else if(xg_mod == 0x10)    /* EGA - z Bios oblasti */
     {
      for(i=0; i<16; i++)           /* Prevod EGA na VGA paletu */
	 { egacol = xg_egapal[i];
	   pal_ega_vga(egacol,pal+3*i);
	 }
      goto End_rea;
     }
   else if(xg_256 == MM_16)         /* 16 VGA  */
     { in.x.cx = 16;                /* kolik registru */
     }
   else if(xg_256==MM_256 || xg_256==MM_Hic)        /* 256 VGA */
     { in.x.cx = 256;               /* kolik registru */
     }
   else
     { goto End_rea;
     }

   in.h.ah = 0x10;
   in.h.al = 0x17;
   in.x.bx = 0;
   in.x.dx = FP_OFF(pal);
   segreg.es = FP_SEG(pal);
   int86x(0x10,&in,&out,&segreg);

   End_rea:;
}


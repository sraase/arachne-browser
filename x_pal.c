/* Nastaveni palety pro 16 a 256 barev */

#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <mem.h>
#include "x_lopif.h"

/*-------- Nastaveni cele palety (EGA/VGA) -----------*/
void x_palett(int npal, char *palette)
{
  union REGS in,out;
  struct SREGS segreg;
  int    i,ega_col,jak;

  if(xg_mod == 7) goto End;      /* Hercules   */

#if HI_COLOR
  if(xg_256 == MM_Hic)           // pouze kopie do xg_hipal
  {
     memcpy(xg_hipal, palette, 3*npal);
     for(i=0; i<npal; i++)
       { xg_hival[i] = xh_RgbHiPal(xg_hipal[3*i], xg_hipal[3*i+1],
				   xg_hipal[3*i+2]);
       }
     return;
  }
#endif

  if(xg_mod == 0x10)             /* EGA paleta */
  {
   if((xg_flag & 0x01) != 0)
     jak = 1;    /* Spec EGA pal */
   else
     jak = 0;

   for(i=0; i<16; i++)
    {
     pal_vga_ega(palette+3*i, &ega_col, jak);
     xg_egapal[i] = ega_col;
    }
   xg_egapal[16]=0;     // overscan reg.

   in.h.ah = 0x10;
   in.h.al = 0x02;
   in.x.dx = FP_OFF(xg_egapal);
   segreg.es = FP_SEG(xg_egapal);
   int86x(0x10,&in,&out,&segreg);
  }
  else                           /* VGA paleta */
  {
  if(npal <= 16) npal = 17;
  in.h.ah = 0x10;
  in.h.al = 0x12;
  in.x.bx = 0;                    /* od ktereho se zacne */
  in.x.cx = npal;                 /* kolik registru */
  in.x.dx = FP_OFF(palette);
  segreg.es = FP_SEG(palette);
  int86x(0x10,&in,&out,&segreg);
  }

  End:;

}


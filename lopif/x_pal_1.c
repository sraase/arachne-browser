#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include "x_lopif.h"

/*-------- Nastaveni palety jedne barvy (EGA/VGA) ---------*/

// Psat primo do HW registru pri zpetnem snimkovem behu (VGA)
void x_pal_1hw(int n_pal, char *pal_1)
{
  unsigned int port;
  unsigned char Val,Inx;

  Inx  = n_pal;
  port = 0x03DA;  // Inp.status VGA
  Read_inp:
  Val = inp( port );
  if(Val&0x08)    // Bit 3 je 1->zpetny beh
  {
    port = 0x03C8;   // DAC write index
    outp( port, Inx);

    port = 0x03C9;   // DAC data
    outp( port, pal_1[0]);
    outp( port, pal_1[1]);
    outp( port, pal_1[2]);
  }
  else
  { goto Read_inp;
  }

}

void x_pal_1(int n_pal, char *pal_1)
{
  union REGS in;
  int   ega_col,jak;
  int   HwPal = 1;

  in.h.ah = 0x10;

#if HI_COLOR
  if(xg_256 == MM_Hic)
  {  int i;
     i = 3*n_pal;
     xg_hipal[i  ] = pal_1[0];
     xg_hipal[i+1] = pal_1[1];
     xg_hipal[i+2] = pal_1[2];
     xg_hival[n_pal] = xh_RgbHiPal(xg_hipal[i], xg_hipal[i+1],
				   xg_hipal[i+2]);
    return;
  }
#endif

  if(xg_mod == 0x10)   /* EGA paleta */
  {
  if((xg_flag & 0x01) != 0)
    jak = 1;    /* Spec EGA pal */
  else
    jak = 0;

  xg_egapal[n_pal]=ega_col;

  pal_vga_ega(pal_1, &ega_col, jak);
  in.h.al = 0;
  in.h.bh = ega_col;
  in.h.bl = n_pal;
  int86(0x10,&in,&in);
  }
  else                 /* VGA paleta */
  {
   if( HwPal )
   { x_pal_1hw(n_pal, pal_1);
   }
   else
   {
   in.h.al = 0x10;
   in.x.bx = n_pal;     /* index barvy */
   in.h.dh = pal_1[0];  /* R G B       */
   in.h.ch = pal_1[1];
   in.h.cl = pal_1[2];
   int86(0x10,&in,&in);
   }
  }

}


/* Nastaveni aktualni barvy pro dalsi graficke funkce */

#include <stdlib.h>
#include <stdio.h>

#include "x_lopif.h"

void x_setcolor(int color)
{
#if HI_COLOR
  int i;
  unsigned char R,G,B;
  if(xg_256 == MM_Hic)
  { if(xg_hipalmod == 0)    // barvu dle palety
    {
      i = 3*color;
      R = xg_hipal[i]; G = xg_hipal[i+1]; B = xg_hipal[i+2];
      if(xg_hi16 == 1)
       xg_color =  RGBHI16(R,G,B);
      else
       xg_color =  RGBHI15(R,G,B);
    }
    else                    // primo RGB 15|16bit
    { xg_color = color;
    }
  }
  else
#endif
   xg_color = color;
}


void x_setfill(int pattern, int color)
{
#if HI_COLOR
  int i;
  unsigned char R,G,B;
  if(xg_256 == MM_Hic)
  { if(xg_hipalmod == 0)    // barvu dle palety
    {
      i = 3*color;
      R = xg_hipal[i]; G = xg_hipal[i+1]; B = xg_hipal[i+2];
      if(xg_hi16 == 1)
       xg_fillc =  RGBHI16(R,G,B);
      else
       xg_fillc =  RGBHI15(R,G,B);
    }
    else                    // primo RGB 15|16bit
    { xg_fillc = color;
    }
  }
  else
#endif
    xg_fillc = color;

  xg_flag  = xg_flag & 0xFFFD;      /* Kresleni plnou */
}


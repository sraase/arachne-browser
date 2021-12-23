#include "x_lopif.h"

void x_setpattern(char *patt, int color)
{
  int i;

#if HI_COLOR
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

  xg_flag  = xg_flag | 0x0002;      /* Kresleni vzorem */
  for(i=0; i<8; i++) xg_upatt[i] = patt[i];

}

void x_setcircf(int flag)
{
  if(flag == 0)
    xg_flag = xg_flag & 0xFFFB;
  else
    xg_flag = xg_flag | 0x0004;
}


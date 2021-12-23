#include "x_lopif.h"

//------ PUTPIX pro video i XMS dle nastaveni

void x_putpix(int x, int y, int col)
{
#ifdef VIRT_SCR
 if(xg_video_XMS == 0)
 {
#endif
#if HI_COLOR
  if(xg_256 == MM_Hic)
  {
   if(xg_hipalmod == 0)    // barvu dle palety
    xh_putpix(x,y,xg_hival[col]);
   else                    // primo RGB 15|16bit
    xh_putpix(x, y, col);
  }
  else
#endif
  x_putpix_v(x, y, col); //zakladni varianta
#ifdef VIRT_SCR
 }
 else
  z_xmsputpix(x, y, col);
#endif
}


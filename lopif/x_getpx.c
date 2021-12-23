#include "x_lopif.h"
//#undef  VIRT_SCR
//------ GETPIX pro video i XMS dle nastaveni
int x_getpix(int x, int y)
{
     if(xg_video_XMS == 0)
#if HI_COLOR
       if(xg_256 == MM_Hic)
	 return( xh_getpix(x,y) );
       else
#endif
	return( x_getpix_v(x, y) );
     else
     {
#ifdef VIRT_SCR
      return( z_xmsgetpix(x, y) );
#else
      return( 0 );
#endif
     }
}

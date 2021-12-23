/*--------- Kresleni vyplneneho obdelnika vzorem --------*/
#include <stdlib.h>
#include <stdio.h>

#include "x_lopif.h"

void x_bar_patt(int x1, int y1, int dx, int dy)
{
   int i,j,LxMask;
   int dx8,dy8,dxz,dyz,kx,ky;

   dx8 = dx/8;          /* Nasobky 8 */
   dy8 = dy/8;
   dxz = dx & 0x0007;   /* Zbytek po deleni 8 */
   dyz = dy & 0x0007;

   kx = x1;
   ky = y1;

   if(xg_256 == MM_2)       // Binar
   { if((xg_fillc&1) == 0)  // pozadi barvou 0, popredi 1
       if(xg_chrmod == 0)
	 LxMask = 0;
       else
	 LxMask = 0x08|0x06;  // neg+and+or
     else
       if(xg_chrmod == 0)
	 LxMask = 0x08;      // Negace, pozadi 1, popredi 0
       else
	 LxMask = 0x08|0x02;  // neg+and
   }

   for(i=0; i<dy8; i++)         /* Matice vzoru */
     {
       for(j=0; j<dx8; j++)     /* cele vzory 8x8 */
	 {
	 if(xg_256 == MM_256 || xg_256 == MM_16)
	   wrt_chr(xg_upatt,kx,ky,8,8,xg_fillc,0);
#if HI_COLOR
	 else if(xg_256 == MM_Hic)
	   xh_wrtchr(xg_upatt,kx,ky,8,8,xg_fillc,0);
#endif
	 else
	   wrt_bincga(xg_upatt,kx,ky,8,8,LxMask,0);

	 kx += 8;
	 }
       if(dxz != 0)             /* Necely v x dxz x 8 */
	 {
	 if(xg_256 == MM_256 || xg_256 == MM_16)
	   wrt_chr(xg_upatt,kx,ky,dxz,8,xg_fillc,0);
#if HI_COLOR
	 else if(xg_256 == MM_Hic)
	   xh_wrtchr(xg_upatt,kx,ky,dxz,8,xg_fillc,0);
#endif
	 else
	   wrt_bincga(xg_upatt,kx,ky,dxz,8,LxMask,0);
	 }
       kx = x1;
       ky += 8;
     }

   if(dyz != 0)                 /* Necely v y 8 x dyz */
     {
       for(j=0; j<dx8; j++)
	 {
	 if(xg_256 == MM_256 || xg_256 == MM_16)
	   wrt_chr(xg_upatt,kx,ky,8,dyz,xg_fillc,0);
#if HI_COLOR
	 else if(xg_256 == MM_Hic)
	   xh_wrtchr(xg_upatt,kx,ky,8,dyz,xg_fillc,0);
#endif
	 else
	   wrt_bincga(xg_upatt,kx,ky,8,dyz,LxMask,0);

	 kx += 8;
	 }
       if(dxz != 0)             /* Necely dxz x dyz  */
	 {
	 if(xg_256 == MM_256 || xg_256 == MM_16)
	  wrt_chr(xg_upatt,kx,ky,dxz,dyz,xg_fillc,0);
#if HI_COLOR
	 else if(xg_256 == MM_Hic)
	  xh_wrtchr(xg_upatt,kx,ky,dxz,dyz,xg_fillc,0);
#endif
	 else
	  wrt_bincga(xg_upatt,kx,ky,dxz,dyz,LxMask,0);
	 }
     }
}
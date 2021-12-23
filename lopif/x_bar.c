/* Nakresli vyplneny obdelnik (barvou ci vzorem) */

#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <bios.h>
#include <malloc.h>

#include "x_lopif.h"

void x_bar(int xz, int yz, int xk, int yk)
{
   extern int xv_bits;

   int  x1,y1,x2,y2,i,dyy;
   unsigned int mask;
   unsigned char masb;
   int  j,k,zb1,zb2,dxx,cosi,col,LxMask;
   unsigned char buf_t[2060];    // ?? Alloc ?
#if HI_COLOR
   unsigned int *buf_i;
   buf_i = (unsigned int *) buf_t;
#endif

   if(xz > xk)
     { i  = xz;
       xz = xk;
       xk = i;
     }
   if(yz > yk)
     { i  = yz;
       yz = yk;
       yk = i;
     }

   cosi = xg_wrt;
   if(xg_wrt != 0)
     { x_wrtmode(0);
     }

       if(xg_notview == 0)
       {
       x1 = xz + xg_view[0];  /* Posun do viewportu */
       y1 = yz + xg_view[1];
       x2 = xk + xg_view[0];
       y2 = yk + xg_view[1];
       }
       else
       {
       x1 = xz;
       y1 = yz;
       x2 = xk;
       y2 = yk;
       }

       if(xg_clip != 0)        /* Oriznuti */
       {
       if(y2 < xg_view[1]) goto Nic_x;            // YPS
       if(y1 < xg_view[1]) y1 = xg_view[1];
       if(y1 > xg_view[3]) goto Nic_x;
       if(y2 > xg_view[3]) y2 = xg_view[3];
       if(x2 < xg_view[0]) goto Nic_x;           // XSY
       if(x1 < xg_view[0]) x1 = xg_view[0];
       if(x1 > xg_view[2]) goto Nic_x;
       if(x2 > xg_view[2]) x2 = xg_view[2];
       }

       if(xg_video_XMS != 0)       // ---- XMS videoram
       { if(xv_bits == 3)
	  col = xg_bincol;
	 else
	  col = xg_fillc;
#ifdef VIRT_SCR
	 z_xmsbar(x1, y1, x2, y2, col);
#endif
       }
       else                        // ---- obrazovka
       {
       if(xg_flag & 0x0002)        /* Vyplnit vzorem */
	 { if(xg_256 == MM_256 || xg_256 == MM_Hic)
	     {                     // 256
	       dxx = x2 - x1 + 1;
	       zb1 = x1&7;
	       for(i=y1; i<=y2; i++)
		{ masb = xg_upatt[(i&7)];
		  masb = masb<<zb1;
		  k=0;
		  zb2 = 7-zb1;
		  Jeste:
		  for(j=0; j<=zb2; j++)
		   { if((masb & 0x80) == 0)
#if HI_COLOR
		       if(xg_256 == MM_Hic)
			buf_i[k] = 0;
		       else
#endif
			buf_t[k] = 0;
		     else
#if HI_COLOR
		       if(xg_256 == MM_Hic)
			buf_i[k] = xg_fillc;
		       else
#endif
			buf_t[k] = xg_fillc;
		     masb = masb<<1;
		     k++;
		   }
		  if(k<=dxx)
		   { masb = xg_upatt[(i&7)];
		     zb2 = 7;
		     goto Jeste;
		   }
#if HI_COLOR
		  if(xg_256 == MM_Hic)
		     xh_write(buf_t, x1, i, dxx, 1);
		  else
#endif
		     wrt_video(buf_t,x1,i,dxx,1,1);
		}
	     }
	   else if(xg_256 == MM_16)
	     { mask = 0x00FF;
	       x_b16(x1,y1,x2,y2,0,mask);
	       for(i=y1; i<=y2; i++)
		{ mask = xg_upatt[(i&7)];
		  x_b16(x1,i,x2,i,xg_fillc,mask);
		}
	     }
	   else if(xg_256 == MM_2)      // 2 colors
	     { if((xg_fillc & 1) == 0)
		LxMask = 0;     // pozadi suda
	       else
		LxMask = 0x08;  // pozadi licha -> negace
	       for(i=y1; i<=y2; i++)
		{ buf_t[0] = xg_upatt[(i&7)];
		  wrt_bincga(buf_t, x1, i, x2-x1+1, 1, LxMask,1);
		}
	     }
	 }
       else  // ------------------- plny bar ---------------------
	 { if(xg_256 == MM_256)        /* 256 plna */
	     {  if(xg_wrt == 0)
		  {x_bar256(xg_fillc, x1, y1, x2-x1+1, y2-y1+1);
		  }
		 else             /* XOR */
		  { dxx = x2-x1+1;
		    dyy = y2-y1+1;
		    for(i=0; i<dyy; i++)
		     { rea_w256(buf_t,x1,y1+i,dxx,1);     // Vodorovne
		       for(j=0; j<dxx; j++) buf_t[j] ^=  xg_fillc;
		       wrt_video(buf_t,x1,y1+i,dxx,1,1);
		     }
		  }
	     }
#if HI_COLOR
	   else if(xg_256 == MM_Hic)        /* Hi plna */
	     {  if(xg_wrt == 0)
		  {xh_barx(xg_fillc, x1, y1, x2-x1+1, y2-y1+1);
		  }
		 else                 /* XOR */
		  { dxx = x2-x1+1;
		    dyy = y2-y1+1;
		    for(i=0; i<dyy; i++)
		     { xh_read(buf_t, x1, y1+i, dxx, 1);     // Vodorovne
		       for(j=0; j<dxx; j++) buf_i[j] ^=  xg_fillc;
		       xh_write(buf_t, x1, y1+i,dxx, 1);
		     }
		  }
	     }
#endif
	   else if(xg_256 == MM_16)  /*  16 barevna plna */
	     { if(xg_wrt == 0)
		 mask = 0x00FF;   // Prepis
	       else
		 mask = 0xFFFF;   // XOR
	       x_b16(x1,y1,x2,y2,xg_fillc,mask);
	     }
	   else                      /* 2 barevna plna */
	     { if((xg_fillc & 1) == 0)
		buf_t[0]=0x00;     // suda -> 0
	       else
		buf_t[0]=0xFF;     // licha -> 1
	       wrt_bincga(buf_t, x1, y1, x2-x1+1, y2-y1+1, 0 ,1);
	     }
	 }
       }

    Nic_x:
    if(cosi != 0)
     { x_wrtmode(1);
     }
    End_err:;
}


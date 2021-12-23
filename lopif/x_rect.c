/* Nakresli nevyplneny obdelnik */

#include <stdlib.h>
#include <stdio.h>

#include "x_lopif.h"

#define  XORY(len)    for(i=0; i<len; i++)  bufo[i] ^=  xor_col
#define  XORY2(len)   for(i=0; i<len; i++) i_buf[i] ^=  xor_col2

void x_rect(int xz, int yz, int xk, int yk)
{
   extern int xv_bits;

   int x1,y1,x2,y2,i,mask,color,LxMask;
   int xsv1,xsv2,ysv1,ysv2,dxx,dyy;
   unsigned char xor_col,bufo[2060];
#if HI_COLOR
   unsigned int  *i_buf, xor_col2;
   i_buf = (unsigned int *)bufo;
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

      xsv1 = xsv2 = ysv1 = ysv2 = 1;

       x1 = xz + xg_view[0];  /* Posun do viewportu */
       y1 = yz + xg_view[1];
       x2 = xk + xg_view[0];
       y2 = yk + xg_view[1];

       if(xg_video_XMS != 0)   //----------------- Vystup do XMS
	 {
    dyy = y2-y1-1;
	 if(xv_bits == 3)
	  color = xg_bincol;
	 else
	  color = xg_color;
#ifdef VIRT_SCR
	 if(ysv1 >0) z_xmsbar(x1,y1,x2,y1,color);     // Vodorovne
	 if(ysv2 >0) z_xmsbar(x1,y2,x2,y2,color);
	 if(dyy <= 0) goto Endx;
	 if(xsv1 >0) z_xmsbar(x1,y1+1,x1,y2-1,color); // Svisle
	 if(xsv2 >0) z_xmsbar(x2,y1+1,x2,y2-1,color);
#endif
	 goto Endx;
	 }

       if(xg_clip != 0)        /* Oriznuti */
	 {
	   if(x1 < xg_view[0]) xsv1 = 0;
	   x1 = max(x1,xg_view[0]);
	   if(x1 > xg_view[2]) goto Endx;

	   if(x2 > xg_view[2]) xsv2 = 0;
	   x2 = min(x2,xg_view[2]);
	   if(x2 < xg_view[0]) goto Endx;

	   if(y1 < xg_view[1]) ysv1 = 0;
	   y1 = max(y1,xg_view[1]);
	   if(y1 > xg_view[3]) goto Endx;

	   if(y2 > xg_view[3]) ysv2 = 0;
	   y2 = min(y2,xg_view[3]);
	   if(y2 < xg_view[1]) goto Endx;
	 }

       dyy = y2-y1-1;

       //------- Vystup na obrazovku -------------
       if(xg_256 == MM_256)
	 { // x_rect_s(xg_color,x1,y1,x2-x1+1,y2-y1+1,xg_wrt);
	   xor_col=xg_color;
	   dxx = x2-x1+1;
	   if(ysv1 >0) { if(xg_wrt != 0)
			 { rea_w256(bufo,x1,y1,dxx,1);     // Vodorovne
			   XORY(dxx);
			   wrt_video(bufo,x1,y1,dxx,1,1);
			 }
			 else
			 { x_bar256(xg_color,x1,y1,dxx,1);
			 }
		       }
	   if(ysv2 >0) { if(xg_wrt != 0)
			 { rea_w256(bufo,x1,y2,dxx,1);
			   XORY(dxx);
			   wrt_video(bufo,x1,y2,dxx,1,1);
			 }
			 else
			 { x_bar256(xg_color,x1,y2,dxx,1);
			 }
		       }

	   if(dyy <= 0) goto Endx;

	   if(xsv1 >0) { if(xg_wrt != 0)
			 { rea_w256(bufo,x1,y1+1,1,dyy); // Svisle
			   XORY(dyy);
			   wrt_video(bufo,x1,y1+1,1,dyy,1);
			 }
			 else
			 { x_bar256(xg_color,x1,y1+1,1,dyy);
			 }
		       }
	   if(xsv2 >0) { if(xg_wrt != 0)
			 { rea_w256(bufo,x2,y1+1,1,dyy);
			   XORY(dyy);
			   wrt_video(bufo,x2,y1+1,1,dyy,1);
			 }
			 else
			 { x_bar256(xg_color,x2,y1+1,1,dyy);
			 }
		       }
	 }
#if HI_COLOR
       else if(xg_256 == MM_Hic)
	{
	   xor_col2=xg_color;
	   dxx = x2-x1+1;
	   if(ysv1 >0) { if(xg_wrt != 0)
			 { xh_read(bufo,x1,y1,dxx,1);     // Vodorovne
			   XORY2(dxx);
			   xh_write(bufo,x1,y1,dxx,1);
			 }
			 else
			 { xh_barx(xg_color,x1,y1,dxx,1);
			 }
		       }
	   if(ysv2 >0) { if(xg_wrt != 0)
			 { xh_read(bufo,x1,y2,dxx,1);
			   XORY2(dxx);
			   xh_write(bufo,x1,y2,dxx,1);
			 }
			 else
			 { xh_barx(xg_color,x1,y2,dxx,1);
			 }
		       }
	   if(dyy <= 0) goto Endx;

	   if(xsv1 >0) { if(xg_wrt != 0)
			 { xh_read(bufo,x1,y1+1,1,dyy); // Svisle
			   XORY2(dyy);
			   xh_write(bufo,x1,y1+1,1,dyy);
			 }
			 else
			 { xh_barx(xg_color,x1,y1+1,1,dyy);
			 }
		       }
	   if(xsv2 >0) { if(xg_wrt != 0)
			 { xh_read(bufo,x2,y1+1,1,dyy);
			   XORY2(dyy);
			   xh_write(bufo,x2,y1+1,1,dyy);
			 }
			 else
			 { xh_barx(xg_color,x2,y1+1,1,dyy);
			 }
		       }
	}
#endif
       else if(xg_256 == MM_16)
	 { mask = 0xFFFF;    // Pomalejsi verze
	   if(ysv1 >0) x_b16(x1,y1,x2,y1,xg_color,mask);     // Vodorovne
	   if(ysv2 >0) x_b16(x1,y2,x2,y2,xg_color,mask);
	   if(dyy <= 0) goto Endx;
	   if(xsv1 >0) x_b16(x1,y1+1,x1,y2-1,xg_color,mask); // Svisle
	   if(xsv2 >0) x_b16(x2,y1+1,x2,y2-1,xg_color,mask);
	 }
       else if(xg_256 == MM_2)
	 {
	   if((xg_color & 1)!=0)
	    bufo[0] = 0xFF;   // licha col
	   else
	    bufo[0] = 0x00;
	   if(xg_wrt != 0)
	   { LxMask = 0x01;    // xor
	     bufo[0] = 0xFF;
	   }
	   else
	    LxMask = 0;        //copy
	   if(ysv1 >0) wrt_bincga(bufo,x1,y1,x2-x1+1,1,LxMask,1);     // Vodorovne
	   if(ysv2 >0) wrt_bincga(bufo,x1,y2,x2-x1+1,1,LxMask,1);
	   if(dyy <= 0) goto Endx;
	   if(xsv1 >0) wrt_bincga(bufo,x1,y1+1,1,y2-y1-1,LxMask,1); // Svisle
	   if(xsv2 >0) wrt_bincga(bufo,x2,y1+1,1,y2-y1-1,LxMask,1);
	 }

     Endx:;
}


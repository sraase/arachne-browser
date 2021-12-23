//------ Write buffer to videoram ----
#include <stdlib.h>
#include <alloc.h>
#include "x_lopif.h"
//#undef VIRT_VIDEO

int wrt_video(char *Buf, int xpix, int ypix, int ncol, int nrow, int npix)
{
// npix > 0 : 1,8:  pocet pixlu v bytu v Buf
// npix < 0 :  -2:  2B na pixel v Buf, -3: 3B na pixel v Buf (BGR)
//                  Pouze pro Hi-Color modes !!!
   int ire;

   if(xg_video_XMS == 0)
   {
   if(xg_256 == MM_16 || xg_256 == MM_256)
   {
     ire = wrt_video1(Buf, xpix, ypix, ncol, abs(nrow), npix);
   }
#if HI_COLOR
   else if(xg_256 == MM_Hic)
   {
     int      LenLin;
     unsigned char *obuf=NULL;
     long     LenAll;

     if(npix == 1 || npix == -3)     // Buf: Byty (paletovy obraz) do Hi-col
     {                               // Buf: Rgb obraz do Hi-col
       LenAll = (long)ncol * abs(nrow) * 2;   // po 2B pixel
       obuf = farmalloc(LenAll);
       if(obuf == NULL) return( 0 );

       if(npix == 1)
	{ LenLin = ncol;
	  if(xg_round != 0)
	    { if(LenLin & 3) LenLin = (LenLin/4 * 4) + 4;
	    }
	  xh_ByteToHi(Buf, obuf, ncol, nrow, LenLin);
	}
       else
	{ LenLin = 3*ncol;
	  if(xg_round != 0)
	    { if(LenLin & 3) LenLin = (LenLin/4 * 4) + 4;
	    }
	  xh_RgbToHi(Buf, obuf, ncol, nrow, LenLin);
	}
       nrow = abs(nrow);
       ire = xh_write(obuf, xpix, ypix, ncol, nrow);
       farfree(obuf);
     }
     else if(npix == -2)     // Buf: Hi-col do Hi-col (1:1)
     {
       ire = xh_write(Buf, xpix, ypix, ncol, nrow);
     }
   }
#endif
   else if(xg_256 == MM_2)
   {
     wrt_bincga(Buf, xpix, ypix, ncol, abs(nrow), 0, 0);
     ire = 1;
   }
   }
   else  // virt virtual screen
   {
#ifdef VIRT_SCR
    ire = xv_wrt_virt(Buf, xpix, ypix, ncol, abs(nrow), npix);
#endif
   }
   return( ire );
}


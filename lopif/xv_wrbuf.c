// equivalent fce wrt_video(...) for virt. screen 1|8 bit/pixel

#include <stdlib.h>
#include <alloc.h>
#include <io.h>
#include <mem.h>
#include <string.h>
#include "x_lopif.h"

#ifdef VIRT_SCR
typedef struct
  {
  unsigned long length;         /* velikost prenasene pameti      */
  unsigned int sourceH;         /* handle pro zdroj (0=konvencni  */
  unsigned long sourceOff;      /* offset zdroje pameti           */
  unsigned int destH;           /* handle pro terc (0=konvencni   */
  unsigned long destOff;        /* offset terce pameti            */
  } XMOVE;

int h_xmove(XMOVE *p);
int long ptr2long(char *p);

int xv_chr_mem(char *fnt_chr,          // Zacatek znaku ve fontu
		int  kx,                // Posun ve smeru X
		int  dx,                // Delka radku
		int  xfnt, int yfnt,    // Velikost fontu v pixlech
		int  color,int fillc,   // Barva znaku a pozadi
		char *subobr,           // Obraz se znaky textu (OUT!)
		int  bin);              // bin=0/1 subobr bytovy/binarni

int xv_wrt_virt(char *buf, int xp, int yp, int ncol, int nrow, int pix)
{
   int           Moff,Moff2,Byt1,Byt2,Len,i,lenb;
   int           ist,dx,dy,mode,LenOff;
   unsigned char *subobr, *hsubobr, *bufa;
   long          size,adrz,adr;
   XMOVE         xmove;

   //---------------------------------------------------------
   if(xv_vv[xv_act].xv_bits == 0)  // (8bit/pixel)
    { if(pix != 1) return( 10 );
    }
   else if(xv_vv[xv_act].xv_bits == 3)  // Binary
    { if(pix != 8) return( 10 );
      Moff = xp % 8;                // Beg offset pro prvni znak
      Moff2= (xp + ncol) % 8;
      Byt1 = xp / 8;                // Prvni byte v XMS
      Byt2 = (xp + ncol - 1) / 8;   // Posledni byte
      Len  = Byt2-Byt1+1;
    }
#if HI_COLOR
   else if(xv_vv[xv_act].xv_bits == -1) // HiCol
   { if(pix != 16) return( 10 );
   }
#endif
   else
   { return( 12 );
   }

   if(xv_vv[xv_act].xv_bits == 3 && (Moff != 0 || Moff2 != 0) )
   {
   // binary & x1|(x2+1) % 8 != 0  -> read buf from virt src
   size = (long)Len * (long)nrow + 4;
   if(size > 65500L) return( 2 );

   hsubobr = farmalloc( size );
   if(hsubobr == NULL) return( 4 );
   subobr = hsubobr + 4;     // data

   ist = xv_int_rea(Byt1, yp, Len*8, nrow, hsubobr);
   if(ist != 1) return( ist );

   mode = xg_chrmod;
   xg_chrmod = 0;    // rewrite
   xv_chr_mem(buf, Moff, Len*8,ncol,nrow,1,0,subobr,3);
   xg_chrmod = mode;

   ist = xv_int_wrt(Byt1, yp, hsubobr);
   if(ist != 1) return( ist );

   farfree(hsubobr);
   }
   else  // 256 + HiCol
   {
   dx = ncol;
   dy = nrow;
   if(xv_vv[xv_act].xv_bits >= 0)
    LenOff = xp>>xv_vv[xv_act].xv_bits;
   else
    LenOff = xp * 2;
   adrz = (long)yp * xv_vv[xv_act].xv_len_col + (long)(LenOff) +
		xv_vv[xv_act].xv_zmap;
   if(xv_vv[xv_act].xv_bits >= 0)
    lenb = dx>>xv_vv[xv_act].xv_bits;   // delka radku v bytech v XMS
   else
    lenb = dx * 2; 
   adr  = adrz;
   bufa = buf;

   if(xv_vv[xv_act].xv_XMS != -1)
     {xmove.length  = (long)lenb;            // Prenos z DOS do XMS
      xmove.sourceH = 0;
      xmove.destH   = xv_vv[xv_act].xv_XMS;
      xmove.destOff = adrz;
     }

   for(i=0; i<dy; i++)
     {
       if(xv_vv[xv_act].xv_XMS == -1)                      // Disk
       {
       lseek(xv_vv[xv_act].xv_file,adr,SEEK_SET);
       ist = write(xv_vv[xv_act].xv_file,bufa,lenb);
       if(ist < lenb) goto Err_wrt;
       adr  += xv_vv[xv_act].xv_len_col;
       }
       else                                  // XMS
       {
       xmove.sourceOff = ptr2long(bufa);     // Nova adr v bufru
       ist = h_xmove(&xmove);
       if(!ist) goto Err_wrt;
       xmove.destOff += xv_vv[xv_act].xv_len_col; // Nova adresa XMS
       }
       bufa += lenb;                              // Nova adr v GETIMG
     }
    }
    return( 1 );

    Err_wrt:
    return( 8 );
}
#endif

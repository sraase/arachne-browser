/*---------------------------------------------------*/
// Write text to virtual videoram : 1|8 bit/pixel
/*---------------------------------------------------*/

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

int xv_chr_mem(char *fnt_chr,           // Zacatek znaku ve fontu
		int  kx,                // Posun ve smeru X
		int  dx,                // Delka radku
		int  xfnt, int yfnt,    // Velikost fontu v pixlech
		int  color,int fillc,   // Barva znaku a pozadi
		char *subobr,           // Obraz se znaky textu (OUT!)
		int  bin);              // bin=0/1 subobr bytovy/binarni

int xv_text_virt(int xp, int yp, char *text)
{
   int len,px_len,Moff,Byt1,Byt2,Len,i,xlu;
   unsigned int off, ii, vypln, *subobr2;

   int nznk1,nznk2,nbyte,j1,len22,nrw,ncc,zb1,jend,py_len,ist,nb;
   unsigned char cx,ch1,zoobuf[3000],M2=0xC0;
   unsigned char *subobr,*zbuf,*hsubobr;
   long size,adr;
   XMOVE xmove;
   unsigned char mezery[600];
   //---------------------------------------------------------
   memset(mezery,0,600);

   py_len = x_txheight(text);        // Vyska v pixlech
   px_len = x_txwidth(text);         // Delka v pixlech

   if(px_len == 0) return( 1 );

   if(xv_vv[xv_act].xv_bits <= 0)                 // Bytovy (8bit/pixel)
    {
      Moff = 0;
      Byt1 = xp;
	  xlu  = px_len;
#if HI_COLOR
      if(xv_vv[xv_act].xv_bits == -1)   // Hicol
       Len  = px_len * 2;
	  else
#endif
	   Len  = px_len;
    }
   else if(xv_vv[xv_act].xv_bits == 3)            //(1bit/pixel) 
    { Moff = xp % 8;                // Beg offset pro prvni znak
      Byt1 = xp / 8;                // Prvni byte v XMS
      Byt2 = (xp + px_len - 1) / 8; // Posledni byte
      Len  = Byt2-Byt1+1;
	  xlu  = Len * 8;
	  Byt1 = Byt1 * 8;
    }
   else
   { return( 10 );
   }
   size = (long)Len * (long)py_len + 4;
   if(size > 65500L) return( 2 );

   //--------- Alokace bufru na text --------------------
   hsubobr = farmalloc( size );
   if(hsubobr == NULL) return( 4 );
   subobr = hsubobr + 4;     // data
   subobr2= (unsigned *)subobr;

   if(xv_vv[xv_act].xv_bits <= 0)   // 8|16bit/pix
    {
      if(xg_chrmod == 0)
      { if(xv_vv[xv_act].xv_bits == 0)
		 memset(subobr,xg_fillc,(unsigned)size);
#if HI_COLOR
	    else
		{ vypln = xg_hival[xg_fillc];
		  for(ii = 0; ii<(unsigned)size/2; ii++)
		  { subobr2[ii] = vypln;
		  }
		}
#endif
      }
      else
      { goto Read_rect;
      }
    }
   else                             // 1bit/pix
    {
      Read_rect:
      ist = xv_int_rea(Byt1, yp, xlu, py_len, hsubobr);
      if(ist != 1) return( ist );
    }

   //---------------- Vlasni psani textu do subobru-------------
   len = strlen(text);
   for(i=0; i<len; i++)
    {
     off = text[i] & 0x00FF;
     if(off < 32)                     // !!! Mezery sirky 1..31
     {  if(xg_31yn == 0) // Nekreslit
	 { Moff += off;
	   continue;
	 }
	else             // Kreslit
	 { zbuf = mezery;
	   xg_xfnt = off;
	 }
     }
     else
     {
     //--------- Ziskat adresu fontu v xg_fbuf ------------------
     if(xg_foncon == 0)                // Konstantni vzdy v pameti
       { xg_xfnt = xg_fonlen[off];
	 off = off * xg_fbyt;
	 zbuf = xg_fbuf+off;
       }
     else                              // Proporcni
       { adr = xg_fonadr[off];
	 if(adr == -1L) continue;      // tento znak neexistuje
	 xg_xfnt = xg_fonlen[off];

	 if(xg_fonmem == 0)            // MEM
	  { zbuf = xg_fbuf + (unsigned int)adr;
	  }
	 else if(xg_fonmem == 1)       // XMS
	  { xmove.sourceH = xg_fonhan;
	    xmove.sourceOff = adr;
	    xmove.destH   = 0;
	    xmove.destOff = ptr2long(xg_fbuf);
	    len22 = xg_yfnt * (((xg_xfnt-1)>>3)+1); // pocet bytu znaku
	    xmove.length = (long)len22;
	    ist = h_xmove(&xmove);
	    if(!ist) return( 2 );
	    zbuf = xg_fbuf;
	  }
	 else                          // DISK - not supported
	  { continue;
	  }
       }
     }

     if(xg_fnt_zoo == 1)    // Original velikost
      {
       xv_chr_mem(zbuf, Moff, px_len,xg_xfnt,xg_yfnt, xg_color,xg_fillc,
		   subobr,xv_vv[xv_act].xv_bits);
       Moff += xg_xfnt;
      }
     else if(xg_fnt_zoo == 2)		  // Zooming 2x, (4x?)
      {
      nb = ((((xg_xfnt-1)>>3)+1)*xg_yfnt)*(xg_fnt_zoo<<1); // pocet bytu
      if(nb > 3000)
       continue;
      else
       memset(zoobuf,0,nb); // Vynuluj zoobuf

      nznk1 = 0;
      nznk2 = 0;
      nbyte = ((xg_xfnt-1)>>3) + 1;
      zb1   = xg_xfnt % 8;
      if(zb1 == 0) zb1 = 8;
      len22 = ((xg_xfnt-1) * xg_fnt_zoo)/8 + 1;

      for(nrw=0; nrw<xg_yfnt; nrw++)  // Cykl pres radky znaku fontu
	{ for(ncc=0; ncc<nbyte; ncc++)    // Cykl pres byty radku
	   { ch1 = *(zbuf+nznk1);
	     if(ncc == (nbyte-1))
	       jend = zb1;
	     else
	       jend = 8;
	     for(j1=0; j1<jend; j1++)    // Zvetsi jeden byte (cast)
	      { cx = ch1 & 0x80;
		if(cx != 0)
		{
		 if(j1 < 4)
		  zoobuf[nznk2  ] |= (M2>>(2*j1));  // M2= 11 00 00 00
		 else
		  zoobuf[nznk2+1] |= (M2>>(2*(j1-4)));
		}
		ch1 = ch1<<1;
	      } // End for j1
	     nznk1++;
	     if(ncc == (nbyte-1))
	      nznk2 += ((zb1-1)*xg_fnt_zoo)/8 + 1;
	     else
	      nznk2 += xg_fnt_zoo;
	   } // End for ncc
	   memcpy(zoobuf+nznk2,zoobuf+nznk2-len22,len22);
	   nznk2 += len22;
	} // End for nrw
       xv_chr_mem(zoobuf,Moff,px_len,xg_xfnt*xg_fnt_zoo,xg_yfnt*xg_fnt_zoo,
		   xg_color,xg_fillc, subobr, xv_vv[xv_act].xv_bits);
       Moff += xg_xfnt*xg_fnt_zoo;
      }   // End if Zoom
    } // End for i = znaky

   //----------- Write subobr to vv -------------------------
   ist = xv_int_wrt(Byt1, yp, hsubobr);
   if(ist != 1) return( ist );

   farfree(hsubobr);
   xg_xfnt = xg_fonlen[32];
   return( 1 );

   // ---------- Chyby -------------------------
   Err_wr:
   xg_xfnt = xg_fonlen[32];
   farfree(hsubobr);
   return( 6 );
}
#endif

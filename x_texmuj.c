/*---------------------------------------------------*/
/* Pise text v grafickem modu od libovolneho pixlu a */
/* fontem nahranym pres x_fnt_load()                 */
/* Pise se barvou xg_color na pozadi xg_fillc        */
/* Bere se v uvahu zarovnani ve smeru X a Y          */
/*---------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <alloc.h>
#include <mem.h>

#include "x_lopif.h"

typedef struct                  // XMS
  {
  unsigned long length;         /* velikost prenasene pameti v B (suda) */
  unsigned int sourceH;         /* handle pro zdroj (0=konvencni  */
  unsigned long sourceOff;      /* offset zdroje pameti           */
  unsigned int destH;           /* handle pro terc (0=konvencni   */
  unsigned long destOff;        /* offset terce pameti            */
  } XMOVE;
int move_xmem(XMOVE *p);
int long ptr2long(char *p);

int x_text_ib(int xp, int yp, unsigned char *text)
{
   int len,xpp,ypp,i,k,px_len,nb,pravy;
   unsigned int off;

   //XMOVE xmove;
   int nznk1,nznk2,nbyte,j1,len22,nrw,ncc,zb1,jend,ist;
   unsigned char  cx,ch1,M2=0xC0;
   //unsigned char  M4=0xF0;
   unsigned char *zbuf;
   long adr;
   unsigned char *zoobuf;
   unsigned char mezery[256];    // pro znaky <32
   int  col_chr, col_bck, LxMask;
   //------------------------------------------------------

#ifdef VIRT_SCR
   if(xg_video_XMS != 0)     // text to virt screen
   {  ist = xv_text_virt(xp, yp, text);
      return( ist );
   }
#endif

   if(xg_fnt_zoo != 1)     // ZOOM
    { zoobuf = farmalloc(6000L);
      if(zoobuf == NULL) return ( 2 );
    }
   memset(mezery,0,256);

   len = strlen(text);
   if(xg_foncon == 0)       // KONSTANTNI SIRE
     px_len = len * xg_xfnt * xg_fnt_zoo;  // Delka v pixlech
   else                     // PROMENNA SIRE
     { xg_xfnt = xg_foncon;
       px_len = 0;
       for(i=0; i<len; i++)
	{ if(text[i] < 32)
	   px_len += text[i];
	  else
	   px_len += xg_fonlen[text[i]];
	}
       px_len *= xg_fnt_zoo;
     }

   switch(xg_tjustx)
     { case 0: xpp = xp;              /* Left   */
	       break;
       case 1: xpp = xp - px_len/2 ;  /* Center */
	       break;
       case 2: xpp = xp - px_len + 1; /* Right  */
     }

   switch(xg_tjusty)
     { case 0: ypp = yp - xg_yfnt*xg_fnt_zoo;      /* Bott   */
	       break;
       case 1: ypp = yp - (xg_yfnt/2)*xg_fnt_zoo;  /* Center */
	       break;
       case 2: ypp = yp;                           /* Top    */
	       break;
     }

   if(xg_notview == 0)
   {
   xpp = xpp + xg_view[0];             /* Do viewportu  */
   if(xpp < 0) xpp = 0;
   ypp = ypp + xg_view[1];
   if(ypp < 0) ypp = 0;

   //if(xg_clip != 0)                    /* Oriznuti ?? MOC NEFUNGUJE !!*/
   //  { if((xpp+px_len) > xg_view[2])
   //	 { k = xpp+px_len - xg_view[2];
   //	   len = len - (k / xg_xfnt*xg_fnt_zoo);  // ?? PROPORCNI ??
   //	 }
   //  }

   pravy = xg_view[2];
   }
   else
   {
   pravy = x_maxx();
   }

   col_chr =  xg_color;     // Nastavene barvy textu
   col_bck =  xg_fillc;

   if(xg_256 == MM_2)      // Binar
   { if((col_bck&1) == 0)  // pozadi barvou 0, popredi 1
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

   k = 0;
   for(i=0; i<len; i++)
    { off = text[i] & 0x00FF;          // Znak
      if(xg_foncon != 0 && off < 32)   // !!! Mezery sirky 1..31 jen PROP!!!
      { if(xg_31yn == 0) // Nekreslit
	 { k += off;
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
       {
	 xg_xfnt = xg_fonlen[off];
	 off = off * xg_fbyt;
	 zbuf = xg_fbuf+off;
       }
      else                              // Proporcni
       { adr = xg_fonadr[off];
	 if(adr == -1L) continue;      // tento znak neexistuje
	 xg_xfnt = xg_fonlen[off];

	 if(xg_fonmem == 0)            // MEM
	  { zbuf = xg_fbuf + adr;
	  }
	 else    // 971002
	  { goto Clipend;
	  }
	/******
	 else if(xg_fonmem == 1)       // XMS
	  { xmove.sourceH = xg_fonhan;
	    xmove.sourceOff = adr;
	    xmove.destH   = 0;
	    xmove.destOff = ptr2long(xg_fbuf);
	    len22 = xg_yfnt * (((xg_xfnt-1)>>3)+1); // pocet bytu znaku
	    if((len22&1) != 0) len22++;             // Nahoru na sude B
	    xmove.length = (long)len22;
	    ist = move_xmem(&xmove);
	    if(!ist) { if(xg_fnt_zoo != 1) farfree(zoobuf);
		       return( 4 );
		     }
	    zbuf = xg_fbuf;
	  }
	 else                          // DISK
	  {len22 = xg_yfnt * (((xg_xfnt-1)>>3)+1); // pocet bytu znaku
	   lseek(xg_fonhan,adr+8,SEEK_SET);        // Hlavicka 8B
	   ist = read(xg_fonhan, xg_fbuf, len22);
	   if(ist < len22) { if(xg_fnt_zoo != 1) farfree(zoobuf);
			     return( 4 );
			   }
	   zbuf = xg_fbuf;
	  }
	****/
       }
      }

     if(off >= 0)
     {
     if(xg_fnt_zoo == 1)    // Original velikost
      {
      if((xpp+k+xg_xfnt-1) > pravy) goto Clipend;    // Oriznuti
      if(xg_chr1c != NULL) col_chr = xg_chr1c[i];
      if(xg_chr2c != NULL) col_bck = xg_chr2c[i];
      if(xg_256 == MM_256 || xg_256 == MM_16)
       { wrt_chr(zbuf,xpp+k,ypp,xg_xfnt,xg_yfnt,col_chr,col_bck);
       }
#if HI_COLOR
      else if(xg_256 == MM_Hic)
       { xh_wrtchr(zbuf,xpp+k,ypp,xg_xfnt,xg_yfnt,col_chr,col_bck);
       }
#endif
      else if(xg_256 == MM_2)
       { wrt_bincga(zbuf,xpp+k,ypp,xg_xfnt,xg_yfnt,LxMask,0);
       }
      k += xg_xfnt;
      }
     else if(xg_fnt_zoo == 2)	 // Zooming 2x, 4x->zrusit
      {
      nb = ((((xg_xfnt-1)>>3)+1)*xg_yfnt)*(xg_fnt_zoo<<1); // pocet bytu
      if(xg_fnt_zoo == 4) nb = nb<<1;
      if(nb > 6000)
       continue;
      else
       memset(zoobuf,0,nb); // Vynuluj zoobuf

      nznk1 = 0;
      nznk2 = 0;
      nbyte = ((xg_xfnt-1)>>3) + 1;
      zb1   = xg_xfnt % 8;
      if(zb1 == 0) zb1 = 8;
      len22 = ((xg_xfnt-1) * xg_fnt_zoo)/8 + 1;

      for(nrw=0; nrw<xg_yfnt; nrw++)      // Cykl pres radky znaku fontu
	{ for(ncc=0; ncc<nbyte; ncc++)    // Cykl pres byty radku
	   { ch1 = *(zbuf+nznk1);
	     if(ncc == (nbyte-1))
	       jend = zb1;
	     else
	       jend = 8;
	     for(j1=0; j1<jend; j1++)     // Zvetsi jeden byte (cast)
	      { cx = ch1 & 0x80;
		if(cx != 0)
		{
		if(xg_fnt_zoo == 2)   // 2x
		 {
		 if(j1 < 4)
		  zoobuf[nznk2  ] |= (M2>>(2*j1));  // M2= 11 00 00 00
		 else
		  zoobuf[nznk2+1] |= (M2>>(2*(j1-4)));
		 }
		else                  // 4x
		 {
		 /***
		 if(j1 < 2)
		  zoobuf[nznk2  ] |= (M4>>(4*j1));  // M4= 11 11 00 00
		 else if(j1 < 4)
		  zoobuf[nznk2+1] |= (M4>>(4*(j1-2)));
		 else if(j1 < 6)
		  zoobuf[nznk2+2] |= (M4>>(4*(j1-4)));
		 else
		  zoobuf[nznk2+3] |= (M4>>(4*(j1-6)));
		 ***/
		 }
		}
		ch1 = ch1<<1;
	      }
	     nznk1++;
	     if(ncc == (nbyte-1))
	      nznk2 += ((zb1-1)*xg_fnt_zoo)/8 + 1;
	     else
	      nznk2 += xg_fnt_zoo;
	   }

	   if(xg_fnt_zoo == 2) // 2 radky
	   {
	     memcpy(zoobuf+nznk2,zoobuf+nznk2-len22,len22);
	     nznk2 += len22;
	     }
	     else              // 4 radky
	     {
	     /***
	     memcpy(zoobuf+nznk2,zoobuf+nznk2-len22,len22);
	     nznk2 += len22;
	     memcpy(zoobuf+nznk2,zoobuf+nznk2-len22,len22);
	     nznk2 += len22;
	     memcpy(zoobuf+nznk2,zoobuf+nznk2-len22,len22);
	     nznk2 += len22;
	     ***/
	   }
	}
      if((xpp+k+xg_xfnt*xg_fnt_zoo-1) > pravy) goto Clipend;    // Oriznuti
      if(xg_chr1c != NULL) col_chr = xg_chr1c[i];
      if(xg_chr2c != NULL) col_bck = xg_chr2c[i];
      if(xg_256 == MM_256 || xg_256 == MM_16)
       { wrt_chr(zoobuf,xpp+k,ypp,xg_xfnt*xg_fnt_zoo,xg_yfnt*xg_fnt_zoo,col_chr,col_bck);
       }
#if HI_COLOR
      else if(xg_256 == MM_Hic)
       { xh_wrtchr(zoobuf,xpp+k,ypp,xg_xfnt*xg_fnt_zoo,xg_yfnt*xg_fnt_zoo,col_chr,col_bck);
       }
#endif
      else if(xg_256 == MM_2)
       { wrt_bincga(zoobuf,xpp+k,ypp,xg_xfnt*xg_fnt_zoo,xg_yfnt*xg_fnt_zoo,LxMask,0);
       }
      k += xg_xfnt*xg_fnt_zoo;
      }
      else
      { goto Clipend;
      }

     }
    }
   Clipend:
   xg_xfnt = xg_fonlen[32];
   if(xg_fnt_zoo != 1) farfree(zoobuf);
   return( 1 );
}


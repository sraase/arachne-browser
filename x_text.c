
// ========================================================================
// X_LOPIF text output port to pure POSIX enviroment
// (c) 2000 Arachne Labs, based on X_LOPIF for DOS (c) Zdenek Harovnik
// ========================================================================

/*---------------------------------------------------*/
// Write text to to real or virtual videoram : 1|8 bit/pixel
/*---------------------------------------------------*/

#include "posix.h"
#include "v_putimg.h"
#include "x_lopif.h"

int xv_chr_mem(char *fnt_chr,           // Zacatek znaku ve fontu
                                        // tr.: beginning of chars in font 
		int  kx,                // Posun ve smeru X
                                        // tr.: move in x direction
                int  dx,                // Delka radku 
                                        // tr.: length of line
		int  xfnt, int yfnt,    // Velikost fontu v pixlech
                                        // tr.: size of font in pixels
		int  color,int fillc,   // Barva znaku a pozadi
                                        // tr.: colour of char and backgr.
		char *subobr,           // Obraz se znaky textu (OUT!)
                                        // tr.: picture with chars of text
		int  bin);              // bin=0/1 subobr bytovy/binarni

int x_text_ib(int xp, int yp, unsigned char *text)
{
   int len,px_len,Moff,Byt1,Byt2,Len,i,xlu;
   unsigned int off, ii;
   unsigned vypln;
   unsigned short *subobr2;

   int nznk1,nznk2,nbyte,j1,len22,nrw,ncc,zb1,jend,py_len,ist,nb;
   unsigned char cx,ch1,zoobuf[3000],M2=0xC0;
   unsigned char *subobr,*zbuf,*hsubobr;
   long size,adr;
   unsigned char mezery[600];
   int bin=0; //for xv_chr_mem ...
   //---------------------------------------------------------
   if(xg_31yn==1)
   {
    printf("[the useless memset code in x_text_ib]");
     memset(mezery,0,600);
   }

   py_len = x_txheight(text);        // Vyska v pixlech (tr. height in pixels)
   px_len = x_txwidth(text);         // Delka v pixlech (tr. length in pixels)

   if(px_len == 0) return( 1 );

   switch(xg_tjustx)
     { 
       case 1: xp -= px_len/2 ;  /* Center */
	       break;
       case 2: xp -= px_len + 1; /* Right  */
     }

   switch(xg_tjusty)
     { case 0: yp -= py_len + 1;      /* Bottom   */
	       break;
       case 1: yp -= py_len/2;  /* Center */
     }

   if(xg_256==MM_256 || xg_256==MM_Hic)                 // Bytovy (8bit/pixel)
    {
      Moff = 0;
      Byt1 = xp;
      xlu  = px_len;
#if HI_COLOR
      if(xg_256==MM_Hic)   // Hicol
      {
       Len  = (px_len + 2)* sizeof(short);
       bin=-1;
      }
      else
#endif
     {
      Len  = px_len+2;
      bin=0;
     }
    }
   else if(xg_256==MM_2)            //(1bit/pixel) 
    { Moff = xp % 8;                // Beg offset pro prvni znak
                                    // tr.: Beg offset for first char
      Byt1 = xp / 8;                // Prvni byte v XMS (tr.: first byte in XMS)
      Byt2 = (xp + px_len - 1) / 8; // Posledni byte (tr.: last byte)
      Len  = Byt2-Byt1+1;
	  xlu  = Len * 8;
	  Byt1 = Byt1 * 8;
     bin=-1;
    }
   else
   { return( 10 );
   }
   size = (long)Len * (long)py_len + 2*sizeof(int);

   if(size > 65500L) return( 2 );

   //--------- Alokace bufru na text --------------------
   // tr.: allocation of text buffer
   hsubobr = farmalloc( size );
   if(hsubobr == NULL) 
   {
    return( 4 );
   }
   subobr = hsubobr + 2*sizeof(short);     // data
   subobr2= (unsigned short *)subobr;

   if(xg_chrmod == 0)
   { 
    short *xbox=(short *)hsubobr;
    short *ybox=(short *)&hsubobr[sizeof(short)];
    *xbox=xlu;
    *ybox=py_len;
    
    if(xg_256==MM_256)
     memset(subobr,xg_fillc,(unsigned)(size-2*sizeof(short)) );
#if HI_COLOR
    else
    {
     vypln = xg_hival[xg_fillc];
     for(ii = 0; ii< size/sizeof(short)-2*sizeof(short) ; ii++)
     {
      subobr2[ii] = vypln;
     }
    }
#endif
   }
   else
   {
    v_getimg(Byt1, yp, Byt1+xlu-1, yp+py_len-1, hsubobr);
   }

   //---------------- Vlasni psani textu do subobru-------------
   // tr.: writing text into file
   len = strlen(text);
   for(i=0; i<len; i++)
    {
     off = text[i] & 0x00FF;
     if(off < 32)                // !!! Mezery sirky 1..31 (tr.: spaces width)
     {  if(xg_31yn == 0) // Nekreslit (tr.: do not draw)
	 { Moff += off;
	   continue;
	 }
        else             // Kreslit (tr.: draw)
	 { 
	   zbuf = mezery;
	   xg_xfnt = off;
	 }
     }
     else
     {
     //--------- Ziskat adresu fontu v xg_fbuf ------------------
     // tr.: get address of font in xg_fbuf
     if(xg_foncon == 0)                // Konstantni vzdy v pameti
                                       // tr.: constant always in memory
       { xg_xfnt = xg_fonlen[off];
	 off = off * xg_fbyt;
	 zbuf = xg_fbuf+off;
       }
     else                              // Proporcni (tr.: proportional)
       { adr = xg_fonadr[off];
         if(adr == -1L) continue;      // tento znak neexistuje
                                       // tr.: this char does not exist
	 xg_xfnt = xg_fonlen[off];

	  { zbuf = xg_fbuf + (unsigned int)adr;
	  }
       }
     }

     if(xg_fnt_zoo == 1)    // Original velikost (tr.: original size)
      {
          
       xv_chr_mem(zbuf, Moff, px_len,xg_xfnt,xg_yfnt, xg_color,xg_fillc,
		   subobr,bin);
       Moff += xg_xfnt;
      }
     else if(xg_fnt_zoo == 2)		  // Zooming 2x, (4x?)
      {
      nb = ((((xg_xfnt-1)>>3)+1)*xg_yfnt)*(xg_fnt_zoo<<1); // pocet bytu
                                                 // tr.: number of bytes
      if(nb > 3000)
       continue;
      else
       memset(zoobuf,0,nb); // Vynuluj zoobuf (tr.: erase zoobuf)

      nznk1 = 0;
      nznk2 = 0;
      nbyte = ((xg_xfnt-1)>>3) + 1;
      zb1   = xg_xfnt % 8;
      if(zb1 == 0) zb1 = 8;
      len22 = ((xg_xfnt-1) * xg_fnt_zoo)/8 + 1;

      for(nrw=0; nrw<xg_yfnt; nrw++)  // Cykl pres radky znaku fontu
                    // tr.: cycle through rows of character of font
	{ for(ncc=0; ncc<nbyte; ncc++)    // Cykl pres byty radku
                    // tr.: cycle through bytes of row
	   { ch1 = *(zbuf+nznk1);
	     if(ncc == (nbyte-1))
	       jend = zb1;
	     else
	       jend = 8;
	     for(j1=0; j1<jend; j1++)    // Zvetsi jeden byte (cast)
                                         // tr.: increase one byte (part)
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
		   xg_color,xg_fillc, subobr, bin);
       Moff += xg_xfnt*xg_fnt_zoo;
      }   // End if Zoom
    } // End for i = znaky (tr.: end for i = characters

   //----------- Write subobr to vv -------------------------
   v_putimg(Byt1, yp, hsubobr);

   farfree(hsubobr);
   xg_xfnt = xg_fonlen[32];
   //printf("|");
   return( 1 );

   // ---------- Chyby -------------------------
   // tr.: errors
   Err_wr:
   xg_xfnt = xg_fonlen[32];
   farfree(hsubobr);
   return( 6 );
}

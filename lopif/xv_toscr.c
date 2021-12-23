//----------------------------------------------------------------------
//   Cteni z virtualni videoram do bufru ve formatu pro "putimage"
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <fcntl.h>
#include <io.h>
#include <mem.h>

#include "x_lopif.h"

#ifdef VIRT_SCR
int  x_bin_256(int xz, int yz, char *buf, int rovin);
void z_bitbyte(char *line, char *out, int lenb);

typedef struct
  {
  unsigned long length;         /* velikost prenasene pameti      */
  unsigned int sourceH;         /* handle pro zdroj (0=konvencni  */
  unsigned long sourceOff;      /* offset zdroje pameti           */
  unsigned int destH;           /* handle pro terc (0=konvencni   */
  unsigned long destOff;        /* offset terce pameti            */
  } XMOVE;

int long ptr2long(char *p);
int h_xmove(XMOVE *p);

//--------  Read rect from XMS / disk to buffer (max. 64kb!!) ----
// 1bit/pixel : nasobky xs a dx.
int xv_int_rea(int xs, int ys, int dx, int dy, char *buf)
{
// xs,ys, dx,dy - pocatek a velikost obrazu v pixlech
// buf - vystupni bufer s obrazem pro putimage

   long     adr,adrz;
   unsigned lenb,i;
   char    *bufa;
   //int     posu,lenbit,j1;
   int     ist;
   XMOVE    xmove;

   _AX = dx;                      // Velikost obrazu v bufru (4B)
   buf[0] = _AL; buf[1] = _AH;
   _AX = dy;
   buf[2] = _AL; buf[3] = _AH;

   // xv_bits : 0 or 3 or -1
   // pocatecni adresa v souboru (XMS)
   if(xv_vv[xv_act].xv_bits >= 0)
   { adrz = (long)ys * xv_vv[xv_act].xv_len_col + (long)(xs>>xv_vv[xv_act].xv_bits) +
		xv_vv[xv_act].xv_zmap;
     lenb = dx>>xv_vv[xv_act].xv_bits;         // delka radku v bytech
   }
   else
   { adrz = (long)ys * xv_vv[xv_act].xv_len_col + (long)(xs*2) +
		xv_vv[xv_act].xv_zmap;
     lenb = dx*2;
   }

   adr  = adrz;
   bufa = buf+4;

   if(xv_vv[xv_act].xv_XMS != -1)
     {xmove.length  = (long)lenb;            // Prenos z XMS do DOS
      xmove.sourceH = xv_vv[xv_act].xv_XMS;
      xmove.sourceOff = adrz;
      xmove.destH   = 0;
      xmove.destOff = ptr2long(bufa);
     }

   for(i=0; i<dy; i++)
     {
       if(xv_vv[xv_act].xv_XMS == -1)
       {
       lseek(xv_vv[xv_act].xv_file,adr,SEEK_SET);          // Z disku
       ist = read(xv_vv[xv_act].xv_file,bufa,lenb);
       if(ist < lenb) goto Err_rea;
       bufa += lenb;
       adr  += xv_vv[xv_act].xv_len_col;
       }
       else                                  // Z XMS
       {
       ist = h_xmove(&xmove);
       if(!ist) goto Err_rea;
       xmove.sourceOff += xv_vv[xv_act].xv_len_col;        // Nova adresa XMS
       bufa += lenb;
       xmove.destOff = ptr2long(bufa);       // Nova adr v bufru
       }
     }
   return( 1 );

   Err_rea:
   return( 4 );
}

//----------------------------------------------------------------------
//   Cteni z virtualni videoram a jeji zapis do fyzicke videoram
int xv_to_scr(int xs, int ys, int xo, int yo, int dx, int dy)
{
//  xs,ys - pravy horni bod obrazu v souboru
//  xo,yo - pravy horni bod obrazu na obrazovce
//  dx,dy - velikost obrazu v pixlech

//  Ppg. precte se souboru .OBR obdelnik a zobrazi ho.
//  Pozn: pro 2 a 16 barev musi byt vsechna x nasobkem 8 !!!

    char *inbuf;
    int  ist,yak,dyak,yoak;
    long lenmap,xx_lenbuf;
    int  nkusu,nline,kk;

    // Neni otevren soubor !!
    if(xv_vv[xv_act].xv_file <= 0 && xv_vv[xv_act].xv_XMS == -1) return( 2 );

    lenmap = farcoreleft();                    // Kolik alokovat
    if(lenmap >= 65512L)
      xx_lenbuf = 55512L;
    else
      xx_lenbuf = lenmap - (dx+1024);

    if(xx_lenbuf < (dx+16)) return( 6 );       // Neni pamet ani na 1 radek

    inbuf = farmalloc(xx_lenbuf);
    if(inbuf == NULL) return( 6 );
    if(xv_vv[xv_act].xv_bits >= 0)
     lenmap = (long)dy * (dx>>xv_vv[xv_act].xv_bits);  // Velikost obdelnika v B
    else
     lenmap = (long)dy * (dx*2); // Velikost obdelnika v B
    if(lenmap > (xx_lenbuf-4))
    {  if(xv_vv[xv_act].xv_bits >= 0)
	nline = (xx_lenbuf-4) / (dx>>xv_vv[xv_act].xv_bits);   // Pocet radek pruhu
       else
	nline = (xx_lenbuf-4) / (dx*2);
       if(nline < 1) goto Err_vrea;
       nkusu = dy / nline;
       if((dy % nline) != 0) nkusu++;
     }
    else                                           // Jeden pruh
     { nkusu = 1;
       nline = dy;
     }

    yak = ys;
    yoak= yo;
    dyak= nline;

    for(kk = 1; kk<=nkusu; kk++)                 // Cykl pres pruhy
     {
       if(kk == nkusu)
	{ dyak = dy - (kk-1)*nline;
	}
       ist = xv_int_rea(xs,yak,dx,dyak,inbuf);
       if((ist&1) == 0) goto Err_vrea;

       if(xv_vv[xv_act].xv_bits == 3)           // 1bit/pix
	{ if(xg_256 == MM_2)
	   x_putimg(xo, yoak, inbuf,0);  //  2 colors
	  else if(xg_256 == MM_16)
	   x_pimg16(xo,yoak,inbuf,0,1);  // 16 colors
	  else if(xg_256 == MM_256)
	   x_bin_256(xo,yoak,inbuf,1);   // 256 colors
	}
       else if(xv_vv[xv_act].xv_bits == 0)      // 8bit/pix
	{ if(xg_256 == MM_256 && xv_vv[xv_act].xv_bits == 0)
	  { x_putimg(xo, yoak, inbuf,0);
	  }
	}
#if HI_COLOR
       else                                     //Hi col mod
	{ if(xg_256 == MM_Hic && xv_vv[xv_act].xv_bits == -1)
	  { x_putimg(xo, yoak, inbuf,0);
	  }
	}
#endif
       yak  += nline;
       yoak += nline;
     } // End for

    farfree(inbuf);

    return( 1 );

    Err_vrea:
    farfree(inbuf);
    return( 4 );
}

// Kresleni bufru 1bit/pixel -> 256 color mode
int x_bin_256(int xz, int yz, char *buf, int rovin)
{
    int  dx,dy,lenb,i;
    char *line,out[1280];

    if(rovin != 1) return( 2 );

   _AL = buf[0];  _AH = buf[1];   // sloupcu
    dx = _AX;
   _AL = buf[2];  _AH = buf[3];   // radku
    dy = _AX;

    lenb = ((dx-1)>>3) + 1;      // Bytu na radek
    line = buf+4;                // Prvni radek

    for(i=0; i<dy; i++)          // Rozpakovani a kresleni
      { z_bitbyte(line,out,lenb);
	wrt_video(out,xz,yz+i,dx,1,1);
	line += lenb;
      }

    return( 1 );
}
#endif
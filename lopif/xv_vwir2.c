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
//------- Zapis obdelnika videoram do XMS/Disk -------------------
int xv_to_virt(int xo, int yo, int xs, int ys, int dx, int dy)
{
//  xo,yo - pravy horni bod obrazu na obrazovce
//  xs,ys - pravy horni bod obrazu v souboru
//  dx,dy - velikost obrazu v pixlech

//  Ppg. zapise do souboru obdelnik z videoram.
//  Pozn: pro 2 a 16 barev musi byt vsechna x nasobkem 8 !!!

    char *inbuf=NULL;
    int  ist,yak,dyak,yoak;
    long lenmap,xx_lenbuf;
    int  nkusu,nline,x2o,kk,LenLine;

    if(xv_vv[xv_act].xv_file <= 0 && xv_vv[xv_act].xv_XMS==-1) return( 2 );
    if(xv_vv[xv_act].xv_bits != 0 && xg_256 == MM_256) return( 4 );
    if(xv_vv[xv_act].xv_bits != 3 && xg_256 == MM_2) return( 4 );
#if HI_COLOR
    if(xv_vv[xv_act].xv_bits != -1 && xg_256 == MM_Hic) return( 4 );
#endif

    lenmap = farcoreleft();                    // Kolik alokovat
    if(lenmap >= 65512L)
      xx_lenbuf = 55512L;
    else
      xx_lenbuf = lenmap - (dx+1024);

    if(xx_lenbuf < (dx+16)) return( 6 );       // Neni pamet ani na 1 radek

    inbuf = farmalloc(xx_lenbuf);
    if(inbuf == NULL) return( 6 );

	if(xv_vv[xv_act].xv_bits >= 0)
     LenLine = dx>>xv_vv[xv_act].xv_bits;
	else
     LenLine = dx*2;

    lenmap = (long)dy * LenLine;  // Velikost obdelnika
    if(lenmap > (xx_lenbuf-4))
     { 
	   nline = (xx_lenbuf-4) / LenLine;  // Pocet radek pruhu
       if(nline < 1) goto Err_wrt;

       nkusu = dy / nline;
       if((dy % nline) != 0) nkusu++;
     }
    else                          // Jeden pruh
     { nkusu = 1;
       nline = dy;
     }

    yak = ys;          // V souboru
    yoak= yo;          // Na obrazovce
    dyak= nline;       // Pocet radku
    x2o = xo + dx - 1; // x2 na obrazovce

    for(kk = 1; kk<=nkusu; kk++)  // Cykl pres pruhy
     {
       if(kk == nkusu)
	{ dyak = dy - (kk-1)*nline;
	  if(dyak == 0) goto End_ok;
	}
	x_getimg(xo,yoak,x2o,yoak+dyak-1,inbuf);

       ist = xv_int_wrt(xs,yak,inbuf);
       if((ist&1) == 0) goto Err_wrt;
       yak  += nline;
       yoak += nline;
     }
    End_ok:
    farfree(inbuf);
    return( 1 );

    Err_wrt:
    farfree(inbuf);
    return( 6 );
}
#endif

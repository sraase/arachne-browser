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

//----------------------------------------------------------------------
//   Zapis do virtualni videoram ve formatu "putimage"
int xv_int_wrt(int xs, int ys, char *buf)
{
// xs,ys - pocatek obrazu v pixlech
// buf - vystupni bufer s obrazem pro putimage

   long  adr,adrz;
   unsigned lenb,i;
   char  *bufa;
   int   ist,dx,dy;
   XMOVE xmove;

   _AL = buf[0];  _AH = buf[1];   // Velikost obrazu v pixlech
   dx = _AX;
   _AL = buf[2];  _AH = buf[3];
   dy = _AX;

   // pocatecni adresa v souboru
   if(xv_vv[xv_act].xv_bits >= 0)
   { adrz = (long)ys * xv_vv[xv_act].xv_len_col + (long)(xs>>xv_vv[xv_act].xv_bits) +
		     xv_vv[xv_act].xv_zmap;
     lenb = dx>>xv_vv[xv_act].xv_bits;            // delka radku v bytech v XMS
   }
   else
   { adrz = (long)ys * xv_vv[xv_act].xv_len_col + (long)(xs*2) +
		     xv_vv[xv_act].xv_zmap;
     lenb = dx*2; 
   }
   adr  = adrz;
   bufa = buf+4;

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
    return( 1 );

    Err_wrt:
    return( 4 );
}
#endif

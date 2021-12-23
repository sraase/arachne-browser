//------------ Zapis pixlu  do XMS (disk) -----------------
#include <mem.h>
#include <io.h>
#include <alloc.h>
//#include "x_virt.h"
#include "x_lopif.h"

// Zapis pixlu do XMS nebo DISK
#ifdef VIRT_SCR
typedef struct
  {
  unsigned long length;         /* velikost prenasene pameti      */
  unsigned int sourceH;         /* handle pro zdroj (0=konvencni  */
  unsigned long sourceOff;      /* offset zdroje pameti           */
  unsigned int destH;           /* handle pro terc (0=konvencni   */
  unsigned long destOff;        /* offset terce pameti            */
  } XMOVE;
int move_xmem(XMOVE *p);
int long ptr2long(char *p);

int z_xmsputpix(int x, int y, int col)
{
   XMOVE xmove;
   long  adr;
   unsigned int  pix;
   unsigned char cosi,bu2[2];
   int      nw2,ist;
   union    { unsigned char bu1[2];
              unsigned int  bu;
   } u;

   // Spocteme adresu
   if(xv_vv[xv_act].xv_bits >= 0)
   { adr = (long)y * xv_vv[xv_act].xv_len_col +
(long)(x>>xv_vv[xv_act].xv_bits) +
		    xv_vv[xv_act].xv_zmap;
   }
   else
#if HI_COLOR
   { adr = (long)y * xv_vv[xv_act].xv_len_col + (long)(x*2) +
		    xv_vv[xv_act].xv_zmap;
   }
#else
   { return( 0 );
   }
#endif
   //adr = (long)y * xv_vv[xv_act].xv_len_col + (long)x + xv_vv[xv_act].xv_zmap;

   // ---- Nacteme 2B z XMS ci disku
   if(xv_vv[xv_act].xv_XMS != -1)
   {
    xmove.sourceH = xv_vv[xv_act].xv_XMS;
    xmove.destH = 0;
    xmove.sourceOff = adr;
    xmove.destOff = ptr2long(bu2);
    xmove.length = 2L;
    ist = move_xmem(&xmove);
    if(!ist) goto End_err1;
   }
   else
   { nw2 = 1;
     if(xv_vv[xv_act].xv_bits == 3)
     { lseek(xv_vv[xv_act].xv_file,adr,SEEK_SET);
       ist = read(xv_vv[xv_act].xv_file,bu2,nw2);
       if(ist < nw2) goto End_err1;
     }
   }

   //------ Opravime prvni byte
   if(xv_vv[xv_act].xv_bits == 3)
   {pix = (unsigned)x & 0x0007;
	cosi= (0x80 >> pix);           // maska s 1 na pozici pixlu
	if((xg_bincol&1) == 0)         // pisu cernou
	 { cosi = ~cosi;
	   bu2[0] &= cosi;
	 }
	else                          // pisu bilou
	 { bu2[0] |= cosi;
	 }
   }
   else if(xv_vv[xv_act].xv_bits == 0)
   { bu2[0] = col;
   }
#if HI_COLOR
   else
   { if(xg_hipalmod == 0)
     { u.bu = xg_hival[col]; // pal -> RGB
     }
     else                   // col je jiz v HiCol
     { u.bu = col;
     }
     bu2[0] = u.bu1[0]; bu2[1] = u.bu1[1];
   }
#endif

   //------ Zapiseme 2B do XMS ci disku
   if(xv_vv[xv_act].xv_XMS != -1)
   {
    xmove.sourceH = 0;
    xmove.destH = xv_vv[xv_act].xv_XMS;
    xmove.sourceOff = ptr2long(bu2);;
    xmove.destOff = adr;
    xmove.length = 2L;
    ist = move_xmem(&xmove);
    if(!ist) goto End_err2;
   }
   else
   {
    lseek(xv_vv[xv_act].xv_file,adr,SEEK_SET);
    ist = write(xv_vv[xv_act].xv_file,bu2,nw2);
    if(ist < nw2) goto End_err2;
   }

   return( 1 );

   End_err1:
   return( 2 );
   End_err2:
   return( 4 );

}
#endif

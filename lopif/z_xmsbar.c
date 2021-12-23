//------- Funkce pro kresleni BARu primo do XMS (disku) ----------
#include <mem.h>
#include <io.h>
#include <alloc.h>
//#include "x_virt.h"
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
int move_xmem(XMOVE *p);
int long ptr2long(char *p);

int z_xmsbar(int x1, int y1, int x2, int y2, int col)
{
// x1..y2 - souradnice obdelniku v pixlech
// col    - barva baru

   XMOVE xmover,xmovew;
   int   xx1,yy1,xx2,yy2,x2o,y2o,ist;
   int   dxp,dxb,dxb2,i,jen_cele,color,irea,cl1,lom1,lom2;
   unsigned char mz,mk,bu2[2];
   long  start,adr4,LinB;
   unsigned int   ii, Rcol, *bline2;
   unsigned char *bline=NULL;
   unsigned char  b_loc[2600];
   unsigned char *b_glo=NULL;

   //--------------------------------------------------
   if(xv_vv[xv_act].xv_bits >= 0)
    x2o = (xv_vv[xv_act].xv_len_col << xv_vv[xv_act].xv_bits) - 1;
   else
    x2o = xv_vv[xv_act].xv_len_col/2 - 1;
   y2o = xv_vv[xv_act].xv_rows - 1;
   if(x1 < 0) xx1 = 0; else  xx1 = x1;  // Oriznuti
   if(y1 < 0) yy1 = 0; else  yy1 = y1;
   if(x2 > x2o) xx2 = x2o; else xx2 = x2;
   if(y2 > y2o) yy2 = y2o; else yy2 = y2;

   dxp = xx2 - xx1 + 1;                // Pixlu
   LinB = (long)yy1 * xv_vv[xv_act].xv_len_col + xv_vv[xv_act].xv_zmap;
   if(xv_vv[xv_act].xv_bits == 3)
    { dxb = ((dxp-1)>>3) + 1;          // Bytu
      start =  LinB + (long)xx1/8;
    }
   else if(xv_vv[xv_act].xv_bits == 0)
    { dxb = dxp;
      start = LinB + (long)xx1;
    }
#if HI_COLOR
   else
   { dxb = dxp * 2;
     start = LinB + (long)xx1*2;
   }
#endif

   if(dxb > 2600)
    { b_glo = farmalloc(16000L);
      if(b_glo == NULL) return( 2 );
      bline = b_glo;
    }
   else
    { bline = b_loc;
    }

   adr4 = start;
   jen_cele = 0;
   dxb2 = dxb;

   xmover.sourceH = xv_vv[xv_act].xv_XMS;
   xmover.destH = 0;

   xmovew.sourceH = 0;
   xmovew.destH = xv_vv[xv_act].xv_XMS;
   xmovew.sourceOff = ptr2long(bline);

   if(xv_vv[xv_act].xv_bits <= 0)     // Bytovy
   { color = col;

     Na_byty:
     if(xv_vv[xv_act].xv_XMS == -1)
	  irea = 0;
     else if((dxb&1) == 0)
	  irea = 0;
     else
	 { irea = 2; dxb2 = dxb+1;
	 }

	 if(xv_vv[xv_act].xv_bits == 0)
      memset(bline,color,dxb);
	 else
	 { bline2 = (unsigned *)bline;
	   //Rcol = xg_hival[col];
	   Rcol = col;
	   for(ii=0; ii<dxb/2; ii++) bline2[ii] = Rcol;
	 }

     xmover.length = 2;
     xmover.destOff = ptr2long(bu2);
     xmovew.length = dxb2;
   }
   else                 // Binarni
   { if((col&1) == 0)
	  color = 0x00;   // Kreslime nulami
     else
	  color = 0xFF;   // kreslime jednickami

     if((xx1&7)==0 && (dxp&7)==0)
	 { jen_cele = 1;
	   goto Na_byty; // jako bytovy (nasobky 8)
	 }
     else
	 { lom1 = xx1>>3;   // Zda byte navic ?
	   lom2 = (xx2>>3)+1;
	   if((lom2-lom1) > dxb) dxb++;

	   irea = dxb;   // necele byty
	   if(xv_vv[xv_act].xv_XMS != -1)
	   { if((irea&1)!=0) irea++; // Na sude
	   }
	  cl1 = xx1&7;        // Zbytek po /8
	  mz = (1<<(8-cl1))-1;// Maska pro prvni byte
	  mz = ~mz;
	  i  = (xx2&7) + 1;   // MAska pro posledni byte
	  mk = (1<<(8-i))-1;
	  if(dxb==1) mz = mz | mk;

	  xmover.length = irea;
	  xmover.destOff = ptr2long(bline);
	  xmovew.length = irea;
	 }
   }
   adr4 = start;

   for(i=0; i<(yy2-yy1+1); i++)    // Cykl pres radky baru
   {
    if(xv_vv[xv_act].xv_bits<=0 || (xv_vv[xv_act].xv_bits==3 && jen_cele==1)) // Cele byty
    { if(irea != 0)
      { xmover.sourceOff = adr4+dxb-1;
	    ist = move_xmem(&xmover);
	    if(!ist) goto End_err;
	    bline[dxb] = bu2[1];
      }

      if(xv_vv[xv_act].xv_XMS != -1)
      { xmovew.destOff = adr4;
	    ist = move_xmem(&xmovew);
	    if(!ist) goto End_err;
      }
      else
      { lseek(xv_vv[xv_act].xv_file,adr4,SEEK_SET);
	    ist = write(xv_vv[xv_act].xv_file,bline,dxb2);
	    if(ist < dxb2) goto End_err;
      }
    }
    else                                         // Maskovani
    { if(xv_vv[xv_act].xv_XMS != -1)
      { xmover.sourceOff = adr4;
	    ist = move_xmem(&xmover);
	    if(!ist) goto End_err;
	  }
      else
	  { lseek(xv_vv[xv_act].xv_file,adr4,SEEK_SET);
	    ist = read(xv_vv[xv_act].xv_file,bline,irea);
	    if(ist < irea) goto End_err;
      }
      if(color == 0)
      { bline[0] &= mz;
	  //bline[0] |= color;
      }
      else
      { bline[0] |= (~mz);
      }
      if(dxb <= 1) goto Zapis;
      if(dxb >  2)
      { memset(&bline[1],color,dxb-2);
      }
      if(color == 0)
      { bline[dxb-1] &= mk;
	  //bline[dxb-1] |= color;
      }
      else
      {  bline[dxb-1] |= (~mk);
      }
      Zapis:
      if(xv_vv[xv_act].xv_XMS != -1)
      { xmovew.destOff = adr4;
	    ist = move_xmem(&xmovew);
	    if(!ist) goto End_err;
	  }
      else
      { lseek(xv_vv[xv_act].xv_file,adr4,SEEK_SET);
	    ist = write(xv_vv[xv_act].xv_file,bline,irea);
	    if(ist < irea) goto End_err;
	  } 
	}
    adr4 += xv_vv[xv_act].xv_len_col;
   }  // End for i

  if(b_glo != NULL) farfree(b_glo);
  return( 1 );

  End_err:
  if(b_glo != NULL) farfree(b_glo);
  return( 4 );
}
#endif

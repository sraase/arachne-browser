
// ========================================================================
// X_LOPIF 1 char output port to pure POSIX enviroment,  for SVGAlib/GGI
// (c) 2000 Arachne Labs, based on X_LOPIF for DOS (c) Zdenek Harovnik
// ========================================================================

// ---------- Zapis jednoho znaku z fontu do vystupniho bufru ------
// bufer subobr ma vzdy pocatek [0,0] tedy Y je vzdy 0

#include "posix.h"
#include "x_lopif.h"    // Pro xg_chrmod

#ifdef VIRT_SCR
int xv_chr_mem(char *fnt_chr,          // Zacatek znaku ve fontu (binar)
		int  kx,                // Posun ve smeru X
		int  dx,                // Delka radku v pixlech
		int  xfnt, int yfnt,    // Velikost fontu v pixlech
		int  color,int fillc,   // Barva znaku a pozadi
		char *subobr,           // Obraz se znaky textu (OUT!)
		int  bin)               // bin=0/1 subobr bytovy/binarni
{
    int nb,i,j,jx,ien,lenx,xx1,xx2,dx8,zbyte,off,neg;
    unsigned int  iz,izz;
    unsigned char mask,mz,mk,cha,cl1,bh,bg;
    union { unsigned short AX;
	        unsigned char A[2];
	} u;

    nb = ((xfnt-1)>>3) + 1; // Pocet bytu na radek z fontu

    if(bin == 0)      //------------------- Bytovy OBR
    {
     for(i=0; i<yfnt; i++)     // Pres radky
	 {iz   = i*dx + kx;        // Zacatek radku
	  lenx = xfnt;
	  for(j=0; j<nb; j++)      // Pres byty radku fontu
	   { mask = fnt_chr[i*nb+j];
	     ien  = min(lenx,8);
	     for(jx=0; jx<ien; jx++)  // Pres byte fontu
	      { if((mask&0x80) != 0)
		      subobr[iz] = color;
		    else if(xg_chrmod == 0)
		      subobr[iz] = fillc;
		    mask = mask<<1;
		    iz++;
	      }
	     lenx = lenx-8;
	   }
	 }
    }
#if HI_COLOR
	else if(bin < 0)      // HiCol
	{ unsigned short Fore, Back;
	  unsigned short *subobr2; 
	  subobr2 = (unsigned short *)subobr;
	  Fore = xg_hival[color];
	  Back = xg_hival[fillc];
	  //Fore = color;
	  //Back = fillc;

      for(i=0; i<yfnt; i++)     // Pres radky
	  {iz   = i*dx + kx;        // Zacatek radku (ve 2B!)
	  lenx = xfnt;
	  for(j=0; j<nb; j++)      // Pres byty radku fontu
	   { mask = fnt_chr[i*nb+j];
	     ien  = min(lenx,8);
	     for(jx=0; jx<ien; jx++)  // Pres byte fontu
	      { if((mask&0x80) != 0)
		      subobr2[iz] = Fore;
		    else if(xg_chrmod == 0)
		      subobr2[iz] = Back;
		    mask = mask<<1;
		    iz++;
	      }
	     lenx = lenx-8;
	   }
	 }
	}
#endif
    else      // Bin
    {
       if((color&1) != 0)
	 neg = 0;
       else
	 neg = 1;
       xx1 =  kx>>3;
       xx2 = ((kx+xfnt-1)>>3) + 1;
       if((xx2-xx1) > nb)
	zbyte = 1;
       else
	zbyte = 0;

       dx8 = dx>>3;
       iz  = kx>>3;  // Byte prvniho radku
       izz = iz;
       cl1 = kx&7;   // Zbytek po /8

       mz = (1<<(8-cl1))-1;      // Maska pro prvni byte
       mz = ~mz;
       i  = ((kx+xfnt-1)&7) + 1; // MAska pro posledni byte
       mk = (1<<(8-i))-1;
       off = 0;
       for(i=0; i<yfnt; i++)     // Cykl pres radky
	{
	  j=nb;
	  if(neg == 0)
	   u.A[0] =  fnt_chr[off++];
	  else
	   u.A[0] = ~fnt_chr[off++];
	  j--;
	  u.A[1] = 0;
	  cha = u.A[0];
	  mask = mz;
	  Line:
	  u.AX = u.AX>>cl1;
	  if(j == 0) goto End_line;
	  bh = subobr[iz];
	  if(xg_chrmod == 0)
	   { bh = bh & mask;    // Prepis
	     bh = bh | u.A[0];
	   }
	  else
	   { if(neg == 1)
	      { bg = bh | mask;    // Jen popredi
		bg = bg & u.A[0];
		bg = bg | mask;
		bh = bg & bh;
	      }
	     else
	      { bg = bh & (~mask);
		bg = bg | u.A[0];
		bg = bg & (~mask);
		bh = bg | bh;
	      }
	   }
	  subobr[iz] = bh;
	  mask = 0;
	  iz++;
	  u.A[1] = cha;
	  if(neg == 0)
	   u.A[0] =  fnt_chr[off++];
	  else
	   u.A[0] = ~fnt_chr[off++];
	  cha = u.A[0];
	  j--;
	  goto Line;
	  End_line:
	  if(zbyte == 0) goto End_byte;
	  bh = subobr[iz];   // Jeste jeden cely
	  if(xg_chrmod == 0)
	   { bh = bh & mask;    // Prepis
	     bh = bh | u.A[0];
	   }
	  else
	   { if(neg == 1)
	      { bg = bh | mask;    // Jen popredi
		bg = bg & u.A[0];
		bg = bg | mask;
		bh = bh & bg;
	      }
	     else
	      { bg = bh & (~mask);
		bg = bg | u.A[0];
		bg = bg & (~mask);
		bh = bg | bh;
	      }
	   }
	  subobr[iz] = bh;
	  iz++;
	  u.A[1] = cha;
	  u.A[0] = 0;
	  u.AX = u.AX>>cl1;
	  mask = 0;
	  End_byte:           // Necely byte
	  //mask = mk;
	  mask = mask | mk;
	  bh = subobr[iz];
	  if(xg_chrmod == 0)
	   { bh = bh & mask;
	     bh = bh | u.A[0];
	   }
	  else                     // Jen popredi
	   { if(neg == 1)          // Nuluju jenicky
	      { bg = bh | mask;
		bg = bg & u.A[0];
		bg = bg | mask;
		bh = bg & bh;
	      }
	     else
	      { bg = bh & (~mask); // Nastavuju jednicky
		bg = bg | u.A[0];
		bg = bg & (~mask);
		bh = bg | bh;
	      }
	   }
	  subobr[iz] = bh;

	  iz = izz+dx8;
	  izz=iz;
	}
     }
    return( 1 );
}
#endif

// ========================================================================
// X_LOPIF 1 char output port to pure POSIX enviroment
// (c) 2000 Arachne Labs, based on X_LOPIF for DOS (c) Zdenek Harovnik
// ========================================================================

// ---------- Write one character from font into output buffer ------
// buffer subobr has always beginning [0,0] i.e. Y is always 0

#include "posix.h"
#include "v_putimg.h"
#include "x_lopif.h"    // For xg_chrmod

#ifdef VIRT_SCR
int xv_chr_mem(char *fnt_chr,       // Beginning of characters in font (binar)
                int  kx,                // Shift in X direction
                int  dx,                // Length of rows in pixels
                int  xfnt, int yfnt,    // Size of fonts in pixels
                int  color,int fillc,   // Colour of characters and background
                char *subobr,           // Picture with characters of text (OUT!)
                int  bin)               // bin=0/1 subobr byte/binary
{
    int nb,i,j,jx,ien,lenx,xx1,xx2,dx8,zbyte,off,neg;
    unsigned int  iz,izz;
    unsigned char mask,mz,mk,cha,cl1,bh,bg;
    union { unsigned short AX;
	        unsigned char A[2];
	} u;

    nb = ((xfnt-1)>>3) + 1; // Number of bytes in a row of font

    if(bin == 0)      //------------------- Byte- OBR
    {
     for(i=0; i<yfnt; i++)     // through rows
         {iz   = i*dx + kx;        // beginning of row 
	  lenx = xfnt;
          for(j=0; j<nb; j++)      // through bytes of row of font
	   { mask = fnt_chr[i*nb+j];
	     ien  = min(lenx,8);
             for(jx=0; jx<ien; jx++)  // through byte of font
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

      for(i=0; i<yfnt; i++)     // loop through rows
          {iz   = i*dx + kx;        // beginning of row (in 2B!)
	  lenx = xfnt;
          for(j=0; j<nb; j++)      // through bytes of row of fonts
	   { mask = fnt_chr[i*nb+j];
	     ien  = min(lenx,8);
             for(jx=0; jx<ien; jx++)  // through byte of font
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
       iz  = kx>>3;  // Byte of first row
       izz = iz;
       cl1 = kx&7;   // remainder after /8

       mz = (1<<(8-cl1))-1;      // Mask for first byte
       mz = ~mz;
       i  = ((kx+xfnt-1)&7) + 1; // Mask for last byte
       mk = (1<<(8-i))-1;
       off = 0;
       for(i=0; i<yfnt; i++)     // Loop through rows
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
           { bh = bh & mask;    // Overwrite
	     bh = bh | u.A[0];
	   }
	  else
	   { if(neg == 1)
              { bg = bh | mask;    // Only foreground
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
          bh = subobr[iz];   // Jeste jeden cely (tr.: still one whole)
	  if(xg_chrmod == 0)
           { bh = bh & mask;    // Overwrite
	     bh = bh | u.A[0];
	   }
	  else
	   { if(neg == 1)
              { bg = bh | mask;    // Only foreground
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
          End_byte:           // Necely byte (tr.: not a whole/less than one byte)
	  //mask = mk;
	  mask = mask | mk;
	  bh = subobr[iz];
	  if(xg_chrmod == 0)
	   { bh = bh & mask;
	     bh = bh | u.A[0];
	   }
          else                     // Only foreground
           { if(neg == 1)          // Nuluju jenicky (tr.: reset ones to zero)
	      { bg = bh | mask;
		bg = bg & u.A[0];
		bg = bg | mask;
		bh = bg & bh;
	      }
	     else
              { bg = bh & (~mask); // Nastavuju jednicky (tr.: Set ones)
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

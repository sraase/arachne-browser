#include "posix.h"

int bit_pix8(char *sourc, char *dest, int npix, int nbit)

/* sourc - pole s jednim radkem po bit. rovinach       */
/*         tr.: field with one line in bit levels      */
/* dest  - pole s jednim radkem pixlu                  */
/*         tr.: field with one row of pixels           */
/* npix  - pocet pixlu na radek                        */
/*         tr.: total number of pixels in a line       */
/* nbit  - pocet bitovych rovin  1..8                  */
/*         tr.: number of bit levels 1..8              */
{
   int i,j,k,nb,npix8;
   unsigned char c;

   memset(dest,0,npix);       /* delete/reset field dest   */
   npix8 = npix/8;
   nb = npix8*(nbit-1);

   for(j=0; j<nbit; j++)      /* loop through bit levels */
    {
     for(i=0; i< npix8; i++)
       { c =  sourc[i+nb];
	 for(k=0; k<8; k++)
	   { if(c & 0x80) dest[(i<<3) + k] |= 1<<j;
	     c = c<<1;
	   }
       }
      nb -= npix8;
     }
   return(1);
}

#include "posix.h"

int bit_pix8(char *sourc, char *dest, int npix, int nbit)

/* sourc - pole s jednim radkem po bit. rovinach       */
/* dest  - pole s jednim radkem pixlu                  */
/* npix  - pocet pixlu na radek                        */
/* nbit  - pocet bitovych rovin  1..8                  */
{
   int i,j,k,nb,npix8;
   unsigned char c;

   memset(dest,0,npix);       /* vynulovani pole dest   */
   npix8 = npix/8;
   nb = npix8*(nbit-1);

   for(j=0; j<nbit; j++)      /* Cykl pres bitove roviny*/
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

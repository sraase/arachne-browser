#include "posix.h"
#include "x_lopif.h"

//--------- mod prusvitneho/plneho textu ----------
int x_getcharmod(void)
{
  return(xg_chrmod);
}

/* ------------->>>>> Urci kolik znaku se vejde do zadane sirky */
int x_charmax(unsigned char *string, int dxpix)
{
   int len,px_len,i,maxch,shf;
   len = strlen(string);

   if(xg_foncon == 0)       // KONSTANTNI SIRE
     { // maxch =  dxpix / (len * xg_xfnt * xg_fnt_zoo);
       maxch =  dxpix / (xg_xfnt * xg_fnt_zoo);
       return( maxch );
     }
   else                     // PROMENNA SIRE
     { px_len = 0;
       shf    = xg_fnt_zoo>>1;
       for(i=0; i<len; i++)
	{ if((unsigned)string[i] < 32)
	   px_len += ((unsigned)string[i]<<shf);
	  else
	   px_len += (xg_fonlen[(unsigned)string[i]]<<shf);

	   if(px_len > dxpix)
	    { return( i );
	    }
	}
       return( len );
     }
}

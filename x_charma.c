#include "posix.h"
#include "x_lopif.h"

//--------- mod prusvitneho/plneho textu ----------
// tr.: mode of transparent/filled text
int x_getcharmod(void)
{
  return(xg_chrmod);
}

/* ------------->>>>> Urci kolik znaku se vejde do zadane sirky */
/* tr.: defines how many characters fit into a given width      */
int x_charmax(unsigned char *string, int dxpix)
{
   int len,px_len,i,maxch,shf;
   len = strlen(string);

   if(xg_foncon == 0)       // CONSTANT WIDTH
     { // maxch =  dxpix / (len * xg_xfnt * xg_fnt_zoo);
       maxch =  dxpix / (xg_xfnt * xg_fnt_zoo);
       return( maxch );
     }
   else                     // VARIABLE WIDTH
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


#include "posix.h"
#include "x_lopif.h"
/* ------------>>>>>> Vrati sirku textu v pixlech */
/* tr.: returns the width of text in pixels */

int x_txwidth(char *string)
{
   int len,px_len,i;

   if(!string)
    return 0;

   len = strlen(string);

   if(xg_foncon == 0)       // KONSTANTNI SIRE (tr.: constant width) 
     px_len = len * xg_xfnt * xg_fnt_zoo;  // Delka v pixlech
                            // tr.: length in pixels
   else                     // PROMENNA SIRE (tr.: variable width)
     { px_len = 0;
       for(i=0; i<len; i++)
	{ if((unsigned)string[i] < 32)
	   px_len += (unsigned char)string[i];
	  else
	   px_len += xg_fonlen[(unsigned char)string[i]];
	}
       px_len *= xg_fnt_zoo;
     }

   return( px_len );
}

/* ------------->>>>> Vrati vysku textu v pixlech */
/* returns the height of text in pixels */
int x_txheight(char *string)
{
  return( xg_yfnt * xg_fnt_zoo);
}

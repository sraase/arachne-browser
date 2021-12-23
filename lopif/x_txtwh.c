#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "x_lopif.h"
/* ------------>>>>>> Vrati sirku textu v pixlech */

int x_txwidth(char *string)
{
   int len,px_len,i;

   len = strlen(string);

   if(xg_foncon == 0)       // KONSTANTNI SIRE
     px_len = len * xg_xfnt * xg_fnt_zoo;  // Delka v pixlech
   else                     // PROMENNA SIRE
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
int x_txheight(char *string)
{
  return( xg_yfnt * xg_fnt_zoo);
}

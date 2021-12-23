#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <alloc.h>

#include "x_lopif.h"

int dealloc_xmem(int h);

//---------------- Uvolneni fontu z MEM,XMS, close DISK
int x_fnt_cls(void)
{
  int ist;

  if(xg_fbuf != NULL)         // Zahozeni fontu
    { farfree(xg_fbuf);
      xg_fbuf = NULL;
      xg_fnt_akt[0]=0;
      xg_fntalloc = 0;
      if(xg_fonmem == 1)      // Dealoc  XMS
       { //ist = dealloc_xmem(xg_fonhan);
	 //if(!ist) return( 2 );
       }
      else if(xg_fonmem == 2) // Close fontu na disku
       { //ist = close(xg_fonhan);
	 //if(ist != 0) return( 4 );
       }
    }

   if(xg_fnt_max > 0)         // Mame fonty v XMS
   {
      farfree(xg_fnt_xtab);
      xg_fnt_xtab = NULL;
      ist = dealloc_xmem(xg_fnt_xms);
      if(!ist) return( 2 );
      xg_fnt_xms = 0;
      xg_fnt_max = 0;
   }

   return( 1 );
}

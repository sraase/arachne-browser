#include <stdlib.h>
#include <stdio.h>

#include "x_lopif.h"

/*-------- Vrati nastavenou barvu cary (textu) ----------*/

int x_getcol(void)
{
   return(xg_color);
}
/*-------- Vrati nastavenou barvu vyplne (pozadi textu) -*/

int x_getfill(void)
{
   return(xg_fillc);
}

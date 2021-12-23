#include <stdlib.h>
#include <stdio.h>
#include "x_lopif.h"

//--------- nastaveni prusvitneho/plneho textu ----------
void x_charmod(int chrmod)
{
  xg_chrmod = chrmod;
}

/* ------------->>>>> Nastavi velikost textu (1x,2x,4x) */
void x_text_zoom(int zoom)
{
  if(zoom == 2)
   xg_fnt_zoo = 2;
  else if(zoom == 4)
   xg_fnt_zoo = 4;
  else
   xg_fnt_zoo = 1;
}


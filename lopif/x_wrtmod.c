/* Nastaveni psani (prepis/XOR) pro dalsi graficke funkce */

#include <stdlib.h>
#include <stdio.h>

#include "x_lopif.h"

void x_wrtmode(int wrtmod)
{
  unsigned char ch1;

   if(wrtmod == 0)
     xg_wrt = 0;
   else
     xg_wrt = 3;            // XOR

   if(xg_256 == MM_16)          // Jen proo 16 bar. mody
   {
   ch1 = xg_wrt;

   asm	mov  DX,3CEh         /* Rotate,AND,OR,XOR registr */
   asm	mov  AL,3
   asm	out  DX,AL
   asm  inc  DX
   asm	mov  AL,ch1
   asm	shl  AL,1
   asm	shl  AL,1
   asm	shl  AL,1
   asm	out  DX,AL
   }
}

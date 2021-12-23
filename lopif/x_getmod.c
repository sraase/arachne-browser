#include <stdlib.h>
#include <stdio.h>
#include <dos.h>

#include "x_lopif.h"
#define VIDEO 0x10

//--------- Vraci skutecne cislo graf. modu
int x_getgrfmode(void)
{
   int rmod;
   union REGS regs;

   regs.h.ah = 0xF;    /* Kontrola nastaveni */
   int86(VIDEO, &regs, &regs);
   rmod = regs.h.al & 0x7F;
   return( rmod );
}

//--------- Vraci moje interni cislo graf. modu
int x_getgrfinter(void)
{
  return(xg_intern);
}

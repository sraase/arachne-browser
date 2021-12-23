/* -------------------------------------------- */
/*        Namapovani 16 registru z 256          */
/* -------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <bios.h>

#define VIDEO 0x10

void x_map_pal(char *palette)   /* namapovani 16 pal.registru */
{
  union REGS in,out;
  struct SREGS segreg;

  in.h.ah = 0x10;
  in.h.al = 2;
  in.x.dx = FP_OFF(palette);
  segreg.es = FP_SEG(palette);
  int86x(0x10,&in,&out,&segreg);
}

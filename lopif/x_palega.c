#include <stdlib.h>
#include <stdio.h>

#include "x_lopif.h"

/*------ Prevod palety VGA (RGB) na EGA cislo -------*/
void pal_vga_ega(char *pal1, int *ega_col, int jak)

/* pal1 - pole s RGB slozkami barvy         */
/* ega_col - cislo EGA barvy                */
/* jak = 0 -> metoda zaokrouhleni           */
/*     = 1 -> spec EGA V pal1 2nejvys. bity(6,7) */
/*     = 2 -> ega pal. V pal1 2 bity (4,5)  */
{
  int color=0;
  unsigned  char M1=0x40, M2=0x80;
  unsigned  char N1=0x10, N2=0x20;

  if(jak == 1)                /* Spec. EGA pal. ve VGA (6,7 bit) */
  {
  color=        (((pal1[2]) & M2)>>7) ;
  color=color | (((pal1[2]) & M1)>>3) ;

  color=color | (((pal1[1]) & M2)>>6) ;
  color=color | (((pal1[1]) & M1)>>2) ;

  color=color | (((pal1[0]) & M2)>>5) ;
  color=color | (((pal1[0]) & M1)>>1) ;
  }
  else if(jak == 2)           /* Orezavani VGA na EGA (4,5 bit) */
  {
  color=        (((pal1[2]) & N2)>>5) ;
  color=color | (((pal1[2]) & N1)>>1) ;

  color=color | (((pal1[1]) & N2)>>4) ;
  color=color | (((pal1[1]) & N1)>>0) ;

  color=color | (((pal1[0]) & N2)>>3) ;
  color=color | (((pal1[0]) & N1)<<1) ;
  }
  else                       /* !!! Zaokrouhlovani EGA pal z VGA */
  {
  /* !!! Zaokrouhlovani EGA pal z VGA */
  if ((0x3f & pal1[2])>40) color = 9; else
  if ((0x3f & pal1[2])>24) color = 1; else
  if ((0x3f & pal1[2])>8)  color = 8; else
			   color = 0;
  if ((0x3f & pal1[1])>40) color = color + 18; else
  if ((0x3f & pal1[1])>24) color = color + 2; else
  if ((0x3f & pal1[1])>8)  color = color + 16; else
			   color = color;
  if ((0x3f & pal1[0])>40) color = color + 36; else
  if ((0x3f & pal1[0])>24) color = color + 4; else
  if ((0x3f & pal1[0])>8)  color = color + 32; else
			   color = color;
  }

  *ega_col = color;

}

/*------ Prevod EGA cisla barvy na RGB slozky ------*/
void pal_ega_vga(int ega_col, char *pal1)

/* ega_col - cislo ega barvy            */
/* pal1    - RGB slozky pro VGA         */
{
  pal1[0] = pal1[1] = pal1[2] = 0;

  pal1[2]  = (ega_col & 0x0001) << 5;   /* B */
  pal1[2] |= (ega_col & 0x0008) << 1;

  pal1[1]  = (ega_col & 0x0002) << 4;   /* G */
  pal1[1] |= (ega_col & 0x0010);

  pal1[0]  = (ega_col & 0x0004) << 3;   /* R */
  pal1[0] |= (ega_col & 0x0020) >> 1;

}

/*----- Nastaveni spec. EGA palety ve VGA palete */
void x_set_spc_ega(int flag)
{
  xg_flag = xg_flag & 0xFFFE;           /* Vynuluj bit 0 */
  if(flag != 0) xg_flag = xg_flag | 1;
}

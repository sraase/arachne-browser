/* Zkouseni lopifu */

#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <dos.h>
#include <bios.h>
#include <alloc.h>

#include "x_lopif.h"
#include "imouse.h"

unsigned  _stklen = 16000;

long mem_all_xmem(void);

int main(int argc, char *argv[])
{
   int ist;
   unsigned char filnam[64],pal[768], buf[1024],t_font[32];
   int  maxx,maxy,c,nr,filx;
   int  color;
   unsigned int len;
   int  izp,i,j,mod,ncol,nrow,npal,nx,xzz,yzz,xmmm,ymmm;
   long i44;
   int  flgm, xm, ym;

   union { char c1[1024];     /* Hlavicka .OBR */
	   int  c2[512];
	 } u;

   char patt[8]= {0xCC,0x33,0xCC,0x33,0xCC,0x33,0xCC,0x33};
   int  n_znk;
   int  ncolx;
   char spc[12],chbuf[128],txmod[16];

   unsigned int cur1[16] =
 { 0x9FFF, 0x0FFF, 0x07FF, 0x83FF, 0xC1FF, 0xE0FF, 0xF067, 0xF003,
   0xF001, 0xF000, 0xF800, 0xF800, 0xF800, 0xFC00, 0xFC00, 0xFC00};
   unsigned int cur2[16] =
 { 0x0000, 0x6000, 0x7000, 0x3800, 0x1C00, 0x0E00, 0x0700, 0x0018,
   0x07EC, 0x07EE, 0x001E, 0x03EE, 0x03EE, 0x001E, 0x00EC, 0x0000 };

   //---- Zkouseni XMS (kolik jsem naalokoval)
   ist = SetXmsEms( 1 );
   nr = alloc_xmem(100);
   ist = dealloc_xmem( nr );

   //-----------------------------------------------
   x_detect(txmod, &n_znk);

   printf("-------->>>> Zkouseni X_lopifu (virt screen) <<<<<--------\n\n");
   // JdS 2004/10/16 {
   //printf("Graf.mod EGA,VGA,VGA2,OAK.B,HI16.I,... : ");
   printf("Mode EGA,VGA,VGA2,TSG3,OAK,TAMARA,TRIDENT,REALTEK,TSG4,M1,VESA,BCGA: ");
   // JdS 2004/10/16 }
   scanf ("%s",txmod);

  /*-----------------------------------------------*/
  ist = x_rea_svga("",txmod,&mod);
  if((ist&1)==0)
    { printf("Problemy s SVGA.SET num = %hd \n",ist);
      return(-1);
    }

printf("intrmode = %u, ist = %u, mod = %Xh\n", xg_intern, ist, mod);
delay(2000);

  ist = x_grf_mod(mod);
  if((ist&1) == 0)
    { printf("Mod %02x nelze nastavit ! \n",mod);
      return(-1);
    }

  // prepneme vystupy z xlopifu do virt screen
  ist = 1;
  //ist = 0;
  //x_video_XMS(ist, 0);

  //x_palett(npal, pal);  // ?? zapisuje se do skutecne videoram ??

  // vytvorime nejake vystupy z xlopifu
  x_setfill(1,11);
  x_bar(100,100,255,190);

  xmmm = x_maxx();
  ymmm = x_maxy();

  x_line(300,300,400,400);
  x_circle(xmmm/2,ymmm/2,150);
  x_setfill(1,4);
  x_bar(0,0,300,150);
  x_setfill(1,15);
  x_bar(xmmm-100,0,xmmm,ymmm);

  for(i=0; i<200; i++)
    for(j=0; j<200; j++)
      x_putpix(j,i,j);

  //--- Prepnu na prime kresleni
  x_video_XMS(0, 0);

  //---- Zrusim virt videoram
#ifdef VIRT_SCR
  c=bioskey(0);
  ist = xv_cls_virt(0,0);
#endif
  /*------------------------------------------*/
  // JdS 2004/10/10 {
  ist = x_fnt_cls();   // text mode
  ist = x_grf_mod(3);  // tidy-up

  printf("%u %u, bpl=%u", xg_view[2], 0);
  return(0);           // normal exit

  // JdS 2004/10/10 }
  End_all:    // text mode
  ist = x_fnt_cls();
  return(1);
}

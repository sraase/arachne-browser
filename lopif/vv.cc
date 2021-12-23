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

   char *muj_t = {"Muj_text_ !BABABABBABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABAB@"};
   char *bio_t = {"Bio_text_ABABABABBABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABAB@"};
   //char patt[8]= {0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55};
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

#ifdef VIRT_SCR
   ist = xv_new_virt("BUBU1", // File name for disk file
		    128, 128,  // Size in pixels
		    0,         // Default color
		    0,         // 3-1bit/pixel, 0-8bit/pixel, -1-16bit/pixel
		    256,       // Length of palette
		    pal,       // Palette, range RGB 0..63, max. 256 entries
		    0,         // Index 0..5, number of virtual videoram
		    0);        // 0-XMS or DISK, 1-XMS, 2-DISK

   ist = xv_new_virt("BUBU2", // File name for disk file
		    128, 128,  // Size in pixels
		    0,         // Default color
		    0,         // 3-1bit/pixel, 0-8bit/pixel, -1-16bit/pixel
		    256,       // Length of palette
		    pal,       // Palette, range RGB 0..63, max. 256 entries
		    1,         // Index 0..5, number of virtual videoram
		    0);        // 0-XMS or DISK, 1-XMS, 2-DISK

   ist = xv_new_virt("BUBU3", // File name for disk file
		    128, 128,  // Size in pixels
		    0,         // Default color
		    0,         // 3-1bit/pixel, 0-8bit/pixel, -1-16bit/pixel
		    256,       // Length of palette
		    pal,       // Palette, range RGB 0..63, max. 256 entries
		    2,         // Index 0..5, number of virtual videoram
		    0);        // 0-XMS or DISK, 1-XMS, 2-DISK

   ist =  xv_cls_virt(0,        // 0-only free XMS, 1-write XMS on disk
		      2);       // number of virtual videoram
   ist =  xv_cls_virt(0,        // 0-only free XMS, 1-write XMS on disk
		      1);       // number of virtual videoram
   ist =  xv_cls_virt(0,        // 0-only free XMS, 1-write XMS on disk
		      0);       // number of virtual videoram
   i44 = mem_all_xmem();

#endif
   //-----------------------------------------------
   x_detect(txmod, &n_znk);

   printf("-------->>>> Zkouseni X_lopifu (virt screen) <<<<<--------\n\n");
   // JdS 2004/10/16 {
   //printf("Graf.mod EGA,VGA,VGA2,OAK.B,HI16.I,... : ");
   printf("Mode EGA,VGA,VGA2,TSG3,OAK,TAMARA,TRIDENT,REALTEK,TSG4,M1,VESA,BCGA: ");
   // JdS 2004/10/16 }
   scanf ("%s",txmod);

   /*--------------Soubor s obrazkem ---------------*/
   filnam[0]=0;
   printf("File with graphic [.OBR] : ");  // JdS 2004/10/16 (translation)
   gets(filnam);
   gets(filnam);
   if(filnam[0] == 0) strcpy(filnam,"OBR.OBR");
   if(strchr(filnam,'.') == NULL) strcat(filnam,".OBR");

   filx=open(filnam,O_BINARY|O_RDONLY);
   if(filx <= 0) { perror("File ");
		   return( -1 );
		 }
   nr=read(filx,u.c1,2);
   if(u.c2[0] == 0)
     len = 512;
   else
     len = u.c2[0];

   i44 = lseek(filx,0L,SEEK_SET);
   nr=read(filx,u.c1,len);

   ncol =u.c2[3];   // Zaokr
   ncolx=u.c2[33];  // Skutec.
   nrow=u.c2[4];
   npal=u.c2[6];

   if(npal <= 16)
     izp = 16;
   else
     izp = 254;

   for(i=0;  i<npal*3;  i += 3)      // Paleta v poli pal()
    { pal[i  ] = u.c1[izp+i] ;
      pal[i+1] = u.c1[izp+i+1];
      pal[i+2] = u.c1[izp+i+2];
    }

   // JdS 2004/10/16 : Added ".FNT" file extension code ...
   printf("Text font [.FNT] : ");
   t_font[0]=0;
   gets(t_font);
   if (strchr(t_font,'.') == NULL)
     strcat(t_font,".FNT");


  /*-----------------------------------------------*/
  ist = x_rea_svga("",txmod,&mod);
  if((ist&1)==0)
    { printf("Problemy s SVGA.SET num = %hd \n",ist);
      return(-1);
    }

  ist = x_grf_mod(mod);
  if((ist&1) == 0)
    { printf("Mod %02x nelze nastavit ! \n",mod);
      return(-1);
    }

  ist = x_fnt_alloc( 20000 );
  ist = x_fnt_initxms( 3 );

   if(t_font[0] != 0)
    { ist = x_fnt_load(t_font,30,1);
      if(ist < 0)
      { printf("Error in load font !\n");
	goto End_all;
      }
    }

  //------- Vytvorime virt videoram a budeme psat do ni
  /*
  ist = xv_new_virt("Pomocny.obr", // File name for disk file
		    640, 1000,     // Size in pixels dx,dy
		    0,             // Default color
		    0,             // 3-1bit/pixel, 0-8bit/pixel, -1-16bit/pixel
		    npal,          // Length of palette
		    pal,           // Palette, range RGB 0..63, max. 256 entries
		    0);            // Index 0..2, index of virtual videoram
  if((ist&1)==0)
  { printf("Error in create virt screen\n");
    goto End_all;
  }
  */
  // Nastavit aktivni videoram (neni treba, nastavi se v xv_new_virt() )
  // int xv_set_actvirt(int Index);    // 0..2, default 0

  // prepneme vystupy z xlopifu do virt screen
  ist = 1;
  //ist = 0;
  //x_video_XMS(ist, 0);

  //x_palett(npal, pal);  // ?? zapisuje se do skutecne videoram ??

  // vytvorime nejake vystupy z xlopifu
  x_setfill(1,11);
  x_bar(100,100,255,190);

  // Cviceni s fonty
  x_text_ib(0,1*xg_yfnt,"Sla Nanynka do zeli, do zeli!");
  x_text_zoom( 2 ) ;
  x_text_ib(0,2*xg_yfnt,"Sla Nanynka do zeli, do zeli!");
  x_text_zoom( 4 );
  x_text_ib(0,4*xg_yfnt,"Sla Nanynka do zeli, do zeli!");
  x_text_zoom( 1 );

  ist=x_fnt_load("A11.fnt",30,1);
  x_text_ib(0,2*xg_yfnt,"Sla Nanynka do zeli, do zeli!");

  ist=x_fnt_load("A12.fnt",30,1);
  x_text_ib(0,3*xg_yfnt,"Sla Nanynka do zeli, do zeli!");

  ist=x_fnt_load(t_font,30,1);
  x_text_ib(0,100,"Sla Nanynka do zeli, do zeli!");

  ist=x_fnt_load("A12.fnt",30,1);
  x_text_ib(0,120,"Sla Nanynka do zeli, do zeli!");

  ist=x_fnt_load("A13.fnt",30,1);
  x_text_ib(0,150,"Sla Nanynka do zeli, do zeli!");

  x_line(300,300,400,400);
  x_circle(200,200,150);

  xmmm = x_maxx();
  ymmm = x_maxy();

  x_setfill(1,4);
  x_bar(0,0,300,150);
  x_setfill(1,15);
  x_bar(300,0,600,ymmm);

  for(i=0; i<200; i++)
   { for(j=0; j<200; j++)
	x_putpix(j,i,j);
   }

  x_setcolor(11);
  x_setfill(1,2);
  x_bar(0,0,100,50);
  x_rect(0,0,100,50);

  x_setpattern(patt,2);
  x_bar(100,50,200,100);

   x_setcircf(0);
   x_circle(100,100,40);

   nrow = min(nrow,maxy+1);
   nx = min(ncol,maxx+1);

   xzz = 20; yzz = 400;
   for(i=0; i<nrow; i++)
   {
    ist = read(filx, buf, ncolx);
    wrt_video(buf, xzz, yzz, ncolx, 1, 1);
    yzz++;
   }

   x_setcolor( 1 );
   x_setfill(1,0);
   x_charmod( 1 );       // Pruhledne pozadi!

   for(i=0; i<10; i++)
   { x_text_ib(i,i*xg_yfnt,"Sla Nanynka do zeli, do zeli!");
   }

  /*--------- Zkouseni kursoru ------------*/
  x_setcolor( 128 );
  ImouseIni( 0, 0, xg_view[2], xg_view[3], 0, 0);
  color = 155;
  x_defcurs(cur1,cur2,color);
  x_setfill(1,color);
  x_yncurs(1,0,40,color);  // Init meho kurzoru

  for(i=0; i<640; i += 1)
   { delay(10);
     x_cursor(i,40);
   }

  Okolo:
  flgm = ImouseRead(&xm,&ym);       // Mys
  x_cursor(xm,ym);
  if(flgm == 0) goto Okolo;
  x_yncurs(0,0,0,0);   // vypnuti

  //---- Copy virt videoram -> screen

  /**
  ist = xv_to_scr(0,0,0,0,640,480);
  c=bioskey(0);
  ist = xv_to_scr(0,480,0,0,640,480);
  c=bioskey(0);
  ist = xv_to_scr(0,0,0,0,640,480);
  **/

  //--- Prepnu na prime kresleni
  x_video_XMS(0, 0);

  //--- Neco nakreslim
  x_rect(50,50,200,200);

  //---- Zrusim virt videoram
#ifdef VIRT_SCR
  c=bioskey(0);
  ist = xv_cls_virt(0,0);
#endif
  /*------------------------------------------*/
  // JdS 2004/10/10 {
  ist = x_fnt_cls();   // text mode
  ist = x_grf_mod(3);  // tidy-up
  return(0);           // normal exit
  // JdS 2004/10/10 }

  End_all:    // text mode
  ist = x_fnt_cls();
  // ist = x_grf_mod( 3 );
  return(1);
}

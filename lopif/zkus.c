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
//void x_defcross(int dx, int dy, int typ);
//void x_cross_cur(int x, int y);
//void x_cross_on(int on, int x, int y, int col);
void x_pal_1hw(int n_pal, char *pal_1);
void x_pal_1ret(int n_pal, char *pal_1, long *Nc, long *Zc);

struct   { char sigBMP[2];                  // Ze jde o BMP  "BM" -- File_header
	   long size;                       // Velikost souboru
	   char resWin[4];                  // Reserva Windows
	   long offBit;                     // Zacatek obrazu

	   int  zn28;                       // Cislo 28 (delka) -- INFOHEADER
	   int  nic1;
	   int  dx_bm;                      // pocet sloupcu
	   int  nic2;
	   int  dy_bm;                      // pocet radku
	   int  nic3;                       //
	   int  rovin;                      // pocet rovin 1
	   int  bit_pix;                    // bitu na pixel
	   long compress;                   // komprese
	   long sizeimg;                    // velikost obrazu
	   long Xpelmet;                    // Rozliseni
	   long Ypelmet;
	   long ClrUsed;                    // Pocet pouzitych barev
	   long CltImport;                  // Pocet dulezitych barev
	 } bmp_hed;                         // Sum = 54bytu


#define VIDEO 0x10

unsigned  _stklen = 16000;


typedef struct
  {
  unsigned long length;         /* velikost prenasene pameti      */
  unsigned int sourceH;         /* handle pro zdroj (0=konvencni  */
  unsigned long sourceOff;      /* offset zdroje pameti           */
  unsigned int destH;           /* handle pro terc (0=konvencni   */
  unsigned long destOff;        /* offset terce pameti            */
  } XMOVE;

int get_xmem(void);             // Fce pro zachazeni s EMS,XMS
int alloc_xmem(int n);          // Alokuje n KB pameti, vraci handle
int dealloc_xmem(int h);        // Dealokuje EMS,XMS (handle)
int move_xmem(XMOVE *p);        // presune blok z/do XMS,EMS
int h_xmove(XMOVE *p);

int mem_xmem(unsigned *total, unsigned *free);
long ptr2long(char *p);


int main(int argc, char *argv[])
{
   int ist;
   unsigned char filnam[64],pal[768], *buf,t_font[32];
   unsigned char bmp_pal[1024];
   //char map[17]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0};
   int  maxx,maxy,c,xr,nr,filx;
   int  x1,y1,x2,y2,color;
   unsigned int len;
   int  izp,i,j,k,mod,ncol,nrow,npal,nx,xzz,yzz,xmmm,ymmm;
   long i44;
   union REGS in;
   //char  *bubu;
   char   mess[80];
   XMOVE  xmv;
   int    hem,kb,Bit13;

   union { char c1[1024];     /* Hlavicka .OBR */
	   int  c2[512];
	 } u;

   char *muj_t = {"Muj_text_ !BABABABBABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABAB@"};
   char *bio_t = {"Bio_text_ABABABABBABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABAB@"};
   //char patt[8]= {0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55};
   char patt[8]= {0xCC,0x33,0xCC,0x33,0xCC,0x33,0xCC,0x33};
   int  n_znk;
   int  xra,yra,zx1,zy1,ncolx;
   unsigned mm1,mm2,mm3;
   char z_bila[3] = {63,63,63};
   //char get_buf[2000];
   //unsigned int his[256];
   char spc[12],chbuf[128],txmod[16];
   unsigned char pal1[3];
   long     Nc,Zc;
   unsigned char *ibuf, *obuf;
   int      Nrow,NrowAll,LenLin,Dx,Dy,ir;
   unsigned LenBuf;

   unsigned int cur1[16] =
 { 0x9FFF, 0x0FFF, 0x07FF, 0x83FF, 0xC1FF, 0xE0FF, 0xF067, 0xF003,
   0xF001, 0xF000, 0xF800, 0xF800, 0xF800, 0xFC00, 0xFC00, 0xFC00};
   unsigned int cur2[16] =
 { 0x0000, 0x6000, 0x7000, 0x3800, 0x1C00, 0x0E00, 0x0700, 0x0018,
   0x07EC, 0x07EE, 0x001E, 0x03EE, 0x03EE, 0x001E, 0x00EC, 0x0000 };

  int     xm,ym,flgm,SimulGRF;
/***
  spc[0] = 0xA0;
  spc[1] = 0x87;
  spc[2] = 0x83;
  spc[3] = 0x82;
  spc[4] = 0x88;
  spc[5] = 0xA1;
  spc[6] = 0x8D;
  spc[7] = 0xA4;
  spc[8] = 0xA2;
  spc[9] = 0x91;
  spc[10]= 0x92;
***/
  spc[0] = 0x1;
  spc[1] = 0x2;
  spc[2] = 0x3;
  spc[3] = 0x4;
  spc[4] = 0x5;
  spc[5] = 0x6;
  spc[6] = 0x7;
  spc[7] = 0x8;
  spc[8] = 0x9;
  spc[9] = 0x10;
  spc[10]= 0x11;
  spc[11]= 0;

  if(argc > 1)
   sscanf(argv[1],"%d",&SimulGRF);
  else
   SimulGRF = 0;

  x_detect(txmod, &n_znk);

  printf("----------->>>> Zkouseni X_lopifu <<<<<-----------------\n\n");
  printf("Graf.mod EGA,VGA,VGA2,OAK.B,HI16.I,... : ");
  scanf ("%s",txmod);

   /*--------------Soubor s obrazkem ---------------*/

   filnam[0]=0;
   printf("Soubor s obrazkem [.BMP] : ");
   gets(filnam);
   gets(filnam);
   if(filnam[0] == 0) strcpy(filnam,"OBR.OBR");
   if(strchr(filnam,'.') == NULL) strcat(filnam,".BMP");

   /******
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

   for(i=0;  i<npal*3;  i += 3)    // Paleta v poli pal()
    { pal[i  ] = u.c1[izp+i] ;
      pal[i+1] = u.c1[izp+i+1];
      pal[i+2] = u.c1[izp+i+2];
    }

   for(i=npal*3;  i<768;  i += 3)    // Nepouzite vynulovat
    { pal[i  ] = 0;
      pal[i+1] = 0;
      pal[i+2] = 0;
    }

 *******/

/*-----------------------------------------------*/
  ist = x_rea_svga("",txmod,&mod);
  if((ist&1)==0)
    { printf("Problemy s SVGA.SET num = %hd \n",ist);
      return(-1);
    }

  ist = 0;
  x_setnomode( ist );
  ist = x_grf_mod(mod);
  if((ist&1) == 0)
    { printf("Mod %02x nelze nastavit ! \n",mod);
      return(-1);
    }

  x_getratio(&xra,&yra);

  //------- Pro ladeni VESA -------
  x_setfill(1,11);
  x_bar(100,100,255,190);

/*------------------------------------------------*/
  printf("Text font : ");
  t_font[0]=0;
  gets(t_font);
  //gets(t_font);
  ist = x_fnt_alloc( 16000 );
  if(t_font[0] != 0)
    { ist = x_fnt_load(t_font,30,1);
      if(ist < 0) return(-1);
    }

  strcpy(spc,"Anicka");
  x_setfill(1,0);
  x_setcolor(1);
  x_charmod( 0 );
  x_text_ib(16,10,spc);

  ist = 1;
  if(ist == 0)
   strcpy(chbuf,"aeiorauiruaeior"); // 15
  else
   strcpy(chbuf," ‚¡“© ‚¡“© ‚¡“©");
  ist = x_charmax(chbuf, 80);
  chbuf[ist] = 0;
  x_text_ib(100,100,chbuf);

  x_getimg(16,10,31,23, mess);  // read "A" 8x14

  for(i=0; i<16; i+=2)
  { x_putimg(16+i,10+i,mess,0);
  }

  x_text_zoom(2);
  x_text_ib(80,80,"MOST");
  x_text_zoom(1);

  c=bioskey(0);

  xmmm = x_maxx();
  ymmm = x_maxy();

  // Kresleni BMP
  /****
   filx = open(filnam,O_BINARY|O_RDONLY);
   if(filx <= 0)
     { return 22;
     }
   nr = read(filx,&bmp_hed,54);  // Nacteni hlavicky .BMP
   if(nr < 54) return 24;

   if(bmp_hed.bit_pix < 24)      // Read pal
   {
      if(bmp_hed.ClrUsed > 0)
       npal = bmp_hed.ClrUsed;
      else
       npal = 1<<bmp_hed.bit_pix;
      nr = read(filx,bmp_pal,npal*4);  // Nacteni palety .BMP
      if(nr < npal*4) return( 26 );
      memset(pal,0,768);
      k=0;
      for(i=0; i<npal*3; i += 3)
	{ pal[i+2] = bmp_pal[k  ]>>2;
	  pal[i+1] = bmp_pal[k+1]>>2;
	  pal[i  ] = bmp_pal[k+2]>>2;
	  k += 4;
	}
      x_palett(256,pal);
      Bit13 = 1;
   }
   else
   {  Bit13 = -3;
   }
   lseek(filx,SEEK_SET,bmp_hed.offBit);    // Zacatek bitmapy

   ist = (int)((long)bmp_hed.dx_bm * (long)bmp_hed.bit_pix / 8);
   if(ist & 3) ist = (ist/4 * 4) + 4;
   LenLin = ist;
   Nrow   = 32;
   LenBuf = LenLin * Nrow;
   ibuf = farmalloc((long)LenBuf);
   if(ibuf == NULL) return 44;

   Dx = bmp_hed.dx_bm;
   Dy = bmp_hed.dy_bm;

#if HI_COLOR
   xh_SetRounding( 1 );    // Nas. 4b pro BMP
#endif
   NrowAll = 0;

   for(i=0; i<100; i++)
   {
   ir = read(filx,ibuf,LenBuf);
   if(ir <= 0) goto End_x1;
   Nrow = ir / LenLin;
   NrowAll += Nrow;

   y1 = Dy - NrowAll;
   ist = wrt_video(ibuf, 0, y1, Dx, -Nrow, Bit13);  // RGB, y-opacne
   }

   End_x1:
   obuf = farmalloc((long)Dx*2*32+8);  // +8 getimg
   if(obuf == NULL) return 44;

   x_getimg(0, 0, Dx-100, 31, obuf);  // x1,y1,x2,y2
   x_putimg(88, 100, obuf, 0);
   x_putimg(99, 200, obuf, 0);

   farfree(obuf); farfree(ibuf);
   i = bioskey(0);
  ****/

  /******
  buf = (char *) malloc(20000L);
  c = 1;
  ist = SetXmsEms( c );     // ##### pouziti XMS,EMS
  if(ist != 1) { printf("ERROR1 EMS,XMS\n"); return 1; }
  ist = mem_xmem(&xzz, &yzz);
  if(ist != 1) { printf("ERROR2 EMS,XMS\n"); return 1; }
  ist = get_xmem();
  kb = (int)(((long)ncol * (long)nrow) / 1024 + 1);
  hem = alloc_xmem(kb);         // Alokuje n KB pameti, vraci handle
  if(hem == -1) { printf("ERROR3 EMS,XMS\n"); return 1; }

  xzz = 0; yzz = 0;
  xmv.length = ncol;
  xmv.sourceH = 0;
  xmv.sourceOff = ptr2long(buf);
  xmv.destH = hem;
  xmv.destOff = 0L;

  for(i=0; i<nrow; i++)
  { ist = read(filx,buf, ncol);
    ist = h_xmove(&xmv);
    if(ist != 1) { printf("ERROR4 EMS,XMS\n"); return 1; }
    xmv.destOff += ncol;
  }

  xmv.length = ncol * 10;
  xmv.sourceH = hem;
  xmv.sourceOff = 0L;
  xmv.destH = 0;
  xmv.destOff = ptr2long(buf);
  ********/

  //x_palett(256, pal);

  /*****
  for(i=0; i<nrow/10; i++)   // z EMS po 10
  {
    ist = h_xmove(&xmv);
    if(ist != 1) { printf("ERROR5 EMS,XMS\n"); return 1; }
    wrt_video(buf, xzz, yzz, ncolx, 10, 1);
    xmv.sourceOff += (ncol*10);
    yzz += 10;
  }

  ist = mem_xmem(&xzz, &yzz);
  ist = dealloc_xmem(hem);        // Dealokuje EMS (handle)
  c = bioskey(0);
  ****/

  /***
   pal1[0] = 63; pal1[1] = 0; pal1[2] = 0;
   x_pal_1(3, pal1);
   x_setfill(1,3);
   x_bar(0,0,xmmm,ymmm);
   c=bioskey(0);

   // Nastavovani palety v cyklu:
   for(i=0; i<62; i++)
   { pal1[0] -= 1;
     pal1[1] += 1;
     x_pal_1ret(3,pal1,&Nc,&Zc);
     delay(50);
   }
   c = bioskey(0);
   ***/

   if(xg_intern >= 6)
   {
   x_setfill(1,4);
   x_bar(0,0,300,150);
   x_setfill(1,15);
   x_bar(300,0,600,ymmm);
   }

  ist = 1;
  for(i=0; i<xmmm; i++)
   { for(j=0; j<ymmm; j++)
	x_putpix(i,j,ist);
   }

  for(i=0; i<200; i++)
   { for(j=0; j<200; j++)
	x_putpix(j,i,j);
   }
  c=bioskey(0);

  ist = -1;
  ist = x_getpix(100,100);

  x_wrtmode(1);
  ist = 1;
  for(i=0; i<xmmm/2; i++)
   { for(j=0; j<ymmm/2; j++)
	x_putpix(i,j,ist);
   }
  x_wrtmode(0);

   x_setcolor(11);
   x_setfill(1,2);
   x_bar(0,0,100,50);
   x_rect(0,0,100,50);

   x_setpattern(patt,2);
   x_bar(100,50,200,100);

   //x_setcircf(1);
   //x_setpattern(patt,4);
   //x_circle(100,100,40);

   x_setcircf(0);
   x_circle(100,100,40);

   //x_bar(100,100,400,199);

   x_setfill(1,2);
   c = bioskey(0);

   //x_getpalette(rea_pal);          /* Nacteni palety    */
   x_setcolor( 11 );

   nrow = min(nrow,maxy+1);
   nx = min(ncol,maxx+1);

   x_setcolor( 0 );
   x_setfill(1,1);
   x_line(50,50, 639, 199);

   //ist = read(filx,buf, ncol/8*nrow);
   xzz = 0; yzz = 0;
   //wrt_video(buf, xzz, yzz, ncolx, nrow, 8);
   //wrt_bincga(buf, xzz, yzz, ncolx, nrow, 0x08, 0);

   x_setcolor( 1 );
   x_setfill(1,0);
   x_charmod( 1 );       // Pruhledne pozadi!

   for(i=0; i<10; i++)
   { x_text_ib(i,i*xg_yfnt,"Sla Nanynka do zeli, do zeli!");
   }

   c=bioskey(0);

   x_setcolor( 0 );
   x_setfill(1,1);
   //wrt_bincga(buf, xzz, yzz, ncolx, nrow, 0x00, 0);
   for(i=0; i<10; i++)
   { x_text_ib(i,i*xg_yfnt,"Sla Nanynka do zeli, do zeli!");
   }
  //goto End_3;

  /*--------- Zkouseni kursoru ------------*/
  /***
  in.x.ax = 0;            // Init mysi
  int86(0x33,&in,&in);

  in.x.ax = 2;            // Disable kurzoru
  int86(0x33,&in,&in);

  in.x.ax = 7;            // Rozsah X Y
  in.x.cx = 0;
  in.x.dx = x_maxx();
  int86(0x33,&in,&in);

  in.x.ax = 8;
  in.x.cx = 0;
  in.x.dx = x_maxy();
  int86(0x33,&in,&in);
  */

  color = 155;
  x_defcurs(cur1,cur2,color);
  x_setfill(1,color);
  x_yncurs(1,0,40,color);  // Init meho kurzoru

  for(i=0; i<640; i += 1)
   { delay(10);
     x_cursor(i,40);
   }
  x_yncurs(0,0,0,0);   // vypnuti

  /***
  in.x.ax = 4;            // Set kursor
  in.x.cx = 180;
  in.x.dx = 100;
  int86(0x33,&in,&in);
  ***/

  x_setcolor( 128 );
  ImouseIni( 0, 0, xg_view[2], xg_view[3], 0, 0);

  //---- Cross kursor
  printf("Typ cross dx dy : ");
  scanf("%d%d%d", &ist, &i, &j);
  if(ist < 1 || ist > 2) goto End_cros;

  x_defcross(i, j, ist);
  x_cross_on(1, 0, 0, 155);

  //x_yncurs(1,0,0,color);  // Init meho kurzoru

  Okolo:
  flgm = ImouseRead(&xm,&ym);       // Mys
  //x_cross_cur(xm,ym);
  x_cursor(xm,ym);
  if(flgm == 0) goto Okolo;
  delay(500);

  End_cros:
  x_yncurs(0,0,0,0);   // vypnuti

  //x_cross_on(0, 0, 0, 0);
  //goto Dalsi_cross;
  //End_cros:

  /*------------------------------------------*/
  x_wrtmode(1);
  zx1 = 0; zy1 = 0;
  xm = 0; ym = 0;
  //x_yncurs(1,xm,ym,color);  // Init meho kurzoru
  x_rect(xm,ym,120+xm,100+ym);

  Okolo2:
  flgm = ImouseRead(&xm,&ym);       // Mys
  if(xm != zx1 || ym != zy1)
   { x_rect(zx1,zy1,120+zx1,100+zy1);  // smazat
     x_rect(xm,ym,120+xm,100+ym);      // na nove misto
     //x_cursor(xm,ym);
     zx1 = xm; zy1 = ym;
   }
  //delay(10);
  if(flgm == 0) goto Okolo2;

  //x_yncurs(0,xm,ym,color);  // Close
  x_wrtmode(0);
  /****
    x_setview(0,0,x_maxx(),x_maxy(),0);
    zx1 = xmmm - 120;
    zy1 = ymmm - 120;
    x_putimg(zx1,zy1,buf,0);
    x_rect(zx1,zy1,zx1+(x2-x1),zy1+(y2-y1));

    if(xg_intern >= 6)
    {
    c=bioskey(0);
    x_putimg(100,480,buf,0);
    x_putimg(220,500,buf,0);
    x_putimg(330,511,buf,0);
    x_putimg(440,512,buf,0);
    x_putimg(550,513,buf,0);

    c=bioskey(0);
    x_getimg(zx1,zy1,zx1+100,zy1+100,buf);
    x_putimg(500,0,buf,0);
    x_putimg(600,500,buf,0);

    c=bioskey(0);
    x_getimg(220,500,320,600,buf);
    x_putimg(900,500,buf,0);
    }
****/
  End_3:
  c=bioskey(0);
  c = (c & 0x00FF);
  if(c == 27)
  { x_getpalette(pal);          /* Nacteni palety    */
  }
  ist = x_grf_mod( 3 );
  return(1);
}

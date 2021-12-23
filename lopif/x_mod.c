/*-------------------------------------------------*/
/*	    Nastaveni grafickeho modu		   */
/*-------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <string.h>

#include "vesa.h"
#include "x_lopif.h"

/*------------ Glob. prom. x_lopifu --------------*/
/* Pozn: xg_curs[256] - tvar kursoru v x_cursor() */

 int xg_mod;		       /* Graf. mod   */
 int xg_256;		       /* 0/1 nastven 256 barevny mod */
 int xg_color;		       /* Nastavena barva (rect,text) */
 int xg_fillc;		       /* Nastavena barva pro x_bar() */
 int xg_wrt;		       /* 0=prepis, 1=XOR	      */
 int xg_style;		       /* Definice cary 	      */
 int xg_xr,xg_yr;	       /* ratio 		      */
 int xg_view[4];	       /* Viewport		      */
 int xg_xfnt,xg_yfnt;	       /* Velikost fontu v pixlech    */
 int xg_tjustx; 	       /* Zarovnani textu ve smeru X  */
 int xg_tjusty; 	       /* Zarovnani textu ve smeru Y  */
 int xg_clip;		       /* Orezavani ve viewportu      */
 int xg_fbyt;		       /* Pocet bytu na znak fontu    */
 int xg_flag;		       /* Priznaky: bit 0 - spec EGA pal */
			       /*	    bit 1 - aktivni patt */
 unsigned char xg_upatt[8];    /* Vzorek 8x8 pro vyplnovani   */
 char *xg_fbuf=NULL;	       /* bufffer na font-alloc dle fontu*/
#if HI_COLOR
 char xg_curs[520];	       /* Image pod kursorem	      */
#else
 char xg_curs[260];	       /* Image pod kursorem	      */
#endif
 char xg_f_cur=0;
 unsigned int xg_c_col;        /* On/Off,barva kurzoru	   */
 int	  xg_x_cur,xg_y_cur;   /* Posledni souradnice	   */
 int	  xg_s1[16] =  {0xFFFF,0xFC3F,0xFC3F,0xFC3F, /* Screen mask */
					  0xFC3F,0xF00F,0x0000,0x0000,
					  0x0000,0x0000,0xF00F,0xFC3F,
					  0xFC3F,0xFC3F,0xFC3F,0xFFFF};
 int	  xg_s2[16] =  {0x0000,0x0000,0x0180,0x0180, /* Cursor mask  */
					  0x0180,0x0180,0x07E0,0x7FFE,
					  0x7FFE,0x07E0,0x0180,0x0180,
					  0x0180,0x0180,0x0000,0x0000};
 int xg_svga;
 int xg_intern;
 unsigned int xg_and16[68];    // Masky pro 16 bar. cursor.
 unsigned int xg_or16[68];
 int   xg_fnt_zoo = 1;
 int   xg_col_plan;	       // Pocet bitovych rovin 1..4 (impl.4)
						 // Getimg,Putimg,Imgsiz
 int xg_notview;	       // Viewport 0-ano, 1-ne

struct VESA_0 xg_vesa;	       // JdS 2004/09/26

char  xg_egapal[17];	       // Paleta pro EGU;

//---- Novinky 930319 pro WINDOWS fonty ------------------------
unsigned char xg_fonlen[256];  // Sirky znaku pro proporcni fonty
long  int     xg_fonadr[256];  // Zacatky znaku pro prop. fonty
unsigned char xg_foncon=0;     // Flag - konst/prop font [0/prumer]
unsigned char xg_fonmem=0;     // Kde je font: 0-MEM(xg_fbuf),1-XMS,2-DISK
int	      xg_fonhan=0;     // Handle pro XMS/DISK
unsigned int  xg_lbfnt =0;     // delka bufru s daty fontu
char	      xg_fnt_akt[64];  // Jmeno akt. fontu
// Pozn: pro fonty v XMS|DISK xv_fbuf 4K pro jeden znak
long	      xg_fntalloc=0;

// New 971002, fonts in XMS
int  xg_fnt_max =  0;	       // max pocet fontu v XMS
int  xg_fnt_fre =  0;	       // prvni volny v tabulce
int  xg_fnt_xms = -1;	       // handle XMS
long xg_fnt_xlen = 0;	       // celkova delka v XMS
long xg_fnt_xoff = 0;	       // volne misto v XMS
struct FNTXTAB *xg_fnt_xtab = NULL;   // tabulka fontu

//---- Zalamovani textu ----------------------------------------
//int	xg_istyle = 0;	       // Parametry funkce IZALOM(...)
//int	xg_ostyle = 0;
//float xg_MaxMez = 2.5;
//float xg_MaxRozt= 0.2;
int   xg_31yn	= 1;	       // Zda znaky <32 kreslit/nebo jen posun

int   xg_chrmod = 0;	       // Kreslit pozadi textu 0-ano, 1-ne
int   xg_no_mode = 0;	       // pro !=0 se grf.mod. doopravdy nenastavi
//---- pro VIRT video -----------------------------
int   xg_video_XMS = 0;        // 0-kresli se do video, 1-do VIRT
int   xg_bincol=0;	       // barva pro kresleni do binarni VIRT

//---- Promenne pro VIRT videoram -----------------
int  xv_file = 0;	       // Otevreny soubor .OBR
int  xv_bits;		       // Pro shift 8bit=0,4bit=1,1bit=3
int  xv_len_col;	       // Pocet bytu radku
int  xv_rows;		       // Pocet radku
int  xv_zmap;		       // Zacatek obrazu v souboru (512,1024)
int  xv_XMS = -1;	       // Pro extended memory ( -1 = disk)
int  xv_clsf= 1;	       // Udelat close(xv_file);
int  xv_rovin=1;	       // Pocet bit. rovin (1 nebo 3 pro CYM)
int  xv_cym[3]; 	       // 0/1 pise se do znacene bit. roviny
long xv_OffCYM[3];	       // Ofsety na zacatky bit rovin pro CYM
long xv_akt_adr=0;	       // Akt. adresa pro linearni cteni z x_virt()

//---- Psani barevneho textu (alokovat pole a naplnit barvami)
unsigned char *xg_chr1c=NULL;  // Popredi
unsigned char *xg_chr2c=NULL;  // Pozadi

//#include "x_lopif.h"

//----- Globaly pro Hi-color !!!
#if HI_COLOR
int xg_hi16= 0;
int xg_xgr=0;		       // poradi RGB v tripletu pro true color, 0=BGR,1=RGB
int xg_round=0; 	       // pro vypocet delky radku v B (0=dle ncol, 1-nas. ctyr(BMP))
int xg_hipalmod=0;	       // x_setcolor(), x_setfill():0=index, 1=primo hicolor
unsigned char xg_hipal[768];   // Pal pro HiCol mode (nastavit pres x_setpal())
unsigned int  xg_hival[256];   // Hi-col hodnoty k palete

void xh_CrePal256(unsigned char *Pal);

#endif

/*------------------------------------------------*/
#define VIDEO 0x10

int x_grf_mod(int mod)
{
   union  REGS	  regs;
   struct REGPACK reg;

   struct VESA_1 a_vesa;
   struct VESA_2 b_vesa;

   int	 rmod, x2;
   char  map[17]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0};
   //-----------------------------------------------

   if(mod != 7)
   {
   if((xg_svga&0x8000) != 0)	    /* VESA standart mody */
    {
     if(mod == 3) goto Z_VESA;	    // Z VESA -> TEXT
     reg.r_ax = 0x4F00; 	    // Get. info o VESA ?
     reg.r_di = FP_OFF(&a_vesa);
     reg.r_es = FP_SEG(&a_vesa);
     intr(0x10, &reg);
     if(reg.r_ax == 0x004F)	    // Vesa O.K.
       { if(strncmp(a_vesa.vesa,"VESA",4) != 0) goto Err1;
       }
      else			 // Karta neni VESA
       { Err1:
         return( 6 );
       }

     reg.r_ax = 0x4F01; 	 // Get. info o modu
     reg.r_cx = mod;
     reg.r_di = FP_OFF(&b_vesa);
     reg.r_es = FP_SEG(&b_vesa);
     intr(0x10, &reg);

     if(reg.r_ax != 0x004F)
      { return( 6 );
      }

     if(xg_no_mode == 0)
     {
     regs.x.ax = 0x4F02;	 // Graf. mod. VESA >= 100h
     regs.x.bx = mod;
     int86(VIDEO, &regs, &regs);

     regs.x.ax = 0x4F03;	 // Kontorla nastaveni VESA
     int86(VIDEO, &regs, &regs);
     rmod = regs.x.bx;
     }
     else
     {
     rmod = mod;
     }

     //------ Ulozit nektere informace o VESA do glob. prom.
     xg_vesa.mode_atr  = b_vesa.mode_atr;
     xg_vesa.win_A  = b_vesa.win_A;
     xg_vesa.win_B  = b_vesa.win_B;
     xg_vesa.granul  = b_vesa.granul;
     xg_vesa.win_size  = b_vesa.win_size;
     xg_vesa.seg_adr_A	= b_vesa.seg_adr_A;
     xg_vesa.seg_adr_B	= b_vesa.seg_adr_B;
     xg_vesa.adrwin  = b_vesa.adrwin;
     xg_vesa.bytes_lin	= b_vesa.bytes_lin;   // JdS 2004/09/26
     xg_vesa.bit_planes  = b_vesa.bit_planes;
     xg_vesa.bit_pixel	= b_vesa.bit_pixel;
     xg_vesa.mem_banks	= b_vesa.mem_banks;
     xg_vesa.mem_model	= b_vesa.mem_model;
    }
   else
    { if(xg_no_mode == 0)
      {
      Z_VESA:
      regs.h.ah = 0;		 /* Nastaveni modu (Krome VESA) */
      regs.h.al = mod;
      int86(VIDEO, &regs, &regs);

      regs.h.ah = 0xF;		 /* Kontrola nastaveni */
      int86(VIDEO, &regs, &regs);
      rmod = regs.h.al & 0x7F;
      }
      else
      {
      rmod = mod;
      }
    }
   }

  if(rmod == mod)
   { xg_mod = mod;
     xg_color = 7;
     xg_wrt   = 0;
     xg_fillc = 0;
     xg_tjustx= 0; /* left */
     xg_tjusty= 2; /* top  */
     xg_clip  = 0;
     xg_style = 0xFFFF;
     xg_flag  = 0;
     xg_col_plan = 4;  // Vsechny 4 bit. roviny
     xg_notview = 0;
     xg_x_cur = xg_y_cur = xg_f_cur = xg_c_col = 0;
     xg_fntalloc = 0L;

     // Masks MM_* mohou mit mastaveny pouze nektery z bitu 5..7 !!!
     if((xg_intern>=10) && (xg_intern<=14))
      { xg_256 = MM_Hic;	 // Hi color modes 800, 1024, 1280, 1600
	      xg_svga |= MM_Hic;
      }
     else if(xg_intern==2 || xg_intern==4 || xg_intern==5 || xg_intern==7 || xg_intern==9)
      { xg_256 = MM_256;
	xg_svga |= MM_256;	 // 256
      }
     else if(xg_intern == 8)
      { xg_svga |= MM_2;	 // 2
	      xg_256 = MM_2;
      }
     else			 // 16
      { xg_svga |= MM_16;
	      xg_256 = MM_16;
      }

#if HI_COLOR
     if(xg_256 == MM_Hic)	 // Default pal pro Hi-color
     {
       int i;
       xh_CrePal256(xg_hipal);
       for(i=0; i<256; i++)
       {
	 xg_hival[i] = xh_RgbHiPal(xg_hipal[3*i], xg_hipal[3*i+1], xg_hipal[3*i+2]);
       }
       x_setcolor(7);
       x_setfill(1,0);
     }
#endif

     if(mod == 3)
      { return(1);
      }
     else if(xg_intern==0 || xg_intern==1 || xg_intern==8 || xg_intern==9)  // 640
      {
       xg_svga |= 0x0002;  // len line
      }
     else if(xg_intern==2)  // 320 x 200 x 256
      {
       xg_svga |= 0x0001;
       goto End_ini;
      }
     else if(xg_intern==4 || xg_intern==10)  // 640 x 480 x 256,Hi
      {
       xg_svga |= 0x0002;
       goto End_ini;
      }
     else if(xg_intern==3 || xg_intern==5 || xg_intern==11)  // 800 x 600 x 16|256
      {
       xg_svga |= 0x0004;
       goto End_ini;
      }
     else if(xg_intern==6 || xg_intern==7 || xg_intern==12)  // 1024 x 768 x 16|256
      {
       xg_svga |= 0x0008;
       goto End_ini;
      }
/*
     else if(xg_intern==13)  // 1280 x 1024 x 16|256
      {
       xg_svga |= 0x000?;    // neni volny bitflag
       goto End_ini;
      }
     else if(xg_intern==14)  // 1600 x 1200 x 16|256
      {
       xg_svga |= 0x000?;    // neni volny bitflag
       goto End_ini;
      }
*/
     else
      {
       goto End_ini;
      }

     End_ini:
     if(xg_mod >= 0x06)
     {
     x2 = (int) ( 10000L * (long) x_maxy() * 4L / ( (long) x_maxx() * 3L ));
     x_setratio( x2, 10000 );

     xg_view[0] = xg_view[1] = 0;     /* Pocatek viewportu */
     xg_view[2] = x_maxx();
     xg_view[3] = x_maxy();
     }

     if(xg_intern == 1 || xg_intern == 3 || xg_intern == 6) // Mapuj palet.reg.
       { if(xg_no_mode == 0) x_map_pal( map );
       }

     return(1);
   }
  else		    /* Tento grf. mod nelze nastavit (BIOSem) */
   return( 2 );
}

/**********************************************************************/
/*		    Funkce nastavujici aspectratio		      */
/**********************************************************************/

void  x_setratio( int xasp, int yasp )
{
  xg_xr = xasp;
  xg_yr = yasp;
}

// Fce pro nastaveni nenastaveni grf. modu
void x_setnomode(int nomode)
{
  xg_no_mode = nomode;
}

#if HI_COLOR
void xh_CrePal256(unsigned char *Pal)
{
  int ir,ig,ib,inx;
  unsigned char R,G,B;
  unsigned char Blue[4] = {0,63,45,25};
  unsigned char Green[8]= {0,16,24,32,40,48,56,63};
  unsigned char Red[8]	= {0,63,56,48,40,32,24,16};

  inx = 0;
  for(ib=0; ib<4; ib++)
  {
    for(ig=0; ig<8; ig++)
    {
      for(ir=0; ir<8; ir++)
      { R = Blue[ib];
	G = Green[ig];
	B = Red[ir];
	Pal[inx] = R; Pal[inx+1] = G; Pal[inx+2] = B;
	inx += 3;
      }
    }
  }

}
#endif
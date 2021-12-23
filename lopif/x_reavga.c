// 9703 - HARO - Nove zakodovani grafickych modu
// Zrusen soubor SVGA.SET !!! Nahrazen tabulkou
// Duvod: Pridavani dalsich modu, karet atd.
// 11.2.2011 Pridani modu 1280x1024 a 1600x1200 Hi-Color by RayeR

#include <stdlib.h>
#include <string.h>
#include "x_lopif.h"

struct GRF_MODES    // TABULKA MODU
{  char Name[10];   // jmeno modu (VGA, VESA, TRIDENT,...)
   int	Svga[6];    // cisla modu SVGA v poradi A,B,C,D,E
   char Intern;     // interni cislo modu xlopifu
   char HwCode;     // kod hardware
};		    // sum 20B na mod, ci kartu

#if HI_COLOR
#define MAX_GRF_MODES	 14
#else
#define MAX_GRF_MODES	 12
#endif

static struct GRF_MODES xg_grf_modes[MAX_GRF_MODES]=
// Name       grf.mode				inter  HwCode
{ {"EGA",    {0x10,	0,    0,    0,	  0,  0},  0,  0},     //640x350x16
  {"VGA",    {0x12,	0,    0,    0,	  0,  0},  1,  0},     //640x480x16
  {"VGA2",   {0x13,	0,    0,    0,	  0,  0},  2,  0},     //320x200x256
  {"TSG3",   {0x29,  0x2E, 0x30, 0x37, 0x00,  0},  0,  0x01},
  {"OAK",    {0x52,  0x53, 0x54, 0x56, 0x59,  0},  0,  0x02},
  {"TAMARA", {0x58,  0x5F, 0x5C, 0x5D, 0x00,  0},  0,  0x04},
  {"TRIDENT",{0x5B,  0x5D, 0x5E, 0x5F, 0x62,  0},  0,  0x08},
  {"REALTEK",{0x1F,  0x26, 0x27, 0x21, 0x00,  0x25 },  0,  0x10},
  {"TSG4",   {0x29,  0x2E, 0x30, 0x37, 0x38,  0},  0,  0x20},
  {"M1",     {0x55,  0x59, 0x5A, 0x56, 0x5B,  0},  0,  0x40},
  {"VESA",   {0x102,0x101,0x103,0x104,0x105,  0x100},  0,  0x80}, //800x600x4,640x480x8,800x600x8,1024x768x4,1024x768x8,640x400x8
  {"BCGA",   {0x06,	0,    0,    0,	  0,  0},  8,  0},     //640x200x1
#if HI_COLOR
  {"HI15",   {0x110,0x113,0x116,0x119,0x11D,  0},  0,  0x80},  //640,800,1024,1280,1600 hicol 15bit
  {"HI16",   {0x111,0x114,0x117,0x11A,0x146,  0},  0,  0x80},  //640,800,1024,1280,1600 hicol 16bit
#endif
};

int x_rea_svga(char *path, char *name, int *mod)
{
// path - vstup: nepouzit
// name - vstup: jmeno graf.modu (VGA, TSG3.C, ...)
// mod	- vystup: skutecny graf. mod SVGA

   char   ch1, *iz, pom[32];
   int	  Inx, i;

// RayeR 13.2.2011 - VESA mode number override for 1600x1200 HiColor modes
// It's not standartized, differs on nVidia/ATI/etc...
// Use e.g. VESATEST.EXE to obrain correct mode numbers for your VGA
// Default values are suited for nVidia cards (tested on 7900GT)
#if HI_COLOR
   if ((iz=getenv("VM1600HI15"))!=NULL) // get environment variable
     xg_grf_modes[12].Svga[4]=strtol(iz,NULL,16); // if set, override mode
   if ((iz=getenv("VM1600HI16"))!=NULL) // get environment variable
     xg_grf_modes[13].Svga[4]=strtol(iz,NULL,16); // if set, override mode
#endif

   xg_svga = 0;  // Bit. mapa pro SVGA mody
   //nizsi byte : bit 0..4  delka radku 320..1024 (5bitu)
   //		  bit 5..7  mod 2/16/256/hicol barev
   //vyssi byte : typ hw karty pro SVGA (0x80=Vesa)

   strcpy(pom,strupr(name));
   iz=strchr(pom,'.');
   if(iz == NULL)
     { ch1 = 'B';   // Implicitne 640 x 480 x 256
     }
   else
     { ch1 = *(iz+1);
       *iz = 0;
     }
   // V "pom" je jmeno graf. karty, v "ch1" jmeno modu

   for(i=0; i<MAX_GRF_MODES; i++)
   { if(stricmp(pom,xg_grf_modes[i].Name) == 0) goto Grf_exist;
   }
   return( 4 );     // Unknown mode

   Grf_exist:
   if(xg_grf_modes[i].Svga[1] == 0)   // Standard modes
   { xg_intern = xg_grf_modes[i].Intern; // Internal mode from table
     *mod = xg_grf_modes[i].Svga[0];  // Only one mode
   }
   else 			      // SVGA modes
   { if(i >= 12)	  // Hi_color line : 12..13
     { Inx  = ch1 - 'I';
       xg_intern = Inx+10;
#if HI_COLOR
       if(i==12)
	 xg_hi16 = 0;
       else
	 xg_hi16 = 1;
#endif
     }
     else		  // Normal   line : 0..11
     { Inx  = ch1 - 'A';
       //if(i == 10 && Inx == 23)  // 9804: 640x400 VESA.X
       if(Inx == 23)  // 9810: 640x400 VESA.X, REALTEK.X, ...
       {xg_intern = 9;
      *mod = xg_grf_modes[i].Svga[5];
      if(*mod == 0) return( 6 );  // mod .X pro kartu neni urcen
      goto Vesa_x;
       }
       else
       {xg_intern = Inx+3;
       }
     }
     if(Inx > 5 || Inx < 0) goto Err_mode;

     *mod = xg_grf_modes[i].Svga[Inx];
     if(*mod == 0) goto Err_mode;

     Vesa_x:
     xg_svga   = ((unsigned int)xg_grf_modes[i].HwCode<<8);
   }
   return( 1 );

   Err_mode:
   return( 6 );
}

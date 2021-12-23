#include <stdlib.h>
#include "x_lopif.h"

#if HI_COLOR

// Prevod bufru z RGB (3Byty, 0..255) do HiColor (2Byty)
void xh_RgbToHi(unsigned char *Rgb, unsigned char *Hi,
		int Pixs, int Rows, int LenLine)
{
// Rgb - (in) buffer s radky RGB
// Hi  - (out)buffer s radky HiCol
// Pixs- pixlu na radek
// Rows- pocet radku (-Rows => Radky v Hi v opacnem poradi)
// LenLine - delka Rgb radku v Bytech

   int   i,j, Rows2;
   unsigned char *Rgbx;
   unsigned int  *Hix;

   Hix = (unsigned int *)Hi;
   Rows2 = abs(Rows);

   for(i=0; i<Rows2; i++)
   {
     if(Rows > 0)
      Rgbx = Rgb + (LenLine * i);
     else
      Rgbx = Rgb + (LenLine * (Rows2-i-1));

     for(j=0; j<Pixs; j++)
     {
	if(xg_hi16)   // 16 bitu na pixel
	{ (*Hix) =  ((unsigned)(*(Rgbx+2)>>3)<<11) |   // R
		    ((unsigned)(*(Rgbx+1)>>2)<<5)  |   // G
		     (unsigned)(*(Rgbx  )>>3);         // B
	}
	else          // 15 bitu na pixel
	{ (*Hix) =  ((unsigned)(*(Rgbx+2)>>3)<<10) |   // R
		    ((unsigned)(*(Rgbx+1)>>3)<<5)  |   // G
		     (unsigned)(*(Rgbx  )>>3);         // B
	}
	Rgbx += 3;
	Hix++;
     }
   }
}

// Prevod bufru s bytovym obrazem na Hi-color
// Pozn: Predpoklada se, ze byla volana fce x_palett() s paletou obrazu.
void xh_ByteToHi(unsigned char *Ibuf, unsigned char *Hi,
		int Pixs, int Rows, int LenLine)
{
// Ibuf- (in) buffer s radky paletoveho obrazu (1B/pixel)
// Hi  - (out)buffer s radky HiCol
// Pixs- pixlu na radek
// Rows- pocet radku (-Rows => Radky v Hi v opacnem poradi)
// LenLine - delka Ibuf radku v Bytech
   int   i,j, Rows2;
   unsigned char *Ibufx;
   unsigned int  *Hix;

   Hix = (unsigned int *)Hi;
   Rows2 = abs(Rows);

   for(i=0; i<Rows2; i++)
   {
     if(Rows > 0)
      Ibufx = Ibuf + (LenLine * i);
     else
      Ibufx = Ibuf + (LenLine * (Rows2-i-1));

     for(j=0; j<Pixs; j++)
     {
	(*Hix) = xg_hival[(*Ibufx)];
	Ibufx++;
	Hix++;
     }

   }
}

// Prevod RGB (0..63) na Hi-color
unsigned xh_RgbHiPal(unsigned char R, unsigned char G, unsigned char B)
{
   if(xg_hi16 == 1)
    return(RGBHI16(R,G,B));
   else
    return(RGBHI15(R,G,B));
}

// Prevod Hi-col do RGB (0..63)
void xh_HiPalRgb(unsigned int Hi, unsigned char *rgb)
{
   rgb[2] = (unsigned char)((Hi&0x001F)<<1);           // B
   if(xg_hi16 == 1)
   { rgb[1] = (unsigned char)((Hi&0x07E0)>>5);         // G
     rgb[0] = (unsigned char)(((Hi&0xF800)>>11)<<1);   // R
   }
   else
   { rgb[1] = (unsigned char)(((Hi&0x03E0)>>5)<<1);    // G
     rgb[0] = (unsigned char)(((Hi&0x7C00)>>10)<<1);   // R
   }
}

// Nastavi zaokrouheni : 0=nic, 1=nas.4bytu
int xh_SetRounding(int Round)
{
   int orig = xg_round;
   xg_round = Round;
   return( orig );
}

// Zpusob prace s paletou
int xh_SetPalMode(int Mode)
{
   int orig = xg_hipalmod;
   xg_hipalmod = Mode;
   return( orig );
}

#endif

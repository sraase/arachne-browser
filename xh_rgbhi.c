#include <stdlib.h>
#include "x_lopif.h"

#if HI_COLOR

// Prevod bufru z RGB (3Byty, 0..255) do HiColor (2Byty)
//tr.: convert buffer from RGB (3bytes,0..255) to HiColor (2bytes)
void xh_RgbToHi(unsigned char *Rgb, unsigned char *Hi,
		int Pixs, int Rows, int LenLine)
{
// Rgb - (in) buffer s radky RGB (tr.: in buffer with rows RGB)
// Hi  - (out)buffer s radky HiCol (tr.: out buffer with rows HiCol)
// Pixs- pixlu na radek (tr.: number of pixels in a row)
// Rows- pocet radku (-Rows => Radky v Hi v opacnem poradi)
// tr.: number of rows (-Rows => rows in Hi in reverse order)
// LenLine - delka Rgb radku v Bytech (tr.: length RGB rows in bytes)

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
        if(xg_hi16)   // 16 bitu na pixel (tr.: 16 bits in every pixel)
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
// tr.: Conversion of buffer with bitmap to Hi-color
//      note: assumes, that fce x_palett() with palette of the picture
//      has been called
void xh_ByteToHi(unsigned char *Ibuf, unsigned char *Hi,
		int Pixs, int Rows, int LenLine)
{
// Ibuf- (in) buffer s radky paletoveho obrazu (1B/pixel)
// tr.: in buffer with rows of the palette-picture
// Hi  - (out)buffer s radky HiCol
// tr.: out buffer with rows HiCol
// Pixs- pixlu na radek
// tr.: number of pixels in a row
// Rows- pocet radku (-Rows => Radky v Hi v opacnem poradi)
// tr.: number of rows (-Rows => rows in Hi in reverse order)
// LenLine - delka Ibuf radku v Bytech (tr.: length of Ibuf rows in Byte
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

// Conversion RGB (0..63) to Hi-color
unsigned xh_RgbHiPal(unsigned char R, unsigned char G, unsigned char B)
{
   if(xg_hi16 == 1)
    return(RGBHI16(R,G,B));
   else
    return(RGBHI15(R,G,B));
}

// Conversion Hi-color to RGB (0..63) 
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
// tr.: settings for rounding: 0=nothing, 1=multiples(?) of 4bytes
int xh_SetRounding(int Round)
{
   int orig = xg_round;
   xg_round = Round;
   return( orig );
}

// Zpusob prace s paletou
// tr.: mode of work with palette
int xh_SetPalMode(int Mode)
{
   int orig = xg_hipalmod;
   xg_hipalmod = Mode;
   return( orig );
}

#endif

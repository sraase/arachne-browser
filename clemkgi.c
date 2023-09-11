/*----------------------------------------------------------------------------
 * Arachne browser - Clementine/KGI display code
 *----------------------------------------------------------------------------
 *
 * Copyright (c) 1998-2000 Suntech. Written by Emmanuel Marty (core).
 *
 * This file is part of the sourcecode for the Clementine operating system.
 * You can redistribute and/or modify it under the terms and conditions of the
 * Clementine license, described in doc/COPYING-CLEMENTINE of this source tree
 *----------------------------------------------------------------------------
 * $Id:$
 *----------------------------------------------------------------------------
 */

#include <clementine/types.h>
#include <clementine/memory.h>
#include <clementine/errno.h>
#include <kgi/kgi.h>
#include "arachne.dep.h"
#include "arachne.h"

extern struct components *view;

static int oldvisx, oldvisy, oldvirtx, oldvirty, oldgt;
static int oldframes, oldfontx, oldfonty;
static boolean doShutdown = FALSE;
static unsigned char *frameBufPtr;
int screenwidth, screenheight;

#ifdef CLEMVGA
unsigned char *realFrameBufPtr;

static void updatefb(int x, int y, int w, int h) {
   unsigned short *srcFbPtr;
   unsigned char *dstFbPtr;
   int i, j;

   x &= ~1; y &= ~1;
   w = (w + 1) >> 1;
   h = (h + 1) >> 1;
   srcFbPtr = ((unsigned short *) frameBufPtr) + x + 640*y;
   dstFbPtr = ((unsigned char *) realFrameBufPtr) + (x>>1) + 320*(y>>1);
   
   for (j = 0; j < h; j++) {
      for (i = 0; i < w; i++) {
         unsigned int p = (unsigned int) (*srcFbPtr++);
         unsigned int p2 = (unsigned int) (*srcFbPtr++);
         unsigned int p3 = (unsigned int) (*(srcFbPtr + 638));
         unsigned int p4 = (unsigned int) (*(srcFbPtr + 639));

         *dstFbPtr ++ = ((((p & 0xF800) >> 11) << 3) * 5 +
                         (((p & 0x07E0) >> 5) << 2) * 6 +
                          ((p & 0x001F) << 3) * 5 +
                         (((p2 & 0xF800) >> 11) << 3) * 5 +
                         (((p2 & 0x07E0) >> 5) << 2) * 6 +
                          ((p2 & 0x001F) << 3) * 5 +
                         (((p3 & 0xF800) >> 11) << 3) * 5 +
                         (((p3 & 0x07E0) >> 5) << 2) * 6 +
                          ((p3 & 0x001F) << 3) * 5 +
                         (((p4 & 0xF800) >> 11) << 3) * 5 +
                         (((p4 & 0x07E0) >> 5) << 2) * 6 +
                          ((p4 & 0x001F) << 3) * 5) >> 6;
      }
      
      srcFbPtr += 640*2 - (w << 1);
      dstFbPtr += 320 - w;
   }
}

#endif

static int bline(int orig_x1,int orig_y1,int orig_x2,int orig_y2,short color,
   int w, unsigned char *frameBuf)
{
	int orig_dx,orig_dy,sx,sy;
	int dx,dy;
	int i;
	int x1,y1,x2,y2;
	int clip_first=0,clip_last=0;
	unsigned short *fb;

	x1 = orig_x1;
	y1 = orig_y1;
	x2 = orig_x2;
	y2 = orig_y2;

	dy = y2 - y1;
	orig_dy = orig_y2 - orig_y1;
	sy=1;
	if (orig_dy<0) {
		orig_dy = -orig_dy;
		dy = -dy;
		sy = -1;
	}

	dx = x2-x1;
	orig_dx = orig_x2 - orig_x1;
	sx=1;
	if (orig_dx<0) {
		sx=-1;
		orig_dx = -orig_dx;
		dx = -dx;
	}

        fb = (unsigned short *) (frameBuf + y1*w+x1*sizeof(unsigned short));
  
	if (dx==0) {
		if (sy<0)
			w = -w;
   
		for (i=dy;i>=0; i--) {
			*fb = color;
			fb = (unsigned short *) (((unsigned char *)fb) + w);
		}
		return 0;
	}

	if (dy==0) {
		for (i=dx;i>=0; i--) {
			*fb = color;
			fb += sx;
		}
		return 0;
	}

	if (orig_dx==orig_dy) {
		if (sy<0)
			w = -w;
		w += sx*sizeof(unsigned short);
		for (i=dx;i>=0; i--) {
			*fb = color;
			fb = (unsigned short *) (((unsigned char *)fb) + w);
		}
		return 0;
	}

	if (orig_dx >= orig_dy) { /* x major */
		int runlen,adjup,adjdown,e,len;
		int firstlen,lastlen;

		runlen = orig_dx/orig_dy;
		adjup = orig_dx%orig_dy;
		lastlen = firstlen = (runlen>>1) + 1;
		if (clip_first) { /* clipped, Adjust firstlen */
			int clip_dx = abs(x1 - orig_x1);
			int clip_dy = abs(y1 - orig_y1);
			int d = (2*clip_dy+1)*orig_dx; 
			firstlen = d/(2*orig_dy) - clip_dx + 1;
			e = d%(2*orig_dy);
			if ((e==0) && (sy>0)) { /* Special case, arbitrary choise. Select lower pixel.(?) */
				firstlen--;
				e += 2*orig_dy;
			}
			e -= (orig_dy*2);
		} else { /* Not clipped, calculate start error term */
			e = adjup - (orig_dy<<1); /* initial errorterm == half a step */
			if ((runlen&1) != 0) {
				e += orig_dy;
			}
		}
		if (clip_last) { /* Last endpoint clipped */
			int clip_dx = abs(x2 - orig_x2);
			int clip_dy = abs(y2 - orig_y2);
			int d = (1+2*clip_dy)*orig_dx; 
			lastlen = d/(2*orig_dy) - clip_dx + 1;
			if ((sy<0)  && ((d%(2*orig_dy))==0) ) /* special arbitrary case */
				lastlen--; 
		}
		adjup <<= 1;
		adjdown = orig_dy<<1;

		if (sy>0) {  /* line goes down */
			if ((adjup==0) && ((runlen&1)==0) && (!clip_first)) {
				firstlen--;
			}
			if (sx>0) { /* line goes right */
				for (;firstlen>0; firstlen--) {
					*fb = color;
					fb++;
				}
				fb = (unsigned short *) (((unsigned char *)fb) + w);
				for (i=dy-1; i>0; i--) {
					len = runlen;
					e += adjup;
					if (e>0) {
						len++;
						e -= adjdown;
					}
					for (;len>0; len--) {
						*fb = color;
						fb++;
					}
					fb = (unsigned short *) (((unsigned char *)fb) + w);
				}
				for (;lastlen>0; lastlen--) {
					*fb = color;
					fb++;
				}
				return 0;
			} else { /* line goes left */
				for (;firstlen>0; firstlen--) {
					*fb = color;
					fb--;
				}
				fb = (unsigned short *) (((unsigned char *)fb) + w);
				for (i=dy-1; i>0; i--) {
					len = runlen;
					e += adjup;
					if (e>0) {
						len++;
						e -= adjdown;
					}
					for (;len>0; len--) {
						*fb = color;
						fb--;
					}
					fb = (unsigned short *) (((unsigned char *)fb) + w);
				} 
				for (;lastlen>0; lastlen--) {
					*fb = color;
					fb--;
				}
				return 0;
			}
		} else { /* line goes up */
			if ((adjup==0) && ((runlen&1)==0) && (!clip_last)) { 
				lastlen--;
			}
			if (sx>0) { /* line goes right */
				for (;firstlen>0; firstlen--) {
					*fb = color;
					fb++;
				}
				fb = (unsigned short *) (((unsigned char *)fb) - w);
				for (i=dy-1; i>0; i--) {
					len = runlen;
					e += adjup;
					if (e>=0) {
						len++;
						e -= adjdown;
					}
					for (;len>0; len--) {
						*fb = color;
						fb++;
					}
					fb = (unsigned short *) (((unsigned char *)fb) - w);
				}
				for (;lastlen>0; lastlen--) {
					*fb = color;
					fb++;
				}
				return 0;
			} else { /* line goes left */
				for (;firstlen>0; firstlen--) {
					*fb = color;
					fb--;
				}
				fb = (unsigned short *) (((unsigned char *)fb) - w);
				for (i=dy-1; i>0; i--) {
					len = runlen;
					e += adjup;
					if (e>=0) {
						len++;
						e -= adjdown;
					}
					for (;len>0; len--) {
						*fb = color;
						fb--;
					}
					fb = (unsigned short *) (((unsigned char *)fb) - w);
				} 
				for (;lastlen>0; lastlen--) {
					*fb = color;
					fb--;
				}
				return 0;
			}
		}
	} else { /* y major */
		int runlen,adjup,adjdown,e,len;
		int firstlen,lastlen;

		runlen = orig_dy/orig_dx;
		adjup = orig_dy%orig_dx;
    
		lastlen = firstlen = (runlen>>1) + 1;
		if (clip_first) { /* clipped, Adjust firstlen */
			int clip_dx = abs(x1 - orig_x1);
			int clip_dy = abs(y1 - orig_y1);
			int d = (2*clip_dx+1)*orig_dy;
			firstlen = d/(2*orig_dx) - clip_dy + 1;
			e = d%(2*orig_dx);
			if ((e==0) && (sx>0)) { /* Special case, arbitrary choise. Select lower pixel.(?) */
				firstlen--;
				e += 2*orig_dx;
			}
			e -= (orig_dx*2);
		} else { /* Not clipped, calculate start error term */
			e = adjup - (orig_dx<<1); /* initial errorterm == half a step */
			if ((runlen&1) != 0) {
				e += orig_dx;
			}
		}
		if (clip_last) { /* Last endpoint clipped */
			int clip_dx = abs(x2 - orig_x2);
			int clip_dy = abs(y2 - orig_y2);
			int d = (1+2*clip_dx)*orig_dy; 
			lastlen = d/(2*orig_dx) - clip_dy + 1;
			if ((sx<0)  && ((d%(2*orig_dx))==0) ) /* special arbitrary case */
				lastlen--;
		}
		adjup <<= 1;
		adjdown = orig_dx<<1;
		if (sy>0) { /* Line goes DOWN */
			if (sx>0) { /* line goes RIGHT */
				if ((adjup==0) && ((runlen&1)==0) && (!clip_first)) {
					firstlen--;
				}
				for (;firstlen>0; firstlen--) {
					*fb = color;
					fb = (unsigned short *) (((unsigned char *)fb) + w);
				}
				fb++;
				for (i=dx-1; i>0; i--) {
					len = runlen;
					e += adjup;
					if (e>0) {
						len++;
						e -= adjdown;
					}
					for (;len>0; len--) {
						*fb = color;
						fb = (unsigned short *) (((unsigned char *)fb) + w);
					}
					fb ++;
				}
				for (;lastlen>0; lastlen--) {
					*fb = color;
					fb = (unsigned short *) (((unsigned char *)fb) + w);
				}
				return 0;
			} else { /* line goes LEFT */
				if ((adjup==0) && ((runlen&1)==0) && (!clip_last)) {
					lastlen--;
				}
				for (;firstlen>0; firstlen--) {
					*fb = color;
					fb = (unsigned short *) (((unsigned char *)fb) + w);
				}
				fb--;
				for (i=dx-1; i>0; i--) {
					len = runlen;
					e += adjup;
					if (e>=0) {
						len++;
						e -= adjdown;
					}
					for (;len>0; len--) {
						*fb = color;
						fb = (unsigned short *) (((unsigned char *)fb) + w);
					}
					fb --;
				}
				for (;lastlen>0; lastlen--) {
					*fb = color;
					fb = (unsigned short *) (((unsigned char *)fb) + w);
				}
				return 0;
			}
		} else { /* Line goes UP */
			if (sx>0) { /* line goes RIGHT */
				if ((adjup==0) && ((runlen&1)==0) && (!clip_first)) {
					firstlen--;
				}
				for (;firstlen>0; firstlen--) {
					*fb = color;
					fb = (unsigned short *) (((unsigned char *)fb) - w);
				}
				fb++;
				for (i=dx-1; i>0; i--) {
					len = runlen;
					e += adjup;
					if (e>0) {
						len++;
						e -= adjdown;
					}
					for (;len>0; len--) {
						*fb = color;
						fb = (unsigned short *) (((unsigned char *)fb) - w);
					}
					fb ++;
				}
				for (;lastlen>0; lastlen--) {
					*fb = color;
					fb = (unsigned short *) (((unsigned char *)fb) - w);
				}
				return 0;
			} else { /* line goes LEFT */
				if ((adjup==0) && ((runlen&1)==0) && (!clip_last)) {
					lastlen--;
				}
				for (;firstlen>0; firstlen--) {
					*fb = color;
					fb = (unsigned short *) (((unsigned char *)fb) - w);
				}
				fb--;
				for (i=dx-1; i>0; i--) {
					len = runlen;
					e += adjup;
					if (e>=0) {
						len++;
						e -= adjdown;
					}
					for (;len>0; len--) {
						*fb = color;
						fb = (unsigned short *) (((unsigned char *)fb) - w);
					}
					fb --;
				}
				for (;lastlen>0; lastlen--) {
					*fb = color;
					fb = (unsigned short *) (((unsigned char *)fb) - w);
				}
				return 0;
			}
		}
	}
}

//---------------------------------------------------------------------------
//X_LOPIF bindings:
//---------------------------------------------------------------------------

// this variable indicates graphics mode, is used to select between
// HiColor vs. 256 color modes
int xg_256;
int xg_video_XMS=0; //real screen
unsigned short xg_hival[256]; //hicolor values for palette
int xg_hi16=1;    // mode : 16bit=1,15bit=0
int xg_hipalmod=0;// x_setcolor(), x_setfill():0=index, 1=directly hicolor
int xg_chrmod=1;
unsigned char xg_hipal[768];  // Pal for HiCol mode (set through x_setpal())

int xg_color;                 /* Colour setting (rect,text)     */
int xg_fillc;                 /* Colour setting for x_bar()     */
int xg_wrt;                   /* 0=overwrite, 1=XOR             */
int xg_style;                 /* Definition of line             */
int xg_xr,xg_yr;              /* ratio                          */
int xg_view[4];               /* Viewport                       */
int xg_xfnt,xg_yfnt;          /* Size of font in pixels         */
int xg_tjustx;                /* Align text in X direction      */
int xg_tjusty;                /* Align text in Y direction      */
int xg_clip;                  /* Whether to cut off in viewport */
int xg_fbyt;                  /* Number of bytes for one character in font  */
int xg_flag;                  /* Types: bit 0 - spec EGA pal    */
                              /*        bit 1 - active patt     */
unsigned char xg_fonlen[256];  // Width of charater for proportional fonts
long  int     xg_fonadr[256];  // Beginnings of character for prop. fonts
unsigned char xg_foncon;       // Flag - const/prop font [0/average]
unsigned char xg_fonmem;       // Where the font is: 0-MEM(xg_fbuf),1-XMS,2-DISK
int           xg_fonhan;       // Handle for XMS/DISK
unsigned int  xg_lbfnt;        // buffer length with data of fonts

char *xg_fbuf;                /* bufffer for font                */
int xg_fnt_zoo=1;      /* For zooming of text 1 | 2 */
char xg_fnt_akt[64];  // Name of current font

int  xg_fnt_max;       // max number of fonts in XMS
int  xg_fnt_fre;       // first free in table 
int  xg_fnt_xms;       // handle XMS
long xg_fnt_xlen;      // total length in XMS
long xg_fnt_xoff;      // free space in XMS
struct FNTXTAB *xg_fnt_xtab;   // table of font/fonts
int   xg_31yn=0;      //ASCII 1 ... ASCII 31 interpreted as 1..31 pixel space

//---------------------------------------------------------------------------

int x_grf_mod(int xmod)
{
 //set text mode here...
 //we will emulate hicolor for now....
 xg_256=MM_Hic;
 return 1;
}

static short kgi_getcolor (int index) {
 unsigned char *palptr = xg_hipal + 3*index;
 int r = ((int) (*palptr++));
 int g = ((int) (*palptr++));
 int b = ((int) (*palptr));
 
 return (short) (((r & 0x3E) << 10) | (g << 5) | (b >> 1));
}

int x_maxx(void)
{
 return screenwidth-1;
}

int x_maxy(void)
{
 return screenheight-1;
}

void x_video_XMS(int vidXMS, int bincol)
{
 //toggle graphics output to real or virtual screen.
}

void x_setcolor(int color) 
{
#ifndef TXTDEBUG
 //set foreground color
 xg_color=color;
#endif
}

void x_line(int x1, int y1, int x2, int y2 )
{
// draw line
#ifndef TXTDEBUG
   short color = kgi_getcolor (xg_color);
   bline (x1,y1,x2,y2,color,screenwidth<<1,frameBufPtr);
#ifdef CLEMVGA
   updatefb(x1,y1,x2-x1+1,y2-y1+1);
#endif   
#endif
}

void x_bar(int xz, int yz, int xk, int yk)
{
#ifndef TXTDEBUG
 short color = kgi_getcolor (xg_fillc);
 int w=(xk-xz)+1;
 int h=(yk-yz)+1;
 int i, j;
 unsigned char *scrPtr = frameBufPtr + ((xz + yz * screenwidth) << 1);
 int delta = (screenwidth - w) << 1;
 
 for (j = 0; j < h; j++) {
   for (i = 0; i < w; i++)
      *(((short *) scrPtr)++) = color;
   scrPtr += delta;
 }
#ifdef CLEMVGA
   updatefb(xz,yz,w,h);
#endif   
#endif
}

void x_rect(int xz, int yz, int xk, int yk)
{
 // draw empty rectangle
#ifndef TXTDEBUG
   short color = kgi_getcolor (xg_color);
   bline (xz,yz,xk,yz,color,screenwidth<<1,frameBufPtr);
   bline (xk,yz,xk,yk,color,screenwidth<<1,frameBufPtr);
   bline (xz,yk,xk,yk,color,screenwidth<<1,frameBufPtr);
   bline (xz,yk,xz,yk,color,screenwidth<<1,frameBufPtr);
#ifdef CLEMVGA
   updatefb(xz,yz,xk-xz+1,yk-yz+1);
#endif   
#endif
}

void x_setfill(int vypln, int color)
{
 // set fill color - first arg is dummy
 xg_fillc=color;
}

void x_settextjusty(int horiz, int vert)
{
  xg_tjustx = horiz;
  xg_tjusty = vert;
}

void x_charmod(int chrmod)
{
 //transparent or solid text blocks...
 xg_chrmod=chrmod;
}

void x_getimg(int x1, int y1, int x2, int y2, char *bitmap)
{
 unsigned char *scrPtr = frameBufPtr + ((x1 + y1 * screenwidth) << 1);
 short int w=(short int)(x2-x1)+1;
 short int h=(short int)(y2-y1)+1;
 int i, stride;
 unsigned char *imgPtr;

 memcpy(bitmap,&w,sizeof(short int));
 memcpy(&bitmap[sizeof(short int)],&h,sizeof(short int));
 
 w <<= 1;
 stride = screenwidth << 1;
 imgPtr = (unsigned char *) &bitmap[2*sizeof(short int)];
 
 for (i = 0; i < h; i++) {
   memcpy (imgPtr, scrPtr, w);
   scrPtr += stride; imgPtr += w; 
 }
}

void x_putimg(int xz,int yz, char *bitmap, int op)
{
 int w, h, i, stride;
 unsigned char *scrPtr = frameBufPtr + ((xz + yz * screenwidth) << 1);
 unsigned char *imgPtr;

 //put image (real screen only)
 w = ((int) *(short int *)bitmap) << 1;
 h = (int) *(short int *)&bitmap[sizeof(short int)];
 stride = screenwidth << 1;

 imgPtr = (unsigned char *) &bitmap[2*sizeof(short int)];
 
 for (i = 0; i < h; i++) {
   memcpy (scrPtr, imgPtr, w);
   imgPtr += w; scrPtr += stride;
 }

#ifdef CLEMVGA
   updatefb(xz,yz,w>>1,h);
#endif   
}

int x_getmaxcol(void)
{
 //return max. number of colors
 return 256;
}

void x_palett(int len, char *paleta)
{
 //set palette to array or (3 R-G-B bytes)*len
  if(xg_256 == MM_Hic)           // pouze kopie do xg_hipal
           // tr.: only a copy into xg_hipal
  {
   int i;

     memcpy(xg_hipal, paleta, 3*len);
     for(i=0; i<len; i++)
       { xg_hival[i] = xh_RgbHiPal(xg_hipal[3*i], xg_hipal[3*i+1],
				   xg_hipal[3*i+2]);
       }
  }
  //else real palette mapping.... later....
 
}

void x_yncurs(int on, int x, int y, int col)
{
 //enable/disable cursor at certain position, using certain color
}

void x_cleardev(void)
{
 //cls
}

int x_detect(char *svga, int *kby)
{
 //detect graphics mode... will be probably unused with SVGAlibg
}

void x_pal_1(int n_pal, char *pal_1)
{
 //set one color in palette
 memcpy(&xg_hipal[3*n_pal],pal_1,3);
 xg_hival[n_pal] = xh_RgbHiPal(xg_hipal[3*n_pal], xg_hipal[3*n_pal+1], xg_hipal[3*n_pal+2]);
}

/*
int x_fnt_load(char *fnt_file, int num, int mod)
{
 //load font file 
 return 1;
}
*/

void x_defcurs(const unsigned short *screen, const unsigned short *cursor, int color)
{
 //define cursor shape
}

void x_cursor(int x, int y)
{
 //move mouse cursor to x,y
}

int x_rea_svga(char *path, char *g_jmeno, int *mod)
{
 //convert mode name to mode number
 //probably won't be used
 return 0;
}

void z_bitbyte(unsigned char *buf1, unsigned char *buf2, int delka)
{
 //converts bit planes to bytes
}

// Prevod bufru s bytovym obrazem na Hi-color
// Pozn: Predpoklada se, ze byla volana fce x_palett() s paletou obrazu.
// tr.: conversion of buffer with bitmap to Hi-color
//      note: it is supposed that fce x_palett() has been called with
//      palette of picture

void xh_ByteToHi(unsigned char *Ibuf, unsigned char *Hi,
		int Pixs, int Rows, int LenLine)
{
// Ibuf- (in) buffer with rows of palette picture (1B/pixel)
// Hi  - (out)buffer with rows HiCol
// Pixs- pixels in one row 
// Rows- total number of rows (-Rows => rows in Hi in inverted order)
// LenLine - length Ibuf of row in bytes 
   int   i,j, Rows2;
   unsigned char *Ibufx;
   unsigned short  *Hix;

   Hix = (unsigned short *)Hi;
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

// conversion RGB (0..63) to Hi-color
unsigned short xh_RgbHiPal(unsigned char R, unsigned char G, unsigned char B)
{
   if(xg_hi16 == 1)
    return(RGBHI16(R,G,B));
   else
    return(RGBHI15(R,G,B));
}

//---------------------------------------------------------------------------
// virtual screens
//---------------------------------------------------------------------------

int xv_set_actvirt(int Index)
{
 //set active virtual screen
 return 1;
}; 

int  xv_to_scr(int xs, int ys, int xo, int yo, int dx, int dy)
{
 //dump virtual screen to real screen
 return 1;
}

int  xv_cls_virt(int Xwrt,        // 0-only free XMS, 1-write XMS on disk
		int Index)        // number of virtual videoram
{
 //clear virt. screen
 return 0;
}

int xv_new_virt(char *filenam, // File name for disk file
	      int dx, int dy,  // Size in pixels
	      int col,         // Default color
	      int bitpix,      // 3-1bit/pixel, 0-8bit/pixel, -1-16bit/pixel
	      int npal,        // Length of palette
	      char *pal,       // Palette, range RGB 0..63, max. 256 entries
	      int Index,       // Index 0..5, number of virtual videoram
	      int Typ)        // 0-XMS or DISK, 1-XMS, 2-DISK
{
 return 1;
}

int  xv_int_wrt(int xs, int ys, char *buf)
{
 //putimage to virt. screen
 return 1;
}

int  xv_int_rea(int xs, int ys, int dx, int dy, char *buf)
{
 //getimage from virt. screen
 return 1;
}


//---------------------------------------------------------------------------
// GIF animations - hanimgif.c, needs to rework memory management....
//---------------------------------------------------------------------------

int  g_NumAnim;       // number of animaged GIFs

int  XInitAnimGIF(int XmsKby)
{
 //call once at start
}

void XCloseAnimGIF(void)
{
 //call once at end
}

int  XResetAnimGif(void)
{
 // when redrawing page
}

int  XAnimateGifs(void)
{
 //called from loop to animage GIFs
}        

void XSetAllAnim1(void)
{
 //rewind animations
}

void XSetAnim1(void)
{
 //rewind animation (?)
}

//---------------------------------------------------------------------------
//mouse interface
//---------------------------------------------------------------------------

int ImouseIni( int xmin, int ymin, int xmax, int ymax,
		int xstart, int ystart)
{
 //initialize mouse
 return 1;
}

int ImouseRead( int *xcurs, int *ycurs)
{
 //read mouse coordinates and return mouse buttons status 0,1,2,4...
 return 0;
}

void ImouseSet( int xstart, int ystart)
{
 //set mouse coordinates
}

void ImouseWait(void)
{
 // wait for user to release mouse key
}

void graphicsinit(char *dummy)
{
  int curvisx, curvisy, curvirtx, curvirty, curgt;
  int curframes, curfontx, curfonty;
  int result;
#ifdef CLEMVGA
  struct clut256 clut;
  int i;
#endif

  graphics=1;
  xg_256=MM_Hic; //set Hicolor flag...
  initpalette();
  x_settextjusty(0,2);  // always write text from upper left corner

  view->kgi->Init();

  view->kgi->GetGraphMode (&oldvisx, &oldvisy,
    &oldvirtx, &oldvirty, &oldgt, &oldframes, &oldfontx, &oldfonty);
  doShutdown = TRUE;

#ifndef CLEMTEXT
#ifndef CLEMVGA
  result = view->kgi->SetGraphMode (800, 600,
    800, 600, KGIGT_16BIT, 1, 1, 1);

  view->kgi->GetGraphMode (&curvisx, &curvisy,
    &curvirtx, &curvirty, &curgt, &curframes, &curfontx, &curfonty);

  screenwidth = curvirtx;
  screenheight = curvirty;

  frameBufPtr = (unsigned char *) view->kgi->GetFrameBuffer
    (view->memory->this);
#else
  result = view->kgi->SetGraphMode (320, 200,
    320, 200, KGIGT_8BIT, 1, 1, 1);

  realFrameBufPtr = (unsigned char *) view->kgi->GetFrameBuffer
    (view->memory->this);

  screenwidth = 640;
  screenheight = 400;
  frameBufPtr = view->memory->malloc (640*400*2*2);

  for (i = 0; i < 256; i++)
     clut.clut[i].r =
        clut.clut[i].g =
        clut.clut[i].b = (i << 8) | i;

  view->kgi->SetClut (&clut);
#endif

#else
  screenwidth = 800;
  screenheight = 600;
  frameBufPtr = view->memory->malloc (800*600*2*2);
#endif
}

void graphicsdeinit(void) {
   if (doShutdown) {
      doShutdown = FALSE;

      view->kgi->SetGraphMode (oldvisx, oldvisy, oldvirtx, oldvirty,
         oldgt, oldframes, oldfontx, oldfonty);

      view->kgi->Exit();
   }
}

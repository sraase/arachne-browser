
// ========================================================================
// SVGA and mouse interface for Arachne
// (c)1996-1999 Michael Polak, Arachne Labs
// ========================================================================

#include "x_lopif.h"  // LOPIF header file

//LOPIF mouse
 int ImouseIni( int xmin, int ymin, int xmax, int ymax,
		int xstart, int ystart);
 int ImouseRead( int *xcurs, int *ycurs);
void ImouseSet( int xstart, int ystart);
void ImouseWait(void);

//----------------------------------------------------------------------------
//basic SVGA inteface
void detectgraphics(void);
void graphicsinit(char *svgamode); //inicializace grafiky, mod dle X_LOPIF
void initpalette(void);

//----------------------------------------------------------------------------
//basic graphical functions
void Box3D(int x1,int y1,int x2,int y2);
void Box3Dh(int x1,int y1,int x2,int y2);
void Box3Dv(int x1,int y1,int x2,int y2);
//void Box3D1pix(int x1,int y1,int x2,int y2);
void Cell3D(int x1,int y1,int x2,int y2,int col);
void htmlfont(int fnum, char style);

//----------------------------------------------------------------------------
//fontsize
char fontx(int fontnum, char style, unsigned char z);
char fonty(int fontnum, char style);

//load font info
void finfo(void);

//----------------------------------------------------------------------------
//special effects
void Scratch3D(int x1,int y,int x2);
void Cross(int x,int y,int a);
void decorated_text(int x,int y,char *text,int *colormap);
void krabice(int x1,int y1,int x2,int y2,int c1,int c2,int c3);

//----------------------------------------------------------------------------
//allocation of DOS memory for big fonts - 16-bit DOS
void bigfonts_allowed(void);
void bigfonts_forbidden(void);

#define SMALL_FONT_BUFFER 13000l
#define BIG_FONT_BUFFER   21000l

//----------------------------------------------------------------------------
//font styles - for HTML atoms of type TEXT, htmlfont() function, etc.
#define BOLD 1
#define ITALIC 2
#define FIXED 4
#define UNDERLINE 8
#define HTMLBLINK 16
#define STRIKE 32
#define TEXT3D 64
#define TEXT3D2 128 //?

#define SYSFONT 100

//----------------------------------------------------------------------------
//pseudo-font size for ASCII printing of HTML - images and tables will be
//smaller than in real documents...
#define FIXEDFONTX 12
#define FIXEDFONTY 12

//----------------------------------------------------------------------------
//ICON (.IKN) files interface:

void InitIcons(void);
void DrawIcons(void);
void DrawIconLater(char *iconame,int x0, int y0);
void DrawIconNow(char *iconame,int x0, int y0);

//----------------------------------------------------------------------------
//Font info structure
struct Finf
{
 char y[6][8];
 char prop_x[6][4][224];
 char fixed_x[6][4];
 char entity[128][6];
};

extern struct Finf *finf;

//----------------------------------------------------------------------------
//ega and vga filters for R,G,B values in palette...
char egafilter(char i);
char vgafilter(char i);

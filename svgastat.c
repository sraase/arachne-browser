
// ========================================================================
// Initialization of graphics, graphics functions
// (c)1997,1998,199 Arachne Communications (xChaos software)
// ========================================================================

#include "arachne.h"

void colorBox(int x1,int y1,int x2,int y2,int c1,int c2,int c3)
{
 x_setcolor(c1);
 x_line(x1,y2,x1,y1);
 x_line(x1+1,y2-1,x1+1,y1+1);
 x_line(x1,y1,x2,y1);
 x_line(x1+1,y1+1,x2-1,y1+1);
 x_setcolor(c2);
 x_line(x1+1,y2,x2,y2);
 x_line(x1+2,y2-1,x2-1,y2-1);
 x_line(x2,y1+1,x2,y2);
 x_line(x2-1,y1+2,x2-1,y2-1);
 if(c3<0)return;
 x_setfill(0,c3);
 x_bar(x1+2,y1+2,x2-2,y2-2);
// x_bar(x1+1,y1+1,x2-1,y2-1);
}
/*

void colorBox1pix(int x1,int y1,int x2,int y2,int c1,int c2,int c3)
{
 x_setcolor(c1);
 x_line(x1,y2,x1,y1);
// x_line(x1+1,y2-1,x1+1,y1+1);
 x_line(x1,y1,x2,y1);
// x_line(x1+1,y1+1,x2-1,y1+1);
 x_setcolor(c2);
 x_line(x1+1,y2,x2,y2);
// x_line(x1+2,y2-1,x2-1,y2-1);
 x_line(x2,y1+1,x2,y2);
// x_line(x2-1,y1+2,x2-1,y2-1);
 if(c3<0)return;
 x_setfill(0,c3);
//old: x_bar(x1+2,y1+2,x2-2,y2-2);
 x_bar(x1+1,y1+1,x2-1,y2-1);
}

*/

#ifdef HICOLOR

#define NEWCOL 18

void shades(int x1,int y1,int x2,int y2)
{
 int i=0;
 char pal[3],sh[8]={63,60,56,52,45,40,35,30};

 while(i<8)
 {
  pal[0]=pal[1]=pal[2]=sh[i];
  x_pal_1(NEWCOL+i,pal);
  i++;
 }

 i=0;
 while(i<4)
 {
  x_setcolor(NEWCOL+i);
  x_line(x1+i,y1+i,x2-i,y1+i);
  x_line(x1+i,y1+i,x1+i,y2-i);
/*  if(horiz)
   x_line(x1,y1+i,x2,y1+i);
  else
   x_line(x1+i,y1,x1+i,y2);
*/
  i++;
 }//loop

 i=0;
 while(i<4)
 {
  x_setcolor(NEWCOL+7-i);
  x_line(x1+i,y2-i,x2-i,y2-i);
  x_line(x2-i,y1+i,x2-i,y2-i);
/*
  if(horiz)
   x_line(x1,y2-i,x2,y2-i);
  else
   x_line(x2-i,y1,x2-i,y2);
*/
  i++;
 }//loop

 x_setfill(0,7);
/*
 if(horiz)
  x_bar(x1,y1+4,x2,y2-4);
 else
  x_bar(x1+4,y1,x2-4,y2);
*/
 x_bar(x1+4,y1+4,x2-4,y2-4);
}

void shades2(int x1,int y1,int x2,int y2,int c3)
{
 int i=0;
 char pal[3],sh[4]={56,63,32,40};

 while(i<4)
 {
  pal[0]=pal[1]=pal[2]=sh[i];
  x_pal_1(NEWCOL+i,pal);
  i++;
 }

 i=0;
 while(i<2)
 {
  x_setcolor(NEWCOL+3-i);
  x_line(x1+i,y1+i,x2-i,y1+i);
  x_line(x1+i,y1+i,x1+i,y2-i);
  i++;
 }//loop

 i=0;
 while(i<2)
 {
  x_setcolor(NEWCOL+i);
  x_line(x1+i,y2-i,x2-i,y2-i);
  x_line(x2-i,y1+i,x2-i,y2-i);
  i++;
 }//loop
 if(c3<0)return;
 x_setfill(0,c3);
 x_bar(x1+2,y1+2,x2-2,y2-2);
}

#endif


/*
void Box3D(int x1,int y1,int x2,int y2)
{
 colorBox(x1,y1,x2,y2,15,8,7);
}
*/
#ifdef HICOLOR
void Box3Dv(int x1,int y1,int x2,int y2 )
{

 if(xg_256 == MM_Hic)
  shades(x1,y1,x2,y2);
 else
 colorBox(x1,y1,x2,y2,15,8,7);
// colorBox1pix(x1,y1,x2,y2,15,8,7);
}
#endif

/*
void Box3Dh(int x1,int y1,int x2,int y2)
{
#ifdef HICOLOR
 if(xg_256 == MM_Hic)
  shades(x1,y1,x2,y2);
 else
#endif
 colorBox(x1,y1,x2,y2,15,8,7);
// colorBox1pix(x1,y1,x2,y2,15,8,7);
}



void Box3D1pix(int x1,int y1,int x2,int y2)
{
 colorBox1pix(x1,y1,x2,y2,15,8,7);
}
*/
#ifdef HICOLOR
void Cell3D(int x1,int y1,int x2,int y2,int col)
{
 if(xg_256 == MM_Hic)
  shades2(x1,y1,x2,y2,col);
 else
 colorBox(x1,y1,x2,y2,8,15,col);
}
#endif

/*
void ColourBox3D(int x1,int y1,int x2,int y2, int nColour)
{
 colorBox(x1,y1,x2,y2,15,8,nColour);
}
*/

void Scratch3D(int x1,int y,int x2)
{
 x_setcolor(8);
 x_line(x1,y,x2,y);
 x_line(x1,y+4,x2,y+4);
 x_line(x1,y-4,x2,y-4);
 x_setcolor(15);
 x_line(x1,y+1,x2,y+1);
 x_line(x1,y+5,x2,y+5);
 x_line(x1,y-3,x2,y-3);
}

void Cross(int x,int y,int a)
{
 x_line(x,y,x+a,y+a);
 x_line(x,y+a,x+a,y);
 x_line(x,y+1,x+a,y+a+1);
 x_line(x,y+a+1,x+a,y+1);
}

void decorated_text(int x,int y,char *text,int *color)
{
 if(!cgamode)
 {
  if(color[0]!=-1)
  {
   x_setcolor(color[0]);
   x_text_ib(x-1,y-1,(unsigned char *)text);
  }
  if(color[1]!=-1)
  {
   x_setcolor(color[1]);
   x_text_ib(x+1,y-1,(unsigned char *)text);
  }
  if(color[2]!=-1)
  {
   x_setcolor(color[2]);
   x_text_ib(x-1,y+1,(unsigned char *)text);
  }
  if(color[3]!=-1)
  {
   x_setcolor(color[3]);
   x_text_ib(x+1,y+1,(unsigned char *)text);
  }
 }
 if(color[4]!=-1)
 {
  x_setcolor(color[4]);
  x_text_ib(x,y,(unsigned char *)text);
 }
}

#ifndef POSIX
char *pom_fbuf;
long pom_fntalloc;

//let's work with larger system fonts:
void bigfonts_allowed(void)
{
 if(memory_model && xg_fntalloc)
 {
  pom_fbuf=xg_fbuf;
  pom_fntalloc=xg_fntalloc;

  xg_fbuf = farmalloc( BIG_FONT_BUFFER );
  xg_fntalloc = BIG_FONT_BUFFER;
  memcpy(xg_fbuf,pom_fbuf,(int)pom_fntalloc);
 }
}

//let's reduce required font space:
void bigfonts_forbidden(void)
{
 if(memory_model && xg_fntalloc == BIG_FONT_BUFFER)
 {
  farfree(xg_fbuf);
  xg_fbuf=pom_fbuf;
  xg_fntalloc=pom_fntalloc;
  //!!!maybe we can speed up Arachne little if we store small font params ?
  xg_fnt_akt[0]='\0';
  currentfont=-1;
  currentstyle=-1;
 }
}
#endif

char egafilter(char i)
{
 if(i>54) return 63;

 if(i>47) return 47;

 if(i>35) return 31;

 if(i>8)return 15;

 return 0;
}


char vgafilter(char i)
{
 if(i>58) return 63;

 if(i>50) return 58;

 if(i>38) return 46;

 if(i>26)return 30;

 if(i>8)return 15;

 return 0;
}

struct Finf *finf=NULL;
extern char *FONTERR;

void err(char *text, char *msg);


//load selected font:

void htmlfont(int fnum, char style)
{
 char string[80];

 if(fnum==SYSFONT && user_interface.bigfont)
  {fnum=user_interface.bigfnum;style=user_interface.bigstyle;}
 else
 if(fnum!=SYSFONT)
 {
  fnum+=user_interface.fontshift;

  if(fnum<1)fnum=1;
  if(fnum>6)fnum=6;
 }

 if(currentfont==fnum &&
    (currentstyle&(BOLD|ITALIC|FIXED))==(style&(BOLD|ITALIC|FIXED)))
  return;

#ifdef POSIX
 strcpy(string,fntpath);
#else
 strcpy(string,exepath);
 strcat(string,"system\\");
#endif
 if(fnum==SYSFONT)
  strcat(string,"8x14");
 else
 {
  if(style & FIXED)
   strcat(string,"fixed");
  else
   strcat(string,"prop");

  if(style & BOLD)
   strcat( string,"b");
  else
   strcat(string,"n");

  if(style & ITALIC)
   strcat(string,"i0");
  else
   strcat(string,"n0");

  string[strlen(string)-1]+=fnum;
 }
 strcat(string,".fnt");
 currentfont=fnum;
 currentstyle=style;
 if(x_fnt_load(string, 25, 1)!=1)
  err(FONTERR,string);
}

char fontx(int fnum,char style,unsigned char z)
{

 if(fixedfont)
  return FIXEDFONTX;

 if(fnum==SYSFONT)
 {
  if(user_interface.bigfont)
   {fnum=user_interface.bigfnum;style=user_interface.bigstyle;}
  else
   return 8; //8x14
 }
 else
  fnum+=user_interface.fontshift;

 if(fnum<1)fnum=1;
 if(fnum>6 )fnum=6;

 if(style & FIXED)
  return(finf->fixed_x[fnum-1][style & (BOLD|ITALIC)]);
 else
  return(finf->prop_x[fnum-1][style & (BOLD|ITALIC)][z-32]);
}

char fonty(int fnum,char style)
{
 if(fixedfont)
  return FIXEDFONTY;

 if(fnum==SYSFONT)
 {
  if(user_interface.bigfont)
   {fnum=user_interface.bigfnum;style=user_interface.bigstyle;}
  else
   return 14; //8x14
 }
 else
  fnum+=user_interface.fontshift;

 if(fnum<1)fnum=1;
 if(fnum>6)fnum=6;

 return(finf->y[fnum-1][style & (BOLD|ITALIC|FIXED)]);
}

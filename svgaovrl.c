
// ========================================================================
// Initialization of graphics, graphics functions
// (c)1997,1998,1999,2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "internet.h"

char *FONTERR=MSG_FNTERR;

void err(char *text, char * msg)
{
 unsigned long f;

 x_grf_mod(3);
 puts(text);
 puts(msg);
#ifndef POSIX
 f=farcoreleft();
 printf(MSG_MEMLFT,f);
#endif
 exit(EXIT_ABNORMAL); //go to FATAL.HTM...
}//end sub

/*
extern char *anykey;
void zoommsg(void)
{
 printf(MSG_F5ZOOM,anykey);
 getch();
}
*/

extern char egamode,cgamode,vga16mode,vgamono;

//-------------------- global variable (IBASE, X_LOPIF) ---------------

//char commBase[64];
//char scratchBase[64];


char *vgadetected=NULL;

//===========================================================================
void detectgraphics(void)
//===========================================================================
{
 //char Grafmode[30];  //requested graphical mode -
 int i;

 vgadetected=farmalloc(30);
 memset(vgadetected,0,30);
 x_detect(vgadetected, &i);

// puts(Grafmode);
// exit(EXIT_TO_DOS);
}


#ifndef POSIX
//===========================================================================
void graphicsinit(char *svgamode)
//===========================================================================
{

char *ptr,Grafmode[30];  //requested graphical mode -
int irc,gmode;

//!!glennmcc: begin Feb 11, 2005
//moved to config.c to make it configurable via CursorType in arachne.cfg
//the next block  of data is the original cursor
/*
const short cur[32] =
         { 0x9FFF, 0x0FFF, 0x07FF, 0x83FF, 0xC1FF, 0xE0FF, 0xF067, 0xF003,
           0xF001, 0xF000, 0xF800, 0xF800, 0xF800, 0xFC00, 0xFC00, 0xFC00,
           0x0000, 0x6000, 0x7000, 0x3800, 0x1C00, 0x0E00, 0x0700, 0x0018,
           0x07EC, 0x07EE, 0x001E, 0x03EE, 0x03EE, 0x001E, 0x00EC, 0x0002 };
*/
//!!glennmcc:end

//------------------------------------------------------

//exit with invalid gr. mode?
if(svgamode[0]=='\0')
 goto GrError;

strcpy(Grafmode,svgamode);
/*

//goto setmode;}

Ask:

memset(Grafmode,0,30);
x_detect(Grafmode, &i);

puts("\n\nX_LOPIF graphics library, Copyright (c) 1990,1998 Zdenek Harovnik+IBASE, Praha\n");

puts("v. VESA standard (VESA card or VESA driver, 256 KB [X-mode] ... 2 MB [HiColor])");
puts("t. Trident (or PROVGA_VC510S)");
puts("a. Tseng ET3000 (or MDB10)");
puts("b. OAK");
puts("c. Tamarack TDVGA-3588 (or CIRRUS, PARADISE, AST-PLUS)");
puts("d. RTVGA-Realtek");
puts("e. Tseng ET4000");
puts("f. M1 (or OCTEK, graphic chip MX86010)");
puts("g. EGA (16 colors)");
puts("h. VGA (256 KB card, 16 colors)");
puts("m. VGA with monochromatic palette (256 KB card, 16 colors)");
puts("i. CGA (experimental implementation!)");
if(Grafmode[0])
 printf("x. Autodetect (%s detected)\n",Grafmode);
puts("q. Quit (no video card recognized)\n");

printf(MSG_SELVGA);

x:

i=toupper(getch());

if (i=='Q') exit(EXIT_TO_DOS); //exit batch!
else
if (i=='T') strcpy(Grafmode,"TRIDENT");
else
if (i=='V') strcpy(Grafmode,"VESA");
else
if (i=='A') strcpy(Grafmode,"TSG3");
else
if (i=='B') strcpy(Grafmode,"OAK");
else
if (i=='C') strcpy(Grafmode,"TAMARA");
else
if (i=='D') strcpy(Grafmode,"REALTEK");
else
if (i=='E') strcpy(Grafmode,"TSG4");
else
if (i=='F') strcpy(Grafmode,"M1");
else
if (i=='G')
{strcpy(Grafmode,"EGA");zoommsg();goto resolutionok;}
else
if (i=='H')
{strcpy(Grafmode,"VGA");zoommsg();goto resolutionok;}
else
if (i=='M')
{strcpy(Grafmode,"VGAMONO");zoommsg();goto resolutionok;}
else
if (i=='I')
{strcpy(Grafmode,"BCGA");zoommsg();goto resolutionok;}
else
 if (i!='X') goto x;

puts("\n");
if(!strcmpi(Grafmode,"VESA") || !strcmpi(Grafmode,"REALTEK"))
 puts("x. 640x400,256 colors (256 KB)");
puts("b. 640x480,256 colors (512 KB)");
puts("c. 800x600,256 colors (512 KB, recommended)");
puts("e. 1024x768,256 colors (1 MB)");
#ifdef HICOLOR
if(!strncmpi(Grafmode,"VESA",4))
{
 puts("1. 640x480,HiColor (32/64K colors, 1MB)");
 puts("2. 800x600,HiColor (32/64K colors, 1MB, recommended)");
 puts("3. 1024x768,HiColor (32/64K colors, 2MB)");
 puts("4. 1280x1024,HiColor (32/64K colors, 4MB)");
 puts("5. 1600x1200,HiColor (32/64K colors, 4MB)");
}
#endif
printf("\n");

printf(MSG_RESOL);

{
 char m[3];

 resolution:
 i=toupper(getch());
 sprintf(m,".%c",i);
 if (i=='X' || i=='B' || i=='C' || i=='E')
  strcat(Grafmode,m);
#ifdef HICOLOR
 else if (i=='1')
  strcpy(Grafmode,"Hi16.I");
 else if (i=='2')
  strcpy(Grafmode,"Hi16.J");
 else if (i=='3')
  strcpy(Grafmode,"Hi16.K");
 else if (i=='4')
  strcpy(Grafmode,"Hi16.L");
 else if (i=='5')
  strcpy(Grafmode,"Hi16.M");
#endif
 else
  goto resolution;
}

resolutionok:

strcpy(svgamode,Grafmode);
*/

setmode:

  // Setting graphical mode. change VGAMONO -> VGA and set vgamono=1
  if(!strcmpi(Grafmode,"VGAMONO"))
  {
   strcpy(Grafmode,"VGA");
   vgamono=1;
  }
  // Change modes end with .M to .A and set vgamono=1 except for HiColor modes
  if (((ptr=strstr(Grafmode,".M"))!=NULL) && (strstr(Grafmode,"Hi1")==NULL) && (strstr(Grafmode,"HI1")==NULL))
  {
   strcpy(ptr,".A");
   vgamono=1;
  }

  irc = x_rea_svga( NULL, Grafmode, &gmode) + x_grf_mod( gmode );
  if(irc != 2)
  {
#ifdef HICOLOR
    if(Grafmode[3]=='6')
    {
     Grafmode[3]='5';
     goto setmode;
    }
#endif
    //exit with invalid gr. mode!
    goto GrError;
  }

//!!glennmcc: begin Feb 11, 2005
//moved to config.c to make it configurable via CursorType in arachne.cfg
//origianl sinle line follows
//  x_defcurs( (short *)cur, (short *)&cur[16], 15); //mouse cursor
//!!glennmcc: end

 if(!strcmpi(Grafmode,"EGA"))
  egamode=1;
 else
 if(!strcmpi(Grafmode,"VGA") || strstr(Grafmode,".A"))
  vga16mode=1;
 else
 if(!strcmpi(Grafmode,"BCGA"))
  cgamode=1;

 initpalette();

 x_setcolor(15); //white!
 x_charmod(1);

 return;

 //========================================================================
 //error while opening graphic >>
 //========================================================================
 GrError:
 x_grf_mod(3);
 puts(MSG_VERR1);
 exit(EXIT_GRAPHICS_ERROR);

/*
  x_grf_mod(3);  //error while opening graphic >>

  puts(MSG_VERR1);
  puts(MSG_VERR2);
/*
#ifdef VIRT_SCR
  puts(MSG_VERR3);
#endif
*/
//  goto Ask;

}
#endif

#ifdef OVRL
#ifndef XTVERSION

void finfo(void)
{
 int f=1,i;
 char str[3]="\0\0";
 char nibble,style;

 if(!finf)
  finf=farmalloc(sizeof(struct Finf));
 if(!finf)
  memerr0();

 while(f<7)
 {
  style=0;
  while(style<8)
  {
   htmlfont(f,style);
   finf->y[f-1][style]=x_txheight("A");

   //corrections apply to font Arial !!!
   if(style==0)
   {
    if(f>2)finf->y[f-1][style]--;
    if(f==6)finf->y[f-1][style]--;
   }
   else
   if(style & FIXED)
   {
    if(f>3)finf->y[f-1][style]--;
   }
   else
   if(style == ITALIC)
   {
    if(f<3)finf->y[f-1][style]++;
    if(f>2)finf->y[f-1][style]--;
    if(f>3)finf->y[f-1][style]--;
   }
   else
   if(style == BOLD)
   {
    if(f==1)finf->y[f-1][style]++;
    if(f==3)finf->y[f-1][style]--;
    if(f==5)finf->y[f-1][style]--;
    if(f==6)finf->y[f-1][style]-=3;
   }
   else
   {
    if(f>2)finf->y[f-1][style]--;
    if(f==3)finf->y[f-1][style]--;
    if(f==5)finf->y[f-1][style]--;
    if(f==6)finf->y[f-1][style]--;
   }
   printf("%2d: y=%2d\n",f,finf->y[f-1][style]);

   nibble=style & (BOLD|ITALIC);
   if(style & FIXED)
    finf->fixed_x[f-1][nibble]=x_txwidth("A");
   else
   {
    i=0;
    while(i<224)
    {
     str[0]=i+32;
     finf->prop_x[f-1][nibble][i]=x_txwidth(str);
     i++;
    }
   }
   style++;
  }
  f++;
 }
// htmlfont(SYSFONT,0);

//!!glennmcc: Mar 06, 2008 -- preserve entity data at end of original fontinfo.bin
 f=a_open(fntinf,O_WRONLY|O_BINARY,S_IREAD|S_IWRITE);
//f=a_open(fntinf,O_WRONLY|O_CREAT|O_BINARY,S_IREAD|S_IWRITE);//original line
 if(f!=-1)
 {
  write(f,finf,sizeof(struct Finf)-768);
//write(f,finf,sizeof(struct Finf));//original ine
//!!glennmcc: end
  a_close(f);
  puts(fntinf);
  exit(EXIT_TO_DOS); //!!glennmcc: Jun 13, 2005
 }
 else
 {
  perror(fntinf);
  exit(EXIT_TO_DOS);
 }
}
#endif
#endif

void finfoload(void)
{
 int f;

 if(!finf)
  finf=farmalloc(sizeof(struct Finf));
 if(!finf)
  memerr0();

 f=a_open(fntinf,O_RDONLY|O_BINARY,0);
 if(f!=-1)
 {
  if(a_read(f,finf,sizeof(struct Finf))!=sizeof(struct Finf))
   goto fontinferr;
  a_close(f);
 }
 else
 {
  fontinferr:
  err(FONTERR,fntinf);
 }
/* always!!
 if(!strcmp(finf->entity[32],"nbsp"))
  ascii160hack=1;
*/
}



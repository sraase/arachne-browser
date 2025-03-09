
// ========================================================================
// GRAPHICAL USER INTERFACE for Arachne WWW browser - GUI init & redraw
// (c)1997,1998,1999,2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "html.h"
#include "gui.h"

#define MOUSEPIX 4

int bioskey(int cmd);
//customizable toolbar icons:

void toolbar(char newtoolbarmode,char forced)
{
 focusedatom=activeadr;
 if(toolbarmode==newtoolbarmode && forced)
  return;
 mouseoff();
 toolbarmode=newtoolbarmode;
 if(arachne.GUIstyle!=STYLE_SMALL1 && arachne.GUIstyle!=STYLE_SMALL2
    && !fullscreen)
 {
  if(arachne.GUIstyle || x_maxx()<640)
  {
   Box3D(450,0,x_maxx()-152,48);

   if(!user_interface.iconsoff && toolbarmode==0)
   {
    int iconx=452,icony=3,iconcount=1;
    char icon[9],dummy[80],iconkey[13];
    while(iconx<x_maxx()-184)
    {
     sprintf(iconkey,"H?%d",iconcount++);

     if(toolbarpage)
      iconkey[1]=toolbarpage;
     else if(tcpip)
      iconkey[1]='I';
     else
      iconkey[1]='D';

     if(geticoninfo(iconkey,icon,dummy,dummy,dummy,dummy))
     {
      if(icon[0]=='*')
      {
       x_setcolor(0);
       htmlfont(1,0);
       x_settextjusty(1,1);
       x_text_ib(iconx+17,icony+10,(unsigned char *)&icon[1]);
      }
      else
       DrawIconLater( icon, iconx,icony);
     }

     if(icony==27)
      iconx+=32;
     icony=30-icony; //3 or 27
    }
   }
   else
   if(toolbarmode)
   {
    DrawIconLater( "SMALL2", 452,3);
    DrawIconLater( "SMALL1", 452,27);
    if(x_maxx()>=640)
    {
     DrawIconLater( "BLK_CUT",484,3);
     DrawIconLater( "BLK_CLIP",484,27);
     DrawIconLater( "BLK_PAST",516,3);
     DrawIconLater( "BLK_COPY",516,27);
     DrawIconLater( "BLK_MOVE",548,3);
     DrawIconLater( "BLK_DEL",548,27);
     DrawIconLater( "SMALL4",580,3);
     //DrawIconLater( "SMALL3",580,27);
    }
   }//end if
  }
  else
  {
   Box3D(x_maxx()-150,250,x_maxx()-1,endvtoolbar());
   htmlfont(1,0);
   if(toolbarmode==0)
   {
    int icony=253,iconcount=1;
    char icon[9],dummy[80],desc1[40],desc2[10],iconkey[13];
    while(icony<endvtoolbar()-20)
    {
     sprintf(iconkey,"V?%d",iconcount++);

     if(toolbarpage)
      iconkey[1]=toolbarpage;
     else if(tcpip)
      iconkey[1]='I';
     else
      iconkey[1]='D';

     if(geticoninfo(iconkey,icon,dummy,dummy,desc1,desc2))
     {
      if(!user_interface.iconsoff && icon[0]!='*')
       DrawIconLater( icon,x_maxx()-148,icony);

      x_setcolor(0);
      x_text_ib(x_maxx()-114,icony+2,(unsigned char *)desc1);
      x_settextjusty(2,2);
      x_text_ib(x_maxx()-6,icony+2,(unsigned char *)desc2);
      x_settextjusty(0,2);  // vzdycky psat pismo od leveho horniho rohu
     }
     icony+=18;
    }
   }
   else
   {
    DrawIconLater( "SMALL2",x_maxx()-148,253);
    DrawIconLater( "SMALL1",x_maxx()-148,271);
    DrawIconLater( "BLK_CUT",x_maxx()-148,289);
    DrawIconLater( "BLK_CLIP",x_maxx()-148,307);
    DrawIconLater( "BLK_PAST",x_maxx()-148,325);
    DrawIconLater( "BLK_COPY",x_maxx()-148,343);
    DrawIconLater( "BLK_MOVE",x_maxx()-148,361);
    DrawIconLater( "BLK_DEL",x_maxx()-148,379);
    DrawIconLater( "SMALL4",x_maxx()-148,397);
    //DrawIconLater( "SMALL3",x_maxx()-148,415);
    x_setcolor(0);
    x_text_ib(x_maxx()-114,255,(unsigned char *)MSG_WRITEF);
    x_text_ib(x_maxx()-114,273,(unsigned char *)MSG_READF);
    x_text_ib(x_maxx()-114,291,(unsigned char *)MSG_BLKCUT);
    x_text_ib(x_maxx()-114,309,(unsigned char *)MSG_BLKCLP);
    x_text_ib(x_maxx()-114,327,(unsigned char *)MSG_BLKPST);
    x_text_ib(x_maxx()-114,345,(unsigned char *)MSG_BLKCOP);
    x_text_ib(x_maxx()-114,363,(unsigned char *)MSG_BLKMOV);
    x_text_ib(x_maxx()-114,381,(unsigned char *)MSG_BLKDEL);
    x_text_ib(x_maxx()-114,399,(unsigned char *)MSG_SRCH4);
    //x_text_ib(x_maxx()-114,417,(unsigned char *)MSG_PRT);
    x_settextjusty(2,2);
    x_text_ib(x_maxx()-6,255,(unsigned char *)"Ctrl+W");
    x_text_ib(x_maxx()-6,273,(unsigned char *)"Ctrl+R");
    x_text_ib(x_maxx()-6,291,(unsigned char *)"Ctrl+X");
    x_text_ib(x_maxx()-6,309,(unsigned char *)"Ctrl+C");
    x_text_ib(x_maxx()-6,327,(unsigned char *)"Ctrl+V");
    x_text_ib(x_maxx()-6,345,(unsigned char *)"Ctrl+K,C");
    x_text_ib(x_maxx()-6,363,(unsigned char *)"Ctrl+K,M");
    x_text_ib(x_maxx()-6,381,(unsigned char *)"Ctrl+K,Y");
    x_text_ib(x_maxx()-6,399,(unsigned char *)"Ctrl+Q");
    //x_text_ib(x_maxx()-6,417,(unsigned char *)"Ctrl+P");
    x_settextjusty(0,2); // vzdycky psat pismo od leveho horniho rohu
   }
  }
 }
 DrawIcons();
 mouseon();
 x_settextjusty(0,2);  // vzdycky psat pismo od leveho horniho rohu
}

//big icons

void buttons(void)
{
 if(fullscreen)
  return;

 if(arachne.GUIstyle==STYLE_SMALL1)
 {
  DrawIconLater( "alticon1",4,3 );
  DrawIconLater( "alticon2",x_maxx()-146,3 );
  Box3Dh(0,p->htscrn_ytop-25,150,p->htscrn_ytop-2);
  Box3Dh(x_maxx()-150,p->htscrn_ytop-25,x_maxx(),p->htscrn_ytop-2);
 }
 else
 if(arachne.GUIstyle==STYLE_SMALL2)
 {
  if(!user_interface.iconsoff)
  {
  DrawIconLater( "alticon2",x_maxx()-146,4 );
  DrawIconLater( "alticon1",x_maxx()-146,27 );
  }
  Box3Dh(x_maxx()-150,p->htscrn_ytop-50,x_maxx(),p->htscrn_ytop-2);
 }
 else
 if(arachne.GUIstyle || x_maxx()<640)
 {
  //char str[10];

  DrawIconLater( "buttons1",0,0 );
  DrawIconLater( "buttons2",150,0 );
  DrawIconLater( "buttons3",300,0 );
 }
 else //toolbar on the right side
 {
  //char str[10];

  DrawIconLater( "buttons1",x_maxx()-150,100 );
  DrawIconLater( "buttons2",x_maxx()-150,150 );
  DrawIconLater( "buttons3",x_maxx()-150,200 );
  //menu
 }
 toolbar(0,0);
}


void GUIInit(void)	       // inicializace mysi, etc.
{
 if(!mousex && !mousey ||
    mousex>x_maxx()-MOUSEPIX ||
    mousey>x_maxy()-MOUSEPIX)
 {
  mousex=x_maxx()/2;
  mousey=x_maxy()/2;
 }
 ImouseIni( 0, 0, x_maxx()-MOUSEPIX, x_maxy()-MOUSEPIX, mousex , mousey );
 ImouseRead(&mousex,&mousey);
 mouseon();
}


int bannerwidth=0;

char *gettitle(char *buf)
{
  char *title=buf;
  strcpy(title,"Arachne ");
  strcat(title,VER);

 htmlfont(3,BOLD);
 bannerwidth=x_txwidth (title);

 return title;
}

void PaintTitle(void)	 // vykresleni nazvu stranky
{
 if(fullscreen)
  return;
 mouseoff();
 if(arachne.GUIstyle==STYLE_SMALL1)
 {
  Box3Dh(152,p->htscrn_ytop-25,x_maxx()-152,p->htscrn_ytop-2);
  DrawIconLater( "SMALL_WM",x_maxx()-187,p->htscrn_ytop-22);
  URLprompt.xx=x_maxx()-192-user_interface.scrollbarsize;
  if(user_interface.scrollbarsize>3)
   Scratch3D(URLprompt.xx+3,p->htscrn_ytop-14,URLprompt.xx+user_interface.scrollbarsize-2);
 }
 else
 {
//!!glennmcc: Begin Jan 20, 2006
// 4 choices at build-time for 'Arachne ver#' color/style
  int colormap[5]={15,-1,-1,7,8};//high-contrast raised
//int colormap[5]={7,-1,-1,15,8};//high-contrast indented
//int colormap[5]={15,-1,-1,8,7};//low-contrast raised
//int colormap[5]={8,-1,-1,15,7};//low_contrast indented
//!!glennmcc: end
//int colormap[5]={8,-1,-1,15,7};//original line
//!!glennmcc: Mar 15, 2006 -- inserted description & short color list
/*
{15,-1,-1,7,8};//high-contrast raised
_^^<--top_edge/left_edge color
{15,-1,-1,7,8}
__________^<--bottom_edge/right_edge color
{15,-1,-1,7,8}
____________^<--fill color
0  BLACK
7  LIGHTGRAY
8  DARKGRAY
15 WHITE
*/
//!!glennmcc: end

  char buf[64];
  char *title;

  Box3D(0,p->htscrn_ytop-50,x_maxx()-152,p->htscrn_ytop-27);
  DrawIconLater( "SMALL_WM",x_maxx()-187,p->htscrn_ytop-46);
  if(fonty(SYSFONT,0)<=16)
  {
   Box3D(0,p->htscrn_ytop-25,x_maxx()-152,p->htscrn_ytop-2);
   URLprompt.xx=x_maxx()-152-user_interface.scrollbarsize;
   if(user_interface.scrollbarsize>3)
    Scratch3D(URLprompt.xx+3,p->htscrn_ytop-13,x_maxx()-155);
  }
  else
  {
   Box3D(0,p->htscrn_ytop-25,48,p->htscrn_ytop-2);
   URLprompt.xx=x_maxx()-152;
  }
  x_setcolor(0);
  title=gettitle(buf);

  decorated_text(10,p->htscrn_ytop-47,MSG_TITLE,colormap);
  decorated_text(10,p->htscrn_ytop-22,"URL",colormap);
  x_settextjusty(2,2);
  decorated_text(x_maxx()-192,p->htscrn_ytop-47,title,colormap);
  x_settextjusty(0,2);	      // vzdycky psat pismo od leveho horniho rohu
  x_setcolor(0);
 }
}

void PaintStatus(void)
{
  Box3D(0,x_maxy()-15,x_maxx()-152,x_maxy());
}

void RedrawALL()		       // redraw entire user interface
{
 char oldshift=user_interface.fontshift;
 user_interface.fontshift=0;
 global_nomouse=1;
 //
 //turn on special XSWAP optimization for speed
 swapoptimize=1;
 InitIcons();
 resetcolorcache();
 xChLogo('0');
 if(!noGUIredraw)
 {
  PaintTitle();
  buttons();
 }
 else
 {
  if(arachne.GUIstyle==STYLE_SMALL1)
   URLprompt.xx=x_maxx()-192-user_interface.scrollbarsize;
  else
  if(fonty(SYSFONT,0)<=16)
   URLprompt.xx=x_maxx()-152-user_interface.scrollbarsize;
  else
   URLprompt.xx=x_maxx()-192-user_interface.scrollbarsize;
 }
 user_interface.fontshift=oldshift;
 PaintStatus();
 defaultmsg();
 statusmsg();
 MemInfo(NORMAL);
 noGUIredraw=0;
 global_nomouse=0;
 lasthisx=-1;
 //turn on special XSWAP optimization for speed
 swapoptimize=0;
}

void ChangeZoom(char style, char plus, char minus)
{
 char *ptr=strchr(arachne.graphics,'.');
//!!glennmcc: Feb 17, 2005 -- to prevent going over MaxRes
int maxres = config_get_int("MaxRes", 0);
//!!glennmcc: end

 mouseoff();
 //1. 800x600 or more na vysku -> 2.
 //2. 800x600 or more na sirku -> 3.
 //3. 600x480 -> 1.
 if(style)
 {
  arachne.GUIstyle++;
  if(arachne.GUIstyle>STYLE_SMALL2)
  {
   arachne.GUIstyle=STYLE_ARACHNE;
   if(!ptr || ptr[1]!='A' && ptr[1]!='C' && ptr[1]!='E' && ptr[1]!='J' && ptr[1]!='K' && ptr[1]!='L' && ptr[1]!='M')
    arachne.GUIstyle++;
  }

  x_cleardev();
 }
 else if(!ptr)
   return;
 else
 {
  if(plus) // prepni na vyssi rozlisnei
  {
   if(!strncmpi(arachne.graphics,"VESA.X",6)) //X==640x400x256c
    ptr[1]='B';         //B==640x480x256c
   else if(ptr[1]=='B' && maxres > 1)
    ptr[1]='C';         //C==800x600x256c
   else if(ptr[1]=='C' && maxres > 2)
    ptr[1]='E';         //E==1024x768x256c
#ifdef HICOLOR
	          	//I==640x480xHiColor
   else if(ptr[1]=='I' && maxres > 1)
    ptr[1]='J';         //J==800x600xHiColor
   else if(ptr[1]=='J' && maxres > 2)
    ptr[1]='K';         //K==1024x768xHiColor
   else if(ptr[1]=='K' && maxres > 3)
    ptr[1]='L';         //L==1280x1024xHiColor
   else if(ptr[1]=='L' && maxres > 4)
    ptr[1]='M';         //M==1600x1200xHiColor
#endif
  }
  else if(minus) // prepni na nizsi rozlisnei
  {
   if(ptr[1]=='E')      //E==1024x768x256c
    ptr[1]='C';         //C==800x600x256c
   else if(ptr[1]=='C')
    ptr[1]='B';         //B==640x480x256c
#ifdef HICOLOR
   else if(ptr[1]=='M') //M==1600x1200xHiColor
    ptr[1]='L';
   else if(ptr[1]=='L') //L==1280x1024xHiColor
    ptr[1]='K';
   else if(ptr[1]=='K') //K==1024x768xHiColor
    ptr[1]='J';
   else if(ptr[1]=='J') //J==800x600xHiColor
    ptr[1]='I'; 	//I==640x480xHiColor
#endif
   else if(!strncmpi(arachne.graphics,"VESA.B",6))
    ptr[1]='X';//X==640x400x256c
  }
  graphicsinit(arachne.graphics); // XLOPIF SVGA GRAPHICS
 }

 lastonbutton=-1;
 zoom();
 lastonbutton=0;
 arachne.target=0; //frameset homepage will be reloaded....
 RedrawALL();
 GUIInit();
#ifdef POSIX
 config_set_str("GraphicsMode", arachne.graphics);
 ie_savef(&ARACHNEcfg);
#else
 savepick();
#endif
 GLOBAL.nothot=1;
 GLOBAL.needrender=1;
 GLOBAL.validtables=0; //tabulky je potreba prepocitat!!!
}

void statusmsg(void)
{
 char *msg;

 if(fullscreen)
  return;

 mouseoff();

 Box3D(x_maxx()-150,x_maxy()-15,x_maxx(),x_maxy());
 x_setcolor(0);

 if(httpstub)
 {
  x_setcolor(1); //blue
  msg="http/stub";
 }
 else if(tcpip>0)
 {
  x_setcolor(3); //green
  msg="TCP/IP";
 }
 else
 {
  x_setcolor(8); //dark grey
  msg="OFFLINE";
 }
 x_settextjusty(2,2);
// x_text_ib(x_maxx()-29,x_maxy()-15,(unsigned char *)msg);
 x_text_ib(x_maxx()-4,x_maxy()-15,(unsigned char *)msg);
 x_settextjusty(0,2);	     // vzdycky psat pismo od leveho horniho rohu

 mouseon();
}

//===========================================================================
//imlementation of line redraw for ib_editor's ie_* family of functions...
//===========================================================================
void ie_redrawline(struct ib_editor *fajl,int x1,int y1, int zoomx, int width,int i)
{
 char *ptr,txt[160];
 int l;
 int bbx,bex,bby,bey,block=0;

 if(width>=160)width=159;
 //memset(txt,' ',width);

 if(fajl->blockflag==2 && i>=fajl->bby && (i<fajl->bey || i==fajl->bey && fajl->bex>0))
 {
  block=1;
  bbx=fajl->bbx;
  bby=fajl->bby;
  bex=fajl->bex;
  bey=fajl->bey;
 }

 if(i<fajl->lines)
 {
  if(fajl->aktrad==i)
  {
   l=strlen(fajl->rad)-zoomx;
   if(l>0)
   {
    if(l>=160)l=159;
    strncpy(txt,&fajl->rad[zoomx],l);
   }
   else
    l=0;
  }
  else
  {
   ptr=ie_getswap(getXSWAPlineadr(fajl,i));
   l=strlen(ptr)-zoomx;
   if(ptr && l>0)
   {
    if(l>=160)l=159;
    strncpy(txt,&ptr[zoomx],l);
   }
   else
    l=0;
  }
 }
 else
  l=0;

 if(l>0 && txt[l-1]=='\r')
  l--;

 if(width>l)
  memset(&txt[l],' ',width-l);
 txt[width]='\0';

 htmlfont(SYSFONT,0);
 if(fonty(SYSFONT,0)!=14)
  y1-=1;

 if(!block || bbx>0 || bex<=l)
 {
  x_setcolor(user_interface.ink);
  x_setfill(0,user_interface.paper);
 }
 else
 {
  x_setcolor(BLOCK_INK);
  x_setfill(0,BLOCK_PAPER);
 }

 if(!xg_video_XMS)
  x_charmod(0);

 x_text_ib(x1,y1,(unsigned char *)txt);

 if(block && (bbx-zoomx>0 || bex-zoomx<=l))
 {
  char *ptr=txt;
  int xshift=0;

  if(bby==i && bbx-zoomx>0)
  {
   xshift=bbx-zoomx;
   ptr=&txt[xshift];
  }
  if(bey==i && bex-zoomx<=l && bex-zoomx>0)
   txt[bex-zoomx]='\0';
  x_setcolor(BLOCK_INK);
  x_setfill(0,BLOCK_PAPER);
  x_text_ib(x1+space(SYSFONT)*xshift,y1,(unsigned char *)ptr);
 }

 if(!xg_video_XMS)
  x_charmod(1);

}//end sub


//===========================================================================
//imlementation of window redraw for ib_editor's ie_* family of functions...
int ie_redrawwin(struct ib_editor *fajl,int x1,int y1, int x2, int y2,char allowkey)
//===========================================================================
{
 int i=0,max;
 int width,y;
// int dummyx,dummyy;

 if(fajl->x-fajl->zoomx>(x2-x1)/fontx(SYSFONT,0,' '))
  fajl->zoomx=fajl->x-(x2-x1)/fontx(SYSFONT,0,' ');
 if(fajl->y-fajl->zoomy>(y2-y1)/fonty(SYSFONT,0))
  fajl->zoomy=fajl->y-(y2-y1)/fonty(SYSFONT,0);

 width=(x2-x1)/fontx(SYSFONT,0,' ');
 if(width>=160)width=159;

 //prekresleni...
 max=(y2-y1)/fonty(SYSFONT,0);
 y=y1;
 while(i<max)
 {
  ie_redrawline(fajl,x1,y,fajl->zoomx,width,i+fajl->zoomy);
  i++;
  y+=fonty(SYSFONT,0);
  if(allowkey)
  {
   int dummy;
   wheelqueue=ImouseRead(&dummy,&dummy)>>8; //gui.h variable
   if(wheelqueue || bioskey(1)) //key pressed or mouse wheel moved
   {
    ie_redrawline(fajl,x1,y1+(fajl->y-fajl->zoomy)*fonty(SYSFONT,0),fajl->zoomx,width,fajl->y);
    atomneedredraw=1;
    return 1; //nedokoncene prekreslovani !!!
   }//endif
  }
 }//loop

 return 0;
}//end sub


//===========================================================================
//output single line of drop-down meny (<OPTION> tag...)
void putoptionline(int x,int y,int limit,struct ib_editor *fajl,int line,char multi)
//===========================================================================
{
 char *val=ie_getline(fajl,line);
 char str[IE_MAXLEN+2],on=0;
 int charlim;

 if(*val=='1')
  on=1;

 x_setcolor(0); //black (tlacitko)
 htmlfont(OPTIONFONT,0);
 strcpy(str,ie_getline(fajl,line+1));
 charlim=x_charmax((unsigned char *)str,limit);
 if(charlim<0 || charlim>IE_MAXLEN)
  charlim=0;
 str[charlim]='\0';

 /*if(fonty(SYSFONT,0)!=14)
  y-=1;*/
 x_text_ib(x+space(SYSFONT)+4,y,(unsigned char *)str);

 if(on)
 {
  if(multi)
   Cross(x+2,y+4,space(SYSFONT)-2);
  else
  {
   x_setfill(0,0);
   x_bar(x+3,y+4,x+space(SYSFONT)-1,y+fonty(OPTIONFONT,0)-6);
  }
 }
}


void DrawTitle(char force)    // vykresleni nazvu stranky
{
 int l;
 char str[256];
 char *titleptr;

 if(fullscreen)
  return;

 mouseoff();

 drawatom(&URLprompt,0u,0,p->htscrn_xsize,p->htscrn_ysize,p->htscrn_xtop,p->htscrn_ytop);
 activeatomptr=NULL; //pod mysi neni nic

 if(title_ok && !force)
  return;

 x_setfill(0,7);
 x_setcolor(0);
 if(!bannerwidth)
 {
  char buf[64];
  gettitle(buf);
 }


//!!glennmcc: Aug 22, 2005 -- maintain size independant of fontshift
if(user_interface.fontshift<0) htmlfont(3+user_interface.fontshift,0); else
 htmlfont(3-user_interface.fontshift,0);
// htmlfont(3,0);

 if(arachne.GUIstyle==STYLE_SMALL1)
 {
  x_bar(156,p->htscrn_ytop-22,x_maxx()/2-(x_maxx()-300)/6-1,p->htscrn_ytop-5);
  l=x_charmax((unsigned char *)arachne.title,(x_maxx()-300)/3-12);
 }
 else
 {
  x_bar(50,p->htscrn_ytop-47,x_maxx()-196-bannerwidth,p->htscrn_ytop-29);
  l=x_charmax((unsigned char *)arachne.title,x_maxx()-266-bannerwidth);
 }


 if(l<strlen(arachne.title))
 {
  titleptr=str;
  makestr(titleptr,arachne.title,l);
  strcat(titleptr,"...");
 }
 else
  titleptr=arachne.title;


 x_settextjusty(0,1);	     // na stred!

 if(arachne.GUIstyle==STYLE_SMALL1)
  x_text_ib(156,p->htscrn_ytop-12,(unsigned char *)titleptr);
 else
  x_text_ib(50,p->htscrn_ytop-37,(unsigned char *)titleptr);
 x_settextjusty(0,2);	     // vzdycky psat pismo od leveho horniho rohu

 mouseon();
 title_ok=1;
}



#ifndef XTVERSION

// Only for overlay, when the mouse on a button, it pops up a little to indicate that it is active
void hidehighlight(void)
{
 if((lastonbutton>9 && lastonbutton<CLICK_SPECIAL || htmlpulldown ||
     arachne.GUIstyle==STYLE_SMALL1 || arachne.GUIstyle==STYLE_SMALL2)
    && lasthisx>=0)
 {
  mouseoff();
  x_setcolor(7);
  x_rect(lasthisx,lasthisy,lasthisxx,lasthisyy);
  mouseon();
  lasthisx=-1;
 }
}

void showhighlight(void)
{
 mouseoff();
 lasthisx=thisx;
 lasthisy=thisy;
 lasthisxx=thisxx-3;
 lasthisyy=thisyy-3;

 x_setcolor(15);
 x_line(lasthisx,lasthisy,lasthisxx,lasthisy);
 x_line(lasthisx,lasthisy,lasthisx,lasthisyy);
 x_setcolor(8);
 x_line(lasthisxx,lasthisy,lasthisxx,lasthisyy);
 x_line(lasthisx,lasthisyy,lasthisxx,lasthisyy);
 mouseon();
}

void onlinehelp(int b)
{
 hidehighlight();

 if(b>9 && b<CLICK_SPECIAL && toolbarmode==0)
 {
  //custozmizable Arachne toolbar
  char iconkey[5],dummy[80],desc1[40],desc2[10];

  sprintf(iconkey,"??%d",b/10);

  if(arachne.GUIstyle || x_maxx()<640)
   iconkey[0]='H';
  else
   iconkey[0]='V';

  if(toolbarpage)
   iconkey[1]=toolbarpage;
  else if(tcpip)
   iconkey[1]='I';
  else
   iconkey[1]='D';

  if(geticoninfo(iconkey,dummy,dummy,dummy,desc1,desc2))
  {
   showhighlight();
   if(desc2[0])
   {
    sprintf(dummy,"%s (hotkey: %s)",desc1,desc2);
    outs(dummy);
   }
   else
    outs(desc1);
   return;
  }
  b=0;
 }

 if((arachne.GUIstyle==STYLE_SMALL1 || arachne.GUIstyle==STYLE_SMALL2) &&
    (b>0 && b<10 ||
     b==CLICK_NETHOME || b==CLICK_IMAGES || b==CLICK_MAIL || b==CLICK_SAVE || b==CLICK_DESKTOP))
  showhighlight();

 switch(b)
 {
  case 1:
  outs(MSG_ICON1);
  break;
  case 2:
  outs(MSG_ICON2);
  break;
  case 3:
  outs(MSG_ICON3);
  break;
  case 4:
  outs(MSG_ICON4);
  break;
  case 5:
  outs(MSG_ICON5);
  break;
  case 6:
  outs(MSG_ICON6);
  break;
  case 7:
  outs(MSG_ICON7);
  break;
  case 8:
  outs(MSG_ICON8);
  break;
  case 9:
  outs(MSG_ICON9);
  break;
  case CLICK_MAIL:
  outs(MSG_MAIL);
  break;
  case CLICK_SAVE:
  outs(MSG_SAVE);
  break;
  case CLICK_NETHOME:
  outs(homepage);
  break;
  case CLICK_HISTORY:
  outs(MSG_ICONH);
  break;
  case CLICK_ZOOM:
  outs(MSG_ZOOM);
  break;

//!!glennmcc: Sep 30, 2005
//moveing 'Up one Level' function to 'URL' instead of 'Arachne ver#'
//still will go up only when remote.. nothing will happen when local
//!!glennmcc: Feb 03, 2005 -- up one level if remote 'about:' if local
  case CLICK_UPLEVEL:
if(!strstr(GLOBAL.location,"file:"))
   {
    outs("Up one level");
    break;
   }
else
//  outs("about:");
  break;
//!!glennmcc: end

  case CLICK_ABOUT:
  outs("about:");
  break;

  case CLICK_EXIT:
  outs(MSG_EXIT);
  break;
  case CLICK_IMAGES:
  outs(MSG_IMAGES);
  break;
  case CLICK_DESKTOP:
  outs(MSG_HOME);
  break;
  case CLICK_MEMINFO:
  outs(MSG_INFO2);
  break;
  case CLICK_TCPIP:
  outs(MSG_DIAL2);
  break;
/*
  case 10:
  outs(MSG_SAVE);
  break;
  case 20:
  outs(MSG_OPEN);
  break;
  case 30:
  outs(MSG_PRINT);
  break;
  case 40:
  outs(MSG_SEARCH);
  break;
  case 50:
  outs(MSG_EDIT);
  break;
  case 60:
  outs(MSG_SOURCE);
  break;
  case 70:
  outs(MSG_INFO);
  break;
  case 80:
  outs(MSG_IMAGES);
  break;
  case 90:
  outs(MSG_MAIL);
  break;
  case 100:
  outs(MSG_HOME);
  break;
*/
  //textarea toolbar:
  case 10:
  showhighlight();
  outs(MSG_WRITEF);
  break;
  case 20:
  showhighlight();
  outs(MSG_READF);
  break;
  case 30:
  showhighlight();
  outs(MSG_BLCUT);
  break;
  case 40:
  showhighlight();
  outs(MSG_BLCLIP);
  break;
  case 50:
  showhighlight();
  outs(MSG_BLPAST);
  break;
  case 60:
  showhighlight();
  outs(MSG_BLKCOP);
  break;
  case 70:
  showhighlight();
  outs(MSG_BLKMOV);
  break;
  case 80:
  showhighlight();
  outs(MSG_BLKDEL);
  break;
  case 90:
  showhighlight();
  outs(MSG_SRCH4);
  break;

  /* not yet implemented in 1.60, sorry ;-)
  case 100:
  showhighlight();
  outs(MSG_PRT);
  break;
  */

  case ONMOUSE_TITLE:
  outs(arachne.title);
  break;

  default:

  if(lastonbutton)
   defaultmsg();
  // ?b=0;
 }

}
#endif

static char *grabstr (char *src, char *dest, int maxlen) {
   char c;

   while (isspace(*src)) src++;

   while (((c = *src) != '\0') && (!isspace(c)) && (maxlen--))
   {
      *dest++ = c;
      src++;
   }
   *dest = '\0';

   while (((c = *src) != '\0') && (!isspace(c))) src++;
   return src;
}


//custozmizable Arachne toolbar
//icon[9],methodarg[80],desc1[40],desc2[10] !

int geticoninfo(char *name,char *icon,char *method,char *methodarg,char *desc1,char *desc2)
{
 char *ptr = config_get_toolbar(name);
 if(ptr)
 {
  char methodstr[5];
  desc2[0]='\0';
  ptr = grabstr (ptr, icon, 8);
  ptr = grabstr (ptr, methodstr, 4);
  ptr = grabstr (ptr, methodarg, 79);
  ptr = grabstr (ptr, desc1, 39);
  (void) grabstr (ptr, desc2, 9);

  ptr=desc1;
  while(*ptr)
  {
   if(*ptr=='_')*ptr=' ';
   ptr++;
  }
  *method=*methodstr;
  return 1;
 }
 else
  return 0;
}

void zoom(void)
{
 //default for most modes:
 p->htscrn_xsize=x_maxx()-user_interface.scrollbarsize;
 if(fullscreen)
 {
  p->htscrn_ysize=x_maxy();
  p->htscrn_ytop=0;
 }
 else if(arachne.GUIstyle==STYLE_SMALL1)
 {
  p->htscrn_ysize=x_maxy()-42;
  p->htscrn_ytop=25;
 }
 else if(arachne.GUIstyle==STYLE_SMALL2)
 {
  p->htscrn_ysize=x_maxy()-67;
  p->htscrn_ytop=50;
 }
 else if( x_maxx()<640)
 {
  p->htscrn_ysize=x_maxy()-117;
  p->htscrn_ytop=100;
 }
 else if(arachne.GUIstyle==STYLE_MOZILLA)
 {
  p->htscrn_ysize=x_maxy()-117;
  p->htscrn_ytop=100;
 }
 else
 {
  p->htscrn_xsize=x_maxx()-152-user_interface.scrollbarsize;
  p->htscrn_ysize=x_maxy()-67;
  p->htscrn_ytop=50;
 }

 //common for all:
 p->htscrn_xtop=0;
 p->htmlframe[0].scroll.xvisible=0;
 p->htmlframe[0].scroll.yvisible=1;
 ScrollInit(&(p->htmlframe[0].scroll),
	     p->htscrn_xsize,
	     p->htscrn_ysize,
	     p->htscrn_ysize,
	     p->htscrn_xtop,
	     p->htscrn_ytop,
	     p->htmlframe[0].scroll.total_x,
	     p->htmlframe[0].scroll.total_y);
 TXTprompt.xx=p->htscrn_xsize-64;
 TXTprompt.y=p->htscrn_ysize/2;
 TXTprompt.yy=p->htscrn_ysize/2+fonty(SYSFONT,0)+4;


 if(arachne.GUIstyle==STYLE_SMALL1)
 {
  URLprompt.xx=x_maxx()-192-user_interface.scrollbarsize;
  URLprompt.x=x_maxx()/2-(x_maxx()-300)/6;
 }
 else
 {
  URLprompt.xx=p->htscrn_xsize;
  URLprompt.x=50;
 }
}

void gohome(void)
{
  strcpy(GLOBAL.location, config_get_str("HomePage", homepage));
#ifndef POSIX
  if (!tcpip)
    sprintf(GLOBAL.location,"file:%shome.htm",guipath2);
#endif
}

void MakeTitle(char *title)
{
 int l=strlen(title);
 char tmp[MAXTITLELEN+1],*titptr;

 if(l>MAXTITLELEN-5)
 {
  l=MAXTITLELEN-5;
  strncpy(tmp,title,l);
  tmp[l]='\0';
  strcat(tmp,"...");
  titptr=tmp;
 }
 else
  titptr=title;

 if(strcmp(arachne.title,titptr))
 {
  strcpy(arachne.title,titptr);
  title_ok=0;
 }
}


//===========================================================================
//Arachne "status bar applet docking" system
//===========================================================================

/*
void status_dock(char *id, char *iconame, char *msg, char *URL)
{

}


void status_undock(char *id)
{


}
*/

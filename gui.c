
//========================================================================
// GUITICK => single step of user interface, called from all loops
// prekresleni mysi, vstup z klavesy, etc.
//========================================================================

#include "arachne.h"
#include "gui.h"
#include "html.h"
#include "internet.h" //because of background tasks...

int bioskey(int cmd);

#define MOUSESTEP 8   //caverge adjustment

int GUITICK(void)
{
 int key,i;

 if(GLOBAL.backgroundimages>BACKGROUND_SLEEPING)
  return 0;

 //ulozim stare souradnice mysi....
 lx=mousex;
 ly=mousey;
 lmouse=mys;

 //klavesnice
 if(bioskey(1))
 {
  SecondsSleeping=0l; //pro screensaver
  key=bioskey(0);
  key=activeatomtick(key,0);
  {
   int ret=GUIEVENT(key,0);
   if(ret>=GUI_MOUSE)
   {
    mys=ret-GUI_MOUSE;
    lmouse=0;
    goto mammys;
   }
   else
    return ret;
  }
 }
 else if(atomneedredraw==1 && SecondsSleeping>1l)
 {
  atomneedredraw=0;
  activeatomredraw();
  activeatomcursor(1);
 }
 //end if kbhit


 //reading mouse...
 mys=ImouseRead( &mousex, &mousey );
 if(mys && lmouse==-1) //special init state
 {
  mys=-1;
  return 0;
 }
 //wheel movement detection
 if(mys & 0xf00) //mouse wheel is in higher byte of button
  mys=analysewheel(mys);

mammys:

 x_cursor (mousex,mousey);
 if(lx!=mousex || ly!=mousey)
 {
  SecondsSleeping=0l; //pro screensaver
  justmoved=1;
  kbmouse=0;
 }
 else
 if(lx==mousex && ly==mousey && justmoved)
 {
  if(mousex>=p->htscrn_xtop &&
     mousex<=p->htscrn_xtop+p->htscrn_xsize &&
     mousey>=p->htscrn_ytop &&
     mousey<=p->htscrn_ytop+p->htscrn_ysize)
  {
   if(lastonbutton)
   {
    lastonmouse=IE_NULL-1; //we moved from buttons to documents -> we don't know anything
#ifdef OVRL
#ifndef XTVERSION
    hidehighlight();
#endif
#endif
    lastonbutton=0;
   }
   onmouse(0);
  }
#ifndef XTVERSION
  else
  {
   int b=onbutton(mousex,mousey);
   if(lastonbutton!=b)
    onlinehelp(b);
   lastonbutton=b;
  }
#endif //XTVERSION
  justmoved=0;
 }

 if(mys)       //lmouse==-1:special state after Arachne start
  return GUIEVENT(0,mys);
 else
 if(lmouse>0)
 {
  lmouse=0;
  return GUIEVENT(0,MOUSE_RELEASE);
 }
 else //mouse was not clicked at all:
 {
  //optional handlers for AltTab and Print Screen:
#ifndef POSIX
  if(g_AltTab)
  {
   g_AltTab = 0;
   GUIEVENT(REDRAW_KEY,0);
   InstalAltTab();
  }
  else if(g_PrtScr)
  {
   g_PrtScr = 0;
//!!glennmcc: begin Dec 12, 2002
//goto prtbmp.ah instead of viewing the screencap (same as Ctrl+P)
    if(PrintScreen2BMP(0))
    {
     strcpy(GLOBAL.location,"gui:prtbmp.ah");
     arachne.target=0;
     return gotoloc();
    }
    else
//!!glennmcc: end
   return PrintScreen2BMP(0);
  }
#endif

  if(redraw)
  {
   if(redraw>2)
    MemInfo(NORMAL);
   if(arachne.framescount && redraw>2 || redraw>3)
    redrawHTML(REDRAW_WITH_MESSAGE,REDRAW_CREATE_VIRTUAL);
   else
    redrawHTML(REDRAW_WITH_MESSAGE,REDRAW_VIRTUAL);
   redraw=0;
  }

  if(scrolledframe!=-1)
  {
   scrolledframe=-1;
   mouseon();
  }

  if(atomneedredraw==2)
  {
   atomneedredraw=0;
   activeatomtick(CURSOR_SYNCHRO,TEXTAREA_SCROLL);
   activescroll.onscrollx=0;
   activescroll.onscrolly=0;
  }

  i=0;
  do
  {
   p->htmlframe[i].scroll.onscrollx=p->htmlframe[i].scroll.onscrolly=0;
  }
  while(i++<arachne.framescount); //[0...3]
 }//endif not mys

 return 0;
}

#define PERCENTBAR_X x_maxx()-150

//bar showing how much percents of document were already processed
void percentbar(int prc)
{
 if(fullscreen)
  return;

 if(prc>99) prc=100;

 if(mousex>PERCENTBAR_X-130 && mousey>x_maxy()-32)
  mouseoff();

 if(!prc)
  return; //do not draw empty percent bar

 Cell3D(PERCENTBAR_X-108,x_maxy()-13,PERCENTBAR_X-4,x_maxy()-2,-1); //transparent

 x_setfill(0,3); //zelena
 x_bar(PERCENTBAR_X-106,x_maxy()-11,PERCENTBAR_X-106+prc,x_maxy()-4);

 x_settextjusty(2,2);        // budu zarovnavat zprava
 x_setcolor(0);
 htmlfont(1,0);

 {
   char msg[10];
   sprintf(msg,"%d%%",prc);

   x_setfill(0,7); //sediva
   x_bar(PERCENTBAR_X-140,x_maxy()-13,PERCENTBAR_X-110,x_maxy()-2);
   x_text_ib(PERCENTBAR_X-110,x_maxy()-15,(unsigned char *)msg);
 }

 if(mousex>PERCENTBAR_X-130 && mousey>x_maxy()-32)
  mouseon();
 x_settextjusty(0,2);        // vzdycky psat pismo od leveho horniho rohu
}


void defaultmsg(void)
{
 if(htmlmsg[0])
  outs(htmlmsg);
 else
  outs(copyright);
}

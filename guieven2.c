
// ========================================================================
// GRAPHICAL USER INTERFACE for Arachne WWW browser - runtime functions
// (c)1997 xChaos software, portions (c)1997 Caldera Inc.
// ========================================================================

#include "arachne.h"
#include "gui.h"
#include "customer.h"

#define MOUSESTEP 10

#ifndef XTVERSION
int thisx,thisy,thisxx,thisyy;

#define MAXBUTT 16004l

void pressthatbutton(int nowait)
{
 char *hideiknbuf;
 char *pressiknbuf;
 long sz=(long)((long)(thisxx-thisx+1)*(long)(thisyy-thisy+1))+4l;

 if(thisxx && sz<MAXBUTT)
 {
  if(!nowait)
  {
#ifdef HICOLOR
  if(xg_256 == MM_Hic)
   hideiknbuf=farmalloc(2*sz);
  else
#endif
   hideiknbuf=farmalloc(sz);
  if(!hideiknbuf)
   return;
  }

#ifdef HICOLOR
  if(xg_256 == MM_Hic)
   pressiknbuf=farmalloc(2*sz);
  else
#endif
   pressiknbuf=farmalloc(sz);
  if(!pressiknbuf)
  {
   if(!nowait)
    farfree(hideiknbuf);
   return;
  }

  mouseoff();
  if(!nowait)
   x_getimg(thisx,thisy,thisxx,thisyy,hideiknbuf);
  x_getimg(thisx,thisy,thisxx-3,thisyy-3,pressiknbuf);
  x_putimg(thisx+1,thisy+1,pressiknbuf,0);
  if(thisx && thisy)
  {
   x_getimg(thisx-1,thisy,thisx-1,thisyy-1,pressiknbuf);
   x_putimg(thisx,thisy,pressiknbuf,0);
   x_getimg(thisx,thisy-1,thisxx-1,thisy-1,pressiknbuf);
   x_putimg(thisx,thisy,pressiknbuf,0);
  }
  else
  {
   x_setcolor(0);
   x_line(thisx,thisyy,thisx,thisy);
   x_line(thisx,thisy,thisxx,thisy);
  }
  if(!nowait)
  {
   mouseon();
   ImouseWait();
   mouseoff();
   x_putimg(thisx,thisy,hideiknbuf,0);
  }
  mouseon();
  farfree(pressiknbuf);
  if(!nowait)
   farfree(hideiknbuf);
 }
}
#endif

int onbig(int x, int y, int x0, int y0 )
{
#ifdef OVRL
#ifndef XTVERSION
 thisx=x0;thisy=y0;thisxx=x0+49;thisyy=y0+49;
#endif
#endif

 if(x>=x0 && x<=x0+50 && y>=y0 && y<=y0+50)
  return 1;
 else
  return 0;
}

int onbutton(int x,int y)
{

 if(fullscreen)
  return 0;

#ifdef CUSTOMER_MODULE
 if(customerscreen)
  return ( customer_onbutton( x, y) );
#endif

#ifndef CUSTOMER

 //Click on logo
 if(arachne.GUIstyle!=STYLE_FULLSCREEN && x>x_maxx()-150 && y<100)
 {
#ifdef OVRL
#ifndef XTVERSION
  thisx=x_maxx()-150;
  thisy=0;
  thisxx=x_maxx();
  thisyy=100;
#endif
#endif
  return CLICK_NETHOME;
 }

 //Click on history button
 if(x>URLprompt.xx && x<x_maxx()-152 && y>URLprompt.y+htscrn_ytop && y<htscrn_ytop)
 {
#ifdef OVRL
#ifndef XTVERSION
  thisxx=0;
#endif
#endif
  return CLICK_HISTORY;
 }

 if(y<htscrn_ytop-25 && y>htscrn_ytop-50)
 {
 if(x>x_maxx()-300 && x<x_maxx()-190)
 {
#ifdef OVRL
#ifndef XTVERSION
  thisxx=0;
#endif
#endif
  return CLICK_ABOUT;
 }
 else
 if(x>x_maxx()-190 && x<x_maxx()-170)
 {
#ifdef OVRL
#ifndef XTVERSION
  thisxx=0;
#endif
#endif
  return CLICK_ZOOM;
 }
 else
 if(x>x_maxx()-170 && x<x_maxx()-152)
 {
#ifdef OVRL
#ifndef XTVERSION
  thisxx=0;
#endif
#endif
  return CLICK_EXIT;
 }
 }

#ifndef AGB
 if(x>x_maxx()-150 && y>x_maxy()-13)
 {
#ifndef XTVERSION
  thisxx=0;
#endif
  if(x>x_maxx()-60)
  {
   return CLICK_TCPIP;
  }
  else
  {
   return CLICK_MEMINFO;
  }
 }
#endif

 else if(arachne.GUIstyle==STYLE_FULLSCREEN)
 {
  if(y>50 || x<x_maxx()-150)
  return 0;
  if(x>x_maxx()-50)
  {
#ifdef OVRL
#ifndef XTVERSION
   thisx=x_maxx()-50;
   thisxx=x_maxx();
#endif
#endif
   if(y<25)
   {
#ifdef OVRL
#ifndef XTVERSION
    thisy=0;
    thisyy=25;
#endif
#endif
    return CLICK_NETHOME;
   }
   else
   {
#ifdef OVRL
#ifndef XTVERSION
    thisy=25;
    thisyy=50;
#endif
#endif
    return CLICK_DESKTOP; //Desktop
   }
  }
#ifdef OVRL
#ifndef XTVERSION
 thisxx=0;
#endif
#endif
  x=x_maxx()-x;
  if(y<25)
  {
   if(x>130)
    return 1;
   else if(x>110)
    return 2;
   else if(x>90)
    return 3;
   else if(x>70)
    return 4;
   else
    return 7;
  }
  else
  {
   if(x>130)
    return 5;
   else if(x>110)
    return 6;
   else if(x>90)
    return 8;  //search
   else if(x>70)
    return 9;  //help
   else
    return CLICK_IMAGES; //images
  }
 }
 else
 if(arachne.GUIstyle || x_maxx()<640)
 {
  if(y>50)
   return 0;
  else
  if(onbig(x,y,0,0))
   return 1;
  else
  if(onbig(x,y,50,0))
   return 2;
  else
  if(onbig(x,y,100,0))
   return 3;
  else
  if(onbig(x,y,150,0))
   return 4;
  else
  if(onbig(x,y,200,0))
   return 5;
  else
  if(onbig(x,y,250,0))
   return 6;
  else
  if(onbig(x,y,300,0))
   return 7;
  else
  if(onbig(x,y,350,0))
   return 8;
  else
  if(onbig(x,y,400,0))
   return 9;

  if(user_interface.iconsoff)
   return 0;
#ifdef CALDERA
  if(iconsoff)
   return 0;
#endif // CALDERA

  //customizable toolbar

  if(x>450 && x<x_maxx()-156 && y<50)
  {
   int rv=2*((x-453)/32)+y/25;
#ifdef OVRL
#ifndef XTVERSION
   thisx=453+32*(rv/2);thisy=3+23*(rv%2);thisxx=thisx+34;thisyy=thisy+22;
#endif
#endif
   return 10*(rv+1);
  }
 }
 else //toolbar on right side
 {
  if(x<x_maxx()-150)
   return 0;
  else
  if(onbig(x,y,x_maxx()-150,100))
   return 1;
  else
  if(onbig(x,y,x_maxx()-100,100))
   return 2;
  else
  if(onbig(x,y,x_maxx()-50,100))
   return 3;
  else
  if(onbig(x,y,x_maxx()-150,150))
   return 4;
  else
  if(onbig(x,y,x_maxx()-100,150))
   return 5;
  else
  if(onbig(x,y,x_maxx()-50,150))
   return 6;
  else
  if(onbig(x,y,x_maxx()-150,200))
   return 7;
  else
  if(onbig(x,y,x_maxx()-100,200))
   return 8;
  else
  if(onbig(x,y,x_maxx()-50,200))
   return 9;

  if(x>x_maxx()-148 && y>252 && y<endvtoolbar()-4)
  {
   int rv=(y-253)/18;
#ifdef OVRL
#ifndef XTVERSION
   thisx=x_maxx()-146;thisy=253+rv*18;thisxx=x_maxx()-1;thisyy=thisy+21;
#endif
#endif
   return 10*(rv+1);
  }

#ifdef OVRL
#ifndef XTVERSION
  thisxx=0;
#endif
#endif
#ifndef AGB
  if(toolbarmode==0)
   return CLICK_MEMINFO;
#endif
 }
#endif //!CUSTOMER
 return 0;
}

//  The keyboard "scroll/lock key" 
char scrolllock(void)
{
 if(bioskey(2)&SCROLL)
  return 1;
 else
  return 0;
}

char shift(void)
{
 if(bioskey(2)&LEFTSHIFT||bioskey(2)&RIGHTSHIFT)
  return 1;
 else
  return 0;
}

#ifdef VIRT_SCR
void smothscroll(long from,long to)
{
 int step=user_interface.step;
 struct HTMLframe *frame=&htmlframe[activeframe];
 struct TMPframedata *htmldata=&tmpframedata[activeframe];

 if(!htmldata->usevirtualscreen)
  return;

 mouseoff();
 if(from>to)
  step=-step;

 while( step>0 && from+2*step<to || step<0 && from+2*step>to)
 {
  from+=step;
  if(frame->posX>=virtualxstart[htmldata->whichvirtual] &&
     frame->posX+frame->scroll.xsize<virtualxend[htmldata->whichvirtual] &&
     from>=virtualystart[htmldata->whichvirtual] &&
     from+frame->scroll.ysize<virtualyend[htmldata->whichvirtual])
  {
   dumpvirtual(frame,htmldata,frame->posX,from);
   if(frame->allowscrolling)
    ScrollDraw(&frame->scroll,frame->posX,from);
  }
  else
   break;
 }
}
#endif

#ifdef VIRT_SCR
void novirtual(void)
{
 int i=0;
 do
 {
  virtualyend[tmpframedata[i].whichvirtual]=0l;
 }
 while(++i<MAXFRAMES);
}
#endif



#ifdef VIRT_SCR
void Try2DumpActiveVirtual(void)
{
 if(allocatedvirtual[tmpframedata[activeframe].whichvirtual] &&
    user_interface.smooth &&
    htmlframe[activeframe].posX>=virtualxstart[tmpframedata[activeframe].whichvirtual] &&
    htmlframe[activeframe].posX+htmlframe[activeframe].scroll.xsize<virtualxend[tmpframedata[activeframe].whichvirtual] &&
    htmlframe[activeframe].posY>=virtualystart[tmpframedata[activeframe].whichvirtual] &&
    htmlframe[activeframe].posY+htmlframe[activeframe].scroll.ysize<virtualyend[tmpframedata[activeframe].whichvirtual])
 {
  dumpvirtual(&htmlframe[activeframe],&tmpframedata[activeframe],
              htmlframe[activeframe].posX,
              htmlframe[activeframe].posY);
 }
}
#endif


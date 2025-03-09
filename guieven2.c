
// ========================================================================
// GRAPHICAL USER INTERFACE for Arachne WWW browser - runtime functions
// (c)1997-2000 Arachne Labs
// ========================================================================

#include "arachne.h"
#include "gui.h"

int bioskey(int cmd);

#define MOUSESTEP 8

#ifndef XTVERSION
int thisx,thisy,thisxx,thisyy;

void pressthatbutton(int nowait)
{
 char *hideiknbuf;
 char *pressiknbuf;
 long sz=(long)((long)(thisxx-thisx+1)*(long)(thisyy-thisy+1))+4*sizeof(int);

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

 //Click on logo
 if(arachne.GUIstyle!=STYLE_SMALL1 &&
    arachne.GUIstyle!=STYLE_SMALL2 &&
    x>x_maxx()-150 && y<100)
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
 if(x>URLprompt.xx && x<URLprompt.xx+user_interface.scrollbarsize && y>URLprompt.y+p->htscrn_ytop && y<p->htscrn_ytop)
 {
#ifdef OVRL
#ifndef XTVERSION
  thisxx=0;
#endif
#endif
  return CLICK_HISTORY;
 }

//!!glennmcc: Sep 30, 2005
//moveing 'Up one Level' function to 'URL' instead of 'Arachne ver#'
 if(x>URLprompt.x-40 && x<URLprompt.x-5 && y>URLprompt.y+p->htscrn_ytop && y<p->htscrn_ytop)
 {
#ifdef OVRL
#ifndef XTVERSION
  thisxx=0;
#endif
#endif
  return CLICK_UPLEVEL;
 }
//!!glennmcc: end

 if(y<p->htscrn_ytop-25 && y>p->htscrn_ytop-50)
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

 //------------------------------------------------------------------------
 else if(arachne.GUIstyle==STYLE_SMALL1)
 {
  if(y>p->htscrn_ytop)
   return 0;

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

  if(x>150 && x<(x_maxx()/2-(x_maxx()-300)/6))
   return ONMOUSE_TITLE;

  if(x<147)
  {
   int i=(x-2)/21;
   thisx=3+21*i;
   thisxx=thisx+21;
   thisy=3;
   thisyy=22;
   if(i==4)
    return CLICK_ABORT; //abort
   else
   if(i==5)
    return CLICK_ADDHOTLIST; //add to hotlist
   else
   if(i==6)
    return CLICK_HOTLIST; //hotlist
   else
    return i+1;
  }
  else if(x>x_maxx()-148)
  {
   int i=(x-x_maxx()+148)/21;
   thisx=x_maxx()-147+21*i;
   thisxx=thisx+21;
   thisy=3;
   thisyy=22;
   if(i==6)
    return CLICK_NETHOME;
   else
   if(i==2)
    return CLICK_IMAGES;
   else
   if(i==3)
    return CLICK_SAVE;
   else
   if(i==4)
    return CLICK_MAIL;
   else
   if(i==5)
    return CLICK_DESKTOP;
   else
    return i+8;
  }
 }
 //------------------------------------------------------------------------
 else if(arachne.GUIstyle==STYLE_SMALL2)
 {
  if(y>p->htscrn_ytop)
   return 0;

  if(x>x_maxx()-148)
  {
   if(y<p->htscrn_ytop-25)
   {
    int i=(x-x_maxx()+148)/21;
    thisx=x_maxx()-147+21*i;
    thisxx=thisx+21;
    thisy=4;
    thisyy=25;
    if(i==6)
     return CLICK_NETHOME;
    else
    if(i==2)
     return CLICK_IMAGES;
    else
    if(i==3)
     return CLICK_SAVE;
    else
    if(i==4)
     return CLICK_MAIL;
    else
    if(i==5)
     return CLICK_DESKTOP;
    else
     return i+8;
   }
   else
   {
    int i=(x-x_maxx()+148)/21;
    thisx=x_maxx()-147+21*i;
    thisxx=thisx+21;
    thisy=26;
    thisyy=47;
    if(i==4)
     return CLICK_ABORT; //abort
    else
    if(i==5)
     return CLICK_ADDHOTLIST; //add to hotlist
    else
    if(i==6)
     return CLICK_HOTLIST; //hotlist
    else
     return i+1;
   }
  }
 }
 //------------------------------------------------------------------------
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
  if(toolbarmode==0)
   return CLICK_MEMINFO;
 }
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
 struct HTMLframe *frame=&(p->htmlframe[p->activeframe]);
 struct TMPframedata *htmldata=&(p->tmpframedata[p->activeframe]);

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
  virtualyend[p->tmpframedata[i].whichvirtual]=0l;
 }
 while(++i<MAXFRAMES);
}
#endif



#ifdef VIRT_SCR
void Try2DumpActiveVirtual(void)
{
 struct HTMLframe *frame=&(p->htmlframe[p->activeframe]);
 if(allocatedvirtual[p->tmpframedata[p->activeframe].whichvirtual] &&
    user_interface.smooth &&
    frame->posX>=virtualxstart[p->tmpframedata[p->activeframe].whichvirtual] &&
    frame->posX+frame->scroll.xsize<virtualxend[p->tmpframedata[p->activeframe].whichvirtual] &&
    frame->posY>=virtualystart[p->tmpframedata[p->activeframe].whichvirtual] &&
    frame->posY+frame->scroll.ysize<virtualyend[p->tmpframedata[p->activeframe].whichvirtual])
 {
  dumpvirtual(frame,&(p->tmpframedata[p->activeframe]),frame->posX,frame->posY);
 }
}
#endif



// ========================================================================
// xChaos ScrollBar library - xChaos/Windows/NextStep style scrollbars !
// (c)1997 xChaos software
// ========================================================================

#include "scrolbar.h"
#include "uiface.h"
#include "svga.h"
#include "apick.h"
#include "gui.h"

// Note: this library doesn't care about turning mouse on/off

void ScrollInit(struct ScrollBar *scroll,
                int draw_x,
                int draw_y,
                int max_y,
                int xtop, int ytop, int total_x, long total_y)
{
#ifdef CALDERA
 if(user_interface.scrollbarstyle==NO_SCROLL_BARS)
  return;
#endif // CALDERA
 scroll->xsize=draw_x;
 scroll->ysize=draw_y;
 scroll->ymax=max_y;
 scroll->xtop=xtop;
 scroll->ytop=ytop;
 scroll->total_x=total_x;
 scroll->total_y=total_y;
 scroll->gap=0;
 scroll->onscrollx=0;
 scroll->onscrolly=0;

 if(scroll->ymax>48 && (scroll->xsize>48 ||! scroll->xvisible)) /*40+4*/
  scroll->scrollbarstyle=user_interface.scrollbarstyle;
 else
  scroll->scrollbarstyle='\0';

 if(scroll->scrollbarstyle)
 {
  scroll->max_xscrsz=draw_x-40;
  scroll->max_yscrsz=max_y-40;

  if(scroll->scrollbarstyle=='W')
  {
   scroll->x_decrease_gap=scroll->xtop;
   scroll->x_increase_gap=scroll->xtop+scroll->max_xscrsz+22;
   scroll->y_decrease_gap=scroll->ytop;
   scroll->y_increase_gap=scroll->ytop+scroll->max_yscrsz+23;
   scroll->gap=20;
  }
  else
  if(scroll->scrollbarstyle=='N')
  {
   scroll->x_decrease_gap=scroll->xtop;
   scroll->x_increase_gap=scroll->xtop+20;
   scroll->y_decrease_gap=scroll->ytop;
   scroll->y_increase_gap=scroll->ytop+20;
   scroll->gap=39;
  }
  else //experimental
  {
   scroll->x_decrease_gap=scroll->xtop+scroll->max_xscrsz+2;
   scroll->x_increase_gap=scroll->xtop+scroll->max_xscrsz+22;
   scroll->y_decrease_gap=scroll->ytop+scroll->max_yscrsz+3;
   scroll->y_increase_gap=scroll->ytop+scroll->max_yscrsz+23;
  }
 }
 else
 {
  scroll->max_xscrsz=draw_x;
  scroll->max_yscrsz=max_y;
 }//endif
}


void ScrollButtons(struct ScrollBar *scroll)
{
/*
#ifdef CALDERA
 if(user_interface.scrollbarstyle==NO_SCROLL_BARS)
  return;
#endif // CALDERA*/
 if(!scroll->scrollbarstyle || !scroll->yvisible)
  return;
 else
 {
/*
#ifdef HICOLOR
  if(xg_256 == MM_Hic) initpalette();
#endif
*/
  //arrow up
  Box3Dv(scroll->xtop+scroll->xsize+1,scroll->y_decrease_gap,
        scroll->xtop+scroll->xsize+user_interface.scrollbarsize,scroll->y_decrease_gap+18);
  x_setcolor(8);
  x_line(scroll->xtop+scroll->xsize+user_interface.scrollbarsize/2,4+scroll->y_decrease_gap,
         scroll->xtop+scroll->xsize+3,14+scroll->y_decrease_gap);
  x_setcolor(15);
  x_line(scroll->xtop+scroll->xsize+3,14+scroll->y_decrease_gap,
         scroll->xtop+scroll->xsize+user_interface.scrollbarsize-3,14+scroll->y_decrease_gap);

  //arrow down
  Box3Dv(scroll->xtop+scroll->xsize+1,scroll->y_increase_gap,
        scroll->xtop+scroll->xsize+user_interface.scrollbarsize,scroll->y_increase_gap+17);
  x_setcolor(15);
  x_line(scroll->xtop+scroll->xsize+user_interface.scrollbarsize/2,scroll->y_increase_gap+13,
         scroll->xtop+scroll->xsize+user_interface.scrollbarsize-3,scroll->y_increase_gap+4);
  x_setcolor(8);
  x_line(scroll->xtop+scroll->xsize+3,scroll->y_increase_gap+4,
         scroll->xtop+scroll->xsize+user_interface.scrollbarsize-3,scroll->y_increase_gap+4);

  if(!scroll->xvisible)
   return;

  //arrow left
  Box3Dh(scroll->x_decrease_gap,scroll->ytop+scroll->ysize+1,
        scroll->x_decrease_gap+18,scroll->ytop+scroll->ysize+user_interface.scrollbarsize);
  x_setcolor(8);
  x_line(scroll->x_decrease_gap+4,scroll->ytop+scroll->ysize+user_interface.scrollbarsize/2,
         scroll->x_decrease_gap+14,scroll->ytop+scroll->ysize+3);
  x_setcolor(15);
  x_line(scroll->x_decrease_gap+14,scroll->ytop+scroll->ysize+3,
         scroll->x_decrease_gap+14,scroll->ytop+scroll->ysize+user_interface.scrollbarsize-3);

  if(scroll->scrollbarstyle!='W')
  {
   x_setcolor(0);
   x_line(scroll->x_increase_gap-1,scroll->ytop+scroll->ysize+1,scroll->x_increase_gap-1,scroll->ytop+scroll->ysize+user_interface.scrollbarsize);
  }

  //arrow right
  Box3Dh(scroll->x_increase_gap,scroll->ytop+scroll->ysize+1,
        scroll->x_increase_gap+18,scroll->ytop+scroll->ysize+user_interface.scrollbarsize);
  x_setcolor(15);
  x_line(scroll->x_increase_gap+14,scroll->ytop+scroll->ysize+user_interface.scrollbarsize/2,
         scroll->x_increase_gap+4,scroll->ytop+scroll->ysize+user_interface.scrollbarsize-3);
  x_setcolor(8);
  x_line(scroll->x_increase_gap+4,scroll->ytop+scroll->ysize+3 ,
         scroll->x_increase_gap+4,scroll->ytop+scroll->ysize+user_interface.scrollbarsize-3);
 }
}

//REDRAW SCROLL BARS
void ScrollDraw(struct ScrollBar *scroll,int fromx,long fromy)
{
 long pom;
 int zblo=0;

/*
#ifdef CALDERA
if(user_interface.scrollbarstyle==NO_SCROLL_BARS)
 return;
#endif // CALDERA
*/
 if(!scroll->onscrollx)
 {
  if(scroll->total_y<=scroll->ysize || scroll->total_y==0)
  {
   scroll->yscrsz=scroll->max_yscrsz;
   scroll->yscr=0;
  }
  else
  {
   pom=(long)scroll->ysize*(long)scroll->max_yscrsz;
   scroll->yscrsz=(int)(pom/scroll->total_y)+1;
   if(scroll->yscrsz<8)
   {
    scroll->yscrsz=8;
    if(scroll->yscrsz>scroll->max_yscrsz)scroll->yscrsz=scroll->max_yscrsz;
   }

   pom=fromy*(long)scroll->max_yscrsz;
   scroll->yscr=(int)(pom/scroll->total_y);
   if(scroll->yscr+scroll->yscrsz>scroll->max_yscrsz)scroll->yscr=scroll->max_yscrsz-scroll->yscrsz;
  }

  if(!scroll->yvisible)
   return;

  x_setfill(0,0);
  if(scroll->scrollbarstyle)
   zblo=1;

  scroll->yscr+=scroll->gap;

  if(scroll->yscr>0)
   x_bar(scroll->xtop+scroll->xsize+1,scroll->ytop+scroll->gap,
         scroll->xtop+scroll->xsize+user_interface.scrollbarsize,scroll->ytop+scroll->yscr-1);
  if(scroll->yscr+scroll->yscrsz<scroll->max_yscrsz+scroll->gap)
   x_bar(scroll->xtop+scroll->xsize+1,scroll->ytop+scroll->yscr+scroll->yscrsz+1,
         scroll->xtop+scroll->xsize+user_interface.scrollbarsize,scroll->ytop+scroll->max_yscrsz+scroll->gap+zblo);

  Box3Dv(scroll->xtop+scroll->xsize+1,scroll->ytop+scroll->yscr,
        scroll->xtop+scroll->xsize+user_interface.scrollbarsize,scroll->ytop+scroll->yscr+scroll->yscrsz+zblo);

  //ozdobicky
  if(scroll->yscrsz>24)
  {
   int ymid=scroll->yscrsz/2;
   int xleft=scroll->xtop+scroll->xsize+3;
   int xright=scroll->xtop+scroll->xsize+user_interface.scrollbarsize-3;
   int yyy=scroll->ytop+scroll->yscr+ymid;

   x_setcolor(15);
   x_line(xleft,yyy,
          xright,yyy);
   x_line(xleft,yyy-4,
          xright,yyy-4);
   x_line(xleft,yyy+4,
          xright,yyy+4);
   x_line(xleft,yyy-8,
	  xright,yyy-8);
   x_line(xleft,yyy+8,
	  xright,yyy+8);
   x_setcolor(8);
   x_line(xleft,yyy-1,
	  xright,yyy-1);
   x_line(xleft,yyy-5,
	  xright,yyy-5);
   x_line(xleft,yyy+3,
	       xright,yyy+3);
   x_line(xleft,yyy-9,
	  xright,yyy-9);
   x_line(xleft,yyy+7,
	  xright,yyy+7);
  }
 }

 if(scroll->onscrolly)
  return;

 if(!scroll->xvisible || scroll->total_x<=scroll->xsize || scroll->total_x==0)
 {
  scroll->xscrsz=scroll->max_xscrsz;
  scroll->xscr=0;
  if(!scroll->xvisible)
   return;
 }
 else
 {
  pom=(long)scroll->xsize*(long)scroll->max_xscrsz;
  scroll->xscrsz=(int)(pom/scroll->total_x)+1;
  if(scroll->xscrsz<8)
  {
   scroll->xscrsz=8;
   if(scroll->xscrsz>scroll->max_xscrsz)scroll->xscrsz=scroll->max_xscrsz;
  }

  pom=fromx*(long)scroll->max_xscrsz;
  scroll->xscr=(int)(pom/scroll->total_x);
  if(scroll->xscr+scroll->xscrsz>scroll->max_xscrsz)scroll->xscr=scroll->max_xscrsz-scroll->xscrsz;
 }

 x_setfill(0,0);
 if(scroll->scrollbarstyle!='N')
  zblo=0;

 scroll->xscr+=scroll->gap+zblo;
 if(scroll->xscr>0)
   x_bar(scroll->xtop+scroll->gap+zblo,scroll->ytop+scroll->ysize+1,
         scroll->xtop+scroll->xscr-1,scroll->ytop+scroll->ysize+user_interface.scrollbarsize);
 if(scroll->xscr+scroll->xscrsz<scroll->max_xscrsz+scroll->gap+zblo)
   x_bar(scroll->xtop+scroll->xscr+scroll->xscrsz+1,scroll->ytop+scroll->ysize+1,
         scroll->xtop+scroll->max_xscrsz+scroll->gap+zblo,scroll->ytop+scroll->ysize+user_interface.scrollbarsize);

 Box3Dh(scroll->xtop+scroll->xscr,scroll->ytop+scroll->ysize+1,
       scroll->xtop+scroll->xscr+scroll->xscrsz,scroll->ytop+scroll->ysize+user_interface.scrollbarsize);

 if(scroll->xscrsz>24)//ozdobicky...
 {
  int xmid=scroll->xscrsz/2;
  int yend=scroll->ytop+scroll->ysize+3;
  int ystart=scroll->ytop+scroll->ysize+user_interface.scrollbarsize-3;
  int xxx=scroll->xtop+scroll->xscr+xmid;

  x_setcolor(15);

  x_line(xxx,yend,
         xxx,ystart);
  x_line(xxx-4,yend,
         xxx-4,ystart);
  x_line(xxx+4,yend,
         xxx+4,ystart);
  x_line(xxx-8,yend,
         xxx-8,ystart);
  x_line(xxx+8,yend,
         xxx+8,ystart);
  x_setcolor(8);
  x_line(xxx-1,yend,
         xxx-1,ystart);
  x_line(xxx-5,yend,
         xxx-5,ystart);
  x_line(xxx+3,yend,
         xxx+3,ystart);
  x_line(xxx-9,yend,
         xxx-9,ystart);
  x_line(xxx+7,yend,
         xxx+7,ystart);
 }

}

int OnScrollButtons(struct ScrollBar *scroll)
{
 if(scroll->scrollbarstyle &&
    !scroll->onscrollx && !scroll->onscrolly)//scroll buttons
 {
  if(scroll->yvisible)
  {
   if (mousex>scroll->xtop+scroll->xsize &&
       mousey>scroll->y_decrease_gap &&
       mousex<scroll->xtop+scroll->xsize+user_interface.scrollbarsize &&
       mousey<scroll->y_decrease_gap+18)
    return 1;

   if (mousex>scroll->xtop+scroll->xsize &&
       mousey>scroll->y_increase_gap &&
       mousex<scroll->xtop+scroll->xsize+user_interface.scrollbarsize &&
       mousey<scroll->y_increase_gap+18)
    return 2;
  }

  if(scroll->xvisible)
  {
   if (mousex>scroll->x_decrease_gap &&
       mousey>scroll->ytop+scroll->ysize &&
       mousex<scroll->x_decrease_gap+18 &&
       mousey<scroll->ytop+scroll->ysize+user_interface.scrollbarsize)
   return 3;

   if (mousex>scroll->x_increase_gap &&
       mousey>scroll->ytop+scroll->ysize  &&
       mousex<scroll->x_increase_gap+18 &&
       mousey<scroll->ytop+scroll->ysize+user_interface.scrollbarsize)
   return 4;
  }//endif
 }
 return 0;
}

// Black zone of scroll bar
int OnBlackZone(struct ScrollBar *scroll)
{
 if(mousex>scroll->xtop+scroll->xsize &&
    mousey>scroll->ytop+scroll->gap &&
    mousex<scroll->xtop+scroll->xsize+user_interface.scrollbarsize &&
    mousey<scroll->ytop+scroll->yscr)
  return 1;

 if(mousex>scroll->xtop+scroll->xsize &&
    mousey>scroll->ytop+scroll->yscr+scroll->yscrsz &&
    mousex<scroll->xtop+scroll->xsize+user_interface.scrollbarsize &&
    mousey<scroll->ytop+scroll->gap+scroll->max_yscrsz)
  return 2;

 if(scroll->xvisible)
 {
  if(mousey>scroll->ytop+scroll->ysize &&
     mousex>scroll->xtop+scroll->gap &&
     mousey<scroll->ytop+scroll->ysize+user_interface.scrollbarsize &&
     mousex<scroll->xtop+scroll->xscr)
   return 3;

  if(mousey>scroll->ytop+scroll->ysize &&
     mousex>scroll->xtop+scroll->xscr+scroll->xscrsz &&
     mousey<scroll->ytop+scroll->ysize+user_interface.scrollbarsize &&
     mousex<scroll->xtop+scroll->gap+scroll->max_xscrsz)
   return 4;
 }
 return 0;
}


int ScrollBarTICK(struct ScrollBar *scroll,int *X, long *Y)
{
 //y scroll
 if((mousex>scroll->xtop+scroll->xsize &&
     mousey>scroll->ytop+scroll->yscr &&
     mousex<scroll->xtop+scroll->xsize+user_interface.scrollbarsize &&
     mousey<scroll->ytop+scroll->yscr+scroll->yscrsz
     || scroll->onscrolly && lmouse && scroll->yvisible) && !scroll->onscrollx)
 {
  int dy=(int)((long)scroll->total_y*(long)(mousey-ly)/scroll->max_yscrsz);
  *Y+=dy;
  if(*Y>scroll->total_y-scroll->ysize)
   *Y=scroll->total_y-scroll->ysize;
  if(*Y<0)
   *Y=0;
  scroll->onscrolly=1;
  if (dy)
   return 1;
 }
 else //x scroll
 if(mousex>scroll->xtop+scroll->xscr &&
    mousey>scroll->ytop+scroll->ysize &&
    mousex<scroll->xtop+scroll->xscr+scroll->xscrsz &&
    mousey<scroll->ytop+scroll->ysize+user_interface.scrollbarsize
    || scroll->onscrollx && lmouse && scroll->xvisible)
 {
  int dx=(int)((long)scroll->total_x*(long)(mousex-lx)/scroll->max_xscrsz);
  *X+=dx;
  if(*X>scroll->total_x-scroll->xsize)
   *X=scroll->total_x-scroll->xsize;
  if(*X<0)
   *X=0;
  scroll->onscrollx=1;
  if(dx)
   return 1;
 }

 return 0;
}


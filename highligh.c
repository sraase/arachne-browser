// ========================================================================
// Highlight HTML atom
// (c)1990-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "html.h"

void highlightatom(struct HTMLrecord *foundatom)
{
 struct HTMLframe *frame=&(p->htmlframe[p->activeframe]);
// struct TMPframedata *htmldata=&(p->tmpframedata[p->activeframe]);
 mouseoff();

 x_setfill(0,14);
 {
  int x1=frame->scroll.xtop+foundatom->x-frame->posX;
  int x2=frame->scroll.xtop+foundatom->xx-frame->posX;
  if(x1<frame->scroll.xtop)
   x1=frame->scroll.xtop;
  if(x2>frame->scroll.xtop+frame->scroll.xsize)
   x2=frame->scroll.xtop+frame->scroll.xsize;
  x_bar(x1,(int)(frame->scroll.ytop+foundatom->y-frame->posY),
        x2,(int)(frame->scroll.ytop+foundatom->yy-frame->posY));
 }

 //draw found string
 foundatom->R=0;
 foundatom->G=0;
 foundatom->B=0;

#ifndef POSIX
 bigfonts_allowed();
#endif
 drawatom(foundatom,frame->posX,frame->posY,
           frame->scroll.xsize,frame->scroll.ysize,
           frame->scroll.xtop,frame->scroll.ytop);
#ifndef POSIX 
 bigfonts_forbidden();
#endif
 mouseon();
}

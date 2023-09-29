
// ========================================================================
// Arachne mouse interface, "activeatom" related
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

// !!JdS 2004/12/01 : Replaced mouse-click literal constants with token
// symbols (which are now defined in 'gui.h'). Similarly for image-input
// atom subtype literal constants (token symbols now defined in 'html.h').

#include "arachne.h"
#include "html.h"
#include "gui.h"

char *onmouse(int click)
{
 int linkonmouse;
 struct HTMLrecord atomonmouse;
 char *ptr;
 unsigned currentHTMLatom=p->firstonscr,nextHTMLatom;
 int dx,x,y;
 long dy;
 char ontoolbar=0;
 struct HTMLrecord *atomptr;
 int count=0;

 if(mousey<p->htscrn_ytop)
 {
  x=mousex-p->htscrn_xtop;
  y=mousey-p->htscrn_ytop;
  atomptr=&URLprompt;
  if(click && click!=MOUSE_RELEASE && !lmouse && atomptr->x<=x && atomptr->xx>=x && atomptr->y<=y && atomptr->yy>=y)
  {
   activeurl("");
   if(click==MOUSE_MIDDLE) //paste
    activeatomtick(ASCIICTRLV,0);
   else
   if(click==MOUSE_RIGHT) //delete
     activeatomtick(ASCIICTRLY,0);
   else if(click!=MOUSE_RELEASE)
   {
    int newx;

    activeatomcursor(0);
    toolbar(0,1);
    //goto mouse pointer:
    editorptr=(struct ib_editor *)ie_getswap(URLprompt.ptr);
    if(!editorptr)
     MALLOCERR();
    newx=editorptr->zoomx+(x-URLprompt.x-4)/fontx(SYSFONT,0,' ');

    if(newx==editorptr->x) //double click = submit
     return try2getURL();

    editorptr->x=newx;
    swapmod=1;
   }
   activeatomtick(CURSOR_SYNCHRO,TEXTAREA_INIT);
   lastonmouse=IE_NULL;
   return NULL;
  }
 }

 if(activeistextwindow && click && click!=MOUSE_RELEASE)
 {
  x=mousex-p->htscrn_xtop;
  y=mousey-p->htscrn_ytop;
  atomptr=&TXTprompt;
  if(atomptr->x<=x && atomptr->xx>=x && atomptr->y<=y && atomptr->yy>=y)
  {
   if(click==MOUSE_MIDDLE)
   {
    activeatomtick(ASCIICTRLV,0);
   }
   lastonmouse=IE_NULL;
   return NULL;
  }
  else
  {
   activeatomptr=NULL;
   activeatomcursor(0);
  }
 }

 if(mousey<p->htscrn_ytop || mousey>p->htscrn_ytop+p->htscrn_ysize ||
     mousex>p->htscrn_xtop+p->htscrn_xsize || mousex<p->htscrn_xtop)
 {
  ontoolbar=1;
  goto nolink;
 }

 if(htmlpulldown)
 {
  x=mousex-p->htmlframe[activeatom.frameID].scroll.xtop;
  y=mousey-p->htmlframe[activeatom.frameID].scroll.ytop;
  dx=p->htmlframe[activeatom.frameID].posX;
  dy=p->htmlframe[activeatom.frameID].posY;
  if(activeatom.type==INPUT && activeatom.data1==SELECT &&
     activeatom.x-dx<=x && activeatom.xx-dx>=x &&
     activeatom.y-dy<=y && activeatom.yy-dy>=y)
  {
   if(click==MOUSE_RELEASE)
    goto nolink;

   if(click)
    SelectSwitch(x+dx,y+dy,0); //0==no key event passed to <SELECT> tag
   else
    SelectSwitch(x+dx,y+dy,CURSOR_SYNCHRO); //let's render nice highlight
   lastonmouse=IE_NULL;
   return NULL;
  }
  else
   hidehighlight();
 }

 if(click && click!=MOUSE_RELEASE && arachne.framescount>0)
 {
  int i=1;
  do
  {
   if(mousex>p->htmlframe[i].scroll.xtop &&
      mousex<p->htmlframe[i].scroll.xtop+p->htmlframe[i].scroll.xsize &&
      mousey>p->htmlframe[i].scroll.ytop &&
      mousey<p->htmlframe[i].scroll.ytop+p->htmlframe[i].scroll.ymax)
    p->activeframe=i;
  }
  while(i++<arachne.framescount);

  mouseoff();
  drawactiveframe();
  mouseon();
 }

 while(currentHTMLatom!=IE_NULL)
 {
  count++;

  if(!(count % 64) && !kbmouse) //give higher priority to mouse pointer
  {
   int newx=mousex,newy=mousey;
   kbhit();
   ImouseRead( &newx, &newy );
   if(newx!=mousex || newy!=mousey)
    return NULL;
  }

  atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if(!atomptr)
   MALLOCERR();
  if(currentHTMLatom==p->lastonscr)
   nextHTMLatom=IE_NULL;
  else
   nextHTMLatom=atomptr->next;

  x=mousex-p->htmlframe[atomptr->frameID].scroll.xtop;//mouse coord rel. to PosX
  y=mousey-p->htmlframe[atomptr->frameID].scroll.ytop;//mouse coord rel. to PosY
  dx=p->htmlframe[atomptr->frameID].posX;
  dy=p->htmlframe[atomptr->frameID].posY;

  if(atomptr->type!=TABLE && atomptr->type!=TD
     && atomptr->type!=TD_BACKGROUND && //no tables !
     atomptr->x-dx<=x && atomptr->xx-dx>=x &&
     atomptr->y-dy<=y && atomptr->yy-dy>=y &&
     x>0 && x<p->htmlframe[atomptr->frameID].scroll.xsize &&
     y>0 && y<p->htmlframe[atomptr->frameID].scroll.ysize)
  {

   if(click) //copy to clipboard
   {
    if(click==MOUSE_RIGHT && atomptr->type==IMG)
    {
     struct picinfo *imgptr=(struct picinfo *)ie_getswap(atomptr->ptr);
     GLOBAL.postdata=0;
     arachne.target=0;
     //!!JdS: 2004/12/08 {
     //Provide the capability to add an image hyperlink to the hotlist
     atomptr = (struct HTMLrecord *) ie_getswap(atomptr->linkptr);
     if (atomptr)
      ptr = (char *) ie_getswap(atomptr->ptr);
     if (atomptr && ptr)
     {
      ie_clipstatus = 0;  //overwrite mode
      ie_appendclip(ptr);
      GLOBAL.clipdel = CLIPBOARD_DEFER_ADD;
     }
     //!!JdS: 2004/12/08 }
     return imgptr->URL;
    }
    if(click==MOUSE_LEFT && atomptr->type==TEXT && atomptr->linkptr==IE_NULL)
    {
     ptr=(char *)ie_getswap(atomptr->ptr);
     if(*ptr)
     {
      outs(ptr);

      ie_appendclip(ptr);

      GLOBAL.clipdel = CLIPBOARD_DEFAULT;
      memcpy(&atomonmouse,atomptr,sizeof(struct HTMLrecord));

      highlightatom(&atomonmouse);
      return NULL;
     }
    }
   }

   linkonmouse=atomptr->linkptr;
   if(linkonmouse==IE_NULL) goto nolink; //IE_NULL=no link //original line

   /* okopiruju HTMLatom pro potreby ISMAP, USEMAP, copy link, view image...*/
   // tr.: I copy HTMLatom for use of ISMAP, USEMAP, copy link, view image...
   memcpy(&atomonmouse,atomptr,sizeof(struct HTMLrecord));
   //now, we won't use "atomptr" pointer until new ie_getswap()

   activeadr=currentHTMLatom;

   if(click &&
     (atomonmouse.type==INPUT ||
      atomonmouse.type==IMG && atomonmouse.data1)) // <IMG ISMAP,USEMAP>, <INPUT TYPE=IMAGE>
   {
    if(click==MOUSE_RELEASE)
     goto textarea_only;

    if(atomonmouse.data1==RESET)
     return "arachne:again";

    if(atomonmouse.data1==BUTTON)
     goto nolink;

    activeatomcursor(0);

    if(!(atomonmouse.type==INPUT && atomonmouse.data1==TEXTAREA))
     toolbar(0,1);

    if(atomonmouse.type==INPUT && atomonmouse.data1==CHECKBOX)
    {
     atomonmouse.data2=1-atomonmouse.data2;
     if(activeatomptr)
      defaultmsg();
     activeatomptr=&atomonmouse;
     activeatomredraw();
     activeatomsave(&atomonmouse);
     activeatomptr=NULL;
    }
    else
    if(atomonmouse.type==INPUT && atomonmouse.data1==RADIO)
    {
     if(activeatomptr)
      defaultmsg();
     RadioSwitch(dx,dy,atomonmouse.ptr,linkonmouse);
     activeatomptr=NULL;
    }
    else
    if(atomonmouse.type==INPUT && atomonmouse.data1==SELECT)
    {
     if(activeatomptr)
      defaultmsg();
     activeatomsave(&atomonmouse);
     editorptr=(struct ib_editor *)ie_getswap(activeatom.ptr);
     if(!editorptr)
      MALLOCERR();
     if(activeatom.yy-activeatom.y<editorptr->lines/2*fonty(OPTIONFONT,0))
     {
      int d=8+fonty(OPTIONFONT,0)*(editorptr->lines/2);
      long realy=activeatom.y-dy+p->htmlframe[activeatom.frameID].scroll.ytop;

      if(realy+d>p->htscrn_ysize+p->htscrn_ytop && realy-p->htscrn_ytop>p->htscrn_ysize/2)
      {
       if(realy-d<p->htscrn_ytop)
	d=(int)(realy+fonty(OPTIONFONT,0)-p->htscrn_ytop-3*FUZZYPIX);
       activeatom.y=activeatom.yy-d;
      }
      else
      {
       if(realy+d>p->htscrn_ysize+p->htscrn_ytop)
	d=(int)(p->htscrn_ysize+p->htscrn_ytop-realy-3*FUZZYPIX);
       activeatom.yy=activeatom.y+d;
      }
      mouseoff();
      drawatom(&activeatom,dx,dy,
		p->htscrn_xsize+p->htscrn_xtop-p->htmlframe[activeatom.frameID].scroll.xtop,
		p->htscrn_ysize+p->htscrn_ytop-p->htmlframe[activeatom.frameID].scroll.ytop, //select window can overwrite other
		p->htmlframe[activeatom.frameID].scroll.xtop,
		p->htmlframe[activeatom.frameID].scroll.ytop);  //frames...
      mouseon();
      htmlpulldown=1;//important flag - tells us that HTML visual is in special mode
     }
     else
      SelectSwitch(x+dx,y+dy,0);
     activeatomptr=NULL;
     goto exit_cursor;
    }
    else
    {
     if(atomonmouse.type==INPUT && atomonmouse.data1==SUBMIT)
      atomonmouse.data2|=1; //set checked bit

     activeatomsave(&atomonmouse);

     if(atomonmouse.type!=INPUT ||
	atomonmouse.data1!=SUBMIT && atomonmouse.data1!=TEXTAREA)
      activeatomcursor(1);

     textarea_only:

     if(click==MOUSE_MIDDLE && atomonmouse.type==INPUT &&
	(atomonmouse.data1==TEXTAREA || atomonmouse.data1==TEXT
	 || atomonmouse.data1==PASSWORD))
      activeatomtick(ASCIICTRLV,0);
     else
     if(atomonmouse.type==INPUT &&
	(atomonmouse.data1==TEXT || atomonmouse.data1==TEXTAREA))
     {
      //goto mouse pointer:
      char textareamode=TEXTAREA_INIT;
      if(atomonmouse.data1==TEXT ||
	 (x<atomonmouse.xx-dx-2-user_interface.scrollbarsize &&
	  y<atomonmouse.yy-dy-2-user_interface.scrollbarsize))
      {
       long realatomy=atomonmouse.y+2-dy;
       int realatomx=atomonmouse.x+2-dx;
       int newx,newy;

       if(realatomy<0l)
	realatomy=0l;
       if(realatomx<0)
	realatomx=0;

       activeatomcursor(0);
       editorptr=(struct ib_editor *)ie_getswap(activeatom.ptr);
       if(!editorptr)
	  goto exit_cursor; //non fatal error
//        MALLOCERR();

       newx=editorptr->zoomx+(x-realatomx)/fontx(SYSFONT,0,' ');

       if(atomonmouse.data1==TEXTAREA)
	newy=editorptr->zoomy+(int)((y-realatomy)/fonty(SYSFONT,0));
       else
	newy=editorptr->y;

       //...................................................doubleclick ?
       if(editorptr->x==newx && editorptr->y==newy)
       {
	if(click==MOUSE_RELEASE)
	{
	 activeatomcursor(1);
	 goto exit_cursor;
	}

	if(atomonmouse.data1==TEXTAREA)
	{
	 if(click==MOUSE_RIGHT && editorptr->blockflag==2) //right mouse button
	 {
	  activeatomtick(ASCIICTRLX,0); //clean line (URL, etc.)
	  goto exit_cursor;
	 }
	 else if(editorptr->blockflag==2) //mark start of block
	 {
	  editorptr->bbx=editorptr->bex=newx;
	  editorptr->bby=editorptr->bey=newy;
	  editorptr->blockflag=4;    //hide block, allow new marking by mouse
	  textareamode=TEXTAREA_SCROLL;
	 }
	 else
	 {
	  editorptr->blockflag=2; //block is highlighted
	  editorptr->bbx=0;
	  editorptr->bex=0;
	  editorptr->bby=newy;
	  editorptr->bey=newy+1;  //mark line
	  swapmod=1;
	  activeatomtick(ASCIICTRLC,0); //clean line (URL, etc.)
	  activeatomtick(CURSOR_SYNCHRO,TEXTAREA_SCROLL);
	  toolbar(1,1);
	  goto exit_cursor;
	 }
	}
       }
       else //close block?
       if(atomonmouse.data1==TEXTAREA && click==MOUSE_RELEASE &&
	  (editorptr->blockflag==2 || editorptr->blockflag==4))
       {
	 editorptr->bbx=editorptr->x;
	 editorptr->bby=editorptr->y;
	 editorptr->bex=newx;
	 editorptr->bey=newy;
	 editorptr->blockflag=2;
	 ie_xblockbeginend(editorptr);
	 textareamode=TEXTAREA_SCROLL;
       }
       else
       if (click==MOUSE_LEFT && editorptr->blockflag!=2) //left mouse ...
	editorptr->blockflag=4; //... allow block marking

       editorptr->x=newx;
       editorptr->y=newy;
       swapmod=1;
       if(click==MOUSE_RIGHT)
       {
	if(atomonmouse.data1!=TEXTAREA)
	 activeatomtick(ASCIICTRLY,0); //copy to clipboard
	else if (editorptr->blockflag==2)
	 activeatomtick(ASCIICTRLC,0); //copy to clipboard
       }
      }
      activeatomtick(CURSOR_SYNCHRO,textareamode);
      activescroll.onscrollx=0;
      activescroll.onscrolly=0;
      if(atomonmouse.data1==TEXTAREA)
       toolbar(1,1);
     }//endif cursor
    }

    exit_cursor:

#ifdef OVRL
#ifndef XTVERSION
    if(click!=MOUSE_RELEASE)
    {
     //press button or INPUT TYPE=IMAGE
     if(atomonmouse.type==INPUT && atomonmouse.data1==SUBMIT ||
	atomonmouse.type==IMG && atomonmouse.data1!=IMG_USEMAP) //not USEMAP!
     {
      thisx=atomonmouse.x-dx+p->htmlframe[atomonmouse.frameID].scroll.xtop;
      thisy=(int)(atomonmouse.y-dy+p->htmlframe[atomonmouse.frameID].scroll.ytop);
      thisxx=atomonmouse.xx-dx+p->htmlframe[atomonmouse.frameID].scroll.xtop+2+2*(atomonmouse.type==INPUT);
      thisyy=(int)(atomonmouse.yy-dy+p->htmlframe[atomonmouse.frameID].scroll.ytop+2);
      if(thisx<p->htscrn_xtop)
       thisx=p->htscrn_xtop;
      if(thisxx>p->htscrn_xtop+p->htscrn_xsize)
       thisxx=p->htscrn_xtop+p->htscrn_xsize;
      if(thisy<p->htscrn_ytop)
       thisy=p->htscrn_ytop;
      if(thisyy>p->htscrn_ytop+p->htscrn_ysize)
       thisyy=p->htscrn_ytop+p->htscrn_ysize;
      pressthatbutton(1);
     }
    }
#endif
#endif
   }//end if INPUT or INPUT TYPE=IMAGE

   atomptr=(struct HTMLrecord *)ie_getswap(linkonmouse);
   if(!atomptr)
    goto nolink;

   // Client side imagemaps (CSIM) are analysed here !!!
   //-------------------------------------------------------------
   if(atomonmouse.type==IMG && atomptr->type==MAP)
   {
    linkonmouse=AnalyseCSIM(dx+x-atomonmouse.x,(int)(dy+y-atomonmouse.y),atomptr);
    atomptr=(struct HTMLrecord *)ie_getswap(linkonmouse);
    if(!atomptr)
     goto nolink;
   }
   //-------------------------------------------------------------

   if (atomptr->type==HREF || atomptr->type==FORM &&
			      (atomonmouse.data1==SUBMIT ||
			       atomonmouse.type==IMG &&
			       atomonmouse.data1==IMG_INPUT))
   {
    char removable=0;
    XSWAP sheetadr=atomptr->linkptr;
    unsigned char usehover=atomptr->R; //hack to determine of CSS - hover is used

    if(click && click!=MOUSE_RELEASE)
    {
     arachne.target=atomptr->data1;
     if(atomptr->type==FORM)
      GLOBAL.postdata=atomptr->data2+1;
     else
     {
      GLOBAL.postdata=0;
      if(atomptr->type==HREF && atomptr->data2)
       removable=1;
     }
    }

    ptr=(char *)ie_getswap(atomptr->ptr);
    if(!ptr)
     return NULL;

    if(linkonmouse!=lastonmouse)
    {
     outs(ptr);
     mouseoff();

     if(atomonmouse.type==TEXT && usehover)
     {

//!!glennmcc: Mar 05, 2005
//see below, no longer need here due to using Ray's new code
//      int xx,yy,xxx,yyy;
//      long sz;
//!!glennmcc: end

      hidehover();

      atomptr=(struct HTMLrecord *)ie_getswap(atomonmouse.next);
      if(atomptr->linkptr!=linkonmouse)
      {
       atomptr=(struct HTMLrecord *)ie_getswap(atomonmouse.prev);
       if(atomptr->linkptr!=linkonmouse)
       {
	struct TMPframedata *sheet;

	if(sheetadr==IE_NULL)
	 sheet=&(p->tmpframedata[p->activeframe]);
	else
	 sheet=(struct TMPframedata *)ie_getswap(sheetadr);

	if(sheet)
	{
	 atomonmouse.R=sheet->hoverR;
	 atomonmouse.G=sheet->hoverG;
	 atomonmouse.B=sheet->hoverB;
	 atomonmouse.data2|=sheet->hoversetbits;
	 atomonmouse.data2-=(atomonmouse.data2&sheet->hoverresetbits);
	}

//!!glennmcc: Mar 05, 2005 -- This entire block is now superseded by code
//from Ray's version of onmouse.c, to fix the CSS (frames) hover bug ...
/*
	xx=atomonmouse.x-dx+p->htmlframe[atomonmouse.frameID].scroll.xtop;
	yy=(int)(atomonmouse.y-dy+p->htmlframe[atomonmouse.frameID].scroll.ytop);
	xxx=atomonmouse.xx-dx+p->htmlframe[atomonmouse.frameID].scroll.xtop;
	yyy=(int)(atomonmouse.yy-dy+p->htmlframe[atomonmouse.frameID].scroll.ytop);
	if(xx<p->htscrn_xtop)xx=p->htscrn_xtop;
	if(yy<p->htscrn_ytop)yy=p->htscrn_ytop;
	if(xxx>x_maxx())xxx=x_maxx();
	if(yyy>x_maxy())yyy=x_maxy();
	sz=(long)((long)(xxx-xx+1)*(long)(yyy-yy+1))+4*sizeof(int);
	p->restorehoveradr=IE_NULL;
	if(sz>0 && 2*sz<MAXHOVER)
	{
	 char *buf=farmalloc((unsigned long)2*sz);
	 if(buf)
	 {
	  x_getimg(xx,yy,xxx,yyy,buf);
	  p->restorehoveradr=ie_putswap(buf,(unsigned)((long)(2l*sz)),CONTEXT_TMPIMG);
	  farfree(buf);
	 }

	 p->restorehoverx=xx;
	 p->restorehovery=yy;

#ifndef POSIX
	 bigfonts_allowed();
#endif

	 drawatom(&atomonmouse,dx,dy,
		   p->htscrn_xsize+p->htscrn_xtop-p->htmlframe[activeatom.frameID].scroll.xtop,
		   p->htscrn_ysize+p->htscrn_ytop-p->htmlframe[activeatom.frameID].scroll.ytop, //select window can overwrite other
		   p->htmlframe[activeatom.frameID].scroll.xtop,
		   p->htmlframe[activeatom.frameID].scroll.ytop);  //frames...
	 bigfonts_forbidden();
	}
       }
      }
      atomptr=(struct HTMLrecord *)ie_getswap(linkonmouse);
      ptr=(char *)ie_getswap(atomptr->ptr);
      if(!atomptr || !ptr)
       return NULL;
     }
     x_yncurs(1,mousex,mousey,(int)user_interface.brightmouse);
    }
*/

//--------------- begin paste-in from Ray's version of onmouse.c --------
//!!glennmcc/RAY: 05-03-03: Fix CSS (frames) hover bug:

#define FRAME p->htmlframe[atomonmouse.frameID].scroll
	{
/*
The screen originates at the top left corner.

p->htscrn_ytop
is the current 'Y' coordinate of the top of the HTML window,
which varies with screen style, but not with resolution:
0   when in fullscreen layout
25  when the small Ikons, URL/title bar are across the top
50  when the small Ikons, URL bar, Title bar are across the top
50  when the buttons are on right side, URL bar, Title bar across the top
100 when the large buttons, URL bar, Title bar are across the top

FRAME.ytop
is the offset of the current frame within the HTML window.

p->htmlframe[atomptr->frameID].posY
is the measure of the amount of vertical scrolling that has taken place
within the active frame (if any).

atomonmouse.y / atomonmouse.yy
The top / bottom of the atom relative to its absolute position ('dy' below).
*/
	 int x1,x2,y1,y2; // Atom horizontal start, stop; vertical start, stop.
	 long sz;                                 // Size of the atom in bytes.
// Asignments for 'dx' and 'dy' made above are:
// dx = p->htmlframe[atomptr->frameID].posX;
// dy = p->htmlframe[atomptr->frameID].posY;

// Absolute coordinates of atom are: scrolling XY - frame XY + HTML window XY
// because scrolling right moves absolute positions on the screen left.
// Changing dx or dy offsets message, but doesn't 'clip' it.
// Bigger numbers move to the left and up repectively because dx/dy work
// subtractively.
	 dx -= FRAME.xtop; // - p->htscrn_xtop // Always 0 with HTML window hard left.
	 dy -= FRAME.ytop - p->htscrn_ytop;

// If atom's left end moves past frame boundary due to scrolling, we want the
//  whole atom to pop into view when hot anyway , so don't trim hot
//  atom on the left unless it is right off the screen (0). NB this will only
//  work when the HTML window is always hard on the left margin of the screen!

//!!glennmcc: May 08, 2007 -- removed... causing problems :(
/*
//!!glennmcc: Mar 04, 2006 -- compensate for _not_ being 'hard left'
	 x1 = (int)(atomonmouse.x +
		    FRAME.xtop -
		    p->htmlframe[atomptr->frameID].posX);

	 if(x1<0)
//!!glennmcc: end
*/
	 x1 = 0;

//Ray: Mar 11, 2005
// This allows *visible* part of over long link to hover.
//!!glennmcc: Mar 04, 2006 -- did not work in 1024x768 res
//this now works in all resolutions
	 if (FRAME.xtop>(p->htscrn_xsize + p->htscrn_xtop))
//	 if (FRAME.xtop>p->htscrn_xtop)
	      x2 = p->htscrn_xsize + p->htscrn_xtop;
	 else
	      x2 = atomonmouse.xx + FRAME.xtop;
//Ray: end

	 y1 = (int)(atomonmouse.y +
		    FRAME.ytop -
		    p->htmlframe[atomptr->frameID].posY);
	 y2 = (int)(atomonmouse.yy +
		    FRAME.ytop -
		    p->htmlframe[atomptr->frameID].posY);

//!!glennmcc: Mar 02, 2006 -- removed, caused problems on non-framed pages
//Ray: Mar 05, 2005
// Guard against intrusion into the status line.
//	 if (y2 > p->htscrn_ysize + p->htscrn_ytop)
//	  y2 = p->htscrn_ysize + p->htscrn_ytop;
//Ray: end
//!!glennmcc: end

	 sz = (long)((x2 - x1 + 1) * (y2 - y1 + 1) + 4 * sizeof(int));
//!!glennmcc: Mar 04, 2006 -- we are now guarding against this by
//limiting max fontshift to +1 ... therefore, we no longer need this fix
//!!glennmcc: Mar 13, 2005 -- fixes <font size="greater than 3"> bug (see above)
//	 if (sz > 0 && sz < MAXHOVER)
	 if (sz > 0 && 2 * sz < MAXHOVER)//original line now re-enabled
//!!glennmcc: end

	 {                  // RAY: typecast made same form as one below.
	  char *buf = malloc((unsigned)((long)(2L * sz)));
	  if (buf)
	  {
	   x_getimg(x1, y1, x2, y2, buf);
//       p->restorehoveradr = IE_NULL; // Moved from a few lines up.
	   p->restorehoveradr =
	    ie_putswap(buf, (unsigned)((long)(2L * sz)), CONTEXT_TMPIMG);
	   free(buf);
	  }
	  p->restorehoverx = x1;
	  p->restorehovery = y1;
#ifndef POSIX
	  bigfonts_allowed();
#endif	  

// hidehover(); // This prevents hover going off!
// drawatom() Draws highlighted atom, but not unhighlighted atom!

	  drawatom(&atomonmouse, dx, dy, // (See drawatom() L80)
		   p->htscrn_xsize + p->htscrn_xtop -
		   p->htmlframe[activeatom.frameID].scroll.xtop,
		   // select window can overwrite other
		   p->htscrn_ysize + p->htscrn_ytop -
		   p->htmlframe[activeatom.frameID].scroll.ytop,
		   p->htmlframe[activeatom.frameID].scroll.xtop,
		   p->htmlframe[activeatom.frameID].scroll.ytop);  // frames...

#ifndef POSIX
	  bigfonts_forbidden();
#endif	  
	 } // End: if (sz > 0 && 2 * sz < MAXHOVER)
	} // End: block
       } // End: if (atomptr->linkptr != linkonmouse)
      } // End: if (atomptr->linkptr != linkonmouse)
      atomptr=(struct HTMLrecord *)ie_getswap(linkonmouse);
      ptr=(char *)ie_getswap(atomptr->ptr);
      if(!atomptr || !ptr)
       return NULL;
     } // End: if (atomonmouse.type==TEXT && usehover)
     x_yncurs(1,mousex,mousey,(int)user_interface.brightmouse);
    } // End: if (linkonmouse!=lastonmouse)
//____end paste-in from Ray's version of onmouse.c __________


    if(click==MOUSE_RIGHT) //copy link to clipboard
    {
     if(!strncmpi("mailto:",ptr,7))
      ptr+=7;
     ie_appendclip(ptr);

     highlightatom(&atomonmouse);
     if(removable)
     {
      outs(MSG_CLPDEL);
      GLOBAL.clipdel=CLIPBOARD_DELETE;
     }
     else
     {
      outs(MSG_CLPADD);
      GLOBAL.clipdel=CLIPBOARD_ADDHOT;
     }

     return NULL;
    }

    if(*ptr=='#' && click && click!=MOUSE_RELEASE) //clicking on A NAME
    {
     Goto_A_NAME(&ptr[1]);
     goto nolink;
    }

    lastonmouse=linkonmouse;
    return ptr;
   }
   else
//JdS 2006/02/05 {
//Per Ray : Make sure mouse's off-link colour is correct;
//          must turn mouse off then on to change colour.
    x_yncurs(0,mousex,mousey,(int)user_interface.darkmouse);
    x_yncurs(1,mousex,mousey,(int)user_interface.darkmouse);
//JdS 2006/02/05 }
   if(atomptr->type==FORM)
    return NULL;

   goto nolink;
  }//end analyse atom
  currentHTMLatom=nextHTMLatom;
 }//loop

 nolink:
//!!glennmcc: Feb 12, 2007 -- display 'alt text'
//imgflag enhancement by Ray
  {
   static int imgflag=0;
   if(atomptr && atomptr->type==IMG)
   {
    struct picinfo *imgptr=(struct picinfo*)ie_getswap(atomptr->ptr);
    if(strncmpi(imgptr->alt,"IMAGE",5))
    {
     outs(imgptr->alt);
     imgflag=1;
    }
   }
   else if(imgflag)
    {
     imgflag=0;
     defaultmsg();
    }
  }
//!!glennmcc: end

 if(click==MOUSE_RIGHT)
 {
  if(p->activeframe) //right click on frame source
  {
   arachne.target=0;
   return p->htmlframe[p->activeframe].cacheitem.URL;
  }
  else
   if(!ontoolbar)
   {
//!!glennmcc: Begin May 17, 2004 -- only go back in history if
// 'RightMouseGoesBack Yes' is in arachne.cfg
//!!glennmcc: Dec 19, 2004 -- changed to goback by default
//now it _will_ goback unless we say "No"
 return config_get_bool("RMGB", 1) ? "arachne:back" : NULL;
//!!glennmcc: end
   }
 }
 else
 if(click==MOUSE_MIDDLE && !ontoolbar) //middle button
   return "arachne:fullscreen";
 else
 if(click==MOUSE_LEFT)
 {
  activeatomcursor(0);
  activeatomptr=NULL;
  activeadr=IE_NULL;
  defaultmsg();
 }
 else
 if(lastonmouse!=IE_NULL && click!=MOUSE_RELEASE)
  {
  defaultmsg();
  mouseoff();
  hidehover();
  x_yncurs(1,mousex,mousey,(int)user_interface.darkmouse);
  }
//!!glennmcc: May 08, 2007 -- also call hidehover() here to make extra-sure
// that hover gets turned off when the mouse is move off of the link.
 else
 hidehover();
//!!glennmcc: end
 lastonmouse=IE_NULL;
 return NULL;
}

void hidehover(void)
{
 if(p->restorehoveradr!=IE_NULL)
 {
  char *ptr=ie_getswap(p->restorehoveradr);
  if(ptr)
   x_putimg(p->restorehoverx,p->restorehovery,ptr,0);
  p->restorehoveradr=IE_NULL;
  ie_killcontext(CONTEXT_TMPIMG);
 }
}

// Copy active atom to xswap
void activeatomsave(struct HTMLrecord *atom)
{
 struct HTMLrecord *atomptr;

 memcpy(&activeatom,atom,sizeof(struct HTMLrecord));
 activeatomptr=&activeatom;
 atomptr=(struct HTMLrecord *)ie_getswap(activeadr);
 if(atomptr)
 {
  memcpy(atomptr,atom,sizeof(struct HTMLrecord));
  swapmod=1;
 }
}


//---------- 8< cut: this is part 2/2 of CSIM implemntation -------------

//Algorithm for Polyline (polygon) is Copyright (c)1998 Michael Polak, Prague
//Analyse <AREA SHAPE=DEFAULT|CIRC|RECT|POLY>
XSWAP AnalyseCSIM(int x, int y,struct HTMLrecord *map)
{
 XSWAP currentHTMLatom=map->next,thislink,linkatom=IE_NULL;
 int *array;
 char shape;
 struct HTMLrecord *atomptr;

 while(currentHTMLatom!=IE_NULL)
 {
  atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if(atomptr)
  {
   currentHTMLatom=atomptr->next;
   if(atomptr->type==AREA)
   {
    thislink=atomptr->linkptr;
    shape=atomptr->data1;
    array=(int *)ie_getswap(atomptr->ptr);
    switch(shape)
    {
     case DEFAULT:
     if(linkatom==IE_NULL)
      linkatom=thislink;
     break;

     case RECT:
     if(x>array[0] && y>array[1] && x<array[2] && y<array[3])
      linkatom=thislink;
     break;

     case CIRCLE:
     {
      int a=array[0]-x;
      int b=array[1]-y;
      if((long)a*(long)a+(long)b*(long)b<(long)array[2]*(long)array[2])
       linkatom=thislink;
     }
     break;

     case POLY:
     {
      int i=0; //coordinates from array
      int n=0; //number of sub-polygons in which mouse is located
      int x1,x2,y1,y2,pom;
      while(array[i]>=0 && array[i+1]>=0 && i<1024)
      {
       x1=array[i];
       y1=array[i+1];
       x2=array[i+2];
       y2=array[i+3];
       if(x2<0 || y2<0) //automaticly close main polygon
       {
	x2=array[0];
        y2=array[1];
       }

       if(x1>x2)
       {
        pom=x1;x1=x2;x2=pom;
        pom=y1;y1=y2;y2=pom;
       }

       //is mouse pointer located in this sub-polygon ?
       if(x>=x1 && x<x2 && y-y1<(long)(x-x1)*(long)(y2-y1)/(x2-x1))
	n++;

       //process next two coordinates:
       i+=2;
      }
      if(n%2) //odd number of sub-polygons -> mouse is inside main polygon
       linkatom=thislink;
     }
    }
   }
   else if(atomptr->type==MAP)
    return linkatom;
  }
  else
   MALLOCERR();
 }

 return linkatom;
}

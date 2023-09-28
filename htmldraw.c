
// ========================================================================
// HTML drawing routines for Arachne WWW browser
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "html.h"
#include "xanimgif.h"

// ================== drawing a single HTML atom: ======================

void drawatom(struct HTMLrecord *atom, 
      int fromx, long fromy, int draw_x, int draw_y, int screen_x, int screen_y)
{
 char txt[IE_MAXLEN+2],*ptr;
 int x1,x2,x0;
 long y1,y2,y0;
 int tfrom=0,i=0,j,max,xskip;
 char font,style;

 x0=x1=atom->x-fromx;
 y0=y1=atom->y-fromy;
 x2=atom->xx-fromx;
 y2=atom->yy-fromy;

 switch(atom->type)
 {
  case TEXT: // ************************************************************

   if(atom->y-fromy<0l || atom->yy-fromy>draw_y)return;

   j=fromx-atom->x;  // j>0 --> nebude videt cely string
              // tr.: j>0 --> the whole string will not be visible 
   max=atom->datalen;

   font=atom->data1;
   style=atom->data2;
   if(cgamode)
    x_setcolor(p->tmpframedata[atom->frameID].cgatext);
   else
    x_setcolor(RGB(atom->R,atom->G,atom->B));

   ptr=ie_getswap(atom->ptr);
   if(!ptr)
     MALLOCERR();

   htmlfont((int)font,style);

   if(j>0)
    tfrom=x_charmax((unsigned char*)ptr,j+FUZZYPIX);
   else
   {
    tfrom=0;
    j=0;
   }

   xskip=j;
   max=x_charmax((unsigned char*)&ptr[tfrom],draw_x-x1-j);
   if(max>IE_MAXLEN)
    max=IE_MAXLEN;

   makestr(txt,&ptr[tfrom],max);

   if(style & TEXT3D)
   {
    int colormap[5]={15,-1,-1,8,7};
    decorated_text(x1+screen_x+xskip,(int)(y1+screen_y),txt,colormap);
   }
   else
   if(style & TEXT3D2)
   {
    int colormap[5]={8,-1,-1,15,7};
    decorated_text(x1+screen_x+xskip,(int)(y1+screen_y),txt,colormap);
   }
   else
    x_text_ib(x1+screen_x+xskip,(int)(y1+screen_y),(unsigned char *)txt);

   if(style & UNDERLINE)
   {
    int sz=fontx(font,style,'_');
    x2=x1+x_txwidth(txt);

    j=x1+sz;
    txt[0]='_';
    i=1;
    while(j+sz<x2 && j<draw_x-FUZZYPIX && i<IE_MAXLEN)
    {
     txt[i]='_';
     j+=sz;
     i++;
    }
    txt[i]='\0';

    x_text_ib(x1+screen_x+xskip,(int)(y1+screen_y),(unsigned char *)txt);
    if(x2-x1>sz && x2<draw_x)
     x_text_ib((int)(x2+screen_x-sz+xskip),(int)(y1+screen_y),(unsigned char *)"_");
   }

//!!JDS: Feb 28, 2007 -- add support for <S> (strike)
   if(style & STRIKE)
   {
    int sz=fontx(font,style,'-');
    x2=x1+x_txwidth(txt);

    j=x1+sz;
    txt[0]='-';
    i=1;
    while(j+sz<x2 && j<draw_x-FUZZYPIX && i<IE_MAXLEN)
    {
     txt[i]='-';
     j+=sz;
     i++;
    }
    txt[i]='\0';

    x_text_ib(x1+screen_x+xskip,(int)(y1+screen_y),(unsigned char *)txt);
    if(x2-x1>sz && x2<draw_x)
     x_text_ib((int)(x2+screen_x-sz+xskip),(int)(y1+screen_y),(unsigned char *)"-");
   }
//!!JDS: end


  break; //end plain text


  case IMG: // *********************************************************** IMG
  case TD_BACKGROUND: // ********************************** TD with BACKGROUND
  {
   struct picinfo *image;
   unsigned image_xswapadr;
   int back=-1,border=0,bordercolor,frameID,txlen;
   XSWAP dummy1;
   unsigned dummy2;
   char atype=atom->type;
   int bgcolor=0;
   int atomx;
   long atomy;
   unsigned char r,g,b;

   if(atype==TD_BACKGROUND && atom->data2)
   {
    r=atom->R;
    g=atom->G;
    b=atom->B;
    bgcolor=1;
   }
   else
    border=atom->data2;

   if(x2>draw_x)x2=draw_x;
   if(y2>draw_y)y2=draw_y;
   if(x1<0)x1=0;
   if(y1<0)y1=0;
   if(border)
    bordercolor=RGB(atom->R,atom->G,atom->B);
   frameID=atom->frameID;
   atomx=atom->x;
   atomy=atom->y;

   image_xswapadr=atom->ptr;
   image=(struct picinfo *)ie_getswap(image_xswapadr);
   if(image)
   {
    if(!image->filename[0])
    {
     struct HTTPrecord HTTPdoc;
     struct Url url;

     AnalyseURL(image->URL,&url,IGNORE_PARENT_FRAME);
     if(SearchInCache(&url,&HTTPdoc,&dummy1,&dummy2))
     {
      image=(struct picinfo *)ie_getswap(image_xswapadr);
      strcpy(image->filename,HTTPdoc.locname);
      swapmod=1; //zapsat ho uz trvale !!!
        // tr.: write it already permanently/definetely!
     }
     else
      image=(struct picinfo *)ie_getswap(image_xswapadr);
    }
    image->hPicInfo=image_xswapadr;
    if(image->filename[0] && !cgamode && !ignoreimages)
    {
     char *ext=strrchr(image->filename,'.');

     image->from_x=image->pic_x=x0+screen_x+border;
     image->from_y=image->pic_y=(int)(y0+screen_y)+border;
     if(image->pic_x<screen_x+1)
      image->pic_x=screen_x+1;
     if(image->pic_y<screen_y+1)
      image->pic_y=screen_y+1;

     //======================================================================
     if(!ext || strcmpi(ext,".ikn")) //..... not Arachne icon file...
     //======================================================================
     {
      //because of animation
      image->html_x=atomx+border;
      image->html_y=atomy+border;

      image->stop_x=(int)(screen_x+x2)+1;
      image->stop_y=(int)(screen_y+y2)+1;
      if(image->stop_x>screen_x+draw_x)image->stop_x=screen_x+draw_x;
      if(image->stop_y>screen_y+draw_y)image->stop_y=screen_y+draw_y;
      image->sizeonly=0;
      image->palonly=0;
      if(atype==TD_BACKGROUND)
      {
       image->is_background=1;
       image->bgindex=p->tmpframedata[frameID].bgindex;
       image->screen_x=image->from_x=image->pic_x=x1+screen_x;
       image->screen_y=image->from_y=image->pic_y=(int)(y1+screen_y);
       image->draw_x=x2-x1;
       image->draw_y=(int)(y2-y1);
       image->stop_x=x2+screen_x-1;
       image->stop_y=(int)(y2+screen_y-1);
       image->alt[0]='\0';

#ifdef HICOLOR
       if(xg_256 == MM_Hic) //HARO
       { image->palismap = 0;
       }
#endif
      }
      else
      {
       image->is_background=0;
       image->screen_x=screen_x;
       image->screen_y=screen_y;
       image->draw_x=draw_x;
       image->draw_y=draw_y;
      }

     image->IsInXms=0;

      if(drawanyimage(image)!=1)
      {
       if(!strstr((char *)strlwr(image->filename),".jpg"))
        back=10; //red frame 
      }
      else
      {
//       printf("]");
       while(border>0)
       {
        border--;
        x_setcolor(bordercolor);
	x_rect((int)(image->pic_x+border-1),(int)(image->pic_y+border-1),
	       (int)(screen_x+x2)-border-1,(int)(screen_y+y2)-border-1);
       }
       return;
      }
     }
     //======================================================================
     else //Arachne icon file - special case...
     //======================================================================
     {
      if(image->from_x>=screen_x && image->from_x+image->size_x<screen_x+draw_x &&
         image->from_y>=screen_y && image->from_y+image->size_y<screen_y+draw_y)
      {
       neediknredraw=1;
       DrawIconLater(image->filename,image->from_x, image->from_y );
      }
      return;
     }
    }
    txlen=(x2-x1-2)/fontx(SYSFONT,0,' ');
    if(txlen<0)txlen=0;
    if(txlen>100)txlen=100;
    makestr(txt,image->alt,txlen);
   }
   else
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return;
//    MALLOCERR();
//!!glennmcc: end

   //draw box instead of image

   //bigger than 4 pixels ?
   if((x2-x1)<4 || (y2-y1)<4)
    return;

   if(atype==TD_BACKGROUND)
   {
    if(bgcolor)
     back=RGB(r,g,b);
    else
     back=RGB(p->tmpframedata[frameID].backR,
              p->tmpframedata[frameID].backG,
              p->tmpframedata[frameID].backB);
   }
   Cell3D((int)(screen_x+x1),(int)(screen_y+y1),
          (int)(screen_x+x2),(int)(screen_y+y2),back);

   if(x1!=x0 || y1!=y0 || (y2-y1)<14 || atype==TD_BACKGROUND)
    return;

   //drawing alternative text:
   htmlfont(SYSFONT,0);
   if(back==10)
    x_setcolor(14);
   else
    x_setcolor(RGB(p->tmpframedata[frameID].textR,
                   p->tmpframedata[frameID].textG,
                   p->tmpframedata[frameID].textB));
   x_text_ib((int)(x1+screen_x+1),(int)(screen_y+y1+1),(unsigned char *)txt);

  }
  break;

  case TD: // ************************************************************* TD
  case TABLE: // ******************************************************* TABLE
   x2--;
   y2--;
  case HR: // ************************************************************* HR
  case LI: // ************************************************************* LI
   if(x2>draw_x)x2=draw_x;
   if(y2>draw_y)y2=draw_y;
   if(x1<0)x1=0;
   if(y1<0)y1=0;

   //ctverecek - LI, HR NOSHADE (tr.: small square)
   if((atom->data1 && (atom->type==HR || atom->type==LI)) ||
      (atom->type==TD && atom->data2))  //data2=BGCOLOR...
   {
    if(atom->type==TD)
    {
     x_setfill(0,RGB(atom->R,atom->G,atom->B));
    }
    else
     x_setfill(0,8);

    x_bar((int)(screen_x+x1),(int)(screen_y+y1),
          (int)(screen_x+x2),(int)(screen_y+y2));

    if(atom->type!=TD)
     return;
   }

   //no border
   if(atom->type==TD && atom->data1==0 ||
      atom->type==TABLE && atom->data1<1) //data 1=BORDER, BORDER=-1 -> FRAME=VOID
    return;

   {
    int c1=8,c2=15;

/*#ifdef HICOLOR
    if(xg_256 == MM_Hic) initpalette();
#endif*/

    if((p->tmpframedata[atom->frameID].bgindex==15 ||
        p->tmpframedata[atom->frameID].bgindex==0) && !atom->data1)
     c2=7;

    if(atom->type==TD)
     x_setcolor(c1);
    else
     x_setcolor(c2);

    x_line((int)(screen_x+x1),(int)(screen_y+y2),
	   (int)(screen_x+x2),(int)(screen_y+y2));
    x_line((int)(screen_x+x2),(int)(screen_y+y1),
           (int)(screen_x+x2),(int)(screen_y+y2));

    if(atom->type==TD)
     x_setcolor(c2);
    else
     x_setcolor(c1);

   }
   x_line((int)(screen_x+x1),(int)(screen_y+y1),
          (int)(screen_x+x2),(int)(screen_y+y1));
   x_line((int)(screen_x+x1),(int)(screen_y+y1),
	  (int)(screen_x+x1),(int)(screen_y+y2));
 break;

  case INPUT: // ******************************************************* INPUT
  {
   char type, checked;

   type=atom->data1;
   checked=atom->data2;

   if(x1<0)x1=0;
   if(x2>draw_x && atom!=&URLprompt)x2=draw_x;

   if(x2-x1<4)
    break;

   if(type==SELECT || type==TEXTAREA)
   {
    if(y2>draw_y)y2=draw_y;
    if(y1<0)y1=0;
   }
   else if(atom!=&URLprompt && (y2+2>draw_y || y1<0))
    break;

   //careful redraw
   if((type==TEXT || type==PASSWORD) && y2>y1+2)
   {
    if(fonty(SYSFONT,0)<=16 || atom!=&URLprompt)
     y2-=2;
    Cell3D((int)(screen_x+x1),(int)(screen_y+y1),
	   (int)(screen_x+x2),(int)(screen_y+y2),user_interface.paper);

    if(checked)
     will_activate_atom((int)(screen_x+x2),(int)(screen_y+y2));
   }
   else
   if(type==TEXTAREA)
   {
    if(y2<=y1+user_interface.scrollbarsize+5+fonty(SYSFONT,0) ||
       x2<=x1+user_interface.scrollbarsize+5+fontx(SYSFONT,0,' ') ||
       y2<=y1+40 || x2<=x1+40+5+fontx(SYSFONT,0,' ')) //dve rolovaci tlacitka vedle sebe...
	  // tr.: two scroll buttons side by side
     break;

    Cell3D((int)(screen_x+x1),(int)(screen_y+y1),
	   (int)(screen_x+x2),(int)(screen_y+y2),user_interface.paper);
    if(checked)
     will_activate_atom((int)(screen_x+x2),(int)(screen_y+y2));
   }
   else
   if((type==SUBMIT || type==RESET || type==SELECT)&& y2>y1+2)
//    Box3D1pix((int)(screen_x+x1),(int)(screen_y+y1),
    Box3Dh((int)(screen_x+x1),(int)(screen_y+y1),
	   (int)(screen_x+x2),(int)(screen_y+y2-2));
   if(type==BUTTON && y2>y1+2)
    Cell3D((int)(screen_x+x1),(int)(screen_y+y1),
	  (int)(screen_x+x2),(int)(screen_y+y2-2),-1);
   else
   if(type==RADIO && y2>y1+6)
    Cell3D((int)(screen_x+x1+2),(int)(screen_y+y1+3),
	   (int)(screen_x+x2-1),(int)(screen_y+y2-4),user_interface.paper);
   else
   if(type==CHECKBOX && y2>y1+6)
    Cell3D((int)(screen_x+x1),(int)(screen_y+y1+3),
	   (int)(screen_x+x2),(int)(screen_y+y2-3),user_interface.paper);

   editorptr=(struct ib_editor *)ie_getswap(atom->ptr);

   if(editorptr && y1+fonty(SYSFONT,0)<draw_y &&
      (y2-fonty(SYSFONT,0)>0 || atom==&URLprompt))
   {
    int width,zoomx;

    zoomx=editorptr->zoomx;

    if(type==SUBMIT || type==RESET)
     width=(x2-x1)/fontx(BUTTONFONT,0,'a');
    else
     width=(x2-x1)/fontx(SYSFONT,0,' ');

    if(width>=160)width=159;

    if(type==TEXTAREA)
    {
     struct ScrollBar tmpscroll,*scroll;
     if(atom==activeatomptr)
      scroll=&activescroll;
     else
      scroll=&tmpscroll;


//?     ActivateWidget(&tmpeditor,
     memcpy(&tmpeditor,editorptr,sizeof(struct ib_editor));

     ie_redrawwin(&tmpeditor,(int)(screen_x+x1+2),(int)(screen_y+y1+2),
                 (int)(screen_x+x2-2-user_interface.scrollbarsize),
                 (int)(screen_y+y2-2-user_interface.scrollbarsize),
                 (atom==activeatomptr));

     scroll->xvisible=1;
     scroll->yvisible=1;
     ScrollInit(scroll,(int)(x2-x1-user_interface.scrollbarsize-5),
                       (int)(y2-y1-user_interface.scrollbarsize-5),
                (int)(y2-y1-5),(int)(screen_x+x1+2),(int)(screen_y+y1+2),
               tmpeditor.cols*fontx(SYSFONT,0,' '),(long)tmpeditor.lines*fonty(SYSFONT,0));
     global_nomouse=1;
     ScrollButtons(scroll);
     ScrollDraw(scroll,tmpeditor.zoomx*fontx(SYSFONT,0,' '),(long)tmpeditor.zoomy*fonty(SYSFONT,0));
     global_nomouse=0;

     break;
    }
    else
    if(type==SELECT)

    // ---- <SELECT> tag output: --------------------------

    {
     char vidimscroll=0;
     memcpy(&tmpeditor,editorptr,sizeof(struct ib_editor));

     if(user_interface.scrollbarsize>3)
     {
      if(tmpeditor.lines>2*(y2-y1-4)/fonty(OPTIONFONT,0))
      {
       struct ScrollBar scroll;
       char resetstyle=0;

       if(!user_interface.scrollbarstyle)
       {
        user_interface.scrollbarstyle='N';
	resetstyle=1;
       }

       scroll.xvisible=0;
       scroll.yvisible=1;
       ScrollInit(&scroll,
		  (int)(x2-x1-user_interface.scrollbarsize-6),
                  (int)(y2-y1-8),
                  (int)(y2-y1-8),
		  (int)(screen_x+x1+3),
                  (int)(screen_y+y1+3),1,1);
       ScrollButtons(&scroll);
       if(scroll.scrollbarstyle)
	vidimscroll=1;
       if(resetstyle)
        user_interface.scrollbarstyle=0;
      }

      //2*40+~sizeof scratch.. grrr, constant value :(
      if(x2>x1+user_interface.scrollbarsize && !vidimscroll)
      {
       Scratch3D((int)(screen_x+x2-user_interface.scrollbarsize+3),
                 (int)(screen_y+y1+(y2-y1)/2-1),
                 (int)(screen_x+x2-3));
      }
     }//end if invisible scrollbars...

     if(y2-y1<2*fonty(OPTIONFONT,0))
     {
      putoptionline((int)(screen_x+x1+2),(int)(screen_y+y1+3),
		    x2-x1-user_interface.scrollbarsize,
                    &tmpeditor,tmpeditor.y,checked);
     }
     else
     {
      int l=tmpeditor.zoomy;
      while(y1<y2-fonty(OPTIONFONT,0)-4 && l<tmpeditor.lines)
      {
       putoptionline((int)(screen_x+x1+2),(int)(screen_y+y1+3),
		     x2-x1-user_interface.scrollbarsize,
                     &tmpeditor,l,checked);
       y1+=fonty(OPTIONFONT,0);
       l+=2;
      }
     }
     break;
    }

    // ----------------------------------------------------

    else
    if(type==TEXT || type==PASSWORD || type==SUBMIT ||
       type==RESET || type==BUTTON)
    {
     char *txtptr=txt;

     /*
     if(type==SUBMIT || type==RESET)
     {
      width--;
      if(width<=0)
       break;
      txtptr++;
      txt[0]=' ';
     }
     */

     if(editorptr->aktrad==0)
      strncpy(txtptr,&editorptr->rad[zoomx],width);
     else
     {
      ptr=ie_getswap(getXSWAPlineadr(editorptr,0));
      if(ptr)
       strncpy(txtptr,&ptr[zoomx],width);
      else
       width=0;
     }
    }
    else if(type==CHECKBOX && checked)
    {
     x_setcolor(user_interface.ink);
     Cross(x1+screen_x+3,(int)(screen_y+y1+6),fontx(SYSFONT,0,' ')-2);
     break;
    }
    else if(type==RADIO && checked)
    {
     x_setfill(0,user_interface.ink);
     x_bar(x1+screen_x+5,
	   (int)(screen_y+y1+6),
           x1+screen_x+fontx(SYSFONT,0,' '),
           (int)(y1+screen_y+fontx(SYSFONT,0,' ')+3));
     break;
    }
    else
     break;

    if(width<=0 || checked & 2) //do not draw value of TAG_BUTTON!
     break;

    txt[width]='\0';
    if(type==PASSWORD)
    {
     width=strlen(txt);
     while(--width>=0)txt[width]='*';
    }

    if(type==SUBMIT || type==RESET || type==BUTTON)
    {
     htmlfont(BUTTONFONT,0);
     width=x_charmax((unsigned char *)txt,x2-x1);
     if(width>0 && width<strlen(txt))
      txt[width]='\0';
     width=x_txwidth(txt);
     if(type==BUTTON)
      x_setcolor(RGB(atom->R,atom->G,atom->B)); //colour of text
     else
      x_setcolor(0); //black (button)
     //x_settextjusty(1,2); //this does not work in virtual screens! :-(
      x_text_ib((int)((x2+x1)/2-width/2+screen_x),(int)(screen_y+y1+1),
               (unsigned char *)txt);
      //x_settextjusty(0,2); //this does not work in virtual screens! :-(
    }
    else
    {
     htmlfont(SYSFONT,0);
     if(fonty(SYSFONT,0)!=14)
      y1-=1;

     x_setcolor(user_interface.ink);
     x_text_ib((int)(x1+screen_x+2),(int)(screen_y+y1+2),
	       (unsigned char *)txt);

    }
   }
  }
  break;
 }
}//end sub



// --------------------------------------------------------------

#ifdef VIRT_SCR
//. Dump part of the virtual screen to the real screen
void dumpvirtual(struct HTMLframe *frame,struct TMPframedata *htmldata, int fromx,long fromy)
{
 if(!htmldata->usevirtualscreen)
 {
  char *ptr=ie_getswap(virtualpalhandle);
  if(ptr)
  {
   memcpy(Iipal,ptr,768);
   IiNpal=virtualIiNpal;
   x_palett( IiNpal, Iipal);
  }
  if(arachne.framescount==0)
  {
   p->firstonscr=virtualfirstonscr;
   p->lastonscr=virtuallastonscr;
  }
  htmldata->usevirtualscreen=1;
 }
 xv_set_actvirt(htmldata->whichvirtual);
 xv_to_scr(fromx-virtualxstart[htmldata->whichvirtual],
           (int)(fromy-virtualystart[htmldata->whichvirtual]),
	   frame->scroll.xtop,
           frame->scroll.ytop,
           frame->scroll.xsize+1,
	   frame->scroll.ysize+1);
}

void redrawatoms(char frame,
		 int from_x, long from_y,
                 int draw_x, int draw_y,
                 int screen_x, int screen_y)

{
 unsigned currentHTMLatom=p->firstonscr,nextHTMLatom;
 struct HTMLrecord *atomptr;

 while(currentHTMLatom!=IE_NULL)
 {
//  kbhit();
  atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if(!atomptr)
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return;
//   MALLOCERR();
//!!glennmcc: end
  nextHTMLatom=atomptr->next;
  if(atomptr->type==INPUT &&
     atomptr->data1!=SUBMIT && atomptr->data1!=RESET && atomptr->data1!=BUTTON &&
     atomptr->yy>=from_y && atomptr->xx>=from_x &&
     atomptr->y<from_y+draw_y && atomptr->x<from_x+draw_x &&
     atomptr->frameID==frame)
   drawatom(atomptr,from_x,from_y,draw_x,draw_y,screen_x,screen_y);
  currentHTMLatom=nextHTMLatom;
 }//loop
}

//. dellocate the XMS memory used by the virtual screen
void deallocvirtual(void)
{
 int i=maxusedvirtual;
 do
 {
  if(allocatedvirtual[i])
  {
   xv_cls_virt(0,i);
   allocatedvirtual[i]=0;
//   printf("dealokuji %d|",i);
// tr.: I deallocate %d
  }
 }
 while(i-->0);
 maxusedvirtual=0;
}
#endif


// --------------------------------------------------------------

void redrawHTML(char nomsg, char virt)
{
 unsigned currentHTMLatom,nextHTMLatom;
 long firstonscrn=p->HTMLatomcounter,lastonscrn=-1,n,from_y[MAXFRAMES];
 int draw_x[MAXFRAMES],draw_y[MAXFRAMES],
     screen_x[MAXFRAMES],screen_y[MAXFRAMES],from_x[MAXFRAMES];
 struct HTMLframe *frame;
 struct TMPframedata *htmldata;
 struct HTMLrecord *atomptr;
 char allvirtual=0;
 int i,lasttyc=-1;
 char pushcurrent;
 int imgcount=1;

 mouseoff();

#ifdef VIRT_SCR

 htmldata=&(p->tmpframedata[p->activeframe]);
 frame=&(p->htmlframe[p->activeframe]);

 if(virt==REDRAW_VIRTUAL && allocatedvirtual[htmldata->whichvirtual] &&
    (p->activeframe>0 || arachne.framescount==0) &&
    frame->posX>=virtualxstart[htmldata->whichvirtual] &&
    frame->posX+frame->scroll.xsize<virtualxend[htmldata->whichvirtual] &&
    frame->posY>=virtualystart[htmldata->whichvirtual] &&
    frame->posY+frame->scroll.ysize<virtualyend[htmldata->whichvirtual])
 {
#ifdef XANIMGIF
  XSetAnim1();
#endif
  dumpvirtual(frame,htmldata,frame->posX,frame->posY);
  redrawatoms(p->activeframe,
              frame->posX,frame->posY,
              frame->scroll.xsize,frame->scroll.ysize,
	      frame->scroll.xtop,frame->scroll.ytop);
  if(frame->allowscrolling)
  {
   ScrollButtons(&frame->scroll);
   ScrollDraw(&frame->scroll,frame->posX,frame->posY);
  }
 }
 else
 {
  allvirtual=1;

#endif

 resetcolorcache();
 neediknredraw=0;
 IiNpal=16;
 p->firstonscr=p->firstHTMLatom;
 p->lastonscr=p->lastHTMLatom;

#ifdef VIRT_SCR

 deallocvirtual();

 i=0;
 while(i<MAXFRAMES-1 && i>=0)
 {
  while(p->htmlframe[i].hidden && i<MAXFRAMES-1 && i!=-1)
  {
   i=p->htmlframe[i].next;
//   kbhit();
  }

  if(i>=MAXFRAMES-1 || i==-1)
   break;

  frame=&(p->htmlframe[i]);
  htmldata=&(p->tmpframedata[i]);

  draw_x[i]=frame->scroll.xsize;
  draw_y[i]=frame->scroll.ysize;
  from_x[i]=frame->posX;
  from_y[i]=frame->posY;
  screen_x[i]=frame->scroll.xtop;
  screen_y[i]=frame->scroll.ytop;


  if(
#ifndef POSIX
     arachne.xSwap==0 && !DisableXMS &&     //0...XMS, 2...disk
#endif
     virt==REDRAW_CREATE_VIRTUAL &&         //Smart - smaller virtual screen
     (user_interface.screenmode ||          //Auto(default) - only if it fits
      frame->scroll.total_y<=user_interface.virtualysize) ||
     virt && user_interface.screenmode=='N')//Nice(slow)- create screen always
  {
   char *fname="$0.obr";
   int   TypVirt = 0;  //HARO

   //reset virtual screen to zero
   virtualxstart[htmldata->whichvirtual]=0;
   virtualystart[htmldata->whichvirtual]=0;
   virtualxend[htmldata->whichvirtual]=0l;
   virtualyend[htmldata->whichvirtual]=0l;

   if(virt && (x_getmaxcol()==255||x_getmaxcol()==0) &&  //HARO
      user_interface.virtualysize && !arachne.xSwap) //xSwap==0 ... XMS
   {
    if(frame->scroll.total_x>frame->scroll.xsize)
    {
//!!Ray: July 07, 2007 -- increase from 1600 to 2048
//!!glennmcc: July 08, 2007 -- '<=' instead of simply '<'
//!!glennmcc: July 09, 2007 -- use BMPwidth CFG variable.
//defaults to min of 640 and max of 2048
 int value = config_get_int("BMPwidth", 640);
 if (value <  640) value = 640;
 if (value > 2048) value = 2048;
//!!glennmcc: July 13, 2008 -- prevent 'split screen'
// when BMPwidth is less than screen width
     if(value<x_maxx()) value=x_maxx();
//!!glennmcc: end

     if(frame->scroll.total_x<=value)
//   if(frame->scroll.total_x<=2048)
//   if(frame->scroll.total_x<2048)
      frame->scroll.xsize=frame->scroll.total_x+FUZZYPIX;
     else
      frame->scroll.xsize=value;//_NOW_ it _is_ definable in ARACHNE.CFG ;-)
//    frame->scroll.xsize=2048; // !!! will be definable in ARACHNE.CFG !!!
//!!Ray: end
    }
    else
     frame->scroll.xsize+=FUZZYPIX;

    if(frame->scroll.total_y>frame->scroll.ysize)
    {
     if(frame->scroll.total_y<user_interface.virtualysize)
      frame->scroll.ysize=(int)frame->scroll.total_y+FUZZYPIX;
     else
      frame->scroll.ysize=user_interface.virtualysize; // !!! will be definable in ARACHNE.CFG !!!
    }
    else
     frame->scroll.ysize+=FUZZYPIX;

    frame->posX-=frame->scroll.xsize/4;
    if(frame->posX+frame->scroll.xsize>frame->scroll.total_x)
     frame->posX=frame->scroll.total_x-frame->scroll.xsize+FUZZYPIX;
    if(frame->posX<0)
     frame->posX=0;

    frame->posY-=frame->scroll.ysize/4;
    if(frame->posY+frame->scroll.ysize>frame->scroll.total_y)
     frame->posY=frame->scroll.total_y-frame->scroll.ysize+FUZZYPIX;
    if(frame->posY<0)
     frame->posY=0;
#ifdef HICOLOR
    if(xg_256 == MM_Hic) TypVirt = -1;  // HARO
#endif
    //"$tmp0$.obr"
    fname[1]+=htmldata->whichvirtual;

    if(xv_new_virt(fname,        // File name for disk file
                   frame->scroll.xsize+FUZZYPIX,
		   frame->scroll.ysize+FUZZYPIX,  // Size in pixels dx,dy
                   0,                   // Default color
		   TypVirt,             // HARO // 3-1bit/pixel, 0-8bit/pixel, -1-16bit/pixel
		   256,                 // Length of palette
		   Iipal,               // Palette, range RGB 0..63, max. 256 entries
		   htmldata->whichvirtual,  // index of virtual videoram 0..5
		   1)==1)               // 1...force XMS, don't allow disk
    {
     //virtual screen was created in XMS:
     virtualxstart[htmldata->whichvirtual]=frame->posX;
     virtualxend[htmldata->whichvirtual]=frame->posX+frame->scroll.xsize;
     virtualystart[htmldata->whichvirtual]=frame->posY;
     virtualyend[htmldata->whichvirtual]=frame->posY+frame->scroll.ysize;
     allocatedvirtual[htmldata->whichvirtual]=1;
     htmldata->usevirtualscreen=1;
     frame->scroll.xtop=frame->scroll.ytop=0;
     if(htmldata->whichvirtual>maxusedvirtual)
      maxusedvirtual=htmldata->whichvirtual;
//     printf("alokuji %d|",htmldata->whichvirtual);
// tr.: I allocate %d
    }
    else
    {
     frame->scroll.xsize=draw_x[i];
     frame->scroll.ysize=draw_y[i];
     frame->posX=from_x[i];
     frame->posY=from_y[i];
     frame->scroll.xtop=screen_x[i];
     frame->scroll.ytop=screen_y[i];
     goto fail;
    }
   }
   else
    goto fail;
  }//endif create
  else
  {
   fail:
   htmldata->usevirtualscreen=0;
   allvirtual=0;
  }
//  kbhit();
  i=p->htmlframe[i].next;
 }//loop

#endif

  //turn on special XSWAP optimization for speed
  swapoptimize=1;

#ifdef HICOLOR
  if(xg_256!=MM_Hic && !cgamode && !ignoreimages)
#else
  if(!cgamode && !ignoreimages)
#endif
   MixVisiblePaletes(1-allvirtual);

#ifdef XANIMGIF
  XResetAnimGif();
#endif

#ifdef VIRT_SCR
  if(htmldata->usevirtualscreen)
   outs(MSG_RENDER);
  else
#endif
  if(!nomsg)
   outs(MSG_REDRAW);

 GLOBAL.activate_textarea=0;

#ifndef POSIX
 bigfonts_allowed();
#endif

 pushcurrent=p->currentframe;
 i=0;
 while(i<MAXFRAMES-1 && i>=0)
 {
  while(p->htmlframe[i].hidden && i<MAXFRAMES-1 && i!=-1)
  {
//   kbhit();
   i=p->htmlframe[i].next;
  }

  if(i>=MAXFRAMES-1 || i==-1)
   break;

  frame=&(p->htmlframe[i]);
  htmldata=&(p->tmpframedata[i]);
  p->currentframe=i;

#ifdef VIRT_SCR
  if(htmldata->usevirtualscreen)
  {
   xv_set_actvirt(htmldata->whichvirtual);
   x_video_XMS(1, 0);
  }
#endif

  //draw background
#ifdef HICOLOR
  if(xg_256 == MM_Hic)
  {
   if(htmldata->backR==255 && htmldata->backG==255 && htmldata->backB==255)
    htmldata->bgindex=15;
   else
   if(htmldata->backR==0 && htmldata->backG==0 && htmldata->backB==0)
    htmldata->bgindex=0;
   else
    goto calcbg;
  }
  else
#endif
  {
   calcbg:
   htmldata->bgindex=RGB(htmldata->backR, htmldata->backG,htmldata->backB);
  }
  html_background(i,frame->scroll.xtop,frame->scroll.ytop,
                    frame->scroll.xsize,frame->scroll.ysize);


  //redraw visible part of the document
  currentHTMLatom=p->firstHTMLatom;
  n=0l; //atom counter
  while(currentHTMLatom!=IE_NULL)
  {
   atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
   if(!atomptr)
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return;
//    MALLOCERR();
//!!glennmcc: end
   nextHTMLatom=atomptr->next;
   if(atomptr->frameID==i &&
      atomptr->yy>frame->posY &&
      atomptr->xx>frame->posX &&
      atomptr->y<frame->posY+frame->scroll.ysize &&
      atomptr->x<frame->posX+frame->scroll.xsize)
   {
    if(atomptr->type!=TABLE && atomptr->type!=TD && atomptr->type!=TD_BACKGROUND)
    {
     if(firstonscrn>n)
     {
      firstonscrn=n;
      p->firstonscr=currentHTMLatom;
     }
     if(lastonscrn<n)
     {
      lastonscrn=n;
      p->lastonscr=currentHTMLatom;
     }
    }
#ifdef VIRT_SCR
    if(xg_video_XMS)
    {
     if(atomptr->type==IMG || atomptr->type==TD_BACKGROUND)
      imgcount++;
     if(n%200==0 || !(imgcount%4))
     {
      int tyc=(int)(100l*n/p->HTMLatomcounter);
//      kbhit();
      if(tyc>=0 && tyc!=lasttyc)
      {
       x_video_XMS(0, 0);
       percentbar(tyc);
       lasttyc=tyc;
       x_video_XMS(1, 0);
      }
     }
    }
#endif

    drawatom(atomptr,frame->posX,frame->posY,
             frame->scroll.xsize,frame->scroll.ysize,
             frame->scroll.xtop,frame->scroll.ytop);

   }//endif visible
#ifndef TABLES
   else if (atomptr->y>frame->posY+2*frame->scroll.ysize) break;
#endif
   n++;

   currentHTMLatom=nextHTMLatom;
  }//loop

  if(neediknredraw)
   DrawIcons();
//   Drawikons(1);

#ifdef VIRT_SCR

  if(htmldata->usevirtualscreen)
  {
   frame->scroll.xsize=draw_x[i];
   frame->scroll.ysize=draw_y[i];
   frame->posX=from_x[i];
   frame->posY=from_y[i];
   frame->scroll.xtop=screen_x[i];
   frame->scroll.ytop=screen_y[i];
   if(GLOBAL.activate_textarea==1)
   {
    mousex+=frame->scroll.xtop;
    mousey+=frame->scroll.ytop;
    GLOBAL.activate_textarea=2;
   }
  }

  if(xg_video_XMS)
  {
   char *ptr=ie_getswap(virtualpalhandle);

   if(!ptr)
   {
    virtualpalhandle=ie_putswap(Iipal,768,CONTEXT_SYSTEM);
    ptr=ie_getswap(virtualpalhandle);
   }
   if(ptr)
   {
    memcpy(ptr,Iipal,768);
    swapmod=1;
    virtualIiNpal=IiNpal;
   }
   virtualfirstonscr=p->firstonscr;
   virtuallastonscr=p->lastonscr;

   x_video_XMS(0, 0);
   if(IiNpal>16)
    x_palett( IiNpal, Iipal);
   //show originaly requested portion of virtual screen:
   dumpvirtual(frame,htmldata,frame->posX,frame->posY);
  }
#endif

  if(frame->allowscrolling)
  {
   //space for scrollbar:
   x_setfill(0,0);
   if(!frame->scroll.onscrollx)
    x_bar(frame->scroll.xtop+frame->scroll.xsize+1,
          frame->scroll.ytop,
          frame->scroll.xtop+frame->scroll.xsize+user_interface.scrollbarsize,
	  frame->scroll.ytop+frame->scroll.ymax);
   ScrollButtons(&frame->scroll);
   ScrollDraw(&frame->scroll,frame->posX,frame->posY);
  }
  drawframeborder(i);

//  kbhit();
  i=p->htmlframe[i].next;
 }//loop
 p->currentframe=pushcurrent;

#ifndef POSIX
 bigfonts_forbidden();
#endif

 if(!nomsg)
  defaultmsg();

#ifdef VIRT_SCR
 } //end if "not from virtual screen"
#endif

 //turn on special XSWAP optimization for speed
 swapoptimize=0;

 activeatomptr=NULL; //there is nothing under mouse 
 htmlpulldown=0;
 p->restorehoveradr=IE_NULL;
 ie_killcontext(CONTEXT_TMPIMG);
 ie_clipstatus=0;
 activeistextwindow=0;
 meminfovisible=0;
 lastfound=IE_NULL;
 lastfoundY=0l;
#ifdef XANIMGIF
 XAnimateGifs();
#endif
 mouseon();
}

//===========================================================================


//--------------------------------------------------------------------------


void html_background(char whichframe,
                     int screen_x, int screen_y,
                     int draw_x, int draw_y)
{
 if(cgamode)
 {
  p->tmpframedata[whichframe].cgatext=15-p->tmpframedata[whichframe].bgindex;
 }
 else if(p->tmpframedata[whichframe].backgroundptr!=IE_NULL && !ignoreimages)
 {
  struct picinfo *background,*imgptr;
  struct HTTPrecord HTTPdoc;
  struct HTMLrecord *atomptr=NULL;

  if(p->tmpframedata[whichframe].backgroundptr!=IE_NULL)
   atomptr=(struct HTMLrecord *)ie_getswap(p->tmpframedata[whichframe].backgroundptr);
  if(atomptr)
  {
//   int frameID=atomptr->frameID;
   imgptr=(struct picinfo *)ie_getswap(atomptr->ptr);
   if(imgptr)
   {
    struct Url burl;
    XSWAP dummy1;
    unsigned dummy2;

    background=farmalloc(sizeof (struct picinfo));
    if(!background)
     memerr();

    memcpy(background,imgptr,sizeof(struct picinfo));

    AnalyseURL(background->URL,&burl,IGNORE_PARENT_FRAME);
    if(SearchInCache(&burl,&HTTPdoc,&dummy1,&dummy2))
    {
     strcpy(background->filename,HTTPdoc.locname);
     background->from_x=screen_x;
     background->from_y=screen_y;
     background->pic_x=screen_x;
     background->pic_y=screen_y;
     background->stop_x=screen_x+draw_x+2; //?!
     background->stop_y=screen_y+draw_y; //!!!
     background->palonly=0;
     background->sizeonly=0;
     background->IsInXms=0;
     background->is_background=1;
     background->bgindex=p->tmpframedata[whichframe].bgindex;
     background->draw_x=draw_x;
     background->draw_y=draw_y+1;
     background->screen_x=screen_x;
     background->screen_y=screen_y;
#ifdef HICOLOR
     if(xg_256 == MM_Hic) //HARO
     { background->palismap = 0;
     }
#endif

     if(drawanyimage(background)==1)
     {
      farfree(background);
      return;
     }
     else
      p->tmpframedata[whichframe].backgroundptr=IE_NULL;
    }
    farfree(background);
   }
  }
 }

 x_setfill(0,p->tmpframedata[whichframe].bgindex);
 x_bar(screen_x,screen_y,screen_x+draw_x,draw_y+screen_y);

}//end sub

void drawframeborder(char i)
{
 struct ScrollBar *scroll=&(p->htmlframe[i].scroll);

 if(i>0 && !p->htmlframe[i].hidden && arachne.framescount>0 && 
     p->htmlframe[i].frameborder)
 {
  int scrollbarsize=0;

  if(p->htmlframe[i].allowscrolling)
   scrollbarsize=user_interface.scrollbarsize;

  if(i==p->activeframe)
   x_setfill(0,8);
  else
   x_setfill(0,15);
  x_bar(scroll->xtop-1,
         scroll->ytop-1,
         scroll->xtop-1,
	 scroll->ytop+scroll->ymax+1);
  x_bar(scroll->xtop-1,
         scroll->ytop-1,
         scroll->xtop+scroll->xsize+scrollbarsize+1,
         scroll->ytop-1);

  if(i==p->activeframe)
   x_setfill(0,15);
  else
   x_setfill(0,8);
  x_bar(scroll->xtop+scroll->xsize+scrollbarsize+1,
         scroll->ytop-1,
         scroll->xtop+scroll->xsize+scrollbarsize+1,
	 scroll->ytop+scroll->ymax+1);
  x_bar(scroll->xtop-1,
         scroll->ytop+scroll->ymax+1,
         scroll->xtop+scroll->xsize+scrollbarsize+1,
         scroll->ytop+scroll->ymax+1);
 }
 p->oldactive=p->activeframe;
}

void drawactiveframe(void)
{
 if(p->oldactive!=p->activeframe)
 {
  drawframeborder(p->oldactive);	  // old active and active frame
  drawframeborder(p->activeframe);
 }
}

void will_activate_atom(int setx, int sety)
{
//!!Ray: Oct 22, 2007 -- guard against activated textarea being off-screen
 if(setx-2 > x_maxx() || sety-2 > x_maxy()) return;
//!!Ray: end
 mousex=setx-2;
 mousey=sety-2;
 GLOBAL.activate_textarea=1;
}

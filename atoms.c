
// ========================================================================
// Arachne "activeatom" and mouse interface
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "html.h"
#include "gui.h"

//Goto next/previous atom when Tab/Shift+tab is pressend
char gotonextlink( int *x, int *y , char back, char asc)
{
 int fx=0;
 long fy=-1;
 char frameID;
 char lastlink=0;
 char rv;
 unsigned currentHTMLatom,nextHTMLatom;
 struct HTMLrecord *atomptr;
 struct HTMLframe *frame;

 if(back)
  currentHTMLatom=p->lastonscr;
 else
  currentHTMLatom=p->firstonscr;

 while(currentHTMLatom!=IE_NULL)
 {
  atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if(!atomptr)
   MALLOCERR();

  if(back) //Shift+Tab
  {
   if(currentHTMLatom==p->firstonscr)
    nextHTMLatom=IE_NULL;
   else
    nextHTMLatom=atomptr->prev;
  }
  else //Tab
  {
   if(currentHTMLatom==p->lastonscr)
    nextHTMLatom=IE_NULL;
   else
    nextHTMLatom=atomptr->next;
  }//endif

  frame=&(p->htmlframe[atomptr->frameID]);

  if(atomptr->linkptr!=IE_NULL &&
     atomptr->x>frame->posX &&
     atomptr->y>frame->posY &&
     atomptr->xx<frame->posX+frame->scroll.xsize &&
     atomptr->yy<frame->posY+frame->scroll.ysize &&
     !(atomptr->type==TABLE || atomptr->type==TD || atomptr->type==TD_BACKGROUND ||
       atomptr->type==INPUT && atomptr->data1==HIDDEN))
  {
   if(fy<0 ||
     (lastlink &&
      (atomptr->linkptr!=lastonmouse || atomptr->type==INPUT) &&
     (asc!=13 || atomptr->type==INPUT &&
      (atomptr->data1==TEXT || atomptr->data1==PASSWORD || atomptr->data1==TEXTAREA)
     )))
   {
    char flag=0;
    if(fy>=0)flag=1;

    fx=atomptr->xx;
    fy=atomptr->yy;
    frameID=atomptr->frameID;

    if(atomptr->type==INPUT && (atomptr->data1==TEXT ||
       atomptr->data1==PASSWORD || atomptr->data1==TEXTAREA))
     rv=1;
    else
     rv=0;
    if(flag)
     goto ret;
   }

   if(currentHTMLatom==activeadr)
    lastlink=1;

  }//endif it is link

  currentHTMLatom=nextHTMLatom;
 }//loop

 ret:
 if(fy>=0)
 {
  *x=(int)(fx-2-p->htmlframe[frameID].posX+p->htmlframe[frameID].scroll.xtop);
  *y=(int)(fy-2-p->htmlframe[frameID].posY+p->htmlframe[frameID].scroll.ytop);
 }
 return rv;
}

// HTML code search for that last input inside <FORM> tag
char *activeislastinput(void)
{
 struct HTMLrecord *atomptr=(struct HTMLrecord*)ie_getswap(activeadr);	// active atom

 if(atomptr && atomptr->type==INPUT && (atomptr->data1==TEXT || atomptr->data1==PASSWORD))
 {
  unsigned submitlink=atomptr->linkptr;
  char found=0;
  unsigned currentHTMLatom=p->firstonscr;

  while(currentHTMLatom!=IE_NULL)
  {
//   kbhit();
   atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
   if(!atomptr)
    MALLOCERR();
   if(found)
   {
    if(atomptr->linkptr==submitlink && atomptr->type==INPUT &&
       atomptr->data1!=SUBMIT && atomptr->data1!=HIDDEN &&
       atomptr->data1!=RADIO && atomptr->data1!=CHECKBOX)
     return NULL;
   }
   else if(currentHTMLatom==activeadr)
    found=1;
   currentHTMLatom=atomptr->next;
  }//loop
  atomptr=(struct HTMLrecord *)ie_getswap(submitlink);
  if(atomptr && atomptr->type==FORM)
  {
   GLOBAL.postdata=atomptr->data2+1; //GET==1, POST==2
   arachne.target=atomptr->data1;
   return(char *)ie_getswap(atomptr->ptr);
  }//endif
 }//endif
 return NULL;
}

void ReadFileBox(void)
{
   setTXTprompt("TEXTAREA.TXT");
   inputatom(MSG_READF,MSG_EFNAME);
   activeistextwindow=INPUT_READFILE;
}

void WriteFileBox(void)
{
   setTXTprompt("TEXTAREA.TXT");
   inputatom(MSG_WRITEF,MSG_EFNAME);
   activeistextwindow=INPUT_WRITEFILE;
}

void SearchInTextBox(void)
{
//   setTXTprompt("TEXTAREA.TXT");
   inputatom(MSG_SRCHTX,MSG_ENTER);
   activeistextwindow=INPUT_SEARCHINTEXT;
}

#define BOXTEXT 3,BOLD
#define BOXBORDER 5

void inputatom(char *msg1,char *msg2)
{
 activeatomcursor(0);
 activeatomptr=&TXTprompt;

 mouseoff();
 Box3D(TXTprompt.x-BOXBORDER,p->htscrn_ytop+(int)(TXTprompt.y-fonty(BOXTEXT)-BOXBORDER),
       TXTprompt.xx+BOXBORDER,p->htscrn_ytop+(int)(TXTprompt.yy+BOXBORDER-2));
 x_setcolor(0);
 htmlfont(BOXTEXT);
 {
  int colormap[5]={8,-1,-1,15,7};
  decorated_text(TXTprompt.x,p->htscrn_ytop+(int)(TXTprompt.y-fonty(BOXTEXT)-2),msg1,colormap);
 }

 activeatomredraw();
 activeatomcursor(1);
 outs(msg2);
 mouseon();
}

// Coorperative multitaskng.  If an atom is active, GUI event should also check this atom
int activeatomtick(int key,char textareacmd)
{
 int redraw;

 if(activeatomptr && activeatomptr->type==INPUT)
 {
  int x1,x2,xs,ys,asc,oldline;
  long y1,y2;
  struct HTMLframe *frame=&(p->htmlframe[activeatomptr->frameID]);

  x2=activeatomptr->xx;
  x1=activeatomptr->x;
  y2=activeatomptr->yy;
  y1=activeatomptr->y;
  if(activeatomptr!=&URLprompt && activeatomptr!=&TXTprompt)

  {
   x1-=frame->posX;
   y1-=frame->posY;
   x2-=frame->posX;
   y2-=frame->posY;

   if(x2>frame->scroll.xsize && activeatomptr!=&URLprompt)
    x2=frame->scroll.xsize;
   if(y2>frame->scroll.ysize)
    y2=frame->scroll.ysize;
   if(x1<0)x1=0;
   if(y1<0 && activeatomptr!=&URLprompt)y1=0;

   if(activeatomptr->data1==TEXTAREA)
   {
    x2-=user_interface.scrollbarsize;
    y2-=user_interface.scrollbarsize;
   }
  }//endif "not URL/textinput" special case

  x2+=frame->scroll.xtop;
  x1+=frame->scroll.xtop;
  y2+=frame->scroll.ytop;
  y1+=frame->scroll.ytop;

  xs=(x2-x1-5)/fontx(SYSFONT,0,' ')-1;
  if(xs<1)xs=1;
  ys=(int)(y2-y1-4)/fonty(SYSFONT,0)-1;
  if(ys<1)ys=1;

  asc=key & 0xFF;
  if(asc>127 && user_interface.keymap)
  {
   int i=0;
   while (i<96)
   {
    if(user_interface.keymap[i]==asc)
    {
     asc=key=161+i;
     break;
    }
    i++;
   }
  }

  editorptr=(struct ib_editor *)ie_getswap(activeatomptr->ptr);
  if(editorptr)
  {
   memcpy(&tmpeditor,editorptr,sizeof(struct ib_editor));
   oldline=tmpeditor.y;
   redraw=ie_key(&tmpeditor,key,bioskey(2),xs,ys);
   //vraci 0:nic se nedeje,
   //      1:nastav kurzor
   //      2:prekresli radku
   //      3:prekresli vsechno
   editorptr=(struct ib_editor *)ie_getswap(activeatomptr->ptr);
   if(editorptr)
   {
    memcpy(editorptr,&tmpeditor,sizeof(struct ib_editor));
    swapmod=1;
   }
   else
    MALLOCERR();
  }
  else
   MALLOCERR();

  if(tmpeditor.maxlines>1) //multiline textarea... -> handle special events
  {
   switch(redraw)
   {
    case 103:ReadFileBox();return 0;
    case 123:WriteFileBox();return 0;
    case 107:SearchInTextBox();return 0;
    case 127:SearchInTextarea(1);return 0;
    case 301:outs(MSG_BLCKWR);return 0;
   }
  }//end if special event ocured (Arachne GUI bindings...)

  if(asc==27) //esc - opustit aktualni atom
  {
   activeatomcursor(0);
   activeatomptr=NULL;
   defaultmsg();
   if(activeistextwindow) //!! hrruuuza :-(
    return key;
  }
  else
  if(asc==13 && activeatomptr->type==INPUT && activeatomptr->data1!=TEXTAREA ||
     asc==9 || key==0xf00 || key==0x3c00 ||
    (activeatomptr->type==INPUT && tmpeditor.maxlines==1 &&
    (key==0x4800 || key==0x5000) && scrolllock()) )
     //enter,tab,shit+tab,F2, lynx mode
  {
   activeatomcursor(0);
   if(activeatomptr!=&URLprompt && activeatomptr!=&TXTprompt)
    activeatomptr=NULL;
   return key;
  }
  else if(redraw || textareacmd)
  {
   if(activeatomptr->data1!=TEXTAREA)
   {
    /* enable blocks ?
    if(activeatomptr->data1==TEXT)
     ie_redrawline(&tmpeditor,
                   x1+2,(int)y1+2,tmpeditor.zoomx, xs,tmpeditor.y);
    else
    */
     activeatomredraw();
   }
   else
   {
    if(textareacmd==TEXTAREA_NOREFRESH)  //hack for smooth screen refreshing..
    {
     return 0;
    }
    else
    if(redraw==3 || textareacmd) //redraw ALL atom and/or init scrollbar
    {
     mouseoff();
     if(textareacmd!=TEXTAREA_INIT)
     {
      if(ie_redrawwin(&tmpeditor,(int)(x1+2),(int)(y1+2),
                                 (int)(x2-2),(int)(y2-2),1))
      {
       mouseon();
       return 0; //(key was pressed and put into bios buffer)
      }
      atomneedredraw=0;
     }

     if(textareacmd!=TEXTAREA_SCROLL)
     {
      char oldonx=activescroll.onscrollx;
      char oldony=activescroll.onscrolly;
      activescroll.xvisible=1;
      activescroll.yvisible=1;
      ScrollInit(&activescroll,(int)(x2-x1-5),(int)(y2-y1-5),
                 (int)(y2-y1-5+user_interface.scrollbarsize),
                 (int)(x1+2),(int)(y1+2),
                tmpeditor.cols*fontx(SYSFONT,0,' '),(long)tmpeditor.lines*fonty(SYSFONT,0));
      activescroll.onscrollx=oldonx;
      activescroll.onscrolly=oldony;
      ScrollDraw(&activescroll,tmpeditor.zoomx*fontx(SYSFONT,0,' '),(long)tmpeditor.zoomy*fonty(SYSFONT,0));
     }
    }//end forced redraw (level 3)
    else //only last and current line redraw...
    {
     mouseoff();

     if(oldline!=tmpeditor.y)
      ie_redrawline(&tmpeditor,
                    x1+2,(int)y1+2+(oldline-tmpeditor.zoomy)*fonty(SYSFONT,0),
                    tmpeditor.zoomx, xs+1,oldline);

     if(bioskey(1))
     {
      atomneedredraw=1;
      return 0;
     }

     //redraw line where cursor IS
     ie_redrawline(&tmpeditor,
                   x1+2,(int)y1+2+(tmpeditor.y-tmpeditor.zoomy)*fonty(SYSFONT,0),
                   tmpeditor.zoomx, xs+1,tmpeditor.y);

    }//end if forced redraw (level 1)

    if(bioskey(1))
    {
     atomneedredraw=1;
     return 0;
    }

    if(tmpeditor.blockflag & 1)
     outs(MSG_BLOCK);
    else
    {
     char str[80];
//     sprintf(str,MSG_LINECOL,tmpeditor.y+1,tmpeditor.lines,tmpeditor.x+1);
     sprintf(str,"%04d/%04d:%03d",tmpeditor.y+1,tmpeditor.lines,tmpeditor.x+1);
     if(tmpeditor.modified)
      strcat(str," <mod>");
     if(tmpeditor.wordwrap)
      strcat(str," <wrap>");
     if(tmpeditor.insert)
      strcat(str," <ins>");
     if(tmpeditor.blockflag & 2)
      strcat(str," <block>");
     outs(str);
    }
   }//endif handle textarea
   activeatomcursor(1); //draw cursor
  }//end if redraw

  key=0;
 }//endif tick
 return key;
}

char *try2getURL(void)
{
 if(activeatomptr && activeatomptr==&URLprompt && activeatomptr->type==INPUT)
 {
  editorptr=(struct ib_editor *)ie_getswap(activeatomptr->ptr);

  if(editorptr)
  {
   memcpy(&tmpeditor,editorptr,sizeof(struct ib_editor));
   return ie_getline(&tmpeditor,0);
  }
  else
   MALLOCERR();
 }
 return NULL;
}

void activeurl(char *url)
{
 activeatomcursor(0);

 if(fullscreen || customerscreen)
 {
  setTXTprompt(url);
  inputatom("URL","");
  activeistextwindow=INPUT_URL;
 }
 else
 {
  toolbar(0,1);
  activeatomptr=&URLprompt;
  if(url[0])
   SetInputAtom(&URLprompt,url);
  activeatomredraw();
  activeatomcursor(1);
  outs(MSG_URL);
 }
}

int activeismap(int *dx, int *dy) //je aktivnim obrazkem klikatelna mapa ?
{
 //data1 | 1=ISMAP,2=USEMAP,4=INPUT TYPE=...
 if(activeatomptr && activeatomptr->type==IMG &&
    activeatomptr->data1 && activeatomptr->data1!=2)
 {
  *dx=(int)(activeatomptr->x+activeatomptr->data2+
       p->htmlframe[activeatomptr->frameID].scroll.xtop-p->htmlframe[activeatomptr->frameID].posX);
  *dy=(int)(activeatomptr->y+activeatomptr->data2+
       p->htmlframe[activeatomptr->frameID].scroll.ytop-p->htmlframe[activeatomptr->frameID].posY);
  return activeatomptr->data1;
 }
 else
  return 0;

}

//. Radio is like check box.   Only one of radios is checked at any moment
void RadioSwitch(int fromx, long fromy,XSWAP current,XSWAP formptr)
{
 char name[80];
 XSWAP currentHTMLatom=p->firstHTMLatom,nextHTMLatom;
 struct ScrollBar *scroll;
 struct HTMLrecord *atomptr;

 //vyresetovat rediobuttons a vykreslit ty, co jsou na obrazovce
 editorptr=(struct ib_editor *)ie_getswap(current);
 if(!editorptr)
  return;
 else
  strcpy(name,editorptr->filename);

 mouseoff();
 while(currentHTMLatom!=IE_NULL)
 {
//  kbhit();
  atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if(!atomptr)
   MALLOCERR();
  nextHTMLatom=atomptr->next;
  if(atomptr->type==INPUT && atomptr->data1==RADIO && atomptr->linkptr==formptr)
  {
   editorptr=(struct ib_editor *)ie_getswap(atomptr->ptr);
   if(editorptr && !strcmpi(editorptr->filename,name))
   {
    atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
    if(!atomptr)
     MALLOCERR();

    if(atomptr->ptr==current)
     atomptr->data2=1; //!je checked!
    else
     atomptr->data2=0; //!neni checked!
    swapmod=1;        //!ulozit!

    scroll=&(p->htmlframe[atomptr->frameID].scroll);
    if(atomptr->y>=fromy && atomptr->y<fromy+scroll->ysize &&
       atomptr->x>=fromx && atomptr->x<fromy+scroll->xsize)
    {
     drawatom(atomptr,fromx,fromy,
              scroll->xsize,scroll->ysize,scroll->xtop,scroll->ytop);
    }
   }//endif jetoeditor
  }//endif je to RADIO
  currentHTMLatom=nextHTMLatom;
 }//loop
 mouseon();
}

void HideLink(char *hideURL)
{
 XSWAP currentHTMLatom=p->firstHTMLatom,nextHTMLatom;
 XSWAP hidethis=IE_NULL;
 struct HTMLrecord *atomptr;

 mouseoff();
 while(currentHTMLatom!=IE_NULL)
 {
//  kbhit();
  atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if(!atomptr)
   MALLOCERR();
  nextHTMLatom=atomptr->next;
  if(atomptr->type==HREF)
  {
   char *href=ie_getswap(atomptr->ptr);
   if(href && !strcmpi(href,hideURL))
    hidethis=currentHTMLatom;
  }
  else
  if(hidethis!=IE_NULL && atomptr->linkptr==hidethis)
  {
   atomptr->type=EMPTY;
   swapmod=1;
  }

  currentHTMLatom=nextHTMLatom;
 }//loop
}

int ProcessLinks(char generateASF)
{
 struct Url url;
 char frameID;
 XSWAP currentHTMLatom=p->firstHTMLatom,nextHTMLatom;
 XSWAP hidethis=IE_NULL;
 unsigned dummy;
 int count=0,asf;
 struct HTMLrecord *atomptr;
 struct HTTPrecord HTTPdoc;
 char oldhref[URLSIZE] = "\0";

 mouseoff();

 if(generateASF)
 {
  char fname[80];
  tempinit(fname);
  strcat(fname,"$tmp$.asf");
  asf=a_open(fname,O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
  if(asf>=0)
   sprintf(GLOBAL.location,"file:%s",fname);
  else
   return 0;
 }

 while(currentHTMLatom!=IE_NULL)
 {
//  kbhit();
  atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if(!atomptr)
   MALLOCERR();
  nextHTMLatom=atomptr->next;
  frameID=atomptr->frameID;
  if(atomptr->type==HREF)
  {
   char *href=ie_getswap(atomptr->ptr);

   if(generateASF)
   {
    char *str;
    str = strrchr(href, '#');
    if(str)
     href[(int)(str - href)] = '\0';
    strcat(href, "\n");
    if(!strncmpi(href, "mailto:", 6))
     continue;
    if(strcmp(oldhref, href) != 0)
     write(asf,href,strlen(href));
    strcpy(oldhref, href);
   }
   else
   {
    AnalyseURL(href,&url,frameID);
    if(SearchInCache(&url,&HTTPdoc,&dummy,&dummy) && file_exists(HTTPdoc.locname))
     hidethis=currentHTMLatom;
    else
    {
     count++;
     hidethis=IE_NULL;
    }
   }
  }
  else
  if(hidethis!=IE_NULL && atomptr->linkptr==hidethis)
  {
   atomptr->R=p->tmpframedata[frameID].textR;
   atomptr->G=p->tmpframedata[frameID].textG;
   atomptr->B=p->tmpframedata[frameID].textB;
   swapmod=1;
  }

  currentHTMLatom=nextHTMLatom;
 }//loop

 if(generateASF)
 {
  write(asf,p->htmlframe[0].cacheitem.URL,strlen(p->htmlframe[0].cacheitem.URL));
  write(asf,"\n",1);
  a_close(asf);
  return 1;
 }


 redrawHTML(REDRAW_WITH_MESSAGE,REDRAW_CREATE_VIRTUAL);
 return count;
}

void activeatomredraw(void)
{
 if(activeatomptr)
 {
  struct HTMLframe *frame=&(p->htmlframe[activeatomptr->frameID]);

  mouseoff();
  if(activeatomptr==&URLprompt || activeatomptr==&TXTprompt)
   drawatom(activeatomptr,0u,0,
            frame->scroll.xsize,
            frame->scroll.ysize,
            frame->scroll.xtop,
            frame->scroll.ytop);
  else
   drawatom(activeatomptr,
            frame->posX,
            frame->posY,
            frame->scroll.xsize,
            frame->scroll.ysize,
            frame->scroll.xtop,
            frame->scroll.ytop);
  mouseon();
 }
}

void activeatomcursor(char cursor)
{
 //have to be optimized!!!

 if(htmlpulldown || (activeistextwindow && activeatomptr!=&TXTprompt))
 {
  if(htmlpulldown && htmlsavex!=-1)
  {
   ImouseSet(htmlsavex,htmlsavey);
   hidehighlight();
  }

  if(arachne.framescount)
  {
   p->activeframe=0;
   redrawHTML(REDRAW_NO_MESSAGE,REDRAW_CREATE_VIRTUAL);
  }
  else
   redrawHTML(REDRAW_WITH_MESSAGE,REDRAW_VIRTUAL);
 }
 if(activeatomptr && activeatomptr->type==INPUT)
 {
  char subtype=activeatomptr->data1;
  long xc,yc;
  char zn[2]=" \0";
  char *ptr;
  int ink,paper;
  struct HTMLframe *frame=&(p->htmlframe[activeatomptr->frameID]);

  if(subtype==SELECT)
  {
   activeatomredraw();
   return;
  }

  editorptr=(struct ib_editor *)ie_getswap(activeatomptr->ptr);
  if(editorptr)
  {
   memcpy(&tmpeditor,editorptr,sizeof(struct ib_editor));

   ptr=ie_getline(&tmpeditor,tmpeditor.y);
   if(ptr && strlen(ptr)>tmpeditor.x)
   {
    if(subtype==PASSWORD)
     zn[0]='*';
    else
     zn[0]=ptr[tmpeditor.x];
   }

   xc=frame->scroll.xtop+activeatomptr->x+2;
   yc=frame->scroll.ytop+activeatomptr->y+2;

   if(activeatomptr!=&URLprompt && activeatomptr!=&TXTprompt)
   {
    xc-=frame->posX;
    yc-=frame->posY;
    if(xc<frame->scroll.xtop+2)
     xc=frame->scroll.xtop+2;
    if(yc<frame->scroll.ytop+2)
     yc=frame->scroll.ytop+2;
   }

   xc+=fontx(SYSFONT,0,zn[0])*(tmpeditor.x-tmpeditor.zoomx);
   yc+=fonty(SYSFONT,0)*(tmpeditor.y-tmpeditor.zoomy);

   if(xc>=frame->scroll.xtop &&
      xc<frame->scroll.xtop+frame->scroll.xsize &&
      yc>=frame->scroll.ytop && yc<frame->scroll.ytop+frame->scroll.ysize ||
      activeatomptr==&URLprompt || activeatomptr==&TXTprompt)
   {
    mouseoff();

    if(tmpeditor.y<tmpeditor.bby || tmpeditor.y>tmpeditor.bey ||
       tmpeditor.y==tmpeditor.bby && tmpeditor.x<tmpeditor.bbx ||
       tmpeditor.y==tmpeditor.bey && tmpeditor.x>=tmpeditor.bex ||
       tmpeditor.blockflag!=2)
    {
     ink=user_interface.ink;
     paper=user_interface.paper;
    }
    else
    {
     ink=BLOCK_INK;
     paper=BLOCK_PAPER;
    }

    if(cursor)
     x_setfill(0,ink); //green
    else
     x_setfill(0,paper); //black

    x_bar((int)xc,(int)yc,(int)xc+fontx(SYSFONT,0,zn[0])-1,(int)yc+fonty(SYSFONT,0)-2);

    if(cursor)
     x_setcolor(paper);
    else
     x_setcolor(ink);

    htmlfont(SYSFONT,0);
    if(fonty(SYSFONT,0)!=14)
     yc-=1;
    x_text_ib((int)xc,(int)yc,(unsigned char *)zn);
   }
   mouseon();
#ifdef GGI
  Smart_ggiFlush();
#endif 
  }
  else
   MALLOCERR();
 }
}

void MakeInputAtom(struct HTMLrecord *atom,struct ib_editor *tmpeditor,int x,long y,int xx,long yy)
{
 atom->x=x;
 atom->y=y;
 atom->xx=xx;
 atom->yy=yy;
 atom->type=INPUT;
 atom->data1=TEXT;
 atom->data2=0;
 atom->ptr=ie_putswap((char *)tmpeditor,sizeof(struct ib_editor),CONTEXT_SYSTEM);
 atom->datalen=sizeof(struct ib_editor);
 atom->linkptr=IE_NULL;
}

void SetInputAtom(struct HTMLrecord *atom,char *str)
{
 if(atom->type!=INPUT)
  return;


 editorptr=(struct ib_editor *)ie_getswap(atom->ptr);
 if(editorptr)
 {
  editorptr->aktrad=0;
  strcpy(editorptr->rad,str);
  editorptr->x=strlen(str);
  if((editorptr->x+1)*fontx(SYSFONT,0,' ')>atom->xx-atom->x-4)
   editorptr->x=0;
  editorptr->zoomx=0;
  swapmod=1;
  /*
  nonsense ?!

  memcpy(&tmpeditor,editorptr,sizeof(struct ib_editor));
  ie_putline(&tmpeditor,0,str);
  swapmod=1;
  */
 }
 else
  MALLOCERR();
}

char *gotoactiveatom(char asc, XSWAP *formID)
{
 struct HTMLrecord *atomptr=(struct HTMLrecord*)ie_getswap(activeadr);

 if(atomptr && atomptr->type==INPUT &&
   (atomptr->data1==TEXT || atomptr->data1==PASSWORD ||
    atomptr->data1==TEXTAREA && asc!=13))
 {
  *formID=atomptr->linkptr;
  atomptr=(struct HTMLrecord *)ie_getswap(*formID);
  if(atomptr && atomptr->type==FORM)
  {
   GLOBAL.postdata=atomptr->data2+1;
   arachne.target=atomptr->data1;
   return(char *)ie_getswap(atomptr->ptr);
  }//endif
 }
 *formID=IE_NULL;
 return NULL;
}

//---------------------------- <SELECT> ---------------------------- begin

// Pull down select
void SelectSwitch(int x,long y,int key) //x coord will be maybe used in future
{
 int selected;
 long selecty=activeatom.y;
 int fromx=p->htmlframe[activeatom.frameID].posX;
 long fromy=p->htmlframe[activeatom.frameID].posY;

 if(selecty<fromy)
  selecty=fromy;
 selected=(int)(y-selecty-1)/fonty(OPTIONFONT,0)*2;
 editorptr=(struct ib_editor *)ie_getswap(activeatom.ptr);
 if(editorptr)
 {
  char *ptr;
  char singleline=0;
  int rows;
  long realy=activeatom.y;
  long realyy=activeatom.yy;
  struct ScrollBar *scroll=&(p->htmlframe[activeatom.frameID].scroll);
  int onscreen_x=(int)(activeatom.x-fromx+scroll->xtop);
  int onscreen_xx=(int)(activeatom.xx-fromx+scroll->xtop);

  if(realy-fromy+scroll->ytop<p->htscrn_ytop)
   realy=scroll->ytop-p->htscrn_ytop+fromy;
  if(realyy-fromy+scroll->ytop>p->htscrn_ytop+p->htscrn_ysize)
   realyy=p->htscrn_ytop+p->htscrn_ysize-scroll->ytop+fromy;

  rows=2*(int)((realyy-realy-4)/fonty(OPTIONFONT,0));
  if(selected>rows)
   selected=rows;

  selected+=editorptr->zoomy;

  if(activeatom.yy-activeatom.y<2*fonty(OPTIONFONT,0))
  {
   singleline=1;
   if(key)
    return;
  }

  if((editorptr->lines>=rows) && !singleline || key)
  {
   int sw=0;
   if(key==0 && user_interface.scrollbarsize>3)
   {
    struct ScrollBar tmpscroll;
    char resetstyle=0;

    if(!user_interface.scrollbarstyle)
    {
     user_interface.scrollbarstyle='N';
     resetstyle=1;
    }
    tmpscroll.xvisible=0;
    tmpscroll.yvisible=1;
    ScrollInit(&tmpscroll,
               (int)(activeatom.xx-activeatom.x-user_interface.scrollbarsize-6),
               (int)(realyy-realy-8),
               (int)(realyy-realy-8),
               onscreen_x+3,
               (int)(realy+3-fromy+scroll->ytop),1,1);

    sw=OnScrollButtons(&tmpscroll);
    if(resetstyle)
     user_interface.scrollbarstyle=0;

   }

   if(sw || key)
   {
    if(sw==2 || key==PAGEDOWN || (key==DOWNARROW && editorptr->x==editorptr->zoomy+rows-2))
    {
     if(rows+editorptr->zoomy<editorptr->lines)
            editorptr->zoomy+=rows;                   //down
      else
       editorptr->x=editorptr->zoomy+rows-2;
     if(key==DOWNARROW)
      key=PAGEDOWN; //not UPARROW  || DOWNARROW
    }
    else
    if(sw==1 || key==PAGEUP || (key==UPARROW && editorptr->x==editorptr->zoomy))
    {
     editorptr->zoomy-=rows;                          //up
     if(key==UPARROW)
      key=PAGEUP; //not UPARROW  || DOWNARROW
    }
    else
    if(key==ENDKEY)
    {
     editorptr->x=editorptr->zoomy+rows-2;
    }
    else
    if(key==HOMEKEY)
    {
     editorptr->x=editorptr->zoomy;
    }

    if(editorptr->zoomy<0)
    {
     editorptr->zoomy=0;
     editorptr->x=0;
    }

    swapmod=1; //editorptr was updated.

    if(key)
    {
     if(key==CURSOR_SYNCHRO)
     {
      if(editorptr->x==selected)
       return;
      else if(x>onscreen_xx-user_interface.scrollbarsize-4)
      {
       hidehighlight();
       return;
      }
      else
       editorptr->x=selected;
     }
     if(key==DOWNARROW && htmlsavex!=-1)
      editorptr->x+=2;
     else
     if(key==UPARROW)
      editorptr->x-=2;

     if(editorptr->x<editorptr->zoomy)
      editorptr->x=editorptr->zoomy;

     if(editorptr->x>editorptr->zoomy+rows-2)
      editorptr->x=editorptr->zoomy+rows-2;

     if(editorptr->x>editorptr->lines-2)
      editorptr->x=editorptr->lines-2;

     if(key!=CURSOR_SYNCHRO) // mouse event, not key event...
     {
      if(htmlsavex==-1)
      {
       htmlsavex=mousex;
       htmlsavey=mousey;
      }

      mousey=(int)(realy-fromy+scroll->ytop)+
              fonty(OPTIONFONT,0)*(editorptr->x-editorptr->zoomy+2)/2;
      mousex=onscreen_xx-user_interface.scrollbarsize-4;
      ImouseSet( mousex, mousey);
     }

     //nice highlight...
     hidehighlight();
     thisx=onscreen_x+3;
     thisxx=onscreen_xx-user_interface.scrollbarsize-1;
     thisy=(int)(realy-fromy+scroll->ytop)+
             fonty(OPTIONFONT,0)*(editorptr->x-editorptr->zoomy)/2+3;
     thisyy=thisy+fonty(OPTIONFONT,0)+2;

     if(key!=PAGEUP && key!=PAGEDOWN)
     {
      showhighlight();
      return;
     }
    }

    mouseoff();
    drawatom(&activeatom,fromx,fromy,
              p->htscrn_xsize+p->htscrn_xtop-scroll->xtop,
              p->htscrn_ysize+p->htscrn_ytop-scroll->ytop, 
              //select window can overwrite other
              scroll->xtop,scroll->ytop);  //frames...
    mouseon();
    if(key)
     showhighlight();
    return;
   }
  }//endif ?scroll?

  if(!singleline && x>onscreen_xx-user_interface.scrollbarsize-4)
  {
   activeatomcursor(0);
   return;
  }

  if(selected<0 || selected>=editorptr->lines)
   return;

  editorptr=(struct ib_editor *)ie_getswap(activeatom.ptr);
  if(!editorptr)
   MALLOCERR();

  memcpy(&tmpeditor,editorptr,sizeof(struct ib_editor));
  if(!activeatom.data2) //not multiple
  {
   tmpeditor.y=0;
   while(tmpeditor.y<tmpeditor.lines)
   {
    ptr=ie_getline(&tmpeditor,tmpeditor.y);
    if(ptr)
    {
     *ptr='0';
     swapmod=1;
    }
    tmpeditor.y+=2; //odd lines are "description"
   }
  }

  tmpeditor.y=selected;
  ptr=ie_getline(&tmpeditor,selected);
  if(activeatom.data2 && *ptr=='1') //multiple and selected ?
   *ptr='0';
  else
   *ptr='1';
  swapmod=1;

  editorptr=(struct ib_editor *)ie_getswap(activeatom.ptr);
  if(editorptr)
  {
   memcpy(editorptr,&tmpeditor,sizeof(struct ib_editor));
   swapmod=1;
   if(!activeatom.data2) //not multiple ? close pulldown!
    activeatomcursor(0);
   else
   {
    mouseoff();
    drawatom(&activeatom,fromx,fromy,
              p->htscrn_xsize+p->htscrn_xtop-scroll->xtop,
              p->htscrn_ysize+p->htscrn_ytop-scroll->ytop, 
              scroll->xtop,scroll->ytop); 
    mouseon();
   }
   return;
  }
 }
 MALLOCERR();
}

//---------------------------- <SELECT> ---------------------------- end

// There may be more than one scroll bars on the screen, but only one is active.  This active SC bar 
// should be checked as a GUI event
char activeatomScrollBarTICK(void)
{
 if(activeatomptr && activeatomptr->type==INPUT && activeatomptr->data1==TEXTAREA)
 {
  int X;
  long Y;

  editorptr=(struct ib_editor *)ie_getswap(activeatomptr->ptr);
  if(!editorptr)
   MALLOCERR();

  X=editorptr->zoomx*fontx(SYSFONT,0,' ');
  Y=(long)editorptr->zoomy*fonty(SYSFONT,0);

  if(ScrollBarTICK(&activescroll,&X,&Y))
  {
   mouseoff();
   ScrollDraw(&activescroll,X,Y);
   editorptr->zoomx=(X+fontx(SYSFONT,0,' ')/2)/fontx(SYSFONT,0,' ');
   editorptr->zoomy=(int)((Y+fonty(SYSFONT,0)/2)/fonty(SYSFONT,0));
   swapmod=1;
   atomneedredraw=2;
   return 1;
  }
 }
 return 0;
}

void activeatomScrollBarBUTTONS(void)
{
 if(activeatomptr && activeatomptr->type==INPUT && activeatomptr->data1==TEXTAREA)
 {
  int s=OnScrollButtons(&activescroll);

  if(!s)
   s=OnBlackZone(&activescroll);

  switch(s)
  {
   case 1:activeatomtick(PAGEUP,0);
          break;
   case 2:activeatomtick(PAGEDOWN,0);
          break;
   case 3:activeatomtick(CTRLHOME,0);
          break;
   case 4:activeatomtick(CTRLEND,0);
          break;

  }
 }
}


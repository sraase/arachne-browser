
// ========================================================================
// GRAPHICAL USER INTERFACE for Arachne WWW browser - runtime functions
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "gui.h"
#include "customer.h"

//------------------------------------------------------------------------
//real events, called from GUIEVENT
//------------------------------------------------------------------------

int gotoloc(void)
{
  if(GLOBAL.location[0])
  {
   GLOBAL.gotolocation=1;
   GLOBAL.isimage=0;
   return 1;
  }
  else
   return 0;
}

int gotohotlist(void)
{
  char *ptr; 
  strcpy(GLOBAL.location,"file:");
  ptr=configvariable(&ARACHNEcfg,"Hotlist",NULL);
  if(ptr)
   strcat(GLOBAL.location,ptr);
  else
   strcat(GLOBAL.location,hotlist);
  arachne.target=0;
  return gotoloc();
}

int gotohistory(void)
{
 strcpy(GLOBAL.location,"arachne:history");
 GLOBAL.reload=RELOAD_NEW_LOCATION;
 arachne.target=0;
 return gotoloc();
}

int gotodialpage(void)
{
 sprintf(GLOBAL.location,"arachne:dialpage");
 arachne.target=0;
 return gotoloc();
}

int add2hotlist(void)
{
 char *urlptr=NULL,*titleptr;
 char str[IE_MAXLEN+1];
 char line[IE_MAXLEN+2]="mailto:",*ptr,*tgtstr=line;

 if(GLOBAL.clipdel==CLIPBOARD_ADDHOT)
 {
  struct ib_editor clipboard;

  strcpy(clipboard.filename,CLIPBOARDNAME);

  if(ie_openf_lim(&clipboard,CONTEXT_TMP,1000)==1) //ok
  {
   ptr=ie_getline(&clipboard,0);
   if(ptr)
   {
    if(strchr(ptr,'@') && !strchr(ptr,':'))
     tgtstr+=7;
    makestr(tgtstr,ptr,IE_MAXLEN);
    urlptr=line;
    titleptr=line;
   }
  }
// ie_closef(&clipboard);
  ie_killcontext(CONTEXT_TMP);
  if(!urlptr)
   return 0;
 }
 else
 {
  urlptr=p->htmlframe[0].cacheitem.URL;
  titleptr=arachne.title;
 }

 if(activeistextwindow!=INPUT_ADDHOT &&
    (user_interface.edithotlistentry || GLOBAL.clipdel==CLIPBOARD_ADDHOT))
 {
  setTXTprompt(titleptr);
  inputatom(MSG_EDTADD,MSG_AENTER);
  activeistextwindow=INPUT_ADDHOT;
  return 0;
 }
 else
 if(activeistextwindow==INPUT_ADDHOT)
 {
  redrawHTML(REDRAW_NO_MESSAGE,REDRAW_VIRTUAL);
  getTXTprompt(str,IE_MAXLEN);
  titleptr=str;
  activeistextwindow=0;
 }

 if(addhot(titleptr,urlptr))
 {
  sprintf(str,MSG_HOTLST,urlptr);
  outs(str);
 }
 else
 {
  outs(MSG_ERRHOT);
  Piip();
 }
 return 0;
}

int mouse2nextlink(int mouse,char asc)
{
  mouse=gotonextlink( &mousex, &mousey, 0,asc);
  ImouseSet( mousex, mousey);
  kbmouse=1;
  return GUI_MOUSE+mouse;
}

int mouse2previouslink(int mouse,char asc)
{
  mouse=gotonextlink( &mousex, &mousey, 1,asc); //back
  ImouseSet( mousex, mousey);
  kbmouse=1;
  return GUI_MOUSE+mouse;
}

int gotopreviouspage(void)
{
 if(goback())
  return gotoloc();

 Piip();
 return 0;
}

int gotonextpage(void)
{
 char *link;
  
 if(arachne.history<history.lines-1) //historie [->]
  link=ie_getline(&history,++arachne.history);
 else
  link=NULL;
 if(link)
 {
  strcpy(GLOBAL.location,link);
  GLOBAL.nothot=1;
  return gotoloc();
 }
 
 Piip();
 return 0;
}

int repaint(void)
{
 if(!GLOBAL.allowdealloc)
 {
  Piip();
  return 0;
 }
 mouseoff();
 graphicsinit(arachne.graphics); // XLOPIF SVGA GRAPHICS
 if(!strncmpi(arachne.graphics,"VGA",3))
  x_cleardev();
 RedrawALL();
 DrawTitle(1);
#ifdef VIRT_SCR
 novirtual();
#endif
 redraw=3;
 return 0;
}

int togglefullscreen(void)
{
 if(!GLOBAL.allowdealloc)
 {
  Piip();
  return 0;
 }

 if(fullscreen)
 {
  mouseoff();
  x_cleardev();
  arachne.GUIstyle-=4;
  if(x_maxx()<640)
   arachne.GUIstyle=0;
 }
 else
  arachne.GUIstyle|=4;

 Deallocmem();
 lastonbutton=-1;
 zoom();
 arachne.target=0; //cannot resize when in frames
 mouseoff();
 RedrawALL();
 GUIInit();
 GLOBAL.nothot=1;
 GLOBAL.needrender=1;
 GLOBAL.validtables=0; //tabulky je potreba prepocitat!!!
 return 1;
}

int erasecache(void)
{
 MemInfo(NORMAL);
 strcpy(GLOBAL.location,"file:clearcache.dgi");
 arachne.target=0;
 return gotoloc();
}

int reloadpage(void)
{
 GLOBAL.reload=RELOAD_CURRENT_LOCATION;
 GLOBAL.gotolocation=1;
 GLOBAL.isimage=0;
 return 1;
}

int scrollpageup(int scrollbarbutton)
{
  if(p->htmlframe[p->activeframe].posY>0)
  {
#ifdef VIRT_SCR
   long old=p->htmlframe[p->activeframe].posY;
#endif
   p->htmlframe[p->activeframe].posY-=p->htmlframe[p->activeframe].scroll.ysize/(1+(shift()||scrollbarbutton))-fonty(8,BOLD|ITALIC);
   if(p->htmlframe[p->activeframe].posY<0)
    p->htmlframe[p->activeframe].posY=0;
#ifdef VIRT_SCR
   if(user_interface.smooth)
    smothscroll(old,p->htmlframe[p->activeframe].posY);
#endif
   redraw=2;
  }
  else
   mouseon();
 return 0;
}

int scrollpagedown(int scrollbarbutton)
{
 struct HTMLframe *frame=&(p->htmlframe[p->activeframe]);
 
  if(frame->posY<frame->scroll.total_y-frame->scroll.ysize && 
     frame->scroll.total_y>frame->scroll.ysize)
  {
#ifdef VIRT_SCR
   long old=frame->posY;
#endif
   frame->posY+=frame->scroll.ysize/(1+(shift()||scrollbarbutton))-fonty(8,BOLD|ITALIC);
   if(frame->posY>frame->scroll.total_y-frame->scroll.ysize)
    frame->posY=frame->scroll.total_y-frame->scroll.ysize;
#ifdef VIRT_SCR
   if(user_interface.smooth)
    smothscroll(old,frame->posY);
#endif
   redraw=2;
  }
  else
   mouseon();
 return 0;
}

int scrollleft(void)
{
 struct HTMLframe *frame=&(p->htmlframe[p->activeframe]);

 if(frame->scroll.total_x>frame->scroll.xsize)
 {
  if(frame->posX>frame->scroll.xsize/2)
   frame->posX-=frame->scroll.xsize/2;
  else
   frame->posX=0;
  redraw=2;
 }
 else
  mouseon();
 return 0;
}

int scrollright(void)
{
 struct HTMLframe *frame=&(p->htmlframe[p->activeframe]);

 if(frame->scroll.total_x>frame->scroll.xsize)
 {
  if(frame->posX+frame->scroll.xsize/2<frame->scroll.total_x-frame->scroll.xsize)
   frame->posX+=frame->scroll.xsize/2;
  else
   frame->posX=frame->scroll.total_x-frame->scroll.xsize;
  redraw=2;
 }
 else
  mouseon();
 return 0;
}

int smothleft(void)
{
 struct HTMLframe *frame=&(p->htmlframe[p->activeframe]);

 if(frame->scroll.total_x>frame->scroll.xsize)
 {
  if(frame->posX>fontx(8,BOLD|ITALIC,'M'))
   frame->posX-=fontx(8,BOLD|ITALIC,'M');
  else
   frame->posX=0;
  redraw=2;
 }
 return 0;
}

int smothright(void)
{
 struct HTMLframe *frame=&(p->htmlframe[p->activeframe]);

 if(frame->scroll.total_x>frame->scroll.xsize)
 {
  if(frame->posX+fontx(8,BOLD|ITALIC,'M')<frame->scroll.total_x-frame->scroll.xsize)
   frame->posX+=fontx(8,BOLD|ITALIC,'M');
  else
   frame->posX=frame->scroll.total_x-frame->scroll.xsize;
  redraw=2;
 }
 return 0;
}

int smothup(int rate)
{
 struct HTMLframe *frame=&(p->htmlframe[p->activeframe]);

 if(frame->posY>0)
 {
  frame->posY-=rate*fonty(8,BOLD|ITALIC);
  if(frame->posY<0)
   frame->posY=0;
  redraw=2;
 }
 return 0;
}

int smothdown(int rate)
{
 struct HTMLframe *frame=&(p->htmlframe[p->activeframe]);

 if(frame->posY<frame->scroll.total_y-frame->scroll.ysize &&
    frame->scroll.total_y>frame->scroll.ysize)
 {
  frame->posY+=rate*fonty(8,BOLD|ITALIC);
  if(frame->posY>frame->scroll.total_y-frame->scroll.ysize)
   frame->posY=frame->scroll.total_y-frame->scroll.ysize;
  redraw=2;
 }
 return 0;
}


int printtxt(void)
{
 saveastext();
 {
  char *ptr=configvariable(&ARACHNEcfg,"PrintURL",NULL);
  if(!ptr)
   sprintf(GLOBAL.location,ptr="file:%s%sprint.ah",sharepath,GUIPATH);
  else
   strcpy(GLOBAL.location,ptr);
 }
 arachne.target=0;
 return gotoloc();
}

int escape(void)
{
 if(GLOBAL.timeout)
 {
  GLOBAL.timeout=0;
  defaultmsg();
 }
 arachne.scriptline=0;
 GLOBAL.gotolocation=0;
 GLOBAL.isimage=0;
 return 1;
}

int exitbrowser(void)
{
 strcpy(GLOBAL.location,"arachne:exit");
 arachne.target=0;
 return gotoloc();
}

int searchevent(void)
{
 inputatom(MSG_SRCH4,MSG_ENTER);
 activeistextwindow=INPUT_SEARCHSTRING;
 return 0;
}

int gotosearchpage(void)
{
 char *ptr=configvariable(&ARACHNEcfg,"SearchPage",NULL);
 if(ptr)
 {
  strcpy(GLOBAL.location,ptr);

  if(!strcmpi(GLOBAL.location,"find:"))
  {
   activeurl(GLOBAL.location);
   return 0;
  }

  arachne.target=0;
  return gotoloc();
 }

 Piip();
 return 0;
}

int gotohelppage(void)
{
#ifdef POSIX
  sprintf(GLOBAL.location,"file:%sindex.html",helppath);
#else
  sprintf(GLOBAL.location,"file:%shelp.htm",exepath);
#endif
 arachne.target=0;
 return gotoloc();
}

int gotolochome(void)
{
#ifdef POSIX
 sprintf(GLOBAL.location,"gui:home.ah",exepath);
#else
 sprintf(GLOBAL.location,"file:%shome.htm",exepath);
#endif
 arachne.target=0;
 return gotoloc();
}

int gotomailpage(void)
{
#ifdef POSIX
 strcpy(GLOBAL.location,"gui:mail.ah");
#else
 sprintf(GLOBAL.location,"file:%smail.htm",exepath);
#endif
 arachne.target=0;
 return gotoloc();
}

//------------------------------------------------------------------------
// GUIEVENT converts keyboard and mouse events to function calls
//------------------------------------------------------------------------

int GUIEVENT(int key, int mouse)
{
 char asc=toupper(key & 0xFF);
 XSWAP formID=IE_NULL;
 char scrollbarbutton=0;
 char *ptr, *link;
 int i;

 if(key)
 {
  if(asc==13) //bylo stisknuto enter ?
  {
   char *gotonewurl=NULL;

   switch(activeistextwindow)
   {
    case INPUT_SEARCHSTRING:
     SearchString();
     return 0;

    case INPUT_ADDHOT:
     return add2hotlist();

    case INPUT_READFILE:
    case INPUT_WRITEFILE:
     ReadWriteTextarea(activeistextwindow);
     return 0;

    case INPUT_SEARCHINTEXT:
     SearchInTextarea(0);
     return 0;

    case INPUT_URL:
     getTXTprompt(GLOBAL.location,URLSIZE-1);
     gotonewurl=GLOBAL.location;
     break;

    default:
     gotonewurl=try2getURL();
     if(gotonewurl)
      strcpy(GLOBAL.location,gotonewurl);
   }//end switch

   if(gotonewurl)
   {
    arachne.target=0;
    GLOBAL.postdata=0;
    GLOBAL.gotolocation=1;
    GLOBAL.isimage=0;
    return 1;
   }
   else
   {
    if(shift())
     return GUI_MOUSE_BUTTON_RIGHT;

    if(gotoactiveatom(asc,&formID))
    {
     link=activeislastinput();
     if(link)
      goto submit;
     else
      return mouse2nextlink(mouse,asc);
    }
    else
     return GUI_MOUSE_BUTTON_LEFT;
   }//endif key==Enter
  }
  else if(asc==10) //Ctrl+Enter
  {
   GLOBAL.backgr=1;
   return GUI_MOUSE_BUTTON_LEFT;
  }
  else if(asc==27) //Esc = abort
  {
   if(htmlpulldown)
    activeatomcursor(0);
   else
   if(activeistextwindow)
    redraw=2;
   else
   {
    if(user_interface.esc==ESC_BACK)
     return gotopreviouspage();
    else
    if(user_interface.esc==ESC_IGNORE || GLOBAL.timeout)
     GLOBAL.abort=ABORT_TRANSFER;
    else
     GLOBAL.abort=ABORT_PROGRAM;

    return escape();
   }
  }
  else if(asc==9) //Tab = nextatom
   return mouse2nextlink(mouse,asc);
  else if(key==0xf00) //Shift+Tab
   return mouse2previouslink(mouse,asc);
  else if(key==CTRLLEFT) //[<-]
   return gotopreviouspage();
  else if(key==CTRLRIGHT) //[->]
   return gotonextpage();
  else if(key==PAGEUP || asc=='B') //PgUp, B
  {
   if(htmlpulldown) //pass key to <SELECT> widget
   {
    SelectSwitch(0,0,key);
    return 0;
   }
   else
    return scrollpageup(0);
  }
  else if(key==PAGEDOWN || asc==32) //PgDn,Space
  {
   if(htmlpulldown) //pass key to <SELECT> widget
   {
    SelectSwitch(0,0,key);
    return 0;
   }
   else
    return scrollpagedown(0);
  }
  else if((key==HOMEKEY || (unsigned)key==0x8400))//home
  {
   if(htmlpulldown) //pass key to <SELECT> widget
   {
    SelectSwitch(0,0,key);
    return 0;
   }
   else
   if(shift()) //shift+home
    return scrollleft();
   else if(p->htmlframe[p->activeframe].posY>0)
   {
    p->htmlframe[p->activeframe].posY=0;
    redraw=2;
   }
  }
  else if((key==ENDKEY || key==0x7600))//end
  {
   if(htmlpulldown) //pass key to <SELECT> widget
   {
    SelectSwitch(0,0,key);
    return 0;
   }
   else
   if(shift()) //shift+end
    return scrollright();
   else if(p->htmlframe[p->activeframe].posY<p->htmlframe[p->activeframe].scroll.total_y-
           p->htmlframe[p->activeframe].scroll.ysize &&
           p->htmlframe[p->activeframe].scroll.total_y>p->htmlframe[p->activeframe].scroll.ysize)
   {
    p->htmlframe[p->activeframe].posY=
     p->htmlframe[p->activeframe].scroll.total_y-p->htmlframe[p->activeframe].scroll.ysize;
    if(p->htmlframe[p->activeframe].posY<0)
     p->htmlframe[p->activeframe].posY=0;
    redraw=2;
   }
  }
  else if(key==LEFTARROW)//doleva = left arrow
  {
   if(scrolllock())
    return gotopreviouspage();
   else
   if(user_interface.smooth && !shift())
    return smothleft();
   else
   {
   if(mousex>MOUSESTEP)
    mousex-=MOUSESTEP;
   ImouseSet( mousex, mousey);
   return GUI_MOUSE;
   }
  }
  else if(key==RIGHTARROW)//doprava = right arrow
  {
   if(scrolllock())
    return GUI_MOUSE_BUTTON_LEFT;
   else
   if(user_interface.smooth && !shift())
    return smothright();
   else
   {
    if(mousex<x_maxx()-MOUSESTEP)
     mousex+=MOUSESTEP;
    ImouseSet( mousex, mousey);
    return GUI_MOUSE;
   }
  }
  else if(key==UPARROW)//nahoru = arrow up
  {
   if(htmlpulldown && !shift()) //pass key to <SELECT> widget
   {
    SelectSwitch(0,0,key);
    return 0;
   }

   if(scrolllock())
    return mouse2previouslink(mouse,asc);
   else if(user_interface.smooth && !shift())
    return smothup(1);
   else
   {
    if(mousey>MOUSESTEP)
     mousey-=MOUSESTEP;
    ImouseSet( mousex, mousey);
    return GUI_MOUSE;
   }
  }
  else if(key==DOWNARROW)//dolu = arrow down
  {
   if(htmlpulldown && !shift()) //pass key to <SELECT> widget
   {
    SelectSwitch(0,0,key);
    return 0;
   }

   if(scrolllock())
    return mouse2nextlink(mouse,asc);
   else if(user_interface.smooth && !shift())
    return smothdown(1);
   else
   {
   if(mousey<x_maxy()-MOUSESTEP)
    mousey+=MOUSESTEP;
   ImouseSet( mousex, mousey);
   return GUI_MOUSE;
   }
  }
  else if(key==F1) //F1 key
   return gotohelppage();
  else if(key==0x3d00 || key==27136)//F3, Alt+F3
  {
   char newurl[URLSIZE]="file:*.htm";

   ptr=configvariable(&ARACHNEcfg,"AutoF3key",NULL);
   if(ptr && (*ptr=='Y' || *ptr=='y'))
   {
    if(tcpip)
    {
     activeurl("http://");
     return 0;
    }
   }
   else
   {
    ptr=configvariable(&ARACHNEcfg,"F3key",NULL);
    if(ptr)
     makestr(newurl,ptr,79);
   }

   activeurl(newurl);

   if(key==27136 && strchr(newurl,'*') && strcmp(newurl,p->htmlframe[0].cacheitem.URL))
   {
    strcpy(GLOBAL.location,newurl);
    arachne.target=0;
    return gotoloc();
   }
   return 0;
  }
  else if(user_interface.hotkeys) //----------------------- other hotkeys --
  {
   if(key==BACKSPACE) //backspace = history
    return gotohistory();
   else if(key==DELETEKEY) //delete = del mail/cache item
   {
    if(GLOBAL.clipdel==CLIPBOARD_DELETE)
    {
     struct ib_editor clipboard;
     struct Url url;
     char str[128], line[IE_MAXLEN+2],*ptr;

     strcpy(clipboard.filename,CLIPBOARDNAME);
     if(ie_openf_lim(&clipboard,CONTEXT_TMP,1000)==1) //ok
     {
      clipboard.y=0;
      while(clipboard.y<=clipboard.lines)
      {
       ptr=ie_getline(&clipboard,clipboard.y);
       if(ptr)
       {
        makestr(line,ptr,IE_MAXLEN);
        ptr=line;

        HideLink(ptr);

        if(!strncmpi(ptr,"reload:",7))
         ptr+=7;

        AnalyseURL(ptr,&url,arachne.target); //(plne zneni...)

        if(!strcmpi(url.protocol,"file"))
        {
         if(!strncmp(url.file,"//",2))
          ptr=&url.file[2];
         else
          ptr=url.file;

         unlink(ptr);
         sprintf(str,MSG_REMOVE,ptr);
         outs(str);
        }
       }//endif
       clipboard.y++;
      }//loop
      redrawHTML(REDRAW_WITH_MESSAGE,REDRAW_SCREEN);
     }
//     ie_closef(&clipboard);
     ie_killcontext(CONTEXT_TMP);
     GLOBAL.del=2; //delete document from cache - HREF was REMOVED
    }
    else
     GLOBAL.del=1;
    return 1;
   }
   else if(asc=='/' || asc==17 || key==F7) // F7 || ^Q - search for string
    return searchevent();
   else if(asc==12) //^L - search next
   {
    SearchString();
    return 0;
   }
   else if(asc=='R')
    return reloadpage();
#ifndef AGB
   else if(asc=='D')
    return gotodialpage();
   else if(asc=='M')
    return gotomailpage();
   else if(asc=='C')
   {
    sprintf(GLOBAL.location,"mailto:",exepath);
    arachne.target=0;
    return gotoloc();
   }
   else if(asc=='I')
   {
    sprintf(GLOBAL.location,"file://inbox.dgi",exepath);
    arachne.target=0;
    return gotoloc();
   }
   else if(asc=='S')
   {
#ifdef POSIX
    strcpy(GLOBAL.location,"gui:setup.ah");
#else
    sprintf(GLOBAL.location,"file:%ssetup.htm",exepath);
#endif
    arachne.target=0;
    return gotoloc();
   }
   else if(asc=='O')
   {
    sprintf(GLOBAL.location,setupdoc,exepath);
    arachne.target=0;
    return gotoloc();
   }
   else if(asc=='U')
   {
#ifdef POSIX
    strcpy(GLOBAL.location,"gui:utils.ah");
#else
    sprintf(GLOBAL.location,"file:%sutils.htm",exepath);
#endif
    arachne.target=0;
    return gotoloc();
   }
#ifndef CLEMTEST
   else if(asc=='Q')
   {
    if(!ProcessLinks(1))
    {
     Piip();
     return 0;
    }
    arachne.target=0;
    return gotoloc();
   }
   else if(asc=='V')
   {
    int i=ProcessLinks(0);
    char str[128];
    sprintf(str,MSG_MISLNK,i);
    outs(str);
    return 0;
   }
#endif //CLEMTEST
#endif
   else if(asc=='K')
   {
    sprintf(GLOBAL.location,"file:%shotkeys.htm",exepath);
    arachne.target=0;
    return gotoloc();
   }
#ifndef AGB
   else if(asc=='T')
   {
    sprintf(GLOBAL.location,"telnet:",exepath);
    arachne.target=0;
    return gotoloc();
   }
#endif
   else if(asc=='X')
   {
    strcpy(GLOBAL.location,homepage);
    arachne.target=0;
    return gotoloc();
   }
   else if(asc=='H' || key==0x2b1c || key==-32512) //Alt+0,Ctrl+backslash
    return gotohotlist();
   else if(asc=='P')
    return printtxt();
   else if(asc==ASCIICTRLP) //Ctrl+P ... BMP export of virtual screen
   {
    if(PrintScreen2BMP(1))
    {
     sprintf(GLOBAL.location,"file:%s%sprtbmp.ah",sharepath,GUIPATH);
     arachne.target=0;
     return gotoloc();
    }
   }
   else if(asc=='A')
   {
    return add2hotlist();
   }
   else if(asc=='F' && arachne.framescount>0)
   {
    char old;
    do
    {
     old=p->activeframe;
     p->activeframe=p->htmlframe[p->activeframe].next;
    }
    while(p->htmlframe[p->activeframe].hidden && p->activeframe!=-1);

    if(p->activeframe>arachne.framescount || p->activeframe==-1)
    {
     mouseoff();
     p->oldactive=old;
     p->activeframe=0;
     drawframeborder(p->oldactive);
     mouseon();
    }
    else
    {
     mouseoff();
     drawactiveframe();
     mouseon();
    }
   }
   else if(asc=='G') // active URL input field, like in Lynx
   {
    if(!customerscreen)
     activeurl("");
    return 0;
   }
   else if(asc=='~')//ignore images
   {
    ignoreimages=1-ignoreimages;
    redrawHTML(REDRAW_WITH_MESSAGE,REDRAW_CREATE_VIRTUAL);
   }
   else if(asc=='?')//query XSWAP
   {
    int i=0,w;
    char *ptr=" ";
#ifndef POSIX
    gotoxy(1,8);
#endif
    while(i<IE_MAXSWAP)
    {
#ifndef POSIX
     w=ie_which(i);
     if(w)
      ptr[0]='0'+w;
     else
#endif
      ptr[0]=' ';
     printf("%s[s=%5u n=%4d c=%2d]",ptr,swapsize[i],swaplen[i],swapcontext[i]);
     if(!((i+1)%3))
      printf("\n");
     i++;
    }
    printf(" w=%d,r=%d",swapsavecount,swaploadcount);

    return 0;
   }
   else if(asc=='!' && vga16mode)//ignore images
   {
    if(!GLOBAL.allowdealloc)
    {
     Piip();
     return 0;
    }

    if(!vgamono)
    {
     if(!strcmpi(arachne.graphics,"VGA"))
      strcpy(arachne.graphics,"VGAMONO");
     else
     {
      char *ptr=strstr(arachne.graphics,".A");
      if(ptr)
       strcpy(ptr,".M");
     }
    }
    else
    {
     char *ptr=strstr(arachne.graphics,".M");
     if(ptr)
      strcpy(ptr,".A");
     else
      strcpy(arachne.graphics,"VGA");
     vgamono=0;
    }

    return repaint();
   }
   else if(asc=='=')
   {
    GLOBAL.source=0;
    GLOBAL.validtables=0;
    sprintf(GLOBAL.location,"file:%s",p->htmlframe[p->activeframe].cacheitem.locname);

    {
     int ishttp=!strncmpi(p->htmlframe[p->activeframe].cacheitem.URL,"http:",5);
     if(ishttp && !user_interface.nohtt)
      makehttfilename(p->htmlframe[p->activeframe].cacheitem.rawname,&GLOBAL.location[5]);
     else
     {
      char *ptr=strrchr(GLOBAL.location,'.');
      if(ptr)
      {
       if(ishttp)
        strcpy(ptr,".fil");
       else
        makestr(&ptr[1],p->htmlframe[p->activeframe].cacheitem.URL,3); //napr. HTT, GOP ...
      }
     }
    }
    strcpy(LASTlocname,p->htmlframe[p->activeframe].cacheitem.locname);
    arachne.target=0;
    return gotoloc();
   }
   else if(asc=='-') //decrease resolution
   {
    if(!GLOBAL.allowdealloc)
    {
     Piip();
     return 0;
    }

    Deallocmem();
    ChangeZoom(0,0,1);
    return 1;
   }
   else if(asc=='+') //increase resolution
   {
    if(!GLOBAL.allowdealloc)
    {
     Piip();
     return 0;
    }
    Deallocmem();
    ChangeZoom(0,1,0);
    return 1;
   }
   else if(asc=='*') //toggle fullscreen
   {
    return togglefullscreen();
   }

   if(key==0x2d00 || key==0x6b00) //Alt+X,Alt+F4
    return exitbrowser();

#ifdef POSIX
   if(key==PRTSCR) //PrintScreen
    return PrintScreen2BMP(0);
#endif

#ifndef AGB
   if(key==0x2300) //Alt+H
   {
    strcpy(GLOBAL.location,"arachne:hangup");
    arachne.target=0;
    return gotoloc();
   }

   if(key==0x2000) //Alt+D
   {
    strcpy(GLOBAL.location,"arachne:dialer");
    arachne.target=0;
    return gotoloc();
   }
#endif

   if(key==0x1200) //Alt+E
   {
    strcpy(GLOBAL.location,"file:dosshell.dgi");
    arachne.target=0;
    return gotoloc();
   }

#ifndef NOPS
#ifdef LINUX
   if(key==ASCIICTRLP) //Ctrl+P
#else
   if(key==0x1900) //Alt+P
#endif   
   {
    saveasps();
    sprintf(GLOBAL.location,"file:%s%sps.ah",sharepath,GUIPATH);
    arachne.target=0;
    return gotoloc();
   }
#endif

   if(key==0x1000) //Alt+Q
   {
    sprintf(GLOBAL.location,"file:%s%squickpad.ah",sharepath,GUIPATH);
    arachne.target=0;
    return gotoloc();
   }

   if(key==0x3200) //Alt+M
   {
    MemInfo(FORCED);
    return 0;
   }

#ifndef AGB
   if(key==0x5200) //Insert
   {
    GLOBAL.nowimages=IMAGES_LOAD;
    return 1;
   }
#endif

   if(key==0x3c00)//F2
   {
    savesend:
    link=gotoactiveatom(asc,&formID);
    if(link)
     goto submit;
    GLOBAL.source=0;
    GLOBAL.validtables=0;
    strcpy(LASTlocname,p->htmlframe[p->activeframe].cacheitem.locname);
    sprintf(p->htmlframe[0].cacheitem.locname,"%s%ssaveas.ah",sharepath,GUIPATH);
    arachne.framescount=0;
    arachne.target=0;
    GLOBAL.needrender=2; //forced HTML
    return 1;
   }

   if(key==0x3e00)//F4
   {
    ptr=configvariable(&ARACHNEcfg,"Editor",NULL);
    if(ptr && strcmpi(ptr,"NUL"))
    {
     sprintf(GLOBAL.location,"edit:%s",p->htmlframe[p->activeframe].cacheitem.locname);
     return gotoloc();
    }
    GLOBAL.source=0;
    GLOBAL.validtables=0;
    strcpy(LASTlocname,p->htmlframe[p->activeframe].cacheitem.locname);
    sprintf(p->htmlframe[0].cacheitem.locname,"%s%stextedit.ah",sharepath,GUIPATH);
    arachne.framescount=0;
    arachne.target=0;
    GLOBAL.needrender=2; //forced HTML
    return 1;
   }

#ifndef AGB
   if(key==0x3f00)//F5 - zoom
   {
    if(!GLOBAL.allowdealloc)
    {
     Piip();
     return 0;
    }
    if(fullscreen || x_maxx()<640 &&
     (arachne.GUIstyle==STYLE_SMALL2 || customerscreen))
     return togglefullscreen();
    Deallocmem();
    ChangeZoom(1,0,0);
    return 1;
   }
#endif

   if(key==0x4000)//F6 - source
   {
    GLOBAL.source=1-GLOBAL.source;
    GLOBAL.needrender=1;
    return 1;
   }

   if(key==0x6800)//Alt+F1 - home
   {
    gohome();
    arachne.target=0;
    return gotoloc();
   }

   if(key==0x6900)//Alt+F2 - accesible drives
   {
    strcpy(GLOBAL.location,"file:@:*.*");
    arachne.target=0;
    return gotoloc();
   }

#ifndef AGB
   if(key==0x6e00)//Alt+F7 - search engine
    return gotosearchpage();
#endif

   else if(key==F8)//F8 - clean cache
    return erasecache();

   else if(key==REDRAW_KEY)//F5 or F9
    return repaint();
#ifndef AGB
   else if(key==F10)//F10 - local home
    return gotolochome();
#endif
   else if(key>=0x5400 && key<=0x5d00 /* &&reg*/)
   {
    char arachnomania[10];

    sprintf(arachnomania,"ShiftF%d",(key-0x5400)/256+1);
    ptr=configvariable(&ARACHNEcfg,arachnomania,NULL);
    if(ptr)
    {
     strcpy(GLOBAL.location,ptr);
     arachne.target=0;
     return gotoloc();
    }
   }//end of keys
  } 
  return 0;
 }//endif key

//========================================================================

 if(mouse)  
 {
  SecondsSleeping=0l; //pro screensaver
  if(!lmouse)
  {
   link=onmouse(mouse);
   if(mouse==MOUSE_RELEASE)
    return 0;

submit:

   x_cursor (mousex,mousey);
#ifdef GGI
  Smart_ggiFlush();
#endif
   if(link)
   {
    int maptype, dx, dy;
    strcpy(GLOBAL.location,link); 
    //warning: "link" is xSwap object - won't be vaild any longer in DOS version

    toolbar(0,1); //show standard toolbar

    if(bioskey(2) & CTRLKEY) //nahrat na pozadi
     GLOBAL.backgr=1;
    maptype=activeismap(&dx,&dy);
#ifdef CUSTOMER_MODULE
    if(customer_URLcheck())
     return gotoloc();
#endif
    if(!strncmpi(GLOBAL.location,"arachne:",8))
    {
/*
 this is now implemented ratherr as file:clearcache.dgi in MIME.CFG
     if(!strncmpi(&GLOBAL.location[8],"kill-cache",10))
      return erasecache(1);
     else
     if(!strncmpi(&GLOBAL.location[8],"kill-html",9))
     {
      erasecache(2);
      if(GLOBAL.abort)
      {
       return 0;
      }
      else
      {
       MemInfo(NORMAL);
       strcpy(GLOBAL.location,"arachne:restart");
       arachne.target=0;
       return gotoloc();
      }
     }
     else
*/
     if(!strncmpi(&GLOBAL.location[8],"fullscreen",10))
      return togglefullscreen();
     else
     if(!strncmpi(&GLOBAL.location[8],"internal-config",15))
     {
      outs(MSG_CONFIG);
      process_form(0,formID); //updateovat Arachne.Cfg
      if(GLOBAL.location[23]=='=' || GLOBAL.location[23]=='?')
      {
       int l=strlen(&GLOBAL.location[24]);
       memmove(GLOBAL.location,&GLOBAL.location[24],l);
       GLOBAL.location[l]='\0';
       GLOBAL.reload=RELOAD_NEW_LOCATION;
      }
      else
       sprintf(GLOBAL.location,setupdoc,exepath);
     }
     else
     if(!strncmpi(&GLOBAL.location[8],"internal-",9))
     {
      char mail=0;
      if(!strncmpi(&GLOBAL.location[17],"mail",4))
       mail=1;
      else if(!strncmpi(&GLOBAL.location[17],"send",4))
       mail=2;

      outs(MSG_WRITE);

      process_form(0,formID); //save editor file, create mail, etc.

      if(!strncmpi(&GLOBAL.location[17],"htmledit",8))
      {
       GLOBAL.needrender=1;
       GLOBAL.validtables=0;
       strcpy(p->htmlframe[0].cacheitem.locname,LASTlocname);
       return 1;
      }
      else if(mail)
      {
       if(!(GLOBAL.mailaction & MAIL_SMTPNOW) &&
          !(GLOBAL.mailaction & MAIL_OUTBOXNOW))
       {
        if(mail==1)
         goback();
        else
         strcpy(GLOBAL.location,p->htmlframe[p->activeframe].cacheitem.URL);
       }

       if(GLOBAL.mailaction & MAIL_ATTACH)
        add2history(GLOBAL.location);
      }
      else if(!strncmpi(&GLOBAL.location[17],"vga",4))
      {
       goback();
       strcpy(GLOBAL.location,"arachne:exit?0");
      }
      else
       goback();
     }
     else
     if(!strcmpi(&GLOBAL.location[8],"copy"))
     {
      outs(MSG_COPY);
      process_form(0,formID); //copy file
      goback();
     }
     else
     if(!strcmpi(&GLOBAL.location[8],"save"))
     {
      outs(MSG_COPY);
      process_form(0,formID); //copy file
      GLOBAL.needrender=1;
      GLOBAL.validtables=0;
      strcpy(p->htmlframe[0].cacheitem.locname,LASTlocname);
      return 1;
     }

     if(!strcmpi(GLOBAL.location,"arachne:pppsetup"))
     {
      ptr=configvariable(&ARACHNEcfg,"Connection",NULL);
      if(ptr && (strstr(ptr,"pppd") || strstr(ptr,"PPPD")))
       sprintf(GLOBAL.location,"file:%s%sconf_ppp.ah",sharepath,GUIPATH);
      else
       sprintf(GLOBAL.location,"file:%s%sconf_ext.ah",sharepath,GUIPATH);
     }//endif pre-process URL

     //no else - allow arachne:internal-config?arachne:back

     if(!strcmpi(GLOBAL.location,"arachne:authenticate"))
     {
      process_form(0,formID);
      AUTHENTICATION->flag=AUTH_FORCED;
      return reloadpage();
     }

     if(!strcmpi(GLOBAL.location,"arachne:back"))
      goback();

     if(!strcmpi(GLOBAL.location,"arachne:ftp-login"))
     {
      process_form(0,formID);
      strcpy(AUTHENTICATION->realm,"$ftp");
      GLOBAL.reload=RELOAD_CURRENT_LOCATION;
     }

     if(!strcmpi(GLOBAL.location,"arachne:again"))
      strcpy(GLOBAL.location,p->htmlframe[p->activeframe].cacheitem.URL);

     if(!strcmpi(GLOBAL.location,"arachne:view"))
     {
      GLOBAL.source=1;
      GLOBAL.needrender=1;
      strcpy(p->htmlframe[p->activeframe].cacheitem.locname,
             p->htmlframe[p->activeframe].cacheitem.rawname);
      return 1;
     }

     GLOBAL.postdata=0;
    }
    else
    if(GLOBAL.postdata)
    {
     char http=0;
     if(!strncmpi(GLOBAL.location,"http:",5))
      http=2;
     //pridat Query string k URL zacinajicimu http://
     process_form(1|http|maptype,formID);
     if(GLOBAL.postdata==1) //FORM METHOD=GET
     {
      int ql;
      char *querystring=ie_getswap(GLOBAL.postdataptr);
      if(!querystring)
       MALLOCERR();
      ql=strlen(querystring);
      if(ql+strlen(GLOBAL.location)+1>=URLSIZE || http==0)
       GLOBAL.postdata=2; //zkusim metodu post
      else
      {
       strcat(GLOBAL.location,"?");
       strcat(GLOBAL.location,querystring);
       GLOBAL.postdata=0;
      }
     }//endif
    }
    else
    if(maptype==1) //je aktivnim obrazkem klikatelna mapa "ISMAP" ?
    {
     char str[20];
     sprintf(str,"?%d,%d",mousex-dx,mousey-dy);
     strcat(GLOBAL.location,str);
    }//endif

    return gotoloc();
   }
   else if(mouse<3)//mouse clicked, but not on link
   {
    int choice=onbutton(mousex,mousey);

#ifdef CUSTOMER_MODULE
    if(customerscreen && customer_event(choice))
     return gotoloc();
#endif

#ifdef OVRL

    if(choice>=CLICK_ANY_BIG_BUTTON && choice<CLICK_SPECIAL && toolbarmode==0)
    {
     //custozmizable Arachne toolbar
     char iconkey[5],method,methodarg[80],dummy[40];

     sprintf(iconkey,"??%d",choice/10);

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

     if(geticoninfo(iconkey,dummy,&method,methodarg,dummy,dummy))
     {
#ifndef XTVERSION
      pressthatbutton(0);
#endif
      if(method=='T') //switch toolbar
      {
       if(methodarg[0]=='0')
        toolbarpage=0;
       else
        toolbarpage=methodarg[0];
       mouseoff();
       toolbar(0,0);
#ifndef XTVERSION
       showhighlight();
#endif
       mouseon();
       return 0;
      }
      else if(method=='U')
      {
       sprintf(GLOBAL.location,methodarg);
       arachne.target=0;
       return gotoloc();
      }
      else
      {
       if(methodarg[0]=='\'')
        return GUIEVENT((int)methodarg[1],0);
       else
        return GUIEVENT(atoi(methodarg),0);
      }
     }
     choice=0;
    }

#ifndef XTVERSION
    if(choice)
     pressthatbutton(0);
#endif

    if(!activeatomptr && (choice<10 || choice>90))
     toolbar(0,1); //show standard toolbar

#endif

    switch(choice)
    {
     //-----------------------------------------------------------------
     case CLICK_PREVIOUS:
     //-----------------------------------------------------------------
     return gotopreviouspage();

     //-----------------------------------------------------------------
     case CLICK_NEXT:
     //-----------------------------------------------------------------
     return gotonextpage();
      
     //-----------------------------------------------------------------
     case CLICK_HOME:
     //-----------------------------------------------------------------
     gohome();
     arachne.target=0;
     return gotoloc();

     case CLICK_RELOAD:
#ifdef AGB
     return printtxt();
#else
     return reloadpage();
#endif

     //-----------------------------------------------------------------
     case CLICK_ADDHOTLIST: //add to hotlist
     //-----------------------------------------------------------------
     
     return add2hotlist();

     //-----------------------------------------------------------------
     case CLICK_HOTLIST: //go to hotlist
     //-----------------------------------------------------------------
     return gotohotlist();

     case CLICK_ABORT: //esc
#ifndef AGB
     GLOBAL.abort=ABORT_TRANSFER;
     return escape();
#else
     return exitbrowser();
#endif

     //-----------------------------------------------------------------
     case CLICK_SEARCHENGINE: //F7
     //-----------------------------------------------------------------
#ifdef AGB
     return searchevent();
#else
     return gotosearchpage();
#endif

     //-----------------------------------------------------------------
     case CLICK_HELP: //go to help
     //-----------------------------------------------------------------
     return gotohelppage();

     //alternative toolbar for TEXAREA --------------------------- start

     case 10:
     WriteFileBox();
     return 0;

     case 20:
     ReadFileBox();
     return 0;

     case 30:
     activeatomptr=&activeatom;
     activeadr=focusedatom;
     activeatomtick(ASCIICTRLX,0);
     return 0;

     case 40:
     activeatomptr=&activeatom;
     activeadr=focusedatom;
     activeatomtick(ASCIICTRLC,0);
     return 0;

     case 50:
     activeatomptr=&activeatom;
     activeadr=focusedatom;
     activeatomtick(ASCIICTRLV,0);
     return 0;

     case 60:
     activeatomptr=&activeatom;
     activeadr=focusedatom;
     activeatomtick(ASCIICTRLC,0);
     activeatomtick(ASCIICTRLV,0);
     return 0;

     case 70:
     activeatomptr=&activeatom;
     activeadr=focusedatom;
     activeatomtick(ASCIICTRLX,0);
     activeatomtick(ASCIICTRLV,0);
     return 0;

     case 80:
     activeatomptr=&activeatom;
     activeadr=focusedatom;
     activeatomtick(ASCIICTRLD,0);
     return 0;

     case 90:
     SearchInTextBox();
     return 0;

//alternative toolbar for TEXAREA ------------------------------------ end

     case CLICK_IMAGES:
     GLOBAL.nowimages=IMAGES_LOAD;
     return 1;

     case CLICK_DESKTOP:
     return gotolochome();

     case CLICK_MAIL:
     return gotomailpage();

     case CLICK_SAVE:
     goto savesend;

     case CLICK_NETHOME://net home
     strcpy(GLOBAL.location,homepage);
     arachne.target=0;
     return gotoloc();

     case CLICK_ABOUT://click on "Arachne Vxx" label
     strcpy(GLOBAL.location,"about:");
     arachne.target=0;
     return gotoloc();

     case CLICK_HISTORY:
     return gotohistory();

     case CLICK_ZOOM://zoom
#ifndef AGB
     ChangeZoom(1,0,0);
     return 1;
#else
     return repaint();
#endif //AGB

#ifndef AGB
     case CLICK_MEMINFO:
     MemInfo(FORCED);
     return 0;

     case CLICK_TCPIP:
     return gotodialpage();     
#endif
     case CLICK_EXIT:
     GLOBAL.abort=ABORT_PROGRAM;
     arachne.target=0;
     GLOBAL.gotolocation=0;
     GLOBAL.isimage=0;
     return 1;

    }//endswith;

#ifndef CUSTOMER
    if(mousex>x_maxx()-206 && mousey>x_maxy()-13 &&
       mousex<x_maxx()-156 && mousey<x_maxy()-2)
    {
     strcpy(lasttime,"*"); //screensaver activated
     SecondsSleeping=32000l;
     ImouseWait();
     return 0;
    }
#endif

    activeatomScrollBarBUTTONS();

    i=0;
    do
    {
     //=====================================================================
     //scroll buttons
#ifdef GGI
     if( !user_interface.smooth && !p->htmlframe[i].hidden && p->htmlframe[i].allowscrolling )
#else

     if((!p->tmpframedata[i].usevirtualscreen || !user_interface.smooth)
      && !p->htmlframe[i].hidden && p->htmlframe[i].allowscrolling )

#endif
     {
      int s=OnScrollButtons(&(p->htmlframe[i].scroll));

      if(s)
      {
       mouseoff();
       p->activeframe=i;
       drawactiveframe();
       ImouseWait(); //msg for Bernie: don't forget, that may be 0...

       switch(s)
       {
        case 1: return scrollpageup(1);           //up
        case 2: return scrollpagedown(1);         //down
        case 3: return scrollleft();              //left
        case 4: return scrollright();             //right
       }//end switch
      }//end if OnScrollButton
     }
     //=====================================================================
     //black zone in scroll bars
     if(!p->htmlframe[i].hidden && p->htmlframe[i].allowscrolling )
     {
      int s=OnBlackZone(&(p->htmlframe[i].scroll));

      if(s)
      {
       mouseoff();
       p->activeframe=i;
       drawactiveframe();
       ImouseWait();

       switch(s)
       {
        case 1: return scrollpageup(0);
        case 2: return scrollpagedown(0);
        case 3: p->htmlframe[p->activeframe].posX=0;
                redraw=2;
                break;
        case 4: p->htmlframe[p->activeframe].posX=
                 p->htmlframe[p->activeframe].scroll.total_x-p->htmlframe[p->activeframe].scroll.xsize;
                if(p->htmlframe[p->activeframe].posX<0)
                 p->htmlframe[p->activeframe].posX=0;
                redraw=2;
       }//end switch
      }//end if OnBlackZone...

     }
    }
    while(i++<arachne.framescount); //[0...3]

    if(mousey<p->htscrn_ytop) //deactive active frame - clicked on title bar...
    {
     mouseoff();
     p->oldactive=p->activeframe;
     p->activeframe=0;
     drawframeborder(p->oldactive);
     mouseon();
    }
   }//end "clicked not on link"
  }
  else //...still holding mouse
  {
   //======================================================================
   //scroll bars

   if( !activeatomScrollBarTICK())
   {
   i=0;
   do if(!p->htmlframe[i].hidden && p->htmlframe[i].allowscrolling &&
         (scrolledframe==-1 || scrolledframe==i))
   {
    if(ScrollBarTICK(&(p->htmlframe[i].scroll),
       &(p->htmlframe[i].posX),&(p->htmlframe[i].posY)))
    {
     mouseoff();
     p->activeframe=i;
     drawactiveframe();
     redraw=1;

#ifdef GGI
    if(user_interface.ggifastscroll)
     redrawHTML(REDRAW_NO_MESSAGE,REDRAW_VIRTUAL);

    else

     ScrollDraw(&(p->htmlframe[p->activeframe].scroll),
                 p->htmlframe[p->activeframe].posX, 
                 p->htmlframe[p->activeframe].posY);
#endif

#ifdef VIRT_SCR
     Try2DumpActiveVirtual();
#endif

#ifndef GGI
     ScrollDraw(&(p->htmlframe[p->activeframe].scroll),
                p->htmlframe[p->activeframe].posX,
                p->htmlframe[p->activeframe].posY);
#endif
     scrolledframe=i;
    }

#ifdef GGI
    if(user_interface.smooth)
    {
     scrollbarbutton=OnScrollButtons(&(p->htmlframe[i].scroll));
     if(scrollbarbutton)
     {
      p->activeframe=i;
      mouseoff();
      if(redraw)
       redrawHTML(REDRAW_WITH_MESSAGE,REDRAW_VIRTUAL);
      else
      if(arachne.framescount)
       drawactiveframe();
      mouseon();
      switch(scrollbarbutton)
      {
       case 1: return smothup();
       case 2: return smothdown();
       case 3: return smothleft();
       case 4: return smothright();
      }//end switch
     }
    }
#else

#ifdef VIRT_SCR
    if(user_interface.smooth)
    {
     scrollbarbutton=OnScrollButtons(&(p->htmlframe[i].scroll));
     if(scrollbarbutton)
     {
      p->activeframe=i;
      mouseoff();
      if(redraw)
      {
       Try2DumpActiveVirtual();
       ScrollDraw(&(p->htmlframe[p->activeframe].scroll),
                  p->htmlframe[p->activeframe].posX,
                  p->htmlframe[p->activeframe].posY);

/*     *** this is what Netscape does, but it is too slow 
       redrawatoms(p->activeframe,
              p->htmlframe[p->activeframe].posX,p->htmlframe[p->activeframe].posY,
              p->htmlframe[p->activeframe].scroll.xsize,p->htmlframe[p->activeframe].scroll.ysize,
              p->htmlframe[p->activeframe].scroll.xtop,p->htmlframe[p->activeframe].scroll.ytop);
*/
      }
      else
      if(arachne.framescount)
       drawactiveframe();
      mouseon();
      switch(scrollbarbutton)
      {
       case 1: return smothup(1);
       case 2: return smothdown(1);
       case 3: return smothleft();
       case 4: return smothright();
      }//end switch
     }
    }
#endif
#endif

   }
   while(i++<arachne.framescount); //[0...3]
   }//endif "aktivni atom se pohnul"

  }//endif holding mouse
  
 }//endif mouse event
 return 0;

}



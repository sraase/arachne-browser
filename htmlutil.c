
// ========================================================================
// HTML rendering routines for Arachne WWW browser
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "html.h"
#include "xanimgif.h"

//start of frames
//============================================================================
void addframeset(char xflag, int *emptyframe, char framewantborder, char *startptr)
{
 char *endptr;
 int velikost[MAXFRAMES];
 struct ScrollBar *from;
 int fromframe,newframecount=0,border=1,starcount=0;
 int vsechno,zbytek,i;
 int prevframe,newframe=findfreeframe();

 if(newframe>=MAXFRAMES-1 || newframe<1)
  return;

 if(*emptyframe==-1) //we are adding first empty frame
  fromframe=p->currentframe;
 else
  fromframe=*emptyframe;

 *emptyframe=newframe;

 if(fromframe && p->htmlframe[fromframe].frameborder)
  border=0;

 if(framewantborder==UNDEFINED_FRAMEBORDER)
 {
  if(fromframe)
   framewantborder=p->htmlframe[fromframe].frameborder & I_WANT_FRAMEBORDER;
  else
   framewantborder=I_WANT_FRAMEBORDER;
 }

 memset(velikost,0,2*MAXFRAMES);
 from=(&p->htmlframe[fromframe].scroll);

 if(xflag)
 {
  vsechno=zbytek=from->xsize+1-border;
  if(p->htmlframe[fromframe].allowscrolling)
   vsechno+=user_interface.scrollbarsize;
 }
 else
  vsechno=zbytek=from->ymax+1-border;

 while(startptr && zbytek>0)
 {
  while(*startptr==' ')
   startptr++;

  endptr=strchr(startptr,',');
  if(endptr)
   *endptr='\0';

  if(strchr(startptr,'*'))
  {
   velikost[newframecount]=-1;
   starcount++;
  }
  else
  {
   velikost[newframecount]=try2getnum(startptr,vsechno);
   if(velikost[newframecount]>vsechno)
    velikost[newframecount]=vsechno;

   if(!endptr && !starcount || velikost[newframecount]>zbytek)
    velikost[newframecount]=zbytek;
   zbytek-=velikost[newframecount];
  }

  newframecount++;
  if(endptr)
   startptr=endptr+1;
  else
   startptr=NULL;
 }//loop

 if(zbytek<0)
  zbytek=0;

 if(starcount)
 {
  zbytek/=starcount;
  starcount=0;
  while(starcount<newframecount)
  {
   if(velikost[starcount]==-1)
    velikost[starcount]=zbytek;
   starcount++;
  }
 }//endif

 zbytek=0;

 i=0;
 while(i<newframecount && newframe>0)
 {
  p->tmpframedata[newframe].whichvirtual=newframe-1; //let's overwrite screen 0
  p->htmlframe[newframe].parent=p->currentframe;
  p->htmlframe[newframe].hidden=0;
  p->htmlframe[newframe].allowscrolling=0;
  p->htmlframe[newframe].frameborder=FRAMEBORDER_IS_ON | framewantborder; //was: =1;
  if(xflag)
  {
   p->htmlframe[newframe].scroll.xsize=velikost[i]/*-user_interface.scrollbarsize*/-3;
   p->htmlframe[newframe].scroll.ymax=from->ymax-2*border;
   p->htmlframe[newframe].scroll.xtop=from->xtop+zbytek+border;
   p->htmlframe[newframe].scroll.ytop=from->ytop+border;
   if(velikost[i]<=0)
    p->htmlframe[newframe].hidden=1;
  }
  else
  {
   p->htmlframe[newframe].scroll.xsize=from->xsize-2*border;
   if(p->htmlframe[fromframe].allowscrolling)
    p->htmlframe[newframe].scroll.xsize+=user_interface.scrollbarsize;
   p->htmlframe[newframe].scroll.ymax=velikost[i]-3; // 3 - reserve for neighbour
   p->htmlframe[newframe].scroll.xtop=from->xtop+border;
   p->htmlframe[newframe].scroll.ytop=from->ytop+zbytek+border;
   if(velikost[i]<=3)
    p->htmlframe[newframe].hidden=1;
  }

  zbytek+=velikost[i++];
  prevframe=newframe;
  newframe=findfreeframe();
  p->htmlframe[prevframe].next=newframe;
 }
 p->htmlframe[prevframe].next=p->htmlframe[fromframe].next;

 if(newframe==-1) //cannot insert new frame - error!
 {
  *emptyframe=-1;
  return;
 }

 p->htmlframe[fromframe].hidden=1;

 if(xflag)
 {
  p->htmlframe[prevframe].scroll.xsize=
   from->xsize-p->htmlframe[prevframe].scroll.xtop+from->xtop-border;
  if(p->htmlframe[fromframe].allowscrolling)
   p->htmlframe[prevframe].scroll.xsize+=user_interface.scrollbarsize;
 }
 else
  p->htmlframe[prevframe].scroll.ymax=
   from->ymax-p->htmlframe[prevframe].scroll.ytop+from->ytop-border;

}

/*
void resetframeborder(struct HTMLframe *frame, char shift)
{
 frame->frameborder=0;
 frame->scroll.xsize+=1;
 frame->scroll.ymax+=1;
 frame->scroll.xtop-=1;
 frame->scroll.ytop-=1;
}
*/
//this will "deallocate" child frames of specified parent frame

void free_children(int parent)
{
 int i=0;

 while(i<MAXFRAMES)
 {
  if(p->htmlframe[i].parent==parent)
  {
   if(p->htmlframe[parent].next==i)
    p->htmlframe[parent].next=p->htmlframe[i].next;
   p->htmlframe[i].framename[0]='\0';
   p->htmlframe[i].hidden=1;
   p->htmlframe[i].parent=-1;
  }
  i++;
 }
 p->htmlframe[parent].hidden=0;
}

void delete_children(int parent)
{
 int i=0;
 struct Url url;
 XSWAP writeadr;
 unsigned status;
 int found;

 while(i<MAXFRAMES)
 {
  if(p->htmlframe[i].parent==parent)
  {
   AnalyseURL(p->htmlframe[i].cacheitem.URL,&url,IGNORE_PARENT_FRAME);
   found=SearchInCache(&url,&(p->htmlframe[i].cacheitem),&writeadr,&status);
   if(found && status!=LOCAL)
    DeleteFromCache(writeadr);
  }
  i++;
 }//loop
}

//this will "allocate" free frame - should return value greater than 0

char findfreeframe(void)
{
 int i=1;

 while(i<MAXFRAMES)
 {
  if(p->htmlframe[i].parent==-1 && !p->htmlframe[i].framename[0])
   return i;
  i++;
 }
 return -1;
}


//=========================================================================
//end of frames implementation



void HTTPcharset(char *charset)
{
 char *my = config_get_str("AcceptCharset", "");
 while(*charset==' ')
  charset++;
 if(my && !strncmpi(charset,"charset=",8))
 {
  char *ptr=strchr(charset,'=');
  char *name;
  if(ptr)
   ptr++;  // ;charset=
  else
   return;
  name=ptr;
  RENDER.translatecharset=0;
  // werner scholz begin  Nov 8,2006  --- utf8 ---
//   RENDER.utf8=0;
//!!glennmcc: Apr 01, 2008 -- Werner's method was not 'catching' both upper & lower case
   if(strstr(charset,"f-8") || strstr(charset,"F-8")) RENDER.utf8=1;
//   if(strstr(charset,"utf-8")) RENDER.utf8=1;
//   if(strstr(charset,"UTF-8")) RENDER.utf8=1;
//Werner's original 2 lines above this comment
//!!glennmcc: end
//   if(RENDER.utf8!=0)
//    {if(strstr(my,"-8859-1")) RENDER.utf8=1;
//     if(strstr(my,"-8859-2")) RENDER.utf8=2;}
  // werner scholz end
  if(strncmpi(name,my,strlen(name)))  //!=
  {
   ptr=strchr(ptr,'-');
   if(ptr)
    ptr++;
   //;charset=windows-1250
   //.................^
   //;charset=iso-8859-2
   //.............^

   if(!strcmpi(ptr,GLOBAL.currentcharset))
    RENDER.translatecharset=1;
   else
   {
    char *fn = newstr("%scodepage%c%s.cp", fontpath, PATHSEP, ptr);
    int f;
#ifdef POSIX
    f = a_open(fn, O_RDONLY, S_IREAD);
#else
    f = a_sopen(fn, O_RDONLY | O_BINARY, SH_COMPAT, S_IREAD);
#endif
    freestr(fn);
    if(f!=-1)
    {
     if( a_read(f,GLOBAL.codepage,256) ==256)
     {
      RENDER.translatecharset=1;
      makestr(GLOBAL.currentcharset,ptr,8);
//      if(!strstr(ptr,"8859"))RENDER.utf8=0; // werner scholz Nov 8.2006
     }
     else
      GLOBAL.currentcharset[0]='\0';
     a_close(f);
    }
   }
  }
 }
}


void METAtag(void)
{
 char *tagarg;
 int MinRedirect;

 if(getvar("HTTP-EQUIV",&tagarg))
 {
  struct Url url;
  char text[URLSIZE+1];
  //JdS 2005/8/10 {
  //An implementation of glennmcc's "IgnoreRefresh" scheme ...
  if (!strcmpi(tagarg,"REFRESH") && !config_get_bool("IgnoreRefresh", 1))
  //JdS 2005/8/10 }
  {
   if(getvar("CONTENT",&tagarg))
   {
    char *ptr=strchr(tagarg,';');
    if(ptr)
    {
     *ptr='\0';
     GLOBAL.secondsleft=atoi(tagarg);
//JdS 2005/8/10 {
//Give the user enough time to stop "undesirable redirects" ...
     MinRedirect = config_get_int("ShortestRefresh", 2);
     if (GLOBAL.secondsleft < MinRedirect)
      GLOBAL.secondsleft = MinRedirect;
//JdS 2005/8/10 }
     //printf("%d",GLOBAL.secondsleft);
     ptr=strchr(&ptr[1],'=');
     if(ptr)
     {
      ptr++;
      if(ptr[0]!='#')
      {
       AnalyseURL(ptr,&url,p->currentframe); //(full length)
       url2str(&url,text);
       ptr=text;
      }
      makestr(GLOBAL.location,ptr,URLSIZE);
      GLOBAL.timeout=1;
      GLOBAL.refreshtarget=p->currentframe;
     }
    }
   }
  }
  else
  if(/* this depends whether HTTP header should have prioroty over META: !RENDER.translatecharset &&*/
      !strcmpi(tagarg,"CONTENT-TYPE"))
  /* real HTTP header or user-forced charset has higher priority */
  {
   if(getvar("CONTENT",&tagarg))
   {
    char *set=strstr(tagarg,"charset=");
//!!glennmcc: Apr 01, 2008
    if(!set) set=strstr(tagarg,"CHARSET=");
    if(set)
     HTTPcharset(set);
   }
  }
 }
}


void LINKtag(XSWAP *stylesheetadr)
{
 char *tagarg;
 struct HTMLrecord HTMLatom;

 if(user_interface.css && getvar("REL",&tagarg) && !strcmpi(tagarg,"STYLESHEET"))
 {

  if(getvar("HREF",&tagarg))
  {
   unsigned status;
   XSWAP dummy;
   struct HTTPrecord HTTPdoc;
   struct Url url;
   char text[URLSIZE];

   AnalyseURL(tagarg,&url,p->currentframe); //(full length)
   url2str(&url,text);
   if(strstr(text,"&amp;"))
    entity2str(text);
   HTMLatom.x=0;
   HTMLatom.y=0;
   HTMLatom.xx=0;
   HTMLatom.yy=0;
   addatom(&HTMLatom,text,strlen(text),STYLESHEET,BOTTOM,0,0,IE_NULL,1);

   if(QuickSearchInCache(&url,&HTTPdoc,&dummy,&status) && HTTPdoc.locname[0])
   {
    // not yet handled now: if(styleshhetadr!=IE_NULL) ...
    strcpy(tmpeditor.filename,HTTPdoc.locname);
//!!JdS 2006/02/25 {
//Fixed xSwap leakage, due to loading stylesheet files into the wrong context
//  if(ie_openf_lim(&tmpeditor,CONTEXT_TMP,8000)==1)
     if (ie_openf_lim(&tmpeditor,CONTEXT_HTML,8000)==1)
//!!JdS 2006/02/25 }
     *stylesheetadr=ie_putswap((char *)&tmpeditor,sizeof(struct ib_editor),CONTEXT_HTML);
   }
   else
    GLOBAL.needrender=1; //CSS not in cache
  }
 }
}

void FRAMEtag(int *emptyframeset,int *previousframe)
{
 char newframe_target;
 struct HTMLframe *frame;
 struct Url url;
 char text[URLSIZE];
 char *tagarg;

 if(!getvar("SRC",&tagarg))
  tagarg="NUL";
 AnalyseURL(tagarg,&url,p->currentframe); //(full length)
 url2str(&url,text);
//!!glennmcc: Nov 28, 2006 -- fix problems with '&amp;' in <frame src="
//this patch fixes loading of each frame when display of frames is enabled
	if(strstr(text,"&amp;"))
	 entity2str(text);
	tagarg=text;
//!!glennmcc: end

 newframe_target=*emptyframeset;
 frame=&(p->htmlframe[newframe_target]);

 *emptyframeset=frame->next;
 frame->next=p->htmlframe[*previousframe].next;
 p->htmlframe[*previousframe].next=newframe_target;
 *previousframe=newframe_target;


 if(newframe_target<MAXFRAMES-1 && newframe_target>arachne.framescount)
  arachne.framescount=newframe_target;

 text[URLSIZE-1]='\0';
 strcpy(frame->cacheitem.URL,text);
 if(getvar("NAME",&tagarg))
 {
  makestr(text,tagarg,FRAMENAMESIZE-1);
  strcpy(frame->framename,text);
 }

 if(! (frame->frameborder & I_WANT_FRAMEBORDER))
   frame->frameborder=DONT_WANT_FRAMEBORDER;
  else
   frame->frameborder=FRAMEBORDER_IS_ON;

 if(getvar("FRAMEBORDER",&tagarg) || getvar("BORDER",&tagarg))
 {
  if(tagarg[0]=='0' || toupper(tagarg[0])=='N' || toupper(tagarg[0])=='F')
   frame->frameborder=DONT_WANT_FRAMEBORDER;
  else
   frame->frameborder=FRAMEBORDER_IS_ON;
 }

 if(frame->frameborder==DONT_WANT_FRAMEBORDER)
 {
  frame->scroll.xsize+=2;
  frame->scroll.ymax+=2;
  frame->scroll.xtop-=1;
  frame->scroll.ytop-=1;
 }

 //else
 // if(frame->frameborder==0)
 // resetframeborder(frame,2);

 //MSIE (& Mozilla ?) extension are implemented here: ...........

 //frame BGCOLOR, etc. ?

 if(getvar("SCROLLING",&tagarg) && toupper(tagarg[0])=='N')
  frame->allowscrolling=0;
 else
 {
  frame->allowscrolling=1;
  frame->scroll.xsize-=user_interface.scrollbarsize;
 }

 if(getvar("MARGINWIDTH",&tagarg)) //Netscape 4.0 emulation
 {
  frame->marginheight=atoi(tagarg);
  frame->marginwidth=atoi(tagarg);
 }
 else
 {
  frame->marginwidth=HTMLBORDER;
  frame->marginheight=HTMLBORDER;
 }

 if(getvar("MARGINHEIGHT",&tagarg))
  frame->marginheight=atoi(tagarg);

 if(getvar("BORDER",&tagarg) || getvar("FRAMESPACING",&tagarg))
 {
  frame->marginheight=atoi(tagarg);
  frame->marginwidth=atoi(tagarg);
 }
 //............................... end of extensions ............

 frame->scroll.xvisible=0;
 frame->scroll.yvisible=0;
 ScrollInit(&frame->scroll,
	    frame->scroll.xsize,
            frame->scroll.ymax,   //visible y
	    frame->scroll.ymax,   //max y
	    frame->scroll.xtop,
	    frame->scroll.ytop,
            frame->scroll.xsize,0);//total x,y
 ResetHtmlPage(&(p->tmpframedata[newframe_target]),TEXT_HTML,1);

 frame->posX=0;
 frame->posY=0l;
}



void ResetHtmlPage(struct TMPframedata *html,char ishtml,char allowuser)
{
 char *ptr=NULL;
 if(ishtml)
 {
  if(allowuser) {
   ptr = config_get_str("HTMLbgColor", "#C4C4C4");
   try2readHTMLcolor(ptr,&html->backR,&html->backG,&html->backB);
  }

  if(allowuser) {
   ptr = config_get_str("HTMLtext", "#000000");
   try2readHTMLcolor(ptr,&html->textR,&html->textG,&html->textB);
  }

  if(allowuser) {
   ptr = config_get_str("HTMLlink", "#0000C4");
   try2readHTMLcolor(ptr,&html->linkR,&html->linkG,&html->linkB);
  }
 }
 else
 {
  int sw=(egamode && user_interface.paper!=0);
  html->backR=16*sw+(Iipal[3*user_interface.paper]<<2);
  html->backG=16*sw+(Iipal[3*user_interface.paper+1]<<2);
  html->backB=16*sw+(Iipal[3*user_interface.paper+2]<<2);

  sw=(egamode && user_interface.ink!=0);
  html->textR=16*sw+(Iipal[3*user_interface.ink]<<2);
  html->textG=16*sw+(Iipal[3*user_interface.ink+1]<<2);
  html->textB=16*sw+(Iipal[3*user_interface.ink+2]<<2);
 }

 html->backgroundptr=IE_NULL;
 html->bgproperties=BGPROPERTIES_SCROLL;

 html->basefontsize=3;    //default font size is 3
 html->tdfontsize=-1;     //do not apply !
 html->ahreffontsize=-1;  //do not apply !
 html->basefontstyle=0;   //default style
 html->tdfontstyle=-1;    //do not apply !
 html->ahrefsetbits=UNDERLINE;
 html->ahrefresetbits=0;
 html->hoversetbits=0;
 html->hoverresetbits=0;
 html->usetdcolor=0;      //do not use any special table font color
 html->usetdbgcolor=0;    //do not use any special table background color
// html->usetablecolor=0;      //do not use any special table font color
// html->usetablebgcolor=0;    //do not use any special table background color
 html->usehover=0;        //do not apply !
 html->name[0]='\0';
 //!!JdS 2006/03/02: Reversed this 2006/02/15 code change ...
 //This was originally one of two code changes to fix the frames/smiley
 //bug. However, as suggested by Glenn, this particular change was not
 //necessary and was causing side-effects. Re-testing seems to confirm
 //that the change in 'init.c' is sufficient, so this one is now out.
 // html->writeadr = IE_NULL;
 //!!JdS 2006/03/02 }
///*
//!!glennmcc -- thank you Joe... now my 'dirty little hack' is not needed ;-)
//*/
 html->nextsheet=IE_NULL;
 html->myadr=IE_NULL;
}

//.  <BODY ARACHNE>
void BodyArachne(struct TMPframedata *html)
{
  char *ptr;

  ptr = config_get_str("BgColor", "#000000");
  try2readHTMLcolor(ptr,&html->backR,&html->backG,&html->backB);

  ptr = config_get_str("Text", "#F0F0F0");
  try2readHTMLcolor(ptr,&html->textR,&html->textG,&html->textB);

  ptr = config_get_str("Link", "#00FF00");
  try2readHTMLcolor(ptr,&html->linkR,&html->linkG,&html->linkB);

 ptr = config_get_str("Background", "NUL");
 if(strcmpi(ptr,"NUL")) // ... ptr!="NUL"
 {
  struct picinfo *img=farmalloc(sizeof(struct picinfo));
  if(!img)
   memerr();

#ifdef POSIX
  makestr(img->URL,ptr,79);
#else
  strcpy(img->URL,"file://");
  makestr(&(img->URL[7]),ptr,79);
#endif
  if(img->URL[0])
  {
   struct HTMLrecord HTMLatom;
   HTMLatom.x=0;
   HTMLatom.xx=0;
   HTMLatom.y=0;
   HTMLatom.yy=0;
   addatom(&HTMLatom,img,sizeof(struct picinfo),BACKGROUND,BOTTOM,0,0,IE_NULL,0);
   html->backgroundptr=p->lastHTMLatom;
  }
  farfree(img);
 }

}


//global structure for HTML rendering - renderHTML() and more...
//in future all HTML rendering data will be moved there.
struct RENDER_DATA RENDER;

void appendline(XSWAP currenttextarea,char *text,int gotothere)
{
 editorptr=(struct ib_editor *)ie_getswap(currenttextarea);
 if(editorptr)
 {
  if(!*text && !editorptr->lines)
   return;
  memcpy(&tmpeditor,editorptr,sizeof(struct ib_editor));

  ie_insline(&tmpeditor,tmpeditor.lines,text);

  if(gotothere)
   tmpeditor.y=tmpeditor.lines-1;
  editorptr=(struct ib_editor *)ie_getswap(currenttextarea);
  if(editorptr)
  {
   memcpy(editorptr,&tmpeditor,sizeof(struct ib_editor));
   swapmod=1;
  }
 }
}


void verticalalign(XSWAP adr,XSWAP tbladr,char valign,long yshift)
{
 if(valign==TOP || adr==IE_NULL ||
     GLOBAL.validtables==TABLES_UNKNOWN && RENDER.willadjusttables)
  return;

 {
  struct HTMLrecord *cellatom;

  if(valign==MIDDLE)
  {
   yshift=yshift/2-1;
   if(yshift<=0)
    return;
  }

  do
  {
   cellatom=(struct HTMLrecord *)ie_getswap(adr);
   if(!cellatom)
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return;
//    MALLOCERR();
//!!glennmcc: end
   if((cellatom->type==TD || cellatom->type==TD_BACKGROUND) &&
      cellatom->linkptr==tbladr) //cell boundary atom
    return;

   cellatom->y+=yshift;
   cellatom->yy+=yshift;
   swapmod=1;

   adr=cellatom->next;
  }//loop
  while(adr!=IE_NULL);
 }
}

//!rowspan fix!
void closeatom_y(XSWAP adr,long absy,int padding)
{
 if(adr!=IE_NULL)
 {
  struct HTMLrecord *atomptr=(struct HTMLrecord *)ie_getswap(adr);
  if(atomptr)
  {
   if(absy>atomptr->yy)
   {
    long oldyy=atomptr->yy;
    atomptr->yy=absy;
    swapmod=1;
    if(atomptr->type==TD || atomptr->type==TD_BACKGROUND)
     verticalalign(atomptr->next,atomptr->linkptr,atomptr->align,absy-padding-oldyy);
   }
  }
  else
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return;
//   MALLOCERR();
//!!glennmcc: end
 }
}//end if
//!rowspan end!

int atom2nextline(int x, long y, XSWAP adr)
{
 if(adr!=IE_NULL)
 {
  struct HTMLrecord *atomptr=(struct HTMLrecord *)ie_getswap(adr);
  if(atomptr)
  {
   int height=(int)(atomptr->yy-atomptr->y);
   atomptr->xx=atomptr->x=x;
   atomptr->y=y;
   atomptr->yy=atomptr->y+height;
   swapmod=1;
   return height;
  }
 }
 return 0;
}


void tablerow(long y,long yy,XSWAP tbladr,int padding)
{
 XSWAP currentHTMLatom=p->lastHTMLatom;
 struct HTMLrecord *atomptr;

 do
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
  currentHTMLatom=atomptr->prev;
  if(atomptr->y==y && (atomptr->type==TD || atomptr->type==TD_BACKGROUND)
     && atomptr->linkptr==tbladr)
  {
   if(atomptr->yy<yy)
   {
    long oldyy=atomptr->yy;
    atomptr->yy=yy;
    swapmod=1;
    verticalalign(atomptr->next,atomptr->linkptr,atomptr->align,yy-padding-oldyy);
   }
  }
  if(currentHTMLatom==tbladr) //cells can't be located before TABLE!
   break;
 }
 while(currentHTMLatom!=IE_NULL && atomptr->yy>=y);
}

// FASTDEALLOC is no more needed...

void Deallocmem(void)
{
 XSWAP currentHTMLatom=p->lastHTMLatom;
 struct HTMLrecord *atomptr;

 if(p->HTMLatomcounter==0)
  return;
 mouseoff();

#ifdef XANIMGIF
 XResetAnimGif();
#endif

 while(currentHTMLatom!=IE_NULL)
 {
//  kbhit();

  atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if(atomptr)
  {
   currentHTMLatom=atomptr->prev;
   if(atomptr->type==INPUT)
   //dealokovat pripadny vyskyt ibase editoru:
   // tr.: deallocate if the ibase editor occurs:
   {
    editorptr=(struct ib_editor *)ie_getswap(atomptr->ptr);
    if(editorptr)
    {
     if(editorptr->modified && atomptr->data1==TEXTAREA)
     {
      memcpy(&tmpeditor,editorptr,sizeof(struct ib_editor));
      sprintf(tmpeditor.filename, "%s%s", userpath, "textarea.tmp");
      ie_savef(&tmpeditor); //modifying xswap, editorptr is no longer valid
      editorptr=&tmpeditor;
     }
//old:     if(editorptr->lineadr)
//      farfree(editorptr->lineadr);
    }
   }
  }
  else
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return;
//   MALLOCERR();
//!!glennmcc: end
 }//loop

 ie_killcontext(CONTEXT_HTML);

 p->HTMLatomcounter=0;
 p->firstonscr=p->lastonscr=IE_NULL;
 p->firstHTMLatom=p->lastHTMLatom=IE_NULL;
 p->memory_overflow=0;
 mouseon();
//!!Bernie: begin July 6, 2002...
// prevents memory allocation errors in atoms.c caused by
// the addition of Deallocmem() to main.c
activeatomptr=NULL;
//!!Bernie: end
}

//!!glennmcc: Mar 12, 2007 -- quick-n-dirty hack to support <iframe
//test page .... http://www.auschess.org.au/
void DummyFrame(struct Page *p,int *x, long *y, int tag)
//void DummyFrame(struct Page *p,int *x, long *y)//originl line
{
 unsigned currentlink;
 char *tagarg;
 struct HTMLrecord HTMLatom;
 struct Url url;
 char str[URLSIZE+1];
 struct picinfo *img=farmalloc(sizeof(struct picinfo));
// int tag;//for Iframe hack
 if(!img)
  memerr();

 if(getvar("SRC",&tagarg))
 {
  HTMLatom.x=*x;
  HTMLatom.y=*y;
  HTMLatom.xx=*x;
  HTMLatom.yy=*y+p->sizeRow;
  //vlozit link:
  AnalyseURL(tagarg,&url,p->currentframe); //(full length)
  url2str(&url,str);
//!!glennmcc: Nov 29, 2006 -- fix problems with '&amp;' in <frame src="
//this patch fixes the links to each frame when display of frames is disabled
	if(strstr(str,"&amp;"))
	 entity2str(str);
	tagarg=str;
//!!glennmcc: end

  //vyrobim si pointr na link, a od ted je vsechno link:
  // tr.: I create a pointer to the link, and from now on everything is link
  addatom(&HTMLatom,str,strlen(str),HREF,BOTTOM,0,0,IE_NULL,1);
  currentlink=p->lastHTMLatom;

  getvar("NAME",&tagarg);
  strcat(str," (");
  strcat(str,tagarg);
  strcat(str,")");

//  pushfont(font,style,&HTMLatom,&fontstack);
  HTMLatom.R=p->tmpframedata[p->currentframe].linkR;
  HTMLatom.G=p->tmpframedata[p->currentframe].linkG;
  HTMLatom.B=p->tmpframedata[p->currentframe].linkB;
  img->size_y=60;
  img->size_x=60;
//!!glennmcc: Mar 12, 2007 -- quick-n-dirty hack to support <iframe
//test page .... http://www.auschess.org.au/
  if (tag==TAG_IFRAME)
  strcpy(img->filename,"IFRAME.IKN");//new Ikon
  else
  strcpy(img->filename,"HTM.IKN");//original line
//!!glennmcc: end
  img->URL[0]='\0';
  HTMLatom.x=*x;
  HTMLatom.y=*y;
  *x+=img->size_x;
  HTMLatom.xx=*x;
  HTMLatom.yy=*y+img->size_y;
  addatom(&HTMLatom,img,sizeof(struct picinfo),IMG,BOTTOM,0,0,currentlink,0);
//!!glennmcc: Mar 12, 2007 -- quick-n-dirty hack to support <iframe
//test page .... http://www.auschess.org.au/
  if(tag!=TAG_IFRAME)
{//for Iframe hack
  HTMLatom.x=*x+FUZZYPIX;
  HTMLatom.xx=p->docRight;
  HTMLatom.yy=*y+fonty(3,UNDERLINE);
  addatom(&HTMLatom,str,strlen(str),TEXT,BOTTOM,3,UNDERLINE,currentlink,0);
  *y+=img->size_y;
}//for Iframe hack
//  if(!popfont(&font,&style,&HTMLatom,&fontstack))
//  {
   HTMLatom.R=p->tmpframedata[p->currentframe].textR;
   HTMLatom.G=p->tmpframedata[p->currentframe].textG;
   HTMLatom.B=p->tmpframedata[p->currentframe].textB;
//  }
 }

 farfree(img);
}


void CheckArachneFormExtensions(struct HTTPrecord *cache,char *value, int *checked)
{
 char *tagarg, *ptr;

 if(getvar("ARACHNECFGVALUE",&tagarg))
 {
  ptr = config_get_str(tagarg, NULL);
  if(ptr)
   makestr(value,ptr,IE_MAXLEN);
 }

 if(getvar("ARACHNESAVE",&tagarg))
 {
  ptr = config_get_str("DownloadPath", "DOWNLOAD\\");
  makestr(value,ptr,79);
  ptr=strrchr(cache->URL,'\\');
  if(!ptr)
  {
   ptr=strrchr(cache->URL,'/');
   if(!ptr)
   {
    if(cache->rawname[0])
    {
     ptr=strrchr(cache->rawname,'\\');
     if(!ptr)
      ptr=cache->rawname;
     else
      ptr++;
    }
    else
    {
     ptr=strrchr(cache->locname,'\\');
     if(!ptr)
      ptr=cache->locname;
     else
      ptr++;
    }
   }
   else
    ptr++; //skip backslash
  }
  else
   ptr++; //skip slash

  strcat(value,ptr);
 }

 if(getvar("ARACHNEMIME",&tagarg))
  strcpy(value,cache->mime);

 if(getvar("ARACHNEREALM",&tagarg))
  strcpy(value,AUTHENTICATION->realm);

 if(getvar("ARACHNEVER",&tagarg))
 {
  strcpy(value,VER);
  strcat(value,beta);
 }

 if(getvar("ARACHNEDOC",&tagarg))
 {
  if(file_exists(cache->rawname))
   strcpy(value,cache->rawname); //rawname is not virtual - .JPG,.CNM
  else
   strcpy(value,LASTlocname); //rawname is not filename - .DGI
  strlwr(value);
 }

 if(getvar("ARACHNECHECKED",&tagarg)) //for VALUE=...
 {
  ptr = config_get_str(tagarg, NULL);
  if(ptr && !strcmpi(value,ptr))
   *checked=1;
 }

 if(getvar("ARACHNEVGA",&tagarg) && vgadetected) //for VALUE=...
 {
  strcpy(value,vgadetected);
 }

 if(getvar("ARACHNENOTCHECKED",&tagarg)) //FOR ARACHNECFGVALUE=...
 {
  if(!strstr(tagarg,value))
   *checked=1;
 }

 if(getvar("ARACHNECFGHIDE",&tagarg)) //FOR ARACHNECFGVALUE=...
 {
  if(strstr(tagarg,value))
   value[0]='\0';
 }
}//end unsecure extensions.......................................


struct TMPframedata *locatesheet_ovrl(struct TMPframedata *rootsheet, struct TMPframedata *tmpsheet,XSWAP stylesheetadr)
{
 XSWAP sheetadr=IE_NULL,newsheetadr;
 struct TMPframedata *thissheet;
 char *tagarg;

 if(stylesheetadr!=IE_NULL && getvar("CLASS",&tagarg))
 {
  thissheet=rootsheet;
  while(thissheet->nextsheet!=IE_NULL)
  {
   sheetadr=thissheet->nextsheet;
   thissheet=(struct TMPframedata *)ie_getswap(sheetadr);
   if(thissheet)
   {
    if(!strcmpi(thissheet->name,tagarg))
    {
     memcpy(tmpsheet,thissheet,sizeof(struct TMPframedata));
     tmpsheet->myadr=sheetadr;
     return tmpsheet;
    }
   }
   else
    return rootsheet;
  }

  memcpy(tmpsheet,rootsheet,sizeof(struct TMPframedata));
  ParseCSS(tmpsheet,stylesheetadr,tagarg);
  tmpsheet->nextsheet=IE_NULL;
  makestr(tmpsheet->name,tagarg,STRINGSIZE);
  newsheetadr=ie_putswap((char *)tmpsheet,sizeof(struct TMPframedata),CONTEXT_HTML);
  tmpsheet->myadr=newsheetadr;
  if(sheetadr==IE_NULL)
   rootsheet->nextsheet=newsheetadr;
  else
  {
   thissheet=(struct TMPframedata *)ie_getswap(sheetadr);
   if(thissheet)
   {
    thissheet->nextsheet=newsheetadr;
    swapmod=1;
   }
  }
  return tmpsheet;
 }
 else
  return rootsheet;
}

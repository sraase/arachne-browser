
// ========================================================================
// HTML rendering routines for Arachne WWW browser
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "html.h"
#include "xanimgif.h"

//start of frames
//============================================================================
void addframeset(char xflag, char *emptyframe, char framewantborder)
{
 char *startptr=text,*endptr;
 int velikost[MAXFRAMES];
 struct ScrollBar *from;
 char fromframe,newframecount=0,border=1,starcount=0;
 int vsechno,zbytek,i;
 char prevframe,newframe=findfreeframe();

 if(newframe>=MAXFRAMES-1 || newframe<1)
  return;

 if(*emptyframe==-1) //we are adding first empty frame
  fromframe=currentframe;
 else
  fromframe=*emptyframe;

 *emptyframe=newframe;

 if(fromframe && htmlframe[fromframe].frameborder)
  border=0;

 if(framewantborder==UNDEFINED_FRAMEBORDER)
 {
  if(fromframe)
   framewantborder=htmlframe[fromframe].frameborder & I_WANT_FRAMEBORDER;
  else
   framewantborder=I_WANT_FRAMEBORDER;
 }

 memset(velikost,0,2*MAXFRAMES);
 from=&htmlframe[fromframe].scroll;

 if(xflag)
 {
  vsechno=zbytek=from->xsize+1-border;
  if(htmlframe[fromframe].allowscrolling)
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

  if(*startptr=='*')
  {
   velikost[newframecount]=-1;
   starcount++;
  }
  else
  {
   velikost[newframecount]=try2getnum(startptr,vsechno);
   /*
   if(!strchr(startptr,'%') && xflag)
    velikost[newframecount]+=user_interface.scrollbarsize;
    */

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
  tmpframedata[newframe].whichvirtual=newframe-1; //let's overwrite screen 0
  htmlframe[newframe].parent=currentframe;
  htmlframe[newframe].hidden=0;
  htmlframe[newframe].allowscrolling=0;
  htmlframe[newframe].frameborder=FRAMEBORDER_IS_ON | framewantborder; //was: =1;
  if(xflag)
  {
   htmlframe[newframe].scroll.xsize=velikost[i]/*-user_interface.scrollbarsize*/-3;
   htmlframe[newframe].scroll.ymax=from->ymax-2*border;
   htmlframe[newframe].scroll.xtop=from->xtop+zbytek+border;
   htmlframe[newframe].scroll.ytop=from->ytop+border;
  }
  else
  {
   htmlframe[newframe].scroll.xsize=from->xsize-2*border;
   if(htmlframe[fromframe].allowscrolling)
    htmlframe[newframe].scroll.xsize+=user_interface.scrollbarsize;
   htmlframe[newframe].scroll.ymax=velikost[i]-3; // 3 - reserver for neighbour
   htmlframe[newframe].scroll.xtop=from->xtop+border;
   htmlframe[newframe].scroll.ytop=from->ytop+zbytek+border;
  }

  zbytek+=velikost[i++];
  prevframe=newframe;
  newframe=findfreeframe();
  htmlframe[prevframe].next=newframe;
 }
 htmlframe[prevframe].next=htmlframe[fromframe].next;

 if(newframe==-1) //cannot insert new frame - error!
 {
  *emptyframe=-1;
  return;
 }

 htmlframe[fromframe].hidden=1;

 if(xflag)
 {
  htmlframe[prevframe].scroll.xsize=
   from->xsize-htmlframe[prevframe].scroll.xtop+from->xtop-border;
  if(htmlframe[fromframe].allowscrolling)
   htmlframe[prevframe].scroll.xsize+=user_interface.scrollbarsize;
 }
 else
  htmlframe[prevframe].scroll.ymax=
   from->ymax-htmlframe[prevframe].scroll.ytop+from->ytop-border;

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

void free_children(char parent)
{
 int i=0;

 while(i<MAXFRAMES)
 {
  if(htmlframe[i].parent==parent)
  {
   if(htmlframe[parent].next==i)
    htmlframe[parent].next=htmlframe[i].next;
   htmlframe[i].framename[0]='\0';
   htmlframe[i].hidden=1;
   htmlframe[i].parent=-1;
  }
  i++;
 }
 htmlframe[parent].hidden=0;
}

void delete_children(char parent)
{
 int i=0;
 struct Url url;
 XSWAP writeadr;
 unsigned status;
 char found;

 while(i<MAXFRAMES)
 {
  if(htmlframe[i].parent==parent)
  {
   AnalyseURL(htmlframe[i].cacheitem.URL,&url,IGNORE_PARENT_FRAME);
   found=SearchInCache(&url,&htmlframe[i].cacheitem,&writeadr,&status);
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
  if(htmlframe[i].parent==-1 && !htmlframe[i].framename[0])
   return i;
  i++;
 }
 return -1;
}


//=========================================================================
//end of frames implementation



void HTTPcharset(char *charset)
{
 char *my=configvariable(&ARACHNEcfg,"AcceptCharset",NULL);
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
    char fn[80];
    int f;
#ifdef POSIX
    sprintf(fn,"%scodepages/%s.cp",fntpath,ptr);
    f=a_open(fn,O_RDONLY, S_IREAD);
#else
    sprintf(fn,"%s\\system\\codepage\\%s.cp",exepath,ptr);
    f=a_sopen(fn,O_RDONLY|O_BINARY,SH_COMPAT, S_IREAD);
#endif
    if(f!=-1)
    {
     if( a_read(f,GLOBAL.codepage,256) ==256)
     {
      RENDER.translatecharset=1;
      makestr(GLOBAL.currentcharset,ptr,8);
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

 if(getvar("HTTP-EQUIV",&tagarg))
 {
  struct Url url;
  if(!strcmpi(tagarg,"REFRESH"))
  {
   if(getvar("CONTENT",&tagarg))
   {
    char *ptr=strchr(tagarg,';');
    if(ptr)
    {
     *ptr='\0';
     GLOBAL.secondsleft=atoi(tagarg);
     //printf("%d",GLOBAL.secondsleft);
     ptr=strchr(&ptr[1],'=');
     if(ptr)
     {
      ptr++;
      if(ptr[0]!='#')
      {
       AnalyseURL(ptr,&url,currentframe); //(plne zneni...)
       url2str(&url,text);
       ptr=text;
      }
      makestr(GLOBAL.location,ptr,URLSIZE);
      GLOBAL.timeout=1;
      GLOBAL.refreshtarget=currentframe;
     }
    }
   }
  }
  else
  if(!RENDER.translatecharset && !strcmpi(tagarg,"CONTENT-TYPE")) 
  /* real HTTP header or user-forced charset has higher priority */
  {
   if(getvar("CONTENT",&tagarg))
   {
    char *set=strstr(tagarg,"charset=");
    if(set)
     HTTPcharset(set);
   }
  }
 }
}

void ResetHtmlPage(struct TMPframedata *html,char ishtml,char allowuser)
{
 char *ptr=NULL;
 if(ishtml)
 {
  if(allowuser)
   ptr=configvariable(&ARACHNEcfg,"HTMLbgColor",NULL);
  if(ptr)
   try2readHTMLcolor(ptr,&html->backR,&html->backG,&html->backB);
  else
  {
   html->backR=196;
   html->backG=196;
   html->backB=196;
  }
  if(allowuser)
   ptr=configvariable(&ARACHNEcfg,"HTMLtext",NULL);
  if(ptr)
   try2readHTMLcolor(ptr,&html->textR,&html->textG,&html->textB);
  else
  {
   html->textR=0;
   html->textG=0;
   html->textB=0;
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
 if(allowuser)
  ptr=configvariable(&ARACHNEcfg,"HTMLlink",NULL);
 if(ptr)
  try2readHTMLcolor(ptr,&html->linkR,&html->linkG,&html->linkB);
 else
 {
  html->linkR=0;
  html->linkG=0;
  html->linkB=196;
 }
//  html->vlinkR=10;
//  html->vlinkG=10;
//  html->vlinkB=130;
 html->backgroundptr=IE_NULL;
// html->scroll.xvisible=0;
 html->bgproperties=BGPROPERTIES_SCROLL;
}

//.  <BODY ARACHNE>
void BodyArachne(struct TMPframedata *html)
{
 char *ptr=configvariable(&ARACHNEcfg,"BgColor",NULL);
 if(ptr)
  try2readHTMLcolor(ptr,&html->backR,&html->backG,&html->backB);
 ptr=configvariable(&ARACHNEcfg,"Text",NULL);
 if(ptr)
  try2readHTMLcolor(ptr,&html->textR,&html->textG,&html->textB);
 ptr=configvariable(&ARACHNEcfg,"Link",NULL);
 if(ptr)
  try2readHTMLcolor(ptr,&html->linkR,&html->linkG,&html->linkB);

 ptr=configvariable(&ARACHNEcfg,"Background",NULL);
 if(ptr && strcmpi(ptr,"NUL")) // ... ptr!="NUL"
 {
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
   html->backgroundptr=lastHTMLatom;
  }
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
    MALLOCERR();
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
void closeatom_y(XSWAP adr,long absy)
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
     verticalalign(atomptr->next,atomptr->linkptr,atomptr->align,absy-oldyy);
   }
  }
  else
   MALLOCERR();
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


void tablerow(long y,long yy,XSWAP tbladr)
{
 XSWAP currentHTMLatom=lastHTMLatom;
 struct HTMLrecord *atomptr;

 do
 {
  kbhit();
  atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if(!atomptr)
   MALLOCERR();
  currentHTMLatom=atomptr->prev;
  if(atomptr->y==y && (atomptr->type==TD || atomptr->type==TD_BACKGROUND)
     && atomptr->linkptr==tbladr)
  {
   if(atomptr->yy<yy)
   {
    long oldyy=atomptr->yy;
    atomptr->yy=yy;
    swapmod=1;
    verticalalign(atomptr->next,atomptr->linkptr,atomptr->align,yy-oldyy);
   }
  }
  if(currentHTMLatom==tbladr) //cells can't be located before TABLE!
   break;
 }
 while(currentHTMLatom!=IE_NULL && atomptr->yy>=y);
}


#ifdef JAVASCRIPT

void addjsevents(int tag)
{



}

#endif


// FASTDEALLOC is no more needed...

void Deallocmem(void)
{
 XSWAP currentHTMLatom=lastHTMLatom;
 struct HTMLrecord *atomptr;

 if(!HTMLatomcounter)
  return;
 mouseoff();

#ifdef XANIMGIF
 XResetAnimGif();
#endif

 while(currentHTMLatom!=IE_NULL)
 {
  kbhit();

  atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if(atomptr)
  {
   currentHTMLatom=atomptr->prev;
   if(atomptr->type==INPUT)
   //dealokovat pripadny vyskyt ibase editoru:
   {
    editorptr=(struct ib_editor *)ie_getswap(atomptr->ptr);
    if(editorptr)
    {
     if(editorptr->modified && atomptr->data1==TEXTAREA)
     {
      memcpy(&tmpeditor,editorptr,sizeof(struct ib_editor));
#ifdef POSIX
      strcpy(tmpeditor.filename,dotarachne);
      strcat(tmpeditor.filename,"textarea.tmp");
#else
      strcpy(tmpeditor.filename,"textarea.tmp");
#endif
      ie_savef(&tmpeditor); //modifying xswap, editorptr is no longer valid
      editorptr=&tmpeditor;
     }
     if(editorptr->lineadr)
      farfree(editorptr->lineadr);
    }
   }
  }
  else
   MALLOCERR();
 }//loop

 ie_killcontext(CONTEXT_HTML);

 HTMLatomcounter=0;
 firstonscr=lastonscr=IE_NULL;
 firstHTMLatom=lastHTMLatom=IE_NULL;
 memory_overflow=0;
 mouseon();
}



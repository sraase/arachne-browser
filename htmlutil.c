
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

  if(*startptr=='*')
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
       AnalyseURL(ptr,&url,p->currentframe); //(plne zneni...)
       url2str(&url,p->text);
       ptr=p->text;
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
  struct picinfo *img=p->img;
 
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
 XSWAP currentHTMLatom=p->lastHTMLatom;
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

 p->HTMLatomcounter=0;
 p->firstonscr=p->lastonscr=IE_NULL;
 p->firstHTMLatom=p->lastHTMLatom=IE_NULL;
 p->memory_overflow=0;
 mouseon();
}


void DummyFrame(struct Page *p,int *x, long *y)
{
 unsigned currentlink;
 char *tagarg;
 struct HTMLrecord HTMLatom;
 struct Url url;
 char str[URLSIZE+1];

 if(getvar("SRC",&tagarg))
 {
  HTMLatom.x=*x;
  HTMLatom.y=*y;
  HTMLatom.xx=*x;
  HTMLatom.yy=*y+p->sizeRow;
  //vlozit link:
  AnalyseURL(tagarg,&url,p->currentframe); //(plne zneni...)
  url2str(&url,str);

  //vyrobim si pointr na link, a od ted je vsechno link:
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
  p->img->size_y=60;
  p->img->size_x=60;
  strcpy(p->img->filename,"HTM.IKN");
  p->img->URL[0]='\0';
  HTMLatom.x=*x;
  HTMLatom.y=*y;
  *x+=p->img->size_x;
  HTMLatom.xx=*x;
  HTMLatom.yy=*y+p->img->size_y;
  addatom(&HTMLatom,p->img,sizeof(struct picinfo),IMG,BOTTOM,0,0,currentlink,0);
  HTMLatom.x=*x+FUZZYPIX;
  HTMLatom.xx=p->docRight;
  HTMLatom.yy=*y+fonty(3,UNDERLINE);
  addatom(&HTMLatom,str,strlen(str),TEXT,BOTTOM,3,UNDERLINE,currentlink,0);
  *y+=p->img->size_y;
//  if(!popfont(&font,&style,&HTMLatom,&fontstack))
//  {
   HTMLatom.R=p->tmpframedata[p->currentframe].textR;
   HTMLatom.G=p->tmpframedata[p->currentframe].textG;
   HTMLatom.B=p->tmpframedata[p->currentframe].textB;
//  }
 }

}


void CheckArachneFormExtensions(struct HTTPrecord *cache,char *value, int *checked)
{
 char *tagarg, *ptr;

 if(getvar("ARACHNECFGVALUE",&tagarg))
 {
  ptr=configvariable(&ARACHNEcfg,tagarg,NULL);
  if(ptr)
   makestr(value,ptr,IE_MAXLEN);
 }

 if(getvar("ARACHNESAVE",&tagarg))
 {
  ptr=configvariable(&ARACHNEcfg,"DownloadPath",NULL);
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
  ptr=configvariable(&ARACHNEcfg,tagarg,NULL);
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


// ========================================================================
// HTML rendering routines for Arachne WWW browser, statical part
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "html.h"
#include "xanimgif.h"

// ========================================================================
// Fast arguments for HTML tags.
// ========================================================================


// getvar() reads value of variable of given name
int getvar(char *name,char **value)
{
 int i=0,nameidx=0,valueidx=0;

 while(i<argnamecount)
 {
  if(!strcmpi(argnamestr+nameidx+sizeof(int),name))
  {
   *value=argvaluestr+valueidx+sizeof(int);
   return 1;
  }

  nameidx+=*(int*)(&argnamestr[nameidx]);
  valueidx+=*(int*)(&argvaluestr[valueidx]);
  i++;
 }

 return 0;
}

int searchvar(char *name)
{
 int i=0,nameidx=0;
 int l=strlen(name);

 while(i<argnamecount)
 {
  if(!strncmpi(argnamestr+nameidx+sizeof(int),name,l))
   return 1;
  nameidx+=*(int*)(&argnamestr[nameidx]);
  i++;
 }
 return 0;
}

// putvarname() sets new variable name
void putvarname(char *name,int size)
{
 int idx=0,i=argnamecount;

 while(i--)
 {
  idx+=*(int*)(&argnamestr[idx]);
 }//loop

 size+=1+sizeof(int);
 if(idx+size<MAXARGNAMES)
 {
  memcpy(argnamestr+idx,&size,sizeof(int));
  memcpy(argnamestr+idx+sizeof(int),name,size-1-sizeof(int));
  argnamestr[idx+size-1]='\0';
  argnamecount++;
 }
}

// putvarname() sets new variable value (putvarname must be called first)
void putvarvalue(char *value,int size)
{
 int idx=0,i=argvaluecount;

 while(i--)
 {
  idx+=*(int*)(&argvaluestr[idx]);
 }//loop

 size+=1+sizeof(int);
 if(idx+size<BUF/4)
 {
  memcpy(argvaluestr+idx,&size,sizeof(int));
  memcpy(argvaluestr+idx+sizeof(int),value,size-1-sizeof(int));
  argvaluestr[idx+size-1]='\0';
  argvaluecount++;
 }

}

//converts hexadecimal character to decimal value
char hexdigit(char c)
{
 if(c>='0' && c<='9') return c-'0';
 if(c>='A' && c<='F') return 10+c-'A';
 return 0;
}

//try2readHTMLcolor reads RGB color in HTML format "#RRGGBB" - supports also 
//16 basic color names acording to HTML/4.0 specification.

void try2readHTMLcolor(char *str,unsigned char *r,unsigned char *g,unsigned char *b)
{
 char string[7]="0000000";
 strupr(str);

 if(*str=='#')
 {
  strncpy(string,&str[1],6);
  goto resolve;
 }
 else
 if(strstr(str,"BLACK"))
  {*r=0;*g=0;*b=0;}
 else
 if(strstr(str,"GREEN"))
  {*r=0;*g=0x80;*b=0;}
 else
 if(strstr(str,"SILVER"))
  {*r=0xC0;*g=0xC0;*b=0xC0;}
 else
 if(strstr(str,"LIME"))
  {*r=0;*g=0xFF;*b=0;}
 else
 if(strstr(str,"GRAY"))
  {*r=0x80;*g=0x80;*b=0x80;}
 else
 if(strstr(str,"OLIVE"))
  {*r=0x80;*g=0x80;*b=0;}
 else
 if(strstr(str,"WHITE"))
  {*r=0xFF;*g=0xFF;*b=0xFF;}
 else
 if(strstr(str,"YELLOW"))
  {*r=0xFF;*g=0xFF;*b=0;}
 else
 if(strstr(str,"MAROON"))
  {*r=0x80;*g=0;*b=0;}
 else
 if(strstr(str,"NAVY"))
  {*r=0;*g=0;*b=0x80;}
 else
 if(strstr(str,"RED"))
  {*r=0xFF;*g=0;*b=0;}
 else
 if(strstr(str,"BLUE"))
  {*r=0;*g=0;*b=0xFF;}
 else
 if(strstr(str,"PURPLE"))
  {*r=0x80;*g=0;*b=0x80;}
 else
 if(strstr(str,"TEAL"))
  {*r=0;*g=0x80;*b=0x80;}
 else
 if(strstr(str,"FUCHSIA"))
  {*r=0xFF;*g=0;*b=0xFF;}
 else
 if(strstr(str,"AQUA"))
  {*r=0;*g=0xFF;*b=0xFF;}
 else
 {
  strncpy(string,str,6);
  resolve:
  *r=16*hexdigit(string[0])+hexdigit(string[1]);
  *g=16*hexdigit(string[2])+hexdigit(string[3]);
  *b=16*hexdigit(string[4])+hexdigit(string[5]);
 }
}

//try2getnum converts HTML object metrics to number: pixels or percents
int try2getnum(char *str,unsigned proczaklad)
{
 int l;

 l=strlen(str);
 if(str[l-1]=='%')
 {
  str[l-1]='\0';
  return (int)( (long)((long)atoi(str)*(long)proczaklad) /100 );
 }
 else
 {
  return atoi(str);
 }
}

// ========================================================================
// fast HTML tag analysis
// ========================================================================

int FastTagDetect(char *tagname)
{
 char *ptr=&tagname[1];

 tagname[0]=toupper(tagname[0]);
 switch(tagname[0])
 {
  case '/':
  return 1000+FastTagDetect(ptr);

  case 'A':   //most frequent tag: <A HREF=...>
  if(!*ptr)
   return TAG_A;
  strupr(ptr);
  if(!strcmp(ptr,"REA"))
   return TAG_AREA;
  if(!strcmp(ptr,"DDRESS"))
   return TAG_I;
  if(!strcmp(ptr,"RACHNE"))
   return TAG_ARACHNE_BONUS;
  break;

  case 'T':   //second most frequend tag: <TABLE>, <TD>, etc.
  strupr(ptr);
  if(!strcmp(ptr,"D"))
   return TAG_TD;
  if(!strcmp(ptr,"R"))
   return TAG_TR;
  if(!strcmp(ptr,"H"))
   return TAG_TH;
  if(!strcmp(ptr,"T"))
   return TAG_TT;
  if(!strcmp(ptr,"ABLE"))
   return TAG_TABLE;
  if(!strcmp(ptr,"ITLE"))
   return TAG_TITLE;
  if(!strcmp(ptr,"EXTAREA"))
   return TAG_TEXTAREA;
  break;

  case 'B':
  if(!*ptr)
   return TAG_B;
  strupr(ptr);
  if(!strcmp(ptr,"R"))
   return TAG_BR;
  if(!strcmp(ptr,"IG"))
   return TAG_BIG;
  if(!strcmp(ptr,"ODY"))
   return TAG_BODY;
  if(!strcmp(ptr,"ASE"))
   return TAG_BASE;
  if(!strcmp(ptr,"ASEFONT"))
   return TAG_BASEFONT;
  if(!strcmp(ptr,"LOCKQUOTE"))
   return TAG_BLOCKQUOTE;
  if(!strcmp(ptr,"UTTON"))
   return TAG_BUTTON;
  break;

  case 'C':
  strupr(ptr);
  if(!strcmp(ptr,"ENTER"))
   return TAG_CENTER;
  if(!strcmp(ptr,"APTION"))
   return TAG_CAPTION;
  if(!strcmp(ptr,"ODE"))
   return TAG_CODE;
  if(!strcmp(ptr,"ITE"))
   return TAG_I;
  break;

  case 'D':
  strupr(ptr);
  if(!strcmp(ptr,"T"))
   return TAG_LI;
  if(!strcmp(ptr,"D"))
   return TAG_DD;
  if(!strcmp(ptr,"L") || !strcmp(ptr,"IR"))
   return TAG_OL;
  if(!strcmp(ptr,"IV"))
   return TAG_DIV;
  break;

  case 'E':
  strupr(ptr);
  if(!strcmp(ptr,"M"))
   return TAG_B;
  if(!strcmp(ptr,"MBED"))
   return TAG_EMBED;
  break;

  case 'F':
  strupr(ptr);
  if(!strcmp(ptr,"ONT"))
   return TAG_FONT;
  if(!strcmp(ptr,"ORM"))
   return TAG_FORM;
  if(!strcmp(ptr,"RAME"))
   return TAG_FRAME;
  if(!strcmp(ptr,"RAMESET"))
   return TAG_FRAMESET;
  break;

  case 'H':
  if(*ptr<'7' && *ptr>'0')
   return TAG_H1+*ptr-'1';
  strupr(ptr);
  if(!strcmp(ptr,"R"))
   return TAG_HR;
  if(!strcmp(ptr,"EAD"))
   return TAG_HEAD;
  break;

  case 'I':
  if(!*ptr)
   return TAG_I;
  strupr(ptr);
  if(!strcmp(ptr,"MG"))
   return TAG_IMG;
  if(!strcmp(ptr,"NPUT"))
   return TAG_INPUT;
  break;

  case 'K':
  strupr(ptr);
  if(!strcmp(ptr,"BD"))
   return TAG_CODE;
  break;

  case 'L':
  strupr(ptr);
  if(!strcmp(ptr,"I"))
   return TAG_LI;
  if(!strcmp(ptr,"INK"))
   return TAG_LINK;
  break;

  case 'M':
  strupr(ptr);
  if(!strcmp(ptr,"AP"))
   return TAG_MAP;
  if(!strcmp(ptr,"ENU"))
   return TAG_OL;
  if(!strcmp(ptr,"ETA"))
   return TAG_META;
  break;

  case 'N':
  strupr(ptr);
  if(!strcmp(ptr,"OBR"))
   return TAG_NOBR;
  if(!strcmp(ptr,"OFRAMES"))
   return TAG_NOFRAMES;
  if(!strcmp(ptr,"OSCRIPT"))
   return TAG_NOSCRIPT;
  break;

  case 'O':
  strupr(ptr);
  if(!strcmp(ptr,"L"))
   return TAG_OL;
  if(!strcmp(ptr,"PTION"))
   return TAG_OPTION;
  break;

  case 'P':
  if(!*ptr)
   return TAG_P;
  strupr(ptr);
  if(!strcmp(ptr,"RE"))
   return TAG_PRE;
  break;

  case 'S':
  if(!*ptr)
   return TAG_B;
  strupr(ptr);
  if(!strcmp(ptr,"ELECT"))
   return TAG_SELECT;
  if(!strcmp(ptr,"MALL"))
   return TAG_SMALL;
  if(!strcmp(ptr,"UP"))
   return TAG_SUP;
  if(!strcmp(ptr,"UB"))
   return TAG_SUB;
  if(!strcmp(ptr,"CRIPT"))
   return TAG_SCRIPT;
  if(!strcmp(ptr,"TYLE"))
   return TAG_STYLE;
  if(!strcmp(ptr,"TRONG"))
   return TAG_B;
  break;

  case 'U':
  if(!*ptr)
   return TAG_U;
  strupr(ptr);
  if(!strcmp(ptr,"L"))
   return TAG_UL;
  break;

 }
 return 0;
}

//close HTML atom == set coordinates of right bottom corner
char closeatom(XSWAP adr,int deltax,long absy)
{
 char expand=0;
 struct HTMLrecord *atomptr;


 if(adr!=IE_NULL)
 {
  atomptr=(struct HTMLrecord *)ie_getswap(adr);
  if(atomptr)
  {
   if(abs((int)atomptr->x+deltax-(int)atomptr->xx)>FUZZYPIX)
    expand=1;
   p->xsum+=(atomptr->x+deltax)-atomptr->xx;
   atomptr->xx=atomptr->x+deltax;
   if(absy) //0...don't overwrite!
    atomptr->yy=absy;
   swapmod=1;
  }
  else
   MALLOCERR();
 }
 return expand;
}//end if


// find target frame for any HTML tag with TARGET attribute
// <A HREF=... TARGET=...>
int findtarget(int basetarget)
{
 int target=basetarget;
 char *tagarg;

 if(arachne.framescount && getvar("TARGET",&tagarg))
 {
  int fr=0;

  if(!strcmpi(tagarg,"_parent"))
   return p->htmlframe[p->currentframe].parent;

  do
  {
   if(!strncmpi(p->htmlframe[fr].framename,tagarg,39))
    target=fr;
  }
  while(fr++<arachne.framescount);
 }
 return target;
}

//add atom to metafile
void addatom(struct HTMLrecord *atom,void *ptr,int len,char t, char align,
           char d1, unsigned char d2,unsigned currentlink,char norightedge)
{
 unsigned dataptr,prevHTMLatom=p->lastHTMLatom;

 // printf("Adding atom of type %d...\n",t);

 if(len==0 && t==TEXT) return;


 if(len)
  dataptr=ie_putswap((char *)ptr,len,CONTEXT_HTML);
 else
  dataptr=IE_NULL;
 atom->type=t;
 atom->align=align;
 atom->data1=d1;
 atom->data2=d2;
// atom->R=r;   //this comes pre-defined in HTML atom...
// atom->G=g;
// atom->B=b;
 atom->ptr=dataptr;
 atom->datalen=len;
 atom->linkptr=currentlink;
 atom->next=IE_NULL;
 atom->prev=prevHTMLatom;
 atom->frameID=p->currentframe;
#ifdef JAVASCRIPT
 atom->jsptr=IE_NULL;
#endif

 p->lastHTMLatom=ie_putswap((char *)atom,sizeof(struct HTMLrecord),CONTEXT_HTML);
 if(p->firstHTMLatom==IE_NULL)
  p->firstHTMLatom=p->lastHTMLatom;
 else
 {
  struct HTMLrecord *atomptr=(struct HTMLrecord *)ie_getswap(prevHTMLatom);
  if(atomptr)
  {
   atomptr->next=p->lastHTMLatom;
   swapmod=1;
  }
  else
   MALLOCERR();
 }

 if(p->lastHTMLatom==IE_NULL)
 {
  p->memory_overflow=1;
  return;
 }
 else
  p->HTMLatomcounter++;

 if(!norightedge)
 {
  if(atom->xx>p->docRight)
   p->docRight=atom->xx;
  if(atom->xx>p->docRightEdge)
   p->docRightEdge=atom->xx;
  if(atom->xx+HTMLBORDER>p->htmlframe[p->currentframe].scroll.total_x)
   p->htmlframe[p->currentframe].scroll.total_x=atom->xx+HTMLBORDER;

  p->xsum+=(long)(atom->xx-atom->x);
 }

}

// move HTML document "cursor" d pixels to the right
void xshift(int *x,int d)
{
 *x+=d;
 if(*x>p->docRight)
  p->docRight=*x;
 if(*x>p->docRightEdge)
  p->docRightEdge=*x;
 if(*x+HTMLBORDER>p->htmlframe[p->currentframe].scroll.total_x)
  p->htmlframe[p->currentframe].scroll.total_x=*x+HTMLBORDER;

 p->xsum+=(long)d;
}

//called after the current line has been processed
void alignrow(int x,long y,int islist)
{
 int xhop;
 //int idx;
 long vsize;
 unsigned currentHTMLatom=p->lastHTMLatom;
 struct HTMLrecord *atomptr;

 if(p->HTMLatomcounter==0)
  return;

 if(!p->sizeTextRow)p->sizeTextRow=p->sizeRow;

 alignloop:
 atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
 if(!atomptr)
  MALLOCERR();
 if(atomptr->y==y && //eliminate unclosed buttons:
   !(atomptr->type==INPUT && (atomptr->data2 & 2) && atomptr->x==atomptr->xx) &&
    !(atomptr->type==TD || atomptr->type==TD_BACKGROUND ||
      atomptr->type==TABLE || atomptr->type==NAME))
 {
  if(atomptr->align & BOTTOM) //ALIGN=BOTTOM
  {
   if(atomptr->type==TEXT) //text
   {
    atomptr->y+=p->sizeTextRow;
    atomptr->yy=atomptr->y;
    atomptr->y-=fonty((int)atomptr->data1,atomptr->data2);
   }
   else if(atomptr->type!=INPUT || ! (atomptr->data2 & 2))
        //^^^^^^^^^^^^^^^^^ this is for <BUTTON> tags
   {
    vsize=atomptr->yy-atomptr->y;
    atomptr->y+=p->sizeRow;
    atomptr->yy=atomptr->y;
    atomptr->y-=vsize;
   }
   swapmod=1; //zapsal jsem do swapovane pameti!
  }

  if(atomptr->align & CENTER) //ALIGN=CENTER
  {
   xhop=(p->docRight-x)/2;
   atomptr->x+=xhop;
   atomptr->xx+=xhop;
   swapmod=1; //zapsal jsem do swapovane pameti!
  }
  else
  if(atomptr->align & RIGHT) //ALIGN=RIGHT
  {
   xhop=(p->docRight-x);
   atomptr->x+=xhop;
   atomptr->xx+=xhop;
   swapmod=1; //zapsal jsem do swapovane pameti!
  }

  if(atomptr->align & SUP) //ALIGN=SUP
  {
   vsize=fonty((int)atomptr->data1,atomptr->data2)/3;
   atomptr->y-=vsize;
   atomptr->yy-=vsize;
   swapmod=1; //zapsal jsem do swapovane pameti!
  }

  if(atomptr->align & SUB) //ALIGN=SUB
  {
   vsize=fonty((int)atomptr->data1,atomptr->data2)/3;
   atomptr->y+=vsize;
   atomptr->yy+=vsize;
   swapmod=1; //zapsal jsem do swapovane pameti!
  }

  atomptr->align=0;
  if(atomptr->prev!=IE_NULL && atomptr->yy>y)
  {
   currentHTMLatom=atomptr->prev;
   goto alignloop;
  }
 }
 if(p->docClearRight && y+p->sizeRow>=p->docClearRight)
 {
  p->docRight=p->docRightEdge;
  p->docClearRight=0;
 }
 if(p->docClearLeft && y+p->sizeRow>=p->docClearLeft)
 {
  if(islist==0)p->docLeft=p->docLeftEdge;
  p->docClearLeft=0;
 }

 p->sizeTextRow=0;
}//end sub

//implementatino of HTML tag <BR CLEAR=ALL>
void clearall(long *y)
{
 if(p->docClearLeft && p->docClearLeft>=p->docClearRight)
  *y=p->docClearLeft;
 else if (p->docClearRight)
  *y=p->docClearRight;

 if(p->docClearLeft)
 {
  p->docClearLeft=0;
  p->docLeft=p->docLeftEdge;
 }

 if(p->docClearRight)
 {
  p->docClearRight=0;
  p->docRight=p->docRightEdge;
 }
}

void fixrowsize(int font,char style)
{
     if(fonty(font,style)>p->sizeRow)
      p->sizeRow=fonty(font,style);
     if(fonty(font,style)>p->sizeTextRow)
      p->sizeTextRow=fonty(font,style);
}

//save current font information to font stack
void pushfont(int font,char style, struct HTMLrecord *atom,struct Fontstack *fontstack)
{
 if(fontstack->depth<MAXFONTSTACK)
 {
  if(fontstack->depth<0) //default value is -1
   fontstack->depth=0;
  else
   fontstack->depth++; //default value is -1
  fontstack->font[fontstack->depth]=font;
  fontstack->style[fontstack->depth]=style;
  fontstack->rgb[3*fontstack->depth]=atom->R;
  fontstack->rgb[3*fontstack->depth+1]=atom->G;
  fontstack->rgb[3*fontstack->depth+2]=atom->B;
 }
}

//restore current font information to font stack
int popfont(int *font,char *style, struct HTMLrecord *atom, struct Fontstack *fontstack)
{
 if(fontstack->depth>=0)
 {
  *font=fontstack->font[fontstack->depth];
  *style=fontstack->style[fontstack->depth];
  atom->R=fontstack->rgb[3*fontstack->depth];
  atom->G=fontstack->rgb[3*fontstack->depth+1];
  atom->B=fontstack->rgb[3*fontstack->depth+2];
  fontstack->depth--; //margin value is -1
  return 1;
 }
 else
  return 0;
}

int RGB256(unsigned char r,unsigned char g,unsigned char b);

int RGB(unsigned char r,unsigned char g,unsigned char b)
{
#ifdef HICOLOR
 if(xg_256==MM_Hic)
 {
  char pal[3];

  pal[0]=r>>2;
  pal[1]=g>>2;
  pal[2]=b>>2;
  x_pal_1(17, pal);
  return 17;
 }
 else
#endif
  return RGB256(r,g,b);
}


//returns width of space (ASCI 32)
int space(char font)
{
 return fontx(font,0,' ');
}


//initialization of color cache, which speeds up color lookup
void resetcolorcache(void)
{
 memset(cacher,0,16);
 memset(cacheg,0,16);
 memset(cacheb,0,16);
 memset(coloridx,0,16);
 cacher[1]=255;
 cacheg[1]=255;
 cacheb[1]=255;
 coloridx[1]=15;
 rgbcacheidx=2;
}

int InitInput(struct ib_editor *fajl,char *name,char *str,int lines,int context)
{
 int rc;

 fajl->filename[0]='\0';
 rc=ie_openf_lim(fajl,context,lines);
 strcpy(fajl->filename,name);
 if(rc==1 && str)
  ie_insline(fajl,0,str);

 return rc;
}

struct TMPframedata *locatesheet(struct TMPframedata *rootsheet, struct TMPframedata *tmpsheet,XSWAP stylesheetadr)
{
 if(stylesheetadr==IE_NULL)
  return rootsheet;
 else
  return locatesheet_ovrl(rootsheet,tmpsheet,stylesheetadr);
}

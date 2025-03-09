
// ========================================================================
// HTML rendering routines for Arachne WWW browser
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

// !!JdS 2004/12/02 : Replaced image-input atom subtype literal
// constants with token symbols (which are now defined in 'html.h').

#include "arachne.h"
#include "html.h"
#include "internet.h"

int kbhit(void);

int renderHTML(struct Page *p)
{
 //not specific to frames:
 char search4maps=0;
 unsigned char in; //in=buf[i]...
 //specific to frames:
 int i,bflen;

//!!glennmcc: July 13, 2006
int tablecount=0;
//!!glennmcc: end

//!!glennmcc: begin Feb 24, 2002
// (added 'bflenold' and 'retry' for use by Quick-n-dirty Fix for RDLC bug)
#ifndef NORDLC
 int bflenold=0;
 unsigned long int retry=0;
#endif
//!!glennmcc: end

//!!glennmcc: July 08, 2006 -- kill any image wider than 'MaxImgWidth'
//defaults to 2048 if variable missing from CFG
int maxwidth;
//!!glennmcc: end

 long fpos;
 char tagname[16],entityname[10],pom[2],*tagarg,*tagargptr;
 int taglen,tag,lasttag,endoftag;
 char param,entity;
 int vallen,txtlen,entilen;
 char str[80];
 char uvozovky,apostrof,argument,comment,nolt,invisibletag,argspc;
 int basetarget;
 int x;
 long y,currentbuttony;
 long tdheight=0;
 int font;
 char style,align,basealign[17],valign=MIDDLE;
 struct Fontstack fontstack;
 int xsize,lastspcpos,lastspcx,maxoption;
 char pre,charsize,nobr;
 char lastspace,lastentity,alreadyframe,notrefreshed;
 long lastredraw,lastredrawy;
 char nownobr,nbsp,boom,istd;
 int tabledepth;
 int orderedlist[MAXTABLEDEPTH+2];  //0..unordered, >0...num, <0...nolist
 int nobr_x_anchor,currentbuttonx;
 struct Url url;
 struct HTMLrecord HTMLatom;
 char plaintext,input_image,ext[5];
 int insidetag; // ==TAG_TITLE, ==TAG_SCRIPT... etc.
 int stackLeftEdge[MAXTABLEDEPTH+2],stackRightEdge[MAXTABLEDEPTH+2];
 int stackLeft[MAXTABLEDEPTH+2],stackRight[MAXTABLEDEPTH+2];
 long clearstackLeft[MAXTABLEDEPTH+2],clearstackRight[MAXTABLEDEPTH+2];
 long maxsumstack[MAXTABLEDEPTH+2];
 unsigned tableptrstack[MAXTABLEDEPTH+2];
 int fontstackdepth[MAXTABLEDEPTH+2];
 int centerdepth[MAXTABLEDEPTH+2];
 int alignstack[MAXTABLEDEPTH+2];
 int tdwidth[MAXTABLEDEPTH+2];
 int emptyframeset;
 int previousframe;
 char noresize;
 long timer;
 int currentnobr=0;
 int alreadyselected=0,multiple=0;
 int listdepth,listdepthstack[2*MAXTABLEDEPTH],listedge[2*MAXTABLEDEPTH];
 int pre_RightEdge, pre_Right;
 struct HTTPrecord *cache;
 struct HTMLframe *frame;
 struct TMPframedata *htmldata,tmpsheet,*sheet;
 struct HTMLrecord *atomptr;
 //xSwap pointers:
 XSWAP currentlink,currentform,currentbutton,stylesheetadr,
       currenttextarea,thistableadr,
       currenttable[MAXTABLEDEPTH+2],currentcell[MAXTABLEDEPTH+2];
 struct HTMLtable *tmptable,*thistable;
//#ifdef POSIX ??
// struct HTMLtable  *newtable;
//#endif
 struct picinfo *img;
 long percflag;
 char *text="";

// werner scholz Nov 08,2006  --- utf8 ---
int utf8=0;           // utf8-sequenceflag
unsigned char utf8_1,utf8_2,utf8_3,utf8_4;  // utf8-sequenzbytes  1...4
int utf8_byte;        // loadpointer for utf8-sequenzbytes 1...4
RENDER.utf8=0;
// werner end  --- utf8 ---

// --------------------------------------------------------------------------
/* This function is called in following modes:

    1) unknown HTML page - we don't know about frames, images, tables, etc.
    2) we known size of tables, without frames - accept URL
    3) we known size of frames - use URLs from frame table
    4) we known size of the frames and tables - use URLs from frame table
*/
// --------------------------------------------------------------------------

 GLOBAL.needrender=0;
 RENDER.willadjusttables=0;

 if(arachne.target==0)
  SetInputAtom(&URLprompt,p->htmlframe[0].cacheitem.URL);

 //start from p->htmlframe[0] (will be skipped later if it is parrent frame)
 if(p->forced_html==RELOAD_HTML_FRAMES)
 {
  p->forced_html=0;
  p->currentframe=p->htmlframe[0].next;
  if(p->currentframe<0)
  {
   p->currentframe=0;
   arachne.target=0;
   arachne.framescount=0; //delete frames!
   reset_frameset();
   p->htmlframe[0].next=-1;
  }

  if(!p->rendering_target)
   DrawTitle(0);
 }
 else
 {
  p->currentframe=0;
  if(arachne.target==0)
  {
   arachne.framescount=0; //delete frames!
   reset_frameset();
   p->htmlframe[0].next=-1;
  }
 }

 //dealocate memory
 Deallocmem();

 //reset table pointer
 p->nextHTMLtable=p->firstHTMLtable;
 p->prevHTMLtable=IE_NULL;

 tagargptr=farmalloc(BUF/2);
 img=farmalloc(sizeof(struct picinfo));
 thistable=farmalloc(sizeof(struct HTMLtable));
 text=farmalloc(BUF+8);

//!!Bernie: Mar 01, 2007 -- moved below to prevent possibility of crash 
//text[0]='\0';
//!!Bernie: end

//#ifdef POSIX
//  newtable=thistable;
//#endif

 if(!text || !thistable || !img || !tagargptr)
  memerr();

//!!Bernie: Mar 01, 2007 -- moved from up above to prevent possibility of crash
  text[0]='\0';
//!!Bernie: end

 //show some info to user
 MemInfo(NORMAL);

 htmlmsg[0]='\0';
 htmlpulldown=0;
 activeistextwindow=0;
 lastfound=IE_NULL;
 lastfoundY=0l;

 //------------------------------------------------------------------------
 insertframe:  // repeat for ALL FRAMES in specified document:
 //------------------------------------------------------------------------

 i=0;
 bflen=0;
 fpos=0;
 tagname[0]='\0';
 entityname[0]='\0';
 taglen=0;
 tag=0;
 lasttag=0;
 endoftag=0;
 param=0;
 entity=0;
 vallen=0;
 txtlen=0;
 entilen=0;
 uvozovky=0;
 apostrof=0;
 argument=0;
 comment=0;
 nolt=0;
 invisibletag=0;
 argspc=0;
 x=0;
 y=0;
 align=BOTTOM;
 basealign[0]=BOTTOM;
 lastspcpos=0;
 nobr=0;
 nownobr=0;
 nbsp=0;
 lastspace=1;
 lastentity=0;
 alreadyframe=0;
 notrefreshed=1;
 lastredraw=0l;
 lastredrawy=-0l;
 orderedlist[0]='\0';
 tabledepth=0;
 istd=0;
 plaintext=1;
 insidetag=0;
 emptyframeset=-1;
 noresize=0;
 timer=time(NULL);
 fontstack.depth=-1;

 currentlink=IE_NULL;
 currentform=IE_NULL;
 currenttextarea=IE_NULL;
 currentbutton=IE_NULL;
 thistableadr=IE_NULL;
 stylesheetadr=IE_NULL;

 currenttable[0]=IE_NULL;
 currentcell[0]=IE_NULL;
 centerdepth[0]=0;
 tdwidth[0]=0;

 //skip unmodified parent frames (typicaly p->htmlframe[0])
 while(p->htmlframe[p->currentframe].hidden && p->currentframe<MAXFRAMES-1 &&
       p->currentframe!=arachne.target && p->htmlframe[p->currentframe].next!=-1)
 {
  kbhit();
  p->currentframe=p->htmlframe[p->currentframe].next;
 }
 p->activeframe=basetarget=p->currentframe;
 previousframe=p->currentframe;

 //define pointer to current html frame:
 frame=&(p->htmlframe[p->currentframe]);

 //define pointer to current cache item:
 cache=&frame->cacheitem;

 //define pointer to current temporary frame data
 sheet=htmldata=&(p->tmpframedata[p->currentframe]);

 htmldata->basefontsize=3;
 htmldata->basefontstyle=0;
 p->docLeft=p->docLeftEdge=0;
 pre_Right=pre_RightEdge=p->docRight=p->docRightEdge=frame->scroll.xsize;

 //only for first rendering:
 if(!GLOBAL.isimage || GLOBAL.source)
 {
  if(p->currentframe==0)
  {
   if(GLOBAL.source)
   {
    sprintf(text,"Source of %s",cache->URL);
    MakeTitle(text);
   }
   else
   if(GLOBAL.validtables==TABLES_UNKNOWN && p->currentframe==0)
   {
    MakeTitle("");
    if(p->html_source==HTTP_HTML)
     DrawTitle(0);
   }
  }

  ResetHtmlPage(htmldata,TEXT_PLAIN,0);
  frame->scroll.xvisible=0;
  frame->scroll.yvisible=1;
  frame->posX=0;
  frame->posY=0l;
 }

 get_extension(frame->cacheitem.mime,ext);
 if((!strcmpi(ext,"HTM") || p->forced_html) && !GLOBAL.source)
 {
  //formatovani a barvy v HTML dokumentu:
  // tr.: formatting and colours in HTML document
  x=frame->marginwidth;
  y=frame->marginheight;
  plaintext=0;
  p->docLeft=p->docLeftEdge=stackLeft[0]=stackLeftEdge[0]=frame->marginwidth;;
  p->docRight=p->docRightEdge=stackRight[0]=stackRightEdge[0]=frame->scroll.xsize-frame->marginwidth-FUZZYPIX;
  ResetHtmlPage(htmldata,TEXT_HTML,1);
  frame->scroll.xvisible=0;
  if(p->currentframe==0)
   frame->scroll.yvisible=1;
  else
   frame->scroll.yvisible=0;
 }

 RENDER.translatecharset=0;
 {
  char *set=strstr(frame->cacheitem.mime,"charset=");
  if(set)
   HTTPcharset(set);
 }

 if(fixedfont)
  p->docRight=p->docRightEdge=stackRightEdge[0]=(CONSOLEWIDTH-2)*space(SYSFONT);
#ifndef NOPS
 else if(p->rendering_target)
  p->docRight=p->docRightEdge=stackRightEdge[0]=(int)((user_interface.postscript_x-51)*5);
#endif

 if(!GLOBAL.isimage /*&& GLOBAL.validtables!=TABLES_EXPAND*/ && !GLOBAL.source && !p->forced_html)
 {
  frame->posX=cache->x;
  frame->posY=cache->y;
 }

 lastspcx=p->docLeft;
 percflag=0;
 HTMLatom.x=x;
 HTMLatom.y=y;
 HTMLatom.R=htmldata->textR;
 HTMLatom.G=htmldata->textG;
 HTMLatom.B=htmldata->textB;
 listdepth=0;
 orderedlist[0]=0;
 p->docClearLeft=p->docClearRight=0;
 clearstackLeft[0]=clearstackRight[0]=0;
 maxsumstack[0]=0;
 tableptrstack[0]=IE_NULL;
 font=htmldata->basefontsize;
 style=htmldata->basefontstyle;

 pre = config_get_bool("WrapPre", 0) ? 0 : plaintext;
 if(plaintext)font=SYSFONT;

 p->sizeTextRow=p->sizeRow=fonty(font,style); //?p->sizeRow?
 xsize=0;

#ifdef VIRT_SCR
 //vynuluji virtualni obrazovku
 //tr.: set virtual screen to zero, erase virtual screen
 virtualxstart[htmldata->whichvirtual]=0;
 virtualystart[htmldata->whichvirtual]=0;
 virtualxend[htmldata->whichvirtual]=0;
 virtualyend[htmldata->whichvirtual]=0;
#endif

 //by default, we suppose this HTML frame not to be parent frame
 //if FRAMESET is encountered, we will toggle this flag.
 frame->hidden=0;

 ScrollInit(&frame->scroll,
       frame->scroll.xsize,
       frame->scroll.ymax,   //visible y
       frame->scroll.ymax,   //max y
       frame->scroll.xtop,
       frame->scroll.ytop,
       frame->scroll.xsize,0);//total x,y
 ScrollButtons(&frame->scroll);

 if(!GLOBAL.isimage && GLOBAL.validtables==TABLES_UNKNOWN && p->html_source==HTTP_HTML)
  redrawHTML(REDRAW_WITH_MESSAGE,REDRAW_SCREEN);

 //-----pouze obrazek ------------------
 //tr.: only picture
 if(*ext && strstr(imageextensions,ext) && !p->forced_html)
 {
  int znamrozmerx=0,znamrozmery=0;

#ifndef NOTCPIP
  if(p->html_source==HTTP_HTML && arachne.target==p->currentframe)
   Download(cache);
#endif

  init_picinfo(img);
  img->html_x=0;
  img->html_y=0l;
  strcpy(img->URL,cache->URL);
  strcpy(img->filename,cache->locname);
  img->sizeonly=1;

  if(!strcmpi(ext,"IKN"))
  {
   img->size_y=60;
   img->size_x=60;
   znamrozmerx=znamrozmery=1;
  }
  else
  {
   if(drawanyimage(img)==1)
   {
    znamrozmerx=znamrozmery=1;
    cache->size=img->filesize;
    cache->knowsize=1;
   }
   else
   {
    strcpy(text,cache->URL);
    goto supsem;
   }
  }

  sprintf(text,MSG_IMAGE,strupr(ext),img->size_x,img->size_y);
  if(img->npal)
  {
   sprintf(str,MSG_COLORS,img->npal);
   strcat(text,str);
  }

  supsem:
  strcpy(img->alt,text);
  MakeTitle(text);
  if(!p->rendering_target)
   DrawTitle(0);

  if(!znamrozmerx)
   img->size_x=8*strlen(img->alt)+4;

  if(!znamrozmery) img->size_y=18;

  HTMLatom.x=0;
  HTMLatom.y=0;
  HTMLatom.xx=img->size_x;
  HTMLatom.yy=img->size_y;
  p->sizeRow=img->size_y;
  frame->scroll.total_x=img->size_x;
  addatom(&HTMLatom,img,sizeof(struct picinfo),IMG,0,0,0,currentlink,1);
  lastredraw=-1l;

  goto exitloop;
 }

 if(!openHTML(cache,p->html_source))
 {
  if(arachne.target==p->currentframe)
  {
   farfree(tagargptr);
   farfree(text);
   farfree(thistable);
   farfree(img);
   return 0; //error!
  }
  else
   goto exitloop;
 }

loopstart: //------------- vlastni cykl - analyza HTML i plain/text---------
      //tr.: own (or separate) cycle - analyzis of HTML and plain/text
  if((percflag & 0x1ff) == 0x1ff ) //kazdych 512 (tr.: for every 512)
   if(!p->rendering_target && GUITICK())
    if(GLOBAL.gotolocation || GLOBAL.abort)
     goto exitloop;

  if(!percflag /* && !(p->currentframe<arachne.framescount)*/)
  {
   int prc;

   frame->scroll.total_y=y+p->sizeRow;

   if(!p->rendering_target && (user_interface.quickanddirty  || noresize ||
   !(GLOBAL.validtables==TABLES_UNKNOWN && RENDER.willadjusttables)))
   {
    if(y>frame->posY+frame->scroll.ysize &&
       lastredrawy+fonty(htmldata->basefontsize,0)<frame->posY+frame->scroll.ysize &&
       y!=lastredrawy &&
       GLOBAL.validtables==TABLES_UNKNOWN && !noresize)
    {
     //REDRAW_SCREEN?
     redrawHTML(REDRAW_NO_MESSAGE,REDRAW_SCREEN);
     lastredrawy=y;
     lastredraw=fpos;
     notrefreshed=0;
    }
    else
    if(y<frame->scroll.ysize && time(NULL)-timer>5 &&
       GLOBAL.validtables==TABLES_UNKNOWN && !noresize)
    {
     //REDRAW_SCREEN?
     redrawHTML(REDRAW_NO_MESSAGE,REDRAW_SCREEN);
     timer=time(NULL);
     notrefreshed=0;
    }
    else
    {
     ScrollInit(&frame->scroll,
      frame->scroll.xsize,
      frame->scroll.ymax,     //visible y
      frame->scroll.ymax,     //max y
		frame->scroll.xtop,
      frame->scroll.ytop,
      frame->scroll.total_x,  //total x
      frame->scroll.total_y); //total y

     if(frame->allowscrolling)
     {
      mouseoff();
      if(notrefreshed)
       ScrollButtons(&frame->scroll);
      ScrollDraw(&frame->scroll,frame->posX,frame->posY);
      mouseon();
     }
    }//endif draw scrallbar
   }//endif !p->rendering_target (=hidden output)

   if(cache->size>100)
//!!glennmcc:Oct 23, 2008 -- 'reversed the logic'
// to keep from overflowing at 21megs
    prc=(int)(fpos/(cache->size/100));
//  prc=(int)(100*fpos/cache->size);
   else
    prc=0;

   if(p->html_source==LOCAL_HTML)
   {
    char *msg;

    if(GLOBAL.validtables==TABLES_UNKNOWN)
     msg=MSG_DISK;
    else
     msg=MSG_ADJUST;

    if(user_interface.logoiddle)
    {
     percflag=2l*user_interface.logoiddle+plaintext*12000l;
     ikniddle=user_interface.logoiddle;
     xChLogoTICK(1); // animace loga pri nacitani souboru...
     //tr.: animation of logo while loading file
    }
    else
     percflag=4000l+plaintext*12000l;
    if(fpos+2*percflag>cache->size)
     percflag*=2;

    if(fpos==0l)
     outs(msg);
    percentbar(prc);
   }
   else //tcp/ip
   {
    if (cache->knowsize)
    {
     sprintf(str,MSG_DLPERC,fpos,cache->size);
     percflag=2*loadrefresh;
     outs(str);
     percentbar(prc);
    }
    else
    {
     sprintf(str,MSG_DLBYTE,fpos);
     percflag=loadrefresh;
     outs(str);
    }
   }//endif
  }//endif somethign to output
  percflag--;

  if(i==bflen)
  {

//!!glennmcc: begin Feb 24, 2002 (Quick-n-dirty Fix for RDLC bug)
#ifndef NORDLC
   bflenold=bflen;
#endif
//!!glennmcc: end

   bflen=readHTML(cache,p->html_source);


   if(bflen<=0 || p->memory_overflow) //end of page, or out of memory
    goto exitloop;
   else
    i=0;
  }

//!!glennmcc: begin Feb 24, 2002 (Quick-n-dirty Fix for RDLC bug)
#ifndef NORDLC
if (cache->knowsize)goto knowsize;
   if (bflenold==bflen)
   {
    retry++;
   }
   else
   {
    retry=0;
   }
   if (retry>96000l) //was 32000 //was 24000
   {
    retry=0;
    goto exitloop;
   }
knowsize:
#endif
//!!glennmcc: end

  if(RENDER.translatecharset)
   in=GLOBAL.codepage[(unsigned char)p->buf[i]];
  else
{  in=(unsigned char)p->buf[i];

//!!glennmcc: June 07, 2007 -- use entity.cfg to 'recode' these situations
//!!glennmcc: July 31, 2007 -- do not use entity.cfg recoding when
//Werner's (much better), UTF8 recoding is 'in-play'
 if(in>127 && !utf8)
// if(in>127)
 {
  char recode[11], *ptr;
  itoa(in,recode,10);
  ptr = config_get_entity(recode);
  if (ptr) in = *ptr;
 }
}
//!!glennmcc: end

   // werner scholz Nov 11,2006    '<' has priority and ends utf8sequence
   if((utf8)&&(in!='<'))goto utf8start;
   utf8=0;
   // werner scholz  end

  //text/html ignoruje konce radku, uvnitr <PRE> nebo text/plain ne:
  //tr.: ignores end of line, but not in <PRE> or text/plain
  if(in<' ')
  {
   if((insidetag==TAG_TEXTAREA || insidetag==TAG_STYLE) &&
      (in=='\n' || in=='\r'))
   {
    if(txtlen>IE_MAXLEN)txtlen=IE_MAXLEN;
    text[txtlen]='\0';
    txtlen=0;
    appendline(currenttextarea,text,0);
    lastspace=0;
    goto loop;
   }
   else
   if(!pre || (in!='\n' /*&& in!='\r'*/) || tag ||
      insidetag==TAG_OPTION || insidetag==TAG_SELECT)
   {
    if(tag && param && uvozovky || endoftag)
     goto loop;
    in=' ';
   }
   else
   {
    HTMLatom.xx=x;
    HTMLatom.y=y;
    HTMLatom.yy=y+fonty(font,style);
    text[txtlen]='\0';
    fixrowsize(font,style);
    if(!invisibletag)
     addatom(&HTMLatom,text,txtlen,TEXT,align,font,style,currentlink,0);
    y+=p->sizeRow;
    goto linebreak;
   }
  }//endif

  //......................................................zpracovani 1 znaku
  //tr.: processing 1 character
  if(!tag)
  {
   endoftag=0;
//!!glennmcc: July 30, 2006
// to fix problems with '<' symbols in JS and CSS,
//all html tags _should_ be an alphabetic character immediately following
//that '<' symbol
// (except for the "comment tag" '<!--' and the "slashtag" '</tag>')
//!!glennmcc: July 31, 2006 -- modified to simply by-pass
//spaces, numbers and all other characters up-to and including '@' (:;<=>?@)
//Aug 02, 2006 -- causing more trouble than it's worth :(
/*
if((in=='<' &&
//   (p->buf[i+1]!=' ' &&
//    (p->buf[i+1]<'0' || p->buf[i+1]>'@')
//    ))
     (isalpha(p->buf[i+1]) || p->buf[i+1]=='!' || p->buf[i+1]=='/'))
       && !plaintext) //zacatek HTMLtagu (tr.: start of HTML tag)
*/
   if(in=='<' && !plaintext) //zacatek HTMLtagu (tr.: start of HTML tag)
//original single line above this comment
//!!glennmcc: end
   {
    HTMLatom.xx=x;
     HTMLatom.y=y;
    HTMLatom.yy=y+fonty(font,style);
    if(txtlen)
    {
     fixrowsize(font,style);
     if(!invisibletag)
     {
      text[txtlen]='\0';
      addatom(&HTMLatom,text,txtlen,TEXT,align,font,style,currentlink,0);
     }
     if(lasttag==TAG_BODY)
      lasttag=TAG_P;
     if(insidetag==TAG_TITLE && !arachne.title[0] && !p->currentframe)
     {
      text[txtlen]='\0';
      MakeTitle(text);
      if(!p->rendering_target)
       DrawTitle(0);
      invisibletag=0;
      insidetag=0;
     }
    }
    tag=1;
    param=0;
    taglen=0;
    argument=0;
    argspc=0;
    vallen=0;
    entity=0;
    argnamecount=0; //reset argument counter
    argvaluecount=0;
    goto loop;
   }

  // werner scholz  Nov 8,2006  ----  begin  UTF8 processing ---
   utf8start:
   if(RENDER.utf8==0)goto utf8end;      // jump if no utf-8  !
   if((utf8==0)&&(in<128))goto utf8end; // no utf8-character !

      if(utf8==0)          // start utf8 encoding
       {utf8_1=in&240;     // length of utf8-sequence is given by highbyte
   if(utf8_1==192)utf8=2;   // 2 bytes of utf8  C
   if(utf8_1==208)utf8=2;   // 2 bytes of utf8  D
   if(utf8_1==224)utf8=3;   // 3 bytes of utf8  E
   if(utf8_1==240)utf8=4;   // 4 bytes of utf8  F
   utf8_byte=1;             // Bytecounter
   utf8_1=in;               // Load byte 1
   utf8--;
   if(utf8)goto loop;       // get next sequencebyte
   goto utf8end;            // no utf8
       }
      if((in&192)!=128){utf8=0;goto loop;} // check sequencebyte !
   if(utf8_byte==1) utf8_2=in;  // Load byte 2
   if(utf8_byte==2) utf8_3=in;  // Load byte 3
   if(utf8_byte==3) utf8_4=in;  // Load byte 4
      utf8_byte++;         // Address next byte
      utf8--;              // countdown to zero
      if(utf8)goto loop;   // get next byte
      in=utf8table(utf8_1,utf8_2,utf8_3,utf8_4); // get in from utf8table
  utf8end:
// Werner Scholz    --- end utf8-processing  ---

   //Entity (&lt;,&jt; &copy ...)
   //tr.: entities
   if(in=='&' && !plaintext && (isalpha(p->buf[i+1]) || p->buf[i+1]=='#' || i==bflen))
//!!glennmcc: Dec 18, 2006 -- '&' followed by an alpha is only an entity
// if there is also ';' within the next 7 characters
//!!glennmcc: Jan 28, 2007 -- bad idea :(
// I'll leave this here for others to have a look at.
/*
   if(in=='&' && !plaintext &&
      (isalpha(p->buf[i+1]) &&
       (
   (p->buf[i+3]==';' ||
    p->buf[i+3]=='<' ||  //catches entities that incorrectly end without ';'
    p->buf[i+3]==' ') || //catches entities that incorrectly end in a space
   (p->buf[i+4]==';' ||
    p->buf[i+4]=='<' ||  //catches entities that incorrectly end without ';'
    p->buf[i+4]==' ') || //catches entities that incorrectly end in a space
   (p->buf[i+5]==';' ||
    p->buf[i+5]=='<' ||  //catches entities that incorrectly end without ';'
    p->buf[i+5]==' ') || //catches entities that incorrectly end in a space
   (p->buf[i+6]==';' ||
    p->buf[i+6]=='<' ||  //catches entities that incorrectly end without ';'
    p->buf[i+6]==' ') || //catches entities that incorrectly end in a space
   (p->buf[i+7]==';' ||
    p->buf[i+7]=='<' ||  //catches entities that incorrectly end without ';'
    p->buf[i+7]==' ')    //catches entities that incorrectly end in a space
       )
     || p->buf[i+1]=='#' || i==bflen))
*/
//!!glennmcc: end

   //HTML entita zacina '&'
   //uvnitr tagu by byt nemela
   //tr.: HTML entity begins with &, this should not happen within TAG
   {
    entity=1;
    entilen=0;
    entityname[0]='\0';
    goto loop;
   }
   else
   if(entity)       //vnitrek entity, tr.: within an entity
   {
    if(entilen>8 || in==';' || in==' ')
    {
     entityname[entilen]='\0';
     if(entilen>0)
      in=(char)HTMLentity(entityname);
     else
      in='&';
     entity=0;
    }
    else
    {
     entityname[entilen++]=in;
     goto loop;
    }
   }//endif entity (tr.: entities)

   if(((unsigned char)in==160)&&(!utf8)) // ASCII 160 is ALWAYS nobreak space in HTML
   {   // werner scholz  Nov 8,2006 skip for character  'a  in  utf8 !
/*
   if((unsigned char)in==160) // ASCII 160 is ALWAYS nobreak space in HTML
   {
*/
    in=' ';
    lastspace=0;
    lastentity=1;
    nbsp=1;
   }

/*
   we will show anything which may be in font set...
   if((unsigned char)in>128 && (unsigned char)in<160)
   {
    in=' ';
   }
*/
   // <-----------------------------------------<--------<-----------mimo tag
   //tr.: outside of tag
   if(in!=' ' || !lastspace || pre)
   {
    if(txtlen<BUF) text[txtlen++]=in;
    if(!invisibletag)
    {
     charsize=fontx(font,style,in);
     x+=charsize;
     if(x>p->docRight)
     {
      if(lastspcpos==0 || pre || nownobr)
      {
       xsize=x-HTMLatom.x;
       if(xsize<p->docRight-p->docLeft && !pre && !nownobr && !nobr /*&&
     (HTMLatom.x!=p->docLeft || GLOBAL.validtables)*/ || p->docRight<p->docRightEdge)
       {
   // <------------------------------------------------odsunout cely atom
   // tr.: move out the entire atom
	alignrow(HTMLatom.x,y,orderedlist[listdepth]);
   y+=p->sizeRow;

   //kdyz jsou levy a pravy okraj moc blizko u sebe...
	//tr.: if left and right margins are too close to each other
   if(xsize>p->docRight-p->docLeft)
    clearall(&y);

   p->sizeTextRow=p->sizeRow=fonty(font,style);
   x=p->docLeft+xsize;
	HTMLatom.x=p->docLeft;
   HTMLatom.y=y;
   HTMLatom.xx=x;
	HTMLatom.yy=y+p->sizeRow;
   addatom(&HTMLatom,text,txtlen,TEXT,align,font,style,currentlink,0);
   HTMLatom.x=x;
   HTMLatom.y=y;
	txtlen=0;
   lastspcpos=0;
   lastspcx=x;
	nownobr=nobr;
       }
       else
       {
   p->docRight+=charsize;
   //during second pass, certain <NOBR> elements will be treated as <BR>
	if(nobr)
	boom=1;
       }
      }
      else
      {
       // <---------------------------------------------------------novy radek
       //tr. new line
       //!!!text[lastspcpos-1]='\0';
       text[lastspcpos-1]='\0';
       text[txtlen]='\0';

       fixrowsize(font,style);
       HTMLatom.xx=lastspcx;
       HTMLatom.yy=y+p->sizeRow;

       addatom(&HTMLatom,text,lastspcpos,TEXT,align,font,style,currentlink,0);
       alignrow(lastspcx,y,orderedlist[listdepth]);

       txtlen=strlen(&(text[lastspcpos]));
       memmove(text,&(text[lastspcpos]),txtlen);
       HTMLatom.x=p->docLeft;
       y=y+p->sizeRow;
       p->sizeTextRow=p->sizeRow=fonty(font,style); //?p->sizeRow?
       HTMLatom.y=y;
       xsize=x-lastspcx;
       x=p->docLeft+xsize;
       HTMLatom.y=y;
       lastspace=0;
       lastspcpos=0;
       lastspcx=0;
      }
     }
    }//endif viditelny text (tr.: visible text)

//!!glennmcc: May 06, 2007---
//only wrap long pathnames which are in an unordered list ( <ul> ) ---
   if(in==' ' && !nbsp || (in=='/' && orderedlist[listdepth]<0))
    {
     lastspcpos=txtlen;
     lastspcx=x;//-fontx(font,' ');
     if(!lastentity)
     {
      nownobr=nobr;
      lastspace=1;
     }
     lastentity=0;
    }
    else
    {
     lastspace=0;
     nbsp=0;
    }

   }//konec zpracovani jednoho znaku (tr.: end of processing of 1 char.)

  }
  else
  //................................................zpracovani vnitrku tagu
  //tr.: processing tag content
  {
//!!glennmcc: Aug 03, 2006 -- prevent problems with '<' within script or style
   if(strncmp(pom,"--",2) && in=='>') nolt=0;
//!!glennmcc: end
   if (in=='>' && !uvozovky && (nolt || !(comment && strncmp(pom,"--",2))))
   {
    if(param && !comment)
    {
     if(argument)
      putvarvalue(tagargptr,vallen);
     else if (vallen)
     {
      putvarname(tagargptr,vallen);
      putvarvalue(tagargptr,0);
     }
    }//endif

    //analyza HTML TAGu -------------------------------------- HTML level 0/1
    tagname[taglen]='\0';
    tag=FastTagDetect(tagname);

#ifdef TABLES
    if(tag<TAG_SLASH || tag==TAG_SLASH_TABLE)
#else
    if(tag<TAG_SLASH)// || tag==TAG_SLASH_TABLE)
#endif
     endoftag=1;

    if(insidetag)
    {
     if((insidetag==TAG_TEXTAREA && tag!=TAG_SLASH_TEXTAREA) ||
   (insidetag==TAG_TITLE && tag!=TAG_SLASH_TITLE) ||
   (insidetag==TAG_STYLE && tag!=TAG_SLASH_STYLE) ||
   (insidetag==TAG_SELECT && tag!=TAG_SLASH_SELECT && tag!=TAG_OPTION && tag!=TAG_SLASH_OPTION) ||
   (insidetag==TAG_OPTION && tag!=TAG_SLASH_SELECT && tag!=TAG_OPTION && tag!=TAG_SLASH_OPTION) ||
   (insidetag==TAG_SCRIPT && tag!=TAG_SLASH_SCRIPT && tag!=TAG_SLASH_NOSCRIPT &&
    tag!=TAG_SLASH_NOFRAMES && tag!=TAG_ARACHNE_BONUS))
      tag=0;
    }

    switch(tag)
    {
     case TAG_P: //<P>
     case TAG_DIV: //<DIV>

     p:
     PARAGRAPH;
     p->sizeTextRow=p->sizeRow=0; //fonty(font,style); //?p->sizeRow?
     invisibletag=0; //paragraf ukonci chybny option, title, apod.
     //tr.: paragraph will be terminated by wrong option, title, etc.
     //align=basealign[tabledepth];
     {
      int reset=0;
      if(sheet!=htmldata)
       reset=1;
      sheet=locatesheet(htmldata,&tmpsheet,stylesheetadr);
      if(sheet!=htmldata || reset)
      {
       style=sheet->basefontstyle;
       font=sheet->basefontsize;
       HTMLatom.R=sheet->textR;
       HTMLatom.G=sheet->textG;
       HTMLatom.B=sheet->textB;
      }
     }

     alignset:
     x=p->docLeft;
     lastspace=1;//mazat mezery (tr.: erase spaces)
     if(getvar("ALIGN",&tagarg))
     {
//       basealign[tabledepth]=align;
      if(align & CENTER) align-=CENTER;
      if(align & RIGHT) align-=RIGHT;
      if(!strcmpi(tagarg,"CENTER"))    align=align | CENTER;
      else if(!strcmpi(tagarg,"RIGHT")) align=align | RIGHT;
     }
     break;

     case TAG_A: //<A ...>

     {
      char removable=0;

      HTMLatom.x=x;
      HTMLatom.y=y;
      HTMLatom.xx=x;
      HTMLatom.yy=y;

      if(getvar("REMOVABLE",&tagarg))
       removable=1;

      if(getvar("HREF",&tagarg))
      {
       char target;

//!!glennmcc: Apr 23, 2008 -- convert unicoded 'hot links'
//!!glennmcc; Jan 24, 2009 -- better fix ;-)
   if(strstr(tagarg,"&#"))
// if(tagarg[0]=='&' && tagarg[1]=='#')
    {
     entity2str(tagarg);
    }
//!!glennmcc: end

       if(!strncmpi(tagarg,"mailto:",7))
//!!glennmcc: Apr 23, 2008 -- convert unicoded 'hot links'
{
//!!glennmcc; Jan 24, 2009 -- better fix ;-)
   if(strstr(tagarg,"&#"))
// if(tagarg[7]=='&' && tagarg[8]=='#')
    {
     entity2str(tagarg);
    }
//!!glennmcc: end
	target=findtarget(0);
}//!!glennmcc: Apr 23, 2008
       else
	target=findtarget(basetarget);

       //vlozit link:
       //tr.: insert link
       if(tagarg[0]!='#')
       {
   AnalyseURL(tagarg,&url,p->currentframe); //(plne zneni...)
   //tr.: entire, full, complete version/text
   url2str(&url,text);
   if(strstr(text,"&amp;"))
    entity2str(text);
	tagarg=text;
       }

       pushfont(font,style,&HTMLatom,&fontstack);
       sheet=locatesheet(htmldata,&tmpsheet,stylesheetadr);
       if(sheet->usehover)
   HTMLatom.R=1;
       else
   HTMLatom.R=0;

       //vyrobim si pointr na link, a od ted je vsechno link:
       //tr.: I create a pointer for link, and from now on
       //     everything will be a link
       addatom(&HTMLatom,tagarg,strlen(tagarg),HREF,align,target,removable,sheet->myadr,1);
       currentlink=p->lastHTMLatom;

       style|=sheet->ahrefsetbits;
       style-=(style&sheet->ahrefresetbits);
       if(sheet->ahreffontsize!=-1)
   font=sheet->ahreffontsize;
       HTMLatom.R=sheet->linkR;
       HTMLatom.G=sheet->linkG;
       HTMLatom.B=sheet->linkB;
       fixrowsize(font,style);
      }

      if(getvar("NAME",&tagarg))
      {
       addatom(&HTMLatom,tagarg,strlen(tagarg),NAME,align,0,0,IE_NULL,1);
      }
     }
     break;

     case TAG_SLASH_A:

     if(currentlink==IE_NULL)
      break;

     currentlink=IE_NULL;

     if(!popfont(&font,&style,&HTMLatom,&fontstack))
     {
      style=htmldata->basefontstyle;
      HTMLatom.R=htmldata->textR;
      HTMLatom.G=htmldata->textG;
      HTMLatom.B=htmldata->textB;
     }
     fixrowsize(font,style);
     break;

     case TAG_IMG: //<IMG>
//!!glennmcc: Dec 03, 2005 - optionally do not display any remote images
   if((!strncmpi(cache->URL,"http:",5) || !strncmpi(cache->URL,"ftp:",4)) &&
       config_get_bool("IgnoreImages", 0))
      break;
//!!glennmcc: end
     input_image=0;
     process_input_image:
     {
      char imgalign,ismap=0; //ismap enum | 1=ISMAP,2=USEMAP,4=INPUT TYPE=..
      char border=0;
      unsigned imglink;
      char imgleft=0;
      char imgright=0;
      char znamrozmerx=0,znamrozmery=0;
      char ext[5];
      char failedGIF=0;

      init_picinfo(img);

      if(input_image) //   ...  INPUT TYPE=IMAGE
      {
       ismap=IMG_INPUT;
       imglink=currentform;
      }
      else
      {
       imglink=currentlink;
       if(imglink!=IE_NULL)
   border=1;
      }

      if(getvar("SRC",&tagarg) && tagarg[0])
      {
       unsigned status;
       XSWAP dummy;
       struct HTTPrecord HTTPdoc;

       AnalyseURL(tagarg,&url,p->currentframe);
       url2str(&url,img->URL);

//!!glennmcc: Nov 29, 2006 -- fix problems with '&amp;' in <img src="
   if(strstr(img->URL,"&amp;"))
   {
    entity2str(img->URL);
    tagarg=img->URL;
   }
//!!glennmcc: end

       //printf("Image URL is: %s\n",img->URL);

       if(QuickSearchInCache(&url,&HTTPdoc,&dummy,&status))
       {
	strcpy(img->filename,HTTPdoc.locname);

   if(img->filename[0])
   {
    img->sizeonly=1;

    get_extension(HTTPdoc.mime,ext);
    //printf("image extension: %s\n",ext);
    if(strcmpi(ext,"IKN")) // != IKN
	 {
     if(drawanyimage(img)==1)
      znamrozmerx=znamrozmery=1;
	  else
	   failedGIF=1;
    }
    else
    if(!cgamode)
    {
     img->size_y=60;
     img->size_x=60;
     znamrozmerx=znamrozmery=1;
    }
	}//endif nasel jsem neco (tr.: I have found something)
       }
      }
      else
      if(status==LOCAL)
       img->URL[0]='\0';

      imgalign=align-BOTTOM;
      if(getvar("ALIGN",&tagarg))
      {
       if(!strcmpi(tagarg,"TOP"))
   imgalign = imgalign | TOP;
       else
       if(!strcmpi(tagarg,"MIDDLE"))
   imgalign = imgalign | MIDDLE;
       else
       if(!strcmpi(tagarg,"RIGHT"))
   imgright=1;
       else
       if(!strcmpi(tagarg,"LEFT"))
   imgleft=1;
       else
   imgalign = imgalign | BOTTOM;
      }
      else
       imgalign = imgalign | BOTTOM;


      if(getvar("ISMAP",&tagarg))
      {
       ismap=ismap|IMG_ISMAP;
      }

      if(getvar("USEMAP",&tagarg))
      {
       ismap=IMG_USEMAP;
       addatom(&HTMLatom,tagarg,strlen(tagarg),USEMAP,align,0,0,IE_NULL,1);
       imglink=p->lastHTMLatom;
       search4maps=1;
      }

      if(getvar("BORDER",&tagarg))
      {
       if(*tagarg)
   border=(char)atoi(tagarg);
       else
   border=1;
      }

      if(getvar("HEIGHT",&tagarg))
      {
       int i=try2getnum(tagarg,frame->scroll.ysize);
       if(i<2)i=2;//!!glennmcc: Feb 15, 2005 -- fixes 'sticky mouse'
       if((!znamrozmery || i!=img->size_y) && !egamode && !vga16mode)
       {
	img->resize_y=i;
	img->resize_x=img->size_x;
       }
       img->size_y=i;
       znamrozmery=1;
      }

      if(getvar("WIDTH",&tagarg))
      {
       int max=p->docRight-p->docLeft;
       int i;
       if(max<0) max=0;
       i=try2getnum(tagarg,max);
       if(i<2)i=2;//!!glennmcc: Feb 15, 2005 -- fixes 'sticky mouse'
       if((!znamrozmerx || i!=img->size_x) && !egamode && !vga16mode)
       {
   img->resize_x=i;
   if(!img->resize_y)
    img->resize_y=img->size_y;
       }
       img->size_x=i;
       znamrozmerx=1;
      }

      //kill adds (468x60)?
      if(user_interface.killadds && img->size_x==468 && img->size_y==60)
       break;
//!!glennmcc: July 08, 2006 -- kill any image wider than 'MaxImgWidth'
//defaults to max 2048 if variable missing from CFG
//min setting 100
maxwidth = config_get_int("MaxImgWidth", 2048);
if (maxwidth <  100) maxwidth = 100;
if (maxwidth > 2048) maxwidth = 2048;
if(img->size_x > maxwidth)
   break;
//!!glennmcc: end

      if(getvar("ALT",&tagarg) || getvar("NAME",&tagarg))
      {
       strcpy(text,tagarg);
       entity2str(text);
       makestr(img->alt,text,79);
      }
      else if(input_image)
       img->alt[0]='\0';
      else
       strcpy(img->alt,"IMAGE");

      if(!znamrozmerx)
       img->size_x=8*strlen(img->alt)+2;

      if(!znamrozmery) img->size_y=fonty(SYSFONT,0)+2;

      img->size_x+=2*border;
      img->size_y+=2*border;

      if(!znamrozmerx || !znamrozmery || failedGIF)
       GLOBAL.needrender=1; //we don't know all image sizes...

      //vlastni vlozeni obrazku:
      //tr.: own/separate inserting of picture

      if(imgleft)
      {
       if(x>p->docLeft)
   HTMLatom.y=y+p->sizeRow;
       else
       {
   HTMLatom.y=y;
   x+=img->size_x;
       }
       HTMLatom.x=p->docLeft;
//!!glennmcc & Ray_Andrews: Nov 22, 2006 -- fix 'stair-step' effect in <DL><DT><DD> lists
//example: http://www.cisnet.com/glennmcc/my-stuff/aligntest.htm
if(orderedlist[listdepth])
       HTMLatom.xx=HTMLatom.x+img->size_x; //this fixes it
   else
      {
       p->docLeft+=img->size_x;  //original line
       HTMLatom.xx=p->docLeft;   //original line
      }
//!!glennmcc & Ray_Andrews: end
       HTMLatom.yy=HTMLatom.y+img->size_y;
       if(HTMLatom.yy>p->docClearLeft)
   p->docClearLeft=HTMLatom.yy;

       if(p->docLeft>p->docRight)
   clearall(&y);

       imgalign=0; //a hlavne tady: rikam, ze uz s tim nebudu hejbat !!!
       //tr.: and mainly here: I tell you, that I won't do any more changes
      }
      else if(imgright)
      {
       if(p->docLeft+img->size_x>p->docRight)
   clearall(&y);

       if(p->docRight-img->size_x<x)
       {
   clearall(&y);
   x=p->docLeft;
       }

       if(p->docLeft+img->size_x>p->docRight)
       {
   p->docRight+=img->size_x;
   p->docRightEdge=p->docRight;
       }

       HTMLatom.xx=p->docRight+1;
       HTMLatom.y=y;
       p->docRight-=img->size_x;
       HTMLatom.x=p->docRight+1;
       HTMLatom.yy=y+img->size_y;
       if(HTMLatom.yy>p->docClearRight)
   p->docClearRight=HTMLatom.yy;

       imgalign=0; //a hlavne tady: rikam, ze uz s tim nebudu hejbat !!!
       //tr.: and mainly here: I tell you, that I won't do any more changes
      }
      else
      {
       //normalni - misto jednoho atomu
       //tr.: normal - location of on atom
       //nevejde se nam tam ?
       //tr.: does not fit into there?
       if(x+img->size_x>p->docRight)
       {
   if(!pre && !nownobr)
	{
         if(img->size_x<=p->docRight-p->docLeft && x>p->docLeft ||
	    !tabledepth && !p->docClearLeft && !p->docClearRight)
    {
     alignrow(x,y,orderedlist[listdepth]);
          y+=p->sizeRow;
	  x=p->docLeft;
          p->sizeRow=0; //!hned spravim (tr.: I will repair it immediately)
    }
    else clearall(&y);
   }
       }

       if(p->sizeRow<img->size_y)
        p->sizeRow=img->size_y;

       if(imgalign & BOTTOM)
   p->sizeTextRow=p->sizeRow;
       else if(imgalign & MIDDLE && p->sizeTextRow<p->sizeRow/2)
	p->sizeTextRow=p->sizeRow/2;

       HTMLatom.x=x;
       HTMLatom.y=y;
       x+=img->size_x;
       HTMLatom.xx=x;
       HTMLatom.yy=y+img->size_y;
      }

      addatom(&HTMLatom,img,sizeof(struct picinfo),IMG,imgalign,ismap,border,imglink,imgright);
      //imgright=1 --> neposouvat pravy okraj!!!
      //tr.: do not move the right margin

      nownobr=nobr;
      lastspace=1;//mazat mezery (tr. erase spaces)
     }
     break;

     case TAG_BR: //<BR>
     if(p->xsum>p->maxsum)
      p->maxsum=p->xsum;
     p->xsum=0;
     if(p->sizeRow==0)
      p->sizeTextRow=p->sizeRow=fonty(font,style);
     br:

     alignrow(x,y,orderedlist[listdepth]);
     y+=p->sizeRow;

     if(getvar("CLEAR",&tagarg))
     {
      if(p->docClearLeft>y && !strcmpi(tagarg,"LEFT"))
      {
       y=p->docClearLeft;
       p->docClearLeft=0;
       p->docLeft=p->docLeftEdge;
      }
      else
      if(p->docClearRight>y && !strcmpi(tagarg,"RIGHT"))
      {
       y=p->docClearRight;
       p->docClearRight=0;
       p->docRight=p->docRightEdge;
      }
      else
      if(!strcmpi(tagarg,"ALL"))
      {
       if(p->docClearLeft>y && p->docClearLeft>=p->docClearRight)
       {
   y=p->docClearLeft;
       }
       else
       if(p->docClearRight>y)
       {
   y=p->docClearRight;
       }
       p->docLeft=p->docLeftEdge;
       p->docRight=p->docRightEdge;
       p->docClearLeft=p->docClearRight=0;

      }
     }

     linebreak:
     p->sizeTextRow=p->sizeRow=fonty(font,style); //?p->sizeRow?
     x=p->docLeft;
     lastspace=1;//mazat mezery (tr. spaces)
     HTMLatom.x=x;
     HTMLatom.y=y;
     break;

    //  -----------------------------------------(netscape extensions block)
     case TAG_CENTER: //<CENTER>

     align=align | CENTER;
     basealign[tabledepth]=align;
     centerdepth[tabledepth]++;
     if(x>p->docLeft)
      goto br;

     break;

     case TAG_SLASH_CENTER:

     if(centerdepth[tabledepth])
      centerdepth[tabledepth]--;
     if((align & CENTER) && !centerdepth[tabledepth])
     {
      align=align - CENTER;
      basealign[tabledepth]=align;
     }
     if(x>p->docLeft)
      goto br;
     break;

     case TAG_SUP: //<SUP>

     align=align | SUP;
     font-=1;
     if(p->sizeRow<3*fonty(font,style)/2)
      p->sizeRow+=fonty(font,style)/2;
     if(pre) p->sizeRow+=fonty(font,style)/2;
     break;

     case TAG_SLASH_SUP:

     if(align & SUP)
     {
      align=align - SUP;
      font=htmldata->basefontsize;
     }
//!!glennmcc: Mar 07, 2008 -- allow use of <sup> within <pre>
if(pre) alignrow(x,y,0);
//!!glennmcc: end
     break;

     case TAG_SUB: //<SUB>

     align=align | SUB;
     font-=1;
     if(p->sizeRow<3*fonty(font,style)/2)
      p->sizeRow+=fonty(font,style)/2;
     break;

     case TAG_SLASH_SUB:

     if(align & SUB)
     {
      align=align - SUB;
      font=htmldata->basefontsize;
     }
//!!glennmcc: Mar 07, 2008 -- allow use of <sub> within <pre>
if(pre) alignrow(x,y,0);
//!!glennmcc: end
     break;

     case TAG_NOBR: //</NOBR>

     nobr=1;
     nownobr=0;
     boom=0;
     nobr_x_anchor=x;
     if(p->xsum>p->maxsum)
      p->maxsum=p->xsum;
     p->xsum=0;
     if(!tabledepth)
     {
      pre_Right=p->docRight;
      pre_RightEdge=p->docRightEdge;
     }

     if(currentnobr<MAXNOBR)
     {
      if((GLOBAL.validtables && nobr_overflow[currentnobr] || noresize) && x>p->docLeft)
       goto br;
      else
       nobr_overflow[currentnobr]=0;

     }

     break;

     case TAG_SLASH_NOBR:

     if(nobr && boom && !GLOBAL.validtables)
     {
      RENDER.willadjusttables=1;
      nobr_overflow[currentnobr]=1;
      x=nobr_x_anchor;
      y+=p->sizeRow;
     }
     if(p->xsum>p->maxsum)
      p->maxsum=p->xsum;
     p->xsum=0;
     nobr=0;
     nownobr=0;
     if(currentnobr<MAXNOBR-1)
      currentnobr++;
     if(!tabledepth)
     {
      if(pre_Right<p->docRight)
       p->docRight=pre_Right;
      if(pre_RightEdge<p->docRightEdge)
       p->docRightEdge=pre_RightEdge;
     }
     break;

     case TAG_SLASH_P: //</P>
     case TAG_SLASH_DIV: //</DIV>

     align=basealign[tabledepth];
     goto p;

     // --------------------------------------------- HTML level 1 - headers
     case TAG_H1: //<H1>

     pushfont(font,style,&HTMLatom,&fontstack);
     font=6;
     header:
     style=BOLD;
     PARAGRAPH;
     p->sizeTextRow=p->sizeRow=fonty(font,style);
     goto alignset;

     case TAG_H2: //<H2>

     pushfont(font,style,&HTMLatom,&fontstack);
     font=5;
     goto header;

     case TAG_H3: //<H3>

     pushfont(font,style,&HTMLatom,&fontstack);
     font=4;
     goto header;

     case TAG_H4: //<H4>

     pushfont(font,style,&HTMLatom,&fontstack);
     font=3;
     goto header;

     case TAG_H5: //<H5>

     pushfont(font,style,&HTMLatom,&fontstack);
     font=2;
     goto header;

     case TAG_H6: //<H6>

     pushfont(font,style,&HTMLatom,&fontstack);
     font=1;
     goto header;

     //</H1>,</H2>,</H3>,</H4>,</H5>,</H6>
     case TAG_SLASH_H1:
     case TAG_SLASH_H2:
     case TAG_SLASH_H3:
     case TAG_SLASH_H4:
     case TAG_SLASH_H5:
     case TAG_SLASH_H6:

     if(!popfont(&font,&style,&HTMLatom,&fontstack))
     {
      font=htmldata->basefontsize;
      style=htmldata->basefontstyle;
     }
     align=basealign[tabledepth];
     goto p;

     case TAG_TT: //<TT>

     style=style | FIXED;
     fixrowsize(font,style);
     break;

     case TAG_SLASH_TT:

     if(style & FIXED)
      style -= FIXED;
     fixrowsize(font,style);
     break;

     case TAG_PRE: //<PRE>
     if (config_get_bool("WrapPre", 0)) {
      pre = 0;
      break;
     }
     font=htmldata->basefontsize;
     style=FIXED;
     pre=1;
     if(!tabledepth)
     {
      pre_Right=p->docRight;
      pre_RightEdge=p->docRightEdge;
     }
     fixrowsize(font,style);
     break;

     case TAG_SLASH_PRE:

     font=htmldata->basefontsize;
     style=htmldata->basefontstyle;
     pre=0;
     if(!tabledepth)
     {
      if(pre_Right<p->docRight)
       p->docRight=pre_Right;
      if(pre_RightEdge<p->docRightEdge)
       p->docRightEdge=pre_RightEdge;
     }

     goto br;

     case TAG_FONT:     //<FONT>
     case TAG_BASEFONT: //<basefont>

     pushfont(font,style,&HTMLatom,&fontstack);
     if(getvar("SIZE",&tagarg))
     {
      if(tagarg[0]=='+')
       font+=atoi(&tagarg[1]);
      else
      if(tagarg[0]=='-')
       font-=atoi(&tagarg[1]);
      else
   font=atoi(tagarg);
      if(font<1)font=1;
      if(font>8)font=8; //! tady do budoucna pocitam s rozsirenim jako Netscape!
      //here, for the future I plan extension/improvement like/similar Netscape
     }

//!!glennmcc: Nov 27, 2007 -- also enable AlwaysUseCFGcolors for font colors
// in addition to text color in <body> tag
     if(getvar("COLOR",&tagarg) && (!user_interface.alwaysusecfgcolors || !strcmpi(url.protocol,"file")))
//   if(getvar("COLOR",&tagarg)) //original line
     {
      try2readHTMLcolor(tagarg,&(HTMLatom.R),&(HTMLatom.G),&(HTMLatom.B));
     }

     if(getvar("3D",&tagarg))
     {
      if(tagarg[0]=='2')
       style|=TEXT3D2;
      else
       style|=TEXT3D;
     }

     /*
     if(getvar("OUTLINE",&tagarg))
     {
      struct Fontdecoration decoration;

      HTMLatom.x=0;
      HTMLatom.xx=0;
      HTMLatom.y=0;
      HTMLatom.yy=0;
      addatom(&HTMLatom,decoration,sizeof(struct Fontstyle),DECORATION,BOTTOM,0,0,IE_NULL,0);
     }
     */

     if(tag==TAG_BASEFONT) // <basefont>
      htmldata->basefontsize=font;

     fixrowsize(font,style);
     break;

     case TAG_SLASH_BASEFONT: // </basefont>

     htmldata->basefontsize=3; //normalni velikost (tr.: normal size)
     // continue...

     case TAG_SLASH_FONT:

     if(!popfont(&font,&style,&HTMLatom,&fontstack))
     {
      font=htmldata->basefontsize;
      HTMLatom.R=htmldata->textR;
      HTMLatom.G=htmldata->textG;
      HTMLatom.B=htmldata->textB;
      if(style & TEXT3D)
       style -= TEXT3D;
     }
     break;

     case TAG_SLASH_FRAMESET:

     if(arachne.framescount>=p->currentframe)
      goto br;

     case TAG_BIG: //<BIG>

     font+=2;
     break;

     case TAG_SMALL: //<SMALL>

     font-=1;
     break;

     case TAG_SLASH_BIG:
     case TAG_SLASH_SMALL:

     font=htmldata->basefontsize;
     break;

     case TAG_HR: //<HR>

     {
      int size=2,noshade=0,hralign=CENTER;
      int width;

      if(fixedfont)
       size=FIXEDFONTY;
      else
      if(getvar("SIZE",&tagarg))
      {
       size=atoi(tagarg);
      }

      if(getvar("NOSHADE",&tagarg))
      {
       noshade=1;
      }

      if(getvar("ALIGN",&tagarg))
      {
       if(!strcmpi(tagarg,"RIGHT"))
   hralign=RIGHT;
       else
       if(!strcmpi(tagarg,"LEFT"))
   hralign=LEFT;
      }

      if(getvar("WIDTH",&tagarg))
      {
       width=try2getnum(tagarg,p->docRight-p->docLeft);
      }
      else
       width=p->docRight-p->docLeft;

      alignrow(x,y,orderedlist[listdepth]);
      if(x>p->docLeft)y+=p->sizeRow;
      p->sizeRow=fonty(htmldata->basefontsize,0);
      if(size+4>p->sizeRow)p->sizeRow=size+4;
      x=p->docLeft;
      HTMLatom.x=x;
      HTMLatom.y=y+p->sizeRow/2-size/2;
      HTMLatom.xx=x+width;
      HTMLatom.yy=y+p->sizeRow/2-size/2+size;
      addatom(&HTMLatom,"",0,HR,hralign,noshade,0,IE_NULL,0);
      alignrow(HTMLatom.xx,HTMLatom.y,orderedlist[listdepth]);
      y+=p->sizeRow;
      p->sizeTextRow=p->sizeRow=fonty(font,style);
     }
     break;

     case TAG_B: //<B>,<STRONG>

     style=style | BOLD;
     fixrowsize(font,style);
     break;

     case TAG_SLASH_B:

     if(style & BOLD)
      style -= BOLD;
     fixrowsize(font,style);
     break;

     case TAG_U: //<U>

     style=style | UNDERLINE;
     fixrowsize(font,style);
     break;

     case TAG_SLASH_U:

     if(style & UNDERLINE)
      style -= UNDERLINE;
     fixrowsize(font,style);
     break;

//!!JDS: Feb 28, 2007 -- add support for <S> (strike)
//(see code in htmldraw.c)
     case TAG_S: //<S>

     style=style | STRIKE;
     fixrowsize(font,style);
     break;

     case TAG_SLASH_S:

     if(style & STRIKE)
      style -= STRIKE;
     fixrowsize(font,style);
     break;
//!!JDS: end

     case TAG_I: //<I>,<ADDRESS>,<CITE>

     style=style | ITALIC;
     fixrowsize(font,style);
     break;

     case TAG_SLASH_I:

     if(style & ITALIC)
      style -= ITALIC;
     fixrowsize(font,style);
     break;

#ifdef TABLES
     // =========================================================== //<TABLE>
     case TAG_TABLE:


     //hack for missing <TD> elements - Netscape's invention...
     if(tabledepth && !istd)
      goto tag_td;
     tag_table:

     // <TABLE> tag analysing code starts here
     // =====================================================================
     {
      char border=0,newtab=0,tabalign=0;
      int newx,twidth,alignarg;

//!!glennmcc: July 13, 2006
tablecount++;
if(tablecount>(MAXTABLEDEPTH*10)) goto p;
//!!glennmcc:end
      //if there are too many nested tables, than we will give up...
      if(tabledepth>MAXTABLEDEPTH)
       goto p;

      sheet=locatesheet(htmldata,&tmpsheet,stylesheetadr);

      //backup-of temporary table structure - 16bit DOS only, XSWAP pointers are persistent in POSIX
      if(thistableadr!=IE_NULL)
      {
       tmptable=(struct HTMLtable *)ie_getswap(thistableadr);
       if(tmptable)
       {
   memcpy(tmptable,thistable,sizeof(struct HTMLtable));
   swapmod=1;
   tableptrstack[tabledepth]=thistableadr;
   thistableadr=IE_NULL;
       }
      }

      //some alignment...
      if(x>p->docLeft)
      {
       alignrow(x,y,orderedlist[listdepth]);
       y+=p->sizeRow;
       x=p->docLeft;
      }

      //alocation of new table (max. number of tables is currently limited)
      if(p->nextHTMLtable==IE_NULL)
      {
       newtab=1;
       thistableadr=ie_putswap((char *)thistable,sizeof(struct HTMLtable),CONTEXT_TABLES);
       if(thistableadr==IE_NULL)
	goto p;

       if(p->firstHTMLtable==IE_NULL)
	p->firstHTMLtable=thistableadr;

       if(p->prevHTMLtable!=IE_NULL)
       {
   tmptable=(struct HTMLtable *)ie_getswap(p->prevHTMLtable);
   if(tmptable)
	{
    tmptable->nextHTMLtable=thistableadr;
    swapmod=1;
   }
	else
    MALLOCERR();
       }
       p->prevHTMLtable=thistableadr;

      }
      else
      {
       tmptable=(struct HTMLtable *)ie_getswap(p->nextHTMLtable);
       if(tmptable)
       {
   memcpy(thistable,tmptable,sizeof(struct HTMLtable));
   thistableadr=p->prevHTMLtable=p->nextHTMLtable;
	p->nextHTMLtable=tmptable->nextHTMLtable;
       }
       else
	MALLOCERR();
      }

      //border attribute: stored both in table structure and in HTML atom...
      if(getvar("BORDER",&tagarg))
      {
       if(tagarg[0] && tagarg[0]>='0' && tagarg[0]<='9')
   border=atoi(tagarg);
       else
   border=2; // BORDER="BORDER"
      }

      //HTML/4.0 attribute: should table border be visible ?
      if(getvar("FRAME",&tagarg) && border &&
	(toupper(tagarg[0])=='V' || toupper(tagarg[0])=='N' )) // FRAME="VOID"
       border=-1;

      //let's do this only for NEW tables:
      if(GLOBAL.validtables==TABLES_UNKNOWN || newtab)
      {
       inittable(thistable);
       if(newtab)
   thistable->nextHTMLtable=IE_NULL;

       if(fixedfont)
       {
	thistable->cellspacing=FIXEDFONTY;
	thistable->cellpadding=0;
       }
       else
       {
   if(getvar("CELLSPACING",&tagarg))
	 thistable->cellspacing=atoi(tagarg);
   else
    thistable->cellspacing=2;

	if(getvar("CELLPADDING",&tagarg))
    thistable->cellpadding=atoi(tagarg);
   else
	 thistable->cellpadding=2;
       }
      }

      //table width consists of COLS columns and COLS+1 cellspacings:
      //table columns include exactly 2 cellpaddings.
      //table border is not included in table->maxwidth (?)
      //final width of HTMLatom should be equal to table->realwidth (?)

      thistable->maxwidth=p->docRight-p->docLeft-2*border;
//!!glennmcc: Sep 24, 2007 -- fix problem of 'empty width'
//eg: <table width="">
      if(getvar("WIDTH",&tagarg) && *tagarg)
//    if(getvar("WIDTH",&tagarg))
//original line above this comment
//!!glennmcc: end
      {
       char *perc=strchr(tagarg,'%');
       thistable->maxwidth=try2getnum(tagarg,thistable->maxwidth);
       if(perc)
       thistable->fixedmax=PERCENTS_FIXED_TABLE;
       else
       {
	thistable->fixedmax=PIXELS_FIXED_TABLE;
	thistable->maxwidth-=2*border;
       }
      }

      if(thistable->maxwidth<0)
       thistable->maxwidth=0;

      //expand
      if(GLOBAL.validtables==TABLES_EXPAND)
       expand(thistable);

      //twidth is width of table, which is valid for current pass
      if(GLOBAL.validtables/* &&
	 (thistable->realwidth>thistable->maxwidth/2 || !tabledepth)*/)
       twidth=thistable->realwidth;
      else
       twidth=thistable->maxwidth;

      HTMLatom.x=p->docLeft;
      HTMLatom.xx=HTMLatom.x+twidth;

       if(HTMLatom.xx>p->docRight &&
     (GLOBAL.validtables!=TABLES_UNKNOWN || thistable->fixedmax==PIXELS_FIXED_TABLE))
	clearall(&y);

// we dont really want this....
//      if(HTMLatom.xx>p->docRight)
//       p->docRight=HTMLatom.xx;

      alignarg=getvar("ALIGN",&tagarg);
      if(alignarg || (align & RIGHT) || (align & CENTER))
      {
       if(GLOBAL.validtables==TABLES_UNKNOWN)
	RENDER.willadjusttables=1;
       if(!strcmpi(tagarg,"LEFT"))
       {
	if(GLOBAL.validtables==TABLES_UNKNOWN && thistable->fixedmax==0)
   {
    thistable->maxwidth/=2;
	 twidth/=2;
   }
   tabalign=LEFT;
       }
       else
       if(!strcmpi(tagarg,"RIGHT") || !alignarg && (align & RIGHT))
       {
	if(GLOBAL.validtables==TABLES_UNKNOWN && thistable->fixedmax==0)
	{
    thistable->maxwidth/=2;
	 twidth/=2;
   }
   HTMLatom.x=p->docRight-FUZZYPIX-twidth;
	if(HTMLatom.x<p->docLeft)
   {
    clearall(&y);
    HTMLatom.x=p->docLeft;
	}
   HTMLatom.xx=p->docLeft+twidth;
   tabalign=RIGHT;
       }
       else
       if(!strcmpi(tagarg,"CENTER") || !alignarg && (align & CENTER))
       {
   newx=(int)(p->docRight-p->docLeft-twidth)/2;
   if(newx<p->docLeft || GLOBAL.validtables==TABLES_UNKNOWN)
	 HTMLatom.x=p->docLeft;
   else
    HTMLatom.x=p->docLeft+newx;
   HTMLatom.xx=HTMLatom.x+twidth;
	tabalign=CENTER;
       }
      }

      if(!alignarg) //in this case, table won't behave like aligned image
       tabalign=0;

//!!glennmcc: Nov 28, 2007 -- also enable AlwaysUseCFGcolors for TABLE bgcolor
// in addition to text color in <body> tag
      if(getvar("BGCOLOR",&tagarg) && (!user_interface.alwaysusecfgcolors || !strcmpi(url.protocol,"file")))
//    if(getvar("BGCOLOR",&tagarg)) //original line
      {
       try2readHTMLcolor(tagarg,&(thistable->tablebgR),&(thistable->tablebgG),&(thistable->tablebgB));
       thistable->usetablebg=1;
      }
      else
      if (sheet->usetdbgcolor)
      {
       thistable->usetablebg=1;
       thistable->tablebgR=sheet->tdbgR;
       thistable->tablebgG=sheet->tdbgG;
       thistable->tablebgB=sheet->tdbgB;
      }

      if(thistable->usetablebg)
      {
       thistable->userowbg=1;
       thistable->rowbgR=thistable->tablebgR;
       thistable->rowbgG=thistable->tablebgG;
       thistable->rowbgB=thistable->tablebgB;
      }

      img->URL[0]='\0';
      if(config_get_bool("BGimages", 1) &&
      getvar("BACKGROUND",&tagarg) && tagarg[0] && !cgamode && strcmp(tagarg,"0")) //???
      {
       AnalyseURL(tagarg,&url,p->currentframe);
       url2str(&url,img->URL);
       init_picinfo(img);
       img->URL[URLSIZE-1]='\0';
      }

      HTMLatom.y=y;
      HTMLatom.yy=y+2*border;
      listdepthstack[tabledepth]=listdepth;
      orderedlist[++listdepth]=0;
      stackLeft[tabledepth]=p->docLeft;
      stackRight[tabledepth]=p->docRight;
      stackLeftEdge[tabledepth]=p->docLeftEdge;
      stackRightEdge[tabledepth]=p->docRightEdge;
      clearstackLeft[tabledepth]=p->docClearLeft;
      clearstackRight[tabledepth]=p->docClearRight;
      if(p->xsum>p->maxsum)
       p->maxsum=p->xsum;
      maxsumstack[tabledepth]=p->maxsum;
      p->maxsum=0l;
      pushfont(font,style,&HTMLatom,&fontstack);
      fontstackdepth[tabledepth]=fontstack.depth;
      alignstack[tabledepth]=align;
      tabledepth++;
      currentcell[tabledepth]=IE_NULL;
      basealign[tabledepth]=align;

      thistable->depth=tabledepth; //for resizing optimization
      if(tabledepth>GLOBAL.tabledepth)
       GLOBAL.tabledepth=tabledepth;

      //initizalizations for both rendering passes:
      if(GLOBAL.validtables)
       thistable->maxwidth=thistable->realwidth;

      thistable->x=0;
      thistable->y=0;
      thistable->tdstart=HTMLatom.y+border+thistable->cellspacing;
      thistable->nexttdend=thistable->tdend=thistable->maxtdend=thistable->tdstart;
      memset(thistable->rowspan,0,sizeof(char)*MAXTD);

      thistable->valignrow=MIDDLE;

      //ulozim tabulku do seznamu tabulek
      //tr.: I put/save the table into the list of tables
      tmptable=(struct HTMLtable *)ie_getswap(thistableadr);
      if(tmptable)
      {
       memcpy(tmptable,thistable,sizeof(struct HTMLtable));
       swapmod=1;
      }
      else
       MALLOCERR();

      //vyrobim si pointer na tabulku:
      //tr.: I create a pointer for the table
      if(img->URL[0])
       addatom(&HTMLatom,img,sizeof(struct picinfo),TD_BACKGROUND,TOP,border,tabalign,thistableadr,1);
      else
       addatom(&HTMLatom,"",0,TABLE,TOP,border,tabalign,thistableadr,1);
      currenttable[tabledepth]=p->lastHTMLatom;

      //a v seznamu je na rade dalsi tabulka...
      //tr.: and it is the turn of the next table in the list
      //Tablelist.cur++;
      istd=0;
     }
     break;

#ifdef TABLES
     case TAG_SLASH_TABLE: //</TABLE>

     tag_slash_table:
     {
      int cellx;
      long celly;
      int tblstart;
      long tblystart;
      char border,tabalign;

      if(tabledepth && currenttable[tabledepth]!=IE_NULL)
      {
       clearall(&y);
       if(x>p->docLeft)
   {
    alignrow(x,y,orderedlist[listdepth]);
    y+=p->sizeRow;
   }

       //uzavreni posledniho policka
       //tr.: closing the last small field
       atomptr=(struct HTMLrecord *)ie_getswap(currenttable[tabledepth]);
       if(atomptr)
       {
   XSWAP parenttableadr=atomptr->linkptr;

   tblstart=atomptr->x;
   tblystart=atomptr->y;
   border=atomptr->data1;
   tabalign=atomptr->data2;

   if(currentcell[tabledepth]!=IE_NULL) //uzavrit posledni ctverecek na radce
   //tr.: close the last small square on the line
   {
    if(thistableadr==parenttableadr)
     tmptable=thistable;
    else
     tmptable=(struct HTMLtable *)ie_getswap(parenttableadr);

    if(tmptable)
    {
     // fix desired table cell width data:
     // tabledepth>1 == we are wrapped in another table cell...
     if(p->xsum>p->maxsum)
      p->maxsum=p->xsum;
     if(tabledepth>1)
     {
      if(tdwidth[tabledepth-1] && tdwidth[tabledepth-1]<p->maxsum)
       p->maxsum=tdwidth[tabledepth-1];
     }

     if(y<tdheight)
      y=tdheight;

     if(processcell(tmptable,p->maxsum,p->docRightEdge-p->docLeftEdge+2*tmptable->cellpadding,
	 y+tmptable->cellpadding,&cellx) && GLOBAL.validtables==TABLES_UNKNOWN)
      RENDER.willadjusttables=1;
     if(thistableadr!=parenttableadr)
      swapmod=1;

     if(noresize || user_interface.quickanddirty || GLOBAL.validtables!=TABLES_UNKNOWN || RENDER.willadjusttables==0) //acceleration
      closeatom(currentcell[tabledepth],cellx,y);
    }
    else
     MALLOCERR();
   }

   //spocitam sirku a zjistim posledni udaje
   //I calculate width and get the last variable
   if(thistableadr==parenttableadr)
    tmptable=thistable;
   else
   {
    tmptable=(struct HTMLtable *)ie_getswap(parenttableadr);
      //printf("tables out of sync");
   }

   if(tmptable)
   {
    XSWAP closeptrs[MAXROWSPANTD+1];
    long start=tmptable->tdstart,end;
    int padding=tmptable->cellpadding;

    fixrowspan(tmptable,1,closeptrs);
    end=tmptable->tdend;

    if(calcwidth(tmptable) && GLOBAL.validtables==TABLES_UNKNOWN)
     RENDER.willadjusttables=1;

    //----------------------------------------------------------------
    //return to previous state of reneding engine - calc max. desired
    //cell width - p->maxsum and p->xsum, where p->maxsum>=p->xsum ...

    {
     long desired=2*border+tmptable->realwidth+tmptable->totalxsum;

     if(tmptable->fixedmax==PIXELS_FIXED_TABLE) //not percent specification!
     {
      desired=2*border+tmptable->maxwidth;
     }

     if(desired>p->maxsum)
      p->maxsum=desired;
    }

    switch(tmptable->fixedmax)
    {
     case PIXELS_FIXED_TABLE:
     if(maxsumstack[tabledepth-1]>p->maxsum)
      p->maxsum=maxsumstack[tabledepth-1];

     if(tmptable->maxwidth>p->maxsum)
      p->maxsum=tmptable->maxwidth;

     default:
     p->xsum=0;
     break;

     case PERCENTS_FIXED_TABLE:
     p->xsum=p->maxsum;
    }

    //----------------------------------------------------------------

    //zapsani zavrene tabulky
    //tr.: writing closed table

    if(thistableadr==parenttableadr)
    {
     tmptable=(struct HTMLtable *)ie_getswap(thistableadr);
     if(tmptable)
     {
      memcpy(tmptable,thistable,sizeof(struct HTMLtable));
     }
    }
    swapmod=1;

    cellx=2*border+tmptable->realwidth;
    celly=tmptable->tdend+tmptable->cellspacing+border;

    //dirty fix of desired width - should be already ok, but just
    if(cellx>p->maxsum)
      p->xsum=p->maxsum=cellx;

    if(closeatom(currenttable[tabledepth],cellx,celly)
       && tabalign && !GLOBAL.validtables)
     RENDER.willadjusttables=1;
    if(tblstart+cellx>frame->scroll.total_x)
    {
     frame->scroll.total_x=tblstart+cellx;
    }

    //zarovnam posledni radek tabulky
    //tr.: I align the last row of the table
    fixrowspan_y(closeptrs,end,padding);
    tablerow(start,end,parenttableadr,padding);
   }
   else
    MALLOCERR();
       }
       else
   MALLOCERR();

       //tablerightedge
       tabledepth--;
       listdepth=listdepthstack[tabledepth];
       p->docLeftEdge=stackLeftEdge[tabledepth];
       p->docLeft=stackLeft[tabledepth];
       p->docRightEdge=stackRightEdge[tabledepth];
       p->docRight=stackRight[tabledepth];
       p->docClearLeft=clearstackLeft[tabledepth];
       p->docClearRight=clearstackRight[tabledepth];
       fontstack.depth=fontstackdepth[tabledepth];
       if(tabledepth)
       {
   thistableadr=tableptrstack[tabledepth];
   tmptable=(struct HTMLtable *)ie_getswap(thistableadr);
   if(tmptable)
    memcpy(thistable,tmptable,sizeof(struct HTMLtable));
   else
    MALLOCERR();
       }
       else
   thistableadr=IE_NULL;

       if(cellx>=p->docRight-p->docLeft && tabledepth>0)
       {
   p->docRightEdge=p->docRight=p->docLeft+cellx;
       }

       align=alignstack[tabledepth];
       currentlink=IE_NULL;
       if(!popfont(&font,&style,&HTMLatom,&fontstack))
       {
   font=htmldata->basefontsize;
   style=htmldata->basefontstyle;
   HTMLatom.R=htmldata->textR;
   HTMLatom.G=htmldata->textG;
   HTMLatom.B=htmldata->textB;
       }
       y=celly;

       /*if(!tabledepth)
   y+=fonty(font,style)/4;*/

       invisibletag=0;
       p->sizeRow=p->sizeTextRow=0;
       nobr=0;
       nownobr=0;
       if(GLOBAL.validtables)
       {
   if(tabalign==LEFT && cellx+FUZZYPIX<frame->scroll.total_x)
   {
    p->docLeft=tblstart+cellx+FUZZYPIX;
    if(p->docRight>p->docLeft)
    {
     if(celly>p->docClearLeft)
      p->docClearLeft=celly;
     y=tblystart;
    }
    else
     p->docLeft=p->docLeftEdge;

   }
   else
   if(tabalign==RIGHT && cellx+FUZZYPIX<frame->scroll.total_x)
   {
    p->docRight=tblstart-FUZZYPIX;
    if(p->docRight>p->docLeft)
    {
     if(celly>p->docClearRight)
      p->docClearRight=celly;
     y=tblystart;
    }
    else
     p->docRight=p->docRightEdge;
   }
       }

       //clear left and right ... AFTER returning to <TABLE ALIGN=.....>
       if(p->docClearRight && y>=p->docClearRight)
       {
   p->docRight=p->docRightEdge;
   p->docClearRight=0;
       }

       if(p->docClearLeft && y>=p->docClearLeft)
       {
   if(orderedlist[listdepth]==0)p->docLeft=p->docLeftEdge;
   p->docClearLeft=0;
       }

       if(!GLOBAL.validtables && (tabalign==LEFT || tabalign==RIGHT || tabalign==CENTER))
   RENDER.willadjusttables=1;

       tdheight=y;
       x=p->docLeft;
       //fixrowsize(font,style);
       if(tabledepth)
   istd=1;
      }
      else
       goto p;
     }
     break;
#endif
     case TAG_TR: //<TR>
     case TAG_SLASH_TR: //<TR>
     case TAG_SLASH_CAPTION: //<CAPTION>

     {
      char bgcolor[SHORTSTR+1];
      char valignrow=MIDDLE;

      if(!getvar("BGCOLOR",&tagarg))
       bgcolor[0]='\0';
      else
       makestr(bgcolor,tagarg,SHORTSTR);

      if(getvar("VALIGN",&tagarg))
      {
       if(!strcmpi(tagarg,"TOP"))
	valignrow=TOP;
       else
       if(!strcmpi(tagarg,"BOTTOM"))
   valignrow=BOTTOM;
      }

      //kvuli caption
      //tr.: because of caption
      if(!GLOBAL.validtables)
       invisibletag=0;
      //pokud jsem uvnitr tabulky
      //tr.: if I am within a table
      if(tabledepth && currenttable[tabledepth]!=IE_NULL)
      {
       clearall(&y);
       if(x>p->docLeft)
       {
   alignrow(x,y,orderedlist[listdepth]);
   y+=p->sizeRow;
       }

       atomptr=(struct HTMLrecord *)ie_getswap(currenttable[tabledepth]);
       if(atomptr)
       {
   XSWAP parenttableadr=atomptr->linkptr;
	int cellx;

   //getswap musim delat pokazde, protoze tabulka je dynamicky ulozena
	//tr.: I have to do getswap every time, becaus the table
   //     is allocated dynamically
   if(thistableadr==parenttableadr)
     tmptable=thistable;
	else
   {
    tmptable=(struct HTMLtable *)ie_getswap(parenttableadr);
      //printf("tables out of sync");
	}
   if(tmptable)
	{
	 if(bgcolor[0])
    {
	  try2readHTMLcolor(bgcolor,&(tmptable->rowbgR),&(tmptable->rowbgG),&(tmptable->rowbgB));
     thistable->userowbg=1;
    }
    else
	 {
     if(tmptable->usetablebg)
     {
      thistable->userowbg=1;
	   tmptable->rowbgR=tmptable->tablebgR;
      tmptable->rowbgG=tmptable->tablebgG;
	   tmptable->rowbgB=tmptable->tablebgB;
	  }
     else
	   thistable->userowbg=0;
    }

    tmptable->valignrow=valignrow;
	 if(thistableadr!=parenttableadr)
     swapmod=1;

    if(currentcell[tabledepth]!=IE_NULL)
	 {

	  // fix desired table cell width data:
	  if(p->xsum>p->maxsum)
      p->maxsum=p->xsum;
	  if(tdwidth[tabledepth] && tdwidth[tabledepth]<p->maxsum)
      p->maxsum=tdwidth[tabledepth];

     if(y<tdheight)
	   y=tdheight;

     if(processcell(tmptable,p->maxsum,p->docRightEdge-p->docLeftEdge+2*tmptable->cellpadding,y+tmptable->cellpadding,&cellx) && GLOBAL.validtables==TABLES_UNKNOWN)
       RENDER.willadjusttables=1;
	  if(thistableadr!=parenttableadr)
      swapmod=1;

	  if(noresize || user_interface.quickanddirty || GLOBAL.validtables!=TABLES_UNKNOWN || RENDER.willadjusttables==0) //acceleration
      closeatom(currentcell[tabledepth],cellx,y);
	  currentcell[tabledepth]=IE_NULL;
     invisibletag=1;
    }
   }
	else
    MALLOCERR();

   if(thistableadr==parenttableadr)
	 tmptable=thistable;
   else
	{
	 tmptable=(struct HTMLtable *)ie_getswap(parenttableadr);
      //printf("tables out of sync");
	}
   if(tmptable)
   {
    XSWAP closeptrs[MAXROWSPANTD+1];
	 long start=tmptable->tdstart,end;

    if(tmptable->x) //prvni <TR> ignorovat! (tr.: ignore first <TR>)
    {
	  fixrowspan(tmptable,0,closeptrs);
     end=tmptable->tdend;
	  tmptable->y++;
	  tmptable->x=0;
     tmptable->tdstart=end+tmptable->cellspacing;
	  tmptable->tdend=tmptable->tdstart;

     if(tmptable->tdend<tmptable->nexttdend)
      tmptable->tdend=tmptable->nexttdend;
	  tmptable->nexttdend=tmptable->tdend;

     if(thistableadr!=parenttableadr)
      swapmod=1;//<-ulozit zmeny! (tr.: save changes)

     fixrowspan_y(closeptrs,end,tmptable->cellpadding);
	  tablerow(start,end,parenttableadr,tmptable->cellpadding);
	 }
   }
	else
    MALLOCERR();
       }
       else
	MALLOCERR();
      }
      else
       goto br;
     }
     istd=0;
     break;

     case TAG_SLASH_TD: //</TD>
     invisibletag=1;
     break;

     case TAG_TD: //<TD> <TH> <CAPTION>
     case TAG_TH:
     case TAG_CAPTION:
     tag_td:
     {
      int cellx,width=0,xspan=1,yspan=1;
      char widthstr[SHORTSTR+1]="\0",perc=0;
      char bgcolor=0,caption=0, noalign=0;
      long newtdheight=0;

      invisibletag=0;
      noalign=1;
      align=BOTTOM;
      sheet=locatesheet(htmldata,&tmpsheet,stylesheetadr);

      if(tag!=TAG_TABLE && getvar("ALIGN",&tagarg))
      {
       if(!strcmpi(tagarg,"CENTER"))
       {
   align=align | CENTER;
	noalign=0;
       }
       else
       if(!strcmpi(tagarg,"RIGHT"))
       {
   align=align | RIGHT;
   noalign=0;
       }
       else
       if(!strcmpi(tagarg,"LEFT"))
   noalign=0;
      }

      basealign[tabledepth]=align;

      if(sheet->tdfontstyle!=-1) //-1 ... nothing special
       style=sheet->tdfontstyle;
      else
      if(tag==TAG_TH)
      {
       style=BOLD;
       if(noalign)
   basealign[tabledepth]=align=CENTER;
      }
      else
      if(tag==TAG_CAPTION)
      {
       if(!GLOBAL.validtables)
     RENDER.willadjusttables=1;
       style=BOLD;
       if(noalign)
   basealign[tabledepth]=align=CENTER;
       caption=1;
       if(!GLOBAL.validtables)
   invisibletag=1;
      }
      else
       style=sheet->basefontstyle;

      clearall(&y);
      if(x>p->docLeft)
      {
       alignrow(x,y,orderedlist[listdepth]);
       y+=p->sizeRow;
      }

      img->URL[0]='\0';
      if(tag!=TAG_TABLE)
      {
       valign=-1;
       if(getvar("VALIGN",&tagarg))
       {
	if(!strcmpi(tagarg,"TOP"))
	 valign=TOP;
   else
	if(!strcmpi(tagarg,"BOTTOM"))
	 valign=BOTTOM;
	 else
	 valign=MIDDLE;
       }

       if(getvar("NOWRAP",&tagarg))
	nobr=1;
	else
	nobr=0;

       if(getvar("COLSPAN",&tagarg))
       {
	xspan=atoi(tagarg);
       }

       if(getvar("ROWSPAN",&tagarg))
       {
	yspan=atoi(tagarg);
       }

//!!glennmcc: Nov 28, 2007 -- also enable AlwaysUseCFGcolors for TD bgcolor
// in addition to text color in <body> tag
//!!glennmcc: Nov 30, 2005 -- compensate for 'empty' bgcolor
//such-as <td bgcolor=>
   if(getvar("BGCOLOR",&tagarg)//)//original line
   && strlen(tagarg)>=3 && strcmpi(&tagarg[1]," ")>0//)//Nov 30, 2005
   && (!user_interface.alwaysusecfgcolors || !strcmpi(url.protocol,"file")))//Nov 28, 2007
//!!glennmcc: end
       {
   try2readHTMLcolor(tagarg,&(HTMLatom.R),&(HTMLatom.G),&(HTMLatom.B));
   bgcolor=1;
       }

       if(config_get_bool("BGimages", 1) &&
       getvar("BACKGROUND",&tagarg) && tagarg[0] && !cgamode)
       {
   AnalyseURL(tagarg,&url,p->currentframe);
	url2str(&url,img->URL);
   init_picinfo(img);
	img->URL[URLSIZE-1]='\0';
       }

      if(getvar("HEIGHT",&tagarg) && yspan==1)
       newtdheight=try2getnum(tagarg,0);

       if(getvar("WIDTH",&tagarg))
	makestr(widthstr,tagarg,SHORTSTR);

      }

      nownobr=0;

      if(tabledepth && currenttable[tabledepth]!=IE_NULL &&
	 !(caption && !GLOBAL.validtables))
      {
       atomptr=(struct HTMLrecord *)ie_getswap(currenttable[tabledepth]);
       if(atomptr)
       {
   char border=atomptr->data1;
	int tblx=atomptr->x;
   XSWAP parenttableadr=atomptr->linkptr;

   if(thistableadr==parenttableadr)
	 tmptable=thistable;
   else
	{
	 tmptable=(struct HTMLtable *)ie_getswap(parenttableadr);
      //printf("tables out of sync");
	}
   if(tmptable)
   {
    if(sheet->usetdbgcolor)
	 {
     HTMLatom.R=sheet->tdbgR;
     HTMLatom.G=sheet->tdbgG;
     HTMLatom.B=sheet->tdbgB;
	  bgcolor=1;
    }
	 else
	 if(!bgcolor && tmptable->userowbg)
    {
	  HTMLatom.R=tmptable->rowbgR;
     HTMLatom.G=tmptable->rowbgG;
     HTMLatom.B=tmptable->rowbgB;
     bgcolor=1;
	 }

    if(valign==-1)
     valign=tmptable->valignrow;

    if(widthstr[0])
	 {
	  char *percstr=strchr(widthstr,'%');
     if(percstr && !noresize) //noresize is hack for <BODY NORESIZE>
	  {
      *percstr='\0';
      perc=atoi(widthstr);
     }
	  else //special case, <BODY NORESIZE> onlye...
     {
      width=try2getnum(widthstr,tmptable->maxwidth-tmptable->cellspacing);
      if(percstr)
	   {
       width-=tmptable->cellspacing;
	   }
	  }
    }

    if(width<=0 || perc)
    {
     if(GLOBAL.validtables)
     {
      width=determine_new_width(tmptable,xspan);
     }
     else
     {
      width=2*tmptable->cellpadding;
	   if(!width)
	    width=1;

	   if(xspan>1 && !GLOBAL.validtables)
       RENDER.willadjusttables=1;
     }
    }

    //we are closing cell opened by previous <TD> tag: -----------
    if(currentcell[tabledepth]!=IE_NULL)
    {

     // fix desired table cell width data:
	  if(p->xsum>p->maxsum)
	   p->maxsum=p->xsum;

     if(tdwidth[tabledepth] && tdwidth[tabledepth]<p->maxsum)
	   p->maxsum=tdwidth[tabledepth];

     if(y<tdheight)
      y=tdheight;

     if(processcell(tmptable,p->maxsum,p->docRightEdge-p->docLeftEdge+2*tmptable->cellpadding,y+tmptable->cellpadding,&cellx) && GLOBAL.validtables==TABLES_UNKNOWN)
       RENDER.willadjusttables=1;
     if(thistableadr!=parenttableadr)
	   swapmod=1;

	  if(noresize || user_interface.quickanddirty || GLOBAL.validtables!=TABLES_UNKNOWN || RENDER.willadjusttables==0) //acceleration
	   closeatom(currentcell[tabledepth],cellx,y);
    }
	 //ok, cell closed. -------------------------------------------
   }
   else
    MALLOCERR();

   if(widthstr[0] && !perc)
    tdwidth[tabledepth]=width; //define maximum TD width (in pixels!)
   else
    tdwidth[tabledepth]=0; //undefined maximum TD width - can expand

//!!glennmcc: May 05, 2007 -- fix problem with <td> widths when
//one was speced in percent and the next is not speced at-all
//eg: <td width="25%"><td>
// value=configvariable(&ARACHNEcfg,"FixTD",NULL);
// if(value && toupper(*value)=='Y')
//!!Ray: Sep 22, 2007
if (perc>99) perc=0;
//!!glennmcc: end

	if(thistableadr==parenttableadr)
	 tmptable=thistable;
   else
	{
    tmptable=(struct HTMLtable *)ie_getswap(parenttableadr);
      //printf("tables out of sync");
   }
	if(tmptable)
   {
    if(caption)
    {
	  xspan=tmptable->columns-tmptable->x;
     if(xspan<=0)
	   xspan=1;
	 }

	 newcell(tmptable,xspan,yspan,&HTMLatom.x,&HTMLatom.y,&width,perc,tdwidth[tabledepth]);
    if(thistableadr!=parenttableadr)
     swapmod=1;
    HTMLatom.x+=tblx+border;
	 HTMLatom.xx=HTMLatom.x+width;
    x=p->docLeftEdge=p->docLeft=HTMLatom.x+tmptable->cellpadding;
    p->docRightEdge=p->docRight=HTMLatom.xx-tmptable->cellpadding;
    y=HTMLatom.y+tmptable->cellpadding;
	 HTMLatom.yy=tmptable->tdend+tmptable->cellpadding;

	 tdheight=y+newtdheight;

    if(caption) //nadpis (tr.: headline)
	  border=0;

    if(p->docRight-p->docLeft<FUZZYPIX) //v uzkych sloupcich nedelat bordel!
    //tr.: don't mess up narrow tables
	  align=BOTTOM;

    if(img->URL[0])
     addatom(&HTMLatom,img,sizeof(struct picinfo),TD_BACKGROUND,valign,border,bgcolor,parenttableadr,1);
	 else
     addatom(&HTMLatom,"",0,TD,valign,border,bgcolor,parenttableadr,1);

	 currentcell[tabledepth]=p->lastHTMLatom;
    //!rowspan fix!
	 if(yspan>1)
    {
     if(thistableadr==parenttableadr)
      tmptable=thistable;
	  else
     {
      tmptable=(struct HTMLtable *)ie_getswap(parenttableadr);
      //printf("tables out of sync");
	  }
     if(tmptable)
	  {
	   if(tmptable->x-xspan+1<MAXROWSPANTD)
      {
	    tmptable->closerowspan[tmptable->x-xspan+1]=p->lastHTMLatom;
       if(thistableadr!=parenttableadr)
	swapmod=1;
      }
	  }
     else
      MALLOCERR();
    }
	 //!rowspan end!
   }
	else
	 MALLOCERR();
       }
       else
   MALLOCERR();
       fontstack.depth=fontstackdepth[tabledepth-1];
      } //endif uvnitr table (tr.: within table)

      if(sheet->usetdcolor)
      {
       HTMLatom.R=sheet->tdR;
       HTMLatom.G=sheet->tdG;
       HTMLatom.B=sheet->tdB;
      }
      else
      {
       HTMLatom.R=sheet->textR;
       HTMLatom.G=sheet->textG;
       HTMLatom.B=sheet->textB;
      }
      currentlink=IE_NULL;
      if(sheet->tdfontsize!=-1)
       font=sheet->tdfontsize;
      else
       font=sheet->basefontsize;
      p->sizeTextRow=p->sizeRow=0;
      p->xsum=0l;
      p->maxsum=0l;
     }
     istd=1;
     if(tag==TAG_TABLE)
      goto tag_table;
     break;

#else // ----------------------------nouzova interpretace tabulek:
//tr.: emergency interpretation of tables
     case TAG_TABLE:
#ifdef TABLES
     case TAG_SLASH_TABLE:
#endif
     case TAG_SLASH_CAPTION:
     case TAG_TR:

     style=htmldata->basefontstyle;
     currentlink=IE_NULL;
     HTMLatom.R=htmldata->textR;
     HTMLatom.G=htmldata->textG;
     HTMLatom.B=htmldata->textB;
     font=htmldata->basefontsize;
     goto br;

     case TAG_TD:
     case TAG_TH:

     style=htmldata->basefontstyle;
     currentlink=IE_NULL;
     HTMLatom.R=htmldata->textR;
     HTMLatom.G=htmldata->textG;
     HTMLatom.B=htmldata->textB;
     font=htmldata->basefontsize;
     break;
#endif

     case TAG_LI: // <LI> <DT>

     alignrow(x,y,orderedlist[listdepth]);
     if(x>p->docLeft)
      y+=p->sizeRow;
     if(orderedlist[listdepth]!=0 || p->docLeft+LISTINDENT>p->docRight)
      x=p->docLeft;
     else
      x=p->docLeft+LISTINDENT;
     p->sizeTextRow=p->sizeRow=fonty(font,style);

     if(orderedlist[listdepth]<1)
     {
      int type=0; //unordered list - currently decoration type= 1 or 2
      if(orderedlist[listdepth]<0)
       type=2+orderedlist[listdepth];

      HTMLatom.x=x-13+type;
      if(fixedfont)
       HTMLatom.y=y;
      else
       HTMLatom.y=y+p->sizeRow/2-5+type;
      HTMLatom.xx=x-5-type;
      HTMLatom.yy=y+p->sizeRow/2+5-type;
      addatom(&HTMLatom,"",0,LI,align,type,0,IE_NULL,0);
     }
     else
     {
      char number[12]; //ordered list - output item number instead of bullet
      sprintf(number,"%d.",orderedlist[listdepth]++);

      htmlfont(font,style);
      HTMLatom.x=x-strlen(number)*fontx(font,style,'0');
      HTMLatom.y=y;
      HTMLatom.xx=x;
      HTMLatom.yy=y+fonty(font,style);
      addatom(&HTMLatom,number,strlen(number),TEXT,BOTTOM,font,style,IE_NULL,0);
     }
     nownobr=1;
     lastspace=1;//mazat mezery (tr.: erase spaces)
     align=BOTTOM; //seznam zarovnavat doleva (tr.: list align left)
     if(p->xsum>p->maxsum)
      p->maxsum=p->xsum;
     p->xsum=0;
     break;

     case TAG_DD: //<DD>

     alignrow(x,y,orderedlist[listdepth]);
     y+=p->sizeRow;
     p->sizeTextRow=p->sizeRow=fonty(font,style);
     x=p->docLeft+LISTINDENT;
     lastspace=1;
     break;

     case TAG_OL: //<OL> <MENU> <DL> <DIR>

     orderedlist[listdepth+1]=1; //ordered list, item no.=1
     goto list;

     case TAG_BLOCKQUOTE: //<BLOCKQUTE>
     case TAG_UL: //<UL>

     if(orderedlist[listdepth]!=-2)
      orderedlist[listdepth+1]=-2; //unordered list type 1
     else
      orderedlist[listdepth+1]=-1; //unordered list type 2
     list:
     {
      char flag=(x>p->docLeft);
      int indent=LISTINDENT;

      if(orderedlist[listdepth+1]==1) //yes, it is ordered list
       indent*=2;

//      printf("[indent=%d]",indent);
      if(p->docLeft+indent<p->docRight && listdepth<2*MAXTABLEDEPTH-1)
      {
       listedge[listdepth]=p->docLeft;
       p->docLeft+=indent;
       listdepth++;
       x=p->docLeft;
//       printf("[listedge=%d,left=%d]",listedge[listdepth-1],p->docLeft);
      }

      if(tag==TAG_BLOCKQUOTE)
       y+=fonty(font,style)/2;

      if(flag)
       goto br;
     }
     break;

     case TAG_SLASH_UL:
     case TAG_SLASH_BLOCKQUOTE:
     case TAG_SLASH_OL:

     if(listdepth && orderedlist[listdepth])
     {
      listdepth--;
      p->docLeft=listedge[listdepth];
     }
     goto p;

     case TAG_INPUT: //<INPUT>
     case TAG_BUTTON: //<BUTTON>

     {
      int type=TEXT,size=10,checked=0;
      char value[IE_MAXLEN+1]="\0",name[80]="\0";
      char notresize=0;

      //official extensions

      if(tag==TAG_BUTTON)
       type=SUBMIT;

      if(getvar("URI",&tagarg))
      {
       AnalyseURL(cache->URL,&url,p->currentframe); //(plne zneni...)
       //tr.: entire text
       strcpy(value,url.file);
      }

      if(getvar("USR",&tagarg))
      {
       AnalyseURL(cache->URL,&url,p->currentframe); //(plne zneni...)
       //tr.: entire text
       strcpy(value,url.user);
      }

      if(getvar("URL",&tagarg))
       strcpy(value,cache->URL);


//!!glennmcc: Mar 27, 2007 -- cut mailto URLs at '?'
//and grab that subject from mailto addresses
//mailto:someone@somewhere.net?subject=something
//new lines needed in sendmail.ah for this feature
//<TD ALIGN=RIGHT><B><FONT 3D>To:&nbsp;
//<TD COLSPAN=2><INPUT TYPE=TEXT SIZE=61 NAME="$TO" TO active>
//__________________________________________________^^ 'URI' changed to 'TO'
//<TD ALIGN=RIGHT><B><FONT 3D>Subject:
//<TD COLSPAN=2><INPUT TYPE=TEXT SIZE=61 NAME="$SUBJ" SUBJECT>
//____________________________________________________^^^^^^^ 'SUBJECT' added
#ifndef LINUX
      if(getvar("TO",&tagarg))
      {
       AnalyseURL(cache->URL,&url,p->currentframe); //(plne zneni...)
//       strlwr(url.file);
       if(strstr(url.file,"?"/*subject=*/))
       makestr(value,url.file,strcspn(url.file,"?"));
       else
       strcpy(value,url.file);
      }

      if(getvar("SUBJECT",&tagarg))
      {
       AnalyseURL(cache->URL,&url,p->currentframe); //(plne zneni...)
//       strlwr(url.file);
       if(strstr(url.file,"?"/*subject=*/))
       {
       strrev(url.file);
       makestr(value,url.file,strcspn(url.file,"="));
       strrev(value);
       }
       else value[0]='\0';
      }
#endif
//!!glennmcc: end

      //end of official extensions

      if(getvar("VALUE",&tagarg))
      {
       strcpy(text,tagarg);
       entity2str(text);
       makestr(value,text,IE_MAXLEN);
      }

      if(getvar("SIZE",&tagarg))
      {
       size=atoi(tagarg);
      }

      if(getvar("TYPE",&tagarg))
      {
       if(!strcmpi(tagarg,"TEXT"))
   type=TEXT;
       else
       if(!strcmpi(tagarg,"PASSWORD"))
   type=PASSWORD;
       else
       if(!strcmpi(tagarg,"SUBMIT"))    //tlacitko (tr.: button)
       {
   type=SUBMIT;
   butt:
   if(!value[0])
    strcpy(value,tagarg);
   size=strlen(value);
       }
       else
       if(!strcmpi(tagarg,"RESET"))
       {
   type=RESET;
   goto butt;
       }
       else
     if(!strcmpi(tagarg,"BUTTON"))
       {
//!!glennmcc: Feb 26, 2007 -- now duplicates type=submit in our compiles
   break; //not yet implemented ! //won't break in our compiles
//!!glennmcc: end
       }
       else
       if(!strcmpi(tagarg,"OUTPUT"))
   type=OUTPUT;
       else
       if(!strcmpi(tagarg,"HIDDEN"))
   type=HIDDEN;
       else
       if(!strcmpi(tagarg,"RADIO"))
       {
   type=RADIO;
   size=1;
       }
       else
       if(!strcmpi(tagarg,"CHECKBOX"))
       {
   type=CHECKBOX;
   size=1;
   if(!value[0])
    strcpy(value,"on");
       }
       else
       if(!strcmpi(tagarg,"IMAGE"))
       {
   input_image=1;
   goto process_input_image;
       }
      }

//!!glennmcc: Feb 14, 2006 -- optionally ignore 'active'
      if(getvar("CHECKED",&tagarg) || (getvar("ACTIVE",&tagarg) &&
         config_get_bool("IgnoreActive", 0)))
      checked=1;
//!!glennmcc: end

      if(getvar("NAME",&tagarg))
       makestr(name,tagarg,79);

//!!glennmcc: Apr 03, 2007 -- make this frame 'active'
//also requires <arachne nocache>
/*
<arachne nocache>
<frameset cols="100%">
<frameset rows="10,*,96">
 <frame src="file://rule800.htm" scrolling=NO border=0>
 <frame src="file://*.*" NAME="activeframe" scrolling=NO border=0 NORESIZE>
 <frame src="file://control.htm" NAME="Control" scrolling=NO>
</frameset>
*/
if(!strcmpi(name,"activeframe") && arachne.framescount>0)
   p->activeframe=p->currentframe;
//!!glennmcc: end

      //unsecure arachne extensions to <INPUT> tag.....................
      //allowed only for local or forced-html documents
      if(searchvar("ARACHNE") &&
    (!strncmpi(cache->URL,"file",4) || !strncmpi(cache->URL,"mailto",4)
     || !strncmpi(cache->URL,"about",4) || !strncmpi(cache->URL,"gui",3)
     || p->forced_html))
       CheckArachneFormExtensions(cache,value, &checked);

      if(type==SUBMIT || type==RESET /*|| type==BUTTON*/)
      {
       int i=0,l=strlen(value),spccount=0,spc;
       int maxsize;

       size=fontx(BUTTONFONT,0,'a');

       while(i<l)
       {
   if(value[i]==' ' && spccount>2)
   {
    spc=1;
    spccount++;
   }
   else
    spc=0;

   if(spc)
    size+=space(BUTTONFONT);
   else
    size+=fontx(BUTTONFONT,0,'a');
   i++;
       }

       htmlfont(BUTTONFONT,0);
       maxsize=x_txwidth(value)+2*space(0);
       if(maxsize>size)
   size=maxsize;
      }
      else if(type==OUTPUT)
      {
       size=fontx(BUTTONFONT,0,'a')*(size+1);
       type=BUTTON;
      }
      else
       size=size*space(SYSFONT)+4;

      if(type!=HIDDEN)
      {
       if(x+size>p->docRight && x>p->docLeft && size<p->docRight-p->docLeft && !pre && !nownobr)
       {
   alignrow(x,y,orderedlist[listdepth]);
   y+=p->sizeRow;
   x=p->docLeft;
       }

       HTMLatom.x=x;
       if(tag!=TAG_BUTTON)
   x+=size;
       else
       {
   checked=2; //to indicate that it's button, not input ! No text shown

   //checked & 1 ...checked/pressed
   //checked & 2 ...it is BUTTON

   currentbuttonx=x;
   nobr=1;
   nownobr=0;
       }
       HTMLatom.xx=x;
       HTMLatom.y=y;

       if(type==CHECKBOX)
   HTMLatom.yy=y+11+space(SYSFONT);
       else if(type==RADIO)
   HTMLatom.yy=y+10+space(SYSFONT);
       else if(type==SUBMIT || type==RESET || type==BUTTON)
   HTMLatom.yy=y+4+fonty(BUTTONFONT,0);
       else if(tag==TAG_BUTTON)
   HTMLatom.yy=y;
       else
   HTMLatom.yy=y+4+fonty(SYSFONT,0);

       if(tag!=TAG_BUTTON)
       {
   int ygap=(int)(HTMLatom.yy-HTMLatom.y)+2;
   if(p->sizeRow<ygap)
    p->sizeRow=ygap;
   if(p->sizeTextRow<ygap)
    p->sizeTextRow=ygap;
       }
      }
      else
      {
       //neviditelna polozka (tr.: invisible element)
       HTMLatom.x=-256;
       HTMLatom.y=y;
       HTMLatom.xx=-256;
       HTMLatom.yy=y;
       notresize=1;
      }

      //add atom only if object was succesfuly created:
      if(InitInput(&tmpeditor,name,value,1,CONTEXT_HTML)==1)
       addatom(&HTMLatom,&tmpeditor,sizeof(struct ib_editor),INPUT,align,type,checked,currentform,notresize);

      if(tag==TAG_BUTTON)
      {
       int ygap=fonty(font,style);
       if(p->sizeRow<ygap)
   p->sizeRow=ygap;
       if(p->sizeTextRow<ygap)
   p->sizeTextRow=ygap;
       currentbutton=p->lastHTMLatom;
       currentbuttony=y;
       xshift(&x,space(0));
       p->docLeft=x;
       y+=2;
       break;
      }
      else
       currentbutton=IE_NULL;

      if(notresize)
       break;
     }//end block!

     {
      int oldright=p->docRight;
      xshift(&x,space(0));
      if(x>oldright && !pre && !nownobr && !nobr)
       goto br;
     }
     break;

     case TAG_SLASH_BUTTON: //</BUTTON>
     if(currentbutton!=IE_NULL)
     {
      long oldxsum;

      xshift(&x,space(0));
      oldxsum=p->xsum;
      closeatom(currentbutton,x-currentbuttonx,y+p->sizeRow+5); //0=don't overwrite Y coordinate
      p->xsum=oldxsum;
      xshift(&x,space(0));
      if(currentbuttony!=y)
      {
       unsigned pushlast=p->lastHTMLatom;
       p->lastHTMLatom=currentbutton;
       alignrow(x,currentbuttony,orderedlist[listdepth]);
       p->lastHTMLatom=pushlast;
      }
      alignrow(x,y,orderedlist[listdepth]);
      p->sizeRow+=5;
      currentbutton=IE_NULL;
      p->docLeft-=space(0);
      if(x>p->docRight && !pre && !nownobr && !nobr)
       goto br;
     }
     break;

     case TAG_FORM: //<FORM>
     {
      char target=findtarget(basetarget);
      char method=0;

      HTMLatom.x=x;
      HTMLatom.y=y;
      HTMLatom.xx=x;
      HTMLatom.yy=y;
      if(getvar("METHOD",&tagarg))
      {
       if(!strcmpi(tagarg,"POST"))
   method=1;
       else
       if(!strcmpi(tagarg,"HREF"))
   method=-1;
      }
      if(getvar("ACTION",&tagarg))
      {
       //vlozit link:
       //tr.: insert link
       if(tagarg[0]=='#' && method==-1)
   makestr(text,tagarg,URLSIZE);
       else
ReAnalyse:
       {
   AnalyseURL(tagarg,&url,p->currentframe); //(plne zneni...)
   //tr.: entire text
   url2str(&url,text);
//!!glennmcc: July 26, 2008 -- fix problems with '&amp;' in <form action="
   if(strstr(text,"&amp;"))
   {
    entity2str(text);
    tagarg=text;
   }
//!!glennmcc: end
       }
       //vyrobim si pointr na link, a od ted je vsechno link:
       //tr.: I create a pointer for link, and from now on
       //     everything will be a link
//!!glennmcc: Jan 11, 2006 -- 'backup' one step if '?' and 'action=""'
  if(strcmpi(url.protocol,"file:") && strstr(text,"?") && strlen(tagarg)<1)
    {strcat(tagarg,"../"); goto ReAnalyse;}
//!!glennmcc: end

//!!glennmcc: Feb 27, 2008 -- if form action is speced as current page...
//'null it out' and use existing code which uses current page
//when action is blank or missing.
/*
  if(strcmpi(url.protocol,"file:") && strstr(url.file,text)
     && strlen(tagarg)>10)
    {tagarg[0]='\0'; goto ReAnalyse;}
*/
//!!glennmcc: end

       addatom(&HTMLatom,text,strlen(text),FORM,align,target,method,IE_NULL,1);
       currentform=p->lastHTMLatom;
      }

//!!glennmcc: begin: Aug 12, 2002 - use current URL if 'action' is missing from form
       else
       {
       *tagarg=p->currentframe;
   AnalyseURL(tagarg,&url,p->currentframe); //(plne zneni...)
   //tr.: entire text
   url2str(&url,text);
       //vyrobim si pointr na link, a od ted je vsechno link:
       //tr.: I create a pointer for link, and from now on
       //     everything will be a link
//!!glennmcc: Jan 11, 2006 -- 'backup' one step if '?' and 'action' is missing
  if(strcmpi(url.protocol,"file:") && strstr(text,"?"))
    {strcat(tagarg,"../"); goto ReAnalyse;}
//!!glennmcc: end
       addatom(&HTMLatom,text,strlen(text),FORM,align,target,method,IE_NULL,1);
       currentform=p->lastHTMLatom;
       }
//!!glennmcc: end

     }
     if(!nownobr)
      goto p;
     break;

     case TAG_SLASH_FORM:
     currentform=IE_NULL;
     if(!nownobr)
      goto p;
     break;

     case TAG_NOFRAMES://<NOFRAMES>
     if(!alreadyframe && user_interface.frames)
     {
      insidetag=TAG_SCRIPT;
      invisibletag=1;
     }
     break;

     case TAG_SCRIPT://<SCRIPT>

//!!glennmcc: begin May 03, 2002
// added to optionally "ignore" <script> tag
// (defaults to No if "IgnoreJS Yes" line is not in Arachne.cfg)
   if(user_interface.ignorejs){insidetag=0;}else
//!!glennmcc: end
      insidetag=tag;
//   case TAG_HEAD: //<HEAD>
     invisibletag=1;
     break;

     case TAG_TITLE: //<TITLE>
     invisibletag=1;
     insidetag=tag;
     break;

     case TAG_SLASH_SCRIPT:
     case TAG_SLASH_NOSCRIPT:
     case TAG_SLASH_NOFRAMES:
     insidetag=0;
//     case TAG_SLASH_HEAD:
     invisibletag=0;
     break;

     case TAG_SLASH_TITLE:
     //accept title only for the main frame...
     if(insidetag==TAG_TITLE && !arachne.title[0] && !p->currentframe)
     {
      text[txtlen]='\0';
      MakeTitle(text);
      if(!p->rendering_target)
       DrawTitle(0);
     }
     invisibletag=0;
     insidetag=0;
     break;

     case TAG_SELECT: //SELECT
     {
      char name[80]="\0";
      int size=1,rowspix;

      if(getvar("SIZE",&tagarg))
      {
       size=atoi(tagarg);
       if(size<1)size=1;
      }

      if(getvar("NAME",&tagarg))
       makestr(name,tagarg,79);

      if(getvar("MULTIPLE",&tagarg))
       multiple=1;
      else
       multiple=0;

      if(InitInput(&tmpeditor,name,NULL,1000,CONTEXT_HTML)==1)
      {
       HTMLatom.x=x;
       HTMLatom.y=y;
       rowspix=4+fonty(BUTTONFONT,0)*size+fonty(OPTIONFONT,0)*(size-1);
       HTMLatom.xx=x;
       HTMLatom.yy=y+rowspix;
       if(p->sizeRow<rowspix+2)p->sizeRow=rowspix+2;
       if(p->sizeTextRow<rowspix+2)p->sizeTextRow=rowspix+2;
       maxoption=0;
       addatom(&HTMLatom,&tmpeditor,sizeof(struct ib_editor),INPUT,align,SELECT,multiple,currentform,0);
       atomptr=(struct HTMLrecord *)ie_getswap(p->lastHTMLatom);
       currenttextarea=atomptr->ptr;
      }
      insidetag=tag; //flag: read contens of <SELECT>... </SELECT>!
      invisibletag=1;
      alreadyselected=0;
     }
     break;

     case TAG_SLASH_OPTION: //</OPTION>
     case TAG_SLASH_SELECT: //</SELECT>
     case TAG_OPTION:       //<OPTION>

     text[txtlen]='\0';
     if(insidetag==TAG_OPTION)         // <(last)OPTION> text <(this)OPTION|/SELECT>
     {
      int txtx;

      // "<OPTION>blabla\n" hack
      if(txtlen>0 && text[txtlen-1]==' ')
       text[txtlen-1]='\0';

      appendline(currenttextarea,text,0);
      htmlfont(OPTIONFONT,0);
      txtx=x_txwidth(text);
      if(txtx>maxoption)maxoption=txtx;
     }

     insidetag=tag; //flag: inside <OPTION> ?

     if(tag==TAG_SLASH_SELECT) //</SELECT> - end of select tag
     {
      int dx=maxoption+space(SYSFONT)+user_interface.scrollbarsize+11;
      int oldright=p->docRight;
      insidetag=0;
      invisibletag=0;
      if(x+dx>p->docRight)
      {
       alignrow(x,y,orderedlist[listdepth]);
       y+=p->sizeRow;
       x=p->docLeft;
       lastspace=1;//mazat mezery (tr.: erase spaces)
       p->sizeTextRow=p->sizeRow=atom2nextline(x,y,p->lastHTMLatom);
      }
      closeatom(p->lastHTMLatom,dx,0); //0=don't overwrite Y coordinate
      xshift(&x,dx);
      if(!alreadyselected && !multiple)
      {
       editorptr=(struct ib_editor *)ie_getswap(currenttextarea);
       if(editorptr)
       {
        char *ptr;
        memcpy(&tmpeditor,editorptr,sizeof(struct ib_editor));
   ptr=ie_getline(&tmpeditor,0);
	if(ptr)
	{
    ptr[0]='1';
	 swapmod=1;
	}
       }
      }

      xshift(&x,space(0));
      if(x>=oldright && !pre && !nownobr && !nobr)
       goto br;

     }
     else
     if(tag==TAG_SLASH_OPTION) //</OPTION> - end of option tag
      insidetag=TAG_SELECT;
     else
     if(tag==TAG_OPTION)       //<OPTION>
     {
      if(getvar("SELECTED",&tagarg) && !(alreadyselected && !multiple))
      {
       alreadyselected=1;
       text[0]='1';
      }
      else
       text[0]='0';

      if(!getvar("VALUE",&tagarg))
       text[1]='\0';
      else
       strcpy(&text[1],tagarg);

//!!glennmcc: Apr 02, 2010
//to prevent problems due to <option value="">blabla</option>
      if(getvar("VALUE",&tagarg) && strlen(tagarg)==0)
       strcpy(&text[1]," ");
//!!glennmcc: end

      appendline(currenttextarea,text,(text[0]=='1'));
     }//endif
     break;

     case TAG_TEXTAREA: //<TEXTAREA>

     {
     char name[80]="\0",active=0;
     int rows=5,cols=20;
     int rowspix,rv;

     if(getvar("ROWS",&tagarg))
     {
      rows=atoi(tagarg);
//!!glennmcc: Sep 10, 2008 -- min of 2 to fix 'invisible textarea'
      if(rows<2)rows=2;
     }

     if(getvar("ARACHNEROWS",&tagarg))
     {
      //recalculate for current system font:
      rows=14*rows/fonty(SYSFONT,0);
      rows-=(540-frame->scroll.ysize+fonty(SYSFONT,0)/2)/fonty(SYSFONT,0);
//!!glennmcc: Sep 10, 2008 -- min of 2 to fix 'invisible textarea' at 640x480
      if(rows<2)rows=2;
     }

     if(rows<1)rows=1;

     if(getvar("COLS",&tagarg))
     {
      cols=atoi(tagarg);
//!!glennmcc: Nov 23, 2008 -- fix width problem with textarea in nested table
      if(user_interface.scrollbarsize)
      cols-=2;
//!!glennmcc: end
      if(cols<2)cols=2;
     }

     if(getvar("NAME",&tagarg))
      makestr(name,tagarg,79);

     HTMLatom.x=x;
     HTMLatom.y=y;
     x+=user_interface.scrollbarsize+5+space(SYSFONT)*cols;
     rowspix=user_interface.scrollbarsize+5+fonty(SYSFONT,0)*rows;
     HTMLatom.xx=x;
     HTMLatom.yy=y+rowspix;
     if(p->sizeRow<rowspix+2)p->sizeRow=rowspix+2;
     if(x>p->docRight)p->docRight=x;
     if(p->docRight>p->docRightEdge)p->docRightEdge=p->docRight;

     tmpeditor.cols=cols;
     if(getvar("ARACHNEEDITOR",&tagarg))
     {
      if(tagarg[0])
       strcpy(tmpeditor.filename,tagarg);
      else
       strcpy(tmpeditor.filename,LASTlocname);
      rv=ie_openf_lim(&tmpeditor,CONTEXT_HTML,8000);
      if(name[0])
       strcpy(tmpeditor.filename,name);
     }
     else
     {
      rv=InitInput(&tmpeditor,name,NULL,8000,CONTEXT_HTML);
      insidetag=tag; //flag: read contens of <TEXTAREA>... </TEXTAREA>!
     }

//!!glennmcc: Apr 07, 2008 -- default to wrap
     if(getvar("WRAP",&tagarg) && toupper(tagarg[0])=='N')
     tmpeditor.wordwrap=0;
//     if(getvar("WRAP",&tagarg) && toupper(tagarg[0])!='N')
//      tmpeditor.wordwrap=1;
//original 2 lines above this comment
//!!glennmcc: end

//!!glennmcc: Feb 14, 2006 -- optionally ignore 'active'
     if(getvar("ACTIVE",&tagarg) && config_get_bool("IgnoreActive", 0))
      active=1;
//!!glennmcc: end

     if(rv==1)
     {
      addatom(&HTMLatom,&tmpeditor,sizeof(struct ib_editor),INPUT,align,TEXTAREA,active,currentform,0);
      atomptr=(struct HTMLrecord *)ie_getswap(p->lastHTMLatom);
      currenttextarea=atomptr->ptr;
     }
     invisibletag=1;
     }
     break;

     case TAG_SLASH_TEXTAREA:
     case TAG_SLASH_STYLE:

     invisibletag=0;
     text[txtlen]='\0';
     if(insidetag==tag-TAG_SLASH)
     {
      appendline(currenttextarea,text,0);
      if(insidetag==TAG_STYLE && user_interface.css)
       stylesheetadr=currenttextarea;
     }
     insidetag=0;
     break;

     case TAG_STYLE:

     if(InitInput(&tmpeditor,"",NULL,1000,CONTEXT_HTML)==1)
     {
      if(stylesheetadr==IE_NULL)
       currenttextarea=ie_putswap((char *)&tmpeditor,sizeof(struct ib_editor),CONTEXT_HTML);
      else
       currenttextarea=stylesheetadr;
      insidetag=tag; //flag: read contens of <TEXTAREA>... </TEXTAREA>!
      invisibletag=1;
     }

     break;

     case TAG_BODY: //<BODY>

//!!glennmcc: Oct 29, 2005 - optionally do not display bgimages
     if((!strncmpi(cache->URL,"file:",5) || config_get_bool("BGimages", 1)) &&
     getvar("BACKGROUND",&tagarg) && tagarg[0])
//     if(getvar("BACKGROUND",&tagarg) && tagarg[0])//original single line
//!!glennmcc: end
     {
      ResetHtmlPage(htmldata,TEXT_HTML,0);
      AnalyseURL(tagarg,&url,p->currentframe); //(plne zneni...)
      //tr.: entire text
      url2str(&url,img->URL);
      init_picinfo(img);
      img->URL[URLSIZE-1]='\0';
      if(img->URL[0])
      {
       addatom(&HTMLatom,img,sizeof(struct picinfo),BACKGROUND,BOTTOM,0,0,IE_NULL,0);
       htmldata->backgroundptr=p->lastHTMLatom;
      }
      //printf("background image=%s\n",img->URL);
     }
//!!glennmcc: June 27, 2007 -- only do this for remote pages... not local
//!!glennmcc: July 14, 2005 -- use a new config setting to override instead
//"AlwaysUseCFGcolors Yes" will override 'bgcolor' , 'text' and 'link'
//!!glennmcc: begin Jan 3, 2003
//use arachne.cfg values if either bgcolor= or text= is missing from <body>
//if(getvar("BGCOLOR",&tagarg) && getvar("TEXT",&tagarg))
//{//(closing '}' below)//no longer needed... commented-out

// if(getvar("BGCOLOR",&tagarg) && !user_interface.alwaysusecfgcolors)
   if(getvar("BGCOLOR",&tagarg) && (!user_interface.alwaysusecfgcolors || !strcmpi(url.protocol,"file")))
     {
      try2readHTMLcolor(tagarg,&htmldata->backR,&htmldata->backG,&htmldata->backB);
     }

// if(getvar("TEXT",&tagarg) && !user_interface.alwaysusecfgcolors)
   if(getvar("TEXT",&tagarg) && (!user_interface.alwaysusecfgcolors || !strcmpi(url.protocol,"file")))
     {
      try2readHTMLcolor(tagarg,&htmldata->textR,&htmldata->textG,&htmldata->textB);
     }

//!!glennmcc: Jan 3, 2003
//always use CFG color for text when it is missing from <body>
//no matter what bgcolor might be
/*
     else if (htmldata->backR<8 && htmldata->backG<8 && htmldata->backB<8 &&
	      htmldata->backgroundptr==IE_NULL )
     {
      htmldata->textR=255;
      htmldata->textG=255;
      htmldata->textB=255;
     }
*/
//}//!!glennmcc: end -- Jan 3, 2003

// if(getvar("LINK",&tagarg) && !user_interface.alwaysusecfgcolors)
   if(getvar("LINK",&tagarg) && (!user_interface.alwaysusecfgcolors || !strcmpi(url.protocol,"file")))
     {
      try2readHTMLcolor(tagarg,&htmldata->linkR,&htmldata->linkG,&htmldata->linkB);
     }
#ifdef VLINK
     if(getvar("VLINK",&tagarg))
     {
      try2readHTMLcolor(tagarg,&htmldata->vlinkR,&htmldata->vlinkG,&htmldata->vlinkB);
     }
#endif

     if(getvar("MARGINWIDTH",&tagarg))
     {
      p->docLeft=p->docLeftEdge=stackLeft[0]=stackLeftEdge[0]=x=frame->marginwidth=atoi(tagarg);
      p->docRight=p->docRightEdge=stackRight[0]=stackRightEdge[0]=frame->scroll.xsize-frame->marginwidth-FUZZYPIX;
     }
     else
      x=p->docLeft;

     if(getvar("MARGINHEIGHT",&tagarg))
     {
      if(y==frame->marginheight)
       y=atoi(tagarg);
      frame->marginheight=atoi(tagarg);
     }

     if(getvar("ARACHNE",&tagarg))
      BodyArachne(htmldata);

     if(getvar("MAIL",&tagarg))
     {
      //force JPEG/PNG conversion...
      GLOBAL.nowimages=IMAGES_NOTNOW;

      if(user_interface.mailisdesktop)
       BodyArachne(htmldata);
      else
       ResetHtmlPage(htmldata,TEXT_HTML,1);
     }

     if(getvar("NORESIZE",&tagarg))
      noresize=1;

     if(getvar("BGPROPERTIES",&tagarg) && toupper(tagarg[0])=='F') //fixed
      htmldata->bgproperties=BGPROPERTIES_FIXED;

     invisibletag=0;
     if(stylesheetadr!=IE_NULL)
      ParseCSS(htmldata,stylesheetadr,"");
     style=htmldata->basefontstyle;
     font=htmldata->basefontsize;
     HTMLatom.R=htmldata->textR;
     HTMLatom.G=htmldata->textG;
     HTMLatom.B=htmldata->textB;

     break;

     case TAG_BASE: //<BASE>
//!!glennmcc: begin July 14, 2003
// added to optionally "ignore" <base href="> tag
// (defaults to No if "IgnoreBaseHref Yes" line is not in Arachne.cfg)
if(!http_parameters.ignorebasehref || !strncmpi(cache->URL,"file",4)){
//!!glennmcc: added July 15, 2003  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//do not ignore if URL is 'local'
//!!glennmcc: end

     if(getvar("HREF",&tagarg))
     {
      add2history(frame->cacheitem.URL);
      GLOBAL.nothot=1;

//!!glennmcc: Mar 03, 2006 -- correct problem with <base href="">
//which results in URL of 'http:///'
if(strlen(tagarg)<1) strcpy(text,frame->cacheitem.URL); else
//!!glennmcc: end
      strcpy(text,tagarg);
      entity2str(text);
      AnalyseURL(text,&url,IGNORE_PARENT_FRAME);

      //reset BASE url:
      if(!p->currentframe)
       memcpy(&baseURL,&url,sizeof(struct Url));

      url2str(&url,frame->cacheitem.URL);
      if(!p->currentframe)
      {
       SetInputAtom(&URLprompt,frame->cacheitem.URL);
       if(!p->rendering_target)
DrawTitle(0);
      }
     }
}//!!glennmcc: ending '}' of "IgnoreBaseHref"

     basetarget=findtarget(p->currentframe);

     break;

     case TAG_CODE: //<CODE> <KBD>

     style=style | FIXED | BOLD;
     fixrowsize(font,style);
     break;

     case TAG_SLASH_CODE:

     if(style & BOLD)
      style -= BOLD;
     if(style & FIXED)
      style -= FIXED;
     fixrowsize(font,style);
     break;

     case TAG_LINK: //<LINK REL=...>

     LINKtag(&stylesheetadr);
     break;

     case TAG_FRAMESET: //<FRAMESET>

     if(user_interface.frames && arachne.framescount<MAXFRAMES-1) //[0...6]
     {
      char framewantborder=UNDEFINED_FRAMEBORDER;
      p->oldactive=0; //deactivate any possibly active frames
      arachne.backtrace=0; //frameset changed -> old targets are not valid!

      if(getvar("FRAMEBORDER",&tagarg) || getvar("BORDER",&tagarg))
      {
       if(tagarg[0]=='0' || toupper(tagarg[0])=='N' || toupper(tagarg[0])=='F')
   framewantborder=DONT_WANT_FRAMEBORDER;
       else
   framewantborder=I_WANT_FRAMEBORDER;
      }

      if(getvar("ROWS",&tagarg) && strchr(tagarg,','))
      {
       makestr(text,tagarg,STRINGSIZE);
//!!glennmcc: Nov 28, 2007 -- prevent problems caused by rows="100%,*"
//!!glennmcc: Sep 16, 2008 -- this works even better ;-)
       if(strstr(text,"100%,*")) strcpy(text,"99%,*");
//     if(!strstr(text,"100%"))
//!!glennmcc: end
       addframeset(0,&emptyframeset,framewantborder,text);
      }
      else
      if(getvar("COLS",&tagarg))
      {
       makestr(text,tagarg,STRINGSIZE);
//!!glennmcc: Nov 28, 2007 -- prevent problems caused by cols="100%,*"
//!!glennmcc: Sep 16, 2008 -- this works even better ;-)
       if(strstr(text,"100%,*")) strcpy(text,"99%,*");
//     if(!strstr(text,"100%"))
//!!glennmcc: end
       addframeset(1,&emptyframeset,framewantborder,text);
      }
     }

     if(!alreadyframe && emptyframeset==-1)
     {
      char *msg=MSG_FRAMES;
      fixrowsize(font,style);
      alignrow(x,y,orderedlist[listdepth]);
      HTMLatom.x=x;
      HTMLatom.y=y;
      HTMLatom.xx=p->docRight;
      y+=fonty(6,BOLD);
      HTMLatom.yy=y;
      addatom(&HTMLatom,msg,strlen(msg),TEXT,BOTTOM,6,BOLD,IE_NULL,0);
      alreadyframe=1;
      goto br;
     }

     break;

     case TAG_FRAME: //<FRAME>
//!!glennmcc: Mar 12, 2007 -- quick-n-dirty hack to support <iframe
//test page .... http://www.auschess.org.au/
     case TAG_IFRAME: //<IFRAME>
     if(user_interface.frames && emptyframeset!=-1)
      FRAMEtag(&emptyframeset,&previousframe);
     else
     {
     if(tag==TAG_IFRAME)//<BR> 4 times normal height to move the Ikon down
     //and out of the way of other items already on the page
     {
      p->sizeTextRow=p->sizeRow=fonty(font,style)*4; //?p->sizeRow?
      x=p->docLeft;
      lastspace=1;//mazat mezery (tr. spaces)
      HTMLatom.x=x;
      HTMLatom.y=y;
     }
      DummyFrame(p,&x,&y,tag);//added 'tag' to the function
     if(tag==TAG_IFRAME)//<BR> 4 times normal height to move the rest
     //of the items on the page out of the way of the Ikon
     {
      p->sizeTextRow=p->sizeRow=fonty(font,style)*4; //?p->sizeRow?
      x=p->docLeft;
      lastspace=1;//mazat mezery (tr. spaces)
      HTMLatom.x=x;
      HTMLatom.y=y;
     }
//    DummyFrame(p,&x,&y);//original line
//!!glennmcc: end
      goto br;
     }
     break;//!!ray: May 15, 2006

     case TAG_META:
     METAtag();
     break;

     // * * * * * * * * * * implementation of CSIM - client side imagemaps

     case TAG_MAP:
     HTMLatom.x=x;
     HTMLatom.y=y;
     HTMLatom.xx=x;
     HTMLatom.yy=y+p->sizeRow;

     if(!getvar("NAME",&tagarg))
      tagarg[0]='\0';

     addatom(&HTMLatom,tagarg,strlen(tagarg),MAP,align,0,0,IE_NULL,1);
     break;

     case TAG_AREA:
     USEMAParea(&HTMLatom,basetarget);
     break;

     // * * * * * * * * * * * * * * * * * * * * * * * * * * * * end of CSIM

     case TAG_EMBED:
//!!glennmcc: Jan 19, 2003 added support for 'BGSOUND'
     case TAG_BGSOUND:
//!!glennmcc: Aug 05, 2011 --- added support for 'AUDIO' & 'VIDEO'
     case TAG_AUDIO:
     case TAG_VIDEO:

     if(getvar("SRC",&tagarg) || getvar("FILENAME",&tagarg))// && !strncmpi(cache->URL,"file",4))
//!!glennmcc: Oct 30, 2002... re-enabled embed for remote pages
//by commenting out " && !strncmpi(cache->URL,"file",4))" on line above
//Nov 12, 2002 ... now also will recognize <embed filename=".....>
     {
      AnalyseURL(tagarg,&url,p->currentframe); //(plne zneni...)
      url2str(&url,img->URL);
      img->URL[URLSIZE-1]='\0';
      if(img->URL[0])
       addatom(&HTMLatom,img,sizeof(struct picinfo),EMBED,BOTTOM,0,0,IE_NULL,0);
     }
     break;

     case TAG_ARACHNE_BONUS:

     if(getvar("MSG",&tagarg))
      makestr(htmlmsg,tagarg,99);

     if(getvar("RAW",&tagarg))
     {
      makestr(cache->rawname,tagarg,79);
      frame->status=MAIL;
     }

     if(getvar("NOCACHE",&tagarg))
     {
      GLOBAL.del=2;
//!!glennmcc: Feb 14, 2008 -- removed last year's fix
//see new code in urlovrl.c
//    HTTPcache.len--; //!!glennmcc: Feb 24, 2007 -- decrement cache item count
//!!glennmcc: end
     }

     if(getvar("TARGET",&tagarg))
      config_set_str("FTPpath", tagarg);

     if(getvar("PRINT",&tagarg) && p->rendering_target)
     {
      if(toupper(*tagarg)=='N')
      {
       insidetag=TAG_SCRIPT;
       invisibletag=1;
      }
      else
      {
       insidetag=0;
       invisibletag=0;
      }
     }

    }//end switch tag analysing

    lasttag=tag;
    tag=0; //current tag=0
    HTMLatom.x=x;
    HTMLatom.y=y;
    if(!comment || !insidetag ||
       insidetag!=TAG_STYLE && insidetag!=TAG_TEXTAREA && insidetag!=TAG_SCRIPT)
     txtlen=0;
    comment=0;
    nolt=0;
    lastspcpos=0;
    lastspcx=x;
   }
   // <------------------------<---------<------zpracovani parametru HTML tagu
   //tr.: processing parameters of HTML tags
   else if(!comment && in>=' ')
   {
    //hehehe.... jestli nasledujicim radku nerozumite, nejste sami :-))))
    //autor programu pro vas ma plne pochopeni...

    //tr.: if you do not understand the following row/s, you are not alone,
    //     be assured of the author's full empathy
    if(!param && in!=' ' && taglen<sizeof(tagname))
    {
     tagname[taglen++]=in;
     if(taglen==3 && !strncmp(tagname,"!--",3))//||(tag=script)
//        insidetag!=TAG_TEXTAREA && insidetag!=TAG_STYLE && insidetag!=TAG_SCRIPT)
     {
      comment=1;
      nolt=1;
     }
    }
    else if(in==' ' && vallen && !uvozovky && argument)
    {
     if(param)
     {
      putvarvalue(tagargptr,vallen);
      argument=0;
     }
     vallen=0;
    }
    else if((in=='\"' || in=='\'' && (!uvozovky || apostrof)) && (!vallen || uvozovky))
    {
   if(argument && uvozovky)      //kvuli ' XXX="" ' (tr.: because of)
      tagargptr[vallen++]='\0';

     uvozovky=1-uvozovky;
     if(uvozovky && in=='\'')
      apostrof=1;
     else
      apostrof=0;
    }
    else if(in=='=' && !uvozovky && !argument)
    {
     putvarname(tagargptr,vallen);
     vallen=0;
     argument=1;
    }
    else if(vallen<BUF/2)
    {
     if(in!=' ' || uvozovky)
     {
      if(!argument && argspc && vallen)
      {
       putvarname(tagargptr,vallen);
       putvarvalue(tagargptr,0);
       vallen=0;
      }
      tagargptr[vallen++]=in;
      argspc=0;
     }
     else
     {
      if(param)
       argspc=1;
      else
       param=1;
     }
    }
   }
   else //ukonceni HTML komentare ? zjistuju '-->'
   //tr.: end of HTML comment, ? I find '-->'
   {
    pom[1]=pom[0];
    pom[0]=in;
    if(in=='<')
     nolt=0;

    if(txtlen<BUF && insidetag)
     text[txtlen++]=in;
   }

  if(p->memory_overflow)
   goto exitloop;


  }//(...end if tag)

loop:

  i++;
  fpos++;
 if(fpos<cache->size)goto loopstart;

 //=========================== end of page rendering =======================
exitloop:

 HTMLatom.xx=x;
 clearall(&y);
 frame->scroll.total_y=y+p->sizeRow+FUZZYPIX;
 HTMLatom.yy=frame->scroll.total_y;
 if(txtlen)
 {
  text[txtlen]='\0';
  fixrowsize(font,style);
  addatom(&HTMLatom,text,txtlen,TEXT,align,font,style,currentlink,0);
 }
 alignrow(x,y,orderedlist[listdepth]);

#ifdef TABLES
 if(tabledepth && currenttable[tabledepth]!=IE_NULL)
  goto tag_slash_table;
#endif

 //Arachne formatted document?
 if(noresize)
  RENDER.willadjusttables=0;

 if(frame->allowscrolling)
   frame->scroll.yvisible=1;
 else
   frame->scroll.yvisible=0;

 //kreslit rolovatko ?
 //tr.: draw scrollbar ?
 if(frame->scroll.total_x>frame->scroll.xsize+FUZZYPIX &&
    frame->scroll.ymax>user_interface.scrollbarsize &&
    frame->allowscrolling && !p->rendering_target)
 {
  frame->scroll.xvisible=1;

  ScrollInit(&frame->scroll,
             frame->scroll.xsize,
             frame->scroll.ymax-user_interface.scrollbarsize, //visible y
             frame->scroll.ymax, //max y
             frame->scroll.xtop,
             frame->scroll.ytop,
             frame->scroll.total_x, //total x
             frame->scroll.total_y);//total y
 }
 else
 {
  frame->scroll.xvisible=0;

  if(frame->posX>0)
   frame->posX=0;
  ScrollInit(&frame->scroll,
             frame->scroll.xsize,
             frame->scroll.ymax, //visible y
             frame->scroll.ymax, //max y
             frame->scroll.xtop,
             frame->scroll.ytop,
             frame->scroll.total_x, //total x
             frame->scroll.total_y);//total y
 }

 closeHTML(cache,p->html_source);

 //kdyz budu muset delat tabulky, tak se ani nezdrzovat:
 //tr.: if I have to create tables, then not to loose time:
 //if download was aborted, document will be redrawn later
 if((  (GLOBAL.validtables!=TABLES_UNKNOWN || !RENDER.willadjusttables)
     || p->html_source==HTTP_HTML)
    && !GLOBAL.abort)
 {
  mouseoff();

  if(frame->posY+frame->scroll.ysize>frame->scroll.total_y)
  {
   frame->posY=frame->scroll.total_y-frame->scroll.ysize;
   if(frame->posY<0)
    frame->posY=0;
   notrefreshed=1;
  }

  // Do not try to understand this, I don't understand it neither ;--)
  // But it works, and it should prevent users from seeing distorted
  // tables
  if(!GLOBAL.norefresh
     &&
     !p->rendering_target
     &&
     (
      p->htmlframe[p->currentframe].next==-1 || p->html_source==HTTP_HTML
     )
     &&
     (
      user_interface.quickanddirty || !RENDER.willadjusttables || GLOBAL.validtables!=TABLES_UNKNOWN
     )
     &&
     (
      notrefreshed
      ||
      (
       user_interface.virtualysize && GLOBAL.validtables==TABLES_FINISHED
      )
      ||
      (
       y>=frame->posY
       &&
       !noresize
       &&
       lastredraw!=fpos
       &&
       (
        y<=frame->posY+frame->scroll.ysize
        ||
        lastredrawy<=frame->posY+frame->scroll.ysize
        ||
        p->prevHTMLtable!=IE_NULL
       )
      )
     )
    )
  {
   if(RENDER.willadjusttables && !user_interface.quickanddirty)
    redrawHTML(REDRAW_NO_MESSAGE,REDRAW_SCREEN);
   else
    redrawHTML(REDRAW_NO_MESSAGE,REDRAW_CREATE_VIRTUAL);
  }
  else if(!arachne.framescount)
   ScrollDraw(&frame->scroll,frame->posX,frame->posY);
 }//endif redraw


 p->activeframe=arachne.target;

 //treat frames
 if(arachne.framescount)
 {
  XSWAP dummy1;
  unsigned dummy2;

  //clear ugly remains of previous screen:
  if(!p->currentframe)
   redrawHTML(REDRAW_NO_MESSAGE,REDRAW_SCREEN);

  if(p->htmlframe[p->currentframe].next!=-1)
  {
   do
   {
    kbhit();
    p->currentframe=p->htmlframe[p->currentframe].next;
   }
   while(p->htmlframe[p->currentframe].hidden && p->htmlframe[p->currentframe].next!=-1);

   AnalyseURL(p->htmlframe[p->currentframe].cacheitem.URL,&url,IGNORE_PARENT_FRAME);

   if(p->currentframe==arachne.target && p->forced_html)
    goto insertframe;


   if(SearchInCache(&url,&(p->htmlframe[p->currentframe].cacheitem),&dummy1,&dummy2) ||
      p->html_source==HTTP_HTML && p->currentframe==arachne.target)
    goto insertframe;

   arachne.newframe=p->currentframe;    //load missing frames from...
  }
 }

 if(search4maps)
  LinkUSEMAPs();

 if(GLOBAL.validtables==TABLES_UNKNOWN && RENDER.willadjusttables)
  GLOBAL.validtables=TABLES_EXPAND;
 else
  GLOBAL.validtables=TABLES_FINISHED;

 farfree(tagargptr);
 farfree(text);
 farfree(thistable);
 farfree(img);
 return 1;
}

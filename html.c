// ========================================================================
// HTML rendering routines for Arachne WWW browser
// (c)1997,1998,1999 Michael Polak, Arachne Labs (xChaos software)
// ========================================================================

#include "arachne.h"
#include "html.h"
#include "htmtable.h"
#include "internet.h"

int renderHTML(char source,char forced_html, char printing)
{
 //not specific to frames:
 char search4maps=0;

 //specific to frames:
 int i,bflen;
 long fpos;
 char tagname[16],entityname[10],pom[2],*tagarg,*tagargptr;
 int taglen,tag,lasttag,endoftag;
 char param,entity;
 int vallen,txtlen,entilen;
 char str[80];
 char uvozovky,apostrof,argument,comment,nolt,invisibletag,argspc,basetarget;
 int x;
 long y,currentbuttony;
 long tdheight=0;
 int font,basefont;
 char style,align,basealign[17],valign=MIDDLE;
 struct Fontstack fontstack;
 int xsize,lastspcpos,lastspcx,maxoption;
 char pre,charsize,nobr;
 char lastspace,lastentity,alreadyframe,notrefreshed;
 long lastredraw,lastredrawy;
 char nownobr,nbsp,boom,tabledepth,istd;
 int orderedlist[MAXTABLEDEPTH+2];  //0..unordered, >0...num, <0...nolist
 int nobr_x_anchor,currentbuttonx;
 struct Url url;
 struct HTMLrecord HTMLatom;
 char plaintext,istextarea,isselect,istitle,isscript,input_image,ext[5];
 int leftedgestack[MAXTABLEDEPTH+2],rightedgestack[MAXTABLEDEPTH+2];
 int leftstack[MAXTABLEDEPTH+2],rightstack[MAXTABLEDEPTH+2];
 long clearleftstack[MAXTABLEDEPTH+2],clearrightstack[MAXTABLEDEPTH+2];
 long maxsumstack[MAXTABLEDEPTH+2];
 unsigned tableptrstack[MAXTABLEDEPTH+2];
 char fontstackdepth[MAXTABLEDEPTH+2];
 char centerdepth[MAXTABLEDEPTH+2];
 int tdwidth[MAXTABLEDEPTH+2];
 char emptyframeset;
 char previousframe;
 char noresize;
 long timer;
 int currentnobr=0;
 int alreadyselected=0,multiple=0;
 int listdepth,listdepthstack[2*MAXTABLEDEPTH],listedge[2*MAXTABLEDEPTH];
 struct HTTPrecord *cache;
 struct HTMLtable *tmptable;
 struct HTMLframe *frame;
 struct TMPframedata *htmldata;
 struct HTMLrecord *atomptr;
 //xSwap pointers:
 XSWAP currentlink,currentform,currentbutton,
       currenttextarea,thistableadr,
       currenttable[MAXTABLEDEPTH+2],currentcell[MAXTABLEDEPTH+2];


 // ---------------------------------------------------------------
 /* This function is called in following modes:

    1) unknown HTML page - we don't know about frames, images, tables, etc.
    2) we known size of tables, without frames - accept URL
    3) we known size of frames - use URLs from frame table
    4) we known size of the frames and tables - use URLs from frame table

 */
 // ----------------------------------------------------------------

 GLOBAL.needrender=0;
 RENDER.willadjusttables=0;

 if(arachne.target==0)
  SetInputAtom(&URLprompt,htmlframe[0].cacheitem.URL);

 //start from htmlframe[0] (will be skipped later if it is parrent frame)
 if(forced_html==RELOAD_HTML_FRAMES)
 {
  forced_html=0;
  currentframe=htmlframe[0].next;
  if(currentframe<0)
  {
   currentframe=0;
   arachne.target=0;
   arachne.framescount=0; //delete frames!
   reset_frameset();
   htmlframe[0].next=-1;
  }

  if(!printing)
   DrawTitle(0);
 }
 else
 {
  currentframe=0;
  if(arachne.target==0)
  {
   arachne.framescount=0; //delete frames!
   reset_frameset();
   htmlframe[0].next=-1;
  }
 }

 //dealocate memory
 Deallocmem();

 //reset table pointer
 nextHTMLtable=firstHTMLtable;
 prevHTMLtable=IE_NULL;

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
 tagargptr=&text[BUF/2];
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
 font=3;
 basefont=3;
 style=0;
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
 istextarea=0;
 isselect=0;
 istitle=0;
 isscript=0;
 emptyframeset=-1;
 noresize=0;
 timer=time(NULL);
 fontstack.depth=-1;

 currentlink=IE_NULL;
 currentform=IE_NULL;
 currenttextarea=IE_NULL;
 currentbutton=IE_NULL;
 thistableadr=IE_NULL;
 currenttable[0]=IE_NULL;
 currentcell[0]=IE_NULL;
 centerdepth[0]=0;
 tdwidth[0]=0;

 //skip unmodified parent frames (typicaly htmlframe[0])
 while(htmlframe[currentframe].hidden && currentframe<MAXFRAMES-1 &&
       currentframe!=arachne.target && htmlframe[currentframe].next!=-1)
 {
  kbhit();
  currentframe=htmlframe[currentframe].next;
 }
 activeframe=basetarget=currentframe;
 previousframe=currentframe;

 //define pointer to current html frame:
 frame=&htmlframe[currentframe];
 //define pointer to current cache item:
 cache=&frame->cacheitem;
 //define pointer to current temporary frame data
 htmldata=&tmpframedata[currentframe];

 left=leftedge=0;
 right=rightedge=frame->scroll.xsize;

 //only for first rendering:
 if(!GLOBAL.isimage || GLOBAL.source)
 {
  if(currentframe==0)
  {
   if(GLOBAL.source)
   {
    sprintf(text,"Source of %s",cache->URL);
    MakeTitle(text);
   }
   else
   if(GLOBAL.validtables==TABLES_UNKNOWN && currentframe==0)
   {
    MakeTitle("");
    if(source==HTTP_HTML)
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
 if((!strcmpi(ext,"HTM") || forced_html) && !GLOBAL.source)
 {
  //formatovani a barvy v HTML dokumentu:
  x=frame->marginwidth;
  y=frame->marginheight;
  plaintext=0;
  left=leftedge=leftstack[0]=leftedgestack[0]=frame->marginwidth;;
  right=rightedge=rightstack[0]=rightedgestack[0]=frame->scroll.xsize-frame->marginwidth-FUZZYPIX;
  ResetHtmlPage(htmldata,TEXT_HTML,1);
  frame->scroll.xvisible=0;
  if(currentframe==0)
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
  right=rightedge=rightedgestack[0]=(CONSOLEWIDTH-2)*space(SYSFONT);
#ifndef NOPS
 else if(printing)
  right=rightedge=rightedgestack[0]=(int)((user_interface.postscript_x-51)*5);
#endif

 if(!GLOBAL.isimage /*&& GLOBAL.validtables!=TABLES_EXPAND*/ && !GLOBAL.source && !forced_html)
 {
  frame->posX=cache->x;
  frame->posY=cache->y;
 }

 lastspcx=left;
 percflag=0;
 HTMLatom.x=x;
 HTMLatom.y=y;
 r=htmldata->textR;
 g=htmldata->textG;
 b=htmldata->textB;
 listdepth=0;
 orderedlist[0]=0;
 clearleft=clearright=0;
 clearleftstack[0]=clearrightstack[0]=0;
 maxsumstack[0]=0;
 tableptrstack[0]=IE_NULL;

 pre=plaintext;
 if(plaintext)font=SYSFONT;

 textrowsize=rowsize=fonty(font,style); //?rowsize?
 xsize=0;

#ifdef VIRT_SCR
 //vynuluji virtualni obrazovku
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
 if(!GLOBAL.isimage && GLOBAL.validtables==TABLES_UNKNOWN && source==HTTP_HTML)
  redrawHTML(REDRAW_WITH_MESSAGE,REDRAW_SCREEN);

 //-----pouze obrazek ------------------
 if(*ext && strstr(imageextensions,ext) && !forced_html)
 {
  int znamrozmerx=0,znamrozmery=0;

#ifndef NOTCPIP
  if(source==HTTP_HTML && arachne.target==currentframe)
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
   if(drawGIF(img)==1)
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
  if(!printing)
   DrawTitle(0);

  if(!znamrozmerx)
   img->size_x=8*strlen(img->alt)+4;

  if(!znamrozmery) img->size_y=18;

  HTMLatom.x=0;
  HTMLatom.y=0;
  HTMLatom.xx=img->size_x;
  HTMLatom.yy=img->size_y;
  rowsize=img->size_y;
  frame->scroll.total_x=img->size_x;
  addatom(&HTMLatom,img,sizeof(struct picinfo),IMG,0,0,0,currentlink,1);
  lastredraw=-1l;

  goto exitloop;
 }

 if(!openHTML(cache,source))
 {
  if(arachne.target==currentframe)
   return 0;
  else
   goto exitloop;
 }


loopstart: //------------- vlastni cykl - analyza HTML i plain/text---------
  if((percflag & 0x1ff) == 0x1ff ) //kazdych 512
   if(!printing && GUITICK())
    if(GLOBAL.gotolocation || GLOBAL.abort)
     goto exitloop;

  if(!percflag /* && !(currentframe<arachne.framescount)*/)
  {
   int prc;

   frame->scroll.total_y=y+rowsize;

   if(!printing && (user_interface.quickanddirty  || noresize ||
      !(GLOBAL.validtables==TABLES_UNKNOWN && RENDER.willadjusttables)))
   {
    //rendering of distorted tables is disabled by user_interface.quickanddirty

    if(y>frame->posY+frame->scroll.ysize &&
       lastredrawy+fonty(basefont,0)<frame->posY+frame->scroll.ysize &&
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
      ScrollDraw(&frame->scroll,frame->posX,frame->posY);
      mouseon();
     }
    }
   }

   if(cache->size)
    prc=(int)(100*fpos/cache->size);
   else
    prc=0;

   if(source==LOCAL_HTML)
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
  }
  percflag--;

  if(i==bflen)
  {
   bflen=readHTML(cache,source);
   //printf("buffer lenght:%d\n",bflen);

   if(bflen<=0 || memory_overflow) //end of page, or out of memory
    goto exitloop;
   else
    i=0;
  }

  if(RENDER.translatecharset)
   buf[i]=GLOBAL.codepage[(unsigned char)buf[i]];

  //text/html ignoruje konce radku, uvnitr <PRE> nebo text/plain ne:
  if(buf[i]<' ' && buf[i]>=0)
  {
   if(istextarea && (buf[i]=='\n' || buf[i]=='\r'))
   {
    if(txtlen>IE_MAXLEN)txtlen=IE_MAXLEN;
    text[txtlen]='\0';
    txtlen=0;
    appendline(currenttextarea,text,0);
    lastspace=0;
    goto loop;
   }
   else
   if(!pre || (buf[i]!='\n' /*&& buf[i]!='\r'*/) || tag || isselect)
   {
    if(tag && param && uvozovky || endoftag)
     goto loop;
    buf[i]=' ';
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
    y+=rowsize;
    goto linebreak;
   }
  }//endif

  //......................................................zpracovani 1 znaku
  if(!tag)
  {
   endoftag=0;
   if(buf[i]=='<' && !plaintext) //zacatek HTMLtagu
   {
    HTMLatom.xx=x;
     HTMLatom.y=y;
    HTMLatom.yy=y+fonty(font,style);
    text[txtlen]='\0';
    if(txtlen)
    {
     fixrowsize(font,style);
     if(!invisibletag)
      addatom(&HTMLatom,text,txtlen,TEXT,align,font,style,currentlink,0);
     if(lasttag==TAG_BODY)
      lasttag=TAG_P;
     if(istitle && !arachne.title[0] && !currentframe)
     {
      MakeTitle(text);
      if(!printing)
       DrawTitle(0);
      invisibletag=0;
      istitle=0;
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
   //Entity (&lt;,&rt; &copy ...)
   if(!plaintext && buf[i]=='&')  //HTML entita zacina '&'
   //uvnitr tagu by byt nemela
   {
    entity=1;
    entilen=0;
    entityname[0]='\0';
    goto loop;
   }
   else
   if(entity)       //vnitrek entity
   {
    if(entilen>8 || buf[i]==';' || buf[i]==' ')
    {
     entityname[entilen]='\0';
     if(entilen>0)
     {
      buf[i]=(char)HTMLentity(entityname);
      if(buf[i]==' ')
       goto nbsp;

     }
     else
      buf[i]='&';
     entity=0;
    }
    else
    {
     entityname[entilen++]=buf[i];
     goto loop;
    }
   }//endif

   if(ascii160hack && (unsigned char)buf[i]==160)
   {
    buf[i]=' ';
    nbsp:
    lastspace=0;
    lastentity=1;
    nbsp=1;
   }

   if((unsigned char)buf[i]>128 && (unsigned char)buf[i]<160)
   {
    buf[i]=' ';
   }

   // <-----------------------------------------<--------<-----------mimo tag
   if(!(buf[i]==' ' && lastspace) || pre)
   {
    if(txtlen<BUF) text[txtlen++]=buf[i];
    if(!invisibletag)
    {
     charsize=fontx(font,style,buf[i]);
     x+=charsize;
     if(x>right)
     {
      if(lastspcpos==0 || pre || nownobr)
      {
       xsize=x-HTMLatom.x;
       if(xsize<right-left && !pre && !nownobr && !nobr /*&&
          (HTMLatom.x!=left || GLOBAL.validtables)*/ || right<rightedge)
       {
        // <------------------------------------------------odsunout cely atom
        alignrow(HTMLatom.x,y,orderedlist[listdepth]);
        y+=rowsize;

        //kdyz jsou levy a pravy okraj moc blizko u sebe...
        if(xsize>right-left)
         clearall(&y);

        textrowsize=rowsize=fonty(font,style);
        x=left+xsize;
        HTMLatom.x=left;
        HTMLatom.y=y;
        HTMLatom.xx=x;
        HTMLatom.yy=y+rowsize;
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
        right+=charsize;
        //during second pass, certain <NOBR> elements will be treated as <BR>
        if(nobr)
         boom=1;
       }
      }
      else
      {
       // <---------------------------------------------------------novy radek
       //!!!text[lastspcpos-1]='\0';
       text[lastspcpos-1]='\0';
       text[txtlen]='\0';

       fixrowsize(font,style);
       HTMLatom.xx=lastspcx;
       HTMLatom.yy=y+rowsize;

       addatom(&HTMLatom,text,lastspcpos,TEXT,align,font,style,currentlink,0);
       alignrow(lastspcx,y,orderedlist[listdepth]);

       txtlen=strlen(&text[lastspcpos]);
       memmove(text,&text[lastspcpos],txtlen);
       HTMLatom.x=left;
       y=y+rowsize;
       textrowsize=rowsize=fonty(font,style); //?rowsize?
       HTMLatom.y=y;
       xsize=x-lastspcx;
       x=left+xsize;
       HTMLatom.y=y;
       lastspace=0;
       lastspcpos=0;
       lastspcx=0;
      }
     }
    }//endif viditelny text

    if(buf[i]==' ' && !nbsp || buf[i]=='/') //wrap also long unix pathnames !
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

   }//konec zpracovani jednoho znaku

  }
  else
  //................................................zpracovani vnitrku tagu
  {
   if (buf[i]=='>' && !uvozovky && (nolt || !(comment && strncmp(pom,"--",2))))
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

    tagname[taglen]='\0';
    comment=0;
    nolt=0;

    //analyza HTML TAGu -------------------------------------- HTML level 0/1
    tag=FastTagDetect(tagname);

    if(tag<TAG_SLASH || tag==TAG_SLASH_TABLE)
     endoftag=1;

    if(istextarea && tag!=TAG_SLASH_TEXTAREA ||
       istitle && tag!=TAG_SLASH_TITLE ||
       isselect && tag!=TAG_SLASH_SELECT && tag!=TAG_OPTION && tag!=TAG_SLASH_OPTION ||
       isscript && tag!=TAG_SLASH_SCRIPT && tag!=TAG_SLASH_NOFRAMES && tag!=TAG_ARACHNE_BONUS)
     tag=0;

    switch(tag)
    {
     case TAG_P: //<P>
     case TAG_DIV: //<DIV>

     p:
     PARAGRAPH;
     textrowsize=rowsize=0; //fonty(font,style); //?rowsize?
     invisibletag=0; //paragraf ukonci chybny option, title, apod.
     //align=basealign[tabledepth];

     alignset:
     x=left;
     lastspace=1;//mazat mezery
     if(getvar("ALIGN",&tagarg))
     {
//       basealign[tabledepth]=align;
      if(!strcmpi(tagarg,"CENTER"))    align=align | CENTER;
      else if(!strcmpi(tagarg,"RIGHT"))align=align | RIGHT;
      else if(!strcmpi(tagarg,"LEFT"))
      {
       if(align & CENTER) align-=CENTER;
       if(align & RIGHT) align-=RIGHT;
      }
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

       if(!strncmpi(tagarg,"mailto:",7))
        target=findtarget(0);
       else
        target=findtarget(basetarget);

       //vlozit link:
       if(tagarg[0]!='#')
       {
        AnalyseURL(tagarg,&url,currentframe); //(plne zneni...)
        url2str(&url,text);
        tagarg=text;
       }


       //vyrobim si pointr na link, a od ted je vsechno link:
       addatom(&HTMLatom,tagarg,strlen(tagarg),HREF,align,target,removable,IE_NULL,1);
       currentlink=lastHTMLatom;
       pushfont(font,style,&fontstack);

       contlink:
       style=style | UNDERLINE;
       r=htmldata->linkR;
       g=htmldata->linkG;
       b=htmldata->linkB;
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

     nolink:
     if(!popfont(&font,&style,&fontstack))
     {
      if(style & UNDERLINE)
       style -= UNDERLINE;
      r=htmldata->textR;
      g=htmldata->textG;
      b=htmldata->textB;
     }
     fixrowsize(font,style);
     break;

     case TAG_IMG: //<IMG>

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
       ismap=4;
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

       AnalyseURL(tagarg,&url,currentframe);
       url2str(&url,img->URL);

       //printf("Image URL is: %s\n",img->URL);
       
       if(QuickSearchInCache(&url,&HTTPdoc,&dummy,&status))
       {
        strcpy(img->filename,HTTPdoc.locname);

        if(img->filename[0])
        {
         img->sizeonly=1;

         get_extension(HTTPdoc.mime,ext);
         //printf("image extension: %s\n",ext);
         if(!strcmpi(ext,"GIF") ||
            !strcmpi(ext,"BMP"))
         {
          if(drawGIF(img)==1)
           znamrozmerx=znamrozmery=1;
          else
           failedGIF=1;
         }
         else
         if(!strcmpi(ext,"IKN") && !cgamode)
         {
          img->size_y=60;
          img->size_x=60;
          znamrozmerx=znamrozmery=1;
         }
        }//endif nasel jsem neco
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
       ismap=ismap|1;
      }

      if(getvar("USEMAP",&tagarg))
      {
       ismap=2;
       addatom(&HTMLatom,tagarg,strlen(tagarg),USEMAP,align,0,0,IE_NULL,1);
       imglink=lastHTMLatom;
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
       int max=right-left;
       int i;
       if(max<0) max=0;
       i=try2getnum(tagarg,max);
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
      if(user_interface.killadds && reg && img->size_x==468 && img->size_y==60)
       break;

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

      if(imgleft)
      {
       if(x>left)
        HTMLatom.y=y+rowsize;
       else
       {
        HTMLatom.y=y;
        x+=img->size_x;
       }
       HTMLatom.x=left;
       left+=img->size_x;
       HTMLatom.xx=left;
       HTMLatom.yy=HTMLatom.y+img->size_y;
       if(HTMLatom.yy>clearleft)
        clearleft=HTMLatom.yy;

       if(left>right)
        clearall(&y);

       imgalign=0; //a hlavne tady: rikam, ze uz s tim nebudu hejbat !!!
      }
      else if(imgright)
      {
       if(left+img->size_x>right)
        clearall(&y);

       if(right-img->size_x<x)
       {
        clearall(&y);
        x=left;
       }

       if(left+img->size_x>right)
       {
        right+=img->size_x;
        rightedge=right;
       }

       HTMLatom.xx=right+1;
       HTMLatom.y=y;
       right-=img->size_x;
       HTMLatom.x=right+1;
       HTMLatom.yy=y+img->size_y;
       if(HTMLatom.yy>clearright)
        clearright=HTMLatom.yy;

       imgalign=0; //a hlavne tady: rikam, ze uz s tim nebudu hejbat !!!
      }
      else
      {
       //normalni - misto jednoho atomu
       //nevejde se nam tam ?
       if(x+img->size_x>right)
       {
        if(!pre && !nownobr)
        {
         if(img->size_x<=right-left && x>left ||
            !tabledepth && !clearleft && !clearright)
         {
          alignrow(x,y,orderedlist[listdepth]);
          y+=rowsize;
          x=left;
          rowsize=0; //!hned spravim
         }
         else clearall(&y);
        }
       }

       if(rowsize<img->size_y)
        rowsize=img->size_y;

       if(imgalign & BOTTOM)
        textrowsize=rowsize;
       else if(imgalign & MIDDLE && textrowsize<rowsize/2)
        textrowsize=rowsize/2;

       HTMLatom.x=x;
       HTMLatom.y=y;
       x+=img->size_x;
       HTMLatom.xx=x;
       HTMLatom.yy=y+img->size_y;
      }

      addatom(&HTMLatom,img,sizeof(struct picinfo),IMG,imgalign,ismap,border,imglink,imgright);
      //imgright=1 --> neposouvat pravy okraj!!!

      nownobr=nobr;
      lastspace=1;//mazat mezery
     }
     break;

     case TAG_BR: //<BR>
     if(xsum>maxsum)
      maxsum=xsum;
     xsum=0;
     if(rowsize==0)
      textrowsize=rowsize=fonty(font,style);
     br:

     alignrow(x,y,orderedlist[listdepth]);
     y+=rowsize;

     if(getvar("CLEAR",&tagarg))
     {
      if(clearleft>y && !strcmpi(tagarg,"LEFT"))
      {
       y=clearleft;
       clearleft=0;
       left=leftedge;
      }
      else
      if(clearright>y && !strcmpi(tagarg,"RIGHT"))
      {
       y=clearright;
       clearright=0;
       right=rightedge;
      }
      else
      if(!strcmpi(tagarg,"ALL"))
      {
       if(clearleft>y && clearleft>=clearright)
       {
        y=clearleft;
       }
       else
       if(clearright>y)
       {
        y=clearright;
       }
       left=leftedge;
       right=rightedge;
       clearleft=clearright=0;

      }
     }

     linebreak:
     textrowsize=rowsize=fonty(font,style); //?rowsize?
     x=left;
     lastspace=1;//mazat mezery
     HTMLatom.x=x;
     HTMLatom.y=y;
     break;

    //  -----------------------------------------(netscape extensions block)
     case TAG_CENTER: //<CENTER>

     align=align | CENTER;
     basealign[tabledepth]=align;
     centerdepth[tabledepth]++;
     if(x>left)
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
     if(x>left)
      goto br;
     break;

     case TAG_SUP: //<CENTER>

     align=align | SUP;
     font-=1;
     if(rowsize<3*fonty(font,style)/2)
      rowsize+=fonty(font,style)/2;
     break;

     case TAG_SLASH_SUP:

     if(align & SUP)
     {
      align=align - SUP;
      font=basefont;
     }
     break;

     case TAG_SUB: //<CENTER>

     align=align | SUB;
     font-=1;
     if(rowsize<3*fonty(font,style)/2)
      rowsize+=fonty(font,style)/2;
     break;

     case TAG_SLASH_SUB:

     if(align & SUB)
     {
      align=align - SUB;
      font=basefont;
     }
     break;

     case TAG_NOBR: //</NOBR>

     nobr=1;
     nownobr=0;
     boom=0;
     nobr_x_anchor=x;
     if(xsum>maxsum)
      maxsum=xsum;
     xsum=0;

     if(currentnobr<MAXNOBR)
     {
      if((GLOBAL.validtables && nobr_overflow[currentnobr] || noresize) && x>left)
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
      y+=rowsize;
     }
     if(xsum>maxsum)
      maxsum=xsum;
     xsum=0;
     nobr=0;
     nownobr=0;
     if(currentnobr<MAXNOBR-1)
      currentnobr++;
     break;

     case TAG_SLASH_P: //</P>
     case TAG_SLASH_DIV: //</DIV>

     align=basealign[tabledepth];
     goto p;

     // --------------------------------------------- HTML level 1 - headers
     case TAG_H1: //<H1>

     pushfont(font,style,&fontstack);
     font=6;
     header:
     style=1;
     PARAGRAPH;
     textrowsize=rowsize=fonty(font,style);
     goto alignset;

     case TAG_H2: //<H2>

     pushfont(font,style,&fontstack);
     font=5;
     goto header;

     case TAG_H3: //<H3>

     pushfont(font,style,&fontstack);
     font=4;
     goto header;

     case TAG_H4: //<H4>

     pushfont(font,style,&fontstack);
     font=3;
     goto header;

     case TAG_H5: //<H5>

     pushfont(font,style,&fontstack);
     font=2;
     goto header;

     case TAG_H6: //<H6>

     pushfont(font,style,&fontstack);
     font=1;
     goto header;

     //</H1>,</H2>,</H3>,</H4>,</H5>,</H6>
     case TAG_SLASH_H1:
     case TAG_SLASH_H2:
     case TAG_SLASH_H3:
     case TAG_SLASH_H4:
     case TAG_SLASH_H5:
     case TAG_SLASH_H6:

     if(!popfont(&font,&style,&fontstack))
     {
      font=basefont;
      style=0;
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

     font=basefont;
     style=FIXED;
     pre=1;
     fixrowsize(font,style);
     break;

     case TAG_SLASH_PRE:

     font=basefont;
     style=0;
     pre=0;
     goto br;

     case TAG_FONT:     //<FONT>
     case TAG_BASEFONT: //<BASEFONT>

     pushfont(font,style,&fontstack);
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
     }

     if(getvar("COLOR",&tagarg))
     {
      try2readHTMLcolor(tagarg,&r,&g,&b);
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


     if(tag==TAG_BASEFONT) // <BASEFONT>
      basefont=font;

     fixrowsize(font,style);
     break;

     case TAG_SLASH_BASEFONT: // </BASEFONT>

     basefont=3; //normalni velikost
     // continue...

     case TAG_SLASH_FONT:

     if(!popfont(&font,&style,&fontstack))
     {
      font=basefont;
      r=htmldata->textR;
      g=htmldata->textG;
      b=htmldata->textB;
      if(style & TEXT3D)
       style -= TEXT3D;

      if(currentlink==IE_NULL)
       goto nolink;
      else
       goto contlink;
     }
     break;

     case TAG_SLASH_FRAMESET:

     if(arachne.framescount>=currentframe)
      goto br;

     case TAG_BIG: //<BIG>

     font+=2;
     break;

     case TAG_SMALL: //<SMALL>

     font-=1;
     break;

     case TAG_SLASH_BIG:
     case TAG_SLASH_SMALL:

     font=basefont;
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
       width=try2getnum(tagarg,right-left);
      }
      else
       width=right-left;

      alignrow(x,y,orderedlist[listdepth]);
      if(x>left)y+=rowsize;
      rowsize=fonty(basefont,0);
      if(size+4>rowsize)rowsize=size+4;
      x=left;
      HTMLatom.x=x;
      HTMLatom.y=y+rowsize/2-size/2;
      HTMLatom.xx=x+width;
      HTMLatom.yy=y+rowsize/2-size/2+size;
      addatom(&HTMLatom,"",0,HR,hralign,noshade,0,IE_NULL,0);
      alignrow(HTMLatom.xx,HTMLatom.y,orderedlist[listdepth]);
      y+=rowsize;
      textrowsize=rowsize=fonty(font,style);
     }
     break;

     case TAG_B: //<B>,<STRONG>,<S>

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

      //if there are too many nested tables, than we will give up...
      if(tabledepth>MAXTABLEDEPTH)
       goto p;

      //let's get temporary pointer to current table structure (16bit DOS only)
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
      if(x>left)
      {
       alignrow(x,y,orderedlist[listdepth]);
       y+=rowsize;
       x=left;
      }

      //alocation of new table (max. number of tables is currently limited)
      if(nextHTMLtable==IE_NULL)
      {
       newtab=1;
       thistableadr=ie_putswap((char *)thistable,sizeof(struct HTMLtable),CONTEXT_TABLES);
       if(thistableadr==IE_NULL)
        goto p;

       if(firstHTMLtable==IE_NULL)
        firstHTMLtable=thistableadr;

       if(prevHTMLtable!=IE_NULL)
       {
        tmptable=(struct HTMLtable *)ie_getswap(prevHTMLtable);
        if(tmptable)
        {
         tmptable->nextHTMLtable=thistableadr;
         swapmod=1;
        }
        else
         MALLOCERR();
       }
       prevHTMLtable=thistableadr;

      }
      else
      {
       //tmptable=(struct HTMLtable *)ie_getswap(Tablelist.lineadr[Tablelist.cur]);
       tmptable=(struct HTMLtable *)ie_getswap(nextHTMLtable);
       if(tmptable)
       {
        memcpy(thistable,tmptable,sizeof(struct HTMLtable));
        //thistableadr=Tablelist.lineadr[Tablelist.cur];
        thistableadr=prevHTMLtable=nextHTMLtable;
        nextHTMLtable=tmptable->nextHTMLtable;
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

      thistable->maxwidth=right-left-2*border;
      if(getvar("WIDTH",&tagarg))
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

      HTMLatom.x=left;
      HTMLatom.xx=HTMLatom.x+twidth;

      if(2*HTMLatom.xx>3*right)
       clearall(&y);
      else if(HTMLatom.xx>right)
       right=HTMLatom.xx;

      alignarg=getvar("ALIGN",&tagarg);
      if(alignarg || (align & RIGHT) || (align & CENTER))
      {
       if(GLOBAL.validtables==TABLES_UNKNOWN)
        RENDER.willadjusttables=1;
       if(!strcmpi(tagarg,"LEFT"))
       {
        if(!GLOBAL.validtables && thistable->fixedmax==0)
        {
         thistable->maxwidth/=2;
         twidth/=2;
        }
        tabalign=LEFT;
       }
       else
       if(!strcmpi(tagarg,"RIGHT") || !alignarg && (align & RIGHT))
       {
        if(!GLOBAL.validtables && thistable->fixedmax==0)
        {
         thistable->maxwidth/=2;
         twidth/=2;
        }
        HTMLatom.x=right-FUZZYPIX-twidth;
        if(HTMLatom.x<left)
        {
         clearall(&y);
         HTMLatom.x=left;
        }
        HTMLatom.xx=left+twidth;
        tabalign=RIGHT;
       }
       else
       if(!strcmpi(tagarg,"CENTER") || !alignarg && (align & CENTER))
       {
        newx=(int)(right-left-twidth)/2;
        if(newx<left || GLOBAL.validtables==TABLES_UNKNOWN)
         HTMLatom.x=left;
        else
         HTMLatom.x=left+newx;
        HTMLatom.xx=HTMLatom.x+twidth;
        tabalign=CENTER;
       }
      }

      if(getvar("BGCOLOR",&tagarg))
      {
       makestr(thistable->tablebg,tagarg,SHORTSTR);
       makestr(thistable->rowbg,tagarg,SHORTSTR);
      }
      else
      {
       thistable->tablebg[0]='\0';
       thistable->rowbg[0]='\0';
      }

      HTMLatom.y=y;
      HTMLatom.yy=y+2*border;
      listdepthstack[tabledepth]=listdepth;
      orderedlist[++listdepth]=0;
      leftstack[tabledepth]=left;
      rightstack[tabledepth]=right;
      leftedgestack[tabledepth]=leftedge;
      rightedgestack[tabledepth]=rightedge;
      clearleftstack[tabledepth]=clearleft;
      clearrightstack[tabledepth]=clearright;
      if(xsum>maxsum)
       maxsum=xsum;
      maxsumstack[tabledepth]=maxsum;
      maxsum=0l;
      pushfont(font,style,&fontstack);
      fontstackdepth[tabledepth]=fontstack.depth;
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
      tmptable=(struct HTMLtable *)ie_getswap(thistableadr);
      if(tmptable)
      {
       memcpy(tmptable,thistable,sizeof(struct HTMLtable));
       swapmod=1;
      }
      else
       MALLOCERR();

      //vyrobim si pointer na tabulku:
      //addatom(&HTMLatom,"",0,TABLE,TOP,border,tabalign,Tablelist.lineadr[Tablelist.cur],1);
      addatom(&HTMLatom,"",0,TABLE,TOP,border,tabalign,thistableadr,1);
//      currenttable[tabledepth]=HTMLdoc.lineadr[HTMLdoc.len-1];
      currenttable[tabledepth]=lastHTMLatom;

      //a v seznamu je na rade dalsi tabulka...
      //Tablelist.cur++;
      istd=0;
     }
     break;

     case TAG_SLASH_TABLE: //</TABLE>

     {
      int cellx;
      long celly;
      int tblstart;
      long tblystart;
      char border,tabalign;

      if(tabledepth && currenttable[tabledepth]!=IE_NULL)
      {
       clearall(&y);
       if(x>left)
        {
         alignrow(x,y,orderedlist[listdepth]);
         y+=rowsize;
        }

       //uzavreni posledniho policka
       atomptr=(struct HTMLrecord *)ie_getswap(currenttable[tabledepth]);
       if(atomptr)
       {
        XSWAP parenttableadr=atomptr->linkptr;

        tblstart=atomptr->x;
        tblystart=atomptr->y;
        border=atomptr->data1;
        tabalign=atomptr->data2;

        if(currentcell[tabledepth]!=IE_NULL) //uzavrit posledni ctverecek na radce
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

          // fix desired table cell width data:
          if(xsum>maxsum)
           maxsum=xsum;
          if(tdwidth[tabledepth] && tdwidth[tabledepth]<maxsum)
           maxsum=tdwidth[tabledepth];

          if(y<tdheight)
           y=tdheight;

          if(processcell(tmptable,maxsum,rightedge-leftedge+2*tmptable->cellpadding,y+tmptable->cellpadding,&cellx) && GLOBAL.validtables==TABLES_UNKNOWN)
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

         fixrowspan(tmptable,1,closeptrs);
         end=tmptable->tdend;

         if(calcwidth(tmptable) && GLOBAL.validtables==TABLES_UNKNOWN)
          RENDER.willadjusttables=1;

         //return to previous state of reneding engine
         maxsum=maxsumstack[tabledepth-1];

         //consider total real width of inline table:
         if(tmptable->fixedmax
            && tmptable->maxwidth>maxsum)
          maxsum=tmptable->maxwidth;

         //consider total "desired width" of inline table:
         {
          long desired=2*border+tmptable->realwidth+tmptable->totalxsum;

          if(tmptable->fixedmax==PIXELS_FIXED_TABLE) //not percent specification!
            desired=2*border+tmptable->maxwidth;

          if(desired>maxsum)
           maxsum=desired;
         }

         xsum=0;

         //zapsani zavrene tabulky
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
         if(cellx>maxsum)
           maxsum=cellx;

         if(closeatom(currenttable[tabledepth],cellx,celly)
            && tabalign && !GLOBAL.validtables)
          RENDER.willadjusttables=1;
         if(tblstart+cellx>frame->scroll.total_x)
         {
          frame->scroll.total_x=tblstart+cellx;
         }

         //zarovnam posledni radek tabulky
         fixrowspan_y(closeptrs,end);
         tablerow(start,end,parenttableadr);
        }
        else
         MALLOCERR();
       }
       else
        MALLOCERR();

       //tablerightedge
       tabledepth--;
       listdepth=listdepthstack[tabledepth];
       leftedge=leftedgestack[tabledepth];
       left=leftstack[tabledepth];
       rightedge=rightedgestack[tabledepth];
       right=rightstack[tabledepth];
       clearleft=clearleftstack[tabledepth];
       clearright=clearrightstack[tabledepth];
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



       if(cellx>=right-left && tabledepth)
       {
        rightedge=right=left+cellx;
       }

       align=basealign[tabledepth];
       currentlink=IE_NULL;
       if(!popfont(&font,&style,&fontstack))
       {
        font=basefont;
        style=0;
        r=htmldata->textR;
        g=htmldata->textG;
        b=htmldata->textB;
       }
       y=celly;

       /*if(!tabledepth)
        y+=fonty(font,style)/4;*/

       invisibletag=0;
       rowsize=textrowsize=0;
       nobr=0;
       nownobr=0;
       if(GLOBAL.validtables)
       {
        if(tabalign==LEFT && cellx+FUZZYPIX<frame->scroll.total_x)
        {
         left=tblstart+cellx+FUZZYPIX;
         if(right>left)
         {
          if(celly>clearleft)
           clearleft=celly;
          y=tblystart;
         }
         else
          left=leftedge;

        }
        else
        if(tabalign==RIGHT && cellx+FUZZYPIX<frame->scroll.total_x)
        {
         right=tblstart-FUZZYPIX;
         if(right>left)
         {
          if(celly>clearright)
           clearright=celly;
          y=tblystart;
         }
         else
          right=rightedge;
        }
       }

       //clear left and right ... AFTER returning to <TABLE ALIGN=.....>
       if(clearright && y>=clearright)
       {
        right=rightedge;
        clearright=0;
       }

       if(clearleft && y>=clearleft)
       {
        if(orderedlist[listdepth]==0)left=leftedge;
        clearleft=0;
       }

       if(!GLOBAL.validtables && (tabalign==LEFT || tabalign==RIGHT || tabalign==CENTER))
        RENDER.willadjusttables=1;

       tdheight=y;
       x=left;
       //fixrowsize(font,style);
       if(tabledepth)
        istd=1;
      }
      else
       goto p;
     }
     break;

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
      if(!GLOBAL.validtables)
       invisibletag=0;
      //pokud jsem uvnitr tabulky
      if(tabledepth && currenttable[tabledepth]!=IE_NULL)
      {
       clearall(&y);
       if(x>left)
       {
        alignrow(x,y,orderedlist[listdepth]);
        y+=rowsize;
       }

       atomptr=(struct HTMLrecord *)ie_getswap(currenttable[tabledepth]);
       if(atomptr)
       {
        XSWAP parenttableadr=atomptr->linkptr;
        int cellx;

        //getswap musim delat pokazde, protoze tabulka je dynamicky ulozena
        if(thistableadr==parenttableadr)
          tmptable=thistable;
        else
        {
         tmptable=(struct HTMLtable *)ie_getswap(parenttableadr);
           //printf("tables out of sync");
        }
        if(tmptable)
        {
         if(!bgcolor[0])
          strcpy(tmptable->rowbg,tmptable->tablebg);
         else
          makestr(tmptable->rowbg,bgcolor,SHORTSTR);

         tmptable->valignrow=valignrow;
         if(thistableadr!=parenttableadr)
          swapmod=1;

         if(currentcell[tabledepth]!=IE_NULL)
         {

          // fix desired table cell width data:
          if(xsum>maxsum)
           maxsum=xsum;
          if(tdwidth[tabledepth] && tdwidth[tabledepth]<maxsum)
           maxsum=tdwidth[tabledepth];

          if(y<tdheight)
           y=tdheight;

          if(processcell(tmptable,maxsum,rightedge-leftedge+2*tmptable->cellpadding,y+tmptable->cellpadding,&cellx) && GLOBAL.validtables==TABLES_UNKNOWN)
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

         if(tmptable->x) //prvni <TR> ignorovat!
         {
          fixrowspan(tmptable,0,closeptrs);
          end=tmptable->tdend;
          tmptable->y++;
          tmptable->x=0;
          tmptable->tdstart=tmptable->tdend+tmptable->cellspacing;
          tmptable->tdend=tmptable->tdstart;

          if(tmptable->tdend<tmptable->nexttdend)
           tmptable->tdend=tmptable->nexttdend;
          tmptable->nexttdend=tmptable->tdend;

          if(thistableadr!=parenttableadr)
           swapmod=1;//<-ulozit zmeny!

          fixrowspan_y(closeptrs,end);
          tablerow(start,end,parenttableadr);

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
      {
       style=0;
      }

      clearall(&y);
      if(x>left)
      {
       alignrow(x,y,orderedlist[listdepth]);
       y+=rowsize;
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

       if(getvar("BGCOLOR",&tagarg))
       {
        //printf("[%s]",tagarg);
        try2readHTMLcolor(tagarg,&r,&g,&b);
        bgcolor=1;
       }

       if(getvar("BACKGROUND",&tagarg) && tagarg[0] && !cgamode)
       {
        AnalyseURL(tagarg,&url,currentframe); //(plne zneni...)
        url2str(&url,img->URL);
        init_picinfo(img);
        img->URL[URLSIZE-1]='\0';
        //printf("background image=%s\n",img->URL);
       }

      if(getvar("HEIGHT",&tagarg))
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
         if(!bgcolor && tmptable->rowbg[0])
         {
          try2readHTMLcolor(tmptable->rowbg,&r,&g,&b);
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
          else
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
          if(xsum>maxsum)
           maxsum=xsum;
          if(tdwidth[tabledepth] && tdwidth[tabledepth]<maxsum)
           maxsum=tdwidth[tabledepth];

          if(y<tdheight)
           y=tdheight;

          if(processcell(tmptable,maxsum,rightedge-leftedge+2*tmptable->cellpadding,y+tmptable->cellpadding,&cellx) && GLOBAL.validtables==TABLES_UNKNOWN)
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
         x=leftedge=left=HTMLatom.x+tmptable->cellpadding;
         rightedge=right=HTMLatom.xx-tmptable->cellpadding;
         y=HTMLatom.y+tmptable->cellpadding;
         HTMLatom.yy=tmptable->tdend+tmptable->cellpadding;

         tdheight=y+newtdheight;

         if(caption) //nadpis
          border=0;

         if(right-left<FUZZYPIX) //v uzkych sloupcich nedelat bordel!
          align=BOTTOM;

         if(img->URL[0])
          addatom(&HTMLatom,img,sizeof(struct picinfo),TD_BACKGROUND,valign,border,bgcolor,parenttableadr,1);
         else
          addatom(&HTMLatom,"",0,TD,valign,border,bgcolor,parenttableadr,1);

//         currentcell[tabledepth]=HTMLdoc.lineadr[HTMLdoc.len-1];
         currentcell[tabledepth]=lastHTMLatom;
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
            tmptable->closerowspan[tmptable->x-xspan+1]=lastHTMLatom;
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
      } //endif uvnitr table

      r=htmldata->textR;
      g=htmldata->textG;
      b=htmldata->textB;
      currentlink=IE_NULL;
      font=basefont;
      textrowsize=rowsize=0;
      xsum=0l;
      maxsum=0l;
     }
     istd=1;
     if(tag==TAG_TABLE)
      goto tag_table;
     break;

#else // ----------------------------nouzova interpretace tabulek:
     case TAG_TABLE:
     case TAG_SLASH_TABLE:
     case TAG_SLASH_CAPTION:
     case TAG_TR:

     style=0;
     currentlink=IE_NULL;
     r=htmldata->textR;
     g=htmldata->textG;
     b=htmldata->textB;
     font=basefont;
     goto br;

     case TAG_TD:
     case TAG_TH:

     style=0;
     currentlink=IE_NULL;
     r=htmldata->textR;
     g=htmldata->textG;
     b=htmldata->textB;
     font=basefont;
     break;
#endif

     case TAG_LI: // <LI> <DT>

     alignrow(x,y,orderedlist[listdepth]);
     if(x>left)
      y+=rowsize;
     if(orderedlist[listdepth]!=0 || left+LISTINDENT>right)
      x=left;
     else
      x=left+LISTINDENT;
     textrowsize=rowsize=fonty(font,style);

     if(orderedlist[listdepth]<1)
     {
      int type=0; //unordered list - currently decoration type= 1 or 2
      if(orderedlist[listdepth]<0)
       type=2+orderedlist[listdepth];

      HTMLatom.x=x-13+type;
      if(fixedfont)
       HTMLatom.y=y;
      else
       HTMLatom.y=y+rowsize/2-5+type;
      HTMLatom.xx=x-5-type;
      HTMLatom.yy=y+rowsize/2+5-type;
      addatom(&HTMLatom,"",0,LI,align,type,0,IE_NULL,0);
     }
     else
     {
      char number[10]; //ordered list - output item number instead of bullet
      sprintf(number,"%d.",orderedlist[listdepth]++);

      htmlfont(font,style);
      HTMLatom.x=x-strlen(number)*fontx(font,style,'0');
      HTMLatom.y=y;
      HTMLatom.xx=x;
      HTMLatom.yy=y+fonty(font,style);
      addatom(&HTMLatom,number,strlen(number),TEXT,BOTTOM,font,style,IE_NULL,0);
     }
     nownobr=1;
     lastspace=1;//mazat mezery
     align=BOTTOM; //seznam zarovnavat doleva
     if(xsum>maxsum)
      maxsum=xsum;
     xsum=0;
     break;

     case TAG_DD: //<DD>

     alignrow(x,y,orderedlist[listdepth]);
     y+=rowsize;
     textrowsize=rowsize=fonty(font,style);
     x=left+LISTINDENT;
     lastspace=1;
     break;

     case TAG_OL: //<OL> <MENU> <DL> <DIR>

     orderedlist[listdepth+1]=1; //ordered list, item no.=1
     goto list;

     case TAG_UL: //<UL>

     if(orderedlist[listdepth]!=-2)
      orderedlist[listdepth+1]=-2; //unordered list type 1
     else
      orderedlist[listdepth+1]=-1; //unordered list type 2
     list:
     {
      char flag=(x>left);
      int indent=LISTINDENT;

      if(orderedlist[listdepth+1]==1) //yes, it is ordered list
       indent*=2;

//      printf("[indent=%d]",indent);
      if(left+indent<right && listdepth<2*MAXTABLEDEPTH-1)
      {
       listedge[listdepth]=left;
       left+=indent;
       listdepth++;
       x=left;
//       printf("[listedge=%d,left=%d]",listedge[listdepth-1],left);
      }

      if(flag)
       goto br;
     }
     break;

     case TAG_SLASH_UL:
     case TAG_SLASH_OL:

     if(listdepth)
     {
      listdepth--;
      left=listedge[listdepth];
     }
     goto p;

     case TAG_INPUT: //<INPUT>
     case TAG_BUTTON: //<BUTTON>

     {
      int type=TEXT,size=10,checked=0;
      char value[IE_MAXLEN+1]="\0",name[80]="\0",*ptr;
      char notresize=0;

      //official extensions

      if(tag==TAG_BUTTON)
       type=SUBMIT;

      if(getvar("URI",&tagarg))
      {
       AnalyseURL(cache->URL,&url,currentframe); //(plne zneni...)
       strcpy(value,url.file);
      }

      if(getvar("USR",&tagarg))
      {
       AnalyseURL(cache->URL,&url,currentframe); //(plne zneni...)
       strcpy(value,url.user);
      }

      if(getvar("URL",&tagarg))
       strcpy(value,cache->URL);

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
       if(!strcmpi(tagarg,"SUBMIT"))    //tlacitko
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
        break; //not yet implemented !

//        type=BUTTON;
//        goto butt;
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

      if(getvar("CHECKED",&tagarg) || getvar("ACTIVE",&tagarg))
       checked=1;

      if(getvar("NAME",&tagarg))
       makestr(name,tagarg,79);

      //unsecure arachne extensions to <INPUT> tag.....................
      //allowed only for local or forced-html documents
      if(!strncmpi(cache->URL,"file",4) || !strncmpi(cache->URL,"mailto",4)
          || !strncmpi(cache->URL,"about",4) || !strncmpi(cache->URL,"gui",3)
          || forced_html)
      {
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
         checked=1;
       }

       if(getvar("ARACHNEVGA",&tagarg) && vgadetected) //for VALUE=...
       {
        strcpy(value,vgadetected);
       }

       if(getvar("ARACHNENOTCHECKED",&tagarg)) //FOR ARACHNECFGVALUE=...
       {
        if(!strstr(tagarg,value))
         checked=1;
       }

       if(getvar("ARACHNECFGHIDE",&tagarg)) //FOR ARACHNECFGVALUE=...
       {
        if(strstr(tagarg,value))
         value[0]='\0';
       }
      }//end unsecure extensions.......................................

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
       if(x+size>right && x>left && size<right-left && !pre && !nownobr)
       {
        alignrow(x,y,orderedlist[listdepth]);
        y+=rowsize;
        x=left;
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
        if(rowsize<ygap)
         rowsize=ygap;
        if(textrowsize<ygap)
         textrowsize=ygap;
       }
      }
      else
      {
       //neviditelna polozka
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
       if(rowsize<ygap)
        rowsize=ygap;
       if(textrowsize<ygap)
        textrowsize=ygap;
       currentbutton=lastHTMLatom;
       currentbuttony=y;
       xshift(&x,space(0));
       left=x;
       y+=2;
       break;
      }
      else
       currentbutton=IE_NULL;

      if(notresize)
       break;
     }//end block!

     {
      int oldright=right;
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
      oldxsum=xsum;
      closeatom(currentbutton,x-currentbuttonx,y+rowsize+5); //0=don't overwrite Y coordinate
      xsum=oldxsum;
      xshift(&x,space(0));
      if(currentbuttony!=y)
      {
       unsigned pushlast=lastHTMLatom;
       lastHTMLatom=currentbutton;
       alignrow(x,currentbuttony,orderedlist[listdepth]);
       lastHTMLatom=pushlast;
      }
      alignrow(x,y,orderedlist[listdepth]);
      rowsize+=5;
      currentbutton=IE_NULL;
      left-=space(0);
      if(x>right && !pre && !nownobr && !nobr)
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
       AnalyseURL(tagarg,&url,currentframe); //(plne zneni...)
       url2str(&url,text);

       //vyrobim si pointr na link, a od ted je vsechno link:
       addatom(&HTMLatom,text,strlen(text),FORM,align,target,method,IE_NULL,1);
       currentform=lastHTMLatom;
      }
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
      isscript=1;
      invisibletag=1;
     }
     break;

     case TAG_SCRIPT://<SCRIPT>
     isscript=1;
//     case TAG_HEAD: //<HEAD>
     invisibletag=1;
     break;

     case TAG_TITLE: //<TITLE>
     invisibletag=1;
     istitle=1;
     break;

     case TAG_SLASH_SCRIPT:
     case TAG_SLASH_NOFRAMES:
     isscript=0;
//     case TAG_SLASH_HEAD:
     invisibletag=0;
     break;

     case TAG_SLASH_TITLE:
     //accept title only for the main frame...
     if(istitle && !arachne.title[0] && !currentframe)
     {
      text[txtlen]='\0';
      MakeTitle(text);
      if(!printing)
       DrawTitle(0);
     }
     invisibletag=0;
     istitle=0;
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
       if(rowsize<rowspix+2)rowsize=rowspix+2;
       if(textrowsize<rowspix+2)textrowsize=rowspix+2;
       maxoption=0;
       addatom(&HTMLatom,&tmpeditor,sizeof(struct ib_editor),INPUT,align,SELECT,multiple,currentform,0);
       atomptr=(struct HTMLrecord *)ie_getswap(lastHTMLatom);
       currenttextarea=atomptr->ptr;
      }
      isselect=1; //flag: read contens of <SELECT>... </SELECT>!
      invisibletag=1;
      alreadyselected=0;
     }
     break;

     case TAG_SLASH_OPTION: //</OPTION>
     case TAG_SLASH_SELECT: //</SELECT>
     case TAG_OPTION:       //<OPTION>

     text[txtlen]='\0';
     if(isselect==2)         // <(last)OPTION> text <(this)OPTION|/SELECT>
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

     if (isselect==1 ) //after first <OPTION> or after </SELECT>
      isselect++;

     if(tag==TAG_SLASH_SELECT) //</SELECT> - end of select tag
     {
      int dx=maxoption+space(SYSFONT)+user_interface.scrollbarsize+7;
      int oldright=right;
      isselect=0;
      invisibletag=0;
      if(x+dx>right)
      {
       alignrow(x,y,orderedlist[listdepth]);
       y+=rowsize;
       x=left;
       lastspace=1;//mazat mezery
       textrowsize=rowsize=atom2nextline(x,y,lastHTMLatom);
      }
      closeatom(lastHTMLatom,dx,0); //0=don't overwrite Y coordinate
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
      if(x>=oldright && !pre && !nownobr && !nobr)
       goto br;
      else
       xshift(&x,space(0));
     }
     else
     if(tag==TAG_SLASH_OPTION) //</OPTION> - end of option tag
      isselect=1;
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

      appendline(currenttextarea,text,(text[0]=='1'));
     }//endif
     break;

     case TAG_TEXTAREA: //<TEXTAREA>

     {
     char name[80]="\0",active=0;
     int rows=5,cols=20;
     int lines,rowspix,rv;

     if(getvar("ROWS",&tagarg))
     {
      rows=atoi(tagarg);
     }

     if(getvar("ARACHNEROWS",&tagarg))
     {
      //recalculate for current system font:
      rows=14*rows/fonty(SYSFONT,0);
      rows-=(540-frame->scroll.ysize+fonty(SYSFONT,0)/2)/fonty(SYSFONT,0);
     }

     if(rows<1)rows=1;

     if(getvar("COLS",&tagarg))
     {
      cols=atoi(tagarg);
      if(cols<2)cols=2;
     }

     if(getvar("NAME",&tagarg))
      makestr(name,tagarg,79);

#ifdef POSIX
     lines=10000;
#else
     lines=(int)(farcoreleft()/16); //160000 -> 10000, 80000->5000
     if(lines>8000)lines=8000;
     if(lines<256)lines=256;
#endif

     HTMLatom.x=x;
     HTMLatom.y=y;
     x+=user_interface.scrollbarsize+5+space(SYSFONT)*cols;
     rowspix=user_interface.scrollbarsize+5+fonty(SYSFONT,0)*rows;
     HTMLatom.xx=x;
     HTMLatom.yy=y+rowspix;
     if(rowsize<rowspix+2)rowsize=rowspix+2;
     if(x>right)right=x;
     if(right>rightedge)rightedge=right;

     tmpeditor.cols=cols;
     if(getvar("ARACHNEEDITOR",&tagarg))
     {
      if(tagarg[0])
       strcpy(tmpeditor.filename,tagarg);
      else
       strcpy(tmpeditor.filename,LASTlocname);
      rv=ie_openf_lim(&tmpeditor,CONTEXT_HTML,lines);
      if(name[0])
       strcpy(tmpeditor.filename,name);
     }
     else
     {
      rv=InitInput(&tmpeditor,name,NULL,lines,CONTEXT_HTML);
      istextarea=1; //flag: read contens of <TEXTAREA>... </TEXTAREA>!
     }

     if(getvar("WRAP",&tagarg) && toupper(tagarg[0])!='N')
      tmpeditor.wordwrap=1;

     if(getvar("ACTIVE",&tagarg))
      active=1;

     if(rv==1)
     {
      addatom(&HTMLatom,&tmpeditor,sizeof(struct ib_editor),INPUT,align,TEXTAREA,active,currentform,0);
      atomptr=(struct HTMLrecord *)ie_getswap(lastHTMLatom);
      currenttextarea=atomptr->ptr;
     }
     invisibletag=1;
     }
     break;

     case TAG_SLASH_TEXTAREA:

     invisibletag=0;
     text[txtlen]='\0';
     if(istextarea)
      appendline(currenttextarea,text,0);
     istextarea=0;
     break;

     case TAG_BODY: //<BODY>

     if(getvar("BACKGROUND",&tagarg) && tagarg[0])
     {
      ResetHtmlPage(htmldata,TEXT_HTML,0);
      AnalyseURL(tagarg,&url,currentframe); //(plne zneni...)
      url2str(&url,img->URL);
      init_picinfo(img);
      img->URL[URLSIZE-1]='\0';
      if(img->URL[0])
      {
       addatom(&HTMLatom,img,sizeof(struct picinfo),BACKGROUND,BOTTOM,0,0,IE_NULL,0);
       htmldata->backgroundptr=lastHTMLatom;
      }
      //printf("background image=%s\n",img->URL);
     }
     if(getvar("BGCOLOR",&tagarg))
     {
      try2readHTMLcolor(tagarg,&htmldata->backR,&htmldata->backG,&htmldata->backB);
     }

     if(getvar("TEXT",&tagarg))
     {
      try2readHTMLcolor(tagarg,&htmldata->textR,&htmldata->textG,&htmldata->textB);
     }
     else if (htmldata->backR<8 && htmldata->backG<8 && htmldata->backB<8 &&
              htmldata->backgroundptr==IE_NULL )
     {
      htmldata->textR=255;
      htmldata->textG=255;
      htmldata->textB=255;
     }


     if(getvar("LINK",&tagarg))
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
      left=leftedge=leftstack[0]=leftedgestack[0]=x=frame->marginwidth=atoi(tagarg);
      right=rightedge=rightstack[0]=rightedgestack[0]=frame->scroll.xsize-frame->marginwidth-FUZZYPIX;
     }
     else
      x=left;

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
     r=htmldata->textR;
     g=htmldata->textG;
     b=htmldata->textB;
     break;

     case TAG_BASE: //<BASE>

     if(getvar("HREF",&tagarg))
     {
      add2history(frame->cacheitem.URL);
      GLOBAL.nothot=1;

      strcpy(text,tagarg);
      entity2str(text);
      AnalyseURL(text,&url,IGNORE_PARENT_FRAME);

      //reset BASE url:
      if(!currentframe)
       memcpy(&baseURL,&url,sizeof(struct Url));

      url2str(&url,frame->cacheitem.URL);
      if(!currentframe)
      {
       SetInputAtom(&URLprompt,frame->cacheitem.URL);
       if(!printing)
        DrawTitle(0);
      }
     }

     basetarget=findtarget(currentframe);

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


     case TAG_FRAMESET: //<FRAMESET>

     if(user_interface.frames && arachne.framescount<MAXFRAMES-1) //[0...6]
     {
      char framewantborder=UNDEFINED_FRAMEBORDER;
      oldactive=0; //deactivate any possibly active frames
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
       strcpy(text,tagarg);
       addframeset(0,&emptyframeset,framewantborder);
      }
      else
      if(getvar("COLS",&tagarg))
      {
       strcpy(text,tagarg);
       addframeset(1,&emptyframeset,framewantborder);
      }
     }

     if(!alreadyframe && emptyframeset==-1)
     {
      char *msg=MSG_FRAMES;
      fixrowsize(font,style);
      alignrow(x,y,orderedlist[listdepth]);
      HTMLatom.x=x;
      HTMLatom.y=y;
      HTMLatom.xx=right;
      y+=fonty(6,BOLD);
      HTMLatom.yy=y;
      addatom(&HTMLatom,msg,strlen(msg),TEXT,BOTTOM,6,BOLD,IE_NULL,0);
      alreadyframe=1;
      goto br;
     }

     break;

     case TAG_FRAME: //<FRAME>
     if(user_interface.frames && emptyframeset!=-1)
     {
      char newframe_target;
      struct HTMLframe *frame;

      if(!getvar("SRC",&tagarg))
       tagarg="NUL";
      AnalyseURL(tagarg,&url,currentframe); //(plne zneni...)
      url2str(&url,text);

      newframe_target=emptyframeset;
      frame=&htmlframe[newframe_target];

      emptyframeset=frame->next;
      frame->next=htmlframe[previousframe].next;
      htmlframe[previousframe].next=newframe_target;
      previousframe=newframe_target;


      if(newframe_target<MAXFRAMES-1 && newframe_target>arachne.framescount)
       arachne.framescount=newframe_target;

      text[URLSIZE-1]='\0';
      strcpy(frame->cacheitem.URL,text);
      if(getvar("NAME",&tagarg))
      {
       strcpy(text,tagarg);
       text[FRAMENAMESIZE-1]='\0';
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
      ResetHtmlPage(&tmpframedata[newframe_target],TEXT_HTML,1);

      frame->posX=0;
      frame->posY=0l;
     }
     else
     {
      unsigned currentlink;

      if(getvar("SRC",&tagarg))
      {
       HTMLatom.x=x;
       HTMLatom.y=y;
       HTMLatom.xx=x;
       HTMLatom.yy=y+rowsize;
       //vlozit link:
       AnalyseURL(tagarg,&url,currentframe); //(plne zneni...)
       url2str(&url,text);

       //vyrobim si pointr na link, a od ted je vsechno link:
       addatom(&HTMLatom,text,strlen(text),HREF,BOTTOM,0,0,IE_NULL,1);
       currentlink=lastHTMLatom;

       getvar("NAME",&tagarg);
       strcat(text," (");
       strcat(text,tagarg);
       strcat(text,")");

       pushfont(font,style,&fontstack);
       r=htmldata->linkR;
       g=htmldata->linkG;
       b=htmldata->linkB;
       img->size_y=60;
       img->size_x=60;
       strcpy(img->filename,"HTM.IKN");
       img->URL[0]='\0';
       HTMLatom.x=x;
       HTMLatom.y=y;
       x+=img->size_x;
       HTMLatom.xx=x;
       HTMLatom.yy=y+img->size_y;
       addatom(&HTMLatom,img,sizeof(struct picinfo),IMG,BOTTOM,0,0,currentlink,0);
       HTMLatom.x=x+FUZZYPIX;
       HTMLatom.xx=right;
       HTMLatom.yy=y+fonty(3,UNDERLINE);
       addatom(&HTMLatom,text,strlen(text),TEXT,BOTTOM,3,UNDERLINE,currentlink,0);
       y+=img->size_y;
       if(!popfont(&font,&style,&fontstack))
       {
        r=htmldata->textR;
        g=htmldata->textG;
        b=htmldata->textB;
       }
      }
      goto br;
     }

     case TAG_META:
     METAtag();
     break;

     // * * * * * * * * * * implementation of CSIM - client side imagemaps

     case TAG_MAP:
     case TAG_SLASH_MAP:

     HTMLatom.x=x;
     HTMLatom.y=y;
     HTMLatom.xx=x;
     HTMLatom.yy=y+rowsize;

     if(!getvar("NAME",&tagarg))
      tagarg[0]='\0';

     addatom(&HTMLatom,tagarg,strlen(tagarg),MAP,align,0,0,IE_NULL,1);
     break;

     case TAG_AREA:
     USEMAParea(basetarget);
     break;

     // * * * * * * * * * * * * * * * * * * * * * * * * * * * * end of CSIM

     case TAG_EMBED:

     if(getvar("SRC",&tagarg) && !strncmpi(cache->URL,"file",4))
     {
      AnalyseURL(tagarg,&url,currentframe); //(plne zneni...)
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
      GLOBAL.del=2;

     if(getvar("TARGET",&tagarg))
      configvariable(&ARACHNEcfg,"FTPpath",tagarg);

     if(getvar("PRINT",&tagarg) && printing)
     {
      if(toupper(*tagarg)=='N')
      {
       isscript=1;
       invisibletag=1;
      }
      else
      {
       isscript=0;
       invisibletag=0;
      }
     }

    }//end switch tag analysing

    lasttag=tag;
    tag=0; //current tag=0
    HTMLatom.x=x;
    HTMLatom.y=y;
    txtlen=0;
    lastspcpos=0;
    lastspcx=x;
   }
   // <------------------------<---------<------zpracovani parametru HTML tagu
   else if(!comment && (buf[i]>=' ' || buf[i]<0))
   {
    //hehehe.... jestli nasledujicim radku nerozumite, nejste sami :-))))
    //autor programu pro vas ma plne pochopeni...

    if(!param && buf[i]!=' ' && taglen<sizeof(tagname))
    {
     tagname[taglen++]=buf[i];
     if(taglen==3 && !strncmp(tagname,"!--",3))
     {
      comment=1;
      nolt=1;
     }
    }
    else if(buf[i]==' ' && vallen && !uvozovky && argument)
    {
     if(param)
     {
      putvarvalue(tagargptr,vallen);
      argument=0;
     }
     vallen=0;
    }
    else if((buf[i]=='\"' || buf[i]=='\'' && (!uvozovky || apostrof)) && (!vallen || uvozovky))
    {
     if(argument && uvozovky)      //kvuli ' XXX="" '
      tagargptr[vallen++]='\0';

     uvozovky=1-uvozovky;
     if(uvozovky && buf[i]=='\'')
      apostrof=1;
     else
      apostrof=0;
    }
    else if(buf[i]=='=' && !uvozovky && !argument)
    {
     putvarname(tagargptr,vallen);
     vallen=0;
     argument=1;
    }
    else if(vallen<BUF/2)
    {
     if(buf[i]!=' ' || uvozovky)
     {
      if(!argument && argspc && vallen)
      {
       putvarname(tagargptr,vallen);
       putvarvalue(tagargptr,0);
       vallen=0;
      }
      tagargptr[vallen++]=buf[i];
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
   {
    pom[1]=pom[0];
    pom[0]=buf[i];
    if(buf[i]=='<')
     nolt=0;
   }

  if(memory_overflow)
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
 frame->scroll.total_y=y+rowsize+FUZZYPIX;
 HTMLatom.yy=frame->scroll.total_y;
 if(txtlen)
 {
  text[txtlen]='\0';
  fixrowsize(font,style);
  addatom(&HTMLatom,text,txtlen,TEXT,align,font,style,currentlink,0);
 }
 alignrow(x,y,orderedlist[listdepth]);

 //Arachne formatted document?
 if(noresize)
  RENDER.willadjusttables=0;

 if(frame->allowscrolling)
   frame->scroll.yvisible=1;
 else
   frame->scroll.yvisible=0;

 //kreslit rolovatko ?
 if(frame->scroll.total_x>frame->scroll.xsize+FUZZYPIX &&
    frame->scroll.ymax>user_interface.scrollbarsize &&
    frame->allowscrolling && !printing)
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

 closeHTML(cache,source);

 //kdyz budu muset delat tabulky, tak se ani nezdrzovat:
 //if download was aborted, document will be redrawn later
 if((  (GLOBAL.validtables!=TABLES_UNKNOWN || !RENDER.willadjusttables)
     || source==HTTP_HTML)
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
     !printing
     &&
     (
      htmlframe[currentframe].next==-1 || source==HTTP_HTML
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
        prevHTMLtable!=IE_NULL
       )
      )
     )
    )
  {
   if(GLOBAL.needrender && !user_interface.quickanddirty)
    redrawHTML(REDRAW_NO_MESSAGE,REDRAW_SCREEN);
   else
    redrawHTML(REDRAW_NO_MESSAGE,REDRAW_CREATE_VIRTUAL);
  }
  else if(!arachne.framescount)
   ScrollDraw(&frame->scroll,frame->posX,frame->posY);
 }//endif redraw


 activeframe=arachne.target;

 //treat frames
 if(arachne.framescount)
 {
  XSWAP dummy1;
  unsigned dummy2;

  //clear ugly remains of previous screen:
  if(!currentframe)
   redrawHTML(REDRAW_NO_MESSAGE,REDRAW_SCREEN);

  if(htmlframe[currentframe].next!=-1)
  {
   do
   {
    kbhit();
    currentframe=htmlframe[currentframe].next;
   }
   while(htmlframe[currentframe].hidden && htmlframe[currentframe].next!=-1);

   AnalyseURL(htmlframe[currentframe].cacheitem.URL,&url,IGNORE_PARENT_FRAME);

   if(currentframe==arachne.target && forced_html)
    goto insertframe;


   if(SearchInCache(&url,&htmlframe[currentframe].cacheitem,&dummy1,&dummy2) ||
      source==HTTP_HTML && currentframe==arachne.target)
    goto insertframe;

   arachne.newframe=currentframe;    //load missing frames from...
  }
 }

 if(search4maps)
  LinkUSEMAPs();

 if(GLOBAL.validtables==TABLES_UNKNOWN && RENDER.willadjusttables)
  GLOBAL.validtables=TABLES_EXPAND;
 else
  GLOBAL.validtables=TABLES_FINISHED;
 percflag=0;

 return 1;
}



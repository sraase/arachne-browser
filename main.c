
/******************************************************************************

Wattcp  : Copyright (c)1990,1991,1992,1993,1999 Erick Engelke
X_LOPIF : Copyright (c)1990,1999 Zdenek Harovnik
Arachne : Copyright (c)1996,1997,1998,1999,2000 Michael Polak, Arachne Labs

ARACHNE WWW BROWSER (TM) is a trademark of Michael Polak, Arachne Labs (TM)

******************************************************************************/

#include "arachne.h"
#include "internet.h"
#include "main.h"
#include "xanimgif.h"
#include "gui.h"

// ========================================================================
// Arachne WWW browser main() function:
// ========================================================================

#ifdef CLEMTEST
int arachne_main(int argc, char **argv )
#else
int main(int argc, char **argv )
#endif
{
 struct Url url;
 struct HTTPrecord inlineimage;
 struct HTTPrecord *cacheitem;
 XSWAP update_redirection=IE_NULL,inlineimage_writeadr=IE_NULL;
 unsigned inlineimage_status;
 XSWAP currentimage;
 XSWAP *cacheitem_writeadr;
 unsigned *cacheitem_status;
 char found;
 char html_source=LOCAL_HTML,forced_html=0;
#ifndef NOTCPIP
 int closesock;
#endif
 long timer;
 char needredraw,imageismissing,ivegotallimages;
 int returnvalue=EXIT_TO_DOS;
 int lastseconds;
 int plugin=0;
 char error=0;
#ifndef NOTCPIP
 char isframe=0;
#endif

#ifndef POSIX
if(argc==1 || argv[1][0]=='-' && argv[1][1]=='s')
#endif
 printf(MSG_START,VER,beta,copyright);

Initialize_Arachne(argc,argv,&url);

if(arachne.scriptline!=0)
 goto ReadScriptLine;

//--------------------------------------------------------------------------
// I have got new URL - either main document (GLOBAL.target=0), inline frame
// (GLOBAL.target>0) or inline image (GLOBAL.isimage=1).
//--------------------------------------------------------------------------

IveGotNewUrl:

 GLOBAL.gotolocation=0;
 GLOBAL.abort=0;
 GLOBAL.allowdealloc=0;
 error=0;
 strcpy(Referer,htmlframe[activeframe].cacheitem.URL);
 if(!strncmpi(Referer,"file:",5))
  Referer[0]='\0';

 if(GLOBAL_justrestarted && arachne.framescount) // arachne -c, -r + frames
 {
  forced_html=RELOAD_HTML_FRAMES;
  if(!arachne.target)
   GLOBAL.nothot=1;
 }
 else
 {
  forced_html=0;
  if(GLOBAL.postdata && arachne.framescount)
  {
   char push=GLOBAL.postdata;
   GLOBAL.postdata=0;
   delete_children(arachne.target);
   GLOBAL.postdata=push;
  }
  free_children(arachne.target);
 }

 if(GLOBAL.isimage)   // do this for inline image
 {
  cacheitem=&inlineimage;
  cacheitem_status=&inlineimage_status;     //status of inline image
  cacheitem_writeadr=&inlineimage_writeadr; //update adr of inline image
 }
 else                 // do this for other objects
 {
  //pseudoprotocols - reload, find
  if(!strncmpi(GLOBAL.location,"reload:",7))
  {
   int l=strlen(&GLOBAL.location[7]);

   if(!l)
   {
    GLOBAL.reload=RELOAD_CURRENT_LOCATION;
    strcpy(GLOBAL.location,htmlframe[activeframe].cacheitem.URL);
    arachne.target=activeframe;
   }
   else
   {
    memmove(GLOBAL.location,&GLOBAL.location[7],l);
    GLOBAL.location[l]='\0';
    if(tcpip || strncmpi(GLOBAL.location,"http:",5))
     GLOBAL.reload=RELOAD_NEW_LOCATION;
   }
  }
  else
  if(!strncmpi(GLOBAL.location,"find:",5))
  {
   char *ptr=configvariable(&ARACHNEcfg,"SearchEngine",NULL);
   if(!ptr)
    ptr="http://www.altavista.com/cgi-bin/query?q=";
   cgiquery((unsigned char *)&GLOBAL.location[5],(unsigned char *)buf,1);
   strcpy(GLOBAL.location,ptr);
   strncat(GLOBAL.location,buf,URLSIZE-strlen(GLOBAL.location)-2);
   GLOBAL.location[URLSIZE-1]='\0';
  }
  else
  if(!strcmpi(GLOBAL.location,"arachne:addressbook"))
  {
   char *ptr=configvariable(&ARACHNEcfg,"Hotlist",NULL);
   if(ptr)
    sprintf(GLOBAL.location,"file:%s#mailto",ptr);
   else
    strcpy(GLOBAL.location,"file:hotlist.htm#mailto");
  }
  //endif pseudoprotocols

  currentimage=IE_NULL;
  GLOBAL.validtables=TABLES_UNKNOWN;
  GLOBAL.tabledepth=0;
  ie_killcontext(CONTEXT_TABLES);
  firstHTMLtable=IE_NULL;

  cacheitem=&htmlframe[arachne.target].cacheitem;
  //status of main displayed document
  cacheitem_status=&htmlframe[arachne.target].status;
  //update adr of main displayed document
  cacheitem_writeadr=&tmpframedata[arachne.target].writeadr;
  timer=time(NULL);
  if(GLOBAL.nowimages!=IMAGES_SEEKCACHE)
   pagetime=timer;
  needredraw=0;
  ivegotallimages=0;
  GLOBAL.source=0;
  GLOBAL.timeout=0;
  GLOBAL.backgroundimages=BACKGROUND_EMPTY;
  GLOBAL.clipdel=0;

  if(!arachne.target)
  {
   arachne.backtrace=0;
  }
  else if(!GLOBAL.nothot)
  {
   if(arachne.backtrace==MAXBACKTRACE-1)
    memmove(arachne.backtrace_target,arachne.backtrace_target+1,MAXBACKTRACE-1);
   arachne.backtrace_target[arachne.backtrace]=arachne.target;
   if(arachne.backtrace<MAXBACKTRACE)
    arachne.backtrace++;
  }
 }//endif not image

 //analyza toho, co mam vlastne delat,zpracovani URL, reset dokumentu...
 *cacheitem_writeadr=IE_NULL; //my NULL pointer
 AnalyseURL(GLOBAL.location,&url,IGNORE_PARENT_FRAME);

 //--------------------------------------------------------------------------
 if(!strcmpi(url.protocol,"arachne") && !GLOBAL.isimage)
 //--------------------------------------------------------------------------
 {
  switch(protocol_arachne(cacheitem,&url,&returnvalue))
  {
   case GOTO_IVEGOTNEWURL: goto IveGotNewUrl;
   case GOTO_END:          goto end;
   case GOTO_USEREND:      goto userend;
  }
 }

 //------------------------------------------------------------------------
 Retry:
 //------------------------------------------------------------------------

 //reset structure cacheitem
 memset(cacheitem,0,sizeof(struct HTTPrecord));

 //recognized URL is default URL:
 if(!GLOBAL.isimage && !arachne.target)
  memcpy(&baseURL,&url,sizeof(struct Url));

 cacheitem->handle=-1; //file Not EXISTs /not open  !!

 //---------------------------------------------------------- arachne:history
 if(!strcmpi(url.protocol,"arachne") && !strcmpi(url.file,"history"))
 {
  url2str(&url,cacheitem->URL);
  html_source=HISTORY_HTML;
  forced_html=1;
  GLOBAL.nothot=1;
  *cacheitem_status=VIRTUAL;
  strcpy(url.kotva,"current");
  goto Render;
 }

 html_source=LOCAL_HTML;

 //------------------------------------------------------------------------
 found=SearchInCache(&url,cacheitem,cacheitem_writeadr,cacheitem_status);
 //------------------------------------------------------------------------

 //redirection flag was just used to expire non-static cache items....
 GLOBAL.redirection=0;

 if(found || *cacheitem_status!=REMOTE)
 //it's local file
 {
  if(GLOBAL.isimage)
   goto Search4Image;
 }
 else
 //------------------------------------------------------------------------
 //not found on disk, we will try to get document from Internet:
 //------------------------------------------------------------------------
 {
  //---------------------------------------------------------------- telnet:
  if(!strcmpi(url.protocol,"telnet"))
  //------------------------------------------------------------------------
  {
   plugin=externalprotocol(url.protocol,text);

   if(plugin)
   {
    external:
    make_cmd(text,buf,
             htmlframe[arachne.target].cacheitem.URL,
             url.host, url.file, text, "NUL");
   }
   else
    sprintf(buf,"telnet %s\n",url.host);

#ifdef POSIX
   system(buf);
   goto Wait4Orders;
#else
   closebat(buf,RESTART_REDRAW);
   returnvalue=willexecute(buf);
   x_grf_mod(3);
   goto end;
#endif
  }
  //------------------------------------------------------------------------
  else if(!tcpip) //other protocols are available only online...
  //------------------------------------------------------------------------
  {
#ifndef NOTCPIP
   if(httpstub && (!strcmpi(url.protocol,"http") || !strcmpi(url.protocol,"ftp")))
    goto proxy;
#endif

   if(GLOBAL.isimage)
   {
    *cacheitem_writeadr=Write2Cache(&url,cacheitem,1,1);
    goto errimage;
   }
   else
   {
    char str[80];

#ifndef NOTCPIP
    if(user_interface.autodial && !httpstub && !GLOBAL_justrestarted)
    {
     add2history(GLOBAL.location);
     strcpy(GLOBAL.location,"arachne:dialer");
     goto IveGotNewUrl;
    }
#endif

    sprintf(str,"external/%s",url.protocol);
    plugin=externalprotocol(url.protocol,text);
    if(plugin)
     goto external;
    else
     sprintf(htmlframe[0].cacheitem.locname,"%s%serr_net.ah",sharepath,GUIPATH);

    *cacheitem_writeadr=IE_NULL;
    forced_html=1;
    arachne.target=0;
    error=1;
    goto Render;
   }
  }
  else
#ifndef NOTCPIP
  //------------------------------------------------------------------ http:
  if(!strcmpi(url.protocol,"http"))
  //------------------------------------------------------------------------
  {

   proxy:
   if(!httpstub)
   {
    html_source=HTTP_HTML;
    if(user_interface.multitasking==MULTI_SAFE)
     FinishBackground(BG_FINISH);
   }
   else
   {
    if(GLOBAL.isimage)
     html_source=HTTP_HTML;
    else
     html_source=LOCAL_HTML;
   }

   *cacheitem_writeadr=Write2Cache(&url,cacheitem,1,1);
   //http 2 cache
   if(openhttp(&url,cacheitem))
   {

    if(GLOBAL.abort)
     goto Abort;
    UpdateInCache(*cacheitem_writeadr,cacheitem);
    if(update_redirection!=IE_NULL)
    {
     UpdateFilenameInCache(update_redirection, cacheitem);
     update_redirection=IE_NULL;
    }
    *cacheitem_status=REMOTE;
   }
   else //error - bad host name, connection reset, etc.
   {
    Abort:

    if(GLOBAL.redirection)
    {
     if((GLOBAL.isimage || arachne.target) && update_redirection==IE_NULL)
     {
      strcpy(cacheitem->locname,"NUL");
      cacheitem->rawname[0]='\0';
      UpdateInCache(*cacheitem_writeadr,cacheitem);
      update_redirection=*cacheitem_writeadr;
//      printf("[image redirection, update=%u]",update_redirection);

     }
     else //....because guys from Microsoft will redirect us back to this URL!
      DeleteFromCache(*cacheitem_writeadr);

     goto IveGotNewUrl;
    }
    else
     DeleteFromCache(*cacheitem_writeadr);

    if(!GLOBAL.isimage)
    {
     if(AUTHENTICATION->flag==AUTH_REQUIRED)
     {
      sprintf(cacheitem->locname,"%s%slogin.ah",sharepath,GUIPATH);
      forced_html=1;
      html_source=LOCAL_HTML;
      goto Render;
     }

     if(GLOBAL.abort)
      goto AbortRedrawAndMsg;
     else
     if(GLOBAL.gotolocation)
     {
      isframe=0;
      GLOBAL.isimage=0;
      goto IveGotNewUrl;
     }

     sprintf(htmlframe[0].cacheitem.locname,"%s%serr_open.ah",sharepath,GUIPATH);
     forced_html=1;
     html_source=LOCAL_HTML;
     arachne.target=0;
     error=1;
     goto Render;
    }
    else
      goto errimage;
   }

  }
  else
#endif //NOTCPIP
  switch(protocol_nohttp(cacheitem,&url,cacheitem_status,cacheitem_writeadr))
  {
   case GOTO_IVEGOTNEWURL: goto IveGotNewUrl;
   case GOTO_PROXY:        goto proxy;
   case GOTO_ABORT:        goto Abort;
   case GOTO_READSCRIPT:   goto ReadScriptLine;
   case GOTO_EXTERNAL:     goto external;
   case GOTO_TRYPLUGIN:    goto tryplugin;

   case GOTO_ERROR:
   arachne.target=0;
   error=1;
   forced_html=1;
   goto Render;

   case GOTO_LOCAL_HTML:
   html_source=LOCAL_HTML;
   forced_html=1;
   goto Render;

   case UNKNOWN_PROTOCOL:
   if (!GLOBAL.isimage)
   {
    plugin=externalprotocol(url.protocol,text);
    if(plugin)
     goto external;

    sprintf(htmlframe[0].cacheitem.locname,"%s%serr_url.ah",sharepath,GUIPATH);
    arachne.target=0;
    error=1;
    forced_html=1;
    goto Render;
   }
   else
   {
    errimage:

    outs(MSG_ERRIMG);
    strcpy(inlineimage.locname,"NUL");
    inlineimage.rawname[0]='\0';
    UpdateInCache(inlineimage_writeadr,&inlineimage);

    if(GLOBAL.abort)
     goto AbortRedrawAndMsg;
    else
    if(GLOBAL.gotolocation)
     goto IveGotNewUrl;
    else
     goto Search4Image;
   }
  }//end switch
 }//endif online

 //...it will continue to rendering page naturaly if all is ok...

 //------------------------------------------------------------------------
 //Download inline image
 //------------------------------------------------------------------------


#ifndef NOTCPIP
// if((arachne.newframe>0 || arachne.target>0)&& html_source==HTTP_HTML)
 if((arachne.target || isframe) && html_source==HTTP_HTML && !GLOBAL.isimage)
 {
 // html_source will not be set to HTTP_HTML when we are offline
  Download(cacheitem);
  closehttp(cacheitem);
  html_source=LOCAL_HTML;
 }

 if(GLOBAL.isimage && html_source==HTTP_HTML)
 {
  if(update_redirection!=IE_NULL)
  {
   UpdateFilenameInCache(update_redirection, &inlineimage);
   update_redirection=IE_NULL;
  }

  if(!httpstub)
  {
   if(GoBackground(&inlineimage))
   {
    if(GLOBAL.imagevisible)
     needredraw=1; //viditelny obrazek BUDE potreba prekreslit
    goto Search4Image;
   }

   Download(&inlineimage);
   closehttp(&inlineimage);
  }
/*  if(GLOBAL.backgroundimages!=BACKGROUND_EMPTY)
   goto Search4Image;
*/
  if(GLOBAL.abort)
  {
   unlink(inlineimage.locname);
   FinishBackground(BG_ABORT);
   goto AbortRedrawAndMsg;
  }
  else
  if(GLOBAL.gotolocation)
  {
   unlink(inlineimage.locname);
   FinishBackground(BG_ABORT);
   goto IveGotNewUrl;
  }

  if(!GLOBAL.imagevisible)
   goto Search4Image;
  else
  {
   //obrazky stazene rychleji nez za 20 sekund neprekreslovat!
   needredraw=1; //viditelny obrazek BUDE potreba prekreslit
   if(time(NULL)-timer<user_interface.refresh)
    goto Search4Image;
  }

  FinishBackground(BG_FINISH);

  if(!GLOBAL.needrender)
  {
   redrawHTML(REDRAW_NO_MESSAGE,REDRAW_SCREEN);
   //timer je nulovan na zacatku stahovani a pak po kazdem prekresleni
   timer=time(NULL);
   needredraw=0; //prave jsem prekreslil
   goto Search4Image;
  }

  //we are rendering HTML again, but we read it from disk:
  html_source=LOCAL_HTML;
 }//endif is image
#endif

 //----------------------------------------------------------- plugin?
 tryplugin:
 //------------------------------------------------------------------------
//. Arachne does not supports jpeg, so we have to rely on plugin

 if(!GLOBAL.isimage)
 {
  char command[IE_MAXLEN+1]; //misto na stacku by byt melo, viz RenderHTML..
  char ext[5];
  char str[120];
  char weird=0;

  if(!strstr(cacheitem->mime,"???")) //!=???
  {
   get_extension(cacheitem->mime, ext);
   if(!strcmpi(ext,"ASF")) //script hack
   {
     copy(cacheitem->locname,history.filename);
     ie_clearf(&history,0);
     if(ie_openf_lim(&history,CONTEXT_SYSTEM,256)==1); //historie - max. 256 radku
     {
      ie_insline(&history,0,"");
      arachne.scriptline=1;
      goto ReadScriptLine;
     }
   }//end if script
   weird=(strstr("HTM TXT",ext)==NULL);
  }

  plugin=call_plugin(cacheitem->mime, command,ext);

#ifndef NOTCPIP
  //download on background:
  if((weird || plugin || GLOBAL.backgr) && html_source==HTTP_HTML)
  {
   Download(cacheitem);
   closehttp(cacheitem);
   found=1;
   if(GLOBAL.backgr)
    goto Wait4Orders;

   if(GLOBAL.abort)
   {
#ifndef CLEMENTINE
    unlink("download.tmp");
    rename(cacheitem->locname,"download.tmp");
#endif
    goto PageDone;
   }
  }
#endif

  //download/save as prompt:
  if(!plugin && weird && !strstr(imageextensions,ext) && cacheitem->mime[0] && found)
  {
   strcpy(LASTlocname,cacheitem->locname);
   strcpy(cacheitem->locname,sharepath);
   strcat(cacheitem->locname,GUIPATH);
   if(*cacheitem_status==REMOTE)
    strcat(cacheitem->locname,"download.ah");
   else
    strcat(cacheitem->locname,"copy.ah");

   forced_html=1;
   html_source=LOCAL_HTML;
   goto Render;
  }

  if(plugin) //convertor or viewer
  {
   char *pom;
   int mode;
   char oldmime[STRINGSIZE];
#ifndef POSIX
   char mman[80]="\0";
#endif

   strcpy(oldmime,cacheitem->mime);
   //------------------------------------------------------------------------
   if(plugin==1) //conversion
   //------------------------------------------------------------------------
   {

    strcpy(str,cacheitem->locname);   //store original local filename
    *cacheitem_writeadr=Write2Cache(&url,cacheitem,0,1);
    strcpy(cacheitem->rawname,str);   //update original local filename
    pom=strrchr(cacheitem->locname,'.');
    if(pom)
     strcpy(&pom[1],ext);

    sprintf(cacheitem->mime,"file/.%s",ext);
    cacheitem->dynamic=1;

    mode=make_cmd(command,buf,cacheitem->URL,
                  url.host, url.file, str,cacheitem->locname);
    unlink(cacheitem->locname);
    UpdateInCache(*cacheitem_writeadr,cacheitem);
    *cacheitem_status=VIRTUAL;
    MemInfo(NORMAL);

    //------------------------------------------------------------------------
#ifndef POSIX
    //enough memory - conversion should be fast enough
    if(mode>0 && farcoreleft()>(long)((long)mode+10l)*1024)
#endif
    {
//     struct ffblk ff;
     sprintf(str,MSG_CONV,oldmime,ext,MSG_DELAY2,ctrlbreak);
     outs(str);

     savepick();

     //this would be safer, but too slow....
     //ie_savef(&history);
     //ie_savebin(&HTTPcache);

#ifndef POSIX
     if(strstr(strlwr(buf),"insight"))
     {
      tempinit(mman);
      strcat(mman,"$roura2.bat");
      unlink(mman);
     }
#endif     
//     printf("Executing command: %s\n",buf);
     system(buf);
#ifndef POSIX
     if(mman[0] && file_exists(mman))    
      system(mman);
#endif
     html_source=LOCAL_HTML;
     GLOBAL.postdata=0;
     GLOBAL.nowimages=IMAGES_SEEKCACHE;
     //if POSIX is defined, this will automaticaly proceed to Render: label
     goto Render;
    }
    if(url.kotva[0])
    {
     strcat(cacheitem->URL,"#");
     strcat(cacheitem->URL,url.kotva);
    }
    add2history(cacheitem->URL);
   }
   //------------------------------------------------------------------------
   else //viewer
   if(GLOBAL.nowimages!=IMAGES_SEEKCACHE || arachne.scriptline)
   //------------------------------------------------------------------------
   {
    sprintf(str,MSG_PLUGIN,cacheitem->mime,ctrlbreak);
    outs(str);
    mode=make_cmd(command,buf , cacheitem->URL,
                  url.host, url.file,cacheitem->locname, "NUL");
#ifdef POSIX
    system(buf);
    goto Wait4Orders;
#endif
   }
   //------------------------------------------------------------------------
   else //do not loop!!!
   //------------------------------------------------------------------------
   {
    SetInputAtom(&URLprompt,cacheitem->URL);
    MakeTitle(cacheitem->URL);
    DrawTitle(0);
    goto Wait4Orders;
   }
#ifndef POSIX
   //------------------------------------------------------------------------
   sprintf(str,MSG_CONV,oldmime,ext,MSG_DELAY1,ctrlbreak);
   outs(str);
   if(strstr(strlwr(buf),"insight"))
   {
    tempinit(mman);
    strcat(mman,"$roura2.bat");
    unlink(mman);
    sprintf(str,"\nif exist %s call %s",mman,mman);
    strcat(buf,str);
   }
   closebat(buf,(mode!=-1));
   returnvalue=willexecute(buf);
   if(mode==-1)
   {
    x_grf_mod(3);
   }
   else
   {
    gotoxy(1,8);
    mouseoff();
    if(mode==-2)
    {
     x_setfill(0,0);
     x_bar(htscrn_xtop,htscrn_ytop,htscrn_xtop+htscrn_xsize,htscrn_ysize+htscrn_ytop);
    }
   }
   goto end;
#endif //not POSIX
  }//endif plugin
 }//endif "not image"

//------------------------------------------------- render HTML (all frames)
Render:
//--------------------------------------------------------------------------

 GLOBAL.allowdealloc=0;         // Prevent GUIEVENT from doing some risky things
 GLOBAL.reload=NO_RELOAD;
 if(url.kotva[0]) // jump to anchor (#xyz) ?
  GLOBAL.norefresh=1;
 else
  GLOBAL.norefresh=0;

//--------------------------------------------------------------------------
//process all frames, write cacheitem to htmlframe[arachne.target]

 if(!renderHTML(html_source,forced_html,RENDER_SCREEN))
//--------------------------------------------------------------------------
 {
  //send error message/reload page to currently processed frame:
  cacheitem=&htmlframe[currentframe].cacheitem;

  //failed to load remote document - try again
  if(!error && *cacheitem_status!=LOCAL && html_source!=HISTORY_HTML && !(GLOBAL_justrestarted))
  {
   *cacheitem_writeadr=Write2Cache(&url,cacheitem,0,0);
   DeleteFromCache(*cacheitem_writeadr);
   error=1;
   goto Retry;
  }

  //failed to load local document:
  sprintf(buf,"%s%serr_load.ah",sharepath,GUIPATH);
  if(!strcmp(buf,htmlframe[currentframe].cacheitem.locname) || error)
  {
   error=1;
   MakeTitle(MSG_ERROR);
   DrawTitle(0);
  }
  else
  {
   strcpy(htmlframe[currentframe].cacheitem.locname,buf);
   forced_html=1;
   html_source=LOCAL_HTML;
   error=1;
   arachne.target=0;
   goto Render;
  }
 }
//--------------------------------------------------------------------------
 else // renderHTML succeeded
//--------------------------------------------------------------------------
 {
#ifndef NOTCPIP
  //timer is reset after each screen refresh / rendering
  timer=time(NULL);
#endif

  if(!GLOBAL.isimage && !GLOBAL.source)   //. source -->html_source
  {
   //pridani do historie
   if(url.kotva[0])
   {
    strcat(cacheitem->URL,"#");
    strcat(cacheitem->URL,url.kotva);
    Goto_A_NAME(url.kotva);
    GLOBAL.abort=0;
   }
   add2history(cacheitem->URL);
  }

  needredraw=0;       //prave jsem prekreslil

  if(!arachne.title[0] && !arachne.target)
  {
   MakeTitle(cacheitem->URL);
   DrawTitle(0);
  }
 }//endif success

 GLOBAL.nothot=0;
 GLOBAL.postdata=0;
 GLOBAL.reload=NO_RELOAD;

//-------------------------------------------------------------------------
//adjusting tables BEFORE loading images - good idea on faster PCs!
//-------------------------------------------------------------------------

 if(!GLOBAL.isimage &&
    !error &&
    !user_interface.quickanddirty &&
    !GLOBAL.needrender &&
    GLOBAL.validtables==TABLES_EXPAND)
 {
  //printf("error=%d, validtables=%d\n",error, GLOBAL.validtables);
  //while redrawing page we are always reading it from disk
  html_source=LOCAL_HTML;
  //because we are just redrawiing, so we do not modify the history list...
  GLOBAL.nothot=1;
  goto Render;
 }


//-------------------------------------------------------------------------
Search4Image:
//-------------------------------------------------------------------------


 if(update_redirection!=IE_NULL)
 {
  UpdateFilenameInCache(update_redirection, cacheitem);
  update_redirection=IE_NULL;
 }

 //load missing frames
 if(!GLOBAL.abort &&  !GLOBAL.gotolocation && !error &&
    arachne.framescount && arachne.newframe>0)
 {
  XSWAP dummy1;
  unsigned dummy2;
  struct HTTPrecord HTTPdoc;

  while(arachne.newframe<=arachne.framescount && arachne.newframe>0)
  {
   AnalyseURL(htmlframe[arachne.newframe].cacheitem.URL,&url,IGNORE_PARENT_FRAME);
   if(!SearchInCache(&url,&HTTPdoc,&dummy1,&dummy2))
   {
    strcpy(GLOBAL.location,HTTPdoc.URL);
    arachne.target=arachne.newframe;
    GLOBAL.nothot=1;
    GLOBAL.needrender=1;
    GLOBAL.validtables=TABLES_UNKNOWN;
#ifndef NOTCPIP
    isframe=1;
#endif
    goto IveGotNewUrl;
   }
   kbhit();
   arachne.newframe=htmlframe[arachne.newframe].next;
  }
 }
 arachne.newframe=0;

 //search for inline images ?
 if(!GLOBAL.abort &&
    !GLOBAL.source &&
    !cgamode &&
    !GLOBAL.gotolocation &&
    !ignoreimages &&
    !error &&
    !ivegotallimages)
   imageismissing=NeedImage(FIND_MISSING_IMAGE,currentimage);
 else
 {
  imageismissing=0;
  ivegotallimages=1;
 }

//-------------------------------------------------------------------------
 if(imageismissing)   //we need to load missing image.....
//-------------------------------------------------------------------------
 {
  if(GLOBAL.needrender)
   GLOBAL.validtables=TABLES_UNKNOWN;
#ifndef NOTCPIP
  isframe=0;
#endif
  goto IveGotNewUrl;
 }
//-------------------------------------------------------------------------
 else //we don't need to load missing image.
//-------------------------------------------------------------------------
 {
#ifndef NOTCPIP
  //just to be sure...
  FinishBackground(BG_FINISH);
#endif

  //now let's handle special situations if they occured....
#ifndef POSIX
  if(GLOBAL.willexecute) //something to execute ?
  {
   returnvalue=GLOBAL.willexecute;
   goto end;
  }
  else 
#endif
  if(GLOBAL.gotolocation) //somewhere to go ?
   goto IveGotNewUrl;

  AbortRedrawAndMsg:

  //it was last image, and we still need to redraw....
  if((GLOBAL.isimage || needredraw)&& GLOBAL.needrender)
  {
   GLOBAL.isimage=0;

   //when redrawing images, we always load document from disk...
   html_source=LOCAL_HTML;
   GLOBAL.nothot=1;
   GLOBAL.validtables=TABLES_UNKNOWN;
   goto Render;
  }
  else if(needredraw && GLOBAL.validtables==TABLES_FINISHED)
   redrawHTML(REDRAW_NO_MESSAGE,REDRAW_CREATE_VIRTUAL);
  GLOBAL.isimage=0;
 }//end if now images are missing.

 if(GLOBAL.validtables==TABLES_EXPAND)
 {
  //while redrawing page we are always reading it from disk
  html_source=LOCAL_HTML;
  //because we are just redrawiing, so we do not modify the history list...
  GLOBAL.nothot=1;
  goto Render;
 }

//-------------------------------------------------------------------------
PageDone:
//-------------------------------------------------------------------------

 pagetime=time(NULL)-pagetime; //. Much time we have used to do this page
 if(GLOBAL.timeout)
  GLOBAL.secondsleft-=(int)pagetime;

 if(memory_overflow)
  outs(MSG_NOTMEM);
 else if(GLOBAL.abort)
 {
  redrawHTML(REDRAW_NO_MESSAGE,REDRAW_CREATE_VIRTUAL);
  outs(MSG_ABORT);
 }
 else
 {
  char dummy[128];
  sprintf(dummy,MSG_DOCDON,copyright,pagetime/60,(int)(pagetime % 60));
  outs(dummy);
  //defaultmsg();
 }

 if(GLOBAL.activate_textarea)
 {
  ImouseSet( mousex, mousey);
  lmouse=0;
  GUIEVENT(0,1); //emulate mouse click...
 }

//-------------------------------------------------------------------------
Wait4Orders:
//-------------------------------------------------------------------------
#ifndef POSIX
 updtdiskspace(cacheitem->locname);
#endif
 MemInfo(NORMAL);
 GLOBAL.abort=0;
 GLOBAL.needrender=0;
 GLOBAL.nowimages=IMAGES_NOTNOW;
 GLOBAL.isimage=0;
 GLOBAL.reload=0;
 GLOBAL.redirection=0;
 GLOBAL.backgr=0;
 GLOBAL.allowdealloc=1;
#ifndef NOTCPIP
 isframe=0;
#endif
 lastseconds=-1;
 arachne.target=0;             //A HREF by default goes to main frame!
 if(tcpip && !strcmpi(url.protocol,"http"))
  ie_savebin(&HTTPcache);
 if(user_interface.logoiddle)
  xChLogo('0');


//-------------------------------------------------------------------------
ReadScriptLine:
//-------------------------------------------------------------------------

 if(arachne.scriptline)
 {

  if(arachne.scriptline>=history.lines)
   arachne.scriptline=0;
  else
  {
   if(GLOBAL.timeout && tcpip)
   {
    GLOBAL.reload=RELOAD_NEW_LOCATION;
    goto IveGotNewUrl;
   }

   strcpy(GLOBAL.location,ie_getline(&history,arachne.scriptline));
   arachne.history=arachne.scriptline++;
   goto IveGotNewUrl;
  }
 }

//-------------------------------------------------------------------------
//the main loop, where browser remains most of the time:
//-------------------------------------------------------------------------

 //.  mouse, keyboard, animation, timeout
 while(GLOBAL.abort!=ABORT_PROGRAM)
 {
#ifdef XANIMGIF
  if(!redraw && !htmlpulldown && g_NumAnim > 0)  // something to animate
   XAnimateGifs();
#endif

#ifdef JAVASCRIPT
  JSchecktimeouts(); //JavaScript timeouts will be handled there...
#endif

#ifdef POSIX
//  usleep(100); //We don't need all CPU time...
#endif

#ifndef NOTCPIP
#ifndef POSIX
  if(tcpip)
   tcp_tick(NULL);

  if(html_source==HTTP_HTML)
  {
   if(closing[0])
   {
    closesock=0;
    sock_tick( sock[0], &status ); //TCP/IP close ?
   }
   if(closing[1])
   {
    closesock=1;
    sock_tick( sock[1], &status ); //TCP/IP close ?
   }
  }
#endif
#endif

  GUITICK();    // Check mouse click and keyboard pressing

  if(GLOBAL.timeout && GLOBAL.secondsleft>=0 && GLOBAL.secondsleft!=lastseconds)
  {
   char str[128];

   sprintf(str,MSG_REFRSH,GLOBAL.location,GLOBAL.secondsleft,MSG_ESC);
   outs(str);
   lastseconds=GLOBAL.secondsleft;
//   printf("[%d]",GLOBAL.secondsleft);
  }

  clock_and_timer(NULL);

  if(GLOBAL.gotolocation || GLOBAL.timeout && GLOBAL.secondsleft<=0)
  {
//   printf("[%d]",GLOBAL.secondsleft);
   if(!GLOBAL.source)
   {
    //update x,y of document:
    int fr=0;
    do
    {
     cacheitem=(struct HTTPrecord *)ie_getswap(tmpframedata[fr].writeadr);
     if(cacheitem)
     {
      cacheitem->x=htmlframe[fr].posX;
      cacheitem->y=htmlframe[fr].posY;
      cacheitem->knowsize=htmlframe[fr].cacheitem.knowsize;
      cacheitem->size=htmlframe[fr].cacheitem.size;
      swapmod=1;
     }
    }
    while(fr++<arachne.framescount);
   }

   if(GLOBAL.willexecute)//!!!!! important ?! e-mail composition.
   {
    returnvalue=GLOBAL.willexecute;
    goto end;
   }

   if(GLOBAL.reload==RELOAD_CURRENT_LOCATION) // not RELOAD_NEW_LOCATION...
   {
    strcpy(GLOBAL.location,htmlframe[activeframe].cacheitem.URL);
    arachne.target=activeframe;
   }

   if(GLOBAL.timeout && tcpip)
    GLOBAL.reload=RELOAD_NEW_LOCATION;

   if(GLOBAL.timeout)
    arachne.target=GLOBAL.refreshtarget;

   goto IveGotNewUrl;
  }
  else
  if(GLOBAL.needrender)
  {
   forced_html=0;
   GLOBAL.timeout=0;
   GLOBAL.validtables=TABLES_UNKNOWN;
   if(GLOBAL.source)       //HTML source code - special mode...
   {
    if(activeframe>0)
     memcpy(&htmlframe[0].cacheitem,
            &htmlframe[activeframe].cacheitem,sizeof(struct HTTPrecord));
    arachne.target=0;
    arachne.framescount=0;
    GLOBAL.nothot=1;
   }
   else
   if(GLOBAL.needrender>1) //special dialog boxes, etc.
    forced_html=1;

   html_source=LOCAL_HTML;
   pagetime=time(NULL);
   goto Render;
  }
  else
  if(GLOBAL.nowimages)
  {
   if(!NeedImage(FIND_MISSING_IMAGE,IE_NULL) || ignoreimages)
   {
    if(*cacheitem_status==REMOTE)      //nejde o lok. soubor
    {
     NeedImage(EXPIRE_ALL_IMAGES,IE_NULL);         //potrebuju reloadovat obrazky ?
     if(NeedImage(FIND_MISSING_IMAGE,IE_NULL))
     {
      ignoreimages=0;
      GLOBAL.needrender=1;
      GLOBAL.validtables=TABLES_UNKNOWN;
      goto IveGotNewUrl;
     }
     else
      GLOBAL.nowimages=IMAGES_NOTNOW;
    }
    else
    {
     GLOBAL.nowimages=IMAGES_NOTNOW;
     Piip();
     defaultmsg();
    }
   }
   else
   {
    GLOBAL.needrender=1;
    GLOBAL.validtables=TABLES_UNKNOWN;
    goto IveGotNewUrl;
   }
  }
  else
  if(GLOBAL.del) //delete key was pressed - delete from cache/mail directory
  {
   if(htmlframe[activeframe].status!=LOCAL &&
      tmpframedata[activeframe].writeadr!=IE_NULL)
   {
    //delete cached version:
    if(GLOBAL.del==1)
    {
     sprintf(buf,MSG_REMOVE,htmlframe[activeframe].cacheitem.locname);
     outs(buf);
    }
    DeleteFromCache(tmpframedata[activeframe].writeadr);
   }
   else
    Piip();
   GLOBAL.del=0;
  }

 }
 //-------------------------------------------------------------------------

userend:
 memset(AUTHENTICATION,0,sizeof(struct AUTH_STRUCT));

 arachne.target=0; //!!!
 arachne.scriptline=0;
 if(!tcpip)
  unlink("PPP.LOG");
 if(fullscreen)
  arachne.GUIstyle-=4;
 goto end;

#ifndef NOTCPIP
 sock_err:
 sockmsg(status,closesock);
 defaultmsg();
 goto ReadScriptLine;
#endif

end:
 return Terminate_Arachne(returnvalue);
}


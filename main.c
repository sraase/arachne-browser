
/******************************************************************************

Wattcp  : Copyright (c)1990,1991,1992,1993,1999 Erick Engelke
X_LOPIF : Copyright (c)1990,1999 Zdenek Harovnik
Arachne : Copyright (c)1996,1997,1998,1999,2000 Michael Polak, Arachne Labs

ARACHNE WWW BROWSER (TM) is a trademark of Michael Polak, Arachne Labs (TM)

******************************************************************************/

//!!JdS 2004/10/21 : Added conditional compilation
//  of 386+387 (requires BC++3.1) or 286+287 build
#ifdef FPU3
#  pragma option -3
#  pragma option -f287  // (not a typo.)
#else
#  ifdef FPU2
#    pragma option -2
#    pragma option -f287
#  else
#    pragma option -3-
#    pragma option -2-
#    pragma option -f
#  endif
#endif

#include "arachne.h"
#include "internet.h"
#include "main.h"
#include "xanimgif.h"
#include "gui.h"

#ifdef SVGALIB
#include <vga.h>
#endif

// ========================================================================
// Arachne WWW browser main() function:
// ========================================================================
int arachne_main(struct Url *initurl)
{
 struct Url url;
 struct HTTPrecord inlineimage;
 struct HTTPrecord *cacheitem;
 XSWAP update_redirection[2]={IE_NULL,IE_NULL},inlineimage_writeadr=IE_NULL;
 unsigned inlineimage_status;
 XSWAP currentimage=IE_NULL;
 XSWAP *cacheitem_writeadr;
 unsigned *cacheitem_status;
 char found;
// char html_source=LOCAL_HTML,p->forced_html=0;
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
 int addobjectsnow=0;  //append embeded objects to .htt file now ?

 memcpy(&url, initurl, sizeof(struct Url));

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
 strcpy(Referer,p->htmlframe[p->activeframe].cacheitem.URL);
 if(!strncmpi(Referer,"file:",5))
  Referer[0]='\0';

 if(GLOBAL_justrestarted && arachne.framescount) // arachne -c, -r + frames
 {
  p->forced_html=RELOAD_HTML_FRAMES;
  if(!arachne.target)
   GLOBAL.nothot=1;
 }
 else
 {
  p->forced_html=0;
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

//!!glennmcc: Feb 13, 2006 -- at Ray's suggestion,
// changed variable name to match the keyword
  if(addobjectsnow && user_interface.keephtt)
//if(addobjectsnow && !user_interface.nohtt)
  {
   char httfile[2*URLSIZE+80];
   int htt;
   makehttfilename(p->htmlframe[arachne.target].cacheitem.rawname,httfile);
   htt=a_fast_open(httfile,O_BINARY|O_WRONLY|O_APPEND,S_IREAD|S_IWRITE);
   if(htt>=0)
   {
    sprintf(httfile,"<LI><A HREF=\"%s\">%s</A>\n",GLOBAL.location,GLOBAL.location);
    write(htt,httfile,strlen(httfile));
    close(htt);
   }
  }
 }
 else                 // do this for other objects
 {
  addobjectsnow=0;
  //pseudoprotocols - reload, find
  if(!strncmpi(GLOBAL.location,"reload:",7))
  {
   int l=strlen(&GLOBAL.location[7]);

   if(!l)
   {
    GLOBAL.reload=RELOAD_CURRENT_LOCATION;
    strcpy(GLOBAL.location,p->htmlframe[p->activeframe].cacheitem.URL);
    arachne.target=p->activeframe;
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
//!!glennmcc: Feb 25, 2006 -- modified to allow colons, spaces and dots
// in the new 'define' function (see below)
  if((!strncmpi(GLOBAL.location,"find:",5) ||
     (!strchr(GLOBAL.location,':') || strchr(GLOBAL.location,' '))
      && !strchr(GLOBAL.location,'.')) &&
//!!glennmcc: Apr 12, 2007 -- added 'wiki:' (see below)
//!!glennmcc: Apr 13, 2007 -- duplicated 'define:' with 'webster:'
//added 'oxford:' and combined all 4
//(define:, webster:, oxford:, wiki:) into the same block of code
      strncmpi(GLOBAL.location,"define:",7)
   && strncmpi(GLOBAL.location,"webster:",8)
   && strncmpi(GLOBAL.location,"oxford:",7)
   && strncmpi(GLOBAL.location,"wiki:",5)
//!!glennmcc: Jan 19, 2008 -- added spellchecker
   && strncmpi(GLOBAL.location,"spell:",6)
//!!glennmcc: Jan 24, 2008 -- added ping of domain_name or IP_address
   && strncmpi(GLOBAL.location,"ping:",5)
//!!glennmcc: Jan 24, 2008 -- added traceroute to domain_name or IP_address
   && strncmpi(GLOBAL.location,"traceroute:",11)
//!!glennmcc: Jan 24, 2008 -- added whois of domain_name
   && strncmpi(GLOBAL.location,"whois:",6)
//!!glennmcc: Jan 24, 2008 -- added whois of IP_address
   && strncmpi(GLOBAL.location,"whoisip:",8)
      )
  {
   int i=0;
   char *ptr = config_get_str("SearchEngine", "http://www.google.com/search?q=");
   char buf[4*URLSIZE];
   if(!strncmpi(GLOBAL.location,"find:",5))
    i=5;
   cgiquery((unsigned char *)&GLOBAL.location[i],(unsigned char *)buf,1);
   strcpy(GLOBAL.location,ptr);
   makestr(&GLOBAL.location[strlen(ptr)],buf,URLSIZE-strlen(ptr)-2);
  }
//!!glennmcc: Feb 25, 2006 -- new 'define' function to get the
// definition of a word or phrase via Webster's online dictionary
//!!glennmcc: Apr 13, 2007 -- duplicated 'define:' with 'webster:'
//added 'oxford:' and combined all 4
//(define:, webster:, oxford:, wiki:) into the same block of code
  else
  if(!strncmpi(GLOBAL.location,"define:",7)
     || !strncmpi(GLOBAL.location,"webster:",8)
     || !strncmpi(GLOBAL.location,"oxford:",7)
     || !strncmpi(GLOBAL.location,"wiki:",5)
//!!glennmcc: Jan 19, 2008 -- added spellchecker
     || !strncmpi(GLOBAL.location,"spell:",6)
//!!glennmcc: Jan 24, 2008 -- added ping
     || !strncmpi(GLOBAL.location,"ping:",5)
//!!glennmcc: Jan 24, 2008 -- added traceroute
     || !strncmpi(GLOBAL.location,"traceroute:",11)
//!!glennmcc: Jan 24, 2008 -- added whois of domain_name
     || !strncmpi(GLOBAL.location,"whois:",6)
//!!glennmcc: Jan 24, 2008 -- added whois of IP_address
     || !strncmpi(GLOBAL.location,"whoisip:",8)
    )
  {
   int i=7;
   char buf[4*URLSIZE];
// char *ptr="http://www.learnersdictionary.com/search/";
   char *ptr="http://www.merriam-webster.com/dictionary/";
// char *ptr="http://www.merriam-webster.com/cgi-bin/dictionary?";
   if (!strncmpi(GLOBAL.location,"webster:",8)) i=8;
   if (!strncmpi(GLOBAL.location,"oxford:",7))
//!!glennmcc: June 16, 2013 -- askoxford no-longer working
   ptr="http://oxforddictionaries.com/definition/english/";
//!!glennmcc: June 06, 2007 -- much better than /concise_oed/
//   ptr="http://www.askoxford.com/results/?view=dict&freesearch=";
//   ptr="http://www.askoxford.com/concise_oed/";
   if (!strncmpi(GLOBAL.location,"wiki:",5))
   {
    i=5;
    ptr="http://en.wikipedia.org/wiki/Special:Search?search=";
   }
//!!glennmcc: Jan 19, 2008 -- added spellchecker
   if (!strncmpi(GLOBAL.location,"spell:",6))
   {
    i=6;
    ptr="http://www.spellcheck.net/cgi-bin/spell.exe?action=CHECKWORD&string=";
//must disable keepalive due to spellcheck.net being on MS IIS v6 server :(
    http_parameters.keepalive=0;
   }
//!!glennmcc: Jan 24, 2008 -- added ping
   if (!strncmpi(GLOBAL.location,"ping:",5))
   {
    i=5;
    ptr="http://www.fifi.org/services/ping?packetcount=1&packetwait=1&formatted=yes&hostname=";
   }
//!!glennmcc: Jan 24, 2008 -- added traceroute
   if (!strncmpi(GLOBAL.location,"traceroute:",11))
   {
    i=11;
    ptr="http://www.fifi.org/services/traceroute?nprobes=1&resolved=yes&submit=Traceroute&hostname=";
   }
//!!glennmcc: Jan 24, 2008 -- added whois of domain_name
   if (!strncmpi(GLOBAL.location,"whois:",6))
   {
    i=6;
//    ptr="http://www.networksolutions.com/whois/results.jsp?domain=";
    ptr="http://www.whois.com/whois/";
   }
//!!glennmcc: Jan 24, 2008 -- added whois of IP_address
   if (!strncmpi(GLOBAL.location,"whoisip:",8))
   {
    i=8;
//    ptr="http://www.networksolutions.com/whois/results.jsp?ip=";
    ptr="http://www.whois.com/whois/";
   }
//the next 3 lines are common to all added protocols.
  {
   cgiquery((unsigned char *)&GLOBAL.location[i],(unsigned char *)buf,1);
   strcpy(GLOBAL.location,ptr);
   makestr(&GLOBAL.location[strlen(ptr)],buf,URLSIZE-strlen(ptr)-2);
  }
//!!glennmcc: end

  }
  else
  if(!strcmpi(GLOBAL.location,"arachne:addressbook"))
  {
//!!glennmcc: Aug 25, 2005 -- change to new adrsbook.htm
    strcpy(GLOBAL.location,"file:adrsbook.htm");
/*
   char *ptr=configvariable(&ARACHNEcfg,"Hotlist",NULL);
   if(ptr)
    sprintf(GLOBAL.location,"file:%s#mailto",ptr);
   else
    strcpy(GLOBAL.location,"file:hotlist.htm#mailto");
*/
//!!glennmcc: end
  }
  //endif pseudoprotocols

  currentimage=IE_NULL;
  GLOBAL.validtables=TABLES_UNKNOWN;
  GLOBAL.tabledepth=0;
  ie_killcontext(CONTEXT_TABLES);

//!!Bernie: begin, July 4, 2002
// inserted to save to textarea.tmp whenever a page containing
// <textarea> is aborted without submitting the form.
  Deallocmem();
//!!Bernie: end

  p->firstHTMLtable=IE_NULL;
  p->firstHTMLatom=p->lastHTMLatom=IE_NULL;
  p->firstonscr=p->lastonscr=IE_NULL;

  cacheitem=&(p->htmlframe[arachne.target].cacheitem);
  //status of main displayed document
  cacheitem_status=&(p->htmlframe[arachne.target].status);
  //update adr of main displayed document
  cacheitem_writeadr=&(p->tmpframedata[arachne.target].writeadr);
  timer=time(NULL);
  if(GLOBAL.nowimages!=IMAGES_SEEKCACHE)
   pagetime=timer;
  needredraw=0;
  ivegotallimages=0;
  GLOBAL.source=0;
  GLOBAL.timeout=0;
  GLOBAL.backgroundimages=BACKGROUND_EMPTY;
//  GLOBAL.clipdel=0;
  //!!JdS: 2004/12/08 {
  //Allow for deferred addition of clipboard link to hotlist
  //GLOBAL.clipdel=0;
  if (GLOBAL.clipdel == CLIPBOARD_DEFER_ADD)
   GLOBAL.clipdel = CLIPBOARD_ADDHOT;
  else
   GLOBAL.clipdel = CLIPBOARD_DEFAULT;
  //!!JdS: 2004/12/08 }
  //reset keepalive for this session
#ifndef NOTCPIP
  sock_keepalive[0][0]='\0';
  sock_keepalive[1][0]='\0';
#endif

  if(!arachne.target)
  {
   arachne.backtrace=0;
  }
  else if(!GLOBAL.nothot)
  {
   if(arachne.backtrace==MAXBACKTRACE-1)
    memmove(arachne.backtrace_target,arachne.backtrace_target+1,MAXBACKTRACE-1);
   arachne.backtrace_target[arachne.backtrace]=arachne.target;
//   if(arachne.backtrace<MAXBACKTRACE)
   if(arachne.backtrace<MAXBACKTRACE-1)  //!!JdS 2004/11/09
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
   strcpy(url.kotva,"current");
   p->html_source=HISTORY_HTML;
   p->forced_html=1;
   GLOBAL.nothot=1;
   *cacheitem_status=VIRTUAL;
   goto Render;
 }

 p->html_source=LOCAL_HTML;

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
   char text[IE_MAXLEN+2];
   char buf[IE_MAXLEN+2];
   plugin=externalprotocol(url.protocol,text);

   if(plugin)
   {
    make_cmd(text,buf,
             p->htmlframe[arachne.target].cacheitem.URL,
             url.host, url.file, text, "NUL");
   }
   else
    sprintf(buf,"telnet %s\n",url.host);

#ifdef POSIX
   printf("Executing command:\n%s\n",buf);
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

  //--------------------------------------------------------- rtsp: and mms:
  if(!strcmpi(url.protocol,"rtsp") || !strcmpi(url.protocol,"mms"))
  //------------------------------------------------------------------------
  {
   char buf[IE_MAXLEN+2];
   sprintf(buf,"call mp-get.bat %s://%s%s\n",url.protocol,url.host,url.file);

#ifdef POSIX
   printf("Executing command:\n%s\n",buf);
   system(buf);
   goto Wait4Orders;
#else
   closebat(buf,RESTART_REDRAW);
   returnvalue=willexecute(buf);
   x_grf_mod(3);
   goto end;
#endif
  }
  //----------------------------------------------------- end rtsp: and mms:

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
    char text[IE_MAXLEN+2];

#ifndef NOTCPIP
//!!glennmcc: May 6, 2004
// added '&& !strstr(GLOBAL.location,"edit:")' to fix the bug of dialing
// when a local file is edited with F4 and an 'external editor'
// while 'offline' and Autodial is set to yes
   if(user_interface.autodial && !httpstub
     && !GLOBAL_justrestarted && !strstr(GLOBAL.location,"edit:"))
   //status!=LOCAL) //1st attempt to fix completly disabled autodial :(
//!!glennmcc: end

    {
     add2history(GLOBAL.location);
     strcpy(GLOBAL.location,"arachne:dialer");
     goto IveGotNewUrl;
    }
#endif

    sprintf(str,"external/%s",url.protocol);
    plugin=externalprotocol(url.protocol,text);
    if(plugin)
    {
     char buf[IE_MAXLEN+2];
     make_cmd(text,buf,
              p->htmlframe[arachne.target].cacheitem.URL,
              url.host, url.file, text, "NUL");

#ifdef POSIX
     printf("Executing command:\n%s\n",buf);
     system(buf);
     goto Wait4Orders;
#else
     closebat(buf,RESTART_REDRAW);
     returnvalue=willexecute(buf);
     x_grf_mod(3);
     goto end;
#endif
    }
    else
     sprintf(p->htmlframe[0].cacheitem.locname,"%serr_net.ah",guipath);

    *cacheitem_writeadr=IE_NULL;
    p->forced_html=1;
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
    p->html_source=HTTP_HTML;
    if(user_interface.multitasking==MULTI_SAFE)
     FinishBackground(BG_FINISH);
   }
   else
   {
    if(GLOBAL.isimage)
     p->html_source=HTTP_HTML;
    else
     p->html_source=LOCAL_HTML;
   }

   *cacheitem_writeadr=Write2Cache(&url,cacheitem,1,1);
   //http 2 cache
   if(openhttp(&url,cacheitem))
   {
    if(GLOBAL.abort)
     goto Abort;

    if(!GLOBAL.isimage)  //add other downloaded objects to .htt file
     addobjectsnow=1;

    UpdateInCache(*cacheitem_writeadr,cacheitem);
    if(update_redirection[socknum]!=IE_NULL)
    {
     UpdateFilenameInCache(update_redirection[socknum], cacheitem);
     update_redirection[socknum]=IE_NULL;
    }
    *cacheitem_status=REMOTE;
   }
   else //error - bad host name, connection reset, etc.
   {
    Abort:

//!!glennmcc: May 31, 2007 -- add2history upon aborting transfer
//but only when no data has yet been received and cached
if(GLOBAL.abort==ABORT_TRANSFER && GLOBAL.location != cacheitem->URL)
add2history(GLOBAL.location);
//!!glennmcc: end

    if(GLOBAL.redirection)
    {
     if((GLOBAL.isimage || arachne.target) && update_redirection[socknum]==IE_NULL)
     {
      strcpy(cacheitem->locname,"NUL");
      cacheitem->rawname[0]='\0';
      UpdateInCache(*cacheitem_writeadr,cacheitem);
      update_redirection[socknum]=*cacheitem_writeadr;
//      printf("[image redirection, update=%u]",update_redirection[socknum]);

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
      sprintf(cacheitem->locname,"%slogin.ah",guipath);
      p->forced_html=1;
      p->html_source=LOCAL_HTML;
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

     sprintf(p->htmlframe[0].cacheitem.locname,"%serr_open.ah",guipath);
     p->forced_html=1;
     p->html_source=LOCAL_HTML;
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
   case GOTO_TRYPLUGIN:    goto tryplugin;

   case GOTO_ERROR:
   arachne.target=0;
   error=1;
   p->forced_html=1;
   goto Render;

   case GOTO_LOCAL_HTML:
   p->html_source=LOCAL_HTML;
   p->forced_html=1;
   goto Render;

   case GOTO_EXTERNAL:
   case UNKNOWN_PROTOCOL:
   if (!GLOBAL.isimage)
   {
    char cmd[IE_MAXLEN+2];
    plugin=externalprotocol(url.protocol,cmd);
    if(plugin)
    {
     char buf[IE_MAXLEN+2];
     make_cmd(cmd,buf,
	      p->htmlframe[arachne.target].cacheitem.URL,
              url.host, url.file, cmd, "NUL");

#ifdef POSIX
     printf("Executing command:\n%s\n",buf);
     system(buf);
     goto Wait4Orders;
#else
     closebat(buf,RESTART_REDRAW);
     returnvalue=willexecute(buf);
     x_grf_mod(3);
     goto end;
#endif
    }
//!!glennmcc: begin Jan 16, 2005 -- no error page when 'arachne:'
if(!strstr(url.protocol,"arachne:"))
   {
    sprintf(p->htmlframe[0].cacheitem.locname,"%serr_url.ah",guipath);
   }
//    sprintf(p->htmlframe[0].cacheitem.locname,"%serr_url.ah",guipath);
//!!glennmcc: end
    arachne.target=0;
    error=1;
    p->forced_html=1;
    goto Render;
   }
   else
   {

    errimage:

    outs(MSG_ERRIMG);
    strcpy(inlineimage.locname,"NUL");
    strcpy(inlineimage.mime,"???");
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
// if((arachne.newframe>0 || arachne.target>0)&& p->html_source==HTTP_HTML)
 if((arachne.target || isframe) && p->html_source==HTTP_HTML && !GLOBAL.isimage)
 {
 // p->html_source will not be set to HTTP_HTML when we are offline
  Download(cacheitem);
  closehttp(cacheitem);
  p->html_source=LOCAL_HTML;
 }

 if(GLOBAL.isimage && p->html_source==HTTP_HTML)
 {
  if(update_redirection[socknum]!=IE_NULL)
  {
   UpdateFilenameInCache(update_redirection[socknum], &inlineimage);
   update_redirection[socknum]=IE_NULL;
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
//   FinishBackground(BG_ABORT);
   FinishBackground(BG_FINISH);  //!!JdS 2004/11/06
   goto AbortRedrawAndMsg;
  }
  else
  if(GLOBAL.gotolocation)
  {
   unlink(inlineimage.locname);
   FinishBackground(BG_ABORT);
   goto IveGotNewUrl;
  }

  if(!GLOBAL.imagevisible && !needredraw)
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
  p->html_source=LOCAL_HTML;
  //we have invalidate table in this case
  GLOBAL.validtables=TABLES_UNKNOWN;

 }//endif is image
#endif

 //----------------------------------------------------------- plugin?
 tryplugin:
 //------------------------------------------------------------------------
 //. Arachne does not support jpeg, so we have to rely on plugin

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
     ie_clearf(&history,0);       //!!JdS 2004/11/05 removed if() semi-colon :
//!!glennmcc: Dec 03, 2005 -- increased to 388 or 1024
//in 'non-release' experimental compiles only (see 'LINES define' in init.c)
//works in tandom with the increase of LINES define in init.c
//and MAXCONV define in urlovrl.c
     if(ie_openf_lim(&history,CONTEXT_SYSTEM,1024)==1) //historie - max. 256 radku
     {
      ie_insline(&history,0,"");
      arachne.scriptline=1;
      goto ReadScriptLine;
     }
   }//end if script
   weird=(strstr("HTM TXT CSS",ext)==NULL);
  }

  plugin=search_mime_cfg(cacheitem->mime, ext, command);

#ifndef NOTCPIP
  //download on background:
  if((weird || plugin || GLOBAL.backgr) && p->html_source==HTTP_HTML)
  {
   Download(cacheitem);
   closehttp(cacheitem);
   found=1;
   if(GLOBAL.backgr)
//!!glennmcc: begin Feb 06, 2005 -- fix 'dead links' on page after backgr dl
   {
//    goback();
//    gotonextpage();
//!!glennmcc: Feb 07, 2005 Ray's fix works better than mine.
//!!glennmcc: Aug 26, 2005 -- 'Enter' does BG DL/view when DL complete
//'left mouse click' will still 'view as downloading'
//see companion code in guievent.c
if(//!strstr(configvariable(&ARACHNEcfg,"EnterBGDL",NULL),"Y")
   GLOBAL.backgr<2)
//!!glennmcc: end
    strcpy(GLOBAL.location, ie_getline(&history, arachne.history));
    GLOBAL.gotolocation=1;
//!!glennmcc: July 06, 2006 -- scroll to top left
   if(GLOBAL.backgr && found==1)
      {
       p->htmlframe[p->activeframe].posY=0;
       p->htmlframe[p->activeframe].posX=0;
      }
//!!glennmcc: end
    goto Wait4Orders; //original single line
   }
//!!glennmcc: end

   if(GLOBAL.abort)
   {
    unlink("download.tmp");
    rename(cacheitem->locname,"download.tmp");
    goto PageDone;
   }
  }
#endif

  //download/save as prompt:
  if(!plugin && weird && !strstr(imageextensions,ext) && cacheitem->mime[0] && found)
  {
   strcpy(LASTlocname,cacheitem->locname);
   strcpy(cacheitem->locname,guipath);
   if(*cacheitem_status==REMOTE)
    strcat(cacheitem->locname,"download.ah");
   else
    strcat(cacheitem->locname,"copy.ah");

   p->forced_html=1;
   p->html_source=LOCAL_HTML;
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
   char buf[IE_MAXLEN+1];

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
     sprintf(str,MSG_CONV,oldmime,ext,MSG_DELAY2,ctrlbreak);
     outs(str);

     savepick();

     //this would be safer, but too slow....
     //ie_savef(&history);
     //ie_savebin(&HTTPcache);

#ifndef POSIX
//!!glennmcc: Nov 18, 2005 mime.cfg lines will now retain case
     if(!strcmpi(buf,"insight"))
//     if(strstr(strlwr(buf),"insight"))
     {
      tempinit(mman);
      strcat(mman,"$roura2.bat");
      unlink(mman);
     }
#endif
//     printf("Executing command: %s\n",p->buf);
     system(buf);
#ifndef POSIX
     if(mman[0] && file_exists(mman))
      system(mman);
#endif
     p->html_source=LOCAL_HTML;
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
//!!glennmcc: Nov 18, 2005 mime.cfg lines will now retain case
     if(!strcmpi(buf,"insight"))
//     if(strstr(strlwr(buf),"insight"))
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
     x_bar(p->htscrn_xtop,p->htscrn_ytop,p->htscrn_xtop+p->htscrn_xsize,p->htscrn_ysize+p->htscrn_ytop);
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
 GLOBAL.needrender=0;           // maybe we will not need to render again ?
 GLOBAL.reload=NO_RELOAD;
 if(url.kotva[0]) // jump to anchor (#xyz) ?
  GLOBAL.norefresh=1;
 else
  GLOBAL.norefresh=0;

//--------------------------------------------------------------------------
//process all frames, write cacheitem to p->htmlframe[arachne.target]

 currentimage=IE_NULL;  //memory atoms will be deallocated
 p->rendering_target=RENDER_SCREEN;
 if(!renderHTML(p))
//--------------------------------------------------------------------------
 {
  char loc[URLSIZE+1];
  //send error message/reload page to currently processed frame:
  cacheitem=&(p->htmlframe[p->currentframe].cacheitem);

  //failed to load remote document - try again
  if(!error && *cacheitem_status!=LOCAL &&
     p->html_source!=HISTORY_HTML &&
     !(GLOBAL_justrestarted))
  {
   *cacheitem_writeadr=Write2Cache(&url,cacheitem,0,0);
   DeleteFromCache(*cacheitem_writeadr);
   error=1;
   goto Retry;
  }

  //failed to load local document:
  sprintf(loc,"%serr_load.ah",guipath);
  if(!strcmp(loc,p->htmlframe[p->currentframe].cacheitem.locname) || error)
  {
   error=1;
   MakeTitle(MSG_ERROR);
   DrawTitle(0);
  }
  else
  {
   strcpy(p->htmlframe[p->currentframe].cacheitem.locname,loc);
   p->forced_html=1;
   p->html_source=LOCAL_HTML;
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

  if(!GLOBAL.isimage && !GLOBAL.source)   //. source -->p->html_source
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

 if(/*!GLOBAL.isimage &&*/
    !error &&
    !user_interface.quickanddirty &&
    !GLOBAL.needrender &&
    GLOBAL.validtables==TABLES_EXPAND)
 {
  //printf("error=%d, validtables=%d\n",error, GLOBAL.validtables);
  //while redrawing page we are always reading it from disk
  p->html_source=LOCAL_HTML;
  //because we are just redrawiing, so we do not modify the history list...
  GLOBAL.nothot=1;
  goto Render;
 }


//-------------------------------------------------------------------------
Search4Image:
//-------------------------------------------------------------------------


 if(update_redirection[socknum]!=IE_NULL)
 {
  UpdateFilenameInCache(update_redirection[socknum], cacheitem);
  update_redirection[socknum]=IE_NULL;
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
   AnalyseURL(p->htmlframe[arachne.newframe].cacheitem.URL,&url,IGNORE_PARENT_FRAME);
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
   arachne.newframe=p->htmlframe[arachne.newframe].next;
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
   imageismissing=NeedImage(FIND_MISSING_IMAGE,&currentimage);
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
  FinishBackground(BG_FINISH_ALL);
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
   p->html_source=LOCAL_HTML;
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
  p->html_source=LOCAL_HTML;
  //because we are just redrawiing, so we do not modify the history list...
  GLOBAL.nothot=1;
  goto Render;
 }

//-------------------------------------------------------------------------
PageDone:
//-------------------------------------------------------------------------

//!!glennmcc: Nov 08, 2005 -- when Xswap memory gets too low
//!!glennmcc: Nov 22, 2005 -- the experiment did not work correctly :(
//I'll leave this here but commented-out for others to play with ;-)
/*
    if(ie_free()<ie_used())
      {
       char cmd[128];
       closebat(cmd,RESTART_KEEP_GRAPHICS);
       GLOBAL.willexecute=willexecute(cmd);
       farfree(cmd);
       goto Render;
      }
*/
//!!glennmcc: end

 pagetime=time(NULL)-pagetime; //. Much time we have used to do this page
 if(GLOBAL.timeout)
  GLOBAL.secondsleft-=(int)pagetime;

 if(p->memory_overflow)
  outs(MSG_NOTMEM);
 else if(GLOBAL.abort)
 {
  redrawHTML(REDRAW_NO_MESSAGE,REDRAW_CREATE_VIRTUAL);
  outs(MSG_ABORT);
 }
 //!!JdS 2004/12/12 {
 else if (GLOBAL.clipdel == CLIPBOARD_ADDHOT)
  outs(MSG_CLPADD);
 //!!JdS 2004/12/12 }
 else
 {
  char dummy[128];
  sprintf(dummy,MSG_DOCDON,copyright,pagetime/60,(int)(pagetime % 60));
  outs(dummy);
 }
/*}//!!glennmcc: added for 'Xswap clear code' above*/

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
 addobjectsnow=0; //HTT file is complete ?
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
  if(!redraw && !activeistextwindow && !htmlpulldown && g_NumAnim > 0)  // something to animate
   XAnimateGifs();
#endif

#ifdef POSIX
 //redraw is special global flag of GUITICK() system...
 //if justmoved is true or mys ("mouse") is nonzero, we have to do something
 if(!redraw && mys==0 && !justmoved)
#ifdef SVGALIB
 {
  struct timeval tv={0,500000};
  WaitForEvent(&tv); //NULL pointer means >>wait forever for mouse or keystroke<<
                     //later, timeval structure with time limit will be passed,
                     //considering  animation, JavaScript and redirection timouts,
                     //and also DrawTime function - used in fullscreen version...
 }
#endif
#ifdef SDL2
 {
  struct timeval tv = { 0, 500000 };
  WaitForEvent(&tv);
 }
#endif

#endif

#ifndef NOTCPIP
#ifdef WATTCP
  if(tcpip)
   tcp_tick(NULL);

  if(p->html_source==HTTP_HTML)
  {
   if(closing[0])
   {
    sock_keepalive[0][0]='\0';
    closesock=0;
    sock_tick( sock[0], &status ); //TCP/IP close ?
   }
   if(closing[1])
   {
    sock_keepalive[1][0]='\0';
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
     // This is where the frames/smiley bug manifested itself :
     // Without the 'writeadr' field being initialized, this part
     // of the code could corrupt other XSWAP data. [JdS 2006/02/19]
//note added by gelennmcc: the 'fix' itself is in htmlutil.c and init.c
//Joe, THANK YOU !!! this bug has been with us for too many years.
     cacheitem=(struct HTTPrecord *)ie_getswap(p->tmpframedata[fr].writeadr);
     if(cacheitem)
     {
      cacheitem->x=p->htmlframe[fr].posX;
      cacheitem->y=p->htmlframe[fr].posY;
      cacheitem->knowsize=p->htmlframe[fr].cacheitem.knowsize;
      cacheitem->size=p->htmlframe[fr].cacheitem.size;
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
    strcpy(GLOBAL.location,p->htmlframe[p->activeframe].cacheitem.URL);
    arachne.target=p->activeframe;
   }

   if(GLOBAL.timeout)
   {
    if(tcpip)
     GLOBAL.reload=RELOAD_NEW_LOCATION;
    else
     arachne.target=GLOBAL.refreshtarget; //??? I forgot what is this ;-)
   }
   goto IveGotNewUrl;
  }
  else
  if(GLOBAL.needrender)
  {
   p->forced_html=0;
   GLOBAL.timeout=0;
   GLOBAL.validtables=TABLES_UNKNOWN;
   if(GLOBAL.source)       //HTML source code - special mode...
   {
    if(p->activeframe>0)
     memcpy(&(p->htmlframe[0].cacheitem),
            &(p->htmlframe[p->activeframe].cacheitem),sizeof(struct HTTPrecord));
    arachne.target=0;
    arachne.framescount=0;
    GLOBAL.nothot=1;
   }
   else
   if(GLOBAL.needrender>1) //special dialog boxes, etc.
    p->forced_html=1;

   p->html_source=LOCAL_HTML;
   pagetime=time(NULL);
   goto Render;
  }
  else
  if(GLOBAL.nowimages)
  {
   if(!NeedImage(FIND_MISSING_IMAGE,NULL) || ignoreimages)
   {
    if(*cacheitem_status==REMOTE)      //nejde o lok. soubor
    {
     NeedImage(EXPIRE_ALL_IMAGES,NULL);         //potrebuju reloadovat obrazky ?
     if(NeedImage(FIND_MISSING_IMAGE,NULL))
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
   if(p->htmlframe[p->activeframe].status!=LOCAL &&
      p->tmpframedata[p->activeframe].writeadr!=IE_NULL)
   {
    //delete cached version:
    if(GLOBAL.del==1)
    {
     char str[256];
     sprintf(str,MSG_REMOVE,p->htmlframe[p->activeframe].cacheitem.locname);
     outs(str);
    }
    DeleteFromCache(p->tmpframedata[p->activeframe].writeadr);
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
#ifdef MSDOS
 if(!tcpip)
  unlink("PPP.LOG");
#endif
 if(fullscreen)
  arachne.GUIstyle-=4;
 goto end;

#ifndef NOTCPIP
#ifndef POSIX
 sock_err:
 sockmsg(status,closesock);
 defaultmsg();
 goto ReadScriptLine;
#endif
#endif

end:
 return returnvalue;
}

/* main entry point */
int main(int argc, char **argv)
{
	struct Url url;
	int ret;

	printf(MSG_START, VER, beta, copyright);

	Initialize_Arachne(argc, argv, &url);

#ifdef SVGALIB
	if (vga_init()) {
		printf("Failed to initialize SVGALIB.\n");
		return 100;
	}
#endif

	ret = arachne_main(&url);

	return Terminate_Arachne(ret);
}

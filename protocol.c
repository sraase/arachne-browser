
#include "arachne.h"
#include "internet.h"

int protocol_arachne(struct HTTPrecord *cacheitem,struct Url *url,int *returnvalue)
{
 char *value;
#ifndef NOTCPIP
#ifndef CLEMTEST
  if(!strncmpi(url->file,"ftp-",4))
  {
   char back=1;

   if(!strncmpi(&(url->file[4]),"send",4))
    back=0;

   strcpy(url->user,configvariable(&ARACHNEcfg,"FTPusername",NULL));
   strcpy(url->password,configvariable(&ARACHNEcfg,"FTPpassword",NULL));
   strcpy(url->host,configvariable(&ARACHNEcfg,"FTPserver",NULL));
   strcpy(url->file,configvariable(&ARACHNEcfg,"FTPpath",NULL));
   url->port=21;

   if(tcpip)
    ftpsession(url,cacheitem,LASTlocname);

   if(!GLOBAL.gotolocation)
   {
    if(back)
     goback();
    else
     strcpy(GLOBAL.location,htmlframe[activeframe].cacheitem.URL);
    arachne.target=activeframe;
   }

   GLOBAL.reload=0;
   GLOBAL.postdata=0;
   return GOTO_IVEGOTNEWURL;
  }
#endif
#endif
#ifndef POSIX
  if(!strcmpi(url->file,"restart"))
  {
   char *ptr=getenv("ASETUP");
   if(ptr && !strcmp(ptr,"inst"))  //special case - "Finish Setup" button
    return GOTO_USEREND;

   arachne.target=0; //!!!
   strcpy(buf,"@arachne");
   if(tcpip)
    strcat(buf," -o\n");
   else
    strcat(buf,"\n");
   unlink("lock");
   *returnvalue=willexecute(buf);
   return GOTO_END;
  }
  else
#endif
 ///!!!!
 ///POSIX restart: TerminateArachne + exec.. (argv[0],argv[0],NULL) !
#ifndef NOTCPIP
  if(!strcmpi(url->file,"dialpage"))
  {
   value=configvariable(&ARACHNEcfg,"DialPage",NULL);
   if(value)
   {
    strcpy(GLOBAL.location,value);
    AnalyseURL(GLOBAL.location,url,IGNORE_PARENT_FRAME);
   }
  }
  else
#ifndef CLEMTEST
  if(!strcmpi(url->file,"dialer"))
  {
   *returnvalue=willexecute(ArachneDIAL());
   return GOTO_END;
  }
  else
  if(!strcmpi(url->file,"hangup"))
  {
   arachne.target=0; //!!!
   outs(MSG_HANGUP);
   if(reg && tcpip)
    PPPtimelog();
   process_form(0,IE_NULL); //updateovat Arachne.Cfg
   sprintf(buf,"%s\nif exist PPP.LOG del PPP.LOG\n",configvariable(&ARACHNEcfg,"Hangup",NULL));
   value=configvariable(&ARACHNEcfg,"ExitOnHangup",NULL);
   if(!(value && toupper(*value)=='Y'))
   {
    strcat(buf,"@arachne -c\n");
   }
   else
    exitmsg();
   *returnvalue=willexecute(buf);
   return GOTO_END;
  }
  else
#endif
#endif //NOTCPIP
  if(!strncmpi(url->file,"exit",4))
  {
   if(url->file[4]=='=' || url->file[4]=='?')
    *returnvalue=atoi(&(url->file[5]));
   return GOTO_USEREND;
  }
 return 0;
}



int protocol_nohttp(struct HTTPrecord *cacheitem,struct Url *url, unsigned *cacheitem_status, XSWAP *cacheitem_writeadr)
{
 char *value;

#ifndef CLEMTEST
#ifdef OVRL
#ifndef XTVERSION
 //---------------------------------------------------------------- finger:
 if(!strcmpi(url->protocol,"finger"))
 //------------------------------------------------------------------------
 {
  //finger to cache
  *cacheitem_writeadr=Write2Cache(url,cacheitem,1,1);
  strcpy(cacheitem->mime,"finger/out");
  if(xfinger(url,cacheitem,url->user))
  {
   if(GLOBAL.abort)
    return GOTO_ABORT;
   UpdateInCache(*cacheitem_writeadr,cacheitem);
   *cacheitem_status=REMOTE;
  }
  else
  {
   GLOBAL.reload=0;
   return GOTO_ABORT;
  }
 }
 else if(!strcmpi(url->protocol,"news") ||
         !strcmpi(url->protocol,"nntp") )
 {
  //finger to cache
  *cacheitem_writeadr=Write2Cache(url,cacheitem,1,1);

  if(!url->host[0])
  {
   value=configvariable(&ARACHNEcfg,"NNTPserver",NULL);
   if(value)
   {
    makestr(url->host,value,STRINGSIZE-1);
   }
  }

  if(!url->file[0] || url->file[0]=='/' && !url->file[1])
   strcpy(url->file,"LIST");

  strcpy(cacheitem->mime,"news/list");
  if(xfinger(url,cacheitem,url->file))
  {
   if(GLOBAL.abort)
    return GOTO_ABORT;
   UpdateInCache(*cacheitem_writeadr,cacheitem);
   *cacheitem_status=REMOTE;
  }
  else
  {
   GLOBAL.reload=0;
   return GOTO_ABORT;
  }
 }
 //---------------------------------------------------------------- gopher:
 else if(!strcmpi(url->protocol,"gopher"))
 //------------------------------------------------------------------------
 {
  char *selector;
  int plugin=externalprotocol(url->protocol,text);
  if(plugin)
   return GOTO_EXTERNAL;

  //finger to cache
  *cacheitem_writeadr=Write2Cache(url,cacheitem,1,1);

  selector=&(url->file[1]);
  if(!url->file[1])
  {
   strcpy(cacheitem->mime,"gopher/1");
  }
  else if(url->file[2]=='/')
  {
   sprintf(cacheitem->mime,"gopher/%c",url->file[1]);
   if(url->file[1]=='7' && !strchr(url->file,'?')) //search
    return GOTO_TRYPLUGIN;
   selector=&(url->file[3]);
  }

  if(xfinger(url,cacheitem,selector))
  {
   if(GLOBAL.abort)
    return GOTO_ABORT;
   UpdateInCache(*cacheitem_writeadr,cacheitem);
   *cacheitem_status=REMOTE;
  }
  else
  {
   GLOBAL.reload=0;
   return GOTO_ABORT;
  }
 }
 else
#endif //XTVERSION
#endif //statical version
 //------------------------------------------------------------------- ftp:
 if(!strcmpi(url->protocol,"ftp"))
 //------------------------------------------------------------------------
 {
  char *ptr;
  int plugin=externalprotocol(url->protocol,text);

  if(plugin)
   return GOTO_EXTERNAL;

  if(url->user[0] && !url->password[0] &&
     (strcmp(url->host,AUTHENTICATION->host) ||
      strcmp(url->user,AUTHENTICATION->user) ||
      strcmp(AUTHENTICATION->realm,"$ftp")))
  {
   sprintf(cacheitem->locname,"%s%sftplogin.ah",sharepath,GUIPATH);
   strcpy(AUTHENTICATION->host,url->host);
   return GOTO_LOCAL_HTML;
  }

  ptr=configvariable(&ARACHNEcfg,"UseFTPproxy",NULL);
  if(ptr && toupper(*ptr)!='N')
  {
   char *no4all=configvariable(&ARACHNEcfg,"NoFTPproxy4all",NULL);
   ptr=configvariable(&ARACHNEcfg,"NoFTPproxy",NULL);
   if((!ptr || !strstr(strlwr(ptr),strlwr(url->host) )) &&
      (!no4all || !strstr(strlwr(url->host), strlwr(no4all)) ) )
    return GOTO_PROXY;
  }

  //ftp to cache
  *cacheitem_writeadr=Write2Cache(url,cacheitem,1,1);

  if(ftpsession(url,cacheitem,NULL))
  {
   if(GLOBAL.abort)
    return GOTO_ABORT;
   UpdateInCache(*cacheitem_writeadr,cacheitem);
   *cacheitem_status=REMOTE;
  }
  else
  {
   GLOBAL.reload=0;
   return GOTO_ABORT;
  }
 }
 //------------------------------------------------------------------ pop3:
 else if(!strcmpi(url->protocol,"pop3"))
 //------------------------------------------------------------------------
 {
  char dele=1,log=0;

  if(!url->host[0])
  {
   value=configvariable(&ARACHNEcfg,"POP3server",NULL);
   if(value)
    makestr(url->host,value,STRINGSIZE-1);

  }
  if(!url->user[0])
  {
   value=configvariable(&ARACHNEcfg,"POP3username",NULL);
   if(value)
    makestr(url->user,value,STRINGSIZE-1);
  }

  //Draw title and URL (without password):

  MakeTitle(MSG_MAILDL);
  url2str(url,GLOBAL.location);
  SetInputAtom(&URLprompt,GLOBAL.location);
  DrawTitle(0);
  add2history(GLOBAL.location);

  //Get password and other settings:

  if(!url->password[0])
  {
   value=configvariable(&ARACHNEcfg,"POP3password",NULL);
   if(value)
    makestr(url->password,value,PASSWORDSIZE-1);
  }

  value=configvariable(&ARACHNEcfg,"KeepOnServer",NULL);
  if(value && toupper(*value)=='Y') dele=0;

  value=configvariable(&ARACHNEcfg,"POP3log",NULL);
  if(value && toupper(*value)=='Y') log=1;

  //POP3 download is performed here:

  if(!xpopdump(url,dele,log))
  {
   sprintf(htmlframe[0].cacheitem.locname,"%s%serr_pop3.ah",sharepath,GUIPATH);
   return GOTO_ERROR;
  }
  else if(arachne.scriptline==0)
  {
   value=configvariable(&ARACHNEcfg,"AfterPOP3",NULL);
   if(!value)
    strcpy(GLOBAL.location,"file://inbox.dgi");
   else
    strcpy(GLOBAL.location,value);
   GLOBAL.reload=RELOAD_CURRENT_LOCATION;
   return GOTO_IVEGOTNEWURL;
  }
  else
   return GOTO_READSCRIPT;
 }
 //------------------------------------------------------------------ smtp:
 else if(!strcmpi(url->protocol,"smtp"))
 //------------------------------------------------------------------------
 {
  char helo=1,log=0;

  if(!url->host[0])
  {
   value=configvariable(&ARACHNEcfg,"SMTPserver",NULL);
   if(value)
    makestr(url->host,value,STRINGSIZE-1);
  }

  value=configvariable(&ARACHNEcfg,"NoHELO",NULL);
  if(value && toupper(*value)=='Y')
   helo=0;

  value=configvariable(&ARACHNEcfg,"SMTPlog",NULL);
  if(value && toupper(*value)=='Y') log=1;

  if(!url->user[0])
  {
   value=configvariable(&ARACHNEcfg,"eMail",NULL);
   makestr(url->user,value,STRINGSIZE-1);
  }

  if(!url->file[0] || !url->file[1] ) //stmp: or smtp:/
  {
   value=configvariable(&ARACHNEcfg,"MailPath",NULL);
   if(value)
   {
    sprintf(url->file,"/%s*.TBS",value);
   }
  }

  //Draw title and URL:

  MakeTitle(MSG_MAILUP);
  url2str(url,GLOBAL.location);
  SetInputAtom(&URLprompt,GLOBAL.location);
  DrawTitle(0);
  add2history(GLOBAL.location);

  //SMTP upload is performed here:

  if(!xsendmail(url,helo,log))
  {
   sprintf(htmlframe[0].cacheitem.locname,"%s%serr_smtp.ah",sharepath,GUIPATH);
   return GOTO_ERROR;
  }
  else if(arachne.scriptline==0)
  {
   if(strchr(url->file,'*') || GLOBAL.mailaction & MAIL_OUTBOXNOW)
   {
    value=configvariable(&ARACHNEcfg,"AfterSMTP",NULL);
    if(!value)
     strcpy(GLOBAL.location,"file://outbox.dgi");
    else
     strcpy(GLOBAL.location,value);
    GLOBAL.reload=RELOAD_CURRENT_LOCATION;
   }
   else
   {
    goback(); //return to mailto: page...
    goback(); //return to page with <A HREF=mailto:...> tag...
    GLOBAL.postdata=0;
    if(!strcmp(GLOBAL.location,"file://outbox.dgi"))
     GLOBAL.reload=RELOAD_CURRENT_LOCATION;
    else
     GLOBAL.reload=NO_RELOAD;
   }
   return GOTO_IVEGOTNEWURL;
  }
  else
   return GOTO_READSCRIPT;
 }
 else
#endif //CLEMTEST
 //--------------------------------------------------------------- error?
  return UNKNOWN_PROTOCOL;
#ifndef CLEMTEST
 return CONTINUE_TO_RENDER;
#endif //CLEMTEST
}
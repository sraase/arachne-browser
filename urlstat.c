
// ========================================================================
// Staticaly linked part of Arachne URL/CACHE management/hostory
// (c)1997,1998,1999,2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "internet.h"

#ifndef POSIX
char *getShortname(char *longname); //for Windows 9x VFAT ...
#endif

int meta_SearchInCache(struct Url *absURL,struct HTTPrecord *cacheitem, XSWAP *cacheitemadr, unsigned *status, char quicksearch)
{
 //cache je inicializovana jinde..
 int i, startfname=0;
 struct HTTPrecord *cacheptr;
// struct ffblk ff;
 char firstkotva=absURL->kotva[0];
 char dummy[120],ext[5];

 *cacheitemadr=IE_NULL;
 *status=LOCAL;
 absURL->kotva[0]='\0';
 url2str(absURL,cacheitem->URL);
 absURL->kotva[0]=firstkotva;


 if(!strcmpi(absURL->protocol,"file"))
 {
  makestr(cacheitem->locname,absURL->file,79);
  i=strlen(cacheitem->locname);

  if(cacheitem->locname[0]=='/' && cacheitem->locname[1]=='/')
  {
   memmove(cacheitem->locname,&cacheitem->locname[2],i-2);
   i-=2;
   cacheitem->locname[i]='\0';
  }

  //!!!msdos only =============================
#ifndef POSIX
  if(cacheitem->locname[0]=='/' && (cacheitem->locname[2]=='|' || cacheitem->locname[2]==':'))
  {
   memmove(cacheitem->locname,&cacheitem->locname[1],i-1);
   i-=1;
   cacheitem->locname[1]=':';
   cacheitem->locname[i]='\0';
   startfname=2;
  }

  while(i>0)
  {
   i--;
   if(cacheitem->locname[i]=='/')
   {
    cacheitem->locname[i]='\\';
   }

   if(cacheitem->locname[i]=='\\' && i+1>startfname)
    startfname=i+1;

  }//loop
  strupr(cacheitem->locname);
#endif

  //!!!msdos only ============================= (end)

  while(strlen(&cacheitem->locname[startfname])>STRINGSIZE-12)
   startfname++;

  if(!cacheitem->locname[startfname]) //file name ends with '\'!
  {
   char *ptr=configvariable(&ARACHNEcfg,"Index",NULL);
   if(!ptr)ptr="*.htm";
   strncat(cacheitem->locname,ptr,12);
   cacheitem->locname[startfname+12]='\0';
  }


  strcpy(cacheitem->mime,"file/");
  strncat(cacheitem->mime,&cacheitem->locname[startfname],STRINGSIZE-6);
  cacheitem->mime[STRINGSIZE-1]='\0';
  strlwr(cacheitem->mime);

#ifndef POSIX
#ifndef XTVERSION
  if(user_interface.vfat)
   getShortname(cacheitem->locname);
#endif
#endif

  strcpy(cacheitem->rawname,cacheitem->locname);

  //printf("local filename is : %s\n", cacheitem->locname);
  if(search_mime_cfg(cacheitem->mime, ext, dummy)==1) //convert only!
   *status=VIRTUAL;
  else
  {
   char ext[5];

   get_extension(cacheitem->mime,ext);
   if(!strstr("HTM TXT",ext) && !strstr(imageextensions,ext) && !file_exists(cacheitem->locname))
    return 0;
   else
    return 1;
  }
 }
 else if(!strcmpi(absURL->protocol,"mailto"))
 {
  sprintf(cacheitem->locname,"%s%ssendmail.ah",sharepath,GUIPATH);
  cacheitem->rawname[0]='\0';
  strcpy(cacheitem->mime,"text/html");
  return 1;
 }
 else if(!strcmpi(absURL->protocol,"about"))
 {
#ifdef POSIX
  if(absURL->file[0])
  {
   sprintf(cacheitem->mime,"file/%s",absURL->file);
   sprintf(cacheitem->locname,"%s%s",helppath,absURL->file);
  }
  else
  {
   sprintf(cacheitem->locname,"%sabout.html",helppath);
   strcpy(cacheitem->mime,"text/html");
  }
#else
  sprintf(cacheitem->locname,"%sabout.htm",exepath);
  strcpy(cacheitem->mime,"text/html");
#endif
  cacheitem->rawname[0]='\0';
  return 1;
 }
 else if(!strcmpi(absURL->protocol,"gui"))
 {
  sprintf(cacheitem->mime,"file/%s",absURL->file);
#ifdef POSIX
  sprintf(cacheitem->locname,"%sgui/%s",sharepath,absURL->file);
#else
  sprintf(cacheitem->locname,"%ssystem/gui/%s",sharepath,absURL->file);
#endif
  cacheitem->rawname[0]='\0';
  return 1;
 }
 else
  *status=REMOTE;

 if(GLOBAL.postdata || GLOBAL.reload)
  return 0;

 HTTPcache.cur=0;
 while(HTTPcache.cur<HTTPcache.len)
 {
#ifndef NOTCPIP
  Backgroundhttp();
#endif
  cacheptr=(struct HTTPrecord *)ie_getswap(HTTPcache.lineadr[HTTPcache.cur]);
  if(!cacheptr)
  {
   //cache index has been corrupted...
   MALLOCERR();
  }

  if(!strcmp(cacheptr->URL,cacheitem->URL))
  {
   char found=1;

   if(!quicksearch)
   {
    long t=time(NULL);
    if(cacheptr->dynamic &&
       (GLOBAL.redirection || (user_interface.expire_dynamic && t-cacheptr->lastseen>user_interface.expire_dynamic))
       && (tcpip || *status!=REMOTE)
       ||
       user_interface.expire_static &&
       t-cacheptr->lastseen>user_interface.expire_static &&
       !GLOBAL.isimage && (tcpip || *status!=REMOTE)
       || !file_exists(cacheptr->locname)) //=not found
     found=0;
    else if(tcpip || *status!=REMOTE)
    {
     cacheptr->lastseen=t;
//     cacheptr->size=ff.ff_fsize;
//     cacheptr->knowsize=1;
     swapmod=1;
    }
   }
   //zkopiroval jsem co jsem nasel
   memcpy(cacheitem,cacheptr,sizeof(struct HTTPrecord));
   *cacheitemadr=HTTPcache.lineadr[HTTPcache.cur];
   if(found) //nalezen soubor ?
    return 1;
   else
    return 0;
  }
  HTTPcache.cur++;
 }
 return 0;
}


void ResetURL(struct Url *url)
{
 url->protocol[0]='\0';
 url->user[0]='\0';
 url->password[0]='\0';
 url->host[0]='\0';
 url->file[0]='\0';
 url->port=80;
 url->kotva[0]='\0';
}


void removedotsfrompath(char *url)
{ char *s,*s1,*su;
  if(!url)
   return;
  su=strstr(url,"//");if(su)su+=2;else su=url;
  if(strncmp(su,"./",2)==0)memmove(su,su+2,strlen(su+1));
  if(strncmp(su,".\\",2)==0)memmove(su,su+2,strlen(su+1));
  url=su;
  do{s=strstr(url,"/./");if(!s)break;
     memmove(s,s+2,strlen(s+1));
    }while(s);
  do{s=strstr(url,"\\.\\");if(!s)break;
     memmove(s,s+2,strlen(s+1));
    }while(s);
  do{s=strstr(url,"/..");if(!s)break;
     s[0]=0;s1=strrchr(su,'/');if(!s1){s[0]='/';break;}
     s[0]='/';memmove(s1,s+3,strlen(s)+1);
    }while(s);
  do{s=strstr(url,"\\..");if(!s)break;
     s[0]=0;s1=strrchr(su,'\\');if(!s1){s[0]='\\';break;}
     s[0]='\\';memmove(s1,s+3,strlen(s)+1);
    }while(s);
}

void removespacesfrompath(char *path)
{
 if(strchr(path,' '))
 {
  char newpath[URLSIZE]="\0";
  int i=0,notmezera=0;
  char *ptr=path;

  while(i<URLSIZE-1 && *ptr)
  {
   if(*ptr==' ')
   {
    strcat(newpath,"%20");
    i+=2;
   }
   else
   {
    newpath[i]=*ptr;
    notmezera=i;
   }
   newpath[++i]='\0';
   ptr++;
  }
  newpath[notmezera+1]='\0';
  strcpy(path,newpath);
 }
}

void AnalyseURL(char *str,struct Url *url,int frame)
{
 char *strbuf;
 char *ptr,*strptr;
 struct Url *base;
 char isfile=0;

 strbuf=farmalloc(2*URLSIZE);
 if(!strbuf)memerr();

 if(frame!=0)
 {
  base=farmalloc(sizeof(struct Url));
  if(!base)memerr();
  if(frame==IGNORE_PARENT_FRAME)
   ResetURL(base);
  else if(frame==GLOBAL_LOCATION_AS_BASEURL)
   AnalyseURL(GLOBAL.location,base,IGNORE_PARENT_FRAME);
  else
   AnalyseURL(p->htmlframe[frame].cacheitem.URL,base,IGNORE_PARENT_FRAME);
 }
 else
  base=&baseURL;

 ResetURL(url);
 makestr(strbuf,str,URLSIZE-1);
 strptr=strbuf;

 //protokol
 ptr=strchr(strptr,':');
 if(ptr==NULL || ptr-strptr>10)//max protocol len=10
 {
  if(!strncmpi(strptr,"internal-gopher-",16)) //internal netscape shit...
  {
   strcpy(url->protocol,"file");   //internal-gopher-menu ->file:menu.ikn
   strcpy(url->file,&strptr[16]);
   strcat(url->file,".IKN");
   goto out;
  }

  noprotocol:
  strcpy(url->protocol,base->protocol);
  if(!url->protocol[0])//implicitni podoba URL, kdyz zadavam uplne nove...
  {
   strcpy(url->protocol,"http");
   memmove(&strptr[2],strptr,strlen(strptr)+1);
   strncpy(strptr,"//",2);
  }

  if(base->user[0] && !url->user[0])
   strcpy(url->user,base->user);

  if(base->password[0] && !url->password[0])
   strcpy(url->password,base->password);
 }
 else
 {
  ptr[0]='\0';
  if(strchr(strptr,'/'))
  {
   ptr[0]=':';
   goto noprotocol;
  }
  makestr(url->protocol,strptr,PROTOCOLSIZE-1);
  strptr=&ptr[1];
 }

 //kotva
 ptr=strchr(strptr,'#');
 if(ptr!=NULL)
 {
  ptr[0]='\0';
  makestr(url->kotva,&ptr[1],STRINGSIZE);
 }

 //hack pro DOS disky
 if(strlen(url->protocol)==1)
 {
  strcpy(url->file,url->protocol);
  strcpy(url->protocol,"file");
  strcat(url->file,":");
  strncat(url->file,strptr,URLSIZE-3);
  url->file[URLSIZE]='\0';
  goto out;
 }
 else
 if(!strcmpi(url->protocol,"mailto") ||
    !strcmpi(url->protocol,"file") ||
    !strcmpi(url->protocol,"reload") ||
    !strcmpi(url->protocol,"find") ||
//    !strcmpi(url->protocol,"freq") ||
//    !strcmpi(url->protocol,"fidonet") ||
    !strcmpi(url->protocol,"arachne") ||
    !strcmpi(url->protocol,"javascript") ||
    !strcmpi(url->protocol,"gui"))
 {
  makestr(url->file,strptr,URLSIZE-1);
  if(!strcmpi(url->protocol,"file")) isfile=1;

 }
 else
 {
  // =============================== PRIRAZENI STANDARTNICH PORTU ===========

  if(!strcmpi(url->protocol,"http"))
   url->port=80;
  else
  if(!strcmpi(url->protocol,"gopher"))
   url->port=70;
  else
  if(!strcmpi(url->protocol,"telnet"))
   url->port=23;
  else
  if(!strcmpi(url->protocol,"ftp"))
   url->port=21;
  else
  if(!strcmpi(url->protocol,"news"))
   url->port=119;
  else
  if(!strcmpi(url->protocol,"nntp"))
   url->port=119;
  else
  if(!strcmpi(url->protocol,"smtp"))
   url->port=25;
  else
  if(!strcmpi(url->protocol,"pop3"))
   url->port=110;
/*
  else
  if(!strcmpi(url->protocol,"irc"))
   url->port=6667;
*/
  else
  if(!strcmpi(url->protocol,"finger"))
   url->port=79;

  // ========================================================================

  if(!strncmp(strptr,"//",2))
  {
   //skip '//'
   strptr+=2;

   //skip to file section of URL
   ptr=strchr(strptr,'/');
   if(ptr)
   {
    makestr(url->file,ptr,URLSIZE-1);
    *ptr='\0';
   }
   else
   {
    strcpy(url->file,"/"); //root file
   }

   //get hostname
   ptr=strrchr(strptr,'@'); //username ?
   if(ptr)
   {
    char *p2;

    *ptr='\0';
    ptr++;

    p2=strrchr(strptr,':'); //password
    if(p2)
    {
     *p2='\0';
     makestr(url->password,&p2[1],PASSWORDSIZE-1);
    }

    makestr(url->user,strptr,STRINGSIZE-1);
   }
   else
    ptr=strptr;

   makestr(url->host,ptr,STRINGSIZE-1);

   ptr=strchr(url->host,':'); //port ?  2
   if(ptr)
   {
    ptr[0]='\0';
    url->port=atoi(&ptr[1]);
   }

  }
  else //pocitac nezadan
  {
   if(!strcmpi(url->protocol,"http") || !strcmpi(url->protocol,"ftp"))
   {
    strcpy(url->host,base->host);
    url->port=base->port;
   }
   else
    url->host[0]='\0';

   makestr(url->file,strptr,URLSIZE-1);
  }//end if

  // ========================================================================

 }//end protocol specific parsing

 if(strcmpi(url->protocol,"http") && !isfile &&
    strcmpi(url->protocol,"ftp")) goto out;

 //v url->file je ted akorat jmeno fajlu

 if(!url->file[0])//stejny soubor ?
  strncpy(url->file,base->file,URLSIZE-1);
 if(url->file[0]!='/' &&
    !(isfile && (url->file[1]==':' || url->file[0]=='\\')))//cesta k fajlu je ta stavajici ?
 {
  int i;

  i=strlen(base->file);
  while(i>0 && !(base->file[i-1]=='/' || isfile &&
                 base->file[i-1]=='\\'))
   i--;
  strncpy(strbuf,base->file,i);
  if(i<URLSIZE-1)
   strncpy(&strbuf[i],url->file,URLSIZE-1-i);

  makestr(url->file,strbuf,URLSIZE-1);
 }

 removedotsfrompath(url->file);
 if(!isfile)
  removespacesfrompath(url->file);

 out:
 if(base && base!=&baseURL)farfree(base);
 if(strbuf)farfree(strbuf);
}


void url2str(struct Url *url,char *out)
{
 char *outbuf;

 outbuf=farmalloc(2*URLSIZE);
 if(!outbuf)memerr();

 if(!strcmpi(url->protocol,"file") || !strcmpi(url->protocol,"mailto") ||
    !strcmpi(url->protocol,"about") || !strcmpi(url->protocol,"reload") ||
    !strcmpi(url->protocol,"news") || !strcmpi(url->protocol,"arachne") ||
    !strcmpi(url->protocol,"find") || !strcmpi(url->protocol,"gui") ||
    !strcmpi(url->protocol,"javascript") )
  sprintf(outbuf,"%s:%s",url->protocol,url->file);
 else
 if(!strcmpi(url->protocol,"finger") || !strcmpi(url->protocol,"telnet") )
 {
  if(url->user[0])
   sprintf(outbuf,"%s://%s@%s",url->protocol,url->user,url->host);
  else
   sprintf(outbuf,"%s://%s",url->protocol,url->host);
 }
 else
 {
  // do not descend beyond root directory !
  while(!strncmp(url->file,"/..",3))
  {
   memmove(url->file,&(url->file[3]),strlen(url->file)-3);
   url->file[strlen(url->file)-3]='\0';
  }

  if(url->user[0])
  {
   if(url->password[0])
    sprintf(outbuf,"%s://%s:%s@%s:%d%s",url->protocol,url->user,url->password,url->host,url->port,url->file);
   else
    sprintf(outbuf,"%s://%s@%s:%d%s",url->protocol,url->user,url->host,url->port,url->file);
  }
  else
  if(!strcmpi(url->protocol,"http") && url->port==80)
   sprintf(outbuf,"%s://%s%s",url->protocol,url->host,url->file);
  else
   sprintf(outbuf,"%s://%s:%d%s",url->protocol,url->host,url->port,url->file);
 }

 if(url->kotva[0])
 {
  strcat(outbuf,"#");
  strcat(outbuf,url->kotva);
 }

 makestr(out,outbuf,URLSIZE-1);
 if(outbuf)farfree(outbuf);
}

void add2history(char *URL)
{

  if (!URL) return;
//  printf("adding to history? GLOBAL.nothot=%d, arachne.scriptline=%d\n",
//   GLOBAL.nothot, arachne.scriptline);
  if(!GLOBAL.nothot && arachne.scriptline==0)
  {
   char *ptr1=ie_getline(&history,arachne.history+1);
   if(!ptr1 || strcmp(ptr1,URL))
   {
    char *ptr2=ie_getline(&history,arachne.history);
    if(!ptr2 || strcmp(ptr2,URL))
    //pridat do historie, pokud sem se pohnul
    {
//     printf("adding to history?\n");
     if(history.lines>=history.maxlines)
     {
      if(arachne.history>history.lines-1)
       arachne.history=history.lines-1;
      if(arachne.history==history.lines-1)//mazat historii na zacatku...
      {
       ie_delline(&history,0);
       if(arachne.history>1)
        arachne.history--;
      }
      else //... nebo na konci
       ie_delline(&history,history.lines-1);
     }
     ie_insline(&history,++arachne.history,URL);
    }
   }
   else
    if(arachne.history<history.maxlines)
     arachne.history++;
  }

 }//end sub


// ========================================================================
// Arachne WWW browser HTTP functions
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#ifdef POSIX
#ifdef LINUX
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#endif
#endif

#include "arachne.h"
#include "internet.h"

#define HTTP_ASLEEP        60  //timeout for "empty documents" (esp. via proxy)
			       //...for images, timeouts are divided by 2

struct Http_parameters http_parameters;

#ifndef LINUX
void find_keepalive_socket(char *hostname)
{
 if(GLOBAL.backgroundimages==BACKGROUND_EMPTY)
 {
  if(!strcmpi(hostname,sock_keepalive[1-socknum]) && !closing[1-socknum])
  {
   socknum=1-socknum;
   socket=sock[socknum];
   status=0;
  }
 }
}
#endif

int authenticated_http(struct Url *url,struct HTTPrecord *cache)
{
 longword host=0;
//!!JdS 2004/2/15 {
// char str[IE_MAXLEN+2];
 char str[MAXARGBUF+2], cookiestr[MAXARGBUF+2];  // The '+2' is for "\r\n"
//!!JdS 2004/2/15 }
 int count=0;
 char *ptr;
 char *querystring=NULL,*cachecontrol;
 char header_done=0;
 int postindex=0;
 char contentlength[80]="";
 char authorization[128]="";
 char acceptcharset[80]="";
// char cookiestr[2*IE_MAXLEN]="";  [JdS 2004/2/15]
 char *nocache="Cache-Control: no-cache\r\nPragma: no-cache\r\n";
 char *httpcommand="GET";
 int line;
 int ql=0;
 char *uri,pocitac[STRINGSIZE]; //uri = uniform resource identifier
 char referstr[URLSIZE+16]="";
 char portstr[10]="";
 int port,i;
 char ftp=0,alive=0;
 char willkeepalive=0;
 char *keepalive="\0";

//!!glennmcc: Dec 21, 2007 -- at Ray's suggestion, also write outgoing traffic
 char outgoing[1024]="\0";
//!!glennmcc: end

 if(!tcpip && !httpstub)return 0;
#ifdef MSDOS
 if(tcpip)
  free_socket();
#endif

//!!glennmcc: Dec 05, 2013 -- do not send blank Referer: in header
 if(http_parameters.referer && strlen(Referer)>0)
// if(http_parameters.referer)
  sprintf(referstr,"Referer: %s\r\n",Referer);

 if(!GLOBAL.isimage)
 {
  ptr = config_get_str("AcceptCharset", NULL);
  if(ptr)
  {
   sprintf(str,"Accept-Charset: %s\r\n",ptr);
   makestr(acceptcharset,str,79);
  }
 }

 //normal URL:
 strcpy(pocitac,url->host);
 port=url->port;
 uri=url->file;
 while(!strncmp(uri,"/..",3)) // do not descend beyond root directory !
  uri+=3;

 //use proxy server ?
 ftp=(toupper(url->protocol[0])=='F');

 if(ftp || http_parameters.useproxy)
 {
  char *no4all=NULL;

  if(ftp)
   ptr=NULL;
  else
  {
   ptr = config_get_str("NoProxy", NULL);
   no4all = config_get_str("NoProxy4all", NULL);
  }

  if( (!ptr || !strstr(strlwr(ptr),strlwr(pocitac)) ) &&
      (!no4all || !strstr(strlwr(pocitac), strlwr(no4all))) )
  {
   if(ftp)
    ptr = config_get_str("FTPproxy" ,NULL);
   else
    ptr = config_get_str("HTTPproxy", NULL);
   if(ptr)
   {
    makestr(pocitac,ptr,79);
    ptr=strrchr(pocitac,':');
    if(ptr)
    {
     *ptr++='\0';
     port=atoi(ptr);
    }
    else
     port=80;
    uri=cache->URL;
   }
  }
 }//end if proxy

 //this will appear in "Host:" http header field..
 if(url->port!=80)
  sprintf(portstr,":%d",url->port);

#ifndef LINUX
 find_keepalive_socket(pocitac);
#endif
 if(tcpip && !httpstub && strcmpi(sock_keepalive[socknum],pocitac))
 {
  GlobalLogoStyle=0;            //SDL set resolve animation

  if (atcp_resolve(pocitac, &host))
  {
    DNSerr(pocitac);
    return 0;
  }
 }

 i=0;
 cookiestr[0]='\0';  //!!JdS 2004/2/15
 strlwr(url->host);

/**** Start of superseded code [JdS 2004/3/2] ****
 while(i<cookies.lines)
 {
  ptr=ie_getline(&cookies,i);
  if(ptr)
  {
   strcpy(str,ptr);
   decompose_inetstr(str);

   if(getarg("domain",&ptr) && strstr(url->host,ptr)) // getarg() was getvar() [JdS 2004/1/30]
   {
    if(getarg("path",&ptr) && strstr(url->file,ptr)) // getarg() was getvar() [JdS 2004/1/30]
    {
//!!JdS 2004/2/15 {
//   if(strlen(cookiestr)+strlen(str)+10<2*IE_MAXLEN)
     if(strlen(cookiestr)+strlen(str)+10<MAXARGBUF)
//!!JdS 2004/2/15 }
     {
      if(cookiestr[0])
       strcat(cookiestr,"; ");
      else
       strcat(cookiestr,"Cookie: ");
      strcat(cookiestr,str);
     }
    }
   }
  }
  i++;
 }//loop
**** End of superseded code [JdS 2004/3/2] ****/

/**** Start of newer cookie code [JdS 2004/3/2] ****/
 while (i+CookieCrumbs<=cookies.lines)
 {
  //Piece together a cookie from crumbs in the 'cookies' jar
  ie_getcookie(str,&cookies,i);
  //If a cookie was found, check if it matches our domain and path
  if (str[0])
  {
   decompose_inetstr(str);
   if (getarg("domain",&ptr) && strstr(url->host,ptr))
   {
    if (getarg("path",&ptr) && strstr(url->file,ptr))
    {
     if (strlen(cookiestr)+strlen(str)+10<MAXARGBUF)
     {
      if (cookiestr[0])
       strcat(cookiestr,"; ");
      else
       strcat(cookiestr,"Cookie: ");
      strcat(cookiestr,str);
     }
    }
   }
  }
  i += CookieCrumbs;
 }//while
/**** End of newer cookie code [JdS 2004/3/2] ****/

 if(cookiestr[0])
 {
  strcat(cookiestr,"\r\n");
//  puts(cookiestr);
 }

 out:

 if(url->user[0] || AUTHENTICATION->flag>=AUTH_OK)
 {
  str[0]='\0';
  if(AUTHENTICATION->flag>=AUTH_OK)
   sprintf(authorization,"%s:%s",AUTHENTICATION->user,AUTHENTICATION->password);
  else
  {
   sprintf(authorization,"%s:%s",url->user,url->password);
   strcpy(AUTHENTICATION->user,url->user);
   strcpy(AUTHENTICATION->password,url->password);
  }

  base64code((unsigned char *)authorization,str);
  sprintf(authorization,"Authorization: Basic %s\r\n",str);
 }

 //this is experimental proxy authorization code !!!
 if(AUTHENTICATION->proxy)
 {
  char tmp[128];
  sprintf(str,"%s:%s",
   config_get_str("ProxyUsername", ""),
   config_get_str("ProxyPassword", ""));
  base64code((unsigned char *)str,tmp);
  sprintf(str,"Proxy-authorization: Basic %s\r\n",tmp);
  strcat(authorization,str);
 }
 //end experiment

// bad idea ?
// if(sock_keepalive[socknum][0] && !tcp_tick(socket)) //connection was lost ?
//  sock_keepalive[socknum][0]='\0';

 if(tcpip && !httpstub && strcmpi(sock_keepalive[socknum],pocitac))
 {
  retry:

  GlobalLogoStyle=2;    //SDL set connect animation

  // connect to server
  sprintf(str, MSG_CON, pocitac, port);
  outs(str);
#ifdef POSIX
  if (atcp_open((void *)&socknum, &host, port) < 0) {
    sprintf(str, MSG_ERRCON, pocitac);
    outs(str);
    return 0;
  }
#else
  if (atcp_open((void *)socket, &host, port) < 0) {
    sprintf(str, MSG_ERRCON, pocitac);
    outs(str);
    return 0;
  }

  // increase socket buffer for WATTCP
  // taken from previous implementation
  if (_SP > (1024 * SETBUFSIZE)) {
    unsigned char setbuf[1024 * SETBUFSIZE];
    sock_setbuf(socket, setbuf, sizeof(setbuf));
    user_interface.multitasking = MULTI_SAFE;
  }
#endif

 }//end if not TCP/IP open (or connection is alive)
 else
  alive=1;

#ifndef LINUX
 //initialize keepalive mechanism:
 if(http_parameters.keepalive)
  makestr(sock_keepalive[socknum],pocitac,STRINGSIZE);
 sock_datalen[socknum]=0;
#endif

 //SDL set data animation
 GlobalLogoStyle=1;

 //echo cookie string
 if(cookiestr[0])
  outs(cookiestr);
 else if(authorization[0])
  outs(authorization);
 else if(alive)
 {
  sprintf(str,MSG_ALIVE,pocitac,uri);
  outs(str);
  //printf("[%s]",str);
 }
 else
 {
  sprintf(str,MSG_REQ,pocitac,uri);
  outs(str);
 }

 //odesilam formular metodou POST ?
 //(metodu GET jsem uz zmaknul jinde...)
 // tr.: am I sending form by method POST ?
 //      (method GET I have already done elsewhere)

 if(GLOBAL.postdata==2) //method==POST
 {
  querystring=ie_getswap(GLOBAL.postdataptr);
  if(!querystring)
   MALLOCERR();
  ql=strlen(querystring);
  httpcommand="POST";

//the problem with CGI forms was "x-www-form-urlencoded"....
  sprintf(contentlength,"Content-type: application/x-www-form-urlencoded\r\nContent-length: %d\r\n",ql);
 }
 if(GLOBAL.reload || GLOBAL.postdata)
  cachecontrol=nocache;
 else
  cachecontrol="\0";

 {
//!!glennmcc: Sep 10, 2008 -- configurable useragent string
  char useragent[120];
  ptr = config_get_str("UserAgent", NULL);
  if(ptr) sprintf(useragent,"%s",ptr);
  if(!ptr || strlen(useragent)<10)
  sprintf(useragent,"xChaos_Arachne (DOS /5.%s%s)",VER,beta);
//!!glennmcc: end

//!!glennmcc: Aug 20, 2005
//don't include 'exestr' nor video settings in User-agent
/*
  char colordepth[10],*c="HiColor";
#ifdef HICOLOR
  if (xg_256!=MM_Hic)
  {
#endif
   sprintf(colordepth,"%dc",x_getmaxcol()+1);
   c=colordepth;
#ifdef HICOLOR
  }
#endif
*/
//!!glennmcc: end

 if(http_parameters.keepalive)
  keepalive="Connection: Keep-Alive\r\n";

//!!glennmcc: Jan 18, 2011 -- fix intermittent posting problems
if(querystring) sleep(1);//Piip();
//!!glennmcc: end

//!!glennmcc: Aug 20, 2005
//don't include 'exestr' nor video settings in User-agent
//User-agent: xChaos_Arachne/4.%s%s (%s; %dx%d,%s; www.arachne.cz)\r\n\
//removed as indicated by '^'________^^^^^^^^^^^^^^___
//also changed to 'version 5 type'
//also changed to my own web site address
 sprintf(p->buf,"\
%s %s HTTP/1.0\r\n\
User-agent: %s\r\n\
Accept: */*\r\n\
Host: %s%s\r\n\
%s%s%s%s%s%s%s\r\n",
     httpcommand,uri,
//!!glennmcc: Sep 10, 2008 -- configurable useragent string
//VER,beta,//moved above for configurable useragent string
// replaced with 'useragent'
useragent,

//   httpcommand,uri,VER,beta,exestr,x_maxx()+1,x_maxy()+1,c,
//removed as indicated by '^'_^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^___
//!!glennmcc: end
 url->host,portstr,
 keepalive,
 cachecontrol,
 contentlength,
 cookiestr,
 authorization,
 referstr,
 acceptcharset);
 }

//!!glennmcc: Dec 21, 2007 -- at Ray's suggestion, also write outgoing traffic
if(!outgoing[0])
 sprintf(outgoing,"<b>Sent to Server:</b>\r\n\r\n\
 %s<hr><b>Received from Server:</b>\r\n\r\n",p->buf);
//!!glennmcc: end

 if(tcpip && !httpstub)    //if TCP/IP is enabled
 {
#ifdef POSIX
  if (atcp_send((void *)&socknum, p->buf, strlen(p->buf)) < 0) //send HTTP reques....
  {
   outs(MSG_CLOSED);
   return 0;
  }
#else

  atcp_send(socket, p->buf, strlen(p->buf));
#endif

  if(querystring)           //if query string has to be posted
  {
   outs(MSG_POST);

#ifdef POSIX
   if (atcp_send((void *)&socknum, querystring, strlen(querystring)) < 0) //send HTTP reques....
   {
    outs(MSG_CLOSED);
    return 0;
   }
#else

//!!glennmcc Jul 16, 2005 -- fix intermitant posting problem
// by sending 16 byte chunks instead of 512 bytes
   while(postindex+16<ql)
// while(postindex+512<ql)
   {
    /*this is needed only for WATTCP*/
    while(sock_tbleft(socket)<16)      //SDL
//  while(sock_tbleft(socket)<512)      //SDL
    {
     sock_tick(socket,&status);
     xChLogoTICK(1); // animation of logo
     if(GUITICK())
      goto post_aborted;
    }

    querystring=ie_getswap(GLOBAL.postdataptr);
    if(!querystring)
     MALLOCERR();

    sock_tick(socket, &status ); //I shift TCP/IP
    sock_fastwrite(socket, (unsigned char *)&querystring[postindex] ,16);
//  sock_fastwrite(socket, (unsigned char *)&querystring[postindex] ,512);

    postindex+=16;
//  postindex+=512;
//!!glennmcc: end
   }//loop

   if(postindex<ql)
   {
    while(sock_tbleft(socket)<strlen(&querystring[postindex]))  //SDL
    {
     sock_tick(socket,&status);
     xChLogoTICK(1); // animation of logo
     if(GUITICK())
      goto post_aborted;
    }

    querystring=ie_getswap(GLOBAL.postdataptr);
    if(!querystring)
     MALLOCERR();
    sock_tick(socket, &status ); //I shift TCP/IP
    sock_fastwrite(socket, (unsigned char *)&querystring[postindex] ,strlen(&querystring[postindex]));
   }
   sock_tick(socket, &status ); //I shift TCP/IP
   atcp_send(socket, "\r\n", 2);
   sprintf(str,MSG_SENT,ql);
   outs(str);

   post_aborted:
   if(GLOBAL.gotolocation || GLOBAL.abort)
    goto abort;
#endif
  }//endif posting querystring...
 }//endif tcpip

//!!glennmcc: begin Oct 17, 2004 -- re-enable HTTPSTUB
//(this entire section had been commented-out)
// /*
#ifndef LINUX
 else //httpstub = non TCP/IP stuf ========================================
 {
  int f,l;
  char reqname[80];
  struct ffblk ff;

  //sprintf(timstr,"%ld",time(NULL));
  outs("Generating http/stub request...");
  strcpy(reqname,cache->locname);
  ptr=strrchr(reqname,'.');
  if(ptr)
   strcpy(ptr,".REQ");

  f=a_fast_open(reqname,O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
  if (f>=0)
  {
   write(f,p->buf ,strlen(p->buf));
//!!glennmcc Oct 17, 2004 -- commented-out to prevent compiler errors
/*
   if(*poststring)
   {
    querystring=ie_getswap(GLOBAL.postdataptr);
    if(!querystring)
     MALLOCERR();
    write(f, querystring,strlen(querystring));
   }
*/
   close(f);
  }
  outs("Waiting for http/stub answer...");

  ptr=strstr(reqname,".REQ");
  if(ptr)
   strcpy(ptr,".OK");

  do
  {
   l=0;

   while(l++<500)
   {
    xChLogoTICK(1); // animation of logo
    GUITICK();
   }

   if(GLOBAL.abort)
     return 0;
  }
  while(findfirst(reqname,&ff,0));

  f=a_fast_open(reqname,O_RDONLY|O_TEXT,0);
  if(f)
  {
   p->httplen=a_read(f,p->buf,BUF-1);
//!!glennmcc Oct 17, 2004 -- commented-out to prevent compiler errors
//   p->buf[httplen]='\0';
   close(f);
  }

  ptr=strstr(reqname,".OK");
  if(ptr)
   strcpy(ptr,".TMP");
  strcpy(cache->locname,reqname);

  goto analyse;
 } // ====================================================================
#endif
// */
//!!glennmcc: end Oct 17, 2004 -- re-enable HTTPSTUB

 //let's initialize this session.
 p->httplen=0;
 cache->size=0l;
 cache->knowsize=0;
 cache->dynamic=1;
 if(GLOBAL.isimage)
  strcpy(cache->mime,"image/gif");
 else
  strcpy(cache->mime,"text/html");
 p->buf[0]='\0';

 // READ HEADER:
 {
#ifndef POSIX
  int iddle=0;
#endif
  long timer=time(NULL),asleep;

  do
  {
#ifdef POSIX
   xChLogoTICK(10); // animation of logo
#else
   xChLogoTICK(1); // animation of logo
#endif
   asleep=time(NULL)-timer;
   if(GLOBAL.isimage)
    asleep*=2;
   if(asleep>HTTP_ASLEEP)
    goto abort;

   if(GUITICK())
   {
    if(GLOBAL.gotolocation || GLOBAL.abort)
     goto abort;
   }

#ifdef POSIX
   if (atcp_has_data(&socknum)) {
      count = atcp_recv(&socknum, &(p->buf[p->httplen]), BUF - p->httplen);
      if (count <= 0) {
         outs(MSG_CLOSED);
         return 0;
      }
   } else {
      count = 0;
   }

   p->httplen+=count;
   p->buf[p->httplen]='\0';
   if(strstr(p->buf,"\r\n\r\n") || strstr(p->buf,"\r\r") || strstr(p->buf,"\n\n") || p->httplen>=p->buf)
    header_done=1;
#else
   if (atcp_has_data(socket))
   {
    if(p->httplen+256<BUF)
    {
     count=atcp_recv(socket, &(p->buf[p->httplen]), 256);
     p->httplen+=count;
     p->buf[p->httplen]='\0';
     if(strstr(p->buf,"\r\n\r\n") || strstr(p->buf,"\r\r") || strstr(p->buf,"\n\n"))
      header_done=1;
    }
    else
    {
     count=atcp_recv(socket, str, 256);
     str[count]='\0';
     if(strstr(str,"\r\n\r\n") || strstr(str,"\r\r") || strstr(str,"\n\n"))
      header_done=1;
    }
    sprintf(str,MSG_READ,count);
    outs(str);
   }//endif
   else
    iddle++;

   if(iddle>1000 && !tcp_tick(socket))
   {
     sockmsg(1,socknum);
     header_done=1;
   }
#endif
  }
  while(!header_done);
 }

#ifndef POSIX
 goto analyse;

sock_err:

 sockmsg(status,socknum);
 if(p->httplen==0)
  return 0;

analyse:
#endif

 if(strncmp(p->buf,"HTTP",4) && p->buf[0] && p->httplen)
 {
  count=0;
  goto write2cache;
 }

 count=0;
 line=0;
 while(count<p->httplen)
 {
  if(p->buf[count]=='\n')
  {
//!!JdS 2004/3/7 {
// makestr(str,&(p->buf[line]),IE_MAXLEN);
   makestr(str,&(p->buf[line]),MAXARGBUF);
//!!JdS 2004/3/7 }
   ptr=strchr(str,'\r');
   if(ptr)*ptr='\0';
   ptr=strchr(str,'\n');
   if(ptr)*ptr='\0';

   // -------------------------------- empty line -> end of HTTP header

   if(!str[0] && !GLOBAL.redirection && AUTHENTICATION->flag!=AUTH_REQUIRED)
    goto write2cache;

//!!glennmcc: Mar 29, 2010 -- compensate for some servers not having a space
//between the ':' and the spec in the HTTP header
//such-as 'content-type:text/plain' instead of the correct
//format  'content-type: text/plain'
//   ptr=strstr(str,": ");//original line
   ptr=strstr(str,":");
//changed that line and changed several ocurrences below
//of ptr[2] to ptr[1+space]
   if(ptr)
   {
    int space=0;
    if(ptr[1]==' ') space=1;
    *ptr='\0';

    // ----------------------------------------------- Content-type:
    if(!strcmpi(str,"Content-type"))
    {
    makestr(cache->mime,&ptr[1+space],STRINGSIZE-1); /* including charset= */
    strlwr(cache->mime);

#ifdef EXP
  {
   char ext[5], *mimestr="\0", mime=0;
   get_extension(cache->URL,ext);

if(
   (
    strstr(cache->mime,"text/plain") ||
    strstr(cache->mime,"application/octet-stream")
   ) &&
    (
     !strcmpi(ext,"avi")  ||
     !strcmpi(ext,"f4v")  ||
     !strcmpi(ext,"flv")  ||
     !strcmpi(ext,"mp4")  ||
     !strcmpi(ext,"mpeg") ||
     !strcmpi(ext,"mpg")  ||
     !strcmpi(ext,"mov")  ||
     !strcmpi(ext,"ogg")  ||
     !strcmpi(ext,"oggv") ||
     !strcmpi(ext,"webm") ||
     !strcmpi(ext,"wmv")
    )
  )
   {
    Piip();
    strcpy(mimestr,"video/");
    mimestr[79]='\0';
    strncat(mimestr,ext,70);
    makestr(cache->mime,mimestr,STRINGSIZE-1);
    mime=1;
   }
if(!mime)
   makestr(cache->mime,&ptr[1+space],STRINGSIZE-1); /* including charset= */
   strlwr(cache->mime);
  }
#endif
    }

    // ----------------------------------------------- Content-length:

    else if(!strcmpi(str,"Content-length"))
    {
     cache->size=atol(&ptr[1+space]);
     cache->knowsize=1;
//!!glennmcc: July 08, 2006 -- only do a BGDL if it was
// specifcally requested via CTRL+Enter/CTRL+LeftClick
// or via Enter/LeftClick when size is not known
if(GLOBAL.backgr==2) GLOBAL.backgr=0;
//!!glennmcc: end
    }

    // ----------------------------------------------- Last-modified:

    else if(!strcmpi(str,"Last-modified"))
    {
     cache->dynamic=0;
    }

    // ----------------------------------------------- Connection:

    else if((!strcmpi(str,"Connection") || !strcmpi(str,"Proxy-Connection"))
	     && !strncmpi(&ptr[1+space],"Keep-Alive",10))
    {
     willkeepalive=1;
    }

    // ----------------------------------------------- Set-Cookie:

//!!Ray: Dec 18, 2007 -- some servers say simply 'cookie' instead
    else if((!strcmpi(str,"Set-Cookie") || !strcmpi(str,"Cookie"))
	    && http_parameters.acceptcookies)
//    else if(!strcmpi(str,"Set-Cookie") && http_parameters.acceptcookies)
//!!Ray: end
    {
//!!JdS 2004/2/15 {
//   char *pom1=NULL,*pom2=NULL,*p,*newcookie=NULL;
     char *p;
//!!JdS 2004/2/15 }
     char domain[80],path[80];

     outs(&ptr[2]);

//!!JdS 2004/2/15 {
//   // Allow for independent control of pom* & newcookie size [JdS 2004/1/17]
//   #define POMSIZE IE_MAXLEN
//   #define COOKIESIZE IE_MAXLEN
//   pom1=farmalloc(POMSIZE);         // Was (IE_MAXLEN) [JdS 2004/1/17]
//   pom2=farmalloc(POMSIZE);         // Was (IE_MAXLEN) [JdS 2004/1/17]
//   newcookie=farmalloc(COOKIESIZE); // Was (IE_MAXLEN) [JdS 2004/1/17]
//   if(!pom1 || !pom2 || !newcookie)
//    memerr();
//
//   makestr(pom1,&ptr[2],POMSIZE-1); // Was (,,IE_MAXLEN-1) [JdS 2004/1/17]
//   strcpy(newcookie,pom1); //its safe to call strcpy
//   decompose_inetstr(pom1);
     makestr(cookiestr,&ptr[2],MAXARGBUF-1);
     decompose_inetstr(&ptr[2]);
//!!JdS 2004/2/15 }

     if(!getarg("path",&p)) // getarg() was getvar() [JdS 2004/1/30]
     {
      //p=url->file;
//!!JdS 2004/2/15 {
//    joinstr(newcookie,COOKIESIZE,"; path=/"); // Was strcat() [JdS 2004/1/17]
      joinstr(cookiestr,MAXARGBUF,"; path=/");
//!!JdS 2004/2/15 }
     }

     makestr(path,p,79);

     if(!getarg("domain",&p)) // getarg() was getvar() [JdS 2004/1/30]
     {
//!!JdS 2004/2/15 {
//    joinstr(newcookie,COOKIESIZE,"; domain="); // Was strcat() [JdS 2004/1/17]
      joinstr(cookiestr,MAXARGBUF,"; domain=");
//!!JdS 2004/2/15 }
      if(GLOBAL.redirection)
      {
       struct Url newurl;
       AnalyseURL(GLOBAL.location,&newurl,GLOBAL_LOCATION_AS_BASEURL);
       makestr(domain,newurl.host,79);
       p=domain;
      }
      else
       p=url->host;

//!!JdS 2004/2/15 {
//    joinstr(newcookie,COOKIESIZE,p); // Was strcat() [JdS 2004/1/17]
      joinstr(cookiestr,MAXARGBUF,p);
//!!JdS 2004/2/15 }
     }

     if(p!=domain)
      makestr(domain,p,79);

/**** Start of superseded code [JdS 2004/3/3] ****
     cookies.y=0;
     while(cookies.y<cookies.lines)
     {
//!!JdS 2004/2/15 {
//    strcpy(pom2,ie_getline(&cookies,cookies.y));
//    decompose_inetstr(pom2);
      strcpy(str,ie_getline(&cookies,cookies.y));
      decompose_inetstr(str);
//!!JdS 2004/2/15 }

      getarg("domain",&p); // getarg() was getvar() [JdS 2004/1/30]
      if(strstr(domain,p) && getarg("path",&p)) // getarg() was getvar() [JdS 2004/1/30]
       if(!strcmp(path,p))
       {
//!!JdS 2004/2/15 {
//      p=strchr(pom2,'=');
//	if(p && !strncmp(newcookie,pom2,(int)(p-pom2)))
	p=strchr(str,'=');
	if(p && !strncmp(cookiestr,str,(int)(p-str)))
//!!JdS 2004/2/15 }
	{
	 //replace old cookie with new cookie:
	 ie_delline(&cookies,cookies.y);
//!!JdS 2004/2/15 {
//	 ie_insline(&cookies,cookies.y,newcookie);
	 ie_insline(&cookies,cookies.y,cookiestr);
//!!JdS 2004/2/15 }
	 goto cont;
	}
       }

      cookies.y++;
     } //while

     if(cookies.lines==cookies.maxlines)
      ie_delline(&cookies,0);

//!!JdS 2004/2/15 {
//   ie_insline(&cookies,cookies.lines,newcookie);
     ie_insline(&cookies,cookies.lines,cookiestr);
//!!JdS 2004/2/15 }
**** End of superseded code [JdS 2004/3/3] ****/

/**** Start of newer cookie code [JdS 2004/3/6] ****/
     cookies.y = 0;
     while (cookies.y+CookieCrumbs <= cookies.lines)
     {
      ie_getcookie(str,&cookies,cookies.y);
      decompose_inetstr(str);

      getarg("domain",&p);
      if(strstr(domain,p) && getarg("path",&p))
       if(!strcmp(path,p))
       {
	p = strchr(str,'=');
	if(p && !strncmp(cookiestr,str,(int)(p-str)))
	{
	 //replace old cookie with new cookie:
//	 ie_delcookie(&cookies,cookies.y);
//	 ie_inscookie(&cookies,cookies.y,cookiestr);
	 ie_putcookie(&cookies,cookies.y,cookiestr);
	 goto cont;
	}
       }

      cookies.y += CookieCrumbs;
     } //while

     if (cookies.lines == cookies.maxlines)
      ie_delcookie(&cookies,0);

     ie_inscookie(&cookies,cookies.lines,cookiestr);
/**** End of newer cookie code [JdS 2004/3/6] ****/

     cont:;
//!!JdS 2004/2/15 {
//   farfree(newcookie);
//   farfree(pom2);
//   farfree(pom1);
//!!JdS 2004/2/15 }

    }

    // ----------------------------------------------- WWW-authenticate
    else if(!strcmpi(str,"Proxy-authenticate"))
    {
     AUTHENTICATION->proxy=1;
    }
    else if(!strcmpi(str,"WWW-Authenticate"))
    {
     if(AUTHENTICATION->flag==AUTH_FORCED)
      AUTHENTICATION->host[0]='\0'; //reset authentication
     else
     {
      ptr+=2;
      while(*ptr==' ')ptr++;
      if(!strncmpi(ptr,"basic",5))
      {
       AUTHENTICATION->flag=AUTH_REQUIRED;
       ptr+=5;
       while(*ptr)
       {
	if(!strncmpi(ptr,"realm=",6))
	{
	 char *realm;
	 ptr+=6;
	 realm=ptr;
#ifdef LINUX
	 if(*ptr=="\"")
#else
	 if(*ptr=='\"')
#endif
	 {
	  ptr++;
	  realm=ptr;
#ifdef LINUX
	  while (*ptr && *ptr!="\"")ptr++;
#else
	  while (*ptr && *ptr!='\"')ptr++;
#endif
	  *ptr='\0';
	 }
	 if(!strcmpi(AUTHENTICATION->host,url->host) &&
	    !strcmp(AUTHENTICATION->realm,realm))
	   AUTHENTICATION->flag=AUTH_OK;
	 else
	 {
	  makestr(AUTHENTICATION->realm,realm,79);
	  strcpy(AUTHENTICATION->host,url->host);
	 }
	}
	ptr++;
       }//loop
      }
     }//endif
    }

    // ----------------------------------------------- Redirection:

    else if(!strcmpi(str,"Location"))
    {
     struct Url newurl;

//!!glennmcc: Feb 28, 2008 -- abort to prevent 'looping' when an image
//is sent with 'Location: ' (redirect with 'empty' location)
//this problem seems to be caused by 'expired' hit counter accounts
/*
     static int loop;
     loop++;
     if(loop>5) {loop=0; goto abort;}
*/

//!!Ray: Feb 29, 2008 -- much better than the 'loop counter method'
     if(!ptr[2])
{
//!!glennmcc: Feb 29, 2008 -- display error page
 GLOBAL.gotolocation=1;
 puts(cache->locname);
 strcpy(GLOBAL.location,"gui:err_red.ah");
 return 0;
}
//!!glennmcc: end

     outs(MSG_REDIR);
     url2str(url,GLOBAL.location);
     strcpy(Referer,GLOBAL.location);
     AnalyseURL(&ptr[2],&newurl,GLOBAL_LOCATION_AS_BASEURL); //(full length/text...)
     url2str(&newurl,GLOBAL.location);
     GLOBAL.redirection=1;
    }

   }
   line=count+1;
  }
  count++;
 }//loop


 if(GLOBAL.redirection || AUTHENTICATION->flag==AUTH_REQUIRED)
  goto abort;

write2cache:

 //HTTP/1.0 Connection: Keep-Alive rules according to RFC2068:
 //
 if(!cache->knowsize || !willkeepalive)
 {
  sock_keepalive[socknum][0]='\0';
 // printf("[keepalive disabled]");
 }
 else
 {
#ifndef LINUX
  closing[socknum]=0;
#endif
 // printf("[keepalive enabled]");
 }

//!!glennmcc: Feb 19-21, 2006 -- try again without keepalive
//MS-IIS says... "Bad Request"
//TUX erroneously says... "Not Found"
//(a 'real' 404 not found will have the date)
//therefore, "Not Found" without a date == TUX refused keepalive

if(http_parameters.keepalive &&
   (strstr(p->buf,"Bad Request") ||
    (strstr(p->buf,"Not Found") && !strstr(p->buf,"Date:"))
   )
  )
  {
   http_parameters.keepalive=0;
   return 0;
  }
//!!glennmcc: end

 ptr=strrchr(cache->locname,'.');
 if(ptr)
 {
  char httfile[80];
  int htt;

  if(!httpstub)
  {
   char ext[5];
   get_extension(cache->mime,ext);
   strcpy(&ptr[1],ext);
  }

  strcpy(cache->rawname,cache->locname);


//!!glennmcc: Feb 13, 2006 -- at Ray's suggestion,
// changed variable name to match the keyword
  if(user_interface.keephtt)
//  if(!user_interface.nohtt)
  {
   makehttfilename(cache->rawname,httfile);
   htt=a_fast_open(httfile,O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
   if(htt!=-1)
   {
    char *pom=farmalloc(4*URLSIZE);
    if(!pom)
     memerr();

    sprintf(pom,"<TITLE>%s %s</TITLE><PRE>\n",MSG_HTTP,cache->URL);
    write(htt,pom,strlen(pom));
//!!glennmcc: Dec 21, 2007 -- at Ray's suggestion, also write outgoing traffic
    write(htt,outgoing,strlen(outgoing));
//!!glennmcc: end
    write(htt,p->buf,count);
    ptr=strrchr(cache->locname,PATHSEP);
    if(ptr)
     ptr++;
    else
     ptr=cache->locname;
    sprintf(pom,"</PRE>\n<HR>URL: <A HREF=\"%s\">%s</A><BR>\n\
Local: <A HREF=\"file://%s%s\">%s</A><HR>\n\
",cache->URL,cache->URL,cachepath,ptr,cache->locname);
    write(htt,pom,strlen(pom));
    farfree(pom);
    a_close(htt);
   }//end if htt opened
  }//end if disable .HTT files
 }
 //otevreni souboru kam zapisu vlastni prenaseny soubor
 // tr.: open the file where I am going to write my own transferred file

 if(!httpstub)
 {
  cache->handle=a_fast_open(cache->locname,O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
//!!glennmcc: Nov 25, 2006 -- abort attempt if cache drive
//has too little space available
#ifdef LINUX
  if(cache->handle<0) // original line
#else
  if(cache->handle<0
     || lastdiskspace(cachepath)
	<cache->size)//user_interface.mindiskspace)
#endif
  {
//!!glennmcc: Nov 25, 2006 -- close and delete file if it was opened
//but cache drive had too little space available
#ifdef LINUX
  if(cache->handle<0) // original line
#else
  if(cache->handle>=0
     && lastdiskspace(cachepath)
     <cache->size)//user_interface.mindiskspace)
#endif
     {a_close(cache->handle); unlink(cache->locname);}//origianl line
//!!glennmcc: end
   GLOBAL.gotolocation=1;
   puts(cache->locname);
   strcpy(GLOBAL.location,"gui:err_disk.ah");
   return 0;
  }
 }

 if(strncmp(p->buf,"HTTP",4)) // ... p->buf[0 to 3]!="HTTP" ...!
  count=0;
 else
  p->httplen-=count+1;

 if(p->httplen<0)           //hack for header without CR-LF ?
  p->httplen=0;
 if(count>4)             //at least header HTTP/x.x_xxx_...
  memmove(p->buf,&(p->buf[count+1]),p->httplen);

 return 1;

 abort: /**** abort evevrything ****/

 p->buf[0]='\0';
 p->httplen=0;
#ifdef POSIX
 atcp_close((void *)&socknum);
#else
 atcp_close((void *)socket);
 closing[socknum]=1;
#endif
 sock_keepalive[socknum][0]='\0';
 return 0;
}


int openhttp(struct Url *url,struct HTTPrecord *cache)
{
 int ret;
 int origproxy=AUTHENTICATION->proxy;

 if(AUTHENTICATION->flag==AUTH_FORCED)
 {
  ret=authenticated_http(url,cache);
  AUTHENTICATION->flag=AUTH_UNDEFINED;
  GLOBAL.postdata=0;
  return ret;
 }

 AUTHENTICATION->flag=AUTH_UNDEFINED;

 ret=authenticated_http(url,cache);
 if(AUTHENTICATION->flag==AUTH_OK || AUTHENTICATION->proxy!=origproxy)
  ret=authenticated_http(url,cache);

 GLOBAL.postdata=0;
 return ret;
}


void closehttp(struct HTTPrecord *cache)
{
 if(cache->handle==-1)
  return;

 if(!sock_keepalive[socknum][0])
 {
#ifdef POSIX
  atcp_close(&socknum);
#else
  atcp_close(socket);
  closing[socknum]=1;
#endif
 }

 if(cache->handle!=-1)
 {
  a_close(cache->handle);
  cache->handle=-1;
 }
}


#ifndef POSIX
void free_socket(void)
{
 if(GLOBAL.backgroundimages==BACKGROUND_EMPTY)
  socknum=1-socknum;
 else
  socknum=1-GLOBAL.back_socknum;

 socket=sock[socknum];
 status=0;

 if(closing[socknum])
 {
  //printf("[aborting]");
  sock_abort(socket);
  sock_keepalive[socknum][0]='\0';
  memset(socket,0,sizeof(tcp_Socket));
 }

 return;

sock_err:
sockmsg(status,socknum);
}

int port=1023;
int locport(void)
{
 if(port>16000)
  port=1023;
 return ++port;
}
#endif

void Download(struct HTTPrecord *cache)
{
 long fpos=0;
 char dl[80],str[80];
 int rd=1, prc=0;

//!!glennmcc: Nov 17, 2004 -- include bytes/sec rate
long starttime=(int)(time(NULL)), elapsedtime=0, lastsec=0, bytesec=0;
//!!glennmcc: end

 if(cache->handle!=-1)
  fpos=a_filelength(cache->handle);

 while(rd>0)
 {
#ifdef POSIX
  rd=tickhttp(cache,p->buf,socknum);
#else
  rd=tickhttp(cache,p->buf,socket);
#endif
  fpos+=rd;

//!!glennmcc: Nov 26, 2006 -- abort download when file size is not known,
//and disk gets full during download

#ifndef LINUX
if(!cache->knowsize
   && lastdiskspace(cachepath)
      <(fpos+rd))// rd=-1;
      //!!glennmcc: Nov 26, 2006 -- disk filled during download
//   if(rd<0)
   {
    a_close(cache->handle);
    unlink(cache->locname);
    GLOBAL.gotolocation=1;
    strcpy(GLOBAL.location,"gui:err_disk.ah");
   }
//!!glennmcc: end
//!!glennmcc: end
#endif

  strcpy(dl,MSG_DOWNLD);

#ifndef POSIX
  if (!atcp_has_data(socket) || rd==0)
  {
#endif
   if (cache->knowsize && cache->size>100)
   {
//!!glennmcc:Oct 23, 2008 -- 'reversed the logic'
// to keep from overflowing at 21megs
    prc=(int)(fpos/(cache->size/100));
//  prc=(int)(100*fpos/cache->size);

//!!glennmcc: Nov 17, 2004 -- include bytes/sec rate
//!!glennmcc: Aug 20, 2005 -- restore original MSG_X_of_Y for use by 2nd image
//during parallel image download & use MSG_X_of_Y_byte for the 1st image only
elapsedtime=(int)(time(NULL))-(int)(starttime);
if(elapsedtime>lastsec && prc<=99 &&
 config_get_bool("UseByteSec", 0))
{
 lastsec++;
 bytesec=fpos/lastsec;
}
 sprintf(str,MSG_X_OF_Y_byte,dl,fpos,cache->size,bytesec);

if(lastsec==0 || prc>99 ||
 config_get_bool("UseByteSec", 0))
sprintf(str,MSG_X_OF_Y,dl,fpos,cache->size);//original line
//!!glennmcc: end

    outs(str);
    percentbar(prc);
    if(fpos>=cache->size)
     rd=0; //force connection close
   }
   else
   {
    sprintf(str,MSG_BYTESR,dl,fpos);
    outs(str);
   }
#ifndef POSIX
  }//endif
#endif

 }//loop
diskfull:;
}//end sub download()

//. current HTTP download will continue as "background task"
char GoBackground(struct HTTPrecord *cache)
{
 if(GLOBAL.backgroundimages!=BACKGROUND_EMPTY
 || user_interface.multitasking==MULTI_NO)
  return 0;

 if(p->httplen) //flush rest of header...
 {
  if(cache->handle!=-1)
   write(cache->handle,p->buf,p->httplen);
#ifndef LINUX
  sock_datalen[socknum]+=p->httplen;
#endif
  p->httplen=0;
 }

#ifndef POSIX
 GLOBAL.back_status=status;
 GLOBAL.back_socknum=socknum;
#endif
 GLOBAL.back_knowsize=cache->knowsize;
 GLOBAL.back_size=cache->size;
 GLOBAL.back_handle=cache->handle;
 GLOBAL.backgroundimages=BACKGROUND_SLEEPING;
 GLOBAL.back_iddle=0;
 Backgroundhttp();
 return 1;
}

void FinishBackground(char mode)
{
 Backgroundhttp();

 if(GLOBAL.backgroundimages==BACKGROUND_EMPTY)
  return;



 {
  struct HTTPrecord cacheitem;

  cacheitem.knowsize=GLOBAL.back_knowsize;
  cacheitem.size=GLOBAL.back_size;
  cacheitem.handle=GLOBAL.back_handle;
  GLOBAL.backgroundimages=BACKGROUND_EMPTY;

  socknum=GLOBAL.back_socknum;
#ifndef POSIX
  socket=sock[socknum];
  status=GLOBAL.back_status;
#endif

  if(mode!=BG_ABORT)
  {
#ifndef LINUX
   if(cacheitem.knowsize)
   {
    char str[256];
    sprintf(str,"%s (%d of %d)",MSG_PARALL,sock_datalen[socknum],cacheitem.size);
    outs(str);
   }
   else
   outs(MSG_PARALL);
#endif   
//!!glennmcc: Mar 08, 2006 -- fix intermittent freeze on final image
//when 'HTTPkeepAlive Yes' and 'Multitasking Yes'
#ifndef LINUX
    if(sock_datalen[socknum]<0)
       mode=BG_ABORT;
       else
#endif       
//!!glennmcc: end
   Download(&cacheitem);
  }
  else
  {
   a_lseek(cacheitem.handle,0l,SEEK_SET);
   write(cacheitem.handle,"??",2);
  }

  if(mode!=BG_FINISH) //BG_ABORT || BG_FINISH_ALL
   sock_keepalive[socknum][0]='\0';

  closehttp(&cacheitem);
 }
}

#ifndef NOTCPIP
#ifndef POSIX
void sockmsg(int status,int snum)
{
  char str[80];

  switch (status)
  {
   case 1 : /* foreign host closed */
   if(GLOBAL.backgroundimages==BACKGROUND_RUNNING)
   {
    sprintf(str,"%s%s)",MSG_BACKGR,MSG_CLOSED);
    outs(str);
   }
   else
    outs(MSG_CLOSED);
   break;
   case -1: /* timeout */
   sprintf(str,MSG_TCPERR, sockerr(sock[snum]));
   outs(str);
   break;
   default:
   sprintf(str,MSG_TCPILL, status);
   outs(str);
  }

  closing[snum]=0;
  sock_keepalive[snum][0]='\0';

}
#endif
#endif



// ========================================================================
// Arachne WWW browser HTTP functions
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#ifdef POSIX
#ifdef LINUX
#include <errno.h>
#elif defined (CLEMENTINE)
#include <clementine/errno.h>
extern int posixErrNo;
#define errno posixErrNo
#endif
#endif

#include "arachne.h"
#include "internet.h"

#define HTTP_QUICK_CONNECT 6   //first quick connect attempt (seconds)
#define HTTP_ASLEEP        60  //timeout for "empty documents" (esp. via proxy)
			       //...for images, timeouts are divided by 2

struct Http_parameters http_parameters;

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


char exestr[40]="\0";
void makeexestr(char *exestr);

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
 int delay=HTTP_QUICK_CONNECT, attempt=1;
#ifdef POSIX
 struct sockaddr_in sin;
 fd_set rfds,  efds;
 struct timeval tv;
#endif
 char willkeepalive=0;
 char *keepalive="\0";

 if(!tcpip && !httpstub)return 0;
#ifdef MSDOS
 if(tcpip)
  free_socket();
#endif

 /*create string with executable description - DOS, Linux, etc. Created only once*/
 if(!*exestr)
  makeexestr(exestr);

 if(http_parameters.referer)
  sprintf(referstr,"Referer: %s\r\n",Referer);

 if(!GLOBAL.isimage)
 {
  ptr=configvariable(&ARACHNEcfg,"AcceptCharset",NULL);
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
   ptr=configvariable(&ARACHNEcfg,"NoProxy",NULL);
   no4all=configvariable(&ARACHNEcfg,"NoProxy4all",NULL);
  }

  if( (!ptr || !strstr(strlwr(ptr),strlwr(pocitac)) ) &&
      (!no4all || !strstr(strlwr(pocitac), strlwr(no4all))) )
  {
   if(ftp)
    ptr=configvariable(&ARACHNEcfg,"FTPproxy",NULL);
   else
    ptr=configvariable(&ARACHNEcfg,"HTTPproxy",NULL);
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

 find_keepalive_socket(pocitac);
 if(tcpip && !httpstub && strcmpi(sock_keepalive[socknum],pocitac))
 {
  GlobalLogoStyle=0;            //SDL set resolve animation

/*
#ifdef POSIX
{ //blocking version - not necessary, if non-blocking reslove_fn() implemented in asockets.h works
 struct hostent *phe;            // host information entry

 if((phe = gethostbyname(pocitac)) == NULL)
 {
  if((host = inet_addr(pocitac)) < 0)
   host=0;
 }
 else
  host = *((longword *) phe->h_addr_list[0]);
} //end temporary DNS code
#else
*/
host=resolve_fn( pocitac, (sockfunct_t) TcpIdleFunc );    //SDL
//#endif

  if(!host)
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
          configvariable(&ARACHNEcfg,"ProxyUsername",NULL),
	  configvariable(&ARACHNEcfg,"ProxyPassword",NULL));
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

#ifdef POSIX
  sin.sin_addr.s_addr = host;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  bzero(&(sin.sin_zero), 8);     /* zero the rest of the struct */

  sprintf(str,msg_con,pocitac,port);
  outs(str);

  /* create socket */
  socknum=socket(PF_INET, SOCK_STREAM, 0);
  if(socknum < 0)
  {
   sprintf(str,msg_errcon,pocitac);
   outs(str);
   return 0;
  }

  /* make socket non-blocking */
  fcntl (socknum, F_SETFL, O_NONBLOCK);

  /* connect to server */
  while(connect(socknum, (struct sockaddr *)&sin, sizeof(sin)) < 0)
  {
   if(TcpIdleFunc())
    return 0;
  }

  /*
  //old style, synchrounous (blocking) connect. Not good for Arachne.
  if(connect(socknum, (struct sockaddr *)&sin, sizeof(sin)) < 0)
  {
   sprintf(str,msg_errcon,pocitac);
   outs(str);
   return 0;
  }
  */

#else
  status=tcp_open(socket, locport(), host, port, NULL );
  if (status!=1)
  {
   sprintf(str,msg_errcon,pocitac);
   outs(str);
   return 0;
  }

  sprintf(str,msg_con,pocitac,port);
  outs(str);
  if (_ip_delay0(socket, delay, (sockfunct_t) TcpIdleFunc, &status ))          //SDL
  {
   if(attempt==3)
    goto sock_err;
   else
   {
    if(attempt==2)
    {
     delay=sock_delay;
     if(GLOBAL.isimage)
      delay/=2;
    }
    attempt++;
    sock_abort(socket);
    goto retry;
   }
  }//wait for connection
#endif
 }//end if not TCP/IP open (or connection is alive)
 else
  alive=1;

 //initialize keepalive mechanism:
 if(http_parameters.keepalive)
  makestr(sock_keepalive[socknum],pocitac,STRINGSIZE);
 sock_datalen[socknum]=0;

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
  sprintf(contentlength,"\
Content-type: application/x-www-form-urlencoded\r\n\
Content-length: %d\r\n",ql);
 }
 if(GLOBAL.reload || GLOBAL.postdata)
  cachecontrol=nocache;
 else
  cachecontrol="\0";

 {
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

 if(http_parameters.keepalive)
  keepalive="Connection: Keep-Alive\n";

 sprintf(p->buf,"\
%s %s HTTP/1.0\r\n\
User-agent: xChaos_Arachne/4.%s%s (%s; %dx%d,%s; www.arachne.cz)\r\n\
Accept: */*\r\n\
Host: %s%s\r\n\
%s%s%s%s%s%s%s\r\n",
 httpcommand,uri,VER,beta,exestr,x_maxx()+1,x_maxy()+1,c,
 url->host,portstr,
 keepalive,
 cachecontrol,
 contentlength,
 cookiestr,
 authorization,
 referstr,
 acceptcharset);
 }

 if(tcpip && !httpstub)    //if TCP/IP is enabled
 {
#ifdef POSIX
  if(sock_puts(socknum, p->buf)<0) //send HTTP reques....
  {
   outs(MSG_CLOSED);
   return 0;
  }
#else
  sock_puts(socket, (unsigned char *)p->buf); //send HTTP reques....
#endif

  if(querystring)           //if query string has to be posted
  {
   outs(MSG_POST);

#ifdef POSIX
   if(sock_puts(socknum, querystring)<0) //send HTTP reques....
   {
    outs(MSG_CLOSED);
    return 0;
   }
#else
   while(postindex+512<ql)
   {
    /*this is needed only for WATTCP*/
    while(sock_tbleft(socket)<512)      //SDL
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
    sock_fastwrite(socket, (unsigned char *)&querystring[postindex] ,512);

    postindex+=512;
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
   sock_puts(socket, (unsigned char *)"\r\n");
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

#ifdef GGI
  IfRequested_ggiFlush();
#endif
#ifdef POSIX
   tv.tv_sec = 0;
   tv.tv_usec = 500;

   FD_ZERO (&rfds);
   FD_ZERO (&efds);
   FD_SET (socknum, &rfds);
   FD_SET (socknum, &efds);
   select (socknum+1, &rfds, NULL, &efds, &tv);


   if (FD_ISSET (socknum, &efds) && errno!=EINTR)
   {
     outs(MSG_CLOSED);
     return 0;
   }

   count=read(socknum, &(p->buf[p->httplen]),BUF-p->httplen);
   if(count<0)
   {
    if (errno != EAGAIN) {
           outs(MSG_CLOSED);
       return 0;
    }
    else count = 0;
   }

   p->httplen+=count;
   p->buf[p->httplen]='\0';
   if(strstr(p->buf,"\r\n\r\n") || strstr(p->buf,"\r\r") || strstr(p->buf,"\n\n") || p->httplen>=p->buf)
    header_done=1;
#else
   if (sock_dataready(socket ))
   {
    if(p->httplen+256<BUF)
    {
     count=sock_fastread(socket, (unsigned char *)&(p->buf[p->httplen]), 256);
     p->httplen+=count;
     p->buf[p->httplen]='\0';
     if(strstr(p->buf,"\r\n\r\n") || strstr(p->buf,"\r\r") || strstr(p->buf,"\n\n"))
      header_done=1;
    }
    else
    {
     count=sock_fastread(socket, (unsigned char *)str, 256);
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

   ptr=strstr(str,": ");
   if(ptr)
   {
    *ptr='\0';

    // ----------------------------------------------- Content-type:

    if(!strcmpi(str,"Content-type"))
    {
     makestr(cache->mime,&ptr[2],STRINGSIZE-1); /* including charset= */
     strlwr(cache->mime);
    }

    // ----------------------------------------------- Content-length:

    else if(!strcmpi(str,"Content-length"))
    {
     cache->size=atol(&ptr[2]);
     cache->knowsize=1;
    }

    // ----------------------------------------------- Last-modified:

    else if(!strcmpi(str,"Last-modified"))
    {
     cache->dynamic=0;
    }

    // ----------------------------------------------- Connection:

    else if((!strcmpi(str,"Connection") || !strcmpi(str,"Proxy-Connection"))
	     && !strncmpi(&ptr[2],"Keep-Alive",10))
    {
     willkeepalive=1;
    }

    // ----------------------------------------------- Set-Cookie:

    else if(!strcmpi(str,"Set-Cookie") && http_parameters.acceptcookies)
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

     cont:
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
         if(*ptr=='\"')
	 {
          ptr++;
	  realm=ptr;
          while (*ptr && *ptr!='\"')ptr++;
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

 //HTTP/1.0 Connection: Keep-Alive rules acording to RFC2068:
 //
 if(!cache->knowsize || !willkeepalive)
 {
  sock_keepalive[socknum][0]='\0';
 // printf("[keepalive disabled]");
 }
 else
 {
  closing[socknum]=0;
 // printf("[keepalive enabled]");
 }


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
  if(!user_interface.nohtt)
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

    write(htt,p->buf,count);

    ptr=strrchr(cache->locname,'\\');
    if(ptr)
     ptr++;
    else
     ptr=cache->locname;
    sprintf(pom,"\
</PRE>\n<HR>URL: <A HREF=\"%s\">%s</A><BR>\n\
Local: <A HREF=\"file:%s\">%s</A><HR>\n\
",cache->URL,cache->URL,ptr,cache->locname);

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
  if(cache->handle<0)
  {
   GLOBAL.gotolocation=1;
   puts(cache->locname);
   sprintf(GLOBAL.location,"file:%s%serr_disk.ah",sharepath,GUIPATH);
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
 close(socknum);
#else
 sock_close(socket );
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
  close(socknum);
#else
  sock_close(socket );
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
//!!glennmcc: Nov 21, 2004 -- added time remaining
long starttime=time(NULL), elapsedtime, heta, meta, seta;
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

  strcpy(dl,MSG_DOWNLD);

#ifndef POSIX
  if (!sock_dataready(socket ) || rd==0)
  {
#endif
   if (cache->knowsize && cache->size>0)
   {
    prc=(int)(100*fpos/cache->size);

//!!glennmcc: Nov 17, 2004 -- include bytes/sec rate
//!!glennmcc: Nov 21, 2004 -- added time remaining
elapsedtime=(time(NULL)-starttime);
if(elapsedtime>1 && prc<99)
{
//if(prc>5){ //uncomment to start calculating only after 5%
heta=(cache->size-fpos)/(fpos/(int)(elapsedtime))/3600;//hours
meta=((cache->size-fpos)/(fpos/(int)(elapsedtime))-(heta*3600))/60;
seta=(cache->size-fpos)/(fpos/(int)(elapsedtime))-(heta*3600)-(meta*60);
//}else{heta=0;meta=0;seta=0;} //uncomment to start calculating only after 5%
sprintf(str,MSG_X_OF_Y,dl,fpos,cache->size,fpos/(int)(elapsedtime),heta,meta,seta);
}
else
{
heta=0;meta=0;seta=0;
sprintf(str,MSG_X_OF_Y,dl,fpos,cache->size,fpos/1,heta,meta,seta);
}
//    sprintf(str,MSG_X_OF_Y,dl,fpos,cache->size);
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
}//end sub

//. current HTTP download will continue as "background task"
char GoBackground(struct HTTPrecord *cache)
{
 if(GLOBAL.backgroundimages!=BACKGROUND_EMPTY ||
    user_interface.multitasking==MULTI_NO)
  return 0;

 if(p->httplen) //flush rest of header...
 {
  if(cache->handle!=-1)
   write(cache->handle,p->buf,p->httplen);
  sock_datalen[socknum]+=p->httplen;
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
   if(cacheitem.knowsize)
   {
    char str[256];
    sprintf(str,"%s (%d of %d)",MSG_PARALL,sock_datalen[socknum],cacheitem.size);
    outs(str);
   }
   else
    outs(MSG_PARALL);
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


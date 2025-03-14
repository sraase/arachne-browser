
// ========================================================================
// Arachne WWW browser ""kernel" HTTP functions
// (c)1997,1998,1999 xChaos software
// ========================================================================


#include "arachne.h"
#include "internet.h"
#ifdef LINUX
#include <errno.h>
#endif

#define BACKBUF 2048
#define TCP_IDDLE 1000


//return value: number of bytes or 0...connection closed
#ifdef POSIX
int tickhttp(struct HTTPrecord *cache, char *buf, int sockfd)
#else
int tickhttp(struct HTTPrecord *cache, char *buf, tcp_Socket *socket)
#endif
{
 int count = 0;
 char closed=0;
 int iddle=0;

 if(p->httplen) //flush rest of header...
 {
  count=p->httplen;
#ifndef LINUX
  sock_datalen[socknum]+=count;
#endif  
  p->httplen=0;
  if(cache->handle!=-1)
   write(cache->handle,buf,count);
  Backgroundhttp();
  return count;
 }

 if(cache->knowsize)
 {
  if (cache->size<=0) return 0;
#ifndef LINUX
  if(sock_datalen[socknum]>=cache->size)
   return 0;
#endif   
 }
 else if (cache->handle==-1)
  return 0;

 while(!closed)
 {
  if(GUITICK())
   if(GLOBAL.gotolocation || GLOBAL.abort)
   {
    if(cache->handle!=-1)
    {
     a_close(cache->handle);
     cache->handle=-1;
    }
    return 0;
   }

#ifdef POSIX
  if (atcp_has_data(&sockfd)) {
    count = atcp_recv(&sockfd, buf, BUF);
    if (count <= 0) {
      closed = 1;
    }
  } else {
    count = 0;
  }
#else
  if (atcp_has_data(socket))
  {
   count=atcp_recv(socket, buf, BUF);
#endif
   if(count>0)
   {
#ifndef LINUX
    sock_datalen[socknum]+=count;
#endif    
    if(cache->handle!=-1)
     write(cache->handle,buf,count);
    Backgroundhttp();
    return count;
   }
#ifndef POSIX
  }
  else
  {
   xChLogoTICK(1); // animace loga + Backgroundhttp();
   if(iddle%TCP_IDDLE==0)
   {
    if(!tcp_tick(socket))
    {
     sock_keepalive[socknum][0]='\0';
     sockmsg(1,socknum);
     closed=1;
    }
   }
   iddle++;
  }//endif
#else
  xChLogoTICK(10); // animace loga
#endif
 }//loop

 return 0;
}


//.Background of a downloaded file ?   Used only by the online version
void Backgroundhttp(void)
{
 if(GLOBAL.backgroundimages!=BACKGROUND_SLEEPING)
  return;

 {
  int pushstatus=status;
  int count;
  char buffer[BACKBUF];
#ifdef POSIX
 fd_set rfds, efds;
 struct timeval tv;
 int closed=0;
#endif

  GLOBAL.backgroundimages=BACKGROUND_RUNNING; //"semafor"
  status=GLOBAL.back_status;

#ifdef POSIX
  fcntl (GLOBAL.back_socknum, F_SETFL, O_NONBLOCK);

  tv.tv_sec = 0;
  tv.tv_usec = 500;

  FD_ZERO (&rfds);
  FD_ZERO (&efds);
  FD_SET (GLOBAL.back_socknum, &rfds);
  FD_SET (GLOBAL.back_socknum, &efds);
  select (GLOBAL.back_socknum+1, &rfds, NULL, &efds, &tv);

  count=read(GLOBAL.back_socknum, buffer,BACKBUF);
#else
  if (atcp_has_data(sock[GLOBAL.back_socknum]))
  {
   count=atcp_recv(sock[GLOBAL.back_socknum], buffer, BACKBUF);
#endif
   if(count>0)
   {
#ifndef LINUX
    sock_datalen[GLOBAL.back_socknum]+=count;
#endif
    if(GLOBAL.back_handle!=-1)
    {
     char str[80];
     long fpos=a_filelength(GLOBAL.back_handle);
     write(GLOBAL.back_handle,buffer,count);
     GLOBAL.back_iddle=0;
     if (GLOBAL.back_knowsize && GLOBAL.back_size>100)
     {
//!!glennmcc:Oct 23, 2008 -- 'reversed the logic'
// to keep from overflowing at 21megs
      int prc=(int)(fpos/(GLOBAL.back_size/100));
//    int prc=(int)(100*fpos/GLOBAL.back_size);
//      sprintf(str,MSG_X_OF_Y_BASIC,MSG_BACKGR,fpos,GLOBAL.back_size); //JdS
    sprintf(str,MSG_X_OF_Y,MSG_BACKGR,fpos,GLOBAL.back_size);
      outs(str);
      percentbar(prc);
     }
     else
     {
      sprintf(str,MSG_BYTESR,MSG_BACKGR,fpos);
      outs(str);
     }
    }
    else
     goto sock_err;
   }
#ifdef POSIX
  if (count<0 ||
  (FD_ISSET (GLOBAL.back_socknum, &efds) && errno!=EINTR))  //socket error
   goto sock_err;
#else
   kbhit(); //to allow break (?)
  } //endif
  else
   GLOBAL.back_iddle++;

  if(GLOBAL.back_iddle>TCP_IDDLE)
   sock_tick( sock[GLOBAL.back_socknum], &status );

  GLOBAL.back_status=status;
  status=pushstatus;
#endif

  GLOBAL.backgroundimages=BACKGROUND_SLEEPING;
  return;

  sock_err:
#ifdef POSIX
  outs(MSG_CLOSE);
#else
  sockmsg(status,GLOBAL.back_socknum);
  //ukonceni
  GLOBAL.back_status=status;
  status=pushstatus;
  sock_keepalive[GLOBAL.back_socknum][0]='\0';

#endif

  if(GLOBAL.back_handle!=-1)
   a_close(GLOBAL.back_handle);
  GLOBAL.back_handle=-1;
  GLOBAL.backgroundimages=BACKGROUND_EMPTY;
 }
}

//========================================================================
/*not very important, just displays messages about restting WATTCP port */
//========================================================================

#ifndef POSIX
//typedef int (*procref)();

//reset specified port

char reset_detected;

dataHandler_t resetport(void)
{
 sock_abort(sock[1-socknum]);
 outs(MSG_IDENT);
 reset_detected=1;
 return 0;
}
#endif

/*
#define BUFLEN 1024

int webserver(void)
{
 char str[128];
 char uri[URLSIZE];
 char buffer[BUFLEN];
 int status;
 int f=-1,lenread,done;
 long length,fl;
 char pom[256];
 char *name=NULL,*ptr=NULL;

 sock_wait_input( websocket, sock_delay, NULL, &status );
 sock_gets( websocket, uri, sizeof( uri ));
 outs(buffer);

 while(isalnum(buffer[0]))
 {
  sock_wait_input( websocket, sock_delay, NULL, &status );
  sock_gets( websocket, buffer, sizeof( buffer ));
  outs(buffer);
 }

 if(!strncmp(uri,"GET ",4))
  ptr=strchr(uri,'/');
 if(ptr)
 {
  name=ptr+1;
  ptr=strchr(name,' ');
  if(ptr)
   *ptr='\0';
 }

 if(name)
  f=a_sopen(name,O_RDONLY|O_BINARY, SH_DENYNO, S_IREAD);

 if(f<0)
  sprintf(str,"HTTP/1.0 200 OK\r\n");
 else
  sprintf(str,"HTTP/1.0 404 Can't open file\r\n");


 sock_puts(websocket,str,strlen(str));

 if(f>=0)
 {
  fl=a_filelength(f);
  sprintf(str,"Content-length: %ld\r\n",fl);
  sock_puts(websocket,str,strlen(str));
 }

 sprintf(str,"Server: xChaos_Arachne/%s\r\n\r\n",VER);
 sock_puts(websocket,str,strlen(str));
 if(f<0)
 {
  sprintf(str,"<H1>Can't open requested file!</H1>");
  sock_puts(websocket,str,strlen(str));
  goto close;
 }

 lenread=done=0;
 length=0l;

 while(1)
 {
  sprintf(pom,MSG_UPLOAD,length,a_filelength(f));
  outs(pom);
  percentbar((int)(100*length/fl));
  lenread=a_read(f,buffer,BUFLEN);
  length+=lenread;
  if(lenread<=0)
   done=1;

  //wait until we can write to socket:
  while( (websocket->datalen) > 1024)
  {
   sock_tick(websocket,&status);
   //xChLogoTICK(1); // animace loga
   //if(GUITICK())
  }

  if(done)
   break;

  sock_fastwrite(websocket,buffer,lenread);
  sock_tick(websocket,&status);
 }//loop

 a_close(f);

close:

 a_sock_close( websocket );
 outs("Personal webserver: done");

//    sock_wait_closed( socket, sock_delay, NULL, &status );

sock_err:
 switch (status) {
     case 1 : // foreign host closed
	      break;
     case -1: // timeout
              sprintf(str,MSG_TCPERR, sockerr(websocket));
              outs(str);
	      break;
              }
 return 0;

}

*/

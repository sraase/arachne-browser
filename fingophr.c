
// ========================================================================
// Arachne WWW browser FTP functions
// (c)1997-1999 xChaos software
// ========================================================================

/******************************************************************************

    FINGER - display user/system information
    Copyright (C) 1991, University of Waterloo

******************************************************************************/

#include <stdio.h>
#include <io.h>
#include <string.h>
#include <fcntl.h>
#include <sys\stat.h>

#include "arachne.h"
#include "internet.h"

void xChLogoTICK(char Style); // animace loga
void xChLogo(char n);                  // vykresleni prazdneho loga
int  GUITICK(void);            // prekresleni mysi, vstup z klavesy, etc.
void memerr(void);
extern struct GLOBAL_FLAG GLOBAL;

extern char tcpip;
extern char closing[2];
extern char *msg_con;
extern char *msg_errcon;
extern char *msg_askdns;

int xfinger(struct Url *url, struct HTTPrecord *cache,char *selector)
//void finger(char *userid, longword host, char *hoststring)
{
// tcp_Socket fingersock;
 int status;
 int len=0;
 longword host;
 char str[256];
 char smallbuf[512];
 char *ptr;
 char ext[4];
 long total=0;

 if(!tcpip)return 0;
 free_socket();

 sprintf(str,msg_askdns,url->host);
 outs(str);
 GlobalLogoStyle=0;		//SDL set resolve animation
 host = resolve_fn(url->host, (sockfunct_t) TcpIdleFunc);	//SDL
// host = resolve(url->host);
 if(!host)
 {
  DNSerr(url->host);
  return 0;
 }

 GlobalLogoStyle=2;		//SDL set connect animation
 if (!tcp_open( socket, locport(), host, url->port, NULL ))
 {
  sprintf(str,msg_errcon,url->host);
  outs(str);
  return 0;
 }

 sprintf(str,msg_con,url->host,url->port);
 outs(str);
 sock_wait_established(socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		       &status);		//SDL
 GlobalLogoStyle=1;		//SDL set data animation

 if(selector[0])
 {
  sprintf(str,MSG_ASKING,selector);
  outs(str);
 }
 else
  outs(MSG_WTRPL);

 sprintf(str,"%s\r\n",selector);
 sock_puts( socket, (unsigned char *)str);

 ptr=strrchr(cache->locname,'.');
 if(ptr)
 {
  get_extension(cache->mime,ext);
  strcpy(&ptr[1],ext);
  strcpy(cache->rawname,cache->locname);
 }
// cache->httpresponse[0]='\0';
 cache->handle=a_open(cache->locname,O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);

 while ( 1 )
 {
  xChLogoTICK(1);
  if(GUITICK())
   if(GLOBAL.gotolocation || GLOBAL.abort)
    goto abort;
  if (sock_dataready( socket ))
  {
   len = sock_fastread( socket, (unsigned char*)smallbuf, 512 );
   if(cache->handle>=0)
    write(cache->handle,smallbuf,len);
   total+=len;
   sprintf(str,MSG_RDRPL,total);
  }

  sock_tick( socket, &status ); //posunu TCP/IP

 }

abort:
 sock_close( socket );
 closing[socknum]=1;
 sock_keepalive[socknum][0]='\0';
goto closed;
// sock_wait_closed( socket, sock_delay, NULL, &status );

sock_err:
sockmsg(status,socknum);

 closed:
 cache->size=a_filelength(cache->handle);
 cache->knowsize=1;
 if(cache->handle>=0)
  a_close(cache->handle);
 if(user_interface.logoiddle)
  xChLogo('0');

 return 1;
}


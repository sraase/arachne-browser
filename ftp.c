// ========================================================================
// Arachne WWW browser FTP functions
// (c)1997-1999 Michael Polak, Arachne Labs (xChaos software)
// ========================================================================

#include "arachne.h"
#include "internet.h"

#define BUFLEN 512

int ftpsession(struct Url *url,struct HTTPrecord *cache,char *uploadfile)
{
 longword host;
 char str[256];
 char buffer[BUFLEN+2];
 tcp_Socket datasocket;
 word dataport=0;
 int rv=0,len;
 char *ptr,*datahostptr,datahost[80];
 char isdir=0,retry=0;//,ascii=0;
 long total=0;

 if(!tcpip)return 0;
 free_socket();

 if((!url->file[0] || url->file[strlen(url->file)-1]=='/') && !uploadfile)
  isdir=1;

 sprintf(str,msg_askdns,url->host);
 outs(str);

 GlobalLogoStyle=0;			//SDL set resolve animation
 host=resolve_fn( url->host, (sockfunct_t) TcpIdleFunc ); //SDL
// host=resolve( url->host );
 if(!host)
 {
  DNSerr(url->host);
  return 0;
 }

 GlobalLogoStyle=2;			//SDL set connect animation
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
 sock_mode( socket, TCP_MODE_ASCII );
 outs(MSG_LOGIN);

 do
 {
  sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		   &status );		//SDL
  sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
  outs(buffer);
//  printf("FTP daemon said>");
//  puts(buffer);
  if ( *buffer != '2' && buffer[0]!=' ') goto quit;
 }
 while(buffer[3]=='-' || buffer[0]==' '); //continued message!

 if(!url->user[0])
  ptr="anonymous";
 else
  ptr=url->user;

 sprintf( str, "USER %s", ptr);
 sock_puts(socket,(unsigned char *)str);
 sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		  &status );		//SDL
 sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
 outs(buffer);
//  printf("FTP daemon said>");
//  puts(buffer);
 if ( *buffer != '3' ) goto quit;

 //open cache filename:

 if(uploadfile)
  strcpy(cache->locname,"FTP.LOG");
 else
 {
  ptr=strchr(cache->locname,'.');
  if(ptr)
  {
   strcpy(&ptr[1],"FTP");
   strcpy(cache->rawname,cache->locname);
  }
 }

 cache->handle=a_open(cache->locname,O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
 if(cache->handle<0)
  goto quit;

 strcpy(cache->mime,"text/plain");

 if(url->password[0])
  ptr=url->password;
 else
 {
  if(url->user[0] && !strcmp(url->host,AUTHENTICATION->host)
                  && !strcmp(AUTHENTICATION->realm,"$ftp"))
   ptr=AUTHENTICATION->password;
  else
  {
   ptr=configvariable(&ARACHNEcfg,"FakeFTPeMail",NULL);
   if(!ptr || !strchr(ptr,'@'))
   {
    ptr=configvariable(&ARACHNEcfg,"eMail",NULL);
    if(!ptr)
     ptr="@";
   }
  }
 }

 sprintf( str, "PASS %s", ptr);
 sock_puts(socket,(unsigned char *)str);
 do
 {
  sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
                   &status );		//SDL
  sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
  outs(buffer);
//  printf("FTP daemon said>");
//  puts(buffer);
  if ( *buffer != '2' && buffer[0]!=' ')
  {
   write(cache->handle,buffer,strlen(buffer));
   rv=1;
   goto quit;
  }
  else if ((buffer[3]=='-' || buffer[0]==' ') && (isdir || uploadfile))
  {
   strcat(buffer,"\r\n");
   rv=1;
   write(cache->handle,buffer,strlen(buffer));
  }
 }
 while(buffer[3]=='-' || buffer[0]==' '); //continued message!

 //ask server where we have to connect:

 sock_puts(socket,(unsigned char *)"PASV");
 sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		  &status );		//SDL
 sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
 outs(buffer);
//  printf("FTP daemon said>");
//  puts(buffer);

 //2xx Entering passive mode (a,b,c,d,x,y)

 if ( *buffer != '2' ) goto quit;
 datahostptr=strchr(buffer,'(');
 if(!datahostptr) goto quit;

 ptr=++datahostptr;
 {
  int carka=0;
  char *portptr=NULL;

  while(*ptr)
  {
   if(*ptr==',')
   {
    carka++;
    if(carka<4)
     *ptr='.';
    else if(carka==4)
    {
     *ptr='\0';
     portptr=ptr+1;
    }
    else if (carka==5)
    {
     *ptr='\0';
     dataport=256*(word)atoi(portptr);  // ,x,y -> 256*x+y
     portptr=ptr+1;
    }
   }
   else if(*ptr==')' && portptr)
   {
    *ptr='\0';
    dataport+=atoi(portptr);      // ,x,y -> 256*x+y
   }
   ptr++;
  }
 }

 if(!dataport)
  goto quit;

 makestr(datahost,datahostptr,79);

 retry:

 if(isdir)
 {
  if(url->file[0])
  {
   sprintf( str, "CWD %s", url->file);
   sock_puts(socket,(unsigned char *)str);
   do
   {
    sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
                     &status );		//SDL
    sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
    outs(buffer);
    if ( *buffer != '2' && buffer[0]!=' ')
    {
     write(cache->handle,buffer,strlen(buffer));
     rv=1;
     goto quit;
    }
    else if (buffer[3]=='-' || buffer[0]==' ')
    {
     strcat(buffer,"\r\n");
     rv=1;
     write(cache->handle,buffer,strlen(buffer));
    }
   }
   while(buffer[3]=='-' || buffer[0]==' '); //continued message!
  }
  strcpy(cache->mime,"ftp/list");
  sprintf( str, "LIST");
 }
 else
 {
  char *fnameptr;
  char mimestr[80]="ftp/binary";

  fnameptr=strrchr(url->file,'/');
  if(!fnameptr)
  {
   fnameptr=strrchr(url->file,'\\');
   if(!fnameptr)
    fnameptr=url->file;
   else
    fnameptr++;
   }
   else
    fnameptr++;

  sprintf( str, "TYPE I");
  if(fnameptr)
  {
   char ext[5];
   strcpy(mimestr,"file/");
   strncat(mimestr,fnameptr,70);
   mimestr[79]='\0';
   get_extension(mimestr,ext);
   if(!strncmpi(ext,"TXT",3) || !strncmpi(ext,"HTM",3))
   {
    sprintf( str, "TYPE A");
//    ascii=1;
   }
  }
  strcpy(cache->mime,mimestr);

  sock_puts(socket,(unsigned char *)str);
  sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		  &status );		//SDL
  sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
  outs(buffer);
//  printf("FTP daemon said>");
//  puts(buffer);
  if ( *buffer != '2' || uploadfile)
  {
   strcat(buffer,"\n");
   write(cache->handle,buffer,strlen(buffer));
  }
  if ( *buffer != '2' )
  {
   rv=1;
   goto quit;
  }
  if(!uploadfile)
   sprintf( str, "RETR %s", url->file);
  else
   sprintf( str, "STOR %s", url->file);
 }

 sock_puts(socket,(unsigned char *)str);

 if(!retry)
 {
  //get file using datahost & dataport
  GlobalLogoStyle=0;		//SDL set resolve animation
  host=resolve_fn( datahost, (sockfunct_t) TcpIdleFunc );	//SDL
//  host=resolve( datahost );
  if(!host)
   goto quit;

  GlobalLogoStyle=2;		//SDL set connect animation
  if (!tcp_open( &datasocket, locport(), host, dataport, NULL ))
  {
   sprintf(str,msg_errcon,datahost);
   outs(str);
   goto quit;
  }
  sprintf(str,msg_con,datahost,dataport);
  outs(str);

  //wait for datasocket to open:
  sock_wait_established(&datasocket, sock_delay, (sockfunct_t) TcpIdleFunc,
			&status);	//SDL
  GlobalLogoStyle=1;		//SDL set data animation
 }

 //wait for "110 openning connection" (or "550 ....error....")
 sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		  &status );		//SDL

 sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
 outs(buffer);
// printf("FTP daemon said>");
// puts(buffer);
 if ( *buffer != '1' || uploadfile)
 {
  strcat(buffer,"\n");
  write(cache->handle,buffer,strlen(buffer));
 }

 if ( *buffer != '1' )
 {
  if(!strncmp(buffer,"550",3) && !retry)
  {
   retry=1;
   isdir=1-isdir;
   if(isdir)
    strcat(url->file,"/");
   else
   {
    int i=strlen(url->file);
    if(i>0 && url->file[i-1]=='/')
     url->file[i-1]='\0';
   }
   goto retry;
  }

  strcpy(cache->mime,"text/plain");
  rv=1;
  goto dataquit;
 }

 if(!uploadfile) //-------------------------------------- download ---------------
 {
  while ( 1 )
  {
   xChLogoTICK(1);
   if(GUITICK())
    if(GLOBAL.gotolocation || GLOBAL.abort)
     goto dataquit;

   if (sock_dataready( &datasocket ))
   {
    len = sock_fastread( &datasocket, (unsigned char*)buffer, BUFLEN );
    write(cache->handle,buffer,len);
    total+=len;
    sprintf(str,MSG_BYTESR,MSG_DOWNLD,total);
    outs(str);
   }
   else
    sock_tick( &datasocket, &status ); //posunu TCP/IP
  }
 }
 else //-------------------------------------- upload ------------------
 {
  int f,lenread,done;
  long length;
  char pom[256];

/*  if(ascii)
   f=a_sopen(uploadfile,O_RDONLY|O_TEXT, SH_DENYNO, S_IREAD);
  else*/
   f=a_sopen(uploadfile,O_RDONLY|O_BINARY, SH_DENYNO, S_IREAD);

  if(f<0)
   goto dataquit;

  lenread=done=0;
  length=0l;

  {
   long filel=a_filelength(f);
   while(1)
   {
    sprintf(pom,MSG_UPLOAD,length,filel);
    outs(pom);
    percentbar((int)(100*length/filel));
    lenread=a_read(f,buffer,BUFLEN);
    length+=lenread;
    if(lenread<=0)
     done=1;

    //wait until we can write to socket:
    while(sock_tbleft(&datasocket)<lenread) //SDL
//    while( datasocket.datalen > 1024)
    {
     sock_tick(&datasocket,&status);
     xChLogoTICK(1); // animace loga
     if(GUITICK())
      goto dataquit;
    }

    if(done)
    {
     sock_close( &datasocket );
     a_close(f);
     goto dataclose;
    }

    sock_fastwrite(&datasocket,(unsigned char *)buffer,lenread);
    sock_tick(&datasocket,&status);
   }//loop
  }
 }


dataquit:
 sock_abort( &datasocket );

dataclose:
 outs(MSG_CLOSE);
 sock_wait_closed( &datasocket, sock_delay, (sockfunct_t) TcpIdleFunc,
		   &status );		//SDL

 if(uploadfile)
 {
  sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		   &status );		//SDL
  sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
  outs(buffer);
 }



quit:
  sock_puts(socket,(unsigned char *)"QUIT");
  sock_close( socket );
  closing[socknum]=1;
  sock_keepalive[socknum][0]='\0';

//    sock_wait_closed( socket, sock_delay, NULL, &status );

//we will better wait because we are about to deallocate datasocket

sock_err:
    switch (status) {
	case 1 : /* foreign host closed */
		 break;
	case -1: /* timeout */
                 sprintf(str,MSG_TCPERR, sockerr(socket));
                 outs(str);
		 break;
    }

 if(total)
 {
  cache->knowsize=1;
  cache->size=total;
  rv=1;
 }
 if(cache->handle>=0)
 {
  a_close(cache->handle);
  rv=1;
 }

 return rv;
}
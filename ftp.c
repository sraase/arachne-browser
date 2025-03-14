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

//!!glennmcc: Nov 11, 2007 -- for 'dblp code' below
 int dblp=0;
//!!glennmcc: end

//!!glennmcc: Nov 13, 2007 -- for EZNOS2 fix below
int eznos2=0;
//!!glennmcc: end
int log;

 if(!tcpip)return 0;
 free_socket();

 if((!url->file[0] || url->file[strlen(url->file)-1]=='/') && !uploadfile)
  isdir=1;

 sprintf(str,msg_askdns,url->host);
 outs(str);

 GlobalLogoStyle=0;			//SDL set resolve animation
 if (atcp_resolve(url->host, &host))
 {
  DNSerr(url->host);
  return 0;
 }

// if(!uploadfile) //!glennmcc: Oct 20, 2012 - commented-out to make upload log more verbose
   log=a_open("FTP.LOG",O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);

 GlobalLogoStyle=2;			//SDL set connect animation
 if (atcp_open(socket, &host, url->port)) {
  sprintf(str,msg_errcon,url->host);
  outs(str);
  return 0;
 }
 sprintf(str,msg_con,url->host,url->port);
 outs(str);
 write(log,str,strlen(str));
 write(log,"\r\n",2);

 GlobalLogoStyle=1;		//SDL set data animation
 sock_mode( socket, TCP_MODE_ASCII );
 outs(MSG_LOGIN);

 do
 {
  sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL
  sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
  outs(buffer);
  write(log,buffer,strlen(buffer));
  write(log,"\r\n",2);


//  printf("FTP daemon said>");
//  puts(buffer);
  if ( *buffer != '2' && buffer[0]!=' ') goto quit;
 }
 while(buffer[3]=='-' || buffer[0]==' '); //continued message!

 if(!url->user[0])
  ptr="anonymous";
 else
  ptr=url->user;

 sprintf( str, "USER %s\r\n", ptr);
 write(log,str,strlen(str));
 atcp_send(socket, str, strlen(str));
 sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL
 sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
 outs(buffer);
 write(log,buffer,strlen(buffer));
 write(log,"\r\n",2);

//  printf("FTP daemon said>");
//  puts(buffer);

//!!glennmcc: May 11, 2005
//removed due to the fact that not all sites use '3'
//see additional info below with respect to anonymous password
// if ( *buffer != '3' ) goto quit;
//!!glennmcc: end

 //open cache filename:

//!glennmcc: Oct 20, 2012 - commented-out to make upload log more verbose
// if(uploadfile)
//  strcpy(cache->locname,"FTP.LOG");
// else
//!glennmcc end: Oct 20, 2012

 {
//!!glennmcc: Oct 22, 2008 -- strchr() was preventing the use of 'CachePath .\cache\'
//  ptr=strchr(cache->locname,'.');
  ptr=strrchr(cache->locname,'.');
//!!glennmcc: end
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
   ptr = config_get_str("FakeFTPeMail", NULL);
   if(!ptr || !strchr(ptr,'@'))
    ptr = config_get_str("eMail", "");
  }
 }

//!!glennmcc: May 11, 2005
//some sites do not require a password after 'anonymous'
//therefer, this entire block is now within this 'if()' so that
//the password (email address), will only be sent if asked for
//!!glennmcc: Nov 13, 2007 -- EZNOS2 says "Enter PASS command"
if (strstr(buffer,"sword") || strstr(buffer,"Enter PASS command"))
//if (strstr(buffer,"sword"))//original line
{
 sprintf( str, "PASS %s\r\n", ptr);
 atcp_send(socket, str, strlen(str));
 write(log,str,strlen(str));
}//!!glennmcc: inserted Mar 02, 2008
//Some servers need the following 'do/while' section,
//therefore, only the section above for sending the password needs to be
//'blocked' when no password was requested.

 do
 {
  sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL
  sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
  outs(buffer);
  write(log,buffer,strlen(buffer));
  write(log,"\r\n",2);
//  printf("FTP daemon said>");
//  puts(buffer);
if (strstr(buffer,"Enter PASS command")) eznos2=1;

  if (*buffer != '2' && buffer[0]!=' ' && !eznos2)
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
 while(buffer[3]=='-' || buffer[0]==' ' || buffer[0]=='3'); //continued message!
//}//!!glennmcc: end May 11, 2005 -- removed on Mar 02, 2008

 //ask server where we have to connect:
 atcp_send(socket, "PASV\r\n", 6);
 sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL
 sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
 outs(buffer);
 write(log,buffer,strlen(buffer));
 write(log,"\r\n",2);



//  printf("FTP daemon said>");
//  puts(buffer);

 //2xx Entering passive mode (a,b,c,d,x,y)

 if ( *buffer != '2' ) goto quit;
 datahostptr=strchr(buffer,'(');
//!!glennmcc: Nov 13, 2007 -- EZNOS2 doesn't enclose the info in ()
//therefore, if '(' is not found... look for the last 'space'.
 if(!datahostptr) {datahostptr=strrchr(buffer,' '); eznos2=1;}
//if that still fails... 'quit'
//!!glennmcc: end
 if(!datahostptr) goto quit;//original line

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
//!!glennmcc: Nov 13, 2007 -- part of above fix for EZNO2 info not in ()
 if(eznos2) dataport+=atoi(portptr);      // ,x,y -> 256*x+y
//!!glennmcc: end
    }
   }
   else if(*ptr==')' && portptr)
   {
//!!glennmcc: Nov 11, 2007 -- some servers have double ')'
// at the end of the port address info...
// this 'dblp code' will prevent that from adding the final set twice
//eg: (99,167,219,186,234,255)) instead of.... (99,167,219,186,234,255)
//without this fix ... 255 gets added a 2nd time and
//we end-up-with... port 60414 instead of the correct port of 60159
    *ptr='\0';
    if(!dblp)//!!glennmcc: Nov 11, 2007
    dataport+=atoi(portptr);      // ,x,y -> 256*x+y
    dblp=1;//!!glennmcc: Nov 11, 2007
   }
   ptr++;
  }
 }

 if(!dataport)
  goto quit;

//!!glennmcc: Aug 31, 2009
//EZNOS2 sends the IP of the machine on a router as datahost
//therefore we need to go back to the original host
if(eznos2)
{
makestr(datahost,url->host,79);
outs(datahost);
Piip();
}
else
//!!glennmcc:end

 makestr(datahost,datahostptr,79);//original line

 retry:

 if(isdir)
 {
  if(url->file[0])
  {
//!!glennmcc: Oct 15, 2007 -- fix problems with CWD on FTP servers
//which interpret the leading '/' as an attempted CD to 'root'
   if(url->file[0]=='/')
   sprintf( str, "CWD %s\r\n", url->file+1);
   else
//!!glennmcc: end Oct 15, 2007
   sprintf( str, "CWD %s\r\n", url->file);
   atcp_send(socket, str, strlen(str));
   do
   {
    sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL
    sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
    outs(buffer);

//!!glennmcc: Apr 08, 2005 -- commented-out this block
// to fix the problem of 'broken dir listing' when the
// 'FTP welcome message' contains linefeeds
// such as at ftp://ftp.cdrom.com/.2/simtelnet/
/*
    if ( *buffer != '2' && buffer[0]!=' ')
    {
     write(cache->handle,buffer,strlen(buffer));
     rv=1;
     goto quit;
    }
    else if (buffer[3]=='-' || buffer[0]==' ')
*/
//!!glennmcc: end
    {
     strcat(buffer,"\r\n");
     rv=1;
     write(cache->handle,buffer,strlen(buffer));
    }
   }
//!!glennmcc: Apr 08, 2005 -- added a test for !=' ' which is also
// needed for the same fix at ftp://ftp.cdrom.com/.2/simtelnet/
   while(buffer[3]=='-' || buffer[3]!=' ' || buffer[0]==' '); //continued message!
//   while(buffer[3]=='-' || buffer[0]==' '); //continued message!
//!!glennmcc: end

  }
  strcpy(cache->mime,"ftp/list");
  sprintf( str, "LIST\r\n");
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

  sprintf( str, "TYPE I\r\n");
  if(fnameptr)
  {
   char ext[5];
   strcpy(mimestr,"file/");
   strncat(mimestr,fnameptr,70);
   mimestr[79]='\0';
   get_extension(mimestr,ext);
   if(!strncmpi(ext,"TXT",3) || !strncmpi(ext,"HTM",3))
   {
//!!glennmcc: begin June 09, 2002
//optionally upload TXT and HTM in binary mode
    if (config_get_bool("UseBinaryFTP", 0))
//!!glennmcc: end
    sprintf( str, "TYPE A\r\n");
//    ascii=1;
   }
  }
  strcpy(cache->mime,mimestr);

  atcp_send(socket, str, strlen(str));
  write(log,str,strlen(str));
  sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL
  sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
  outs(buffer);
  write(log,buffer,strlen(buffer));
  write(log,"\r\n",2);
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

//!!glennmcc: Oct 17, 2007 -- fix problems with FTP servers
//which interpret the leading '/' as an attempted CD to 'root'
   if(url->file[0]=='/')
   sprintf( str, "RETR %s\r\n", url->file+1);
   else
   sprintf( str, "RETR %s\r\n", url->file);
//original single line above this comment
//!!glennmcc: end
  else
   sprintf( str, "STOR %s\r\n", url->file);
 }
 atcp_send(socket, str, strlen(str));
 write(log,str,strlen(str));

//!!glennmcc: Oct 19, 2008 -- back to original fix
//!!glennmcc: Nov 15, 2007 -- always 'close' the connection when done
//with both dir listings and file downloads
//Apr 10, 2007 fix did it only for dir listings
if(isdir || strstr(str,"RETR"))
//if(isdir)
 atcp_send(socket, "QUIT\r\n", 6);
//!!glennmcc: end

 if(!retry)
 {
  //get file using datahost & dataport
  GlobalLogoStyle=0;		//SDL set resolve animation
  if (atcp_resolve(datahost, &host))
   goto quit;

  GlobalLogoStyle=2;		//SDL set connect animation
  if (atcp_open(&datasocket, &host, dataport)) {
   sprintf(str,msg_errcon,datahost);
   outs(str);
   goto quit;
  }
  sprintf(str,msg_con,datahost,dataport);
  outs(str);
  write(log,str,strlen(str));
  write(log,"\r\n",2);

//!!glennmcc: Sep 27, 2008 -- increase D/L speed on cable & DSL
//many thanks to 'mik' for pointing me in the right direction. :)
{
#ifdef DEBUG
 char sp[80];
 sprintf(sp,"Available stack = %u bytes",_SP);
 outs(sp);
 Piip(); Piip();
#endif
 if(_SP>(1024*SETBUFSIZE))
 {
  char setbuf[1024*SETBUFSIZE];
  sock_setbuf(&datasocket, (unsigned char *)setbuf, 1024*SETBUFSIZE);
 }
}
//!!glennmcc: end

  GlobalLogoStyle=1;		//SDL set data animation
 }
 //wait for "110 openning connection" (or "550 ....error....")
 sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL

 sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
 outs(buffer);
 write(log,buffer,strlen(buffer));
 write(log,"\r\n",2);


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

   if (atcp_has_data(&datasocket))
   {
    len = atcp_recv(&datasocket, buffer, BUFLEN);
    write(cache->handle,buffer,len);
    total+=len;
    sprintf(str,MSG_BYTESR,MSG_DOWNLD,total);
    outs(str);
   }
   else
    sock_tick( &datasocket, &status ); //shift TCP/IP
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
//!!glennmcc:Oct 23, 2008 -- 'reversed the logic'
// to keep from overflowing at 21megs
    if(filel>100)
    percentbar((int)(length/(filel/100)));
//  percentbar((int)(100*length/filel));
    lenread=a_read(f,buffer,BUFLEN);
    length+=lenread;
    if(lenread<=0)
     done=1;

    //wait until we can write to socket:
    while(sock_tbleft(&datasocket)<lenread) //SDL
//    while( datasocket.datalen > 1024)
    {
     sock_tick(&datasocket,&status);
     xChLogoTICK(1); // animation of logo
     if(GUITICK())
      goto dataquit;
    }

    if(done)
    {
     atcp_close(&datasocket);
     a_close(f);
     goto dataclose;
    }

    sock_fastwrite(&datasocket,(unsigned char *)buffer,lenread);
    sock_tick(&datasocket,&status);
   }//loop
  }
 }


dataquit:
//!!glennmcc: Nov 15,  2007 -- removed sock_abort because it was
// sometimes preventing the connection from being closed,
// therefore preventing access to several files within a short time
// due to too many connections open from the same IP
// sock_abort( &datasocket );//original line
//!!glennmcc: end

dataclose:
 outs(MSG_CLOSE);

 atcp_send(socket, "QUIT\r\n", 6);
 sock_wait_closed( &datasocket, sock_delay, TcpIdleFunc, &status );		//SDL

 if(uploadfile)
 {
  sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL
  sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
  outs(buffer);
 }

quit:
  atcp_send(socket, "QUIT\r\n", 6);
  atcp_close(socket);
  closing[socknum]=1;
  sock_keepalive[socknum][0]='\0';
//    sock_wait_closed( socket, sock_delay, NULL, &status );

//we will better wait because we are about to deallocate datasocket

sock_err:
    switch (status) {
	case 1 : /* foreign host closed */
		 write(log,MSG_CLOSE,strlen(MSG_CLOSE));
		 write(log,"\r\n",2);
		 close(log);
		 break;
	case -1: /* timeout */
		 sprintf(str,MSG_TCPERR, sockerr(socket));
		 outs(str);
		 write(log,str,strlen(str));
		 write(log,"\r\n",2);
		 close(log);
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

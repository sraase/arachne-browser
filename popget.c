
// ========================================================================
// Arachne WWW browser FTP functions
// (c)1997-1999 Michael Polak, Arachne Labs (xChaos software)
// ========================================================================

/******************************************************************************

    popdump.c - dump mail from popmail3 into spool subdirectory

    Copyright (C) 1991, University of Waterloo

    This program is free software; you can redistribute it and/or modify
    it, but you may not sell it.

    This program is distributed in the hope that it will be useful,
    but without any warranty; without even the implied warranty of
    merchantability or fitness for a particular purpose.

	Erick Engelke                   or via E-Mail
	Faculty of Engineering
	University of Waterloo          Erick@development.watstar.uwaterloo.ca
	200 University Ave.,
        Waterloo, Ont., Canada
	N2L 3G1

******************************************************************************/

//changed by xChaos software, 1996

#include "arachne.h"
#include "internet.h"

int getnumbers( char *ascii, long *d1, long *d2 );

#define POP3BUFSIZE 512

int xpopdump(struct Url *url,char dele,char logfile)
{
 int status;
 longword host;
 long process = 0, count, totallength, read, locallength, dummy, thisfile;
 long starttime;
 char str[80];
 char buffer[POP3BUFSIZE+1];
 struct ffblk ff;
 char fname[16];
 int rv=0; //default je chyba
 int f,len;
 char *ptr;
 int log;
 char done,lastchar[2];

//!!glennmcc: Nov 07, 2005 -- also check the drives containing %TEMP%
//and %ARACHNETEMP% if it is being used
int toosmall=0;
//!!glennmcc: end


 if(!tcpip)return 0;
 free_socket();

 if(logfile)
  log= a_open( "POP3.LOG" , O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE );
 else
  log=-1;

 sprintf(str,msg_askdns,url->host);
 outs(str);

 GlobalLogoStyle=0;			//SDL set resolve animation
 host = resolve_fn(url->host,(sockfunct_t) TcpIdleFunc); //SDL
// host = resolve(url->host);
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

 sock_wait_established( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
			&status);  	//SDL
 GlobalLogoStyle=1;			//SDL set data animation

 sock_mode( socket, TCP_MODE_ASCII );
 outs(MSG_LOGIN);

 //dialog

//!!glennmcc: begin Sep 10, 2001 (increase "verbosness" of pop3.log)
	if(log!=-1)
	{
	 write(log,str,strlen(str));
	 write(log,"\r\n",2);
	}
//!!glennmcc: end

    sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		     &status );		//SDL
    sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
    outs(buffer);

//!!glennmcc: begin Sep 10, 2001 (increase "verbosness" of pop3.log)
	if(log!=-1)
	{
	 write(log,buffer,strlen(buffer));
	 write(log,"\r\n",2);
	}
//!!glennmcc: end

    if ( *buffer != '+' ) goto quit;
    sprintf( str, "USER %s", url->user);
    sock_puts(socket,(unsigned char *)str);
    sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		     &status );		//SDL
    sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
    outs(buffer);
    if ( *buffer != '+' ) goto quit;

//!!glennmcc: begin Sep 10, 2001 (increase "verbosness" of pop3.log)
	if(log!=-1)
	{
	 write(log,str,strlen(str));
	 write(log,"\r\n",2);
	}
//!!glennmcc: end

    sprintf( str, "PASS %s", url->password );
    sock_puts(socket,(unsigned char *)str);
    sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		     &status );		//SDL
    sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
    outs(buffer);

//!!glennmcc: begin Sep 10, 2001 (increase "verbosness" of pop3.log)
	if(log!=-1)
	{
	 write(log,str,(strlen(str)-strlen("%s")-4));
	 write(log,"\r\n",2);
	}
//!!glennmcc: end

    if ( *buffer != '+' ) goto quit;

//!!glennmcc: begin Sep 10, 2001 (increase "verbosness" of pop3.log)
	if(log!=-1)
	{
	 write(log,buffer,strlen(buffer));
	 write(log,"\r\n",2);
	}
//!!glennmcc: end


    sprintf(str, "STAT");
    sock_puts(socket,(unsigned char *)str);
    sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		     &status );		//SDL
    sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
    outs(buffer);
    if ( *buffer != '+' ) goto quit;

    /* it must return two valid numbers */
    if ( getnumbers( buffer, &count, &totallength ) < 2 ) {
	outs("?");
	goto quit;
    }

    sprintf(str,MSG_DETECT,count, totallength );
    outs(str);

    starttime=time(NULL);

    read=0; //jiz stazeno

    while ( process++ < count )
    {
	sprintf(str,MSG_GET1, process,count,totallength,MSG_GET3 );
	outs(str);

	sprintf( str, "LIST %lu", process );
	if(log!=-1)
	{
	 write(log,str,strlen(str));
	 write(log,"\r\n",2);
	}
	sock_puts(socket,(unsigned char *)str);
	sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
			 &status );		//SDL
	sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
	if(log!=-1)
	{
	 write(log,buffer,strlen(buffer));
	 write(log,"\r\n",2);
	}
	if ( getnumbers( buffer, &dummy, &locallength ) < 2 ) {
	    sprintf(str,"? LIST %lu\n", process );
	    outs(str);
	    goto quit;
	}

	sprintf(str,MSG_GET2, process,locallength,count,totallength,MSG_GET3 );
	outs(str);
	percentbar((int)(100*read/totallength));
//!!glennmcc: Oct 19, 2005
//check space on drive where mailpath is located instead of the current drive.
//!!glennmcc: Nov 07, 2005 -- also check the drives containing %TEMP%
//and %ARACHNETEMP% if it is being used
toosmall=0;
if(lastdiskspace(configvariable(&ARACHNEcfg,"MailPath",NULL))
   <locallength*2) toosmall=1;
if(lastdiskspace(getenv("TEMP"))
   <locallength*2) toosmall=2;
if(configvariable(&ARACHNEcfg,"Cache2Temp",NULL)[0]=='Y' &&
   lastdiskspace(getenv("ARACHNETEMP"))
   <locallength*2) toosmall=3;
if(toosmall)
	{
//	 if ( localdiskspace() < locallength * 2 ) {
	 sprintf(str,MSG_SKIP, process );
	 outs(str);
//!!glennmcc: Oct 19, 2005 -- if message is too big, write the error into .CNM
	 goto maketime;
//	 continue; //original single line
//!!glennmcc: end this section.....more below
	}
	sprintf( str, "RETR %lu", process );
	if(log!=-1)
	{
	 write(log,str,strlen(str));
	 write(log,"\r\n",2);
	}
	sock_puts(socket,(unsigned char *)str);
	sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
			 &status );		//SDL
	sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
	if(log!=-1)
	{
	 write(log,buffer,strlen(buffer));
	 write(log,"\r\n",2);
	}
	if (*buffer != '+' ) goto quit;

	//make filename
	maketime:
//!!glennmcc: Nov 10, 2004 --- HEX names instead of decimal
//!!glennmcc: Nov 11, 2004 -- begin at Annnnnnn.CNM
//it is currently at 4nnnnnnn.CNM so we need to add 60000000 hex
//      sprintf(fname,"%ld.CNM",starttime+process);
	sprintf(fname,"%lx.CNM",starttime+process+0x60000000l);

	 makename:
	 strcpy(str,configvariable(&ARACHNEcfg,"MailPath",NULL));
	 if(strlen(fname)>12)
	  strcat(str,&fname[strlen(fname)-12]);
	 else
	  strcat(str,fname);

	 if(!findfirst(str,&ff,0))
	 {
	  fname[8]++;
//!!glennmcc: Nov 10, 2004 --- HEX names instead of decimal
//	  if(fname[8]>'9')
	  if(fname[8]>'F')
	  {
	   starttime=time(NULL);
	   goto maketime;
	  }
	  else
	   goto makename;
	 }

	f = a_fast_open( str , O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE );
	if (f<0 )
	{
	 outs(MSG_ERROPN);
	 goto quit;
	}

//!!glennmcc: Oct 19, 2005 -- if message is too big, write the error into .CNM
//if(lastdiskspace(configvariable(&ARACHNEcfg,"MailPath",NULL))<locallength*2)
//!!glennmcc: Nov 07, 2005 -- also check the drives containing %TEMP%
//and %ARACHNETEMP% if it is being used
//now select message depending upon which drive was too small
if(toosmall)
	{
	 sprintf(str,"Subject: "MSG_SKIP,process);
	 write(f,str,strlen(str));
	 if(toosmall==1)
	 {
	 sprintf(str,"\n Available disk space on mail drive: %ld",
	 lastdiskspace(configvariable(&ARACHNEcfg,"MailPath",NULL)));
	 }else
	 if(toosmall==2)
	 {
	 sprintf(str,"\n Available disk space on TEMP drive: %ld",
	 lastdiskspace(getenv("TEMP")));
	 }else
	 if(toosmall==3)
	 {
	 sprintf(str,"\n Available disk space on ARACHNETEMP drive: %ld",
	 lastdiskspace(getenv("ARACHNETEMP")));
	 }
	 write(f,str,strlen(str));
	 sprintf(str,"\n Required disk space for this message: %ld",locallength*2);
	 write(f,str,strlen(str));
	 a_close(f);
	 continue;
	}
//!!glennmcc: end

       sock_mode( socket, TCP_MODE_BINARY );
       thisfile=0l;

       done=0;
       lastchar[0]='\0';
       lastchar[1]='\0';
       do
       {
	sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
			 &status );		//SDL
	len=sock_fastread( socket, (unsigned char*)buffer, POP3BUFSIZE );
	buffer[len]='\0';
	if(log!=-1)
	{
	 write(log,buffer,strlen(buffer));
	}
	read+=len;
	locallength-=len;


	if(thisfile==0l && buffer[0]=='\n')
	{
	 ptr=&buffer[1];
	 len--;
	}
	else
	 ptr=buffer;

//!!glennmcc: May 21, 2005 -- force "done" at end of message
//to fix the 'freeze-up' problem on certain SPAM emails
//the next 3 lines were used during my testing
//sprintf(str,MSG_GET2, process,locallength,count,totallength,MSG_GET3 );
//sprintf(str,"\n buffer = %d \n",strlen(buffer));
//outs(str);

//this first attempt on May 21 caused many aborted downloads of 'good' emails
//if(locallength<1) done=1;

//!!glennmcc: Jun 15, 2005 -- trying something different
//if(locallength<1 && (strstr(ptr,"\n.\r") || strstr(ptr,"\n.\n"))) done=1;

//!!glennmcc: Jun 22, 2005 -- that still did not work correctly,
// here's another method for 'force done'
//if(strlen(buffer)<1) done=1;

//!!glennmcc: Jun 27, 2005 -- This one finally works correctly :))
#ifndef NOKEY
if(locallength<1 &&
   (
    strstr(ptr,"\n\0\n.\n")
//!!glennmcc: Sep 29, 2005 -- next line was still causing a few aborts
//  || strstr(ptr,"\r\n\0\r\n.\r\n")
   )
  ) done=1;
#endif
//!!glennmcc: end

	//treat special situations:
	if(lastchar[1]=='.' && lastchar[0]=='\n' &&
	   (ptr[0]=='\n' || ptr[0]=='\r') ||
	   lastchar[1]=='\n' && ptr[0]=='.' &&
	   (ptr[1]=='\n' || ptr[1]=='\r'))
	{
	 len=0;
	 done=1;
	}
	else
	{
//!!glennmcc:
// this next section had already been commented-out by Michael Polak
/*
	 while(len>2 &&
	       (ptr[len-1]=='\n' || ptr[len-1]=='\r' || ptr[len-1]=='.'))
	 {
	  len--;
	  if(ptr[len-1]=='\n' && ptr[len]=='.' &&
	     (ptr[len+1]=='\n' || ptr[len+1]=='\r'))
	  {
	   ptr[len]='\0';
	   done=1;
	   break;
	  }
	 }
*/


	 if(strstr(ptr,"\n.\r") || strstr(ptr,"\n.\n"))
	 {
	  char *dot=strrchr(ptr,'.');
	  if(dot)
	   *dot='\0';
	  done=1;
	 }

/*
//!!glennmcc: May 21, 2005 -- this old fix is no longer needed :)
//!!glennmcc: begin Aug 07, 2004
//fix the 'freeze-up' problem on certain SPAM emails
    {
     strcpy(str,configvariable(&ARACHNEcfg,"FreezeUpString",NULL));
       if(strlen(str)>1 && strstr(ptr,str))
	 {
	  done=1;
	 }
else
     strcpy(str,configvariable(&ARACHNEcfg,"FreezeUpString2",NULL));
       if(strlen(str)>1 && strstr(ptr,str))
	 {
	  done=1;
	 }
    }
//!!glennmcc: end
*/

	}

	if(len>0)
	 lastchar[1]=ptr[len-1];
	else
	 lastchar[1]='\0';

	if(len>1)
	 lastchar[0]=ptr[len-2];
	else
	 lastchar[0]='\0';

	if(len)
	 write(f,ptr,len);

	thisfile+=len;

//        if(read % 512l == 0l) //kresleni tycky kazdy 1 kB
	 percentbar((int)(100*read/totallength));
       }
	while (!done /*&& locallength>0*/);

//!!glennmcc: May 21, 2005 -- do not add a LF
//I don't understand why it was doing that to begin with. ???
//       write(f,"\n",1);
//!!glennmcc: end

       sock_mode( socket, TCP_MODE_ASCII );

       //skip empty lines ?
       while(sock_dataready(socket))
       {
	len=sock_fastread( socket, (unsigned char*)buffer, sizeof( buffer ) );
	buffer[len]='\0';
	if(log!=-1)
	{
	 write(log,buffer,len);
	 write(log,"\r\n",2);
	}
       }

       a_close(f);
       if(dele)
       {
	sprintf(str,MSG_DELE, process,count);
	outs(str);
	sprintf(str,"DELE %lu", process );
	if(log!=-1)
	{
	 write(log,str,strlen(str));
	 write(log,"\r\n",2);
	}
	sock_puts(socket,(unsigned char *)str);

	sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
			 &status );		//SDL
	sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
	if(log!=-1)
	{
	 write(log,buffer,strlen(buffer));
	 write(log,"\r\n",2);
	}

	if ( *buffer != '+' ) goto quit;
       }
    }//loop

    rv=1;
quit:
    outs(MSG_CLOSE);
    sock_puts(socket,(unsigned char *)"QUIT");

//!!glennmcc: begin Sep 10, 2001
// (changed from 'str' to 'MSG_CLOSE' to reflect "QUIT" message sent)
    if(log!=-1)
    {
     write(log,MSG_CLOSE,strlen(MSG_CLOSE));
     write(log,"\r\n",2);
    }
//!!glennmcc: end

    sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		     &status );		//SDL
    sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
    outs(buffer);
    if(log!=-1)
    {
     write(log,buffer,strlen(buffer));
     write(log,"\r\n",2);
    }
    if ( *buffer != '+' )
     rv=0;
    sock_close( socket );
    closing[socknum]=1;
    sock_keepalive[socknum][0]='\0';

sock_err:
    switch (status) {
	case 1 : /* foreign host closed */
		 break;
	case -1: /* timeout */
		 sprintf(str,MSG_TCPERR, sockerr(socket));
		 outs(str);
		 break;
    }
 if(log!=-1)
  a_close(log);
 return rv;
}

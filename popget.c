
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

//!!glennmcc: Nov 25, 2005 -- for checking the drivespace (see below)
int toobig=0, mult=1;
//!!glennmcc: end
int mailtop = config_get_bool("MailTop", 0);

 if(!tcpip)return 0;
 free_socket();

 if(logfile)
  log= a_open( "POP3.LOG" , O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE );
 else
  log=-1;

 sprintf(str,msg_askdns,url->host);
 outs(str);

 GlobalLogoStyle=0;			//SDL set resolve animation
 if (atcp_resolve(url->host, &host))
 {
  DNSerr(url->host);
  return 0;
 }

 GlobalLogoStyle=2;			//SDL set connect animation
 if (atcp_open(socket, &host, url->port)) {
  sprintf(str,msg_errcon,url->host);
  outs(str);
  return 0;
 }
 GlobalLogoStyle=1;			//SDL set data animation

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
  sock_setbuf(socket, (unsigned char *)setbuf, 1024*SETBUFSIZE);
 }
}
//!!glennmcc: end

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

    sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL
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
    sprintf( str, "USER %s\r\n", url->user);
    atcp_send(socket, str, strlen(str));
    sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL
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

    sprintf( str, "PASS %s\r\n", url->password );
    atcp_send(socket, str, strlen(str));
    sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL
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


    sprintf(str, "STAT\r\n");
    atcp_send(socket, str, strlen(str));
    sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL
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

	sprintf( str, "LIST %lu\r\n", process );
	if(log!=-1)
	{
	 write(log,str,strlen(str));
	}
        atcp_send(socket, str, strlen(str));
	sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL
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
//!!glennmcc:Oct 23, 2008 -- 'reversed the logic'
// to keep from overflowing at 21megs
	if(totallength>100)
	percentbar((int)(read/(totallength/100)));
//      percentbar((int)(100*read/totallength));
//!!glennmcc: Oct 19, 2005
//check space on drive where mailpath is located instead of the current drive.
toobig=0;
//!!glennmcc: Nov 25, 2005 -- we only need 'double space'
// when pop3log is 'on' and mailpath==current disk
if(log!=-1 &&
lastdiskspace(mailpath)==localdiskspace())
   mult=2;else mult=1;

if(lastdiskspace(mailpath)
   <(locallength*mult)+user_interface.mindiskspace) toobig=1;

if(toobig)
	{
//	 if ( localdiskspace() < locallength * 2 ) {
	 sprintf(str,MSG_SKIP, process );
	 outs(str);
//!!glennmcc: Oct 19, 2005 -- if message is too big, write the error into .CNM
	 goto maketime;
//	 continue; //original single line
//!!glennmcc: end this section.....more below
	}

     if(mailtop) sprintf( str, "TOP %lu 1000000\r\n", process );
     else sprintf( str, "RETR %lu\r\n", process );
	if(log!=-1)
	{
	 write(log,str,strlen(str));
	}
/*
//!!glennmcc: Sep 27, 2008 -- increase D/L speed on cable & DSL
//many thanks to 'mik' for pointing me in the right direction. :)
if(!bufset)
{
#ifdef DEBUG
 char sp[80];
 sprintf(sp,"Available stack = %u bytes",_SP);
 outs(sp);
 Piip(); Piip();
#endif
if(_SP>(1024*20))
{
 char setbuf[1024*20];
 bufset=1;
 sock_setbuf(socket, (unsigned char *)setbuf, 1024*20);
}
}
//!!glennmcc: end
*/
        atcp_send(socket, str, strlen(str));
	sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL
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
	 strcpy(str,mailpath);
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
if(toobig)
	{
	 sprintf(str,"Subject: "MSG_SKIP,process);
	 write(f,str,strlen(str));
	 sprintf(str,"\n Available disk space on mail drive: %ld",
	 lastdiskspace(mailpath));
	 write(f,str,strlen(str));
	 sprintf(str,"\n Required disk space for this message: %ld",(locallength*mult)+user_interface.mindiskspace);
	 write(f,str,strlen(str));
	 sprintf(str,"\n Message size of %ld + MinDiskSpace setting of %ld",locallength,user_interface.mindiskspace);
	 write(f,str,strlen(str));
	 if(mult==2)
	 {
	 sprintf(str,"\n (double the message size itself due to POP3LOG)");
	 write(f,str,strlen(str));
	 sprintf(str,"\n Message size of (%ld * 2) + MinDiskSpace setting of %ld",locallength,user_interface.mindiskspace);
	 write(f,str,strlen(str));
	 }
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
	sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL
	len=atcp_recv(socket, buffer, POP3BUFSIZE);
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
#ifdef EXP//experimental
if(mailtop)
{
if(strlen(ptr)<1 || !buffer || (locallength<1 && strstr(ptr,"\n\0\n.\n")
//!!glennmcc: Sep 29, 2005 -- next line was still causing a few aborts
//  || strstr(ptr,"\r\n\0\r\n.\r\n")
   )
  ) done=1;
}
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
//!!glennmcc: Feb 22, 2006 -- don't write 'null CR LF' at end of file
	  len-=3;
//!!glennmcc: end
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
//!!glennmcc:Oct 23, 2008 -- 'reversed the logic'
// to keep from overflowing at 21megs
	 if(totallength>100)
	 percentbar((int)(read/(totallength/100)));
//       percentbar((int)(100*read/totallength));
       }
	while (!done /*&& locallength>0*/);

//!!glennmcc: May 21, 2005 -- do not add a LF
//I don't understand why it was doing that to begin with. ???
//       write(f,"\n",1);
//!!glennmcc: end

       sock_mode( socket, TCP_MODE_ASCII );

       //skip empty lines ?
       while(atcp_has_data(socket))
       {
	len=atcp_recv(socket, buffer, sizeof(buffer));
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
	sprintf(str,"DELE %lu\r\n", process );
	if(log!=-1)
	{
	 write(log,str,strlen(str));
	}
        atcp_send(socket, str, strlen(str));

	sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL
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
    atcp_send(socket, "QUIT\r\n", 6);

//!!glennmcc: begin Sep 10, 2001
// (changed from 'str' to 'MSG_CLOSE' to reflect "QUIT" message sent)
    if(log!=-1)
    {
     write(log,MSG_CLOSE,strlen(MSG_CLOSE));
     write(log,"\r\n",2);
    }
//!!glennmcc: end

    sock_wait_input( socket, sock_delay, TcpIdleFunc, &status );		//SDL
    sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
    outs(buffer);
    if(log!=-1)
    {
     write(log,buffer,strlen(buffer));
     write(log,"\r\n",2);
    }
    if ( *buffer != '+' )
     rv=0;
    atcp_close(socket);
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

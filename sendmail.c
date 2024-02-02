
// ========================================================================
// SMTP sendmail for DOS. Implements RFC821 and RFC822.
// (c)1997-1999 Michael Polak, Arachne Labs (xChaos software)
// ========================================================================

#include "arachne.h"
#include "internet.h"

// ========================================================================
// cut address according to RFC 822
// ========================================================================

void cutaddress(char *string)
{
 int i=0,j=0;
 char c=0,z=0;

 while(string[i]==' ') //cut spaces
  i++;

 // Example: "Michael Polak" <xchaos@main.naf.cz> => xchaos@main.naf.cz
 while(string[i]!='\0' && j<127)
 {
  if(string[i]=='\"')c=1-c;
  else
  if(string[i]=='(')z=1;
  else
  if(string[i]==')')z=0;
  else
  if(string[i]=='<')
   j=0;
  else
  if(string[i]=='>')
   goto smik;
  else
  if(!c && !z && string[i]!=' ')
   string[j++]=string[i];
  i++;
 }//loop
 smik:
 string[j]='\0';
}//endsub


// ========================================================================
// send mail files according to RFC821. Filemask may contain wildcards.
// messages are in format described in RFC822.
// ========================================================================

#define BUFLEN 1024//!!glennmcc: Dec 14, 2006
//#define BUFLEN 512
extern int reset_detected;

int xsendmail(struct Url *url, char helo, char logfile)

{
 longword host;
 long length;
//!!glennmcc: Feb 05, 2008 -- increased from 128 to 512
 char str[512/*128*/],pom[512/*128*/];

//!!glennmcc Apr 30, 2004 -- for AuthSMTP
 char AuthSMTPusername[128]="",AuthSMTPpassword[128]="";
//!!glennmcc: end

 char buffer[BUFLEN];
 struct ffblk ff;
 char filename[80];
 int rv=0;             //default return value == error
 int f,nomoremail;
 int done,lenread,i,j,lastcarka,carka,field,success,u,z;
 int status,err=0;
 char *ptr;
 int log;

//!!glennmcc & Ray: Dec 14, 2006
 int bcc=0;
//!!glennmcc & Ray: end

 if(!tcpip)
  return 0;

 nomoremail=findfirst(&url->file[1],&ff,0);
 if(nomoremail)
  return 1;

 if(logfile)
  log= a_open( "SMTP.LOG" , O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE );
 else
  log=-1;

 free_socket();

 //reset requests on IDENT port:

 reset_detected=0;
 tcp_listen(sock[1-socknum], 113, 0, 0, (dataHandler_t) resetport , 0);

 sprintf(str,msg_askdns,url->host);
 outs(str);

 GlobalLogoStyle=0;		//SDL set resolve animation
 if (atcp_resolve(url->host, &host))
 {
  DNSerr(url->host);
  return 0;
 }

 sprintf(str,msg_con,url->host,url->port);
 outs(str);

 GlobalLogoStyle=2;		//SDL set connect animation
 if (!tcp_open( socket, locport(), host, url->port, NULL ))
 {
  sprintf(str,msg_errcon,url->host);
  outs(str);
  return 0;
 }

 sock_wait_established( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
			&status);		//SDL
 GlobalLogoStyle=1;		//SDL set data animation

 sock_mode( socket, TCP_MODE_ASCII );
 outs(MSG_SMTP);

 //wait for daemon to appear:
 do
 {
  sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		   &status );		//SDL
  sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
  outs(buffer);
  if(log!=-1)
  {
   write(log,buffer,strlen(buffer));
   write(log,"\r\n",2);
  }
  if ( *buffer != '2' ) goto quit;
 }
 while(buffer[3]=='-'); //continued message!

 if(log!=-1 && reset_detected)
 {
  strcpy(str,MSG_IDENT);
  write(log,str,strlen(str));
  write(log,"\r\n",2);
 }

 if(helo)
// if(helo!=0)
 {
  //HELO protocol
  char *mydomain=strchr(url->user,'@');
  if(mydomain)
   mydomain++; // xx@->yyy
  else
//!!glennmcc: Feb 22, 2006 -- use host domain instead of user
  {
   mydomain=strchr(url->host,'.');
   mydomain++; // xx@->yyy
  }
//   mydomain=url->user;
//!!glennmcc: end

//!!glennmcc: begin Nov 09, 2003 --- EHLO == Authenticated SMTP
//finally finished it on Apr 30, 2004 ;-)
if(helo==2)
{
//!!glennmcc: Feb 14, 2006 -- use 'authuser' for AuthSMTP
     mydomain=strrchr(url->authuser,'@');
     if(mydomain)
      mydomain++; // xx@->yyy
      else
//!!glennmcc: Feb 22, 2006 -- use host domain instead of authuser
      {
       mydomain=strchr(url->host,'.');
       mydomain++; // xx@->yyy
      }
//     mydomain=url->authuser;
//!!glennmcc: end

  sprintf( str, "EHLO %s", mydomain);
  outs(str);
  if(log!=-1)
  {
   write(log,str,strlen(str));
   write(log,"\r\n",2);
  }
  sock_puts(socket,(unsigned char *)str);


  do
  {
   sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		    &status );		//SDL
   sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
   outs(buffer);
   if(log!=-1)
   {
    write(log,buffer,strlen(buffer));
    write(log,"\r\n",2);
   }
   if ( *buffer != '2' ) goto quit;
  }
  while(buffer[3]=='-'); //continued message!

  sprintf( str, "AUTH LOGIN");
  outs(str);
  if(log!=-1)
  {
   write(log,str,strlen(str));
   write(log,"\r\n",2);
  }
  sock_puts(socket,(unsigned char *)str);

  do
  {
   sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		    &status );		//SDL
   sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
   outs(buffer);
   if(log!=-1)
   {
    write(log,buffer,strlen(buffer));
    write(log,"\r\n",2);
   }
   if ( *buffer != '3' ) goto quit;
  }
  while(buffer[3]=='-'); //continued message!
//!!glennmcc: Sept 17, 2004
// changed so that "email" will always get used for "mail from"
//  base64code((unsigned char *)url->user,AuthSMTPusername);
  base64code((unsigned char *)url->authuser,AuthSMTPusername);
//!!glennmcc: end
  sprintf( str, AuthSMTPusername);
  outs(str);
  if(log!=-1)
  {
   write(log,str,strlen(str));
   write(log,"\r\n",2);
  }
  sock_puts(socket,(unsigned char *)str);

  do
  {
   sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		    &status );		//SDL
   sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
   outs(buffer);
   if(log!=-1)
   {
    write(log,buffer,strlen(buffer));
    write(log,"\r\n",2);
   }
   if ( *buffer != '3' ) goto quit;
  }
  while(buffer[3]=='-'); //continued message!

//!!glennmcc: Feb 17, 2006 -- switch to new variable 'authpassword'
  base64code((unsigned char *)url->authpassword,AuthSMTPpassword);
//  base64code((unsigned char *)url->password,AuthSMTPpassword);
  sprintf( str, AuthSMTPpassword);
  outs(str);
  if(log!=-1)
  {
   write(log,str,strlen(str));
   write(log,"\r\n",2);
  }
  sock_puts(socket,(unsigned char *)str);

  do
  {
   sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		    &status );		//SDL
   sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
   outs(buffer);
   if(log!=-1)
   {
    write(log,buffer,strlen(buffer));
    write(log,"\r\n",2);
   }
   if ( *buffer != '2' ) goto quit;
  }
  while(buffer[3]=='-'); //continued message!
}
else //begin else HELO
{
//!!glennmcc: end

  sprintf( str, "HELO %s", mydomain);
  outs(str);
  if(log!=-1)
  {
   write(log,str,strlen(str));
   write(log,"\r\n",2);
  }
  sock_puts(socket,(unsigned char *)str);

  do
  {
   sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		    &status );		//SDL
   sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
   outs(buffer);
   if(log!=-1)
   {
    write(log,buffer,strlen(buffer));
    write(log,"\r\n",2);
   }
   if ( *buffer != '2' ) goto quit;
  }
  while(buffer[3]=='-'); //continued message!

}//!!glennmcc: end of else HELO

 }

 //do for all messages
 while (!nomoremail)
 {
  //process msg body

  if(err)
  {
   outs("RSET");
   if(log!=-1)
   {
    write(log,"RSET\r\n",6);
   }
   sock_puts(socket,(unsigned char *)"RSET");

   do
   {
    sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		    &status );		//SDL
    sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
    outs(buffer);
    if(log!=-1)
    {
     write(log,buffer,strlen(buffer));
     write(log,"\r\n",2);
    }
    if ( *buffer != '2' ) goto quit; //cannot reset ??
   }
   while(buffer[3]=='-'); //continued message!
   err=0;
  }

  strcpy(filename,&url->file[1]);
  ptr=strrchr(filename,'\\');
  if(ptr)
   ptr++; //point after "\"
  else
   ptr=filename; //current dir ? (not likelky case)
  strcpy(ptr,ff.ff_name);

  //open file
  f=a_fast_open(filename, O_TEXT|O_RDONLY,0);
  if(f<0)
   goto cont; //go to next message!


  //start SMTP
  sprintf( str, "MAIL FROM: <%s>", url->user);

  outs(str);
  if(log!=-1)
  {
   write(log,str,strlen(str));
   write(log,"\r\n",2);
  }
  sock_puts(socket,(unsigned char *)str);
  do
  {
   sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		    &status );		//SDL
   sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
   outs(buffer);
   if(log!=-1)
   {
    write(log,buffer,strlen(buffer));
    write(log,"\r\n",2);
   }
   if ( *buffer != '2' ) goto quit; //I am not accepted ?!
  }
  while(buffer[3]=='-'); //continued message!

  //process msg header
  done=lenread=j=lastcarka=carka=field=success=u=z=0;
  i=1; //force reading!
  do
  {

   if(i>=lenread)
   {
    lenread=a_read(f,buffer,BUFLEN);
    if(lenread<=0)
     done=1;
    i=0;
   }

   str[j]=buffer[i++];

   if(str[j]=='\"')u=1-u;
   else
   if(str[j]=='(')z=1;
   else
   if(str[j]==')')z=0;
   else
   if(str[j]==',' && !u && !z)carka=1;

   if(j>=127 || str[j]=='\n' || carka || done)
   {
    str[j]='\0';

    if(!str[0] && !lastcarka) //empty line -> end of message header
    {
     done=1;
     field=0;
    }
    else
    if(!strncmpi("TO:",str,3) || !strncmpi("CC:",str,3))
    {
     ptr=&str[3];
     field=1;
    }
    else
    if(!strncmpi("BCC:",str,4))
    {
//!!glennmcc & Ray: Dec 14, 2006
     bcc=1;
//!!glennmcc & Ray: end
     ptr=&str[4];
     field=1;
    }
    else
    if(field && (lastcarka || str[0]==' '))
     ptr=str;
    else
     field=0;

    if(field)
    {
     struct ib_editor expandlist;
     int rcpt;

     if(ptr[0]==' ' && ptr[1]=='@') //expand mailing list
     {
      makestr(expandlist.filename,&ptr[2],79);
      if(ie_openf(&expandlist,CONTEXT_TMP)==1)
       field=-1;
      else
      {
       field=0;
       expandlist.filename[0]='\0';
      }
     }
     else
      expandlist.filename[0]='\0';

     if(field!=0)
      rcpt=1;
     else
      rcpt=0;

     while(rcpt)
     {
      if(field==1) //address in ptr
       rcpt=0;
      else
      {
       ptr=ie_getline(&expandlist,expandlist.y++);
       if(expandlist.y>=expandlist.lines)
       {
	rcpt=0;
	field=0;
       }
      }

      cutaddress(ptr);
      if(*ptr)
      {
       //add SMTP recipient
       sprintf( pom, "RCPT TO: <%s>", ptr);
       outs(pom);
       if(log!=-1)
       {
	write(log,pom,strlen(pom));
	write(log,"\r\n",2);
       }
       sock_puts(socket,(unsigned char *)pom);
       sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
			&status );		//SDL
       sock_gets( socket, (unsigned char *)pom, sizeof( pom ));
       outs(pom);
       if(log!=-1)
       {
	write(log,pom,strlen(pom));
	write(log,"\r\n",2);
       }
       if(*pom == '2')
	success++;
      }
     }//loop

     if(expandlist.filename[0])
     {
      ie_killcontext(CONTEXT_TMP); //clear temporary file
//      ie_closef(&expandlist);
     }
    }//end temp variables

    lastcarka=carka;
    carka=0;
    j=0;
   }
   else
    j++;

  }
  while(!done);

  if(!success) //cannot send to any recipient ?
  {
   err=1;
   goto cont;
  }


  //process msg body
  outs("DATA");
  if(log!=-1)
  {
   write(log,"DATA\r\n",6);
  }
  sock_puts(socket,(unsigned char *)"DATA");
  sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		   &status );		//SDL
  sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
  outs(buffer);
  if(log!=-1)
  {
   write(log,buffer,strlen(buffer));
   write(log,"\r\n",2);
  }
  if ( *buffer != '3')
  {
   err=1;
   goto cont; //failed ? try next message!
  }

  a_lseek(f,0L,SEEK_SET); //reset file
  j=lenread=done=0;
  i=1;
  length=0;
  {
   long fll=a_filelength(f);
   do
   {
    if(i>=lenread)
    {
     sprintf(pom,MSG_SEND,length,fll);
     outs(pom);
/*
//!!glennmcc:Oct 23, 2008 -- 'reversed the logic'
// to keep from overflowing at 21megs
     if(fll>100)
     percentbar((int)(length/(fll/100)));
*/
   percentbar((int)(100*length/fll));
     lenread=a_read(f,buffer,BUFLEN);
     length+=lenread;
     if(lenread<=0)
      done=1;
     i=0;
 //    printf("\nlenread=%d \n",lenread);
//!!glennmcc & Ray: Dec 14, 2006 -- bypass Bcc:
if(bcc)
  {
   char *ptr = strstr(buffer, "Bcc:");//Ray
   if(ptr)
     {
//Ray's method replaces the address(s) with spaces
//it also looks for '\r' (carrage return) as the end of the Bcc line
//      ptr += 4;                        // PASS 'Bcc:'
//    while(*ptr && *ptr != '\r')
//    *ptr++ = ' ';
//printf("After: \n\n%40s", ptr); getch(); // SEE THE RESULT

//!!glennmcc: this method works better by 'nulling-out' the Bcc line and
//looking for '\n' (new line) as the end of the Bcc line
/* method #1 */
/**/
      if(*ptr--=='\n')*ptr='\0';//remove previous '\r\n' if it exists
      do
      {*ptr='\0'; ptr++;}
      while( *ptr && *ptr!='\n');
/**/
/* end method #1 */

/* method *2 */
/*
      if(*ptr--=='\n')*ptr='\0';//remove previous '\r\n' if it exists
      while(*ptr && *ptr!='\n')
      *ptr++ ='\0';
      *ptr='\0';//also remove this '\n'
*/
/* end method #2 */
      bcc=0;
     }
  }
//!!glennmcc & Ray: end
    }
    str[j]=buffer[i++];

//!!glennmcc: Feb 05, 2008 -- prevent character from being lost at line break
//by breaking at a space instead of 127 explicitly
//also increased line length limit to the next space past 200
//instead of just 127 explicitly
    if((j>=200 && str[j]==' ') || str[j]=='\n' || done)
//  if(j>=127 || str[j]=='\n' || done) //original line
//!!glennmcc: end
    {

     str[j]='\0';
     length++; //ASCI mode !!!

     //wait until we can write to socket:
     while(sock_tbleft(socket)<j+1)	//SDL
//     while( socket->datalen > 1024)
     {
      sock_tick(socket,&status);
      xChLogoTICK(1); // animace loga
      if(GUITICK())
      goto quit;
     }

     if (str[0]=='.')			//SDL always double up
      sock_putc(socket,'.');  		//SDL leading periods
     sock_puts(socket,(unsigned char *)str);
     if(log!=-1)
     {
      write(log,str,strlen(str));
      write(log,"\r\n",2);
     }
     sock_tick(socket,&status);
     j=0;
    }
    else
     j++;
   }
   while(!done);
  }
  a_close(f);

  sock_puts(socket,(unsigned char *)".");
  if(log!=-1)
  {
   write(log,".\r\n",3);
  }
  sock_wait_input( socket, sock_delay, (sockfunct_t) TcpIdleFunc,
		   &status );		//SDL
  sock_gets( socket, (unsigned char *)buffer, sizeof( buffer ));
  outs(buffer);
  if(log!=-1)
  {
   write(log,buffer,strlen(buffer));
   write(log,"\r\n",2);
  }
  if ( *buffer != '2' )
  {
   err=1;
  }
  else
  {
   //delete or rename
   ptr=strrchr(filename,'\\');
   if(!ptr)
    ptr=filename;
   else
    ptr++;

   if(*ptr=='!')
    unlink(filename);
   else
   {
    strcpy(str,filename);
    ptr=strrchr(str,'.');
    if(ptr)
     strcpy(&ptr[1],"SNT");
    rename(filename,str);
   }
  }//endif

  cont:
  nomoremail=findnext(&ff);
 }//loop - next message

 rv=1;

quit:
    outs("QUIT");
    if(log!=-1)
    {
     write(log,"QUIT\r\n",6);
     close(log);
    }
    sock_puts(socket,(unsigned char *)"QUIT");
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
 return rv;
}



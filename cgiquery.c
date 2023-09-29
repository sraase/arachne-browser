
// ========================================================================
// CGI (DGI) query processor for Arachne WWW browser
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "html.h"

//this little helper function converts semicolons to colons in TO: and CC:
void adrfield(char *str)
{
 int u=0,z=0;

 while(*str)
 {
  if(*str=='\"')u=1-u;
  else
  if(*str=='(')z=1;
  else
  if(*str==')')z=0;
  else
  if(!u && !z && (*str==';' || *str==':'))
   *str=',';

  str++;
 }
}

void process_form(char cgi, XSWAP formID)
//cgi | 0=internal config, 1=create query string, 2=http conversion, 4=subimg
{
 if(formID!=IE_NULL || activeatomptr &&
   (activeatomptr->type==INPUT ||
    activeatomptr->type==IMG && activeatomptr->data1==4))
 {
  char subtype,checked;
  char *querystring;
  unsigned qlen=0;
  char modified=0,attach=0;
  int mailmsg=-1;
  char delorig=0,nosign=0;
  char mailname[80]="\0";
  XSWAP currentHTMLatom=p->firstHTMLatom;
  char *ptr;
  int f,i;
  struct HTMLrecord *atomtmpptr;

  GLOBAL.mailaction=MAIL_SAVENOW;

  if(formID==IE_NULL)
   formID=activeatomptr->linkptr;

  mouseoff();
  if(cgi)
   outs(MSG_FORM);

  querystring=ie_getswap(GLOBAL.postdataptr);
  if(!querystring)
  {
   GLOBAL.postdataptr=ie_putswap("",MAXQUERY,CONTEXT_SYSTEM);
   if(GLOBAL.postdataptr==IE_NULL)
     MALLOCERR();
   else
    querystring=ie_getswap(GLOBAL.postdataptr);
  }
  *querystring='\0'; //nuluju
  swapmod=1; //zapsal jsem!!

  while(currentHTMLatom!=IE_NULL)
  {
//   kbhit();
   atomtmpptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
   if(!atomtmpptr)
     MALLOCERR();
   currentHTMLatom=atomtmpptr->next;

   subtype=atomtmpptr->data1;
   checked=atomtmpptr->data2;
   if(atomtmpptr->type==INPUT && atomtmpptr->linkptr==formID)
   {
    editorptr=(struct ib_editor *)ie_getswap(atomtmpptr->ptr);
    if(editorptr)
    {
     char name[80],*value;
     char str[256];

     //copy editor from xSwap to memory
     memcpy(&tmpeditor,editorptr,sizeof(struct ib_editor));

     if(subtype==TEXTAREA)
     {
      editorptr->modified=0;
      swapmod=1;
     }

     strcpy(name,tmpeditor.filename);
     value=ie_getline(&tmpeditor,0);
     if(value && *value)
     {
      strcpy(p->buf,value);
     }
     else
      p->buf[0]='\0';

     value=p->buf;

     //----------------------------------------------------------
     if((subtype==RADIO || subtype==CHECKBOX || subtype==SUBMIT && name[0])
	&& (checked & 1) || subtype==TEXT || subtype==PASSWORD || subtype==HIDDEN)
     {
      if(cgi)
      {
       if(qlen<MAXQUERY-80-IE_MAXLEN)
       {
	querystring=ie_getswap(GLOBAL.postdataptr);
	if(!querystring)
	  MALLOCERR();
	if(qlen>0)
	{
	 strcat(querystring,"&");
	 qlen++;
	}
	if(name[0])
	{
	 strcat(querystring,name);
	 strcat(querystring,"=");
	 qlen+=strlen(name)+1;
	}

	qlen+=cgiquery((unsigned char *)value,(unsigned char *)&querystring[qlen],cgi&2); //cgi&2=true if http:..
	swapmod=1; //novy platny querystring:
       }
//!!glennmcc: Apr 18, 2008 -- save form data into $query$.tmp
if(!strstr(GLOBAL.location,"file:"))
      {
       char q[80];
       int f;
       tempinit(q);
       strcat(q,"$query$.tmp");
       f=a_fast_open(q,O_BINARY|O_CREAT|O_TRUNC|O_WRONLY,S_IREAD|S_IWRITE);
       if(f>=0)
       {
	write(f,querystring,strlen(querystring));
	a_close(f);
       }
      }
//!!glennmcc: end
      }
      else //<form action=arachne:internal....>
      {
       //action ------------------------------------------------------------
       if(name[0]=='$')
       {
	char *cmd=&name[1];
	if(!strcmpi(cmd,"MOVE") || !strcmpi(cmd,"COPY")) //copy,move
	{
//         struct ffblk ff;
	 char *src;

	 if(p->htmlframe[p->activeframe].cacheitem.rawname[0]
	    && file_exists(p->htmlframe[p->activeframe].cacheitem.rawname))
	  //rawname is not virtual - .JPG,.CNM
	  src=p->htmlframe[p->activeframe].cacheitem.rawname;
	 else
	  //rawname is not filename - .DGI
	  src=LASTlocname;
	 copy(src,value);
	 if(toupper(name[1])=='M' && file_exists(value))
	  unlink(src);
	}
	else if (!strcmpi(cmd,"PROFILE"))
	{
         config_set_str("Profile", value);
	 ie_savef(&ARACHNEcfg);
	 copy(ARACHNEcfg.filename,value);
	}
	else if (!strcmpi(cmd,"SCRNSVR"))
	{
	 strcpy(lasttime,"**"); //screensaver activated + redraw level 4
	 SecondsSleeping=32000l;
	 sprintf(GLOBAL.location,"arachne:internal-config?file:%s%sopt_misc.ah#scr",sharepath,GUIPATH);
	}
	else if (!strcmpi(cmd,"VGA"))
	{
	 strcpy(arachne.graphics,value);
	}
	else if (!strcmpi(cmd,"MODE"))
	{
	 if(strcmpi(arachne.graphics,"VGA") && strcmpi(arachne.graphics,"VGAMONO"))
	 {
	  if(value[0]=='.')
	   strcat(arachne.graphics,value);
	  else
	   strcpy(arachne.graphics,value);
//!!glennmcc: Oct 25, 2005 -- do not change style when resetting mode
//	  arachne.GUIstyle=8; //undefined.
	 }
	}
	else if (!strcmpi(cmd,"HTUSER"))
	{
	 strcpy(AUTHENTICATION->user,value);
	}
	else if (!strcmpi(cmd,"HTPASS"))
	{
	 strcpy(AUTHENTICATION->password,value);
	}

	else if (!strcmpi(cmd,"MSG")) //start mail msg
	{
	 char str[12];

	 sprintf(str,"%ld",time(NULL));
	 if (config_get_bool("KillSent", 0))
	  str[1]='!';

	 ptr = config_get_str("MailPath", "MAIL\\");

//!!glennmcc: Aug 27, 2006 -- add 'save as draft' function
//file extension will now be taken from the value of $MSG on mail screens
//(TBS for completed messages... DFT for drafts)
if(!strcmpi(value,"DFT")) sprintf(mailname,"%s%s.%s",ptr,&str[1],value);
else
	 sprintf(mailname,"%s%s.TBS",ptr,&str[1]);
//original single line above this comment
//!!glennmcc: end
	 mailmsg=a_open(mailname,O_CREAT|O_TEXT|O_WRONLY|O_TRUNC,S_IREAD|S_IWRITE);
	 if(mailmsg>=0)
	 {
	  char tm[30];
	  char *o,org[IE_MAXLEN+30]="\0";

	  inettime(tm);
	  o = config_get_str("Organization", NULL);
	  if(o)
	   sprintf(org,"Organization: %s\n",o);
	  sprintf(str,"From: \"%s\" <%s>\n%sDate: %s %s\nX-Mailer: Arachne v%s%s\n",
		      config_get_str("PersonalName", ""),
		      config_get_str("eMail", ""),
		      org,
		      tm,
		      config_get_str("TimeZone", "+0000"),
		      VER,beta);
	  write(mailmsg,str,strlen(str));
	 }
	}
	else if (!strcmpi(cmd,"DELORIG"))
	{
	 delorig=1;
	 GLOBAL.mailaction|=MAIL_OUTBOXNOW;
	}
	else if (!strcmpi(cmd,"ORIGMSG") && delorig)
	{
	 unlink(value);
	}
	else if (!strcmpi(cmd,"NOSIGN"))
	{
	 nosign=1;
	}
	else if (!strcmpi(cmd,"TO") && mailmsg>=0)
	{
	 adrfield(value);
	 sprintf(str,"To: %s\n",value);
//!!glennmcc: Dec 16, 2006 -- truncate to 127 (including 'To: ')
      if(strlen(str)>127)
	{
	 write(mailmsg,str,127);
	 write(mailmsg,"\n",1);
	}
	 else
//!!glennmcc: end
	 write(mailmsg,str,strlen(str));
	}
	else if (!strcmpi(cmd,"CC") && mailmsg>=0 && value[0])
	{
	 config_set_str(cmd, value);
	 adrfield(value);
	 sprintf(str,"CC: %s\n",value);
//!!glennmcc: Dec 16, 2006 -- truncate to 127 (including 'CC: ')
      if(strlen(str)>127)
	{
	 write(mailmsg,str,127);
	 write(mailmsg,"\n",1);
	}
	 else
//!!glennmcc: end
	 write(mailmsg,str,strlen(str));
	}
	else if (!strcmpi(cmd,"BCC") && mailmsg>=0 && value[0])
	{
	 config_set_str(cmd, value);
	 adrfield(value);
	 sprintf(str,"Bcc: %s\n",value);
//!!glennmcc: Dec 16, 2006 -- truncate to 90 (including 'Bcc: ')
      if(strlen(str)>90)
	{
	 write(mailmsg,str,90);
	 write(mailmsg,"\n",1);
	}
	 else
//!!glennmcc: end
	 write(mailmsg,str,strlen(str));
	}
//!!glennmcc: begin Oct 19, 2001
//added to create "reply-to" field in composed messages
//!!glennmcc: June 09, 2002
//changed so that replyto only gets added when "UseReplyto == Yes"
       else if (!strcmpi(cmd,"RT") && mailmsg>=0 && value[0])
       {
	if(config_get_bool("UseReplyto", 0) && value[0])
       {
	if(!value[0]) config_set_str(cmd, value);
	adrfield(value);
	sprintf(str,"Reply-To: %s\n",value);
//!!glennmcc: Dec 16, 2006 -- truncate to 127 (including 'Reply-To: ')
      if(strlen(str)>127)
	{
	 write(mailmsg,str,127);
	 write(mailmsg,"\n",1);
	}
	 else
//!!glennmcc: end
	write(mailmsg,str,strlen(str));
	}
       }
//!!glennmcc: end
//!!glennmcc: begin Oct 16, 2005
//added to create "Return-Receipt-To" field in composed messages
//will also add the format "Disposition-Notification-To"
else if (!strcmpi(cmd,"RRT") && mailmsg>=0 && value[0])
{
sprintf(str,"Return-Receipt-To: %s\n",value);
//!!glennmcc: Dec 16, 2006 -- truncate to 127
      if(strlen(str)>127)
	{
	 write(mailmsg,str,127);
	 write(mailmsg,"\n",1);
	}
	 else
//!!glennmcc: end
write(mailmsg,str,strlen(str));
sprintf(str,"Disposition-Notification-To: %s\n",value);
//!!glennmcc: Dec 16, 2006 -- truncate to 127
      if(strlen(str)>127)
	{
	 write(mailmsg,str,127);
	 write(mailmsg,"\n",1);
	}
	 else
//!!glennmcc: end
write(mailmsg,str,strlen(str));
}
//!!glennmcc: end

	else if (!strcmpi(cmd,"SUBJ") && mailmsg>=0)
	{
	 sprintf(str,"Subject: %s\n",value);
//!!glennmcc: Nov 18, 2005 -- truncate to 127 (including 'Subject: ')
//!!glennmcc: Mar 06, 2006 -- doubled the limit.
      if(strlen(str)>255)
	{
	 write(mailmsg,str,255);
	 write(mailmsg,"\n",1);
	}
	 else
//!!glennmcc: end
	 write(mailmsg,str,strlen(str));
	}
	else if (!strcmpi(cmd,"ATTACH") && mailmsg>=0 && value[0])
	{
	 sprintf(str,"X-Attachment: %s\n",value);
	 write(mailmsg,str,strlen(str));
	 attach=1;
	}
	else if (!strcmpi(cmd,"SMTP") && mailmsg>=0)
	 GLOBAL.mailaction|=MAIL_SMTPNOW;
	else if (!strcmpi(cmd,"FILENAME"))
	 strcpy(LASTlocname,value);
       }
       else
       {
	// update arachne.cfg ----------------------------------------------
	config_set_str(name, value);
       }

       modified=1;
      }//endif not CGI
     }

     //----------------------------------------------------------
     else if(subtype==TEXTAREA)
     {
      if(!strcmpi(name,"$BODY") && mailmsg>=0)
      {
       char *charset,*ptr;
       char encoding[STRINGSIZE];
       int len;

       if(attach)
       {
	sprintf(str,"X-Encoding: %s\n", config_get_str("MailEncoding", "MIME"));
	if(config_get_bool("UseCID", 0))
	 strcat(str,"X-cid: 1\n");
	if(config_get_bool("UseCDescr", 0))
	 strcat(str,"X-cdescr: 1\n");
	write(mailmsg,str,strlen(str));
       }

       charset = config_get_str("MyCharset", "US-ASCII");

       strcpy(encoding, config_get_str("MailBodyEncoding", "7bit"));

       sprintf(str,"MIME-Version: 1.0\nContent-type: text/plain; charset=%s\nContent-transfer-encoding: %s\n",charset,encoding);
       write(mailmsg,str,strlen(str));

       //append to mail message:
       i=0;
       while(i<tmpeditor.lines)
       {
	ptr=ie_getline(&tmpeditor,i);
	len=strlen(ptr);

	//quoted-printable:
	if(toupper(encoding[0])=='Q')
	{
	 int j=0,k=0;
	 while(j<len)
	 {
	  if(ptr[j]<32 || ptr[j]=='=')
	  {
	   sprintf(&(p->buf[k]),"=%02X",(unsigned char)ptr[j]);
	   k+=3;
	  }
	  else
	   p->buf[k++]=ptr[j];
	  j++;
	 }//loop
	 ptr=p->buf;
	 len=k;
	}//endif quoted-printable

	write(mailmsg,"\n",1);
	write(mailmsg,ptr,len);
	i++;
       }
       if(!nosign)
       {
	if(config_get_bool("UseSignature", 0))
	{
	 ptr = config_get_str("SignatureFile", NULL);
	 if(ptr)
	 {
	  f=a_open(ptr,O_RDONLY|O_TEXT,0);
	  if(f>=0)
	  {
	   i=a_read(f,&(p->buf[1]),BUF-1);
	   if(i>0)
	   {
	    p->buf[0]='\n';
	    write(mailmsg,p->buf,i+1);
	   }
	   a_close(f);
//!!glennmcc: begin July 8, 7 2002
//remove contents of textarea.tmp
//	tmpeditor.maxlines=-1;
//	strcpy(tmpeditor.filename,"textarea.tmp");
//	ie_savef(&tmpeditor);
//!!glennmcc: end

	  }//endif
	 }//endif
	}//endif
       }//end if modify/resend

//!!glennmcc: begin July 8, 7 2002
//remove contents of textarea.tmp
//!!glennmcc: July 18, 2002... moved to here
//the first try up-above did not work if modify/resend
//      tmpeditor.maxlines=-1;
//	strcpy(tmpeditor.filename,"textarea.tmp");
//	ie_savef(&tmpeditor);
//!!glennmcc: Aug 25, 2002...
// Michael says that just one line will do the same as my 3 lines.
//let's see if he knows his program better than I do ;-)
unlink("textarea.tmp");
//!!glennmcc: end
       }
     else
      {
       //save textarea to temporary file
       if(cgi)
#ifdef POSIX
       {
	strcpy(tmpeditor.filename,dotarachne);
	strcat(tmpeditor.filename,"textarea.tmp");
       }
#else
	strcpy(tmpeditor.filename,"textarea.tmp");
#endif
       ie_savef(&tmpeditor);
      }

      if(cgi)
      {
       //read it to query string
       querystring=ie_getswap(GLOBAL.postdataptr);
       if(!querystring)
	 MALLOCERR();
       if(qlen>0)
       {
	strcat(querystring,"&");
	qlen++;
       }

       strcat(querystring,name);
       strcat(querystring,"=");
       qlen+=strlen(name)+1;
       swapmod=1; //novy platny querystring:

       i=0;
       while(i<tmpeditor.lines && qlen<MAXQUERY-80-IE_MAXLEN)
       {
	ptr=ie_getline(&tmpeditor,i);
	if(ptr)
	{
	 strcpy(p->buf,ptr);
	 strcat(p->buf,"\r\n");
	 querystring=ie_getswap(GLOBAL.postdataptr);
	 if(!querystring)
	  MALLOCERR();
	 qlen+=cgiquery((unsigned char *)p->buf,(unsigned char *)&querystring[qlen],cgi&2); //cgi&2=true if http:..
	 swapmod=1; //novy platny querystring:
	}
	i++;
       }//loop
      }//end if cgi
     }//end if textarea

     //----------------------------------------------------------
     else if(subtype==SELECT && cgi)

     //Note: select tag cannot modify ARACHNE.CFG. It would be nice,
     //maybe when everything else is done we can take a look at it.
     {
      i=0;
      while(i<tmpeditor.lines && qlen<MAXQUERY-80-IE_MAXLEN)
      {
       ptr=ie_getline(&tmpeditor,i);
       if(ptr)
       {
	if(*ptr=='1') //selected value:
	{
	 if(ptr[1])
	  strcpy(p->buf,&ptr[1]);
	 else
//!!glennmcc: Jan 11, 2006 -- if value="".... value=(blank)
//!!glennmcc: Jan 31, 2007 -- bad idea...
// both attempts cause too many other problems :(
/*
//	  if (strnicmp(GLOBAL.location,"file:",5)!=0 && i==0)
	  if (strnicmp(GLOBAL.location,"file:",5)!=0 && strlen(ptr)<3)
	   strcpy(p->buf,"\0"); else
*/
//!!glennmcc: end
	  makestr(p->buf,ie_getline(&tmpeditor,i+1),BUF);

	 querystring=ie_getswap(GLOBAL.postdataptr);
	 if(!querystring)
	  MALLOCERR();
	 if(qlen>0)
	 {
	  strcat(querystring,"&");
	  qlen++;
	 }
	 strcat(querystring,name);
	 strcat(querystring,"=");
	 qlen+=strlen(name)+1;
	 qlen+=cgiquery((unsigned char *)p->buf,(unsigned char *)&querystring[qlen],cgi&2); //cgi&2=true if http:..
	 swapmod=1; //novy platny querystring:
	}
       }
       i+=2;
      }//loop

     }//end if selecy
    }
    else
      MALLOCERR();

   }
   //endif
//   HTMLdoc.cur++;
  }//loop

  if((cgi&4 || mailmsg>=0) &&
     activeatomptr &&
     activeatomptr->type==IMG)//INPUT TYPE=IMAGE
  {
   int dx,dy;
   struct picinfo *img=(struct picinfo *)ie_getswap(activeatomptr->ptr);
   char alt[80]="\0";

   if(img)
    strcpy(alt,img->alt);

   if (!strcmpi(alt,"$SMTP") && mailmsg>=0)
    GLOBAL.mailaction|=MAIL_SMTPNOW;
   else
   {
    querystring=ie_getswap(GLOBAL.postdataptr);
    if(!querystring)
      MALLOCERR();
    if(qlen>0)
     strcat(querystring,"&");
    if(activeismap(&dx,&dy))
    {
     char num[20];
     strcat(querystring,alt);
     sprintf(num,".x=%d&",mousex-dx);
     strcat(querystring,num);
     strcat(querystring,alt);
     sprintf(num,".y=%d",mousey-dy);
     strcat(querystring,num);
     swapmod=1; //novy platny querystring:
    }
   }
  }

  if(mailmsg>=0)
  {
   if(attach)
   {
    char str[128];
    sprintf(str,"@INSIGHT -a %s\n",mailname);
#ifdef POSIX
    system(str);
#else
    closebat(str,RESTART_TEST_ERRORLEVEL);
    GLOBAL.willexecute=willexecute(str);
#endif
   }
   a_close(mailmsg);
  }
  if(cgi==0 && modified)
  {
   if (!config_get_bool("SavePasswords", 1))
    strcpy(ARACHNEcfg.killstr,"sword ");
   else
    ARACHNEcfg.killstr[0]='\0';
   ie_savef(&ARACHNEcfg);
  }

  if(GLOBAL.mailaction & MAIL_OUTBOXNOW)
  {
   sprintf(GLOBAL.location,"file://outbox.dgi");
   GLOBAL.reload=RELOAD_NEW_LOCATION;
  }

  if(GLOBAL.mailaction & MAIL_SMTPNOW)
   sprintf(GLOBAL.location,"smtp:/%s",mailname);

  if(attach)
   GLOBAL.mailaction|=MAIL_ATTACH;

  mouseon();
 }

} //end sub


//konverze nealfanumericky znaku v cgi query-stringu:

int cgiquery(unsigned char *in,unsigned char *out,char http)
{
 int i=0,j=0,l=strlen((char *)in);

 if(in)
 {
  while(j<l)
  {
   /*
   if(!http && (in[j]==' ' || in[j]=='\n'))
   {
    out[i]='+';
    i++;
   }
   else
   */
   if(http && !isalnum(in[j]) && in[j]!='_' || in[j]==' ' || in[j]=='\n' || in[j]=='\r')
   {
    sprintf((char *)&out[i++],"%%%2X",in[j]);
    if(out[i]==' ')
     out[i]='0';
    i+=2;
   }
   else
   {
    out[i]=in[j];
    i++;
   }
   j++;
  }//loop
 }
 out[i]='\0';

 //according to RFC xyz
 return i;
}

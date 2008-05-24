
// ========================================================================
// Arachne (Webspyder) DIALER interface
// (c)1997 xChaos software
// ========================================================================

#include "arachne.h"

#ifndef NOTCPIP
#ifndef NOETHERPPP

#define LOGLEN 1000

//analyza PPP logu -> ziskani IP adresy
//!!JdS: 2003/12/7 {
//  Rewritten so that 'ppplogtime' can also be initialized when
//  using BOOTP via a dial-up connection. See also ArachneTCPIP().
//!!JdS: 2003/12/7 }
int PPPlog(void)
{
 int f,i,j;
 char *p;
 struct ftime ft;
 struct time d_time;
 struct date d_date;

 f=sopen("PPP.LOG",O_RDONLY|O_TEXT,SH_COMPAT, S_IREAD);
 if (f>=0)
 {
  if (ipmode==MODE_PPP)
  {
   long l=a_filelength(f);
   int plen;
   char buf[LOGLEN];

   if (l>LOGLEN)
    l=LOGLEN;
   lseek(f,-l,SEEK_END);
   i=read(f,buf,(int)l);
   buf[i]='\0';

   //puts(buf);
   j=0;
   p=configvariable(&ARACHNEcfg,"IP_Grab",NULL);
   if (!p)
    p="IP address set to";
   plen=strlen(p);
   while(j<i)
   {
    if (!strncmp(&buf[j],p,plen))
    {
     char tecka=0;

     p=strchr(&buf[j],'\n');
     if (p)
      *p='\0';

     outs(&buf[j]);
     i=0;
     j+=plen;
     while(buf[j]==' ')j++;
     while(i<19 &&
	   (buf[j+i]>='0' && buf[j+i]<='9' ||
	    buf[j+i]=='.' && tecka<3))
     {
      myIPstr[i]=buf[j+i];
      if (myIPstr[i]=='.')
       tecka++;
      i++;
     }
     myIPstr[i]='\0';
     my_ip_addr = resolve(myIPstr);
     break;
    }
    j++;
   }
  }

  //determine time online:
  if (my_ip_addr || ipmode==MODE_BOOTP)
  {
   getftime(f, &ft);
   d_date.da_year=ft.ft_year+1980;     /* current year */
   d_date.da_day=ft.ft_day;     /* day of the month */
   d_date.da_mon=ft.ft_month;     /* month (1 = Jan) */
   d_time.ti_min=ft.ft_min;   /* minutes */
   d_time.ti_hour=ft.ft_hour;  /* hours */
   d_time.ti_hund=0;  /* hundredths of seconds */
   d_time.ti_sec=ft.ft_tsec*2;   /* seconds */
   //ppplogtime is by default zero...
   ppplogtime = dostounix(&d_date, &d_time);
  }

  close(f);
  return(my_ip_addr || ipmode==MODE_BOOTP);
 }

 //puts(MSG_NOIP);
 return(0);
}
#endif
#endif


char *ArachneDIAL(void)
{
 char *value;
 char str[80];
 char terminal[80]="\0",hangup[80]="\0",useterm=0,dospppd=0;
 char *pausemsg=ctrlbreak;
 char buf[5*IE_MAXLEN];
 char altnameserver[IE_MAXLEN];
 char *altdns="\0";
//!!glennmcc: Mar 06, 2006 -- allow use of %DNS1 with miniterm/epppd
 char nameserver[IE_MAXLEN];
 char *dns="\0";
//!!glennmcc: end

 process_form(0,IE_NULL); //updateovat Arachne.Cfg

 if(tcpip)
 {
  value=configvariable(&ARACHNEcfg,"Hangup",NULL);
  if(value)
  {
   makestr(hangup,value,128);
   strcat(hangup,"\n");
  }
 }

 value=configvariable(&ARACHNEcfg,"UseTerminal",NULL);
 if(value && toupper(*value)=='Y')
 {
  value=configvariable(&ARACHNEcfg,"TerminalWindow",NULL);
  useterm=1;
 }
 else
  value=configvariable(&ARACHNEcfg,"Dialer",NULL);

 if(value && strcmpi(value,"NUL"))
 {
  makestr(terminal,value,128);
  strcat(terminal,"\n");
 }
 else
  useterm=0;

#ifndef NOETHERPPP
 value=configvariable(&ARACHNEcfg,"Connection",NULL);
 if(!value)
  return ("");

 /*
 if(!strncmpi(value,"@PPP.EXE",8))
 {
  int f,l;

  //vytvorit CONFIG.PPP
  f=a_open(configvariable(&ARACHNEcfg,"PPPconfig",NULL),O_RDONLY|O_TEXT,0);
  if(f!=-1)
  {
   l=a_read(f,buf,BUF/2);
   buf[l]='\0';
   a_close(f);
   sprintf(&buf[BUF/2+1],buf,
     configvariable(&ARACHNEcfg,"PPPusername",NULL),
     configvariable(&ARACHNEcfg,"PPPpassword",NULL));
   f=a_open("config.ppp",O_TEXT|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
   write(f,&buf[BUF/2+1],strlen(&buf[BUF/2+1]));
   a_close(f);
  }


 }//end if ETHERPPP
 else
 */
//!!glennmcc: begin Dec 24, 2000
// changed "PPPD.EXE to "PPP" so that *any* .EXE, .COM or .BAT
// with "PPP" in the name will still result in a new PPPDRC.CFG file
// examples... epppd.exe ppp.exe myppp.exe lsppp.exe loadppp.bat ppp_drv.com
// if(strstr(strupr(value),"PPPD.EXE"))

//!!glennmcc: July 6, 2002 always write pppdrc.cfg
// if(strstr(strupr(value),"PPP"))
//!!glennmcc: end

//!!glennmcc: Jan 16, 2008 -- write data into both pppdrc.cfg & lsppp.cfg
 {
  int f=a_open("pppdrc.cfg",O_TEXT|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
  int f2=a_open("lsppp.cfg",O_TEXT|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
  if(f!=-1 && f2!=-1)
  {
   char *base,*default_base[4]={"0x3f8","0x2f8","0x3e8","0x2e8"};
   char *irq,*default_irq[4]={"4","3","4","3"};
   int port=0;

   value=configvariable(&ARACHNEcfg,"Port",NULL);
   if(value)
    port=atoi(value)-1;

   if(port>=0 && port<4)
   {
    irq=default_irq[port];
    base=default_base[port];
   }
   else
   {
    irq=configvariable(&ARACHNEcfg,"Irq",NULL);
    base=configvariable(&ARACHNEcfg,"Base",NULL);
   }

   dns=configvariable(&ARACHNEcfg,"NameServer",NULL);
   if(*dns=='%')
     {
      char *ptr=strchr(&dns[1],'%');
      if(ptr) *ptr='\0';
      dns=getenv(&dns[1]);
     }
   if(strlen(dns)>6)
      sprintf(nameserver,"namsrv %s\n",dns);
      else
      strcpy(nameserver,"\0");

   altdns=configvariable(&ARACHNEcfg,"AltNameServer",NULL);
   if(*altdns=='%')
     {
      char *ptr=strchr(&altdns[1],'%');
      if(ptr) *ptr='\0';
      altdns=getenv(&altdns[1]);
     }
   if(strlen(altdns)>6)
      sprintf(altnameserver,"namsrv %s\n",altdns);
      else
      strcpy(altnameserver,"\0");

//write pppdrc.cfg
   sprintf(buf,"\
%s\n\
irq %s\n\
base %s\n\
modem\n\
crtscts\n\
asyncmap 0\n\
%s\
%s\
user \"%s\"\n\
passwd \"%s\"\n",
   configvariable(&ARACHNEcfg,"Speed",NULL),
   irq,base,
   nameserver,
   altnameserver,
   configvariable(&ARACHNEcfg,"PPPusername",NULL),
   configvariable(&ARACHNEcfg,"PPPpassword",NULL));
   write(f,buf,strlen(buf));
   a_close(f);

//write lsppp.cfg
   sprintf(buf,"\
/M:%s\n\
/d:%s\n\
/B:%s\n\
/i:%s\n\
/b:%s\n\
/U:%s\n\
/P:%s\n\
/V:60\n",
   configvariable(&ARACHNEcfg,"InitString",NULL),
   configvariable(&ARACHNEcfg,"PhoneNumber",NULL),
   configvariable(&ARACHNEcfg,"Speed",NULL),
   irq,base,
   configvariable(&ARACHNEcfg,"PPPusername",NULL),
   configvariable(&ARACHNEcfg,"PPPpassword",NULL));
//!!glennmcc: Feb 14, 2008 -- also write DNSs into lsppp.cfg
if(strlen(dns)>6)
 {
  strcat(buf,"/N:");
  strcat(buf,dns);
 }
if(strlen(altdns)>6)
 {
  strcat(buf,",");
  strcat(buf,altdns);
 }
if(strlen(dns)>6)
   strcat(buf,"\n");
//!!glennmcc: end, Feb 14, 2008
   write(f2,buf,strlen(buf));
   a_close(f2);
  }
  dospppd=1;
  pausemsg=MSG_ESC;
 }
#endif

 value=configvariable(&ARACHNEcfg,"Connection",NULL);
 sprintf(buf,"%s%s@if errorlevel 1 goto skip\n%s\n:skip\n",hangup,terminal,value);

#ifndef NOETHERPPP
 if(dospppd)
 {
  strcat(buf,"@if exist IP-UP.BAT call IP-UP.BAT\n@echo PPPD status: ");
  value=configvariable(&ARACHNEcfg,"IP_Grab",NULL);
  strcat(buf,value);
  strcat(buf," %MYIP%>>PPP.LOG\n");
  unlink("IP-UP.BAT");
  unlink("PPP.LOG");
 }
#endif

 value=configvariable(&ARACHNEcfg,"DialPage",NULL);
if(!strstr(strlwr(value),".htm")) value="file:ppp_init.htm";
 if((!strcmpi(value,p->htmlframe[p->activeframe].cacheitem.URL) ||
     strstr(p->htmlframe[p->activeframe].cacheitem.URL,"err_")) &&
     arachne.scriptline==0)
  strcat(buf,"@arachne.bat -o\n");
 else
  strcat(buf,"@arachne.bat -r\n");


 if(useterm)
  x_grf_mod(3);
 else
 {
  sprintf(str,MSG_DIAL,pausemsg);
  outs(str);
 }
 return buf;
}

/*
  //vytvorit DIAL.PPP
  buf[BUF/2+1]='\0';
  value=configvariable(&ARACHNEcfg,"CreateScript",NULL);
  if(value && (*value=='Y'||*value=='y'))
  {
   int i=1;
   char kwd[10];

   sprintf(&buf[BUF/2+1],"\
send \"ath0\\r\"\n\
recv 3000 \"OK\"\n\
send \"%s\\r\"\n\
recv 3000 \"OK\"\n\
send \"%s,%s\\r\"\n\
recv 60000 \"CONNECT\"\n",
     configvariable(&ARACHNEcfg,"InitString",NULL),
     configvariable(&ARACHNEcfg,"DialString",NULL),
     configvariable(&ARACHNEcfg,"PhoneNumber",NULL));

   while(i<10)
   {
    sprintf(kwd,"Waitfor%d",i);
    value=configvariable(&ARACHNEcfg,kwd,NULL);
    if(value)
    {
     strcat(&buf[BUF/2+1],"recv 10000 \"");
     strcat(&buf[BUF/2+1],value);
     strcat(&buf[BUF/2+1],"\"\n");
    }
    sprintf(kwd,"Response%d",i);
    value=configvariable(&ARACHNEcfg,kwd,NULL);
    if(value)
    {
     strcat(&buf[BUF/2+1],"send \"");
     strcat(&buf[BUF/2+1],value);
     strcat(&buf[BUF/2+1],"\\r\"\n");
    }
    i++;
   }//loop
  }
  else
  {
   if(terminal[0])
   {
    buf[BUF/2+1]='\0';
   }
   else
   {
    f=a_open(configvariable(&ARACHNEcfg,"DialScript",NULL),O_RDONLY|O_TEXT);
    if(f!=-1)
    {
     l=a_read(f,buf,BUF/2);
     buf[l]='\0';
     a_close(f);
     sprintf(&buf[BUF/2+1],buf,
      configvariable(&ARACHNEcfg,"InitString",NULL),
      configvariable(&ARACHNEcfg,"DialString",NULL),
      configvariable(&ARACHNEcfg,"PhoneNumber",NULL),
      configvariable(&ARACHNEcfg,"PPPusername",NULL),
      configvariable(&ARACHNEcfg,"PPPpassword",NULL));
    }
   }
  }//end if create script

  f=a_open("dial.ppp",O_TEXT|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
  if(f!=-1)
  {
   write(f,&buf[BUF/2+1],strlen(&buf[BUF/2+1]));
   a_close(f);
  }
*/



// ========================================================================
// Arachne (Webspyder) DIALER interface
// (c)1997 xChaos software
// ========================================================================

#include "arachne.h"

#ifndef NOTCPIP
#ifndef NOETHERPPP
//analyza PPP logu -> ziskani IP adresy
int PPPlog(void)
{
 int f,i,j;
 char *p;

 f=sopen("PPP.LOG",O_RDONLY|O_TEXT,SH_COMPAT, S_IREAD);
 if(f>=0)
 {
  long l=a_filelength(f);
  int plen;
  struct ftime ft;
  struct time d_time;
  struct date d_date;

  if(l>1000L)
   l=1000L;
  lseek(f,-l,SEEK_END);
  i=read(f,buf,(int)l);
  buf[i]='\0';

  //puts(buf);
  j=0;
  p=configvariable(&ARACHNEcfg,"IP_Grab",NULL);
  if(!p)
   p="IP address set to";
  plen=strlen(p);
  while(j<i)
  {
   if(!strncmp(&buf[j],p,plen))
   {
    char tecka=0;

    p=strchr(&buf[j],'\n');
    if(p)*p='\0';

    outs(&buf[j]);
    i=0;
    j+=plen;
    while(buf[j]==' ')j++;
    while(i<19 &&
          (buf[j+i]>='0' && buf[j+i]<='9' ||
           buf[j+i]=='.' && tecka<3))
    {
     myIPstr[i]=buf[j+i];
     if(myIPstr[i]=='.')
      tecka++;
     i++;
    }
    myIPstr[i]='\0';
    my_ip_addr = resolve(myIPstr);

    //determine time online:

    if(my_ip_addr)
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

    return (1);
   }
   j++;
  }
  close(f);
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

    process_form(0,IE_NULL); //updateovat Arachne.Cfg

    if(tcpip)
    {
     value=configvariable(&ARACHNEcfg,"Hangup",NULL);
     if(value)
     {
      makestr(hangup,value,78);
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
     makestr(terminal,value,78);
     strcat(terminal,"\n");
    }
    else
     useterm=0;

#ifndef NOETHERPPP
    value=configvariable(&ARACHNEcfg,"Connection",NULL);
    if(!value)
     return ("");

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
    if(!strncmpi(value,"@PPPD.EXE",9) || !strncmpi(value,"@EPPPD.EXE",10))
    {
     int f=a_open("pppdrc.cfg",O_TEXT|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
     if(f!=-1)
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
      sprintf(buf,"\
%s\n\
irq %s\n\
base %s\n\
modem\n\
crtscts\n\
asyncmap 0\n\
namsrv %s\n\
namsrv %s\n\
user \"%s\"\n\
passwd \"%s\"\n",
      configvariable(&ARACHNEcfg,"Speed",NULL),
      irq,base,
      configvariable(&ARACHNEcfg,"NameServer",NULL),
      configvariable(&ARACHNEcfg,"AltNameServer",NULL),
      configvariable(&ARACHNEcfg,"PPPusername",NULL),
      configvariable(&ARACHNEcfg,"PPPpassword",NULL));
      write(f,buf,strlen(buf));
      a_close(f);
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

#ifdef CALDERA
    strcat(buf,"@webspydr.bat -o\n");
#else
    value=configvariable(&ARACHNEcfg,"DialPage",NULL);
    if((!strcmpi(value,htmlframe[activeframe].cacheitem.URL) ||
        strstr(htmlframe[activeframe].cacheitem.URL,"err_")) &&
        arachne.scriptline==0)
     strcat(buf,"@arachne.bat -o\n");
    else
     strcat(buf,"@arachne.bat -r\n");
#endif // CALDERA


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


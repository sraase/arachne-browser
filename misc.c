
// ========================================================================
// Miscelaneous Arachne WWW browser componentes
// (c)1997 xChaos software, portions (c)1987 Caldera Inc.
// ========================================================================

#include "arachne.h"
#include "html.h"
#include "xanimgif.h"

// "multitasking" ;-)

#ifndef POSIX
void closebat(char *cmd, char nomode)
{

 if(nomode==RESTART_TEST_ERRORLEVEL)
 {
/*
#ifdef CALDERA
  strcat(cmd,"if errorlevel 128 webspydr -");
#else
*/
  strcat(cmd,"if errorlevel 128 arachne -");
//#endif // CALDERA
  if(tcpip)
   strcat(cmd,"re");
  else
   strcat(cmd,"ce");
 }

 strcat(cmd,"\narachne -");
 if(tcpip)
  strcat(cmd,"r");
 else
  strcat(cmd,"c");

 if(nomode==RESTART_NONFATAL_ERROR)
  strcat(cmd,"e");
 else
 if(nomode && !fullscreen)
  strcat(cmd,"g");

 strcat(cmd,"\n");
}

#endif //POSIX

//
void Goto_A_NAME(char *name) //name je pointer do xSwapu!!!
{
 long Y;
 char a_name[80];
 unsigned currentHTMLatom=firstHTMLatom;
 struct HTMLrecord *atomptr;

 strcpy(a_name,name);
 while(currentHTMLatom!=IE_NULL)
 {
  kbhit();
  atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if(!atomptr)
   MALLOCERR();
  currentHTMLatom=atomptr->next;
  if(atomptr->type==NAME)
  {
   char *ptr=ie_getswap(atomptr->ptr);
   Y=atomptr->y;
   if((!strcmpi(ptr,a_name) || *ptr=='#' && !strcmpi(ptr+1,a_name) ) &&
   atomptr->frameID==arachne.target)
   {
    long oldY=htmlframe[arachne.target].posY;
    htmlframe[arachne.target].posY=Y;
    if(htmlframe[arachne.target].posY>htmlframe[arachne.target].scroll.total_y-htmlframe[arachne.target].scroll.ysize)
    {
     htmlframe[arachne.target].posY=htmlframe[arachne.target].scroll.total_y-htmlframe[arachne.target].scroll.ysize;
     if(htmlframe[arachne.target].posY<0)
      htmlframe[arachne.target].posY=0;
    }
    if(htmlframe[arachne.target].posY!=oldY || GLOBAL.norefresh)
    {
     if(GLOBAL.norefresh)
      redrawHTML(REDRAW_WITH_MESSAGE,REDRAW_CREATE_VIRTUAL);
     else
      redrawHTML(REDRAW_WITH_MESSAGE,REDRAW_VIRTUAL);
    }
    return;
   }
  }
 }//loop

 Piip();
 if(GLOBAL.norefresh)
  redrawHTML(REDRAW_WITH_MESSAGE,REDRAW_CREATE_VIRTUAL);
}


int addhot(char *titleptr, char *urlptr)
{
 int f=-1;
 char line[256], *ptr;

 ptr=configvariable(&ARACHNEcfg,"Hotlist",NULL);
 if(!ptr)
  ptr=hotlist;

#ifdef POSIX
 f=a_open(ptr,O_WRONLY|O_APPEND,0);
#else
 f=a_open(ptr,O_WRONLY|O_TEXT|O_APPEND,0);
#endif

 if(f==-1)
 {
#ifdef POSIX
  f=a_open(hotlist,O_WRONLY|O_APPEND,0);
#else
  f=a_open(hotlist,O_WRONLY|O_TEXT|O_APPEND,0);
#endif
  if(f<0)
  {
   sprintf(line,"%s%s",exepath,hotlist);
   copy(line,ptr);
#ifdef POSIX
   f=a_open(ptr,O_WRONLY|O_APPEND,0);
#else
   f=a_open(ptr,O_WRONLY|O_TEXT|O_APPEND,0);
#endif
   if(f<0)
#ifdef POSIX
    f=a_open(ptr,O_CREAT|O_WRONLY,S_IREAD|S_IWRITE);
#else
    f=a_open(ptr,O_CREAT|O_TEXT|O_WRONLY,S_IREAD|S_IWRITE);
#endif
  }
 }

 if(!urlptr[0])
  return 0;

 if(f!=-1)
 {
  strcpy(line,"<LI><A HREF=\"");
  write(f,line,strlen(line));
  write(f,urlptr,strlen(urlptr));
  write(f,"\">",2);
  write(f,titleptr,strlen(titleptr));
  write(f,"</A>\n",5);
  a_close(f);
  return 1;
 }
 return 0;
}



void copy(char *from,char *to)
{
 int f1,f2,l;

 f1=a_open(from,O_BINARY|O_RDONLY,0);
 f2=a_open(to,O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
 if(f1>=0 || f2>=0)
 {
  do
  {
   l=a_read(f1,buf,BUF);
   if(l>0)
    write(f2,buf,l); // error checking has to be added later !!!!
  }
  while(l==BUF);
  a_close(f2);
  a_close(f1);
 }
}

char goback(void)
{
 char *link;

// printf("arachne.history=%d\n",arachne.history);

 if(arachne.history>1) //historie [<-]
  link=ie_getline(&history,--arachne.history);
 else
  link=NULL;

 if(link)
 {
  strcpy(GLOBAL.location,link);
  GLOBAL.nothot=1;
  if(arachne.backtrace>1)
  {
   arachne.target=arachne.backtrace_target[arachne.backtrace-2];
   arachne.backtrace--;
  }

  return 1;
 }
 else
 {
  return 0;
 }
}

char externalprotocol(char *protocol,char *command)
{
 char str[80], ext[8];

 sprintf(str,"external/%s",protocol);
 ext[0] = '\0';
 return call_plugin(str, command, ext);
}

// Piiiiiiipnutiiiiiii =======================================

//. 7 is the ascii for beep
void Piip(void)
{
   printf("%c",7);
   return;
}


/* getnumbers - returns the count of numbers received */
int getnumbers( char *ascii, long *d1, long *d2 )
{
    char *p;
    /* it must return a number after the white space */
    if (( p = strchr( ascii, ' ')) == NULL ) return( 0 );

    /* skip space */
    while ( *p == ' ') p++;
    *d1 = atol( p );

    if (( p = strchr( p, ' ')) == NULL ) return( 1 );

    /* skip space */
    while ( *p == ' ') p++;
    *d2 = atol( p );
    return( 2 );
}


long diskspace[27]={0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l};

//#include <stdio.h>

#ifndef POSIX
long updtdiskspace(char *path)
{
 struct dfree d;
 int dr=0;

 if(path[1]==':')
  dr=toupper(path[0])-'@';

 getdfree( dr, &d );

 diskspace[dr] = (long) d.df_avail * (long)d.df_bsec * (long)d.df_sclus;

// printf("%c: %ld bytes free\n",'@'+dr,diskspace[dr]);

 return diskspace[dr];

}

long lastdiskspace(char *path)
{
 int dr=0;

 if(path[1]==':')
  dr=toupper(path[0])-'@';

 if(diskspace[dr]==0l)
  return updtdiskspace(path);
 else
  return diskspace[dr];
}


//muze se hodit
long localdiskspace(void)
{
 return(lastdiskspace("@:"));
}
#endif

//make cmd according to MIME.CFG:

int make_cmd(char *in, char *out, char *url, char *computer, char *document, char *infile, char *outfile)
//returns 0...batch -1...textmode batch nn... need nn kilobytes
//-2 ... graphics mode, but outputs to screen
{
 int rv=0;
 char *pom;

 if(*in=='@')
 {
  in++;
  rv=-1;
 }
 else
 if(*in=='*')
 {
  in++;
  rv=-2;
 }
 else
 if(*in=='[')
 {
  pom=strchr(in,']');
  if(pom)
  {
   *pom='\0';
   rv=atoi(&in[1]);
   in=pom+1;
  }
 }

 while(*in)
 {
  if(*in=='\\')
  {
   in++;
   if(toupper(*in)=='N')
    *out='\n';
   else
    *out=*in;
  }
  else
  if(*in=='$' || *in=='%')
  {
   in++;

   switch(toupper(*in))
   {
    case '1':
    strcpy(out,infile);
    out+=strlen(infile)-1;
    break;
    case '2':
    strcpy(out,outfile);
    out+=strlen(outfile)-1;
    break;
    case 'C':
    strcpy(out,HTTPcache.filename);
    out+=strlen(HTTPcache.filename)-1;
    ie_savebin(&HTTPcache);
    break;
    case 'H':
    strcpy(out,history.filename);
    out+=strlen(history.filename)-1;
    ie_savef(&history);
    break;
    case 'P':
    strcpy(out,computer);
    out+=strlen(computer)-1;
    break;
    case 'D':
    strcpy(out,document);
    out+=strlen(document)-1;
    break;
    case 'I':
    strcpy(out,myIPstr);
    out+=strlen(myIPstr)-1;
    break;
    case 'E':
    strcpy(out,exepath);
    out+=strlen(exepath)-1;
    break;
    case 'Q':
    case 'S':
    if(GLOBAL.postdata)
    {
     pom=ie_getswap(GLOBAL.postdataptr);
     if(pom)
     {
      if(*in=='S' || *in=='s')
      {
       int l=strlen(pom);
       if(l>80)l=80;
       strncpy(out,pom,l);
       out+=l-1;
      }
      else
      {
       char q[80];
       int f;

       tempinit(q);
       strcat(q,"$query$.tmp");
       f=a_fast_open(q,O_BINARY|O_CREAT|O_TRUNC|O_WRONLY,S_IREAD|S_IWRITE);
       if(f>=0)
       {
        write(f,pom,strlen(pom));
	a_close(f);
       }
       strcpy(out,q);
       out+=strlen(q)-1;
      }
     }
    }
    break;
    case 'U':
    strcpy(out,url);
    out+=strlen(url)-1;
    break;
    case 'R':
    {
     char x[10];
     sprintf(x,"%d",htmlframe[arachne.target].scroll.xsize);
     strcpy(out,x);
     out+=strlen(x)-1;
    }
    break;
    case 'T':
    if(!user_interface.cache2temp)
     goto a;
    {
     char temp[80];
     tempinit(temp);
     strcpy(out,temp);
     out+=strlen(temp)-1;
    }
    break;
    case 'L':
    strcpy(out,LASTlocname);
    out+=strlen(LASTlocname)-1;
    break;

    case 'J':  //DJPEG.EXE arguments
    if(xg_256 != MM_Hic)
    {
     strcpy(out,"-colors 256 ");
     out+=12;
    }
    pom=configvariable(&ARACHNEcfg,"JPEGargs",NULL);
    if(!pom)
     pom="-bmp ";
    goto cont;

    case 'F':  //file browser WWWMANE.EXE mode
    pom=configvariable(&ARACHNEcfg,"FILEargs",NULL);
    if(!pom)
     pom="-d";
    goto cont;
    case 'M':
    pom=configvariable(&ARACHNEcfg,"MailPath",NULL);
    goto cont;
    case 'W':
    pom=configvariable(&ARACHNEcfg,"DownloadPath",NULL);
    goto cont;
    case 'N':
    pom=configvariable(&ARACHNEcfg,"NameServer",NULL);
    goto cont;
    case 'G':
    pom=configvariable(&ARACHNEcfg,"Gateway",NULL);
    goto cont;
    case 'X':
    pom=configvariable(&ARACHNEcfg,"Netmask",NULL);
    goto cont;
    case 'A':
    a:
    pom=configvariable(&ARACHNEcfg,"CachePath",NULL);
    goto cont;
    case 'B':
    pom=configvariable(&ARACHNEcfg,"Hotlist",NULL);
    cont:
    if(pom)
    {
     strcpy(out,pom);
     out+=strlen(pom)-1;
    }
    break;
    default:
    *out=*in;
   }
  }
  else
   *out=*in;
  out++;
  in++;
 }
 *out=*in;
 return rv;
}


void makeexestr(char *exestr)
{
#ifdef MSDOS
 if(tcpip)
  sprintf(exestr,"DOS x86;WATTCP/%d.%02d%s",WTCP_VER>>8,WTCP_VER&0xff,exetype);
 else
  sprintf(exestr,"DOS x86;httpstub%s",exetype);
#elif LINUX
  char *ostype=getenv("OSTYPE");
  char *hosttype=getenv("HOSTTYPE");
  if(!ostype)
   ostype="Linux";
  if(!hosttype)
   hosttype="i386";
  sprintf(exestr,"%s %s%s",ostype,hosttype,exetype);
#else
  sprintf(exestr,"Clementine%s",exetype);
#endif
}

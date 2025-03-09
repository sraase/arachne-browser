
// ========================================================================
// Miscelaneous Arachne WWW browser componentes
// (c)1997 xChaos software, portions (c)1987 Caldera Inc.
// ========================================================================

#include "arachne.h"
#include "html.h"
#include "xanimgif.h"

int kbhit(void);

// "multitasking" ;-)

#ifndef POSIX
void closebat(char *cmd, char nomode)
{

 if(nomode==RESTART_TEST_ERRORLEVEL)
 {
  strcat(cmd,"if errorlevel 128 arachne -");
  if(tcpip)
//!!glennmcc: begin Jan 09, 2005
// on email attachment errors, goto err_mail.ah instead of nonfatal.ah
   strcat(cmd,"rm");
  else
   strcat(cmd,"cm");
//   strcat(cmd,"re");
//  else
//   strcat(cmd,"ce");
//!!glennmcc: end
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
 unsigned currentHTMLatom=p->firstHTMLatom;
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
    long oldY=p->htmlframe[arachne.target].posY;
    p->htmlframe[arachne.target].posY=Y;
    if(p->htmlframe[arachne.target].posY>
       p->htmlframe[arachne.target].scroll.total_y-p->htmlframe[arachne.target].scroll.ysize)
    {
     p->htmlframe[arachne.target].posY=
      p->htmlframe[arachne.target].scroll.total_y-p->htmlframe[arachne.target].scroll.ysize;
     if(p->htmlframe[arachne.target].posY<0)
      p->htmlframe[arachne.target].posY=0;
    }
    if(p->htmlframe[arachne.target].posY!=oldY || GLOBAL.norefresh)
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

 ptr = config_get_str("Hotlist", hotlist);

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
   sprintf(line,"%s%s",userpath,hotlist);
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
   l=a_read(f1,p->buf,BUF);
   if(l>0)
    write(f2,p->buf,l); // error checking has to be added later !!!!
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
 return search_mime_cfg(str, ext, command);
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

int paranoid_strncpy(char *dest,char *src,int max)
{
 int count=0;
 while (count<max && *src)
 {
  if(!strchr("|;&>`\'\"",*src))
   dest[count++]=*src;
  src++;
 }
 dest[count]='\0';
 return count;
}

//make cmd according to MIME.CFG:

int make_cmd(char *in, char *out, char *url, char *computer, char *document, char *infile, char *outfile)
//returns 0...batch -1...textmode batch nn... need nn kilobytes
//-2 ... graphics mode, but outputs to screen
{
 int rv=0;
 char *pom;

//!!glennmcc: begin Apr 23, 2002
//needed for $K
 char *cfgkw="_";
 int kcnt=0;
//!!glennmcc: end

//!!glennmcc: Dec 06 ,2006
//needed for $Z
 char *drive="\0", *dir="\0", *file="\0", *ext="\0";
//!!glennmcc: end

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
    out+=paranoid_strncpy(out,infile,80)-1;
    break;
    case '2':

    out+=paranoid_strncpy(out,outfile,80)-1;
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
    out+=paranoid_strncpy(out,computer,STRINGSIZE)-1;
    break;
    case 'D':
    out+=paranoid_strncpy(out,document,STRINGSIZE)-1;
    break;
    case 'I':
    strcpy(out,myIPstr);
    out+=strlen(myIPstr)-1;
    break;
    case 'E':
    strcpy(out,syspath);
    out+=strlen(syspath)-1;
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
    out+=paranoid_strncpy(out,url,80)-1;
    break;
    case 'R':
    {
     char x[10];
     sprintf(x,"%d",p->htmlframe[arachne.target].scroll.xsize);
     strcpy(out,x);
     out+=strlen(x)-1;
    }
    break;
    case 'T':
    if(!user_interface.cache2temp)
     goto a;
    {
     char temp[80]="\0";
     tempinit(temp);
     if(temp[0]=='\0')
      goto a;
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
    pom = config_get_str("JPEGargs", "-bmp ");
    goto cont;

//!!glennmcc: begin Apr 23, 2002
    case 'K':  //pass arguments to dgi from any arachne.cfg keyword
kcnt++;
in++;
if (kcnt==1) //process the first $K
{
strncpy(cfgkw, in, 1);
while (*in && *in!=' ')
{
in++;
strncat(cfgkw, in, 1);
}
    pom = config_get_str(strupr(cfgkw), NULL);
    if(!pom)
    {
    pom=" ";
    }
   else
   {
   strcat(pom," ");
   }
}
else

{
 pom=" "; //space goes into the command line instead of any additional $Ks
 while (*in && *in!=' ')
 {                      //move to next command line option
 in++;                 //past any more than 1 $K in the same DGI line
 }
}
    goto cont;
//!!glennmcc: end

//!!glannmcc: Dec 06, 2006 -- add $z to pass only the drive letter to dosshell.dgi
//and dosshell.bat so that FreeDos can be returned to the correct drive.
#ifndef LINUX
    case 'Z':
    fnsplit(syspath,drive,dir,file,ext);
    strcpy(out,drive);
    out+=strlen(drive)-1;
    break;
#endif
//!!glennmc: end

    case 'F':  //file browser WWWMANE.EXE mode
    pom = config_get_str("FILEargs", "-d");

    // note: this may be incorrect, but original code
    //       copied a random byte from a NULL pointer...
    // need to verify against wwwman.exe sources
    strcat(pom, config_get_bool("CacheDirList", 1) ? "Y" : "N");

    goto cont;
    case 'M':
    pom = mailpath;
    goto cont;
    case 'W':
    pom = downloadpath;
    goto cont;
    case 'N':
    pom = config_get_str("NameServer", NULL);
    goto cont;
    case 'G':
    pom = config_get_str("Gateway", NULL);
    goto cont;
    case '!':
    pom = config_get_str("Editor", NULL);
    goto cont;
    case 'X':
    pom = config_get_str("Netmask", NULL);
    goto cont;
//!!glennmcc & Ray: Sep 05 & 06, 2008 -- $Y == everything after the 1st space
//in any DGI ... ex: file:ping.dgi www.glennmcc.org 5 <-- ping the domain 5 times
// in mime.cfg ---> file/ping.dgi >txt|ping.exe $Y>$2
//many thanks to Ray Andrews for showing me how to do this correctly. :)
    case 'Y':
    {
//     char *ptr;
//     ptr=strchr(GLOBAL.location,' ');
//     pom=&GLOBAL.location[strchr(GLOBAL.location,' ')-GLOBAL.location];
     pom=strchr(GLOBAL.location,' ')+1;
    }
    goto cont;
//!!glennmcc & Ray: end
    case 'A':
    a:
    pom = cachepath;
    goto cont;
    case 'B':
    pom = config_get_str("Hotlist", hotlist);
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

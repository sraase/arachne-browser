
#include "arachne.h"
#include "internet.h"
#include "alttab.h"

#ifdef CALDERA
#include "link.h"
#include "keyboard.h"
#endif // CALDERA

#ifdef POSIX
char ARACHNEPICK[80];
#else
char *ARACHNEPICK="arachne.pck";
#endif // CALDERA

#ifndef MINITERM


#ifndef POSIX
int askgraphics(void)
{
int i;
  puts(MSG_VGASEL);
  puts(MSG_VGAVGA);
  puts(MSG_VGAEGA);
  puts(MSG_VGACGA);
  printf(MSG_VIDEO);

  vga:
  i=getch();
  if (i==27)
   return -1;

  if (i=='1')
  {
   strcpy(arachne.graphics,"EGA");
   return 2;
  }
  else if (i=='2')
  {
   strcpy(arachne.graphics,"BCGA");
   arachne.xSwap=2;
   return 3;
  }
  else if (i!='0' && i!=13)
   goto vga;
  else
  {
   strcpy(arachne.graphics,"VGA");
   return 1;
  }

}
#endif

int loadpick( char *exename) //nahrat konfiguraci
{
 int f,i,rv=0;
 char *str1;

 strncpy( exepath, exename, 63);   // urceni exepath !!!
#ifdef POSIX
 str1= strrchr( exepath, '/');
#else
 str1= strrchr( exepath, '\\');
#endif
 if(str1 == NULL) exepath[0]='\0';
 else                str1[1]='\0';
 strlwr(exepath);

#ifdef POSIX
 if(!strcmp(exepath,"./"))
  helppath[0]='\0';
 else
  strcpy(helppath,exepath);

 str1= strrchr( helppath, '/');
 if(str1)
 {
  *str1='\0';
  str1= strrchr( helppath, '/');
  if(str1)
   str1[1]='\0';
  else
   helppath[0]='\0';
 }
 else
  strcat(helppath,"../");

 strcpy(sharepath,helppath);
 strcat(helppath,"doc/arachne/html/");
 strcat(sharepath,"share/arachne/");

 str1=getenv("HOME");
 if(str1)
  sprintf(dotarachne,"%.60s/.arachne/",str1);

 sprintf(ARACHNEPICK,"%sruntime-data.bin",dotarachne);
 sprintf(CLIPBOARDNAME,"%sclipboard",dotarachne);
#endif

#ifdef POSIX
 f=a_open(ARACHNEPICK,O_RDONLY,0);
#else
 f=a_open(ARACHNEPICK,O_BINARY|O_RDONLY,0);
#endif

#ifdef POSIX
 if(f<0) //globalni pick ? First start ?
 {
  char cmd[256];
  sprintf(cmd,"%sarachne-install %s",sharepath,sharepath);
  system(cmd);
 }
#else
 if(f<0) //globalni pick ? First start ?
 {
  char str[80];
  sprintf(str,"%s%s",exepath,ARACHNEPICK);
  f=a_open(str,O_BINARY|O_RDONLY,0);
 }
#endif

 if(f>=0)
  i=a_read(f,&arachne,sizeof(struct ArachnePick));

 if(f<0 || i!=sizeof(struct ArachnePick))
 {
  memset(&arachne,0,sizeof(struct ArachnePick));

#ifndef POSIX

  puts(MSG_MEMSEL);
  puts(MSG_MEMXMS);
  puts(MSG_MEMEMS);
  puts(MSG_MEMDSK);

  printf(MSG_MEMORY);
  mem:
  i=getch();

  if(i==27)
  {
   printf("\n\n");
   exit(EXIT_ABORT_SETUP);
  }

  if (i=='1')
   arachne.xSwap=1;
  else if (i=='2')
   arachne.xSwap=2;
  else if (i!='0' && i!=13)
   goto mem;

  printf("\n");
  rv=askgraphics();

  if(rv==-1)
  {
   printf("\n\n");
   exit(EXIT_ABORT_SETUP);
  }
#endif
 }//endif first start

 p->htmlframe=(struct HTMLframe*)farmalloc(MAXFRAMES*(2+sizeof(struct HTMLframe)));
 AUTHENTICATION=farmalloc(sizeof(struct AUTH_STRUCT)+2);
 if(p->htmlframe && AUTHENTICATION)
 {
  if(f>=0)
   i=a_read(f,p->htmlframe,MAXFRAMES*(1+sizeof(struct HTMLframe)));
  else
   i=0;
  if(i<MAXFRAMES*(1+sizeof(struct HTMLframe)))
   memset(p->htmlframe,0,MAXFRAMES*(1+sizeof(struct HTMLframe)));

  if(f>=0)
   i=a_read(f,AUTHENTICATION,sizeof(struct AUTH_STRUCT));
  else
   i=0;
  if(i<sizeof(struct AUTH_STRUCT))
   memset(AUTHENTICATION,0,sizeof(struct AUTH_STRUCT));
 }
 else
  memerr0();

 a_close(f);

 return rv;

}

/*

void defaultGUIstyle(void)
{
#ifdef CALDERA
  arachne.GUIstyle = STYLE_MOZILLA;
#endif

#ifdef TELEMED
  arachne.GUIstyle = STYLE_MOZILLA;
#endif

#ifdef AGB
  arachne.GUIstyle = STYLE_MOZILLA;
#endif

  if(x_maxx()<640)
   arachne.GUIstyle = STYLE_SMALL2;
  else
  if(x_maxx()<800)
   arachne.GUIstyle = STYLE_SMALL1;

}

*/
#ifndef POSIX
void meminit(char arg)
{
 switch(arg)
 {
  case 1: //EMS
  SetXmsEms(2);
  break;
  case 2: //disk
  DisableXMS=1;
  break;
  default://like previous versions
  SetXmsEms(1);
 }
}
#endif

void savepick() //ulozit konfiguraci
{
 int f;

#ifdef POSIX
 f=a_open(ARACHNEPICK,O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
#else
 f=a_open(ARACHNEPICK,O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
#endif
 if(f>=0)
 {
  write(f,&arachne,sizeof(struct ArachnePick));
  write(f,p->htmlframe,MAXFRAMES*(1+sizeof(struct HTMLframe)));
  write(f,AUTHENTICATION,sizeof(struct AUTH_STRUCT));
  a_close(f);
 }
}

//pokud newvalue!=NULL
//keystring musi byt alkovan na delku keystring+newvalue dohromady!

#endif //MINITERM

char *configvariable(struct ib_editor *fajl,char *keystring,char *newvalue)
{
 int l;
 char *line,*ptr;

 l=strlen(keystring);
 fajl->y=0;
 while(fajl->y<fajl->lines)
 {
  line=ie_getline(fajl,fajl->y);
  if(line && strlen(line)>l && !strncmpi(line,keystring,l))
  {
   if(newvalue)
   {
    newval:
    line=farmalloc(IE_MAXLEN);
    if(line)
    {
     strcpy(line,keystring);
     strcat(line," ");
     strcat(line,newvalue);
     ie_putline(fajl,fajl->y,line);
     farfree(line);
    }
    return NULL;
   }
   else
   {
    line+=l;                 //ted ukazuju na prvni mezeru
    while(*line==' ')line++; //uriznu mezeru
    ptr=strchr(line,';');
    if(ptr)                  //je za hodnotou komentar ?
    {
     while(*(ptr-1)==' ')ptr--;//uriznu mezeru pred komentarem
     *ptr='\0';
    }
    return line;             //vracim pointer na hodnotu promenne.
   }
  }
  fajl->y++;
 }
 if(newvalue && *newvalue && fajl->y<fajl->maxlines)
 {
  ie_insline(fajl,fajl->y,"");
  goto newval;
 }

 return NULL;
}


#ifndef WWWMAN
#ifndef MINITERM
#ifndef POSIX

//. Roura : name of a batch file.  Allow us to use external executable.  Like Pipe in UNIX
void rouraname(char *fname)
{
 tempinit(fname);
 strcat(fname,"$roura$.bat");
}

// create a batch file executing cmd , uses name created by rouraname( ..)
int willexecute(char *cmd) //vykonat nejakou hloupost:
{
 int f;
 char bat[80];

 rouraname(bat);
 f=a_fast_open(bat,O_TEXT|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
 if(f>=0)
 {
  write(f,cmd,strlen(cmd));
  a_close(f);
  if(tempdir[0])
   return EXIT_FAST_EXECUTE; // %ARACHNETEMP%\$roura$.bat
  else
   return EXIT_EXECUTE;      // .\$roura$.bat
 }
 else
 {
  return 0;
 }
}
#endif

#ifdef MINITERM
struct uiface user_interface;
#endif

//extern char reg;
extern struct ib_editor ARACHNEcfg;//hlavni konfigurace

void configure_user_interface(void)
{
 char *value;

 value=configvariable(&ARACHNEcfg,"SmallIcons",NULL);
/*
 if(value && toupper(*value)=='N' && reg)
  user_interface.iconsoff=1;
 else
*/
  user_interface.iconsoff=0;

 value=configvariable(&ARACHNEcfg,"Hotkeys",NULL);
/*
 if(value && toupper(*value)=='N' && reg)
  user_interface.hotkeys=0;
 else
*/
  user_interface.hotkeys=1;

 user_interface.bigfont=0;

 value=configvariable(&ARACHNEcfg,"AltSysFont",NULL);
 if(value && *value!='0')
 {
  char x = atoi(value);
  if(x < 0)
   x = 0;
  else if(x > 2)/*Will not lock up in 640*480*/
   x = 2;
  user_interface.bigfnum = (unsigned char) x;
  user_interface.bigfont=1;
  user_interface.bigstyle=FIXED;
 }

 value=configvariable(&ARACHNEcfg,"BigFont",NULL);
 if(value && toupper(*value)=='Y')
 {
  user_interface.bigfont=1;
  user_interface.bigfnum=5;
  user_interface.bigstyle=BOLD|FIXED;
 }

 value=configvariable(&ARACHNEcfg,"ScrollBarSize",NULL);
 if(value)
  user_interface.scrollbarsize=atoi(value);
 else
  user_interface.scrollbarsize=10;
 if(user_interface.scrollbarsize>80) user_interface.scrollbarsize=80;

 /*
 value=configvariable(&ARACHNEcfg,"HTTPretry",NULL);
 if(value)
  user_interface.httpretry=atoi(value);
 else
  user_interface.httpretry=10;
 */

 value=configvariable(&ARACHNEcfg,"Colors",NULL);
 if(value)
 { 
  char *newvalue;

  user_interface.ink = (int)strtol (value, &newvalue, 10);
  if (newvalue != value) {
    value = newvalue;
    user_interface.paper = (int)strtol (value, &newvalue, 10);
    if (newvalue == value)
       user_interface.paper=0; //black
  }
  else user_interface.ink=11; //green  
 }
 else
 {
  user_interface.ink=11; //green
  user_interface.paper=0; //black
 }

 value=configvariable(&ARACHNEcfg,"MouseColors",NULL);
 if(value)
 {
  char *newvalue;

  user_interface.darkmouse = (int) strtol (value, &newvalue, 10);
  if (newvalue != value) {
    value = newvalue;
    user_interface.brightmouse = (int)strtol (value, &newvalue, 10);
    if (newvalue == value)
       user_interface.brightmouse=15; //black
  }
  else user_interface.darkmouse=7; //white
 }
 else
 {
  user_interface.darkmouse=7; //white
  user_interface.brightmouse=15; //black
 }

 value=configvariable(&ARACHNEcfg,"Frames",NULL);
 if(value && toupper(*value)=='N')
  user_interface.frames=0;
 else
  user_interface.frames=1;

 value=configvariable(&ARACHNEcfg,"QADT",NULL);
 if(value && toupper(*value)=='Y')
  user_interface.quickanddirty=1;
 else
  user_interface.quickanddirty=0;

 value=configvariable(&ARACHNEcfg,"CacheFonts",NULL);
 if(value && toupper(*value)=='N')
  user_interface.cachefonts=0;
 else
  user_interface.cachefonts=1;

 value=configvariable(&ARACHNEcfg,"Cache2TEMP",NULL);
 if(value && toupper(*value)=='Y')
  user_interface.cache2temp=1;
 else
  user_interface.cache2temp=0;

 value=configvariable(&ARACHNEcfg,"Kill468x60",NULL);
 if(value && toupper(*value)=='Y')
  user_interface.killadds=1;
 else
  user_interface.killadds=0;

#ifndef POSIX
 value=configvariable(&ARACHNEcfg,"AltTab",NULL);
 if(value && toupper(*value)=='Y')
 {
  InstalAltTab();
  atexit(ReleaseAltTab);
 }

 InstalPrtScr();
 atexit(ReleasePrtScr);
#endif

 value=configvariable(&ARACHNEcfg,"MailIsDesktop",NULL);
 if(value && toupper(*value)=='N')
  user_interface.mailisdesktop=0;
 else
  user_interface.mailisdesktop=1;

 value=configvariable(&ARACHNEcfg,"EditHotlistEntry",NULL);
 if(value && toupper(*value)=='N')
  user_interface.edithotlistentry=0;
 else
  user_interface.edithotlistentry=1;

 value=configvariable(&ARACHNEcfg,"Autodial",NULL);
 if(value && toupper(*value)=='Y')
  user_interface.autodial=1;
 else
  user_interface.autodial=0;

#if defined(MSDOS) && !defined(XTVERSION)
 value=configvariable(&ARACHNEcfg,"VFAT",NULL);
 if(value && toupper(*value)=='Y')
  user_interface.vfat=1;
 else
  user_interface.vfat=0;
#endif

 value=configvariable(&ARACHNEcfg,"KeepHTT",NULL);
 if(value && toupper(*value)=='N')
  user_interface.nohtt=1;
 else
  user_interface.nohtt=0;

 user_interface.virtualysize=0;
 user_interface.smooth=0;

#ifdef GGI
 value=configvariable(&ARACHNEcfg,"GGI_FastScroll",NULL);
 if(value && toupper(*value)=='N')
  user_interface.ggifastscroll=0;
 else
  user_interface.ggifastscroll=1;
#endif

#ifdef VIRT_SCR
 value=configvariable(&ARACHNEcfg,"VirtualScreen",NULL);
 if(value)
  user_interface.virtualysize=atoi(value);

 value=configvariable(&ARACHNEcfg,"SmoothScroll",NULL);
 if(value && toupper(*value)=='Y')
 {
  user_interface.smooth=1;
  value=configvariable(&ARACHNEcfg,"ScrollStep",NULL);
  if(value)
   user_interface.step=atoi(value);
  else
   user_interface.step=1000;

 value=configvariable(&ARACHNEcfg,"ScreenMode",NULL);
 if(value && *value!='A')
  user_interface.screenmode=*value; //A[uto],S[mart],N[ice]
 else
  user_interface.screenmode=0; //default = A...Auto
 }

#endif


 value=configvariable(&ARACHNEcfg,"ScrollBarStyle",NULL);
 if(value && *value!='A')
 {
  user_interface.scrollbarstyle=*value; //W[indoze],N[extStep],X[experimental]
  if(*value=='C')
   user_interface.scrollbarsize=0; //no scrollbars
 }
 else
  user_interface.scrollbarstyle=0; //default = A...Arachne

 value=configvariable(&ARACHNEcfg,"ESC",NULL);
 if(value && toupper(*value=='I'))
  user_interface.esc=ESC_IGNORE;
 else
 if(value && toupper(*value=='B'))
  user_interface.esc=ESC_BACK;
 else
  user_interface.esc=ESC_EXIT;

 value=configvariable(&ARACHNEcfg,"Multitasking",NULL);
 if(value && toupper(*value=='N'))
  user_interface.multitasking=MULTI_NO;
 else
 if(value && toupper(*value=='S'))
  user_interface.multitasking=MULTI_SAFE;
 else
  user_interface.multitasking=MULTI_YES;

 value=configvariable(&ARACHNEcfg,"Logo",NULL);
 if(value)
  user_interface.logoiddle=atoi(value);
 else
  user_interface.logoiddle=2000;

#ifdef GGI
 user_interface.logoiddle*=3; //to avoid unecessary GGI framebuffer flushing...
#endif

 value=configvariable(&ARACHNEcfg,"MinDiskSpace",NULL);
 if(value)
  user_interface.mindiskspace=atol(value)*1024;
 else
  user_interface.mindiskspace=128000l;

 value=configvariable(&ARACHNEcfg,"XMS4allgifs",NULL);
 if(value)
  user_interface.xms4allgifs=atoi(value);
 else
  user_interface.xms4allgifs=256;

 value=configvariable(&ARACHNEcfg,"XMS4onegif",NULL);
 if(value)
  user_interface.xms4onegif=atol(value)*1024;
 else
  user_interface.xms4onegif=32000l;

 value=configvariable(&ARACHNEcfg,"ExpireDynamic",NULL);
 if(value)
  user_interface.expire_dynamic=atol(value);
 else
  user_interface.expire_dynamic=3600l;

 value=configvariable(&ARACHNEcfg,"ExpireStatic",NULL);
 if(value)
  user_interface.expire_static=atol(value);
 else
  user_interface.expire_static=86400l;

 if(cgamode)
  user_interface.fontshift=-2;
 else
 {
  value=configvariable(&ARACHNEcfg,"FontShift",NULL);
  if(value)
   user_interface.fontshift=atoi(value);
  else
   user_interface.fontshift=0;
 }

 {
  int w=78;
  value=configvariable(&ARACHNEcfg,"ASCIIwidth",NULL);
  if(value)
  {
   w=atoi(value);
   if(w<40)w=40;
   if(w>255)w=255;
  }
  user_interface.printerwidth=w;

#ifndef NOPS
  value=configvariable(&ARACHNEcfg,"PostScriptWidth",NULL);
  if(value)
  {
   w=atoi(value);
   if(w<100)w=100;
   if(w>1000)w=1000;
  }
  else
   w=210; //A4

  user_interface.postscript_x=w;

  value=configvariable(&ARACHNEcfg,"PostScriptHeight",NULL);
  if(value)
  {
   w=atoi(value);
   if(w<100)w=100;
   if(w>1000)w=1000;
  }
  else w=297; //A4

  user_interface.postscript_y=w;
#endif
 }

 user_interface.keymap=NULL;
 value=configvariable(&ARACHNEcfg,"UseKeyMap",NULL);
 if(value && toupper(*value)=='Y')
 {
  value=configvariable(&ARACHNEcfg,"Keymap1",NULL);
  if(value && *value)
  {
   user_interface.keymap=farmalloc(96);
   if(user_interface.keymap)
   {
    strncpy((char *)user_interface.keymap,value,48);
    value=configvariable(&ARACHNEcfg,"Keymap2",NULL);
    if(value && *value)
     strncpy((char *)&user_interface.keymap[48],value,48);
   }
  }
 }

 value=configvariable(&ARACHNEcfg,"ScreenSaver",NULL);
 if(value)
  ScreenSaver=atol(value);

 value=configvariable(&ARACHNEcfg,"RefreshDelay",NULL);
 if(value)
  user_interface.refresh=atoi(value);
 else
  user_interface.refresh=20;

 value=configvariable(&ARACHNEcfg,"CSS",NULL);
 if(value && toupper(*value)=='N')
  user_interface.css=0;
 else
  user_interface.css=1;

 // HTTP parameters --------------------------------------------------

 value=configvariable(&ARACHNEcfg,"HTTPreferer",NULL);
 if(value && toupper(*value=='N'))
  http_parameters.referer=0;
 else
  http_parameters.referer=1;

 value=configvariable(&ARACHNEcfg,"HTTPKeepAlive",NULL);
 if(value && toupper(*value=='N'))
  http_parameters.keepalive=0;
 else
  http_parameters.keepalive=1;

 value=configvariable(&ARACHNEcfg,"UseProxy",NULL);
 if(value && toupper(*value)!='N')
  http_parameters.useproxy=1;
 else
  http_parameters.useproxy=0;

 value=configvariable(&ARACHNEcfg,"Cookies",NULL);
 if(value && toupper(*value)=='N')
  http_parameters.acceptcookies=0;
 else
  http_parameters.acceptcookies=1;

//!!glennmcc: begin Dec 11, 2001
// added to fix "HTTPS verifying images" loop by trying HTTP instead
// (defaults to No if "HTTPS2HTTP Yes" line is not in Arachne.cfg)
 value=configvariable(&ARACHNEcfg,"HTTPS2HTTP",NULL);
 if(value && toupper(*value=='Y'))
  http_parameters.https2http=1;
 else
  http_parameters.https2http=0;
//!!glennmcc: end

//!!glennmcc: begin May 03, 2002
// added to optionally "ignore" <script> tag
// (defaults to No if "IgnoreJS Yes" line is not in Arachne.cfg)
 value=configvariable(&ARACHNEcfg,"IGNOREJS",NULL);
 if(value && toupper(*value=='Y'))
  http_parameters.ignorejs=1;
 else
  http_parameters.ignorejs=0;
//!!glennmcc: end

}
#endif
#endif

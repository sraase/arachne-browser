// ========================================================================
// Initialization and deinitialization of Arachne WWW browser
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "main.h"
#include "html.h"

#include "xanimgif.h"

#ifndef NOTCPIP
#include "internet.h"
#endif

int mem_xmem(unsigned *total, unsigned *free);

void Initialize_Arachne(int argc,char **argv,struct Url *url)
{
 int grsetup=0;

#ifdef ENABLE_A_IO
 if(!a_alloccache())
  memerr0();
#endif

#ifndef POSIX
 if(argc>1 && argv[1][0]=='-')
 {
  if(argv[1][1]=='s')
  {
   detectgraphics();
   unlink("arachne.pck");
   tcpip=-1; //setup mode
   arachne.GUIstyle|=4;
  }
  else
  if(argv[1][1]=='o' || argv[1][1]=='r')
  {
   tcpip=1;
  }

#ifdef OVRL
#ifndef XTVERSION
  if(argv[1][1]=='f')
  {
   puts(MSG_FONT);
   finfo();
  }
#endif
#endif

  else
  if(argv[1][2]=='g')
  {
   x_setnomode(1);
   noGUIredraw=1;
  }
 }
#endif

 grsetup=loadpick(argv[0]);     //try to load ARACHE.PCK (defines exepath too)

if(argc>1)
{
 if(argv[1][0]=='-')
 {
#ifndef POSIX
  if(argv[1][1]=='d')
  {
   detectgraphics();
   grsetup=askgraphics();
  }
  else
#endif
  if(argv[1][1]=='x')
  {
   tcpip=-1; //setup mode
   arachne.GUIstyle|=4;
  }
  else
  if(argv[1][1]=='i')
  {
   tcpip=1;
   arachne.GUIstyle|=4;
  }
 }
}

//256 color palette - we will allocate it rather here...
Iipal=farmalloc(768); //3*256 = 256 RGB values
if(!Iipal)
 memerr0();

#ifndef POSIX  //MS-DOS style startyp, graphics in arachne.pck ...
meminit(arachne.xSwap);         //0 - try to use XMS

graphicsinit(arachne.graphics); //XLOPIF SVGA GRAPHICS

//defaultGUIstyle();
if(!noGUIredraw && !strcmpi(arachne.graphics,"VGA"))
 x_cleardev();

finfoload();                    //load font information
if(ie_initswap()!=1)            //initialization of swapping system ie_swap
 memerr0();
init_bin();                     //initialization of memory, conf. files, etc.
configure_user_interface();     //icons, hotkeys, scrollbuttons, font...
init_xms();                     //font caching+animated GIFs

#else                           //LINUX, etc. - graphics mode information in arachne.conf

if(ie_initswap()!=1)            //initialization of swapping system ie_swap
 memerr0();
init_bin();                     //initialization of memory, conf. files, etc.

// temporary, force 800x600 HiColor by default
strcpy(arachne.graphics, config_get_str("GraphicsMode", "Hi16.J"));

graphicsinit(arachne.graphics); //XLOPIF SVGA GRAPHICS

//defaultGUIstyle();
finfoload();                    //load font information
configure_user_interface();     //icons, hotkeys, scrollbuttons, font...

x_fnt_initxms(50);              //initialize font table...
#ifdef LINUX
bioskey_init();//switch terminal to raw mode
#endif // LINUX
#endif // not POSIX


if(arachne.GUIstyle==8) //first start ?
{
 if(x_maxx()<640)
  arachne.GUIstyle = STYLE_SMALL2;
 else if(x_maxx()<800)
  arachne.GUIstyle = STYLE_SMALL1;
 else
  arachne.GUIstyle = STYLE_ARACHNE;
}


 InitInput(&tmpeditor,"","",1,CONTEXT_SYSTEM);//URL input prompt
if(fonty(SYSFONT,0)==14)
 MakeInputAtom(&URLprompt,&tmpeditor,50,-21,p->htscrn_xsize,-3);
else if(fonty(SYSFONT,0)<=16)
 MakeInputAtom(&URLprompt,&tmpeditor,50,-22,p->htscrn_xsize,-2);
else
 MakeInputAtom(&URLprompt,&tmpeditor,50,-25,x_maxx()-152,-2);

InitInput(&tmpeditor,"","",1,CONTEXT_SYSTEM);//text input prompt
MakeInputAtom(&TXTprompt,&tmpeditor,
              64,p->htscrn_ysize/2,
              p->htscrn_xsize-128,p->htscrn_ysize/2+fonty(SYSFONT,0)+4);


//initialization of certain global variables:

GLOBAL.needrender=1;        //na zacatku potrebuju prekreslit
                  // tr.: in the beginning I need to redraw
GLOBAL.isimage=0;           //to co nactu po startu, to nebude inline...
                  // tr.: what I load/read after start, will not be inline
GLOBAL.nothot=0;
GLOBAL.reload=NO_RELOAD;
GLOBAL.postdata=0;
GLOBAL.postdataptr=IE_NULL;
GLOBAL.redirection=0;
GLOBAL.backgr=0;
GLOBAL.willexecute=0;
GLOBAL.location[0]='\0';
GLOBAL.currentcharset[0]='\0';
p->restorehoveradr=IE_NULL;
reset_tmpframedata();
reset_frameset();
AUTHENTICATION->proxy=0;

//initialization BASE URL of default frame:
ResetURL(&baseURL);
ResetURL(url);


#ifdef VIRT_SCR
memset(allocatedvirtual,0,MAXVIRTUAL);
#endif
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 //arguments ?
 if(argc>1 && argv[1][0]=='-' && (argv[1][1]=='c' || argv[1][1]=='r'))
 {
  //target is prepared in arachne.target!

  //return to frameset ?!
  if(arachne.framescount)
  {
   SetInputAtom(&URLprompt,p->htmlframe[0].cacheitem.URL);
   strcpy(GLOBAL.location,p->htmlframe[0].cacheitem.URL);
  }

  if(!arachne.framescount || arachne.target)
  {
   strcpy(GLOBAL.location,ie_getline(&history,arachne.history));
  }

  GLOBAL.nowimages=IMAGES_SEEKCACHE;
  if(argv[1][1]=='c') //-c, -cg means "always ignore TCP/IP"
   goto SkipTCPIP;
 }
 else
 {
  GLOBAL.nowimages=IMAGES_NOTNOW;
  arachne.title[0]='\0';
  arachne.framescount=0;
  arachne.newframe=0;
  arachne.target=0;
 }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//setup ?
if(tcpip==-1 || grsetup) //setup only
{
 if(argc>2)   //second parameter = filename
 {
  //base url for command line argument is file:
  strcpy(baseURL.protocol,"file");
  baseURL.file[0]='\0';

  AnalyseURL(argv[2],url,0);
  url2str(url,GLOBAL.location);
 }
 else
 {
  if(grsetup==1 || !strcmpi(arachne.graphics,"VGA"))
   sprintf(GLOBAL.location,"file:%svga.htm",exepath);
  else
   sprintf(GLOBAL.location,"file:%sega_cga.htm",exepath);
  arachne.GUIstyle|=4;
 }
 tcpip=0;
 goto FirstDraw;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef NOTCPIP
#ifdef POSIX
tcpip=1;
#else
ArachneTCPIP();     /* initialize TCP/IP */
#endif
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SkipTCPIP:
//process arguments again...
if(argc>1)
{
 if(argv[1][0]!='-')
 {
  strcpy(baseURL.protocol,"file"); //default is file://
  baseURL.file[0]='\0';            //default no doc
  AnalyseURL(argv[1],url,0);
  url2str(url,GLOBAL.location);
 }
 else if(argv[1][2]=='e')
 {
  sprintf(GLOBAL.location,"file:%s%snonfatal.ah",sharepath,GUIPATH);
 }

//!!glennmcc: begin Jan 09, 2005
// on email attachment errors, goto err_mail.ah instead of nonfatal.ah
 else if(argv[1][2]=='m')
 {
  sprintf(GLOBAL.location,"file:%s%serr_mail.ah",sharepath,GUIPATH);
 }
//!!glennmcc: end

 else if(argv[1][1]=='?')
 {
#ifdef POSIX
  sprintf(GLOBAL.location,"file:%sindex.html",helppath);
#else
  sprintf(GLOBAL.location,"file:%shelp.htm",exepath);
#endif
 }
}
else //no arguments !
{
 toolbarpage=0;
#ifndef POSIX
 if(cacheisfull())
  sprintf(GLOBAL.location,"file:%s%smaintain.ah",sharepath,GUIPATH);
#endif
}

if(!GLOBAL.location[0])
 gohome();

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FirstDraw:                  //Draw NOW!

#ifndef POSIX
x_setnomode(0);             //let us redraw
#endif

zoom();                     //calculate screen size
RedrawALL();                //redraw screen
if(tcpip || arachne.target!=0)
 DrawTitle(1);
GUIInit();                  //initialization of graphical user interface
}

// *************************************************************************

int Terminate_Arachne(int returnvalue)
{
#ifdef VIRT_SCR
 deallocvirtual();
#endif
#ifdef XANIMGIF
 XCloseAnimGIF();  //HGIF 1x at the end
#endif
#ifndef POSIX
 x_fnt_cls();
#endif
 mouseoff();
#ifdef ENABLE_A_IO
 checkDirectory("", 1)
#endif

 savepick();
 if(tcpip)
  ie_savef(&cookies);
 ie_savebin(&HTTPcache);
 ie_savef(&history);
 ie_destroy();

#ifdef LINUX
//switch terminal back to original mode
bioskey_close();
#endif

 if(returnvalue==EXIT_TO_DOS)
 {
  exitmsg();
  if(tcpip)
   puts(MSG_RETURN);
 }


// printf("errorlevel=%d",returnvalue);
 return(returnvalue);
}

//erroneous configuration - moved to errors.c...
void cfgerr (struct ib_editor *f);

//maximum number of lines in CFG files :
#define LINES 1024

//maximum number of lines in cookies file :
#define MAX_HTTP_COOKIES 64*CookieCrumbs
//!!JdS 2004/3/6: Was: #define MAX_HTTP_COOKIES 64

void init_bin(void)
{
 int rc;
 char *ptr;
#ifdef POSIX
 char acfg[80],mcfg[80];

 sprintf(acfg,"%sarachne.conf",dotarachne);
 sprintf(mcfg,"%smime.conf",sharepath);

 BUF=16535;
#else
 char *acfg="arachne.cfg",*mcfg="mime.cfg";

 if(farcoreleft()<MIN_MEMORY)
  memerr0();
 else
 {
  //2*2000:
  memory_model=(char)((farcoreleft()-MIN_MEMORY)/IE_MAXSWAPLEN);
  if(memory_model>2)
   memory_model=2;
 }

 if(memory_model)
 {
  BUF=4096; //!!JdS 2004/2/8: Increased from 4000 to 4096
  //BUF=4000;
  x_fnt_alloc(SMALL_FONT_BUFFER);
  //we will temporarily need some more 21000 kB...
 }
 else
 {
  BUF=2048; //!!JdS 2004/2/8: Increased from 2000 to 2048
  //BUF=2000;
  x_fnt_alloc(BIG_FONT_BUFFER);
 }

 if(memory_model==2)
 {
  if(ie_optimize()!=1)
   memory_model=1;
 }
#endif


 //---ARACHNE.CFG
 strcpy(ARACHNEcfg.filename,acfg);
 ARACHNEcfg.killcomment=0;
//!!glennmcc: Dec 03, 2005 -- increased to 388 via LINES define above (experimental compile only)
 rc=ie_openf_lim(&ARACHNEcfg,CONTEXT_SYSTEM,LINES); //main configuration
 if(!ARACHNEcfg.lines)
 {
  ie_clearf(&ARACHNEcfg,0);
#ifdef POSIX
  sprintf(ARACHNEcfg.filename,"arachne.cfg");
#else
  sprintf(ARACHNEcfg.filename,"%s%s",exepath,acfg);
#endif
//!!glennmcc: Dec 03, 2005 -- increased to 388 via LINES define above (experimental compile only)
  rc=ie_openf_lim(&ARACHNEcfg,CONTEXT_SYSTEM,LINES); //always 256 lines
 }
 if(rc==2)
  memerr0();
 else if(rc!=1)
  cfgerr(&ARACHNEcfg);

#ifdef POSIX
 //set font path first of all...
 sprintf(fntpath,"%s%s",sharepath,config_get_str("FontPathSuffix", "iso-8859-1/"));
 strcpy(fntinf,fntpath);
 strcat(fntinf,"fontinfo.bin");
#endif

  //---MIME.CFG
 strcpy(MIMEcfg.filename,mcfg);
 MIMEcfg.killcomment=1;
//!!glennmcc: Dec 03, 2005 -- increased to 388 via LINES define above (experimental compile only)
 rc=ie_openf_lim(&MIMEcfg,CONTEXT_SYSTEM,LINES); //MIME
 if(!MIMEcfg.lines)
 {
  ie_clearf(&MIMEcfg,0);
#ifdef POSIX
  sprintf(MIMEcfg.filename,"mime.cfg");
#else
  sprintf(MIMEcfg.filename,"%s%s",exepath,mcfg);
#endif
//!!glennmcc: Dec 03, 2005 -- increased to 388 via LINES define above (experimental compile only)
  rc=ie_openf_lim(&MIMEcfg,CONTEXT_SYSTEM,LINES); //MIME
 }
 if(rc==2)
  memerr0();
 else if(rc!=1)
  cfgerr(&MIMEcfg);

 //---toolbar.cfg loading
 ptr = config_get_str("Toolbar", NULL);
 if(!ptr)
#ifdef POSIX
  sprintf(TOOLBARcfg.filename,"%stemplates/toolbar.cfg",sharepath);
#else
  sprintf(TOOLBARcfg.filename,"%stoolbar.cfg",exepath);
#endif
  else
  strcpy(TOOLBARcfg.filename,ptr);
 TOOLBARcfg.killcomment=1;
//!!glennmcc: Dec 03, 2005 -- increased to 388 via LINES define above (experimental compile only)
 rc=ie_openf_lim(&TOOLBARcfg,CONTEXT_SYSTEM,LINES); //Toolbar
 if(rc==2)
  memerr0();
 else if(rc!=1 || TOOLBARcfg.lines==0)
  cfgerr(&TOOLBARcfg);

//!!glennmcc: May 27, 2007 -- read entity conversions from entity.cfg
 //---entity.cfg
#ifdef POSIX
  sprintf(ENTITYcfg.filename,"%stemplates/entity.cfg",sharepath);
#else
  sprintf(ENTITYcfg.filename,"%sentity.cfg",exepath);
#endif
 ENTITYcfg.killcomment=1;
#ifdef MINIMAL
 rc=ie_openf_lim(&ENTITYcfg,CONTEXT_SYSTEM,256);
#else
 rc=ie_openf_lim(&ENTITYcfg,CONTEXT_SYSTEM,512);
#endif
 if(rc==2)
  memerr0();
 else if(rc!=1 || ENTITYcfg.lines==0)
  cfgerr(&ENTITYcfg);
//!!glennmcc: end

 //---History of visited URLs
 strcpy(history.filename,config_get_str("History", "history.lst"));
 history.killcomment=0;
//!!glennmcc: Dec 03, 2005 -- increased to 388 via LINES define above (experimental compile only)
 rc=ie_openf_lim(&history,CONTEXT_SYSTEM,LINES); //history - max. 256 lines
 if(rc==2)
  memerr0();
 if(history.lines==0)
  ie_insline(&history,0,"");
 if(arachne.history>=history.lines)
  arachne.history=history.lines-1;

 //---Index of cache
 strcpy(HTTPcache.filename,config_get_str("CacheIndex", "cache.idx"));
//!!glennmcc: Dec 03, 2005 -- increased to 388 via LINES define above (experimental compile only)
 HTTPcache.maxlines=LINES;
// HTTPcache.maxlines=256; //max 256 files in cache
 rc=ie_openbin(&HTTPcache);
 if(rc==2)
  memerr0();

/*
 this won't be performed automaticaly any more...
 if(rc!=1 || HTTPcache.len==0)
  gumujcache(0);
*/

 //---COOKIES.LST
 strcpy(cookies.filename,config_get_str("CookieFile", "cookies.lst"));
 rc=ie_openf_lim(&cookies,CONTEXT_SYSTEM,MAX_HTTP_COOKIES);
//!!JdS 2004/3/6 {
 if ((cookies.lines/CookieCrumbs)*CookieCrumbs != cookies.lines)
  badcookiesfile();
//!!JdS 2004/3/6 }
 if(rc==2)
  memerr0();
 //---

#ifndef NOTCPIP
#ifdef WATTCP
 sock[0]=farmalloc(sizeof(tcp_Socket)+1);
 sock[1]=farmalloc(sizeof(tcp_Socket)+1);
 socket=sock[0];
 socknum=0;
 if(!sock[0] || !sock[1]) memerr0();
#endif
#endif

 //printf("allocating...\n");

 argnamestr=farmalloc(MAXARGNAMES);
 argvaluestr=farmalloc(BUF/4);
 p->buf=farmalloc(BUF+8);
 GLOBAL.location=farmalloc(URLSIZE);
 Referer=farmalloc(URLSIZE);
 //allocated in loadpick() !
 //p->htmlframe=(struct HTMLframe*)farmalloc(MAXFRAMES*(1+sizeof(struct HTMLframe)));
 p->tmpframedata=(struct TMPframedata*)farmalloc(MAXFRAMES*(2+sizeof(struct TMPframedata)));

 if(!p->buf || !GLOBAL.location || !p->htmlframe ||
    !argnamestr || !argvaluestr || !Referer || !p->tmpframedata)
  memerr0();
}

#ifndef POSIX
void init_xms(void)
{

//puts("Init XMS");

if(user_interface.cachefonts)
{
 if(!arachne.xSwap)                 //try to flush swap to XMS
  ie_swap(-1);                      //if failed, DisableXMS will be set.

 if(!arachne.xSwap && !DisableXMS && user_interface.cachefonts)  //xSwap=0...XMS
  x_fnt_initxms(50);                //48 HTML fonts, 1 system font
 else
  x_fnt_initxms(-1);                //no EMS/XMS ? do not try to store fonts!
}
#endif

#ifdef XANIMGIF
#ifndef POSIX

if(!DisableXMS && user_interface.xms4allgifs)
{
 unsigned int dummy=0u,free=0u;
 int ist;
  ist = mem_xmem( &dummy, &free);
  if(ist!=1 || free>=2*user_interface.xms4allgifs)
   XInitAnimGIF( user_interface.xms4allgifs );  //HGIF 1x at the beginning
}

}
#endif
#endif

void reset_tmpframedata(void)
{
 int i=0;

 while(i<MAXFRAMES)
 {
  p->tmpframedata[i].usevirtualscreen=0;
  p->tmpframedata[i].whichvirtual=i-1;
  p->tmpframedata[i].backgroundptr=IE_NULL;
  //!!JdS 2006/02/15: frames/smiley bug fix {
  p->tmpframedata[i].writeadr = IE_NULL;
  //... and just in case ...
  p->tmpframedata[i].nextsheet = IE_NULL;
  p->tmpframedata[i].myadr = IE_NULL;
  //!!JdS 2006/02/15: frames/smiley bug fix }
  i++;
 }
 p->tmpframedata[0].whichvirtual=0;
}

void reset_frameset(void)
{
 int i=arachne.framescount+1;
 while(i<MAXFRAMES)
 {
  p->htmlframe[i].parent=-1;
  p->htmlframe[i].next=-1;
  p->htmlframe[i].hidden=1;
  p->htmlframe[i].framename[0]='\0';
  i++;
 }

 strcpy(p->htmlframe[0].framename,"_top");
 p->htmlframe[0].allowscrolling=1;
 p->htmlframe[0].marginwidth=HTMLBORDER;
 p->htmlframe[0].marginheight=HTMLBORDER;
}

#ifndef NOTCPIP
void PPPtimelog(void)
{
 char str[80];
 long t=time(NULL)-ppplogtime;
 long totaltime=0l;
 char tm[30];
 int f=a_open("ONLINE.LOG",O_WRONLY|O_TEXT|O_APPEND,0);
 if(f<0)
  f=a_open("ONLINE.LOG",O_CREAT|O_TEXT|O_WRONLY,S_IREAD|S_IWRITE);

 inettime(tm);
 sprintf(str,"%s - online for %2d:%02d:%02d = %ld\n", tm, (int)t/3600, (int)(t/60)%60 , (int)t%60, t);
 if(f>=0)
 {
  write(f,str,strlen(str));
  a_close(f);
 }

 f=a_open("PPPTOTAL.LOG",O_RDONLY|O_TEXT,0);
 if(f>=0)
 {
  int l=a_read(f,str,79);
  if(l)
  {
   str[l]='\0';
   totaltime = strtol (str, NULL, 10);
  }
  a_close(f);
  t+=totaltime;
 }
 sprintf(str,"%ld seconds online = %2d:%02d:%02d\n", t, (int)t/3600, (int)(t/60)%60 , (int)t%60);
 f=a_open("PPPTOTAL.LOG",O_CREAT|O_TEXT|O_TRUNC|O_RDWR,S_IREAD|S_IWRITE);
 if(f>=0)
 {
  write(f,str,strlen(str));
  a_close(f);
 }
}
#endif

void exitmsg(void)
{
 x_grf_mod(3);

#ifdef POSIX
  printf(MSG_ENDX);
#else
  printf(MSG_END,VER,beta,exetype,copyright);
#endif
}

#ifndef POSIX
char cacheisfull(void)
{
 //struct ffblk ff;
 char str[80];

 makestr(str,config_get_str("CachePath", cachepath),65);

 if(updtdiskspace(str)<user_interface.mindiskspace)
  return 1;

 return 0;
}
#endif

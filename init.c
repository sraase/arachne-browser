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

void bioskey_init(void);
void bioskey_close(void);


/* maximum number of lines in cfg file */
#define LINES 1024

/* maximum number of lines in cookies file */
#define MAX_HTTP_COOKIES (64 * CookieCrumbs)

/* in errors.c */
void cfgerr (struct ib_editor *f);

int mem_xmem(unsigned *total, unsigned *free);

/**
 * set system locations
 *   syspath  : read-only
 *   userpath : read-write
 *   helppath : read-only
 */
static int setsyspaths(char *argv0)
{
	char *path;
	int pathlen;
	(void)argv0;

	/* use ARACHNE environment variable */
	path = getenv("ARACHNE");
	if (path) {
		pathlen = strlen(path);
		while (path[pathlen - 1] == PATHSEP)
			pathlen--;

		userpath = newstr("%.*s%c", pathlen, path, PATHSEP);
		syspath  = newstr("%.*s%c%s%c", pathlen, path, PATHSEP,
			"system", PATHSEP);
		helppath = newstr("%.*s%c%s%c%s%c", pathlen, path, PATHSEP,
			"system", PATHSEP, "html", PATHSEP);
		if (!syspath || !userpath || !helppath)
			return 1;
		return 0;
	}

#ifdef POSIX
	/* use POSIX defaults */
	path = getenv("HOME");
	if (!path) path = "/";
	pathlen = strlen(path);
	while (path[pathlen - 1] == PATHSEP)
		pathlen--;

	syspath  = newstr("%s", "/usr/share/arachne/");
	userpath = newstr("%.*s%c%s", pathlen, path, PATHSEP, ".arachne/");
	helppath = newstr("%s", "/usr/share/doc/arachne/html/");
#else
	/* use DOS defaults (exe location) */
	path = strrchr(argv0, PATHSEP);
	if (path) {
		pathlen = (int)(path - argv0);
		syspath  = newstr("%.*s%c", pathlen, argv0, PATHSEP);
		userpath = syspath;
		helppath = syspath;
	}
#endif

	if (!syspath || !userpath || !helppath)
		return 1;
	return 0;
}

/**
 * set user locations (available in configuration)
 *   cachepath    : read-write
 *   mailpath     : read-write
 *   downloadpath : read-write
 *   guipath      : read-only
 *   iconpath     : read-only
 *   fontpath     : read-only
 */
static int setuserpaths(void)
{
	char *ptr, *suffix = "";

#ifndef POSIX
	/* work around the fact that DOS builds store system
	   data both in C:\ARACHNE and C:\ARACHNE\SYSTEM */
	suffix = "SYSTEM\\";
#endif

	ptr = config_get_str("CachePath", NULL);
	if (ptr) cachepath = newstr("%s%c", ptr, PATHSEP);
	else     cachepath = newstr("%s%s%c", userpath, "cache", PATHSEP);

	ptr = config_get_str("MailPath", NULL);
	if (ptr) mailpath = newstr("%s%c", ptr, PATHSEP);
	else     mailpath = newstr("%s%s%c", userpath, "mail", PATHSEP);

	ptr = config_get_str("DownloadPath", NULL);
	if (ptr) downloadpath = newstr("%s%c", ptr, PATHSEP);
	else     downloadpath = newstr("%s%s%c", userpath, "download", PATHSEP);

	ptr = config_get_str("GuiPath", NULL);
	if (ptr) guipath = newstr("%s%c", ptr, PATHSEP);
	else     guipath = newstr("%s%s%s%c", syspath, suffix, "gui", PATHSEP);

	ptr = config_get_str("IconPath", NULL);
	if (ptr) iconpath = newstr("%s%c", ptr, PATHSEP);
	else     iconpath = newstr("%s%s%s%c", syspath, suffix, "ikons", PATHSEP);

#ifdef POSIX
	/* fonts are stored in syspath/encoding */
	ptr = config_get_str("FontPath ", NULL);
	if (ptr) fontpath = newstr("%s%c", ptr, PATHSEP);
	else     fontpath = newstr("%s%s", syspath, "iso-8859-1/");

	/* all gui files are stored in guipath */
	guipath2 = guipath;
#else
	/* fonts are stored in syspath/suffix */
	ptr = config_get_str("FontPath ", NULL);
	if (ptr) fontpath = newstr("%s%c", ptr, PATHSEP);
	else     fontpath = newstr("%s%s", syspath, suffix);

	/* some gui files are stored in syspath */
	guipath2 = syspath;
#endif

	if (!cachepath || !mailpath || !downloadpath ||
	    !guipath || !iconpath || !fontpath)
		return 1;

#ifdef POSIX
	/* create user folders automatically */
	mkdir(userpath, 0700);
	mkdir(cachepath, 0700);
	mkdir(mailpath, 0700);
	mkdir(downloadpath, 0700);
#endif

	return 0;
}

/**
 * set swap device (arachne.xSwap)
 */
static int setswap(void)
{
#ifdef XT086
	/* ask user for XT builds only */
	puts(MSG_MEMSEL);
	puts(MSG_MEMXMS);
	puts(MSG_MEMEMS);
	puts(MSG_MEMDSK);
	printf("%s", MSG_MEMORY);

	while (1) {
		int ch = getch();

		switch (ch) {
		case '0': case 13: arachne.xSwap = 0; return 0;
		case '1':          arachne.xSwap = 1; return 1;
		case '2':          arachne.xSwap = 2; return 2;
		case 27:
			printf("\n\n");
			exit(EXIT_ABORT_SETUP);
			return 0;
		}
	}
#else
	/* set XMS for 386+ builds */
	arachne.xSwap = 0;
	return 0;
#endif
}

/**
 * set graphics device (arachne.graphics)
 */
static int setgraphics(void)
{
#ifdef XT086
	/* ask user for XT builds only */
	puts(MSG_VGASEL);
	puts(MSG_VGAVGA);
	puts(MSG_VGAEGA);
	puts(MSG_VGACGA);
	printf("%s", MSG_VIDEO);

	while (1) {
		int ch = getch();

		switch (ch) {
		case '0': case 13: strcpy(arachne.graphics, "VGA");  return 1;
		case '1':          strcpy(arachne.graphics, "EGA");  return 2;
		case '2':          strcpy(arachne.graphics, "BCGA"); return 3;
		case 27:
			printf("\n\n");
			exit(EXIT_ABORT_SETUP);
			return 1;
		}
	}
#else
	/* set VGA for 386+ builds */
	strcpy(arachne.graphics, "VGA");
	return 1;
#endif
}

/**
 * load pick file (runtime data)
 */
static int loadpick(void)
{
	char *fname;
	int fd, len;

	/* allocate runtime storage */
	p->htmlframe = (struct HTMLframe*)farmalloc(MAXFRAMES * (2 + sizeof(struct HTMLframe)));
	AUTHENTICATION = farmalloc(sizeof(struct AUTH_STRUCT) + 2);
	if (!p->htmlframe || !AUTHENTICATION)
		return -1;

	/* open file */
	fname = newstr("%s%s", userpath, "arachne.pck");
	fd = a_open(fname, O_RDONLY | O_BINARY, 0);
	freestr(fname);

	/* read pick data */
	len = sizeof(struct ArachnePick);
	if (fd < 0 || a_read(fd, &arachne, len) != len) {
		memset(&arachne, 0, sizeof(struct ArachnePick));
		setswap();
		setgraphics();
	}

	/* read frame data */
	len = MAXFRAMES * (1 + sizeof(struct HTMLframe));
	if (fd < 0 || a_read(fd, p->htmlframe, len) != len) {
		memset(p->htmlframe, 0, len);
	}

	/* read auth data */
	len = sizeof(struct AUTH_STRUCT);
	if (fd < 0 || a_read(fd, AUTHENTICATION, len)) {
		memset(AUTHENTICATION, 0, len);
	}

	/* close file */
	if (fd >= 0)
		a_close(fd);

	return 0;
}

#ifndef POSIX
/**
 * delete pick file
 */
static void del_pick(void)
{
	char *fname = newstr("%s%s", userpath, "arachne.pck");
	unlink(fname);
	freestr(fname);
}
#endif

/**
 * load configuration file
 *   if cfgpath is not NULL, load from cfgpath
 *   otherwise, try userpath/name then syspath/name
 */
static int loadcfg(struct ib_editor *fajl, const char *name, const char *cfgpath)
{
	int rc;

	if (cfgpath) {
		/* load from cfgpath */
		strcpy(fajl->filename, cfgpath);
		rc = ie_openf_lim(fajl, CONTEXT_SYSTEM, LINES);
		if (rc == 2) {
			memerr0();
		} else if (rc == 1 && fajl->lines) {
			return 0;
		}
		ie_clearf(fajl, 0);

	} else {
		/* load from userpath */
		sprintf(fajl->filename, "%s%s", userpath, name);
		rc = ie_openf_lim(fajl, CONTEXT_SYSTEM, LINES);
		if (rc == 2) {
			memerr0();
		} else if (rc == 1 && fajl->lines) {
			return 0;
		}
		ie_clearf(fajl, 0);

		/* load from syspath */
		sprintf(fajl->filename, "%s%s", syspath, name);
		rc = ie_openf_lim(fajl, CONTEXT_SYSTEM, LINES);
		if (rc == 2) {
			memerr0();
		} else if (rc == 1 && fajl->lines) {
			return 0;
		}
		ie_clearf(fajl, 0);
	}

	/* failed to load */
	cfgerr(fajl);
	return 1;
}

int Initialize_Arachne(int argc, char **argv, struct Url *url)
{
 int grsetup=0;

 /* set system locations first */
 if (setsyspaths(argv[0]))
  return 1;

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
   del_pick();
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

 /* load runtime data */
 if (loadpick())
  return 1;

if(argc>1)
{
 if(argv[1][0]=='-')
 {
#ifndef POSIX
  if(argv[1][1]=='d')
  {
   detectgraphics();
   grsetup=setgraphics();
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

if(ie_initswap()!=1)            //initialization of swapping system ie_swap
 memerr0();
init_bin();                     //initialization of memory, conf. files, etc.
setuserpaths();                 //font, cache, gui, ... pathes
finfoload();                    //load font information
configure_user_interface();     //icons, hotkeys, scrollbuttons, font...
init_xms();                     //font caching+animated GIFs

#else                           //LINUX, etc. - graphics mode information in arachne.conf

if(ie_initswap()!=1)            //initialization of swapping system ie_swap
 memerr0();
init_bin();                     //initialization of memory, conf. files, etc.
setuserpaths();                 //font, cache, gui, ... pathes

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
   sprintf(GLOBAL.location,"file:%svga.htm",guipath2);
  else
   sprintf(GLOBAL.location,"file:%sega_cga.htm",guipath2);
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
  strcpy(GLOBAL.location,"gui:nonfatal.ah");
 }

//!!glennmcc: begin Jan 09, 2005
// on email attachment errors, goto err_mail.ah instead of nonfatal.ah
 else if(argv[1][2]=='m')
 {
  strcpy(GLOBAL.location,"gui:err_mail.ah");
 }
//!!glennmcc: end

 else if(argv[1][1]=='?')
 {
  sprintf(GLOBAL.location,"file:%shelp.htm",guipath2);
 }
}
else //no arguments !
{
 toolbarpage=0;
#ifndef POSIX
 if(cacheisfull())
  strcpy(GLOBAL.location,"gui:maintain.ah");
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

 return 0;
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

void init_bin(void)
{
 char *ptr;
 int rc;
#ifdef POSIX
 BUF=16535;
#else
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

 /* load arachne.cfg with comments */
 ARACHNEcfg.killcomment = 0;
 loadcfg(&ARACHNEcfg, "arachne.cfg", NULL);

 /* load mime.cfg without comments */
 MIMEcfg.killcomment = 1;
 loadcfg(&MIMEcfg, "mime.cfg", NULL);

 /* load toolbar.cfg without comments */
 TOOLBARcfg.killcomment = 1;
 loadcfg(&TOOLBARcfg, "toolbar.cfg", config_get_str("Toolbar", NULL));

 /* load entity.cfg without comments */
 ENTITYcfg.killcomment = 1;
 loadcfg(&ENTITYcfg, "entity.cfg", NULL);

 /* load history.lst with comments
    make sure it has at least one entry */
 ptr = config_get_str("History", NULL);
 if (ptr) {
  strcpy(history.filename, ptr);
 } else {
  sprintf(history.filename, "%s%s", userpath, "history.lst");
 }
 history.killcomment = 0;
 rc = ie_openf_lim(&history, CONTEXT_SYSTEM, LINES);
 if (rc == 2)
  memerr0();
 if (history.lines == 0) {
  ie_insline(&history, 0, "");
 }
 if (arachne.history >= history.lines) {
  arachne.history = history.lines - 1;
 }

 /* load cache index */
 ptr = config_get_str("CacheIndex", NULL);
 if (ptr) {
  strcpy(HTTPcache.filename, ptr);
 } else {
  sprintf(HTTPcache.filename, "%s%s", userpath, "cache.idx");
 }
 HTTPcache.maxlines = LINES;
 rc = ie_openbin(&HTTPcache);
 if (rc == 2)
  memerr0();

 /* load cookies.lst and check its size */
 ptr = config_get_str("CookieFile", NULL);
 if (ptr) {
  sprintf(cookies.filename, ptr);
 } else {
  sprintf(cookies.filename, "%s%s", userpath, "cookies.lst");
 }
 rc = ie_openf_lim(&cookies, CONTEXT_SYSTEM, MAX_HTTP_COOKIES);
 if (rc == 2)
  memerr0();
 if ((cookies.lines / CookieCrumbs) * CookieCrumbs != cookies.lines)
  badcookiesfile();

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

 makestr(str,cachepath,65);

 if(updtdiskspace(str)<user_interface.mindiskspace)
  return 1;

 return 0;
}
#endif

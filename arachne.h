
// ========================================================================
// Function prototypes for Arachne WWW browser
// Declaration of structures for Arachne WWW browser
// (c)1996-1999 Michael Polak, Arachne Labs
// ========================================================================

/* Explanation of project-wide defines:

NOTCPIP    ... do not use WATTCP TCP/IP stack
NOPS       ... no PostScript printing
NOKEY      ... do not use shareware registration key
OVLR       ... optimizations for overlaid executable (Borland C++ 3.1)
HICOLOR    ... use LOPIF library with support for 16 bits/pixel modes
VIRT_SCR   ... use LOPIF library with support for virtual screens
XANIMGIF   ... include animated GIFs to executable
JAVASCRIPT ... include JavaScript support to executable (no longer finished)
XTVERSION  ... maximal optimizaton for speed and size (no longer tested)

CUSTOMER   ... general customization (currently, module for Vadem is included)
AGB        ... CORE.EXE for G.E.Capital Bank in Czech Republic (acquired AGB)
TELEMED    ... for TeleMED (Germany)
HWWILSON   ... H.W.Wilson customization
CALDERA    ... Caldera Webspyder (no longer functional - being cleaned up)
*/

#include "messages.h"

#ifdef AGB
#define NOTCPIP
#define NOPS
#define NOKEY
#endif

#include "posix.h"

#ifndef NOTCPIP
#ifdef POSIX
#include "asockets.h"
void outs(char *str); //change browser status message
#else
#include "tcp.h"  //Wattcp library header file
#endif
#endif

#include "str.h"  //Arachne Labs extended string library
#include "svga.h"
#include "picinfo.h"
#include "apick.h"
#include "scrolbar.h"
#include "url.h"
#include "glflag.h"
#include "uiface.h"

#include "a_io.h" //arachne i/o stub

#ifndef NOKEY
#include "security\xchreg.h"
#endif


//----------------------------------------------------------------------------
//defines:
//----------------------------------------------------------------------------

#define TABLES            //link HTML tables
#define MODE_NORMAL   0   //TCP/IP configured in ARACHNE.CFG
#define MODE_WATTCP   1   //TCP/IP configured in WATTCP.CFG
#define MODE_BOOTP    2   //TCP/IP configured through BOOTP
#define MODE_PPP      3   //TCP/IP configured through PPP.LOG+TCPconfig file
#define LOCAL_HTML    0   //values for "source" in renderHTEML(source,....)
#define HTTP_HTML     1
#define HISTORY_HTML  2
#define IRC_HTML      3
#define FUZZYPIX      4   //constant for fuzzy logic in HTML rendering

//----------------------------------------------------------------------------
#ifdef POSIX //POSIX-compliant systems - linear access to memory, etc.
//----------------------------------------------------------------------------

#define MAXQUERY 60000    //maximum query length
#define MAXARGNAMES 1024
#define MAXNOBR 512       //maximum nobreak sections (visible frames -= 1)
#define GUIPATH "gui/"
#define MAXFRAMES 24

//----------------------------------------------------------------------------
#else //non-POSIX (DOS)
//----------------------------------------------------------------------------

#ifdef NOTCPIP
#define MIN_MEMORY 80000l //minimal free dos memory to run
#else
#ifdef XTVERSION
#define MIN_MEMORY 80000l //minimal free dos memory to run (???)
#else
#define MIN_MEMORY 80000l //minimal free dos memory to run
#endif
#endif

#define MAXQUERY 16000    //maximum query length

//maximum frame number (visible frames -= 1)
#ifdef AGB
#define MAXFRAMES 3
#else
#ifdef OVRL
#ifdef TELEMED
#define MAXFRAMES 24      //TELEMED
#else
#define MAXFRAMES 9      //overlayed
#endif
#else
#define MAXFRAMES 8       //static
#endif
#endif//maxframes

#define MAXARGNAMES 256
#define MAXNOBR 100       //maximum nobreak sections (visible frames -= 1)
#define GUIPATH "system\\gui\\"

#endif //non-POSIX (DOS)

//----------------------------------------------------------------------------
//structures:
//----------------------------------------------------------------------------

//structure which keeps information about single HTML atom
struct HTMLrecord
{
 int x,xx; //umisteni v ramecku x,y ... xx,yy
 long y,yy;
 char type;     //ATOM type, see html.h
 char align;    //0...do not align - see html.h
 char data1;
 unsigned char data2; //data
 unsigned char R,G,B; //colors
 XSWAP ptr;        //xSwap pointer
 unsigned datalen;
 XSWAP linkptr;    //xSwap pointer --> HTMLrecord <A HREF...>, <TABLE>, etc.
 XSWAP prev;       //xSwap pointer to previouces
 XSWAP next;       //xSwap pointer
 char frameID;        //0...MAXFRAMES-1
#ifdef JAVASCRIPT
 XSWAP jsptr;      //xSwap pointer --> JSrecord
#endif
};

#ifdef JAVASCRIPT
struct JSrecord
{
 //common - <INPUT>,<IMG>,<A>,<FORM>...
 XSWAP prev;
 XSWAP htmlptr;    //xSwap pointer --> JSrecord
 XSWAP name;       //xSwap pointer to string

 //<BODY>
 XSWAP onLoad;
 XSWAP onUnload;

 //<INPUT>,<IMG>,<A HREF...>
 XSWAP onMouseover; //Xswap pointer to JavaScript methods (?)
 XSWAP onMouseout;

//<INPUT>,<IMG>
 XSWAP onBlur;      //Xswap pointer to JavaScript methods (?)
 XSWAP onChange;
 XSWAP onFocus;
 XSWAP onSelect;
 XSWAP onSubmit;
 XSWAP onClick;
}
#endif

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define FRAMENAMESIZE 39

//HTML frame data stored in
struct HTMLframe
{
 //general information:
 char framename[FRAMENAMESIZE];  //target=....
 char frameborder;

 struct HTTPrecord cacheitem;
 //frame size is declared in scrollbar structure:
 struct ScrollBar scroll;

 //Netscape & MSIE compatible stuff...
 char allowscrolling;
 char marginwidth,marginheight;

 //real position in frame, in pixels
 int posX;
 long posY;

 //document status: LOCAL/REMOTE/VIRTUAL/MAIL
 unsigned status;

 //hidden frames are 1) parent frames 2) overwritten child frames
 //if parent is overwritten, this frame should be hidden:
 char hidden,parent,next;

};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//HTML frame related data, which are not saved to disk
struct TMPframedata
{

 //frame colors:
 unsigned char backR,backG,backB;
 unsigned char textR,textG,textB;
 unsigned char linkR,linkG,linkB;
#ifdef VLINK
 unsigned char vlinkR,vlinkG,vlinkB;
#endif
 int bgindex;
 int cgatext;

 //XSWAP handle of background image:
 XSWAP backgroundptr; //xSwap pointer

 //handle to save posX and posY (in cache)
 XSWAP writeadr;

 //for virtual screen mechanism:
 char usevirtualscreen;
 int whichvirtual;
 char bgproperties;
};

//----------------------------------------------------------------------------
//prototypes:
//----------------------------------------------------------------------------

//config
char *configvariable(struct ib_editor *fajl,char *keystring,char *newvalue);
extern char watfname[80];

//initialization of TCP/IP according to ARACHNE.CFG
void ArachneTCPIP(void);

//initialization of dialer according to ARACHNE.CFG
char *ArachneDIAL(void);
int PPPlog(void);

// global idle function for WATTCP
int TcpIdleFunc(void);


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//HTML, graphics, MIME, DGI

//basic function for reading HTML stream and creating HTML atoms....
//very dependent on GLOBAL.* family of variables, and more !!!
int renderHTML(char online,char forced_html, char printing);

void HTTPcharset(char *charset);
void try2readHTMLcolor(char *string,unsigned char *r,unsigned char *g,unsigned char *b);
int try2getnum(char *str,unsigned proczaklad);

//HTML redraww dunctions...
void redrawHTML(char nomsg, char virt);
void drawactiveframe(void);
#ifdef VIRT_SCR
void dumpvirtual(struct HTMLframe *frame,struct TMPframedata *htmldata, int fromx,long fromy);
void redrawatoms(char frame,
                 int from_x, long from_y,
                 int draw_x, int draw_y,
                 int screen_x, int screen_y);
#endif
void drawframeborder(char i);


//picinfo methods:
void init_picinfo(struct picinfo *img);
int drawanyimage(struct picinfo *image);

//general graphics and HTML related prototypes
int RGB(unsigned char r,unsigned char g,unsigned char b);
void resetcolorcache(void);
void initpalette(void);
char NeedImage(char reload, XSWAP from);
unsigned char HTMLentity(char *name);
void entity2str(char *str);

//convert string to "application/x-www-form-urlencoded" format:
int cgiquery(unsigned char *in,unsigned char *out,char http);

//process selected form
//cgi:0...internal config, 1...create query string, 4...isimage
void process_form(char cgi, XSWAP formID);

//get extension asociated with MIME type, according to MIME.CFG
int search_mime_cfg(char *rawmime, char *ext, char *cmd);
#define get_extension(mime,ext) search_mime_cfg(mime,ext,NULL)

//alternate display when frames are disabled
void NoMozzillaFrames(int *x,long *y, int rowsize);
void FrameIcon(int *x,long *y, int rowsize,char *text,struct picinfo *img);

//scroll HTML document to <A NAME="..."
void Goto_A_NAME(char *name);

//return to previous HTML page
char goback(void);

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//user interface
void buttons(void);            // draw buttons
void xChLogo(char n);          // draw logo animation sequence n
void xChLogoTICK(char Style);  // animate logo
void DrawTitle(char force);    // draw name and URL of page
void PaintTitle(void);         // draw name and URL of page
void defaultGUIstyle(void);
void GUIInit(void);            // inicialization of mouse, etc.
int  GUITICK(void);            // handle mouse and keyboard input
void RedrawALL(void);          // redraw everything except HTML
void mouseon(void);            // show mouse
void mouseoff(void);           // hide mouse
void ChangeZoom(char style, char plus, char minus);

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MemInfo(char forced);
#define NORMAL 0 //MemInfo parameters...
#define FORCED 1

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int  InitInput(struct ib_editor *fajl,char *name,char *str,int lines,int context);
void MakeInputAtom(struct HTMLrecord *atom,struct ib_editor *fajl,int x,long y, int xx,long yy);
void SetInputAtom(struct HTMLrecord *atom,char *str);

//draw single HTML atom
void drawatom(struct HTMLrecord *atom,
              int fromx, long fromy,
              int draw_x, int draw_y,
              int screen_x, int screen_y);

//from ie.h
void ie_redrawline(struct ib_editor *fajl,int x1,int y1, int zoomx, int width,int i);
int ie_redrawwin(struct ib_editor *fajl,int x1,int y1, int x2, int y2, char allowkey);

void putoptionline(int x,int y,int limit,struct ib_editor *fajl,int line,char multi);
void percentbar(int prc);   //0 - 100% on statusbar
void statusmsg(void);       //show status
void clock_and_timer(char *wait); //handle time...screensaver... wait=NULL!
void Piip(void);            //sound
void defaultmsg(void);      //show default message on statusbar (Copyright...)
void pressthatbutton(int nowait); //button animation
void timestr(char *str);    //produce readable time string
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//Memory management, initializiton of various stuff
void IniHimem (void);
void init_bin(void);
void Deallocmem(void);
int ie_swap(int newswap);
int ie_killswap(int zabit);
void init_xms(void);
void finfoload(void);  //load font info
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//"task management" system for 16bit DOS (640 KB limit)
int willexecute(char *cmd); //cmd will be executed...
char externalprotocol(char *protocol, char *command);
int make_cmd(char *in, char *out, char *url, char *computer, char *document, char *infile, char *outfile);
void copy(char *from,char *to);
void closebat(char *cmd, char nomode);
void rouraname(char *fname);


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//misc:

void MakeTitle(char *title);
void zoom(void);
void gohome(void);
void add2history(char *URL);
long localdiskspace(void);
long lastdiskspace(char *path);
long updtdiskspace(char *path);
// no longer used - file:cleancache.dgi is called instead...
// void gumujcache(char dukladne);
char cacheisfull(void);
int addhot(char *titleptr, char *urlptr);
void inettime(char *tm);
char scrolllock(void);
void PPPtimelog(void);
void makehttfilename(char *locname, char *httname);
int file_exists(char *name);

//initialization, errors and deinitialization

void tempinit(char *path);
void reset_tmpframedata(void);
void meminit(char arg);
void memerr0(void);
void memerr(void);
void exitmsg(void);
void mallocerr(char *msg, char *file, int line);
int exeisok(char *exename);

//atom interface:
char *onmouse(int click);
char gotonextlink( int *x, int *y , char back, char asc);
void RadioSwitch(int fromx, long fromy, XSWAP current,XSWAP formptr);
void SelectSwitch(int x,long y,int key);
void HideLink(char *hideURL);
int ProcessLinks(char generateASF);
int activeismap(int *dx, int *dy); //je aktivnim obrazkem klikatelna mapa ?
char *activeislastinput(void);
void activeurl(char *url);
char *try2getURL(void);
void deallocvirtual(void);

void activeatomredraw(void);
void activeatomcursor(char cursor);
void MakeInputAtom(struct HTMLrecord *atom,struct ib_editor *tmpeditor,int x,long y,int xx,long yy);
void SetInputAtom(struct HTMLrecord *atom,char *str);
char *gotoactiveatom(char asc, XSWAP *formID);
void activeatomsave(struct HTMLrecord *atom);
char activeatomScrollBarTICK(void);
void activeatomScrollBarBUTTONS(void);

void base64code(unsigned char *in,char *out);

//client side image maps:
XSWAP AnalyseCSIM(int x, int y,struct HTMLrecord *map);

//decompose RFC compliant string:
void decompose_inetstr(char *str);

//argument manipulation
int getvar(char *name,char **value);
void putvarname(char *name,int size);
void putvarvalue(char *value,int size);

//frames
void free_children(int parent);
void delete_children(int parent);
void reset_tmpframedata(void);
void reset_frameset(void);

//GUI action bindings:
void SearchString(void);
void saveastext(void);
void saveasps(void);
void showhighlight(void);
void WriteFileBox(void);
void ReadFileBox(void);
void SearchInTextBox(void);
int PrintScreen2BMP(char virtscr);

//----------------------------------------------------------------------------
//external variables:
//----------------------------------------------------------------------------


// ========================================================================

//screen parameters
extern int htscrn_xsize,htscrn_ysize,htscrn_ytop,htscrn_xtop;

//main structure for all frames - htmlframe[0] is always defined
extern struct HTMLframe *htmlframe;
extern struct TMPframedata *tmpframedata;

//number of frames [0..MAXFRAMES-1] - arachne.framescount

//frame Arachne currently writes to:
extern int currentframe;

//frame currently slected (activated) by user:
extern int activeframe,oldactive;

// ========================================================================

extern int firstswap; //first free swapinng buffer numbrer - ie.h
extern int swapnum;   //current swap number...

//!! This is important variable: if we have hard-written some data to
//xSwap memory area, we must set it to tell memory manager the current
//buffer ("swap") was modified

// ========================================================================
/* important for 16-bit code: made public from ie.h !
   this flag has to be set if memory handled by XSWAP handlers
   (ie_getswap, ie_putswap) was modified - alternative to ie_putswap()
*/
extern int swapmod;
//!!!!
// ========================================================================

extern char DisableXMS;

extern char exepath[65];
#ifdef POSIX
extern char helppath[80];
extern char sharepath[80];
extern char dotarachne[65];
extern char fntpath[80];
extern char fntinf[80];
#else
#define helppath exepath
#define sharepath exepath
extern char *fntinf;
#endif

extern char *buf;

extern char *msg_con;
extern char *msg_errcon;
extern char *msg_askdns;
extern char *msg_coffee;
extern char *ptrmsg;
extern char *delmsg;
extern char *ctrlbreak;
extern char *anykey;
#ifndef NOKEY
extern char regkey[KEYLEN+1],reg;
#else
extern char *regkey,reg;
#endif

extern char *imageextensions;

extern struct HTMLrecord *activeatomptr,activeatom;
extern struct HTMLrecord URLprompt; //main URL prompt
extern struct HTMLrecord TXTprompt; //for text input box - F7, etc.

extern struct ScrollBar activescroll;


extern struct GLOBAL_FLAG GLOBAL;  //global variables and flags (from glflag.h)
extern char LASTlocname[80];
extern char *Referer;

extern char closing[2];
extern int socknum;
extern int status;
extern int httplen;
//extern struct bin_file Tablelist; //indexed binarni structure
extern char nobr_overflow[MAXNOBR];
extern struct ib_editor history;   //history.lst
extern struct ib_editor cookies;   //cookies.lst
extern struct ib_editor MIMEcfg;   //mime configurration - text file
extern struct ib_editor ARACHNEcfg;//main configurration - text file
extern struct ib_editor TOOLBARcfg;//toolbar
extern struct ib_editor tmpeditor,*editorptr;//temporary IBASE editor pointer

extern char myIPstr[20];           //my IP address
extern char graphics;            //is graphics available ?
extern char tcpip;               //is TCP/IP available ?
extern char httpstub;            //is HTTP stub available ?
extern char ipmode;
extern char memory_overflow;     //document is too long!
extern char memory_model;          //LOW DOS memory management (0..4)
extern int BUF;               //velikost vyrovnavaciho buferu
extern int loadrefresh;       //perioda prekreslovani (v bajtech) pro LAN TCP/IP
extern char noGUIredraw;

#define fullscreen (arachne.GUIstyle & 4)
#define customerscreen (arachne.GUIstyle & STYLE_CUSTOMER)

extern char iconsoff;            //hide small icons ?
extern char hotkeys;             //allow hotkeys ?

extern char htmlmsg[100];        //For tag <ARACHNE MSG="....">

extern char htmlpulldown;
extern int htmlsavex,htmlsavey;
extern char meminfovisible;
extern char activeistextwindow;

extern char *setupdoc;

extern char *VER;
extern char *exetype;
extern char *beta;
extern char *copyright;
extern char *homepage;

extern char nemapuj; //do not map icon palette

//variables specific to HTML rendering
//should be moved to single data structure in future...
extern unsigned char r,g,b;
extern int left,right,rowsize,textrowsize,leftedge,rightedge;
extern long xsum,maxsum;
extern int HTMLswap;

extern char islist;
extern long clearleft,clearright;
extern long percflag;
extern char *text;
extern struct picinfo *img;

extern char neediknredraw;

//for 256 color modes:
extern int IiNpal; //total lenghth of paletter --> zero when clear screen!

//XSWAP pointers to various parts of HTML document:
extern XSWAP firstonscr,lastonscr;
extern XSWAP firstHTMLatom,lastHTMLatom;
extern XSWAP firstHTMLtable,nextHTMLtable,prevHTMLtable;

extern long HTMLatomcounter;
extern XSWAP lastonmouse,activeadr,focusedatom;
extern XSWAP lastfound;
extern long lastfoundY;
extern int lastfoundX;
extern char *cachepath;
extern char *tempdir;

extern long ScreenSaver;
extern long SecondsSleeping; //for screensaver
extern char lasttime[32];   //for screensaver
extern char global_nomouse;
extern char *hotlist;
extern char egamode,cgamode,vga16mode,vgamono,ignoreimages;
extern char fixedfont;
extern int ikn;
extern int ikniddle;
extern char global_nomouse;
extern int mys;
extern int lx,ly,lmouse;
extern int lastonbutton,justmoved;
extern char redraw;
extern char scrolledframe;
extern char atomneedredraw;
extern int thisx,thisy,thisxx,thisyy;
extern int currentfont;
extern char currentstyle;

extern char *Iipal; //souhrnna paleta

#ifdef VIRT_SCR
extern int virtualxstart[MAXVIRTUAL];
extern long virtualystart[MAXVIRTUAL];
extern int virtualxend[MAXVIRTUAL];
extern long virtualyend[MAXVIRTUAL];
extern char allocatedvirtual[MAXVIRTUAL];
extern int maxusedvirtual;
extern XSWAP virtualpalhandle;
extern XSWAP virtualfirstonscr,virtuallastonscr;
extern int virtualIiNpal;
#endif

extern int argnamecount,argvaluecount;
extern char *argnamestr,*argvaluestr;
extern long ppplogtime;
extern char ascii160hack;

extern struct Url baseURL;
extern char GlobalLogoStyle;

extern int swapoptimize;
extern char *vgadetected;
#define MALLOCERR() mallocerr(ptrmsg,__FILE__,__LINE__)

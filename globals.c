
// ========================================================================
// Global variables declaration files
// (c)1997 xChaos software, portions (c)1987 Caldera Inc.
// ========================================================================

#include "arachne.h"
#include "htmtable.h"

//velikost stacku
unsigned _stklen=65500u;

#ifdef OVRL
#ifdef HICOLOR
#ifdef LINUX
#ifdef GGI
char *exetype="; GGI";
#else
char *exetype="; svgalib";
#endif
#else
char *exetype="";
#endif
#else
char *exetype="; svgaovrl";
#endif //hicolor
#else
char *exetype="; svgastat";
#endif //ovrl

// ========================================================================

//screen parameters
int htscrn_xsize,htscrn_ysize,htscrn_ytop,htscrn_xtop;

//main structure for all frames - htmlframe[0] should be always defined
struct HTMLframe *htmlframe=NULL; //allocate MAXFRAMES*sizeof(.)+1!
struct TMPframedata *tmpframedata=NULL; //allocate MAXFRAMES*sizeof(.)+1!

//number of frames [0..MAXFRAMES-1] is in arachne.framescount

//frame Arachne currently writes to:
int  currentframe=0;

//frame currently slected (activated) by user:
int activeframe=0,oldactive=0;

// ========================================================================

char exepath[65]="\0";
#ifdef POSIX
char helppath[80]="\0";
char sharepath[80]="\0";
char dotarachne[65]="\0";
char fntinf[80]="\0";
char fntpath[80]="\0";
#else
char *fntinf="system\\fontinfo.bin";
#endif


char *buf=NULL;            //univerzalni buffer vstup vystup
char *msg_con=MSG_CON;
char *msg_errcon=MSG_ERRCON;
char *msg_askdns=MSG_ASKDNS;
char *ptrmsg=MSG_XSWAP;
char *delmsg=MSG_XDEL;
char *ctrlbreak=MSG_BREAK;
char *anykey=MSG_ANYKEY;
#ifdef NOKEY
char *regkey="",reg=1;
#else
char regkey[KEYLEN+1]=MSG_UNREG,reg=0;
#endif

char *imageextensions="GIF IKN BMP gif ikn bmp";

struct HTMLrecord *activeatomptr=NULL,activeatom;
struct HTMLrecord URLprompt; //main URL prompt;
struct HTMLrecord TXTprompt; //secondary input prompt (search, r/w block...)
//struct HTMLrecord pulldown; //pulldown menu
struct ScrollBar activescroll;

struct GLOBAL_FLAG GLOBAL;  //global variables and flags (from glflag.h)
struct AUTH_STRUCT *AUTHENTICATION;
char LASTlocname[80]="\0";
char *Referer;

#ifndef NOTCPIP
#ifdef WATTCP
tcp_Socket *socket;
tcp_Socket *sock[2];
//tcp_Socket *websocket;
char closing[2]={0,0};
#endif
int socknum;
#endif

int status;
int httplen;
//struct bin_file Tablelist; //obsolete binary structurers
struct bin_file HTTPcache;   //binary cache database
char nobr_overflow[MAXNOBR];
struct ArachnePick arachne; //autosave konfiguracni soubor
char title_ok=0;
struct ib_editor history;   //historie "TADYJSEM.BYL"
struct ib_editor cookies;
struct ib_editor MIMEcfg;   //mimekonfigurace
struct ib_editor ARACHNEcfg;//hlavni konfigurace
struct ib_editor TOOLBARcfg;//toolbar
struct ib_editor tmpeditor,*editorptr;//ukazatel na jeden vyskyt IBASE editoru
struct uiface user_interface; //user controled variables

char myIPstr[20]="0.0.0.0";           //moje IP adresa
char graphics=0;            //je k dispozici grafika ?
char tcpip=0;               //je k dispozici TCP/IP ?
char httpstub=0;            //je k dispozici TCP/IP ?
char ipmode=MODE_NORMAL;
char memory_overflow=0;     //prilis dlouhy dokument
char memory_model=0;        //memory_model=1 ... optimum performance
int BUF;                    //velikost vyrovnavaciho buferu
int loadrefresh=1000;       //perioda prekreslovani (v bajtech) pro LAN TCP/IP
char noGUIredraw=0;

char htmlmsg[100]="\0";

char htmlpulldown=0;
int htmlsavex=-1,htmlsavey;
char activeistextwindow=0;
char meminfovisible=0;

#ifdef POSIX
char *setupdoc="gui:options.ah";
#else
char *setupdoc="file:%soptions.htm";
#endif

//global palette
int    IiNpal=0;   //delka souhrnne palety --> vynulovat pri Clrscr apod.!
char   *Iipal; //souhrnna paleta

//variables specific to HTML rendering
unsigned char r,g,b;
int left,right,rowsize,textrowsize,leftedge,rightedge;
long xsum,maxsum;
long clearleft,clearright;
long percflag;
char *text=NULL;
struct HTMLtable *thistable=NULL;
struct picinfo *img=NULL;

char neediknredraw;

//pointers to dynamic array of HTML atoms:
XSWAP firstHTMLatom=IE_NULL,lastHTMLatom=IE_NULL;
XSWAP firstHTMLtable=IE_NULL,nextHTMLtable=IE_NULL,prevHTMLtable=IE_NULL;
long HTMLatomcounter=0;
XSWAP lastonmouse=IE_NULL,activeadr=IE_NULL,focusedatom=IE_NULL;
XSWAP firstonscr=IE_NULL,lastonscr=IE_NULL;
XSWAP lastfound=IE_NULL;
long lastfoundY=0l;
int lastfoundX=0;

#ifdef POSIX
char *cachepath="cache/";
#else
char *cachepath="CACHE\\";
#endif
char *hotlist="hotlist.htm";

//globar variables for GUI
char egamode=0,cgamode=0,vga16mode=0,vgamono=0,ignoreimages=0;
char fixedfont=0;
int ikn=0;
int ikniddle=0;
int lx,ly,lmouse;
char global_nomouse=0, kbmouse=0;
int mys=-1;     //start in special mouse mode to avoid fist return
int lastonbutton=-1,justmoved=0;
char redraw=0;
char scrolledframe=-1;
char atomneedredraw=0;
int currentfont=-1;
char currentstyle=-1;

#ifdef VIRT_SCR
int virtualxstart[MAXVIRTUAL];
long virtualystart[MAXVIRTUAL];
int virtualxend[MAXVIRTUAL];
long virtualyend[MAXVIRTUAL];
char allocatedvirtual[MAXVIRTUAL];
int maxusedvirtual=0;
XSWAP virtualpalhandle=IE_NULL;
XSWAP virtualfirstonscr=IE_NULL,virtuallastonscr=IE_NULL;
int virtualIiNpal;
#endif

int argnamecount=0,argvaluecount=0;
char *argnamestr=NULL,*argvaluestr=NULL;

long ppplogtime=0;

char GlobalLogoStyle;		//SDL

struct Url baseURL;

char toolbarmode=0; //0...HTML, 1...editor
//char toolbarpage=0;

//bufer barev na obrazovce:
unsigned char cacher[16];
unsigned char cacheg[16];
unsigned char cacheb[16];
unsigned char coloridx[16];
char rgbcacheidx;

#ifndef XTVERSION
int lasthisx,lasthisy,lasthisxx,lasthisyy;
#endif

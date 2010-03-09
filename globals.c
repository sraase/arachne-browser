
// ========================================================================
// Global variables declaration files
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"

//velikost stacku (tr.: size of stack)
//!!glennmcc: July 16, 2005 -- bad idea... it causes many, many crashes :(
//unsigned _stklen=32750u;//!!glennmcc: July 09, 2005 -- reduced stack size
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

struct Page P,*p=&P;

//main structure for all frames - htmlframe[0] should be always defined
//struct HTMLframe *htmlframe=NULL; //allocate MAXFRAMES*sizeof(.)+1!
//struct TMPframedata *tmpframedata=NULL; //allocate MAXFRAMES*sizeof(.)+1!

//number of frames [0..MAXFRAMES-1] is in arachne.framescount

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


//char *buf=NULL;            //univerzalni buffer vstup vystup
                         // tr.: universal buffer input output
char *msg_con=MSG_CON;
char *msg_errcon=MSG_ERRCON;
char *msg_askdns=MSG_ASKDNS;
char *ptrmsg=MSG_XSWAP;
char *delmsg=MSG_XDEL;
char *ctrlbreak=MSG_BREAK;
char *anykey=MSG_ANYKEY;
#ifdef NOKEY
//char *regkey="",reg=1;
//char *regkey="GPL  Version",reg=1;//!!glennmcc: Apr 06, 2003 --- GPL now
#if defined(LINUX)
char *regkey="v1.95;Linux",reg=1;
#elif defined(XT086)
char *regkey="v1.95;GPL,286-",reg=1;
#else
char *regkey="v1.95;GPL,387+",reg=1;
#endif//LINUX
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
int sock_datalen[2]={0,0};
#endif
char sock_keepalive[2][STRINGSIZE]={"\0","\0"};
int socknum;
#endif

int status; //for WATTCP

//struct bin_file Tablelist; //obsolete binary structures
struct bin_file HTTPcache;   //binary cache database
char nobr_overflow[MAXNOBR];
struct ArachnePick arachne; //autosave configuration file
char title_ok=0;
struct ib_editor history;   //historie "TADYJSEM.BYL"
            // tr.: "I have been here already"
struct ib_editor cookies;
struct ib_editor MIMEcfg;   //mime configuration
struct ib_editor ARACHNEcfg;//main configuration
//!!glennmcc: May 27, 2007 -- read entity conversions from entity.cfg
struct ib_editor ENTITYcfg;//entity.cfg
struct ib_editor TOOLBARcfg;//toolbar
struct ib_editor tmpeditor,*editorptr;//ukazatel na jeden vyskyt IBASE editoru
         // tr.: pointer to one occurence  of the IBASE editor
struct uiface user_interface; //user controled variables

char myIPstr[20]="0.0.0.0";           // my IP address
char tcpip=0;               // is TCP/IP available?
char httpstub=0;            // is TCP/IP available?
char ipmode=MODE_NORMAL;
char memory_model=0;        //memory_model=1 ... optimum performance
int BUF;                    //work buffer size for p->buf and p->text
int loadrefresh=1000;       //perioda prekreslovani (v bajtech) pro LAN TCP/IP
    // tr.: period of redrawing/overwriting (in Bytes) for LAN TCP/IP
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
            // tr.: length of composite palette --> erase at Clrsrc etc.!
char   *Iipal; //souhrnna paleta (tr.: composite palette)

//this is just optimization flag for IKN library
char neediknredraw;

//pointers to dynamic array of HTML atoms:
XSWAP lastonmouse=IE_NULL,activeadr=IE_NULL,focusedatom=IE_NULL;
XSWAP lastfound=IE_NULL;
long lastfoundY=0l;
int lastfoundX=0;

#ifdef POSIX
char *cachepath="cache/";
#else
char *cachepath="CACHE\\";
#endif
char *hotlist="hotlist.htm";

//global variables for GUI
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
//!!JdS 2004/1/31 Alternative scheme to argnamecount, argvaluecount :
int argvarcount=0;
char *argnamestr=NULL,*argvaluestr=NULL;
//!!JdS 2004/1/31 Alternative scheme to argnamestr, argvaluestr :
char *argnameptr[MAXARGS], *argvalueptr[MAXARGS];

long ppplogtime=0;

char GlobalLogoStyle;      //SDL

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


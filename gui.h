
// ========================================================================
// Graphical user interface header file... 1998
// Cooperation with customer.h defined function for STYLE_CUSTOMER layout
// (c)1996-1999 Michael Polak, Arachne Labs
// ========================================================================

void onlinehelp(int b);
int onbutton(int x,int y);
void inputatom(char *msg1,char *msg2);
//!!glennmcc: Jan 23, 2005 -- at Ray's suggestion, unsigned int
unsigned int GUIEVENT(unsigned int key, unsigned int mys);
//int GUIEVENT(int key, int mys);
void getTXTprompt(char *str,int lim);
void setTXTprompt(char *str);
void ReadWriteTextarea(char sw);
void SearchInTextarea(char cont);
int activeatomtick(int key, char textareacmd);
int smothup(int rate);
int smothdown(int rate);
void hidehover(void);

//values for "textareacmd" argument of activeatomtick
#define TEXTAREA_NOPE      0
#define TEXTAREA_INIT      1
#define TEXTAREA_SCROLL    2
#define TEXTAREA_NOREFRESH 3

#define BLOCK_INK    0
#define BLOCK_PAPER 14

#define GUI_ACTION              1
#define GUI_MOUSE               10
#define GUI_MOUSE_BUTTON_LEFT   11
#define GUI_MOUSE_BUTTON_RIGHT  12
#define GUI_MOUSE_BUTTON_MIDDLE 13

#define MOUSE_RELEASE           3

#ifdef AGB
#define REDRAW_KEY 0x3f00
#else
#define REDRAW_KEY 0x4300
#endif

//click events for online help, etc.
#define CLICK_PREVIOUS       1
#define CLICK_NEXT           2
#define CLICK_HOME           3
#define CLICK_RELOAD         4
#define CLICK_ADDHOTLIST     5
#define CLICK_HOTLIST        6
#define CLICK_ABORT          7
#define CLICK_SEARCHENGINE   8
#define CLICK_HELP           9

#define CLICK_ANY_BIG_BUTTON 10
#define CLICK_SPECIAL        900

#define CLICK_MEMINFO 901
#define CLICK_TCPIP   902
#define CLICK_NETHOME 903
#define CLICK_HISTORY 904
#define CLICK_ZOOM    905
#define CLICK_EXIT    906
#define CLICK_IMAGES  907
#define CLICK_DESKTOP 908
#define CLICK_ABOUT   909
#define CLICK_MAIL    910
#define CLICK_SAVE    911

#define ONMOUSE_TITLE 999

#define CLICK_CUSTOMER 1000

#define MOUSESTEP 8

#include "pckbrd.h"

#ifndef XTVERSION
extern int wheelqueue; //for wheel mice
extern int thisx,thisy,thisxx,thisyy;

int analysewheel(int mys);
void pressthatbutton(int nowait);
#endif

int onbig(int x, int y, int x0, int y0 );
int onverysmall(int x, int y, int x0, int y0 );
int onbutton(int x,int y);
char scrolllock(void);
char shift(void);
#ifdef VIRT_SCR
void smothscroll(long from,long to);
void novirtual(void);
void Try2DumpActiveVirtual(void);
#endif
void toolbar(char newtoolbarmode,char forced);
int gotoloc(void);

extern char toolbarmode; //0...HTML, 1...editor
int geticoninfo(char *name,char *icon,char *method,char *methodarg,char *desc1,char *desc2);
int endvtoolbar(void);
void hidehighlight(void);

extern int swapsavecount,swaploadcount;
extern char kbmouse;

#ifndef XTVERSION
extern int lasthisx,lasthisy,lasthisxx,lasthisyy;
#endif

#define MAXBUTT 200004l  //max allowed animated button size
#define MAXHOVER (long)(IE_MAXSWAPLEN-4000l)  //max allowed hover size


// ========================================================================
// arachne.pck related stuff (structure arachne)
// (c)1996-1999 Michael Polak, Arachne Labs
// ========================================================================

#define MAXBACKTRACE 64
#define MAXTITLELEN  95

#define SWAP_DISK 0
#define SWAP_XMS  1
#define SWAP_EMS  2

struct ArachnePick              //. This struct is stored in arachne.pck !!!!  Run time data
{
 char graphics[14];//last video mode (NUL ('\0')terminated - like "VESA.C\0"
                   //.exampes of modes: EGA,VGA,TRIDENT.B, VESA.C, HI15.I ... HI16.K
 int scriptline;   //0...not running script, 1...next line to go...
 char GUIstyle;    //0-menu on the right side,
                   //1-menu like normal browser
                   //2-almost fullscreen
                   // & 4 ...fullscreen (3rd bit set)
 char xSwap;       //see SWAP_DISK, SWAP_XMS, SWAP_EMS
 int mousex,mousey,framescount; //mouse x and y, number of frames
 int target;      //current target inside frameset.  The number of frame. MAXFRAMES no of frame is adjustable
                                   // == 0 if there is no frame   _top : 0 frame
 int newframe;    //new frame to be loaded

 long cachesize;   //size of HTTP cache - not yet implemented
 int history;      //current pointer to history file history.lst
 int backtrace;                       //pointer to backtrace_target
                                      //. When click on back, redraw the frame only.
                                                                          //. Works correctly for current frame set
 char backtrace_target[MAXBACKTRACE]; //trace targets of clicks
 char title[MAXTITLELEN];             //main title of the entire frameset
 long pagetime;
 int toolbarpage;
};

extern struct ArachnePick arachne; //autosave configuration file

int loadpick( char *exename); //load autosave configuration
void savepick(void);           //save autosave configuration

#define mousex arachne.mousex
#define mousey arachne.mousey
#define toolbarpage arachne.toolbarpage
#define pagetime arachne.pagetime

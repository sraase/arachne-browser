
// ========================================================================
// global actions of Arachne WWW browser
// (c)1996-1999 Michael Polak, Arachne Labs
// ========================================================================

struct GLOBAL_FLAG
{
 char gotolocation;     //goto new location!
 char needrender;       //we have to analyze current page again
 char isimage;          //current processed URL is image 0...no, 1...yes
 char postdata;         //is URL <FORM ACTION> 0...no, 1...GET, 2...POST
 char abort;            //0...no, 1...aborted by user
 XSWAP postdataptr;     //XSWAP pointer to current METHOD=POST query string
 char *location;        //location to go if GLOBAL.gotolocation
 char validtables;      //HTML tables are vailud
// char tableadjust;      //do we need to re-calculate HTML tables ?
 char tabledepth;       //depth of nested tables
 char nothot;
 char source;
 char imagevisible;
 char reload;
 char nowimages;
 char willexecute;
 char redirection;
 char backgr;
 char del;
 char allowdealloc;     //don't allow reloading document during download!!
 char norefresh;
 char mailaction;
// char missingobjects;
 char timeout;
 int secondsleft;
 char clipdel;
 char refreshtarget;
 char activate_textarea;

 //charset
 char currentcharset[9];
 char codepage[256];

 //for background image download...
 char backgroundimages;
 long back_size;            // size in bytes
 char back_knowsize;        // logical - size is valid
 int back_socknum;
 int back_handle;
 int back_status;
 int back_iddle;
};


//values for GLOBAL.nowimages

#define IMAGES_NOTNOW 0
#define IMAGES_LOAD 1
#define IMAGES_SEEKCACHE 2

//values for Needimage()

#define FIND_MISSING_IMAGE 0
#define EXPIRE_ALL_IMAGES  1

#define GLOBAL_justrestarted (GLOBAL.nowimages==IMAGES_SEEKCACHE)

//values for GLOBAL.reload

#define NO_RELOAD 0
#define RELOAD_CURRENT_LOCATION 1
#define RELOAD_NEW_LOCATION 2

//values for second argument of closebat()

#define RESTART_REDRAW 0
#define RESTART_KEEP_GRAPHICS 1
#define RESTART_NONFATAL_ERROR 2
#define RESTART_TEST_ERRORLEVEL 3

//values for redrawHTML

#define REDRAW_WITH_MESSAGE 0
#define REDRAW_NO_MESSAGE 1
#define REDRAW_SCREEN 0
#define REDRAW_CREATE_VIRTUAL 1
#define REDRAW_VIRTUAL 2

//return values of status in SearchInCache:

#define LOCAL   0
#define VIRTUAL 1
#define REMOTE  2
#define MAIL    3

//GLOBAL.mailaction values (bit array!)

#define MAIL_SAVENOW   0
#define MAIL_SMTPNOW   1
#define MAIL_ATTACH    2
#define MAIL_OUTBOXNOW 4


//AUTHENTICATION->flag

#define AUTH_UNDEFINED 0
#define AUTH_REQUIRED  1
#define AUTH_OK        2
#define AUTH_FORCED    3

//GLOBAL.abort

#define ABORT_TRANSFER 1
#define ABORT_PROGRAM  2

//arachne.GUIstyle

#define STYLE_ARACHNE    0
#define STYLE_MOZILLA    1
#define STYLE_SMALL1     2
#define STYLE_SMALL2     3
//#define STYLE_FULLSCREEN 4  //this is real fullscreen mode

//forced_html

#define RELOAD_HTML_FRAMES 2

//renderHTML 3rd parameter:

#define RENDER_SCREEN  0
#define RENDER_PRINTER 1

//GLOBAL.clipdel

#define CLIPBOARD_DEFAULT   0  //!!JdS 2004/12/08
#define CLIPBOARD_DELETE    1
#define CLIPBOARD_ADDHOT    2
#define CLIPBOARD_DEFER_ADD 3  //!!JdS 2004/12/08

//activeistextwindows

#define INPUT_SEARCHSTRING 1
#define INPUT_ADDHOT       2
#define INPUT_READFILE     3
#define INPUT_WRITEFILE    4
#define INPUT_URL          5
#define INPUT_SEARCHINTEXT 6

//GLOBAL.esc

#define ESC_IGNORE 0
#define ESC_EXIT   1
#define ESC_BACK   2

//GLOBAL.backgroundimages

#define BACKGROUND_EMPTY    0
#define BACKGROUND_SLEEPING 1
#define BACKGROUND_RUNNING  2
#define BACKGROUND_ZOMBIE   3

//FinishBackground(BG_FINISH|BG_ABORT)

#define BG_FINISH      0
#define BG_ABORT       1
#define BG_FINISH_ALL  2

//user_interface.multitasking

#define MULTI_NO   0
#define MULTI_YES  1
#define MULTI_SAFE 2

//exit to arachne.bat - ERRORLEVEL

#define EXIT_FAST_EXECUTE     128
#define EXIT_EXECUTE          64
#define EXIT_TO_DOS           32
#define EXIT_GRAPHICS_ERROR   16
#define EXIT_RESOLUTION_ERROR 8
#define EXIT_CONTINUE_SETUP   4
#define EXIT_ABORT_SETUP      2
#define EXIT_ABNORMAL         1   //!!JdS

#define TABLES_UNKNOWN    0
#define TABLES_EXPAND     1
#define TABLES_FINISHED   2

//contexts for ie_putswap

#define CONTEXT_HTML   0
#define CONTEXT_SYSTEM 1
#define CONTEXT_TABLES 2
#define CONTEXT_ICONS  3
#define CONTEXT_TMP    4
#define CONTEXT_TMPIMG 5

//return values of protocol_arachne() and protocols_nonhttp()

#define CONTINUE_TO_RENDER 0
#define GOTO_IVEGOTNEWURL  1
#define GOTO_END           2
#define GOTO_USEREND       3
#define GOTO_ERROR         4
#define GOTO_PROXY         5
#define GOTO_ABORT         6
#define GOTO_LOCAL_HTML    7
#define GOTO_READSCRIPT    8
#define GOTO_EXTERNAL      9
#define GOTO_TRYPLUGIN    10
#define UNKNOWN_PROTOCOL  11


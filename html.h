
// ========================================================================
// HTML rendering header file
// (c)1997-1999 Michael Polak, Arachne Labs
// ========================================================================

// requires included arachne.h

#define MAXFONTSTACK 17
struct Fontstack
{
 int depth;
 int font[MAXFONTSTACK+1];
 char style[MAXFONTSTACK+1];
 char rgb[3*MAXFONTSTACK+1];
};

//HTML stream prototypes
int openHTML(struct HTTPrecord *cache,char source);
int readHTML(struct HTTPrecord *cache,char source);
void closeHTML(struct HTTPrecord *cache,char source);

//TAG analysis optimized for speed
int FastTagDetect(char *tagname);

//for drawing page in 256 color modes...
void MixVisiblePaletes(char writepal);
int MixPal(struct picinfo *o,int n,char writepal);

//frame management
void addframeset(char xflag, int *emptyframe, char framewantborder, char *startptr);
char findfreeframe(void);
int findtarget(int basetarget);
//void resetframeborder(struct HTMLframe *frame, char shift);

#define DONT_WANT_FRAMEBORDER 0
#define FRAMEBORDER_IS_ON     1
#define I_WANT_FRAMEBORDER    2
#define UNDEFINED_FRAMEBORDER 4

#define BGPROPERTIES_SCROLL   0
#define BGPROPERTIES_FIXED    1

//HMTL table related..
void verticalalign(XSWAP adr,XSWAP tbladr,char valign,long yshift);
void tablerow(long y,long yy,XSWAP tbladr,int padding);
char calcnestedtables(int idx);
void ExpandTables(void);

//other prototypes
void addatom(struct HTMLrecord *atom,void *ptr,int len,char t, char align,char d1, unsigned char d2,unsigned currentlink,char norightedge);
void xshift(int *x,int d);
void drawatom(struct HTMLrecord *atom,int fromx, long fromy,int draw_x, int draw_y,int screen_x, int screen_y);
void will_activate_atom(int setx, int sety);
void html_background(char whichframe,int screen_x, int screen_y,int draw_x, int draw_y);
void alignrow(int x,long y,int islist);
char closeatom(XSWAP adr,int deltax,long absy);
int atom2nextline(int x, long y, XSWAP adr);
void clearall(long *y);
void appendline(XSWAP currenttextarea,char *text,int gotothere);
void ResetHtmlPage(struct TMPframedata *html,char ishtml, char allowuser);
void LinkUSEMAPs(void);
void highlightatom(struct HTMLrecord *foundatom);
void fixrowsize(int font,char style);
void pushfont(int font,char style, struct HTMLrecord *atom, struct Fontstack *fontstack);
int popfont(int *font,char *style, struct HTMLrecord *atom, struct Fontstack *fontstack);
int space(char font);

//external treatment of certain tags outside renderHTML:
void METAtag(void);
void USEMAParea(struct HTMLrecord *atom,char basetarget);
void BodyArachne(struct TMPframedata *html);
void DummyFrame(struct Page *p,int *x, long *y);
void CheckArachneFormExtensions(struct HTTPrecord *cache,char *value, int *checked);

#define CONSOLEWIDTH user_interface.printerwidth
#define OPTIONFONT (2+user_interface.fontshift)
#define BUTTONFONT (3+user_interface.fontshift)

extern char title_ok;


struct RENDER_DATA
{
 char willadjusttables;
 char translatecharset;
};

extern struct RENDER_DATA RENDER;

struct Fontdecoration
{
 char colormap[5];
 char rgb[15];
};

extern unsigned char cacher[16];
extern unsigned char cacheg[16];
extern unsigned char cacheb[16];
extern unsigned char coloridx[16];
extern char rgbcacheidx;


#define PARAGRAPH \
alignrow(x,y,orderedlist[listdepth]);\
if(x>p->docLeft && lasttag!=TAG_BODY || lasttag==TAG_SLASH_TABLE || \
   lasttag==TAG_BR || lasttag==TAG_UL || lasttag==TAG_OL || lasttag==TAG_OL)\
{\
 y+=p->sizeRow;\
 if(fixedfont)\
  y+=fonty(basefont,0);\
 else\
  y+=fonty(basefont,0)/2;\
}\
if(p->xsum>p->maxsum)\
 p->maxsum=p->xsum;\
p->xsum=0;


// basic HTML rendering modes:
#define TEXT_PLAIN 0
#define TEXT_HTML  1

// this defines can change page layout:
#define HTMLBORDER 4
#define LISTINDENT 16

// HTML atom types:
#define EMPTY -1
#define TEXT   0
#define IMG    1
#define HR     2
#define HREF   3
#define LI     4
#define TABLE  5
#define INPUT  6
#define FORM   7
#define USEMAP 8
#define NAME   9
#define MAP    10
#define TD     11
#define AREA   12
#define EMBED  13

//special HTML atoms:
#define DECORATION    98
#define BACKGROUND    99
#define TD_BACKGROUND 100

// atom type=INPUT subtypes:
#define PASSWORD  1
#define SUBMIT    2
#define RESET     3
#define HIDDEN    4
#define SELECT    5
#define RADIO     6
#define CHECKBOX  7
#define TEXTAREA  8
#define EDITOR    9
//#define LARGEEDIT 10 //for CALDERA
#define BUTTON    11
#define OUTPUT    12

// align types
#define BOTTOM 1
#define CENTER 2
#define LEFT   4
#define RIGHT  8
#define TOP    16
#define MIDDLE 32
#define SUP    64
#define SUB    128


//HTML tags
#define TAG_SLASH          1000
#define TAG_P              1
#define TAG_SLASH_P        1001
#define TAG_A              2
#define TAG_SLASH_A        1002
#define TAG_IMG            3
#define TAG_BR             4
#define TAG_CENTER         5
#define TAG_SLASH_CENTER   1005
#define TAG_NOBR           6
#define TAG_SLASH_NOBR     1006
#define TAG_DIV            7
#define TAG_SLASH_DIV      1007
#define TAG_H1             11
#define TAG_SLASH_H1       1011
#define TAG_H2             12
#define TAG_SLASH_H2       1012
#define TAG_H3             13
#define TAG_SLASH_H3       1013
#define TAG_H4             14
#define TAG_SLASH_H4       1014
#define TAG_H5             15
#define TAG_SLASH_H5       1015
#define TAG_H6             16
#define TAG_SLASH_H6       1016
#define TAG_TT             20
#define TAG_SLASH_TT       1020
#define TAG_PRE            21
#define TAG_SLASH_PRE      1021
#define TAG_FONT           22
#define TAG_SLASH_FONT     1022
#define TAG_BASEFONT       23
#define TAG_SLASH_BASEFONT 1023
#define TAG_HR             24
#define TAG_B              25
#define TAG_SLASH_B        1025
#define TAG_I              26
#define TAG_SLASH_I        1026
#define TAG_TABLE          27
#define TAG_SLASH_TABLE    1027
#define TAG_TD             28
#define TAG_SLASH_TD       1028
#define TAG_TR             29
#define TAG_SLASH_TR       1029
#define TAG_TH             30
#define TAG_SLASH_TH       1030
#define TAG_LI             31
#define TAG_SLASH_LI       1031
#define TAG_DD             32
#define TAG_SLASH_DD       1032
#define TAG_UL             33
#define TAG_SLASH_UL       1033
#define TAG_OL             34
#define TAG_SLASH_OL       1034
#define TAG_INPUT          35
#define TAG_FORM           36
#define TAG_SLASH_FORM     1036
#define TAG_HEAD           37
#define TAG_SLASH_HEAD     1037
#define TAG_TITLE          38
#define TAG_SLASH_TITLE    1038
#define TAG_SCRIPT         39
#define TAG_SLASH_SCRIPT   1039
#define TAG_CAPTION        40
#define TAG_SLASH_CAPTION  1040
#define TAG_TEXTAREA       41
#define TAG_SLASH_TEXTAREA 1041
#define TAG_BODY           42
#define TAG_SLASH_BODY     1042
#define TAG_BASE           43
#define TAG_SLASH_BASE     1043
#define TAG_CODE           44
#define TAG_SLASH_CODE     1044
#define TAG_FRAMESET       45
#define TAG_SLASH_FRAMESET 1045
#define TAG_FRAME          46
#define TAG_SELECT         47
#define TAG_SLASH_SELECT   1047
#define TAG_OPTION         48
#define TAG_SLASH_OPTION   1048
#define TAG_U              49
#define TAG_SLASH_U        1049
#define TAG_NOFRAMES       50
#define TAG_SLASH_NOFRAMES 1050
#define TAG_SMALL          51
#define TAG_SLASH_SMALL    1051
#define TAG_BIG            52
#define TAG_SLASH_BIG      1052
#define TAG_SUP            53
#define TAG_SLASH_SUP      1053
#define TAG_SUB            54
#define TAG_SLASH_SUB      1054
#define TAG_MAP            55
#define TAG_SLASH_MAP      1055
#define TAG_AREA           56
#define TAG_META           57
#define TAG_EMBED          58
#define TAG_BUTTON         59
#define TAG_SLASH_BUTTON   1059
#define TAG_NOSCRIPT       60
#define TAG_SLASH_NOSCRIPT 1060
#define TAG_BLOCKQUOTE       61
#define TAG_SLASH_BLOCKQUOTE 1061
#define TAG_ARACHNE_BONUS  888

//AREA shapes:

#define DEFAULT 0
#define RECT    1
#define POLY    2
#define CIRCLE  3

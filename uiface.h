
//User interface parameters for Arachne WWW browser:

struct uiface
{
 char iconsoff;
 char hotkeys;
 char bigfont;
 char bigfnum;
 char bigstyle;
 unsigned char scrollbarsize;
 char scrollbarstyle; //A...Arachne, W...Windows, N...NextStep, X...xChaos
 char frames;
 char css; //enable cascading style sheets ?
 char logo;
 int ink;
 int paper;
 char smooth;
 int step;
 int virtualysize;
 char screenmode;
 char mailisdesktop;
 char esc;
 int logoiddle;
 unsigned char printerwidth;
 char fontshift;
// char buttons[9];
 int brightmouse;
 int darkmouse;
#ifndef NOPS
 int postscript_x;
 int postscript_y;
#endif
 unsigned char *keymap; //for mapping of 8-bit keyboard characters...
 char edithotlistentry;
 char autodial;
 char multitasking;
 int refresh;
 char killadds;
#if defined(MSDOS) && !defined(XTVERSION)
char vfat;
#endif
 long expire_static;
 long expire_dynamic;
 char quickanddirty;

 //optimizatons of used disk space and memory:
 long mindiskspace; //bytes
 char cache2temp; //1/0
 char cachefonts; //1/0
 int xms4allgifs; //KB
 long xms4onegif; //bytes
//!!glennmcc: Feb 13, 2006 -- at Ray's suggestion,
// changed variable name to match the keyword
 char keephtt;
// char nohtt;
//!!glennmcc: Nov 24, 2007 -- move both ignorejs & alwaysusecfgcolors
//from internet.h to uiface.h and changed both from http_parameters.
//into user_interface. in all places where they are used.
//!!glennmcc: begin May 03, 2002
// added to optionally "ignore" <script> tag
// (defaults to No if "IgnoreJS Yes" line is not in Arachne.cfg)
char ignorejs;
//!!glennmcc: end
//!!glennmcc: July 14, 2005
char alwaysusecfgcolors;
//!!glennmcc: end
};

extern struct uiface user_interface;

void configure_user_interface(void);

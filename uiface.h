
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
 char nohtt;
#ifdef GGI
int ggifastscroll;
#endif
};

extern struct uiface user_interface;

void configure_user_interface(void);


// ========================================================================
// Key codes returned by DOS/Borland C bioskey() and Linux port of bioskey() 
// (c)2000 Michael Polak, Arachne Labs
// ========================================================================



#ifdef GGI
 
#include <ggi/keyboard.h>

#define RIGHTSHIFT GII_MOD_SHIFT
#define LEFTSHIFT  GII_MOD_SHIFT
#define CTRLKEY    GII_MOD_CTRL
#define ALT        GII_MOD_ALT
#define SCROLL     GII_MOD_SCROLL

#define ALTARROW   ALT

#define BACKSPACE  GIIUC_BackSpace
#define PAGEUP     GIIK_PageUp
#define PAGEDOWN   GIIK_PageDown
#define CTRLHOME   0xff00
#define CTRLEND    0xff00
#define LEFTARROW  GIIK_Left
#define RIGHTARROW GIIK_Right
#define UPARROW    GIIK_Up
#define DOWNARROW  GIIK_Down
#define CTRLLEFT   0xff00
#define CTRLRIGHT  0xff00
#define INSERT     0xff00
#define DELETEKEY  GIIK_Delete
#define HOMEKEY    GIIK_Home
#define ENDKEY     GIIK_End
#define PRTSCR     GIIK_PrintScreen

#define F1 GIIK_F1
#define F2 GIIK_F2
#define F3 GIIK_F3
#define F4 GIIK_F4
#define F5 GIIK_F5
#define F6 GIIK_F6
#define F7 GIIK_F7
#define F8 GIIK_F8
#define F9 GIIK_F9
#define F10 GIIK_F10
#define F11 GIIK_F11
#define F12 GIIK_F12

#else //DOS or SVGAlib

#define RIGHTSHIFT 0x01
#define LEFTSHIFT  0x02
#define CTRLKEY    0x04
#define ALT        0x08
#define SCROLL     0x10

#define ALTARROW   CTRL

#ifdef LINUX
#define BACKSPACE 127     //real ASCII value, returned by console
#else
#define BACKSPACE 0xe08
#endif

#define PAGEUP     0x4900
#define PAGEDOWN   0x5100
#define CTRLHOME   0x7700
#define CTRLEND    0x7500
#define LEFTARROW  0x4b00
#define RIGHTARROW 0x4d00
#define UPARROW    0x4800
#define DOWNARROW  0x5000
#define CTRLLEFT   0x7300
#define CTRLRIGHT  0x7400
#define INSERT     0x5200
#define DELETEKEY  0x5300
#define HOMEKEY    0x4700
#define ENDKEY     0x4f00
#define PRTSCR     0x001c

#define F1 0x3b00
#define F2 0x3c00
#define F3 0x3d00
#define F4 0x3e00
#define F5 0x3f00
#define F6 0x4000
#define F7 0x4100
#define F8 0x4200
#define F9 0x4300
#define F10 0x4400
#define F11 0x4500
#define F12 0x4600

#endif


#define CURSOR_SYNCHRO 0x01
#define ZOOM_SYNCHRO   0x02

#define ASCIICTRLC 3
#define ASCIICTRLD 4
#define ASCIICTRLK 11
#define ASCIICTRLL 12
#define ASCIICTRLM 13
#define ASCIICTRLN 14
#define ASCIICTRLO 15
#define ASCIICTRLP 16
#define ASCIICTRLQ 17
#define ASCIICTRLR 18
#define ASCIICTRLS 19
#define ASCIICTRLT 20
#define ASCIICTRLU 21
#define ASCIICTRLV 22
#define ASCIICTRLW 23
#define ASCIICTRLX 24
#define ASCIICTRLY 25



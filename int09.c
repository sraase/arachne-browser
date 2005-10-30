// Testing ALT + TAB
#include <dos.h>
#include <stdio.h>
//!!JdS 2004/11/30 {
#include <process.h>
#include "main.h"
#include "glflag.h"

#define ESCSCAN 0x01
//!!JdS 2004/11/30 }
#define ALTSCAN 0x38
#define TABSCAN 0x0F
#define EXTSCAN 0xE0

// GLOBALS
unsigned char g_Scan=0,g_PrevScan=0;
char g_Alt =0;
unsigned int g_InDosSeg, g_InDosOfs;  //!!JdS 2004/11/30
int  g_AltTab=0;
int  g_PrtScr=0;

void interrupt (*OldKeyboard)(void);
void interrupt (*OldPrtScr)(void);

void InstalAltTab(void);
void ReleaseAltTab(void);
void InstalPrtScr(void);
void ReleasePrtScr(void);

// Internal ppg.
void STsetvect(int No, void interrupt (*fn) () )
{
  disable();
  poke(0, No*4+2, FP_SEG(fn));
  poke(0, No*4,   FP_OFF(fn));
  enable();
}

void interrupt (*STgetvect(int no))()
{
  static void far *Res;
  disable();
  Res = MK_FP(peek(0, (no<<2)+2), peek(0,no<<2));
  enable();
  return( Res );
}

void AcceptKey(void)
{ static char kbval;
  kbval = inportb(0x61);
  outportb(0x61,kbval|0x80);
  outportb(0x61,kbval);
  outportb(0x20,0x20);
  return;
}

void interrupt NewPrtScr()
{
 g_PrtScr=1;
}

// propousti vsechny stisky klavesnice, pouze testuje <ALT>+<TAB>
// Pozn: odkomentovanim AcceptKey(), lze nepropustit TAB po ALT
// tr.: releases/lets pass all keys, only tests <ALT>+<TAB>
//      note: by uncommenting AcceptKey(), you can hook TAB after ALT
void interrupt NewKeyboard()
{
  g_PrevScan = g_Scan;
  g_Scan = inportb(0x60);
  switch( g_Scan )
  { case ALTSCAN      : g_Alt = g_PrevScan==EXTSCAN ? 2 : 1;  // press ALT left|right
			break;
    case ALTSCAN|0x80 : g_Alt = 0;  // release ALT
			break;
    case TABSCAN      : if (g_Alt)   // press TAB
			{
			  g_AltTab = 1;   // combination <ALT>+<TAB>
			  //AcceptKey();  // polknu TAB ??
				      // tr.: do I swallow TAB ??
			  //return;       // nevolam puvodni ??
				      // tr.: don't I call the original one ??
			  ReleaseAltTab();
			}
			break;
//!!JdS 2004/11/30 {
    case ESCSCAN      : if ((peekb(0x40,0x17) & 0x0C) == 0x0C && // Ctl-Alt-Esc
			    peekb(g_InDosSeg,g_InDosOfs) == 0)  // Exit if DOS ready
			{
			  OldKeyboard();
			  ReleaseAltTab();  // just to be sure
                          exit(Terminate_Arachne(EXIT_ABNORMAL));
                        }
			break;
//!!JdS 2004/11/30 }
    default:;
  }
  //printf(">>> %02x\n",g_Scan);
  OldKeyboard();
}

// EXTERNAL PPG:
// Instalation :
void InstalAltTab()
{
//!!JdS 2004/11/30 {
  union REGS r;
  struct SREGS sr;
  // Get the address of the InDos semaphore
  r.h.ah = 0x34;
  intdosx(&r, &r, &sr);
  g_InDosOfs = r.x.bx;
  g_InDosSeg = sr.es;
//!!JdS 2004/11/30 }
  OldKeyboard = STgetvect(0x09);
  STsetvect(0x09, NewKeyboard);
}

// Deinstallation:
void ReleaseAltTab()
{
  STsetvect(0x09, OldKeyboard);
}

void InstalPrtScr()
{
  OldPrtScr = STgetvect(0x05);  //!!JdS 2004/11/30 (was 0x09!!!)
  STsetvect(0x05, NewPrtScr);
}

// Deinstallation:
void ReleasePrtScr()
{
  STsetvect(0x05, OldPrtScr);
}

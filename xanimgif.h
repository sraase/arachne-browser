// Ppp. + externy pro animaci GIfu.

// Podprogramy : Externi
int  XInitAnimGIF(int XmsKby);  // 1x na zacatku
void XCloseAnimGIF(void);       // 1x na konci
int  XResetAnimGif(void);       // pro novou stranku
int  XAnimateGifs(void);        // provadi animaci (cyklicky volat!)
long XReadTime( void );         // vraci cas v 1/100 sec (modulo 24h)
void XGifFreeXMS(void);
void XSetAllAnim1(void);        // Vsechny anim gify na prvni frame
void XSetAnim1(void);

// Podprogramy : Interni
int XGifFromXms(struct picinfo *gif, int ScrVirt,
		int vdx, int vdy, short int *tAnim);  //vykreleni Gifu z XMS
int XInitImgXms(struct picinfo *gif, int NumImg, int Transp, int TrCol, int tAnim, int disp);
int XSaveImgLine(char *Img1, int yBeg, int yscr); // uloz radku do XMS
int XSaveBackToXMS(struct picinfo *gif, int dxgif, int dygif, long *XmsAdr);
// Struktury
#define MAX_ANIMATEGIF     32
typedef struct
{
  unsigned int hPicInf;     // handle na PicInfo gifu
  unsigned int TimeAnim;    // interval animace (v ms ?)
  long         NextAnim;    // cas kdy spustit dalsi obrazek
} ENTRYGIF;

// Externy
extern long g_SizeAnimXMS;   // velikost pameti XMS pro animaci
extern long g_FreeAnim;
extern int  g_HandleXMS;     // handle XMS pro animaci
extern int  g_NumAnim;       // Pocet anim.gifu v tabulce
extern ENTRYGIF g_TableAnim[MAX_ANIMATEGIF];  // Tabulka animovanych gifu
extern long g_PrevImg;

#ifdef OVRL
#ifndef XTVERSION
#define XANIMGIF
#endif
#endif

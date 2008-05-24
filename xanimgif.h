// Ppp. + externy pro animaci GIfu.

// Podprogramy : Externi
int  XInitAnimGIF(int XmsKby);  // 1x na zacatku (tr.: at the beginning)
void XCloseAnimGIF(void);       // 1x na konci (tr.: at the end)
int  XResetAnimGif(void);       // pro novou stranku (tr.: for a new page)
int  XAnimateGifs(void);        // provadi animaci (cyklicky volat!)
                                // tr.: performs animation (call cyclically)
long XReadTime( void );         // vraci cas v 1/100 sec (modulo 24h)
                                // tr.: returns time 
void XGifFreeXMS(void);
void XSetAllAnim1(void);        // Vsechny anim gify na prvni frame
                                // tr.: all anim gifs in first frame
void XSetAnim1(void);

// Podprogramy : Interni (tr.: Subroutines: Internal)
int XGifFromXms(struct picinfo *gif, int ScrVirt,
		int vdx, int vdy, short int *tAnim);  //vykreleni Gifu z XMS
int XInitImgXms(struct picinfo *gif, int NumImg, int Transp, int TrCol, int tAnim, int disp);
int XSaveImgLine(char *Img1, int yBeg, int yscr); // uloz radku do XMS
int XSaveBackToXMS(struct picinfo *gif, int dxgif, int dygif, long *XmsAdr);
// Struktury (tr.: structures)
#define MAX_ANIMATEGIF     32
typedef struct
{
  unsigned int hPicInf;     // handle na PicInfo gifu
  unsigned int TimeAnim;    // interval animace (v ms ?)
  long         NextAnim;    // cas kdy spustit dalsi obrazek
} ENTRYGIF;

// Externy (External)
extern long g_SizeAnimXMS;   // velikost pameti XMS pro animaci
                             // tr.: size of memory XMS for animation
extern long g_FreeAnim;
extern int  g_HandleXMS;     // handle XMS pro animaci (tr.: for animation)
extern int  g_NumAnim;       // Pocet anim.gifu v tabulce
                             // tr.: number of anim.gifs in the table
extern ENTRYGIF g_TableAnim[MAX_ANIMATEGIF];  // Tabulka animovanych gifu
// tr.: table of animated gifs
extern long g_PrevImg;

#ifdef OVRL
#ifndef XTVERSION
#ifndef LINUX
#define XANIMGIF
#endif
#endif
#endif

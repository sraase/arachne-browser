// HARO - ppg. pro podporu animovaneho GIFu. Idea: Animovane gify
// se nactou do XMS pri volani DrawGIF() a zaroven se ulozi do tabulky
// animovanych GIfu.  V urcitych intervalech se bude muset volat fce
// XAnimateGifs(), ktera projde tabulku gifu a zjisti, ktery je jiz nacase
// animovat, Zavola XGifFromXms() a nastavi cas pristi animace a tak dokola.

#include "ima.h"
#include "arachne.h"
#include "v_putimg.h"

#ifdef XANIMGIF

#include "xanimgif.h"

int h_xmove(XMOVE *p);
int Xcurs_ingif(int xz, int yz, int xe, int ye, int xmouse, int ymouse);

// Globalni promenne pro animaci
long g_SizeAnimXMS = 0;   // velikost pameti XMS pro animaci
int  g_HandleXMS = -1;    // handle XMS pro animaci
int  g_NumAnim = 0;       // Pocet anim.gifu v tabulce
long g_FreeAnim = 0;      // Volne misto v XMS
long g_PrevImg = 0;

ENTRYGIF g_TableAnim[MAX_ANIMATEGIF];  // Tabulka animovanych gifu

extern int g_SaveGif;

// Inicializuje XMS (alokuje) pamet pro anim. gify. Vola se jednou
// na zacatku programu.
int XInitAnimGIF(int XmsKby)
{
   int ist;

   if(!(xg_256==MM_256 || xg_256==MM_Hic))  // pouze pro 256 a Hicol mody
   { g_HandleXMS = -1;
     return( -1 );
   }

   ist = get_xmem();
   if(ist >= 0x0200)                          // HIMEM.SYS O.K.
   {
     g_HandleXMS = alloc_xmem(XmsKby);
     if(g_HandleXMS == -1)
     { g_SaveGif = 0;
       return( -1);
     }
     else
     { g_SizeAnimXMS = 1024L*(long)XmsKby;
       g_NumAnim = 0;
       g_PrevImg = g_FreeAnim = 0L;
       g_SaveGif = 1;
     }
     return( 0 );
   }

   return( -1 );
}

// Uvolni XMS pro Gify, volat na konci programu
void XCloseAnimGIF(void)
{
  int ist;

  XResetAnimGif();
  if(g_HandleXMS != -1)
  { ist = dealloc_xmem( g_HandleXMS );
    if( ist !=1 ) {; }
    g_HandleXMS = -1;
  }
}

/*

moved to guitick.c for overlay optimization...

// Uvolni tabulku a XMS animovanych gifu. Asi volat pri initu HTML
// stranky
int XResetAnimGif(void)
{
   g_NumAnim = 0;
   g_PrevImg = g_FreeAnim = 0L;
   return( 0 );
}
*/

// uvolni posledni img z XMS
void XGifFreeXMS(void)
{ g_FreeAnim = g_PrevImg;
}

// hlavicka gifu(jednoho img) v XMS
typedef struct _XMSGIFH
   { long      Size;
     long      Next;
     short int Flags;     // bit 0: transparent
     short int TranspCol; // transp color (?)
     short int Dx;
     short int Dy;
     short int offx1;     // offsety uvnitr gifu
     short int offy1;
     short int tAnim;     // doba animace
     short int Rezer;
   } XMSGIFH;

// Init pred ulozenim jednoho img: call v drawGIF()
int XInitImgXms(struct picinfo *gif, int NumImg, int Transp, int TraCol, int tAnim, int disp)
{
   XMOVE  xmove;
   long   SizeXms, LenPix;
   int    ist,dygif,dxgif;
   XMSGIFH xhead;

   // Mam misto ?
   if(xg_256 == MM_Hic) LenPix = 2; else LenPix = 1;
   if(gif->resize_x==0 || gif->resize_y==0 /* || interlac */)
   { dygif = gif->size_y; dxgif = gif->size_x;
   }
   else
   { dygif = gif->resize_y; dxgif = gif->resize_x;
   }
   SizeXms = (long)sizeof(xhead) + (long)dxgif * (long)dygif * LenPix;
   if((g_FreeAnim + SizeXms) >= g_SizeAnimXMS)
   { gif->IsInXms = 0; //swapmod = 1;
     return( 2 );   // Neni misto ? prepnout ?
   }

   xhead.Size = SizeXms;
   xhead.Next = 0;
   xhead.Flags= Transp;   // 0/1
   xhead.TranspCol = TraCol;
   xhead.Dx = dxgif; xhead.Dy = dygif;  // dx + dy IMG
   xhead.offx1 = gif->offx1;
   xhead.offy1 = gif->offy1;
   xhead.tAnim = tAnim;
   xhead.Rezer = disp;
   xmove.sourceH   = 0;
   xmove.sourceOff = ptr2long((char *)&xhead);
   xmove.destH     = g_HandleXMS;
   xmove.destOff   = g_FreeAnim;
   xmove.length    = sizeof(xhead);
   ist = h_xmove(&xmove);
   if(!ist) goto End_wrt;

   if(NumImg > 0)    // Zretezit s minulym
   {
    xmove.sourceH   = g_HandleXMS;
    xmove.sourceOff = g_PrevImg;
    xmove.destH     = 0;
    xmove.destOff   = ptr2long((char *)&xhead);
    ist = h_xmove(&xmove);
    if(!ist) goto End_wrt;
    xhead.Next = g_FreeAnim;
    xmove.sourceH   = 0;
    xmove.sourceOff = ptr2long((char *)&xhead);
    xmove.destH     = g_HandleXMS;
    xmove.destOff   = g_PrevImg;
    ist = h_xmove(&xmove);
    if(!ist) goto End_wrt;
   }
   g_PrevImg = g_FreeAnim;
   g_FreeAnim += SizeXms;
   if(NumImg == 0)     // Ulozit zacatek prvniho
   { gif->IsInXms = 1;
     gif->NextImg = 0;
     gif->BegImg  = g_PrevImg;
   }
   gif->is_animation = NumImg;
   swapmod = 1;
   return( 1 );

   End_wrt:
   return( 2 );
}

// uloz radku img do XMS
int XSaveImgLine(char *Img1, int yBeg, int yscr)
{
  int *Img2, ist;
  long Adr, Len, ygif, LenPix;
  XMOVE  xmove;

  if(xg_256 == MM_Hic) LenPix = 2; else LenPix = 1;
  Img2 = (int*)Img1;
  ygif = yscr - yBeg;
  Len  = Img2[0] * LenPix;
  Adr  = (g_PrevImg+sizeof(XMSGIFH)) + ((long)ygif * Len);

  xmove.sourceH   = 0;
  xmove.sourceOff = ptr2long(&Img1[4]);
  xmove.destH     = g_HandleXMS;
  xmove.destOff   = Adr;
  xmove.length    = Len;
  ist = h_xmove(&xmove);
  if(!ist) goto End_wrt;
  return( 1 );

  End_wrt:
  return( 2 );
}


// vykreleni Gifu z XMS
int XGifFromXms(struct picinfo *gif, int ScrVirt,
            int vdx, int vdy, short int *tAnim)
{
// ScrVirt : kam se ma kreslit : 0-scr, 1-virt.scr
// vdx,vdy : posun mezi virtualni obrazovkou a kreslenym vyrezem

   long    LenPix,Adr1,DxXms;
   XMOVE   xmove,xmovebck;
   XMSGIFH xhead,xheadbck;
   int     i,j,ist,xz,yz,xe,oldScrVirt;
   int     x1gif, y1gif, dxgif,dygif;
   long    LenBuf;
   char   *Buf=NULL, *Bufx=NULL, TranspCol1;
   int    *Buf2,*Bufx2, TranspCol2;
   int     ire = 2, DrawBck=0, Vypnuty=0;
   struct HTMLframe *frame;

   oldScrVirt = xg_video_XMS;
   xg_video_XMS = ScrVirt;

   // precist hlavicku framu z XMS
   xmove.sourceH   = g_HandleXMS;
   xmove.destH     = 0;
   xmove.length    = sizeof(xhead);
   xmove.destOff   = ptr2long((char *)&xhead);

   if(gif->is_animation > 0) // vice nez jeden img
   { if(gif->NextImg == 0)
       xmove.sourceOff = gif->BegImg;
      else
       xmove.sourceOff = gif->NextImg;
   }
   else
   { xmove.sourceOff = gif->BegImg;
   }
   ist = h_xmove(&xmove);
   if(!ist) goto End_wrt;

   // Transp. frame a mam ulozene pozadi
   if(xhead.Flags && gif->BckImg > 0)
   {
     xmovebck.sourceH  = g_HandleXMS;
     xmovebck.destH    = 0;
     xmovebck.length   = sizeof(xheadbck);
     xmovebck.destOff  = ptr2long((char *)&xheadbck);
     xmovebck.sourceOff= gif->BckImg;
     ist = h_xmove(&xmovebck);
     if(!ist) goto End_wrt;
     // Zda vykrelit pozadi :
     // if(xhead.Dx==xheadbck.Dx && xhead.Dy==xheadbck.Dy) //stejne -> kreslit
     if(xhead.Rezer >= 2)
     { DrawBck = 1;
     }
   }

   // spocitat si aktualni pix_x,pic_y,stop_x,stop_y,from_x,from_y
   frame=&htmlframe[gif->picinfo_frameID];

   gif->from_x = gif->pic_x = frame->scroll.xtop+gif->html_x-frame->posX;
   gif->from_y = gif->pic_y = (int)(frame->scroll.ytop+gif->html_y-frame->posY);
   if(gif->pic_x<frame->scroll.xtop)
    gif->pic_x = frame->scroll.xtop+1;
   if(gif->pic_y<frame->scroll.ytop)
    gif->pic_y = frame->scroll.ytop+1;

   gif->stop_x = frame->scroll.xtop+frame->scroll.xsize;
   gif->stop_y = frame->scroll.ytop+frame->scroll.ysize;

   // odkud na obrazovce
   *tAnim = xhead.tAnim;
   xz = gif->pic_x + xhead.offx1 + vdx;
   yz = gif->pic_y + xhead.offy1 + vdy;
   // odkud v gifu (oriznuti zleva, zprava atd...)
   x1gif=gif->pic_x - gif->from_x;   // oriznuti z leva ?
   y1gif=gif->pic_y - gif->from_y;

   if((gif->pic_x + xhead.Dx + xhead.offx1 - x1gif - 1) <=gif->stop_x)
    dxgif = xhead.Dx - x1gif;
   else
    dxgif = gif->stop_x - gif->pic_x - xhead.offx1 + 1;

   if((gif->pic_y + xhead.Dy + xhead.offy1 - y1gif - 1) <=gif->stop_y)
    dygif =  xhead.Dy - y1gif;
   else
    dygif = gif->stop_y - gif->pic_y - xhead.offy1 + 1;
   xe = xz + dxgif - 1;

   if(dxgif <= 0) goto End_wrt;

   // vykreslit img z xms do video 1:1
   if(xg_256 == MM_Hic) LenPix = 2; else LenPix = 1;
   LenBuf = xhead.Dx*LenPix+4;
   Buf = farmalloc(LenBuf);
   if(Buf == NULL) goto End_wrt;
   Buf2 = (int*)Buf;
   if(xhead.Flags != 0)
   {
     TranspCol1 = (char)xhead.TranspCol;
     TranspCol2 = xhead.TranspCol;
   }
   Bufx = farmalloc(LenBuf);
   if(Buf == NULL) goto End_wrt;
   Bufx2 = (int*)Bufx;

   DxXms=LenPix*xhead.Dx;   // delka radku v B v Xms
   xmove.length    = dxgif*LenPix;
   xmove.destOff   = ptr2long(Bufx+4);
   Adr1 = xmove.sourceOff + sizeof(XMSGIFH); // zacatek dat img
   Adr1 = Adr1 + (long)y1gif*DxXms + (long)LenPix*x1gif;
   xmove.sourceOff = Adr1;
   Bufx2[0] = dxgif; Bufx2[1] = 1;

   // Nakreslit pozadi (je stejne velke jako frame)
   if(DrawBck)
   {
    xmovebck.length    = dxgif*LenPix;
    xmovebck.destOff   = ptr2long(Buf+4);
    Adr1 = gif->BckImg + sizeof(XMSGIFH); // zacatek dat img
    Adr1 = Adr1 + (long)y1gif*DxXms + (long)LenPix*x1gif;
    xmovebck.sourceOff = Adr1;
    Buf2[0] = dxgif; Buf2[1] = 1;
   }

   // Disable cursor
   // mp: not for virtual screens - it's already disabled!
   if(!xg_video_XMS && Xcurs_ingif(xz, yz, xe, yz+dygif-1, mousex, mousey))
   { Vypnuty = 1;
     mouseoff();
   }

   // Nakreslit frame gifu
   // cykl pres radky gifu v XMS
   for(i = 0; i<dygif; i++)
   {
     ist = h_xmove(&xmove);
     if(!ist) goto End_wrt;

     if(xhead.Flags != 0) // transparentni
     {
       if(DrawBck == 0)
       { v_getimg(xz, yz+i, xe, yz+i,Buf);
       }
       else
       { ist = h_xmove(&xmovebck);
           if(!ist) goto End_wrt;
       }
#ifdef HICOLOR
       if(xg_256 == MM_Hic)
       {
      //for(j=2; j<2+dxgif-1; j++)
      for(j=2; j<2+dxgif; j++)
      { if(Bufx2[j] != TranspCol2) Buf2[j] = Bufx2[j];
      }
       }
       else
       {
#endif
      //for(j=4; j<4+dxgif-1; j++)
      for(j=4; j<4+dxgif; j++)
      { if(Bufx[j] != TranspCol1) Buf[j] = Bufx[j];
      }
#ifdef HICOLOR
       }
#endif
       v_putimg(xz, yz+i, Buf);
     }
     else
     { v_putimg(xz, yz+i, Bufx);
     }
     xmove.sourceOff += DxXms;
     if(DrawBck) xmovebck.sourceOff += DxXms;
   } // end for i..
   ire = 1;

   End_wrt:       // sem hop pri chybach
   if(Buf) farfree(Buf);
   if(Bufx) farfree(Bufx);

   if( Vypnuty )
   { mouseon();
   }

   // pripravit dalsi img pro pristi call
   if(gif->is_animation > 0)
   { if(xhead.Next == 0L)
       gif->NextImg = 0;   // nastaveni na zacatek
     else
       gif->NextImg = xhead.Next;  // next img
      // *ModPic = 1; !!! pres swapmod
     swapmod = 1;
   }

   xg_video_XMS = oldScrVirt;
   return( ire );
}


// Cvicny ppg. pro animaci Gifu
int XAnimateGifs(void)
{
    int  i,hpic,Num;
    long NextTime,AktTime;
    short int AnimInt;
    struct picinfo *pPicInf;
    int ire = 0;

    if(g_NumAnim > 0)  // Je co animovat
    {
      AktTime = XReadTime();        // Akt. cas v 1/100s (0..23.59 hodin)
      Num     = 0;

      for(i=0; i<g_NumAnim; i++)
      { hpic = g_TableAnim[i].hPicInf;     // handle picinfo
        if(hpic == IE_NULL) continue;

          NextTime = g_TableAnim[i].NextAnim;// kdy se ma animovat
          AnimInt  = g_TableAnim[i].TimeAnim;// amim interval

             if(AktTime > NextTime)     // Je cas animace
             {
              //pPicInf = &g_pcinf[hpic];          // z handlu adresu
              //outs("Anim X");
              pPicInf = (struct picinfo *) ie_getswap(hpic);
              if(!pPicInf)                       // pokud se nepovede, tak "Fatal Error"
               MALLOCERR();

            //mouseoff();
            XGifFromXms( pPicInf, 0, 0, 0,  &AnimInt);  // vzdy do scr
            //mouseon();

              g_TableAnim[i].NextAnim = XReadTime() + AnimInt;  // od skonceni kresleni
              Num++;
             }

      } //end for i..
      ire = Num;
    }
    return( ire );
}

//mp:XsetAnim1 moved to animstat.c

// Ulozi podklad do XMS: jen pro anim -> zadny scale !!
int XSaveBackToXMS(struct picinfo *gif, int dxgif, int dygif, long *XmsAdr)
{
   XMOVE   xmove;
   long    SizeXms, LenPix, Adr, Len;
   int     ist,x1,y1,i;
   XMSGIFH xhead;
   char   *Img1 = NULL;

   *XmsAdr = 0;
   // Mam misto ?
   if(xg_256 == MM_Hic) LenPix = 2; else LenPix = 1;

   SizeXms = (long)sizeof(xhead) + (long)dxgif * (long)dygif * LenPix;
   if((g_FreeAnim + SizeXms) >= g_SizeAnimXMS)
   { return( 0 );   // Neni misto ?
   }

   xhead.Size = SizeXms;
   xhead.Next = 0;
   xhead.Flags= 0;   // 0/1
   xhead.TranspCol = 0;
   xhead.Dx = dxgif; xhead.Dy = dygif;  // dx + dy IMG
   xhead.offx1 = 0;
   xhead.offy1 = 0;
   xhead.tAnim = 0;
   xhead.Rezer = 0;
   xmove.sourceH   = 0;
   xmove.sourceOff = ptr2long((char *)&xhead);
   xmove.destH     = g_HandleXMS;
   xmove.destOff   = g_FreeAnim;
   xmove.length    = sizeof(xhead);
   ist = h_xmove(&xmove);
   if(!ist) goto End_wrt;

   *XmsAdr   = g_FreeAnim;
   g_PrevImg = g_FreeAnim;
   g_FreeAnim += SizeXms;

   x1 = gif->pic_x;
   y1 = gif->pic_y;
   Len  = dxgif * LenPix;
   Img1 = farmalloc(Len + 8);
   if(Img1 == NULL) goto End_wrt;

   xmove.sourceH   = 0;
   xmove.sourceOff = ptr2long(&Img1[4]);
   xmove.destH     = g_HandleXMS;
   xmove.length    = Len;

   for(i=0; i<dygif; i++)
   {
     v_getimg(x1, y1+i, x1+dxgif-1, y1+i, Img1);

     Adr  = (g_PrevImg+sizeof(XMSGIFH)) + ((long)i * Len);
     xmove.destOff   = Adr;
     ist = h_xmove(&xmove);
     if(!ist) goto End_wrt;
   }
   farfree(Img1);
   return( 1 );

   End_wrt:
   if(Img1) farfree(Img1);
   return( 0 );
}

// Vraci cas v 1/100 vteriny
long XReadTime( void )
{
   union REGS r;
   long  setiny;

   r.h.ah = 0x2c;
   int86( 0x21, &r, &r);
   setiny =  (long) r.h.dl;          // dl : hundredths
   setiny += (long) r.h.dh * 100;    // dh : sec
   setiny += (long) r.h.cl * 6000L;  // cl : min
   setiny += (long) r.h.ch * 360000L;// ch : hod
   return ( setiny );
}

// Zda cursor zasahuje do prave kresleneho gifu
int Xcurs_ingif(int xz, int yz, int xe, int ye, int xmouse, int ymouse)
{
// xz,yz,xe,ye - obdelnik kresleneho gifu
// xmouse, ymouse - pozice mysi
// return : 0-kursor je mimo, 1-kursor je v gifu
// kursor je velikosti 16 x 16 pixlu napevno: => prunik dvou rect

   int xme, yme, ire = 0;
   xme = xmouse + 16 - 1;
   yme = ymouse + 16 - 1;

   if(xe < xmouse || xz > xme) goto Mimo;
   if(ye < ymouse || yz > yme) goto Mimo;
   ire = 1;
   Mimo:
   return( ire );
}

#endif // XANIMGIF

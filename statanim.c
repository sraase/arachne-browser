
//Staticaly linked portions of GIF animation

#include "arachne.h"
#include "xanimgif.h"

// Uvolni tabulku a XMS animovanych gifu. Asi volat pri initu HTML
// stranky
// tr.: Releases table and XMS of animated GIFs.
//      Probably to be called when initializing HTML page
int XResetAnimGif(void)
{
   g_NumAnim = 0;
   g_PrevImg = g_FreeAnim = 0L;
   return( 0 );
}

// Nastavi vsechny anim. gify na prvni obrazek
// tr.: sets all animated gifs to first picture
//mp!!:zmeneno (tr.: mp!!:changed)
void XSetAnim1(void)
{
 int    i,hpic;
 struct picinfo *pPicInf;

 for(i=0; i<g_NumAnim; i++)
 {
  hpic = g_TableAnim[i].hPicInf;     // handle picinfo
  if(hpic == IE_NULL)
   continue;

  pPicInf = (struct picinfo *)ie_getswap(hpic);
  if(!pPicInf)                       // if it fails, then "Fatal Error"
   MALLOCERR();

  //po volani "dumpvirtual" zarucene vykreslen frejm 0...
  // tr.: after calling "dumpvirtual", frame 0 is guaranteed to be drawn...
  pPicInf->NextImg = 0;   // nastaveni na zacatek
                 // tr.: set to the beginning
  g_TableAnim[i].NextAnim= 0l;   //mp!!: pri nejblizsi prilezitosti prekreslit
                // tr.: mp!!: redraw at next occasion
  swapmod=1;
 } //end for i..
}
//mp!!:end

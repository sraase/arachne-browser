
//Staticaly linked portions of GIF animation

#include "arachne.h"
#include "xanimgif.h"

// Uvolni tabulku a XMS animovanych gifu. Asi volat pri initu HTML
// stranky
int XResetAnimGif(void)
{
   g_NumAnim = 0;
   g_PrevImg = g_FreeAnim = 0L;
   return( 0 );
}

// Nastavi vsechny anim. gify na prvni obrazek
//mp!!:zmeneno
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
  if(!pPicInf)                       // pokud se nepovede, tak "Fatal Error"
   MALLOCERR();

  //po volani "dumpvirtual" zarucene vykreslen frejm 0...
  pPicInf->NextImg = 0;   // nastaveni na zacatek
  g_TableAnim[i].NextAnim= 0l;   //mp!!: pri nejblizsi prilezitosti prekreslit
  swapmod=1;
 } //end for i..
}
//mp!!:end

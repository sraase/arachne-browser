#include "x_lopif.h"

/*--------------------------------------------*/
/* Nastavi zpusob zarovnani textu             */
/* Pouze ve smeru X (levy,stred,pravy)        */
/*--------------------------------------------*/

void x_settextjusty(int horiz, int vert)
{
  xg_tjustx = horiz;
  xg_tjusty = vert;
}
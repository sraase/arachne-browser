#include   <math.h>
#include   <stdlib.h>
#include   "x_lopif.h"

/**********************************************************************/
/*            Funkce pro kresleni polygonu (nevyplneneho)             */
/**********************************************************************/
/*          nump   ... pocet vrcholu                                  */
/*		       ( u uzavreneho se posledni musi opakovat)      */
/*          pole   ... pole vrcholu                                   */

void  x_poly(int nump, int *pole )
{
  int i;

  for ( i = 2; i < nump*2; i += 2 )
     x_line(pole[i-2], pole[i-1], pole[i], pole[i+1] );
}

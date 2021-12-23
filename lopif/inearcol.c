/*********************************************************************/
/*                            INearCol                               */
/*  Hledani nejblizsi barvy k pozadovane barve v dane palete         */
/*********************************************************************/
/*Parametry:
*pal   ... paleta = npal * 3B
 npal  ... delka palety pal
 (colr,colg,colb) ... pozadovana barva = 3B = "RGB"

!! <-- vsude se bere v uvahu jen dolnich 6 bitu! (na hornich 2 nezalezi)

Navratova hodnota INearCol = index barvy z pal, ktera je nejblizsi
				 k pozadovane
*/

#include <math.h>

int INearCol( char* pal, int npal, char colr, char colg, char colb)

{
  int ipal, j, imin;
  char r, g, b, kr, kg, kb, kmax, kmaxmin ;
  unsigned char ksum, ksummin ;
  #define MASKA6  '\077'


  r = colr & MASKA6;
  g = colg & MASKA6;
  b = colb & MASKA6;

  j = 0;
  kmaxmin = 100;
  ksummin = 200;   //? unsigned? !
  for( ipal = 0; ipal < npal; ipal++ )
  {
    kr = r - (pal[j++] & MASKA6);
    kg = g - (pal[j++] & MASKA6);
    kb = b - (pal[j++] & MASKA6);
    if (kr  < 0) kr = -kr;
    if (kg  < 0) kg = -kg;
    if (kb  < 0) kb = -kb;
/*   KMAX= MAX( KR2, KG2, KB2 )	! 1.metrika */
    kmax = kr > kg ? kr : kg;
    kmax = kmax > kb ? kmax : kb;
    if( kmax == 0) {imin = ipal; break;}
    if( kmax > kmaxmin ) continue;
    ksum = kr + kg + kb;
    if( kmax == kmaxmin )
    {
      if( ksum >= ksummin ) continue;
    }
    imin = ipal; kmaxmin = kmax; ksummin=ksum;
  }
return imin;
}

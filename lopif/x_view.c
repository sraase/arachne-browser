#include   "x_lopif.h"

/**********************************************************************/
/*                  Funkce nastavujici viewport                       */
/**********************************************************************/

void  x_setview(int x1, int y1, int x2, int y2, int clip)
{
  xg_view[0] = x1;
  xg_view[1] = y1;
  xg_view[2] = x2;
  xg_view[3] = y2;
  xg_clip    = clip;
}

/**********************************************************************/
/*                  Funkce nastavujici typ cary                       */
/**********************************************************************/

void x_setlinstyle(int typ, int user, int width)
{
  if(typ != 0)
     xg_style = user;
  else
     xg_style = 0xFFFF;
}

/**********************************************************************/
/*                  Funkce vracejici aspectratio                      */
/**********************************************************************/

void  x_getratio( int *xasp, int *yasp )
{
    *xasp = xg_xr;
    *yasp = xg_yr;
}


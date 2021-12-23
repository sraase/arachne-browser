#include <stdlib.h>
#include <alloc.h>
#include "x_lopif.h"

//------ Kresleni velkeho kursoru (kriz) --------

int xg_cr_dx;    // Velikost krize  v X
int xg_cr_dy;    // Velikost krize  v Y
int xg_typcur=0; // Typ kursoru 0-stary, 1-kriz dx,dy, 2-pres xg_view
unsigned char *xg_x1x2=NULL;   // Buffer pro X
unsigned char *xg_y1y2=NULL;   // Buffer pro Y (16bar. mody ??)


//------ Nakresli kursor na pozici x,y ----------
void x_cross_cur(int x, int y)
{
  int x1,y1,x2,y2;

  if(xg_f_cur <= 0) goto End_c;                  /* Vypnuty cursor */
  if(x == xg_x_cur && y == xg_y_cur) goto End_c; /* Bez pohybu     */

  xg_notview = 1;

  if(xg_typcur == 2)   // Pres cely vieport
   {
    x_putimg(xg_view[0],xg_y_cur,xg_x1x2,0);       // OBNOVENI
    x_putimg(xg_x_cur,xg_view[1],xg_y1y2,0);

    x_getimg(xg_view[0],y,xg_view[2],y,xg_x1x2);   // Uschova noveho mista
    x_getimg(x,xg_view[1],x,xg_view[3],xg_y1y2);
    x_setfill(1,xg_c_col);
    x_bar(xg_view[0],y,xg_view[2],y);              // Zobrazeni noveho mista
    x_bar(x,xg_view[1],x,xg_view[3]);
   }
   else                // kriz o zadane velikosti
   {
   x1 =  xg_x_cur - xg_cr_dx/2;  x1 = max(x1, xg_view[0]);  // OBNOVENI
   y1 =  xg_y_cur - xg_cr_dy/2;  y1 = max(y1, xg_view[1]);
    x_putimg(x1,xg_y_cur,xg_x1x2,0);
    x_putimg(xg_x_cur,y1,xg_y1y2,0);

   x1 =  x - xg_cr_dx/2;  x1 = max(x1, xg_view[0]); // Uschova noveho mista
   x2 =  x + xg_cr_dx/2;  x2 = min(x2, xg_view[2]);
   y1 =  y - xg_cr_dy/2;  y1 = max(y1, xg_view[1]);
   y2 =  y + xg_cr_dy/2;  y2 = min(y2, xg_view[3]);
    x_getimg(x1,y,x2,y,xg_x1x2);   // Uschova obrazu
    x_getimg(x,y1,x,y2,xg_y1y2);
    x_setfill(1,xg_c_col);
    x_bar(x1,y,x2,y);                                // Zobrazeni noveho mista
    x_bar(x,y1,x,y2);
   }

  xg_notview = 0;
  xg_x_cur = x;
  xg_y_cur = y;
  End_c:
}

//------ Vypne/Zapne kursor a nastavi jeho barvu ---
void x_cross_on(int on, int x, int y, int col)
{
  int x1,x2,y1,y2;

  if(on != 0)       /* ----------- Zapnuti */
  {
  if(xg_f_cur > 0) return;

  xg_f_cur = 1;
#if HI_COLOR
  if(xg_256 == MM_Hic)
  { if(xg_hipalmod == 0)
      xg_c_col = xg_hival[col];
    else
      xg_c_col = col;
  }
  else
  {  xg_c_col = col;
  }
#else
  xg_c_col = col;
#endif
  xg_x_cur = x;
  xg_y_cur = y;
  xg_notview = 1;

  if(xg_typcur == 2)   // Pres cely vieport
   {
   x_getimg(xg_view[0],y,xg_view[2],y,xg_x1x2);   // Uschova obrazu
   x_getimg(x,xg_view[1],x,xg_view[3],xg_y1y2);
   x_setfill(1,xg_c_col);
   x_bar(xg_view[0],y,xg_view[2],y);                // Zobrazeni kursoru */
   x_bar(x,xg_view[1],x,xg_view[3]);
   }
   else                // kriz o zadane velikosti
   {
   x1 =  x - xg_cr_dx/2;  x1 = max(x1, xg_view[0]);
   x2 =  x + xg_cr_dx/2;  x2 = min(x2, xg_view[2]);
   y1 =  y - xg_cr_dy/2;  y1 = max(y1, xg_view[1]);
   y2 =  y + xg_cr_dy/2;  y2 = min(y2, xg_view[3]);
   x_getimg(x1,y,x2,y,xg_x1x2);   // Uschova obrazu
   x_getimg(x,y1,x,y2,xg_y1y2);
   x_setfill(1,xg_c_col);
   x_bar(x1,y,x2,y);                // Zobrazeni kursoru */
   x_bar(x,y1,x,y2);
   }
  }
  else              /*--------------- Vypnuti */
  {
  xg_f_cur -= 1;
  if(xg_f_cur == 0)
     {
     if(xg_typcur == 2)   // Pres cely vieport
      {
      x_putimg(xg_view[0],xg_y_cur,xg_x1x2,0);       // OBNOVENI
      x_putimg(xg_x_cur,xg_view[1],xg_y1y2,0);
      }
     else
      {
      x1 =  xg_x_cur - xg_cr_dx/2;  x1 = max(x1, xg_view[0]);
      y1 =  xg_y_cur - xg_cr_dy/2;  y1 = max(y1, xg_view[1]);
      x_putimg(x1,xg_y_cur,xg_x1x2,0);
      x_putimg(xg_x_cur,y1,xg_y1y2,0);
      }
     }
  else
     xg_f_cur = -2;
  }
  xg_notview = 0;

}

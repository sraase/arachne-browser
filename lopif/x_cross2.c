#include <stdlib.h>
#include <alloc.h>
#include "x_lopif.h"

extern int xg_cr_dx;      // Velikost krize  v X
extern int xg_cr_dy;      // Velikost krize  v Y
extern int xg_typcur;     // Typ kursoru 0-stary, 1-kriz dx,dy, 2-pres xg_view
extern unsigned char *xg_x1x2;   // Buffer pro X
extern unsigned char *xg_y1y2;   // Buffer pro Y (16bar. mody ??)

#if HI_COLOR
#define  MAX_BUF_XX  2060
#define  MAX_BUF_YY  1600
#else
#define  MAX_BUF_XX  1030
#define  MAX_BUF_YY   800
#endif

//------ Nadefinovani krize ---------------------
void x_defcross(int dx, int dy, int typ)
{
   if(typ != 0)              // ZAPNI -> ALOKOVAT PRO KRIZOVY KURSOR
    { if(xg_x1x2 == NULL)
       { xg_x1x2 = farmalloc((long)MAX_BUF_XX);
	 if(xg_x1x2 == NULL) goto No_cross;
       }
      if(xg_y1y2 == NULL)
       { xg_y1y2 = farmalloc((long)MAX_BUF_YY);
	 if(xg_y1y2 == NULL)
	   { No_cross:
	     if(xg_x1x2 != NULL) { farfree(xg_x1x2); xg_x1x2 = NULL; }
	     xg_typcur = 0;      // Neni misto pro cros_cur
	     return;
	   }
       }
    }
   else                      // VYPNI -> DEALOKOVAT
    { if(xg_x1x2 != NULL) { farfree(xg_x1x2); xg_x1x2 = NULL; }
      if(xg_y1y2 != NULL) { farfree(xg_y1y2); xg_y1y2 = NULL; }
    }

   if(xg_256 == MM_16 || xg_256 == MM_2) // v 16 modech pouze typ 1 max 128 x 128
    { if(typ > 1) typ = 1;
      if(dx > 128) dx = 128;
      if(dy > 128) dy = 128;
    }
   else
    { if(typ > 2) typ = 2;
      if(dx > x_maxx()) dx = x_maxx();
      if(dy > x_maxy()) dy = x_maxy();
    }
   xg_cr_dx = dx;
   xg_cr_dy = dy;
   xg_typcur = typ;
}

#include <string.h>
#include <dos.h>

#include "vesa.h"

char *x_strcmp(char *buf, int lenb, char *str, int lens);

//----------------- Detekce SVGA karty ---------------------
// Pozn: Detekuje pouze v ramci predem uvedeneho seznamu

int x_detect(char *svga, int *kby)
//  svga - retezec dle IBASE (vystup)
//  kby  - pamet SVGA v kB   (vystup) ???
//  ret  - 0 - zadna znama nebyla detekovana
//         1 - byla detekovana
{
   char kopie[1024],*ix,stat[64];
   char *bios;
   int  i,lens;
   struct REGPACK reg;
   struct VESA_1 a_vesa;

// Stringy pro IBASI
char  *xg_ibname[] = {"TSG3",          // TSENG 3000
                      "TSG4",          // TSENG 4000
                      "OAK",           // OAK
                      "TRIDENT",       // TRIDENT
                      "TRIDENT",       // VC510 (Ludva,jako TRIDENT)
                      "M1",            // Haro OCTEK,chip MX86010
                      "TAMARA",        // AVGA2 (nefunguje mod D)
                      "REALTEK",       // Nemam BIOS !!! ???
                      "TAMARA",        // Nemam BIOS !!! ???
                      "VESA"           // VESA standart  (Nemam BIOS);
                     };
// Stringy v BIOSu
char  *xg_hwname[] = {"TSENG LABS VGA BIOS    MODEL",
                      "TsengLabs Int'l ET4000",
                      "OAK VGA BIOS",
                      "TRIDENT MICRO",
                      "Quadtel Corp.",
                      "MXIC",
                      "Acumos AVGA2",
                      "REALTEK",       // ??
                      "TAMARACK",      // ??
                      "VESA"           // ??
                     };
int    xg_hwnnn = 10;            // Pocet detekovatelnych SVGA


   //------ search string in bios from C000 ----
   bios = MK_FP(0xC000,0);
   memcpy(kopie,bios,1024);

   for(i=0; i<xg_hwnnn; i++)
    { lens = strlen(xg_hwname[i]);
      ix = x_strcmp(kopie, 1024, xg_hwname[i], lens);
      if(ix != NULL) goto Mame_ji;
    }

   //------- VESA ---------
   reg.r_ax = 0x4F00;
   reg.r_di = FP_OFF(&a_vesa);
   reg.r_es = FP_SEG(&a_vesa);
   intr(0x10, &reg);
   if(reg.r_ax == 0x004F)      // Vesa O.K.
   { strcpy(svga,"VESA");
     *kby = a_vesa.mem64*64;
     return( 1 );
   }

   return( 0 );      // Nenalezena

   Mame_ji:
   reg.r_ax = 0x1B00;          // Get. info o VGA - pamet
   reg.r_bx = 0x0000;
   reg.r_di = FP_OFF(stat);
   reg.r_es = FP_SEG(stat);
   intr(0x10, &reg);
   *kby = (stat[0x31]+1) * 64; // Pozn: Vraci jen 64..256kB
   //*kby = 512;                 // Natvrdo 512kB ?
   strcpy(svga,xg_ibname[i]);
   return( 1 );
}
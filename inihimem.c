/***********************************************************************/
/*                               IniHimem                              */
/***********************************************************************/
// Inicializace himem.sys (Priprava getIm/putIm)

//(c) pola ivan polak, 1992

#include <ima.h>
//#include <getmess.h>
//#include <ccesky.mdc>

// ================================ Prototypy =============================
// ================================ Globalni promenne =============================
char memMore=-1;              /* promenna pro typ pameti   */
//--------------------------------------------------------------------------
void IniHimem (void)
{
/************************ deklarace ***********************/
int stav;
//====================================== main ==============================
   if(memMore < 0)
   {
     stav = get_xmem();
       /* pokud je driver pritomen (stav != -1) a je verze >= 2 */
       /* (stav >= 0x0200), tak to dam vedet pomoci memMore=1   */
       /* jinak ponecham memMore=0                              */
     if (stav >= 0x0200 && stav != -1) memMore=1;
     else memMore=0;
   }
  return;
}

/***********************************************************************/
/*                               IniHimem                              */
/***********************************************************************/
// Initialization himem.sys (preparing getIm/putIm)

//(c) pola ivan polak, 1992

#include <ima.h>
//#include <getmess.h>
//#include <ccesky.mdc>

// ================================ Prototypes ============================
// ================================ Global variables ======================
char memMore=-1;              /* variable for type memory   */
//--------------------------------------------------------------------------
void IniHimem (void)
{
/************************ declaration ***********************/
int stav;
//====================================== main ==============================
   if(memMore < 0)
   {
     stav = get_xmem();
       /* pokud je driver pritomen (stav != -1) a je verze >= 2 */
       /* (stav >= 0x0200), tak to dam vedet pomoci memMore=1   */
       /* jinak ponecham memMore=0                              */
       /* tr.: if the driver is present (status != -1) and is version >=2  */
       /*      (status >= 0x0200), then I report that through memMore=1,   */
       /*      otherwise I leave memMore=0                                 */
     if (stav >= 0x0200 && stav != -1) memMore=1;
     else memMore=0;
   }
  return;
}

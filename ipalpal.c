/********************************************************************/
/*                      palpalp                                     */
/*  transforms (repalettizes) palette palin into palette palout     */
/*  palpalp ... shortened version of original pappal                */
/********************************************************************/
/*
Parametry:
*palin  - vstupni paleta
 npalin - delka *palin
*palout - "vystupni" paleta (pro palpal take pouze vstupni parametr)
 npalout- delka *palout
*mapio  - prevodni mapa - vystupni parametr
	  ==> j = mapio[i] ... i= vstupni barva, j = vystupni barva

tr.: Parameters:
     *palin  - input palette
     npalin - length of *palin
     *palout - "output" palette (for palpal also only an input parametr)
     npalout - length of *palout
     *mapio  - conversion map - output parameter
          ==> j = mapio[i] ... i= inpput colour, j = output colour

(c) ivan polak, 1990,1997

*/

#include "posix.h"

// ===== prototypes ======================================
void Ipalpal (char *palin, int npalin, char  *palout, int npalout,
	       int *mapio )
{
//  long isttf[256];   // statistics on transformation
  int iin,jin,iout,jout, ir,ig,ib, or,og,ob,
      jmin, kmaxmin, ksummin, kmax, ksum, kr2,kg2,kb2;
//=====main==============================================

//  for(iin=0; iin < 256; iin++) isttf[iin] = 0;  //(statistics)

//p:case 'R': //======================= method "RGB" >>>

  for(jin=0, iin=0; jin < npalin; jin++)  // loop through input palette >>>
  {
    ir= palin[iin++];
    ig= palin[iin++];
    ib= palin[iin++];
// ===================== look for the nearest colour in output palette >>>>
    kmaxmin=32000;
    ksummin=32000;
    for(jout=0, iout=0; jout < npalout; jout++)  // loop through output pal.
    {
      or= palout[iout++];
      og= palout[iout++];
      ob= palout[iout++];

      kr2= abs(ir - or);  // diference slozek (tr.: difference of elements)
      if(kr2 > kmaxmin) continue;
      kg2= abs(ig - og);
      if(kg2 > kmaxmin) continue;
      kb2= abs(ib - ob);
      if(kb2 > kmaxmin) continue;

      kmax= max( kr2, max(kg2, kb2) );  // 1.metrika (tr.: 1. metrics)
      if(kmax == 0)
	{jmin=jout; goto Hotovo;}
//p:      if(kmax > kmaxmin) continue;
      if(kmax == kmaxmin)
      {
        ksum= kr2 + kg2 + kb2;  // 2.metrika (tr.: 2. metrics)
	if(ksum >= ksummin) continue;
	jmin=jout;
	kmaxmin=kmax;
	ksummin= ksum;
      }
      else
      {
	jmin=jout;
	kmaxmin=kmax;
        ksummin= kr2 + kg2 + kb2;   // 2.metrika (tr.: 2. metrics)
      }
    } //end of loop through output palette
Hotovo:
  mapio[jin]= jmin;
//  isttf[jmin]++;     // statistics
//        isttf(kmin)= isttf(kmin) +istat(ipll) !  !statistika
  } //end of loop through input palette

  return;
}

/********************************************************************/
/*                      palpalp                                     */
/*  transformace (repaletizace) palety palin do palety palout       */
/*  palpalp ... zkracena verze puvodniho pappal                     */
/********************************************************************/
/*
Parametry:
*palin  - vstupni paleta
 npalin - delka *palin
*palout - "vystupni" paleta (pro palpal take pouze vstupni parametr)
 npalout- delka *palout
*mapio  - prevodni mapa - vystupni parametr
	  ==> j = mapio[i] ... i= vstupni barva, j = vystupni barva
(c) ivan polak, 1990,1997
*/

#include "posix.h"

// ===== prototypy ======================================
void Ipalpal (char *palin, int npalin, char  *palout, int npalout,
	       int *mapio )
{
//  long isttf[256];   // statistika o transformaci
  int iin,jin,iout,jout, ir,ig,ib, or,og,ob,
      jmin, kmaxmin, ksummin, kmax, ksum, kr2,kg2,kb2;
//=====main==============================================

//  for(iin=0; iin < 256; iin++) isttf[iin] = 0;  //(statistika)

//p:case 'R': //======================= metoda "RGB" >>>

  for(jin=0, iin=0; jin < npalin; jin++)    // cykl pres vstupni paletu >>>
  {
    ir= palin[iin++];
    ig= palin[iin++];
    ib= palin[iin++];
// ========================= hledani nejblizsi barvy ve vyst. palete >>>>
    kmaxmin=32000;
    ksummin=32000;
    for(jout=0, iout=0; jout < npalout; jout++)  //cykl pres vyst. paletu
    {
      or= palout[iout++];
      og= palout[iout++];
      ob= palout[iout++];

      kr2= abs(ir - or);  // diference slozek
      if(kr2 > kmaxmin) continue;
      kg2= abs(ig - og);
      if(kg2 > kmaxmin) continue;
      kb2= abs(ib - ob);
      if(kb2 > kmaxmin) continue;

      kmax= max( kr2, max(kg2, kb2) );	// 1.metrika
      if(kmax == 0)
	{jmin=jout; goto Hotovo;}
//p:      if(kmax > kmaxmin) continue;
      if(kmax == kmaxmin)
      {
	ksum= kr2 + kg2 + kb2;	// 2.metrika
	if(ksum >= ksummin) continue;
	jmin=jout;
	kmaxmin=kmax;
	ksummin= ksum;
      }
      else
      {
	jmin=jout;
	kmaxmin=kmax;
	ksummin= kr2 + kg2 + kb2;   // 2.metrika
      }
    } //konec cyklu pres vystupni paletu
Hotovo:
  mapio[jin]= jmin;
//  isttf[jmin]++;     // statistika
//        isttf(kmin)= isttf(kmin) +istat(ipll) !  !statistika
  } //konec cyklu pres vstupni paletu

  return;
}

#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include "x_lopif.h"

/*-------- Nastaveni palety jedne barvy (EGA/VGA) ---------*/

// Psat primo do HW registru pri zpetnem snimkovem behu (VGA)
void x_pal_1ret(int n_pal, char *pal_1, long *Nc, long *Zc)
{
  unsigned int  port,port1,port2;
  unsigned char Val,Inx;
  unsigned int  Zcount, Ncount;

  Inx  = n_pal;
  Zcount = Ncount = 0;
  port  = 0x03DA;   // Inp.status VGA
  port1 = 0x03C8;   // DAC write index
  port2 = 0x03C9;   // DAC data

  // Cvicne zmerit casy zpet. behu
  Val = inp( port );
  if(Val&0x08)    // Bit 3 je 1->zpetny beh v akci
  {  Inp1:
     Val = inp( port );
     if(Val&0x08)
       goto Inp1;
     else
       goto Beg_normal;   // Zacina dopredny beh
  }
  else            // Neni zpetny beh
  {  Inp2:
     Val = inp( port );
     if((Val&0x08) == 0)
       { Ncount++;
	 goto Inp2;
       }
     else
       goto Beg_back;   // Zacina zpetny beh
  }

  Beg_normal:          // Normal beh
  Val = inp( port );
  if((Val&0x08) == 0)
   { Ncount++;
     goto Beg_normal;
   }


  Beg_back:           // Zpetny beh
  Val = inp( port );
  if(Val&0x08)
   { Zcount++;
     outp( port1, Inx);

     outp( port2, pal_1[0]);
     outp( port2, pal_1[1]);
     outp( port2, pal_1[2]);
     goto Beg_back;
   }

  *Nc = Ncount;
  *Zc = Zcount;
}



// Otevreni *.OBR (1,4,8 bitu na pixel) a inicializace virt. pameti
// Pro XMS nacteni souboru do XMS (musi se cely vejit !!)
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <mem.h>

#include "x_lopif.h"

#ifdef VIRT_SCR
typedef struct
  {
  unsigned long length;         /* velikost prenasene pameti      */
  unsigned int sourceH;         /* handle pro zdroj (0=konvencni  */
  unsigned long sourceOff;      /* offset zdroje pameti           */
  unsigned int destH;           /* handle pro terc (0=konvencni   */
  unsigned long destOff;        /* offset terce pameti            */
  } XMOVE;

int get_xmem(void);             // Fce pro zachazeni s XMS
int alloc_xmem(int n);
int long ptr2long(char *p);
int h_xmove(XMOVE *p);

int xv_opn_virt(char *filnam, int Inx, int Typ)
{
// filnam - jmeno souboru .OBR
// inx    - index vv

//  return- 1 - bude se pracovat se souborem na disku
//          3 - bude se pracovat se souborem v XMS

  int  head[512];     // Hlavicka obru
  int  len_hla,kby,i,ire;
  unsigned int Len2,Rows,ist,zbrow;
  long size,Len4;
  XMOVE xmove;
  char *buflin;

  if(xv_vv[Inx].xv_file <= 0) return( 2 );    // Is some open ?

  xv_vv[Inx].xv_file = open(filnam,O_BINARY|O_RDWR);
  if(xv_vv[Inx].xv_file <= 0) return( 2 );

  ist=read(xv_vv[Inx].xv_file,head,2);           /* Nacist hlavicku */
  if(ist < 2) goto Errea;
  len_hla=head[0];
  if(len_hla == 0) len_hla = 512;
  lseek(xv_vv[Inx].xv_file,0L,SEEK_SET);
  ist=read(xv_vv[Inx].xv_file,head,len_hla);
  if(ist < len_hla)
    {Errea:
     close(xv_vv[Inx].xv_file);
     xv_vv[Inx].xv_file = 0;
     return( 4 );
    }

  ist =  head[5];                     /* Pocet urovni 2,16,256 */
  if(ist == 0)
    xv_vv[Inx].xv_bits = 0;
  else if(ist <= 2)
    xv_vv[Inx].xv_bits = 3;
  else if(ist <= 16)
    xv_vv[Inx].xv_bits = 1;
  else if(ist <= 256)
    xv_vv[Inx].xv_bits = 0;
  else if(ist == 32000)
   xv_vv[Inx].xv_bits = -1;
  else
   goto Errea;

  if(xv_vv[Inx].xv_bits >= 0)
   xv_vv[Inx].xv_len_col = head[3]>>xv_vv[Inx].xv_bits;
  else
   xv_vv[Inx].xv_len_col = head[3]*2;
  xv_vv[Inx].xv_rows = head[4];
  strcpy(xv_vv[Inx].xv_File,filnam);

  if(Typ == 2) goto Disk;

  i = get_xmem();
  if(i >= 0x0200)              // HIMEM.SYS O.K.
    {   size = (long)xv_vv[Inx].xv_rows * (long)xv_vv[Inx].xv_len_col + 1024;// Velikost obrazu
	kby  = (size / 1024L) + 1;                   // z B na kB
	xv_vv[Inx].xv_XMS = alloc_xmem(kby);         // Alokace kby kB
	if(xv_vv[Inx].xv_XMS == -1)                  // Nelze naalokovat
	  { goto Disk;     // Pouziti disku
	  }
	if(xv_vv[Inx].xv_len_col <= 4800) Rows = 8; else Rows = 4;

	Len2   = Rows * (unsigned)xv_vv[Inx].xv_len_col;
	Len4   = (long)Len2;
	buflin = farmalloc(Len4);
	if(buflin == NULL) return( 6 );

	//----93---Zmena pro XMS -> hlava OBRu je v XMS !!!
	xmove.sourceH = 0;
	xmove.destH   = xv_vv[Inx].xv_XMS;

	xmove.length  = (long)len_hla;
	xmove.sourceOff = ptr2long((char*)head);
	xmove.destOff = 0L;
	ist = h_xmove(&xmove);
	if(!ist) goto Err_move;

	//---- Vlasni data ---------------
	xmove.length  = Len4;                 // Prenos z DOS do XMS
	xmove.sourceOff = ptr2long(buflin);
	xmove.destOff = (long)len_hla;
	zbrow = xv_vv[Inx].xv_rows % Rows;

	for(i=0; i<xv_vv[Inx].xv_rows/Rows; i++)         // Nacteni .OBR do XMS (po 8/4)
	 { ist = read(xv_vv[Inx].xv_file,buflin,Len2);
	   if(ist < Len2) goto Errea;
	   ist = h_xmove(&xmove);
	   if(!ist)
	     { Err_move:
	       farfree(buflin);
	       close(xv_vv[Inx].xv_file);
	       xv_vv[Inx].xv_file = 0;
	       return( 8 );
	     }
	   xmove.destOff += Len4;
	 }
	if(zbrow != 0)                        // Zbytek do 8/4
	  { Len2 = zbrow * (unsigned)xv_vv[Inx].xv_len_col;
	    xmove.length = (long)Len2;
	    ist = read(xv_vv[Inx].xv_file,buflin,Len2);
	    if(ist < Len2) goto Errea;
	    ist = h_xmove(&xmove);
	    if(!ist) goto Err_move;
	    xmove.destOff += Len2;
	    Len2 = Len4;
	  }
	 farfree(buflin);
	 close(xv_vv[Inx].xv_file);
	 xv_vv[Inx].xv_file = 0;
	 ire = 3;
   }
  else
   { Disk:
     if(Typ == 1)
     { close(xv_vv[Inx].xv_file);
       xv_vv[Inx].xv_file = 0;
       xv_vv[Inx].xv_XMS = -1;
       return 2;
     }
     xv_vv[Inx].xv_XMS = -1;
     ire = 1;
   }

  xv_vv[Inx].xv_zmap = len_hla;
  xv_act = Inx;
  return( ire );
}
#endif
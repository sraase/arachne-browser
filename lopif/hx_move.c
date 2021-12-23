/***********************************************************************/
/*                        Navrh obalu na move_xmem                     */
/***********************************************************************/
/*                                             (c) Jan Vlaciha 1994    */
// 1997 - pridani EMS (HARO)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "x_lopif.h"

typedef struct
  {
  unsigned long length;         // velikost prenasene pameti
  unsigned int sourceH;         // handle pro zdroj (0=konvencni)
  unsigned long sourceOff;      // offset zdroje pameti
  unsigned int destH;           // handle pro terc (0=konvencni)
  unsigned long destOff;        // offset terce pameti
  } XMOVE;

int  xv_XmsEms = 1;               // 1-Xms, 2-Ems
long xv_XmsAlloc = 0;
extern char *EmsFrame;

int get_emem(void);             // Fce pro zachazeni s EMS
int alloc_emem(int n);          // Alokuje n KB pameti, vraci handle
int dealloc_emem(int h);        // Dealokuje EMS (handle)
int move_emem(XMOVE *p);        // presune blok z/do EMS
int mem_emem(unsigned *total, unsigned *free);
char *pageframeEMS(void);

int get_xmem1(void);             // Fce pro zachazeni s XMS
int dealloc_xmem1(int h);
int alloc_xmem1(int n);
int move_xmem1(XMOVE *p);
int mem_xmem1(unsigned *total, unsigned *free);
int long ptr2long(char *p);
int hinf_xmem(int h, int *kb, int *Freeh);

//------ Pouzivat XMS/EMS, zavolat jednou na zacatku ----
// Mode - 1 - XMS, 2 - EMS
// ret    1 - OK,  0 - ERR
int SetXmsEms(int Mode)
{
  int ist;

  xv_XmsAlloc = 0;

  if(Mode == 1)   // XMS
  { xv_XmsEms = 1;
    ist = get_xmem1();
    if(ist >= 0x0200)
    return( 1 );
  }
  else if(Mode == 2) // EMS
  { xv_XmsEms = 2;
    ist = get_emem();
    if(ist != -1)
    { EmsFrame = pageframeEMS();
      if(EmsFrame != NULL) return( 1 );
    }
  }

  return( 0 );
}

// Presun bufru z/do XMS,EMS
int h_xmove(XMOVE *p)
{
  char  policko[3];
  int   ret, pint;
  long  plong;

  if(xv_XmsEms == 2)      // ###### EMS !!!
  {
     ret = move_emem( p );
  }
  else if(xv_XmsEms == 1) // ###### XMS !!!
  {
  if ( p->length == 1L ) ////////////////////////// delka rovna jedne
   {
    if ( p->destH )                    // terc XMS - cteni terce do policka
    {                                  //            kvuli 2.Bytu
      pint         = p->sourceH;
      plong        = p->sourceOff;
      p->sourceH   = p->destH;
      p->sourceOff = p->destOff;
      p->destH     = 0;
      p->destOff   = ptr2long(policko);
      p->length    = 2L;
      ret = move_xmem1(p);
      p->destH     = p->sourceH;
      p->destOff   = p->sourceOff;
      p->sourceH   = pint;
      p->sourceOff = plong;
      policko[2] = policko[1]; // schovavana 2.Bytu
      if ( !ret ) goto Ko;
    }

    pint         = p->destH;           // cteni zdroje do policka
    plong        = p->destOff;
    p->destH     = 0;
    p->destOff   = ptr2long(policko);
    p->length    = 2L;                 // zdroj XMS,DOS
    ret = move_xmem1(p);
    p->destH     = pint;
    p->destOff   = plong;
    if ( !ret ) goto Ko;
    policko[1] = policko[2];

    pint         = p->sourceH;         // zapis z policka do terce
    plong        = p->sourceOff;
    p->sourceH   = 0;
    p->sourceOff = ptr2long(policko);
    p->length    = 2L;     // terc XMS/DOS  !? to je zatim spatne
//    p->length    = ( p->sourceH ) ? 2L : 1L;     // terc XMS/DOS
    ret = move_xmem1(p);
    p->sourceH   = pint;
    p->sourceOff = plong;
    if ( !ret ) goto Ko;
    p->length = 1L;
   }
  else if ( (p->length) & 1 ) ///////////////////// licha delka
   {
    plong = p->length - 2L;
    p->length -= 1L;
    ret = move_xmem1(p);
    if ( !ret ) goto Ko;

    p->length = 2L;
    p->destOff   = p->destOff   + plong;
    p->sourceOff = p->sourceOff + plong;
    ret = move_xmem1(p);

    p->length = plong + 2L;
    p->destOff   = p->destOff   - plong;
    p->sourceOff = p->sourceOff - plong;
   }
  else
   { ret = move_xmem1(p); /////////////////////// suda delka
   }

  }
  else            // ##### Ani XMS ani EMS
  {
    return( 0 );
  }

Ko:
  return(ret);
}


int get_xmem(void)
{
  int ire;

  if(xv_XmsEms == 2)
  { ire = get_emem();
  }
  else if(xv_XmsEms == 1)
  { ire = get_xmem1();
  }
  else
  { ire = -1;
  }
  return( ire );
}

int dealloc_xmem(int h)
{
  int ire;
  int kb,Freeh,ist;

  if(xv_XmsEms == 2)
  { ire = dealloc_emem( h );
  }
  else if(xv_XmsEms == 1)
  {
    ist = hinf_xmem(h, &kb, &Freeh);
    if(ist != -1)
    { xv_XmsAlloc -= kb;
    }
    ire = dealloc_xmem1( h );
  }
  else
  { ire = 0;
  }
  return( ire );
}

int alloc_xmem(int n)
{
  int ire;

  if(xv_XmsEms == 2)
  { ire = alloc_emem( n );
  }
  else if(xv_XmsEms == 1)
  { ire = alloc_xmem1( n );
    if(ire != -1)
    { xv_XmsAlloc += n;
    }
  }
  else
  { ire = -1;
  }
  return( ire );
}

int move_xmem(XMOVE *p)
{
  int ire;

  if(xv_XmsEms == 2)
  { ire = move_emem( p );
  }
  else if(xv_XmsEms == 1)
  { ire = move_xmem1( p );
  }
  else
  { ire = 0;
  }
  return( ire );
}

int mem_xmem(unsigned int *Free1, unsigned int *Free2)
{
  int ire;

  if(xv_XmsEms == 2)
  { ire = mem_emem( Free1, Free2);
  }
  else if(xv_XmsEms == 1)
  { ire = mem_xmem1(Free1, Free2);
  }
  else
  { ire = 0;
  }
  return( ire );
}

long mem_all_xmem(void)
{ return xv_XmsAlloc;
}
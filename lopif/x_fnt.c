/* Rutina pro nahrani rastrovych fontu ze souboru */
/* Pripdne umisti font do XMS ! 97 */
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <alloc.h>
#include <bios.h>

#include "x_lopif.h"
// fce lokalne pouzite pouze zde
int  x_fnt_isxms(char *fntfile);
int  x_fnt_loadxms( int InxFnt );
int  x_fnt_savexms(void);

typedef struct
  {
  unsigned long length;         /* velikost prenasene pameti v B (suda) */
  unsigned int sourceH;         /* handle pro zdroj (0=konvencni  */
  unsigned long sourceOff;      /* offset zdroje pameti           */
  unsigned int destH;           /* handle pro terc (0=konvencni   */
  unsigned long destOff;        /* offset terce pameti            */
  } XMOVE;

int get_xmem(void);             // Fce pro zachazeni s XMS
int dealloc_xmem(int h);
int alloc_xmem(int n);
int h_xmove(XMOVE *p);
int long ptr2long(char *p);


int x_fnt_load(char *fntfile, int xro, int mod)
{
/* fntfile - soubor s fontem         */
/* xro     - pocet radek             */
/* mod     - 0,1,2 Bios,Muj, Oba     */

  int  fntf,i,ire,ist,InxFnt;
  int  fhead[4];

  char   chr;
  long   len4;
  unsigned int len,ir;

  /*---------------------------------------------*/
  ire = 1;

  if(xg_fntalloc == 0)
  {
  if(xg_fbuf != NULL)         // Zahozeni minuleho fontu
    { farfree(xg_fbuf);
      xg_fbuf = NULL;
      if(xg_fonmem == 1)      // Dealoc stare XMS
       { //ist = dealloc_xmem(xg_fonhan);
       }
      else if(xg_fonmem == 2) // Close stareho fontu
       { //close(xg_fonhan);
       }
    }
  }

  // 1) ------ Zda se nelouduje prave nastaveny font
  ist = strcmpi(fntfile, xg_fnt_akt);
  if(ist == 0) return( 1 );

  // 2) ------ Zda je font jiz v XMS
  InxFnt = x_fnt_isxms(fntfile);
  if(InxFnt >= 0)    // Vraci index do xg_fnt_xms[], nebo -1
  {
    ist = x_fnt_loadxms( InxFnt );
    return( ist );   // load z XMS a ret
  }

  // 3) ------ Neni v XMS : nacist ze souboru jako aktuakni
  fntf = open(fntfile,O_RDONLY|O_BINARY);
  if(fntf <= 0) return(-2);

  ir = read(fntf,fhead,8);    /* Hlavicka */
  if(ir < 8) { ire = -6; goto End_x;}

  xg_yfnt = fhead[2];         // Vyska fontu
  strcpy(xg_fnt_akt,fntfile); // Aktualni font

  // ----- Konstantni sire fontu (stare fonty) 8x...fnt ----
  if(fhead[1] > 0)
  {
  len4 = 256L * fhead[3] + 8;
  if(len4 > 64000)
    { ire = -8; goto End_x;
    }
  len = len4-8;
  xg_xfnt = fhead[1];
  xg_fbyt = fhead[3];
  for(i=0; i<256; i++) xg_fonlen[i] = xg_xfnt;

  if(xg_fntalloc == 0)
  { xg_fbuf = (char *) farmalloc(len4);
    if(xg_fbuf == NULL) { ire = -4; goto End_x;}
  }
  else
  { if(len4 > xg_fntalloc) { ire = -4; goto End_x;}
  }

  ir = read(fntf,xg_fbuf,len);   /* Kody fontu */
  if(ir < len) { ire = -6; goto End_x;}
  xg_lbfnt = len;

  if(xg_xfnt <= 8)
    { ;
    }
  else if(xg_xfnt <= 16)         /* Opacne poradi bytu */
    { for(i=0; i<len; i += 2)
       { chr = xg_fbuf[i];
	 xg_fbuf[i] = xg_fbuf[i+1];
	 xg_fbuf[i+1] = chr;
       }
    }
  else
    { ire = -10; goto End_x;
    }
  xg_foncon = 0;     // Konstantni sire fontu xg_xfnt
  xg_fonmem = 0;     // Je v xg_fbuf (MEM)
  xg_fonhan = 0;     // Zadny handle

  }
  //--------- Proporcni fonty (z WINDOWS) ---------------
  else
  { // Nacist tabulky delek a adres z konce souboru
    len4 = lseek(fntf, -1280L, SEEK_END);
    ir = read(fntf,xg_fonlen,256);
    if(ir<256) { ire = -6; goto End_x;}
    ir = read(fntf,xg_fonadr,1024);
    if(ir<1024) { ire = -6; goto End_x;}
    lseek(fntf,8L,SEEK_SET);

    xg_foncon = (fhead[3]>>8);  // Proporcni s prum. siri znaku
    xg_xfnt   = (fhead[3]>>8);
    xg_fbyt   = 0;

    if((len4-8) < 64000)        // Vejde se cely do xg_xfnt
     {
      if(xg_fntalloc == 0)
      { xg_fbuf = (char *) farmalloc(len4);
	if(xg_fbuf == NULL) { ire = -6; goto End_x;}
      }
      else
      { if(len4 > xg_fntalloc)
	{ ire = -6; goto End_x; }
      }
      xg_fonmem = 0;
      xg_fonhan = 0;
      ir = read(fntf,xg_fbuf,(unsigned int)(len4-8));
      if(ir <= 0)
	{ if(xg_fntalloc == 0)
	  { farfree(xg_fbuf);
	    xg_fbuf = NULL;
	  }
	  ire = -6;
	}
       xg_lbfnt = (unsigned int)(len4-8);
     }
    else      // Delka fontu > 64000
     { ire = -8;
     }
  }
  // 4) Font je nacteny pro pouziti -> kopii do XMS
  ist = x_fnt_savexms();

  End_x:
  close(fntf);
  return( ire );
}

// Nastavi fixni buffer pro allokaci fontu
int x_fnt_alloc(long Size)
{
   x_fnt_cls();

   if( Size <= 0)
   { xg_fntalloc = 0;
   }
   else
   { xg_fbuf = farmalloc( Size );
     if(xg_fbuf == NULL) return( 0 );  // Error
     xg_fntalloc = Size;
   }
   return( 1 );                        // O.K.
}

// Inicializuje XMS a tabulku pro fonty
int  x_fnt_initxms(int Maxfnt)
{ // Maxfnt : maximalni pocet fontu v XMS
  // ret    : 1 - OK, <0 neni XMS
  int  len, ist, kby;
  long size;

  // bez XMS
  if(Maxfnt <= 0)
  { xg_fnt_max = 0;
    return( 1 );
  }

  // Alokace tabulky pro fonty v XMS
  len         = sizeof(struct FNTXTAB) * Maxfnt;
  xg_fnt_max  = Maxfnt;
  xg_fnt_fre  = 0;
  xg_fnt_xtab = (struct FNTXTAB *)farmalloc( len );
  if(xg_fnt_xtab == NULL) return( -2 );
  memset(xg_fnt_xtab, 0, len);

  // Alokace XMS pro fonty
  ist = get_xmem();
  if(ist >= 0x0200)                     // HIMEM.SYS O.K.
  { size = (long)SIZE_XMS_FNT * Maxfnt;
    kby  = (int)(size/1024L);
    xg_fnt_xms = alloc_xmem(kby);      // Alokace kby kB
    if(xg_fnt_xms == -1) goto No_XMS;  // Nelze naalokovat
    xg_fnt_xlen = (long)kby * 1024L;   // Celkova delka XMS v B
    xg_fnt_xoff = 0;
  }
  else
  { No_XMS:
    farfree(xg_fnt_xtab);
    xg_fnt_xtab = NULL;
    xg_fnt_max  = 0;
    return( -4 );
  }

  return( 1 );
}

// Ulozi aktualni font do XMS
int  x_fnt_savexms(void)
{
   unsigned int sizebuf, sizebeg;
   int    ist,ire;
   struct FNTMBUF *fntmbuf;
   char   *buf;
   XMOVE  xmove;

   if(xg_fnt_max <= 0) return( -1);  // do XMS se nic neuklada

   ire     = 1;
   sizebeg = sizeof(struct FNTMBUF);
   sizebuf = sizebeg + xg_lbfnt;
   //if(sizebuf&1) sizebuf++;

   buf = farmalloc(sizebuf);
   if(buf == NULL) return( -2 );

   if( (xg_fnt_fre >= xg_fnt_max) ||           // neni misto v tabulce nebo
       (xg_fnt_xoff+sizebuf >= xg_fnt_xlen) )  // neni misto v XMS
   { xg_fnt_fre = 0;
     xg_fnt_xoff = 0;
     memset(xg_fnt_xtab, 0, sizeof(struct FNTXTAB) * xg_fnt_max);
   }

   // Vyplnit buffer se vsim potrebnym pro save
   fntmbuf = (struct FNTMBUF *)buf;
   fntmbuf->lbfnt = xg_lbfnt;             // popis fontu
   fntmbuf->xfnt = xg_xfnt;
   fntmbuf->yfnt = xg_yfnt;
   fntmbuf->fbyt = xg_fbyt;
   fntmbuf->foncon = xg_foncon;
   fntmbuf->fonmem = xg_fonmem;
   fntmbuf->fonhan = xg_fonhan;
   memcpy(fntmbuf->fonlen, xg_fonlen, 256);
   memcpy(fntmbuf->fonadr, xg_fonadr, 256*4);
   memcpy(buf+sizebeg, xg_fbuf, xg_lbfnt);  // data fontu

   // ulozit do XMS
   xmove.sourceH = 0;
   xmove.sourceOff = ptr2long(buf);
   xmove.destH   = xg_fnt_xms;
   xmove.destOff = xg_fnt_xoff;
   xmove.length = sizebuf;
   ist = h_xmove(&xmove);
   if(!ist)
   { ire = -6;
   }
   else
   {
     xg_fnt_xtab[xg_fnt_fre].Offset = xg_fnt_xoff;
     xg_fnt_xtab[xg_fnt_fre].Size   = sizebuf;
     strcpy(xg_fnt_xtab[xg_fnt_fre].Name, xg_fnt_akt);
     xg_fnt_xoff += sizebuf;
     xg_fnt_fre++;
   }
   farfree(buf);
   return( ire );
}

// Zjisti zda je font v XMS
int  x_fnt_isxms(char *fntfile)
{  int i,ist;

   if(xg_fnt_max <= 0) return( -1);

   for(i=0; i<xg_fnt_fre; i++)
   { ist = strcmp(fntfile, xg_fnt_xtab[i].Name);
     if(ist == 0) return( i );
   }
   return( -1);
}

// Nacte font z XMS jako aktualni
int  x_fnt_loadxms( int InxFnt )
{
   unsigned int sizebuf, sizebeg;
   int    ist,ire;
   struct FNTMBUF *fntmbuf;
   char   *buf;
   XMOVE  xmove;

   if(xg_fnt_max <= 0) return( -1);

   ire = 1;
   sizebuf = xg_fnt_xtab[InxFnt].Size;
   sizebeg = sizeof(struct FNTMBUF);
   buf = farmalloc(sizebuf);
   if(buf == NULL) return( -2 );

   // precist z XMS
   xmove.sourceH = xg_fnt_xms;
   xmove.sourceOff = xg_fnt_xtab[InxFnt].Offset;
   xmove.destH   = 0;
   xmove.destOff = ptr2long(buf);
   xmove.length = sizebuf;
   ist = h_xmove(&xmove);
   if(!ist)
   { ire = -6;
   }
   else
   { fntmbuf = (struct FNTMBUF *)buf;
     xg_lbfnt = fntmbuf->lbfnt;
     xg_xfnt = fntmbuf->xfnt;
     xg_yfnt = fntmbuf->yfnt;
     xg_fbyt = fntmbuf->fbyt;
     xg_foncon = fntmbuf->foncon;
     xg_fonmem = fntmbuf->fonmem;
     xg_fonhan = fntmbuf->fonhan;
     memcpy(xg_fonlen,fntmbuf->fonlen, 256);
     memcpy(xg_fonadr,fntmbuf->fonadr, 256*4);
     strcpy(xg_fnt_akt, xg_fnt_xtab[InxFnt].Name);

     if(xg_fntalloc == 0)
     {
      if(xg_fbuf != NULL)         // Zahozeni minuleho fontu
       { farfree(xg_fbuf);
       }
      xg_fbuf = farmalloc(xg_lbfnt);
      if(xg_fbuf == NULL) { ire = -4; goto End_x;}
     }
     memcpy(xg_fbuf, buf+sizebeg, xg_lbfnt);  // data fontu
   }

   End_x:
   farfree(buf);
   return( ire );
}




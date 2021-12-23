//------------ Create new virtual videoram (in format .OBR) ---
// 1) na disku 2) v XMS
#include <io.h>
#include <mem.h>
#include <dos.h>
#include <dir.h>
#include <string.h>
#include <alloc.h>
#include <fcntl.h>
#include <sys\stat.h>

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

int xv_new_virt(char *filenam, // Jmeno souboru (DISK)
	       int dx, int dy, // Velikost souboru .OBR
	       int col,        // Vypln obru ( < 0 => nedefinovano)
	       int binar,      // 3-binarni obr, 0-bytovy obr, 1-4bit-NO!! -1=Hic
	       int npal,       // Delka palety
	       char *pal,      // Paleta obru
	       int Inx,        // Index (cislo stranky vv)
	       int Typ)        // 0-XMS/DISK, 1-XMS, 2-DISK
//  return   1 - O.K. , sude chyby
{
   long     size;
   char    *buf=NULL;
   int     *buf2=NULL;
   int      nbuf = 16000;
   int      vypln,iw,dxx,i,done,ist,kby;
   int      hlen;
   struct   ffblk ffblk;
   union  { char n1[1024];   // Hlava obru
	    int  n2[ 512];
	  } u1;
   XMOVE    xmove;

   // --------- Init --------------
   if(binar == 3)
    hlen = 512;
   else
    hlen = 1024;

   xv_vv[Inx].xv_zmap = hlen;
   xv_vv[Inx].xv_rows = dy;
   xv_vv[Inx].xv_bits  = binar;     // 0-bytovy, 3-binar
   strcpy(xv_vv[Inx].xv_File,filenam);
   if(binar == 0)
     xv_vv[Inx].xv_len_col = dx;    // 8bit/pix
   else if(binar == 3)
     xv_vv[Inx].xv_len_col = dx>>3; // 1bit/pix
   else if(binar == -1)
     xv_vv[Inx].xv_len_col = dx*2; // 16bit/pix
   else
     return( 10 );   // nepodporovany mod

   buf = farmalloc((long)nbuf);     // Alloc buff
   if(buf == NULL) return( 2 );
   buf2 = (int *)buf;

   memset(u1.n1,0,1024);            // Reset header
   // dxx = dx/8 * 8;  ??
   dxx = dx;

   // ------------ Zda soubor | XMS ------------------
   xv_vv[Inx].xv_file = xv_vv[Inx].xv_XMS = -1;
   if(Typ == 2) goto Disk;

   ist = get_xmem();
   if(ist >= 0x0200)                          // HIMEM.SYS O.K.
     { if(xv_vv[Inx].xv_bits == 0)
	 size = (long)dxx * (long)dy + 1024;   // Velikost OBRu
       else if(xv_vv[Inx].xv_bits == 3)
	 size = (long)dxx/8 * (long)dy + 1024; // Velikost OBRu
       else
	 size = (long)dxx*2 * (long)dy + 1024;   // Velikost OBRu
       kby  = (size / 1024L) + 1;              // z B na kB
       xv_vv[Inx].xv_XMS = alloc_xmem(kby);    // Alokace kby kB
       if(xv_vv[Inx].xv_XMS == -1)             // Nelze naalokovat
	{ goto Disk;     // Pouziti disku
	}
       xmove.sourceH = 0;            // Z DOS do XMS
       xmove.destH   = xv_vv[Inx].xv_XMS;
     }
   else                              // Diskovy soubor
     { Disk:
       if(Typ == 1) goto Err_big;     // Pouze XMS
       //if(xv_NoDisk != 0) return( 2 );

       done = findfirst(filenam,&ffblk,0);
       if(done == 0)
	 { unlink(filenam);
	 }
       xv_vv[Inx].xv_file = open(filenam, O_BINARY|O_CREAT|O_RDWR, S_IWRITE|S_IREAD);
       if(xv_vv[Inx].xv_file <= 0) goto Err_big;
     }

   // ------------ Vyplneni hlavicky -----------------
    u1.n2[0] = hlen;
    u1.n2[3] = dxx;               /* Radky,sloupce */
    u1.n2[4] = dy;
    u1.n2[6] = npal;
    u1.n2[7] = 2;
    u1.n2[33] = dxx;
    u1.n2[34] = dy;
    if(binar == 3)
      u1.n2[5] = 2;
    else if(binar == 0)
      u1.n2[5] = 256;    // 256
    else
      u1.n2[5] = 32000;  // Hicol

    if(npal <= 16)
      for(i=0; i<3*npal; i++) u1.n1[i+ 16] = pal[i];
    else
      for(i=0; i<3*npal; i++) u1.n1[i+254] = pal[i];

   // write header to XMS/DISK
   if(xv_vv[Inx].xv_XMS == -1)
    { lseek(xv_vv[Inx].xv_file,0L,SEEK_SET);
      iw = write(xv_vv[Inx].xv_file,u1.n1,hlen);
      if(iw < hlen) goto Err_wr;
    }
   else
    { xmove.length  = (long)hlen;
      xmove.sourceOff = ptr2long(u1.n1);
      xmove.destOff = 0L;
      ist = h_xmove(&xmove);
      if(!ist) goto Err_wr;
      xmove.destOff = (long)hlen;
    }

   // ------------ Generovani matice obru ------------
   if(col < 0) goto End_nevypln;        // Pro col < 0, nedef. poc.hodnoty

   if(xv_vv[Inx].xv_bits == 0)
    { size = (long)dxx * (long)dy;      // Bytovy
      vypln = col;
      memset(buf,vypln,nbuf);
    }
   else if(xv_vv[Inx].xv_bits == 3)
    { size = (long)dxx/8 * (long)dy;    // Binarni
      if((col&1) == 0)
	vypln = 0x00;
      else
	vypln = 0xFF;  // ### Tady pokusne vzorek
      memset(buf,vypln,nbuf);
    }
#if HI_COLOR
   else
    { size = (long)dxx*2 * (long)dy;      // Hic
      vypln = xg_hival[col];
      for(i=0; i<nbuf/2; i++) buf2[i] = vypln;
    }
#endif

   if(xv_vv[Inx].xv_XMS != -1)
    { xmove.length  = (long)nbuf;
      xmove.sourceOff = ptr2long(buf);
    }

   Write_cykl:
   if(size > (long)nbuf)
    { if(xv_vv[Inx].xv_XMS == -1)
       { iw = write(xv_vv[Inx].xv_file,buf,nbuf);
	 if(iw < nbuf) goto Err_wr;
       }
      else
       { ist = h_xmove(&xmove);
	 if(!ist) goto Err_wr;
	 xmove.destOff += (long)nbuf;
       }
      size -= nbuf;
      goto Write_cykl;
    }
   else
    { nbuf = size;
      if(xv_vv[Inx].xv_XMS == -1)
       { iw = write(xv_vv[Inx].xv_file,buf,nbuf);
	 if(iw < nbuf) goto Err_wr;
       }
      else
       { xmove.length  = (long)nbuf;
	 ist = h_xmove(&xmove);
	 if(!ist) goto Err_wr;
	 xmove.destOff += (long)nbuf;
       }
    }

   //--------- OK end ----------------
   End_nevypln:
   farfree(buf);
   xv_act = Inx;
   return( 1 );

   //---------- Errors ---------------
   Err_wr:
   farfree(buf);
   return( 4 );

   //---------- Errors ---------------
   Err_big:
   farfree(buf);
   return( 2 );
}
#endif
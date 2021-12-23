//------------- Close (save) virtual videoram -----------
#include <io.h>
#include <mem.h>
#include <alloc.h>
#include <fcntl.h>
#include <sys\stat.h>
#include "x_lopif.h"

#ifdef VIRT_SCR
// Global xv_... for virt screen
XV_VV xv_vv[MAXVIRTUAL];
int   xv_act=0;    // Actual vv
int   xv_NoDisk=0; // 1-only XMS, 0-Disk|XMS

// XMS
typedef struct
  {
  unsigned long length;         /* velikost prenasene pameti      */
  unsigned int sourceH;         /* handle pro zdroj (0=konvencni  */
  unsigned long sourceOff;      /* offset zdroje pameti           */
  unsigned int destH;           /* handle pro terc (0=konvencni   */
  unsigned long destOff;        /* offset terce pameti            */
  } XMOVE;

int dealloc_xmem(int h);         // Fce pro zachazeni s XMS
int long ptr2long(char *p);
int h_xmove(XMOVE *p);


//--------- Vystup na disk, dealokace -----------
int xv_cls_virt(int Wrt, int Inx)
{
// wrt - 0->provede se opusteni vv
//       1->zapis a opusteni vv
//       2->pouze zapis na disk

  int  ist,nbuf = 16000,iw;
  long size;
  char *buf;
  union  { char n1[1024];   // Hlava obru
	       int  n2[ 512];
  } u1;
  XMOVE xmove;

  if(xv_vv[Inx].xv_XMS != -1)   // Obr je v XMS
   {
     if(Wrt != 0)    // Zapis na disk
      { if(xv_vv[Inx].xv_file <= 0)
	 {
	  xv_vv[Inx].xv_file = open(xv_vv[Inx].xv_File, O_BINARY|O_CREAT|O_RDWR|O_TRUNC,
				    S_IWRITE|S_IREAD);
	  if(xv_vv[Inx].xv_file < 0) return( 2 );
	 }
	else
	 {  lseek(xv_vv[Inx].xv_file,0L,SEEK_SET);
	 }
	buf = farmalloc((long)nbuf);
	if(buf == NULL) return( 4 );

	xmove.sourceH = xv_vv[Inx].xv_XMS; // Z XMS do DOS
	xmove.destH   = 0;

	xmove.length  = (long)2;           // Hlavicka (512,1024)
	xmove.sourceOff = 0L;
	xmove.destOff = ptr2long(u1.n1);
	ist = h_xmove(&xmove);
	if(!ist) goto Err_wr;
	if(u1.n2[0] == 0 || u1.n2[0] == 512)
	 xmove.length  = 512L;
	else
	 xmove.length  = 1024L;
	ist = h_xmove(&xmove);
	if(!ist) goto Err_wr;
	if(xv_vv[Inx].xv_bits == 0)
	 size = (long)u1.n2[3] * (long)u1.n2[4];
	else
	 size = (long)u1.n2[3]/8 * (long)u1.n2[4];

	iw = write(xv_vv[Inx].xv_file,u1.n1,(int)xmove.length);
	if(iw <= 0) goto Err_wr;
	xmove.sourceOff = xmove.length;
	xmove.destOff   = ptr2long(buf);
	xmove.length    = nbuf;

	Write_cykl:
	if(size > (long)nbuf)
	 { ist = h_xmove(&xmove);
	   if(!ist) goto Err_wr;
	   xmove.sourceOff += (long)nbuf;
	   iw = write(xv_vv[Inx].xv_file,buf,nbuf);
	   if(iw < nbuf) goto Err_wr;
	   size -= nbuf;
	   goto Write_cykl;
	 }
	else
	 { nbuf = size;
	   xmove.length  = (long)nbuf;
	   ist = h_xmove(&xmove);
	   if(!ist) goto Err_wr;
	   iw = write(xv_vv[Inx].xv_file,buf,nbuf);
	   if(iw < nbuf) goto Err_wr;
	  }
	  farfree(buf);
	  close(xv_vv[Inx].xv_file);
	  xv_vv[Inx].xv_file = 0;
      }
     // Uvolneni XMS
     if(Wrt < 2)
      {
      ist = dealloc_xmem(xv_vv[Inx].xv_XMS);
      xv_vv[Inx].xv_XMS = -1;
      }
   }
  else               // OBR je na disku
   {
     if(Wrt < 2)
      { close(xv_vv[Inx].xv_file);
     	xv_vv[Inx].xv_file = 0;
      }
   }
  return( 1 );

  Err_wr:
  farfree(buf);
  return( 2 );
}
#endif
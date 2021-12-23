//----- Zapis obrazovky(casti) do souboru ----------
#include <dos.h>
#include <bios.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys\stat.h>

#include "x_lopif.h"
void get_o(unsigned char *sourc, unsigned char *dest, int npix, int nbit);

// Predpoklada se volani pouze v graf. modu (xlopif)
// Pozn: Zaokrouhluje se na nasobek 8
//       4kB lokalnich pro STACK !

int img_to_file(int x1, int y1, int x2, int y2, char *file)
{
// x1..y2 - cast obrazovky
// file   - soubor .OBR

    union U_1 { char n1[1024];     /* Hlavicka obru */
		int  n2[512];
	      } u1;

    unsigned char pal[768];
    unsigned char inlin[1030],outlin[1030];
    int lenh, iw, fobr, i,dx, dx8;

    fobr = open(file, O_BINARY|O_RDWR|O_CREAT, S_IWRITE|S_IREAD);
    if(fobr <= 0) return( 2 );

    //---------- Hlavicka OBRu ------------------
    for(i=0; i<512; i++) u1.n2[i]=0;

    dx = x2 - x1 + 1;
    dx = dx/8 * 8;
    x2 = x1+dx-1;
    dx8= dx/8;

    u1.n2[3] = dx;
    u1.n2[4] = y2 - y1 + 1;
    u1.n2[5] = 256;
    u1.n2[7] = 2;
    u1.n2[33] = u1.n2[3];
    u1.n2[34] = u1.n2[4];

    x_getpalette(pal);

    if(xg_256 == MM_16)
     { u1.n2[6] = 16;
       for(i=0; i<48; i++)  u1.n1[i+16] = pal[i];
       lenh = 512;
     }
    else if(xg_256 == MM_2)
     { u1.n2[6] = 2;
       u1.n1[19] = u1.n1[20] = u1.n1[21] = 63;
       lenh = 512;
       u1.n2[5] = 2;   // binar
     }
    else
     { u1.n2[6] = 256;
       for(i=0; i<768; i++)  u1.n1[i+254] = pal[i];
       lenh = 1024;
     }
    u1.n2[0] = lenh;
    iw=write(fobr,u1.n1,lenh);
    if(iw < lenh) goto Err_wr;

    //---------- Data OBRu ----------------------
    for(i=y1; i<=y2; i++)
     { x_getimg(x1,i,x2,i,inlin);    // 4B dx,dy
       if(xg_256 == MM_256)          // 256 barev
	 { iw = write(fobr,inlin+4,dx);
	   if(iw < dx) goto Err_wr;
	 }
       else if(xg_256 == MM_16)	   // 2, 16 barev
	 { get_o(inlin+4,outlin,dx,4);
	   iw = write(fobr,outlin,dx);
	   if(iw < dx) goto Err_wr;
	 }
       else if(xg_256 == MM_2)
	 { iw = write(fobr,inlin+4,dx8);
	   if(iw < dx8 ) goto Err_wr;
	 }
     }

    close( fobr );
    return( 1 );

    Err_wr:
    close( fobr );
    return( 4 );
}

//------------ Prevod bitovych rovin do pixluu---------------------

void get_o(unsigned char *sourc, unsigned char *dest, int npix, int nbit)

/* sourc - pole s jednim radkem po bit. rovinach       */
/* dest  - pole s jednim radkem pixlu                  */
/* npix  - pocet pixlu na radek                        */
/* nbit  - pocet bitovych rovin  1..8                  */
{
   int i,j,k,nb;
   unsigned char c;

   memset(dest,0,npix);       /* vynulovani pole dest   */
   nb = 0;

   for(j=0; j<nbit; j++)      /* Cykl pres bitove roviny*/
    {
     for(i=0; i< npix/8; i++)
       { c =  sourc[i+nb];
	 for(k=0; k<8; k++)
	   { if(c & 0x80) dest[(i<<3) + k] |= 1<<(3-j);
	     c = c<<1;
	   }
       }
      nb += npix/8;
     }
}

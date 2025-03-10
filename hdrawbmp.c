
// ========================================================================
// 16 color, 256 color and TrueColor BMP output
// (c)1997,1998,1999,2000 Zdenek Harovnik + Michael Polak, Arachne Labs
// ========================================================================

#include "posix.h"
#include "svga.h"
#include "picinfo.h"
#include "v_putimg.h"
#include "uiface.h"
#include "ie.h"
#include "a_io.h"
#include "messages.h"

int  PalForPaleteModes(struct picinfo *gif, int *mapio, int *Mmapio[2],
          int ReaPal, int nLocPal, unsigned char *pLocPal);
void mosaic_background(struct picinfo *gif,char *obuf,int yz,int imgx);
int  IndexToTranspCol(int TranspInx, char *Palx);

int  draw_gif(struct picinfo *gif , int cod_start,int interlac,
               int transparent, int *pal, char *realpal, long *ReaBytes);

int  PresspalO (int multip, char *Palin[], int *Npalin, char  *palout,
               int *npalout, int *mapio, int *Mmapio[],
               int Swinout, int TypFuse, int Tolerance,
               char *Savecols );

void resetcolorcache(void);
int  XCHdrawBMP(struct picinfo *gif);
void x_img16to256(char *bi, char *bo);
void x_img256to16(char *bi, char *bo);
void x_img2to256(char *bi, char *bo);
void x_img256to2(char *bi, char *bo);

void memerr(void);//I
void Piip(void);  //I
void safemappal(int npalout);
void outs( char far *s);        /* print a ASCIIZ string to stdio */

//int  PrepareScaleAndX(struct picinfo *gif, int interlac);
int  PrepareScaleAndX(struct picinfo *gif, int interlac, int NumImg, int *Prepni);
int  OpnScale(int x1, int y1, int x2, int y2);
int  Scale( unsigned char *inbuf, unsigned char *outbuf);
int  ScaleHI( unsigned short *inbuf, unsigned short *outbuf);
// Externy
extern char egamode,cgamode,vga16mode,vgamono;
extern int   IiNpal; //delka souhrnne palety --> vynulovat pri Clrscr a pod.!
extern char *Iipal;  //souhrnna paleta
extern int   currentframe;

#define MAX_BYTES    4096
#define MAX_PIXELS   2048

extern int g_SetScale;  // globalni zapnuti a vypnuti scalingu;
extern int g_IsScale;     // je zapnut scale do zadaneho obdelnika from_x,y, stop_x,y
extern int g_yzscale;     // aktualni y pro scaling gifu




//kresleni bitmapy
//1 ok, 2 err
//sizeonly a palonly jako u GIFu
int XCHdrawBMP(struct picinfo *bmp)
{
 unsigned char *buf=NULL;
 char *obuf=NULL;
 int rv=2,i,j,k; //err
 char px[7]={10,26,38,50,54,59,62};//prah
 char cx[6]={  15,30,46,52,58,61}; //color

 int col;
 char *Palin[2];
 int Npalin[2];
 int *Mmapio[2];
 int *mapio=NULL;
 char *Savecols=NULL;
 int npalout;
 int ret;
 int xz,yz,imgx;
 int allx=1,obufx=4;
 int relstartx,relendx,linebytes;
 char type;
 char *buf0=NULL,*buf1=NULL;
 short *obuf2=NULL; //16bit - hicolor
 int palindex;
 unsigned char *realpal=NULL;
 unsigned char *bufscl =NULL;
 unsigned int  *bufscl2=NULL;
 int   file, nlin, x1bmp=0, dxbmp=0, x1b, kk, nibble=0;

 // HARO : open z drawGif()
 file = a_fast_open(bmp->filename,O_BINARY|O_RDONLY,0);
 if(file <= 0) return( rv );

 buf=farmalloc(6601l);
 if(!buf) goto err;

 i = a_read(file, buf, 13);
 if(i < 13) goto err;

 i=a_read(file,buf,41); //1 bajt z hlavicky a bitmapinfoheader

 if(*(uint32_t *)&buf[1]!=40l || i!=41) //takovou bitmapu bohuzel neumim || eof
 {
#ifdef POSIX
  printf("Unsupported BMP marker %ld or header size %d!=41 in %s...\n",*(long *)&buf[1],i,bmp->filename);
#endif
  goto err;
 }
 //==============================
 bmp->size_x=(int)(*(int32_t *)&buf[5]);
 bmp->size_y=(int)(*(int32_t *)&buf[9]);
 if(bmp->sizeonly)
  goto ok;
 //==============================

 type=*(int*)&buf[15];

 if(type==4)
 {
  k=bmp->size_x%8;
//!!glennmcc: May 07, 2004 -- Michal Tyc pointed out this error in rounding
//which causes some BMPs to be 'skewed' when displayed
  linebytes=(bmp->size_x+k)/2;
//  linebytes=bmp->size_x/2;
 }
 else if(type==8)
 {
  k=bmp->size_x%4;
  linebytes=bmp->size_x;
 }
 else if(type==24)
 {
  k=(bmp->size_x*3)%4;
  linebytes=3*bmp->size_x;
 }
 else
 {
#ifdef POSIX
  printf("Unsupported BMP type: %d bpp\n",type);
#endif
  goto err;
 }

 if(k)
  linebytes += (4-k)%4;

 if(type!=24)             //bitCount = Palette
 {
  int offset=(int)(a_filelength(file)-54l-(long)linebytes*(long)bmp->size_y);
  bmp->npal=offset/4;

  if(bmp->npal>256)
   bmp->npal=256;

  i=a_read(file,buf,offset);
  if(i!=offset)
   goto err;

  if(!bmp->palismap || bmp->palonly || xg_256==MM_Hic)
  {
//mp!!begin
   if(egamode)
   {
    for(i=0; i<bmp->npal; i++) //convert rgb quad
    {
     bmp->pal[3*i] = egafilter(buf[4*i+2]>>2);
     bmp->pal[3*i+1] = egafilter(buf[4*i+1]>>2);
     bmp->pal[3*i+2] = egafilter(buf[4*i]>>2);
    }
   }
   else
   if(vga16mode && !vgamono)
   {
    for(i=0; i<bmp->npal; i++) //convert rgb quad
    {
     bmp->pal[3*i] = vgafilter(buf[4*i+2]>>2);
     bmp->pal[3*i+1] = vgafilter(buf[4*i+1]>>2);
     bmp->pal[3*i+2] = vgafilter(buf[4*i]>>2);
    }
   }
   else
   {
    for(i=0; i<bmp->npal; i++) //convert rgb quad
    {
     bmp->pal[3*i] = buf[4*i+2]>>2;
     bmp->pal[3*i+1] = buf[4*i+1]>>2;
     bmp->pal[3*i+2] = buf[4*i]>>2;
    }
   }
//mp!!end
  }
 }
 else
 //==============================
 //bitCount = TrueColor
 {
  if(xg_256 == MM_Hic)  // RGB v Hicol -> zadnou pal nepotrebuju
  { bmp->npal=16; //IiNpal;
    //memcpy(bmp->pal, Iipal, 3*IiNpal);
  }
  else                  // 256/16 -> nahradni paleta
  {
   if(!bmp->palismap || bmp->palonly) //kdyz paletu nemam, nebo ji je chci
   {
    i=0;
    while(i<6) //red ..kroky 36
    {
     j=0;
     while(j<6) //green.. kroky 6
     {
      k=0;
      while(k<6) //blue krok 1
      {
       col=3*(36*i+6*j+k);
       bmp->pal[col]=cx[i];
       bmp->pal[col+1]=cx[j];
       bmp->pal[col+2]=cx[k];
       k++;
      }
      j++;
     }
     i++;
    }
    col=3*216;
    bmp->pal[col]=0;
    bmp->pal[col+1]=0;
    bmp->pal[col+2]=0;
    bmp->palismap=0;
    bmp->npal=217;
   }//generate palette
  }
 }

 if(bmp->palonly)
  goto ok;

 if(xg_256 != MM_Hic)
 {
 if(!bmp->palismap)
 {
  mapio=farmalloc(sizeof(int)*512);
  Savecols=farmalloc(512);
  if(mapio==NULL || Savecols==NULL)
   goto err;

  Palin[0]= Iipal;
  Npalin[0]= IiNpal;
  Palin[1]=(char *)bmp->pal;
  Npalin[1]=bmp->npal;
  memset( Savecols+IiNpal, 0, 512-IiNpal);
  memset( Savecols, 1, IiNpal);
  npalout=x_getmaxcol()+1;
  ret=PresspalO (2, Palin, Npalin, Iipal, &npalout, mapio, Mmapio,
             2, 1, 1,  Savecols );
  {
   char str[80];
   sprintf(str,MSG_BMP,
           IiNpal, npalout);
#ifdef VIRT_SCR
   if(xg_video_XMS)
   {
    x_video_XMS(0, 0);
    outs(str);
    x_video_XMS(1, 0);
   }
   else
#endif
    outs(str);
  }
  if(ret!=1)Piip();
  safemappal(npalout);

  farfree(Savecols);
  farfree(mapio);
 }
 else //bmp->pal je kopie Mmapio, ktere je nyni aktualne na obrazovce:
 { Mmapio[1]=(int *)(bmp->pal);
 }
 }

  //==============================
  xz=bmp->from_x;

  // HARO : scaling Bmp
  if(g_SetScale==1 &&  bmp->resize_x>0 && bmp->resize_y>0 &&
     bmp->is_background==0 && xg_256>=0x60 && type>1)
  { g_IsScale = 1;
    yz=bmp->from_y+bmp->resize_y-1;
    OpnScale(bmp->size_x, bmp->size_y, bmp->resize_x, bmp->resize_y);
    relstartx=0;            // cely radek pro scale
    relendx=bmp->size_x;
    if(xg_256 == MM_Hic)
     k = bmp->resize_x * 2 + 8;
    else
     k = bmp->resize_x + 4;
    bufscl = farmalloc( k );
    if(!bufscl) goto err;
    if(xg_256 == MM_Hic) bufscl2 = (unsigned int*)bufscl;
    g_yzscale = yz;
    x1bmp = bmp->pic_x - bmp->from_x;   // oriznuti z leva
    if((bmp->pic_x + bmp->resize_x - x1bmp - 1) <= bmp->stop_x)
     dxbmp = bmp->resize_x - x1bmp;
    else
     dxbmp = bmp->stop_x - bmp->pic_x + 1;
    if(dxbmp <= 0) goto ok;     // neni co kreslit
  }
  else
  { g_IsScale = 0;
    yz=bmp->from_y+bmp->size_y-1;
    relstartx=bmp->pic_x-bmp->from_x;
    relendx=bmp->stop_x-bmp->from_x;
  }

  imgx=xz+bmp->size_x;
  if(imgx>bmp->screen_x +bmp->draw_x)imgx=bmp->screen_x +bmp->draw_x;
  if(imgx>MAX_PIXELS)imgx=MAX_PIXELS;
  if(relendx>MAX_PIXELS) relendx=MAX_PIXELS;

  buf0=farmalloc(MAX_BYTES+2);
  if(!buf0)
   goto err;

  obuf=buf0;
#ifdef HICOLOR
  if(xg_256 == MM_Hic)
  { obuf2 = (short*)buf0;
    obufx = 2;
    realpal = bmp->pal;
  }
#endif

  if(egamode || cgamode || vga16mode)
  {
   buf1=farmalloc(2050);
   if(!buf1)
    memerr();
  }
  
  v_getimg(bmp->pic_x,bmp->pic_y,imgx,bmp->pic_y,obuf);

  if(linebytes>6600) //zvrhlost
   linebytes=6600;

  i=a_read(file,buf,linebytes); //read first line
  j=0;

  do //for all lines
  {
   if(allx>relstartx && allx<relendx)
   {
    if(type!=24)           // 256 colors
    {
#ifdef HICOLOR
     if(xg_256 != MM_Hic)
#endif
     {
      if(type==4)
      {
       if(!nibble)
        col=(buf[j]&0xf0)>>4;
       else
        col=(buf[j++]&0xf);
       nibble=1-nibble;
      }
      else
       col=buf[j++];
     }
#ifdef HICOLOR
     else
     {
      if(type==4)
      {
       if(!nibble)
        palindex=((buf[j]&0xf0)>>4)*3;
       else
        palindex=(buf[j++]&0xf)*3;
       nibble=1-nibble;
      }
      else
       palindex=buf[j++]*3;
      if(xg_hi16)
       col=RGBHI16(realpal[palindex],realpal[palindex+1],realpal[palindex+2]);
      else
       col=RGBHI15(realpal[palindex],realpal[palindex+1],realpal[palindex+2]);
      obuf2[obufx++] = col;
      goto End_put;
     }
#endif
    }
    else                  // RGB bmp
    {
     char b=buf[j++]>>2;
     char g=buf[j++]>>2;
     char r=buf[j++]>>2;

#ifdef HICOLOR
     if(xg_256 != MM_Hic)
#endif
     {
     //REPAL:
     if(r<px[0] && g<px[0] && b<px[0])  // RED
     {
      col=0; //pure black
      goto put;
     }
     else if(r<px[1])
     {
      col=0;
     }
     else if(r<px[2])
     {
      col=36;
     }
     else if(r<px[3])
     {
      col=72;
     }
     else if(r<px[4])
     {
      col=108;
     }
     else if(r<px[5])
     {
      col=144;
     }
     else
     {
      col=180;
      if(r>px[6] && g>px[6] && b>px[6])
      {
       col=15;   //pure white
       goto put;
      }
     }

     if(g>px[1])  // GREEN
     {
      if(g<px[2])
      {
       col+=6;
      }
      else if(g<px[3])
      {
       col+=12;
      }
      else if(g<px[4])
      {
       col+=18;
      }
      else if(g<px[5])
      {
       col+=24;
      }
      else
      {
       col+=30;
      }
     }

     if(b>px[1])  // BLUE
     {
      if(b<px[2])
      {
       col+=1;
      }
      else if(b<px[3])
      {
       col+=2;
      }
      else if(b<px[4])
      {
       col+=3;
      }
      else if(b<px[5])
      {
       col+=4;
      }
      else
      {
       col+=5;
      }
     }
     }
#ifdef HICOLOR
     else  // HiCol
     {
      if(xg_hi16)   // 16 bitu na pixel
       col = RGBHI16(r,g,b);
      else          // 15 bitu na pixel
       col = RGBHI15(r,g,b);
      obuf2[obufx++] = col;
      goto End_put;
     }
#endif
    }//end if TrueColor bitmap

    //mapovani do palety
    col=Mmapio[1][col];

    put:
    obuf[obufx++]=col;

    End_put:;
   }

   if(allx==bmp->size_x)
   {
    allx=1;
    if(xg_256 != MM_Hic)
     obufx=4;
    else
     obufx=2;
    xz=bmp->from_x;

     if(g_IsScale != 0)  // Scaling BMP
     {
      if(g_yzscale>=bmp->pic_y && g_yzscale<=bmp->stop_y) //kreslit tenhle vyrez ?
      {
       if(xg_256 != MM_Hic)
       { nlin = Scale((unsigned char *)obuf+4, bufscl+4);
#ifdef POSIX
         {
          short int AX = dxbmp;
          bufscl[x1bmp  ] = AX & 0xff ; bufscl[x1bmp+1] = (AX>>8) & 0xff;
         }
#else
         _AX = dxbmp;
         bufscl[x1bmp  ] = _AL; bufscl[x1bmp+1] = _AH;
#endif
         bufscl[x1bmp+2] =   1; bufscl[x1bmp+3] =   0;
         x1b = x1bmp;
       }
       else
       { nlin = ScaleHI((unsigned short*)(obuf+4), (unsigned short*)bufscl2+2);
         bufscl2[x1bmp] = dxbmp; bufscl2[x1bmp+1] = 1;
         x1b = 2 * x1bmp;
       }
       if(nlin > 0)
       { for(kk=0; kk<nlin; kk++)
         { v_putimg(bmp->pic_x,g_yzscale-kk,(char *)&bufscl[x1b]);
         }
         g_yzscale -= nlin;
       }
      }
      if(g_yzscale<bmp->from_y) goto ok; //ze bych toho uz nechal ?
     }
     else    // 1:1
     {
      if(yz>=bmp->pic_y && yz<=bmp->stop_y) //kreslit tenhle vyrez ?
      {
       if(egamode || cgamode || vga16mode)
       {
        if(cgamode)
         x_img256to2(obuf,buf1);
        else
         x_img256to16(obuf, buf1);
        obuf=buf1;
       }

      v_putimg(bmp->pic_x,yz,obuf);

      if(bmp->is_background) // pozadi  (nikdy se scale!)
      {
      char saveparam[4]; //4 bajty pro putimg
      memcpy(saveparam,obuf,4);
//      printf("mosaic background in BMP\n");
      mosaic_background(bmp, obuf, yz, imgx);
      memcpy(buf0,saveparam,4);
      }

     obuf=buf0;

     } //end if kreslit
     yz--;

     if(yz<bmp->from_y) goto ok; //ze bych toho uz nechal ?
     } //end if 1:1

    i=a_read(file,buf,linebytes); //dalsi radek
    j=0;
   }
   else
   {
    xz++;
    allx++;
   }
  }
  while(i); //end BMP loop

 //==============================
 ok:
 rv=1;
 //==============================
 err:
 if(buf1)farfree(buf1);
 if(buf0)farfree(buf0);
 if(buf)farfree(buf);
 if(bufscl)farfree(bufscl);
 a_close(file);
 return rv;
}



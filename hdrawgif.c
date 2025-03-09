
// ========================================================================
// Normal and animated GIF routines
// (c)1997,1998,1999,2000 Zdenek Harovnik + Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "xanimgif.h"
#include "v_putimg.h"

#include <sys/param.h>
#define min MIN
#define max MAX

//originaly (c) Ibase group
//changes and BMP implementation (c)1996,1997 xChaos
struct GIFGLB  {  unsigned char sigGIF[6];    /* Globani hlavicka */
                  short unsigned int  screenwide;
		  short unsigned int  screendeep;
                  unsigned char global;
                  unsigned char background;
                  unsigned char pixaspect;
               };

struct GIFIMG
 {                              /* Image descriptor */
  short unsigned int  screex1;
  short unsigned int  screey1;
  short unsigned int  screex2;
  short unsigned int  screey2;
  unsigned char bits1;
 };

int  ReadGHeader(struct picinfo *gif, struct GIFGLB *GifGlb, int *ReaPal);
int  ReadGMarker(void);
int  ReadExtBlok(int *Transp,  unsigned int *backg, short int *tAnim, int *disp, int *GrControl);
int  ReadImgBlok(struct picinfo *gif, struct GIFIMG *GifImg,
                int *interlac, unsigned int *cod_start, int GlbPal,
                int *ReaPal, int *nLocPal, unsigned char *pLocPal);
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

int  PrepareScaleAndX(struct picinfo *gif,  int NumImg, int *Prepni);
int  OpnScale(int x1, int y1, int x2, int y2);
int  Scale( unsigned char *inbuf, unsigned char *outbuf);
int  ScaleHI( unsigned short *inbuf, unsigned short *outbuf);
int  RoundGE(double d);
// Externy
extern char egamode,cgamode,vga16mode,vgamono;
extern int   IiNpal; //delka souhrnne palety --> vynulovat pri Clrscr a pod.!
extern char *Iipal;  //souhrnna paleta

// Globaly
int filx;          /* .GIF soubor a .OBR soubor */
int g_SaveGif = 0; // male gify (animovane) do XMS, je-li!
int g_gifDrawXms;  // aktualni nastaveni kreleni=1/sav_xms=0

int g_SetScale=1;  // globalni zapnuti a vypnuti scalingu;
int g_IsScale;     // je zapnut scale do zadaneho obdelnika from_x,y, stop_x,y
int g_yzscale;     // aktualni y pro scaling gifu
double scl_yy;     // newY/origY
int g_resiz_x, g_resiz_y;  // puvodni velikost resize $$$
int g_size_x, g_size_y;    // velikost prvniho gifu   $$$

//int basebackg;
int gifx0,gify0;
unsigned char g_rT=0,g_gT=0,g_bT=2;  // RGB pro transp. color

//#define MAX_SAVEGIF 16384    // 128*128
#define MAX_SAVEGIF 40000L

#ifdef MSDOS
#define MAX_BYTES    2400
#define MAX_PIXELS   1200
#else
#define MAX_BYTES    16000
#define MAX_PIXELS   8000
#endif

int (*g_DrawFce)(struct picinfo *gif, int Flags, char *pal,
                 int yz, char *Buf, char *Work, char *px16);
int DrawLine(struct picinfo *gif, int Flags,char *pal,
             int yz, char *Buf, char *Work, char *px16);

void init_picinfo(struct picinfo *img)
{
  img->size_y=0;
  img->size_x=0;
  img->resize_y=0;
  img->resize_x=0;
  img->palismap=0;
  img->npal=0;
  img->IsInXms=0;
  img->hPicInfo=IE_NULL;
  img->is_animation=0;
  img->NextImg=0;
  img->BegImg=0;
  img->is_background=0;
  img->filename[0]='\0';
  img->picinfo_frameID=p->currentframe;
  img->BckImg = 0;
}


// ------------- haro -----------------------
int drawGIF(struct picinfo *gif)
{
//picinfo (in/out) - jmeno gifu a kam nakreslit
//hPicinfo(in) - handle picinfo (jen pro anim.gify)
//ModPic  (out)- zda uchovat zmeny v picinfo
//   char   drive[8], dir[80], name[16], ext[8];
//   char *ext;
   struct GIFGLB GifGlb;
   struct GIFIMG GifImg;
   int    ist, mark, interlac, NumImg = 0;
   int    *mapio=NULL, *Mmapio[2];
   int     i, ire=1, TraCol, Prepni;
   unsigned int backg,code_start;
   int    Transp = 0, GrCtrl = 0;
   short int    tAnim  = 0; // cas animace v setinach vteriny
   int    disp   = 0; // dispozice
   int    ReaPal = 0; // zda se cetla local paleta
   int    GlbPal = 0; // zda se cetla globalni paleta
#ifdef XANIMGIF
   int    vdx = 0, vdy = 0;
#endif
   long   CurPos, ReaBytes, BckAdr;
   char   *pAktpal;
   unsigned char *pLocPal=NULL; // lolalni gif paleta
   int    nLocPal=0;    // delka loc. palety
   int    bSaveBck1=0,bSaveBck2=0;// zda ukladat background pro animovane

   //*ModPic = 0; nahrazeno swapmod !!
   //basebackg=-1; //mp!!

//   fnsplit(gif->filename, drive, dir, name, ext);

/* //no longer needed - drawGIF is always called from drawanyimage()

   ext=strrchr(gif->filename,'.');
   if(ext && strcmpi(ext,".bmp") == 0)
   { // Kresli bmp
     ist = XCHdrawBMP(gif);
     return( ist );
   }
*/
/*
#ifdef XANIMGIF
   if(gif->palonly==0 && gif->sizeonly==0 && gif->IsInXms)
   {  // Uz je v xms -> krelit z ni
     ist = XGifFromXms(gif, xg_video_XMS, vdx, vdy, &tAnim);
     return( ist );
   }
#endif
*/


   New_read_gif:
   NumImg = 0;
   ist = ReadGHeader(gif, &GifGlb, &GlbPal);
   if((ist&1) == 0) //failure
   { /*ire = ist*/
     a_close (filx);
     if(ist==6) //mp: kdyz to neni GIF, zkusit BMP
      return XCHdrawBMP(gif);
     else
      return 0;
   }

   pLocPal = farmalloc(768);     // Buf. na lokalni palety
   if(pLocPal==NULL) memerr();

   g_DrawFce = DrawLine;   // fce volana uvnitr dekomprese
/**
#ifdef XANIMGIF
   if(ReaPal && gif->palonly==0 && gif->sizeonly==0)
   { if(pGifPal==NULL)
     { pGifPal = farmalloc(768);
       if(pGifPal==NULL) memerr();
     }
     memcpy(pGifPal, gif->pal, 3*gif->npal); // jen pro anim
     nGifPal = gif->npal;                   // schovat glb. paletu gifu
     for(i=0; i<3*nGifPal; i++) pGifPal[i]>>=2;
   }
#endif
**/
   // Hlavni cykl pres jednotlive bloky v GIFu
   Next_marker:
   mark = ReadGMarker();
   if(mark < 0)
   { /*ire = 8*/;
     //ERRGIF
     if(NumImg == 0)
       goto Only_sp;
     else if(NumImg == 1)
     {
      ire = 1; //mp!
      goto Err_frame;
     }
     else
     {
      ire = 1;
      goto Add_to_anim;
     }
     //ERRGIF
   }
   else if(mark == 1)     // Ext. blok
   {
     ist = ReadExtBlok(&Transp, &backg, &tAnim, &disp, &GrCtrl);
     if((ist&1)==0) { /*ire = 10*/ ire=1; goto Err_frame; } //mp!
     if(GrCtrl)  // Cetlo se grf. ext.
     { //if(backg >= gif->npal) backg = gif->npal-1;  // Toto se vyskytuje ??
       gif->bgindex = backg;   // back, nebo transp index
       swapmod = 1;            // mark XSWAP as modified

           //RGB001
           if(Transp && !gif->palismap && GlbPal)
	   { gif->pal[3*backg  ] = g_rT;
             gif->pal[3*backg+1] = g_gT;
             gif->pal[3*backg+2] = g_bT;
           }
     }
   }
   else if(mark == 2)     // Img. blok
   {
     ist = ReadImgBlok(gif, &GifImg, &interlac, &code_start, GlbPal,
		       &ReaPal, &nLocPal, pLocPal);
     if((ist&1)==0)   { /*ire = 12*/ ire=1; goto Err_frame; } //mp!
     if(gif->sizeonly){ ire = 1; goto Only_sp; }

     //SaveBck
     if( Transp )
         { //RGB001

           if(ReaPal==0 && GlbPal) //!!mp
            ReaPal=1;              //!!mp


           if(ReaPal ==1)
	   { gif->pal[3*backg  ] = g_rT;
             gif->pal[3*backg+1] = g_gT;
             gif->pal[3*backg+2] = g_bT;
           }
	   else if(ReaPal == 2)
           { pLocPal[3*backg  ] = g_rT;
             pLocPal[3*backg+1] = g_gT;
             pLocPal[3*backg+2] = g_bT;
           }

          //if(GifImg.screex2==GifGlb.screenwide && GifImg.screey2==GifGlb.screendeep)
          if(disp >= 2)
           bSaveBck1++;   // max. velikost s transp
	  else
           bSaveBck2++;   // mensi nez. cele s transp
     }

     if(!gif->palismap || gif->palonly || xg_256==MM_Hic)
     {
       gif->palismap=0;
       if(gif->palonly)
       { if(GlbPal == 0) // Zadna globalni->vezmu lokalni
         {
//mp!!begin
          //specialni filtry pro 16ti barevne mody
          if(egamode)
	   for(i=0; i<3*gif->npal; i++) gif->pal[i]=egafilter(gif->pal[i]);
          else if(vga16mode && !vgamono)
           for(i=0; i<3*gif->npal; i++) gif->pal[i]=vgafilter(gif->pal[i]);
          else
//mp!!end
           memcpy(gif->pal, pLocPal, 3*nLocPal);
           gif->npal = nLocPal;
         }
         ire = 1; goto Only_sp;
       }
     }

     CurPos = a_lseek(filx, 0, SEEK_CUR);
     // Pripravit scaling (bakground NE, pripadne vzdy zakazat),nebo oriznuti
     //if(NumImg == 0)
     { ist = PrepareScaleAndX(gif, NumImg, &Prepni);
       if(ist == 0) { ire = 1; goto Err_frame; }   // gif mimo scr
       //if(Prepni) goto Prepni; -> kreslit prvni frame gifu -> zacykleni
     }


     if(xg_256 != MM_Hic) // * * * * * * * * * * * * * 16 color, 256 color
     {
       // Kde vezmu Mmapio ??
       if(gif->palismap && ReaPal != 2)  // uz mam mapu a img mema loc.paletu
       { Mmapio[1]=(int *)(gif->pal);
       }
       else  // musim ji vyrobit
       { mapio=farmalloc(1024*sizeof(int)); if(!mapio) return(2);
         ist = PalForPaleteModes(gif, mapio, Mmapio, ReaPal, nLocPal, pLocPal);
	 if(ist != 1) { /*ire=ist*/ ire=1; goto Err_frame;} //mp!
       }

#ifdef XANIMGIF
       if(g_gifDrawXms == 0)
       { TraCol = IndexToTranspCol((int)gif->bgindex, (char*)Mmapio[1]);
         ist = XInitImgXms(gif, NumImg, Transp, TraCol, tAnim, disp);
         if(ist != 1)  // Neni misto v XMS : prepnout na screen a znovu
         { goto Prepni;
	 }
       }
#endif

       ire = draw_gif(gif,code_start,interlac,Transp,Mmapio[1],NULL, &ReaBytes);
       if(mapio) { farfree(mapio); mapio = 0; }

     }
     else // * * * * * * * * * * * * * * * * * * * * * * * * * * * Hicolor...
     {
       if( ReaPal==2 )
        pAktpal = (char *) pLocPal;
       else
	pAktpal = (char *)gif->pal;
#ifdef XANIMGIF
       if(g_gifDrawXms == 0)
       { TraCol = IndexToTranspCol((int)gif->bgindex, pAktpal);
	 ist = XInitImgXms(gif, NumImg, Transp, TraCol, tAnim, disp);
         if(ist != 1)  // neni XMS ?
         { Prepni:
           a_close(filx); goto New_read_gif;
         }
       }
#endif
      ire = draw_gif(gif,code_start,interlac,Transp,NULL,pAktpal, &ReaBytes);
     }


     ReaPal = 0;
     if(ire != 1)    // chyby pri dekompresi gifu
     { ire = 1;      // v NumImg je pocet OK framu //mp!
          // if(NumImg > 0)
           { goto Err_frame;
           }
          // else
          // { goto Only_sp;
          // }
     }

     NumImg++;
     CurPos = a_lseek(filx, CurPos+ReaBytes, SEEK_SET);
     disp = 0;
     //i=getch();
     if(g_gifDrawXms == 1 && NumImg > 0)
     { // Nakreslit vzdy jen prvni
       ire = 1; goto Only_sp;
     }
   }
   else if(mark == 0)     // OK end file
   {
     //char Msg[80];
     //sprintf(Msg,"EndGIFN: DrawXMS=%d NumImg=%d",g_gifDrawXms,NumImg);
     //outs(Msg);
     //getch();

     //ERRGIF
     Add_to_anim:
#ifdef XANIMGIF
     if(NumImg > 1 && g_gifDrawXms == 0) // pridat k animovanym
     {
       if(g_NumAnim < MAX_ANIMATEGIF)
       { g_TableAnim[g_NumAnim].hPicInf  = gif->hPicInfo; // ???
           g_TableAnim[g_NumAnim].NextAnim = 0;        // Cas pristi animace

           // SaveBck : Ulozit background ?
	   if(bSaveBck1 /*&& bSaveBck2*/)
           { ist = XSaveBackToXMS(gif, GifGlb.screenwide, GifGlb.screendeep, &BckAdr);
             if(ist==0)
             { if(BckAdr) XGifFreeXMS(); // uvolnit alloc. XMS pro pozadi
	     }
             else
             { gif->BckImg = BckAdr;
               swapmod = 1;
             }
           }
           g_NumAnim++;
         //outs("Add To Anim");
       }
     }

     Err_frame:
     if(NumImg == 0)
      goto Only_sp;

     if(g_gifDrawXms == 0 && gif->IsInXms)     // prvni obrazek z gifu v XMS
     { ist = XGifFromXms(gif, xg_video_XMS, vdx, vdy, &tAnim);
       // Je-li jeden frame, pak ho uvolnit
       if(NumImg == 1)
       { XGifFreeXMS();
       }
     }
#else
     Err_frame:
      ire = 1;
#endif
     goto Only_sp;
   }
   goto Next_marker;

   Only_sp:
   if(pLocPal) farfree(pLocPal);
   a_close(filx);
   return( ire );
}

// Pro transparentni animaci: z indexu hodnotu transparentni barvy
int IndexToTranspCol(int TranspInx, char *palx)
{   unsigned int TrCol;
    int      palindex, *Mapio;

#ifdef HICOLOR
    if(xg_256 == MM_Hic)
    { palindex=TranspInx*3;
      if(xg_hi16)
       TrCol=RGBHI16(palx[palindex],palx[palindex+1],palx[palindex+2]);
      else
       TrCol=RGBHI15(palx[palindex],palx[palindex+1],palx[palindex+2]);
    }
    else
#endif
    { Mapio = (int*)palx;
      TrCol = Mapio[TranspInx];
    }

    return( TrCol );
}

int  ReadGHeader(struct picinfo *gif, struct GIFGLB *GifGlb, int *ReaPal)
{
   unsigned int glb_pix,glb_pal;
   int          len_pal, nr, i;

   *ReaPal = 0;
   filx = a_fast_open(gif->filename,O_BINARY|O_RDONLY,0);
   if(filx <= 0) return 2;

   nr = a_read(filx, GifGlb, 13);         /* Nacteni hlavicky .GIF */
   if(nr < 13) return 4;

   if(strncmp((char *)GifGlb->sigGIF,"GIF",3) != 0)
   { return 6;
   }

   if(!gif->palismap)
    memset(gif->pal,0,768);

   glb_pix = (GifGlb->global & 0x07) + 1; /* Global pixel size */
   glb_pal = (GifGlb->global & 0x80);     /* Globalni paleta   */
   if(glb_pal != 0)
   { len_pal = ( 1<<glb_pix );
     if(!gif->palismap)
     { gif->npal=len_pal;
       nr = a_read(filx,gif->pal,len_pal*3);
       if(nr < len_pal) return( 8 );
       for(i=0; i<3*gif->npal; i++) gif->pal[i]>>=2;
       *ReaPal = 1;
     }
     else
       a_lseek(filx,len_pal*3,SEEK_CUR);
   }

   return( 1 );
}

int ReadGMarker(void)
{
// ret: 0 = ; | 1 = ! | 2 = ,
   int nr;
   unsigned char znacka;

   nr = a_read(filx,&znacka,1); // Co za blok
   if(nr < 1) return( -1 );
   if(znacka == ';')
    return( 0 );
   else if(znacka == ',')
    return( 2 );
   else if(znacka == '!')
    return( 1 );
   else
    return( -1 );
}

int ReadExtBlok(int *Transp,  unsigned int *backg, short int *tAnim, int *disp,
                int *GrControl)
{
    int  nr, len;
    char grcontrol=0;
    unsigned char znacka, bufx[256];

    *GrControl = 0;
    *backg = 0;

    nr = a_read(filx,&znacka,1); // typ
    if(znacka==0xF9)
     grcontrol=1;

    Cyk1:
    nr = a_read(filx,&znacka,1); // delka
    if(nr < 1) goto Err_end;
    if(znacka != 0)
    {
     len = znacka;
     nr = a_read(filx,bufx,len);
     if(nr < len) goto Err_end;
     if( grcontrol )
     {
      *GrControl = 1;
      memcpy(tAnim, &bufx[1], sizeof(short int));  // cas animace
      if(bufx[0]&1)  // Zadana transparence !
      {
        *Transp=1;
        *backg = bufx[3];            // ???TRANSP
      }
      else
      { *Transp=0;
      }
      *disp = (int)((bufx[0]&0x1C)>>2);
//      printf("[Ext=%d|Trans=%d|bg=%d|tanim=%d]",bufx[0],*Transp,*backg,*tAnim);
     }
     goto Cyk1;
    }
    else
    { return( 1 );
    }

    Err_end:
    return( 2 );
}

int ReadImgBlok(struct picinfo *gif, struct GIFIMG *GifImg,
                int *interlac, unsigned int *cod_start, int GlbPal,
                int *ReaPal, int *nLocPal, unsigned char *pLocPal)
{
    int nr, loc_pal,pix_size, len_pal, i;
    unsigned char cod;

    len_pal = gif->npal;

    nr = a_read(filx, GifImg, 9);
    if(nr < 9) goto Err_rea;
    *interlac = GifImg->bits1 & 0x40;      /* Prokladany obraz */
    loc_pal   = GifImg->bits1 & 0x80;      /* Lokalni paleta   */
    if(loc_pal != 0)
    {
     pix_size = (GifImg->bits1 & 0x07) + 1;
     len_pal = ( 1<<pix_size );
     if(!gif->palismap && GlbPal==0)  // neni mapa a neni globalni paleta (?)
     { gif->npal=len_pal;
       nr = a_read(filx,gif->pal,len_pal*3);
       if(nr < len_pal*3) goto Err_rea;
       for(i=0; i<3*gif->npal; i++) gif->pal[i]>>=2;
       *ReaPal = 1;
     }
     else               // je mapa a mam dalsi loc. palety
     { // lokalni palety pro dalsi anim. framy
       *nLocPal = len_pal;
       nr = a_read(filx,pLocPal,len_pal*3);
       if(nr < len_pal*3) goto Err_rea;
       for(i=0; i<3*len_pal; i++) pLocPal[i]>>=2;
       *ReaPal = 2;
     }
    }
    nr = a_read(filx, &cod, 1);
    *cod_start = cod;

    if(len_pal == 0 && !gif->palismap)   // Nema paletu (binar ?)
    { len_pal = 2;
      gif->pal[3] = gif->pal[4] = gif->pal[5] = 255;
    }
    gif->size_x=GifImg->screex2;
    gif->size_y=GifImg->screey2;
    if(gif->size_x==gif->resize_x && gif->size_y==gif->resize_y)
    {
     gif->resize_y=0;
     gif->resize_x=0;
    }
    gif->offx1 =GifImg->screex1;
    gif->offy1 =GifImg->screey1;
    return( 1 );

    Err_rea:
    return( 2 );
}

int PalForPaleteModes(struct picinfo *gif, int *mapio, int *Mmapio[2],
                      int ReaPal, int nLocPal, unsigned char *pLocPal)
{
    char *Palin[2];
    int   Npalin[2];
    char *Savecols = NULL;
    int   npalout, ire, Tol=1;

    Savecols=farmalloc(256*sizeof(int));
    if(!Savecols) return(2);

    if(!gif->palismap)
    {
     resetcolorcache();
     //michani palet
     Palin[0]= Iipal;
     Palin[1]= (char *)gif->pal;
     Npalin[0]= IiNpal;
     Npalin[1]= gif->npal;
     memset( Savecols+IiNpal, 0, 512-IiNpal);
     memset( Savecols, 1, IiNpal);
     npalout=x_getmaxcol()+1; //256 ?
     ire = PresspalO(2, Palin, Npalin, Iipal, &npalout, mapio, Mmapio,
                     2, 1, 1,  Savecols);
     {
      char str[80];
      sprintf(str,MSG_GIF,IiNpal,gif->npal,npalout);
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
     safemappal(npalout);
    }
    else //gif->pal je kopie Mmapio,
    {
      if(ReaPal != 2)
      { Mmapio[1]=(int *)(gif->pal);
      }
      else   // mam lokalni paletu
      {      // zadnou barvu nepridat, jen novou mapu
       Palin[0]= Iipal;
       Palin[1]= (char *)pLocPal;
       Npalin[0]= IiNpal;
       Npalin[1]= nLocPal;
       memset( Savecols+IiNpal, 0, 512-IiNpal);
       memset( Savecols, 1, IiNpal);
       npalout=IiNpal;
       ire = PresspalO(2, Palin, Npalin, Iipal, &npalout, mapio, Mmapio,
                       2, 1, Tol,  Savecols);
      }
    }

    farfree(Savecols);
    return( ire );
}

// priprava pro scaling a zda draw ci xms
int PrepareScaleAndX(struct picinfo *gif, int NumImg, int *Prepni)
{
//Return:  1: nejaka cast gifu padne do kreslici plochy
//         0: cely obrazek je mimo kreslici plochu, dal nic nedelat
  int    dxo, dyo, relx;
#ifdef XANIMGIF
  long   Size, SizeXms, LenPix;
  int    dygif,dxgif;
#endif

  // Kreslit / ulozit
#ifdef XANIMGIF
  if(g_SaveGif && gif->is_background==0 && xg_256 >= 0x60)
  { // Do XMS : je povoleno, neni bckg, jen pro 256&Hicol
    // a ma mensi rozmery nez MAX_SAVEGIF (scale|normal)
    // uklada se cely (pripadne scalovany)      $$$
    if((g_SetScale==0 || /*interlac != 0 ||*/ gif->resize_x==0 || gif->resize_y==0)
       || (gif->size_x==gif->resize_x && gif->size_y==gif->resize_y) /*|| NumImg>0*/)
     {
      Size = (long)gif->size_x * (long)gif->size_y;
      dxo = gif->size_x;
      g_IsScale = 0;
      gif->resize_x=0;
      gif->resize_y=0;
      //outs("g_SaveGif");
      //getch();
    }
    else
    {
      if(NumImg == 0)   // $$$ animace scale.gifu
      { g_resiz_x=gif->resize_x; g_resiz_y=gif->resize_y; // puvodni velikost resize
	g_size_x =gif->size_x  ; g_size_y =gif->size_y;   // velikost prvniho gifu
      }
      else
      { if(gif->size_x==g_size_x && gif->size_y==g_size_y) // stejny jako prvni
	{ gif->resize_x=g_resiz_x; gif->resize_y=g_resiz_y;
	}
	else  // jina (mensi) velikost => nove resize_ a offsety
	{ double pomx,pomy;
	  pomx = (double)g_resiz_x/(double)g_size_x;
	  pomy = (double)g_resiz_y/(double)g_size_y;
	  gif->resize_x = RoundGE(pomx*gif->size_x);
	  gif->resize_y = RoundGE(pomy*gif->size_y);
	  gif->offx1 = RoundGE(pomx*gif->offx1);
	  gif->offy1 = RoundGE(pomy*gif->offy1);
	}
      }
      Size = (long)gif->resize_x * (long)gif->resize_y;
      dxo = gif->resize_x; dyo = gif->resize_y;
      g_IsScale = 1;
      OpnScale(gif->size_x, gif->size_y, dxo, dyo);
    }

    // Mam misto ?
    if(xg_256 == MM_Hic) LenPix = 2; else LenPix = 1;

    if(gif->resize_x==0 || gif->resize_y==0 /* || interlac */)
    { dygif = gif->size_y; dxgif = gif->size_x;
    }
    else
    { dygif = gif->resize_y; dxgif = gif->resize_x;
    }
    SizeXms = (long)/*sizeof(xhead)*/24 + (long)dxgif * (long)dygif * LenPix;

    if(Size <= user_interface.xms4onegif && ((g_FreeAnim + SizeXms) < g_SizeAnimXMS))
    { g_gifDrawXms = 0;
    }
    else
    {
     goto Obrazovka;
    }
    gif->x1gif = 0;
    gif->dxgif = dxo;
  }
  else
#endif
  { Obrazovka:
    g_gifDrawXms = 1;      // Obrazovka
    if(NumImg > 0)         // Uz jsou nejake framy v XMS -> uvolnit
    {
#ifdef XANIMGIF
      g_FreeAnim = gif->BegImg;
#endif
      gif->IsInXms = 0;
      gif->NextImg = 0;
      gif->BegImg  = 0;
      gif->is_animation = 0;
      gif->BckImg  = 0;
      swapmod = 1;
      *Prepni = 1;         // Nakreslit prvni z anim. na scr
    }
    else
    { *Prepni = 0;
    }
    //outs("Obrazovka");
    //getch();

    relx=gif->pic_x - gif->from_x;   // oriznuti z leva a zhora ?
    gif->x1gif = relx;

    if(g_SetScale==0 || /*interlac != 0 ||*/ gif->is_background ||
       gif->resize_x==0 || gif->resize_y==0)
    { g_IsScale = 0;

      // Urcit oriznuti gifu (vyrez)
      if((gif->pic_x + gif->size_x - relx - 1) <= gif->stop_x)
       gif->dxgif = gif->size_x - relx;
      else
       gif->dxgif = gif->stop_x - gif->pic_x + 1;
    }
    else
    { g_IsScale = 1;  // Neni background, interlac a je zadano resize_x,y
      dxo = gif->resize_x;
      dyo = gif->resize_y;
      OpnScale(gif->size_x, gif->size_y, dxo, dyo);

      if((gif->pic_x + gif->resize_x - relx - 1) <= gif->stop_x)
       gif->dxgif = gif->resize_x - relx;
      else
       gif->dxgif = gif->stop_x - gif->pic_x + 1;
    }
  }

  if(gif->dxgif <= 0) { return( 0 ); }
  return( 1 );
}

// Zaokrouhleni double->int (pro >= 0)
int RoundGE(double d)
{
  double zbt,cele;
  int    nint;

  zbt = modf(d, &cele);
  if(zbt >= 0.5)
    nint = (int)(cele+1);
  else
    nint = (int)cele;
  return( nint );
}

// Kresleni jednoho dekomprimovaneho radku gifu
int DrawLine(struct picinfo *gif, int Flags, char *palx,
             int yzg, char *Buf, char *Work, char *px16)
{
// yzg - radek v gifu (od 0)
// Buf - dekomprim. radek gifu
// Work- pracovni buffer
// px16- pro 16 a 1 barevne mody (jinak NULL)
// Flags : bit 0: transparent 0-ne|1-ano
//         bit 1: mapa/paleta 0-mapa|1-paleta
//         bit 2: interlaced  0-ne, 1-ano
   int xz, yz, dx, x1, i, iout, imgx, palindex;
   int nlin, ii, Transp, ist, intrlc;
   unsigned char         *pInp, *pWork, *pWr;
   unsigned short *Work2;
   int *pal;

   xz=gif->pic_x;   // misto na obrazovce
   imgx=xz+gif->dxgif-1;
   if(imgx>MAX_PIXELS)imgx=MAX_PIXELS;

   if(Flags&4) intrlc=1; else intrlc = 0;

   if(g_IsScale)      // gif do zadaneho obdelnika from_x,y, stop_x,y
   { 
     nlin = Scale((unsigned char *)Buf, (unsigned char *)Work);    // buf->Work, nlin: kolik radku nakreslit
     if(intrlc == 0)
     { if(nlin == 0) return( 1 );// nlin je promene radek od radku
       yz = g_yzscale;
     }
     else                        // interlaced
     { yz = gif->from_y + RoundGE( scl_yy*yzg );
       //nlin = RoundGE( scl_yy ); $$$
       //if(nlin < 1) nlin = 1; else nlin++;
       if(nlin == 0)
        return( 1 );
       else
        nlin++;
     }
     pInp = (unsigned char *) Work;      // toto je ted vstup (vystup ze scalu)
     pWork= (unsigned char *)Buf;       // vstupni buf je pouzit jako pracovni
   }
   else     // SCALE = 1, kresleni puvodni velikosti, oriznuti
   { nlin = 1;
     yz=gif->from_y + yzg;
     pInp = (unsigned char *)Buf;
     pWork = (unsigned char *)Work;
   }

   x1 = gif->x1gif;                         // oriznuti radku v gifu
   dx = gif->dxgif;                         // velikost img
   Work2 = (unsigned short *)pWork;
   pWr= (unsigned char *)Work2;

   for(ii = 0; ii<nlin; ii++)                 // pro scale muze byt nlin > 1
   {
    if(g_gifDrawXms==1)                       // pro save do Xms vzdy vse
    { if(yz < gif->pic_y ) goto Next_line;    // ignorovat radek + pokracovat
      if(yz > gif->stop_y)
      { if((Flags&4)==0)
         return( 0 );       // ukoncit kresleni
        else
         goto Next_line;    // interlaced
      }
    }

    if((Flags&1) && (g_gifDrawXms==1))  // transparent + kresleni
    { if(egamode || cgamode || vga16mode)
      { 
       v_getimg(gif->pic_x, yz, imgx, yz, px16);
        if(cgamode)
         x_img2to256(px16, (char *)pWork);
        else
         x_img16to256(px16, (char *)pWork);
      }
      else
      {
       v_getimg(gif->pic_x, yz, imgx, yz, (char *)pWork);
      }
      Transp = 1;
    }
    else
    { Transp = 0;
      Work2[0] = dx; Work2[1] = 1;
    }

    // repaletizace + prepis netransparentnich pixlu
#ifdef HICOLOR
    if(xg_256 == MM_Hic)     // HiCol mody
    {
    iout = 2;
    for(i=x1; i<x1+dx; i++)
    { if(Transp && gif->bgindex == pInp[i])
      { if(gif->is_background)
        { palindex=gif->bgindex*3;
          if(xg_hi16)
           Work2[iout]=RGBHI16(Iipal[palindex],Iipal[palindex+1],Iipal[palindex+2]);
          else
           Work2[iout]=RGBHI15(Iipal[palindex],Iipal[palindex+1],Iipal[palindex+2]);
        }
      }
      else
      {
        palindex=((unsigned char)pInp[i])*3;
        if(xg_hi16)
         Work2[iout]=RGBHI16(palx[palindex],palx[palindex+1],palx[palindex+2]);
        else
         Work2[iout]=RGBHI15(palx[palindex],palx[palindex+1],palx[palindex+2]);
      }
      iout++;
    }
    pWr = (unsigned char *)Work2;
    }
    else                     // 256,16 atd mody
    {
#endif
    iout = 4; pal = (int*)palx;
    for(i=x1; i<x1+dx; i++)
    { if(Transp && gif->bgindex == pInp[i])
      { if(gif->is_background) pWork[iout] = gif->bgindex;
      }
      else
      { pWork[iout] = pal[(unsigned char)pInp[i]];
      }
      iout++;
    }
    if(egamode || cgamode || vga16mode)
    {
      if(cgamode)
        x_img256to2((char *)pWork, (char *)pInp);
      else
        x_img256to16((char *)pWork, (char *)pInp);
      pWr = pInp;
    }
    else
    { pWr = pWork;
    }
#ifdef HICOLOR
    }
#endif

#ifdef XANIMGIF
    if(g_gifDrawXms==1)
#endif
    { v_putimg(gif->pic_x, yz, (char *)pWr); // rovnou nakresli
    }
#ifdef XANIMGIF
    else
    { ist = XSaveImgLine((char *)pWr, gif->from_y, yz);   // uloz radku do XMS
      if(ist != 1) return( ist );
    }
#endif
    Next_line:
    yz++;
   } // end for nlin

   if(gif->is_background) // pozadi  (nikdy se scale!)
   { 
     mosaic_background(gif, (char *)pWr, yz-1, imgx);
   }

   if(g_IsScale) g_yzscale += nlin;
   return( 1 );
}

// Kresleni pozadi
// Fill an area with background
/*

//moved to v_putimg.c

void mosaic_background(struct picinfo *gif,char *obuf,int yz,int imgx)
{
 int pomx=0;
 int pomy=yz+gif->size_y;

 //dlazdicky:
 //while(pomx+imgx<=gif->screen_x+gif->draw_x) //HARO stop_x > draw_x ??
 //mp: stop_x je v souradnicich obrazovky!
// while(pomx+imgx<=gif->screen_x+gif->stop_x)
 while(pomx+imgx<=gif->stop_x)
 {
  while(pomy<gif->screen_y+gif->draw_y) //<-tady by mohlo/melo byt stop_y
  {
   v_putimg(gif->screen_x+pomx,pomy,obuf);
   pomy+=gif->size_y;
  }
  pomy=yz;
  pomx+=gif->size_x;
 }

 //dokresleni:
 pomx=gif->draw_x%gif->size_x;
 //mp: 0 je taky blbost!!!!
 if(pomx >= gif->draw_x || !pomx) return;    //HARO : optimalizace

 pomy=yz;
 while(pomy<gif->screen_y+gif->draw_y)
 {
  v_getimg(gif->screen_x ,pomy,gif->screen_x +pomx,pomy,obuf);
  v_putimg(gif->screen_x +gif->draw_x-pomx,pomy,obuf);
  pomy+=gif->size_y;
 }
}//end if pozadi

*/
/*----- Dekodovoani GIF dat (LZW) ---------------*/
/*----------- Pouzita makra (misto ppg) ---------*/
#define  FGETC  if(ibx == 0)                    \
                  { ibx = a_read(filx,ibuf,8192); \
                    if(ibx <= 0) goto Err_rea;  \
                    jx = 0;                     \
                  }                             \
                 ax = ibuf[jx];                 \
                 RByts++; jx++; ibx--

#define GETB    if(bufct == 0)                     \
                  { FGETC;                         \
                    bufct = ax;                    \
                    if(bufct == 0) goto End_data;  \
                  }                                \
                FGETC;                             \
                bufct--

int draw_gif(struct picinfo *gif , int cod_start,int interlac,
                 int transparent, int *pal, char *realpal,
                 long *ReaBytes)
{
   int yz,dx,dy;
   unsigned char *ctfirst=NULL,*ctlast=NULL; /* Dekompr. tabulka */
   int           *ctlink=NULL;
   unsigned char *ostack=NULL;            /* Docasny vystup   */
   char *obuf=NULL,*buf1=NULL,*buf2=NULL;  /* Vystupni radek  PRO PUTIMG */
   char *px16=NULL;

   unsigned char *ibuf=NULL;       /* Vstupni buffer   */
   int      ibx,bufct;             /* Index+blok       */
   long     RByts = 0;
   int      obufx=0;

   int cmask[9] = {0x0000,0x0001,0x0003,0x0007,0x000F, /* Maskovani */
                   0x001F,0x003F,0x007F,0x00FF};

   int inctable[5] = {8,8,4,2,1};  /* Interlaced obraz */
   int startable[5]= {0,4,2,1,0};
   int interlace=0;

   int   Flags;
   char *paldraw;

   int code,oldcode,clearcode,nextcode=0,nextlim=0,eoi;
   int rem=0,remct,reqct,done,ax,req1,co1,co2;
   int i,jx=0,ire=1,ist;

   /*--------------- Alokace --------------------------*/
   buf1 =  farmalloc(MAX_BYTES+sizeof(int));
   if(!buf1) goto Err_mem;
   obuf=buf1;
   buf2 =  farmalloc(MAX_BYTES+sizeof(int));
   if(!buf2) goto Err_mem;
   if(egamode || cgamode || vga16mode)
   { px16 = farmalloc(MAX_PIXELS);
     if(!px16) goto Err_mem;
   }

   ctlast =  farmalloc(4096L*sizeof(int));
   if(!ctlast) goto Err_mem;
   ctlink =  farmalloc(4096*sizeof(int));
   if(!ctlink) goto Err_mem;
   ctfirst =  farmalloc(4096L*sizeof(int));
   if(!ctfirst) goto Err_mem;
   ostack = farmalloc(4096L);
   if(!ostack) goto Err_mem;
   ibuf   =  farmalloc(8192L);
   if(!ibuf) goto Err_mem;

   memset (ctlast, 0, 4096*sizeof(int));
   for (i=0; i<4096; i++)
      ctlink [i] = -1;
   memset (ctfirst, 0, 4096*sizeof(int));
   memset (ostack, 0, 4096);
   memset (ibuf, 0, 8192);

   yz=0;
   dx=gif->size_x;
   dy=gif->size_y;
   g_yzscale = gif->from_y;

   if(transparent == 0) Flags = 0; else Flags = 1;
   if(pal != NULL)
   { paldraw = (char*)pal;
   }
   else
   { paldraw = realpal; Flags |= 2;
   }
   if(interlac) Flags |= 4;

   /*--------------- Inicializace ---------------------*/
   clearcode = 1 << cod_start;
   eoi       = clearcode + 1;
   reqct     = cod_start + 1;

   oldcode   = -1;
   done = 0;

   ibx  = bufct= 0;
   remct = 0;

   /*--------------- Hlavni cykl pres kody ------------*/

   HLAVNI_CYKL:
   /*---------- Ziskani dalsiho kodu ze vstupu --------*/

   if(reqct > 8)
     req1 = 8;
   else
     req1  = reqct;

   if(remct == 0)
   {
    GETB;
    rem = ax;
    remct = 8;
   }

   if(req1 > remct)
   {
    GETB;
    ax = ax<<remct;
    rem = rem | ax;
    remct += 8;
   }
   co1 = rem & cmask[req1];
   remct -= req1;
   rem = rem>>req1;

   if(reqct < 8)
   {
    code = co1;
   }
   else
   {
    req1 = reqct - 8;        /* Co zbylo */

    if(remct == 0)
    {
     GETB;
     rem = ax;
     remct = 8;
    }

    if(req1 > remct)
    {
     GETB;
     ax = ax<<remct;
     rem = rem | ax;
     remct += 8;
    }
    co2 = rem & cmask[req1];
    remct -= req1;
    rem = rem>>req1;
    code = (co2<<8) | co1;
   }//end if

   /*--------- Pro clearcode -> reinit tabulky --------*/
   if(code == clearcode)
   {
    nextcode = clearcode + 2;
    nextlim  = clearcode<<1;
    for(i=0; i<clearcode; i++)
    {
     ctfirst[i] = i;
     ctlast [i] = i;
     ctlink [i] = -1;
    }

    for(i=clearcode; i<4096; i++)
    {
      ctfirst[i] = 0;
      ctlast [i] = 0;
      ctlink [i] = -2;
    }

    reqct = cod_start + 1;
    oldcode = -1;
   }
   /*--------- Dekodovani inp kodu --------------------*/
   else
   {
    if(code == eoi)
    {
     done = -1;
     goto End_d;
    }

    if(ctlink[code] == -2)  /* Kod neni v tab */
    {
      ctlink [nextcode] = oldcode;
      ctlast [nextcode] = ctfirst[oldcode];
      ctfirst[nextcode] = ctfirst[oldcode];
      nextcode++;
      if(nextcode == nextlim)
      {
        if(reqct < 12)
        {
         reqct++;
         nextlim = nextlim<<1;
        }
      }
    }
    else                    /* Kod je v tab */
    {
     if(oldcode != -1)
     {
       ctlink [nextcode] = oldcode;
       ctlast [nextcode] = ctfirst[code];
       ctfirst[nextcode] = ctfirst[oldcode];
       nextcode++;
       if(nextcode == nextlim)
       {
        if(reqct < 12)
        {
         reqct++;
         nextlim = nextlim<<1;
        }
       }
     }
    }
    /*--- Prevod kodu na string -------------*/
    ax = code; i=0;
    do
    {
     ostack[i++] = ctlast[ax];
     ax = ctlink[ax];
    }
    while((i < 4096) && (ax >= 0));
    i--;

    do
    {
      if(obufx<MAX_PIXELS)
       obuf[obufx]=ostack[i];
      obufx++;

      if(obufx >= dx)   // Konec radku
      {
       ist = (*g_DrawFce)(gif, Flags, paldraw, yz, obuf, buf2, px16);
       if(ist == 0) goto Free;

       if(interlac)
       {
        yz+=inctable[interlace];
        if(yz>=dy)
        {
          interlace++;
          if(interlace<4)
           yz=startable[interlace];
          else
           yz--;
        }
       }
       else
       { yz++;
       }

       obufx = 0;
      }       // end if konec radku
      i--;
    }
    while(i >= 0);

    oldcode = code;
   }

   End_d:
   if(done == 0) goto HLAVNI_CYKL;

/*
 ????
   if(obufx > 0)
   {;
   }
*/
   ire = 1;

   Free:
   while( 1 )
   { GETB;     // docteni na konec dat obrazu
   }
   End_data:
   *ReaBytes = RByts; // pocet bytu zkomprim. obrazu

   if(ibuf)farfree(ibuf);
   if(ostack)farfree(ostack);
   if(ctfirst)farfree(ctfirst);
   if(ctlink)farfree(ctlink);
   if(ctlast)farfree(ctlast);

   if(px16)farfree(px16);
   if(buf2)farfree(buf2);
   if(buf1)farfree(obuf);

   return( ire ); /*--------- O.K. konec (EOI) ----*/

   Err_rea: // predcasny konec dat
   ire = 2; goto End_data;

   Err_mem: // neni pamet
   ire = 2; goto End_data;
}



// -------------- SCALING OBRAZU : 8 bitu/pixel
#define  POZADI 0xFFFF
#define  MSB    0x80
// globalni promenne pro scaling
int           scl_kx, scl_px, scl_nx, scl_tx, scl_ex;
int           scl_ky, scl_py, scl_ny, scl_ty;
unsigned char *scl_buf = NULL, scl_mode = 0;


//. Called by drawgif
int OpnScale(int x1, int y1, int x2, int y2)
{
// Parametry: x1, y1 ... rozmery puvodniho obrazku
//            x2, y2 ... rozmery pozadovaneho obrazku
// Return   : 1 ........ OK
//            2 ........ chyba pri alokaci (mozne jen pro scl_mode != 0)
   scl_yy = (double)y2 / (double)y1;   // jen pro interlac gify

   if ( (scl_kx = ( x1 < x2 )) != 0 )  // zvetsovani ve smeru x
   {
     scl_px = 2 * x1;
     scl_nx = 2 * x2 - scl_px;
     scl_tx = x2 - scl_px;
     for ( ; ; ) { if ( scl_tx < 0 ) { scl_tx += scl_nx; break; }
                   else                scl_tx -= scl_px;
                 }
     scl_ex = x2;
     scl_mode = 0;                     // integrovani nema smysl
   }
   else                                // zmensovani ve smeru x
   {
     scl_px = 2 * x2;
     scl_nx = 2 * x1 - scl_px;
     scl_tx = x1 - scl_px;
     scl_ex = x1;
   }

   if ( (scl_ky = ( y1 < y2 )) != 0 )  // zvetsovani ve smeru y
   {
     scl_py = 2 * y1;
     scl_ny = 2 * y2 - scl_py;
     scl_ty = y2 - scl_py;
     for ( ; ; ) { if ( scl_ty < 0 ) { scl_ty += scl_ny; break; }
                   else                scl_ty -= scl_py;
                 }
   }
   else                                // zmensovani ve smeru y
   {
     scl_py = 2 * y2;
     scl_ny = 2 * y1 - scl_py;
     scl_ty = y1 - scl_py;
   }

   if ( scl_mode )
   {
     if ( scl_buf != NULL ) farfree(scl_buf);
     scl_buf = farmalloc( (unsigned long)scl_ex );
     if ( scl_buf == NULL )
     {
       scl_mode = 0;
       return(2);
     }
     memset( scl_buf, POZADI, scl_ex );
   }
   return(1);
}

//. Low level functions
int Scale( unsigned char *inbuf, unsigned char *outbuf)
{
// Parametry : inbuf  ... radek puvodniho obrazku
//             outbuf ... radek pozadovaneho obrazku
// Return    : kolikrat opakovat vystupni radek ( 0 nebo kladne n )
   int           i = 0, j, tx;
   int uch = POZADI;

Zas:
   i++;
   if ( scl_ty < 0 )
   {
     scl_ty += scl_ny;
     tx = scl_tx;
     for ( j = 0; j < scl_ex; j++)
     {
       if ( scl_mode )
       {
         scl_buf[j] = min( scl_buf[j], *inbuf );
         uch        = min( uch, scl_buf[j] );
         scl_buf[j] = (char)POZADI;     // nastaveni pro dalsi radek
       }
       else uch = *inbuf;

       if ( tx < 0 )
       {
         tx += scl_nx;
         *outbuf++ = uch;
         inbuf++;                 // kopirovani vstupniho sloupce
         uch = POZADI;
       }
       else
       {
         tx -= scl_px;
         if ( scl_kx )
              *outbuf++ = uch;    // opakovani vstupniho sloupce
         else inbuf++;            // vypusteni vstupniho sloupce
       }
     }
     return(i);                   // vystup prevedeneho radku, i-krat
   }
   else
   {
     scl_ty -= scl_py;
     if ( scl_mode )             // jalovy radek
     {
       for ( j = 0; j < scl_ex; j++)
       {
         scl_buf[j] = min( scl_buf[j], *inbuf );
         inbuf++;
       }
     }
     if ( !scl_ky ) return(0);    // vypusteni vstupniho radku
   }
   goto Zas;
}

// Pro Hi-color radek : pouze pro scl_mode=0 !
int ScaleHI( unsigned short *inbuf, unsigned short *outbuf)
{
// Parametry : inbuf  ... radek puvodniho obrazku
//             outbuf ... radek pozadovaneho obrazku
// Return    : kolikrat opakovat vystupni radek ( 0 nebo kladne n )
   int           i = 0, j, tx;
   unsigned short uch = POZADI;

Zas:
   i++;
   if ( scl_ty < 0 )
   {
     scl_ty += scl_ny;
     tx = scl_tx;
     for ( j = 0; j < scl_ex; j++)
     {
       uch = *inbuf;

       if ( tx < 0 )
       {
         tx += scl_nx;
         *outbuf++ = uch;
         inbuf++;                 // kopirovani vstupniho sloupce
         uch = POZADI;
       }
       else
       {
         tx -= scl_px;
         if ( scl_kx )
              *outbuf++ = uch;    // opakovani vstupniho sloupce
         else inbuf++;            // vypusteni vstupniho sloupce
       }
     }
     return(i);                   // vystup prevedeneho radku, i-krat
   }
   else
   {
     scl_ty -= scl_py;
     if ( !scl_ky ) return(0);    // vypusteni vstupniho radku
   }
   goto Zas;
}


void far PreScale( int x1, int y1, int *x2, int *y2)
{
// Parametry : x1, y1 ... rozmery puvodniho obrazku    (vstup)
//             x2, y2 ... rozmery pozadovaneho obrazku (vstup/vystup)
// Funkce    : pokud pomer stran neodpovida vstupnimu pomeru, bude jeden
//             rozmer prislusnym zpusobem zkracen
  long  a, b;

  a = (long) x1 * (long) *y2;
  b = (long) y1 * (long) *x2;

  if ( a > b ) *y2 = (int) ( b / x1 );
  else         *x2 = (int) ( a / y1 );
}

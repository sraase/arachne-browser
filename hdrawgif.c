
// ========================================================================
// Normal and animated GIF routines
// (c)1997,1998,1999,2000 Zdenek Harovnik + Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "xanimgif.h"
#include "v_putimg.h"

//originaly (c) Ibase group
//changes and BMP implementation (c)1996,1997 xChaos
struct GIFGLB  {  unsigned char sigGIF[6]; /* Globalni hlavicka */
                                           // tr.: global header 
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
// Externals 
extern char egamode,cgamode,vga16mode,vgamono;
extern int   IiNpal; //delka souhrnne palety --> vynulovat pri Clrscr a pod.!
       // tr.: length of composite palette --> erase at Clrsrc etc.!
extern char *Iipal;  //souhrnna paleta (tr.: composite palette)

// Globals
int filx;          /* .GIF soubor a .OBR soubor (tr.: GIF and OBR files)*/
int g_SaveGif = 0; // male gify (animovane) do XMS, je-li!
  // tr.: small GIFs (animated) into XMS, if it exists! 
int g_gifDrawXms;  // aktualni nastaveni kreleni=1/sav_xms=0
  // tr.: current settings drawing=1/sav_xms=0
int g_SetScale=1;  // globalni zapnuti a vypnuti scalingu;
  // tr.: global switch of scaling on/off
int g_IsScale;     // je zapnut scale do zadaneho obdelnika from_x,y, stop_x,y
  // tr.: is the scale for a given rectangle from_x,y, stop_x,y switchted on?
int g_yzscale;     // aktualni y pro scaling gifu
  // tr.: current y for scaling GIF
double scl_yy;     // newY/origY
int g_resiz_x, g_resiz_y;  // puvodni velikost resize $$$
  // tr.: original size resize $$$
int g_size_x, g_size_y;    // velikost prvniho gifu   $$$
  // tr.: size of first GIF $$$

//int basebackg;
int gifx0,gify0;
unsigned char g_rT=0,g_gT=0,g_bT=2;  // RGB for transp. color

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


// ------------- HARO -----------------------
int drawGIF(struct picinfo *gif)
{
//picinfo (in/out) - jmeno gifu a kam nakreslit
   // tr.: name of gif and where to draw to 
//hPicinfo(in) - handle picinfo (only for anim. GIFs)
//ModPic (out) - zda uchovat zmeny v picinfo
   // tr.: whether to save/keep changes in picinfo
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
     // tr.: time of animation in 1/100 sec.
   int    disp   = 0; // disposition 
   int    ReaPal = 0; // whether local palette has been read
   int    GlbPal = 0; // whether global palette has been read
#ifdef XANIMGIF
   int    vdx = 0, vdy = 0;
#endif
   long   CurPos, ReaBytes, BckAdr;
   char   *pAktpal;
   unsigned char *pLocPal=NULL; // local GIF palette
   int    nLocPal=0;    // length of local palette
   int    bSaveBck1=0,bSaveBck2=0;// zda ukladat background pro animovane
     // tr.: whether to save background for animated
   //*ModPic = 0; nahrazeno swapmod !!
     // tr.: replaced swapmod / replaced with swapmod
   //basebackg=-1; //mp!!

//   fnsplit(gif->filename, drive, dir, name, ext);

/* //no longer needed - drawGIF is always called from drawanyimage()

   ext=strrchr(gif->filename,'.');
   if(ext && strcmpi(ext,".bmp") == 0)
   { // Kresli bmp (tr.: draw BMP)
     ist = XCHdrawBMP(gif);
     return( ist );
   }
*/
/*
#ifdef XANIMGIF
   if(gif->palonly==0 && gif->sizeonly==0 && gif->IsInXms)
   {  // Uz je v xms -> krelit z ni
      // tr.: is already in XMS, draw from memory
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
       // tr.: if it is not GIF, try BMP
      return XCHdrawBMP(gif);
     else
      return 0;
   }

   pLocPal = farmalloc(768);     // buffer for local palette
   if(pLocPal==NULL) memerr();

   g_DrawFce = DrawLine;   // fce called from within decompression
/**
#ifdef XANIMGIF
   if(ReaPal && gif->palonly==0 && gif->sizeonly==0)
   { if(pGifPal==NULL)
     { pGifPal = farmalloc(768);
       if(pGifPal==NULL) memerr();
     }
     memcpy(pGifPal, gif->pal, 3*gif->npal); // jen pro anim
          // tr.: only for animated
     nGifPal = gif->npal;                   // schovat glb. paletu gifu
          // tr.: keep/save palette of the GIF
     for(i=0; i<3*nGifPal; i++) pGifPal[i]>>=2;
   }
#endif
**/
   // Hlavni cykl pres jednotlive bloky v GIFu
   // tr.: main loop on individual blocks in the GIF
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
   else if(mark == 1)     // Ext. block
   {
     ist = ReadExtBlok(&Transp, &backg, &tAnim, &disp, &GrCtrl);
     if((ist&1)==0) { /*ire = 10*/ ire=1; goto Err_frame; } //mp!
     if(GrCtrl)  // Cetlo se grf. ext. (
        // tr.: grf. ext. has been read/loaded
     { //if(backg >= gif->npal) backg = gif->npal-1;  // Toto se vyskytuje ??
         // tr.: can this occur ??
       gif->bgindex = backg;   // back, or transp index
       swapmod = 1;            // mark XSWAP as modified

           //RGB001
           if(Transp && !gif->palismap && GlbPal)
           { gif->pal[3*backg  ] = g_rT;
             gif->pal[3*backg+1] = g_gT;
             gif->pal[3*backg+2] = g_bT;
           }
     }
   }
   else if(mark == 2)     // Img. block
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
           bSaveBck1++;   // max. size with transp
          else
           bSaveBck2++;   // mensi nez. cele s transp
             // tr.: smaller than/??? all/entire with transp
     }

     if(!gif->palismap || gif->palonly || xg_256==MM_Hic)
     {
       gif->palismap=0;
       if(gif->palonly)
       { if(GlbPal == 0) // Zadna globalni->vezmu lokalni
           // tr.: there is no global -> take local
         {
//mp!!begin
          //special filters for 16-coloured modes
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
          // tr.: prepare scaling (background NOT, if it occurs
          //      always forbid/exclude), or cutting
     //if(NumImg == 0)
     { ist = PrepareScaleAndX(gif, NumImg, &Prepni);
       if(ist == 0) { ire = 1; goto Err_frame; }   // GIF beyond screen
       //if(Prepni) goto Prepni; -> kreslit prvni frame gifu -> zacykleni
       // tr.: if(Switch) goto Switch; -> draw first frame of the GIF -> loop
     }


     if(xg_256 != MM_Hic) // * * * * * * * * * * * * * 16 color, 256 color
     {
       // Kde vezmu Mmapio ?? (tr.: from where will I take Mmapio ??)
       if(gif->palismap && ReaPal != 2)  // uz mam mapu a img mema loc.paletu
       // tr.: I already have a map and img does not have a local palette
       { Mmapio[1]=(int *)(gif->pal);
       }
       else  // musim ji vyrobit (tr.: I must create the palette)
       { mapio=farmalloc(1024*sizeof(int)); if(!mapio) return(2);
         ist = PalForPaleteModes(gif, mapio, Mmapio, ReaPal, nLocPal, pLocPal);
         if(ist != 1) { /*ire=ist*/ ire=1; goto Err_frame;} //mp!
       }

#ifdef XANIMGIF
       if(g_gifDrawXms == 0)
       { TraCol = IndexToTranspCol((int)gif->bgindex, (char*)Mmapio[1]);
         ist = XInitImgXms(gif, NumImg, Transp, TraCol, tAnim, disp);
         if(ist != 1)  // Neni misto v XMS : prepnout na screen a znovu
             // tr.: no space in XMS : switch to screen and again
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
         if(ist != 1)  // neni XMS ? (tr.: there is no XMS ?)
         { Prepni:
           a_close(filx); goto New_read_gif;
         }
       }
#endif
      ire = draw_gif(gif,code_start,interlac,Transp,NULL,pAktpal, &ReaBytes);
     }


     ReaPal = 0;
     if(ire != 1)    // chyby pri dekompresi gifu
                     // tr.: errors during decompressing the GIF
     { ire = 1;      // v NumImg je pocet OK framu //mp!
                     // tr.: in NumImg is number of OK frames //mp!
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
     { // Nakreslit vzdy jen prvni (tr.: draw always only the first)
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
                                 // tr.: add to animated
     {
       if(g_NumAnim < MAX_ANIMATEGIF)
       { g_TableAnim[g_NumAnim].hPicInf  = gif->hPicInfo; // ???
           g_TableAnim[g_NumAnim].NextAnim = 0;        // Cas pristi animace
                                // tr.: time of next animation

           // SaveBck : Save background ?
           if(bSaveBck1 /*&& bSaveBck2*/)
           { ist = XSaveBackToXMS(gif, GifGlb.screenwide, GifGlb.screendeep, &BckAdr);
             if(ist==0)
             { if(BckAdr) XGifFreeXMS(); // uvolnit alloc. XMS pro pozadi
                               // tr.: free allocated XMS for background
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
                        // tr.: first picture from the GIF in XMS
     { ist = XGifFromXms(gif, xg_video_XMS, vdx, vdy, &tAnim);
       // Je-li jeden frame, pak ho uvolnit
                       // tr.: if there is one single frame, free it
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
// tr.: for transparent animation:
//      (take???) value of transparent colour from index
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
                                          /* tr.: load header .GIF */
   if(nr < 13) return 4;

   if(strncmp((char *)GifGlb->sigGIF,"GIF",3) != 0)
   { return 6;
   }

   if(!gif->palismap)
    memset(gif->pal,0,768);

   glb_pix = (GifGlb->global & 0x07) + 1; /* Global pixel size */
   glb_pal = (GifGlb->global & 0x80);     /* Global palette  */
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

   nr = a_read(filx,&znacka,1); // Co za blok (tr.: what kind of block is it?)
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

    nr = a_read(filx,&znacka,1); // type
    if(znacka==0xF9)
     grcontrol=1;

    Cyk1:
    nr = a_read(filx,&znacka,1); // length 
    if(nr < 1) goto Err_end;
    if(znacka != 0)
    {
     len = znacka;
     nr = a_read(filx,bufx,len);
     if(nr < len) goto Err_end;
     if( grcontrol )
     {
      *GrControl = 1;
      memcpy(tAnim, &bufx[1], sizeof(short int));  // time of animation
      if(bufx[0]&1)  // Zadana transparence !
         // tr.: defined/defined transparency
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
                                           /* tr. interlaced picture */
    loc_pal   = GifImg->bits1 & 0x80;      /* Lokalni paleta   */
                                           /* tr. local palette */
    if(loc_pal != 0)
    {
     pix_size = (GifImg->bits1 & 0x07) + 1;
     len_pal = ( 1<<pix_size );
     if(!gif->palismap && GlbPal==0)  // neni mapa a neni globalni paleta (?)
               // tr.: there is no map and no global palette (?)
     { gif->npal=len_pal;
       nr = a_read(filx,gif->pal,len_pal*3);
       if(nr < len_pal*3) goto Err_rea;
       for(i=0; i<3*gif->npal; i++) gif->pal[i]>>=2;
       *ReaPal = 1;
     }
     else               // je mapa a mam dalsi loc. palety
               // tr.: there is a map and I have another/next local palette
     { // local palettes for next anim. frames
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
       // tr.: does not have palette (binar ?)
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
     //michani palet (tr.: mixing palettes)
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
    else //gif->pal is copy of Mmapio,
    {
      if(ReaPal != 2)
      { Mmapio[1]=(int *)(gif->pal);
      }
      else   // mam lokalni paletu
         // tr.: I have local palette
      {      // zadnou barvu nepridat, jen novou mapu
         // tr.: do not add any colour, only a/the new map
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
// tr.: preparation for scaling and whether draw or xms
int PrepareScaleAndX(struct picinfo *gif, int NumImg, int *Prepni)
{
//Return:  1: nejaka cast gifu padne do kreslici plochy
   // tr.: 1: some part of the GIF is located in the drawing area
//         0: cely obrazek je mimo kreslici plochu, dal nic nedelat
   // tr.: 0: the entire picture is beyond the drawing area, nothing to do
  int    dxo, dyo, relx;
#ifdef XANIMGIF
  long   Size, SizeXms, LenPix;
  int    dygif,dxgif;
#endif

  // Draw / save 
#ifdef XANIMGIF
  if(g_SaveGif && gif->is_background==0 && xg_256 >= 0x60)
  { // Do XMS : je povoleno, neni bckg, jen pro 256&Hicol
    // a ma mensi rozmery nez MAX_SAVEGIF (scale|normal)
    // uklada se cely (pripadne scalovany)      $$$
// tr.: Into XMS : it is allowed, there is/it is no background, only for 256&Hicol
//      and has smaller size than MAX_SAVEGIF (scale|normal)
//      it is saved entirely (if need be: scaled) $$$
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
      if(NumImg == 0)   // $$$ animation scale.gifu
      { g_resiz_x=gif->resize_x; g_resiz_y=gif->resize_y; // original size resize
        g_size_x =gif->size_x  ; g_size_y =gif->size_y;   // size of first GIF
      }
      else
      { if(gif->size_x==g_size_x && gif->size_y==g_size_y) // stejny jako prvni
               // tr.: the same as the first
        { gif->resize_x=g_resiz_x; gif->resize_y=g_resiz_y;
        }
        else  // jina (mensi) velikost => nove resize_ a offsety
              // tr.: different (smaller) size => new resize_ and offsets
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

    // Mam misto ? (tr.: do I have space?)
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
    { goto Obrazovka;
    }
    gif->x1gif = 0;
    gif->dxgif = dxo;
  }
  else
#endif
  { Obrazovka:
    g_gifDrawXms = 1;      // Obrazovka (tr.: screen)
    if(NumImg > 0)         // Uz jsou nejake framy v XMS -> uvolnit
                   // tr.: there are already frames in XMS -> release
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
      *Prepni = 1;       // tr.: Draw first of/from anim. on screen
    }
    else
    { *Prepni = 0;
    }
    //outs("Obrazovka");
    //getch();

    relx=gif->pic_x - gif->from_x;   // oriznuti z leva a zhora ?
      // tr.: extract/cut from left and upper ?
    gif->x1gif = relx;

    if(g_SetScale==0 || /*interlac != 0 ||*/ gif->is_background ||
       gif->resize_x==0 || gif->resize_y==0)
    { g_IsScale = 0;

      // Urcit oriznuti gifu (vyrez)
      // tr.: define extract/sector from GIF
      if((gif->pic_x + gif->size_x - relx - 1) <= gif->stop_x)
       gif->dxgif = gif->size_x - relx;
      else
       gif->dxgif = gif->stop_x - gif->pic_x + 1;
    }
    else
    { g_IsScale = 1;  // Neni background, interlac a je zadano resize_x,y
           // tr.: there is no background, interlac
           //      and resize_x,y was defined/given
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
// tr.: rounding double->int (pro >= 0)
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
// tr.: drawing of one uncompressed line of the GIF
int DrawLine(struct picinfo *gif, int Flags, char *palx,
             int yzg, char *Buf, char *Work, char *px16)
{
// yzg - line in GIF (from 0)
// Buf - uncompressed line of GIF
// Work- working buffer
// px16- for 16 and 1 colour modes (otherwise NULL)
// Flags : bit 0: transparent 0-no|1-yes
//         bit 1: map/palette 0-map|1-palette
//         bit 2: interlaced  0-no, 1-yes
   int xz, yz, dx, x1, i, iout, imgx, palindex;
   int nlin, ii, Transp, ist, intrlc;
   unsigned char         *pInp, *pWork, *pWr;
   unsigned short *Work2;
   int *pal;

   xz=gif->pic_x;   // misto na obrazovce (tr.: location on screen)
   imgx=xz+gif->dxgif-1;
   if(imgx>MAX_PIXELS)imgx=MAX_PIXELS;

   if(Flags&4) intrlc=1; else intrlc = 0;

   if(g_IsScale)      // gif do zadaneho obdelnika from_x,y, stop_x,y
            // tr.: GIF into the given rectangle from_x,y, stop_x,y
   { 
     nlin = Scale((unsigned char *)Buf, (unsigned char *)Work);  // buf->Work, nlin: how many lines to draw
     if(intrlc == 0)
     { if(nlin == 0) return( 1 );// nlin je promene radek od radku
              // tr.: nlin is variable line from lines
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
     pInp = (unsigned char *) Work;     // toto je ted vstup (vystup ze scalu)
            // tr.: this now is the entrance (exit from scale)
     pWork= (unsigned char *)Buf;       // vstupni buf je pouzit jako pracovni
            // tr.: initial buf is used as working buf
   }
   else     // SCALE = 1, kresleni puvodni velikosti, oriznuti
            // tr.: SCALE = 1, drawing in original size, cutting off
   { nlin = 1;
     yz=gif->from_y + yzg;
     pInp = (unsigned char *)Buf;
     pWork = (unsigned char *)Work;
   }

   x1 = gif->x1gif;                         // cutting off rows in GIF
   dx = gif->dxgif;                         // size of img
   Work2 = (unsigned short *)pWork;
   pWr= (unsigned char *)Work2;

   for(ii = 0; ii<nlin; ii++)                 // for scale can be nlin > 1
   {
    if(g_gifDrawXms==1)                       // for save to Xms everything 
    { if(yz < gif->pic_y ) goto Next_line;    // ignore row + continue
      if(yz > gif->stop_y)
      { if((Flags&4)==0)
         return( 0 );       // terminate drawing
        else
         goto Next_line;    // interlaced
      }
    }

    if((Flags&1) && (g_gifDrawXms==1))  // transparent + drawing
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

    // repallettizing + overwrite non-transparent pixels
#ifdef HICOLOR
    if(xg_256 == MM_Hic)     // HiCol modes
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
    else                     // 256,16 atd modes
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
                                // tr.: draw right now
    }
#ifdef XANIMGIF
    else
    { ist = XSaveImgLine((char *)pWr, gif->from_y, yz);   // uloz radku do XMS
                                // tr.: save row to XMS
      if(ist != 1) return( ist );
    }
#endif
    Next_line:
    yz++;
   } // end for nlin

   if(gif->is_background) // pozadi  (nikdy se scale!)
                          // tr.: background (sometimes with scale!)
   { 
     mosaic_background(gif, (char *)pWr, yz-1, imgx);
   }

   if(g_IsScale) g_yzscale += nlin;
   return( 1 );
}

// Fill an area with background
/*

//moved to v_putimg.c

void mosaic_background(struct picinfo *gif,char *obuf,int yz,int imgx)
{
 int pomx=0;
 int pomy=yz+gif->size_y;

 //dlazdicky: (tr.: texture? - literally it means paving stones)
 //while(pomx+imgx<=gif->screen_x+gif->draw_x) //HARO stop_x > draw_x ??
 //mp: stop_x je v souradnicich obrazovky!
 // tr.: stop_x is in coordinates of the screen!
// while(pomx+imgx<=gif->screen_x+gif->stop_x)
 while(pomx+imgx<=gif->stop_x)
 {
  while(pomy<gif->screen_y+gif->draw_y) //<-tady by mohlo/melo byt stop_y
   // tr.: here could/should be stop_y
  {
   v_putimg(gif->screen_x+pomx,pomy,obuf);
   pomy+=gif->size_y;
  }
  pomy=yz;
  pomx+=gif->size_x;
 }

 //dokresleni: (tr.: finish drawing)
 pomx=gif->draw_x%gif->size_x;
 //mp: 0 je taky blbost!!!! (tr.: 0 is also nonsense!)
 if(pomx >= gif->draw_x || !pomx) return;    //HARO : optimalization

 pomy=yz;
 while(pomy<gif->screen_y+gif->draw_y)
 {
  v_getimg(gif->screen_x ,pomy,gif->screen_x +pomx,pomy,obuf);
  v_putimg(gif->screen_x +gif->draw_x-pomx,pomy,obuf);
  pomy+=gif->size_y;
 }
}//end if background

*/
/*----- Dekodovoani GIF dat (LZW) ---------------*/
/*  tr.: decoding GIF data */
/*----------- Pouzita makra (misto ppg) ---------*/
/*  tr.: used markos (instead of ppg) */
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
                            /* tr.: uncompressed/uncompressing table */
   int           *ctlink=NULL;
   unsigned char *ostack=NULL;   /* Docasny vystup (tr. tempor. output)  */
   char *obuf=NULL,*buf1=NULL,*buf2=NULL;  /* Vystupni radek  PRO PUTIMG */
                       /* tr.: output line FOR PUTIMG */
   char *px16=NULL;

   unsigned char *ibuf=NULL;       /* Vstupni buffer (tr.: input buffer) */
   int      ibx,bufct;             /* Index+blok       */
   long     RByts = 0;
   int      obufx=0;

   int cmask[9] = {0x0000,0x0001,0x0003,0x0007,0x000F, /* Maskovani */
                                                      /* tr.: masking */
                   0x001F,0x003F,0x007F,0x00FF};

   int inctable[5] = {8,8,4,2,1};  /* Interlaced picture */
   int startable[5]= {0,4,2,1,0};
   int interlace=0;

   int   Flags;
   char *paldraw;

   int code,oldcode,clearcode,nextcode=0,nextlim=0,eoi;
   int rem=0,remct,reqct,done,ax,req1,co1,co2;
   int i,jx=0,ire=1,ist;

   /*--------------- Allocation) --------------------------*/
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

   /*--------------- Initialization -----------------*/
   clearcode = 1 << cod_start;
   eoi       = clearcode + 1;
   reqct     = cod_start + 1;

   oldcode   = -1;
   done = 0;

   ibx  = bufct= 0;
   remct = 0;

   /*-- Hlavni cykl pres kody (tr.: main loop through/on codes)----*/

   HLAVNI_CYKL:
   /*-- Ziskani dalsiho kodu ze vstupu (tr.: getting more code from input)-*/

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
    req1 = reqct - 8;        /* Co zbylo (tr.: what remains)*/

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

   /*--------- For clearcode -> reinit tables --------*/
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
   /*--------- Decoding inp codes --------------------*/
   else
   {
    if(code == eoi)
    {
     done = -1;
     goto End_d;
    }

    if(ctlink[code] == -2)  /* Kod neni v tab (tr.: code is not in table) */
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
    else                    /* Kod je v tab (tr.: code is in table) */
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
    /*--- Convert code to string -------------*/
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

      if(obufx >= dx)   // End of row
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
      }       // end if end of row
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
               // tr.: finish reading until end of data in picture
   }
   End_data:
   *ReaBytes = RByts; // pocet bytu zkomprim. obrazu
               // tr.: number of bytes in compressed picture

   if(ibuf)farfree(ibuf);
   if(ostack)farfree(ostack);
   if(ctfirst)farfree(ctfirst);
   if(ctlink)farfree(ctlink);
   if(ctlast)farfree(ctlast);

   if(px16)farfree(px16);
   if(buf2)farfree(buf2);
   if(buf1)farfree(obuf);

   return( ire ); /*--------- O.K. end (EOI) ----*/

   Err_rea: // predcasny konec dat (tr.: premature end of data)
   ire = 2; goto End_data;

   Err_mem: // there is no memory 
   ire = 2; goto End_data;
}



// -------------- SCALING PICTURE : 8 bits/pixel
#define  POZADI 0xFFFF
#define  MSB    0x80
// global variable for scaling
int           scl_kx, scl_px, scl_nx, scl_tx, scl_ex;
int           scl_ky, scl_py, scl_ny, scl_ty;
unsigned char *scl_buf = NULL, scl_mode = 0;


//. Called by drawgif
int OpnScale(int x1, int y1, int x2, int y2)
{
// Parameters: x1, y1 ... extension of original picture
//             x2, y2 ... extension of requested picture
// Return   : 1 ........ OK
//            2 ........ allocation error (possibly only because scl_mode != 0)
   scl_yy = (double)y2 / (double)y1;   // only for interlaced gifs

   if ( (scl_kx = ( x1 < x2 )) != 0 )  // increase in x direction
   {
     scl_px = 2 * x1;
     scl_nx = 2 * x2 - scl_px;
     scl_tx = x2 - scl_px;
     for ( ; ; ) { if ( scl_tx < 0 ) { scl_tx += scl_nx; break; }
                   else                scl_tx -= scl_px;
                 }
     scl_ex = x2;
     scl_mode = 0;                     // there is no sense in integrating 
   }
   else                                // decrease in x direction
   {
     scl_px = 2 * x2;
     scl_nx = 2 * x1 - scl_px;
     scl_tx = x1 - scl_px;
     scl_ex = x1;
   }

   if ( (scl_ky = ( y1 < y2 )) != 0 )  // increase in y direction
   {
     scl_py = 2 * y1;
     scl_ny = 2 * y2 - scl_py;
     scl_ty = y2 - scl_py;
     for ( ; ; ) { if ( scl_ty < 0 ) { scl_ty += scl_ny; break; }
                   else                scl_ty -= scl_py;
                 }
   }
   else                                // decrease in y direction
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
// Parameters: inbuf  ... row of original picture
//             outbuf ... row of requested picture
// Return    : how much times repeat output row ( n >=0 )
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
                           // tr.: initializing for next row
       }
       else uch = *inbuf;

       if ( tx < 0 )
       {
         tx += scl_nx;
         *outbuf++ = uch;
         inbuf++;                 // kopirovani vstupniho sloupce
                          // tr.: copying entering column
         uch = POZADI;
       }
       else
       {
         tx -= scl_px;
         if ( scl_kx )
              *outbuf++ = uch;    // opakovani vstupniho sloupce
                         // tr.: repeating entering column
         else inbuf++;            // vypusteni vstupniho sloupce
                         // tr.: leaving out entering column
       }
     }
     return(i);                   // vystup prevedeneho radku, i-krat
                         // tr.: output converted row, i times
   }
   else
   {
     scl_ty -= scl_py;
     if ( scl_mode )             // jalovy radek   (tr.: empty/idle row)
     {
       for ( j = 0; j < scl_ex; j++)
       {
         scl_buf[j] = min( scl_buf[j], *inbuf );
         inbuf++;
       }
     }
     if ( !scl_ky ) return(0);    // vypusteni vstupniho radku
                        // tr.: leaving out entering row
   }
   goto Zas;
}

// For Hi-color row : only in case scl_mode=0 !
int ScaleHI( unsigned short *inbuf, unsigned short *outbuf)
{
// Parameters: inbuf  ... row in original picture
//             outbuf ... row in requested picture
// Return    : how many times repeat output row ( n >= 0)
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
                        // tr.: copy entering column
         uch = POZADI;
       }
       else
       {
         tx -= scl_px;
         if ( scl_kx )
              *outbuf++ = uch;    // opakovani vstupniho sloupce
                       // tr.: repeat entering column
         else inbuf++;            // vypusteni vstupniho sloupce
                       // tr.: leave out entering column
       }
     }
     return(i);                   // vystup prevedeneho radku, i-krat
                       // tr.: output of converted row, i times
   }
   else
   {
     scl_ty -= scl_py;
     if ( !scl_ky ) return(0);    // vypusteni vstupniho radku
                       // tr.: leave out entering row
   }
   goto Zas;
}


void far PreScale( int x1, int y1, int *x2, int *y2)
{
// Parameters: x1, y1 ... dimensions of original picture (input)
//             x2, y2 ... dimensions of requested picture (input/output)
// Function  : if width/height relation does not correspond to input relation,
//             one side will be reduced 
  long  a, b;

  a = (long) x1 * (long) *y2;
  b = (long) y1 * (long) *x2;

  if ( a > b ) *y2 = (int) ( b / x1 );
  else         *x2 = (int) ( a / y1 );
}


// ========================================================================
// Arachne Icons (.IKN) library
// (c)1997,1998,1999,2000 Zdenek Harovnik + Michael Polak, Arachne Labs
// ========================================================================

// HARO verze : 02.02.1998
// Verze : vyuziva ie_swap... funkce
// verze bez messages, pro hru universe a pro Arachne WWW Browser

#include "arachne.h"
#include "v_putimg.h"
#include "putikons.h"

void memerr(void);
int  PresspalO (int multip, char *Palin[], int *Npalin, char  *palout,
	       int *npalout, int *mapio, int *Mmapio[],
	       int Swinout, int TypFuse, int Tolerance,
	       char *Savecols );
void z_bitbyte(unsigned char *buf1, unsigned char *buf2, int delka);
int  z_bytebit(char *inp, char *out, int npix);
int  bit_pix8(char *sourc, char *dest, int npix, int nbit);
void x_img256to2(unsigned char *bi, unsigned char *bo);
void x_img256to16(unsigned char *bi, unsigned char *bo);
int v_transpimg(int x0, int y0, unsigned char *bi1, int Transp);

int ie_swap(int newswap);

extern int swapmod;   //priznak modifikace swapovane pameti

#define BUFERSIZE 16100l

#define MAX_ICNINDEX 120         // max. ikon v seznamu
#define MAX_ICNLIST  480         // max. pocet ikon v draw listu
#define MAX_ICONSWAP 16384       // buffer pro N-ikon (swap)

#define DRAW_LIST_IKN 16         // optimalizace kresleni seznamu

//#define IKNDEBUG printf("Line: %d\n",__LINE__)
#define IKNDEBUG

// Ikony v indexu ikon (je jich max MAX_ICNINDEX)
struct IknIndex
{ char iconame[80];      // jmeno ikony
  XSWAP  hSwap;    // handle swapu s bufrem
  unsigned int  Off;      // offset ve swapu
};

// seznam ikon pro hromadne vykresleni
struct IknList
{  XSWAP hSwap;
   unsigned int Off;
   int          x0;
   int          y0;
};

#ifdef POSIX
struct IknIndex *IknPtr=NULL;
#else
XSWAP g_hIndex=IE_NULL;    // handle swapu s indexem a listem ikon
#endif
unsigned int g_AktIndex=0;  // prvni volne misto pro pridani ikony do indexu
unsigned int g_BegList; // Offset v B na zacatek listu v bufru
unsigned int g_AktList; // Prvni volny IknList

XSWAP g_AktImgSwap = IE_NULL; // handle swapu pro pridavani
unsigned int g_FreeOff = 0;       // kolik mam mista ve swapu pro IMG
unsigned int g_maxswap = 0;       // velikost bufru pro ikony
unsigned int g_AktPalLen = 0;     // pamatovani si minule delky palety
//int          g_UpIcnData = 0;     //no longer used
char         g_LastPal[768];      // minula paleta (pro porovnani s akt)
//int          g_UpAllIcn = 0;      // pocet ikon nad FIRSTHTMLSWAP

int          g_SetTransp= 1;      // Kreslit transparentne$$

// ----- Interni pomocne fce pro ikony
void freeinkons(void);
void DrawIcon1(int x0, int y0, XSWAP int hSwap, unsigned int Off);
int  PrepareIconDraw(XSWAP hSwap, unsigned int Off, char **bo, int *TrCol);
int  GetListNext(int From, int Kolik, struct IknList *List);
int  SortListIcons(void);
void RepalIcon1(short int mcol, short int mrow, unsigned char *bwork, int
*Mmapio[]);
void RepalIcon2(short int mcol, short int mrow, unsigned char* bwork,
		unsigned char* bio, int *Mmapio[]);
int  MergePalIcons(int npalikn, char *palin, int *mapio, int *Mmapio[],
		  int *Mapujo);

// ------ Verejny interface na ikony
void InitIcons(void);
void DrawIcons(void);
void DrawIconLater(char *iconame,int x0, int y0);
void DrawIconNow(char *iconame,int x0, int y0);
//void UpdIconIndex(void);

//##----------- Inicializace
void InitIcons(void)
{
  unsigned int LenBuf;

  if(!egamode && !vga16mode)
   IiNpal=0;             // Zadna paleta
  g_AktPalLen = 0;
  memset(g_LastPal, 0, 768);

#ifdef POSIX
  IKNDEBUG;
  if(IknPtr == NULL)     //alokace mista pro index
  {
    g_BegList = MAX_ICNINDEX * sizeof(struct IknIndex);
    LenBuf    = g_BegList + MAX_ICNLIST * sizeof(struct IknList);
    IknPtr = malloc(LenBuf);
    if(!IknPtr)
     memerr();
    //printf("%d of memory allocated for icon index....\n",LenBuf);
#else
  if(g_hIndex == IE_NULL)     //alokace mista pro index
  { g_BegList = MAX_ICNINDEX * sizeof(struct IknIndex);
    LenBuf    = g_BegList + MAX_ICNLIST * sizeof(struct IknList);

   //printf("allocating XSWAP for index..\n");
   g_hIndex = ie_putswap("",LenBuf,CONTEXT_ICONS);
#endif

    g_AktIndex = 0;      // Pri opetovnem zavolani InitIcons se index
    g_AktList  = 0;      // pouzije znova od zacatku
    g_maxswap  = 0;
    g_FreeOff  = 0;

  }

  freeinkons();        // ale co IMG swapy ?
}

//##----------- Pridani do seznamu
//You have to call DrawIcon to draw
void DrawIconLater(char *iconame,int x0, int y0)
{
  Putikonx(x0, y0, iconame, 0);
}

//##----------- Kresleni ted hned
void DrawIconNow(char *iconame,int x0, int y0)
{
  Putikonx(x0, y0, iconame, 1);
}

//##----------- Vykresleni ikon v seznamu a jeho zruseni
void DrawIcons(void)
{
  int      nIcon, From, i, ist, Transp;
#ifdef POSIX
 struct   IknList *List;
#else  
  struct   IknList List[DRAW_LIST_IKN];
#endif
  XSWAP YhSwap, YOff;
  char     *bi1 = NULL;

  if((xg_256 != MM_256 && !xg_video_XMS) || (xg_256 == MM_Hic)) //HARO
  { if(g_AktPalLen == IiNpal)
    { if(memcmp(Iipal, g_LastPal, 3*IiNpal) == 0 ) goto Ident1;
    }
    x_palett(IiNpal, Iipal);
    g_AktPalLen = IiNpal;
    memcpy(g_LastPal, Iipal, 3*IiNpal);
    Ident1:;
  }

  YhSwap = YOff = 0xFFFF;

#ifndef POSIX
  // sort list ikon
  //not really needed with linear access to memory
  i = SortListIcons();
  if(i != 1) return;
#endif
  g_AktPalLen = IiNpal;
  memcpy(g_LastPal, Iipal, 3*IiNpal);

  swapmod = 1;
  From =  0;

#ifdef POSIX
  List=( struct   IknList *)((void *)IknPtr+g_BegList);
  nIcon=g_AktList;
#else
  Next_draw:
  nIcon = GetListNext(From, DRAW_LIST_IKN, List);   // Optimalizace swapovani
#endif

  if(nIcon > 0)
  {
   for(i=0; i<nIcon; i++)
   { if(xg_256 != MM_256)
     { 
      //printf("Drawing icon No. %d at %d, %d\n",i,List[i].x0, List[i].y0 );
      DrawIcon1(List[i].x0, List[i].y0, List[i].hSwap, List[i].Off);
     }
     else
     { if(!(YhSwap == List[i].hSwap && YOff == List[i].Off))
       { if(bi1 != NULL) farfree(bi1);

	 ist = PrepareIconDraw(List[i].hSwap, List[i].Off, &bi1, &Transp);
	 if(ist != 1) goto cont1;
	 YhSwap = List[i].hSwap;
	 YOff = List[i].Off;

	 if(!xg_video_XMS || xg_256 == MM_Hic)
	 { 
	   if(g_AktPalLen == IiNpal)
	   { if(memcmp(Iipal, g_LastPal, 3*IiNpal) == 0) goto Ident2;
	   }
	   x_palett(IiNpal, Iipal);
	   g_AktPalLen = IiNpal;
	   memcpy(g_LastPal, Iipal, 3*IiNpal);
	   Ident2:;
	 }
       }
#ifdef TRANSPARENT_ICONS
       if(Transp >= 0 && g_SetTransp > 0) // Transparentni ikona $$
       { v_transpimg(List[i].x0, List[i].y0, (unsigned char *)bi1, Transp);
       }
       else
#endif
       { 
  IKNDEBUG;
         v_putimg(List[i].x0, List[i].y0, bi1);
       }
     }
    cont1:;
   }
   From += nIcon;
#ifndef POSIX
   goto Next_draw;
#endif
  }

  if(bi1 != NULL) farfree(bi1);
  freeinkons();
}

// Volat pri uvolneni swapu nad FIRSTHTMLSWAP
// Oznaci nektere ikony v indexu za neplatne


// -------- INTERNI FCE --------------------------
int PrepareIconDraw(XSWAP hSwap, unsigned int Off, char **bo, int *TrCol)
{
     char *BufImg,*bi1, *palin, *boo;
     int  npalikn,Mapuj;
     short int mcol,mrow;
     int  ist, Transp;
     int   *mapio, *Mmapio[2], *mapio2;

     BufImg = ie_getswap(hSwap);
     if(!BufImg)
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return 0;
//      MALLOCERR();
//!!glennmcc: end

     mapio=farmalloc(512*sizeof(int));
     if(mapio == NULL) return( 0 );

     memcpy(&npalikn, BufImg+Off, sizeof(int));
     memcpy(&Transp, BufImg+Off+sizeof(int), sizeof(int));
     palin = BufImg + Off + 2*sizeof(int);
     bi1   = BufImg + Off + (2*sizeof(int) + npalikn*3);

     ist = MergePalIcons(npalikn, palin, mapio, Mmapio, &Mapuj);
     if(ist != 1) return( 0 );

     memcpy(&mcol, bi1, sizeof(short));
     memcpy(&mrow, bi1+2, sizeof(short)); //16 bit integers
     //??mcol=(short int) *bi1;
     //??mrow=(short int) *(bi1+2);
     boo = farmalloc(mcol*mrow+16);
     if(boo == NULL) return( 0 );

     if(Mapuj)
     { memcpy(boo, &mcol, sizeof(short)); 
      memcpy(boo+2, &mrow, sizeof(short)); //16 bit integers!!!
       RepalIcon2(mcol, mrow, (unsigned char*)bi1, (unsigned char*)boo, Mmapio);
       if(Transp >= 0)
       { mapio2=Mmapio[1];
	 *TrCol = mapio2[Transp];
       }
       else
       { *TrCol = Transp;
       }
     }
     else
     { memcpy(boo, bi1, mcol*mrow+4);
      *TrCol = Transp;
     }
     farfree(mapio);
     *bo = boo;

     return( 1 );
}


void DrawIcon1(int x0, int y0, XSWAP hSwap, unsigned int Off)
{
    char *BufImg, *bi1, *bo=NULL;
    int ist,  Transp = -1;
#ifdef TRANSPARENT_ICONS
    int i;
    short int *bi2, HiTransp;
#endif

    if(xg_256 != MM_256)       // mam pripraveny IMG ve swapu
    {
      if(!xg_video_XMS || xg_256 == MM_Hic)
      { if(g_AktPalLen == IiNpal)
	{ if(memcmp(Iipal, g_LastPal, 3*IiNpal) == 0) goto Ident1;
	}
	x_palett(IiNpal, Iipal);
	g_AktPalLen = IiNpal;
	memcpy(g_LastPal, Iipal, 3*IiNpal);
	Ident1:;
      }

      BufImg = ie_getswap(hSwap);
      if(!BufImg) //return on error !
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return;
//       MALLOCERR();
//!!glennmcc: end

      bi1    = BufImg + Off;

#ifdef TRANSPARENT_ICONS
      if(xg_256 == MM_Hic)
      {
        bi2 = (short *)bi1;
	i = 2 + bi2[0] * bi2[1];
      	if(bi2[i] != 0)  // Hicolor transparentni barva
	{ HiTransp = bi2[i+1]; Transp = 1;
	}
      }

      if(Transp >= 0 && g_SetTransp > 0) // Transparentni ikona $$
      {
       //printf("Icon is transparent?\n");
       v_transpimg(x0, y0, (unsigned char*)bi1, HiTransp);
      }
      else
#endif
      {
/*
       int w;
       int h;
    
       w = *(short int *)bi1;
       h = *(short int *)&bi1[sizeof(short int)];
       printf("In DrawIcon1, x=%d, y=%d, w=%d, h=%d\n",x0,y0,h,w);
       IKNDEBUG;
     
//       if(x0>=0 && x0+w<=x_maxx() && y0>=0 && y0+h<=x_maxy())
*/
        v_putimg( x0, y0, bi1); 
      }
    }
    else                       // v 256 color modu musim IMG vyrobit
    { g_AktPalLen = IiNpal;
      memcpy(g_LastPal, Iipal, 3*IiNpal);

      ist = PrepareIconDraw(hSwap, Off, &bo, &Transp);
      if(ist != 1) return;

      if(!xg_video_XMS || xg_256 == MM_Hic)
      { if(g_AktPalLen == IiNpal)
	{ if(memcmp(Iipal, g_LastPal, 3*IiNpal) == 0) goto Ident2;
	}
	x_palett(IiNpal, Iipal);
	g_AktPalLen = IiNpal;
	memcpy(g_LastPal, Iipal, 3*IiNpal);
	Ident2:;
      }

#ifdef TRANSPARENT_ICONS
      if(Transp >= 0 && g_SetTransp > 0) // Transparentni ikona $$
      { v_transpimg(x0, y0, (unsigned char*)bo, Transp);
      }
      else
#endif      
      { 
  IKNDEBUG;
        v_putimg(x0, y0, bo);
      }
      if(bo != NULL) farfree(bo);
    }
}

void freeinkons(void)   //uvolneni seznamu ikon
{
   g_AktList  = 0;
}

// Zda je ikona v indexu
int IsInIndex(char *iconame, XSWAP *hSwap, unsigned int *Off,
	      int *InxDel)
{
    char  *BegSwap;
    struct IknIndex *IknInx;
    int    i;

#ifdef POSIX
    IKNDEBUG;
    IknInx=IknPtr;
#else
    BegSwap = ie_getswap(g_hIndex);
    if(!BegSwap)
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return NULL;
//     MALLOCERR();
//!!glennmcc
    IknInx = (struct IknIndex *)BegSwap;
#endif     

    for(i=0; i<g_AktIndex; i++)
    { if(strcmpi(iconame, IknInx[i].iconame) == 0)
      { goto Mam_ji;
      }
    }
    *InxDel = -1;
    return( -1 );

    Mam_ji:
    *hSwap = IknInx[i].hSwap;
    *Off   = IknInx[i].Off;
    /*
    if(IknInx[i].Del != 0)
    { *InxDel = i;
      return( -1 );
    }
    else
    */
    { *InxDel = -1;
      return( i );
    }
}

// Prida ikonu do seznamu pro pozdejsi vykresleni
int AddToList(int x0, int y0, int JeVinx)
{
    char *BegSwap;
    struct IknIndex *IknInx;
    struct IknList  *IknLst;

    if(g_AktList >= MAX_ICNLIST) return( -1 );


#ifdef POSIX
    IKNDEBUG;
    IknInx = IknPtr;
    IknLst = (struct IknList *)((void *)IknPtr+g_BegList);
#else
    BegSwap = ie_getswap(g_hIndex);
    if(!BegSwap)
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return NULL;
//     MALLOCERR();
//!!glennmcc: end
    IknInx = (struct IknIndex *)BegSwap;
    IknLst = (struct IknList *)(BegSwap+g_BegList);
#endif

//    printf("g_AktList=%d, g_BegList=%d, IknLst[0].x0=%p, x0=%d\n",g_AktList,g_BegList,&(IknLst[0].x0),x0);
    IknLst[g_AktList].x0 = x0;
    IknLst[g_AktList].y0 = y0;
    IknLst[g_AktList].hSwap = IknInx[JeVinx].hSwap;
    IknLst[g_AktList].Off = IknInx[JeVinx].Off;
    g_AktList++;

    return( 1 );
}

// Prida ikonu do indexu
int  AddIconToIndex(char *iconame, XSWAP hSwap,
		    unsigned int Off, int AddOrRw)
{
// AddOrRw : -1 = pridat na konec indexu
//          >=0 = prepsat tento index
    char *BegSwap;
    struct IknIndex *IknInx;

    if(g_AktIndex >= MAX_ICNINDEX) return( -1 );

#ifdef POSIX
    IKNDEBUG;
    IknInx = IknPtr;
#else
    BegSwap = ie_getswap(g_hIndex);
    if(!BegSwap)
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return NULL;
//     MALLOCERR();
//!!glennmcc: end
    IknInx = (struct IknIndex *)BegSwap;
#endif     

    if(AddOrRw < 0)  // pripsat do indexu
    {
    IknInx[g_AktIndex].hSwap = hSwap;
    IknInx[g_AktIndex].Off   = Off;
/*
    IknInx[g_AktIndex].Up    = Up;
    IknInx[g_AktIndex].Del   = 0;
*/
    strcpy(IknInx[g_AktIndex].iconame, iconame);

    g_AktIndex++;
    return(g_AktIndex-1);
    }
    else   // prepsat jiz existujici
    {
    IknInx[AddOrRw].hSwap = hSwap;
    IknInx[AddOrRw].Off   = Off;
/*
    IknInx[AddOrRw].Up    = Up;
    IknInx[AddOrRw].Del   = 0;
*/
    return(AddOrRw);
    }
}

// Vraci poitr na buffer + handle a offset
// Predpoklada se pouze pridavani !
//. low level funtion for icon management
char *MemIconImg(unsigned int Len, XSWAP *hSwap, unsigned int *Off, int *Up)
{
   char *BufImg,*RetMem;
   unsigned int FreeMem;

   FreeMem = g_maxswap - g_FreeOff;

   if(Len > FreeMem)    // V akt. swapu uz neni misto -> zalozit novy
   {  g_maxswap = max(Len, MAX_ICONSWAP);
      // no lonnger needed: ie_swap(0);
      //printf("allocating XSWAP for icon..\n");
      g_AktImgSwap = ie_putswap("",g_maxswap,CONTEXT_ICONS);
//      g_UpIcnData = 0;
      g_FreeOff = 0;
   }

   BufImg = ie_getswap(g_AktImgSwap);
   if(!BufImg)
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return NULL;
//     MALLOCERR();
//!!glennmcc: end

   RetMem = BufImg + g_FreeOff;
   *hSwap = g_AktImgSwap;
   *Off   = g_FreeOff;
//   *Up    = g_UpIcnData;
   *Up    = 0;

   g_FreeOff += Len;
   return( RetMem );
}

// Slucovani palet ikon
//.Pal : Pallete
int MergePalIcons(int npalikn, char *palin, int *mapio, int *Mmapio[],
		  int *Mapujo)
{
  int   maxpalview, Mapuj, irc, n1, i;
  char *Palin[2];
  int   Npalin[2];
  int   npalout;
  char *Savecols/*[512]*/;

  if(xg_256 == MM_Hic)
   maxpalview= 256;
  else
   maxpalview= x_getmaxcol() + 1;
  Mapuj=0;
  Savecols=farmalloc(256*sizeof(int));
  if(Savecols == NULL) 
  {
   //printf("Could not allocate ?!\n");
   return( 0 );
  }

  if(IiNpal<=0)     //1.ikona
  { IiNpal= npalikn;
    memmove(Iipal,palin, 3 * IiNpal);
    if(IiNpal<=maxpalview) goto Kresli;
  }

  if(egamode)
  {
   int i=0;
   while(i<npalikn*3)
   {
    palin[i]=egafilter(palin[i]);
    i++;
   }
  }
  if(vga16mode && !vgamono)
  {
   int i=0;
   while(i<npalikn*3)
   {
    palin[i]=vgafilter(palin[i]);
    i++;
   }
  }

  // Slucovani palet
  Palin[0]= Iipal; Palin[1]= palin;
  Npalin[0]= IiNpal; Npalin[1]= npalikn;
  npalout= maxpalview;
  n1=min(IiNpal,maxpalview); //(muze zabrat pri 1.ikone)
  memset( Savecols, 1, n1); memset( &Savecols[n1], 0, npalikn);
  irc=PresspalO(2, Palin, Npalin, Iipal,
		&npalout, mapio, Mmapio,
		2, 1, 4,  Savecols );
  if((irc&1)==0) { ; }      // Error ?!

  if(!xg_video_XMS || xg_256 == MM_Hic) //!!mp
  {
   if(npalout > IiNpal)      // Nove barvy
   { for(i=IiNpal; i<npalout; i++) x_pal_1(i, &Iipal[i]);
   }
  }
  IiNpal=npalout; Mapuj=1;

  Kresli:
  *Mapujo = Mapuj;
  farfree(Savecols);
  return( 1 );
}

// vrati ikonu v 256 color modu (bud alokuje, nebo je to puvodni bi)
unsigned char *TransferTo256(short mcol, short mrow, unsigned char *bi,
	      int inp16, int *Nbworko)
{
// Nbwork: 0 == vraceny buffer je vstupni, jinak delka alloc bufru
 int ikkk, Nbwork, len8, len2, i, j;
 unsigned char *bwork = NULL;

 if(inp16 == 1)  // 16 color IKN
 {
   // transfer do 256
   Nbwork = (int)mcol * (int)mrow + 16;           // delka prac. bufru
   //printf("Allocating: mcol=%d,mrow=%d\n",mcol,mrow);
   bwork  = farmalloc(Nbwork);
   if(bwork == NULL) 
   {
    //printf("malloc failed! Nbwork=%d\n",Nbwork);
    goto No_mem;
   }
   
   ikkk=4;
   memcpy(bwork, &mcol,sizeof(short)); 
   memcpy(&bwork[2], &mrow,sizeof(short));
   len2= (mcol+7)/8; len8= len2 * 8;
   for(i=0,j=4;i<mrow;i++)
   { bit_pix8((char *)&bi[j], (char *)&bwork[ikkk], len8, 4);
     j += 4 * len2; ikkk += mcol;
   }
 }
 else           // 256 color IKN
 { bwork = bi;
   Nbwork = 0;
 }

 No_mem:
 *Nbworko = Nbwork;
 return( bwork );
}

//. Repal : change pallete
void RepalIcon1(short mcol, short mrow, unsigned char *bwork, int *Mmapio[])
{
   int *mapio2, ikk, i, j;

   if(!bwork || !Mmapio || !Mmapio || !Mmapio[1])
    return; // to avoid SIGSEGV

   mapio2=Mmapio[1];
   ikk= ((int)mcol * (int)mrow) + 4;
   //printf("RepalIcon1: mcol=%d,mrow=%d,ikk=%d\n",mcol,mrow,ikk);  
   for(i=4;i<ikk;i++)
   { j= bwork[i]; bwork[i]= mapio2[j];
   }
}

void RepalIcon2(short int mcol, short int mrow, unsigned char* bwork,
		unsigned char* bio, int *Mmapio[])
{
   int *mapio2, ikk, i, j;

   mapio2=Mmapio[1];
   ikk= (mcol * mrow) + 4;
   for(i=4;i<ikk;i++)
   { j= bwork[i]; bio[i]= mapio2[j];
   }
}

#ifdef TRANSPARENT_ICONS
//. transparent icons.  not used by Arachne yet
int v_transpimg(int x0, int y0, unsigned char *bi1, int Transp)
{
   unsigned int  ncol, nrow, i, nEnd;
   unsigned short int HiTransp, *getHiBit, *bi1Hi;
   unsigned char ChTransp;
   unsigned char *getBit;
   long Size;

   bi1Hi = (unsigned int*)bi1;
   ncol = bi1Hi[0];
   nrow = bi1Hi[1];

   if(xg_256 == MM_256)
   { ChTransp = (unsigned char)Transp;
     Size = (long)ncol * (long)nrow + 8;
     nEnd =  ncol*nrow+4;
   }
   else if(xg_256 == MM_Hic)
   { HiTransp = Transp;
     Size = 2*(long)ncol * (long)nrow + 8;
     nEnd =  ncol*nrow+2;
   }
   else
   { return( 10 );
   }

   getBit = farmalloc(Size);
   if(getBit == NULL) return( 2 );

   v_getimg(x0, y0, x0+ncol-1, y0+nrow-1, (char*)getBit);
   getHiBit = (unsigned int*)getBit;

   if(xg_256 == MM_256)
   {  for(i = 4; i<nEnd; i++)
      { if(bi1[i] != ChTransp)
	{ getBit[i] = bi1[i];
	}
      }
   }
   else
   {  for(i = 2; i<nEnd; i++)
      { if(bi1Hi[i] != HiTransp)
	{ getHiBit[i] = bi1Hi[i];
	}
      }
   }
   v_putimg(x0, y0, (char*)getBit);
   farfree(getBit);
   return( 1 );
}
#endif
//##---------------------------------------------------------------------

//nacteni 1 ikony do pameti, pripadne vykresleni
void Putikonx(int x0, int y0, char *iconame, char noswap)
//priznak noswap=1 -> rovnou vykresli
//              =0 -> jen prida do seznamu
{
  short int ip1;
  int ik,len1;
  short int mcol,mrow;
  int inp16,npalikn;
  int   irc,Mapuj, Npalw;
  int   *mapio/*[512]*/, *Mmapio[2];
  char  *palin;
  unsigned short nbufo;
  unsigned char *bi1;
  short *bi2;
  int  fin, Nbwork, lenpix, Nbufw;
  char filnam[77];
#ifndef POSIX
  char dir[77],drive[4];
#endif
  char dummy[10],iconame2[10];
  char          *bufw= NULL;
  unsigned char *bi=NULL;
  unsigned char *bwork = NULL;
  char          *palcpy = NULL;
  XSWAP   hSwap = 0;
  unsigned int Off = 0;
#ifdef HICOLOR
  unsigned short int   *biHi = NULL;
  int i;
#endif
  int      JeVinx, AddInx, InxDel, UpIcnMem;
  int      Transp;      // Transparentni ikony

  // -------------------------------------------------
  strcpy(filnam, iconpath);
#ifdef POSIX
  {
   char *ptr=strrchr(iconame,'/');
   if(ptr)
    strncat(filnam,&ptr[1],12);
   else
    strncat(filnam,iconame,12);
   filnam[76]='\0';
  }
  strlwr(filnam);
  if(!strstr(filnam,".ikn"))
   strcat(filnam,".ikn");
//  printf("Ikon to draw: %s at x=%d,y=%d\n",filnam,x0,y0);
#else
  fnsplit(iconame, drive,dir, iconame2, dummy);
  fnsplit(filnam, drive, dir, dummy, dummy);
  fnmerge(filnam, drive, dir, iconame2,".ikn");
#endif
  // Zda je jiz v indexu, nebo -1, neni
  JeVinx = IsInIndex(filnam, &hSwap, &Off, &InxDel); // vraci index
  if( JeVinx >= 0)                // ?? problem palet ??
  {
   if(!noswap)                    //  pridat do seznamu
   { AddToList(x0,y0, JeVinx);
     swapmod = 1;
   }
   else                           // rovnou vykreslim
   { //x__palett(IiNpal, Iipal);     // paleta ?
     DrawIcon1(x0, y0, hSwap, Off);
   }
   return;
  }

  // Ikonu je treba nacist, prevest,... a ulozit do swapu
  bi=farmalloc(BUFERSIZE);   // docasny buffer pro Read() z disku
  mapio=farmalloc(512*sizeof(int));
  if(!bi || !mapio) goto freebi;

  bi1=bi;
  bi2= (short *)bi;

/*   struktura souboru IKN:
 +---------------------------------+--------------+-
 | paleta (= 16*3 B) |(mcolview-1) | (mrowview-1) |
 +---------------------------------+--------------+-

-+-----------------------------------------------------------------------

--+
  | ip1 | nbuf1 | data1 | ip2 | nbuf2 | data2 ...               ... | 0
| 0 |

-+-----------------------------------------------------------------------

--+

   struktura zac.souboru IKN pr 256-bar.mod:
 +----+-----+----+---------------------+-------------+--------------+-
 | Npal(2B) | FF | paleta (= Npal*3 B) |(mcolview-1) | (mrowview-1) |
 +----+-----+----+---------------------+-------------+--------------+-
*/
#ifdef POSIX
  if ((fin = a_open(filnam, O_RDONLY | O_BINARY, S_IREAD | S_IWRITE )) == -1 )
#else
  if ((fin = a_sopen(filnam, O_RDONLY | O_BINARY,
		   SH_COMPAT | SH_DENYNONE,S_IREAD | S_IWRITE )) == -1 )
#endif 
  { goto freebi;
  }


//------- vstup 1. bufferu --------->>
  if ( (len1=a_read( fin, bi, 779u )) == -1 ) goto ErrRead;

  if(bi[2]==0xFF)
  {
   inp16=0;
   npalikn=bi2[0]; palin=(char *) &bi[3];
   ik= 7 + 3 * npalikn;
   memcpy(&ip1, &bi[ik],sizeof(short));
   memcpy(&nbufo, &bi[ik+2],sizeof(short)); 
   ik += 4;
   //printf("Icon %s at %d, %d is 256 color...\n",filnam,x0,y0);
  }
  else
  {
   inp16=1;//priznak reprezentace "getimg/16barev"
   npalikn=16; palin=(char *) bi;
   ip1  =bi2[26]; nbufo=bi2[27]; ik=56;
  }

  // Transparentni ikony $$
  Transp = -1;
#ifdef TRANSPARENT_ICONS
  for(i=0; i<3*npalikn; i += 3)
  { if(palin[i+1] & 0x80)         // Nastaveny nejvyssi bit v G slozce
    { if(Transp < 0)              // Jenom prvni
      { Transp = i/3;
      }
      palin[i+1] &= 0x7F;         // Vynulovat  vsechny
    }
  }
#endif
  //----------------- osetreni palet ------------->
  if(xg_256 != MM_256)   // Nejsem v 256 color modu
  { 
    irc = MergePalIcons(npalikn, palin, mapio, Mmapio, &Mapuj);
    //printf("After MergePalIcons, IiNpal=%d\n", IiNpal);
    if(irc == 0) goto Memover;
  }
  else  // schovat si paletu na pozdejc
  { palcpy = farmalloc(npalikn*3);
    if(palcpy == NULL) goto Memover;
    memcpy(palcpy, palin, npalikn*3);
  }

//Kresli:
  //--------------------------------- vstup N bufferuu --------->>
  len1= len1 -ik;       //precteny kus bufferu
  memmove(bi, &bi[ik], len1);
  ik= len1;
  len1= nbufo - len1 +4;

Dalsi:
 if(len1>0)
 {  if ( (a_read( fin, &bi[ik], len1)) == -1 ) goto ErrRead;
    len1=0; ik=0;
 }

 // bi,bi2 - vstupni buffer (soubor IKN)
 bi2=(short *)bi;
 mcol= bi2[0]; 
 mrow= bi2[1];

 if(xg_256==MM_2)
   { lenpix=1; }
 else if(xg_256==MM_16)
   { lenpix = 4; }
 else if(xg_256==MM_256)
   { lenpix = 8; }
 else
   { lenpix = 16; }

 // bi - vstupni buffer s IMG ikony v inp16 modu
 // bwork - pracovni buffer s 1B/pixel  (schova se v 256 color modu)
 // bufw - buffer s IMG v aktualnim grf. modu (schova se v ostatnich modech)


 if(xg_256 == MM_256)  // pouze: ulozit paletu + 256 color img
 {
   bwork = TransferTo256(mcol, mrow, bi, inp16, &Nbwork);
   if(bwork == NULL)  goto Memover;

   Npalw = 2*sizeof(int) + 3*npalikn;            // delka palety, Transparent, paleta
   Nbufw = (mcol*lenpix)/8 * mrow + 16;  // (img ikony)

   bufw= MemIconImg((unsigned int)(Npalw+Nbufw), &hSwap, &Off, &UpIcnMem);
   if(bufw==NULL) goto Memover;

   memcpy(bufw, &npalikn, sizeof(int));
   memcpy(bufw+sizeof(int), &Transp, sizeof(int));
   memcpy(bufw+2*sizeof(int), palcpy, 3*npalikn);
   if(palcpy != NULL) { farfree(palcpy); palcpy = NULL;}
   memcpy(bufw+Npalw, bwork, Nbufw);

  
   // michani palet a repaletizace se odlozi az do kresleni !!
 }
 else                  // v ostatnich modech repal+ulozeni vysedneho IMG
 {
   if(inp16 == 1 && xg_256 == MM_16 && Mapuj == 0)  // spec pripad
   { Nbwork = 0;
     bwork = bi;
   }
   else
   { 
     bwork = TransferTo256(mcol, mrow, bi, inp16, &Nbwork);
     if(bwork == NULL) 
     {
      goto Memover;
     }
   }
   //------------- preklad barev dle mapiol ------------------
   if(Mapuj)
   { RepalIcon1(mcol, mrow, bwork, Mmapio);
   }

   //------------ alokace out bufru (ve swapu) pro ikonu
   if(xg_256 == MM_2 || xg_256 == MM_16)         // 2,16 barev
    Nbufw = ((mcol+7)/8 * lenpix) * mrow + 16;
   else                                          // 256,Hi-col barev
    Nbufw = (mcol*lenpix)/8 * mrow + 16;

   bufw= MemIconImg((unsigned int)Nbufw, &hSwap, &Off, &UpIcnMem);
   if(bufw==NULL) goto Memover;
   bi1 = (unsigned char *) bufw;
#ifdef HICOLOR
   biHi = (unsigned short int *)bufw;
#endif

   //-------- Prevod ikony do spravneho formatu dle grf. modu
   // IKN je vetsinou ted v 256 barvach, nekdy v 16 barvach
  if(inp16 == 1 && xg_256 == MM_16 && Mapuj == 0)
  { 
   memcpy(bi1, bi, nbufo);
  }
  else        // ve workb je 256 color ikona
  {
  switch(xg_256)
   { case   MM_Hic:        // Hi color

#ifdef HICOLOR
     memcpy(bi1, &mcol,sizeof(short)); 
     memcpy(&bi1[2],&mrow,sizeof(short));

     // Musi byt nastavena paleta (je volano x__palett() ?)
     memcpy(xg_hipal, Iipal, 3*IiNpal);
     for(i=0; i<IiNpal; i++)
     { xg_hival[i]=xh_RgbHiPal(xg_hipal[3*i],xg_hipal[3*i+1],xg_hipal[3*i+2]);
     }
     xh_ByteToHi(&bwork[4], &bi1[4], mcol,mrow,mcol);
     i = 2 + mcol * mrow;   // na konci bufru mam 12B rezervu
     if(Transp < 0)         // 4B pro ulozeni transp. barvy v HiCol
     { biHi[i] = 0;
     }
     else
     { biHi[i] = 1; biHi[i+1] = xg_hival[Transp];
     }
#endif //HICOLOR
	       break;
     case MM_2: x_img256to2(bwork, bi1);       // Binarni
	       break;
     case MM_16: x_img256to16(bwork, bi1);
	       break;
     case MM_256: memcpy(bi1, bwork, Nbufw);   // sem se to nedostane!
	       break;
     default :
	       break;
   }
  }
 }// end if 256|ostatni

 swapmod=1;  // psal jsem data

 // Pridam ji do indexu
   AddInx = AddIconToIndex(filnam, hSwap, Off, InxDel);
   if(!noswap && AddInx >= 0)
   {
    //printf("Adding to list at coordinates %d,%d, index=%d\n",x0,y0,AddInx);
    AddToList(x0,y0, AddInx);      // Pridam do seznamu
   }
  swapmod=1;  // psal jsem do indexu

  if(Nbwork > 0)      // alokoval jsem pomocny buffer
  { farfree(bwork);
  }

  bi2= (short *)&bi[nbufo];
  ip1=  bi2[0];
  nbufo=bi2[1];
  if( ip1 > 0){ len1 +=nbufo+4; goto Dalsi; }  // viceframove ikony ?

  a_close(fin);
  goto freebi;

//------------ Chybove stavy
Memover:
  memerr();
ErrRead:
Error:
  x_grf_mod(3);
  printf(MSG_ERRIKN);
  exit(EXIT_ABNORMAL);

//--------- OK konec
  freebi:
 
  if(mapio)
   farfree(mapio);
  if(bi)
   farfree(bi);

  if(noswap)
  { 
    if (hSwap) DrawIcon1(x0, y0, hSwap, Off);
  }
}

#ifndef POSIX

//optimalization sorting is not really required if we have linear access to memory.
//it was just optimizing access to memory areas swapped to XMS/EMS/disk

int  GetListNext(int From, int Kolik, struct IknList *List)
{
    char   *BegSwap;
    struct IknList  *IknLst;
    int    Kopiruj;

    BegSwap = ie_getswap(g_hIndex);
    if(!BegSwap)
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return NULL;
//     MALLOCERR();
//!!glennmcc: end
    IknLst = (struct IknList *)(BegSwap+g_BegList);

    if((From + Kolik) > g_AktList)
    { Kopiruj = g_AktList - From;
      if(Kopiruj <= 0) return( 0 );
    }
    else
     Kopiruj = Kolik;
    memcpy(List, &IknLst[From], Kopiruj*sizeof(struct IknList));
    return( Kopiruj );
}

#define MQSTA  64

#define SIZEOFICON (sizeof(XSWAP)+3*sizeof(int))
//  quick sort na setrideni listu ikon (optimalizace kresleni)

int SortListIcons(void)
{
    char   *BegSwap;
    unsigned int *A;
    int           N4;
    int      QSTA[MQSTA];       // Zasobnik
    int      L,R,I,J,IS,ISPR;
    int      X,X2;
    char pom[SIZEOFICON];

    BegSwap = ie_getswap(g_hIndex);
    if(!BegSwap)
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return NULL;
//     MALLOCERR();
//!!glennmcc: end
    A  = (unsigned int *)(BegSwap+g_BegList);

    if(g_AktList <= 1) return( 1 );
    N4 =  g_AktList*4 - 4;

// ***************************************************************
      IS=0;               // CELE POLE DO ZASOBNIKU
      QSTA[IS]=0;
      QSTA[IS+1]=N4;
//--------------------
 _10: L=QSTA[IS];         // VYBER ZE ZASOBNIKU
      R=QSTA[IS+1];
      IS=IS-2;
//--------------------
 _20: I=L;                // TRIDENI USEKU <I,J>
      J=R;
      ISPR=((L+R)/2)/4*4;
      if(ISPR > R) ISPR=R;
      if(ISPR < L) ISPR=L;
      X =A[ISPR];        // STREDOVY PRVEK  X (handle swapu)
      X2=A[ISPR+1];      // Offset ve swapu
//--------------------
 _30: if(A[I] <=  X)
      {
       if(A[I] == X)
	if(A[I+1] >= X2) goto _40;
      }
      else
      { goto _40;
      }
      I=I+4;
      goto _30;

 _40: if(A[J] >= X)
      {
       if(A[J] == X)
	 if(A[J+1] <= X2) goto _50;
      }
      else
      { goto _50;
      }
      J=J-4;
      goto _40;

 _50: if(I <= J)   // PROHOZENI
      {
       memcpy(pom, &A[I], SIZEOFICON);
       memcpy(&A[I], &A[J], SIZEOFICON);
       memcpy(&A[J], pom, SIZEOFICON);
       I=I+4;
       J=J-4;
      }
      if(I <= J) goto _30;

      if(I < R)      // ULOZENI PRAVE CASTI DO STACKU
      {
       IS=IS+2;
       if(IS >= MQSTA) return( 2 );
       QSTA[IS]=I;
       QSTA[IS+1]=R;
      }
      R=J;                 // TRIDENI ZBYTKU LEVE CASTI
      if(L  <  R) goto _20;
      if(IS >= 0) goto _10;

      return( 1 );
}

#endif

int z_bytebit(char *inp, char *out, int npix)
{
/* inp   - pole s jednim radkem pixlu                  */
/* out   - pole s bitovou rovinou                      */
/* npix  - pocet pixlu v inp                           */

   int i,lenbit,zbt;

   zbt = npix & 0x0007;
   lenbit = npix>>3;
   if(zbt != 0) lenbit++;      /* Bytu na 1 bit. rovinu */

   memset(out,0,lenbit);       /* vynulovani pole dest   */

   for(i=0; i<npix; i++)       /* Cykl pres pixly */
     { if(inp[i] & 0x01) out[(i>>3)] |= (0x80 >> (i & 0x07));
     }
   return(1);

}

//--------------- iknbuf ----------------
struct iknbuf
{
  void far *nextikn;
  int x0,y0,mcol,mrow,inp16,ip1;
  char iconame[80];
  unsigned char *bufimg;
};
#define LENIKNBUF sizeof(struct iknbuf)
//---------- globalni prom. -------------
//(z ...\flood\xvirt.c  :)
void z_bitbyte(unsigned char *buf1, unsigned char *buf2, int delka);
int bit_pix8(char *sourc, char *dest, int npix, int nbit);
int pix8_bit(char *sour8, char *dest4, int npix, int nbit);
//##---------------------------------------------------------------------
//------ prevod img/16 na img/256 --------
void x_img16to256(char *bi, char *bo)
{
// bi buffer ziskany z getimg v 16-ti barev. modu
// bo buffer urceny pro putimg v 256-ti barev. modu
// bo = vystup, musi byt prislusne dlouhy!(max. 65K!)
  short int mcol, mrow;
  int ikkk,len2,mcol8,i,j;

	memcpy(&mcol,bi,sizeof(short int)); memcpy(&mrow, &bi[2],sizeof(short));
	memcpy(bo, &mcol,sizeof(short int)); memcpy(&bo[2], &mrow,sizeof(short));
	ikkk=4;
	len2= (mcol+7)/8;
	mcol8= len2 * 8;
	for(i=0,j=4;i<mrow;i++)
	{
	  bit_pix8(&bi[j], &bo[ikkk], mcol8, 4);
	  j += 4 * len2; ikkk += mcol;
	}//konec cyklu pres radky
	return;
}
//##---------------------------------------------------------------------
//------ prevod img/256 na img/16 --------
void x_img256to16(unsigned char *bi, unsigned char *bo)
{
// bi buffer ziskany z getimg v 256-ti barev. modu
// bo buffer urceny pro putimg v 16-ti barev. modu
// predpoklada se samozrejme, ze v bi se vyskytuji pouze
// barvy 0..15 resp. horni 4 bity v pixelu se ignoruji!
// !! obe pole jsou max.65K, (zvlaste bi)
  short int mcol, mrow;
  int ikkk,len2,mcol8,j,irow;

	memcpy(&mcol,bi,sizeof(short)); 
	memcpy(&mrow, &bi[2],sizeof(short));
	memcpy(bo, &mcol,sizeof(short)); 
	memcpy(&bo[2], &mrow,sizeof(short));
	ikkk=4;
	len2= (mcol+7)/8;
	mcol8= len2 * 8;
	for(irow=0,j=4;irow<mrow;irow++)
	{
	  pix8_bit((char *)&bi[ikkk], (char *)&bo[j], mcol8, 4);
	  j += 4 * len2; ikkk += mcol;
	}//konec cyklu pres radky
	return;
}
//##---------------------------------------------------------------------
//------ prevod img/256 na img/2 --------
void x_img256to2(unsigned char *bi, unsigned char *bo)
{
// bi buffer ziskany z getimg v 256-ti barev. modu
// bo buffer urceny pro putimg v 2 barev. modu
// predpoklada se samozrejme, ze v bi se vyskytuji pouze
// barvy 0/1 resp. hornich 7 bitu v pixelu se ignoruje!
// !! obe pole jsou max.65K, (zvlaste bi)
  short int mcol, mrow;
  int ikkk,len2,mcol8,j,irow;

	memcpy(&mcol,bi,sizeof(short)); 
	memcpy(&mrow, &bi[2],sizeof(short));
	memcpy(bo, &mcol,sizeof(short)); 
	memcpy(&bo[2], &mrow,sizeof(short));
	ikkk=4;
	len2= (mcol+7)/8;
	mcol8= len2 * 8;
	for(irow=0,j=4;irow<mrow;irow++)
	{
          z_bytebit((char *)&bi[ikkk], (char *)&bo[j], mcol8);
	  j +=  len2; ikkk += mcol;
	}//konec cyklu pres radky
	return;
}
//##---------------------------------------------------------------------
//------ prevod img/2 na img/256 --------
void x_img2to256(unsigned char *bi, unsigned char *bo)
{
// bi buffer ziskany z getimg v 16-ti barev. modu
// bo buffer urceny pro putimg v 256-ti barev. modu
// bo = vystup, musi byt prislusne dlouhy!(max. 65K!)
  short int mcol, mrow;
  int ikkk,len2,i,j;

	memcpy(&mcol,bi,sizeof(short)); 
	memcpy(&mrow, &bi[2],sizeof(short));
	memcpy(bo, &mcol,sizeof(short)); 
	memcpy(&bo[2], &mrow,sizeof(short));
	ikkk=4;
	len2= (mcol+7)/8;
	//mcol8= len2 * 8;
	for(i=0,j=4;i<mrow;i++)
	{
//void z_bitbyte(unsigned char *buf1, unsigned char *buf2, int delka);
	  z_bitbyte((unsigned char *)&bi[j], (unsigned char *)&bo[ikkk], mcol);
	  j += len2; ikkk += mcol;
	}//konec cyklu pres radky
	return;
}
//----- Prevod bytoveho radku na radek po bitovych rovinach ----
int pix8_bit(char *sour8, char *dest4, int npix, int nbit)
/* sour8  - pole s jednim radkem pixlu                  */
/* dest4 - pole s jednim radkem po bit. rovinach       */
/* npix  - pocet pixlu na radek                        */
/* nbit  - pocet bitovych rovin  1..8                  */
{
	int i,j,k,nb,npix8;
	unsigned char c;

	memset(dest4,0,npix/2);       /* vynulovani pole dest   */
	npix8 = npix/8;
	nb = npix8*(nbit-1);

	for(j=0; j<nbit; j++)      /* Cykl pres bitove roviny*/
	 {
/*p  for(i=0; i< npix8; i++)
		 {  c =  dest4[i+nb];
			 for(k=0; k<8; k++)
			 { if(c & 0x80) sour8[(i<<3) + k] |= 1<<j;
				c = c<<1;
			 }
		 }
		nb -= npix8;*/
	  for(i=0; i< npix8; i++)
		 { c = 0;
		   for(k=0; k<8; k++)
		   {
		     c = c<<1;
           if( sour8[(i<<3) + k] & (1<<j)) c |= 1;
		   }
		   dest4[i+nb]= c;
		 }
		nb -= npix8;
	  }//endcykl pres bit.roviny
	return 1;
}


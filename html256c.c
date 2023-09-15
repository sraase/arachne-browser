
// ========================================================================
// HTML palette mixing routines for Arachne WWW browser
// (c)1990-1999 Ivan Polak + Arachne Labs (xChaos software)
// ========================================================================


#include "arachne.h"
#include "html.h"

//. Mix only the paletes on screen (visible)
void MixVisiblePaletes(char writepal)
{

 int pocet=0,i,celkpocet=0;
 int secidx[MAXPALMIX+1];
 XSWAP atomadr[MAXPALMIX+1];
 struct picinfo *dataptr;
 struct picinfo *obrazky;
 struct Url url;
 int maxpocet;//,frameID;
 XSWAP dummy1;
 unsigned dummy2;
 unsigned currentHTMLatom=p->firstHTMLatom,nextHTMLatom;
 struct HTMLframe *frame;
 struct HTMLrecord *imgatomptr;
 struct HTTPrecord HTTPdoc;
// struct TMPframedata *htmldata;

#ifdef POSIX
 maxpocet=MAXPALMIX;
#else 
 maxpocet=(int)((farcoreleft()-2000l)/(5500+sizeof(struct picinfo)));
 if(maxpocet>MAXPALMIX)
  maxpocet=MAXPALMIX;
 else if(maxpocet<1)
  maxpocet=1;
#endif

 obrazky=farmalloc(sizeof(struct picinfo)*maxpocet);
 if(!obrazky)memerr();

// while(HTMLdoc.cur<HTMLdoc.len)
 while(currentHTMLatom!=IE_NULL)
 {
  kbhit();
  imgatomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if(!imgatomptr)
   MALLOCERR();
  nextHTMLatom=imgatomptr->next;
  frame=&(p->htmlframe[imgatomptr->frameID]);
  if( (imgatomptr->type==IMG || imgatomptr->type==TD_BACKGROUND) &&
      (imgatomptr->y>=frame->posY ||
       imgatomptr->yy>=frame->posY) &&
      (imgatomptr->y<frame->posY+frame->scroll.ysize ||
       imgatomptr->yy<frame->posY+frame->scroll.ysize) &&
      (imgatomptr->x>=frame->posX ||
       imgatomptr->xx>=frame->posX) &&
      (imgatomptr->x<frame->posX+frame->scroll.xsize ||
       imgatomptr->xx<frame->posX+frame->scroll.xsize)
      || imgatomptr->type==BACKGROUND)
  {
//   idx[celkpocet]=HTMLdoc.cur;
   atomadr[celkpocet]=currentHTMLatom;
   dataptr=(struct picinfo *)ie_getswap(imgatomptr->ptr);
   if(dataptr)
   {
    dataptr->palismap=0;
    swapmod=1;

    if(celkpocet<MAXPALMIX)
    {
     memcpy(&obrazky[pocet],dataptr,sizeof(struct picinfo));
     AnalyseURL(obrazky[pocet].URL,&url,IGNORE_PARENT_FRAME);
     if(SearchInCache(&url,&HTTPdoc,&dummy1,&dummy2))
     {
      i=0;
      while(i<pocet)
      {
       if(!strcmp(obrazky[i].filename,HTTPdoc.locname))
       {
        secidx[celkpocet++]=i;
        goto nextobrazek;
       }
       i++;
      }
      //pocitaji se jenom GIFy a bitmapy:
      // tr.: only GIFs and bitmaps are counted/calculated
      if(HTTPdoc.locname[0] &&
      (strstr(HTTPdoc.locname,".GIF") ||
       strstr(HTTPdoc.locname,".BMP")))
      {
       strcpy(obrazky[pocet].filename,HTTPdoc.locname);
       secidx[celkpocet++]=pocet; //u prvniho obrazku 0, u druheho 1;
         // tr.: at the first picture 0, at the second 1
       pocet++;
      }

      if(pocet==maxpocet)
       goto allobrazky;

     }


    }
    else
     goto allobrazky;

   }//endif nasel sem neco (tr.: endif I found something)
   else
    MALLOCERR();

  }
  nextobrazek:
//  HTMLdoc.cur++;
  currentHTMLatom=nextHTMLatom;
 }
 allobrazky:


 i=0;
 if(celkpocet>0)
 {
  if(!MixPal(obrazky,pocet,writepal)) //zkusim smichat palety...
    // tr.: I try to mix the palette
   goto ven;
  while(i<celkpocet)
  {

   //zrychlena verze (tr.: accelerated version)
   imgatomptr=(struct HTMLrecord *)ie_getswap(atomadr[i]);
   if(imgatomptr && (imgatomptr->type==IMG || imgatomptr->type==BACKGROUND || imgatomptr->type==TD_BACKGROUND))
   {
    dataptr=(struct picinfo *)ie_getswap(imgatomptr->ptr);
    if(dataptr)
    {
     int resize_x=dataptr->resize_x;
     int resize_y=dataptr->resize_y;
     memcpy(dataptr,&obrazky[secidx[i]],sizeof(struct picinfo));
     dataptr->resize_x=resize_x;
     dataptr->resize_y=resize_y;
     swapmod=1;
    }
    else
     MALLOCERR();
   }
   else
    MALLOCERR();
   i++;
  }
 }
 ven:
 if(obrazky) farfree(obrazky);
}

int  PresspalO (int multip, char *Palin[], int *Npalin, char  *palout,
               int *npalout, int *mapio, int *Mmapio[],
               int Swinout, int TypFuse, int Tolerance,
               char *Savecols );

void safemappal(int npalout) //safe = will avoid blinking
{
 //palette will be mapped only if it is shorter than max. number of colors
 if(IiNpal<x_getmaxcol()+1 && IiNpal!=npalout || xg_256==MM_Hic)
 {
  IiNpal=npalout;
#ifdef VIRT_SCR
  if(xg_video_XMS==0 || xg_256 == MM_Hic) //HARO
  //not while in virtual screen!
#endif
   x_palett(x_getmaxcol()+1, Iipal);
 }
}



int MixPal(struct picinfo *o, int n, char writepal)
{
 char *Palin[MAXPALMIX+1];
 int Npalin[MAXPALMIX+1];
 int Idxtolist[MAXPALMIX+1];
 int *Mmapio[MAXPALMIX+1];
 int *mapio=NULL;
 char *Savecols=NULL;
 int npalout=0,npic=1,i=0;
 char str[80];
 int rv=1;
 //int Uloz; (tr.: Save it)

 sprintf(str,MSG_LDPAL,n );
 outs(str);

 while(npic<MAXPALMIX && i<n)
 {
  o[i].palonly=1;
  o[i].sizeonly=0;
  if(drawanyimage(&o[i])==1)
  {
   npalout+=o[i].npal;
   Palin[npic]=(char *)o[i].pal;
   Npalin[npic]=o[i].npal;
   Idxtolist[npic]=i;
   npic++;
  }
  i++;
 }

 if(npic<2) return 0;

 sprintf(str,MSG_MIXPAL,npalout );
 outs(str);

 mapio=farmalloc(512+2*npalout);
 Savecols=farmalloc(256+npalout);
 if(mapio==NULL || Savecols==NULL)memerr();

 if(x_getmaxcol()==1)IiNpal=2;
 else                IiNpal=16;
 Palin[0]= Iipal;
 Npalin[0]= IiNpal;
 memset( Savecols, 0, IiNpal+npalout);
 memset( Savecols, 1, IiNpal);
 npalout=x_getmaxcol()+1;//256

 i=PresspalO (npic, Palin, Npalin, Iipal, &npalout, mapio, Mmapio,
            2, 1, 2,  Savecols );
 if(i!=1)
 {
  Piip();
  rv=0;
  goto out;
 }

 if(writepal)
  safemappal(npalout);
 else
  IiNpal=npalout;

 i=1;
 while(i<npic)
 {
  memcpy(o[Idxtolist[i]].pal,Mmapio[i],Npalin[i]*2);
  o[Idxtolist[i]].palismap=1;
  i++;
 }

 out:
 if(Savecols)farfree(Savecols);
 if(mapio)farfree(mapio);

 return rv;
}


//initialization of palette...
void initpalette(void)
{
 char paleta[48]={0,0,0, 0,0,40, 40,0,0, 0,40,0, 0,40,40, 40,0,40, 40,40,0,
 49,49,49, 33,33,33, 10,10,63, 63,10,10, 10,63,10, 10,63,63, 63,10,63,
 63,63,10, 63,63,63};
 char vgapal[48]={0,0,0, 0,0,30, 40,0,0, 0,30,0, 0,40,40, 40,0,40, 40,40,0,
 49,49,49, 33,33,33, 10,10,63, 63,10,10, 10,63,10, 10,63,63, 56,56,56,
 63,63,10, 63,63,63};
 char monopal[48]={0,0,0, 8,8,8, 12,12,12, 28,28,28, 16,16,16, 20,20,20, 24,24,24,
 48,48,48, 32,32,32, 36,36,36, 42,42,42, 51,51,51, 54,54,54, 58,58,58,
 61,61,61, 63,63,63};
/* char monopal[48]={0,0,0, 8,8,8, 12,12,12, 28,28,28, 16,16,16, 20,20,20, 24,24,24,
 48,48,48, 32,32,32, 36,36,36, 40,40,40, 52,52,52, 55,55,55, 58,58,58,
 61,61,61, 63,63,63};*/
 char egapal[48]={0,0,0, 0,0,31, 31,0,0, 0,15,0, 0,15,15, 15,0,15, 15,15,0,
 31,31,31, 15,15,15, 0,0,63, 63,0,0, 0,47,0, 0,47,47, 47,0,47,
 47,47,0, 63,63,63};

 if(x_getmaxcol()==1)
 {
   Iipal[0]=Iipal[1]=Iipal[2]=0;
   Iipal[3]=Iipal[4]=Iipal[5]=63;
   IiNpal=2;
 }
 else
 {
   if(egamode)
    memcpy(Iipal,egapal,48);
   else
   if(vgamono)
    memcpy(Iipal,monopal,48);
   else
   if(vga16mode)
    memcpy(Iipal,vgapal,48);
   else
    memcpy(Iipal,paleta,48);
   IiNpal=16;
#ifdef VIRT_SCR
   if(xg_video_XMS==0 || xg_256 == MM_Hic) //HARO
   //not while in virtual screen!
#endif
    x_palett( IiNpal, Iipal);
 }
}

// ========================================================================
// RGB color selection module
// (c)1997,1998,1999 xChaos software
// ========================================================================


int PresspalO (int multip, char *Palin[], int *Npalin, char  *palout,
               int *npalout, int *mapio, int *Mmapio[],
               int Swinout, int TypFuse, int Tolerance,
               char *Savecols );

//prida barvu nebo ji najde na obrazovce; vraci index
// tr.: adds colour or finds it on the screen; returns index
//. Translate RGB to integers for x_setcolor(.)
int RGB256(unsigned char r,unsigned char g,unsigned char b)
{
 char *Palin[2];
 int Npalin[2];
 int *Mmapio[2];
 int *mapio=NULL;
 char *Savecols=NULL;
 int npalout;
 char pal[3];
 int ret;

 //nepamatuju si uz tuhle barvu nahodou ? (99% pripadu - jo!)
 // tr.: have'nt I seen this colour already before?
 //      (in 99 percent of cases - yes)
// if(!cgamode)
// {
  ret=0;
  while(ret<16)
  {
   if(cacher[ret]==r && cacheg[ret]==g && cacheb[ret]==b )
    return coloridx[ret];
   ret++;
  }
// }

 mapio=farmalloc(1024);
 Savecols=farmalloc(512);
 if(mapio==NULL || Savecols==NULL)memerr();

 if(egamode)
 {
  pal[0]=egafilter(r>>2);
  pal[1]=egafilter(g>>2);
  pal[2]=egafilter(b>>2);
 }
 else
 if(vga16mode && !vgamono)
 {
  pal[0]=vgafilter(r>>2);
  pal[1]=vgafilter(g>>2);
  pal[2]=vgafilter(b>>2);
 }
 else
 {
 pal[0]=r>>2;
 pal[1]=g>>2;
 pal[2]=b>>2;
 }


 /*pal[3]=0;
 pal[4]=0;
 pal[5]=0;*/

/* if(cgamode)
 {
  char cgapal[6]={0,0,0,255,255,255};
  Palin[0]=cgapal;
  Npalin[0]= 2;
 }
 else
 {*/
  Palin[0]=Iipal;
  Npalin[0]= IiNpal;
// }
 Palin[1]=pal;
//? if(cgamode) Npalin[1]=2;
//? else        Npalin[1]=16;
//p: Npalin[1]=16;
 Npalin[1]=1;
 memset( Savecols, 0, 512);
 memset( Savecols, 1, IiNpal);

/*
#ifdef HICOLOR
 if(xg_256==MM_Hic)
  npalout=256;
 else
#endif*/
  npalout=x_getmaxcol()+1;//256

 ret=PresspalO (2, Palin, Npalin, Iipal, &npalout, mapio, Mmapio,
            2, 1, 2,  Savecols );
 if(ret!=1)Piip();
if(npalout>IiNpal)   // I/970429
{
#ifdef VIRT_SCR
 if(xg_video_XMS==0 || xg_256 == MM_Hic) //HARO
 //not while in virtual screen!
#endif
  x_pal_1(IiNpal, &Iipal[3 * IiNpal]);
 IiNpal=npalout;
}// << I/970429
 ret= Mmapio[1][0];
 cacher[rgbcacheidx]=r;
 cacheg[rgbcacheidx]=g;
 cacheb[rgbcacheidx]=b;
 coloridx[rgbcacheidx]=ret;
 rgbcacheidx++;
 if(rgbcacheidx>15)rgbcacheidx=2;

 if(Savecols)farfree(Savecols);
 if(mapio)farfree(mapio);

 return ret;
}


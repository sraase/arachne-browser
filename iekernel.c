
#include "posix.h"

#ifndef POSIX
#include "bufbuf.h"
#endif

#include "ie.h"
#include "a_io.h"

#ifdef POSIX
char *swapbuf[IE_MAXSWAP+1];
char CLIPBOARDNAME[80];
#else
struct T_buf_buf swapbuf[IE_MAXSWAP+1]; //pole odkladacich buferu
#endif

char swapbufflag[IE_MAXSWAP+1];

unsigned *swapidx; //uznavam i=0-MAXLS; 1023=error flag, empty line
//toto pole je vzdy obsazeno ve swapstr, nesmi tedy obsahovat pointery mensi
//nez IE_SWAPSTART, ukazovalo by samo do sebe!!!

char *swapstr=NULL; //od adresy swapstr[IE_SWAPSTART] zacina vlastni odkladani radku

#ifndef POSIX
char *swap1=NULL,*swap2=NULL;
int which1=-1,which2=-1,mod1=0,mod2=0;
#endif

int swapcount; //pocet swapu
int swapnum; //cislo swapu, ktery je prave v ulozen v swapstr, 0-IE_MAXSWAP
int swaplen[IE_MAXSWAP]; //delka vsech swapu v radkach
unsigned swapsize[IE_MAXSWAP]; //delka vsech swapu v bytech
int swapcontext[IE_MAXSWAP]; //context of swapped buffer (SYSTEM, HTML, TABLE, ...)
int swapsavecount=0,swaploadcount=0,swapoptimize=0;

int swapmod; //priznak modifikace
int firstswap=0; //prvni swap, od ktereho se bude pridelovat colne misto

//-----------------------------------------------
//XSWAP "malloc": context sensitive
//-----------------------------------------------

//XSWAP       ...defined as "usigned" for DOS
//char *line  ...memory pointer
//unsigned l  ...length of memory area to store
//int context ...for fast garbage collection - ie_killcontext()

#ifndef POSIX
//non-POSIX version
//Nastavuje, se kterym odkladacim buferem se bude pracovat...
//-----------------------------------------------------------
int ie_swap(int newswap) //nastavi ten spravny bufer do pameti...
{
 int rc;
 unsigned i;

 if(swapnum==newswap) return 1;

 if(swap2 && newswap>=0) //we are trying to optimize access to virtual memory now:
 {
  if(newswap==which1 && swapstr==swap2)            // go from 2 to 1
  {
   mod2=swapmod;
   swapstr=swap1;
   swapidx=(unsigned *)swap1;
   swapnum=which1;
   swapmod=mod1;
   return 1;
  }

  if(newswap==which2 && swapstr==swap1)       // go from 1 to 2
  {
   mod1=swapmod;
   swapstr=swap2;
   swapidx=(unsigned *)swap2;
   swapnum=which2;
   swapmod=mod2;
   return 1;
  }

  if(swapstr==swap2 && (swapoptimize || swapcontext[newswap]<=swapcontext[which1] &&
     swapcontext[newswap]<=swapcontext[which1]) ) //2 to 1
  {
   mod2=swapmod;
   swapstr=swap1;
   swapidx=(unsigned *)swap1;
   swapnum=which1;
   swapmod=mod1;
  }
  else
  if(swapstr==swap1 && (swapoptimize ||swapcontext[newswap]<=swapcontext[which2] &&
     swapcontext[newswap]<=swapcontext[which2] || which2==-1)) //1 to 2
  {
   mod1=swapmod;
   swapstr=swap2;
   swapidx=(unsigned *)swap2;
   swapnum=which2;
   swapmod=mod2;
  }
 } //----------------------------------------------------------------------

 if(swapmod && swapnum>=0)   // byl akt.buffer modifikovan?
 {
  //vymazat pripadne stary swap:
  if(swapbufflag[swapnum])
  {
   delBuf(&swapbuf[swapnum]);
   swapbufflag[swapnum]=0;
  }

  //do prvnich 2048 bytu ulozim pole swapidx
  //!memcpy(swapstr,swapidx,2048);

  //do handleru swapf[swapnym] ulozit bufer swapstr
  rc=saveBuf(swapstr,(unsigned)((IE_SWAPSTART+swapsize[swapnum])),&swapbuf[swapnum]);
//  printf("swapnum=%d, ulozeno %u bajtu, rc=%d",newswap,(unsigned)(IE_SWAPSTART+swapsize[swapnum]),rc);
  if(rc!=1) return 2;
  swapbufflag[swapnum]=1;
  swapmod=0;
  swapsavecount++;
 }//endif

 if(newswap==-1) return 1;

 //nacteni noveho akt.bufferu:
 i=IE_MAXSWAPLEN+IE_SWAPSTART; //!!! tady byla tezka chyba
 if(swapbufflag[newswap])
 {
  rc=fromBuf(swapstr,0,&i,&swapbuf[newswap]);
  if(rc!=1) return 2;
 }
 else
 {
  while(i>0)
  {
   i--;
   swapstr[i]='\0';
  }//loop
 }//endif
 swapnum=newswap;
 if(swap2==swapstr)
 {
  which2=swapnum;
  mod2=0;
 }
 else
 {
  which1=swapnum;
  mod1=0;
 }

 swaploadcount++;
 return 1;
}
#else  // POSIX-compliant
int ie_swap(int newswap) //nastavi ten spravny bufer do pameti...
{
// printf("*** Current swap is %d, requesting swap %d\n",swapnum,newswap);

 if((swapnum==newswap && swapbuf[newswap]!=NULL) || newswap==-1) return 1;
 
 if(swapbuf[newswap]==NULL)
  swapbuf[newswap]=malloc(IE_MAXSWAPLEN+IE_SWAPSTART+1);
 swapstr=swapbuf[newswap];    
 swapidx=(unsigned *)swapbuf[newswap];
 
 if(!swapstr)
 {
  printf("Could not allocate memory !\n");
  return 2;
 }
  
 swapnum=newswap;
 return 1;
}
#endif

XSWAP ie_putswap(char *line, unsigned l, int context)
{
 XSWAP ladr;
 unsigned newswap=firstswap;

 //printf("***storing %u bytes....",l);
 
 retry: //prohledavani dalsiho bufferu
 ladr=0;

 if((long)swapsize[swapnum]+(long)l>IE_MAXSWAPLEN || (swapcontext[swapnum]!=context))
  goto newswap;

 while (ladr<swaplen[swapnum] && swapidx[ladr]>=IE_SWAPSTART) ladr++;

 if(ladr==swaplen[swapnum])
 {
  if (swaplen[swapnum]>IE_MAXLS)
  {
   newswap++;

   newswap:

   while( newswap<IE_MAXSWAP &&
          ((swapcontext[newswap]>=0 && swapcontext[newswap]!=context)
           || (long)swapsize[newswap]+(long)l>IE_MAXSWAPLEN) )
    newswap++;

   // zrejme uz to bylo prohledany vsechno,vrat jedinou neplatnou adresu radky
   if (newswap>=IE_MAXSWAP)
     return IE_NULL;

   swapcontext[newswap]=context;
   ie_swap(newswap);

   goto retry;
   // drive tady bylo rovnou: ladr=swaplen[swapnum];
  }//endif
  swaplen[swapnum]++;
 }//endif

 swapidx[ladr]=IE_SWAPSTART+swapsize[swapnum];
 swapsize[swapnum]+=(l+1);
 memcpy(&swapstr[swapidx[ladr]],line,l);
 swapstr[swapidx[ladr]+l]='\0';
 swapcontext[swapnum]=context;
 swapmod=1;

 //printf("stored to swap %u at position %u\n",newswap,swapidx[ladr]);

 return (swapnum << 10 ) | ladr;
}//end sub

//Cte radku dane 4+12 bitove adresy ze swapu
//------------------------------------------
char *ie_getswap(XSWAP adr)
{
 XSWAP lswap,ladr;

 if(adr==IE_NULL)
  return NULL;

 lswap = adr & 0xFC00;
 lswap >>= 10;
 ladr  = adr & 0x3FF;
 
 if(ladr>swaplen[lswap] || ladr > IE_MAXLS) 
 {
//  printf("Illegal xSwap handler ! (swaplen[%d]=%d)\n",lswap,swaplen[lswap]);
  return NULL;
 }
 if(ie_swap(lswap)!=1)
 {
//  printf("Failed to get xSwap buffer !\n");
  return NULL;
 }
 if(swapidx[ladr]<IE_SWAPSTART)
 {
  printf("Illegal xSwap offset !\n");
//  return NULL; //chybna adresa ! (implicitne je tam totiz nula)
 }

 return(&swapstr[swapidx[ladr]]);
}//end sub

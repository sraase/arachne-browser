#include "posix.h"
#include "bin_file.h"
#include "a_io.h"
#include "glflag.h"

extern unsigned *swapidx; //uznavam i=0-MAXLS; 1023=error flag, empty line
//toto pole je vzdy obsazeno ve swapstr, nesmi tedy obsahovat pointery mensi
//nez IE_MAXSTART, ukazovalo by samo do sebe!!!

extern char *swapstr; //od adresy swapstr[IE_MAXSTART] zacina vlastni odkladani radku

//extern int swapcount; //pocet swapu
extern int swapnum; //cislo swapu, ktery je prave v ulozen v swapstr, 0-IE_MAXSWAP
extern int swaplen[IE_MAXSWAP]; //delka vsech swapu v radkach
extern unsigned swapsize[IE_MAXSWAP]; //delka vsech swapu v bytech
extern int swapmod; //priznak modifikace
extern int firstswap; //prvni swap, od ktereho se bude pridelovat volne misto

//otevrit/nahrat z disku
//. Load binary structures from the disk.  Used only by cache.idx
int ie_openbin(struct bin_file *fajl) //load, nebo open, 1. nebo 2.
{
 int i,f=-1,rv;

 fajl->len=0;
// fajl->firstswap=0;

 fajl->lineadr=farmalloc((sizeof(XSWAP))*(IE_MAXLINES+2));
 if(fajl->lineadr==NULL) return 2;
 fajl->linesize=farmalloc((sizeof(unsigned short))*(IE_MAXLINES+2));
 if(fajl->linesize==NULL) return 2;

 i=0;
 while (i<IE_MAXLINES)
 {
  fajl->lineadr[i]=IE_NULL;
  fajl->linesize[i]=0;
  i++;
 }//loop

 if (swapstr == NULL ) return 2;

 if(fajl->filename[0])
#ifdef POSIX
  f=a_open(fajl->filename,O_RDONLY, 0);
#else
  f=a_sopen(fajl->filename,O_RDONLY|O_BINARY,SH_COMPAT | SH_DENYNONE, S_IREAD);
#endif

 if(f==-1)
 {
  //if(strlen(fajl->filename)==0) strcpy(fajl->filename,"NONAME.BIN");
  fajl->modified=0; //!!!
  return 1;
 }
 else
 {
  unsigned short linelen;
  char *inbuf;

  inbuf=farmalloc(16001);
  if(!inbuf)return 2;
  rv=1;

  fajl->modified=0; //!!!
  // load file
  while (1)
  {
   if(a_read(f,&linelen,sizeof(unsigned short))!=sizeof(unsigned short))
    goto exit_load; //end of file
   if(linelen>16000)
    {rv=6;goto exit_load;}
   if(a_read(f,inbuf,linelen)!=linelen)
    {rv=6;goto exit_load;}
   fajl->lineadr[fajl->len]=ie_putswap(inbuf,linelen,CONTEXT_SYSTEM);
   fajl->linesize[fajl->len]=linelen;
   if(fajl->len>IE_MAXLINES)
    {rv=5;goto exit_load;} //moc radku, moc velky soubor
   fajl->len++;
  }//loop

  exit_load:
  farfree(inbuf);
  a_close (f);
  return rv;
 }//end if
}//end sub

int ie_savebin(struct bin_file *fajl)
{
 int f,i=0,l;
 char *r;

#ifdef POSIX
 f = a_open( fajl->filename, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD|S_IWRITE);
#else
 f = a_sopen( fajl->filename, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC,
     SH_COMPAT | SH_DENYNONE, S_IREAD|S_IWRITE );
#endif
 if ( f == -1 ) return 2; //chyba pri otevirani

 while(i<fajl->len)
 {
  r=ie_getswap(fajl->lineadr[i]);
  if (r!=NULL)
  {
   l=fajl->linesize[i];
   if(write(f,&l,sizeof(unsigned short))!=sizeof(unsigned short)) 
    {a_close (f); return 4;} //chyba pri psani!
   if(write(f,r,l)!=l) 
    {a_close (f); return 4;} //chyba pri psani!
  }//endif
  i++;
 }//loop
 a_close(f);
 fajl->modified=0;
 return 1;
}//end sub

//smazat soubor z pameti:
void ie_resetbin(struct bin_file *fajl)
{
 int i=0;

 while(i<fajl->len)
 {
  if (ie_getswap(fajl->lineadr[i]))
   ie_delswap(fajl->lineadr[i],fajl->linesize[i]);
  i++;
 }//loop

 fajl->len=0;
}//end sub

void ie_clearbin(struct bin_file *fajl) //kdyz je nemazat NULL, tak nic
{
 //tohle je udelany zatim provizorne!!!
 //tady se budou swapy ktere pati jenom jednomu souboru mazat kopmpletne...

 ie_resetbin(fajl);

#ifndef MYMEMORY
 if(fajl->lineadr!=NULL)farfree(fajl->lineadr);fajl->lineadr=NULL;
 if(fajl->linesize!=NULL)farfree(fajl->linesize);fajl->linesize=NULL;
#endif


}//end sub


// ========================================================================
// Funkce pro spravu textoveho souboru otevreneho Ibase Editorem
// Functions for the xswap and text editor  - ie_*(..)
// (c)1995-97 Michael Polak, xChaos software
// ========================================================================

#ifdef POSIX
#include "posix.h"
#else
//very old IBASE stuff... maybe already unused 
#include "bufbuf.h"
#endif

#include "ie.h"
#include "a_io.h"
#include "str.h"

#ifndef MINITERM
#include "pckbrd.h"
#endif

#ifdef POSIX
extern char *swapbuf[IE_MAXSWAP+1];
#else
extern struct T_buf_buf swapbuf[IE_MAXSWAP+1]; //pole odkladacich buferu
     // tr.: field of swap buffer
#endif

extern char swapbufflag[IE_MAXSWAP+1];
extern unsigned *swapidx; //uznavam i=0-MAXLS; 1023=error flag, empty line
     // tr.: I acknowledge i=0-MAXLS; 1023=error flag, empty line

extern char *swapstr; //od adresy swapstr[1024*siizeof(UNSIGNED)] zacina vlastni odkladani radku
     // tr.: from the address swapstr[1024*siizeof(UNSIGNED)] actually begins the buffering of lines
#ifndef POSIX
extern char *swap1,*swap2;
extern int which1,which2,mod1,mod2;
#endif

extern int swapcount; //total number of swaps
extern int swapnum; //cislo swapu, ktery je prave v ulozen v swapstr, 0-IE_MAXSWAP
     // tr.: no. of the swap that is about to be saved into swapstr, 0-IE_MAXSWAP
extern int swaplen[IE_MAXSWAP]; //delka vsech swapu v radkach
     // tr.: length of all swaps in lines
extern unsigned swapsize[IE_MAXSWAP]; //delka vsech swapu v bytech
     // tr.: length of all swaps in bytes
extern int swapcontext[IE_MAXSWAP]; //context of swapped buffer (SYSTEM, HTML, TABLE, ...)
extern int swapsavecount,swaploadcount,swapoptimize;

extern int swapmod; //priznak modifikace
     // tr.: indication/character of modification
extern int firstswap; //prvni swap, od ktereho se bude pridelovat colne misto
     // tr.: first swap, beginning with which will be assigned/allocated (?) space/place

//vysvetlivka spravy swapbuf
     // tr.: explanation of swapbuf management

// adresa radku je typu unsigned:
     // tr.: address of row is type unsigned
// adr=n*1024+m, kde n je cislo buferu, m cislo radky v pameti
     // tr.: adr=n*1024+m, where n is buffer no., m is line no. in memory

int ie_swap(int newswap); //nastavi ten spravny bufer do pameti...
     // tr.: defines/sets the correct buffer into memory...

//initialization
//--------------
int ie_initswap(void)
{
 int i;
#ifndef POSIX 
 if ( swapstr == NULL ) swapstr = (char *)farmalloc( IE_MAXSWAPLEN+IE_SWAPSTART+1);
 if ( swapstr == NULL) return 2;
#endif

 swapidx=(unsigned *)swapstr;
 swapcount=0; //0 lines
 i=0;
 do
 {
  swaplen[i]=0;
  swapsize[i]=0;
#ifndef POSIX
  swapbufflag[i]=0;
#else
  swapbuf[i]=NULL; 
#endif
  swapcontext[i]=-1;
 }
 while(++i<IE_MAXSWAP);
 swapnum=-1;
 ie_swap(0);
 firstswap=0;

 return 1;
}//end sub

#ifndef POSIX
int ie_optimize(void)
{
 //we will allocate second buffer to optimize access to virtual memory:
 swap2 = (char *)farmalloc( IE_MAXSWAPLEN+IE_SWAPSTART+1);

 if(!swap2 /*|| !idx2*/)
  return 2;

 swap1 = swapstr;
 //idx1  = swapidx;
 which1= swapnum;

 return 1;
}

#endif
//smaz radek dane 4+12 adresy ! (tr.: delete line of given 4+12 address !)
//-----------------------------
int ie_delswap(XSWAP adr, int l)
{
 XSWAP lswap,ladr;
 unsigned i,pom;

 lswap = adr & 0xFC00;
 lswap >>= 10;
 ladr  = adr & 0x3FF;
 if(ladr>swaplen[lswap]) return 2; //erroneous address !
 if(ie_swap(lswap)!=1) return 2;
 if(swapidx[ladr]<IE_SWAPSTART) return 2; //(implicitne je tam totiz nula)
   // tr.: because implicitely there is zero

 pom=swapidx[ladr];
 if(l==-1) l=strlen(&swapstr[pom]);

 if (ladr<swaplen[lswap])
 {
  memmove(&swapstr[pom],&swapstr[pom+l+1],swapsize[lswap]-(pom-IE_SWAPSTART+1));
 }//endif

 swapsize[lswap]-=l+1;
 swapmod=1;
 i=0;
 while (i<swaplen[lswap])
 {
  if(swapidx[i]>pom)swapidx[i]-=l+1;
  i++;
 }//loop
 swapidx[ladr]=IE_NULL; //empty
 return 1;
}


//Meni jiz existujici radku (tr.: changes already existing line)
//-------------------------
XSWAP ie_chngswap(char*line, XSWAP adr, int context)
{
 char *ptr;
 XSWAP lswap,ladr;

 ptr=ie_getswap(adr);
 if(ptr!=NULL)
 {
  if(strlen(ptr)==strlen(line))
  {
   lswap = adr & 0xFC00;
   lswap >>= 10;
   ladr  = adr & 0x3FF;
   if(ladr>swaplen[lswap]) return IE_NULL; //erroneous address !
   if(ie_swap(lswap)!=1) return IE_NULL;
   if(swapidx[ladr]<IE_SWAPSTART) return IE_NULL; //(implicitne je tam totiz nula)
          // tr.: because implicitely there is zero
   strcpy(&swapstr[swapidx[ladr]],line);
   swapmod=1;
   return adr;
  }
  else
   if (ie_delswap(adr,-1)!=1) return IE_NULL;
 }
 return ie_putswap(line,strlen(line),context);
}//end sub

int ie_killswap(int zabit)
{
#ifdef POSIX
 if(swapbuf[zabit]!=NULL)
 {
  free(swapbuf[zabit]);
  swapbuf[zabit]=NULL;
 } 
#else
 if (swapbufflag[zabit])
 {
  delBuf(&swapbuf[zabit]);
  swapbufflag[zabit]=0;
 }//endif
#endif
 swapsize[zabit]=0;
 swaplen[zabit]=0;
 return 1;
}//end sub

void ie_killcontext(int context)
{
 int i=0;
 while(i<IE_MAXSWAP)
 {
  if(swapcontext[i]==context)
  {
   swapsize[i]=0;
   swaplen[i]=0;
   swapcontext[i]=-1; //undefined
  }
  i++;
 }

}


void *getXSWAPlineptr(struct ib_editor *fajl)
{
 return ie_getswap(fajl->linearray);
}

XSWAP getXSWAPlineadr(struct ib_editor *fajl, int i)
{
 XSWAP *la;

// if(fajl->arraycache==i)
//  return fajl->arraycacheadr;

 la=(XSWAP *)ie_getswap(fajl->linearray);
 if(la && i<=fajl->maxlines)
 {
//  fajl->arraycache=i;
//  fajl->arraycacheadr=la[i];
  return la[i];
 }
 else
  return IE_NULL;
}

void putXSWAPlineadr(struct ib_editor *fajl, int i,XSWAP adr)
{
 XSWAP *la=(XSWAP *)ie_getswap(fajl->linearray);
 if(la && i<=fajl->maxlines)
 {
  la[i]=adr;
//  fajl->arraycache=i;
//  fajl->arraycacheadr=adr;
  swapmod=1;
 }
}


//funkce pro spravu radku... (tr.: function for line management...)
//==========================
char *ie_getline(struct ib_editor *fajl, int i)
{
 if (i>=fajl->lines || fajl==NULL) return NULL;
 if(fajl->aktrad==i)
 {
  int modified=fajl->modified;
  ie_putline(fajl,fajl->aktrad,fajl->rad);
  fajl->modified=modified;
 }
 return ie_getswap(getXSWAPlineadr(fajl,i));
}//endsub

//!!JdS 2004/3/6 {
void ie_getcookie(char *cookie, struct ib_editor *jar, int start)
//Fetch a cookie from the "cookie jar"
{
 int j;
 char *ptr;
 cookie[0] = '\0';
 if (start+CookieCrumbs<=jar->lines)
 {
  //Each cookie is stored as crumbs, so collect and join these together
  for (j=0; j<CookieCrumbs; j++)
  {
   ptr = ie_getline(jar,start+j);
   if (ptr)
    if ((ptr[0]!='\\')||(ptr[1]!='\0'))  //Omit "\" placeholder strings
     joinstr(cookie,MAXARGBUF,ptr);
  }
 }
}//ie_getcookie
//!!JdS 2004/3/6 }

int ie_putline(struct ib_editor *fajl, int i, char *str)
{
 if (i>=fajl->lines || fajl==NULL) return 2;
 //firstswap=fajl->firstswap;
 if (getXSWAPlineadr(fajl,i)==IE_NULL)
  putXSWAPlineadr(fajl,i,ie_putswap(str,strlen(str),fajl->swapcontext));
 else
  putXSWAPlineadr(fajl,i,ie_chngswap(str,getXSWAPlineadr(fajl,i),fajl->swapcontext));
 fajl->modified=1;
 if (getXSWAPlineadr(fajl,i)==IE_NULL) return 2;
 if(fajl->aktrad==i)
 {
  makestr(fajl->rad,str,IE_MAXLEN);
  fajl->modrad=0;
 }
 return 1;
}//end sub

//!!JdS 2004/3/6 {
int ie_putcookie(struct ib_editor *jar, int start, char *cookie)
//Replace a selected cookie in the "cookie jar"
{
 int j, rc=1, len=strlen(cookie);
 char savchr, *ptr=cookie;
 if (start+CookieCrumbs <= jar->lines)
 {
  //Break the cookie into crumbs, to fit it in the "cookie jar"
  for (j=0; j<CookieCrumbs; j++)
  {
   if (len>CrumbSize)  //limit the size of this crumb
   {
    savchr = ptr[CrumbSize];
    ptr[CrumbSize] = '\0';
   }
   if (len)
    rc = ie_putline(jar,start+j,ptr); //crumb available, place in the jar
   else
    rc = ie_putline(jar,start+j,"\\"); //no more crumbs, use a placemarker
   //prepare the next crumb, if available
   if (len>CrumbSize)
   {
    ptr[CrumbSize] = savchr;  //restore the size of the remaining cookie
    ptr = &ptr[CrumbSize];
    len -= CrumbSize;
   }
   else
    len = 0;
  }//for
 }
 else
  return 2;
 return rc;
}//ie_putcookie
//!!JdS 2004/3/6 }

int ie_insline(struct ib_editor *fajl, int i, char *str)
{
 int rc;

 if (i>fajl->lines || fajl==NULL || fajl->lines>=IE_MAXLINES) return 2;
 if (i<fajl->lines)
 {
  XSWAP *linearray=getXSWAPlineptr(fajl);
  memmove(&linearray[i+1],&linearray[i],sizeof(XSWAP)*(fajl->lines-i));
//  fajl->arraycache=-1; //invalidate cache
  linearray[i]=IE_NULL; //undefined row...
  swapmod=1;
 }//endif
 fajl->lines++;
 rc=ie_putline(fajl,i,str);
 return rc;
}//end sub

//!!JdS 2004/3/6 {
int ie_inscookie(struct ib_editor *jar, int start, char *cookie)
//Insert a new cookie into the "cookie jar"
{
 int j, rc=1, len=strlen(cookie);
 char savchr, *ptr=cookie;
 if (jar->lines+CookieCrumbs <= jar->maxlines)
 {
  //Break the cookie into crumbs, to fit it in the "cookie jar"
  for (j=0; j<CookieCrumbs; j++)
  {
   if (len>CrumbSize)  //limit the size of this crumb
   {
    savchr = ptr[CrumbSize];
    ptr[CrumbSize] = '\0';
   }
   if (len)
    rc = ie_insline(jar,start+j,ptr); //crumb available, place in the jar
   else
    rc = ie_insline(jar,start+j,"\\"); //no more crumbs, use a placemarker
   //prepare the next crumb, if available
   if (len>CrumbSize)
   {
    ptr[CrumbSize] = savchr;  //restore the size of the remaining cookie
    ptr = &ptr[CrumbSize];
    len -= CrumbSize;
   }
   else
    len = 0;
  }//for
 }
 else
  return 2;
 return rc;
}//ie_inscookie
//!!JdS 2004/3/6 }

int ie_delline(struct ib_editor *fajl, int i)
{
 if (i>=fajl->lines || fajl==NULL || i<0) return 2;
 if (ie_delswap(getXSWAPlineadr(fajl,i),-1)!=1) return 2;
 if (i<fajl->lines-1)
 {
  XSWAP *linearray=getXSWAPlineptr(fajl);
  memmove(&linearray[i],&linearray[i+1],sizeof(XSWAP)*(fajl->lines-i-1));
//  fajl->arraycache=-1; //invalidate cache
  swapmod=1;
 }
 fajl->lines--;
 putXSWAPlineadr(fajl,fajl->lines,IE_NULL); //odkazuje to nikam...
    // tr.: points to nowhere
 fajl->modified=1;
 //fajl->x=0;
 if(fajl->aktrad==i)
  fajl->aktrad=-1;
 return 1;
}//end sub

//!!JdS 2004/3/6 {
int ie_delcookie(struct ib_editor *jar, int posn)
//Delete a selected cookie from the "cookie jar"
{
 int i, rc=1;
 if (posn+CookieCrumbs <= jar->lines)
  for (i=0; i<CookieCrumbs; i++)
   rc = ie_delline(jar,posn);
 else
  return 2;
 return rc;
}//ie_delcookie
//!!JdS 2004/3/6 }

long ie_free(void)
{
 int i=0;
 long j=0;

 do
 {
  j+=IE_MAXSWAPLEN-swapsize[i];
 }
 while(++i<IE_MAXSWAP);
 return j;
}//end sub

long ie_used(void)
{
 int i=0;
 long j=0;

 do
 {
  j+=swapsize[i];
 }
 while(++i<IE_MAXSWAP);
 return j;
}//end sub


//pro spravu otevreneho souboru: (tr.: for management of open file)
//==============================

void ie_destroy(void)
{
 int i=0;

#ifndef POSIX
 if(swapstr!=NULL)farfree(swapstr);swapstr=NULL;
#endif
 firstswap=0;
 while(i<IE_MAXSWAP)
  ie_killswap(i++);
}


//smazat soubor (tr.: delete file)
void ie_clearf(struct ib_editor *fajl,char all) //kdyz je all 0, tak nemazat
  // tr.: if all is 0, then do not delete
{

 if(all)
 {
  ie_destroy();
 }
 else
 {
  //tohle je udelany zatim provizorne!!!
  //tady se budou swapy ktere pati jenom jednomu souboru mazat kopmpletne...
  // tr.: this is provisional for the moment
  //      here those swaps belonging only to one file will be deleted entirely/completely
  int i=0;

  while(i<fajl->lines)
  {
   ie_delswap(getXSWAPlineadr(fajl,i),-1);
   i++;
  }//loop
 }//endif
// ie_closef(fajl);
}//end sub

/* not necessary:
void ie_closef(struct ib_editor *fajl)
{
 if(fajl->lineadr!=NULL)farfree(fajl->lineadr);fajl->lineadr=NULL;
}
*/

//simple openf

int ie_openf(struct ib_editor *fajl,int context) //load, or open, 1. or 2.
{
 fajl->killcomment=0;
 return ie_openf_lim(fajl,context,10000);
}

//open/load from disk
int ie_openf_lim(struct ib_editor *fajl,int context,int max) //load, or open, 1. or 2.
{
 int i;
 XSWAP *linearray;

 fajl->x=0;
 fajl->y=0;
 fajl->lines=0;
 fajl->zoomx=0; //tyka se zobrazeneho vyrezu
   // tr.: applies to the window on the screen
 fajl->zoomy=0;
 fajl->bbx=0; //tyka se bloky
   // tr.: applies to block
 fajl->bby=0;
 fajl->bex=0;
 fajl->bey=0;
 fajl->blockflag=0;
 fajl->insert=1;
//!!glennmcc: Apr 07, 2008 -- default to wrap (see also html.c)
 fajl->wordwrap=1;
// fajl->wordwrap=0;
//!!glennmcc: end
 fajl->swapcontext=context;
 fajl->cols=0;
 fajl->killstr[0]='\0';
 fajl->aktrad=-1; //aktualni radek neni definovan...
   // tr.: current line is not defined
 fajl->modrad=0;

 IE_MAXLINES=max;

// fajl->lineadr=farmalloc(sizeof(XSWAP)*(IE_MAXLINES+2));
// if(fajl->lineadr==NULL) return 2;

// fajl->arraycache=-1;
// fajl->arraycacheadr=IE_NULL;
 fajl->linearray=ie_putswap(NULL,(max+2)*sizeof(XSWAP),context);

 linearray=getXSWAPlineptr(fajl);

 i=0;
 while (i<IE_MAXLINES)
 {
  linearray[i]=IE_NULL;
  i++;
 }//loop

 if ( /*swapbuf == NULL ||*/ swapstr == NULL ) return 2;

 fajl->modified=0; //!!!
 {
  int rv=ie_insblock(fajl,fajl->filename);
  if(!fajl->filename[0]) strcpy(fajl->filename,"noname.txt");
  return rv;
 }

}//end sub

int ie_insblock(struct ib_editor *fajl,char *filename)
{
 int i,f=-1;
 char *inbuf=NULL;

 if(filename[0])
#ifdef POSIX
  f=a_open(filename,O_RDONLY,S_IREAD);
#else
  f=a_sopen(filename,O_RDONLY|O_TEXT,SH_COMPAT | SH_DENYNONE, S_IREAD);
#endif
 if(f==-1)
 {
  return 1;
 }
 else
 {
  int lenread=16000, precteno=0, rv=1, rpos=0, modified=0;
  char radka[IE_MAXLEN+1],end=0;
  char restofline[IE_MAXLEN+1]="\0",breakline=0;
  int y=fajl->y;

  if(fajl->x>0 || fajl->maxlines==1) //special case
  {
   char *ptr=ie_getline(fajl,y);
   if(ptr)
   {
    makestr(radka,ptr,IE_MAXLEN);
    makestr(restofline,&ptr[fajl->x],IE_MAXLEN);
    rpos=fajl->x; //set current position in string row
    ie_putline(fajl,y,radka);
    breakline=1;
   }
  }

  fajl->aktrad=-1; //aktualni radek neni definovan...
   // tr.: current line is not defined...
  fajl->modrad=0;

  inbuf=(char *)farmalloc(lenread+1);
  if(inbuf==NULL) {rv=4;goto exit_load;} //chyba - malo pameti
    // tr.: error - no more memory left

  // load file
  do
  {
   precteno=a_read(f,inbuf,lenread);

   if(precteno<0)
   {
    end=1;
    rv=2;
   }
   else
   if(precteno==0)
   {
    end=1;
    if(rpos>0 && radka[rpos-1] !='\n')
    {
     precteno=1;
     inbuf[0]='\n';
    }

   }
   i=0;

   while(i<precteno)
   {
    radka[rpos]=inbuf[i];
    if(radka[rpos]=='\n' || (radka[rpos]==' ' && rpos>9*IE_MAXLEN/10) || rpos>=IE_MAXLEN-1)
    {
     if(rpos>=IE_MAXLEN-1)
      {rpos=IE_MAXLEN;rv=3;}//zaokorouhleni radku
        // tr.: rounding lines 
     else
     if(radka[rpos]!='\n')
      modified=1;
     radka[rpos]='\0';
     if(rpos>fajl->cols)
      fajl->cols=rpos;
     rpos=0;
     if(!fajl->killcomment || !strchr(";!#'",radka[0]))
     {
      if(breakline)
      {
       if(ie_putline(fajl,y,radka)!=1)
        {rv=5;goto exit_load;} //moc radku, moc velky soubor
          // tr.: too many lines, file is too big 
       breakline=0;
      }
      else
      {
       if(ie_insline(fajl,y,radka)!=1)
        {rv=5;goto exit_load;} //moc radku, moc velky soubor
          // tr.: too many lines, file is too big 
      }
      y++;

     }
    }
    else rpos++;
    i++;
   }//loop

  }
  while (!end && precteno>0);

  exit_load:

  if(y==fajl->y)
   y++;

  if(fajl->y>0 || y<fajl->lines)
  {
   fajl->blockflag=2; //visible
   fajl->bby=fajl->y;
   fajl->bey=y;
   fajl->bbx=fajl->x;
   fajl->bex=0;
  }

  if(restofline[0]) //special case - was "breakline" mode
  {
   char *ptr=ie_getline(fajl,fajl->y);
   if(ptr)
   {
    int l;
    makestr(radka,ptr,IE_MAXLEN);
    l=strlen(radka);
    makestr(&radka[l],restofline,IE_MAXLEN-l);
    ie_putline(fajl,y-1,radka);
    fajl->bey=y-1;
    fajl->bex=l;
   }
  }

  a_close (f);
  if(inbuf!=NULL)farfree(inbuf);
  fajl->modified=modified;
  return rv;
 }//end if
}//end sub

#ifndef MINITERM

int ie_savef(struct ib_editor *fajl)
{
 int f,i=1,l;
 char *r,radka[IE_MAXLEN+1];

 if(fajl->aktrad!=-1)
  ie_putline(fajl,fajl->aktrad,fajl->rad);
#ifdef POSIX
 f = a_open( fajl->filename, O_RDWR | O_CREAT | O_TRUNC, S_IREAD|S_IWRITE );
#else
 f = a_sopen( fajl->filename, O_RDWR | O_TEXT | O_CREAT | O_TRUNC,
     SH_COMPAT | SH_DENYNONE, S_IREAD|S_IWRITE );
#endif
 if ( f == -1 ) return 2; //chyba pri otevirani
   // tr.: error while opening

 r=ie_getswap(getXSWAPlineadr(fajl,0));
 if (r!=NULL)
 {
  l=strlen(r);
  if(write(f,r,l)!=l) {a_close (f); return 4;} //chyba pri psani!
   // tr.: error while writing
 }//endif
 while(i<fajl->lines)
 {
  r=ie_getswap(getXSWAPlineadr(fajl,i));
  if (r!=NULL)
  {
   radka[0]='\n';
   radka[1]='\0';
   strcat(radka,r);
   l=strlen(radka);
   if(fajl->killstr[0])
   {
    if(r[0]!=';')
    {
     char *ptr=strstr(r,fajl->killstr);
     if(ptr && strchr(r,' ')>ptr)
      goto cont;
    }
   }
   if(write(f,radka,l)!=l)
    {a_close (f); return 4;} //chyba pri psani!
   // tr.: error while writing
  }//endif
  cont:
  i++;
 }//loop
 a_close(f);
 fajl->modified=0;
 return 1;
}//end sub


/*

//fce vyrizne slovo na pozici kurzoru
// tr.: fce cuts word at cursor position
int ie_cutword(struct ib_editor *fajl,char *slovo)
{
 int pos,i=0;
 char *r;

 slovo[0]='\0';
 if(fajl==NULL) return 2;

 if(fajl->aktrad!=fajl->y)//zapamatovani posledni opracovane radky
 // tr.: remember last row that has been worked on
  r=ie_getswap(fajl->lineadr[fajl->y]);
 else
  r=fajl->rad;

 pos=fajl->x;
 while(pos>0 && r[pos-1]>='A')pos--;
 while(r[pos]>='A' && i<39) slovo[i++]=r[pos++];
 slovo[i]='\0';
 return 1;
}//end sub

//create BAK
void ie_bak(char* filename)
{
 char drv[3],pth[65],name[10],ext[5];
 char bak[80];

 fnsplit(filename,drv,pth,name,ext);
 if(strcmpi(ext,".BAK")!=0)
 {
  fnmerge(bak,drv,pth,name,".BAK");
  unlink(bak);
  rename(filename,bak);
 }//end if
}//end sub

*/
#endif

#ifndef POSIX
int ie_which(int i)
{
 if(swap2 && i==which1 || !swap2 && i==swapnum)
  return 1;
 else if(swap2 && i==which2)
  return 2;
 else
  return 0;

}
#endif

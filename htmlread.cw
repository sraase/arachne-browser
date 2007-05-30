
// ========================================================================
// HTML source read routines for Arachne WWW browser
// (c)1998-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "html.h"
#include "internet.h"

#ifdef POSIX
extern int socknum;
#endif

int openHTML(struct HTTPrecord *cache,char source)
{
 //-------------------------------------------------------------- http open
 if(source==HTTP_HTML && arachne.target==p->currentframe)
 {
  if(!cache->knowsize)
   cache->size=10000000l;
  return 1;
 }
 //----------------------------------------------------------- history open
 else if(source==HISTORY_HTML)
 {
  cache->knowsize=0;
  cache->size=10000000l;

  history.y=history.lines-1;
  return 1;
 }
 else // source==LOCAL_HTML
 {
 //------------------------------------------------------------- local open
  char *fnameptr;

  if(p->htmlframe[p->currentframe].status==MAIL && GLOBAL.source)
   fnameptr=cache->rawname;
  else
   fnameptr=cache->locname;
#ifdef POSIX
  cache->handle=a_open(fnameptr,O_RDONLY, S_IREAD);
#else
  cache->handle=a_fast_sopen(fnameptr,O_RDONLY|O_TEXT,SH_COMPAT, S_IREAD);
#endif
  if(cache->handle<0)
   return 0;
  cache->size=a_filelength(cache->handle);
  cache->knowsize=1;
  //printf("Size of object is %ld\n",cache->size);
  return 1;
 }
}

int readHTML(struct HTTPrecord *cache,char source)
{
 //-------------------------------------------------------------- http read
#ifndef NOTCPIP
 if(source==HTTP_HTML && arachne.target==p->currentframe)
#ifdef POSIX
  return tickhttp(cache,p->buf,socknum);
#else
  return tickhttp(cache,p->buf,socket);
#endif
 //----------------------------------------------------------- history read
 else
#endif
 if(source==HISTORY_HTML)
 {
  if(history.y>0)
  {
   char *ptr="\0",*ptr2="\0",*line;
   char *head="\0";

   if(history.y==history.lines-1)
    head="<TITLE>History</TITLE><BODY>\n";
   if(history.y==arachne.history)
   {
    ptr="<A NAME=\"current\"><B>";
    ptr2="</B>";
   }
   line=ie_getline(&history,history.y);
   sprintf(p->buf,"%s%s<LI><A HREF=\"%s\">%s</A>%s\n",head,ptr,line,line,ptr2);
   history.y--;
   return strlen(p->buf);
  }
 }
 //------------------------------------------------------------- local read
 //not used in overalid executable!
 else
 {
  int ret=a_read(cache->handle,p->buf,BUF);

  if(ret<0)
   return 0;
  else
   return ret;
 }
 return 0;
}

void closeHTML(struct HTTPrecord *cache,char source)
{
 //-------------------------------------------------------------- http close
#ifndef NOTCPIP
 if(source==HTTP_HTML && p->currentframe==arachne.target)
  closehttp(cache);
 else
#endif
 if(cache->handle>=0 && source==LOCAL_HTML)
  a_close(cache->handle);

}

// werner scholz  begin  Nov 8,2006   ------  utf8-table  -------
unsigned char utf8table(unsigned char x1,unsigned char x2,unsigned char x3,unsigned char x4)
{ unsigned char c;
   c=164;          // square is default if utf8 converting fails !

  if(x1==194)     // C2
  {if((x2>160)&&(x2<192))c=x2;}

  if(x1==195)     // C3
  {if((x2>127)&&(x2<192))c=x2+64;}
  // End  "basic charset"  ISO-8859-1 /ISO-8859-2

  // More utf8 characters can be added here ....
  if(x1==196)     // C4
  {if(x2==141) c=135;
   if(x2==140) c=135;
   if(x2==184) c=227;
   if(x2==131) c=229;
  }
  if(x1==197)     // C5
  {if(x2==161) c=154;
  }
  if(x1==226)      // E2
  {if((x2==128)&&(x3==158))c=132;
   if((x2==128)&&(x3==156))c=148;
   if((x2==128)&&(x3==147))c=173;
   if((x2==128)&&(x3==148))c=45;
   if((x2==128)&&(x3==162))c=183;
  }
 // End of ISO-8859-1

 if(RENDER.utf8==1)return c;   // return if  "charset=ISO-8859-1"

 // From now on we translate to "charset=ISO-8859-2"

  if(x1==195)      // c3
  {if(x2==135) c=199;
   if(x2==147) c=211;
   if(x2==167) c=231;
   if(x2==179) c=243;
  }

 if(x1==196)     // C4
  {
   if(x2==130) c=196;  // Romania: not quite correct !
   if(x2==131) c=228;  // Romania: not quite correct !
   if(x2==132) c=161;
   if(x2==133) c=177;
   if(x2==134) c=198;
   if(x2==135) c=230;
   if(x2==140) c=200;
   if(x2==142) c=207;
   if(x2==143) c=239;
   if(x2==144) c=208;
   if(x2==145) c=240;
   if(x2==152) c=202;
   if(x2==153) c=234;
   if(x2==154) c=137;
   if(x2==155) c=136;
   if(x2==189) c=156;
   if(x2==176) c=105; // Turkey: not quite correct i
   if(x2==159) c=103; // Turkey: not quite correct g
   if(x2==177) c=73;  // Turkey: not quite correct I
  }

  if(x1==197)      // C5
  {if(x2==158) c=170;
   if(x2==159) c=186;
   if(x2==129) c=163;
   if(x2==132) c=241;
   if(x2==131) c=209;
   if(x2==155) c=182;
   if(x2==154) c=166;
   if(x2==144) c=213;
   if(x2==145) c=245;
   if(x2==176) c=219;
   if(x2==177) c=251;
   if(x2==186) c=188;
   if(x2==185) c=172;
   if(x2==188) c=191;
   if(x2==187) c=175;
   if(x2==135) c=210;
   if(x2==136) c=242;
   if(x2==152) c=216;
   if(x2==153) c=248;
   if(x2==160) c=169;
   if(x2==164) c=134;
   if(x2==165) c=187;
   if(x2==174) c=252;
   if(x2==175) c=249;
   if(x2==189) c=142;
   if(x2==190) c=190;
   if(x2==162) c=222;
   if(x2==163) c=254;
   if(x2==148) c=192;
   if(x2==149) c=224;
   if(x2==130) c=179; // Poland: is this original character ?
  }
    return c;   // x4 is not used yet !
}
// werner scholz end

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
  // Here ends charset ISO-8859-1

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
 return c;
}
// werner scholz end


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
  return tickhttp(cache,p->buf,&socknum);
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
{
 long uc;//!!glennmcc: Mar 27, 2008 -- convert UTF-8 into numerical equiv
 char number[11], *ptr;

 unsigned char c;
   c=127;          // square is default if utf8 converting fails !

  if(x1==194)     // C2
  {
   if((x2>160)&&(x2<192))c=x2;
  }

  if(x1==195)     // C3
  {
   if((x2>127)&&(x2<192))c=x2+64;
  }
  // Here ends charset ISO-8859-1

//!!glennmcc: Mar 27, 2008 -- convert UTF-8 into numerical equiv
//and then read its needed character from entity.cfg
  if(x1>=192 && x1<=223)//2 byte UTF-8 encoded characters
  {
   uc=(x1-192) * 64 + (x2-128);
  }

  if(x1>=224 && x1<=239)//3 byte UTF-8 encoded characters
  {
   uc=(x1-224) * 4096 + (x2-128) * 64 + (x3-128);
  }

  if(x1>=240 && x1<=247)//4 byte UTF-8 encoded characters
  {
   uc=(x1-240) * 262144l + (x2-128) * 4096 + (x3-128) * 64 + (x4-128);
  }

   if((uc>31 && uc<128)/* || (uc>159 && uc<256)*/) return uc;
   ptr = config_get_entity(ltoa(uc,number,10));
   if(ptr)
   {
//    Piip();//during testing... 'beep' when a conversion takes place
    return *ptr;
   }
return c;//1st part of Werners code returns char when not found in entity.cfg
}

//Werner's original conversion code below now inactivated
//but can be re-activated if needed
//!!glennmcc: end

/*


  // More utf8 characters can be added here ....

  if(x1==196)     // C4
  {
   if(x2==141) c=135;
   if(x2==140) c=135;
   if(x2==184) c=227;
   if(x2==131) c=229;
  }

  if(x1==197)     // C5
  {
   if(x2==161) c=154;
//   if(x2==146) c=140;//!!glennmcc: Mar 16, 2008 -- for OElig
//   if(x2==147) c=156;//!!glennmcc: Mar 16, 2008 -- for oelig
//   if(x2==139) c=110;//!!glennmcc: Mar 26, 2008 -- simulate 'nj'
  }

//  if(x1==201)     // C9
//  {
//   if(x2==170) c=105;//!!glennmcc: Mar 26, 2008
//  }

  if(x1==226)      // E2
  {
   if((x2==128)&&(x3==158))c=132;
   if((x2==128)&&(x3==156))c=148;
   if((x2==128)&&(x3==147))c=173;
   if((x2==128)&&(x3==148))c=45;
   if((x2==128)&&(x3==162))c=183;
   if((x2==128)&&(x3==153))c=39;//!!glennmcc: Mar 26, 2008 -- apostrophe
  }
 return c;
}
// werner scholz end
*/

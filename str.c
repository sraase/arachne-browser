
// ==================================================================
// str.h is Arachne Labs enhancent and replacement of string.h
// GNUpyright (G) 1999-2000 Michael Polak, Arachne Labs
// ==================================================================

#include "str.h"

// ==================================================================
// pathstr() creates pathname which is terminated by DIRSLASHCHAR
// *dest must be allocated at least one byte bigger than *src!!!
// ==================================================================
char *pathstr(char *dest, char *src)
{
 int l=strlen(src);
 strcpy(dest,src);

 if(l>0 && dest[l-1]!=DIRSLASHCHAR)
 {
  dest[l]=DIRSLASHCHAR;
  dest[l+1]='\0';
 }

 return dest;
}

// ==================================================================
// makestr() is common-sense compatible replacement for both strcpy()
// and strncpy()
// ==================================================================

char *makestr(char *dest, char *src, int lim)
{
 if(lim>=0 && dest && src)
 {
  char *realdest=dest;
  int i=0;
  
  while(*src && i++<lim)
   *(realdest++)=*(src++); 
  *realdest='\0';
  return dest;
 }
 else
  return src;
}

// ==================================================================
// joinstr() is an alternative to strcat() and strncat() that
// incorporates a safety limit for the 'dest' string. Parameter
// ordering is deliberately different to strncat().  !!JdS 2004/1/17
// ==================================================================

char *joinstr(char *dest, int lim, const char *src)
{
 int used = strlen(dest) + 1;
 if(lim>used && dest && src)
  strncat(dest,src,lim-used);
 return dest;
}

#ifdef POSIX

// ==================================================================
// reimplementation of some Borland-libc string.h functions, using
// only pure Linux-libc calls.
// ==================================================================

char *strlwr(char *str)
{
 char *pushstr=str;
 while(*str)
 {
  *str=tolower(*str);
  str++;
 }
 return pushstr;
}

char *strupr(char *str)
{
 char *pushstr=str;
 while(*str)
 {
  *str=toupper(*str);
  str++;
 }
 return pushstr;
}

#endif

#ifdef LINUX
char *itoa(int val, char *s, int base)
{
    switch(base) {
    case  8: sprintf(s, "%o", val); break;
    case 10: sprintf(s, "%d", val); break;
    case 16: sprintf(s, "%x", val); break;
    default: abort();
    }
    return s;
}

char *ltoa(long val, char *s, long base)
{
    switch(base) {
    case  8: sprintf(s, "%lo", val); break;
    case 10: sprintf(s, "%ld", val); break;
    case 16: sprintf(s, "%lx", val); break;
    default: abort();
    }
    return s;
}
#endif

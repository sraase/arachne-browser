
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

int strncmpi(char *str1,char *str2, size_t n)
{
 char c1,c2;
 size_t l=0;
 
 if(n==0) 
  return 0;
 
 if(str1 && str2)
 { 
//  printf("[%.*s|%.*s]",(int)n,str1,(int)n,str2);
  while(*str1 && *str2)
  {
   c1=toupper(*str1);
   c2=toupper(*str2);
   if(c1<c2)
   return -1;
   else
   if (c1>c2)
   return 1;
 
   str1++;
   str2++;
   l++;
   if(l==n)
   return 0;
  }
 }
 
 if(str1 && *str1)
  return 1;
 else
 if(str2 && *str2)
  return -1;
 else
  return 0; 
}

int strcmpi(char *str1,char *str2)
{
 return strncmpi(str1,str2,-1);
}

#endif

#ifdef LINUX
char* itoa(int val, int base)
{
 static char buf[32] = {0};
 int i = 30;
 for(; val && i ; --i, val /= base)
 buf[i] = "0123456789abcdef"[val % base];
 return &buf[i+1];
}

char* ltoa(long val, long base)
{
 static char buf[32] = {0};
 int i = 30;
 for(; val && i ; --i, val /= base)
 buf[i] = "0123456789abcdef"[val % base];
 return &buf[i+1];
}
#endif

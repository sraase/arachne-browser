
// ==================================================================
// str.h is Arachne Labs enhancent and replacement of string.h
// it offers some quite
// GNUpyright (Gnu) 1999 Michael Polak, Arachne Labs
// ==================================================================

#ifndef _alabs_str_h
#define _alabs_str_h

#define DIRSLASHCHAR '\\'   //MS-DOS
//#define DIRSLASHCHAR '/'    //Unix

#include "posix.h"

// ==================================================================
// pathstr() creates pathname which is terminated by DIRSLASHCHAR
// *dest must be allocated at least one byte bigger than *src!!!
// ==================================================================
char *pathstr(char *dest, char *src);

// ==================================================================
// makestr() is common-sense compatible replacement for both strcpy()
// and strncpy()
// ==================================================================
char *makestr(char *dest, char *src, int lim);

// ==================================================================
// joinstr() is an alternative to strcat() and strncat() that
// incorporates a safety limit for the 'dest' string. Parameter
// ordering is deliberately different to strncat().  !!JdS 2004/1/17
// ==================================================================
char *joinstr(char *dest, int lim, const char *src);

#ifdef POSIX
char *strlwr(char *str);
char *strupr(char *str);
int strncmpi(char *str1,char *str2, size_t n);
int strcmpi(char *str1,char *str2);
#endif

#endif


#ifndef _POSIXH_
#define _POSIXH_

/* enable format-string checking if available */
#ifdef __GNUC__
#define HAS_FORMAT(FMT, AP) __attribute__((format(printf, FMT, AP)))
#else
#define HAS_FORMAT(FMT, AP)
#endif

#ifdef MSDOS

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dos.h>
#include <alloc.h>
#include <conio.h>
#include <io.h>
#include <bios.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <share.h>
#include <time.h>
#include <dir.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#ifdef __BORLANDC__
/* define some C99 fixed width integer types,
   because Borland C++ 3.1 lacks stdint.h */
typedef signed   char  int8_t;
typedef unsigned char  uint8_t;
typedef signed   short int16_t;
typedef unsigned short uint16_t;
typedef signed   long  int32_t;
typedef unsigned long  uint32_t;
#if defined(__COMPACT__) || defined(__LARGE__) || defined(__HUGE__)
typedef signed   long  intptr_t;
typedef unsigned long  uintptr_t;
#else
typedef signed   short intptr_t;
typedef unsigned short uintptr_t;
#endif
#else
#include <stdint.h>
#endif

#define PATHSEP '\\'

#elif LINUX
//==========================================================================

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>

char *itoa(int val, char *s, int base); /* in str.c */
char *ltoa(long val, char *s, long base); /* in str.c */
int filelength (int handle);  /* in linglue.c */

#define strncmpi strncasecmp
#define strcmpi strcasecmp
#define farmalloc malloc
#define farfree free
#define sopen open
#define far
#define O_BINARY 0
#define O_TEXT   0

#define PATHSEP '/'

#endif
#endif //POSIXH

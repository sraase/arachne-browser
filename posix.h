
#ifndef _POSIXH_
#define _POSIXH_

#ifdef MSDOS

#include <stdio.h>
#include <stdlib.h>
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


#elif LINUX
//==========================================================================

//....  will include standard header files later ....
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

int filelength (int handle);  /* in linglue.c */

#define farmalloc malloc
#define farfree free
#define sopen open
#define far
#define O_BINARY 0
#define O_TEXT   0
 
//========================================================================== 
#else  //for now: CLEMENTINE 

#include <clementine/types.h>
#include <clementine/string.h>
#include <clementine/file.h>
#include <clementine/ctype.h>

extern int posixErrNo;
void *malloc(size_t size);
void free(void *ptr);
int printf(char *format, ...);
int sprintf (char *buffer, const char *fmt, ...);
int puts(const char *s);
int atoi (const char *s);
int atol (const char *s);
int getchar (void);
int putchar (int c);
int open(char* path, int oflags, ...);
ssize_t read(int fildes, void *buf, size_t nbyte);
ssize_t write(int fildes, void *buf, size_t nbyte);
off_t lseek(int fildes, off_t offset, int whence);
int filelength (int handle);
int close(int fildes);

#define farmalloc malloc
#define farfree free
#define sopen open
#define far 
#define O_BINARY 0
#define O_TEXT   0
#define min(a,b) ((a)<=(b) ? (a) : (b))
#define max(a,b) ((a)>=(b) ? (a) : (b))

#endif
#endif //POSIXH

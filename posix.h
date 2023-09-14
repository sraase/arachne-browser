
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
#include <time.h>

char *itoa(int val, char *s, int base); /* in str.c */
char *ltoa(long val, char *s, long base); /* in str.c */
int filelength (int handle);  /* in linglue.c */

#define farmalloc malloc
#define farfree free
#define sopen open
#define far
#define O_BINARY 0
#define O_TEXT   0
 
#endif
#endif //POSIXH


#ifdef CLEMENTINE
#include "posix.h"
#else
#include <time.h>
#include <string.h>
#endif

void inettime(char *tm)
{
 char str[27];
 long t=time(NULL);

 strcpy(str,ctime(&t));

 memset(tm,' ',28);
 //Mon Oct 20 11:31:54 1952
 //-------------------------
 //Sat, 07 Dec 1996 15:13:04 -0800
 strncpy(tm,str,3);
 tm[3]=',';
 strncpy(&tm[5],&str[8],2);
 strncpy(&tm[8],&str[4],3);
 strncpy(&tm[12],&str[20],4);
 strncpy(&tm[17],&str[11],8);
 tm[25]='\0';
}

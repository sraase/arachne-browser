
#include "arachne.h"

int file_exists(char *str)
{
#ifdef POSIX
 int f=a_open(str,O_RDONLY,0);
 if(f<0)
  return 0;
 close(f);
 return 1;
#else
 struct ffblk ff;
 return !findfirst(str,&ff,0);
#endif
}


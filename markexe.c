// Not in the project

#include <io.h>
#include <fcntl.h>
#include <alloc.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECKBUF 16000


long calcexe(char *exename,long offset)
{
 long x=0;
 int i=0,step[5]={7,3,1,2,7},j=0;
 int f=a_open(exename,O_RDONLY|O_BINARY);
 char *buf=farmalloc(CHECKBUF+1);

 if(buf)
 {
  if(f>=0)
  {
   lseek(f, offset, SEEK_SET);
   a_read(f,buf,CHECKBUF);

   while(j<CHECKBUF)
   {
    x+=buf[j];
    j+=step[i++];
    if(i==4)i=0;
   }

   a_close(f);

   return x;
  }
  farfree(buf);
 }

 puts("?!");
 exit(0);
}

void main(char argc,char **argv)
{
 int f;
 long checksum,offset;

 if(argc==3)
 {
  puts("Calculating...");
  offset=atol(argv[2]);
  checksum=calcexe(argv[1],offset);
  puts("Updating...");
  f=a_open(argv[1],O_WRONLY|O_BINARY|O_APPEND);
  if(f>=0)
  {
   write(f,&offset,4);
   write(f,&checksum,4);
   a_close(f);
  }
 }
 else
  puts("MARKEXE filename offset");
}

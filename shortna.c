#include <string.h>
#include <malloc.h>
#include <dos.h>
#include <stdio.h>

unsigned int LFNError = 0;
/*
 2000-03-06 Chris
 The function takes a longname and fills the longname string
 with the shortname if found else the longname still is the input name
 it returns returns a pointer to the longname.
 Only tested in Win9x (lfndos 1.06 didn't work)
 LFNError holds the errorcode
*/
char *getShortname(char *longname)
{
 struct REGPACK reg;
 //int ret;

 char *buffer = (char *) malloc(128);
 /*
  67-byte (possibly 128-byte) buffer for short filename.

  Maybe you should use some internal memmory allocation (mem_calloc) with
  allocation success test.
 */

 reg.r_ax = 0x7160;
 reg.r_cx = 0x0001;//0x8001 gives the substed path
 reg.r_si = FP_OFF(longname);
 reg.r_ds = FP_SEG(longname);
 reg.r_di = FP_OFF(buffer);
 reg.r_es = FP_SEG(buffer);
 intr(0x21, &reg);
 if(reg.r_flags & 1) /*Carry interrupt failed*/
 {
  /*
    AX = error code
    02h invalid component in directory path or drive letter only
    03h malformed path or invalid drive letter
  */
  LFNError = reg.r_ax;
  free(buffer);
  return longname;
 }
 strcpy(longname,buffer);
 free(buffer);
 return(longname);
}

#ifdef SHORT2LONG

/*
 Returns a poiner to the longname if found
 else returns NULL
*/
char *getLongname(char *shortname)
{
 struct REGPACK reg;
 int ret;

 char *buffer = (char *)mem_calloc(261);

 reg.r_ax = 0x7160;
 reg.r_cx = 0x0002;//0x8002 gives the substed path
 reg.r_si = FP_OFF(shortname);
 reg.r_ds = FP_SEG(shortname);
 reg.r_di = FP_OFF(buffer);
 reg.r_es = FP_SEG(buffer);
 intr(0x21, &reg);
 if(reg.r_flags & 1) /*Carry interrupt failed*/
 {
  free(buffer);
  return NULL;
 }
 return(buffer);
}


/*
Exchanges the shortname in the shortname buffer with the longname if found
else the shortname remains.
The function returns a pointer to the shortname var.
*/

char *longname(char *path,char *shortname)
{
 char fullname[261];
 char *buffer;
 int i,i2;

 strcpy(fullname,path);
 strcat(fullname,shortname);
 buffer = getLongname(fullname);
 if(buffer == NULL)
  return shortname;

 for(i = strlen(buffer); (i >= 0) && (buffer[i] != '\\');i--);

 strcpy(shortname,"");
 i++;
 for(i2=0;buffer[i];i2++,i++)
  shortname[i2] = buffer[i];
 shortname[i2] = '\0';

 free(buffer);

 return(shortname);
}
#endif
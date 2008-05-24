
// ========================================================================
// All errors are here:
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "html.h"

void memory_destroy(void)
{
#ifndef CLEMTEST
#ifdef VIRT_SCR
 deallocvirtual();
#endif
 x_fnt_cls();
#endif
 mouseoff();
 ie_destroy();
}


//chybove hlaseni na zacatku
// tr.: error message at the beginning
void memerr0(void)
{
 x_grf_mod(3);
#ifdef POSIX
 printf(MSG_MEMERR);
#else
 printf(MSG_MEM);
 printf(MSG_BYTES,MIN_MEMORY-farcoreleft());
#endif
 memory_destroy();
 exit(EXIT_TO_DOS);
}

void memerr(void)
{
 char command[80];

 command[0]='\0';
 savepick();
 ie_savef(&history);
 ie_savebin(&HTTPcache);
 memory_destroy();

#ifdef POSIX
 printf(MSG_MEMERR);
 exit(-1);
#else
 closebat(command,RESTART_NONFATAL_ERROR);
 exit(willexecute(command));
#endif
}

void cfgerr (struct ib_editor *f)
{
 x_grf_mod(3);
 printf(MSG_CFGERR,f->filename,f->maxlines);
 memory_destroy();
 exit(EXIT_TO_DOS);
}

//!!JdS 2004/3/6 {
void badcookiesfile ()
{
#ifndef LINUX
 x_grf_mod(3);
 printf("Error: The 'cookies.lst' file is either broken or incompatible.");
 memory_destroy();
 exit(EXIT_TO_DOS);
#else
return;
#endif
}
//!!JdS 2004/3/6 }

void mallocerr(char *msg, char *file , int line)
{
 x_grf_mod(3);

// debug version
 printf(MSG_ALLOC,msg, line, file);
// non-debug version
// printf(MSG_ALLOC,msg);

 memory_destroy();
 puts("ok");
 exit(EXIT_ABNORMAL); //goto FATAL.HTM
}

void DNSerr(char *host)
{
 char str[128];

 sprintf(str,MSG_DNSERR,host);
 outs(str);
}

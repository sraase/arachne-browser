#define MINUSONE ((DWORD)-1L)
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef signed long S_DWORD;
typedef signed short S_WORD;
typedef signed char S_BYTE;
typedef signed long INT32;
typedef signed short INT16;
typedef signed char INT8;
typedef unsigned long U_INT32;
typedef unsigned short U_INT16;
typedef unsigned char U_INT8;

  typedef unsigned char BOOL;

#undef atoi
  #define atoi(s) atol(s)
  typedef sock_type SOK_T;
  typedef SOK_T *P_SOK_T;
  typedef unsigned int ssize_t;
  #define sockread(s,p,l) sock_fastread(s,(char *) p,l)

/* use SOCKWRITE !!!!! */
  #define sockwrite(s,p,l) sock_write(s,(char *) p,l)
  #define SOCKCLOSE( s ) sock_close(s)
  #define strcasecmp(s,s1)  stricmp(s,s1)
  #define strncasecmp(s,s1,l)  strnicmp(s,s1,l)

#ifdef _TSRMSDOS
    typedef int FD_T;
    #define STDIN 0
    #define STDOUT 1
    #define STDERR 2
 
    #define OPENWA(file) fdxopen(file,1,1)
    #define OPENR(file) fdxopen(file,0,0)
    #define OPENRW(file) fdxopen(file,2,0)
    #define OPENRWA(file) fdxopen(file,2,1)
    #define CLOSE(rcf) fdxclose(rcf)
    #define READCH(fd,buf) fdxreadch(buf,fd)
    #define WRITECH(fd,buf) fdxwritech(buf,fd)
    #define BAD_FILE NULL
    #define FD_EOF 0
#endif
#ifndef TRUE
  #define TRUE 1
#endif

#ifndef FALSE
  #define FALSE 0
#endif
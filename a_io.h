
//Arachne-accelerated I/O operations. write() is not accelerated.

//...something has to be done about sopen() later :(

#ifndef ENABLE_A_IO

#define a_open          open
#define a_fast_open     open
#define a_read          read
#define a_lseek         lseek
#define a_close         close
#define a_eof           eof
#define a_filelength    filelength
#define a_sopen         sopen
#define a_fast_sopen    sopen

#else
int a_open(char *path, int access, unsigned mode);

int a_fast_open(char *path, int access, unsigned mode);

int a_sopen(char *path, int access, int shflag, int mode);

int a_fast_sopen(char *path, int access, int shflag, int mode);

int a_read(int handle, void *buf, unsigned len);

long a_lseek(int handle, long offset, int fromwhere);

int a_close(int handle);

int a_eof(int handle);

// int a_getftime(int handle, struct ftime _FAR *ftimep);

long a_filelength(int handle);

void a_chdir_initialize(void);

int a_alloccache(void);
#endif


// ============================================================================================
// implements basic WATTCP calls using BSD sockets
// (c)2000 Michael Polak, Arachne Labs
// ============================================================================================

#ifdef LINUX
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdarg.h>
#else if defined (CLEMENTINE)
#include <clementine/types.h>
#include <clementine/fdset.h>
#include <clementine/socket.h>
#include <clementine/string.h>
#include <libc/fcntl.h>
#include <libc/arpa/inet.h>
#include <libc/sys/time.h>
#include <libc/netinet/in.h>
#include <libc/netdb.h>
#include <stdarg.h>
#endif

typedef int           tcp_Socket;       /* POSIX file descriptor is int */
typedef unsigned long longword;         /* 32 bits IPv4 address*/

typedef int (*sockfunct_t) ( void );  /* A socket function for delay routines */

longword resolve_fn( char *name, sockfunct_t fn );     // S. Lawson
int sock_puts(int sock, char *str);



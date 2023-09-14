
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
#endif

typedef int           tcp_Socket;       /* POSIX file descriptor is int */
typedef unsigned long longword;         /* 32 bits IPv4 address*/

typedef int (*sockfunct_t) ( void );  /* A socket function for delay routines */

longword resolve_fn( char *name, sockfunct_t fn );     // S. Lawson
int sock_puts(int sock, char *str);



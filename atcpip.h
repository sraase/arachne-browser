// ========================================================================
// Arachne TCP/IP interface
// ========================================================================
#ifndef ATCPIP_H
#define ATCPIP_H

char *atcp_get_ip_str(void);
char *atcp_get_dns_str(void);

int  atcp_open(void *handle, const uint32_t *ip, uint16_t port);
void atcp_close(void *handle);
int  atcp_send(void *handle, const char *buf, size_t len);
int  atcp_recv(void *handle, char *buf, size_t len);
int  atcp_has_data(void *handle);
int  atcp_resolve(const char *hostname, uint32_t *ip);

/* DOS version: WATTCP */
#ifndef POSIX

#define MAX_NAMESERVERS 10
#define TCP_MODE_BINARY 0
#define TCP_MODE_ASCII  1

#define sock_tick( s, statusptr ) \
	if ( !tcp_tick(s)) { *statusptr = 1 ; goto sock_err; }
#define sock_wait_established( s, seconds, fn, statusptr ) \
	if (_ip_delay0( s, seconds, fn, statusptr )) goto sock_err;
#define sock_wait_input( s, seconds, fn , statusptr ) \
	if (_ip_delay1( s, seconds, fn, statusptr )) goto sock_err;
#define sock_wait_closed(s, seconds, fn, statusptr )\
	if (_ip_delay2( s, seconds, fn, statusptr )) goto sock_err;

typedef unsigned long  longword;  /* 32 bit */
typedef unsigned short word;      /* 16 bit */
typedef unsigned char  byte;      /*  8 bit */
typedef void           sock_type; /* opaque */
typedef struct         { byte undoc[2200]; } udp_Socket;
typedef struct         { byte undoc[4300]; } tcp_Socket;

typedef struct {
	longword src;
	longword dst;
	byte     mbz;
	byte     protocol;
	word     length;
	word     checksum;
} tcp_PseudoHeader;

typedef int (*dataHandler_t)(void *s,
	byte *data, int len, tcp_PseudoHeader *pseudohdr, void *protohdr);
typedef int (*sockfunct_t)(void *s);

extern longword def_nameservers[MAX_NAMESERVERS];
extern longword my_ip_addr;
extern longword sin_mask;
extern word sock_delay;
extern int _last_nameserver;

void _add_server(int *counter, int max, longword *array, longword value);
void _arp_add_gateway(char *data, longword ip);
int  _ip_delay0(sock_type *s, int timeoutseconds, sockfunct_t fn, int *statusptr);
int  _ip_delay1(sock_type *s, int timeoutseconds, sockfunct_t fn, int *statusptr);
int  _ip_delay2(sock_type *s, int timeoutseconds, sockfunct_t fn, int *statusptr);

longword resolve(char *name);
longword resolve_fn(char *name, sockfunct_t fn);

char *sockerr(sock_type *s);

void sock_abort(sock_type *s);
void sock_close(sock_type *s);
int  sock_dataready(sock_type *s);
int  sock_fastread(sock_type *s, byte *dp, int len);
int  sock_fastwrite(sock_type *s, byte *dp, int len);
void sock_flush(sock_type *s);
int  sock_gets(sock_type *s, byte *dp, int len);
int  sock_init_noexit(void);
word sock_mode(sock_type *s, word mode);
byte sock_putc(sock_type *s, byte c);
int  sock_setbuf(sock_type *s, byte *dp, int len);
int  sock_tbleft(sock_type *s);
int  sock_write(sock_type *s, byte *dp, int len);

void tcp_config_file(const char *fname);
int  tcp_listen(tcp_Socket *s, word lport, longword ina, word port, dataHandler_t datahandler, word timeout);
int  tcp_open(tcp_Socket *s, word lport, longword ina, word port, dataHandler_t datahandler);
int  tcp_tick(sock_type *s);

void outs(char far *str);

#endif

/* POSIX version */
#ifdef POSIX

#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>

typedef uint32_t longword;   /* 32-bit */
typedef int      tcp_Socket; /* file descriptor */

void outs(char *str);

#endif

#endif

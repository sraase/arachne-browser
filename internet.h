
// ========================================================================
// Internet function prototypes
// (c)1997-1999 Michael Polak, Arachne Labs
// ========================================================================

#include "url.h"

int openhttp(struct Url *url,struct HTTPrecord *cache);
void closehttp(struct HTTPrecord *cache);

#ifndef NOTCPIP
int xfinger(struct Url *url, struct HTTPrecord *cache, char *selector);
int xpopdump(struct Url *url, char dele, char logfile);
int xsendmail(struct Url *url, char helo, char logfile);
int ftpsession(struct Url *url,struct HTTPrecord *cache,char *uploadfile);
void Download(struct HTTPrecord *cache);
void Backgroundhttp(void);
char GoBackground(struct HTTPrecord *cache);
void FinishBackground(char abort);
#ifdef POSIX
int tickhttp(struct HTTPrecord *cache, char *buf, int sockfd);
#else
int tickhttp(struct HTTPrecord *cache, char *buf,tcp_Socket *socket);
#endif
void DNSerr(char *host);

#ifdef WATTCP
//these functions and variables are used only by WATTCP

void free_socket(void);
int locport(void);
void sockmsg(int status,int snum);

dataHandler_t resetport(void);
//dataHandler_t webserver(void);

extern tcp_Socket *socket;
//extern tcp_Socket *websocket;
extern tcp_Socket *sock[2];
extern int socknum;
#endif

// TcpIdleFunc should be called from all waiting loops

extern char GlobalLogoStyle;		//SDL
int TcpIdleFunc(void);		//SDL

#endif

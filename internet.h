
// ========================================================================
// Internet function prototypes
// (c)1997-1999 Michael Polak, Arachne Labs
// ========================================================================

#include "url.h"
#include "atcpip.h"

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
void FinishBackground(char mode);
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

struct Http_parameters
{
 char referer;
 char useproxy;
 char keepalive;
 char acceptcookies;

//!!glennmcc: begin Dec 11,2001---- made it configurable y/n
// added to fix "HTTPS verifying images" loop by trying HTTP instead
 char https2http;
//!!glennmcc: end

//!!glennmcc: begin May 03, 2002
// added to optionally "ignore" <script> tag
// (defaults to No if "IgnoreJS Yes" line is not in Arachne.cfg)
//char ignorejs;
//!!glennmcc: end

//!!glennmcc: begin July 14, 2003
// added to optionally "ignore" <base href=> tag
// (defaults to No if "IgnoreBaseHref Yes" line is not in Arachne.cfg)
char ignorebasehref;
//!!glennmcc: end

//!!glennmcc: July 14, 2005
//char alwaysusecfgcolors;
//!!glennmcc: end

};

extern struct Http_parameters http_parameters;


// TcpIdleFunc should be called from all waiting loops

extern char GlobalLogoStyle;		//SDL
int TcpIdleFunc(void);		//SDL

#endif

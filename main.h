
// ========================================================================
// "constructor" and "destrtuctor" for Arachne's main() function
// (c)1999 Michael Polak, Arachne Labs
// ========================================================================

#include "url.h"

void Initialize_Arachne(int argc,char **argv,struct Url *url);
int Terminate_Arachne(int returnvalue);
int askgraphics(void);
int protocol_arachne(struct HTTPrecord *cacheitem,struct Url *url,int *returnvalue);
int protocol_nohttp(struct HTTPrecord *cacheitem,struct Url *url, unsigned *cacheitem_status, XSWAP*cacheitem_writeadr);

#ifdef POSIX
void WaitForEvent(struct timeval *tv); //waits for user input or whatever... NULL=forever
#endif


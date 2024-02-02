
// ============================================================================================
// implements basic WATTCP calls using BSD sockets
// (c)2000 Michael Polak, Arachne Labs
// ============================================================================================

#include "arachne.h"

#ifdef LINUX
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

// ============================================================================================
// same as sock_puts() in WATTCP 
// ============================================================================================

int sock_puts(int sock, char *str)
{
 char *ptr=str;
 int i,sent=0,l=strlen(str);
 
 do 
 {
  i = send(sock, ptr, l-sent, 0);
  if(i<0)
   return i;
  sent += i;
  ptr += i;
  if(TcpIdleFunc(NULL)!=0)
   return -1; 
 } 
 while(sent < l);
 return sent;
}




// ============================================================================================
// implements basic WATTCP calls using BSD sockets
// (c)2000 Michael Polak, Arachne Labs
// ============================================================================================

#include "arachne.h"

#ifdef LINUX
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#endif

int TcpIdleFunc(void);

// ============================================================================================
//non-blocking version of gethostbyname()
// ============================================================================================

#ifdef NOTHREADS
longword resolve_fn ( char *hostname, sockfunct_t fn )
{
 pid_t child;
 int pipeline[2];
 longword host=0;

 if(pipe(pipeline))
  return 0;

 child=fork();
 if(child<0)
  return 0;
 else
 if(child==0) //child process
 {
  struct hostent *phe;		 /* host information entry */

  close(pipeline[0]);
  phe=gethostbyname(hostname);
  if(phe) //child process sent structure through pipe
   host=*((longword *) phe->h_addr_list[0]);

  write(pipeline[1],&host,sizeof(longword));
  exit(0);  //end child process
 }

 /* parent process continues here */
  
 close(pipeline[1]);

 fcntl (pipeline[0], F_SETFL, O_NONBLOCK);
 while(read(pipeline[0],&host,sizeof(longword))!=sizeof(longword))
 {
  if(fn()) /* call idle function */
  {
   kill(child,9);
   return 0;
  }
 }
 if(host==0)
 {
  long addr=inet_addr(hostname);
  if(addr< 0)
   return 0;
  else
   return (longword)addr;
 }
 else
  return host;
}
#else


volatile longword threaded_DNS_result=0;
volatile int threaded_DNS_flag=0;

void DNSthread(char *hostname)
{
 struct hostent *phe;		 /* host information entry */

 phe=gethostbyname(hostname);
 if(phe) //child process sent structure through pipe
  threaded_DNS_result=*((longword *) phe->h_addr_list[0]);
 threaded_DNS_flag=1;
}

longword resolve_fn ( char *hostname, sockfunct_t fn )
{
#if defined (LINUX)
 pthread_t thread;

 threaded_DNS_flag=0;
 if(pthread_create(&thread,NULL,DNSthread,hostname)!=0)
  return 0;
#elif defined (CLEMENTINE)
 int tid;
 
 threaded_DNS_flag=0;
 tid = createthread ((int (*)(void *)) DNSthread, hostname);
 if (!tid) return 0;
#else
#error Unsupported operating system.
#endif

 while(threaded_DNS_flag==0)
 {
  if(fn()) /* call idle function */
  {
#if defined (LINUX)
// it is not really necessary to kill threads... it is safer...
   pthread_cancel(thread);
#elif defined (CLEMENTINE)
   destroythread (tid);
#endif
   return 0;
  }
 }

 if(threaded_DNS_result==0)
 {
  long addr=inet_addr(hostname);
  if(addr< 0)
   return 0;
  else
   return (longword)addr;
 }
 else
  return threaded_DNS_result;
}

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
  if(TcpIdleFunc()!=0)
   return -1; 
 } 
 while(sent < l);
 return sent;
}



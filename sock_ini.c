/*
 * sock_init - easy way to guarentee:
 *	- card is ready
 *	- shutdown is handled
 *	- cbreaks are handled
 *      - config file is read
 *	- bootp is run
 *
 * 0.1 : May 2, 1991  Erick - reorganized operations
 */
//1996: optimized 4 BorlandC by xCh

//#include "capalloc.h"
//#include "capstdio.h"
#include <copyright.h>
#include <wattcp.h>
#include <stdlib.h>
#include <dos.h>
#include <alloc.h>

word _survivebootp = 0;

void sock_exit()
{
    tcp_shutdown();
}

char tcp_init(void);

char sock_init(void)
{
 if(!tcp_init())
  return 0; /* must precede tcp_config because we need eth addr */
 atexit(sock_exit);	/* must not precede tcp_init() incase no PD */
 tcp_cbrk( 0x10 );	/* allow control breaks, give message */

 if (tcp_config( NULL ))
 {	/* if no config file use BOOTP w/broadcast */
  _bootpon = 1;
  outs("Configuring through BOOTP\n");
 }

 if (_bootpon)	/* non-zero if we use bootp */
 if (_dobootp())
 {
  outs("BOOTP failed\n");
  if ( !_survivebootp )
  sleep(5);
// 	exit( 3 ); zmena pro arachne
  return 0;
 }
 return 1;
}

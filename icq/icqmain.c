/*
 * ICQMAIN.C
 *
 * ICQ core main module
 *
 * David Lindauer, 1999
 */
#include "lsicq.h"

extern ICQ_CALLBACK icq_callback;
extern P_SOK_T icq_sok;
extern char password[9];
extern USER_INFO our_user;
extern DWORD Current_Status;

static time_t resend_time, keepalive_time;

/* at initialization time we just copy a few parameters and init the
 * multitasking timers
 */
void icq_init(P_SOK_T sok, ICQ_CALLBACK callback,DWORD port, DWORD ip)
{
	resend_time = keepalive_time = time(0);
	icq_sok = sok;
	icq_callback = callback;
}
/* at rundown we send the quit packet
 */
void icq_rundown(P_SOK_T sok)
{
	Quit_ICQ();
}
/*
 * Cooperative multitasking handler for the core
 */
void icq_tick(void)
{
	time_t thistime = time(0);

	/* handle incoming packets */
	if (sock_dataready(icq_sok))
		Handle_Server_Response();

	/* resend packets every ten seconds if not acked */
	if (thistime - resend_time >= 10) {
		resend_time = thistime;
		Do_Resend();
	}
	/* Strobe the server once every two minutes */
	if (thistime - keepalive_time >= 120) {
		keepalive_time = thistime;
		Keep_Alive();
	}
}
/*
 * globals.c
 *
 * global definitions
 *
 * david lindauer, 1999
 */
#include "lsicq.h"

P_SOK_T icq_sok;
int seq_num;
int inforeplymaskbyte,inforeplysuccessbyte;
int infosuccessbyte, infomaskbyte;
USER_INFO our_user;
CONTACT_INFO Contacts[100];
int ext_info_seq;
DWORD Current_Status;
int beep;
char password[9];
ICQ_CALLBACK icq_callback;
BOOL serv_mess[1024];
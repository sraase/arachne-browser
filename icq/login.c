/*
 * LOGIN.C
 *
 * Handles login, exit, and various maintenance issues
 * 
 * David Lindauer, 1999
 */
#include "lsicq.h"

extern int seq_num;
extern CONTACT_INFO Contacts[100];
extern USER_INFO our_user;
extern DWORD Current_Status;

/* Log us into the server */
void Login(  DWORD uin, char *pass, DWORD ip, DWORD port, DWORD status)
{
   net_icq_pak pak;
   int size;
   login_1 s1;
   login_2 s2;

	 status = GetStatusFlags(Current_Status = status);
   seq_num = 1;

   DW_2_Chars( s1.port, ntohs( port ) + 0x10000 );
   Word_2_Chars( s1.len, strlen( pass ) + 1 );
   DW_2_Chars( s1.time, time( NULL ) );  
   
   DW_2_Chars( s2.ip, ip);
   DW_2_Chars( s2.status, status );
   DW_2_Chars( s2.X1, LOGIN_X1_DEF );
	 s2.peerflag[0] = 6;
	 DW_2_Chars(s2.TCPver,3);
   DW_2_Chars( s2.x2, LOGIN_X2_DEF );
   DW_2_Chars( s2.X3, LOGIN_X3_DEF );
   
   memcpy( pak.data, &s1, sizeof( s1 ) );
   size = sizeof( s1 );
   memcpy( &pak.data[size], pass, Chars_2_Word( s1.len ) );
   size += Chars_2_Word( s1.len );
	 memcpy( &pak.data[size],&s2,sizeof(s2));
	 size += sizeof(s2);
   SOCKWRITE( CMD_LOGIN, &pak, size + sizeof(pak.head), seq_num++ );
} 
/* tell the server it can remove the messages it has queued for us
 */
void icq_snd_got_messages( void )
{
   net_icq_pak pak;
   SOCKWRITE( CMD_ACK_MESSAGES, &pak, sizeof(pak.head),seq_num++ );
}
/* send over our contact list */
void snd_contact_list( void )
{
   net_icq_pak pak;
   int num_used;
   int i, size;
   char *tmp;
   
   tmp = pak.data;
   tmp++;
   for ( i=0, num_used=0; i < 100 ; i++ )
   {
      if ( Contacts[ i ].flags & FL_VALID )
      {
         DW_2_Chars( tmp, Contacts[i].ci.uin );
         tmp+=4;
         num_used++;
      }
   }
   pak.data[0] = num_used;
   size = sizeof( DWORD ) * num_used + 1;
   size += sizeof(pak.head);
   SOCKWRITE( CMD_CONT_LIST, &pak, size,seq_num++ );
}
/* Send our invisible list */
void snd_invis_list( void )
{
   net_icq_pak pak;
   int num_used;
   int i, size;
   char *tmp;
   
   tmp = pak.data;
   tmp++;
   for ( i=0, num_used=0; i < 100 ; i++ )
   {
      if (Contacts[ i ].flags & FL_VALID)
      {
         if ( Contacts[i].flags & FL_INVISIBLE)
         {
            DW_2_Chars( tmp, Contacts[i].ci.uin );
            tmp+=4;
            num_used++;
         }
      }
   }
   if ( num_used != 0 )
   {
      pak.data[0] = num_used;
      size = ( ( int ) tmp - ( int ) pak.data );
      size += sizeof(pak.head);
      SOCKWRITE( CMD_INVIS_LIST, &pak, size,seq_num++ );
   }
}
/* Send our visible list */
void snd_vis_list( void )
{
   net_icq_pak pak;
   int num_used;
   int i, size;
   char *tmp;
   
   tmp = pak.data;
   tmp++;
   for ( i=0, num_used=0; i < 100; i++ )
   {
      if ( Contacts[ i ].flags & FL_VALID) 
      {
         if ( Contacts[i].flags & FL_VISIBLE )
         {
            DW_2_Chars( tmp, Contacts[i].ci.uin );
            tmp+=4;
            num_used++;
         }
      }
   }
   if ( num_used != 0 )
   {
      pak.data[0] = num_used;
      size = ( ( int ) tmp - ( int ) pak.data );
      size += sizeof(pak.head);
      SOCKWRITE( CMD_VIS_LIST, &pak, size, seq_num++ );
   }
}
/*
 * command sent during login */
void snd_login_1( void )
{
   net_icq_pak pak;
   
   SOCKWRITE( CMD_LOGIN_1, &pak, sizeof(pak.head),seq_num++ );
}
/* register a new user */
void reg_new_user(  char *pass)
{
	net_icq_pak pak;
   char len_buf[2];
	int size, len; 

	len = strlen(pass);
   Word_2_Chars( len_buf, len );
   memcpy(&pak.data[0], len_buf, 2 );
	memcpy(&pak.data[2], pass, len + 1);
   DW_2_Chars( &pak.data[2+len], 0xA0 );
   DW_2_Chars( &pak.data[6+len], 0x2461 );
   DW_2_Chars( &pak.data[10+len], 0xa00000 );
   DW_2_Chars( &pak.data[14+len], 0x00 );
	size = len + 18;
	SOCKWRITE( CMD_REG_NEW_USER, &pak, size + sizeof(pak.head),seq_num ++);
}
/* Quit ICQ */
void Quit_ICQ( void )
{
	net_icq_pak pak;
	int size, len;
   
   len = strlen( "B_USER_DISCONNECTED" ) + 1;
   *(short * ) pak.data = len;
   size = len + 4;
   
   memcpy( &pak.data[2], "B_USER_DISCONNECTED", len );
   pak.data[ 2 + len ] = 05;
   pak.data[ 3 + len ] = 00;

   SOCKWRITE( CMD_SEND_TEXT_CODE, &pak, size + sizeof(pak.head),seq_num++);

}
/* Strobe the server so it knows we are still online
 */
void Keep_Alive( void )
{
   net_icq_pak pak;
   
   SOCKWRITE( CMD_KEEP_ALIVE, &pak, sizeof(pak.head), seq_num++ );

   SOCKWRITE( CMD_KEEP_ALIVE2, &pak, sizeof(pak.head), seq_num++ );
}
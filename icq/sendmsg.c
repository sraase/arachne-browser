/*
 * SENDMSG.C
 *
 * messaging basic core, packet encryption for V4
 */
#include "lsicq.h"

extern USER_INFO our_user;
extern P_SOK_T icq_sok;
extern BOOL serv_mess[ 1024 ]; /* used so that we don't get duplicate messages with the same SEQ */
extern int verbose;

/* Historical */
void Initialize_Msg_Queue()
{
    msg_queue_init();
}

/* This routine sends the aknowlegement cmd to the
   server it appears that this must be done after
   everything the server sends us                 */
void ack_srv( int seq )
{
   net_icq_pak pak;
   
   
   SOCKWRITE( CMD_ACK, &pak, sizeof( pak.head ),seq);

}
/*
 * if a packet hasn't received an ack, resend it after a while
 * (cooperative multitasking)
 */
void Do_Resend( void )
{
    struct msg *queued_msg;
    char temp[1024];

    if ((queued_msg = msg_queue_pop()) != NULL)
    {
        queued_msg->attempts++;
        if (queued_msg->attempts <= 6)
        {
            if ( 0x1000 < Chars_2_Word( &queued_msg->body[6] ) ) {
		Dump_Queue();
            }
	    			memcpy( temp, queued_msg->body, queued_msg->len);
						if (verbose) {
							M_print("resent cmd %d\n",Chars_2_Word(((ICQ_PAK_PTR)temp)->cmd));
						}
            SOCKWRITE_LOW(temp, queued_msg->len);
            msg_queue_push( queued_msg );
        }
        else {
            free_msg(queued_msg);
        }

    }
}


/********************************************************
The following data constitutes fair use for compatibility.
*********************************************************/
unsigned char icq_check_data[257] = {
	0x0a, 0x5b, 0x31, 0x5d, 0x20, 0x59, 0x6f, 0x75, 
	0x20, 0x63, 0x61, 0x6e, 0x20, 0x6d, 0x6f, 0x64, 
	0x69, 0x66, 0x79, 0x20, 0x74, 0x68, 0x65, 0x20, 
	0x73, 0x6f, 0x75, 0x6e, 0x64, 0x73, 0x20, 0x49,
	0x43, 0x51, 0x20, 0x6d, 0x61, 0x6b, 0x65, 0x73, 
	0x2e, 0x20, 0x4a, 0x75, 0x73, 0x74, 0x20, 0x73, 
	0x65, 0x6c, 0x65, 0x63, 0x74, 0x20, 0x22, 0x53, 
	0x6f, 0x75, 0x6e, 0x64, 0x73, 0x22, 0x20, 0x66, 
	0x72, 0x6f, 0x6d, 0x20, 0x74, 0x68, 0x65, 0x20,
	0x22, 0x70, 0x72, 0x65, 0x66, 0x65, 0x72, 0x65,
	0x6e, 0x63, 0x65, 0x73, 0x2f, 0x6d, 0x69, 0x73,
	0x63, 0x22, 0x20, 0x69, 0x6e, 0x20, 0x49, 0x43,
	0x51, 0x20, 0x6f, 0x72, 0x20, 0x66, 0x72, 0x6f,
	0x6d, 0x20, 0x74, 0x68, 0x65, 0x20, 0x22, 0x53,
	0x6f, 0x75, 0x6e, 0x64, 0x73, 0x22, 0x20, 0x69,
	0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6f,
	0x6e, 0x74, 0x72, 0x6f, 0x6c, 0x20, 0x70, 0x61,
	0x6e, 0x65, 0x6c, 0x2e, 0x20, 0x43, 0x72, 0x65,
	0x64, 0x69, 0x74, 0x3a, 0x20, 0x45, 0x72, 0x61,
	0x6e, 0x0a, 0x5b, 0x32, 0x5d, 0x20, 0x43, 0x61,
	0x6e, 0x27, 0x74, 0x20, 0x72, 0x65, 0x6d, 0x65,
	0x6d, 0x62, 0x65, 0x72, 0x20, 0x77, 0x68, 0x61,
	0x74, 0x20, 0x77, 0x61, 0x73, 0x20, 0x73, 0x61,
	0x69, 0x64, 0x3f, 0x20, 0x20, 0x44, 0x6f, 0x75,
	0x62, 0x6c, 0x65, 0x2d, 0x63, 0x6c, 0x69, 0x63,
	0x6b, 0x20, 0x6f, 0x6e, 0x20, 0x61, 0x20, 0x75,
	0x73, 0x65, 0x72, 0x20, 0x74, 0x6f, 0x20, 0x67,
	0x65, 0x74, 0x20, 0x61, 0x20, 0x64, 0x69, 0x61,
	0x6c, 0x6f, 0x67, 0x20, 0x6f, 0x66, 0x20, 0x61,
	0x6c, 0x6c, 0x20, 0x6d, 0x65, 0x73, 0x73, 0x61,
	0x67, 0x65, 0x73, 0x20, 0x73, 0x65, 0x6e, 0x74,
	0x20, 0x69, 0x6e, 0x63, 0x6f, 0x6d, 0x69, 0x6e, 0
};
/* V4 encryption */
void Wrinkle( void * ptr, size_t len )
{
   static BOOL before = FALSE;
   BYTE *buf;
   DWORD chksum;
   DWORD key;
   DWORD tkey, temp;
   BYTE r1,r2;
   int n, pos;
   DWORD chk1,chk2;

   buf = ptr;

   if ( ! before ) {
      srand( time( NULL ) );
			before = TRUE;
   }
	 srand(5);

   buf[2] = rand() & 0xff ;
   buf[3] = rand() & 0xff ;
   buf[4] = 0;
   buf[5] = 0;

   r1 = rand() % ( len - 4 );
   r2 = rand() & 0xff;

   chk1 = (BYTE) buf[8];
   chk1 <<= 8;
   chk1 += (BYTE) buf[4];
   chk1 <<= 8;
   chk1 += (BYTE) buf[2];
   chk1 <<= 8;
   chk1 += (BYTE) buf[6];
   chk2 = r1;
   chk2 <<= 8;
   chk2 +=(BYTE) ( buf[ r1 ] );
   chk2 <<= 8;
   chk2 += r2;
   chk2 <<= 8;
   chk2 +=(BYTE) ( icq_check_data[ r2 ] );
   chk2 ^= 0x00ff00ff;
   
   chksum = chk2 ^ chk1;
   
   DW_2_Chars( &buf[ 0x10 ], chksum );
   key = len;
   key *= 0x66756B65;
   key += chksum;
   n = ( len + 3 ) / 4;   
   pos = 0;
   while ( pos < n )
   {
      if ( pos != 0x10 )
      {
         tkey = key + icq_check_data[ pos & 0xff ];
         temp = Chars_2_DW( &buf[ pos ] );
         temp ^= tkey;
         DW_2_Chars( &buf[ pos ], temp );
      }
      pos += 4;
   }
   Word_2_Chars( &buf[0], ICQ_VER );
}
/* fill in the packet header, register the packet with the resend
 * mechanism, then send it out
 */
size_t SOCKWRITE( WORD cmd, net_icq_pak * ptr, size_t len, WORD seq )
{
   struct msg *msg_to_queue;

	 seq = seq % 1024;
   Word_2_Chars( ptr->head.ver, ICQ_VER );
   Word_2_Chars( ptr->head.seq, seq );
   Word_2_Chars( ptr->head.seq2, seq);
   Word_2_Chars( ptr->head.cmd, cmd );
	 Word_2_Chars( ptr->head.zero, 0);
   DW_2_Chars( ptr->head.UIN, our_user.uin);
	 if (verbose) {
	   M_print("Sent cmd %d\n",cmd);
	 }
   if ( cmd != CMD_ACK ) {
      serv_mess[ seq  ] = FALSE;
      msg_to_queue = alloc_msg();
      msg_to_queue->seq = seq ;
      msg_to_queue->attempts = 1;
      msg_to_queue->len = len;
      memcpy(msg_to_queue->body, ptr, msg_to_queue->len);
      msg_queue_push( msg_to_queue );

   }

   return SOCKWRITE_LOW( ptr, len );
}
/* low level packet handlers */
static size_t SOCKWRITE_LOW( void * ptr, size_t len )
{

   Wrinkle( ptr, len );
   return sock_write( icq_sok, ptr, len );
}

size_t SOCKREAD( void * ptr, size_t len )
{
   size_t sz;
   
   sz = sock_fastread( icq_sok, ptr, len );
   return sz;
}
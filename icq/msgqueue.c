/*
 * MSGQUEUE.C
 *
 * handle resending missed packets
 *
 * David Lindauer, 1999
 */
/************************************************************
Adapted from the work of:

Author Lawrence Gold
Handles resending missed packets.
*************************************************************/
#include "lsicq.h"

static struct msg_queue queue;
static struct msg msgs[MAX_MSG_QUEUE];
unsigned short timestamp;

void msg_queue_init( void )
{
    queue.entries = 0;
    queue.head = queue.tail = NULL;
}

void free_msg(struct msg *val)
{
	val->inuse = FALSE;
}
struct msg *alloc_msg(void)
{
	int i;
	unsigned short time = 65535;
	int val = 0;
	for (i=0; i < MAX_MSG_QUEUE; i++) {
		if (!msgs[i].inuse) {
			msgs[i].inuse = TRUE;
			msgs[i].time = timestamp++;
			return &msgs[i];
		}
		if (msgs[i].time <= time-MAX_MSG_QUEUE || msgs[i].time == 65535) {
			time = msgs[i].time;
			val = i;
		}
	}
	return &msgs[val];
}

struct msg *msg_queue_peek( void )
{
    if ( NULL != queue.head )
    {
        return queue.head;
    }
    else
    {
        return NULL;
    }
}


struct msg *msg_queue_pop( void )
{
    struct msg *popped_msg;
    struct msg *temp;

    if ( ( NULL != queue.head ) && ( queue.entries > 0 ) )
    {
        popped_msg = queue.head;    
        temp = queue.head->next;
        queue.head = temp;
        queue.entries--;
        if ( NULL == queue.head )
        {
            queue.tail = NULL;
        }
        return popped_msg;
    }
    else
    {
        return NULL;
    }
}


void msg_queue_push( struct msg *new_msg)
{
    assert( NULL != new_msg );

    new_msg->next = NULL;

    if (queue.tail)
    {
        queue.tail->next = new_msg;
        queue.tail = new_msg;
    }
    else
    {
        queue.head = queue.tail = new_msg; 
    }

    queue.entries++;
}

void Dump_Queue( void )
{
   int i;
   struct msg *queued_msg;
	
   assert( 0 <= queue.entries );
   
   M_print( "\nTotal entries %d\n", queue.entries );
   for (i = 0; i < queue.entries; i++)
   {
       queued_msg = msg_queue_pop();
       M_print( "SEQ = %04x\tCMD = %04x\tattempts = %d\tlen = %d",
          queued_msg->seq, Chars_2_Word( &queued_msg->body[6] ),
	  queued_msg->attempts, queued_msg->len );
       msg_queue_push( queued_msg );
   }
}

void Check_Queue( WORD seq )
{
   int i;
   struct msg *queued_msg;
	
   assert( 0 <= queue.entries );
   
   for (i = 0; i < queue.entries; i++)
   {
       queued_msg = msg_queue_pop();
       if (queued_msg->seq != seq)
       {
           msg_queue_push( queued_msg );
       }
       else
       {
					 free_msg(queued_msg);

           break;
       }
   }
}
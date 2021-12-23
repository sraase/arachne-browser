#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include "datatype.h"

#define MAX_MSG_QUEUE 10
struct msg
{
    struct msg * next;
    int seq;
    int attempts;
    int exp_time;
    BYTE body[1024];
    int len;
    int inuse;
    unsigned time;
};

struct msg_queue
{
    int entries;
    struct msg *head;
    struct msg *tail;
};

void msg_queue_init( void );
void free_msg(struct msg *val);
struct msg *alloc_msg(void);
struct msg *msg_queue_peek( void );
struct msg *msg_queue_pop( void );
void msg_queue_push( struct msg *new_msg );
void Check_Queue( WORD seq );
void Dump_Queue( void );

#endif
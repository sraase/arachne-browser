
/*
 * Packet De-Fragmentaion code for WATTCP
 * Written by and COPYRIGHT (c)1993 to Quentin Smart
 *                               smart@actrix.gen.nz
 * all rights reserved.
 *
 *   This software is distributed in the hope that it will be useful,
 *   but without any warranty; without even the implied warranty of
 *   merchantability or fitness for a particular purpose.
 *
 *   You may freely distribute this source code, but if distributed for
 *   financial gain then only executables derived from the source may be
 *   sold.
 *
 *
 * Based on RFC815
 */
//#include"capalloc.h"
//#include"capstdio.h"
#include <mem.h>
#include <stdlib.h>
#include "wattcp.h"


#define MAXBUFS         5       /* maximum number of Ethernet buffers */
#define MAXFRAGS        MAXBUFS-1
#define FRAGHOLDTIME    15       /* 15 secs to hold before discarding */
#define INFINITY        30000
#define IP_HEADER_SIZE  20
#define IP_MF           0x0020  // More fragment in INTEL form
//#define IP_DF           0x2000  // Don't fragments



typedef struct {
	longword    source;
//      longword destination;
//    byte        proto;           My app only does TCP so don't care
	word        identification;
	} fragkey;

typedef struct hd {
	struct hd   * next;
	int           start;
	int           end;
	} hole_descr;

typedef struct {
	byte         used;           // this position in table in use
	fragkey      key;
	hole_descr  *hole_first;
	longword     timer;
	in_Header   *ip;
	byte        *data_offset;
	} fraghdr;

static fraghdr fraglist[MAXFRAGS] = {{0,{0,0},NULL,0,NULL},
			      {0,{0,0},NULL,0,NULL},
			      {0,{0,0},NULL,0,NULL},
			      {0,{0,0},NULL,0,NULL}};

int active_frags = 0;
extern word _pktipofs;  /* offset from header to start of pkt */

/* Fragment is called if the frag section of the IP header is not zero and DF bit not set */
byte * fragment(in_Header * ip)
{
//int        fc;
fraghdr    *my_frag;
hole_descr *hole = NULL;
hole_descr *prev_hole = NULL;

fragkey     key;
int         found = 0;
int         got_hole = 0;
int         data_start;
int         data_end;
int         data_length;
int         temp,i;
int         more_frags;   // Set to true if this is the last frag
byte     * buffer;

// Should check that the packet is actually valid and do a checksum on it.
// Assemble key
//key.proto = ip->proto;
key.source=ip->source;
key.identification = ip->identification;

// Check if we have a match

for (i=0;i<MAXFRAGS && !found;i++)
    if (fraglist[i].used && !memcmp(&key,&fraglist[i].key,sizeof(fragkey)))
       {
	  found = TRUE;
	  my_frag = &fraglist[i];
       }

if (!found && active_frags == MAXFRAGS) {
   // Can't handle any new frags, biff packet so that we can continue
   // Could do direct!
   pkt_buf_release((char*)ip);
   return(NULL);  // We can't handle any new frags!
   }
						 // Should biff packet?

// Calc where data should go

   data_start  = intel16(ip->fo ) << 3; //* 8
   data_length = intel16(ip->length)-in_GetHdrlenBytes(ip);
   data_end    = data_start + data_length;
   more_frags = ip->frags & IP_MF;



if (!found)
 {
   // Mark as used
   *((byte *)ip - (2 + _pktipofs)) = 2;
   // Find first empty slot
   for (i=0;i < MAXFRAGS && fraglist[i].used;i++);
   my_frag = &fraglist[i];
   // mark as used
   my_frag->used = 1;
   // inc active frags counter
   active_frags++;
   // Setup frag header data, first packet
   memcpy(&my_frag->key,&key,sizeof(key));
   my_frag->timer  = set_timeout(max(FRAGHOLDTIME,ip->ttl));
   my_frag->ip = ip;
   // Set pointers to beinging of IP packet data
   my_frag->data_offset = (byte *)my_frag->ip + in_GetHdrlenBytes(ip);
   // Setup initial hole table
   if (data_start) // i.e. not Zero
    {
      memcpy(my_frag->data_offset + data_start,(byte *)ip+in_GetHdrlenBytes(ip),data_length);
      // Bracket beginning of data
      hole = my_frag->hole_first = my_frag->data_offset;
      hole->start = 0;
      hole->end = data_start-1;
      if (more_frags) {
	 hole->next = my_frag->data_offset + data_length + 1;
	 hole = hole->next;
	 }
      else
	{
	 hole = my_frag->hole_first->next = NULL;
	 // Adjust length
	 ip->length = intel16(data_end + in_GetHdrlenBytes(ip));
	}
    }
   else
    {
     // Setup
     hole = my_frag->hole_first = my_frag->data_offset + data_length + 1;
    }
   // Bracket end
   if (hole) {
     hole->start = data_length;// + 1;
     hole->end = INFINITY;
     hole->next = NULL;
    }
   return NULL; // Go back for more!
 } // End !found
// Adjust length
   if (!more_frags)
      my_frag->ip->length = intel16(data_end + in_GetHdrlenBytes(ip));

// Hole handling
   hole = my_frag->hole_first;

   do {
      if (!(data_start > hole->end))
	 if (!(data_end < hole->start))  // We've found the spot
	    {
	     // Mark as got.
	     got_hole =1;
	     // Find where to insert
	     // Check is there a hole before the new frag
	     temp = hole->end;   // Pick up old hole end for later;

	     if (data_start > hole->start)
	       {
		 hole->end = data_start-1;
		 prev_hole = hole;  // We have a new prev
	       }
	      else
	       {
		// No, delete current hole
		if (!prev_hole)
		   my_frag->hole_first=hole->next;
		else
		   prev_hole->next = hole->next;
	       }

	     // Is there a hole after the current fragment
	     // Only if we're not last and more to come
	     if (data_end < hole->end && more_frags)
	       {
		 hole = data_end + 1 + my_frag->data_offset;
		 hole->start = data_end+1;
		 hole->end = temp;
		 // prev_hole = NULL if first
		 if (!prev_hole)
		  {
		   hole->next = my_frag->hole_first;
		   my_frag->hole_first = hole;
		  }
		 else
		  {
		   hole->next = prev_hole->next;
		   prev_hole->next = hole;
		  }
		}
	    }
	    prev_hole = hole;
	    hole = hole->next;
	}
	while (!hole && !got_hole); // Until we got to the end or found
       // Thats all setup so copy in the data
       if (got_hole)
	   memcpy(my_frag->data_offset + data_start,(byte *)ip+in_GetHdrlenBytes(ip),data_length);
       // And release the buffer;
       pkt_buf_release((byte *)ip);
       // Now do we have all the parts?
       if (!my_frag->hole_first)
	{
	  my_frag->used = 0;
	  active_frags--;
	  // Redo checksum as we've changed the length in the header
	  my_frag->ip->checksum = 0; // Zero
	  my_frag->ip->checksum = ~ checksum( my_frag->ip, sizeof( in_Header ));
	  return((byte *)my_frag->ip - _pktipofs);
	}
return NULL;
}

void timeout_frags(void)
{
int i;

for (i=0;i<MAXFRAGS;i++)
    if (fraglist[i].used)
       if (chk_timeout(fraglist[i].timer))
	 {
	  fraglist[i].used = 0;
	  active_frags--;
	  pkt_buf_release((byte *)fraglist[i].ip);
	 }
}


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
  #include "wattcp.h"
  #include <conio.h>
  #include <dos.h>
#ifdef _TSRMSDOS
  #include "proto.h"
#endif

#include "datatype.h"
#include "msgqueue.h"

/* packet headers in use */
#define ICQ_VER 4

   typedef struct
   {
      BYTE ver[2];
      BYTE rand[2];
      BYTE zero[2];
      BYTE cmd[2];
      BYTE seq[2];
      BYTE seq2[2];
      BYTE  UIN[4];
      BYTE checkcode[4];
   } ICQ_pak, *ICQ_PAK_PTR;

   typedef struct
   {
      BYTE ver[2];
      BYTE cmd[2];
      BYTE seq[2];
      BYTE seq2[2];
      BYTE UIN[4];
      BYTE check[4];
   } SRV_ICQ_pak, *SRV_ICQ_PAK_PTR;

typedef struct
{
   ICQ_pak  head;
   unsigned char  data[1024];
} net_icq_pak, *NET_ICQ_PTR;

typedef struct
{
   SRV_ICQ_pak  head;
   unsigned char  data[1024];
} srv_net_icq_pak, *SRV_NET_ICQ_PTR;

/* status flags for info and update processes */
#define SUCCESS 1
#define FAIL 0

/* commands to the server */
#define CMD_ACK               0x000A 
#define CMD_SENDM             0x010E
#define CMD_LOGIN             0x03E8
#define CMD_CONT_LIST         0x0406
#define CMD_SEARCH_UIN        0x041a
#define CMD_SEARCH_USER       0x0424
#define CMD_KEEP_ALIVE        0x042e
#define CMD_KEEP_ALIVE2       0x051e
#define CMD_SEND_TEXT_CODE    0x0438
#define CMD_LOGIN_1           0x044c
#define CMD_INFO_REQ          0x0460
#define CMD_EXT_INFO_REQ      0x046a
#define CMD_CHANGE_PW         0x049c
#define CMD_EXT_INFO_UPDATE   0x04b0
#define CMD_STATUS_CHANGE     0x04d8
#define CMD_AUTHORIZE_CHANGE  0x0514
#define CMD_LOGIN_2           0x0528
#define CMD_UPDATE_INFO       0x050A
#define CMD_UPDATE_EXT_INFO   0X04B0
#define CMD_ADD_TO_LIST       0X053C
#define CMD_REQ_ADD_LIST      0X0456
#define CMD_QUERY_SERVERS     0X04BA
#define CMD_QUERY_ADDONS      0X04C4
#define CMD_NEW_USER_1        0X04EC
#define CMD_NEW_USER_INFO     0x04A6
#define CMD_ACK_MESSAGES      0x0442
#define CMD_MSG_TO_NEW_USER   0x0456
#define CMD_REG_NEW_USER      0x03FC
#define CMD_VIS_LIST          0x06AE
#define CMD_INVIS_LIST        0x06A4
#define CMD_META_USER         0x064A
#define CMD_RAND_SEARCH       0x056E
#define CMD_RAND_SET          0x0564
#define CMD_ADD_VISI_LIST     0x06b8

/* Things the server can send us */
#define SRV_ACK            0x000A
#define SRV_LOGIN_REPLY    0x005A
#define SRV_USER_ONLINE    0x006E
#define SRV_USER_OFFLINE   0x0078
#define SRV_USER_FOUND     0x008C
#define SRV_RECV_MESSAGE   0x00DC
#define SRV_END_OF_SEARCH  0x00A0
#define SRV_INFO_REPLY     0x0118
#define SRV_EXT_INFO_REPLY 0x0122
#define SRV_INFO_NONE	   0x012c
#define SRV_EXT_INFO_NONE  0x0136
#define SRV_STATUS_UPDATE  0x01A4
#define SRV_X1             0x021C
#define SRV_X2             0x00E6
#define SRV_UPDATE_EXT_SUCCESS     0x00C8
#define SRV_UPDATE_EXT_FAIL 0x00d2
#define SRV_NEW_UIN        0x0046
#define SRV_NEW_USER       0x00B4
#define SRV_QUERY          0x0082
#define SRV_SYSTEM_MESSAGE 0x01C2
#define SRV_SYS_DELIVERED_MESS 0x0104
#define SRV_GO_AWAY        0x00F0
#define SRV_BAD_PASS       0x0064
#define SRV_TRY_AGAIN      0x00FA
#define SRV_UPDATE_FAIL    0x01EA
#define SRV_UPDATE_SUCCESS 0x01E0
#define SRV_AUTHORIZE_SUCCESS 0x1f4
#define SRV_AUTHORIZE_FAIL 0x1fe
#define SRV_MULTI_PACKET   0x0212
#define SRV_META_USER      0x03DE
#define SRV_RAND_USER      0x024E

/* Meta info (not supported) */
#define META_INFO_SET 0x3E8
#define META_INFO_REQ 0x04B0
#define META_INFO_GEN 0xC8
#define META_INFO_MORE 0xDC
#define META_INFO_WORK 0xD2

/* Status flags for user status (Network version) */	   
#define STF_OFFLINE  ((unsigned long)(-1L))
#define STF_ONLINE  0x00
#define STF_INVISIBLE 0x100
#define STF_OCCUPIED_MAC 0x10
#define STF_NA_99        0x04
#define STF_NA      0x05
#define STF_FREE_CHAT 0x20
#define STF_OCCUPIED 0x11
#define STF_AWAY    0x01
#define STF_DND    0x13

/* Status flags for user status (internal version ) */
#define STATUS_OFFLINE 0
#define STATUS_ONLINE 1
#define STATUS_INVISIBLE 2
#define STATUS_OCCUPIED 3
#define STATUS_NA 4
#define STATUS_FREE_CHAT 5
#define STATUS_AWAY 6
#define STATUS_DND 7

/* Message types for text messages */
#define USER_ADDED_MESS 0x000C
#define AUTH_DENIED_MESS 0x0007
#define AUTH_ACCEPTED_MESS 0x0008
#define AUTH_REQ_MESS 0x0006
#define URL_MESS	0x0004
#define MASS_MESS_MASK  0x8000
#define MRURL_MESS	0x8004
#define NORM_MESS		0x0001
#define MRNORM_MESS	0x8001
#define CONTACT_MESS	0x0013
#define MRCONTACT_MESS	0x8013

/* response flags for update requests */
#define IR_INFO 1
#define IR_AUTH 2
#define IR_EXT 4

/* response flags for info requests */
#define UI_BASIC 1
#define UI_EXTENDED 2

/* random group names */
#define RG_NONE 0
#define RG_GENERAL 1
#define RG_ROMANCE 2
#define RG_GAMES 3
#define RG_STUDENTS 4
#define RG_20s 6
#define RG_30s 7
#define RG_40s 8
#define RG_50_PLUS 9
#define RG_MANNEEDSWOMAN 10
#define RG_WOMANNEEDSMAN 11




/* login definitions */
   typedef struct
   {
      BYTE time[4];
      BYTE port[4];
      BYTE len[2];
   } login_1, *LOGIN_1_PTR;

   typedef struct
   {
      BYTE X1[4];
      BYTE ip[4];
      BYTE  peerflag[1];
      BYTE  status[4];
      BYTE TCPver[4];
      BYTE  x2[4];
      BYTE X3[4];
   } login_2, *LOGIN_2_PTR;

   #define LOGIN_X1_DEF 0x00000098
   #define LOGIN_X2_DEF 0x00000000
   #define LOGIN_X3_DEF 0x00980000

/* header for timestamped text messages */
typedef struct
{
   BYTE   uin[4];
   BYTE year[2];
   BYTE  month;
   BYTE  day;
   BYTE  hour;
   BYTE  minute;
   BYTE type[2];
   BYTE len[2];
} RECV_MESSAGE, *RECV_MESSAGE_PTR;

/* header for non-timestamped text messages */
typedef struct
{
   BYTE uin[4];
   BYTE type[2]; 
   BYTE len[2];
} SIMPLE_MESSAGE, *SIMPLE_MESSAGE_PTR;

/* TCP/IP connection information */
typedef struct {
   DWORD uin;
   DWORD status;
   DWORD TCPver;
   DWORD port;
   BYTE ip[4];
   BYTE connection;
} CONNECT_INFO;

/* Contact info structure */
typedef struct
{
   CONNECT_INFO ci;
   DWORD last_time; /* last time online or when came online */
   WORD flags;
#define FL_INVISIBLE 1
#define FL_VISIBLE 2
#define FL_NOT_IN_LIST 4
#define FL_RECEIVED_FROM 8
#define FL_WAITING_FOR_ACK 16
#define FL_WAITING_FOR_AUTH 32
#define FL_AUTH_IMPERM 64
#define FL_VALID 128
   char nick[20];
} CONTACT_INFO;

/*
 * user info structure */
typedef struct
{
   DWORD uin;
   char nick[20];
   char first[20];
   char last[40];
   char email[80];
   BOOL auth;      
   char city[20];
   WORD country;
   char state[6];
   WORD age;
   char sex;
   char phone[20];
   char homepage[80];
   char about[400];
   WORD timezone;
} USER_INFO;

/* Equates for the ICQ core callbacks */
#define IM_UNHANDLED	10
#define IM_LOGIN	20
#define IM_LOGIN2	30
#define IM_BADPASS	40
#define IM_NEWUIN	50
#define IM_ACCOUNTBUSY	60
#define IM_FORCEDDISCONNECT	70
#define IM_USEROFFLINE	80
#define IM_USERONLINE	90
#define IM_STATUSUPDATE	100
#define IM_NOTIFYADDED	110
#define IM_ASKFORAUTH	120
#define IM_AUTHREQ	130
#define IM_AUTHDENIED	140
#define IM_AUTHACCEPTED	150
#define IM_USERADDED	160
#define IM_URLRECEIVED	170
#define IM_CONTACTRECEIVED 	180
#define IM_INSTANTMESS	190
#define IM_INFORESPONSE	200
#define IM_UPDATEDONE	210
#define IM_RANDRESPONSE	220
#define IM_USERFOUND	230
#define IM_SEARCHDONE	240

/* callback type */
typedef void (*ICQ_CALLBACK)(int msg, DWORD intval, void *parm1, void *parm2, time_t tim);

#include "icqproto.h"
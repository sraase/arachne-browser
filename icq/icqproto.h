
/* Protogen Version 1.00 Wednesday May 05, 1999  18:37:45 */

                               /* File.c */

void WriteProfileString(FILE *fil, char *key, char *val);
void WriteProfileDW(FILE *fil, char *key, DWORD val);
int ReadProfileString(FILE *fil, char *key, char *val, int len);
DWORD ReadProfileDW(FILE *fil, char *key, DWORD def);
void WriteProfile(void);
int ReadProfile(void);

                              /* Icqmain.c */

void icq_init(P_SOK_T sok, ICQ_CALLBACK callback,DWORD port, DWORD ip);
void icq_rundown(P_SOK_T sok);
void icq_tick(void);

                               /* Login.c */

void Login(  DWORD uin, char *pass, DWORD ip, DWORD port, DWORD status);
void snd_got_messages( void );
void snd_contact_list( void );
void snd_invis_list( void );
void snd_vis_list( void );
void snd_login_1( void );
void icq_snd_got_messages( void );
void reg_new_user(  char *pass);
void Quit_ICQ( void );
void Keep_Alive( void );

                                /* Msg.c */

int Putstring(char *dest, char *string);                          /* STATIC */
int Putword(char *dest, int word);                                /* STATIC */
int Putbyte(char *dest, int byte);                                /* STATIC */
void icq_sendmsg(  DWORD uin, char *text, int len, DWORD msg_type);
void icq_status_change(DWORD status);
void icq_send_info_req(  DWORD uin );
void icq_start_search(  USER_INFO * user);
int icq_Add_User( DWORD uin);
void icq_Update_User_Info( USER_INFO * user);
void icq_sendurl( DWORD uin, char *description, char *url );
void icq_send_auth_msg( DWORD uin);
void icq_send_reject_auth_msg( DWORD uin);
void icq_Notify_Added( DWORD uin);
void icq_Ask_For_Authorize( DWORD uin, char *reason);
void icq_rand_user_req( DWORD group );
void icq_rand_set( DWORD group );
void icq_remove_invisible ( DWORD uin);
void icq_remove_visible ( DWORD uin);
void icq_add_visible( DWORD uin);
void icq_add_invisible( DWORD uin);

                             /* Msgqueue.c */

void msg_queue_init( void );
void free_msg(struct msg *val);
struct msg *alloc_msg(void);
struct msg *msg_queue_peek( void );
struct msg *msg_queue_pop( void );
void msg_queue_push( struct msg *new_msg);
void Dump_Queue( void );
void Check_Queue( WORD seq );

                              /* Sendmsg.c */

void Initialize_Msg_Queue(void);
void ack_srv( int seq );
void Do_Resend( void );
void Wrinkle( void * ptr, size_t len );
size_t SOCKWRITE( WORD cmd, net_icq_pak * ptr, size_t len, WORD seq );
size_t SOCKWRITE_LOW( void * ptr, size_t len );                   /* STATIC */
size_t SOCKREAD( void * ptr, size_t len );

                              /* Server.c */

int GetString(char *string, char *pos, int fieldlen);             /* STATIC */
int GetFEString(char *string, char *pos, int fieldlen);           /* STATIC */
void FillUserInfo(USER_INFO *user,char *data);                    /* STATIC */
void FillConnectInfo(CONNECT_INFO *connect, char *data);          /* STATIC */
void HandleInfoReply( int flag, int type, char *data);            /* STATIC */
void Recv_Message( BYTE * pak );                                  /* STATIC */
void Do_Msg( DWORD type, WORD len, char * data, DWORD uin, int dated, time_t thedate ) ;
void infoupdate(int flag,int type);                               /* STATIC */
void Multi_Packet( BYTE *data );                                  /* STATIC */
void Server_Response( BYTE *data, DWORD len, WORD cmd, WORD ver, WORD seq, DWORD uin );
void Handle_Server_Response( void );

                                /* Ui.c */

int Online( CONTACT_INFO *contact);                               /* STATIC */
void ShowStatus( CONTACT_INFO *contact);                          /* STATIC */
void ShowOffline(void);                                           /* STATIC */

                               /* Util.c */

void DW_2_Chars(unsigned char *buf,DWORD val);
void Word_2_Chars(unsigned char *buf, DWORD val);
DWORD Chars_2_DW(unsigned char *buf);
unsigned Chars_2_Word(unsigned char *buf);
CONTACT_INFO *FindContact(DWORD uin);
CONTACT_INFO *FindEmptyContact(void);
DWORD GetStatusFlags(DWORD status);
DWORD GetStatusVal(DWORD status);
const char *Get_Country_Name( int code );
const int Get_Country_Code( char *buf );
char *UIN2Nick(DWORD uin);
DWORD Nick2UIN(char *nick);
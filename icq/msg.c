/*
 * MSG.C
 *
 * Messages we send to the server 
 *
 * David Lindauer, 1999
 */
#include "lsicq.h"

extern int seq_num;
extern int inforeplymaskbyte,inforeplysuccessbyte;
extern int infosuccessbyte, infomaskbyte;
extern USER_INFO our_user;
extern CONTACT_INFO Contacts[100];
extern int ext_info_seq;
extern DWORD Current_Status;

/* Stick a counted string in a packet */
static int Putstring(char *dest, char *string)
{
	int l = strlen(string)+1;
  Word_2_Chars(dest,l);
	strcpy(dest+2,string);
	return l+2;
}
/* stick a word in a packet */
static int Putword(char *dest, int word)
{
	Word_2_Chars(dest,word);
	return 2;
}
/* stick a byte in a packet (DUH) */
static int Putbyte(char *dest, int byte)
{
	*dest = byte;
	return 1;
}
/* Send an instant message to user.  The is used to send both text
 * and authorization messages, as well as URL and Contact lists
 */
void icq_sendmsg(  DWORD uin, char *text, int len, DWORD msg_type)
{
	SIMPLE_MESSAGE msg;
	net_icq_pak pak;
	int size; 

	DW_2_Chars( msg.uin, uin );
	Word_2_Chars(msg.type, msg_type);
	Word_2_Chars( msg.len, len + 1 );		/* length + the NULL */

	memcpy(&pak.data, &msg, sizeof( msg ) );
	memcpy(&pak.data[8], text, len + 1);

	size = sizeof( msg ) + len + 1;

	SOCKWRITE( CMD_SENDM, &pak, size + sizeof(pak.head),seq_num++);
}
/*
 * we have changed status
 */
void icq_status_change(DWORD status)
{
	net_icq_pak pak;
	Current_Status = status;
	status = GetStatusFlags(status);
	DW_2_Chars(pak.data, status);
	SOCKWRITE( CMD_STATUS_CHANGE, &pak, 4 + sizeof(pak.head), seq_num++);
}
/* request info about a user
 */
void icq_send_info_req(  DWORD uin )
{
	net_icq_pak pak;

	inforeplymaskbyte = inforeplysuccessbyte = 0;
   DW_2_Chars( pak.data, uin );

	SOCKWRITE( CMD_INFO_REQ, &pak, 4 + sizeof(pak.head), seq_num++);

  DW_2_Chars( pak.data , uin );

	SOCKWRITE( CMD_EXT_INFO_REQ, &pak, 4 + sizeof(pak.head), seq_num++);
}
/* start a search */
void icq_start_search(  USER_INFO * user)
{
	net_icq_pak pak;
	int size ;

   size = 0;
	 size += Putstring(pak.data + size, user->nick);
	 size += Putstring(pak.data + size, user->first);
   size += Putstring(pak.data + size, user->last);
	 size += Putstring(pak.data + size, user->email);
	SOCKWRITE( CMD_SEARCH_USER, &pak, size + sizeof(pak.head),seq_num++);
}
/* add a user to our contact list */
int icq_Add_User( DWORD uin)
{
	 int i;
	 CONTACT_INFO *contact;

	 contact = FindContact(uin);
	 if (contact && !(contact->flags & FL_WAITING_FOR_AUTH))
			return 0;

	 if (!contact) {
			contact = FindEmptyContact();
			if (!contact)
				return 0;
   		contact->ci.uin = uin;
   		contact->ci.status = STATUS_OFFLINE;
   		contact->last_time = MINUSONE;
   		contact->ci.ip[0] = 0xff;
   		contact->ci.ip[1] = 0xff;
   		contact->ci.ip[2] = 0xff;
   		contact->ci.ip[3] = 0xff;
   		contact->ci.port = 0;
			contact->flags = FL_WAITING_FOR_ACK | FL_VALID;
	 }
	 else {
		 contact->flags |= FL_WAITING_FOR_ACK;
	 }
	 contact->flags &= ~FL_NOT_IN_LIST;
	 icq_send_info_req(uin);
	 return 1;
}
/* update our user info.  ABOUT field is broken, don't know why, make sure
 * it is null
 */
void icq_Update_User_Info( USER_INFO * user)
{
	net_icq_pak pak;
	int size ;
	char buf[256];
	infosuccessbyte = infomaskbyte = 0;
	our_user = *user;

   size = 0;
	 size += Putstring(pak.data + size, user->nick);
	 size += Putstring(pak.data + size, user->first);
   size += Putstring(pak.data + size, user->last);
	 size += Putstring(pak.data + size, user->email);
   pak.data[ size ] = user->auth ? 0 : 1;
   size++;
	SOCKWRITE( CMD_UPDATE_INFO, &pak, size + sizeof(pak.head),seq_num++);
	size = 0;
	size += Putword(pak.data, user->auth ? 0 : 1);
	SOCKWRITE( CMD_AUTHORIZE_CHANGE, &pak, size + sizeof(pak.head), seq_num++);
	size = 0;
	size += Putstring(pak.data + size, user->city);
	size += Putword(pak.data + size,user->country);
	pak.data[size++] = user->timezone;
	size += Putstring(pak.data + size, user->state);
	size += Putword(pak.data + size, user->age);
	pak.data[size++] = user->sex;
	size += Putstring(pak.data + size, user->phone);
	size += Putstring(pak.data + size, user->homepage);
	size += Putstring(pak.data + size, user->about);
	SOCKWRITE( CMD_EXT_INFO_UPDATE, &pak, size + sizeof(pak.head), seq_num++);
}
/* send a URL */
void icq_sendurl( DWORD uin, char *description, char *url )
{
   char buf[450];
   
   sprintf( buf, "%s\xFE%s", url, description );
   icq_sendmsg( uin, buf, strlen(buf)+1, URL_MESS );
}
/* authorize him to add us */
void icq_send_auth_msg( DWORD uin)
{
  int i;
	CONTACT_INFO *contact = FindContact(uin);

	if (contact) {
		if (contact->flags & FL_AUTH_IMPERM)
			contact->flags |= FL_NOT_IN_LIST;
	}
	contact->flags &= ~FL_WAITING_FOR_AUTH;

  icq_sendmsg(uin, "\1\0\0",3,AUTH_ACCEPTED_MESS);
}
/* deny him adding us */
void icq_send_reject_auth_msg( DWORD uin)
{
  int i;
	CONTACT_INFO *contact = FindContact(uin);

	if (contact) {
		if (contact->flags & FL_AUTH_IMPERM)
			contact->flags |= FL_NOT_IN_LIST;
	}
	contact->flags &= ~FL_WAITING_FOR_AUTH;

  icq_sendmsg(uin, "\1\0\0",3,AUTH_DENIED_MESS);
}
/* notify him we added him */
void icq_Notify_Added( DWORD uin)
{
	char userinfo[1000];
	sprintf(userinfo,"%s\xfe%s\xfe%s\xfe%s",
			our_user.nick,our_user.first,our_user.last,our_user.email);
	icq_sendmsg(uin, userinfo, strlen(userinfo)+1, USER_ADDED_MESS);
}
/* ask him if we can add him */
void icq_Ask_For_Authorize( DWORD uin, char *reason)
{
	char userinfo[1000];
	sprintf(userinfo,"%s\xfe%s\xfe%s\xfe%s\xfe%c\xfe%s",
			our_user.nick,our_user.first,our_user.last,our_user.email,our_user.auth ? 0 : 1,
			reason);
	icq_sendmsg(uin, userinfo, strlen(userinfo)+1, AUTH_REQ_MESS);
}
/* find a random user */
void icq_rand_user_req( DWORD group )
{
   net_icq_pak pak;
   int size ;

   DW_2_Chars( pak.data, group);

   size = 4;

   SOCKWRITE( CMD_RAND_SEARCH, &pak, size + sizeof( pak.head ), seq_num++);
}
/* set our random group */
void icq_rand_set( DWORD group )
{
   net_icq_pak pak;
   int size ;

   DW_2_Chars( pak.data, group);

   size = 4;

   SOCKWRITE( CMD_RAND_SET, &pak, size + sizeof( pak.head ), seq_num++);
}
/* remove someone from the invisible list */
void icq_remove_invisible ( DWORD uin)
{
	net_icq_pak pak;
	CONTACT_INFO *contact = FindContact(uin);
	if (!contact || !(contact->flags & FL_INVISIBLE))
		return;
  pak.data[4] = 2;
	pak.data[5] = 0;
	SOCKWRITE(  CMD_ADD_VISI_LIST, &pak,6 + sizeof(pak.head), seq_num++);
	contact->flags &= ~FL_INVISIBLE;
}
/* remove someone from the visible list */
void icq_remove_visible ( DWORD uin)
{
	net_icq_pak pak;
	CONTACT_INFO *contact = FindContact(uin);
	if (!contact || !(contact->flags & FL_VISIBLE))
		return;
  pak.data[4] = 1;
	pak.data[5] = 0;
	SOCKWRITE(  CMD_ADD_VISI_LIST, &pak,6 + sizeof(pak.head), seq_num++);
	contact->flags &= ~FL_VISIBLE;
}
/* add someone to the visible list */
void icq_add_visible( DWORD uin)
{
	net_icq_pak pak;
	CONTACT_INFO *contact = FindContact(uin);
	if (!contact)
		return;
	icq_remove_invisible(uin);
	DW_2_Chars(pak.data,uin);
	pak.data[4] = 1;
	pak.data[5] = 1;
	SOCKWRITE(  CMD_ADD_VISI_LIST, &pak,6 + sizeof(pak.head), seq_num++);
	contact->flags |= FL_VISIBLE;
}
/* add someone to the invisible list */
void icq_add_invisible( DWORD uin)
{
	net_icq_pak pak;
	CONTACT_INFO *contact = FindContact(uin);
	if (!contact)
		return;
	icq_remove_visible(uin);
	DW_2_Chars(pak.data,uin);
	pak.data[4] = 2;
	pak.data[5] = 1;
	SOCKWRITE(  CMD_ADD_VISI_LIST, &pak,6 + sizeof(pak.head), seq_num++);
	contact->flags |= FL_INVISIBLE;
}
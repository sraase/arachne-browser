/*
 * Server.C
 *
 * This module handles messages received from the ICQ server
 *
 * David Lindauer, 1999
 */
#include "lsicq.h"

extern int inforeplymaskbyte,inforeplysuccessbyte,infosuccessbyte,infomaskbyte;
extern BOOL serv_mess[ 1024 ]; /* used so that we don't get duplicate messages with the same SEQ */
extern ICQ_CALLBACK icq_callback;
extern USER_INFO our_user;
extern int verbose;

/* Copy a counted string (WORD length + null-term string)
 * into a normal C string
 * fieldlen is the max length of the C string
 */
static int GetString(char *string, char *pos, int fieldlen)
{
	int len;
	len = Chars_2_Word(pos);
	memcpy(string,pos+2,fieldlen);
	string[fieldlen-1] = 0;
	return len+2;
}
/*
 * Copy a string terminated with the 0xfe character into a normal C string
 */
static int GetFEString(char *string, char *pos, int fieldlen)
{
	int len,xlen;
	char *xpos;
	xpos = strchr(pos,'\xfe');
	if (!xpos)
		xpos = pos + strlen(pos);
	xlen = len = xpos - pos;
	if (xlen > fieldlen-1)
		xlen = fieldlen-1;
	memcpy(string,pos,xlen);
	string[xlen] = 0;
	string[fieldlen-1] = 0;
	if (*xpos)
		return len+1;
	return len;
}
/*
 * Fill user info struct with basic info.  Basic info results from an
 * info request or search comand
 */
static void FillUserInfo(USER_INFO *user,char *data)
{
			int len;
			user->uin = Chars_2_DW(&data[0]);
			len = 4 + GetString(user->nick,data + 4,20);
			len += GetString(user->first,data+len,20);
			len += GetString(user->last,data+len,40);
			len += GetString(user->email,data+len,80);
			if (data[len] == 1)
				user->auth = 0;
			else
				user->auth = 1;
}
/*
 * Get the TCP/IP info connected with the user
 */
static void FillConnectInfo(CONNECT_INFO *connect, char *data)
{
			connect->uin = Chars_2_DW(&data[0]);
			connect->status = GetStatusVal(Chars_2_DW(&data[17]));
      connect->ip[0] =  data[4];
      connect->ip[1] =  data[5];
      connect->ip[2] =  data[6];
      connect->ip[3] =  data[7];
      connect->port = Chars_2_DW( &data[8] );
			connect->TCPver = Chars_2_DW(&data[21] );
			connect->connection = data[16];
}
/*
 * request for info replies come here.  We request info from the UI if
 * the user wants info or if we need to know the AUTH flag so we can
 * decide wether to ask for authorization
 */
static void HandleInfoReply( int flag, int type, char *data)
{
	static USER_INFO user;
	CONTACT_INFO *contact;
	int len = 0,i;

	inforeplymaskbyte |= type;
  if (flag == SUCCESS) {
		inforeplysuccessbyte |= type;
		if (type == UI_BASIC) {
			/* load basic info */
			FillUserInfo(&user,data);
			contact = FindContact(Chars_2_DW(data));
			if (contact && (contact->flags & FL_WAITING_FOR_ACK)) {
				/* if we get here we are going to try to add him to our contact
				 * list.  */
				strcpy(contact->nick,user.nick);
				if (!user.auth) {
					/* Don't need auth, just notify him and do it */
					contact->flags &= ~(FL_WAITING_FOR_ACK | FL_NOT_IN_LIST);
					icq_Notify_Added(contact->ci.uin);
					icq_callback(IM_NOTIFYADDED,0,contact,0,0);
					snd_contact_list();
				}
				else {
					/* Need auth, go ask for it. */
					icq_Ask_For_Authorize(contact->ci.uin,"");
					icq_callback(IM_ASKFORAUTH,0,contact,0,0);
				}
				inforeplymaskbyte = 0;
				return;
			}
		}
		else if (type == UI_EXTENDED) {
			/* load extended info */
		  len = 4 + GetString(user.city,data+4,20);
	  	user.country = Chars_2_Word(&data[len]);
		  len += 2;
		  user.timezone = data[len];
	  	len ++;
	    len += GetString(user.state,data+len,6);
		  user.age = Chars_2_Word(&data[len]);
			len += 2;
			user.sex = data[len++];
		  len += GetString(user.phone,data+len,20);
	  	len += GetString(user.homepage,data+len,80);
		  len += GetString(user.about,data+len,400);
		}
	}
	else
		user.uin = Chars_2_DW(data);
	if (inforeplymaskbyte == (UI_BASIC | UI_EXTENDED))
		icq_callback(IM_INFORESPONSE,inforeplysuccessbyte,0,&user,0);
}
/* get here if a server has sent a time-stamped text message
 * (CMD 220)
 */
static void Recv_Message( BYTE * pak )
{
   RECV_MESSAGE_PTR r_mesg;
	 CONTACT_INFO *contact;
   DWORD uin;
	struct tm tms;
  time_t thetime;

	/* gather the time */
   r_mesg = ( RECV_MESSAGE_PTR )pak;
	tms.tm_sec = 0;
	tms.tm_min = r_mesg->minute;
	tms.tm_hour = r_mesg->hour;
	tms.tm_mday = r_mesg->day-1;
	tms.tm_mon = r_mesg->month-1;
	tms.tm_year = Chars_2_Word(r_mesg->year)-1900;
		thetime = mktime(&tms);
		thetime += our_user.timezone * 3600*2;
   uin = Chars_2_DW( r_mesg->uin );

	 /* process the message */
   Do_Msg(Chars_2_Word( r_mesg->type ), Chars_2_Word( r_mesg->len ), ( r_mesg->len + 2 ), uin, 1, thetime ); 
   
	 /* register the user if he is not in our list */
	 contact = FindContact(uin);
	 if (!contact) {
		 contact = FindEmptyContact();
		 if (!contact)
			 return;
		 contact->flags = FL_RECEIVED_FROM | FL_NOT_IN_LIST;
	   contact->ci.uin = uin;
     contact->ci.status = STATUS_OFFLINE;
     contact->last_time = MINUSONE;
     contact->ci.ip[0] = 0xff;
     contact->ci.ip[1] = 0xff;
     contact->ci.ip[2] = 0xff;
     contact->ci.ip[3] = 0xff;
     contact->ci.port = 0;
   	 snd_contact_list();
	 }
	 else
		 contact->flags |= FL_RECEIVED_FROM;
}

/* main handling for text messages.  Called from the CMD 220 and CMD 270
 * handlers */
static void Do_Msg( DWORD type, WORD len, char * data, DWORD uin, int dated, time_t thedate )
{
	CONTACT_INFO *Contact;
	USER_INFO user;
	char reason[500],url[200];
	int xlen,i,m;
	switch (type) {
		/* if they ask for authorization */
		case AUTH_REQ_MESS:
			user.uin = uin;
			xlen = GetFEString(user.nick,data,20);
			xlen += GetFEString(user.first,data+xlen,20);
			xlen += GetFEString(user.last,data+xlen,40);
			xlen += GetFEString(user.email,data+xlen,80);
			user.auth = data[xlen] ? 0 : 1;
			xlen+=2;
			xlen += GetFEString(reason,data+len,500);
			Contact = FindContact(uin);
			if (Contact) {
				Contact->flags |= FL_WAITING_FOR_AUTH;
			}
			else {
				Contact = FindEmptyContact();
				if (!Contact)
					return;
				strcpy(Contact->nick,user.nick);
				Contact->flags = FL_RECEIVED_FROM | FL_WAITING_FOR_AUTH | FL_AUTH_IMPERM | FL_VALID;
			  Contact->ci.uin = uin;
		    Contact->ci.status = STATUS_OFFLINE;
		    Contact->last_time = MINUSONE;
		    Contact->ci.ip[0] = 0xff;
		    Contact->ci.ip[1] = 0xff;
		    Contact->ci.ip[2] = 0xff;
		    Contact->ci.ip[3] = 0xff;
		    Contact->ci.port = 0;
			}
			icq_callback(IM_AUTHREQ | dated,0,Contact,&user,thedate);
			break;
		case AUTH_DENIED_MESS:
			/* they denied us authorization */
			Contact = FindContact(uin);
			if (!Contact)
				return;
			Contact->flags |= FL_NOT_IN_LIST ;
			Contact->flags &= ~(FL_WAITING_FOR_ACK);
			icq_callback(IM_AUTHDENIED | dated,0,Contact,0,thedate);
			break;
			/* they authorized us */
		case AUTH_ACCEPTED_MESS:
			Contact = FindContact(uin);
			if (!Contact)
				return;
			Contact->flags &= ~(FL_WAITING_FOR_ACK | FL_NOT_IN_LIST | FL_AUTH_IMPERM);
			icq_callback(IM_AUTHACCEPTED | dated,0,Contact,0,thedate);
			snd_contact_list();
			break;
			/* Someone added us to his contact list */
		case USER_ADDED_MESS:
			user.uin = uin;
			xlen = GetFEString(user.nick,data,20);
			xlen += GetFEString(user.first,data+len,20);
			xlen += GetFEString(user.last,data+len,40);
			xlen += GetFEString(user.email,data+len,80);
			icq_callback(IM_USERADDED | dated,0,0,&user,thedate);
			break;
			/* someone has sent a URL */
		case URL_MESS:
		case MRURL_MESS:
			xlen = GetFEString(url,data,200);
			xlen += GetFEString(reason,data+xlen,500);
			icq_callback(IM_URLRECEIVED| dated,uin,url,reason,thedate);
			break;
			/* someone has sent a contact list (not fully supported ) */
		case CONTACT_MESS:
		case MRCONTACT_MESS:
			xlen = GetFEString(reason,data,20);
			m = atoi(reason);
			for (i=0; i < m; i++) {
				xlen += GetFEString(reason,data+len,200);
				xlen += GetFEString(url,data+len,200);
				icq_callback(IM_CONTACTRECEIVED | dated,uin,reason,url,thedate);
			}
			icq_callback(IM_CONTACTRECEIVED,0,0,0,0);
			break;
			/* other messages we just treat as normal text */
		default:
			icq_callback(IM_INSTANTMESS | dated,uin,data,0,thedate);
			break;
	}
}
/*
 * after updating we wait for three messages telling us the update
 * status then perform the callback
 */
static void infoupdate(int flag,int type)
{
	if (flag)
		infosuccessbyte |= type;
	infomaskbyte |= type;
	if (infomaskbyte == (IR_INFO | IR_AUTH | IR_EXT))
		icq_callback(IM_UPDATEDONE,infosuccessbyte,0,0,0);
}
/* Server has sent a CMD 530, which has other packets nested inside it */
static void Multi_Packet( BYTE *data )
{
   int num_pack, i;
   int len;
   BYTE * j;
   srv_net_icq_pak pak;
   num_pack = (unsigned char) data[0];
   j = data;
   j++;
   
   for ( i=0; i < num_pack; i++ )
   {
       len = Chars_2_Word( j );
       pak = *( srv_net_icq_pak *) (j+2) ;
       j += 2+len;
			 if (verbose) {
				 M_print("Multi response cmd %d\n",Chars_2_Word(pak.head.cmd));
			 }
       Server_Response( pak.data, (len+2) - sizeof( pak.head ), Chars_2_Word( pak.head.cmd ),
                 Chars_2_Word( pak.head.ver ), Chars_2_Word( pak.head.seq )
                 , Chars_2_Word( pak.head.UIN ) );
   }
}
/* This is the main handler for messages received from the server */
void Server_Response( BYTE *data, DWORD len, WORD cmd, WORD ver, WORD seq, DWORD uin )
{
   SIMPLE_MESSAGE_PTR s_mesg;
   int i;
	 CONNECT_INFO random;
	 CONTACT_INFO *contact;
   USER_INFO user;
	 DWORD xuin;
   
   switch ( cmd )
   {
	   case SRV_ACK:
  	    Check_Queue( seq );
    	  break;
	   case SRV_MULTI_PACKET:
  	    Multi_Packet(  data );
    	  break;
	   case SRV_NEW_UIN:
				icq_callback(IM_NEWUIN,uin,0,0,0);
    	  break;
	   case SRV_UPDATE_FAIL:
				infoupdate(FAIL,IR_INFO);
    	  break;
	   case SRV_UPDATE_SUCCESS:
				infoupdate(SUCCESS,IR_INFO);
    	  break;
		 case SRV_AUTHORIZE_SUCCESS:
				infoupdate(SUCCESS,IR_AUTH);
				break;      
		 case SRV_AUTHORIZE_FAIL:
				infoupdate(FAIL,IR_AUTH);
				break;      
		 case SRV_UPDATE_EXT_SUCCESS:
				infoupdate(SUCCESS,IR_EXT);
				break;
		 case SRV_UPDATE_EXT_FAIL:
				infoupdate(FAIL,IR_EXT);
				break;
	   case SRV_LOGIN_REPLY:
      	snd_contact_list();
	      snd_invis_list();
  	    snd_vis_list();
				icq_callback(IM_LOGIN,0,0,0,0);
	      break;
  	 case SRV_X1: /* unknown message  sent after login*/
				icq_callback(IM_LOGIN2,0,0,0,0);
      	break;
	   case SRV_X2: /* unknown message  sent after login*/
  	    icq_snd_got_messages();
    	  break;
	   case SRV_GO_AWAY:
				icq_callback(IM_FORCEDDISCONNECT,0,0,0,0);
    	  break;
	   case SRV_END_OF_SEARCH:
				icq_callback(IM_SEARCHDONE,0,0,0,0);
    	  break;
	   case SRV_BAD_PASS:
				icq_callback(IM_BADPASS,0,0,0,0);
    	  break;
	   case SRV_TRY_AGAIN:
				icq_callback(IM_ACCOUNTBUSY,0,0,0,0);
    	  for ( i = 0; i< 1024; i++ )
      	{
        	 serv_mess[ i ]=FALSE;
  	    }
	      break;
	   case SRV_USER_OFFLINE:
				contact = FindContact(Chars_2_DW(&data[0]));
				if (!contact)
					return;
				icq_callback(IM_USEROFFLINE,0,contact,0,0);
	      contact->ci.status = STATUS_OFFLINE;
  	    contact->last_time = time( NULL );
      	break;
	   case SRV_USER_ONLINE:
				contact = FindContact(Chars_2_DW( &data[0] ));
				if (!contact)
					return;
				FillConnectInfo(&contact->ci,data);
				icq_callback(IM_USERONLINE,0,contact,0,0);
    	  break;
	   case SRV_STATUS_UPDATE:
				contact = FindContact(Chars_2_DW(&data[0]));
				if (!contact)
					return;
				contact->ci.status = GetStatusVal(Chars_2_DW(&data[4]));
				icq_callback(IM_STATUSUPDATE,0,contact,0,0);
    	  break;
	   case SRV_RAND_USER:
				if (len == 37) {
					FillConnectInfo(&random,data);
					icq_callback(IM_RANDRESPONSE,0,&random,0,0);
				}
				else
					icq_callback(IM_RANDRESPONSE,0,0,0,0);
      	break;
	   case SRV_USER_FOUND:
				FillUserInfo(&user,data);
				icq_callback(IM_USERFOUND,0,0,&user,0);
      	break;
	   case SRV_INFO_REPLY:
				HandleInfoReply(SUCCESS, UI_BASIC, data);
    	  break;
		 case SRV_INFO_NONE:
				HandleInfoReply(FAIL, UI_BASIC, data);
				break;
	   case SRV_EXT_INFO_REPLY:
				HandleInfoReply(SUCCESS, UI_EXTENDED, data);
    	  break;
		 case SRV_EXT_INFO_NONE:
				HandleInfoReply(FAIL, UI_EXTENDED, data);
				break;
	   case SRV_RECV_MESSAGE:
    	  Recv_Message(  data );
	      break;
  	 case SRV_SYS_DELIVERED_MESS:
    	  s_mesg = ( SIMPLE_MESSAGE_PTR ) data;
      	xuin = Chars_2_DW( s_mesg->uin );
      	Do_Msg(  Chars_2_Word( s_mesg->type ), Chars_2_Word( s_mesg->len ), 
           s_mesg->len + 2,xuin, 0, 0); 
	 			break;
	   default: /* commands we dont handle yet */
				 icq_callback(IM_UNHANDLED,cmd,data,(void *)len,0);
    	   break;
   }

}
/*
 * this is a wrapper that sends ACK packets as required then
 * calls the server message handler
 */
void Handle_Server_Response( void )
{
   srv_net_icq_pak pak;
   int s;
   s = SOCKREAD( &pak.head.ver, sizeof( pak ) );
   if ( s < 0 )
   	return;

	 if (verbose) {
		 M_print("received Cmd %d\n",Chars_2_Word(pak.head.cmd));
	 }	      
   if ( ( serv_mess[ Chars_2_Word( pak.head.seq ) ] ) && 
      ( Chars_2_Word( pak.head.cmd ) != SRV_NEW_UIN ) &&
   		( Chars_2_Word( pak.head.cmd ) != SRV_ACK ) ) {
         ack_srv( Chars_2_Word( pak.head.seq ) ); /* LAGGGGG!! */
         return;
      }
   if ( Chars_2_Word( pak.head.cmd ) != SRV_ACK )
   {
      serv_mess[ Chars_2_Word( pak.head.seq ) ] = TRUE;
      ack_srv( Chars_2_Word( pak.head.seq ) );
   }
	 
   Server_Response( pak.data, s - sizeof( pak.head ), 
        Chars_2_Word( pak.head.cmd ), Chars_2_Word( pak.head.ver ),
        Chars_2_Word( pak.head.seq ), Chars_2_DW( pak.head.UIN ) );
}
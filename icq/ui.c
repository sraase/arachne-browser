/*
 * UI.C
 *
 * This module is a simple text-based User Interface
 *
 * David Lindauer, 1999
 */
#include "lsicq.h"

extern CONTACT_INFO Contacts[100];
extern USER_INFO our_user;
extern DWORD Current_Status;
extern int beep;
extern char password[9];
int verbose;

/* Prompt constants */
#define GET_USER_INFO 100
#define SEARCH_USER_INFO 200
#define RANDGROUP 1000
#define RANDSEARCH 1001
#define GET_UIN_INFO 1002
#define ADDUSER 1003
#define SEND_URL_UIN 1004
#define SEND_URL 1005
#define SEND_URL_DESCRIPTION 1006
#define SEND_MSG_UIN 1007
#define SEND_MSG 1008
#define REJECT_MSG 1009
#define ACCEPT_MSG 1010
#define AUTH_REASON 1011
#define STATUS_GET 1012

static char authreason[400];
static DWORD authuin;
static int prompt_level = 0;
static SOK_T s;                   
static char hostname[] = "icq1.mirabilis.com";
static int done;
static int at_prompt;
static int account_busy;
static time_t login_time, retry_time;
static DWORD randresponseuin, last_received_uin;

/* put a char */
static void M_putch(char v)
{
	fputc(v,stdout);
}
/* put a string, the hard way */
static void M_prints( char *str )
{
   int i,v;
   
   for ( i=0; (v = str[i]) != 0; i++ )
			M_putch(v);
}
/* print a string, handling the prompt and using varargs */
void M_print( char *str, ... )
{
   va_list args;
   char buf[2048];
   
   if (at_prompt) {
     M_putch('\n');
     at_prompt = FALSE;
   }
   va_start( args, str );
   vsprintf( buf, str, args );
   M_prints( buf );
   va_end( args );
}
/* get a key from the keyboard */
static int getkey(void)
{
		if (!kbhit())
			return 0;
		else
			return getch();
}
/* display whatever prompt we are at */
static void Prompt(void)
{
	at_prompt = TRUE;
	switch(prompt_level) {
		default:
			 M_print("lsicq> ");
			break;
		case SEND_URL:
			M_print("URL: ");
			break;
		case SEND_URL_DESCRIPTION:
			M_print("URL description: ");
			break;
		case SEND_MSG:
			M_print("Message text: ");
			break;
		case RANDGROUP:
			M_print("enter group to be a part of: ");
			break;
		case RANDSEARCH:
			M_print("enter group to search in: ");
			break;
		case GET_UIN_INFO:
		case ADDUSER:
		case SEND_URL_UIN:
		case SEND_MSG_UIN:
		case REJECT_MSG:
		case ACCEPT_MSG:
			M_print("uin: ");
			break;
		case AUTH_REASON:
			M_print("reason: ");
			break;
		case STATUS_GET:
			M_print("status: ");
			break;
		case SEARCH_USER_INFO:
		case GET_USER_INFO:
			M_print("nickname? ");
			break;
		case SEARCH_USER_INFO + 1:
		case GET_USER_INFO + 1:
			M_print("first name? ");
			break;
		case SEARCH_USER_INFO + 2:
		case GET_USER_INFO + 2:
			M_print("last name? ");
			break;
		case GET_USER_INFO + 3:
			M_print("city? ");
			break;
		case GET_USER_INFO + 4:
			M_print("state? ");
			break;
		case GET_USER_INFO + 5:
			M_print("Country? ");
			break;
		case GET_USER_INFO + 6:
			M_print("age? ");
			break;
		case GET_USER_INFO + 7:
			M_print("gender? ");
			break;
		case SEARCH_USER_INFO + 3:
		case GET_USER_INFO + 8:
			M_print("email? ");
			break;
		case GET_USER_INFO + 9:
			M_print("phone? ");
			break;
		case GET_USER_INFO + 10:
			M_print("home page? ");
			break;
		case GET_USER_INFO + 11:
			M_print("timezone? ");
			break;
		case GET_USER_INFO + 12:
			M_print("auth? ");
			break;
		case GET_USER_INFO + 13:
			M_print("about? ");
			break;
	}
}
/*
 * input a string, with cooperative multitasking.
 * If a sting has been printed, display the prompt then redisplay
 * what the user was ty[ing.  If any keys process them and return to
 * the caller
 */
static int M_fdnGetCH(char *buf, char **pos)
{
			int i;
			/* handle refresh of prompt */
			if (!at_prompt) {
				Prompt();
				M_prints(buf);
				at_prompt = TRUE;
			}
			/* reset position if turned null */
			if (!*pos)
				*pos = buf;
	
			/* quit if no keys */
			if (!(i = getkey())) return 0;

			/* process the key */
			switch (i) {
				case '\r':
					M_putch('\n');
					*(*pos)++ = 0;
					*pos = buf;
					at_prompt = FALSE;
					return 1;
				case '\b':
					if (*pos > buf) {
						M_putch('\b');
						M_putch(' ');
						M_putch('\b');
						*(--(*pos)) = 0;
					}
					break;
				default:
					M_putch(i);
					*(*pos)++ = i;
					*(*pos) = 0;
					break;
			}
	return 0;
}
/* determine if we can display their online status  */
static int Online( CONTACT_INFO *contact)
{
	if (contact->ci.status == STATUS_OFFLINE)
		return FALSE;
	if (contact->ci.status == STATUS_INVISIBLE && !(contact->flags & FL_RECEIVED_FROM))
		return FALSE;
	return TRUE;
}
/* return a string reflecting their status
 */
static char * GetStatusString(CONTACT_INFO *contact)
{
	char *status;
	switch (contact->ci.status) {
		case STATUS_ONLINE:
			status = "online";
			break;
    case STATUS_OCCUPIED:
			status = "occupied";
			break;
    case STATUS_NA:
			status = "not available";
			break;
    case STATUS_FREE_CHAT:
			status = "free for chat";
			break;
    case STATUS_AWAY:
			status = "away";
			break;
    case STATUS_DND:
			status = "do not disturb";
			break;
    case STATUS_INVISIBLE:
			if (contact->flags & FL_RECEIVED_FROM) {
				status = "invisible";
				break;
			}
		default:
		case STATUS_OFFLINE:
			status = "offline";
			break;
	}
	return status;
}
/* show one line of status */
static void ShowStatus( CONTACT_INFO *contact)
{
	M_print("%-20s\t%s\n",contact->nick,GetStatusString(contact));
}
/* M_print with embedded === characters */
static void PrintBar(char *id,...)
{
	int l,i;
  va_list args;
  char buf[2048];
   
  va_start( args, id );
  vsprintf( buf, id, args );
  va_end( args );
	
	l=70 - strlen(buf);
	M_print("===%s",buf);
	for (i=0; i < l; i++)
		M_print("=");
	M_print("\n");
}
/* show all offline users */
static void ShowOffline(void)
{
	int i;
	for (i=0; i < 100; i++)
		if ((Contacts[i].flags & FL_VALID) && !(Contacts[i].flags & (FL_WAITING_FOR_AUTH | FL_WAITING_FOR_ACK | FL_NOT_IN_LIST))) {
			if (!Online(&Contacts[i]))
				ShowStatus(&Contacts[i]);
		}
}
/* show all online users */
static void ShowOnline(void)
{
	int i;
	for (i=0; i < 100; i++)
		if ((Contacts[i].flags & FL_VALID) && !(Contacts[i].flags & (FL_WAITING_FOR_AUTH | FL_WAITING_FOR_ACK | FL_NOT_IN_LIST))) {
			if (Online(&Contacts[i]))
				ShowStatus(&Contacts[i]);
		}
}
/* show users we are waiting to authorize */
static void ShowAuth(void)
{
	int i;
	for (i=0; i < 100; i++)
		if ((Contacts[i].flags & FL_VALID) && (Contacts[i].flags & FL_WAITING_FOR_AUTH)) {
			M_print("%s\n",Contacts[i].nick);
		}
}
/* show users who need to acknowledge us */
static void ShowAck(void)
{
	int i;
	for (i=0; i < 100; i++)
		if ((Contacts[i].flags & FL_VALID) && (Contacts[i].flags & FL_WAITING_FOR_ACK)) {
			M_print("%s\n",Contacts[i].nick);
		}
}
/* show users who aren't in contact list for some reason */
static void ShowNotInList(void)
{
	int i;
	for (i=0; i < 100; i++)
		if ((Contacts[i].flags & FL_VALID) && (Contacts[i].flags & FL_NOT_IN_LIST)) {
			M_print("%s\n",Contacts[i].nick);
		}
}

/* show all users */
static void ShowUsers(void)
{
	PrintBar("offline");
	ShowOffline();
	PrintBar("online");
	ShowOnline();
	PrintBar("needs your authorization");
	ShowAuth();
	PrintBar("waiting for his authorization");
	ShowAck();
	PrintBar("Not in list");
	ShowNotInList();
	PrintBar("");
}
/* display help screen */
static void helpscreen(void)
{
	PrintBar("");
	M_print("accept     - accept a user for authorization\n");
	M_print("adduser    - add a user to the contact list\n");
	M_print("getinfo    - get info on a UIN\n");
  M_print("help       - this help\n");
	M_print("quit       - log off icq network and quit program\n");
	M_print("randgroug  - set a random user group\n");
	M_print("randsearch - search for a random user\n");
	M_print("search     - search for users matching description\n");
	M_print("reject			- reject a user for authorization\n");
	M_print("reply      - reply to message\n");
	M_print("send       - send message\n");
	M_print("sendlast   - send message to last uin you sent to\n");
	M_print("sendurl    - send a URL to someone\n");
	M_print("status			- show/change online status\n");
	M_print("update     - update user info\n");
	M_print("w          - show user list\n");
	M_print("\nYou will be prompted for arguments, don't put them on the command line\n");
}
/* memcpy with a null terminator */
static void CopyData(char *dest, char *src, int len)
{
	memcpy(dest,src,len);
	dest[len-1] = 0;
}
/* handle one of the update info prompts and move to the next prompt in
 * the sequence */
static void InfoPrompt(char *buf)
{
	switch(prompt_level) {
		case GET_USER_INFO:
			if (!buf[0])
				M_print("You MUST enter a nickname\n");
			else {
				CopyData(our_user.nick,buf,20);
				prompt_level++;
			}	
			break;
		case GET_USER_INFO + 1:
			CopyData(our_user.first,buf,20);
			prompt_level++;
			break;
		case GET_USER_INFO + 2:
			CopyData(our_user.last,buf,40);
			prompt_level++;
			break;
		case GET_USER_INFO + 3:
			CopyData(our_user.city,buf,40);
			prompt_level++;
			break;
		case GET_USER_INFO + 4:
			CopyData(our_user.state,buf,6);
			prompt_level++;
			break;
		case GET_USER_INFO + 5:
			if (buf[0]) {
				our_user.country = Get_Country_Code(buf);
				if (our_user.country == 0xffff)
					M_print("Unknown country, try again\n");
				else prompt_level++;
			}
			else our_user.country = 0xffff;
			break;
		case GET_USER_INFO + 6:
			if (buf[0])
				our_user.age = atoi(buf);
			else
				our_user.age = 0xffff;
			prompt_level++;
			break;
		case GET_USER_INFO + 7:
			if (buf[0])
				our_user.sex = buf[0] == 'M' || buf[0] == 'm' ? 2 : 1;
			else
				our_user.sex = 0;
			prompt_level++;
			break;
		case GET_USER_INFO + 8:
			CopyData(our_user.email,buf,80);
			prompt_level++;
			break;
		case GET_USER_INFO + 9:
			CopyData(our_user.phone,buf,20);
			prompt_level++;
			break;
		case GET_USER_INFO + 10:
			CopyData(our_user.homepage,buf,80);
			prompt_level++;
			break;
		case GET_USER_INFO + 11:
			our_user.timezone = atoi(buf) * 2;
			prompt_level++;
			break;
		case GET_USER_INFO + 12:
			our_user.auth = buf[0] == 'Y' || buf[0] == 'y';
			prompt_level++;
			our_user.about[0] = 0;

  				prompt_level = 0;
	  			icq_Update_User_Info(&our_user);
			break;
		case GET_USER_INFO + 13:
			if (!buf[0]) {
				prompt_level = 0;
				icq_Update_User_Info(&our_user);
			}
			else {
				if (strlen(our_user.about) + strlen(buf) + 1 < 400) {
					strcat(our_user.about,buf);
					strcat(our_user.about,"\n");
				}
			}
			break;
		default:
			prompt_level = 0;
			break;
	}
}
/*
 * handle data for one of the search info prompts and move to the next prompt
 */
static void SearchInfoPrompt(char *buf)
{
	static USER_INFO search;
	switch(prompt_level) {
		case SEARCH_USER_INFO:
			memset (&search,0,sizeof(search));
			CopyData(search.nick,buf,20);
			prompt_level++;
			break;
		case SEARCH_USER_INFO + 1:
			CopyData(search.first,buf,20);
			prompt_level++;
			break;
		case SEARCH_USER_INFO + 2:
			CopyData(search.last,buf,40);
			prompt_level++;
			break;
		case SEARCH_USER_INFO + 3:
			CopyData(search.email,buf,80);
			icq_start_search(&search);
		default:
			prompt_level = 0;
			break;
	}
}
/* show a list of random group identifiers */
static void GroupList(void)
{
	M_print("Enter the group as follows:\n");
	M_print(" 0 = None\n");
	M_print(" 1 = General\n");
	M_print(" 2 = Romance\n");
	M_print(" 3 = Games\n");
	M_print(" 4 = Students\n");
	M_print(" 6 = 20 something\n");
	M_print(" 7 = 30 something\n");
	M_print(" 8 = 40 something\n");
  M_print(" 9 = 50+\n");
	M_print("10 = Man seeking woman\n");
	M_print("11 = Woman seeking man\n");
	
}
/* show a list of possible status values */
static void StatusList(void)
{
	M_print("Your status is %d\n",Current_Status);
	M_print("Select from the list below\n");
	M_print(" 0 = leave status the same\n");
	M_print(" 1 = online\n");
	M_print(" 2 = invisible\n");
	M_print(" 3 = occupied\n");
	M_print(" 4 = Not Available\n");
	M_print(" 5 = Free for chat\n");
	M_print(" 6 = away\n");
	M_print(" 7 = dnd\n");
}
/*
 * See if the first string in buf is the given command */
static char *iscommand(char*buf, char *cmd)
{
	int l = strlen(cmd);
	if (!strncmp(buf,cmd,l) && (buf[l] == ' ' || !buf[l]))
		return buf + l + 1;
	return 0;
}
/*
 * read in a command at a prompt and process it, moving to the next
 * prompt as necessary */
static void ReadCommand(void)
{
	static char buf[256],*pos = buf;
	static char text[400],url[80];
	static DWORD theuin, last_sent_uin;
	if (M_fdnGetCH(buf,&pos)) {
		if (prompt_level == SEND_MSG_UIN || prompt_level == SEND_URL_UIN) {
			prompt_level++;
			last_sent_uin = theuin = Nick2UIN(buf);
		} else if (prompt_level == SEND_MSG) {
			if (!buf[0]) {
				icq_sendmsg(theuin,text,strlen(text),NORM_MESS);
				prompt_level = 0;
			}
			else {
				if (strlen(text) + strlen(buf) + 2< 400) {
					strcat(text,buf);
					strcat(text,"\n");
				}
			}
		} else if (prompt_level == SEND_URL) {
			CopyData(url,buf,80);
			prompt_level = SEND_URL_DESCRIPTION;
		} else if (prompt_level == SEND_URL_DESCRIPTION) {
			if (!buf[0]) {
				icq_sendurl(theuin,text,url);
				prompt_level = 0;
			}
			else {
				if (strlen(text) + strlen(buf) + 2< 400) {
					strcat(text,buf);
					strcat(text,"\n");
				}
			}
		} else if (prompt_level == ADDUSER) {
			icq_Add_User(Nick2UIN(buf));
			prompt_level = 0;
		} else if (prompt_level == GET_UIN_INFO) {
			if (buf) {
				icq_send_info_req(Nick2UIN(buf));
				prompt_level = 0;
			} else
				M_print("Please enter uin\n");
		} else if (prompt_level == RANDGROUP) {
			icq_rand_set(atoi(buf));
			prompt_level = 0;
		} else if (prompt_level == RANDSEARCH) {
			icq_rand_user_req(atoi(buf));
			prompt_level = 0;
		} else if (prompt_level >= SEARCH_USER_INFO && prompt_level < SEARCH_USER_INFO + 100) {
			SearchInfoPrompt(buf);
		} else if (prompt_level == ACCEPT_MSG) {
			icq_send_auth_msg(Nick2UIN(buf));
			prompt_level = 0;
		} else if (prompt_level == REJECT_MSG) {
			icq_send_reject_auth_msg(Nick2UIN(buf));
			prompt_level = 0;
		} else if (prompt_level == AUTH_REASON) {
			if (!buf[0]) {
				icq_Ask_For_Authorize(authuin,authreason);
				M_print("%s has been notified you wish to add him\n", UIN2Nick(authuin));
				prompt_level = 0;
			}
			else {
				if (strlen(authreason) + strlen(buf) + 2 < 400) {
					strcat(authreason,"\n");
					strcat(authreason,buf);
				}
			}
		} else if (prompt_level == STATUS_GET) {
			DWORD val = atoi(buf);
			if (!val)
				M_print("Status not changed\n");
			else {
				M_print("Status changed to %d\n",val);
				icq_status_change(val);
			}
			prompt_level = 0;
		} else if (prompt_level)
			InfoPrompt(buf);
		else if (iscommand(buf,"randgroup")) {
			GroupList();
			prompt_level = RANDGROUP;
		} else if (iscommand(buf,"randsearch")) {
			GroupList();
			prompt_level = RANDSEARCH;
		}
		else if (iscommand(buf,"adduser"))
			prompt_level = ADDUSER;
		else if (iscommand(buf,"accept"))
			prompt_level = ACCEPT_MSG;
		else if (iscommand(buf,"reject"))
			prompt_level = REJECT_MSG;
		else if (iscommand(buf,"getinfo"))
			prompt_level = GET_UIN_INFO;
		else if (iscommand(buf,"quit") || iscommand(buf,"q")) 
			done = true;
		else if (iscommand(buf,"help"))
			helpscreen();
		else if (iscommand(buf,"w"))
			ShowUsers();
		else if (iscommand(buf,"update"))
			prompt_level = GET_USER_INFO;
		else if (iscommand(buf,"search"))
			prompt_level = SEARCH_USER_INFO;
		else if (iscommand(buf,"send")) {
			prompt_level = SEND_MSG_UIN;
			text[0] = 0;
		} else if (iscommand(buf,"sendlast")) {
			prompt_level = SEND_MSG;
			theuin = last_sent_uin;
			text[0] = 0;
		} else if (iscommand(buf,"reply")) {
			prompt_level = SEND_MSG;
			theuin = last_received_uin;
			text[0] = 0;
		} else if (iscommand(buf,"sendurl")) {
			prompt_level = SEND_URL_UIN;
			text[0] = 0;
		} else if (iscommand(buf,"status")) {
			prompt_level = STATUS_GET;
			StatusList();
		} else M_print("Unknown command\n");
		*buf = 0;
	}
}
static void BeepIt(void)
{
	if (beep)
		M_print("\a");
}
/* time stamp a message */
static void timestamp(time_t tmx)
{
	M_print("at time: %s\n",asctime(gmtime(&tmx)));
}
/*
 * main callback from ICQ core catches what is going on
 */
static void callback(int msg, DWORD intval, void *parm1, void *parm2, time_t tim)
{
	static BOOL login_done;
	CONTACT_INFO *contact = (CONTACT_INFO *)parm1;
	CONNECT_INFO *connect = (CONNECT_INFO *) parm1;
	USER_INFO *user = (USER_INFO *)parm2;
	FILE *fil;
	switch(msg) {
		case IM_LOGIN:
			/* login command has completed, rest of login sequence has started */
			M_print("Logged in\n");
			login_done = FALSE;
			break;
		case IM_BADPASS:
			/* tried to log in with a bad password */
			M_print("Server responded: bad password\n");
			done = 1;
			break;
		case IM_ACCOUNTBUSY:
			/* account is in use */
			if (time(0) > 300 + login_time) {
				M_print("Account is busy, giving up");
				done = 1;
			}
			else {
				account_busy = 1;
				M_print("Account is busy, trying again");
			}
			break;
		case IM_NEWUIN:
			/* we asked to be a new user and got the uin */
			fil = fopen("users.dat","a+");
			fprintf(fil,"%ld",intval);
			M_print("New UIN is %ld\n",intval);
			our_user.uin = intval;
			account_busy = TRUE;
			retry_time = time(0) - 15;
			break;
		case IM_LOGIN2:
			/* Login sequence has completed */
			login_done = TRUE;
			ShowUsers();
			break;
		case IM_USEROFFLINE:
			/* a user has gone offline */
			if (Online(contact) && login_done)
				PrintBar("%s has gone offline",contact->nick);
			break;
		case IM_USERONLINE:
			/* a user has come online */
			if (Online(contact) && login_done)
				PrintBar("%s has come online",contact->nick);
			break;
		case IM_STATUSUPDATE:
			/* a user has changed status */
			if (Online(contact))
				PrintBar("%s has changed status to %s",contact->nick,GetStatusString(contact));
			break;
		case IM_NOTIFYADDED:
			/* you are adding him without authorization*/
			M_print("%s has been notified that you added him\n",contact->nick);
			break;
		case IM_ASKFORAUTH:
			/* you are requesting authorization to add him to your list */
			prompt_level = AUTH_REASON;
			authreason[0] = 0;
			authuin = contact->ci.uin;
			M_print("Enter reason you wish to add %s\n",contact->nick);
			break;
		case IM_AUTHREQ + 1:
			timestamp(tim);
		case IM_AUTHREQ:
			/* you have been asked if he can add you to the list */
			BeepIt();
			M_print("%s has asked you for permission to add you to his contact list\n",contact->nick);
			break;
		case IM_UPDATEDONE:
			/* we have finished updating our info */
			if (intval == (IR_EXT | IR_INFO | IR_AUTH))
				M_print("User information updated successfully\n");
			else if (intval == 0)
				M_print("No user information updated\n");
			else M_print("User information only partially updated\n");
			break;
		case IM_INFORESPONSE:
			/* We have gotten an information response about the user */
			contact = FindContact(user->uin);
			if (contact && (contact->flags & FL_WAITING_FOR_ACK))
				break;
			if (intval != (UI_BASIC | UI_EXTENDED)) {
				M_print("No information available for user %ld\n",user->uin);
				break;
			}
			if (user->uin == randresponseuin) {
				PrintBar("Information for random uin %ld",user->uin);
				randresponseuin = 0;
			}
			else
				PrintBar("Information for uin %ld",user->uin);
			M_print("Nickname: %s\n",user->nick);
			M_print("Name    : %s %s\n",user->first, user->last);
			M_print("City    : %s\n",user->city);
			M_print("State   : %s\n",user->state);
			M_print("Country : %s\n",Get_Country_Name(user->country));
			if (user->age == 0xffff)
				M_print("Age     : Not specified\n");
			else
				M_print("Age     : %d\n",user->age);
			if (user->sex == 2)
				M_print("Sex     : Male\n");
			else if (user->sex == 1)
				M_print("Sex     : Female\n");
			else
				M_print("Sex     : Not specified\n");
			M_print("Email   : %s\n",user->email);
			M_print("Phone   : %s\n",user->phone);
			M_print("Homepage: %s\n",user->homepage);
			M_print("Timezone: %d\n",user->timezone/2);
			if (user->auth)
				M_print("User needs to authorize you to add him to contact list\n");
			else
				M_print("You may add user to contact list without authorization\n");
			M_print("About   :\n");
			M_print(user->about);
			break;
		case IM_RANDRESPONSE:
			/* you have found a random user */
			if (connect) {
				randresponseuin = connect->uin;
				icq_send_info_req(connect->uin);
				M_print("Random user been found with uin %ld\n",connect->uin);
				M_print("Getting user info\n");
			}
			else
				M_print("no random users available");
			break;
		case IM_USERFOUND:
			/* search has returned a result record */
			PrintBar("Information for uin %ld",user->uin);
			M_print("Nickname: %s\n",user->nick);
			M_print("Name    : %s %s\n",user->first, user->last);
			M_print("Email   : %s\n",user->email);
			if (user->auth)
				M_print("User needs to authorize you to add him to contact list\n");
			else
				M_print("You may add user to contact list without authorization\n");
			break;
		case IM_SEARCHDONE:                        	
			/* search has finished processing */
			M_print("Search is complete, no more matches found\n");
			break;
		case IM_USERADDED+1:
			timestamp(intval);
		case IM_USERADDED:
			/* he added you to his contact list */
			BeepIt(); 
			M_print("User %s added you to their contact list\n",UIN2Nick(user->uin));
			break;
		case IM_AUTHDENIED+1:
			timestamp(tim);
		case IM_AUTHDENIED:
			/* he won't let you add him to your list */
			BeepIt();
			M_print("User %s denied you adding them to your contact list\n", UIN2Nick(contact->ci.uin));
			break;
		case IM_AUTHACCEPTED+1:
			timestamp(tim);
		case IM_AUTHACCEPTED:
			/* he will let you add him to your list */
			BeepIt();
			M_print("User %s authorized you to add them to your contact list\n", UIN2Nick( contact->ci.uin));
			break;
		case IM_URLRECEIVED+1:
			timestamp(tim);
		case IM_URLRECEIVED:
			/* you have received a URL */
			BeepIt();
			last_received_uin = intval;
			M_print("User %s has sent you a url\n",UIN2Nick(intval));
			M_print("URL: %s\n",parm1);
			M_print("Description: %s\n",parm2);
			break;
		case IM_INSTANTMESS+1:
			timestamp(tim);
		case IM_INSTANTMESS:
			/* you have received a text message */
			BeepIt();
			last_received_uin = intval;
			M_print("User %s has sent you a message\n",UIN2Nick(intval));
			M_print("%s\n",parm1);
			break;
	}
}
int main()
{
  longword ip;
	longword host;
	BOOL newuser = FALSE;
	verbose = 0;

	/* Open up WATTCP socket for connection */
	sock_init();
	host = resolve(hostname);
	if (!udp_open(&s.udp,0,host,4000,NULL)) {
		M_print("Can't connect to ICQ server");
		exit(1);
	}
	ip = gethostid();

	/* register our basic info with the ICQ core */
	icq_init(&s,callback,0,ip);

	/* read the profile and initialize a new user if none exists */
	if (!ReadProfile()) {
		char buf[256];
		M_print("Please enter UIN (0 for new UIN): ");
		fgets(buf,256,stdin);
		our_user.uin = Nick2UIN(buf);
		M_print("Please enter password: ");
		fgets(buf,256,stdin);
		buf[strlen(buf)-1] = 0;
		memcpy(password,buf,8);
		password[9] = 0;
		Current_Status = STATUS_ONLINE;
		beep = 1;
		Contacts[0].flags = FL_VALID;
		Contacts[0].ci.uin = 11290140L;
		strcpy(Contacts[0].nick,"MICQ author");
		Contacts[1].flags = FL_VALID;
		Contacts[1].ci.uin = 8699592L;
		strcpy(Contacts[1].nick,"LSICQ author");
		if (our_user.uin == 0) {
			newuser = TRUE;
			prompt_level = GET_USER_INFO;
			M_print("Please enter info about yourself\n");
		}
	}

	/* auto login or register new user */
	if (!newuser)
		Login(our_user.uin,password,ip,0, Current_Status);
	else
		reg_new_user(password);
	login_time = retry_time = time(0);
	while (!done) {
		/*WATTCP cooperative multitasking */
    if (!tcp_tick(&s)) {
      M_print("\nHost closed-aborting\n");
			done = TRUE;
      return 1;
    }
		/* icq core cooperative multitasking */
		icq_tick();
		/* command line cooperative multitasking */
		ReadCommand();

		/* if account was busy (or we registered a new user)
     * try logging in again after a while
     */
		if (account_busy && time(0) > 10 + retry_time) {
			retry_time = time(0);
			account_busy = FALSE;
			Login(our_user.uin,password,ip,0,Current_Status);
		}		
	}
	/* close ICQ and WATTCP socket */
	icq_rundown(&s);
	sock_close(&s);

	/* write out the profile */
	WriteProfile();
	return 0;
}
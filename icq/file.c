/*
 * FILE.C
 *
 * handle the profile functionality
 *
 * David Lindauer, 1999
 */
#include "lsicq.h"

extern DWORD Current_Status;
extern USER_INFO our_user;
extern CONTACT_INFO Contacts[100];
extern int beep;
extern char password[9];
void WriteProfileString(FILE *fil, char *key, char *val)
{
	fprintf(fil,"%s=%s\n",key,val);
}
void WriteProfileDW(FILE *fil, char *key, DWORD val)
{
	char buf[40];
	sprintf(buf,"%ld",val);
	WriteProfileString(fil,key,buf);
}
int ReadProfileString(FILE *fil, char *key, char *val, int len)
{
	int xlen = strlen(key);
	fseek(fil,0,SEEK_SET);
	while (!feof(fil)) {
		char buf[256];
		int l;
		fgets(buf,256,fil);
		if (buf[l=strlen(buf)-1] == '\n')
			buf[l] = 0;
		if (!strncmp(key,buf,xlen)) {
			while (buf[xlen] && buf[xlen] == ' ')
				xlen++;
			if (buf[xlen] == '=') {
				memcpy(val,buf+xlen+1,strlen(buf+xlen+1)+1);
				buf[len-1] = 0;
				return 1;
			}
		}
	}
	return 0;
}
DWORD ReadProfileDW(FILE *fil, char *key, DWORD def)
{
	char buf[256];
	if (!ReadProfileString(fil,key,buf,256))
		return def;
	return atoi(buf);
}
void WriteProfile(void)
{
	int i,j=0;
	FILE *fil = fopen("lsicq.dat","w");
	WriteProfileDW(fil,"UIN",our_user.uin);
	WriteProfileString(fil,"Password",password);
	WriteProfileDW(fil,"NeedAuthorize",our_user.auth);
	WriteProfileDW(fil,"Beep",beep);
	WriteProfileDW(fil,"Status",Current_Status);
	WriteProfileString(fil,"Nickname",our_user.nick);
	WriteProfileString(fil,"Firstname",our_user.first);
	WriteProfileString(fil,"Lastname",our_user.last);
	WriteProfileString(fil,"Email",our_user.email);
	WriteProfileString(fil,"City", our_user.city);
	WriteProfileString(fil,"State",our_user.state);
	WriteProfileDW(fil,"Country",our_user.country);
	WriteProfileDW(fil,"Age",our_user.age);
	WriteProfileDW(fil,"Sex",our_user.sex);
	WriteProfileString(fil,"Phone",our_user.phone);
	WriteProfileString(fil,"Homepage",our_user.homepage);
	WriteProfileString(fil,"About",our_user.about);
	WriteProfileDW(fil,"Timezone",our_user.timezone);
	for (i=0; i < 100; i++) {                             
		if ((Contacts[i].flags & FL_VALID) && !(Contacts[i].flags & FL_NOT_IN_LIST)) {
			char key[100],buf[100],*p = buf;
			if (Contacts[i].flags & FL_INVISIBLE)
				*p++ = '~';
			if (Contacts[i].flags & FL_VISIBLE)
				*p++ = '!';
			if (Contacts[i].flags & FL_WAITING_FOR_ACK)
				*p++ = '&';
			if (Contacts[i].flags & FL_WAITING_FOR_AUTH)
				*p++ = '|';
			if (Contacts[i].flags & FL_AUTH_IMPERM)
				*p++ = '^';
			sprintf(p,"%ld:%s",Contacts[i].ci.uin,Contacts[i].nick);
			sprintf(key,"Contact%03d",j++);
			WriteProfileString(fil,key,buf);
			
		}
	}
	fclose(fil);
}
int ReadProfile(void)
{
	FILE *fil = fopen("lsicq.dat","r");
	int i;
	if (!fil)
		return 0;
	memset(&our_user,0,sizeof(our_user));
	our_user.uin = ReadProfileDW(fil,"UIN", 0);
	ReadProfileString(fil,"Password",password,9);
	our_user.auth = ReadProfileDW(fil,"NeedAuthorize",1);
	beep = ReadProfileDW(fil,"Beep",1);
	Current_Status = ReadProfileDW(fil,"Status",STATUS_ONLINE);
	ReadProfileString(fil,"Nickname",our_user.nick,20);
	ReadProfileString(fil,"Firstname",our_user.first,20);
	ReadProfileString(fil,"Lastname",our_user.last,40);
	ReadProfileString(fil,"Email",our_user.email,80);
	ReadProfileString(fil,"City", our_user.city,20);
	ReadProfileString(fil,"State",our_user.state,6);
	our_user.country = ReadProfileDW(fil,"Country",0xffff);
	our_user.age = ReadProfileDW(fil,"Age",0xffff);
	our_user.sex = ReadProfileDW(fil,"Sex",0);
	ReadProfileString(fil,"Phone",our_user.phone,20);
	ReadProfileString(fil,"Homepage",our_user.homepage,80);
	ReadProfileString(fil,"About",our_user.about,400);
	our_user.timezone = ReadProfileDW(fil,"Timezone",0);
	for (i=0; i < 100; i++) {       
		char buf[100],key[100];
  	char *p = buf;
			sprintf(key,"Contact%03d",i);
		if (ReadProfileString(fil,key,buf,100)) { 
			if (*p == '~') {
				p++;
				Contacts[i].flags += FL_INVISIBLE;
			}
			if (*p == '!') {
				p++;
				Contacts[i].flags += FL_VISIBLE;
			}
			if (*p == '&') {
				p++;
				Contacts[i].flags += FL_WAITING_FOR_ACK;
			}
			if (*p == '|') {
				p++;
				Contacts[i].flags += FL_WAITING_FOR_AUTH;
			}
			if (*p == '^') {
				p++;
				Contacts[i].flags += FL_AUTH_IMPERM;
			}
			Contacts[i].ci.status = STATUS_OFFLINE;
			Contacts[i].ci.uin = atoi(p);
			Contacts[i].flags |= FL_VALID;
			p = strchr(p,':');
			if (p) {
				memcpy(Contacts[i].nick,p+1,20);
				Contacts[i].nick[19] = 0;
			}
		}
	}
	fclose(fil);

	return 1;
}
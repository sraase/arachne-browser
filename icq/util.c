/*
 * UTIL.C
 *
 * Utilitie subroutines
 *
 * David Lindauer, 1999
 */
#include "lsicq.h"

extern CONTACT_INFO Contacts[100];

/* a few routines to handle endianness */
void DW_2_Chars(unsigned char *buf,DWORD val)
{
	buf[0] = val & 0xff;
	buf[1] = (val >> 8) & 0xff;
	buf[2] = (val >> 16) & 0xff;
	buf[3] = (val >> 24) & 0xff;
}
void Word_2_Chars(unsigned char *buf, DWORD val)
{
	buf[0] = val & 0xff;
	buf[1] = (val >> 8 ) & 0xff;
}
DWORD Chars_2_DW(unsigned char *buf)
{
	DWORD val;
	val = buf[0];
	val += ((DWORD)buf[1]) << 8;
	val += ((DWORD)buf[2]) << 16;
	val += ((DWORD)buf[3]) << 24;
	return val;
}
unsigned Chars_2_Word(unsigned char *buf)
{ 	
	WORD val;
	val = buf[0];
	val += buf[1] << 8;
	return val;
}
/* Given a UIN, find a contact in the contact list */
CONTACT_INFO *FindContact(DWORD uin)
{
	int i;
	for (i=0; i < 100; i++) {
		if (!(Contacts [ i ].flags & FL_VALID))
			continue;
		if (uin == Contacts[ i ].ci.uin)
			return &Contacts[ i ];
	}
	return 0;
}
/* find an unused position in the contact list */
CONTACT_INFO *FindEmptyContact(void)
{
	int i;
	for (i=0; i < 100; i++) {
		if (!(Contacts [ i ].flags & FL_VALID))
			return &Contacts[ i ];
	}
	return 0;
}
/*
 * convert our internal user status flag to a network value */
DWORD GetStatusFlags(DWORD status)
{
	switch(status) {
    case STATUS_ONLINE:
			return 0;
    case STATUS_INVISIBLE:
			return STF_INVISIBLE;
    case STATUS_OCCUPIED:
			return STF_OCCUPIED;
    case STATUS_NA:
			return STF_NA;
    case STATUS_FREE_CHAT:
			return STF_FREE_CHAT;
    case STATUS_AWAY:
			return STF_AWAY;
    case STATUS_DND:
			return STF_DND;
		default:
			return 0;
	}
}
/*
 * convert a networked value to an internal status
 */
DWORD GetStatusVal(DWORD status)
{
	if (status & 2)
		return STATUS_DND;
	else if (status & 0x10)
		return STATUS_OCCUPIED;
	else if (status & 0x04)
		return STATUS_NA;
	else if (status & 1)
		return STATUS_AWAY;
	else if (status & 0x100)
		return STATUS_INVISIBLE;
	else if (status & 0x20)
		return STATUS_FREE_CHAT;
	else
		return STATUS_ONLINE;
}
typedef struct 
{
   const char *name;
   WORD code;
} COUNTRY_CODE;

static COUNTRY_CODE Country_Codes[] = { {"USA",1 },
                                 {"Afghanistan", 93 },
				 {"Albania", 355 },
                                 {"Algeria", 213 },
                                 {"American Samoa", 684 },
				 {"Andorra", 376 },
				 {"Angola", 244 },
				 {"Anguilla", 101 },
				 {"Antigua", 102 },
                                 {"Argentina", 54 },
				 {"Armenia", 374 },
                                 {"Aruba", 297 },
				 {"Ascention Island", 274 },
                                 {"Australia", 61 },
                                 {"Australian Antartic Territory", 6721 },
                                 {"Austria", 43 },
                                 {"Azerbaijan", 934 },
                                 {"Bahamas", 103 },
                                 {"Bahrain", 973 },
                                 {"Bangladesh", 880 },
                                 {"Barbados", 104 },
                                 {"Belarus", 375 },
                                 {"Belgium", 32 },
                                 {"Belize", 501 },
                                 {"Benin", 229 },
                                 {"Bermuda", 105 },
                                 {"Bhutan", 975 },
                                 {"Bolivia", 591 },
                                 {"Bosnia & Herzegovina", 387 },
                                 {"Botswana", 267 },
                                 {"Brazil",55 },
				 {"British Virgin Islands", 106 },
				 {"Brunei", 673 },
				 {"Bulgaria", 359 },
				 {"Burkina Faso", 226 },
				 {"Burundi", 257 },
				 {"Cambodia", 855 },
                                 {"Cameroon", 237 },
                                 {"Canada",107 },
				 {"Cape Verde Islands", 238 },
				 {"Cayman Islands", 108},
                                 {"Chile", 56 },
                                 {"China", 86 },
                                 {"Columbia", 57 },
                                 {"Costa Rice", 506 },
                                 {"Croatia", 385 }, /* Observerd */
				 { "Cuba", 53 },
                                 {"Cyprus", 357 },
                                 {"Czech Republic", 42 },
                                 {"Denmark",45 },
                                 {"Ecuador", 593 },
                                 {"Egypt", 20 },
                                 {"El Salvador", 503 },
                                 {"Ethiopia", 251 },
				 {"Federated States of Micronesia", 691 },
                                 {"Fiji", 679 },
                                 {"Finland", 358 },
                                 {"France", 33 },
                                 {"French Antilles", 596 },
                                 {"French Polynesia", 689 },
                                 {"Gabon", 241 },
                                 {"German", 49 },
                                 {"Ghana", 233 },
                                 {"Greece", 30 },
                                 {"Guadeloupe", 590 },
                                 {"Guam", 671 },
                                 {"Guantanomo Bay", 5399 },
                                 {"Guatemala", 502 },
                                 {"Guyana", 592 },
                                 {"Haiti", 509 },
                                 {"Honduras", 504 },
                                 {"Hong Kong", 852 },
                                 {"Hungary", 36 },
                                 {"Iceland", 354 },
                                 {"India", 91 },
                                 {"Indonesia", 62 },
                                 {"Iran", 98 },
                                 {"Iraq", 964 },
                                 {"Ireland", 353 },
                                 {"Israel", 972 },
                                 {"Italy", 39 },
                                 {"Ivory Coast", 225 },
                                 {"Japan", 81 },
                                 {"Jordan", 962 },
                                 {"Kenya", 254 },
                                 {"South Korea", 82 },
                                 {"Kuwait", 965 },
                                 {"Liberia", 231 },
                                 {"Libya", 218 },
                                 {"Liechtenstein", 41 },
                                 {"Luxembourg", 352 },
                                 {"Malawi", 265 },
                                 {"Malaysia", 60 },
                                 {"Mali", 223 },
                                 {"Malta", 356 },
                                 {"Mexico", 52 },
                                 {"Monaco", 33 },
                                 {"Morocco", 212 },
                                 {"Namibia", 264 },
                                 {"Nepal", 977 },
                                 {"Netherlands", 31 },
                                 {"Netherlands Antilles", 599 },
                                 {"New Caledonia", 687 },
                                 {"New Zealand", 64 },
                                 {"Nicaragua", 505 },
                                 {"Nigeria", 234 },
                                 {"Norway",47 }, 
                                 {"Oman", 968 },
                                 {"Pakistan", 92 },
                                 {"Panama", 507 },
                                 {"Papua New Guinea", 675 },
                                 {"Paraguay", 595 },
                                 {"Peru", 51 },
                                 {"Philippines", 63 },
                                 {"Poland", 48 },
                                 {"Portugal", 351 },
                                 {"Qatar", 974 },
                                 {"Romania", 40 },
                                 {"Russia",7 },
                                 {"Saipan", 670 },
                                 {"San Marino", 39 },
                                 {"Saudia Arabia", 966 },
                                 {"Saipan", 670 },
                                 {"Senegal", 221},
                                 {"Singapore", 65 },
                                 {"Slovakia", 42 },
                                 {"South Africa", 27 },
                                 {"Spain", 34 },
                                 {"Sri Lanka", 94 },
                                 {"Suriname", 597 },
                                 {"Sweden",46 },
                                 {"Switzerland", 41 },
                                 {"Taiwan", 886 },
                                 {"Tanzania", 255 },
                                 {"Thailand", 66 },
                                 {"Tunisia", 216 },
                                 {"Turkey", 90 },
                                 {"United Arab Emirates", 971 },
                                 {"Uruguay", 598 },
                                 {"UK",0x2c },
				 {"Ukraine", 380 },
                                 {"Vatican City", 39 },
                                 {"Venezuela", 58 },
                                 {"Vietnam", 84 },
                                 {"Yemen", 967 },
                                 {"Yugoslavia", 38 },
                                 {"Zaire", 243 },
                                 {"Zimbabwe", 263 },
#ifdef FUNNY_MSGS
                                 {"Illegal alien",0 },
                                 {"Illegal alien",0xffff } };
#else
                                 {"Not entered",0 },
                                 {"Not entered",0xffff } };
#endif

/* conversions between country names and their codes */
const int Get_Country_Code( char *buf)
{
	int i;
	for (i=0; Country_Codes[i].code != 0xffff; i++)
		if (!stricmp(Country_Codes[i].name,buf))
			return Country_Codes[i].code;
	return 0xffff;
}
const char *Get_Country_Name( int code )
{
   int i;
   
   for ( i = 0; Country_Codes[i].code != 0xffff; i++)
   {
      if ( Country_Codes[i].code == code )
      {
         return Country_Codes[i].name;
      }
   }
   if ( Country_Codes[i].code == code )
   {
      return Country_Codes[i].name;
   }
   return NULL;
}
/* conversions between nicknames and UINs */
char *UIN2Nick(DWORD uin)
{	
	int i;
	static char buf[256];
	for (i=0; i < 100; i++)
		if ((Contacts[i].flags & FL_VALID) && uin == Contacts[i].ci.uin)
			return Contacts[i].nick;

	sprintf(buf,"%ld",uin);
	return buf;
}
DWORD Nick2UIN(char *nick)
{
	int i;
	for (i=0; i < 100; i++)
		if ((Contacts[i].flags & FL_VALID) && !stricmp(nick,Contacts[i].nick))
			return Contacts[i].ci.uin;

	return atoi(nick);
}

// ===============================================================================
// HTML/2.0 entities (defined in <fontpaht>/fontinfo.bin)
// ===============================================================================

#include "arachne.h"

unsigned char HTMLentity(char *name)
{
 if(*name=='#')
//!!glennmcc: begin Dec 29, 2004 -- 'fix' for punctuation in numeric code format
 if(!strcmpi(name,"#146") || !strcmpi(name,"#8217")) return atoi("39"); else // 39== '
 if(!strcmpi(name,"#8216")) return atoi("96"); else // 96== `
 if(!strcmpi(name,"#8220") || !strcmpi(name,"#8221")) return atoi("34"); else // 34== "
 if(!strcmpi(name,"#8211")) return atoi("45"); else // 45== -
 if(atoi(&name[1])>255) return atoi("127"); else // 127==
//!!glennmcc: end
  return atoi(&name[1]);

 if(!strcmpi(name,"lt"))
  return '<';
 else
 if(!strcmpi(name,"gt"))
  return '>';
 else
 if(!strcmpi(name,"amp"))
  return '&';
 else
 if(!strcmpi(name,"quot"))
  return '\"';
//!!glennmcc: Jan 04, 2005 -- 'bullet'
 else
 if(!strcmpi(name,"bul") || !strcmpi(name,"bull") || !strcmpi(name,"bullet"))
  return atoi("127");
//!!glennmcc: end
 else//------------------------- ISO Latin entities
 {
  int i=0,l=strlen(name);

  while(i<128)
  {
   if(!strncmp(finf->entity[i],name,l))
    return (unsigned char)128+i;
   i++;
  }
 }

 if(!strcmpi(name,"copy"))
  return 'c';
 else
 if(!strcmpi(name,"reg"))
  return 'r';
 else
 if(!strcmpi(name,"middot"))
  return 127;
 else
 if(!strcmpi(name,"sp"))
  return ' ';
 else
 if(!strcmpi(name,"nbsp"))
  return 160;
 else
  return name[0]; // "Aacute" -> 'A'

}

void entity2str(char *str)
{
 int i=0,j=0,l=strlen(str);
 char *ptr;

 while(i<l)
 {
  if(str[i]=='&' && !(i<l && str[i+1]==' '))
  {
   ptr=&str[++i];
   while(i<l && str[i]!=';') i++;
   str[i++]='\0';
   str[j]=HTMLentity(ptr);
   if((unsigned char)str[j]==160) //&nbsp;
    str[j]=' ';
  }
  else
   str[j]=str[i++];

  j++;
 }//loop
 str[j]='\0';
}


  /*
  if(!strcmpi(name,"reg"))
   return 174; //(R)
  else
  if(!strcmp(name,"aacute"))
   return ' ';
  else
  if(!strcmp(name,"Aacute"))
   return '';
  else
  if(!strcmp(name,"auml"))
   return 228;
  else
  if(!strcmp(name,"Auml"))
   return 196;
  else
  if(!strcmp(name,"Eacute"))
   return '';
  else
  if(!strcmp(name,"eacute"))
   return '‚';
  else
  if(!strcmp(name,"iacute"))
   return '¡';
  else
  if(!strcmp(name,"Iacute"))
   return '‹';
  else
  if(!strcmp(name,"ntilde"))
   return '¤';
  else
  if(!strcmp(name,"Ntilde"))
   return '¥';
  else
  if(!strcmp(name,"oacute"))
   return '¢';
  else
  if(!strcmp(name,"Oacute"))
   return '•';
  else
  if(!strcmp(name,"ouml"))
   return 246;
  else
  if(!strcmp(name,"Ouml"))
   return 214;
  else
  if(!strcmp(name,"szlig"))
   return 223;
  else
  if(!strcmp(name,"uuml"))
   return 252;
  else
  if(!strcmp(name,"Uuml"))
   return 220;
  else
  if(!strcmp(name,"uacute"))
   return '£';
  else
  if(!strcmp(name,"Uacute"))
   return '—';
  else
  if(!strcmp(name,"uacute"))
   return '£';
  else
  if(!strcmp(name,"Uacute"))
   return '—';
  else
  if(!strcmp(name,"yacute"))
   return '˜';
  else
  if(!strcmp(name,"Yacute"))
   return '';
  */

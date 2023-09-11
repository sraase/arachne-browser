
// ===============================================================================
// HTML/2.0 entities (defined in <fontpaht>/fontinfo.bin)
// ===============================================================================

#include "arachne.h"

unsigned char HTMLentity(char *name)
{
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
  return atoi("183");
//!!glennmcc: end

//!!glennmcc: MAY 30, 2005 -- 'trade' == small 'TM' Trade Mark symbol
 else
 if(!strcmpi(name,"trade"))
  return atoi("153");
//!!glennmcc: end

//!!glennmcc: May 27, 2007 -- next section no-longer needed
//!!glennmcc: May 27, 2007 -- read entity conversions from entity.cfg
//using this format
/*
[Entity conversions]
lsquo `
rsquo '
ldquo "
rdquo "
mdash -
ndash -
*/

 else
 if(configvariable(&ENTITYcfg,name,NULL))
 return *configvariable(&ENTITYcfg,name,NULL);
//!!glennmcc: end

/*
//commented-out in favor of entity.cfg method above
//!!glennmcc: May 26, 2007 -- lsquo & rsquo
 else
 if(!strcmpi(name,"lsquo"))// lsquo == left single quote
  return atoi("96"); // 96 == `
 else
 if(!strcmpi(name,"rsquo"))// rsquo == right single quote
  return atoi("39"); // 39 == '
 else
//ldquo == left double quote, rdquo == right double quote
 if(!strcmpi(name,"ldquo") || !strcmpi(name,"rdquo"))
  return atoi("34"); // 34 == "
//!!glennmcc: end
//!!glennmcc: May 27, 2007 -- mdash and ndash
 else
 if(!strcmpi(name,"mdash") || !strcmpi(name,"ndash"))
  return atoi("45"); // 45 == -
//!!glennmcc: end
*/
//end commenting

//!!glennmcc: May 29, 2007 -- moved numeric section to after named section
//!!glennmcc: Feb 05, 2007 -- interpret HEX, OCT and DEC format entities
// HEX &#x0027; == OCT &#\37; == DEC &#39;
// HEX &#x2013; == OCT &#\20023; == DEC &#8211;
// Now using a combination of methods from Joe and Ray
// -- Eureka !! we have perfection... (or darned close to it) ;-)
 if(*name=='#')
    {
     int value;
     char number[11];
     if(name[1]=='x' || name[1]=='X')
     value=(int)strtoul(&name[2],NULL,16);//HEX format
//!!glennmcc: Feb 07, 2007 -- OCTAL is not needed after-all,
//but I'll leave this here just in case it's needed some time in the future
//     else
//     if(name[1]=='\\')
//     value=(int)strtoul(&name[2],NULL,8);//OCT format
     else
     value=(int)strtoul(&name[1],NULL,10);//DEC format

//!!glennmcc: begin Dec 29, 2004 -- 'fix' for punctuation in numeric code format
//!!glennmcc: Mar 27, 2005 -- added 8222 == "
//!!glennmcc: May 26, 2007 -- added 145 == `
//!!glennmcc: May 27, 2007 -- added 8212 == -
//!!glennmcc: May 29, 2007 -- next section no-longer needed
//also read numeric entity conversions from entity.cfg
if(value>127
   && configvariable(&ENTITYcfg,itoa(value,number,10),NULL))
   return *configvariable(&ENTITYcfg,itoa(value,number,10),NULL);
/*
     if (value==146 || value==8217)
      return 39; // 39 == '
     else if (value==8216 || value==145)
      return 96; //96 == `
     else if (value==8220 || value==8221 || value==8222)
      return 34; // 34 == "
     else if (value==8211 || value==8212)
      return 45; // 45 == -
*/

//!!glennmcc: May 30, 2007 --
     else if (value==8194 || value==8195 || value==8201)
      return 32; // 32 == space
//!!glennmcc: end

//!!glennmcc: Mar 26, 2008 --
     else if (value==710)
      return 94; // 94 == caret
     else if (value==732)
      return 126; // 126 == tilde
//!!glennmcc: end

     else if(value>255)
      return 127; // 127 == 
     else
      return value;
    }
//!!glennmcc: end -- Dec 29, 2004

//!!glennmcc: Feb 06, 2007 -- commented-out entire original block
// in-favor of new method above for HEX, OCT and DEC format entities
// the original method supported only DEC format
/*
 if (*name=='#')
//!!glennmcc: begin Dec 29, 2004 -- 'fix' for punctuation in numeric code format
//!!glennmcc: Mar 27, 2005 -- added 8222 == "
  {
  if (!strcmpi(name,"#146") || !strcmpi(name,"#8217"))
   return atoi("39"); // 39 == '
  else if (!strcmpi(name,"#8216"))
   return atoi("96"); // 96 == `
  else if (!strcmpi(name,"#8220") || !strcmpi(name,"#8221") || !strcmpi(name,"#8222"))
   return atoi("34"); // 34 == "
  else if (!strcmpi(name,"#8211"))
   return atoi("45"); // 45 == -
  else if (atoi(&name[1])>255)
   return atoi("127"); // 127 == 
  else
//!!glennmcc: end
   return atoi(&name[1]);
  }
*/

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

 if (!strcmpi(name,"copy"))
  return 'c';
 else if (!strcmpi(name,"reg"))
  return 'r';
 else if (!strcmpi(name,"middot"))
//!!glennmcc: Jun 12, 2005
  return 183;
//return 127;
//!!glennmcc: end
//!!glennmcc: May 29, 2007 -- added 'ensp, emsp and thinsp'
 else
 if(!strcmpi(name,"sp") || !strcmpi(name,"ensp")
    || !strcmpi(name,"emsp") || !strcmpi(name,"thinsp"))
  return ' ';
 else if (!strcmpi(name,"nbsp"))
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
//!!glennmcc: Mar 06, 2005 -- fix '&' and '&amp;' in URL bug
//http://www.cisnet.com/glennmcc/testing_&_symbol/&-amp-bug.htm
  if (str[i]=='&' && !(i<l && str[i+1]==' ') &&
      (str[i+2]==';' || str[i+3]==';' || str[i+4]==';' ||
       str[i+5]==';' || str[i+6]==';'))
//  if (str[i]=='&' && !(i<l && str[i+1]==' ')) //original line
//!!glennmcc: end
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

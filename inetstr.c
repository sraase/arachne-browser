
// This function analyses standart RFC-formated argument in format

// content [;extension=value] [;extension=value]

// for example Set-Cookie: RFC-string

// Note: uses fast arguments library as output! (FASTARG.C)
// str is adjusted to contain only first part of the string.

#include "arachne.h"
#include "html.h"

void decompose_inetstr(char *str)
{
 char argflag=0,*ptr=NULL;

 argnamecount=0; //reset argument counter
 argvaluecount=0;

 while(*str)
 {
  if(*str==';' && argflag<2)
  {
   *str='\0';
   if(argflag==1)
   {
    putvarvalue(ptr,(int)(str-ptr));
   }
   argflag=2; //wating for arg name
   str++;
   while(*str==' ')str++; //skip spaces
   ptr=str;
  }
  else if(str[0]=='=' && argflag==2)
  {
   *str='\0';
   putvarname(ptr,(int)(str-ptr));
   ptr=++str;
   argflag=1;
  }
  else
   str++;
 }//loop

 if(argflag==1)
  putvarvalue(ptr,(int)(str-ptr));
}

/*

just for testing...

void main(void)
{
 char *str="blah=blah0; domain=blah1; path=blah2",*ptr;

 BUF=8000;
 argnamestr=farmalloc(2000);
 argvaluestr=farmalloc(2000);
 if(!argnamestr || !argvaluestr)
  exit(0);

 puts("--------");
 decompose_inetstr(str);
 puts(str);
 if(getvar("domain",&ptr))
  puts(ptr);
 if(getvar("path",&ptr))
  puts(ptr);
}
*/
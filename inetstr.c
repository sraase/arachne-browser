
// This function analyses standard RFC-formated argument in format
//     content [;extension=value] [;extension=value]
// for example    Set-Cookie: RFC-string

// Note: uses fast arguments library as output! (FASTARG.C)

// 'str' is adjusted to contain only the first part of the string,
// just before the first ";" (if any). The remainder of the string
// is decomposed into the 'argnamestr' and 'argvaluestr' arrays.


#include "arachne.h"
#include "html.h"

/**** !!JdS 2004/1/27: Obsoleted, see new version below ...
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
	  while(*str==' ')
	    str++; //skip spaces
	  ptr=str;
	}
      else
	if(str[0]=='=' && argflag==2)
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
****/

/* Rewritten using argvarcount, putargname(), putargvalue(). Additional
   comments. 'argflag' sequence changed to 0, 1, 2, 1, 2, 1, 2 ...
   Attributes without a value are now ignored.          [JdS 2004/1/27] */
void decompose_inetstr(char *str)
{
  char argflag=0,*ptr=NULL;

  argvarcount=0; //reset argument counter

  while(*str)
    {
      if(*str==';')
	{
	  *str='\0';
	  if(argflag==2)
	    {
	      putargvalue(ptr);
	    }
	  argflag=1; //expecting arg name next
	  str++;
	  while(*str==' ')
	    str++; //skip leading spaces
	  ptr=str; //mark start of arg name
	}
      else
	if(*str=='=' && argflag==1)
	  {
	    *str='\0';
	    putargname(ptr);
	    ptr=++str; //mark start of arg value
	    argflag=2; //expecting arg value next
	  }
	else
	  str++;
    }//loop

  if(argflag==2) //don't forget the last arg value
    putargvalue(ptr);
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

//========================================================================
// JavaScript for Arachne by xChaos & Homeless
//========================================================================

#include "arachne.h
#include "javascr.h

// parse javascript...

// js_parse_string(string, )

// parser
void promenna(char* string,char* out1, char* out2)
{
 char * pesek;
 pesek=string;
 while (pesek==' ')
  pesek++;
 while (pesek)
  {
  switch (*pesek)
   {
   case '=' :break;
   case '(' :break;
   case ';' :break;
   case '+' :break;
   case '-' :break;
   default  :pesek++;
   }
  }
  strncpy(out1,string,pesek-string-1);
  out1[pesek-string-1]='\0';


}
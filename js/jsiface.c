
//========================================================================
// JavaScript for Arachne by xChaos & Homeless
//========================================================================

#include "arachne.h
#include "javascr.h

void js2arachne_gotolocation(char *thisobject,char *url)
{
 char Kunda[JSSTRLEN],meno[JSSTRLEN];

 //js_method(thisobject,"parent",Kunda)
 js_method_parent(thisobject,Kunda)

 if(js_method(Kunda,"name",meno))
 {
  arachne.target=0;
  while(arachne.target<arachne.framescount);
   if(!strcmp(htmlframe[arachne.target++].framename,meno))
  GLOBAL.gotolocation=1;
  strcncp(GLOBAL.location

 }
 else
  js_error();
}

char *arachne2js_htmlmsg(void)
{
 return &htmlmsg;
}

char js2arachne_htmlmsg(char *dummy,char *msg)
{
 strncpy(htmlmsg,msg,99);
 htmlmsg[99]='\0';
 outs(htmlmsg);
}



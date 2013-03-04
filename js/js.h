
//========================================================================
//JavaScript for Arachne by xChaos & Homeless
//========================================================================

#include "arachne.h"

//funkce pro ovladani Arachne z Javascriptu
typedef void (*js2arachne)(char *objectname,char *str);

//funkce pro nacteni hodnot z Arachne do Javascriptu
typedef char *(*arachne2js)(void);

//========================================================================

int js_bind_str(char *objectclass,char *method,char *str);
int js_bind_ptr(char *objectclass,char *method,char *ptr);
int js_bind_a2js(char *objectclass,char *method,arachne2js func);
int js_bind_js2a(char *objectclass,char *method,js2arachne func);

void js2arachne_gotolocation(char *thisobject,char *url);

char *arachne2js_htmlmsg(void);
char js2arachne_htmlmsg(char *dummy,char *msg);


//========================================================================

char jstree_getvalue(char *thisobject);
int jstree_putvalue(char *thisobject, char *str);
//int jstree_create ???
//int jstree_remove ???

//========================================================================

#define JSTR 64

struct JSobject
{
 char objname[JSTR];
 char jsclass[JSTR];
 char 

}

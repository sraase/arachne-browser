
#include "arachne.h"

extern char graphics;

void outch( char ch )          /* print character to stdio */
{
#ifdef POSIX
 putchar(ch);
#else
 putch(ch);
#endif
}
extern char lasttime[32];

void outs( char far *s)        /* print a ASCIIZ string to stdio */
{
 if(fullscreen)
  return;
 if(graphics)
 {
  char str[105], *ptr;
  int l;

  htmlfont(1,0);
  l=x_charmax((unsigned char *)s,x_maxx()-200);
  if(l>100)
   l=100;
#ifndef AGB
#ifdef CUSTOMER
  Box3D(0,x_maxy()-15,x_maxx(),x_maxy());
#else
  Box3D(0,x_maxy()-15,x_maxx()-152,x_maxy());
#endif // CUSTOMER
#else
  Box3D(0,x_maxy()-15,x_maxx(),x_maxy());
#endif // AGB
  if(strlen(s)>l && l>3)
  {
   l-=3;
   makestr(str,s,l);
   strcat(str,"...");
   ptr=str;
  }
  else
   ptr=s;

  x_setcolor(0);
  x_text_ib(4,x_maxy()-15,(unsigned char *)ptr);

  //redraw time!
  lasttime[0]='\0';

 }
 else
  puts(s);
}

void outsn( char far *s,int n) /* print a string with len max n */
{
 char str[128];
 if(n>127)
  n=127;
 makestr(str,s,n);
 outs(str);
}

void outhex( char ch )
{
 printf("%x",ch);
}

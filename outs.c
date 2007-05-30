
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
#ifdef CAV
 if (GLOBAL.clipdel == CLIPBOARD_DEFER_ADD
     || GLOBAL.clipdel == CLIPBOARD_ADDHOT)
return;
#endif

 if(fullscreen)
  return;

#ifndef TEXTONLY
 {
  char str[105], *ptr;
  int l;

//!!glennmcc: Aug 22, 2005 -- maintain size independant of fontshift
 htmlfont(1-user_interface.fontshift,0);
//  htmlfont(1,0);
  l=x_charmax((unsigned char *)s,x_maxx()-200);
  if(l>100)
   l=100;


 x_setfill(0,7);
#ifndef AGB

#ifdef CUSTOMER
  x_setcolor(15);
  x_line(2,x_maxy()-14,x_maxx()-2,x_maxy()-14);
  x_setcolor(8);
  x_line(2,x_maxy()-1,x_maxx()-2,x_maxy()-1);
  x_bar(2,x_maxy()-13,x_maxx()-2,x_maxy()-2);
#else
  x_setcolor(15);
  x_line(2,x_maxy()-14,x_maxx()-154,x_maxy()-14);
  x_setcolor(8);
  x_line(2,x_maxy()-1,x_maxx()-154,x_maxy()-1);
  x_bar(2,x_maxy()-13,x_maxx()-154,x_maxy()-2);
#endif // CUSTOMER

#else
  x_setcolor(15);
  x_line(2,x_maxy()-14,x_maxx()-2,x_maxy()-14);
  x_setcolor(8);
  x_line(2,x_maxy()-1,x_maxx()-2,x_maxy()-1);
  x_bar(2,x_maxy()-13,x_maxx()-2,x_maxy()-2);
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
#else
  puts(s);
#endif
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

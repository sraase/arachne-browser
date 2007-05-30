
// ========================================================================
// GRAPHICAL USER INTERFACE for Arachne WWW browser - runtime functions
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "internet.h" //because of background tasks...
#include "gui.h"
#include "xanimgif.h"

#ifdef EXP//experimental
#include "html.h"//!!glennmcc: Feb 18, 2005 -- for 'Select test' in mouseon()
#endif//exp

int TcpIdleFunc(void)
{
   if(GLOBAL.abort || GUITICK())
    return -1;
   xChLogoTICK(GlobalLogoStyle); // animation of logo
   return 0;
}

//#ifndef ULTRALIGHT

void timestr(char *str)
{
#ifdef POSIX
  time_t t=time(NULL);
  struct tm *gt=gmtime(&t);

 sprintf(str,"%2d:%02d:%02d",
   gt->tm_hour, gt->tm_min, gt->tm_sec );
#else
 struct  time t;

 gettime(&t);
 sprintf(str,"%2d:%02d:%02d",
   t.ti_hour, t.ti_min, t.ti_sec );
#endif//posix
}

void draw_time_online(void)
{
 char str[10];
 long t=time(NULL)-ppplogtime;

 if(fullscreen)
  return;

// Box3D(x_maxx()-58,x_maxy()-15,x_maxx()-1,x_maxy());
 x_setfill(0,7); //light gray
 x_bar(x_maxx()-56,x_maxy()-13,x_maxx()-3,x_maxy()-2);
 sprintf(str,"%2d:%02d:%02d", (int)t/3600, (int)(t/60)%60 , (int)t%60 );
 x_settextjusty(2,2);
 x_setcolor(2); //red
 htmlfont(1,0);
 x_text_ib(x_maxx()-4,x_maxy()-15,(unsigned char *)str);
 x_settextjusty(0,2);        // always write text from upper left corner
}
//#endif //ULTRALIGHT

//animate logo:

void xChLogo(char n)
{
 char logo[10]="XCHLOGO_";
 int x=x_maxx()-150,y=0;

 if(fullscreen || customerscreen)
  return;

#ifndef TEXTONLY
 if(arachne.GUIstyle==STYLE_SMALL1 || arachne.GUIstyle==STYLE_SMALL2)
 {
  if(n=='0')
  {
   int y=3;
   if(arachne.GUIstyle==STYLE_SMALL2)
    y=4;
   if(!user_interface.iconsoff)
   DrawIconNow( "ALTICON2",x_maxx()-146,y );
   return;
  }
  sprintf(logo,"SMALOGO_");
  x=x_maxx()-19;
  if(arachne.GUIstyle==STYLE_SMALL1)
   y=5;
  else
   y=6;
  if(n>'4' && n<'A')n-=4;
 }

 logo[7]=n;
 DrawIconNow(  logo, x,y);

 if(ppplogtime && tcpip)
 {
  char cas[32];
  timestr(cas);
  if (strcmp(lasttime,cas) == 0 ) return;
  strcpy(lasttime,cas);
  draw_time_online();
 }
#endif//textonly

}//end sub

void xChLogoTICK(char Style) // animation of logo
{
#ifndef NOTCPIP
 Backgroundhttp();
#endif//notcpip

 if( fullscreen || customerscreen ||
    !user_interface.logoiddle || GLOBAL.backgroundimages>BACKGROUND_SLEEPING)
  return;

#ifndef TEXTONLY
#ifdef POSIX
 if(Style==10)
 {
  Style=1;
  ikniddle+=800;
 }
 else
#endif//posix
  ikniddle++;
 if(ikniddle<user_interface.logoiddle)return;

 ikniddle=0;
 ikn++;
 if(Style==1)
 {
  xChLogo('0'+ikn);
  if(ikn>=8)ikn=0;
 }
 else
 if(Style==2)
 {
  if(ikn==1)
   xChLogo('X');
  else if(ikn==2)
   xChLogo('Y');
  else if(ikn>=3)
  {
   xChLogo('Z');
   ikn=0;
  }
 }
 else
 {
  if(ikn==1)
   xChLogo('A');
  else
  {
   xChLogo('B');
   ikn=0;
  }
 }
#endif//textonly
}//end sub


void mouseon(void)
{
 if(!global_nomouse)
//!!glennmcc: Feb 15, 2005 -- fix pointer colors and 'stuck pointer'
 {
//!!glennmcc: added 'Select test' Feb 18, 2005
//calling onmouse(0) while in a <SELECT> tag causes a crash
#ifdef EXP//experimental
if(activeatom.data1!=SELECT && GLOBAL.clipdel != CLIPBOARD_ADDHOT)
  {
   if(onmouse(0))
   x_yncurs(1,mousex,mousey,user_interface.brightmouse);
   else
   x_yncurs(1,mousex,mousey,user_interface.darkmouse);
  }
  else
#endif//exp
//the following original single line will still be used if <SELECT>
  x_yncurs(1,mousex,mousey,15);
 }
//!!glennmcc: end
}

void mouseoff(void)
{
 if(!global_nomouse)
//!!glennmcc: Feb 15, 2005 -- fix pointer colors
  x_yncurs(0,mousex,mousey,user_interface.darkmouse);
//  x_yncurs(0,mousex,mousey,15);//original line
}


#ifndef TEXTONLY
//total information on memory...
char lastinfo=0;

void MemInfoLine(char *text1,char *text2,int color,int *y)
{
 x_setcolor(0);
 x_text_ib(x_maxx()-148,*y,(unsigned char *)text1);
 x_setcolor(color);
 x_settextjusty(2,2);
 x_text_ib(x_maxx()-4,*y,(unsigned char *)text2);
//!!glennmcc: Aug 22, 2005 -- prevent fontshift >1 from messing-up meminfo
#ifdef CAV
 *y+=fonty(0-user_interface.fontshift,NORMAL)-1;
#else
 *y+=fonty(0-user_interface.fontshift,NORMAL);
#endif//cav
// *y+=fonty(0,NORMAL);
 x_settextjusty(0,2);        //always write text for upper left corner
}
#endif//textonly

int endvtoolbar(void)
{
 if(x_maxy()<200)
  return 72;
 else
 if(x_maxy()>600 || x_maxx()<640 || arachne.GUIstyle)
  return x_maxy()-14*fonty(0,NORMAL)-24;
 else
  return 472;
}

void MemInfo(char forced)
{
#ifndef TEXTONLY
 char str[30];
 unsigned long ldsp;
 int y=endvtoolbar();
 int color;
 long value;

 if(forced && meminfovisible)
 {
  if(arachne.framescount)
  {
   p->activeframe=0;
   redrawHTML(REDRAW_NO_MESSAGE,REDRAW_CREATE_VIRTUAL);
  }
  else
   redrawHTML(REDRAW_WITH_MESSAGE,REDRAW_VIRTUAL);
  meminfovisible=0;
  return;
 }

 if((x_maxx()<640 || arachne.GUIstyle ||
     fullscreen || customerscreen || fixedfont) /*fixedfont=printing hack*/
  && !forced) return;

 if(arachne.GUIstyle)
  meminfovisible=1;

 mouseoff();
 y+=2;
 Box3D(x_maxx()-150,y,x_maxx()-1,x_maxy()-17);
 y+=1;
 htmlfont(0,NORMAL);

 if(forced && lastinfo && endvtoolbar()>=x_maxy()-14*(fonty(0,NORMAL)-1))
  goto moreinfo;

#ifndef POSIX
 //DOS memory
 value=farcoreleft();//>>10; //!!glennmcc Oct 09, 2004 don't round off to KB
 sprintf(str,"%4lu",value);
 if(value<50000l || memory_model==0)
 {
  color=2;
  strcat(str,"[!!!]");
 }
 else
 if(memory_model==2)
 {
  color=3;
  strcat(str,"[+]");
 }
 else
 {
  color=0;
  strcat(str,"[?]");
 }
 MemInfoLine("Dos mem",str,color,&y);//!!glennmcc Oct 09, 2004 don't round off to KB

 //XMS used
 sprintf(str,"%5lu",mem_all_xmem());
 MemInfoLine("Used XMS (KB)",str,0,&y);
#endif//posix

 //XSWAP free
 ldsp=ie_free()>>10;
 sprintf(str,"%4ld",ldsp);
 if(ldsp<500)
 {
  color=2;
  strcat(str,"[!!!]");
 }
 else
  color=0;
 MemInfoLine("Free xSwap (KB)",str,color,&y);

 //XSWAP used
 sprintf(str,"%4ld",ie_used()>>10);
 MemInfoLine("Used xSwap (KB)",str,0,&y);

#ifndef POSIX
 //disk space
//!!glennmcc: Nov 25, 2005 -- check space on the cache drive
//and the current drive .... display both when not the same drive
//when cache is on local drive, display only one
#ifndef NOKEY
 {
 ldsp=lastdiskspace(configvariable(&ARACHNEcfg,"CachePath",NULL))>>20;
//local & cache
/**/
 if(ldsp==(localdiskspace()>>20))
 sprintf(str,"%4lu",ldsp);
 else
 sprintf(str,"%4lu %4lu",localdiskspace()>>20,ldsp);
/**/
//temp & cache
/*
 if(ldsp==(lastdiskspace(getenv("TEMP"))>>20))
 sprintf(str,"cache_%4lu",ldsp);
 else
 sprintf(str,"temp_%4lu cache_%4lu",lastdiskspace(getenv("TEMP"))>>20,ldsp);
*/
 }
#else
//local only
 {
 ldsp=localdiskspace()>>20; //original line
 sprintf(str,"%4lu",ldsp);  //original line
 }
#endif//nokey
 if(ldsp<(user_interface.mindiskspace>>20)
    || lastdiskspace(getenv("TEMP"))<user_interface.mindiskspace)
 {
  color=2;
  strcat(str,"[!!!]");
 }
 else
  color=0;
MemInfoLine("Disk space (MB)",str,color,&y); //original line

#endif//posix

 //cache items
 sprintf(str,"%5d",HTTPcache.len);
 MemInfoLine("HTTP cache items",str,0,&y);

 //HTML atoms
 if(pagetime<1000000l && p->HTMLatomcounter>0l)
 {
  char atoms[30];
  sprintf(str,"%ld:%02d",pagetime/60,(int)(pagetime % 60));
  sprintf(atoms,"%ld HTML atoms",p->HTMLatomcounter);
  x_setcolor(8);
  x_line(x_maxx()-147,y,x_maxx()-4,y);
  y++;
  MemInfoLine(atoms,str,0,&y);
 }

 lastinfo=1;
 if(y<x_maxy()-7*(fonty(0,NORMAL)-1))
 {
  x_setcolor(8);
  x_line(x_maxx()-147,y,x_maxx()-4,y);
  y++;

  moreinfo:

  MemInfoLine("Profile",configvariable(&ARACHNEcfg,"Profile",NULL),0,&y);
  MemInfoLine("",configvariable(&ARACHNEcfg,"PersonalName",NULL),0,&y);
  MemInfoLine("",configvariable(&ARACHNEcfg,"eMail",NULL),0,&y);
  y++;
  x_setcolor(8);
  x_line(x_maxx()-147,y,x_maxx()-4,y);
  MemInfoLine("Charset",configvariable(&ARACHNEcfg,"AcceptCharset",NULL),0,&y);
  MemInfoLine("HTTP cookies",configvariable(&ARACHNEcfg,"Cookies",NULL),0,&y);
//!!Ray: Feb 09, 2007 -- remove keep pop3 to show DNS instead
//  MemInfoLine("Keep POP3 mail",configvariable(&ARACHNEcfg,"KeepOnServer",NULL),0,&y);
//!!Ray: end
  y++;
  x_setcolor(8);
  x_line(x_maxx()-147,y,x_maxx()-4,y);
//!!Ray: Feb 09, 2007
  sprintf(str,"%ld.%ld.%ld.%ld", *def_nameservers>>24, (*def_nameservers>>16)&0xFF,
	      (*def_nameservers>>8)&0xFF, *def_nameservers&0xFF);
  MemInfoLine("DNS",str,0,&y);
//!!Ray: end
//!!JdS 2005/08/16 {
//Fix the useless "0.0.0.0" IP address info. display ...
//  MemInfoLine("Local IP",myIPstr,0,&y); //original line
  sprintf(str,"%ld.%ld.%ld.%ld", my_ip_addr>>24, (my_ip_addr>>16)&0xFF,
	      (my_ip_addr>>8)&0xFF, my_ip_addr&0xFF);
  MemInfoLine("Local IP",str,0,&y);
//!!JdS 2005/08/16 }
  lastinfo=0;
 }

 mouseon();

#endif//textonly
}

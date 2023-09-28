
#include "arachne.h"

void draw_time_online(void);

long ScreenSaver=0l;
long SecondsSleeping=0l; //for screensaver
char lasttime[32];   //for screensaver


//Draw time
void clock_and_timer(char *wait) //kresleni casu a screensaver
{
#ifndef POSIX
 struct  time t;
#endif 
 char cas2[32];

 timestr(cas2);
 if (strcmp(lasttime,cas2) == 0 ) return;

 if((ScreenSaver>0l||lasttime[0]=='*') && SecondsSleeping>(long)ScreenSaver*60l)
 {
 /*
 if(1)
 {
  mouseoff();
//  strcpy(buf,"system\\scrnsvrs\\stin.exe");
//  closebat(buf,RESTART_REDRAW);
//  GLOBAL.willexecute=willexecute(buf);
//  GLOBAL.gotolocation=1;

  execl("system\\scrnsvrs\\stin.exe","system\\scrnsvrs\\stin.exe",NULL);
  graphicsinit(arachne.graphics); // XLOPIF SVGA GRAPHICS
 }
 else
 */
 {
  int j;
  int fullscr[4]={0,0,0,0};
  int x,y,px,py,done=0;

  fullscr[2]=x_maxx();
  fullscr[3]=x_maxy();
  {
#define MAX_CERFS 20
#define MAX_TRACK 100
   int xx[MAX_CERFS],yy[MAX_CERFS],xs[MAX_CERFS],ys[MAX_CERFS],col[MAX_CERFS];
   int x2[MAX_CERFS],y2[MAX_CERFS],xs2[MAX_CERFS],ys2[MAX_CERFS];
   int tracklen[MAX_CERFS];
   int trackcnt[MAX_CERFS];
   int trackbuf[MAX_CERFS];
   int xtr[MAX_CERFS][MAX_TRACK],ytr[MAX_CERFS][MAX_TRACK];
   int xt2[MAX_CERFS][MAX_TRACK],yt2[MAX_CERFS][MAX_TRACK];
   int pom;
   int cerfs=MAX_CERFS/2;
   char barva0[6]={1,3,9,11,7,8};
   char barva1[6]={1,2,3,6,10,14};
   char *barva;
   char *value;
#ifdef HICOLOR
   int dc=1,p=0;
#endif

   //----------------------------screensaver
   mouseoff();
   x_cleardev();
#ifdef LINUX

#define RND(X) (random()*X)

   srandom(time(NULL));
#else

#define RND(X) random(X)

   randomize();
#endif

   if (config_get_int("ScreenSaverColors", 0) == 1)
    barva=barva1;
   else
    barva=barva0;

   cerfs = config_get_int("ScreenSaverMess", MAX_CERFS/2);
   if (cerfs < 1)         cerfs = 1;
   if (cerfs > MAX_CERFS) cerfs = MAX_CERFS;

   value = config_get_str("ScreenSaverStyle", "R");

   j=0;
   while(j<cerfs) //deklarace car
   {
    xx[j]=100+RND(fullscr[2]-200);
    yy[j]=100+RND(fullscr[3]-200);

    if(*value=='C')
     x2[j]=10+RND(fullscr[3]/3);
    else
    {
     x2[j]=120+RND(fullscr[2]-240);
     y2[j]=120+RND(fullscr[3]-240);
    }

    if(*value=='R')
     {xs[j]=RND(2);if(xs[j]==0)xs[j]=-1;}
    else
     {xs[j]=RND(3);if(xs[j]==2)xs[j]=-1;}
    ys[j]=RND(2);if(ys[j]==0)ys[j]=-1;

    xs2[j]=RND(2);if(xs2[j]==0)xs2[j]=-1;
    if(*value=='R')
     {ys2[j]=RND(2);if(ys2[j]==0)ys2[j]=-1;}
    else
     {ys2[j]=RND(3);if(ys2[j]==2)ys2[j]=-1;}
    col[j]=barva[RND(6)];
    pom=RND(MAX_TRACK-10)+10;
    tracklen[j]=pom;
    trackcnt[j]=1; //odsud se bude cist
    trackbuf[j]=0; //sem se bude zapisovat
    while(pom>=0)
    {
     xtr[j][pom]=0;
     ytr[j][pom]=0;
     xt2[j][pom]=0;
     yt2[j][pom]=0;
     pom--;
    }
    j+=1;
   }//loop
   j=0;

   ImouseRead( &x, &y );
   ImouseWait();
   px=x;py=y;
   while(!done) //animace
   {
    if(wait!=NULL)
    {
#ifdef POSIX
     time_t t=time(NULL);
     struct tm *gt=gmtime(&t);
    
     sprintf(cas2,"%2d:%02d:%02d",
      gt->tm_hour, gt->tm_min, gt->tm_sec );
#else
     gettime(&t);
     sprintf(cas2,"%2d:%02d:%02d", t.ti_hour, t.ti_min, t.ti_sec );
#endif
     if(strstr(wait,cas2)!=NULL)break;
    }//endif

#ifndef LINUX
    if(g_PrtScr)
    {
     g_PrtScr = 0;
     PrintScreen2BMP(0);
     goto out;
    }
#endif
    
    //mazani stopy
    x_setcolor(0);
#ifndef LINUX    
    if(*value=='C')
     x_circle(xtr[j][trackcnt[j]],ytr[j][trackcnt[j]],xt2[j][trackcnt[j]]);
    else
#endif    
    if(*value=='R')
     x_rect(xtr[j][trackcnt[j]],ytr[j][trackcnt[j]],xt2[j][trackcnt[j]],yt2[j][trackcnt[j]]);
    else
     x_line(xtr[j][trackcnt[j]],ytr[j][trackcnt[j]],xt2[j][trackcnt[j]],yt2[j][trackcnt[j]]);
    if(++trackcnt[j]>tracklen[j])trackcnt[j]=0;
    xtr[j][trackbuf[j]]=xx[j];
    ytr[j][trackbuf[j]]=yy[j];
    xt2[j][trackbuf[j]]=x2[j];
    yt2[j][trackbuf[j]]=y2[j];
    if(++trackbuf[j]>tracklen[j])trackbuf[j]=0;

    //kresleni nove cary
    x_setcolor(col[j]);
#ifndef LINUX
    if(*value=='C')
     x_circle(xx[j],yy[j],x2[j]);
    else 
#endif
    if(*value=='R')
     x_rect(xx[j],yy[j],x2[j],y2[j]);
    else
     x_line(xx[j],yy[j],x2[j],y2[j]);
    xx[j]+=xs[j];
    yy[j]+=ys[j];
    x2[j]+=xs2[j];
    y2[j]+=ys2[j];
    if(xx[j]>=fullscr[2]||xx[j]<=0)xs[j]=-xs[j];
    if(yy[j]>=fullscr[3]||yy[j]<=0)ys[j]=-ys[j];
    if(x2[j]>=fullscr[2]||x2[j]<=0)xs2[j]=-xs2[j];
    if(y2[j]>=fullscr[3]||y2[j]<=0)ys2[j]=-ys2[j];
    j++;
    if(j==cerfs)
     j=0;

    if(*value=='C' || j%10==0)
    {
//!!RAY: Sep 30, 2006 -- CTRL key will now deactivate the screensaver
     if((bioskey(1) || bioskey(2) &4) || ImouseRead( &x, &y ) || x!=px || y!=py)
//     if(bioskey(1) || ImouseRead( &x, &y ) || x!=px || y!=py)
      done=1;
    }
    px=x;py=y;

#ifdef HICOLOR
   if(xg_256 == MM_Hic)
   {
    if(p>20*cerfs)
    {
     dc=-dc;
     p=0;
    }
    p++;
    if(p%cerfs==0)
    {
     int c=1;
     while(c<16)
     {
      if(c%2)
       dc=-dc;
      Iipal[c*3]+=dc;
      if(Iipal[c*3]>63)
       Iipal[c*3]=63;
      if(Iipal[c*3]<0)
       Iipal[c*3]=0;
      Iipal[c*3+1]+=dc;
      if(Iipal[c*3+1]>63)
       Iipal[c*3+1]=63;
      if(Iipal[c*3+1]<0)
       Iipal[c*3+1]=0;
      Iipal[c*3+2]+=dc;
      if(Iipal[c*3+2]>63)
       Iipal[c*3+2]=63;
      if(Iipal[c*3+2]<0)
       Iipal[c*3+2]=0;
      c++;
     }//loop
    }
    x_palett( 16, Iipal);
   }
#endif

   }//loop
   if(bioskey(1))bioskey(0);
   ImouseWait();
   out:
#ifdef HICOLOR
   if(xg_256 == MM_Hic)
    initpalette();
#endif
   x_cleardev();
   RedrawALL();
   DrawTitle(1);
   if(lasttime[1]=='*')
    redraw=4;
   else
    redraw=3;
  }
 }
  SecondsSleeping=0l;
 }//endif screensaver

 if(!fullscreen)
 {
  x_setfill(0,7); //sediva
  if(mousey>x_maxy()-30 && mousex>x_maxx()-230)
   mouseoff();

  x_bar(x_maxx()-206,x_maxy()-13,x_maxx()-156,x_maxy()-2);
  x_setcolor(0); //cerna
//!!glennmcc: Aug 22, 2005
//prevent fontshift >0 from causing the clock to go 'off the right'
  htmlfont(0-user_interface.fontshift,0);
//  htmlfont(1,0);
  x_text_ib( x_maxx()-206,x_maxy()-15,(unsigned char *)cas2);

  if(mousey>x_maxy()-30 && mousex>x_maxx()-230)
   mouseon();
 }

 if(lasttime[0]) //not if time redraw was forced!
 {
  SecondsSleeping++;
  if(GLOBAL.timeout)
   GLOBAL.secondsleft--;
  if(ppplogtime && tcpip)
   draw_time_online();
 }

 strcpy(lasttime,cas2);
}//end sub



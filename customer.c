
#include "arachne.h"
#include "customer.h"

#define CUSTOMER_Y        20
#define CUSTOMER_BUTTON_X 20

//customer_onbutton will convert coordinates to event number
int customer_onbutton( int x, int y)
{
 if(y > CUSTOMER_Y )
  return 0;

 switch(x/CUSTOMER_BUTTON_X)
 {
  case 0:
  return CUSTOMER_BUTTON_1;
  case 1:
  return CUSTOMER_BUTTON_2;
 }
 return 0;
}

//define size of HTML rendering area
void customer_zoom(void)
{
 //p->htscrn_xtop=0; //currently this must be always zero
 p->htscrn_xsize=x_maxx()-user_interface.scrollbarsize-1;
 p->htscrn_ysize=x_maxy()-CUSTOMER_Y+17;
 p->htscrn_ytop=CUSTOMER_Y;
}

//draw anything you want into area outside HTML rendering area
void customer_draw(void)
{
 initpalette();
 DrawIconLater( "TOOLBAR",0,0 );
}

//optinal help on status bar for certain customer defined events
int customer_onlinehelp(int event)
{
 switch(event)
 {
  case CUSTOMER_BUTTON_1:
  outs("This is button 1");
  break;

  case CUSTOMER_BUTTON_2:
  outs("This is button 2");
  break;

  default:
  defaultmsg();
  return 0;
 }
return 1;

}

//return 1 if new GLOBAL.location was set....
int customer_event(int choice)
{
 switch(choice)
 {
  case CUSTOMER_BUTTON_1:
  strcpy(GLOBAL.location,"file://help.htm");
  return 1;

  case CUSTOMER_BUTTON_2:
  strcpy(GLOBAL.location,"about:");
  return 1;
 }
 return 0;
}

//return 1 if new GLOBAL.location was set....
int customer_URLcheck(void)
{
 if(!strncmpi(GLOBAL.location,"customer:",9))
 {
  //1. do whaetever you like
  Piip();
  //2. specify new GLOBAL.location to jump to && return 1
  strcpy(GLOBAL.location,"file://help.htm");
  return 1;
  //otherwise, behaviour is undefined (will result probably
  //in "protocol not defined error..."
 }
 return 0;
}

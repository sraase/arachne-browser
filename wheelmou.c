
//========================================================================
//Implementation of Arachne Labs Wheel Mouse Protocol V 1.0
//see http://gnu.arachne.cz/ for driver and specification
//========================================================================

#include "arachne.h"
#include "gui.h"
#include "html.h"

int wheelqueue=0;
int analysewheel(int mys)
{
 int wheel=mys>>8;

 if(!wheel)
  return mys;

 while(wheel)
 {
  int key=0;

  if(wheel & 0x08)
  {
   wheel^=0x0f;
   wheel++;
   if(!htmlpulldown && !activeatomptr)
    smothup(2*wheel);
   else if(htmlpulldown || (activeatomptr->type==INPUT && activeatomptr->data1==TEXTAREA))
    key=UPARROW;
   else
    key=RIGHTARROW;
  }
  else
  {
   if(!htmlpulldown && !activeatomptr)
    smothdown(2*wheel);
   else if(htmlpulldown || (activeatomptr->type==INPUT && activeatomptr->data1==TEXTAREA))
    key=DOWNARROW;
   else
    key=LEFTARROW;
  }

  if(key)
  {
   if(wheel>1)
    wheel*=2;
   while(wheel--)
   {
    if(htmlpulldown)
     GUIEVENT(key,0);
    else
     activeatomtick(key,0);
   }
  }

  wheel=wheelqueue; //any wheel events happened while refreshing ?
  wheelqueue=0;
 }//loop
 return 0;
}

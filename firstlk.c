
#include "arachne.h"
#include "customer.h"
#include "firstlk.h"
#include "gui.h"

int customer_event(int choice)
{
 char *value;

 switch(choice)
 {
     case CLICK_1STLOOK_FIRSTLOOK:
		value = configvariable(&ARACHNEcfg,"1stLook_Bits2GoPage",NULL) ;
		if (value)
			strcpy(GLOBAL.location,value);
		return 1;
     case CLICK_1STLOOK_CONTACTS:
		value = configvariable(&ARACHNEcfg,"1stLook_ContactsPage",NULL) ;
  		if (value)
  			strcpy(GLOBAL.location,value);
	///	strcpy (GLOBAL.location,"file:bits2go\\contact.htm");
		return 1;
     case CLICK_1STLOOK_CALENDER:
		value = configvariable(&ARACHNEcfg,"1stLook_CalenderPage",NULL) ;
		if ( value )
			strcpy(GLOBAL.location,value);
		return 1;
     case CLICK_1STLOOK_NOTES:
		value = configvariable(&ARACHNEcfg,"1stLook_NotesPage",NULL) ;
		if (value)
			strcpy(GLOBAL.location,value);
		return 1;
     case CLICK_1STLOOK_EMAIL:
		value = configvariable(&ARACHNEcfg,"1stLook_EmailPage",NULL) ;
		if (value)
			strcpy(GLOBAL.location,value);
		return 1;
    case CLICK_1STLOOK_TASKS:
		value = configvariable(&ARACHNEcfg,"1stLook_TasksPage",NULL) ;
		if (value)
			strcpy(GLOBAL.location,value);
		return 1;
     case CLICK_1STLOOK_VIEWFINDER:
		value = configvariable(&ARACHNEcfg,"1stLook_ViewFinderPage",NULL) ;
		if (value)
			strcpy(GLOBAL.location,value);
		return 1;

 }
return 0;
}

int customer_onbutton( int x, int y)
{
 if(y > TOP_Y + BTN_HEIGHT )	 //. 7+40 = 47
	 return 0;

 x-=TOP_X;
 /*
 if(x>       10 * BTN_WIDTH)
	 return CLICK_1STLOOK_LOGO ;
 else if(x > 9 * BTN_WIDTH )
	 return CLICK_EXIT ;
 if(x>       8 * BTN_WIDTH)
	 return CLICK_1STLOOK_VIEWFINDER ;
         */

 if(x > x_maxx()-5-BTN_WIDTH)
        return CLICK_EXIT;
 else if(x > x_maxx()-5-2*BTN_WIDTH)
        return CLICK_HELP;

 else if(x > 7 * BTN_WIDTH )
        return 0;
	 ///return CLICK_EXIT;	//. for 640 screen temp
 else if(x > 6 * BTN_WIDTH )
	 return CLICK_EXIT;
 else if(x > 5 * BTN_WIDTH )
         return 0 ;
 else if(x > 4 * BTN_WIDTH )
	 return 0;
 else if(x > 3 * BTN_WIDTH )
	 return CLICK_RELOAD;
 else if(x > 2 * BTN_WIDTH )
	 return CLICK_ABORT;
 else if(x > BTN_WIDTH )
	 return CLICK_NEXT;
else
 return CLICK_PREVIOUS ;
}

void customer_draw(void)
{
/*
#define TOP_Y 7

  char str[10];
*/
  //. Draw Bits2Go buttons

  // to do something ONLY in HiColor modes use that conditional:
  // if(xg_256 == MM_Hic)

  initpalette();
  DrawIconLater( "TOOLBAR1",0,0 );
  DrawIconLater( "TOOLBAR2",x_maxx()-399,0 );
/*
  sprintf(str,"v_prev");
  DrawIconLater( str,0,TOP_Y );
  sprintf(str,"v_next");
  DrawIconLater( str,BTN_WIDTH,TOP_Y );
  sprintf(str,"v_b2g");  //. USA today
  DrawIconLater( str,2*BTN_WIDTH,TOP_Y );
  sprintf(str,"v_contac");
  DrawIconLater( str,3*BTN_WIDTH,TOP_Y );
  sprintf(str,"v_calend");
  DrawIconLater( str,4*BTN_WIDTH,TOP_Y );
  sprintf(str,"v_tasks");
  DrawIconLater( str,5*BTN_WIDTH,TOP_Y );
  sprintf(str,"v_notes");
  DrawIconLater( str,6*BTN_WIDTH,TOP_Y );
  sprintf(str,"v_email");
  DrawIconLater( str,7*BTN_WIDTH,TOP_Y );
  sprintf(str,"v_viewf");
  DrawIconLater( str,8*BTN_WIDTH,TOP_Y );
  sprintf(str,"v_exit");
  DrawIconLater( str,9*BTN_WIDTH,TOP_Y );
  sprintf(str,"v_logo");
  DrawIconLater( str,10*BTN_WIDTH,TOP_Y );
*/
//  Box3D(0,0,x_maxx()-1,28);
}

void customer_zoom(void)
{
 p->htscrn_xsize=x_maxx()-user_interface.scrollbarsize-1;
// p->htscrn_ysize=x_maxy()-67;
// p->htscrn_ytop=50;
 p->htscrn_ysize=x_maxy()-47;
 p->htscrn_ytop=30;
}

int customer_URLcheck(void)
{
 if(!strncmpi(GLOBAL.location,"vadem:",6))
 {
  //1. do whaetever you like
  putch(7);
  //2. specify new GLOBAL.location to jump to && return 1
  strcpy(GLOBAL.location,"file:help.htm");
  return 1;
  //otherwise, behaviour is undefined (will result probably
  //in "protocol not defined error..."
 }
 return 0;
}

int customer_onlinehelp(int b)
{
 switch(b)
 {
  case CLICK_1STLOOK_CONTACTS:
  outs("These are your contacts - e-mails, phons, etc.");
  break;

  default:
  defaultmsg();
  return 0;
 }
return 1;
}


#ifdef CUSTOMER
char *VER="0.1";
char *beta=";alpha";
char *copyright="Copyright (c)1999 Vadem";
char *homepage="http://www.vadem.com/";

char customerpalette[48]={0,0,0, 10,10,20, 14,14,28, 17,17,32, 32,32,50,
25,25,38, 28,28,45, 51,51,51, 38,38,38, 20,20,36, 43,31,38, 63,38,38, 36,36,54,
42,42,63, 63,63,10, 63,63,63};

#endif

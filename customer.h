
//======================================================================
//Header file for customer-defined user interface for Arachne
//======================================================================

//customer_onbutton converts coordinates to event number
int customer_onbutton( int x, int y);

//customer_zoom defines size of HTML page on screen
void customer_zoom(void);

//customer_draw draws customized user interface on screen
void customer_draw(void);

//output online help for specified event number
int customer_onlinehelp(int event);

//convert event numbers to URLs
//return 1 if new GLOBAL.location was set....
int customer_event(int choice);

//check URL for cutomer-locked prefix (eg. URL filter, protocol "customer:"..
int customer_URLcheck(void);

#define CUSTOMER_BUTTON_1 1001
#define CUSTOMER_BUTTON_2 1002
#define CUSTOMER_BUTTON_3 1003
#define CUSTOMER_BUTTON_4 1004
#define CUSTOMER_BUTTON_5 1005
#define CUSTOMER_BUTTON_6 1006
#define CUSTOMER_BUTTON_7 1007
#define CUSTOMER_BUTTON_8 1008
#define CUSTOMER_BUTTON_9 1009
#define CUSTOMER_BUTTON_10 1010

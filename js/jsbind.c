
//========================================================================
//JavaScript for Arachne by xChaos & Homeless
//========================================================================

#include "arachne.h
#include "javascr.h

int js_init(void)
{
 js_bind_str("navigator","version","5.0");
 js_bind_str("navigator","name","Arachne (Netscape/Mozilla spoofer)");

 js_bind_a2js("navigator","status",arachne2js_htmlmsg);
 js_bind_js2a("document","href",js2arachne_gotolocation);

}


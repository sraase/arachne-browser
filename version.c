#include "arachne.h"

#ifndef CUSTOMER
char *VER="1.75";

//!!glennmcc: Begin Apr 06, 2003--- NOKEY==GPL now
#ifndef NOKEY
//!!glennmcc: end

#ifdef MSDOS
char *beta=";UE03";          //MS-DOS version is stable
#else
char *beta=";beta";    //for other platforms, not even beta....
#endif

#else
char *beta=";GPL";
#endif//!!glennmcc: endif for '#ifdef NOKEY' above

#ifdef NOKEY//!!glennmcc: Apr 06,2003 --- NOKEY==GPL now
char *copyright="";
#else
char *copyright="Copyright (c)1996-2002 Michael Polak, Arachne Labs";
#endif//!!glennmcc: endif

char *homepage="http://arachne.cz/";

#endif


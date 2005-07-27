#include "arachne.h"

#ifndef CUSTOMER
char *VER="1.85";

//!!glennmcc: Begin Apr 06, 2003--- NOKEY==GPL now
#ifndef NOKEY
//!!glennmcc: end

#ifdef MSDOS
char *beta=";UE08";          //MS-DOS version is stable
#else
char *beta=";beta";    //for other platforms, not even beta....
#endif

#else
char *beta=";GPL,386+";//!!glennmcc: Jun 02, 2004 --- added ,386+
#endif//!!glennmcc: endif for '#ifndef NOKEY' above

#ifdef NOKEY//!!glennmcc: Apr 06,2003 --- NOKEY==GPL now
char *copyright="";
#else
char *copyright="Copyright (c)1996-2002 Michael Polak, Arachne Labs";
#endif//!!glennmcc: endif

char *homepage="http://arachne.cz/";

#endif

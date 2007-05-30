#include "arachne.h"

#ifndef CUSTOMER
char *VER="1.91";

//!!glennmcc: Begin Apr 06, 2003--- NOKEY==GPL now
#ifndef NOKEY

#ifdef MSDOS
char *beta=";UE11.r33"; //MS-DOS version is stable
#else
char *beta=";beta";    //for other platforms, not even beta....
#endif//ifndef NOKEY

#else
char *beta=";GPL,387+";
#endif//ifndef NOKEY

#ifdef NOKEY//!!glennmcc: Apr 06,2003 --- NOKEY==GPL now
#ifdef BEAR
char *ident=" (Bear)";
#else
#ifdef ERIC
char *ident=" (Eric)";
#else
#ifdef GREGY
char *ident=" (Gregy)";
#else
char *ident="";
#endif//ifdef Gregy
#endif//ifdef Eric
#endif//ifdef Bear
char *copyright="";//ifdef NOKEY
#else
char *copyright="Copyright (c)1996-2002 Michael Polak, Arachne Labs";
#endif//ifdef NOKEY

char *homepage="http://arachne.cz/";

#endif//ifndef CUSTOMER
